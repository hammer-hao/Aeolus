#pragma once

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	class AeolusBot;

	/*
	* @brief Issues a move command, but avoids repeating the command if it is already being performed.
	*/
	class Move : public MicroManeuver
	{
	public:
		Move(::sc2::Point2D target) : m_target(target) {}

		~Move() override = default;

		bool execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit) override;

	private:
		::sc2::Point2D m_target;
	};
}