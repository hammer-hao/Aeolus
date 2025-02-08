#pragma once

#include <sc2api/sc2_unit.h>
#include <cstddef>
#include <queue>
#include <string>
#include "build_order_enum.h"

namespace Aeolus
{
	class AeolusBot;
	/**
	* @brief The build order executor in charge of converting build orders into
	* macro behaviors and register them for execution.
	*/

	struct BuildOrderStep {
		int supply_threshold;
		::sc2::UNIT_TYPEID unit_type;
		bool is_wall;

		BuildOrderStep(int supp, ::sc2::UNIT_TYPEID unit, bool wall) :
			supply_threshold(supp), unit_type(unit), is_wall(wall) {}
	};

	class BuildOrderExecutor
	{
	public:
		void execute(AeolusBot& aeolusbot);

		bool isDone() const { return m_build_order_steps.empty(); }

		std::string_view getCurrentStep();

		//constructor
		BuildOrderExecutor(BuildOrderEnum build_order) : m_build_order_steps()
		{
			if (build_order == BuildOrderEnum::MACRO_STALKERS)
			{
				_addStep(13, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, true);
				_addStep(15, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true);
				_addStep(15, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true);
				_addStep(16, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false);
				_addStep(19, ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, false);
			}

			if (build_order == BuildOrderEnum::STALKER_IMMORTAL)
			{
				_addStep(13, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, true);
				_addStep(15, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true);
				_addStep(16, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false);
				_addStep(19, ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, true);
				_addStep(22, ::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, false);
			}
		}
		//destructor
		~BuildOrderExecutor() = default;

	private:

		std::queue<BuildOrderStep> m_build_order_steps;

		void _addStep(int supply, ::sc2::UNIT_TYPEID to_build, bool is_wall)
		{
			m_build_order_steps.push(BuildOrderStep(supply, to_build, is_wall));
		}
	};
}