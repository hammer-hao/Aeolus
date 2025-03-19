#pragma once

#include "buildorderinterface.h"
#include "buildorderstep.h"
#include "buildorderenum.h"
#include <vector>
#include <memory>
#include <queue>

namespace Aeolus
{
	class AeolusBot;

	class BuildOrder : public BuildOrderInterface
	{
	public:
		/**
		* create a build order from a vector of unique pointers to BuildOrderStep.
		*/
		BuildOrder(std::vector<std::unique_ptr<BuildOrderStep>>&& buildOrderSteps);

		/**
		* @brief returns the next step in the build order without modifying it.
		*/
		std::unique_ptr<BuildOrderStep> peekNextStep() override;

		/**
		* @brief mark the current step as complete and go to the next step.
		*/
		void nextStep() override;

		/**
		* @brief returns the string representation of the current step.
		*/
		std::string_view getCurrentStep() override;

		/**
		* @brief attempt to execute the current step. Returns whether the step
		* has been successfully executed.
		*/
		bool execute(AeolusBot& aeolusbot) override;

		/**
		* @brief returns whether this build order is finished.
		*/
		bool isFinished() override;

		/**
		* @brief returns whether this build should auto expand.
		*/
		bool autoExpand() override;

	private:
		std::queue<std::unique_ptr<BuildOrderStep>> m_build_order_queue;
		bool m_auto_expand;
	};
}