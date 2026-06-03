#include "use_ability.h"
#include "../../Aeolus.h"

namespace Aeolus
{
	bool UseAbility::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		if (m_has_target)
		{
			aeolusbot.Actions()->UnitCommand(unit, m_ability, m_target);
		}
		else 
		{
			aeolusbot.Actions()->UnitCommand(unit, m_ability);
		}
		return true;
	}
}