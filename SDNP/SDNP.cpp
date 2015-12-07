#include "stdafx.h"

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "MyDebugNew.h"
#endif

#include <limits>
#include <set>
#include <map>
#include <queue>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "kshortestpaths_2.1/GraphElements.h"
#include "kshortestpaths_2.1/Graph.h"
#include "kshortestpaths_2.1/DijkstraShortestPathAlg.h"
#include "kshortestpaths_2.1/YenTopKShortestPathsAlg.h"
#include "SequenceSolver.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <chrono>
#include <random>
#include <exception>
#include <thread>
#include <boost/lexical_cast.hpp>
#include "Defs.h"
#include "OptionalSequencePolicy.h"
#include "SrcDstPolicies.h"
#include "NetworkSolver.h"

#ifdef _DEBUG
#define new MYDEBUG_NEW
#endif

using namespace std;

void testDijkstraGraph()
{
	Graph* my_graph_pt = new Graph("C:\\Users\\famini.HUN7\\Google Drive\\Visual Studio 2013\\Projects\\SDNP\\Data\\ISPMaps\\1221.r0.graph");
	DijkstraShortestPathAlg shortest_path_alg(my_graph_pt);
	/*BasePath* result =
		shortest_path_alg.get_shortest_path(
		my_graph_pt->get_vertex(0), my_graph_pt->get_vertex(5));
	result->PrintOut(cout);*/
	my_graph_pt->my_remove_vertex(107);
	shortest_path_alg.get_shortest_path_flower(my_graph_pt->get_vertex(5));
}

void testYenAlg()
{
	//Graph my_graph("../data/test_6_2");
	Graph my_graph("kshortestpaths_2.1\\data\\test_sequence");

	YenTopKShortestPathsAlg yenAlg(my_graph, my_graph.get_vertex(0),
		my_graph.get_vertex(3));

	int i = 0;
	while (yenAlg.has_next())
	{
		++i;
		yenAlg.next()->PrintOut(cout);
	}

	// 	System.out.println("Result # :"+i);
	// 	System.out.println("Candidate # :"+yenAlg.get_cadidate_size());
	// 	System.out.println("All generated : "+yenAlg.get_generated_path_size());

}

void testAllPaths()
{
	//Graph my_graph("kshortestpaths_2.1\\data\\test_sequence");
	Graph my_graph("C:\\Users\\famini.HUN7\\Documents\\Visual Studio 2013\\Projects\\SDNP\\Data\\ISPMaps\\3257.r0.graph1");
	YenTopKShortestPathsAlg yenAlg(my_graph, my_graph.get_vertex(116),
		my_graph.get_vertex(222));

	unsigned long i = 0;
	BasePath* temp = nullptr;
	while (yenAlg.has_next())
	{
		++i;
		if (i > 10000)//4294967290)
			break;
		temp = yenAlg.next();// ->PrintOut(cout);
	}

	cout << i << "\n";
	cout << temp->length() << "\n";
}

void readSequencesFromFile(const string& sequencesFileName, vector<VertexVectorPtr>& sequenceVector, Graph& graph)
{
	ifstream sequencesFile(sequencesFileName);
	string line;
	uint i = 0;
	if (sequencesFile.is_open())
	{
		while (getline(sequencesFile, line))
		{
			std::string delimiter = ", ";
			size_t pos = 0;
			std::string vertexNumberStr;
			VertexVectorPtr pSequence(new vector<BaseVertex*>());
			sequenceVector.push_back(pSequence);
			while ((pos = line.find(delimiter)) != std::string::npos) 
			{
				vertexNumberStr = line.substr(0, pos);
				sequenceVector[i]->push_back(graph.get_vertex(boost::lexical_cast<uint>(vertexNumberStr)));
				line.erase(0, pos + delimiter.length());
			}
			sequenceVector[i]->push_back(graph.get_vertex(boost::lexical_cast<uint>(line)));//the last node after the last delimiter
			i++;
		}
		sequencesFile.close();
	}

	else cout << "Unable to open file";
}

void testSequenceSolver(const string& sequencesFile = "")
{
	//Graph my_graph("kshortestpaths_2.1\\data\\test_sequence");
	Graph my_graph(g_graphFileName);
	
	vector<VertexVectorPtr> sequenceVector;
	readSequencesFromFile(sequencesFile, sequenceVector, my_graph);
  
    for (VertexVectorPtr pSequence : sequenceVector)
	{
		vector<BaseVertex*> satisfyingPathWithLoops;
		
		logSequence(*pSequence, "sequence: ");
		VertexSet st;
		EdgeSet st2;
		if (findShortestSatisfyingPathWithLoops(my_graph, *pSequence, st, st2, satisfyingPathWithLoops))
		{
			g_logFile.open(g_logFileName, ios::app);
			g_logFile << "Shortest path: ";
            for (BaseVertex* v : satisfyingPathWithLoops)
			{
				g_logFile << v->getID() << ", ";
			}
			g_logFile << "\n";
			g_logFile << "Shortest path length: " << satisfyingPathWithLoops.size()-1 << "\n"; //length dictated by the number of edges
			g_logFile.close();
		}
		my_graph.recoverAll();

		vector<BaseVertex*> satisfyingPath;
		boost::posix_time::ptime t1 = boost::posix_time::second_clock::local_time();
		VertexSet prohibitedVertices;
		prohibitedVertices.insert(my_graph.get_vertex(9));
		prohibitedVertices.insert(my_graph.get_vertex(18));
		EdgeSet prohibitedEdges;
		bool foundPath = findSatisfyingPath(my_graph, *pSequence, prohibitedVertices, prohibitedEdges, satisfyingPath, true);
		boost::posix_time::ptime t2 = boost::posix_time::second_clock::local_time();
		
		g_logFile.open(g_logFileName, ios::app);
		if (foundPath)
		{
			g_logFile << "Found path: ";
            for (BaseVertex* v : satisfyingPath)
			{
				g_logFile << v->getID() << ", ";
			}
			g_logFile << "\n";
			g_logFile << "Path length: " << satisfyingPath.size()-1 << "\n"; //length dictated by the number of edges
		}
		else
		{
			g_logFile << "No path found :(\n";
		}
		
		boost::posix_time::time_duration diff = t2 - t1;
		g_logFile << "Sequence solving time: " << diff.total_seconds() << endl << endl;
		g_logFile.close();

		my_graph.recoverAll();
	}
}

void testSpecificSequence()
{
	Graph my_graph("C:\\Users\\famini.HUN7\\Documents\\Visual Studio 2013\\Projects\\SDNP\\Data\\ISPMaps\\1221.r0.graph");
	vector<BaseVertex*> satisfyingPath;
	vector<BaseVertex*> sequence;
	/*sequence.push_back(my_graph.get_vertex(71));
	sequence.push_back(my_graph.get_vertex(116));
	sequence.push_back(my_graph.get_vertex(43));
	sequence.push_back(my_graph.get_vertex(24));
	sequence.push_back(my_graph.get_vertex(131));*/
	sequence.push_back(my_graph.get_vertex(221));
	sequence.push_back(my_graph.get_vertex(167));
	sequence.push_back(my_graph.get_vertex(213));

	boost::posix_time::ptime t1 = boost::posix_time::second_clock::local_time();
	VertexSet prohibitedVertices;
	prohibitedVertices.insert(my_graph.get_vertex(9));
	EdgeSet prohibitedEdges;
	bool foundPath = findSatisfyingPath(my_graph, sequence, prohibitedVertices, prohibitedEdges, satisfyingPath, true);
	boost::posix_time::ptime t2 = boost::posix_time::second_clock::local_time();

	g_logFile.open(g_logFileName, ios::app);
	if (foundPath)
	{
		g_logFile << "Found path: ";
        for (BaseVertex* v : satisfyingPath)
		{
			g_logFile << v->getID() << ", ";
		}
		g_logFile << "\n";
	}
	else
	{
		g_logFile << "No path found :(\n";
	}

	boost::posix_time::time_duration diff = t2 - t1;
	g_logFile << "Sequence solving time: " << diff.total_seconds() << endl << endl;
	g_logFile.close();
}

int main(int argc, char *argv[])
{
	boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
	//_crtBreakAlloc = 11091;
	if (argc < 3)
	{
		cout << "error in input";
		return 1;
	}
	g_graphFileName = argv[1];
	g_rulesFileName = argv[2];
	
	//used for reinitializing the log file if it is not new
	g_logFile.open(g_logFileName);
	g_logFile << "Start" << endl;
	g_logFile.close();
	
	cout << "Starting!" << endl;
	
	/*Graph topologyGraph(g_graphFileName);
	vector<BaseVertex*> sequence;
	sequence.push_back(topologyGraph.get_vertex(0));
	sequence.push_back(topologyGraph.get_vertex(5));
	sequence.push_back(topologyGraph.get_vertex(7));
	sequence.push_back(topologyGraph.get_vertex(6));
	vector<BaseVertex*> satisfyingPath;
	findSatisfyingPath2MB(topologyGraph, sequence, satisfyingPath);*/
		
	if (argc >= 4)
	{
		g_sequencesFileName = argv[3];
		bool shouldSearchVertexDisjoint = argc == 5;
		NetworkSolver ns(g_graphFileName, g_sequencesFileName, shouldSearchVertexDisjoint);
		ns.solveNetwork();
		//testSequenceSolver(g_sequencesFileName);
	}
	else
	{
		testSequenceSolver();
		//testSpecificSequence();
	}
	
	boost::posix_time::ptime t2 = boost::posix_time::microsec_clock::local_time();
	//g_logFile.open(g_outputFileName, ios::app);
	boost::posix_time::time_duration diff = t2 - t1;
	cout << "Solving time seconds: " << diff.total_seconds() << endl << endl;
	cout << "Solving time ms: " << diff.total_milliseconds() << endl << endl;
	//g_logFile.close();
	//_CrtDumpMemoryLeaks();
}
