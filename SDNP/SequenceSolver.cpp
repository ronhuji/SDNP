#include "stdafx.h"
#include "Defs.h"
#include "SequenceSolver.h"
#include "kshortestpaths_2.1/YenTopKShortestPathsAlg.h"
#include <vector>
#include <memory>
#include "kshortestpaths_2.1/Graph.h"
#include <list>
#include <iosfwd>
#include <sstream>
#include "kshortestpaths_2.1/DijkstraShortestPathAlg.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <chrono>
#include <random>
#include <xutility>

bool findSatisfyingPath(Graph& topologyGraph, vector<BaseVertex*>& originalSequence, VertexSet& stProhibitedVertices, EdgeSet& stProhibitedEdges, OUT vector<BaseVertex*>& satisfyingPath, bool edgeDisjoint)
{
	//remove the prohibited vertices and edges so they won't appear in the solution
	for (BaseVertex* pProhibitedVertex : stProhibitedVertices)
	{
		topologyGraph.my_remove_vertex(pProhibitedVertex->getID());
	}
	for (Edge edge : stProhibitedEdges)
	{
		topologyGraph.my_remove_edge(edge);
	}

	//relax the problem
	list < shared_ptr<vector<BaseVertex*>> > relaxedSequencesList;
	shared_ptr<vector<BaseVertex*>> pOriginalSequence(new vector<BaseVertex*>(originalSequence));
	relaxedSequencesList.push_back(pOriginalSequence);
	if (!edgeDisjoint)
	{
		//in the edge version there are no relaxations
		relaxSequences(relaxedSequencesList, topologyGraph);
	}

	if (0 == relaxedSequencesList.size())
	{
		return false;
	}

	//solve the problem for each relaxed sequence separately
    for (VertexVectorPtr& pRelaxedSequence : relaxedSequencesList)
	{	
		//initialize vars for the recursion
		//initiate a map that will hold the information about the differences between the paths
		FindSatisfyingRecVars recursionVars;
		initRecursionVars(recursionVars, pRelaxedSequence->size(), edgeDisjoint);

		if (!edgeDisjoint)
		{
			//remove all sequence vertices
			for (BaseVertex* v : *pRelaxedSequence)
			{
				topologyGraph.my_remove_vertex(v->getID());
			}
		}
		
		//try solving the relaxed sequence
		bool res = findSatisfyingPathRec(topologyGraph, *pRelaxedSequence, satisfyingPath, 0, recursionVars);

		if (false == res)
		{
			//we couldn't find a solution using this order. try using the reverse solution order
			logFailures(topologyGraph, pRelaxedSequence, recursionVars);
			recursionVars.longestFailingPathPerIndex.clear();
			recursionVars.longestFailingPathPerIndex.resize(pRelaxedSequence->size() - 1);

			reverseSolutionOrder(recursionVars);
			logSolutionOrder(recursionVars.solutionOrder);
			//g_logFile.open(g_outputFileName, ios::app);
			res = findSatisfyingPathRec(topologyGraph, *pRelaxedSequence, satisfyingPath, 0, recursionVars);
			//g_logFile.close();
		}

		if (!edgeDisjoint)
		{
			//recover all sequence vertices
			for (BaseVertex* v : *pRelaxedSequence)
			{
				topologyGraph.recover_my_removed_vertex(v->getID());
			}
		}

		logFailures(topologyGraph, pRelaxedSequence, recursionVars);
		
		if (false == res)
		{
			//we couldn't find a solution for this relaxed sequence, proceed to the next one
			continue;
		}
		
		//we found a solution to this relaxed sequence, it is saved in satisfyingPath
		//recover all vertices before you return
		topologyGraph.recoverAll();
		return true;
	}
	//we couldn't find a solution for any relaxed sequence, return false
	//g_logFile.open(g_outputFileName, ios::app);
	//g_logFile << "No path found :(\n";
	//g_logFile.close();
	//recover all vertices before you return
	topologyGraph.recoverAll();
	return false;
}

bool findSatisfyingPathRec(Graph& topologyGraph, vector<BaseVertex*>& sequence, OUT vector<BaseVertex*>& satisfyingPath, int stageIndex, FindSatisfyingRecVars& recursionVariables)
{
    //get the index we are going to try next
    uint index = recursionVariables.solutionOrder[stageIndex];
    
	if (!recursionVariables.edgeDisjoint)
	{
		//we need to find a path between this two vertices so we should recover them
	    topologyGraph.recover_my_removed_vertex(sequence[index]->getID());
		topologyGraph.recover_my_removed_vertex(sequence[index + 1]->getID());
	}

	//initialize the algorithm for finding the shortest paths between: sequence[index] and sequence[index + 1]
	YenTopKShortestPathsAlg yenAlg(topologyGraph, sequence[index], sequence[index + 1]);
	uint shortestPathLength = topologyGraph.getOriginalVertexNum();
	uint k = 0;
	while (yenAlg.has_next())
	{
		//stop when there are no more paths between sequence[index] and sequence[index + 1]. there is more stopping criteria throughout the loop
		k++; //increase the attempts number

		//retrieve the next path as vector
		vector<BaseVertex*> pathToAdd;
		yenAlg.next()->GetVertexVector(pathToAdd);
		if (pathToAdd.size() < shortestPathLength)
		{
			shortestPathLength = pathToAdd.size();
		}

		//recursion stop - we found a path going all the way to the destination of the sequence. concatenate it and return the outcome
		if (stageIndex == recursionVariables.solutionOrder.size() - 1)
		{
			//g_logFile << "index: " << index << " k: " << k << " shortest length: " << shortestPathLength - 1 << " this length: " << pathToAdd.size() - 1 << "\n"; //length dictated by the number of edges
			recursionVariables.pathSoFar[index]->insert(recursionVariables.pathSoFar[index]->end(), pathToAdd.begin(), pathToAdd.end()-1); //don't add the last one so it won't appear twice
            for (auto indexPath : recursionVariables.pathSoFar)
            {
                satisfyingPath.insert(satisfyingPath.end(), indexPath->begin(), indexPath->end());
            }
            satisfyingPath.push_back(sequence[sequence.size()-1]); //last vertex won't appear in any path
            return true;
		}


		if ((pathToAdd.size() > STOP_RATIO*shortestPathLength) //the paths are too long, we want to stop
			|| k > MAX_K) //we have tried enough paths
		{
			//cout << "k or length too big. index: " << index << " k: " << k << " shortest length: " << shortestPathLength << " this length: " << pathToAdd.size() << "\n";
			break; //braking from the while loop will lead to returning false
		}

		//add this path to the path collected so far - we don't copy the destination because it will appear in the path going from it
		recursionVariables.pathSoFar[index]->insert(recursionVariables.pathSoFar[index]->end(), pathToAdd.begin(), pathToAdd.end()-1);  

		if (!recursionVariables.edgeDisjoint)
		{
			//delete the used vertices from the graph
	        for (BaseVertex* pVertex : pathToAdd)
			{
				topologyGraph.my_remove_vertex(pVertex->getID());
			}
		}
		else
		{
			for (auto it = pathToAdd.begin(); it != pathToAdd.end() - 1; it++)
			{
				topologyGraph.my_remove_edge(make_pair((*it)->getID(), (*(it + 1))->getID()));
			}
		}

		//the recursion step
		if (findSatisfyingPathRec(topologyGraph, sequence, satisfyingPath, stageIndex+1, recursionVariables))
		{
			//g_logFile << "index: " << index << " k: " << k << " shortest length: " << shortestPathLength - 1 << " this length: " << pathToAdd.size() - 1 << "\n"; //length dictated by the number of edges
			return true;
		}

		//this path couldn't go all the way through, recover the used vertices/edges and go on to the next one
		//if the source and destination were removed by another index, they will also be recovered here
        if (!recursionVariables.edgeDisjoint)
		{
			for (BaseVertex* pVertex : pathToAdd)
			{
				topologyGraph.recover_my_removed_vertex(pVertex->getID());
			}
		}
		else
		{
			for (auto it = pathToAdd.begin(); it != pathToAdd.end() - 1; it++)
			{
				topologyGraph.recover_my_removed_edge(make_pair((*it)->getID(), (*(it + 1))->getID()));
			}
		}

		//clear pathSoFar in this index
		recursionVariables.pathSoFar[index]->clear();

		if (1 == shortestPathLength)
		{
			//no longer path can fulfill the need because it will delete more vertices
			break;
		}
	}

	if (0 == k)
	{
		saveFailure(topologyGraph, index, recursionVariables);
	}
	
	if (!recursionVariables.edgeDisjoint)
	{
		if (0 != stageIndex)
		{
			//remove the source and destination of this index so they won't be used by other stages in the recursion
			topologyGraph.my_remove_vertex(sequence[index]->getID());
			topologyGraph.my_remove_vertex(sequence[index + 1]->getID());
		}
	}
	return false;
}

void insertBeforeAndAfter(VertexVectorPtr& pSequence, BaseVertex* pBefore, BaseVertex* pAfter, BaseVertex* pBase)
{
	if (pSequence->front() != pBase)
	{
		auto findIter = std::find(pSequence->begin(), pSequence->end(), pBase);
		findIter--;
		if (*findIter != pBefore)
		{
			//if pBefore is already before pBase in the sequence there is no need to insert it again
			findIter++;
			pSequence->insert(findIter, pBefore);
		}
		
	}
	if (pSequence->back() != pBase)
	{
		auto findIter = std::find(pSequence->begin(), pSequence->end(), pBase);
		findIter++;
		if (*findIter != pAfter)
		{
			//if pAfter is already after pBase in the sequence there is no need to insert it again
			pSequence->insert(findIter, pAfter);
		}
	}
	
}

void relaxSequences(IN OUT list<VertexVectorPtr>& sequenceList, Graph& topologyGraph)
{
	//we remove all the vertices that cannot help us finding a path between two nodes in the sequence. we need to pass the sequence so it's nodes won't be removed also
	removeUnusableVertices(sequenceList, topologyGraph);

	//we use an outside iterator and a while loop because we don't always advance it using ++
	auto sequencesIterator = sequenceList.begin();
	while (sequencesIterator != sequenceList.end())
	{
		vector<BaseVertex*>& sequence = **sequencesIterator;
		if (hasDuplicates(sequence))
		{
			//the sequence has duplicates so it's unsolvable. we delete this sequence from the list and go on to the next one
			logRelaxedUnsolvableSequence(sequence, "Sequence has duplicates\n");
			sequencesIterator = sequenceList.erase(sequencesIterator);
			continue;
		}

		bool shouldIncrement = true;
		for (uint i = 0; i < sequence.size(); i++)
		{
			BaseVertex* pVertex = sequence[i];
			set<BaseVertex*> neighbor_vertex_list;
			topologyGraph.get_adjacent_vertices(pVertex, neighbor_vertex_list);
			
			if (0 == neighbor_vertex_list.size())
			{
				//all of the vertices's neighbors were removed because they can't help in finding a path
				stringstream message;
				message << pVertex->getID() << " has no helping neighbor\n";
				logRelaxedUnsolvableSequence(sequence, message.str());

				sequencesIterator = sequenceList.erase(sequencesIterator);
				shouldIncrement = false;
				break;
			}

			if (1 == neighbor_vertex_list.size())
			{
				BaseVertex* onlyNeighbor = *(neighbor_vertex_list.begin());
				if (sequence.back() == pVertex) 
				{
					if (onlyNeighbor == sequence[i - 1])
					{
						//it is already relaxed go to the next vertices in the sequence
						continue;
					}
					sequence.pop_back();
					sequence.push_back(onlyNeighbor);
					sequence.push_back(pVertex);
					shouldIncrement = false; //we want to relax this sequence again because it was changed
					break;
				}

				if (sequence.front() == pVertex) 
				{
					if (onlyNeighbor == sequence[i + 1])
					{
						//it is already relaxed go to the next vertices in the sequence
						continue;
					}
					sequence.insert(++sequence.begin(), onlyNeighbor);
					shouldIncrement = false; //we want to relax this sequence again because it was changed
					break;
				}

				//the vertex with one neighbor is in the middle so the sequence is unsolvable
				stringstream message;
				message << pVertex->getID() << " has only one neighbor\n";
				logRelaxedUnsolvableSequence(sequence, message.str());
								
				sequencesIterator = sequenceList.erase(sequencesIterator);
				shouldIncrement = false;							
				break;
			}

			if (2 == neighbor_vertex_list.size())
			{
				if ( (i == 0								&& neighbor_vertex_list.count(sequence[i+1]) >= 1)	||		//first in the sequence and the next one is one of it's neighbour
					 (i == sequence.size() - 1			&& neighbor_vertex_list.count(sequence[i - 1]) >= 1) ||		//first in the sequence and the prior one is one of it's neighbour
					 (i != 0 && i != sequence.size() - 1 && neighbor_vertex_list.count(sequence[i - 1]) >= 1 && neighbor_vertex_list.count(sequence[i + 1]) >= 1) ) //somewhere inside the sequence and both of it's neighbors are around it 
				{
					//it is already relaxed
					continue;
				}

				//create two relaxed sequences with the two neighbors before and after
				auto neighboursIterator = neighbor_vertex_list.begin();
				BaseVertex* pFirstNeighbour = *neighboursIterator;
				BaseVertex* pSecondNeighbour = *(++neighboursIterator);
				VertexVectorPtr pRelaxedSequence1(new vector<BaseVertex*>(sequence));
				insertBeforeAndAfter(pRelaxedSequence1, pFirstNeighbour, pSecondNeighbour, pVertex); //inserts before only if pVertex is not first and after only if pVertex is not last
				VertexVectorPtr pRelaxedSequence2(new vector<BaseVertex*>(sequence));
				insertBeforeAndAfter(pRelaxedSequence2, pSecondNeighbour, pFirstNeighbour, pVertex); //inserts before only if pVertex is not first and after only if pVertex is not last
				sequenceList.push_back(pRelaxedSequence1);
				sequenceList.push_back(pRelaxedSequence2);
				sequencesIterator = sequenceList.erase(sequencesIterator);
				shouldIncrement = false; //was already incremented in the erasure
				break;
			}
		}	

		if (shouldIncrement)
		{
			sequencesIterator++;
		}
	}
}

bool hasDuplicates(vector<BaseVertex*>& sequence)
{
	set<BaseVertex*> checkSet;
    for (BaseVertex* v : sequence)
	{
		if (checkSet.count(v) >= 1)
		{
			return true;
		}
		checkSet.insert(v);
	}
	return false;
}

void removeUnusableVertices(list<VertexVectorPtr>& sequenceList, Graph& topologyGraph)
{
	vector<BaseVertex*>& sequence = **sequenceList.begin();
	set<BaseVertex*> setForSearch(sequence.begin(), sequence.end());
	bool wasRemoved = true;
	while (wasRemoved == true)
	{
		wasRemoved = false;
		for (uint i = 0; i < topologyGraph.getOriginalVertexNum(); i++)
		{
			BaseVertex* v = topologyGraph.get_vertex(i);
			if (NULL == v //v is already removed
				|| setForSearch.count(v) >= 1) //v is in the sequence
			{
				continue;
			}
			set<BaseVertex*> neighbor_set;
			topologyGraph.get_adjacent_vertices(v, neighbor_set);
			if (neighbor_set.size() <= 1)
			{
				topologyGraph.my_remove_vertex(i);
				wasRemoved = true;
			}
		}
	}	
}

void logRelaxedUnsolvableSequence(vector<BaseVertex*>& sequence, string message)
{
	//logSequence(sequence, "Relaxed unsolvable sequence :");
	//g_logFile.open(g_outputFileName, ios::app);
	//g_logFile << message;
	//g_logFile.close();
}

bool findShortestSatisfyingPathWithLoops(Graph& topologyGraph, std::vector<BaseVertex*>& originalSequence, VertexSet& stProhibitedVertices, EdgeSet& stProhibitedEdges, OUT vector<BaseVertex*>& satisfyingPath)
{
	//remove the prohibited vertices and edges so they won't appear in the solution
	for (BaseVertex* pProhibitedVertex : stProhibitedVertices)
	{
		topologyGraph.my_remove_vertex(pProhibitedVertex->getID());
	}
	for (Edge edge : stProhibitedEdges)
	{
		topologyGraph.my_remove_edge(edge);
	}

	//open log file for writing during solving, and try solving the relaxed sequence
	//g_logFile.open(g_outputFileName, ios::app);
	vector<BaseVertex*> pathSoFar;
	bool res = findShortestSatisfyingPathWithLoopsRec(topologyGraph, originalSequence, satisfyingPath, pathSoFar, 0);
	//g_logFile.close();

	topologyGraph.recoverAll();
	return res;
}

bool findShortestSatisfyingPathWithLoopsRec(Graph& topologyGraph,
	std::vector<BaseVertex*>& sequence,
	OUT vector<BaseVertex*>& satisfyingPath,
	vector<BaseVertex*>& pathSoFar,
	int index)
{
	//retrieve the next path as vector
	vector<BaseVertex*> pathToAdd;
	DijkstraShortestPathAlg dijkstra_alg(&topologyGraph);
	BasePath* basePathToAdd = dijkstra_alg.get_shortest_path(sequence[index], sequence[index + 1]);
	if (basePathToAdd->Weight() == Graph::DISCONNECT)
	{
		//g_logFile << "No path, even with loops" << "\n";
		delete basePathToAdd;
		return false;
	}
	
	basePathToAdd->GetVertexVector(pathToAdd);
	delete basePathToAdd;
	//g_logFile << "With loops!! index: " << index << ", until vertex: " << sequence[index + 1]->getID() << ", shortest path length: " << pathSoFar.size() + pathToAdd.size() - 1 << "\n"; //length dictated by the number of edges

	//recursion stop - we found a path going all the way to the destination of the sequence. concatenate it and return the outcome
	if (index == sequence.size() - 2)
	{
		pathSoFar.insert(pathSoFar.end(), pathToAdd.begin(), pathToAdd.end());
		satisfyingPath.insert(satisfyingPath.end(), pathSoFar.begin(), pathSoFar.end());
		return true;
	}

	//remove the destination of the path. it will be added as part of the next path
	pathToAdd.pop_back();

	//add this path to the path collected so far
	pathSoFar.insert(pathSoFar.end(), pathToAdd.begin(), pathToAdd.end());

	return findShortestSatisfyingPathWithLoopsRec(topologyGraph, sequence, satisfyingPath, pathSoFar, index + 1);
}

void createRandomPermutation(uint size, OUT vector<uint>& permutationVector)
{
    permutationVector.clear();
    vector<uint> tempVector;
    for (uint i=0; i<size; i++)
    {
        tempVector.push_back(i);
    }
    
    unsigned seed = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    
    while (size>0)
    {
        std::uniform_int_distribution<int> sizeDistribution(0, size-1);
        uint index = sizeDistribution(generator);
        permutationVector.push_back(tempVector[index]);
        tempVector.erase(tempVector.begin() + index);
        size--;
    }
}

size_t getTotalSize(vector<VertexVectorPtr>& vectorOfVectors)
{
	size_t totalSize = 0;
	for (VertexVectorPtr& pVec : vectorOfVectors)
	{
		if (nullptr != pVec.get())
		{
			totalSize += pVec->size();
		}
	}
	return totalSize;
}

void initRecursionVars(FindSatisfyingRecVars& recursionVars, size_t sequenceSize, bool edgeDisjoint)
{
	recursionVars.edgeDisjoint = edgeDisjoint;

	recursionVars.occurencesPerIndexVector.resize(sequenceSize - 1);
	for (uint i = 0; i < sequenceSize - 1; i++)
	{
		recursionVars.occurencesPerIndexVector[i].reset(new map< uint, uint >());
	}

	for (uint i = 0; i < sequenceSize - 1; i++)
	{
		VertexVectorPtr pVertexVector(new vector<BaseVertex*>());
		recursionVars.pathSoFar.push_back(pVertexVector);
	}

	recursionVars.longestFailingPathPerIndex.resize(sequenceSize - 1);

	createRandomPermutation(sequenceSize - 1, recursionVars.solutionOrder);
	//log the solution order
	logSolutionOrder(recursionVars.solutionOrder);
}

void saveFailure(Graph& topologyGraph, uint index, FindSatisfyingRecVars& recursionVariables)
{
	size_t pathSoFarTotalSize = getTotalSize(recursionVariables.pathSoFar);
	
	if (recursionVariables.longestFailingPathPerIndex[index].get() == nullptr
		|| (getTotalSize(*recursionVariables.longestFailingPathPerIndex[index]) < pathSoFarTotalSize))
	{
		recursionVariables.longestFailingPathPerIndex[index].reset(new vector<VertexVectorPtr>());
		for (VertexVectorPtr& indexPath : recursionVariables.pathSoFar)
		{
			VertexVectorPtr longestIndexPath(new VertexVector(*indexPath));
			recursionVariables.longestFailingPathPerIndex[index]->push_back(longestIndexPath);
		}
	}
}

void logFailures(Graph& topologyGraph, VertexVectorPtr& pRelaxedSequence, FindSatisfyingRecVars& recursionVars)
{
	//g_logFile.open(g_outputFileName, ios::app);
	for (uint index = 0; index < pRelaxedSequence->size() - 1; index++)
	{
		if (nullptr != recursionVars.longestFailingPathPerIndex[index].get())
		{
			//g_logFile << "longest failing path when searching from index " << index << ", vertex " << (*pRelaxedSequence)[index]->getID() << ": " << getTotalSize(*(recursionVars.longestFailingPathPerIndex[index])) << "\n";
		}
	}
	//g_logFile.close();
}

void reverseSolutionOrder(FindSatisfyingRecVars& recursionVars)
{
	vector<uint> originalSolutionOrder(recursionVars.solutionOrder);
	size_t size = originalSolutionOrder.size();
	for (uint i = 0; i < size; i++)
	{
		recursionVars.solutionOrder[size - 1 - i] = originalSolutionOrder[i];
	}
}

void logSolutionOrder(vector<uint>& solutionOrder)
{
	//g_logFile.open(g_outputFileName, ios::app);
	//g_logFile << "solution order: ";
	for (uint i : solutionOrder)
	{
		//g_logFile << i << ", ";
	}
	//g_logFile << "\n";
	//g_logFile.close();
}




bool findSatisfyingPath2MB(Graph& topologyGraph, vector<BaseVertex*>& sequence, VertexSet& stProhibitedVertices, EdgeSet& stProhibitedEdges, OUT vector<BaseVertex*>& satisfyingPath)
{
	//remove the prohibited vertices and edges so they won't appear in the solution
	for (BaseVertex* pProhibitedVertex : stProhibitedVertices)
	{
		topologyGraph.my_remove_vertex(pProhibitedVertex->getID());
	}
	for (Edge edge : stProhibitedEdges)
	{
		topologyGraph.my_remove_edge(edge);
	}

	if (sequence.size() != 4)
	{
		return false;
	}

	//retrieve the next path as vector
	vector<BaseVertex*> aToBPath;
	DijkstraShortestPathAlg dijkstra_alg(&topologyGraph);
	BasePath* basePathToAdd = dijkstra_alg.get_shortest_path(sequence[0], sequence[1]);
	if (basePathToAdd->Weight() == Graph::DISCONNECT)
	{
		cout << "No path, even with loops" << "\n";
		delete basePathToAdd;
		return false;
	}

	basePathToAdd->GetVertexVector(aToBPath);
	delete basePathToAdd;

	for (BaseVertex* pVertex : aToBPath)
	{
		topologyGraph.my_remove_vertex(pVertex->getID());
	}

	VertexVector ABVDPathToC;
	auto itABVDPathToCSource = aToBPath.rbegin();
	VertexVector ABVDPathToD;
	for (auto it = aToBPath.rbegin(); it != aToBPath.rend(); ++it)
	{
		if (ABVDPathToC.empty())
		{
			if (!recoverSourceAndFindPath(topologyGraph, *it, sequence[2], ABVDPathToC))
			{
				continue;
			}
			itABVDPathToCSource = it;
		} 
		if (recoverSourceAndFindPath(topologyGraph, *it, sequence[3], ABVDPathToD))
		{
			buildFullPath(aToBPath, ABVDPathToC, ABVDPathToD, satisfyingPath);
			topologyGraph.recoverAll();
			return true;
		}
	}
	//we couldn't find an ABVD path to d upper than than the ABVD path to c, so search one that is lower
	for (auto it = itABVDPathToCSource; it != aToBPath.rbegin(); --it)
	{
		if (recoverSourceAndFindPath(topologyGraph, *it, sequence[3], ABVDPathToD))
		{
			//cout << "yeah" << "\n";
			bool res = buildSpecialPath(topologyGraph, aToBPath, ABVDPathToC, ABVDPathToD, satisfyingPath);
			topologyGraph.recoverAll();
			return res;
		}
	}
	
	topologyGraph.recoverAll();
	return false;
}

bool recoverSourceAndFindPath(Graph& topologyGraph, BaseVertex* src, BaseVertex* dst, OUT VertexVector& path)
{
	topologyGraph.recover_my_removed_vertex(src->getID());
	bool res = findPath(topologyGraph, src, dst, path);
	topologyGraph.my_remove_vertex(src->getID());
	return res;
}

bool findPath(Graph& topologyGraph, BaseVertex* src, BaseVertex* dst, OUT VertexVector& path)
{
	if (src == dst)
	{
		path.push_back(src);
		return true;
	}

	DijkstraShortestPathAlg dijkstra_alg(&topologyGraph);
	BasePath* basePathToAdd = dijkstra_alg.get_shortest_path(src, dst);
	if (basePathToAdd->Weight() == Graph::DISCONNECT)
	{
		//cout << "No path, even with loops" << "\n";
		delete basePathToAdd;
		return false;
	}

	basePathToAdd->GetVertexVector(path);
	delete basePathToAdd;	
	return true;
}

void buildFullPath(VertexVector& aToBPath, VertexVector& ABVDPathToC, VertexVector& ABVDPathToD, OUT VertexVector& satisfyingPath)
{
	satisfyingPath = aToBPath;
	satisfyingPath.pop_back(); //remove b because we will add it later
	bool beforeC = true;
	for (auto it = aToBPath.rbegin(); it != aToBPath.rend(); it++)
	{
		if (beforeC)
		{
			if (*it != ABVDPathToC[0])
			{
				satisfyingPath.push_back(*it);
				continue;
			}

			satisfyingPath.insert(satisfyingPath.end(), ABVDPathToC.begin(), ABVDPathToC.end());

			if (ABVDPathToC[0] != ABVDPathToD[0])
			{
				satisfyingPath.insert(satisfyingPath.end(), ABVDPathToC.rbegin() + 1, ABVDPathToC.rend());
				beforeC = false;
				continue;
			}

			//ABVDPathToC[0] == ABVDPathToD[0]
			satisfyingPath.pop_back();
			VertexVector reversedFinalPathToD;
			for (auto dIt = ABVDPathToD.rbegin(); dIt != ABVDPathToD.rend(); dIt++)
			{
				auto cMeetsDIt = find(ABVDPathToC.rbegin(), ABVDPathToC.rend(), *dIt);
				if (cMeetsDIt == ABVDPathToC.rend())
				{
					reversedFinalPathToD.push_back(*dIt);
					continue;
				}
				satisfyingPath.insert(satisfyingPath.end(), ABVDPathToC.rbegin(), cMeetsDIt+1);
				break;
			}
			satisfyingPath.insert(satisfyingPath.end(), reversedFinalPathToD.rbegin(), reversedFinalPathToD.rend());
			break;
		}

		if (*it != ABVDPathToD[0])
		{
			satisfyingPath.push_back(*it);
			continue;
		}

		satisfyingPath.insert(satisfyingPath.end(), ABVDPathToD.begin(), ABVDPathToD.end());
		break;
	}
}

bool findMostBypassingPath(Graph& topologyGraph, EdgeVector& edgesToByPass, VertexVector& aToBPath, VertexVector& path)
{
	for (auto e : edgesToByPass)
	{
		//remove undirected edges
		topologyGraph.my_remove_undirected_edge(e);
	}
	bool foundPath = false;
	for (auto e : edgesToByPass)
	{
		if (!foundPath)
		{
			foundPath = findPath(topologyGraph, aToBPath.back(), aToBPath.front(), path);
		}
		//continue to recover all edges we have removed
		topologyGraph.recover_my_removed_undirected_edge(e);
	}
	return foundPath;
}

bool neighboursOnPath(BaseVertex * formerVertex, BaseVertex* vertex, VertexVector& path)
{
	bool foundOne = false;
	for (auto vertexOnPath : path)
	{
		if (vertexOnPath == vertex || vertexOnPath == formerVertex)
		{
			if (foundOne)
			{
				return true;
			}
			foundOne = true;
		}
		else
		{
			if (foundOne)
			{
				return false;
			}
		}
	}
	return false;
}

VertexVectorPtr trimBypassingPath(VertexVector& path, VertexVector& aToBPath)
{
	VertexVectorPtr pTrimmedPath(new VertexVector());
	pTrimmedPath->push_back(aToBPath.front()); //just dummy
	BaseVertex * formerVertex = nullptr;
	bool foundFirstOfTrimmed = false;
	for (auto vertex : path)
	{
		if (!foundFirstOfTrimmed)
		{
			if (find(aToBPath.begin(), aToBPath.end(), vertex) != aToBPath.end())
			{
				pTrimmedPath->pop_back();
			}
			else
			{
				foundFirstOfTrimmed = true;
			}
			pTrimmedPath->push_back(vertex);
		}
		else
		{
			if (neighboursOnPath(formerVertex, vertex, aToBPath))
			{
				break;
			}
			pTrimmedPath->push_back(vertex);
		}
		formerVertex = vertex;		
	}
	return pTrimmedPath;
}

bool comesBeforeOnPath(BaseVertex * vertex1, BaseVertex * vertex2, VertexVector& path)
{
	for (auto vertex : path)
	{
		if (vertex == vertex1)
		{
			return true;
		}
		if (vertex == vertex2)
		{
			return false;
		}
	}
	throw exception(); //both supposed to be on path
}

void buildFullSidePath(BaseVertex * firstVertex, BaseVertex * endVertex, vector<VertexVectorPtr> vSidePaths, VertexVector& aToBPath, VertexVector& fullSidePath)
{
	//start from building path from end to start
	auto sidePathsIt = vSidePaths.begin();
	bool isParallel = false;
	for (auto aToBIt = find(aToBPath.rbegin(), aToBPath.rend(), endVertex);
		aToBIt != find(aToBPath.rbegin(), aToBPath.rend(), firstVertex) + 1;
		aToBIt++)
	{
		if (isParallel)
		{
			if ((*sidePathsIt)->back() == *aToBIt)
			{
				sidePathsIt++;
				isParallel = false;
			}
		} 
		else
		{
			if (sidePathsIt != vSidePaths.end() && (*sidePathsIt)->front() == *aToBIt)
			{
				fullSidePath.insert(fullSidePath.end(), (*sidePathsIt)->begin(), (*sidePathsIt)->end());
				isParallel = true;
			}
			else
			{
				fullSidePath.push_back(*aToBIt);
			}
		}		
	}
	reverse(fullSidePath.begin(), fullSidePath.end());
}

void buildFinalSpecialFullPath(VertexVector fullRightPath, VertexVector fullLeftPath, VertexVector& aToBPath, VertexVector& ABVDPathToC, VertexVector& ABVDPathToD, VertexVector& satisfyingPath)
{
	for (auto v : aToBPath)
	{
		if (v == fullRightPath.front())
		{
			break;
		}
		satisfyingPath.push_back(v);
	}
	VertexVector& goingDownPath =
		find(fullRightPath.begin(), fullRightPath.end(), ABVDPathToD.front()) != fullRightPath.end() ?
		fullLeftPath :
		fullRightPath;
	satisfyingPath.insert(satisfyingPath.end(), goingDownPath.begin(), goingDownPath.end());
	
	satisfyingPath.insert(
		satisfyingPath.end(),
		find(aToBPath.begin(), aToBPath.end(), goingDownPath.back()) + 1,
		aToBPath.end()
		);

	satisfyingPath.insert(
		satisfyingPath.end(),
		aToBPath.rbegin()+1,
		find(aToBPath.rbegin(), aToBPath.rend(), fullRightPath.back())+1);
	
	VertexVector& goingUpPath =
		find(fullRightPath.begin(), fullRightPath.end(), ABVDPathToC.front()) != fullRightPath.end() ?
		fullRightPath :
		fullLeftPath;
	satisfyingPath.insert(
		satisfyingPath.end(),
		goingUpPath.rbegin()+1,
		find(goingUpPath.rbegin(), goingUpPath.rend(), ABVDPathToC.front())+1);

	satisfyingPath.insert(satisfyingPath.end(), ABVDPathToC.begin()+1, ABVDPathToC.end());
	satisfyingPath.insert(satisfyingPath.end(), ABVDPathToC.rbegin() + 1, ABVDPathToC.rend());
	
	if (goingUpPath != goingDownPath)
	{
		satisfyingPath.insert(
			satisfyingPath.end(),
			find(goingUpPath.begin(), goingUpPath.end(), ABVDPathToC.front()) + 1,
			find(goingUpPath.begin(), goingUpPath.end(), ABVDPathToD.front())
			);
	}
	else
	{
		// go up and then down
		VertexVector& finalGoingDown = goingDownPath == fullRightPath ? fullLeftPath : fullRightPath;
		
		satisfyingPath.insert(
			satisfyingPath.end(),
			find(goingUpPath.rbegin(), goingUpPath.rend(), ABVDPathToC.front())+1,
			goingUpPath.rend()
			);

		satisfyingPath.insert(
			satisfyingPath.end(),
			finalGoingDown.begin()+1,
			find(finalGoingDown.begin(), finalGoingDown.end(), ABVDPathToD.front())
			);
	}

	satisfyingPath.insert(satisfyingPath.end(), ABVDPathToD.begin(), ABVDPathToD.end());
}

bool buildSpecialPath(Graph& topologyGraph, VertexVector& aToBPath, VertexVector& ABVDPathToC, VertexVector& ABVDPathToD, OUT VertexVector& satisfyingPath)
{
	//recover the a to b path before we try to use it to build the full path
	for (BaseVertex* pVertex : aToBPath)
	{
		topologyGraph.recover_my_removed_vertex(pVertex->getID());
	}
	EdgeVector edgesToBypass = getCToDEdges(aToBPath, ABVDPathToC.front(), ABVDPathToD.front());
	
	vector<VertexVectorPtr> vRightPaths;
	vector<VertexVectorPtr> vLeftPaths;
	bool chooseRightPaths = true;
	while (true)
	{
		VertexVector path;
		if (!findMostBypassingPath(topologyGraph, edgesToBypass, aToBPath, path))
		{
			return false;
		}
		VertexVectorPtr pBypassingPath = trimBypassingPath(path, aToBPath);

		if (chooseRightPaths)
		{
			vRightPaths.push_back(pBypassingPath);
		} 
		else
		{
			vLeftPaths.push_back(pBypassingPath);
		}

		if (comesBeforeOnPath(pBypassingPath->back(), ABVDPathToC.front(), aToBPath))
		{
			break;
		}

		chooseRightPaths = !chooseRightPaths;
		while (edgesToBypass.back().second != pBypassingPath->back()->getID())
		{
			edgesToBypass.pop_back();
		}
	}
	
	BaseVertex * endVertex = vRightPaths.front()->front();
	BaseVertex * firstVertex = 
		comesBeforeOnPath(vRightPaths.back()->back(), vLeftPaths.back()->back(), aToBPath) ?
		vRightPaths.back()->back() :
		vLeftPaths.back()->back();
	
	VertexVector fullRightPath;
	VertexVector fullLeftPath;
	buildFullSidePath(firstVertex, endVertex, vRightPaths, aToBPath, fullRightPath);
	buildFullSidePath(firstVertex, endVertex, vLeftPaths, aToBPath, fullLeftPath);

	buildFinalSpecialFullPath(fullRightPath, fullLeftPath, aToBPath, ABVDPathToC, ABVDPathToD, satisfyingPath);
	return true;
}

EdgeVector getCToDEdges(VertexVector& aToBPath, BaseVertex * sourceToC, BaseVertex * sourceToD)
{
	EdgeVector cToDEdges;
	bool beforeC = true;
	for (size_t i = 0; i < aToBPath.size(); i++)
	{
		if (beforeC)
		{
			if (aToBPath[i] == sourceToC)
			{
				beforeC = false;
			}
		}
		else
		{
			cToDEdges.push_back(Edge(aToBPath[i - 1]->getID(), aToBPath[i]->getID()));
			if (aToBPath[i] == sourceToD)
			{
				return cToDEdges;
			}
		}
	}
	cout << "ah\n";
	throw exception();
}