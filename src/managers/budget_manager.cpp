#pragma once

#include "budget_manager.h"
#include "../Aeolus.h"

namespace Aeolus
{
	std::any BudgetManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case constants::ManagerRequestType::GET_MINERALS:
			return _getMinerals();
		case constants::ManagerRequestType::GET_VESPENE:
			return _getVespene();
		case constants::ManagerRequestType::RESERVE_MINERALS:
		{
			auto params = std::any_cast<std::tuple<int>>(args);
			int amount = std::get<0>(params);
			_reserveMinerals(amount);
			return 0;
		}
		case constants::ManagerRequestType::RESERVE_VESPENE:
		{
			auto params = std::any_cast<std::tuple<int>>(args);
			int amount = std::get<0>(params);
			_reserveVespene(amount);
			return 0;
		}
		case constants::ManagerRequestType::FREE_MINERALS:
		{
			auto params = std::any_cast<std::tuple<int>>(args);
			int amount = std::get<0>(params);
			_freeMinerals(amount);
			return 0;
		}
		case constants::ManagerRequestType::FREE_VESPENE:
		{
			auto params = std::any_cast<std::tuple<int>>(args);
			int amount = std::get<0>(params);
			_freeVespene(amount);
			return 0;
		}
		}
	}

	void BudgetManager::update(int iteration) {}
	
	int BudgetManager::_getMinerals()
	{
		return m_bot.Observation()->GetMinerals() + m_mineralOffset;
	}

	int BudgetManager::_getVespene()
	{
		return m_bot.Observation()->GetVespene() + m_vespeneOffset;
	}

	void BudgetManager::_reserveMinerals(int amount)
	{
		m_mineralOffset -= amount;
	}

	void BudgetManager::_reserveVespene(int amount)
	{
		m_vespeneOffset -= amount;
	}

	void BudgetManager::_freeMinerals(int amount)
	{
		m_mineralOffset += amount;
	}

	void BudgetManager::_freeVespene(int amount)
	{
		m_vespeneOffset += amount;
	}
}