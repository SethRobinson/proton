#include "PlatformPrecomp.h"
#include "ScriptAccelerator.h"
#include "util/MiscUtils.h"

ScriptAccelerator::ScriptAccelerator()
{
}

ScriptAccelerator::~ScriptAccelerator()
{
}

void ScriptAccelerator::Kill()
{
	m_data.clear();
}

void ScriptAccelerator::AddPosition(string label, int current)
{
	label = ToUpperCaseString(label);
	m_data[label] = ScriptPosition(current);
}

ScriptPosition * ScriptAccelerator::GetPositionByName( string label )
{
	label = ToUpperCaseString(label);

	ScriptMap::iterator itor = m_data.find(label);

	if (itor != m_data.end())
	{
		//bingo!
		return &(itor->second);
	}

	return NULL;

}