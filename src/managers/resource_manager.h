#pragma once

#include "manager.h"
#include "../constants.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_interfaces.h>
#include <unordered_map>
#include <optional>

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

		void OnUnitDestroyed(const ::sc2::Unit* unit);

		::sc2::Units GetAvailableMinerals();

		// Implement the update method.
		void update(int iteration) override;

	private:

		bool m_initial_workers_assigned = false;

		::sc2::Units m_all_minerals;

		std::unordered_map<const ::sc2::Unit*, ::sc2::Units> m_patch_to_workers;

		std::unordered_map<const ::sc2::Unit*, ::sc2::Units> m_geyser_to_workers;

		std::unordered_map<const ::sc2::Unit*, const ::sc2::Unit*> m_worker_to_patch;

		std::unordered_map<const ::sc2::Unit*, const ::sc2::Unit*> m_worker_to_geyser;

		std::map<std::pair<float, float>, ::sc2::Point2D> m_mineral_gathering_points;

		void _removeWorkerFromMineral(const ::sc2::Unit* worker);

		std::optional<const ::sc2::Unit*> _selectWorker(::sc2::Point2D target_position);

		void CalculateMineralGatheringPoints(
			AeolusBot& aeolusbot, 
			std::vector<::sc2::Point2D> expansion_locations);

		void _assignWorkersToMineralPatches(::sc2::Units workers, ::sc2::Units patches);
		void _assignWorkersToGasBuildings(const ::sc2::Units& workers, const ::sc2::Units& gas_buildings);
		AeolusBot& m_bot;
	};
}
