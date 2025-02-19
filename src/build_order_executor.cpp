#include "build_order_executor.h"
#include "behaviors/macro_behaviors/build_structure.h"
#include "Aeolus.h"

namespace Aeolus
{

	void BuildOrderExecutor::execute(AeolusBot& aeolusbot)
	{
		auto* observations = aeolusbot.Observation();
		int supply_count = observations->GetFoodUsed();

		if (m_build_order.IsEmpty()) return;

		if (supply_count >= m_build_order.PeekNextStep().supply_threshold)
		{
			::sc2::UNIT_TYPEID to_build = m_build_order.PeekNextStep().unit_type;
			bool is_wall = m_build_order.PeekNextStep().is_wall;
			std::make_unique<BuildStructure>(to_build, 0, is_wall).get()->execute(aeolusbot);
			m_build_order.PopNextStep();
		}
	}

	std::string_view BuildOrderExecutor::getCurrentStep()
	{
		return m_build_order.getCurrentStep();
	}
}