#include "combat_sim_manager.h"
#include "../thirdparty/libvoxelbot/combat/simulator.h"

#include "../Aeolus.h"
#include "../enums.h"
#include <unordered_map>
#include <numbers>
#include <limits>
#include <iostream>
#include <iomanip>
#include <tuple>
#include <any>
#include <chrono>

namespace Aeolus
{
	std::any CombatSimManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case (constants::ManagerRequestType::PREDICT_ENGAGEMENT):
		{
			auto params = std::any_cast<std::tuple<::sc2::Units, ::sc2::Units, ::sc2::Units>>(args);
			::sc2::Units own_army = std::get<0>(params);
			::sc2::Units oppoenent_army = std::get<1>(params);
			::sc2::Units opponent_static_defenses = std::get<2>(params);
			return _predictEngagement(own_army, oppoenent_army, opponent_static_defenses);
		}
		default:
			return 0;
		}
	}

	CombatSimManager::CombatSimManager(AeolusBot& aeolusbot) : m_bot(aeolusbot) {
	}

	void CombatSimManager::Initialize()
	{
		initMappings();
		m_simulator = std::make_unique<CombatPredictor>();
	}

	void CombatSimManager::update(int iteration)
	{
	}

	CombatSimulationResult CombatSimManager::_predictEngagement(::sc2::Units own_army, ::sc2::Units opponent_army, ::sc2::Units opponent_static_defenses)
	{
		std::cout << "[Combad Sim] Predicting the engagement... " << std::endl;

		CombatSimulationResult combatSimulationResult;

		std::unordered_map<::sc2::UNIT_TYPEID, int> own_counts;
		std::unordered_map<::sc2::UNIT_TYPEID, int> opponent_counts;

		std::vector<CombatUnit> combatUnits;

		for (const auto& unit : own_army)
		{
			own_counts[unit->unit_type]++;
			CombatUnit ownUnit(*(unit));
			ownUnit.owner = 1; // 1 stands for self in this context
			combatUnits.push_back(ownUnit);
		}

		for (const auto& unit : opponent_army)
		{
			if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_BUNKER)
			{
				for (int i = 0; i < 4; ++i)
				{
					opponent_counts[unit->unit_type]++;
					CombatUnit enemyUnit(2, ::sc2::UNIT_TYPEID::TERRAN_MARINE, 200, false);
					enemyUnit.owner = 2; // 2 stands for enemy in this context
					combatUnits.push_back(enemyUnit);
				}
			}
			else if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_SIEGETANK)
			{
				opponent_counts[unit->unit_type]++;
				CombatUnit enemyUnit(2, ::sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED, 175, false);
				enemyUnit.owner = 2; // 2 stands for enemy in this context
				combatUnits.push_back(enemyUnit);
			}
			else
			{
				opponent_counts[unit->unit_type]++;
				CombatUnit enemyUnit(*(unit));
				enemyUnit.owner = 2; // 2 stands for enemy in this context
				combatUnits.push_back(enemyUnit);
			}
		}

		CombatState state = { combatUnits };

		// Opponent is usually the defender (defenders do the first hit)
		const int defenderPlayer = 2;

		// Calculate our army score to compare after the fight
		float armySupplyScore = 0.f;
		for (const auto unit : own_army)
		{
			const sc2::UnitTypeData& unitTypeData = m_bot.Observation()->GetUnitTypeData()[unit->unit_type];
			armySupplyScore += unitTypeData.food_required * (0.25f + 0.75f * unit->health / std::max(1.f, unit->health_max));
		}
		float enemyArmySupplyScore = 0.f;
		for (const auto unit : opponent_army)
		{
			const sc2::UnitTypeData& unitTypeData = m_bot.Observation()->GetUnitTypeData()[unit->unit_type];
			enemyArmySupplyScore += unitTypeData.food_required * (0.25f + 0.75f * unit->health / std::max(1.f, unit->health_max));
		}
		for (const auto defense : opponent_static_defenses)
		{
			if (defense->unit_type == ::sc2::UNIT_TYPEID::TERRAN_BUNKER)
			{
				const sc2::UnitTypeData& unitTypeData = m_bot.Observation()->GetUnitTypeData()[defense->unit_type];
				enemyArmySupplyScore += 4 * (defense->health / std::max(1.f, defense->health_max));
			}
		}

		CombatUpgrades player1upgrades = {};
		CombatUpgrades player2upgrades = {};

		state.environment = &m_simulator->getCombatEnvironment(player1upgrades, player2upgrades);

		auto start = std::chrono::high_resolution_clock::now();

		CombatSettings settings;
		// Simulate for at most 100 *game* seconds
		settings.maxTime = 100;
		settings.enableTimingAdjustment = false;
		const CombatResult outcome = m_simulator->predict_engage(state, settings, nullptr, defenderPlayer);

		auto end = std::chrono::high_resolution_clock::now();

		const CombatState& finalState = outcome.state;

		std::unordered_map<int, int> unitCounts;
		std::unordered_map<int, float> totalHealth;

		for (const CombatUnit& unit : finalState.units) {
			unitCounts[unit.owner]++;
			totalHealth[unit.owner] += unit.health + unit.shield;
		}

		float resultArmySupplyScore = 0.f;
		float resultEnemyArmySupplyScore = 0.f;
		for (const auto& unit : outcome.state.units)
		{
			if (unit.health > 0)
			{
				const sc2::UnitTypeData& unitTypeData = m_bot.Observation()->GetUnitTypeData()[sc2::UnitTypeID(unit.type)];
				const float score = unitTypeData.food_required * (0.25f + 0.75f * unit.health / unit.health_max);
				if (unit.owner == 1)
					resultArmySupplyScore += score;
				else
					resultEnemyArmySupplyScore += score;
			}
		}
		const float armyRating = resultArmySupplyScore / std::max(1.f, armySupplyScore);
		const float enemyArmyRating = resultEnemyArmySupplyScore / std::max(1.f, enemyArmySupplyScore);

		combatSimulationResult.supplyLost = armySupplyScore - resultArmySupplyScore;
		combatSimulationResult.supplyPercentageRemaining = armyRating;
		combatSimulationResult.enemySupplyLost = enemyArmySupplyScore - resultEnemyArmySupplyScore;
		combatSimulationResult.enemySupplyPercentageRemaining = enemyArmyRating;

		std::cout << "\n=== Combat Simulation Summary ===\n";

		// Winner
		std::cout << "Winner: ";
		if (outcome.state.owner_with_best_outcome() == 0)
			std::cout << "Tie\n";
		else
			std::cout << (outcome.state.owner_with_best_outcome() == 1 ? "Our Army (Owner 1)" : "Enemy Army (Owner 2)") << "\n";

		// Before the fight: unit compositions
		std::cout << "\n--- Before the Fight ---\n";
		std::cout << "Our Army Composition:\n";
		for (const auto& [type, count] : own_counts) {
			std::cout << "  - " << sc2::UnitTypeToName(type) << ": " << count << "\n";
		}

		std::cout << "Enemy Army Composition:\n";
		for (const auto& [type, count] : opponent_counts) {
			std::cout << "  - " << sc2::UnitTypeToName(type) << ": " << count << "\n";
		}

		// After the fight
		std::cout << "\n--- After the Fight ---\n";
		std::cout << " Supply Lost: \n";
		std::cout << "  Our Army: " << combatSimulationResult.supplyLost << " supply\n";
		std::cout << "  Enemy Army: " << combatSimulationResult.enemySupplyLost << " supply\n";

		std::cout << "\nRemaining Supply Percentage relative to before the fight:\n";
		std::cout << "  Our Army: " << armyRating << "\n";
		std::cout << "  Enemy Army: " << enemyArmyRating << "\n";

		std::cout << "\nSimulation Time: " << std::fixed << std::setprecision(2) << std::chrono::duration<double, std::micro>(end - start).count() << " us\n";
		std::cout << "=================================\n" << std::endl;

		return combatSimulationResult;
	}
}