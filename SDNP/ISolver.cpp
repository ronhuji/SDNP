#include "stdafx.h"
#include "ISolver.h"

const string DST_POLICY_STRING = "DST";
const string SRC_DST_POLICY_STRING = "SRC_DST";

void ISolver::splitToPolicies(const string& policiesLine, vector<string>& vPoliciesStr)
{
	string remainingLine(policiesLine);

	std::string delimiter = COMMA;
	size_t pos = 0;
	while ((pos = remainingLine.find(delimiter)) != std::string::npos)
	{
		vPoliciesStr.push_back(remainingLine.substr(0, pos));
		remainingLine.erase(0, pos + delimiter.length());
	}
	vPoliciesStr.push_back(remainingLine.substr(0, pos));
}

ISolver::SolverType ISolver::getType(const string& line)
{
	if (line == DST_POLICY_STRING)
	{
		return DST_SOLVER;
	}
	
	if (line == SRC_DST_POLICY_STRING)
	{
		return SRC_DST_SOLVER;
	}

	throw exception();
}

