#include "tech_up.h"

#include "../../managers/manager_mediator.h"
#include "../../Aeolus.h"
#include "../../utils/unit_utils.h"
#include "build_structure.h"

namespace Aeolus
{
	bool TechUp::execute(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();

		std::vector<::sc2::UNIT_TYPEID> techTreeRoute;

		if (std::holds_alternative<::sc2::UPGRADE_ID>(m_target))
		{
			// case 1: the target is an upgrade
			::sc2::UPGRADE_ID upgradeId = std::get<::sc2::UPGRADE_ID>(m_target);
			auto upgrades = aeolusbot.Observation()->GetUpgrades();

			// already have this upgrade, return false
			if (std::find(upgrades.begin(), upgrades.end(), upgradeId) != upgrades.end()) return false;

			techTreeRoute = constants::UPGRADE_TECH_REQUIREMENT.at(upgradeId);
		}
		else if (constants::ALL_STRUCTURES.find(std::get<::sc2::UNIT_TYPEID>(m_target)) 
			!= constants::ALL_STRUCTURES.end())
		{
			// case 2: the target is a structure
			::sc2::UNIT_TYPEID structureId = std::get<::sc2::UNIT_TYPEID>(m_target);
			techTreeRoute = constants::UNIT_TECH_REQUIREMENT.at(structureId);
		}
		else
		{
			// case 3: the target is a unit
			::sc2::UNIT_TYPEID structureId = utils::_isTrainedFrom(std::get<::sc2::UNIT_TYPEID>(m_target)).value();
			techTreeRoute = constants::UNIT_TECH_REQUIREMENT.at(structureId);
		}

		if (techTreeRoute.empty())
		{
			std::cout << "TechUp: No tech tree found for the given unit/upgrade" << std::endl;
			return true;
		}

		// initialize "toBuild". This holds the next tech structure we want to build in the tech tree
		// in order to tech up to target. Leave empty for now
		std::optional<::sc2::UNIT_TYPEID> toBuild = std::nullopt;

		for (auto it = techTreeRoute.rbegin(); it != techTreeRoute.rend(); it++)
		{
			// iterate the tech tree, from back to front
			// try to locate the first structure that we already have
			if (mediator.IsStructureAvailable(aeolusbot, *it))
			{
				// this is the point to break the loop and build our
				// toBuild structure
				break;
			}
			else if (mediator.GetNumberPending(aeolusbot, *it) > 0)
			{
				// something is is the tech tree, is not available yet but pending, 
				// technically automatically teching up. Hence we stop the logic and 
				// return true
				return true;
			}
			else
			{
				// found a building in the tech tree, is not available, and is not pending yet
				// we want to build this structure for now
				toBuild = *it;
			}
		}

		// if there is nothing to build; we have successfully teched up and
		// no action is executed
		if (!toBuild.has_value())
		{
			return false;
		}
		
		// there is something to build, build it.
		auto cost = mediator.GetUnitCost(aeolusbot, toBuild.value());
		if (mediator.GetMinerals(aeolusbot) < cost.first || mediator.GetVespene(aeolusbot) < cost.second)
			// waiting for minerals & gas
			return true;
		return std::make_unique<BuildStructure>(toBuild.value(), 0, false)->execute(aeolusbot);
	}
}