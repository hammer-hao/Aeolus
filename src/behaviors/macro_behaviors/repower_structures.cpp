#include "repower_structures.h"

#include "../../managers/manager_mediator.h"
#include "../../Aeolus.h"
#include "../../utils/position_utils.h"
#include "../../enums.h"
#include "build_structure.h"

#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_api.h>
#include <sc2api/sc2_common.h>
#include <algorithm>

namespace Aeolus
{
	bool RepowerStructures::execute(AeolusBot& aeolusbot)
	{
		ManagerMediator& mediator = ManagerMediator::getInstance();

		auto allStructures = mediator.GetAllOwnStructures(aeolusbot);
		
		::sc2::Units allPylons;
		::sc2::Units structureWithoutPower;

		auto terrainHeight = ::sc2::HeightMap(aeolusbot.Observation()->GetGameInfo());
		
		std::copy_if(
			allStructures.begin(), allStructures.end(),
			std::back_inserter(allPylons),
			[](const ::sc2::Unit* structure)
			{
				return structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PYLON;
			}
		);

		std::copy_if(
			allStructures.begin(), allStructures.end(),
			std::back_inserter(structureWithoutPower),
			[&](const ::sc2::Unit* structure)
			{
				// units that do not required power
				if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PYLON) return false;
				if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_NEXUS) return false;
				if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR) return false;

				// check for unpowered units
				return !utils::isPowered(structure->pos, allPylons, terrainHeight, 0.0f);
			}
		);

		// all structures are powered, return false since there's nothing to execute
		if (structureWithoutPower.empty()) return false;

		const ExpansionMap& buildingPlacements = mediator.GetBuildingPlacements(aeolusbot);

		for (const auto& structure : structureWithoutPower)
		{
			if (isRepowering(aeolusbot, structure)) continue;

			if (repower(aeolusbot, structure, buildingPlacements)) return true;
		}

		return false;
	}

	bool RepowerStructures::isRepowering(AeolusBot& aeolusbot, const ::sc2::Unit* structure)
	{
		auto buildingTracker = ManagerMediator::getInstance().GetBuildingTracker(aeolusbot);

		for (const auto& job : buildingTracker)
		{
			if (job.second.building_id == ::sc2::UNIT_TYPEID::PROTOSS_PYLON)
			{
				if (::sc2::DistanceSquared2D(job.second.target, structure->pos) <= 42.25f)
				{
					return true;
				}
			}
		}
		return false;
	}

	bool RepowerStructures::repower(AeolusBot& aeolusbot, const ::sc2::Unit* structure, const ExpansionMap& allPlacements)
	{
		auto placementGrid = ::sc2::PlacementGrid(aeolusbot.Observation()->GetGameInfo());

		for (int i = 0; i < allPlacements.size(); ++i)
		{
			const auto& twoByTwos = allPlacements[i].at(BuildingTypes::BUILDING_2X2);

			std::vector<std::pair<float, float>> available;

			for (const auto& [pos, attr] : twoByTwos) {
				if (!attr.available) continue;
				if (attr.worker_on_route) continue;

				if (::sc2::DistanceSquared2D(pos.toWorld(), structure->pos) > 42.25f) continue;

				int start_x = static_cast<int>(std::round(pos.toWorld().x - 1));
				int start_y = static_cast<int>(std::round(pos.toWorld().y - 1));
				int size = 2;
				if (!utils::canPlaceStructure(start_x, start_y, size, placementGrid)) continue;

				available.push_back({ pos.toWorld().x, pos.toWorld().y });
			}

			if (!available.empty())
			{
				return std::make_unique<BuildStructure>(
					::sc2::UNIT_TYPEID::PROTOSS_PYLON,
					i,
					::sc2::Point2D(available.front().first, available.front().second)
				)->execute(aeolusbot);
			}
		}

		// somehow could not find any place to repower structures
		std::cout << "RepowerStructures: Cannot find a valid pylon position for restoring power!" << std::endl;
		return false;
	}
}