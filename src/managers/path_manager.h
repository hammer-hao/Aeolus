#pragma once

#include "manager.h"
#include "../pathing/map_data.h"
#include "../pathing/grid.h"

#include <sc2api/sc2_common.h>

namespace Aeolus
{
	class AeolusBot;
	class MapData;

	class PathManager : public Manager
	{
	public:
		// constructor will take AeolusBot object reference
		PathManager(AeolusBot& aeolusbot)
			: m_mapdata(aeolusbot), m_bot(aeolusbot)
		{
			std::cout << "Path manager initialization at address: " << this << std::endl;
		}

		std::string_view GetName() const override {
			static const std::string name = "PathManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		// Implement the update method.
		void update(int iteration) override;

		void AddUnitInfluence(const ::sc2::Unit* enemy);

	private:
		MapData m_mapdata;
		AeolusBot& m_bot;
		Grid m_ground_grid;

		void _addUnitInfluence(const ::sc2::Unit* unit);

		bool _isGroundPositionSafe(::sc2::Point2D position);

		void _reset_grids(); // reset all grids to cached versions

		::sc2::ImageData _getDefaultGridData();

		::sc2::Point2D _getClosestSafeSpot(::sc2::Point2D position, const double& radius);
	};
}