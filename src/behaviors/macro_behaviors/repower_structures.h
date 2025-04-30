#pragma once

#include "macro_behavior.h"
#include "../../enums.h"
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief Macro behavior class responsible for detecting any protoss structures
	* left unpowered by pylons, and building pylons to repower those structures.
	*/
	class RepowerStructures : public MacroBehavior
	{
	public:
		RepowerStructures() {}

		~RepowerStructures() override = default;

		bool execute(AeolusBot& aeolusbot) override;

	private:
		bool isRepowering(AeolusBot& aeolusbot, const ::sc2::Unit* structure);

		bool repower(AeolusBot& aeolusbot, const ::sc2::Unit* structure, const ExpansionMap& allPlacements);
	};
}