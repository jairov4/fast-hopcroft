// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

#include <stdint.h>
#include <assert.h>
#include <list>
#include <string>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <queue>
#include "Dfa.h"

/// Hopcroft's DFA Minimization Algorithm.
template<class TState, class TSymbol, class TToken = uint64_t>
class MinimizationHopcroft
{	
public:
	typedef TState TStateSize;
	typedef Dfa<TState, TSymbol, TToken> TDfa;
	typedef typename TDfa::TSet TSet;
	typedef std::pair<TStateSize,TStateSize> TPartition;
	typedef std::vector<TPartition> TPartitionSet;

private:
	
	std::string to_string(const TPartition& P, const std::vector<TState>& Pcontent)
	{
		using namespace std;
		string str;
		str.append("{");
		for(TStateSize j=P.first, cnt=0; cnt<P.second; j++, cnt++)
		{
			if(cnt > 0) str += ", ";
			str.append(std::to_string((uint64_t)Pcontent[j]));
		}
		str.append("}");
		return str;
	}

	std::string to_string(const TPartitionSet& p, TStateSize size, const std::vector<TStateSize>& Pcontent)
	{
		using namespace std;
		string str;
		int cnt = 0;
		str.append("{");
		for_each(p.cbegin(), p.cbegin()+size, [&](const TPartition& x)
		{
			if(cnt++ > 0) str.append(", ");
			str.append(to_string(x, Pcontent));
		});
		str.append("}");
		return str;
	}
	
public:

	/// Controls the debugging info output
	bool ShowConfiguration;

	MinimizationHopcroft()
		:ShowConfiguration(true)
	{
	}

	void Minimize2(const TDfa& dfa)
	{
		using namespace std;

		TStateSize final_states_count = (TStateSize)dfa.Final.count();
		TStateSize non_final_states_count = (TStateSize)dfa.GetStates() - final_states_count;

		// Un automata sin estado final?
		assert(final_states_count > 0);

		// cantidad de estados en la particion y particion
		// Maximo puede exisitir una particion por cada estado, por ello reservamos de esta forma
		vector<TState> Pcontent(dfa.GetStates());
		TPartitionSet P(dfa.GetStates());
		
		P[0].first = 0;
		P[0].second = final_states_count;

		P[1].first = final_states_count;
		P[1].second = non_final_states_count;

		// Inicializa funcion inversa para obtener la particion a la que pertenece un estado
		vector<TStateSize> state_to_partition(dfa.GetStates());		

		auto it_f = Pcontent.begin();
		auto it_nf = Pcontent.rbegin();
		for(TState st=0; st<dfa.GetStates(); st++)
		{
			if(dfa.IsFinal(st)) *it_f++ = st;
			else *it_nf++ = st;
			state_to_partition[st] = dfa.IsFinal(st) ? 0 : 1;
		}

		TStateSize new_index = 2;

		TStateSize min_initial_partition_index = final_states_count < non_final_states_count ? 0 : 1;

		// conjunto de espera
		TSet wait_set_membership(dfa.GetStates());
		wait_set_membership.set(min_initial_partition_index);

		TSet partitions_to_split(dfa.GetStates());

		// conjunto de predecesores
		TSet predecessors(dfa.GetStates());

		// El peor caso de W es cuando cada division 
		// añada un elemento por cada estado (y por cada letra, claro esta)
		for(TStateSize splitter_set=(TStateSize)wait_set_membership.find_first(); splitter_set!=(TStateSize)TSet::npos; splitter_set=(TStateSize)wait_set_membership.find_first())
		{
			assert(new_index < dfa.GetStates());

			// remove current
			wait_set_membership.reset(splitter_set);

			const auto& splitter_partition = P[splitter_set];
			
			for(TSymbol splitter_letter=0; splitter_letter<dfa.GetAlphabethLength(); splitter_letter++)
			{
				
				if(ShowConfiguration)
				{
					cout << "P=" << to_string(P, new_index, Pcontent) << endl;
					cout << "Spliter=" << to_string(splitter_partition, Pcontent) << ", symbol=" << (uint64_t)splitter_letter << endl;
				}

				predecessors.reset();
				
				// recorre elementos de la particion
				auto partition_it_begin = Pcontent.begin() + splitter_partition.first;
				auto partition_it_end = partition_it_begin + splitter_partition.second;
				for(; partition_it_begin != partition_it_end; partition_it_begin++)
				{
					TState state = *partition_it_begin;
					predecessors |= dfa.GetPredecessor(state, splitter_letter);
				}

				partitions_to_split.reset();

				// let a=splitter_letter, B belongs P
				// O(card(a^{-1}.B))				
				for(TStateSize ss=(TStateSize)predecessors.find_first(); ss!=(TStateSize)TSet::npos; ss=(TStateSize)predecessors.find_next(ss))
				{
					TStateSize partition_index = state_to_partition[ss];
					if(partitions_to_split.test(partition_index)) continue;

					partitions_to_split.set(partition_index);
					
					auto& partition_desc = P[partition_index];

					const TStateSize partition_size = partition_desc.second;
					if(partition_size == 1) continue;

					if(ShowConfiguration)
					{
						cout << "pred state=" << (uint64_t)ss << " in partition " << partition_index << endl;
					}

					const auto partition_it_original_begin = Pcontent.begin() + partition_desc.first;
					
					partition_it_begin = partition_it_original_begin;					
					partition_it_end = partition_it_begin + partition_desc.second - 1;
					// itera intentando particionar
					do
					{
						TState state = *partition_it_begin;
						if(predecessors.test(state))
						{
							partition_it_begin++;							
						}
						else 
						{
							iter_swap(partition_it_begin, partition_it_end);
							state_to_partition[state] = new_index;
							partition_it_end--;
						}
					} while(partition_it_begin != partition_it_end);
					
					// queda un elemento sin elegir bando
					{ 
						TState state = *partition_it_begin;
						if(predecessors.test(state)) partition_it_begin++;
						else state_to_partition[state] = new_index;
					}

					// tamaño en el que quedó la particion vieja
					TStateSize split_size = (TStateSize)(partition_it_begin - partition_it_original_begin);
					
					// si no hubo particionamiento continuar
					if(split_size == partition_size) continue;
					
					// tamaño en el que quedo la nueva particion
					TStateSize split_complement_size = partition_size - split_size;

					partition_desc.second = split_size;

					// confirm descriptor for new partition
					P[new_index].first = partition_desc.first + split_size;
					P[new_index].second = split_complement_size;

					// Si uno de los dos es 1 es el menor pero no se puede dividir mas, 
					// por lo tanto no vale la pena añadirlo al conjunto de espera
					if(split_size != 1 && split_complement_size != 1)
					{
						auto add_index = split_size < split_complement_size ? splitter_set : new_index;
						wait_set_membership.set(add_index);
					}
					
					// si ha llegado hasta aqui entonces hizo division, el indice para la nueva particion debe elevarse
					new_index++;
				}
			}
		}
		cout << "Finished, " << new_index << " states of " << dfa.GetStates() << endl;
		if(ShowConfiguration)
		{
			cout << "Final P=" << to_string(P, new_index, Pcontent) << endl;
		}
	}
};
