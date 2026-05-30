#pragma once

#include "manager.h"
#include "../enums.h"
#include <sc2api/sc2_common.h>
#include <unordered_map>

namespace Aeolus
{
	class AeolusBot;

	class BuildingManager : public Manager
	{
	public:
		BuildingManager(AeolusBot& aeolusbot) : m_bot(aeolusbot)
		{
		}

		std::string_view GetName() const override {
			static const std::string name = "BuildingManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		void OnUnitDestroyed(const ::sc2::Unit* unit);

		void update(int iteration) override;

	private:
		AeolusBot& m_bot;

		/*
		@brief have a specific worker build a structure.
			@param worker: The chose worker.
			@param structure_type: What type of structure to build.
			@param position: Where to build the structure.
			@param assign_role: Assign BUILDING UnitRole to the worker?
			@return Whether the position for building is found and worker is available.
		*/
		bool _buildWithSpecificWorker(const ::sc2::Unit* worker, ::sc2::UNIT_TYPEID structure_type,
			::sc2::Point2D position, bool assign_role = true);

		std::unordered_map<const ::sc2::Unit*, BuildingOrder> m_building_tracker;

		std::unordered_map<::sc2::UNIT_TYPEID, int> m_building_counter;

		size_t _getNumPending(::sc2::UNIT_TYPEID structure_type);

		const std::unordered_map<const ::sc2::Unit*, BuildingOrder>& getBuildingTracker() const;

		void _handleConstructionOrders();

		void _clearBuildingOrders();
	};
}