#pragma once

#include "macro_behavior.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_common.h>

namespace Aeolus
{
	class AeolusBot;
	/*
	* @brief Builds a structure at a requested location
	*/
	class BuildStructure : public MacroBehavior
	{
	public:
		/**
		* @brief Creates a new BuildStructure behavior.
		*/
		BuildStructure(::sc2::UNIT_TYPEID structure_id, int base_index, bool is_wall) :
			structure_id(structure_id), base_index(base_index), is_wall(is_wall)
		{
		}
		~BuildStructure() override = default;
		bool execute(AeolusBot& aeolusbot) override;

	private:
		::sc2::UNIT_TYPEID structure_id;
		int base_index;
		bool is_wall;
	};
}