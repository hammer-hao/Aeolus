#include "door_manager.h"
#include <sc2api/sc2_unit.h>
#include "manager_mediator.h"
#include "../utils/unit_utils.h"
#include "../Aeolus.h"
#include <tuple>
#include <any>

namespace Aeolus
{
	std::any DoorManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case (constants::ManagerRequestType::SET_DOOR_POSITION):
		{
			auto params = std::any_cast<std::tuple<std::pair<float, float>>>(args);
			std::pair<float, float> doorPos = std::get<0>(params);
			setDoorPos(doorPos);
			return 0;
		}
		case (constants::ManagerRequestType::ENABLE_DOOR_MANAGER):
		{
			enable();
			return 0;
		}
		default: return 0;
		}
	}

	void DoorManager::update(int iteration)
	{
		// not enabled, return
		if (!m_enabled) return;

		// door position is not set, return
		if (m_doorPos == std::pair<float, float>(0.0f, 0.0f)) return;

		/*
		std::cout << "door pos: " << m_doorPos.first << " " << m_doorPos.second << std::endl;

		auto* debug = m_bot.Debug();
		::sc2::HeightMap heightMap = ::sc2::HeightMap(m_bot.Observation()->GetGameInfo());
		float height = heightMap.TerrainHeight({
			static_cast<int>(m_doorPos.first),
			static_cast<int>(m_doorPos.second) });

		debug->DebugSphereOut({ m_doorPos.first + 0.5f, m_doorPos.second + 0.5f, height }, 0.5f, ::sc2::Colors::Red);
		debug->SendDebug();
		*/

		if (m_doorUnit == nullptr)
		{
			catchNewDoorUnit();
			return;
		}

		::sc2::Units knockingUnits = ManagerMediator::getInstance().GetOwnUnitsInRange(
			m_bot,
			{ {m_doorPos.first + 0.5f, m_doorPos.second + 0.5f} },
			1.5f
		);

		if (knockingUnits.size() > 1) openDoor();
		else closeDoor();
	}

	void DoorManager::enable()
	{
		m_enabled = true;
	}

	void DoorManager::closeDoor()
	{
		// this door is already moving, don't issue new order
		if (!m_doorUnit->orders.empty())
		{
			return;
		}

		if (::sc2::DistanceSquared2D(m_doorUnit->pos, { m_doorPos.first + 0.5f, m_doorPos.second + 0.5f }) < 0.05f)
		{
			m_bot.Actions()->UnitCommand(m_doorUnit, ::sc2::ABILITY_ID::GENERAL_HOLDPOSITION);
		}
		else
		{
			m_bot.Actions()->UnitCommand(m_doorUnit, ::sc2::ABILITY_ID::GENERAL_MOVE, { m_doorPos.first + 0.5f, m_doorPos.second + 0.5f });
		}
	}

	void DoorManager::openDoor()
	{
		if (m_doorUnit->orders.empty()) return;

		m_bot.Actions()->UnitCommand(m_doorUnit, ::sc2::ABILITY_ID::STOP);
	}
	
	void DoorManager::setDoorPos(std::pair<float, float> doorPos)
	{
		m_doorPos = doorPos;
	}

	void DoorManager::catchNewDoorUnit()
	{
		ManagerMediator& mediator = ManagerMediator::getInstance();
		::sc2::Units allUnits = mediator.GetUnitsFromRole(m_bot, constants::UnitRole::ATTACKING);

		if (allUnits.empty()) return;

		::sc2::Units groundUnits;

		std::copy_if(allUnits.begin(), allUnits.end(), std::back_inserter(groundUnits),
			[](const ::sc2::Unit* unit)
			{
				return (!unit->is_flying);
			}
		);

		if (groundUnits.empty()) return;

		::sc2::Units sortedGroundUnits = utils::SortByDistanceTo(groundUnits, { m_doorPos.first, m_doorPos.second });

		// best candidate: closest combat ground unit to the door position
		m_doorUnit = sortedGroundUnits.front();

		mediator.AssignRole(m_bot, m_doorUnit, constants::UnitRole::DOOR);

		closeDoor();
	}

	void DoorManager::onUnitDestroyed(const ::sc2::Unit* unit)
	{
		if (unit->tag == m_doorUnit->tag)
		{
			m_doorUnit = nullptr;
		}
	}
}