#pragma once

#include <sc2api/sc2_unit.h>
#include <unordered_map>
#include <sc2api/sc2_interfaces.h>
#include "manager.h"
#include "../constants.h"

namespace Aeolus
{
	class AeolusBot;

	class UnitPropertyManager : public Manager
	{
	public:
		UnitPropertyManager(AeolusBot& aeolusbot);

		std::string_view GetName() const override {
			static const std::string name = "UnitPropertyManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		void update(int iteration) override;

	private:

		bool CanAttackGround(const ::sc2::Unit* unit);

		std::unordered_map<uint64_t, bool> m_can_attack_ground_cache;

		bool CanAttackAir(const::sc2::Unit* unit);

		std::unordered_map<uint64_t, bool> m_can_attack_air_cache;

		double GroundRange(const ::sc2::Unit* unit);

		std::unordered_map<uint64_t, double> m_ground_range_cache;

		double AirRange(const ::sc2::Unit* unit);

		std::unordered_map<uint64_t, double> m_air_range_cache;

		double GroundDPS(const ::sc2::Unit* unit);

		std::unordered_map<uint64_t, double> m_ground_dps_cache;

		double AirDPS(const ::sc2::Unit* unit);

		std::unordered_map<uint64_t, double> m_air_dps_cache;

		::sc2::Units InAttackRange(const ::sc2::Unit* unit, ::sc2::Units targets);

		::sc2::UnitTypes m_unit_data_cache;

		AeolusBot& m_bot;
	};
}