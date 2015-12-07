#pragma once
#include "Defs.h"
#include <algorithm>

class IPolicy
{
public:

	enum PolicyType
	{
		OPTIONAL_SEQUENCE,
		AREA_AVOIDANCE,
		DEFAULT_DAG,
		ACCESS_CONTROL,
		UNKNOWN
	};

	static PolicyType getPolicyType(string line)
	{
		if (0 == line.find(NEGATION))
		{
			if (1 == count(line.begin(), line.end(), '>'))
			{
				return ACCESS_CONTROL;
			}
			return AREA_AVOIDANCE;
		}
		if (1 == count(line.begin(), line.end(), '>'))
		{
			return DEFAULT_DAG;
		}
		else
		{
			return OPTIONAL_SEQUENCE;
		}
	}

private:

};