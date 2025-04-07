#include "chronobuildorderstep.h"
#include <string>
#include "../Aeolus.h"
#include "../managers/manager_mediator.h"

namespace Aeolus
{
	bool ChronoBuildOrderStep::execute(AeolusBot& aeolusbot)
	{
		ManagerMediator& mediator = ManagerMediator::getInstance();
		
		::sc2::Units nexi = mediator.GetOwnReadyTownHalls(aeolusbot);

		for (const auto& nexus : nexi)
		{
			if (nexus->energy >= 50)
			{
				::sc2::Units all_structures = mediator.GetAllOwnStructures(aeolusbot);
				for (const auto& structure : all_structures)
				{
					if (structure->unit_type == m_to_chrono && structure->buffs.empty()
						&& !(structure->orders.empty()) && structure->build_progress >= 1.0)
					{
						aeolusbot.Actions()->UnitCommand(
							nexus,
							::sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST,
							structure);
						m_started = true;
						return true;
					}
				}
			}
		}
		m_started = false;
		return false;
	}

	int ChronoBuildOrderStep::getSupplyThreshold()
	{
		return m_supply_threshold;
	}

	std::string_view ChronoBuildOrderStep::toString()
	{
		return ::sc2::UnitTypeToName(m_to_chrono);
	}

	bool ChronoBuildOrderStep::isDone(AeolusBot& aeolusbot)
	{
		return true;
	}

	bool ChronoBuildOrderStep::started()
	{
		return m_started;
	}
}