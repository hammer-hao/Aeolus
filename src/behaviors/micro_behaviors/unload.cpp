#include "unload.h"
#include "../../Aeolus.h"

namespace Aeolus
{
	bool Unload::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		if (unit->cargo_space_taken <= 0) return false;

		std::cout << "issueing unload all command..." << std::endl;
		aeolusbot.Actions()->UnitCommand(unit, ::sc2::ABILITY_ID::UNLOADALLAT, unit->pos);
		return true;
	}
}