//
//  OptionalSequencePolicy.cpp
//  SDNP
//
//  Created by Ron Famini on 1/10/15.
//  Copyright (c) 2015 Ron Famini. All rights reserved.
//

#include "stdafx.h"
#include <stdio.h>
#include "OptionalSequencePolicy.h"
#include <boost/lexical_cast.hpp>

OptionalSequencePolicy::OptionalSequencePolicy(Graph& topologyGraph, const string& optionalSequenceLine) :
    m_topologyGraph(topologyGraph), m_hasNextSequence(true)
{
    readOptionalSequenceFromLine(optionalSequenceLine);
    initVCurIndexes();
}

void OptionalSequencePolicy::readOptionalSequenceFromLine(const string& optionalSequenceLine)
{
    string remainingLine(optionalSequenceLine);

    std::string delimiter = ARROW;
    size_t pos = 0;
    string optionalVerticesStr;
    while ((pos = remainingLine.find(delimiter)) != std::string::npos)
    {
        optionalVerticesStr = remainingLine.substr(0, pos);
        readOptionalVertices(optionalVerticesStr);
        remainingLine.erase(0, pos + delimiter.length());
    }
    readOptionalVertices(remainingLine);
}

void OptionalSequencePolicy::readOptionalVertices(string& optionalVerticesStr)
{
    VertexVectorPtr optionalVertices(new vector<BaseVertex*>());
    std::string delimiter = "|";
    size_t pos = 0;
    std::string vertexNumberStr;
    while ((pos = optionalVerticesStr.find(delimiter)) != std::string::npos)
    {
        vertexNumberStr = optionalVerticesStr.substr(0, pos);
        optionalVertices->push_back(m_topologyGraph.get_vertex(boost::lexical_cast<uint>(vertexNumberStr)));
        optionalVerticesStr.erase(0, pos + delimiter.length());
    }
    optionalVertices->push_back(m_topologyGraph.get_vertex(boost::lexical_cast<uint>(optionalVerticesStr)));//the last node after the last delimiter
    
    m_optionalSequence.push_back(optionalVertices);
}

VertexVectorPtr OptionalSequencePolicy::getNextSequence()
{
    if (!m_hasNextSequence)
    {
        return nullptr;
    }
    VertexVectorPtr pSequence(new vector<BaseVertex*>());
    for (size_t i=0; i < m_optionalSequence.size(); i++)
    {
        pSequence->push_back((*m_optionalSequence[i])[m_vCurIndexes[i]]);
    }
    incrementSequences();
    return pSequence;
}

void OptionalSequencePolicy::incrementSequences()
{
    for (int i = (int)m_vCurIndexes.size()-1; i >= 0; i--)
    {
        if (m_vCurIndexes[i] < m_optionalSequence[i]->size()-1)
        {
            m_vCurIndexes[i]++;
            return;
        }
        m_vCurIndexes[i] = 0; // restarting this index, others will be incremented
    }
    m_hasNextSequence = false;
}

bool OptionalSequencePolicy::hasNextSequence()
{
    return m_hasNextSequence;
}

void OptionalSequencePolicy::resetToFirstSequence()
{
    m_hasNextSequence = true;
    initVCurIndexes();
}

void OptionalSequencePolicy::initVCurIndexes()
{
    m_vCurIndexes.clear();
    for (size_t i=0; i<m_optionalSequence.size(); i++)
    {
        m_vCurIndexes.push_back(0);
    }
}

BaseVertex* OptionalSequencePolicy::getSource()
{
	return (*m_optionalSequence[0])[0];
}

BaseVertex* OptionalSequencePolicy::getDestination()
{
	return (*m_optionalSequence[m_optionalSequence.size() - 1])[0];
}
