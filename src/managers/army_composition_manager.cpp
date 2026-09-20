#include "army_composition_manager.h"
#include "manager.h"
#include "../enums.h"
#include "../managers/manager_mediator.h"
#include <sc2api/sc2_common.h>
#include <unordered_map>
#include <map>
#include <iostream>
#include <iomanip>

namespace Aeolus
{
	std::any ArmyCompositionManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case (constants::ManagerRequestType::GET_OPTIMAL_ARMY_COMPOSITION):
		{
			return m_best_army_composition;
		}
		default: return 0;
		}
	}

	void ArmyCompositionManager::update(int iteration) 
	{
		// update 
		if (iteration < 22 * 60 * 9)
		{
			m_best_army_composition =
				std::map<::sc2::UNIT_TYPEID, float>{ 
					{::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.00f} 
			};
			return;
		}
		if (iteration % 44 == 1)
		{
			auto& mediator = ManagerMediator::getInstance();
			auto all_enemy = mediator.GetAllSeenEnemyUnits(m_bot);

			CompositionWeights totalWeights = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
			for (const auto& enemy_unit : all_enemy)
			{
				auto it = ARMY_COMPOSITION_LOOKUP.find(enemy_unit);
				if (it == ARMY_COMPOSITION_LOOKUP.end())
				{
					continue;
				}
				CompositionWeights counterWeights = it->second;
				int supplyCost = mediator.GetUnitSupplyCost(m_bot, enemy_unit);
				totalWeights.stalker += supplyCost * counterWeights.stalker;
				totalWeights.archon += supplyCost * counterWeights.archon;
				totalWeights.immortal += supplyCost * counterWeights.immortal;
				totalWeights.colossus += supplyCost * counterWeights.colossus;
				totalWeights.tempest += supplyCost * counterWeights.tempest;
			}

			std::vector<std::pair<::sc2::UNIT_TYPEID, float>> weights = {
			{::sc2::UNIT_TYPEID::PROTOSS_STALKER, totalWeights.stalker},
			{::sc2::UNIT_TYPEID::PROTOSS_IMMORTAL, totalWeights.immortal},
			{::sc2::UNIT_TYPEID::PROTOSS_TEMPEST, totalWeights.tempest},
			{::sc2::UNIT_TYPEID::PROTOSS_COLOSSUS, totalWeights.colossus},
			{::sc2::UNIT_TYPEID::PROTOSS_ARCHON, totalWeights.archon}
			};

			std::sort(weights.begin(), weights.end(),
				[](const auto& a, const auto& b)
				{
					return a.second > b.second;
				});

			weights.resize(3);

			// Normalize top 3
			float total = 0.0f;
			for (const auto& [unit, weight] : weights)
			{
				total += weight;
			}

			if (total > 0.0f)
			{
				for (auto& [unit, weight] : weights)
				{
					weight /= total;
				}
			}

			std::cout << "\n========== ARMY COMPOSITION ==========\n";

			for (const auto& [unit, weight] : weights)
			{
				std::cout << std::left << std::setw(15)
					<< ::sc2::UnitTypeToName(unit)
					<< std::right << std::fixed << std::setprecision(1)
					<< weight * 100.0f << "%\n";
			}

			std::cout << "======================================\n";

			m_best_army_composition = std::map<::sc2::UNIT_TYPEID, float>(weights.begin(), weights.end());
		}
	}
}