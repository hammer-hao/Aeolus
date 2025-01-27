#include "unit_utils.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <algorithm>
#include <cmath>
#include "../constants.h"
#include "../managers/manager_mediator.h"
#include "../Aeolus.h"


namespace Aeolus
{
	namespace utils {
		::sc2::Point2D ConvertTo2D(const ::sc2::Point3D& point3D)
		{
			return ::sc2::Point2D(point3D.x, point3D.y);
		}

		std::vector<sc2::Point2D> ConvertTo2DVector(const std::vector<::sc2::Point3D> vector_3d)
		{
			std::vector<::sc2::Point2D> vector_2d;
			vector_2d.reserve(vector_3d.size());

			for (const auto& point : vector_3d) {
				vector_2d.emplace_back(point.x, point.y);
			}

			return vector_2d;
		}

		// sort units by distance to a given point, returns sorted ::sc2::Units vector
		::sc2::Units SortByDistanceTo(
			const ::sc2::Units& units,
			const ::sc2::Point2D& target,
			bool reverse)
		{

			// std::cout << "Sorting units by distance to target..." << std::endl;
			// copy the vector to avoid modifying the original
			::sc2::Units sorted_units = units;

			// sort based on squared distance to target
			std::sort(sorted_units.begin(), sorted_units.end(),
				[&target, reverse](const ::sc2::Unit* a, const ::sc2::Unit* b)
				{
					if (reverse)
					{
						return ::sc2::Distance2D(ConvertTo2D(a->pos), target) > ::sc2::Distance2D(ConvertTo2D(b->pos), target);
					}
					else
					{
						return ::sc2::Distance2D(ConvertTo2D(a->pos), target) < ::sc2::Distance2D(ConvertTo2D(b->pos), target);
					}
				});

			return sorted_units;
		}

		bool HasAbilityQueued(const sc2::Unit* unit, sc2::ABILITY_ID ability) {
			if (unit == nullptr) {
				return false;
			}

			for (const auto& order : unit->orders) {
				if (order.ability_id == ability) {
					return true;
				}
			}
			return false;
		}

		const ::sc2::Unit* PickAttackTarget(::sc2::Units targets)
		{
			const ::sc2::Unit* attack_target = targets[0];
			float max_health = 10000.0;

			for (const auto& target : targets)
			{
				if ((target->health + target->shield) < max_health)
				{
					max_health = target->health + target->shield;
					attack_target = target;
				}
			}
			return attack_target;
		}

		std::optional<::sc2::UNIT_TYPEID> _isTrainedFrom(::sc2::UNIT_TYPEID unit_type)
		{
			if (constants::GATEWAY_UNITS.find(unit_type) != constants::GATEWAY_UNITS.end())
			{
				return ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY;
			}
			else if (constants::ROBO_UNITS.find(unit_type) != constants::ROBO_UNITS.end())
			{
				return ::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY;
			}
			else if (constants::STARGATE_UNITS.find(unit_type) != constants::STARGATE_UNITS.end())
			{
				return ::sc2::UNIT_TYPEID::PROTOSS_STARGATE;
			}
			return std::nullopt;
		}

		float GetTurnRate(const ::sc2::UNIT_TYPEID& unit_type) {
			static const std::unordered_map<::sc2::UNIT_TYPEID, float> TURN_RATE = {
				{::sc2::UNIT_TYPEID::PROTOSS_COLOSSUS, std::numeric_limits<float>::infinity()},
				{::sc2::UNIT_TYPEID::ZERG_INFESTORTERRAN, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_BANELING, 999.8437f},
				{::sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIP, std::numeric_limits<float>::infinity()},
				{::sc2::UNIT_TYPEID::ZERG_CHANGELING, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_CHANGELINGZEALOT, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_CHANGELINGMARINESHIELD, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_CHANGELINGMARINE, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_CHANGELINGZERGLINGWINGS, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_CHANGELINGZERGLING, 999.8437f},
				{::sc2::UNIT_TYPEID::TERRAN_SIEGETANK, 360.0f},
				{::sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED, 360.0f},
				{::sc2::UNIT_TYPEID::TERRAN_VIKINGASSAULT, 720.0f},
				{::sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER, 999.8437f},
				{::sc2::UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING, 720.0f},
				{::sc2::UNIT_TYPEID::TERRAN_FACTORYFLYING, 720.0f},
				{::sc2::UNIT_TYPEID::TERRAN_STARPORTFLYING, 720.0f},
				{::sc2::UNIT_TYPEID::TERRAN_SCV, 999.8437f},
				{::sc2::UNIT_TYPEID::TERRAN_BARRACKSFLYING, 720.0f},
				{::sc2::UNIT_TYPEID::TERRAN_MARINE, 999.8437f},
				{::sc2::UNIT_TYPEID::TERRAN_REAPER, 999.8437f},
				{::sc2::UNIT_TYPEID::TERRAN_GHOST, 999.8437f},
				{::sc2::UNIT_TYPEID::TERRAN_MARAUDER, 999.8437f},
				{::sc2::UNIT_TYPEID::TERRAN_THOR, 360.0f},
				{::sc2::UNIT_TYPEID::TERRAN_HELLION, 720.0f},
				{::sc2::UNIT_TYPEID::TERRAN_MEDIVAC, 999.8437f},
				{::sc2::UNIT_TYPEID::TERRAN_BANSHEE, 1499.9414f},
				{::sc2::UNIT_TYPEID::TERRAN_RAVEN, 999.8437f},
				{::sc2::UNIT_TYPEID::TERRAN_BATTLECRUISER, 720.0f},
				{::sc2::UNIT_TYPEID::PROTOSS_ZEALOT, 999.8437f},
				{::sc2::UNIT_TYPEID::PROTOSS_STALKER, 999.8437f},
				{::sc2::UNIT_TYPEID::PROTOSS_HIGHTEMPLAR, 999.8437f},
				{::sc2::UNIT_TYPEID::PROTOSS_DARKTEMPLAR, 999.8437f},
				{::sc2::UNIT_TYPEID::PROTOSS_SENTRY, 999.8437f},
				{::sc2::UNIT_TYPEID::PROTOSS_PHOENIX, 1499.9414f},
				{::sc2::UNIT_TYPEID::PROTOSS_CARRIER, 720.0f},
				{::sc2::UNIT_TYPEID::PROTOSS_VOIDRAY, 999.8437f},
				{::sc2::UNIT_TYPEID::PROTOSS_WARPPRISM, 720.0f},
				{::sc2::UNIT_TYPEID::PROTOSS_OBSERVER, 720.0f},
				{::sc2::UNIT_TYPEID::PROTOSS_IMMORTAL, std::numeric_limits<float>::infinity()},
				{::sc2::UNIT_TYPEID::PROTOSS_PROBE, 999.8437f},
				{::sc2::UNIT_TYPEID::PROTOSS_INTERCEPTOR, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_DRONE, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_ZERGLING, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_OVERLORD, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_HYDRALISK, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_MUTALISK, 1499.9414f},
				{::sc2::UNIT_TYPEID::ZERG_ULTRALISK, 360.0f},
				{::sc2::UNIT_TYPEID::ZERG_ROACH, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_INFESTOR, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_CORRUPTOR, 999.8437f},
				{::sc2::UNIT_TYPEID::ZERG_BROODLORD, 720.0f},
				// Add more entries here as needed
			};

			auto it = TURN_RATE.find(unit_type);
			return (it != TURN_RATE.end()) ? it->second : 0.0f; // Default turn rate if not found
		}

		bool isAttackReady(AeolusBot& aeolusbot, const ::sc2::Unit* unit, const ::sc2::Unit* target)
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
			float target_angle = std::atan2f(unit_pos.y - target_pos.y, unit_pos.x - target_pos.x);

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
}