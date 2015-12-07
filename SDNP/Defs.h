#pragma once

#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <list>
#include "kshortestpaths_2.1/GraphElements.h"
#include <set>


#define OUT
#define IN

using namespace std;

typedef set<BaseVertex*> VertexSet;
typedef vector<BaseVertex*> VertexVector;
typedef shared_ptr<vector<BaseVertex*>> VertexVectorPtr;
typedef shared_ptr<list<BaseVertex*>> VertexListPtr;

typedef pair<int, int> Edge;
typedef vector<Edge> EdgeVector;
typedef shared_ptr<EdgeVector> EdgeVectorPtr;
typedef set<Edge> EdgeSet;

extern ofstream g_logFile;
extern string g_graphFileName;
extern string g_logFileName;
extern string g_sequencesFileName; 
extern string g_rulesFileName;

typedef unsigned int uint;


//MAGICS
extern const double STOP_RATIO;
extern const uint MAX_K;

extern int MIN_SEQUENCE_LENGTH;
extern int MAX_SEQUENCE_LENGTH;

extern int NUM_OF_SEQUENCES;

extern const string NEGATION;
extern const string ARROW; 
extern const string COMMA;
extern const string PREDICATE_SEPERATOR;
//FUNCTIONS
void logSequence(VertexVector& sequence, string header);