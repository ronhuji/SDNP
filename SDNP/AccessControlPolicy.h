#pragma once

#include "Defs.h"
#include <string>
#include "kshortestpaths_2.1\Graph.h"

class AccessControlPolicy
{
public:
	AccessControlPolicy(Graph& topologyGraph, string line);

	~AccessControlPolicy();

	BaseVertex* getDestination();
	BaseVertex* getSource();

private:
	Graph& m_topologyGraph;
	BaseVertex* m_pDstVertex;
	BaseVertex* m_pSrcVertex;

	void readPolicyFromLine(string& line);
};

typedef shared_ptr<AccessControlPolicy> AccessControlPolicyPtr;
