#pragma once
#include "ISolver.h"
#include "kshortestpaths_2.1\Graph.h"
#include "SrcDstPolicies.h"
#include "ForwardingRule.h"

class SrcDstSolver : public ISolver
{
public:
	SrcDstSolver(Graph& topologyGraph, string metaData, bool shouldSearchVertexDisjoint);
	virtual ~SrcDstSolver();

	virtual void solve();

	virtual void readPolicies(const string& policiesLine);

	void createForwardingRules();

	void printRules();

private:
	VertexSet m_stACVertices;
	SrcDstPolicies m_srcDstPolicies;
	VertexVector m_satisfyingPath;
	BaseVertex* m_pDst;
	BaseVertex* m_pSrc;
	vector<ForwardingRulePtr> m_vForwardingRules;
	string m_policiesLine;
};