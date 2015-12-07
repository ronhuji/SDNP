#include "stdafx.h"
#include "Defs.h"

ofstream g_logFile;
string g_graphFileName;
string g_logFileName("C:\\Users\\famini.HUN7\\Google Drive\\Visual Studio 2013\\Projects\\SDNP\\Data\\Results\\log.res");
string g_sequencesFileName;
string g_rulesFileName;

//MAGICS
const double STOP_RATIO = 3;
const uint MAX_K = 1;

int MIN_SEQUENCE_LENGTH = 4;
int MAX_SEQUENCE_LENGTH = 5;

int NUM_OF_SEQUENCES = 10;

static const string NEGATION = "!";
static const string ARROW = "->";
static const string COMMA = ",";
static const string PREDICATE_SEPERATOR = "--__--";

void logSequence(std::vector<BaseVertex*>& sequence, string header)
{
	g_logFile.open(g_logFileName, ios::app);
	g_logFile << header;
    for (BaseVertex* v : sequence)
	{
		g_logFile << v->getID() << ", ";
	}
	g_logFile << "\n";
	g_logFile.close();
}