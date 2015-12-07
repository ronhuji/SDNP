#pragma once
#include "Defs.h"
#include "kshortestpaths_2.1\GraphElements.h"
#include <memory>

struct ForwardingRule
{
public:
	BaseVertex* m_pCurrentSwitch;
	BaseVertex* m_pFormerSwitch;
	VertexVector m_nextSwitches;
	string m_metaData;

	ForwardingRule(const string& metaData, BaseVertex* pSwitch, const VertexVector& pNextSwitches, BaseVertex* pFormerSwitch = nullptr);

	~ForwardingRule();
	void printOut() const;
	void printOut(ofstream& rulesFile) const;
	string getSwitchesString() const;
private:

};

typedef shared_ptr<ForwardingRule> ForwardingRulePtr;
