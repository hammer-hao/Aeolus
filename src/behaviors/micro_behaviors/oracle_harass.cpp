#include "oracle_harass.h"
#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <vector>

namespace Aeolus
{
	bool OracleHarass::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit) {
		// must be oracle for this behavior
		if (unit->unit_type != ::sc2::UNIT_TYPEID::PROTOSS_ORACLE) return false;


	}
}