#pragma once

#include <vector>
#include <memory>

namespace Aeolus
{
	class Behavior;  // Forward declare Behavior class

	class AeolusBot;

	class BehaviorExecutor
	{
		/*
		Executes the behavior added by the user at each step.
		During the step, behaviors will be added into the
		behavior executor as specified by game logic.

		At the very end of each step, the behavior executor
		shall be called and each behavior will be executed
		sequentially in the same order they are added.

		Clears out all behaviors once executed.

		Attributes:
		----------------
		ai: AeolusBot
			The bot object running the game
		mediator: ManagerMediator
			Used for getting information from other managers
		*/
	private:
		std::vector<std::unique_ptr<Behavior>> behaviors;

		//constructor
		BehaviorExecutor();
		//destructor
		~BehaviorExecutor();

	public:
		static BehaviorExecutor& GetInstance();

		// for adding a behavior to the list
		void AddBehavior(std::unique_ptr<Behavior> behavior);

		// for executing behaviors
		void ExecuteBehaviors(AeolusBot& aeolusbot);
	};
}
