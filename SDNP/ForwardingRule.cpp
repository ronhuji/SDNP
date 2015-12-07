#include "stdafx.h"
#include "ForwardingRule.h"
#include <string>
#include <sstream>

ForwardingRule::ForwardingRule(const string& metaData, BaseVertex* pSwitch, const VertexVector& pNextSwitches, BaseVertex* pFormerSwitch /*= nullptr*/) : 
	m_metaData(metaData), m_pCurrentSwitch(pSwitch), m_nextSwitches(pNextSwitches), m_pFormerSwitch(pFormerSwitch)
{

}

ForwardingRule::~ForwardingRule()
{
	//intentionally empty
}

void ForwardingRule::printOut() const
{
	ofstream rulesFile;
	rulesFile.open(g_rulesFileName, ios::app);
	string nextSwitches = getSwitchesString();
	rulesFile << m_metaData << PREDICATE_SEPERATOR << "current_switch=" << m_pCurrentSwitch->getID();
	if (nullptr != m_pFormerSwitch)
	{
		rulesFile << PREDICATE_SEPERATOR << "former_switch=" << m_pFormerSwitch->getID();
	}
	rulesFile << PREDICATE_SEPERATOR << "next_switches=" << nextSwitches << "\n";
	rulesFile.close();
}

void ForwardingRule::printOut(ofstream& rulesFile) const
{
	string nextSwitches = getSwitchesString();
	rulesFile << m_metaData << PREDICATE_SEPERATOR << "current_switch=" << m_pCurrentSwitch->getID();
	if (nullptr != m_pFormerSwitch)
	{
		rulesFile << PREDICATE_SEPERATOR << "former_switch=" << m_pFormerSwitch->getID();
	}
	rulesFile << PREDICATE_SEPERATOR << "next_switches=" << nextSwitches << "\n";
}

std::string ForwardingRule::getSwitchesString() const
{
	stringstream nextSwitchesStream;
	nextSwitchesStream << "[";
	for (BaseVertex* pVertex : m_nextSwitches)
	{
		nextSwitchesStream << pVertex->getID() << ',';
	}
	nextSwitchesStream << "]";
	return nextSwitchesStream.str();
}
