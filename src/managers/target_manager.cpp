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

		ManagerMediator& mediator = ManagerMediator::getInstance();

		std::vector<::sc2::Point2D> expansionLocations = mediator.GetExpansionLocations(m_bot);

		for (int i = 0; i < 5 && expansionLocations.size() > i; ++i)
		{
			bool found = false;

			std::vector<::sc2::Point2D> astarPath = AStarPathFind(expansionLocations[i], expansionLocations.back(),
				mediator.GetAstarGrid(m_bot).GetGrid());

			for (const auto& pathpt : astarPath)
			{
				if (sc2::DistanceSquared2D(expansionLocations[i], pathpt) >= 25)
				{
					found = true;
					m_defenseTarget.push_back(pathpt);
					break;
				}
			}

			if (!found)
			{
				std::cout << "something went wrong when calculating defensive position for base "
					<< i << std::endl;
				m_defenseTarget.push_back({ 0.0f, 0.0f });
			}
		}
	}

	void TargetManager::update(int iteration)
	{
		ManagerMediator& mediator = ManagerMediator::getInstance();

		auto enemy_structures = mediator.GetAllEnemyStructures(m_bot);
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
			auto targets = mediator.GetExpansionLocations(m_bot);
			if (m_bot.Observation()->GetVisibility(targets[m_currentBaseTarget]) == ::sc2::Visibility::Visible)
			{
				if (m_currentBaseTarget == 0) m_currentBaseTarget = (targets.size() - 1);
				else if (m_currentBaseTarget == targets.size() - 1) m_currentBaseTarget = 1;
				else m_currentBaseTarget = (m_currentBaseTarget + 1) % targets.size();
			}
			m_attackTarget = targets[m_currentBaseTarget];
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
}