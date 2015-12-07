#include "stdafx.h"
#include "AreaAvoidancePolicy.h"
#include <boost\lexical_cast.hpp>

AreaAvoidancePolicy::AreaAvoidancePolicy(Graph& topologyGraph, string line) : m_topologyGraph(topologyGraph), m_pAAVertices(new VertexVector()), m_pAAEdges(new EdgeVector())
{
	readPolicyFromLine(line);
}

AreaAvoidancePolicy::~AreaAvoidancePolicy()
{
}

void AreaAvoidancePolicy::readPolicyFromLine(string& line)
{
	std::string delimiter = ARROW;
	string remainingLine(line);
	remainingLine.erase(0, NEGATION.length());
	
	size_t pos = remainingLine.find(delimiter);
	if (pos == 0)
	{
		m_pSrcVertex = nullptr;
	}
	else
	{
		string vertexNumberStr = remainingLine.substr(0, pos);
		m_pSrcVertex = m_topologyGraph.get_vertex(boost::lexical_cast<uint>(vertexNumberStr));
		remainingLine.erase(0, pos);
	} 
	remainingLine.erase(0, delimiter.length());


	string ACVerticesAndEdgesStr;
	pos = remainingLine.find(delimiter);
	ACVerticesAndEdgesStr = remainingLine.substr(0, pos);
	readAAVerticesAndEdges(ACVerticesAndEdgesStr);
	remainingLine.erase(0, pos + delimiter.length());
	
	m_pDstVertex = m_topologyGraph.get_vertex(boost::lexical_cast<uint>(remainingLine));
}

VertexVectorPtr AreaAvoidancePolicy::getAAVertices()
{
	return m_pAAVertices;
}

EdgeVectorPtr AreaAvoidancePolicy::getAAEdges()
{
	return m_pAAEdges;
}

BaseVertex* AreaAvoidancePolicy::getDestination()
{
	return m_pDstVertex;
}

BaseVertex* AreaAvoidancePolicy::getSource()
{
	return m_pSrcVertex;
}

void AreaAvoidancePolicy::readAAVerticesAndEdges(string& ACVerticesStr)
{
	const string delimiter = "|";
	size_t delimiter_pos = 0;
	
	std::string vertexOrEdgeStr;
	while ((delimiter_pos = ACVerticesStr.find(delimiter)) != std::string::npos)
	{
		vertexOrEdgeStr = ACVerticesStr.substr(0, delimiter_pos);
		addVertexOrEdge(vertexOrEdgeStr);
		ACVerticesStr.erase(0, delimiter_pos + delimiter.length());
	}
	addVertexOrEdge(ACVerticesStr);//the last one after the last delimiter
}

void AreaAvoidancePolicy::addVertexOrEdge(const string& vertexOrEdgeStr)
{
	const string dash = "-";
	size_t dash_pos = 0;
	if ((dash_pos = vertexOrEdgeStr.find(dash)) != std::string::npos)
	{
		//it is an edge
		int vertex1 = boost::lexical_cast<uint>(vertexOrEdgeStr.substr(0, dash_pos));
		int vertex2 = boost::lexical_cast<uint>(vertexOrEdgeStr.substr(dash_pos+1));
		m_pAAEdges->push_back(Edge(vertex1, vertex2));
		m_pAAEdges->push_back(Edge(vertex2, vertex1));
	}
	else
	{
		//it is a vertex
		m_pAAVertices->push_back(m_topologyGraph.get_vertex(boost::lexical_cast<uint>(vertexOrEdgeStr)));
	}
}
