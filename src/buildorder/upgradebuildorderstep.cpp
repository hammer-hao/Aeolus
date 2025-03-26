#include "upgradebuildorderstep.h"

#include "buildorderstep.h"
#include <algorithm>
#include <iterator>
#include <sc2api/sc2_data.h>
#include <sc2api/sc2_unit.h>
#include "../Aeolus.h"
#include "../managers/manager_mediator.h"
#include "../constants.h"

namespace Aeolus
{
	UpgradeBuildOrderStep::UpgradeBuildOrderStep(int supply_threshold, 
		::sc2::UPGRADE_ID to_research) : 
		m_supply_threshold(supply_threshold), m_to_research(to_research)
	{
	}

	bool UpgradeBuildOrderStep::execute(AeolusBot& aeolusbot)
	{
		// get the building type that can research the upgrade
		::sc2::UNIT_TYPEID researched_from = constants::isResearchedFrom(m_to_research);

		auto all_structures = ManagerMediator::getInstance().GetAllOwnStructures(aeolusbot);
		::sc2::Units can_research;

		std::copy_if(all_structures.begin(), all_structures.end(), std::back_inserter(can_research),
			[researched_from](const ::sc2::Unit* unit) { return unit->unit_type == researched_from; });

		auto existingUpgrades = aeolusbot.Observation()->GetUpgrades();
		if (std::find(existingUpgrades.begin(), existingUpgrades.end(), m_to_research) != existingUpgrades.end())
			return false; // already researched this upgrade

		if (can_research.empty()) return false; // upgrade not available via any structure

		if (m_to_research == ::sc2::UPGRADE_ID::BLINKTECH
			&& (aeolusbot.Observation()->GetMinerals() < 150 || aeolusbot.Observation()->GetVespene() < 150))
			return false;

		for (const auto& structure : can_research)
		{
			if (structure->orders.empty() && structure->build_progress >= 1.0)
			{
				aeolusbot.Actions()->UnitCommand(structure,
					ManagerMediator::getInstance().GetUpgradeCreationAbility(aeolusbot, m_to_research));
				std::cout << "EXECUTED UPGRADE ID " << 
					::sc2::AbilityTypeToName(ManagerMediator::getInstance().GetUpgradeCreationAbility(aeolusbot, m_to_research)) << std::endl;
				return true;
			}
		}
		return false;
	}

	int UpgradeBuildOrderStep::getSupplyThreshold()
	{
		return m_supply_threshold;
	}

	std::string_view UpgradeBuildOrderStep::toString()
	{
		return ::sc2::UpgradeIDToName(m_to_research);
	}
}