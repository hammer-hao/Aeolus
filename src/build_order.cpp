#include "build_order.h"

#include <sc2api/sc2_unit.h>
#include <queue>

namespace Aeolus
{
	void BuildOrder::AddStep(BuildOrderStep build_order_step)
	{
		m_build_order_steps.push(build_order_step);
	}

	BuildOrderStep BuildOrder::PopNextStep()
	{
		auto result = m_build_order_steps.front();
		m_build_order_steps.pop();
		return result;
	}

	BuildOrderStep BuildOrder::PeekNextStep() const
	{
		return m_build_order_steps.front();
	}

	bool BuildOrder::GetAutoExpand() const
	{
		return m_auto_expand;
	}

	bool BuildOrder::IsEmpty() const
	{
		return m_build_order_steps.empty();
	}

	std::string_view BuildOrder::getCurrentStep()
	{
		if (m_build_order_steps.empty()) return "Build finished.";
		return (::sc2::UnitTypeToName(m_build_order_steps.front().unit_type));
	}
}