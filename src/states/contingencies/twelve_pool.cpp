#include "twelve_pool.h"

#include "../contingency.h"
#include "../base_state.h"
#include "../bot_state.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_common.h"
#include "sc2api/sc2_typeenums.h"

#include "../../behaviors/macro_behaviors/build_structure.h"
#include "../../behaviors/macro_behaviors/mining.h"
#include "../../behaviors/macro_behaviors/scout.h"
#include "../../behaviors/macro_behaviors/spawn_controller.h"
#include "../../behaviors/macro_behaviors/auto_supply.h"
#include "../../behaviors/macro_behaviors/build_workers.h"
#include "../../behaviors/macro_behaviors/chrono_controller.h"
#include "../../behaviors/macro_behaviors/repower_structures.h"

#include "../consolidate.h"

#include "../../Aeolus.h"

namespace Aeolus
{
	std::string_view TwelvePool::getName() const
	{
		return "TWELVE_POOL";
	}

	void TwelvePool::micro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);

		int baseToDefend = 1; // defend the main base
		::sc2::Point2D target = mediator.GetDefenseTarget(aeolusbot, baseToDefend);
		doGeneralMicro(aeolusbot, forces, target);
	}

	void TwelvePool::macro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();

		aeolusbot.RegisterBehavior(std::make_unique<Mining>());
		aeolusbot.RegisterBehavior(std::make_unique<Scout>());

		std::map<::sc2::UNIT_TYPEID, float> army_comp = { {::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0} };

		// handle constructing
		int count_gateways = mediator.GetNumberPending(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
		int count_cybercore = mediator.GetNumberPending(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
		int count_batteries = mediator.GetNumberPending(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY);
		bool has_available_gateway = false;
		int count_available_cybercore = 0;
		auto all_structures = mediator.GetAllOwnStructures(aeolusbot);
		for (const auto& structure : all_structures)
		{
			if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY)
			{
				if (structure->build_progress >= 1.0 && structure->orders.empty())
				{
					has_available_gateway = true;
				}
				count_gateways++;
			}
			if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE)
			{
				count_cybercore++;
				if (structure->build_progress >= 1.0f)
				{
					count_available_cybercore++;
				}
			}
			if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY)
			{
				count_batteries++;
			}
		}

		const int base_number = 1; // build at natural base
		const bool is_wall = true; // build as part of the wall

		if (count_gateways < 2)
		{
			aeolusbot.RegisterBehavior(std::make_unique<BuildStructure>(::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, m_rebuild_gateway? 0 : base_number, is_wall));
		}
		if (count_cybercore < 1)
		{
			aeolusbot.RegisterBehavior(std::make_unique<BuildStructure>(::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, m_rebuild_cybercore? 0 : base_number, is_wall));
		}
		if (count_available_cybercore < 1)
		{
			army_comp = { {::sc2::UNIT_TYPEID::PROTOSS_ZEALOT, 1.0} };
		}

		aeolusbot.RegisterBehavior(std::make_unique<SpawnController>(army_comp));
		aeolusbot.RegisterBehavior(std::make_unique<AutoSupply>());
		aeolusbot.RegisterBehavior(std::make_unique<BuildWorkers>(18));
		aeolusbot.RegisterBehavior(std::make_unique<ChronoController>());
		aeolusbot.RegisterBehavior(std::make_unique<RepowerStructures>());

		if (count_available_cybercore >= 1 && !has_available_gateway && mediator.GetMinerals(aeolusbot) >= 100 && count_batteries < 2)
		{
			if (!m_battery_destroyed)
			{
				aeolusbot.RegisterBehavior(
					std::make_unique<BuildStructure>(
						::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY,
						base_number,
						mediator.GetDefenseTarget(aeolusbot, base_number)
					)
				);
			}
		}

		::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);
		if (forces.size() > 7)
		{
			aeolusbot.ChangeState(MakeState<ConsolidateState>());
		}
	}

	void TwelvePool::OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		if (unit->alliance == ::sc2::Unit::Alliance::Self)
		{
			if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY) m_rebuild_gateway = true;
			if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) m_rebuild_cybercore = true;
			if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY) m_battery_destroyed = true;
		}
	}
}