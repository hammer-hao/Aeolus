#include "proxy_gateways.h"

#include "../contingency.h"
#include "../consolidate.h"
#include "../base_state.h"
#include "../bot_state.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_common.h"

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"

#include "../../behaviors/macro_behaviors/mining.h"
#include "../../behaviors/macro_behaviors/scout.h"
#include "../../behaviors/macro_behaviors/tech_up.h"
#include "../../behaviors/macro_behaviors/spawn_controller.h"
#include "../../behaviors/macro_behaviors/production_controller.h"
#include "../../behaviors/macro_behaviors/auto_supply.h"
#include "../../behaviors/macro_behaviors/chrono_controller.h"
#include "../../behaviors/macro_behaviors/build_workers.h"
#include "../../behaviors/macro_behaviors/repower_structures.h"
#include "../../behaviors/macro_behaviors/build_structure.h"

namespace Aeolus
{
	std::string_view ProxyGateways::getName() const
	{
		return "PROXY_GATEWAYS";
	}

	void ProxyGateways::micro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);

		int baseToDefend = 0; // defend the main base
		::sc2::Point2D target = mediator.GetDefenseTarget(aeolusbot, baseToDefend);
		doGeneralMicro(aeolusbot, forces, target);
	}

	void ProxyGateways::macro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		auto allStructures = mediator.GetAllOwnStructures(aeolusbot);
		std::map<::sc2::UNIT_TYPEID, float> army_comp = { {::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0} };
		bool needCybercore = mediator.GetNumberPending(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) == 0;
		bool coreReady = false;
		bool gateAvailable = false;
		int numBatteries = mediator.GetNumberPending(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY);

		for (const auto& structure : allStructures)
		{
			if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE)
			{
				needCybercore = false;
				if (structure->build_progress >= 1.0)
				{
					coreReady = true;
				}
			}
			if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY &&
				structure->build_progress >= 1.0f && structure->orders.empty())
			{
				gateAvailable = true;
			}
			if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY) numBatteries++;
		}

		bool can_build_battery = false;
		if (!coreReady) 
		{
			army_comp = { {::sc2::UNIT_TYPEID::PROTOSS_ZEALOT, 1.0} };
		}
		else
		{
			if (gateAvailable && mediator.GetMinerals(aeolusbot) >= 100)
			{
				can_build_battery = true;
			}
		}

		aeolusbot.RegisterBehavior(std::make_unique<Mining>());
		aeolusbot.RegisterBehavior(std::make_unique<Scout>());
		aeolusbot.RegisterBehavior(std::make_unique<SpawnController>(army_comp));

		if (needCybercore) 
		{
			aeolusbot.RegisterBehavior(std::make_unique<TechUp>(::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE));
		}
		else
		{
			aeolusbot.RegisterBehavior(std::make_unique<ProductionController>(army_comp));
		}

		// auto supply since we are no longer relying on a build order
		aeolusbot.RegisterBehavior(std::make_unique<AutoSupply>());

		aeolusbot.RegisterBehavior(std::make_unique<BuildWorkers>(22));
		aeolusbot.RegisterBehavior(std::make_unique<ChronoController>());
		aeolusbot.RegisterBehavior(std::make_unique<RepowerStructures>());

		if (can_build_battery && numBatteries < 1)
		{
			aeolusbot.RegisterBehavior(
				std::make_unique<BuildStructure>(
					::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY, 0, mediator.GetDefenseTarget(aeolusbot, 0)));
		}

		if (aeolusbot.Observation()->GetGameLoop() % 22 == 1)
		{
			const int numGateways = 3;
			auto ownAttacking = ManagerMediator::getInstance().GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);
			if (ownAttacking.size() >= numGateways)
			{
				aeolusbot.ChangeState(MakeState<ConsolidateState>());
			}
		}
	}
}