#include "structurebuildorderstep.h"
#include "../Aeolus.h"
#include "../behaviors/macro_behaviors/build_structure.h"
#include <string>
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	int StructureBuildOrderStep::getSupplyThreshold()
	{
		return m_supply_threshold;
	}

	bool StructureBuildOrderStep::execute(AeolusBot& aeolusbot)
	{
		if (aeolusbot.Observation()->GetFoodUsed() >= m_supply_threshold)
		{
			std::make_unique<BuildStructure>(m_to_build, 0, m_is_wall).get()->execute(aeolusbot);
			return true;
		}
		return false;
	}

	std::string_view StructureBuildOrderStep::toString()
	{
		return ::sc2::UnitTypeToName(m_to_build);
	}
}