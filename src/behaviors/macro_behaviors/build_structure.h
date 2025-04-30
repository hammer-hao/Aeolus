#pragma once

#include "macro_behavior.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_common.h>

namespace Aeolus
{
	class AeolusBot;
	/*
	* @brief Builds a structure at a requested location. Arguments: structure id, base-index (starts from 0), is-wall
	*/
	class BuildStructure : public MacroBehavior
	{
	public:
		/**
		* @brief Creates a new BuildStructure behavior.
		*/
		BuildStructure(::sc2::UNIT_TYPEID structure_id, int base_index, bool is_wall) :
			structure_id(structure_id), base_index(base_index), is_wall(is_wall), m_build_close_to(false)
		{
			m_close_to = { 0, 0 };
		}

		/**
		* @brief Creates a new BuildStructure behavior. Builds close to a target point
		*/
		BuildStructure(::sc2::UNIT_TYPEID structure_id, int base_index, ::sc2::Point2D close_to) :
			structure_id(structure_id), base_index(base_index), is_wall(true), m_close_to(close_to),
			m_build_close_to(true)
		{
		}

		~BuildStructure() override = default;
		bool execute(AeolusBot& aeolusbot) override;

	private:
		::sc2::UNIT_TYPEID structure_id;
		::sc2::Point2D m_close_to;
		bool m_build_close_to;
		int base_index;
		bool is_wall;
	};
}