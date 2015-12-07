#pragma once
#include "Defs.h"
#include <string>
#include "kshortestpaths_2.1\Graph.h"

class AreaAvoidancePolicy
{
public:
	AreaAvoidancePolicy(Graph& topologyGraph, string line);
	
	~AreaAvoidancePolicy();

	VertexVectorPtr getAAVertices();
	EdgeVectorPtr getAAEdges();
	BaseVertex* getDestination();
	BaseVertex* getSource();
	
private:
	Graph& m_topologyGraph;
	VertexVectorPtr m_pAAVertices;
	EdgeVectorPtr m_pAAEdges;
	BaseVertex* m_pDstVertex;
	BaseVertex* m_pSrcVertex;
	
	void readPolicyFromLine(string& line);
	void readAAVerticesAndEdges(string& ACVerticesStr);
	void addVertexOrEdge(const string& vertexOrEdgeStr);
};

typedef shared_ptr<AreaAvoidancePolicy> AreaAvoidancePolicyPtr;
