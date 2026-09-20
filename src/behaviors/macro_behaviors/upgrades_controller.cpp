#include "upgrades_controller.h"

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include "../../constants.h"
#include "tech_up.h"
#include "build_structure.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_typeenums.h>
#include <set>
#include <algorithm>
#include <iterator>

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
			{
				unreserveResources(aeolusbot, upgrade);
				continue;
			}
			if (std::find_if(allStructures.begin(), allStructures.end(),
				[&](const ::sc2::Unit* structure)
				{
					if (structure->unit_type != researchedFrom) return false;
					if (structure->orders.empty()) return false;
					auto researching = structure->orders.front().ability_id;
					if (upgrade == ::sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL1)
					{
						return (researching == ::sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS ||
							researching == ::sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL1);
					}
					if (upgrade == ::sc2::UPGRADE_ID::PROTOSSAIRARMORSLEVEL1)
					{
						return (researching == ::sc2::ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONS ||
							researching == ::sc2::ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONSLEVEL1);
					}
					return researching == researchAbility;
				}) != allStructures.end())
			{
				unreserveResources(aeolusbot, upgrade);
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
			std::copy_if(
				allStructures.begin(), allStructures.end(),
				std::back_inserter(allUpgradeBuildings),
				[&](const ::sc2::Unit* structure)
				{
					return structure->unit_type == researchedFrom
						&& structure->build_progress >= 1.0f
						&& structure->orders.empty();
				});

			if (allUpgradeBuildings.empty())
			{
				// not a single one of this upgrade building, build one
				if (!std::any_of(allStructures.begin(), allStructures.end(), [&](const ::sc2::Unit* structure) {
					return structure->unit_type == researchedFrom;
					}))
				{
					auto cost = mediator.GetUnitCost(aeolusbot, researchedFrom);
					if (mediator.GetMinerals(aeolusbot) >= cost.first && mediator.GetVespene(aeolusbot) >= cost.second)
					{
						return std::make_unique<BuildStructure>(researchedFrom, 0, false)->execute(aeolusbot);
					}
					// waiting for minerals & gas, return true for now
					return true;
				}
				// waiting for building to complete / order to finish
				return true;
			}

			// upgrade requires another building to finish first, wait it out
			if (upgrade == ::sc2::UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL2 ||
				upgrade == ::sc2::UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL3 ||
				upgrade == ::sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL2 ||
				upgrade == ::sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL3 ||
				upgrade == ::sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL2 ||
				upgrade == ::sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL3)
			{
				if (!std::any_of(allStructures.begin(), allStructures.end(), [](const ::sc2::Unit* structure) {
					return structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL &&
						structure->build_progress >= 0.95f;
					}))
				{
					return true;
				}
			}
			if (upgrade == ::sc2::UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL2 ||
				upgrade == ::sc2::UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL3 ||
				upgrade == ::sc2::UPGRADE_ID::PROTOSSAIRARMORSLEVEL2 ||
				upgrade == ::sc2::UPGRADE_ID::PROTOSSAIRARMORSLEVEL3)
			{
				if (!std::any_of(allStructures.begin(), allStructures.end(), [](const ::sc2::Unit* structure) {
					return structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_FLEETBEACON &&
						structure->build_progress >= 0.95f;
					}))
				{
					return true;
				}
			}

			// reserve minerals and gas for the upgrade if not already
			if (reserved_upgrades.find(upgrade) == reserved_upgrades.end())
			{
				auto researchCost = mediator.GetUpgradeCost(aeolusbot, upgrade);
				mediator.ReserveMinerals(aeolusbot, researchCost.first);
				mediator.ReserveVespene(aeolusbot, researchCost.second);
				reserved_upgrades.insert(upgrade);
			}

			for (const auto& building : allUpgradeBuildings)
			{
				if (building->build_progress >= 1.0f && building->orders.empty())
				{
					aeolusbot.Actions()->UnitCommand(building, researchAbility);
					return true;
				}
			}

			// found building, but is not finished/currently researching something else
			return true;
		}

		return false;
	}

	void UpgradesController::unreserveResources(AeolusBot& aeolusbot, ::sc2::UPGRADE_ID upgrade)
	{
		if (reserved_upgrades.erase(upgrade) == 0)
			return;

		auto& mediator = ManagerMediator::getInstance();
		auto cost = mediator.GetUpgradeCost(aeolusbot, upgrade);
		mediator.FreeMinerals(aeolusbot, cost.first);
		mediator.FreeVespene(aeolusbot, cost.second);
	}
}