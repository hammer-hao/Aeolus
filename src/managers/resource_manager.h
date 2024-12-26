#pragma once

#include "manager.h"
#include "../constants.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_interfaces.h>
#include <unordered_map>

// Forward declaration to avoid circular dependency
namespace Aeolus {
	class ManagerMediator;  // Forward declaration
	class AeolusBot;
}

namespace Aeolus
{
	class ResourceManager : public Manager
	{
	public:
		ResourceManager(AeolusBot& aeolusbot) : m_bot(aeolusbot)
		{
		}

		std::string_view GetName() const override {
			static const std::string name = "ResourceManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		sc2::Units _getAllMineralPatches(AeolusBot& aeolusbot);

		void AssignWorkerToPatch(const ::sc2::Unit* worker, const ::sc2::Unit* patch);

		void ClearAssignment(const ::sc2::Unit* worker);

		void AssignInitialWorkers();

		::sc2::Units GetAvailableMinerals();

		// Implement the update method.
		void update(int iteration) override;

	private:

		bool m_initial_workers_assigned = false;

		::sc2::Units m_all_minerals;

		std::unordered_map<const ::sc2::Unit*, ::sc2::Units> m_patch_to_workers;

		std::unordered_map<const ::sc2::Unit*, const ::sc2::Unit*> m_worker_to_patch;

		std::unordered_map<const ::sc2::Unit*, ::sc2::Point2D> m_mineral_gathering_points;

		void CalculateMineralGatheringPoints(
			AeolusBot& aeolusbot, 
			std::vector<::sc2::Point3D> expansion_locations);

		void _assignWorkersToMineralPatches(::sc2::Units workers, ::sc2::Units patches);
		AeolusBot& m_bot;
	};
}
