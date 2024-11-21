#pragma once

#include "manager.h"
#include "../pathing/map_data.h"

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

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		// Implement the update method.
		void update(int iteration) override {
			std::cout << "Updating PathManager at iteration: " << iteration << std::endl;
			m_mapdata.update();
		}

	private:
		MapData m_mapdata;
	};
}