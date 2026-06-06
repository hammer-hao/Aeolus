#include "build_detection.h"
#include "tech_up.h"

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include <algorithm>

namespace Aeolus
{
	bool BuildDetection::execute(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();

		const auto allUnits = mediator.GetAllOwnUnits(aeolusbot);
		bool hasObserver = std::any_of(
			allUnits.begin(),
			allUnits.end(),
			[](const ::sc2::Unit* unit) {
				return unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_OBSERVER;
			}
		);
		if (hasObserver) return false;

		const auto allEnemyUnits = mediator.GetAllEnemyUnits(aeolusbot);
		bool hasInvisible = std::any_of(
			allEnemyUnits.begin(),
			allEnemyUnits.end(),
			[](const ::sc2::Unit* unit) {
				bool match = unit->is_burrowed || unit->cloak == ::sc2::Unit::CloakState::Cloaked;
				if (match) std::cout << "cloaked unit: " << ::sc2::UnitTypeToName(unit->unit_type) << std::endl;
				return match;
			}
		);

		if (!hasInvisible && !m_force) return false; // no need for detection if no cloaked units

		std::make_unique<TechUp>(::sc2::UNIT_TYPEID::PROTOSS_OBSERVER)->execute(aeolusbot);
		if (!mediator.IsStructureAvailable(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY))
		{
			return true;
		}
		else
		{
			// robo ready, no observer yet
			const auto allOwnStructures = mediator.GetAllOwnStructures(aeolusbot);
			::sc2::Units robos;
			std::copy_if(allOwnStructures.begin(), allOwnStructures.end(), std::back_inserter(robos),
				[](const ::sc2::Unit* structure) {
					return structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY;
				});

			auto observerCreationAbility = mediator.GetCreationAbility(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_OBSERVER);

			for (const auto* robo : robos) 
			{
				for (const auto& order : robo->orders)
				{
					if (order.ability_id == observerCreationAbility) return false;
				}
			}
			// no robo currently training observers, just queue it up
			for (const auto* robo : robos)
			{
				if (robo->orders.empty())
				{
					aeolusbot.Actions()->UnitCommand(robo, observerCreationAbility);
					return true;
				}
			}
			// all robos occupied, just queue it up
			aeolusbot.Actions()->UnitCommand(robos.front(), observerCreationAbility);
			return true;
		}
	}
}