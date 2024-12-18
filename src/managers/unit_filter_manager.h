#pragma once

#include <sc2api/sc2_unit.h>
#include "manager.h"
#include "../constants.h"

namespace Aeolus
{
	class AeousBot;
	class UnitFilterManager : public Manager
	{
	public:
		UnitFilterManager(AeolusBot& aeolusbot) 
		{
			std::cout << "Unit filter manager initialization" << std::endl;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		// Implement the update method.
		void update(int iteration) override {
			// std::cout << "Updating UnitFilterManager at iteration: " << iteration << std::endl;
		}

	private:
		::sc2::Units _getAllStructures(AeolusBot& aeolusbot, ::sc2::Unit::Alliance alliance);
	};
}