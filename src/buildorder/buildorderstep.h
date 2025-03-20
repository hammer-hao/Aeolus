#pragma once

#include <memory>
#include <string>

namespace Aeolus
{
	class AeolusBot;

	struct BuildOrderStep {
	public:
		// execute this build order
		virtual bool execute(AeolusBot& aeolusbot) = 0;

		// get the supply threshold to execute this build order
		virtual int getSupplyThreshold() = 0;

		// get the srting representation of this step
		virtual std::string_view toString() = 0;

		virtual ~BuildOrderStep() {}

		/*
		int supply_threshold;
		::sc2::UNIT_TYPEID unit_type;
		bool is_wall;

		BuildOrderStep(int supp, ::sc2::UNIT_TYPEID unit, bool wall) :
			supply_threshold(supp), unit_type(unit), is_wall(wall) {}
		*/
	};
}