// June 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#include "MinimizationHopcroft.h"
#include "Dfa.h"
#include "DfaGenerator.h"
#include "DfaGraphVizExporter.h"
#include "AfdParser.h"
#include <fstream>
#include <boost/timer/timer.hpp>

using namespace std;

void test1()
{	
	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	MinimizationHopcroft<TState, TSymbol> mini;	
	DfaGraphVizExporter<TState, TSymbol> exporter;	
	Dfa<TState, TSymbol> dfa(2, 4);
		
	//   / 2 \
	// 0 - 1 - 3
	dfa.SetInitial(0);
	dfa.SetFinal(3);

	dfa.SetTransition(0, 0, 1);
	dfa.SetTransition(0, 1, 0);

	dfa.SetTransition(1, 0, 3);
	dfa.SetTransition(1, 1, 1);

	dfa.SetTransition(2, 0, 3);
	dfa.SetTransition(2, 1, 2);

	dfa.SetTransition(3, 0, 3);
	dfa.SetTransition(3, 1, 3);


	mini.Minimize(dfa);
	exporter.Export(dfa, ofstream("test1.dot"));
	cout << endl;
}

void test2()
{
	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	MinimizationHopcroft<TState, TSymbol> mini;	
	DfaGraphVizExporter<TState, TSymbol> exporter;	
	//    / 1 - 3
	//  0
	//    \ 2 - 4
	Dfa<TState, TSymbol> dfa(2, 5);

	dfa.SetInitial(0);
	dfa.SetFinal(3);
	dfa.SetFinal(4);

	dfa.SetTransition(0, 0, 1);
	dfa.SetTransition(0, 1, 2);

	dfa.SetTransition(1, 0, 3);
	dfa.SetTransition(1, 1, 1);

	dfa.SetTransition(2, 0, 4);
	dfa.SetTransition(2, 1, 2);

	dfa.SetTransition(3, 0, 3);
	dfa.SetTransition(3, 1, 3);

	dfa.SetTransition(4, 0, 4);
	dfa.SetTransition(4, 1, 4);

	mini.Minimize(dfa);
	exporter.Export(dfa, std::ofstream("test2.dot"));
	cout << endl;
}

void test3()
{
	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	MinimizationHopcroft<TState, TSymbol> mini;	
	DfaGraphVizExporter<TState, TSymbol> exporter;	
	// uses zero as invisible null-sink state
	//      2 - 5 - 8  - 11
	//    /
	//   1 -3 - 6 - 9  - 12
	//    \
	//      4 - 7 - 10 - 13
	Dfa<TState, TSymbol> dfa(3, 14);

	dfa.SetInitial(1);
	dfa.SetFinal(11);
	dfa.SetFinal(12);
	dfa.SetFinal(13);

	dfa.SetTransition(1, 0, 2);
	dfa.SetTransition(1, 1, 3);
	dfa.SetTransition(1, 2, 4);

	dfa.SetTransition(2, 0, 5);
	dfa.SetTransition(3, 0, 6);
	dfa.SetTransition(4, 0, 7);

	dfa.SetTransition(5, 0, 8);
	dfa.SetTransition(6, 1, 9);
	dfa.SetTransition(7, 2, 10);

	dfa.SetTransition(8, 0, 11);
	dfa.SetTransition(9, 1, 12);
	dfa.SetTransition(10, 2, 13);

	mini.Minimize(dfa);
	exporter.Export(dfa, std::ofstream("test3.dot"), false);		
	cout << endl;
}

void test4()
{
	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	MinimizationHopcroft<TState, TSymbol> mini;	
	DfaGraphVizExporter<TState, TSymbol> exporter;	
	// uses zero as invisible null-sink state
	//      2 - 5 - 8  - 11
	//    /       \   /
	//   1 -3 - 6 - 9  - 12
	//    \       \   \
	//      4 - 7 - 10 - 13
	Dfa<TState, TSymbol> dfa(3, 14);

	dfa.SetInitial(1);
	dfa.SetFinal(11);
	dfa.SetFinal(12);
	dfa.SetFinal(13);

	dfa.SetTransition(1, 0, 2);
	dfa.SetTransition(1, 1, 3);
	dfa.SetTransition(1, 2, 4);

	dfa.SetTransition(2, 0, 5);
	dfa.SetTransition(3, 0, 6);
	dfa.SetTransition(4, 0, 7);

	dfa.SetTransition(5, 0, 8);		
	dfa.SetTransition(5, 1, 9);		
	dfa.SetTransition(5, 2, 9);		

	dfa.SetTransition(6, 1, 9);
	dfa.SetTransition(6, 0, 10);
	dfa.SetTransition(6, 2, 10);

	dfa.SetTransition(7, 0, 10);
	dfa.SetTransition(7, 1, 10);
	dfa.SetTransition(7, 2, 10);

	dfa.SetTransition(8, 0, 11);
	dfa.SetTransition(8, 1, 11);
	dfa.SetTransition(8, 2, 11);

	dfa.SetTransition(9, 0, 11);
	dfa.SetTransition(9, 1, 12);
	dfa.SetTransition(9, 2, 13);

	dfa.SetTransition(10, 0, 13);
	dfa.SetTransition(10, 1, 13);
	dfa.SetTransition(10, 2, 13);

	mini.Minimize(dfa);
	exporter.Export(dfa, std::ofstream("test4.dot"), false);		
	cout << endl;
}

void test5()
{
	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	MinimizationHopcroft<TState, TSymbol> mini;		
	AfdParser<TState, TSymbol> parser;
	vector<string> files;

	files.push_back("afd\\000_n512k2.afd");
	files.push_back("afd\\000_n1024k2.afd");
	files.push_back("afd\\000_n2046k2.afd");
	files.push_back("afd\\000_n4092k2.afd");
	files.push_back("afd\\000_n8192k2.afd");
	files.push_back("afd\\000_n16384k2.afd");

	files.push_back("afd\\001_n512k2.afd");
	files.push_back("afd\\001_n1024k2.afd");
	files.push_back("afd\\001_n2046k2.afd");
	files.push_back("afd\\001_n4092k2.afd");
	files.push_back("afd\\001_n8192k2.afd");
	files.push_back("afd\\001_n16384k2.afd");

	files.push_back("afd\\002_n512k2.afd");
	files.push_back("afd\\002_n1024k2.afd");
	files.push_back("afd\\002_n2046k2.afd");
	files.push_back("afd\\002_n4092k2.afd");
	files.push_back("afd\\002_n8192k2.afd");
	files.push_back("afd\\002_n16384k2.afd");

	files.push_back("afd\\003_n512k2.afd");
	files.push_back("afd\\003_n1024k2.afd");
	files.push_back("afd\\003_n2046k2.afd");
	files.push_back("afd\\003_n4092k2.afd");
	files.push_back("afd\\003_n8192k2.afd");
	files.push_back("afd\\003_n16384k2.afd");

	files.push_back("afd\\004_n512k2.afd");
	files.push_back("afd\\004_n1024k2.afd");
	files.push_back("afd\\004_n2046k2.afd");
	files.push_back("afd\\004_n4092k2.afd");
	files.push_back("afd\\004_n8192k2.afd");
	files.push_back("afd\\004_n16384k2.afd");

	files.push_back("afd\\005_n512k2.afd");
	files.push_back("afd\\005_n1024k2.afd");
	files.push_back("afd\\005_n2046k2.afd");
	files.push_back("afd\\005_n4092k2.afd");
	files.push_back("afd\\005_n8192k2.afd");
	files.push_back("afd\\005_n16384k2.afd");

	files.push_back("afd\\006_n512k2.afd");
	files.push_back("afd\\006_n1024k2.afd");
	files.push_back("afd\\006_n2046k2.afd");
	files.push_back("afd\\006_n4092k2.afd");
	files.push_back("afd\\006_n8192k2.afd");
	files.push_back("afd\\006_n16384k2.afd");

	files.push_back("afd\\007_n512k2.afd");
	files.push_back("afd\\007_n1024k2.afd");
	files.push_back("afd\\007_n2046k2.afd");
	files.push_back("afd\\007_n4092k2.afd");
	files.push_back("afd\\007_n8192k2.afd");
	files.push_back("afd\\007_n16384k2.afd");

	files.push_back("afd\\008_n512k2.afd");
	files.push_back("afd\\008_n1024k2.afd");
	files.push_back("afd\\008_n2046k2.afd");
	files.push_back("afd\\008_n4092k2.afd");
	files.push_back("afd\\008_n8192k2.afd");
	files.push_back("afd\\008_n16384k2.afd");

	files.push_back("afd\\009_n512k2.afd");
	files.push_back("afd\\009_n1024k2.afd");
	files.push_back("afd\\009_n2046k2.afd");
	files.push_back("afd\\009_n4092k2.afd");
	files.push_back("afd\\009_n8192k2.afd");
	files.push_back("afd\\009_n16384k2.afd");

	ifstream afd;
	ofstream report ("report.txt");
	Dfa<uint16_t, uint8_t> dfa(0,0);
	mini.ShowConfiguration = false;
				
	for(auto filename : files) 
	{
		afd.open(filename);
		if(afd.fail())  
		{
			cout << "No se pudo abrir el archivo: " << filename << endl;
			cout << endl;
			continue;
		}
		dfa = parser.Parse(afd);
		afd.close();

		{
			stringstream str;
			str << "Begin, states: " << dfa.GetStates() << ", alpha: " << dfa.GetAlphabetLength() << endl;
			cout << str.str();
			report << str.str();
		}

		boost::timer::cpu_timer timer;
		timer.start();		
		mini.Minimize(dfa);		
		timer.stop();

		{
			stringstream str;
			str << "Done with " << filename << " elapsed (ms) " << timer.elapsed().wall / 1000000UL << endl;
			cout << str.str();
			report << str.str();
			cout << endl;
		}
	}
}

void test6()
{
	typedef uint16_t TState;
	typedef uint8_t TSymbol;
	typedef MinimizationHopcroft<TState, TSymbol> TMinimizer;
	TMinimizer mini;
	DfaGraphVizExporter<TState, TSymbol> exporter;	
	DfaGenerator<TState, TSymbol> gen;

	for(int a=2; a!=5; a++)
	{
		for(int n=2; n!=10; n++) 
		{
			auto rnd = boost::random::mt19937();
			rnd.seed(static_cast<unsigned int>(std::time(nullptr)));
			auto dfa = gen.Generate(a, n, 1, 1, 0.4f, rnd);
			string filename = string("test6_a") + to_string((size_t)a) + "_n" + to_string((size_t)n);
			
			ofstream generated_stream(filename + ".dot");
			exporter.Export(dfa, generated_stream, false);
			generated_stream.close();

			TMinimizer::TPartitionVector partitions;
			TMinimizer::TStateToPartition state_to_partition;
			mini.Minimize(dfa, partitions, state_to_partition);
			TMinimizer::TDfa ndfa = mini.Synthetize(dfa, partitions, state_to_partition);
			
			ofstream minimized_stream(filename + "_min.dot");
			exporter.Export(ndfa, minimized_stream, false);
			minimized_stream.close();

		}
	}
}

int main(int argc, char** argv)
{		
	/*test1();
	test2();
	test3();
	test4();
	test5();*/
	test6();
	
	return 0;
}