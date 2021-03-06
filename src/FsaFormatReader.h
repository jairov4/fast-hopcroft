#pragma once

#include <stdexcept>
#include <string>
#include <map>
#include <unordered_set>
#include <vector>
#include <istream>
#include <boost/lexical_cast.hpp>
#include <stdint.h>
#include "IFsaReader.h"

template<typename TFsa>
class FsaFormatReader : public IFsaReader<TFsa>
{
protected:
	int state;
	bool second_step;
	std::string token;

	int number_states;
	int source;
	int destination;
	char symbol;

	std::unordered_set<int> initial_states;
	std::unordered_set<int> final_states;
	std::map<char, int> alphabet;
	std::vector<std::tuple<int,char,int>> transitions;	

	void expect(std::string ref)
	{
		if(token == ref) state++;
		else 
		{
			auto msg = std::string("Se esperaba el literal: ")+ref; 
			throw std::invalid_argument(msg.c_str()); 
		}
	}

	void read(int& i)
	{
		if(!std::all_of(token.begin(), token.end(), isdigit))
		{
			throw std::invalid_argument("Se esperaba token numerico");
		}
		i = boost::lexical_cast<int,std::string>(token);
		state++;
	}

	void read(char& i)
	{
		if(token.size() != 1 || !isalpha(token[0]))
		{
			throw std::invalid_argument("Se esperaba token caracter");
		}
		i = token[0];
		state++;
	}

	bool is_numeric()
	{
		return isdigit(token[0]) != 0;
	}

	void process_token()
	{		
		int i;
		switch (state)
		{
		case 0: expect("fa"); break;
		case 1: expect("("); break;
		case 2: expect("r"); break;
		case 3: expect("("); break;
		case 4: expect("fsa_frozen"); break;
		case 5: expect(")"); break;
		case 6: expect(","); break;
		case 7: read(number_states); break;
		case 8: expect(","); break;

			// int collection
		case 9: expect("["); break;
		case 10: read(i); 
			if(!second_step) 
			{
				initial_states.emplace(i); 
			} else {
				final_states.emplace(i); 
			}
			break;
		case 11: 
			if(token == ",") 
			{
				expect(","); 
				state = 10; // goto 10
			} else { 
				expect("]"); 
			} 
			break;
		case 12: 
			expect(","); 
			if(second_step) 
			{
				state = 13; // goto 13
			} else { 
				second_step = true; 
				state = 9; // goto 9
			} 
			break;

			// trans collection
		case 13: expect("["); break;
		case 14: expect("trans"); break;
		case 15: expect("("); break;
		case 16: read(source); break;
		case 17: expect(","); break;
		case 18: read(symbol); break;
		case 19: expect(","); break;
		case 20: read(destination); break;
		case 21: 
			expect(")"); 
			transitions.push_back(std::tuple<int,char,int>(source, symbol, destination)); 
			alphabet.emplace(symbol, 0);
			break;
		case 22: 
			if(token == ",") 
			{ 
				expect(","); 
				state = 14; // goto 14
			} else { 
				expect("]"); 
			} 
			break;

		case 23: expect(","); break;

			// empty collection
		case 24: expect("["); break;
		case 25: expect("]"); break;
		case 26: expect(")"); break;
		case 27: expect("."); break;

		default:
			break;
		}
	}

public:
	virtual void ReadHeader(std::istream& is) override
	{
	}

	virtual TFsa Read(std::istream& is) override
	{
		using namespace std;
		state = 0;
		second_step = false;
		initial_states.clear();
		final_states.clear();
		transitions.clear();
		alphabet.clear();

		// lexer
		while(is.good())
		{
			char c;
			is.read(&c, 1);

			if(c == '%') 
			{
				if(!token.empty()) process_token();
				string line;				
				getline(is, line);	// skip line			
				token.clear();
			}
			else if(isspace(c))
			{
				if(!token.empty()) process_token();
				token.clear();
			}
			else if(isalpha(c) || c=='_') 
			{
				if( !token.empty() && !(isalpha(token.back())||(token.back()=='_')) ) 
				{
					process_token();
					token.clear();
				}
				token.push_back(c);
			}
			else if(isdigit(c)) 
			{
				if(!token.empty() && !isdigit(token.back())) 
				{
					process_token();
					token.clear();
				}
				token.push_back(c);
			} else {
				if(!token.empty()) process_token();
				token.clear();
				token.push_back(c);
			}
		}

		unsigned alpha_size = (unsigned)alphabet.size();
		TFsa fsm(alpha_size, number_states);
		for(auto i : initial_states) fsm.SetInitial(i);
		for(auto j : final_states) fsm.SetFinal(j);
		int l = 0;
		for(auto k : alphabet) 
		{
			alphabet[k.first] = l++;
		}
		for(auto m : transitions)
		{
			fsm.SetTransition(get<0>(m), alphabet[get<1>(m)], get<2>(m));
		}

		return fsm;
	}
};
