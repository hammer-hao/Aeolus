#include "build_order_executor.h"
#include "behaviors/macro_behaviors/build_structure.h"
#include "Aeolus.h"

namespace Aeolus
{

	void BuildOrderExecutor::execute(AeolusBot& aeolusbot)
	{
		auto* observations = aeolusbot.Observation();
		int supply_count = observations->GetFoodUsed();

		if (m_build_order_steps.empty()) return;

		if (supply_count >= m_build_order_steps.front().supply_threshold)
		{
			::sc2::UNIT_TYPEID to_build = m_build_order_steps.front().unit_type;
			bool is_wall = m_build_order_steps.front().is_wall;
			std::make_unique<BuildStructure>(to_build, 0, is_wall).get()->execute(aeolusbot);
			m_build_order_steps.pop();
		}
	}

	std::string_view BuildOrderExecutor::getCurrentStep()
	{
		if (m_build_order_steps.empty()) return "Build finished.";
		return (::sc2::UnitTypeToName(m_build_order_steps.front().unit_type));
	}
}