#include "buildorder.h"

#include "buildorderinterface.h"
#include "buildorderstep.h"
#include "buildorderenum.h"
#include <vector>
#include <memory>
#include <queue>
#include <sc2api/sc2_unit.h>

#include "../Aeolus.h"

namespace Aeolus
{
	BuildOrder::BuildOrder(AeolusBot& aeolusbot, 
		std::vector<std::unique_ptr<BuildOrderStep>>&& buildOrderSteps) : m_aeolusbot(aeolusbot)
	{
		for (auto& step : buildOrderSteps)
		{
			m_build_order_queue.push(std::move(step));
		}
		m_auto_expand = true;
	}

	std::unique_ptr<BuildOrderStep> BuildOrder::peekNextStep()
	{
		return std::move(m_build_order_queue.front());
	}

	void BuildOrder::nextStep()
	{
		m_build_order_queue.pop();
	}

	bool BuildOrder::isFinished()
	{
		return m_build_order_queue.empty();
	}

	bool BuildOrder::execute()
	{
		// no build instructions left, return false
		if (m_build_order_queue.empty()) return false;

		// else, execute the build order
		if (m_build_order_queue.front()->execute(m_aeolusbot))
		{
			m_build_order_queue.pop();
			return true;
		}
		return false;
	}

	std::string_view BuildOrder::getCurrentStep()
	{
		if (m_build_order_queue.empty()) return "Build finished.";
		return m_build_order_queue.front()->toString();
	}

	bool BuildOrder::autoExpand()
	{
		return m_auto_expand;
	}
}