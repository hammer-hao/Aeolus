#include "scoutingbuildorderstep.h"

#include "buildorderstep.h"
#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include <vector>
#include <sc2api/sc2_unit.h>
#include "../utils/game_utils.h"
#include "../utils/position_utils.h"
#include "../../constants.h"

namespace Aeolus
{
	ScoutingBuildOrderStep::ScoutingBuildOrderStep(int supply_threshold, bool scout_enemy_base, bool scout_own_half_of_map) :
		m_supply_threshold(supply_threshold), m_scout_enemy_base(scout_enemy_base),
		m_scout_own_half_of_map(scout_own_half_of_map), m_started(false)
	{
	}

	int ScoutingBuildOrderStep::getSupplyThreshold()
	{
		return m_supply_threshold;
	}

	bool ScoutingBuildOrderStep::execute(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();

		m_started = true;

		::sc2::Units allOwnWorkers = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::GATHERING);
		::sc2::Units nonMineralCarryingWorkers;

		std::copy_if(allOwnWorkers.begin(), allOwnWorkers.end(), std::back_inserter(nonMineralCarryingWorkers),
			[](const ::sc2::Unit* worker) {
				return !utils::IsWorkerCarryingResource(worker);
			});

		auto enemySpawn = mediator.GetExpansionLocations(aeolusbot).back();
		const ::sc2::Unit* bestCandidate = utils::GetClosestUnitTo(enemySpawn, nonMineralCarryingWorkers);

		mediator.AssignRole(aeolusbot, bestCandidate, constants::UnitRole::SCOUTING);

		size_t numBases = mediator.GetExpansionLocations(aeolusbot).size();

		std::vector<int> basesToScout;
		if (m_scout_own_half_of_map)
		{
			basesToScout = { 1, 2, 3, 4, 5 };
		}
		if (m_scout_enemy_base)
		{
			basesToScout.push_back(numBases - 1);
		}
		mediator.registerScout(aeolusbot, bestCandidate, basesToScout);
		return true;
	}

	std::string_view ScoutingBuildOrderStep::toString()
	{
		return "Scouting Build Order";
	}

	bool ScoutingBuildOrderStep::isDone(AeolusBot& aeolusbot)
	{
		return m_started; // started = isDone because logic is instantaneous.
	}

	bool ScoutingBuildOrderStep::started()
	{
		return m_started;
	}
}