#include "unit_property_manager.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_data.h>
#include <sc2api/sc2_api.h>
#include "../Aeolus.h"

namespace Aeolus
{
	UnitPropertyManager::UnitPropertyManager(AeolusBot& aeolusbot) :
		m_bot(aeolusbot)
	{
		std::cout << "Neutral Property manager initialization" << std::endl;
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
		switch (request)
		{
		case constants::ManagerRequestType::CAN_ATTACK_GROUND:
		{
			auto params = std::any_cast<std::tuple<const ::sc2::Unit*>>(args);
			const ::sc2::Unit* unit = std::get<0>(params);
			return CanAttackGround(unit);
		}
		case constants::ManagerRequestType::CAN_ATTACK_AIR:
		{
			auto params = std::any_cast<std::tuple<const ::sc2::Unit*>>(args);
			const ::sc2::Unit* unit = std::get<0>(params);
			return CanAttackAir(unit);
		}
		case constants::ManagerRequestType::GROUND_RANGE:
		{
			auto params = std::any_cast<std::tuple<const ::sc2::Unit*>>(args);
			const ::sc2::Unit* unit = std::get<0>(params);
			return GroundRange(unit);
		}
		case constants::ManagerRequestType::AIR_RANGE:
		{
			auto params = std::any_cast<std::tuple<const ::sc2::Unit*>>(args);
			const ::sc2::Unit* unit = std::get<0>(params);
			return AirRange(unit);
		}
		case constants::ManagerRequestType::GROUND_DPS:
		{
			auto params = std::any_cast<std::tuple<const ::sc2::Unit*>>(args);
			const ::sc2::Unit* unit = std::get<0>(params);
			return GroundDPS(unit);
		}
		case constants::ManagerRequestType::AIR_DPS:
		{
			auto params = std::any_cast<std::tuple<const ::sc2::Unit*>>(args);
			const ::sc2::Unit* unit = std::get<0>(params);
			return AirDPS(unit);
		}
		case constants::ManagerRequestType::UNITS_IN_ATTACK_RANGE:
		{
			auto params = std::any_cast<std::tuple<const ::sc2::Unit*, ::sc2::Units>>(args);
			const ::sc2::Unit* unit = std::get<0>(params);
			::sc2::Units targets = std::get<1>(params);
			return InAttackRange(unit, targets);
		}
		case constants::ManagerRequestType::GET_CREATION_ABILITY:
		{
			auto params = std::any_cast<std::tuple<::sc2::UNIT_TYPEID>>(args);
			::sc2::UNIT_TYPEID unit_id = std::get<0>(params);
			return CreationAbility(unit_id);
		}
		case constants::ManagerRequestType::GET_UNIT_COST:
		{
			auto params = std::any_cast<std::tuple<::sc2::UNIT_TYPEID>>(args);
			::sc2::UNIT_TYPEID unit_type = std::get<0>(params);
			return GetCost(unit_type);
		}
		case constants::ManagerRequestType::GET_REQUIRED_TECH:
		{
			auto params = std::any_cast<std::tuple<::sc2::UNIT_TYPEID>>(args);
			::sc2::UNIT_TYPEID unit_type = std::get<0>(params);
			return GetTechRequirement(unit_type);
		}
		case constants::ManagerRequestType::GET_UNIT_SUPPLY_COST:
		{
			auto params = std::any_cast<std::tuple<::sc2::UNIT_TYPEID>>(args);
			::sc2::UNIT_TYPEID unit_type = std::get<0>(params);
			return GetUnitSupplyCost(unit_type);
		}
		case constants::ManagerRequestType::GET_UNIT_MOVEMENT_SPEED:
		{
			auto params = std::any_cast<std::tuple<::sc2::UNIT_TYPEID>>(args);
			::sc2::UNIT_TYPEID unit_type = std::get<0>(params);
			return GetMovementSpeed(unit_type);
		}
		default:
			std::cout << "UnitPropertyMnager: Unknown request type!" << std::endl;
		}
		return 0;
	}

	bool UnitPropertyManager::CanAttackGround(const ::sc2::Unit* unit)
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

	bool UnitPropertyManager::CanAttackAir(const ::sc2::Unit* unit)
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

	double UnitPropertyManager::GroundRange(const ::sc2::Unit* unit)
	{
		uint64_t unit_id = unit->unit_type;

		auto it = m_ground_range_cache.find(unit_id);
		if (it != m_ground_range_cache.end()) return m_ground_range_cache[unit_id];

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

	double UnitPropertyManager::AirRange(const ::sc2::Unit* unit)
	{
		uint64_t unit_id = unit->unit_type;

		auto it = m_air_range_cache.find(unit_id);
		if (it != m_air_range_cache.end()) return m_air_range_cache[unit_id];

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

	double UnitPropertyManager::GroundDPS(const ::sc2::Unit* unit)
	{
		uint64_t unit_id = unit->unit_type;

		auto it = m_ground_dps_cache.find(unit_id);
		if (it != m_ground_dps_cache.end()) return m_ground_dps_cache[unit_id];

		if (CanAttackGround(unit))
		{
			for (const auto& weapon:m_unit_data_cache[unit_id].weapons)
				if (weapon.type == ::sc2::Weapon::TargetType::Ground ||
					weapon.type == ::sc2::Weapon::TargetType::Any)
				{
					double ground_dps = weapon.damage_ * weapon.attacks / weapon.speed;
					m_ground_dps_cache[unit_id] = ground_dps;
					return ground_dps;
				}
		}
		// if cannot attack ground, return 0
		return 0;
	}

	double UnitPropertyManager::AirDPS(const ::sc2::Unit* unit)
	{
		uint64_t unit_id = unit->unit_type;

		auto it = m_air_dps_cache.find(unit_id);
		if (it != m_air_dps_cache.end()) return m_air_dps_cache[unit_id];

		if (CanAttackAir(unit))
		{
			for (const auto& weapon : m_unit_data_cache[unit_id].weapons)
				if (weapon.type == ::sc2::Weapon::TargetType::Air ||
					weapon.type == ::sc2::Weapon::TargetType::Any)
				{
					double air_dps = weapon.damage_ * weapon.attacks / weapon.speed;
					m_air_dps_cache[unit_id] = air_dps;
					return air_dps;
				}
		}
		// if cannot attack ground, return 0
		return 0;
	}

	::sc2::Units UnitPropertyManager::InAttackRange(const ::sc2::Unit* unit, ::sc2::Units targets)
	{
		::sc2::Units units_in_range;
		if (CanAttackGround(unit))
		{
			double ground_range = GroundRange(unit);
			for (const auto& target : targets)
			{
				float total_range = ground_range + static_cast<double>(unit->radius) + static_cast<double>(target->radius);
				if (::sc2::Distance2D(::sc2::Point2D(unit->pos), ::sc2::Point2D(target->pos))
					<= total_range)
				{
					units_in_range.push_back(target);
				}
			}
		}
		return units_in_range;
	}

	::sc2::ABILITY_ID UnitPropertyManager::CreationAbility(::sc2::UNIT_TYPEID unit_id)
	{
		uint64_t id = static_cast<uint64_t>(unit_id);
		auto it = m_creation_ability_cache.find(id);
		if (it != m_creation_ability_cache.end()) return m_creation_ability_cache[id];

		::sc2::ABILITY_ID result = m_unit_data_cache[id].ability_id;
		m_creation_ability_cache[id] = result;
		return result;
	}

	std::pair<int, int> UnitPropertyManager::GetCost(::sc2::UNIT_TYPEID unit_type)
	{
		uint64_t id = static_cast<uint64_t>(unit_type);
		auto it = m_cost_cache.find(id);
		if (it != m_cost_cache.end()) return m_cost_cache[id];

		std::pair<int, int> result = { m_unit_data_cache[id].mineral_cost, m_unit_data_cache[id].vespene_cost };
		m_cost_cache[id] = result;
		return result;
	}

	::sc2::UNIT_TYPEID UnitPropertyManager::GetTechRequirement(::sc2::UNIT_TYPEID unit_type)
	{
		uint64_t id = static_cast<uint64_t>(unit_type);
		auto it = m_tech_requirement_cache.find(id);
		if (it != m_tech_requirement_cache.end()) return m_tech_requirement_cache[id];

		::sc2::UNIT_TYPEID result;
		if (unit_type == ::sc2::UNIT_TYPEID::PROTOSS_IMMORTAL) result = ::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY;
		else result = m_unit_data_cache[id].tech_requirement;
		m_tech_requirement_cache[id] = result;
		return result;
	}

	int UnitPropertyManager::GetUnitSupplyCost(::sc2::UNIT_TYPEID unit_type)
	{
		uint64_t id = static_cast<uint64_t>(unit_type);
		auto it = m_supply_cost_map.find(id);
		if (it != m_supply_cost_map.end()) return m_supply_cost_map[id];

		int result = static_cast<int>(m_unit_data_cache[id].food_required);
		m_supply_cost_map[id] = result;
		return result;
	}

	float UnitPropertyManager::GetMovementSpeed(::sc2::UNIT_TYPEID unit_type)
	{
		uint64_t id = static_cast<uint64_t>(unit_type);
		auto it = m_movement_speed_cache.find(id);
		if (it != m_movement_speed_cache.end()) return m_movement_speed_cache[id];

		float result = static_cast<float>(m_unit_data_cache[id].movement_speed);
		m_movement_speed_cache[id] = result;
		return result;
	}
}