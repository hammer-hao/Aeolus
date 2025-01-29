#include "build_order_executor.h"
#include "behaviors/macro_behaviors/build_structure.h"
#include "Aeolus.h"

namespace Aeolus
{
	BuildOrderExecutor& BuildOrderExecutor::GetInstance()
	{
		static BuildOrderExecutor buildorderexecutor;
		return buildorderexecutor;
	}

	void BuildOrderExecutor::execute(AeolusBot& aeolusbot, BuildOrderEnum build_order)
	{
		auto* observations = aeolusbot.Observation();
		int supply_count = observations->GetFoodUsed();

		if (build_order == BuildOrderEnum::MACRO_STALKERS)
		{
			if (supply_count >= 14 && build_order_step == 0)
			{
				build_order_step++;
				std::make_unique<BuildStructure>(::sc2::UNIT_TYPEID::PROTOSS_PYLON, 0, true).get()->execute(aeolusbot);
			}
			if (supply_count >= 16 && build_order_step == 1)
			{
				build_order_step++;
				std::make_unique<BuildStructure>(::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, 0, true).get()->execute(aeolusbot);
			}
			if (supply_count >= 17 && build_order_step == 2)
			{
				build_order_step++;
				std::make_unique<BuildStructure>(::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, 0, true).get()->execute(aeolusbot);
			}
			if (supply_count >= 20 && build_order_step == 3)
			{
				build_order_step++;
				std::make_unique<BuildStructure>(::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, 0, false).get()->execute(aeolusbot);
			}
		}
	}
}