#include "unit_property_manager.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_data.h>
#include "../Aeolus.h"

namespace Aeolus
{
	UnitPropertyManager::UnitPropertyManager(AeolusBot& aeolusbot) :
		m_bot(aeolusbot)
	{
	}

	void UnitPropertyManager::update(int iteration)
	{
		if (iteration == 0) m_unit_data_cache = m_bot.Observation()->GetUnitTypeData();
		// std::cout << "Unit Type Data Size: " << m_unit_data_cache.size() << std::endl;

		if (iteration == 0)
		{
			::sc2::UnitTypeData immortal = m_unit_data_cache[static_cast<int>(::sc2::UNIT_TYPEID::PROTOSS_IMMORTAL)];
			std::cout << "Immortal mineral cost: " << immortal.mineral_cost << std::endl;
			std::cout << "Immortal gas cost: " << immortal.vespene_cost << std::endl;

			::sc2::UnitTypeData stalker = m_unit_data_cache[static_cast<int>(::sc2::UNIT_TYPEID::PROTOSS_STALKER)];
			std::cout << "Stalker mineral cost: " << stalker.mineral_cost << std::endl;
			std::cout << "Stalker gas cost: " << stalker.vespene_cost << std::endl;
		}
	}

	std::any UnitPropertyManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		return 0;
	}

	bool UnitPropertyManager::CanAttackGround(::sc2::Unit* unit)
	{
		uint64_t unit_id = unit->unit_type;

		auto it = m_can_attack_ground_cache.find(unit_id);
		if (it != m_can_attack_ground_cache.end()) {
			return it->second;
		}
		
		if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_BATTLECRUISER ||
			unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ORACLE)
		{
			// battlecrisers and oracles can attack ground
			m_can_attack_ground_cache[unit_id] = true;
			return true;
		}

		for (const auto& weapon : m_unit_data_cache[unit_id].weapons)
		{
			if (weapon.type == ::sc2::Weapon::TargetType::Ground ||
				weapon.type == ::sc2::Weapon::TargetType::Any)
			{
				// if the unit has a weapon that targets ground, set can attack ground to true.
				m_can_attack_ground_cache[unit_id] = true;
				return true;
			}
		}
		return false;
	}

	bool UnitPropertyManager::CanAttackAir(::sc2::Unit* unit)
	{
		uint64_t unit_id = unit->unit_type;

		auto it = m_can_attack_air_cache.find(unit_id);
		if (it != m_can_attack_air_cache.end())
		{
			return m_can_attack_air_cache[unit_id];
		}

		if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_BATTLECRUISER)
		{
			// Battlecruisers can attack air
			m_can_attack_air_cache[unit_id] = true;
			return true;
		}

		for (const auto& weapon : m_unit_data_cache[unit_id].weapons)
		{
			if (weapon.type == ::sc2::Weapon::TargetType::Air ||
				weapon.type == ::sc2::Weapon::TargetType::Any)
			{
				// if the unit has a weapon that targets air, return true!
				m_can_attack_air_cache[unit_id] = true;
				return true;
			}
		}

		return false;
	}

	double UnitPropertyManager::GroundRange(::sc2::Unit* unit)
	{
		uint64_t unit_id = unit->unit_type;

		if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ORACLE) return 4;
		if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_BATTLECRUISER) return 6;
		if (CanAttackGround(unit))
		{
			// if the unit can attack ground, find the range of that weapon
			for (const auto& weapon : m_unit_data_cache[unit_id].weapons)
			{
				if (weapon.type == ::sc2::Weapon::TargetType::Ground ||
					weapon.type == ::sc2::Weapon::TargetType::Any)
				{
					m_ground_range_cache[unit_id] = weapon.range;
					return weapon.range;
				}
			}
		}
		// if the unit cannot attack ground, return 0
		return 0;
	}

	double UnitPropertyManager::AirRange(::sc2::Unit* unit)
	{
		uint64_t unit_id = unit->unit_type;

		if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_BATTLECRUISER) return 6;

		if (CanAttackAir(unit))
		{
			for (const auto& weapon : m_unit_data_cache[unit_id].weapons)
			{
				// find the weapon that attacks air and return its range
				if (weapon.type == ::sc2::Weapon::TargetType::Air ||
					weapon.type == ::sc2::Weapon::TargetType::Any)
				{
					m_air_range_cache[unit_id] = weapon.range;
					return weapon.range;
				}
			}
		}
		// if the unit cannot attack air, return 0
		return 0;
	}
}