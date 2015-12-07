//
//  SrcDstPolicies.cpp
//  SDNP
//
//  Created by Ron Famini on 1/10/15.
//  Copyright (c) 2015 Ron Famini. All rights reserved.
//

#include "stdafx.h"
#include <stdio.h>
#include "SrcDstPolicies.h"
#include "SequenceSolver.h"

void SrcDstPolicies::addOptionalSequencePolicy(OptionalSequencePolicyPtr& pOsp)
{
    m_vOptionalSequencesPolicies.push_back(pOsp);
}

bool SrcDstPolicies::findSrcDstPath(OUT vector<BaseVertex*>& satisfyingPath)
{
	if (m_hasAccessControl)
	{
		if (0 < m_vOptionalSequencesPolicies.size())
		{
			return false;
		}
		return true;
	}
	

	bool foundSatisfyingPath = false;
	bool vd_foundSatisfyingPath = false;
    calculateAllSatisfyingSequences();
    while (hasNextSatisfyingSequence())
    {
        VertexVectorPtr pSequence(getNextSatisfyingSequence());
        //logSequence(*pSequence, "\nSatisfying sequence: ");
		VertexVector pathWithLoops;
		if (!findShortestSatisfyingPathWithLoops(m_topologyGraph, *pSequence, m_stACVertices, m_stACEdges, pathWithLoops))
		{
			//there is no path with loops so there will be no path without them
			continue;
		}
		if (0 == m_pathWithLoops.size() || pathWithLoops.size() < m_pathWithLoops.size())
		{
			//keep the shortest path with loops
			m_pathWithLoops = pathWithLoops;
		}
		
		if (m_shouldSearchVertexDisjoint)
		{
			VertexVector vd_specificSequenceSatisfyingPath; //vertex disjoint
			if (findSatisfyingPath(m_topologyGraph, *pSequence, m_stACVertices, m_stACEdges, vd_specificSequenceSatisfyingPath, false))
			{
				vd_foundSatisfyingPath = true;
				if (0 == m_vd_satisfyingPath.size() || vd_specificSequenceSatisfyingPath.size() < m_vd_satisfyingPath.size())
				{
					//keep the shortest satisfying path
					m_vd_satisfyingPath = vd_specificSequenceSatisfyingPath;
				}
			}
		}
		
		
		
		VertexVector specificSequenceSatisfyingPath;
		if (findSatisfyingPath(m_topologyGraph, *pSequence, m_stACVertices, m_stACEdges, specificSequenceSatisfyingPath, true) ||
			(pSequence->size() == 4 &&
			findSatisfyingPath2MB(m_topologyGraph, *pSequence, m_stACVertices, m_stACEdges, specificSequenceSatisfyingPath)))
        {
			foundSatisfyingPath = true;
			if (0 == m_satisfyingPath.size() || specificSequenceSatisfyingPath.size() < m_satisfyingPath.size())
			{
				//keep the shortest satisfying path
				m_satisfyingPath = specificSequenceSatisfyingPath;
			}
        }
    }
	//cout << m_pathWithLoops.size() << "," << m_satisfyingPath.size() << "," << (m_pathWithLoops != m_satisfyingPath) << "\n";
	if (m_shouldSearchVertexDisjoint && vd_foundSatisfyingPath)
	{
		satisfyingPath = m_vd_satisfyingPath;
	}
	else
	{
		satisfyingPath = m_satisfyingPath;
	}
	
    return foundSatisfyingPath;
}

void SrcDstPolicies::calculateAllSatisfyingSequences()
{
	if (0 == m_vOptionalSequencesPolicies.size())
	{
		//if there are no sequence policy that need to be enforced the sequence is just src->dst
		VertexVectorPtr pSatisfyingSequence(new VertexVector());
		pSatisfyingSequence->push_back(m_src);
		pSatisfyingSequence->push_back(m_dst);
		m_vSatisfyingSequences.push_back(pSatisfyingSequence);
		return;
	}
    while (nextCombination())
    {
        addSequencesForCurrentCombination();
    }
}

bool SrcDstPolicies::nextCombination()
{
    if (m_isFirstCombination)
    {
        for (size_t i = 0; i < m_vOptionalSequencesPolicies.size(); i++)
        {
            if (m_vOptionalSequencesPolicies[i]->hasNextSequence())
            {
                VertexVectorPtr firstSequenceFromOptional(m_vOptionalSequencesPolicies[i]->getNextSequence());
                m_combinationOfSequences.push_back(firstSequenceFromOptional);
            }
        }
        m_isFirstCombination = false;
        return true;
    }
    
    
    for (size_t i = 0; i < m_vOptionalSequencesPolicies.size(); i++)
    {
        if (m_vOptionalSequencesPolicies[i]->hasNextSequence())
        {
            m_combinationOfSequences[i] = m_vOptionalSequencesPolicies[i]->getNextSequence();
            return true;
        }
        m_vOptionalSequencesPolicies[i]->resetToFirstSequence();
        m_combinationOfSequences[i] = m_vOptionalSequencesPolicies[i]->getNextSequence();
    }
    return false;
}

bool SrcDstPolicies::hasNextSatisfyingSequence()
{
    return (m_satisfyingSequenceIndex < m_vSatisfyingSequences.size());
}

VertexVectorPtr SrcDstPolicies::getNextSatisfyingSequence()
{
    if (m_satisfyingSequenceIndex == m_vSatisfyingSequences.size())
    {
        throw exception();
    }
    m_satisfyingSequenceIndex++;
    return m_vSatisfyingSequences[m_satisfyingSequenceIndex-1];
}

void SrcDstPolicies::addSequencesForCurrentCombination()
{
    vector<VertexListPtr> vListSequences;
    for (VertexVectorPtr& pVectorSequence : m_combinationOfSequences)
    {
        VertexListPtr pListSequence(new list<BaseVertex*>(pVectorSequence->begin(), pVectorSequence->end())); // we do not want the source and the destination because they are similar in all the sequences
        vListSequences.push_back(pListSequence);
    }
    
    VertexVector vSequenceSoFar;
    
    addSequencesRec(vListSequences, vSequenceSoFar);
}

void SrcDstPolicies::addSequencesRec(vector<VertexListPtr>& vListSequences, vector<BaseVertex*>& vSequenceSoFar)
{
    bool areAllListsEmpty = true;
    for (VertexListPtr& pListSequence : vListSequences)
    {
        if (pListSequence->empty())
        {
            continue;
        }
        areAllListsEmpty = false;
        BaseVertex* vertexToAdd = pListSequence->front();
        pListSequence->pop_front();
        vSequenceSoFar.push_back(vertexToAdd);
        
        addSequencesRec(vListSequences, vSequenceSoFar);
        
        pListSequence->push_front(vertexToAdd);
        vSequenceSoFar.pop_back();
    }
    if (!areAllListsEmpty)
    {
        return;
    }
	VertexVectorPtr pSatisfyingSequence(getValidNewSequence(vSequenceSoFar));
	if (nullptr == pSatisfyingSequence.get())
	{
		return;
	}
	m_vSatisfyingSequences.push_back(pSatisfyingSequence);
}

VertexVectorPtr SrcDstPolicies::getValidNewSequence(VertexVector& satisfyingSequence)
{
	VertexVectorPtr pValidSequence(getValidSequence(satisfyingSequence));
	if (nullptr == pValidSequence.get())
	{
		return nullptr;
	}
	for (VertexVectorPtr& pSequence : m_vSatisfyingSequences)
	{
		if ((*pValidSequence) == (*pSequence))
		{
			//this sequence already exists in m_vSatisfyingSequences, no need to add it again
			return nullptr;
		}
	}
	return pValidSequence;
}

VertexVectorPtr SrcDstPolicies::getValidSequence(VertexVector& satisfyingSequence)
{
	VertexVectorPtr pValidSequence(new VertexVector());
	for (BaseVertex* pVertex : satisfyingSequence)
	{
		auto it = find(pValidSequence->begin(), pValidSequence->end(), pVertex);
		if (it == pValidSequence->end())
		{
			//the vertex is not yet in the sequence
			pValidSequence->push_back(pVertex);
			continue;
		}
		if (it == pValidSequence->end()-1)
		{
			//the vertex appears two (or more) times in a row, we should keep only one of it's occurrences
			continue;
		}
		//TODO - fix for times we approve for a vertex to appear several times in the sequence
		//the vertex appeared earlier in the sequence. the sequence is invalid
		return nullptr;
	}
	return pValidSequence;
}

void SrcDstPolicies::addAreaAvoidancePolicy(AreaAvoidancePolicyPtr& pAaP)
{
	m_src = pAaP->getSource();
	m_dst = pAaP->getDestination();
	
	VertexVectorPtr pACVertices = pAaP->getAAVertices();
	for (BaseVertex* pVertex : *pACVertices)
	{
		m_stACVertices.insert(pVertex);
	}

	EdgeVectorPtr pACEdges = pAaP->getAAEdges();
	for (Edge edge : *pACEdges)
	{
		m_stACEdges.insert(edge);
	}
}

void SrcDstPolicies::addAccessControlPolicy(AccessControlPolicyPtr pAcPolicy)
{
	m_src = pAcPolicy->getSource();
	m_dst = pAcPolicy->getDestination();
	m_hasAccessControl = true;
}