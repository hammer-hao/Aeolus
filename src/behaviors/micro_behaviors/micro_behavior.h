#pragma once

#include "../behavior.h"
#include "micro_maneuver.h"
#include <sc2api/sc2_unit.h>
#include <vector>

namespace Aeolus
{
	class AeolusBot;
	/* @brief The MicroBehavior encapsulates multiple MiroManeuver to form
	a complete micro logic. Manages and executes MicroManeuver sequentially
	for a single unit. */
	class MicroBehavior : public Behavior
	{
	public:
		/*
		@ Creates a new micro behavior for the give unit 
		*/
		MicroBehavior(const ::sc2::Unit* unit) : m_unit(unit) {}

		~MicroBehavior() override = default;

		/*
		@brief Registers a maneuver to this behavior.
		*/
		void AddBehavior(std::unique_ptr<MicroManeuver> maneuver)
		{
			m_maneuvers.push_back(std::move(maneuver));
		}

		/*
		@brief Executes the first valid maneuver in the list of maneuvers.
		*/
		virtual bool execute(AeolusBot& aeolusbot) override
		{
			for (auto& maneuver : m_maneuvers)
			{
				if (maneuver->execute(aeolusbot, m_unit)) return true;
			}
			return false;
		}

	private:
		std::vector<std::unique_ptr<MicroManeuver>> m_maneuvers;

		const ::sc2::Unit* m_unit;
	};
}