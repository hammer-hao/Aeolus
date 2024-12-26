#include "build_workers.h"
#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"

namespace Aeolus
{
	void BuildWorkers::execute(AeolusBot& aeolusbot)
	{
		std::cout << "BuildWorkers: current minerals: " << aeolusbot.Observation()->GetMinerals() << '\n'
			<< "unused supply: " << aeolusbot.Observation()->GetFoodCap() - aeolusbot.Observation()->GetFoodUsed() << '\n'
			<< std::endl;
		if (aeolusbot.Observation()->GetMinerals() >= 50 &&
			(aeolusbot.Observation()->GetFoodCap() - aeolusbot.Observation()->GetFoodUsed()) >= 1)
		{
			::sc2::Units own_town_halls = ManagerMediator::getInstance().GetOwnTownHalls(aeolusbot);
			for (const auto& town_hall : own_town_halls)
			{
				if (town_hall->orders.empty())
				{
					aeolusbot.Actions()->UnitCommand(town_hall, ::sc2::ABILITY_ID::TRAIN_PROBE);
				}
			}
		}
	}
}