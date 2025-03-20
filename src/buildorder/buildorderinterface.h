#pragma once

#include "buildorderstep.h"
#include <memory>
#include <string>

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief interface for the build order that Aeolus bot will be using.
	* Any build order must adhere to this in order to be plugged into the bot.
	* adding step will not be available in this interface, as it is expected to
	* be used during the game execution.
	* To add step to the build order, modify its related build order factory function.
	*/
	struct BuildOrderInterface
	{
	public:
		/**
		* @brief returns the next step in the build order without modifying it.
		*/
		virtual std::unique_ptr<BuildOrderStep> peekNextStep() = 0;

		/**
		* @brief mark the current step as complete and go to the next step.
		*/
		virtual void nextStep() = 0;

		/**
		* @brief returns the string representation of the current step.
		*/
		virtual std::string_view getCurrentStep() = 0;

		/**
		* @brief attempt to execute the current step. Returns whether the step
		* has been successfully executed.
		*/
		virtual bool execute() = 0;

		/**
		* @brief returns whether this build order is finished.
		*/
		virtual bool isFinished() = 0;

		/**
		* @brief returns whether this build should auto expand.
		*/
		virtual bool autoExpand() = 0;

		virtual ~BuildOrderInterface() = default;
	};
}