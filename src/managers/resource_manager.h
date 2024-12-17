#pragma once

#include "manager.h"
#include "../constants.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_interfaces.h>
#include <unordered_map>

// Forward declaration to avoid circular dependency
namespace Aeolus {
	class ManagerMediator;  // Forward declaration
}

namespace Aeolus
{
	class ResourceManager : public Manager
	{
	public:
		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		sc2::Units _getAllMineralPatches(AeolusBot& aeolusbot);

		void AssignWorkerToPatch(const ::sc2::Unit* worker, const ::sc2::Unit* patch);

		void ClearAssignment(const ::sc2::Unit* worker);

		void AssignInitialWorkers(AeolusBot& aeolusbot);

		// Implement the update method.
		void update(int iteration) override {
			// std::cout << "Updating UnitRoleManager at iteration: " << iteration << std::endl;
		}

	private:

		bool m_initial_workers_assigned = false;

		::sc2::Units m_all_minerals;

		std::unordered_map<const ::sc2::Unit*, ::sc2::Units> m_patch_to_workers;

		std::unordered_map<const ::sc2::Unit*, const ::sc2::Unit*> m_worker_to_patch;

		std::unordered_map<const ::sc2::Unit*, ::sc2::Point2D> m_mineral_gathering_points;

		void CalculateMineralGatheringPoints(
			AeolusBot& aeolusbot, 
			std::vector<::sc2::Point3D> expansion_locations);
		
	};
}
