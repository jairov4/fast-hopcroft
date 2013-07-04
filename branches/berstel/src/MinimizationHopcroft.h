// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <stdint.h>
#include <assert.h>
#include <list>
#include <string>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <typeinfo>
#include "Dfa.h"

template<class T1, class T2>
struct p_hash
{
	std::size_t operator()(std::pair<BitSet<T1,T2>, char> const& p)
	{
		BitSet<T1,T2>::hash bsHasher;
		return bsHasher(p.first) ^ std::hash_value(p.second);
	}
};

/// Hopcroft's DFA Minimization Algorithm.
template<class TState, class TSymbol, class TToken = uint64_t>
class MinimizationHopcroft
{	
public:
	typedef BitSet<TState, TToken> TSet;
	typedef Dfa<TState, TSymbol, TToken> TDfa;
	typedef std::pair<TSet, TSymbol> TSplitter;	
	typedef std::unordered_set<TSplitter, p_hash<TState, TToken>> TSplitterSet;
	typedef std::list<TSet> TPartition;

private:

	/// Extract one member of partition.
	/// O(1)
	TSplitter ExtractOne(TSplitterSet& lset)
	{		
		TSplitter r = *lset.begin();
		lset.erase(lset.begin());
		return r;
	}
		
	/// Util to get a printable string representing the Set
	std::string SetToString(const TSet& set)
	{
		int cnt = 0;
		std::string str;
		str.append("{");
		set.ForEachMember( [&cnt, &str] (TState i)
		{
			if(cnt++ > 0) str += ", ";			
			str.append(std::to_string(i));
			return true;
		});
		str.append("}");
		return str;
	}

	/// Util to get a printable string representing the partition
	std::string PartitionToString(const TPartition& p)
	{
		std::string str;
		int cnt = 0;
		str.append("{");
		std::for_each(p.cbegin(), p.cend(), [&](const TSet& x)
		{
			if(cnt++ > 0) str.append(", ");			
			str.append(SetToString(x));			
		});
		str.append("}");
		return str;
	}

	std::string SplitterToString(const TSplitter& c)
	{
		std::string str;		
		str.append("(");
		str.append(SetToString(c.first));
		str.append(", ");
		str.append(std::to_string((uint64_t)c.second));
		str.append(")");
		return str;
	}

	std::string SplitterSetToString(const TSplitterSet& s)
	{
		std::string str;
		int cnt = 0;
		str.append("{");
		for(auto c : s)
		{
			if(cnt++ > 0) str.append(", ");
			str.append(SplitterToString(c));
		}
		str.append("}");
		return str;
	}

	/// Split operation, Refine B' using the Splitter (B, a) resulting B1 and B2.
	/// <math>(B1, B2) = B' \ (B,a)</math>
	void Split(const TDfa& dfa, const TSet& Bc, const TSet& B, TSymbol a, TSet& B1, TSet& B2)
	{
		// En 'B1' los estados de 'Bc' que llegan a 'B' consumiendo 'a'
		// O(N) -> N estados de B
		B1.Clear();		
		B.ForEachMember([&, a](TState q)
		{
			// O(1)
			const TSet& pred = dfa.GetPredecessor(q, a);
			// O(1)
			B1.UnionWith(pred);
			return true;
		});
		B2 = B1;
		B1.IntersectWith(Bc);
		B2.Complement();
		B2.IntersectWith(Bc);
	}

public:

	/// Controls the debugging info output
	bool ShowConfiguration;

	MinimizationHopcroft()
		:ShowConfiguration(true)
	{
	}

	/// Entry point to minimize any DFA
	void Minimize(const TDfa& dfa) 
	{
		// P = { F, ~F }
		TPartition P;
		P.push_front(dfa.Final);

		TSet t = dfa.Final;
		t.Complement();
		P.push_front(t); 

		// L = { min(P) }
		unsigned final_states_count = dfa.Final.Count();
		unsigned non_final_states_count = dfa.GetStates() - final_states_count;

		// Un automata sin estado final?
		assert(final_states_count > 0);

		TSplitterSet W(dfa.GetStates() * dfa.GetAlphabethLength());
		for(TSymbol a=0; a < dfa.GetAlphabethLength(); a++)
		{
			if(final_states_count < non_final_states_count) 
			{
				W.emplace(dfa.Final, a);
			} else {
				W.emplace(t, a);
			}
		}

		TSet B1(dfa.GetStates()), B2(dfa.GetStates());
		while (!W.empty())
		{
			const TSplitter S = ExtractOne(W);
			if(ShowConfiguration) 
			{
				std::cout << "S=" << SplitterToString(S) << std::endl;
			}
			for(auto B = P.begin(); B != P.end(); )
			{				
				if(ShowConfiguration) 
				{
					std::cout << "Test B=" << SetToString(*B) << std::endl;
				}

				// O(Card(S.first))
				Split(dfa, *B, S.first, S.second, B1, B2);
				// was there split?
				if(B1.IsEmpty() || B2.IsEmpty()) 
				{
					B++; // advance B
					continue;
				}

				if(ShowConfiguration)
				{
					std::cout << "B1=" << SetToString(B1) << std::endl;
					std::cout << "B2=" << SetToString(B2) << std::endl;
				}

				for(TSymbol b=0; b<dfa.GetAlphabethLength(); b++)
				{
					auto f = W.find(TSplitter(*B, b));
					if(f != W.end())
					{
						W.erase(f);
						W.emplace(B1, b);
						W.emplace(B2, b);
					} else {
						auto B1count = B1.Count();
						auto B2count = dfa.GetStates() - B1count;
						if(B1count < B2count) 
						{
							W.emplace(B1, b);
						} else {
							W.emplace(B2, b);
						}						
					}
				}

				// B1 y B2 must be next iterables
				B = P.erase(B); // advance B
				B = P.insert(B, B1);
				B = P.insert(B, B2);

				if(ShowConfiguration)
				{
					std::cout << "P=" << PartitionToString(P) << std::endl;
					std::cout << "W=" << SplitterSetToString(W) << std::endl;
				}
			}
			if(W.size() % 1000 == 0) std::cout << W.size() << " splitters remaining" << std::endl;
		}
		if(ShowConfiguration)
		{			
			std::cout << "FINALLY P=" << PartitionToString(P) << std::endl;
		}
	}

	void Minimize2(const TDfa& dfa)
	{
		// P = { F, ~F }
		TPartition P;
		P.push_front(dfa.Final);
		auto t = dfa.Final;
		t.Complement();
		P.push_front(t); 

		// L = { min(P) }
		TPartition L;
		L.push_front(dfa.Final);

		// split sets
		TSet B1(dfa.GetStates()), B2(dfa.GetStates());
		int countB1, countB2;

		// O(1) empty test
		while(!L.empty())
		{
			auto S = L.back();
			L.pop_back(); // extraction O(1)                      
			for(TSymbol a = 0; a < dfa.GetAlphabethLength(); a++)
			{
				if(ShowConfiguration) 
				{
					auto strS = SetToString(S);
					auto stra = std::to_string((uint64_t)a);
					std::cout << "Splitter = (S:" << strS << ", a:" << stra << ")" << std::endl;
				}
				auto Biter = P.begin();
				auto Bend = P.end();
				while(Biter != Bend)
				{
					const auto& B = *Biter;
					if(ShowConfiguration)
					{
						auto strB = SetToString(B);
						std::cout << "-> B:" << strB << std::endl;
					}

					B1.Clear(); // O(Nmax) -> constant respect to N
					B2.Clear(); // O(Nmax) -> constant respect to N
					Split(dfa, B, S, a, B1, B2); // O(N) -> N elements within B

					auto countB1 = B1.Count();
					auto countB2 = B2.Count();
					// was there split?
					if(countB1 != 0 && countB2 != 0)
					{
						// care, it destroys B, do not reference B beyond this point
						Biter = P.erase(Biter); // O(1)
						Biter = P.insert(Biter, B1); // O(1)
						Biter = P.insert(Biter, B2); // O(1)
						if(countB1 < countB2)
						{
							L.push_back(B1); // O(1)
						} 
						else 
						{
							L.push_back(B2); // O(1)
						}
						if(ShowConfiguration) 
						{
							auto strB1 = SetToString(B1);
							auto strB2 = SetToString(B2);
							auto strL = PartitionToString(L);
							auto strP = PartitionToString(P);

							std::cout 
								<< "+ Splits" << std::endl
								<< " B1:" << strB1 << " B2:" << strB2 << std::endl 
								<< " L:" << strL << " P:" << strP << std::endl;
						}
					} 
					else 
					{
						Biter++;
					}                                                                               
				}
			}
		}
	}
};
