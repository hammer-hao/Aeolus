#pragma once
#include "build_order_enum.h"
#include <sc2api/sc2_unit.h>
#include <queue>

namespace Aeolus
{
	struct BuildOrderStep {
		int supply_threshold;
		::sc2::UNIT_TYPEID unit_type;
		bool is_wall;

		BuildOrderStep(int supp, ::sc2::UNIT_TYPEID unit, bool wall) :
			supply_threshold(supp), unit_type(unit), is_wall(wall) {}
	};

	/**
	* @brief build order class in charge of storing complete build information
	*/
	struct BuildOrder
	{
	public:
		BuildOrder(bool auto_expand = true) : m_auto_expand(auto_expand) {}
		~BuildOrder() = default;

		/**
		* @brief Add a new step to the build order.
		*/
		void AddStep(BuildOrderStep build_order_step);

		/**
		* @brief returns the next step
		*/
		BuildOrderStep BuildOrder::PeekNextStep() const;

		/**
		* Returns the next step to be executed and removed it from the build order.
		*/
		BuildOrderStep PopNextStep();

		/**
		* @brief Returns the string representation of the current step
		*/
		std::string_view getCurrentStep();

		/**
		* Returns whether this build
		*/
		bool GetAutoExpand() const;

		/**
		* @brief Returns whether the build order queue is empty
		*/
		bool IsEmpty() const;

	private:
		std::queue<BuildOrderStep> m_build_order_steps;

		bool m_auto_expand;
	};
}