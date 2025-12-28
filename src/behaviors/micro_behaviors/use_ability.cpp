#include "use_ability.h"
#include "../../Aeolus.h"

namespace Aeolus
{
	bool UseAbility::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		aeolusbot.Actions()->UnitCommand(unit, m_ability);
		return true;
	}
}