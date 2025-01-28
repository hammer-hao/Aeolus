#include "shoot_target_in_range.h"
#include "attack_target_unit.h"

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <cmath>

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include "../../utils/unit_utils.h"

namespace Aeolus
{
	bool ShootTargetInRange::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		if (m_targets.empty()) return false;

		auto& mediator = ManagerMediator::getInstance();

		::sc2::Units targets;
		for (const auto& target : m_targets)
			if (target->cloak == ::sc2::Unit::CloakState::CloakedDetected
				|| target->cloak == ::sc2::Unit::CloakState::NotCloaked)
				targets.push_back(target);

		auto in_attack_range = mediator.GetUnitsInAtttackRange(aeolusbot, unit, targets);
		if (in_attack_range.empty()) return false;

		if (!unit->orders.empty() && unit->weapon_cooldown == 0.0f)
		{
			for (const auto& order : unit->orders)
			{
				for (const auto& enemy : in_attack_range) if (order.target_unit_tag == enemy->tag) return true;
			}
		}

		auto enemy_target = utils::PickAttackTarget(in_attack_range);

		if (_isAttackReady(aeolusbot, unit, enemy_target))
		{
			AttackTargetUnit attack_target_unit = AttackTargetUnit(enemy_target);
			return attack_target_unit.execute(aeolusbot, unit);
		}

		return false;
	}

	bool ShootTargetInRange::_isAttackReady(AeolusBot& aeolusbot, const ::sc2::Unit* unit, const ::sc2::Unit* target)
	{
		bool can_attack_air = ManagerMediator::getInstance().CanAttackAir(aeolusbot, unit);
		bool can_attack_ground = ManagerMediator::getInstance().CanAttackGround(aeolusbot, unit);

		if (!can_attack_air
			&& !can_attack_ground
			&& !(unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ORACLE))
		{
			return false;
		}

		constexpr float step_time = 1.0f / 22.4f;

		// get positions of unit and target
		auto unit_pos = unit->pos;
		auto target_pos = target->pos;

		// get the relative angle of the units
		float facing_angle = unit->facing;
		float target_angle = std::atan2(unit_pos.y - target_pos.y, unit_pos.x - target_pos.x);

		// get the difference between facing angle and target
		facing_angle = (facing_angle >= 0) ? facing_angle : facing_angle + 2 * 3.14159f;
		target_angle = (target_angle >= 0) ? target_angle : target_angle + 2 * 3.14159f;
		float angle_diff = std::fabs(facing_angle - target_angle);

		// time it will take for unit to turn and face the target
		float turn_time = angle_diff / utils::GetTurnRate(unit->unit_type);

		float range_vs_target = (target->is_flying) ?
			ManagerMediator::getInstance().AirRange(aeolusbot, unit) : ManagerMediator::getInstance().GroundRange(aeolusbot, unit);

		// time it will take for unit to move in range of the target
		float distance = ::sc2::Distance2D(unit->pos, target->pos) - unit->radius - target->radius - range_vs_target;
		distance = (distance >= 0) ? distance : 0;

		float time_needed =
			distance / ManagerMediator::getInstance().GetUnitMovementSpeed(aeolusbot, unit->unit_type)
			+ turn_time
			+ step_time;

		return time_needed >= (unit->weapon_cooldown / 22.4);
	}
}