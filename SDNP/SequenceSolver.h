#pragma once

#include "Defs.h"
#include "kshortestpaths_2.1/Graph.h"
#include "kshortestpaths_2.1/GraphElements.h"
#include <vector>
#include <list>

bool findSatisfyingPath(Graph& topologyGraph, vector<BaseVertex*>& originalSequence, VertexSet& stProhibitedVertices, EdgeSet& stProhibitedEdges, OUT vector<BaseVertex*>& satisfyingPath, bool edgeDisjoint);


struct FindSatisfyingRecVars
{
	vector<VertexVectorPtr> pathSoFar;
	vector< shared_ptr< map< uint, uint > > > occurencesPerIndexVector;
	vector<uint> solutionOrder;
	vector< shared_ptr< vector<VertexVectorPtr> > > longestFailingPathPerIndex;
	bool edgeDisjoint;
};

/*
find a path from sequence[index] to sequence[index+1] and than go on recursively to finding path from [index+1] to [index+2] until the last vertex in the sequence is reached.
pathSoFar's vertices are expected to b e deleted from the topology graph.
try doing so using K_MAX best path for each two consecutive indexes.
if the last is reached it will return true and satisfyingPath will hold the satisfying path. otherwise will return false and satisfyingPath will be empty.
occurencesPerIndexVector is used for logging vertices from unsuccessful attempts
*/
bool findSatisfyingPathRec(Graph& topologyGraph,
	vector<BaseVertex*>& sequence,
	OUT vector<BaseVertex*>& satisfyingPath,
	vector<BaseVertex*>& pathSoFar,
	int index,
	vector< shared_ptr< map< uint, uint > > >& occurencesPerIndexVector);
bool findSatisfyingPathRec(Graph& topologyGraph,
	vector<BaseVertex*>& sequence,
	OUT vector<BaseVertex*>& satisfyingPath,
    int index,
	FindSatisfyingRecVars& recursionVariables);


/*
relax the sequences in the list so there will stay inside it only optionally solvable sequences according to some base rules about neighbors of the vertices in the sequence
*/
void relaxSequences(list< VertexVectorPtr >& sequenceVector, Graph& topologyGraph);

void insertBeforeAndAfter(VertexVectorPtr& pSequence, BaseVertex* pBefore, BaseVertex* pAfter, BaseVertex* pBase);
bool hasDuplicates(vector<BaseVertex*>& sequence);

/*
 iteratively removes from the graph all the vertices that are left with one neighbour (because they cannot be used for finding paths) excluding the vertices in the sequence
 */
void removeUnusableVertices(list<VertexVectorPtr>& sequenceList, Graph& topologyGraph);

void logRelaxedUnsolvableSequence(vector<BaseVertex*>& sequence, string message);

bool findShortestSatisfyingPathWithLoops(Graph& topologyGraph, std::vector<BaseVertex*>& originalSequence, VertexSet& stProhibitedVertices, EdgeSet& stProhibitedEdges, OUT vector<BaseVertex*>& satisfyingPath);
bool findShortestSatisfyingPathWithLoopsRec(Graph& topologyGraph, std::vector<BaseVertex*>& sequence, OUT vector<BaseVertex*>& satisfyingPath, vector<BaseVertex*>& pathSoFar, int index);

void createRandomPermutation(uint size, OUT vector<uint>& permutationVector);
size_t getTotalSize(vector<VertexVectorPtr>& vectorOfVectors);

void initRecursionVars(FindSatisfyingRecVars& recursionVars, size_t sequenceSize, bool edgeDisjoint);
void logFailures(Graph& topologyGraph, VertexVectorPtr& pRelaxedSequence, FindSatisfyingRecVars& recursionVars);
void saveFailure(Graph& topologyGraph, uint index, FindSatisfyingRecVars& recursionVariables);

void reverseSolutionOrder(FindSatisfyingRecVars& recursionVars);
void logSolutionOrder(vector<uint>& solutionOrder);

bool findSatisfyingPath2MB(Graph& topologyGraph, vector<BaseVertex*>& sequence, VertexSet& stProhibitedVertices, EdgeSet& stProhibitedEdges, OUT vector<BaseVertex*>& satisfyingPath);
bool recoverSourceAndFindPath(Graph& topologyGraph, BaseVertex* src, BaseVertex* dst, OUT VertexVector& path);
bool findPath(Graph& topologyGraph, BaseVertex* src, BaseVertex* dst, OUT VertexVector& path);
void buildFullPath(VertexVector& aToBPath, VertexVector& ABVDPathToC, VertexVector& ABVDPathToD, OUT VertexVector& satisfyingPath);
bool buildSpecialPath(Graph& topologyGraph, VertexVector& aToBPath, VertexVector& ABVDPathToC, VertexVector& ABVDPathToD, OUT VertexVector& satisfyingPath);
EdgeVector getCToDEdges(VertexVector& aToBPath, BaseVertex * sourceToC, BaseVertex * sourceToD);