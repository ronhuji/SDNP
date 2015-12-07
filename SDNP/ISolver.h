#pragma once

#include "Defs.h"
#include "kshortestpaths_2.1\Graph.h"

extern const string DST_POLICY_STRING;
extern const string SRC_DST_POLICY_STRING;

class ISolver
{
public:

	enum SolverType
	{
		DST_SOLVER,
		SRC_DST_SOLVER
	};

	ISolver(Graph& topologyGraph, string& metaData) : m_topologyGraph(topologyGraph), m_metaData(metaData){}
	virtual ~ISolver(){}

	virtual void solve() = 0;
	virtual void readPolicies(const string& policiesLine) = 0;

	virtual void splitToPolicies(const string& policiesLine, vector<string>& vPoliciesStr);

	static SolverType getType(const string& line);
	
protected:
	string m_metaData;
	Graph& m_topologyGraph;
};

typedef shared_ptr<ISolver> ISolverPtr;