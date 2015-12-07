#include "stdafx.h"
#include "NetworkSolver.h"
#include "IPolicy.h"
#include "AreaAvoidancePolicy.h"
#include <algorithm>
#include "SrcDstSolver.h"

NetworkSolver::NetworkSolver(string graphFileName, string policiesFileName, bool shouldSearchVertexDisjoint) : 
	m_topologyGraph(graphFileName), m_shouldSearchVertexDisjoint(shouldSearchVertexDisjoint)
{
	initSolvers(policiesFileName);
}

NetworkSolver::~NetworkSolver()
{
}

void NetworkSolver::initSolvers(string policiesFileName)
{
	ifstream file(policiesFileName);
	string solverTypeLine;
	string metaDataLine;
	string policiesLine;
	if (file.is_open())
	{
		while (getline(file, solverTypeLine))
		{
			ISolverPtr pISolver;
			getline(file, metaDataLine);
			getline(file, policiesLine);
			
			switch (ISolver::getType(solverTypeLine))
			{
			case ISolver::DST_SOLVER:
				pISolver.reset(new DestinationSolver(m_topologyGraph, metaDataLine, nullptr));
				pISolver->readPolicies(policiesLine);
				break;
			case ISolver::SRC_DST_SOLVER:
				pISolver.reset(new SrcDstSolver(m_topologyGraph, metaDataLine, m_shouldSearchVertexDisjoint));
				pISolver->readPolicies(policiesLine);
				break;
			}
			
			m_vpSolvers.push_back(pISolver);
		}
	}
}

void NetworkSolver::solveNetwork()
{
	ofstream rulesFile;
	rulesFile.open(g_rulesFileName);
	rulesFile.close();
	try
	{
		for (auto pISolver : m_vpSolvers)
		{
			pISolver->solve();
		}
	}
	catch (exception e)
	{
		cout << "exception thrown, could find solution, aborting..\n";
	}
	
}
