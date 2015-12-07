#include "stdafx.h"
#include "SrcDstSolver.h"
#include <sstream>
#include "IPolicy.h"
#include <iosfwd>
#include "AccessControlPolicy.h"

SrcDstSolver::SrcDstSolver(Graph& topologyGraph, string metaData, bool shouldSearchVertexDisjoint) : ISolver(topologyGraph, metaData), m_srcDstPolicies(topologyGraph, shouldSearchVertexDisjoint)
{

}

SrcDstSolver::~SrcDstSolver()
{

}

void SrcDstSolver::solve()
{
	if (!m_srcDstPolicies.findSrcDstPath(m_satisfyingPath))
	{
		stringstream ss;
		ofstream rulesFile;
		rulesFile.open(g_rulesFileName, ios::app);
		ss << "no path for packets: " << m_metaData << "\n";
		ss << "cannot satify all policies: " << m_policiesLine << "\n";
		rulesFile << ss.str();
		cout << ss.str();
		rulesFile.close();
		throw exception(ss.str().c_str());
	}
	else
	{
		createForwardingRules();
	}

	printRules();
}

void SrcDstSolver::readPolicies(const string& policiesLine)
{
	m_policiesLine = policiesLine;
	vector<string> vPoliciesStr;
	splitToPolicies(policiesLine, vPoliciesStr);
	for (string policyStr : vPoliciesStr)
	{
		switch (IPolicy::getPolicyType(policyStr))
		{
		case IPolicy::OPTIONAL_SEQUENCE:
		{
			OptionalSequencePolicyPtr pOsp(new OptionalSequencePolicy(m_topologyGraph, policyStr));
			m_srcDstPolicies.addOptionalSequencePolicy(pOsp);
			break;
		}
		case IPolicy::AREA_AVOIDANCE:
		{
			AreaAvoidancePolicyPtr pAaPolicy(new AreaAvoidancePolicy(m_topologyGraph, policyStr));
			m_srcDstPolicies.addAreaAvoidancePolicy(pAaPolicy);
			break;
		}
		case IPolicy::ACCESS_CONTROL:
		{
			AccessControlPolicyPtr pAcPolicy(new AccessControlPolicy(m_topologyGraph, policyStr));
			m_pSrc = pAcPolicy->getSource();
			m_srcDstPolicies.addAccessControlPolicy(pAcPolicy);
			break;
		}
		default:
			break;
		}
	}
}

void SrcDstSolver::createForwardingRules()
{
	if (0 == m_satisfyingPath.size())
	{
		//if we got here and it is empty we need to create access control
		ForwardingRulePtr pRule(new ForwardingRule(m_metaData, m_pSrc, VertexVector(), nullptr));
		m_vForwardingRules.push_back(pRule);
		return;

	}
	BaseVertex* pSrc = m_satisfyingPath[0];
	BaseVertex* pFormerSwitch = nullptr;
	for (uint i = 0; i < m_satisfyingPath.size() - 1; i++)
	{
		VertexVector vNextSwitches;
		vNextSwitches.push_back(m_satisfyingPath[i + 1]);
		ForwardingRulePtr pRule(new ForwardingRule(m_metaData, m_satisfyingPath[i], vNextSwitches, pFormerSwitch));
		m_vForwardingRules.push_back(pRule);
		pFormerSwitch = m_satisfyingPath[i];
	}
}

void SrcDstSolver::printRules()
{
	ofstream rulesFile;
	rulesFile.open(g_rulesFileName, ios::app);
	for (auto pRule : m_vForwardingRules)
	{
		pRule->printOut(rulesFile);
	}
	rulesFile.close();
}