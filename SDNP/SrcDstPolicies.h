//
//  SrcDstPolicies.h
//  SDNP
//
//  Created by Ron Famini on 1/10/15.
//  Copyright (c) 2015 Ron Famini. All rights reserved.
//

#pragma once

#include "defs.h"
#include <vector>
#include "kshortestpaths_2.1/graphelements.h"
#include "OptionalSequencePolicy.h"
#include "AreaAvoidancePolicy.h"
#include "AccessControlPolicy.h"

class SrcDstPolicies
{
public:
    
	SrcDstPolicies(Graph& topologyGraph, bool shouldSearchVertexDisjoint)
		: m_topologyGraph(topologyGraph), m_isFirstCombination(true), m_satisfyingSequenceIndex(0), 
		m_shouldSearchVertexDisjoint(shouldSearchVertexDisjoint), m_hasAccessControl(false)
    {
        //intentionally empty
    }
    
    ~SrcDstPolicies()
    {
        //intentionally empty
    }
    
    void addOptionalSequencePolicy(OptionalSequencePolicyPtr& pOsp);

	void addAreaAvoidancePolicy(AreaAvoidancePolicyPtr& pAcp);

	void addAccessControlPolicy(AccessControlPolicyPtr pAcPolicy);

    bool findSrcDstPath(OUT vector<BaseVertex*>& srcDstPath);

private:
    
    //MEMBERS
    Graph& m_topologyGraph;
    
	bool m_hasAccessControl;

    vector<OptionalSequencePolicyPtr> m_vOptionalSequencesPolicies; //all the sequence policies, where some of the vertices in the sequences are an option from a set
    VertexSet m_stACVertices; //vertices that are not allowed to be visited
	EdgeSet m_stACEdges;
	VertexVector m_satisfyingPath;
	VertexVector m_vd_satisfyingPath;
	VertexVector m_pathWithLoops;
    
    vector<VertexVectorPtr> m_vSatisfyingSequences;
    size_t m_satisfyingSequenceIndex;
    
    vector<VertexVectorPtr> m_combinationOfSequences;
    bool m_isFirstCombination; 
	BaseVertex* m_src;
	BaseVertex* m_dst;
	bool m_shouldSearchVertexDisjoint;
	//PRIVATE METHODS
    bool hasNextSatisfyingSequence();
    VertexVectorPtr getNextSatisfyingSequence();
    
    bool nextCombination();
    
    void calculateAllSatisfyingSequences();
    
    void addSequencesForCurrentCombination();
    void addSequencesRec(vector<VertexListPtr>& vListSequences, vector<BaseVertex*>& pSequenceSoFar);
	VertexVectorPtr getValidNewSequence(VertexVector& satisfyingSequence);
	//deletes two neighbour vertices that are the same
	VertexVectorPtr getValidSequence(VertexVector& satisfyingSequence);

};

typedef shared_ptr<SrcDstPolicies> SrcDstPoliciesPtr;