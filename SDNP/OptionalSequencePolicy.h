//
//  OptionalSequencePolicy.h
//  SDNP
//
//  Created by Ron Famini on 1/10/15.
//  Copyright (c) 2015 Ron Famini. All rights reserved.
//

#pragma once
#include "defs.h"
#include "kshortestpaths_2.1/Graph.h"

class OptionalSequencePolicy
{
public:
    OptionalSequencePolicy(Graph& topologyGraph, const string& optionalSequenceLine);

    ~OptionalSequencePolicy()
    {
        //intentionally empty
    }
    
    
    VertexVectorPtr getNextSequence();
    bool hasNextSequence();
    void resetToFirstSequence();

	BaseVertex* getSource();
	BaseVertex* getDestination();
    
private:
    Graph& m_topologyGraph;
    vector<VertexVectorPtr> m_optionalSequence;
    vector<size_t> m_vCurIndexes;
    bool m_hasNextSequence;
    
    void readOptionalSequenceFromLine(const string& optionalSequenceLine);
    void readOptionalVertices(string& optionalVerticesStr);
    void incrementSequences();
    void initVCurIndexes();
};

typedef shared_ptr<OptionalSequencePolicy> OptionalSequencePolicyPtr;
