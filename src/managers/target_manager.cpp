#include "target_manager.h"
#include "manager_mediator.h"
#include "../utils/Astar.hpp"
#include "../utils/unit_utils.h"
#include "../utils/position_utils.h"
#include "../Aeolus.h"
#include <any>
#include <tuple>

namespace Aeolus
{
	std::any TargetManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case (constants::ManagerRequestType::GET_ATTACK_TARGET):
		{
			return getAttackTarget();
		}
		case (constants::ManagerRequestType::GET_PRISM_TARGET):
		{
			return getPrismTarget();
		}
		case (constants::ManagerRequestType::GET_DEFENSE_TARGET):
		{
			auto params = std::any_cast<std::tuple<int>>(args);
			int baseIndex = std::get<0>(params);
			return getDefenseTarget(baseIndex);
		}
		default:
			return 0;
		}
	}

	void TargetManager::Initialize()
	{
		m_defenseTarget.clear();
		m_defenseTargetExtended.clear();

		ManagerMediator& mediator = ManagerMediator::getInstance();

		std::vector<::sc2::Point2D> expansionLocations = mediator.GetExpansionLocations(m_bot);

		for (int i = 0; i < 5 && expansionLocations.size() > i; ++i)
		{
			bool found = false;
			bool found_extended = false;

			AStarWorkspace workspace;

			const auto astarPath = AStarPathFind(expansionLocations[i], expansionLocations.back(), mediator.GetAstarGrid(m_bot).GetGrid(), workspace);

			for (const auto& pathpt : astarPath)
			{
				const float distSq =
					sc2::DistanceSquared2D(expansionLocations[i], pathpt);

				if (!found && distSq >= 25.0f)
				{
					m_defenseTarget.push_back(pathpt);
					found = true;
				}

				if (!found_extended && distSq >= 100.0f)
				{
					m_defenseTargetExtended.push_back(pathpt);
					found_extended = true;
				}

				if (found && found_extended)
					break;
			}

			if (!found)
			{
				std::cout
					<< "something went wrong when calculating defensive position for base "
					<< i << std::endl;

				m_defenseTarget.push_back(
					utils::GetPositionTowards(
						expansionLocations[i],
						expansionLocations.back(),
						6.0f
					)
				);
			}

			if (!found_extended)
			{
				std::cout
					<< "something went wrong when calculating extended defensive position for base "
					<< i << std::endl;

				m_defenseTargetExtended.push_back(
					utils::GetPositionTowards(
						expansionLocations[i],
						expansionLocations.back(),
						10.0f
					)
				);
			}
		}
	}

	void TargetManager::update(int iteration)
	{
		ManagerMediator& mediator = ManagerMediator::getInstance();

		auto enemy_structures = mediator.GetAllEnemyStructures(m_bot);

		// calculate attack target
		::sc2::Units filtered_structures;
		for (const auto& structure : enemy_structures)
		{
			if (structure->unit_type != ::sc2::UNIT_TYPEID::ZERG_CREEPTUMOR
				&& structure->unit_type != ::sc2::UNIT_TYPEID::ZERG_CREEPTUMORBURROWED
				&& structure->unit_type != ::sc2::UNIT_TYPEID::ZERG_CREEPTUMORQUEEN)
				filtered_structures.push_back(structure);
		}
		if (!filtered_structures.empty())
		{
			m_attackTarget = utils::GetClosestUnitTo(m_bot.Observation()->GetStartLocation(), filtered_structures)->pos;
		}
		else if (m_bot.Observation()->GetGameLoop() / 22.4f < 240.0f)
			m_attackTarget = mediator.GetExpansionLocations(m_bot).back();
		else
		{
			const auto targets = mediator.GetExpansionLocations(m_bot);
			const auto* observation = m_bot.Observation();

			if (!targets.empty())
			{
				// Current search location has been checked.
				if (observation->GetVisibility(targets[m_currentBaseTarget]) ==
					::sc2::Visibility::Visible)
				{
					const size_t start = m_currentBaseTarget;
					bool foundUnsearched = false;

					for (size_t offset = 1; offset < targets.size(); ++offset)
					{
						const size_t candidate =
							(start + offset) % targets.size();

						if (observation->GetVisibility(targets[candidate]) !=
							::sc2::Visibility::Visible)
						{
							m_currentBaseTarget = candidate;
							foundUnsearched = true;
							break;
						}
					}

					// IMPORTANT:
					// If every expansion is currently visible, DON'T cycle
					// through all of them one per frame.
					if (!foundUnsearched)
					{
						m_currentBaseTarget = start;
					}
				}

				m_attackTarget = targets[m_currentBaseTarget];
			}
		}

		// calculate prism target

		// auto start = std::chrono::high_resolution_clock::now();

		::sc2::Units attackingUnits = mediator.GetUnitsFromRole(m_bot, constants::UnitRole::ATTACKING);
		
		int bestCount = 0;
		const ::sc2::Unit* seed = nullptr;

		for (const auto* unit : attackingUnits)
		{
			auto neighbours = mediator.GetOwnAttackingUnitsInRange(m_bot, { unit->pos }, 5.0f);
			if (neighbours.size() > bestCount)
			{
				bestCount = neighbours.size();
				seed = unit;
			}
		}

		if (seed) m_prismTarget = seed->pos;

		if (iteration > 22)
		{
			auto ownTownHalls = mediator.GetOwnTownHalls(m_bot);
			if (ownTownHalls.size() >= 3)
			{
				std::vector<::sc2::Point2D> startingPoints;
				for (const auto& th : ownTownHalls)
				{
					startingPoints.push_back(th->pos);
				}
				auto threats = mediator.GetUnitsInRange(m_bot, startingPoints, 15.0f);

				if (threats.empty() && m_patrolling == nullptr)
				{
					auto candidate = mediator.SelectWorkerClosestTo(m_bot, m_defenseTarget[1]);
					if (candidate)
					{
						mediator.AssignRole(m_bot, candidate.value(), constants::UnitRole::PATROLLING);
						m_patrolling = candidate.value();
						m_bot.Actions()->UnitCommand(m_patrolling, ::sc2::ABILITY_ID::MOVE_MOVE, m_defenseTargetExtended[2], false);
						m_bot.Actions()->UnitCommand(m_patrolling, ::sc2::ABILITY_ID::MOVE_MOVEPATROL, m_defenseTargetExtended[3], true);
					}
				}
			}
		}
	}

	void TargetManager::OnUnitDestroyed(const ::sc2::Unit* unit)
	{
		if (unit->tag == m_patrolling->tag)
		{
			m_patrolling = nullptr;
		}
	}

	::sc2::Point2D TargetManager::getDefenseTarget(int baseLocation)
	{
		if (baseLocation < 0 || baseLocation > 4)
		{
			std::cout << "defensive target calculation not supported for base index " << baseLocation << std::endl;
			return { 0.0f, 0.0f };
		}

		std::vector<::sc2::Point2D> startingPoints;
		for (const auto& th : ManagerMediator::getInstance().GetOwnTownHalls(m_bot))
		{
			startingPoints.push_back(th->pos);
		}
		auto threats = ManagerMediator::getInstance().GetUnitsInRange(m_bot, startingPoints, 15.0f);
		::sc2::Units high_priority_threats;
		std::copy_if(threats.begin(), threats.end(), std::back_inserter(high_priority_threats), [](const ::sc2::Unit* threat) {
			return threat->unit_type != ::sc2::UNIT_TYPEID::ZERG_OVERLORD && threat->unit_type != ::sc2::UNIT_TYPEID::ZERG_OVERSEER
				&& threat->unit_type != ::sc2::UNIT_TYPEID::ZERG_OVERLORDCOCOON && threat->unit_type != ::sc2::UNIT_TYPEID::ZERG_OVERSEERSIEGEMODE;
			});

		if (!high_priority_threats.empty())
		{
			auto* target = utils::GetClosestUnitTo(m_defenseTarget[baseLocation], high_priority_threats);
			return target->pos;
		}

		if (!threats.empty())
		{
			auto* target = utils::GetClosestUnitTo(m_defenseTarget[baseLocation], threats);
			return target->pos;
		}

		return m_defenseTarget[baseLocation];
	}

	::sc2::Point2D TargetManager::getAttackTarget()
	{
		return m_attackTarget;
	}

	::sc2::Point2D TargetManager::getPrismTarget()
	{
		return m_prismTarget;
	}
}