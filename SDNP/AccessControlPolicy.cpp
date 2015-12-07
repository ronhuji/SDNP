#include "stdafx.h"
#include "AccessControlPolicy.h"
#include <boost\lexical_cast.hpp>

AccessControlPolicy::AccessControlPolicy(Graph& topologyGraph, string line) : m_topologyGraph(topologyGraph)
{
	readPolicyFromLine(line);
}

AccessControlPolicy::~AccessControlPolicy()
{
}

void AccessControlPolicy::readPolicyFromLine(string& line)
{
	std::string delimiter = ARROW;
	string remainingLine(line);
	remainingLine.erase(0, NEGATION.length());

	size_t pos = remainingLine.find(delimiter);
	if (pos == 0)
	{
		throw exception();
	}
	else
	{
		string vertexNumberStr = remainingLine.substr(0, pos);
		m_pSrcVertex = m_topologyGraph.get_vertex(boost::lexical_cast<uint>(vertexNumberStr));
		remainingLine.erase(0, pos);
	}
	remainingLine.erase(0, delimiter.length());
	m_pDstVertex = m_topologyGraph.get_vertex(boost::lexical_cast<uint>(remainingLine));
}

BaseVertex* AccessControlPolicy::getDestination()
{
	return m_pDstVertex;
}

BaseVertex* AccessControlPolicy::getSource()
{
	return m_pSrcVertex;
}
