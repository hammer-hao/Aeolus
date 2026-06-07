#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_typeenums.h>

namespace Aeolus
{
	struct ScoutingCondition
	{
		::sc2::UNIT_TYPEID unitType;
		int count;
		int before_seconds;
		bool is_proxied;
	};

	struct ContingencyPlan
	{
	public:
		std::string name;
		std::vector<ScoutingCondition> conditions;
		std::unordered_map<::sc2::UNIT_TYPEID, float> army_composition;
		int move_out_supply;
		int cannons_to_add;
		bool check_no_enemy_expansion;
	};
}