#pragma once

#include "manager.h"
#include <sc2api/sc2_common.h>
#include <unordered_map>

namespace Aeolus
{
	class AeolusBot;

	struct BuildingOrder {
		::sc2::UNIT_TYPEID building_id = ::sc2::UNIT_TYPEID::PROTOSS_PYLON;
		::sc2::Point2D target = {0, 0};
		double time_requested = 0.0;
		bool order_complete = false;

		BuildingOrder(
			::sc2::UNIT_TYPEID building_id = ::sc2::UNIT_TYPEID::PROTOSS_PYLON,
			::sc2::Point2D target = { 0, 0 },
			double time_requested = 0.0,
			bool order_complete = false
		) : building_id(building_id), 
			target(target),
			time_requested(time_requested),
			order_complete(order_complete)
		{
		}
	};

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

		size_t _getNumPending(::sc2::UNIT_TYPEID structure_type);

		void _handleConstructionOrders();
	};
}