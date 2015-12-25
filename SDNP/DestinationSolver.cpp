#include "stdafx.h"
#include "DestinationSolver.h"
#include "kshortestpaths_2.1\DijkstraShortestPathAlg.h"
#include <exception>
#include "IPolicy.h"
#include <boost\lexical_cast.hpp>

DestinationSolver::DestinationSolver(Graph& topologyGraph, string& metaData, BaseVertex* pDstVertex) : ISolver(topologyGraph, metaData), m_pDstVertex(pDstVertex)
{
}

DestinationSolver::~DestinationSolver()
{
}

void DestinationSolver::addAreaAvoidancePolicy(AreaAvoidancePolicyPtr& pAcp)
{
	BaseVertex* sourceVertex = pAcp->getSource();
	if (nullptr != sourceVertex)
	{
		throw exception(); //there is not supposed to be a source here
	}
	//this vertices and edges are not allowed to reach the destination
	VertexVectorPtr pACVertices(pAcp->getAAVertices());
	for (BaseVertex* pVertex : *pACVertices)
	{
		m_stACVertices.insert(pVertex);
	}
	EdgeVectorPtr pACEdges(pAcp->getAAEdges());
	for (Edge edge : *pACEdges)
	{
		m_stACEdges.insert(edge);
	}
}

void DestinationSolver::solve()
{
	//find a destination dag

	//remove prohibited vertices and edges
	for (BaseVertex* pVertex : m_stACVertices)
	{
		//remove AC vertices
		m_topologyGraph.my_remove_vertex(pVertex->getID());
	}
	for (Edge edge : m_stACEdges)
	{
		m_topologyGraph.remove_edge(edge);
	}

	DijkstraShortestPathAlg shortest_path_alg(&m_topologyGraph);
	m_dstDag = (shortest_path_alg.get_shortest_path_flower(m_pDstVertex));

	createForwardingRules();
	
	//printRules();
}

void DestinationSolver::createForwardingRules()
{
	for (auto switchPair : *m_dstDag)
	{
		VertexVector vNextSwitches;
		for (auto nextSwitch : *(switchPair.second))
		{
			vNextSwitches.push_back(nextSwitch);
		}
		ForwardingRulePtr pRule(new ForwardingRule(m_metaData, switchPair.first, vNextSwitches));
		m_vForwardingRules.push_back(pRule);
	}
}

void DestinationSolver::printRules()
{
	ofstream rulesFile;
	rulesFile.open(g_rulesFileName, ios::app);
	for (auto pRule : m_vForwardingRules)
	{
		pRule->printOut(rulesFile);
	}
	rulesFile.close();
}

void DestinationSolver::setDstFromDefaulDagPolicy(string policyStr)
{
	string remainingLine(policyStr);
	remainingLine.erase(0, ARROW.length());
	m_pDstVertex = m_topologyGraph.get_vertex(boost::lexical_cast<uint>(remainingLine));	
}

void DestinationSolver::readPolicies(const string& policiesLine)
{
	vector<string> vPoliciesStr;
	splitToPolicies(policiesLine, vPoliciesStr);
	for (string policyStr : vPoliciesStr)
	{
		switch (IPolicy::getPolicyType(policyStr))
		{
		case IPolicy::OPTIONAL_SEQUENCE:
			throw exception("illegal policy");
		case IPolicy::AREA_AVOIDANCE:
		{
			AreaAvoidancePolicyPtr pAaP(new AreaAvoidancePolicy(m_topologyGraph, policyStr));
			addAreaAvoidancePolicy(pAaP);
			m_pDstVertex = pAaP->getDestination();
			break;
		}
		case IPolicy::DEFAULT_DAG:
		{
			setDstFromDefaulDagPolicy(policyStr);
			break;
		}
		default:
			break;
		}
	}
}

