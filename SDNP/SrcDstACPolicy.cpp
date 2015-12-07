#include "stdafx.h"
#include "SrcDstACPolicy.h"

SrcDstACPolicy::SrcDstACPolicy(Graph& topologyGraph, string line) : m_topologyGraph(topologyGraph), m_pACVertices()
{
	readPolicyFromLine(line);
}

SrcDstACPolicy::~SrcDstACPolicy()
{
}