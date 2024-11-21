#pragma once

#include <sc2api/sc2_unit.h>
#include "manager.h"
#include "vector"
#include "string"
#include "map"

namespace Aeolus
{
	class AeolusBot;
	class NeutralUnitManager : public Manager
	{

	public:
		NeutralUnitManager(AeolusBot& aeolusbot)
		{
			std::cout << "Neutral Unit manager initialization" << std::endl;

			m_neutral_units = _GetNeutralUnits(aeolusbot);

			std::cout << "Neutral Unit manager initialization complete" << std::endl;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		::sc2::Units GetAllMineralPatches()
		{
			return m_neutral_units["Minerals"];
		}

		::sc2::Units GetAllVespeneGeysers()
		{
			return m_neutral_units["Vespenes"];
		}

		::sc2::Units GetAllDestructables()
		{
			return m_neutral_units["Destructables"];
		}

		// Implement the update method.
		void update(int iteration) override {
			std::cout << "Updating UnitRoleManager at iteration: " << iteration << std::endl;
		}

	private:
		std::map<std::string_view, ::sc2::Units> m_neutral_units;

		std::map<std::string_view, ::sc2::Units> _GetNeutralUnits(AeolusBot& aeolusbot);
	};
}

