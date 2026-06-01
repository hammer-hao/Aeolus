#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <functional>

#include <sc2api/sc2_typeenums.h>
#include <sc2api/sc2_common.h>

namespace Aeolus
{
	struct TilePos {
		int x, y;

		TilePos() = default;
		TilePos(int xx, int yy) noexcept : x(xx), y(yy) {}

		TilePos(float xf, float yf) noexcept
			: x(int(std::round(xf * 2.0f))),
			y(int(std::round(yf * 2.0f)))
		{}

		TilePos(std::pair<float, float> p) noexcept
			: x(int(std::round(p.first * 2.0f))),   // *2 half-tile to integer
			y(int(std::round(p.second * 2.0f)))
		{}

		sc2::Point2D toWorld() const noexcept {
			return { x * 0.5f, y * 0.5f };
		}

		bool operator==(TilePos o) const { return x == o.x && y == o.y; }
	};

	struct BuildingAttributes {
		bool available = true;
		bool is_wall = false;
		int building_tag = 0;
		bool worker_on_route = false;
		double time_requested = 0.0;
		bool production_pylon = false;
		bool optimal_pylon = false;

		BuildingAttributes(
			bool available = true,
			bool is_wall = false,
			int building_tag = 0,
			bool worker_on_route = false,
			double time_requested = 0.0,
			bool production_pylon = false,
			bool optimal_pylon = false
		) : available(available), is_wall(is_wall), building_tag(building_tag),
			worker_on_route(worker_on_route), time_requested(time_requested),
			production_pylon(production_pylon), optimal_pylon(optimal_pylon)
		{
		}
	};

	enum BuildingTypes
	{
		BUILDING_2X2,
		BUILDING_3X3,
		BUILDING_5X5,
		NOT_FOUND
	};

	using BuildingMap = std::unordered_map<TilePos, BuildingAttributes>;
	using BuildingTypeMap = std::unordered_map<BuildingTypes, BuildingMap>;
	using ExpansionMap = std::vector<BuildingTypeMap>;

	struct BuildingOrder {
		::sc2::UNIT_TYPEID building_id = ::sc2::UNIT_TYPEID::PROTOSS_PYLON;
		::sc2::Point2D target = { 0, 0 };
		double time_requested = 0.0;
		bool order_complete = false;

		BuildingOrder(
			::sc2::UNIT_TYPEID building_id = ::sc2::UNIT_TYPEID::PROTOSS_PYLON,
			::sc2::Point2D target = { 0, 0 },
			double time_requested = 0.0,
			bool order_complete = false
		) : building_id(building_id),
			target(target),
			time_requested(time_requested),
			order_complete(order_complete)
		{
		}
	};

	enum class HarassmentStatus
	{
		HEADING_TO_BASE,
		HARASSING_AT_MAIN,
		HARASSING_AT_NATURAL,
		HARASSING_AT_THIRD,
		SURVIVING,
	};
}

namespace std 
{
	template<>
	struct hash<Aeolus::TilePos> {
		size_t operator()(Aeolus::TilePos const& p) const noexcept {
			// pack x,y into a 64-bit and xor
			return (uint64_t(uint32_t(p.x)) << 32)
				^ uint32_t(p.y);
		}
	};
}