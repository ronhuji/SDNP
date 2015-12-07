#pragma once
#include <string>
#include "kshortestpaths_2.1\Graph.h"
#include "SrcDstPolicies.h"
#include "DestinationSolver.h"

class NetworkSolver
{
public:
	NetworkSolver(string graphFileName, string policiesFileName, bool shouldSearchVertexDisjoint);
	
	~NetworkSolver();

	void solveNetwork();

private:
	Graph m_topologyGraph;
	vector<ISolverPtr> m_vpSolvers;
	bool m_shouldSearchVertexDisjoint;
	void initSolvers(string policiesFileName);
};