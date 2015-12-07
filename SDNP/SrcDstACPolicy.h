#pragma once
#include "Defs.h"
#include "kshortestpaths_2.1\Graph.h"

class SrcDstACPolicy
{
public:
	SrcDstACPolicy(Graph& topologyGraph, string line);
	~SrcDstACPolicy();

private:
	Graph m_topologyGraph;
	VertexVectorPtr m_pACVertices;
	BaseVertex* m_srcVertex;
	BaseVertex* m_dstVertex;
};

typedef shared_ptr<SrcDstACPolicy> SrcDstACPolicyPtr;