#include "upgrades_controller.h"

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include "../../constants.h"
#include "tech_up.h"
#include "build_structure.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_typeenums.h>

namespace Aeolus
{
	bool UpgradesController::execute(AeolusBot& aeolusbot)
	{
		ManagerMediator& mediator = ManagerMediator::getInstance();
		auto allStructures = mediator.GetAllOwnStructures(aeolusbot);
		auto existingUpgrades = aeolusbot.Observation()->GetUpgrades();

		for (const auto& upgrade : m_upgrades)
		{
			auto researchedFrom = constants::isResearchedFrom(upgrade);
			auto researchAbility = mediator.GetUpgradeCreationAbility(aeolusbot, upgrade);

			// if we have researched, or have a building researching this, continue
			if (std::find(existingUpgrades.begin(), existingUpgrades.end(), upgrade)
				!= existingUpgrades.end())
				continue;
			if (std::find_if(allStructures.begin(), allStructures.end(),
				[&](const ::sc2::Unit* structure)
				{
					if (structure->unit_type != researchedFrom) return false;
					if (structure->orders.empty()) return false;
					return structure->orders.front().ability_id == researchAbility;
				}) != allStructures.end())
			{
				continue;
			}

			// now we arrive at the upgrade we want to research
			// first, tech up. If still teching up, we are technically making progress.
			if (std::make_unique<TechUp>(upgrade)->execute(aeolusbot))
			{
				return true;
			}

			// gather all buildings available for research
			::sc2::Units allUpgradeBuildings;
			std::copy_if(allStructures.begin(), allStructures.end(), std::back_inserter(allUpgradeBuildings),
				[&](const ::sc2::Unit* structure) { return structure->unit_type == researchedFrom; });

			if (allUpgradeBuildings.empty())
			{
				// not a single one of this upgrade building, build one
				auto cost = mediator.GetUnitCost(aeolusbot, researchedFrom);
				if (mediator.GetMinerals(aeolusbot) >= cost.first && mediator.GetVespene(aeolusbot) >= cost.second)
				{
					return std::make_unique<BuildStructure>(researchedFrom, 0, false)->execute(aeolusbot);
				}
				// waiting for minerals & gas, return false for now
				return false;
			}

			for (const auto& building : allUpgradeBuildings)
			{
				if (building->build_progress >= 1.0f && building->orders.empty())
				{
					// TODO: should check for mineral/gas here
					aeolusbot.Actions()->UnitCommand(building, researchAbility);
					return true;
				}
			}

			// found building, but is not finished/currently researching something else
			return false;
		}

		return false;
	}
}