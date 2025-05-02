#pragma once

#include "macro_behavior.h"
#include <vector>
#include <sc2api/sc2_typeenums.h>

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief The upgrade controller class will attempt to research the next
	* upgrade in the given vector of upgrade ids that is available.
	*/
	class UpgradesController : public MacroBehavior
	{
	public:
		UpgradesController(std::vector<::sc2::UPGRADE_ID> upgrades) : m_upgrades(upgrades)
		{
		}

		~UpgradesController() override = default;

		bool execute(AeolusBot& aeolusbot) override;

	private:
		std::vector<::sc2::UPGRADE_ID> m_upgrades;
	};
}