#pragma once

#include "manager.h"
#include "../pathing/map_data.h"
#include "../pathing/grid.h"

namespace Aeolus
{
	class AeolusBot;
	class MapData;

	class PathManager : public Manager
	{
	public:
		// constructor will take AeolusBot object reference
		PathManager(AeolusBot& aeolusbot)
			: m_mapdata(aeolusbot) 
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

		void AddUnitInfluence(::sc2::Unit* enemy);

	private:
		MapData m_mapdata;

		Grid m_ground_grid;

		void _addUnitInfluence(::sc2::Unit* unit);

		::sc2::ImageData _getDefaultGridData();
	};
}