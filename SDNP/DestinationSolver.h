#pragma once

#include "Defs.h"
#include <memory>
#include "SrcDstPolicies.h"
#include "AreaAvoidancePolicy.h"
#include "kshortestpaths_2.1\Graph.h"
#include "kshortestpaths_2.1\DijkstraShortestPathAlg.h"
#include "ForwardingRule.h"
#include "ISolver.h"

class DestinationSolver : public ISolver
{
public:
	DestinationSolver(Graph& topologyGraph, string& metaData, BaseVertex* pDstVertex);
	~DestinationSolver();

	virtual void solve();
	virtual void readPolicies(const string& policiesLine);

	void addAreaAvoidancePolicy(AreaAvoidancePolicyPtr& pDacp);
	void createForwardingRules();
	void printRules();

private:
	VertexSet m_stACVertices;
	EdgeSet m_stACEdges;
	BaseVertex* m_pDstVertex;
	DagPtr m_dstDag;
	vector<ForwardingRulePtr> m_vForwardingRules;

	void setDstFromDefaulDagPolicy(string policyStr);
};

typedef shared_ptr<DestinationSolver> DestinationSolverPtr;