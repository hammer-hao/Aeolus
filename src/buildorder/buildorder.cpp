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

	std::unique_ptr<BuildOrderStep> BuildOrder::peekNextStep() {
		if (m_build_order_queue.empty())
			return nullptr;  // or handle the empty case as needed
		return std::move(m_build_order_queue.front());
	}

	void BuildOrder::nextStep() {
		if (!m_build_order_queue.empty())
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

		// first item is pending, return false
		if (m_build_order_queue.front()->started())
		{
			if (m_build_order_queue.front()->isDone(m_aeolusbot))
			{
				// started and finished, go to the next build order
				m_build_order_queue.pop();
			}
			else 
			{
				// started but not finished, wait it out
				return false;
			}
		}

		// no build instructions left, return false
		if (m_build_order_queue.empty()) return false;
		// else, execute the build order
		if (!m_build_order_queue.front()->execute(m_aeolusbot))
		{
			return false;
		}
		else return true;
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