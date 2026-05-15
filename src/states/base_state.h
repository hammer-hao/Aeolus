#pragma once

#include "bot_state.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_common.h>
#include <string>

namespace Aeolus 
{
	class AeolusBot;

	/**
	* @brief Base state for all states to inherit from
	*/
	class BaseState : public BotState
	{
	public:
		virtual std::string_view getName() const = 0;

		void declareEnter();

		void declareExit();

	protected:
		/**
		* @brief Perform bookkeeping macro tasks.
		* 1. Mine
		* 2. Scout
		* 3. Build workers
		* 4. Chrono boost
		* 5. Repower structures
		*/
		void doBookKeepingMacroTasks(AeolusBot& aeolusbot);

		/**
		* @brief Perform general micro, i.e. most naive type of self-preservation +
		* target fire micro, against a given 2D position as target.
		* Can be used by most ranged units with an auto attack.
		* 
		* This should be the default micro method to use for ranged units unless explicitly
		* specified otherwise.
		*/
		void doGeneralMicro(AeolusBot& aeolusbot, ::sc2::Units forces, ::sc2::Point2D target);

		/**
		* @brief Execute warp prism micro.
		* Will retrieve the most optinum prism move target from manager mediator,
		* then control all available warp prisms to perform pick-up micro
		* to save low-health units nearby
		*/
		void doPrismPickUpMicro(AeolusBot& aeolusbot);

		/**
		* @brief Execute Oracle Harassment micro
		* Will gather existing oracles and "harass" micro them against an array of target points.
		*/
		void doOracleHarassMicro(AeolusBot& aeolusbot);

		/**
		* @brief perform high economy macro tasks
		*/
		void doHighEconomyMacroTasks(AeolusBot& aeolusbot, bool forceDetection);

		/**
		* @brief perform micro on observers
		*/
		void doObserverMicro(AeolusBot& aeolusbot);
	};
} // namespace Aeolus

