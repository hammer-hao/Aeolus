#include "combat_sim_manager.h"
#include "../thirdparty/libvoxelbot/combat/simulator.h"

#include "../Aeolus.h"
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
			auto params = std::any_cast<std::tuple<std::vector<::sc2::UNIT_TYPEID>, std::vector<::sc2::UNIT_TYPEID>>>(args);
			std::vector<::sc2::UNIT_TYPEID> own_army = std::get<0>(params);
			std::vector<::sc2::UNIT_TYPEID> oppoenent_army = std::get<1>(params);
			return _predictEngagement(own_army, oppoenent_army);
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

	bool CombatSimManager::_predictEngagement(std::vector<::sc2::UNIT_TYPEID> own_army, std::vector<::sc2::UNIT_TYPEID> opponent_army)
	{
		std::cout << "[Combad Sim] Predicting the engagement... " << std::endl;

		std::unordered_map<::sc2::UNIT_TYPEID, int> own_counts;
		std::unordered_map<::sc2::UNIT_TYPEID, int> opponent_counts;

		std::vector<CombatUnit> combatUnits;

		for (const auto& unit_type : own_army)
		{
			own_counts[unit_type]++;
			combatUnits.push_back(makeUnit(1, unit_type));
		}

		for (const auto& unit_type : opponent_army)
		{
			opponent_counts[unit_type]++;
			combatUnits.push_back(makeUnit(2, unit_type));
		}

		CombatState state = { combatUnits };
		auto start = std::chrono::high_resolution_clock::now();
		CombatResult outcome = m_simulator->predict_engage(state);
		auto end = std::chrono::high_resolution_clock::now();

		const CombatState& finalState = outcome.state;

		std::unordered_map<int, int> unitCounts;
		std::unordered_map<int, float> totalHealth;

		for (const CombatUnit& unit : finalState.units) {
			unitCounts[unit.owner]++;
			totalHealth[unit.owner] += unit.health + unit.shield;
		}


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
		std::cout << "Remaining Units:\n";
		std::cout << "  Our Army: " << unitCounts[1] << " units\n";
		std::cout << "  Enemy Army: " << unitCounts[2] << " units\n";

		std::cout << "\nRemaining Total HP + Shields:\n";
		std::cout << "  Our Army: " << std::fixed << std::setprecision(1) << totalHealth[1] << "\n";
		std::cout << "  Enemy Army: " << totalHealth[2] << "\n";

		std::cout << "\nSimulation Time: " << std::fixed << std::setprecision(2) << std::chrono::duration<double, std::micro>(end - start).count() << " us\n";
		std::cout << "=================================\n" << std::endl;

		return outcome.state.owner_with_best_outcome() == 1;
	}
}