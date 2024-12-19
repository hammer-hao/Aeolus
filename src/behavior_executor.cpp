#include "behavior_executor.h"
#include "behaviors/behavior.h" 
#include "managers/manager_mediator.h"
#include <iostream>

namespace Aeolus
{
	// constructor implementation (not needed for now)
	BehaviorExecutor::BehaviorExecutor() 
	{
		std::cout << "Behavior executor created!" << std::endl;
	}
	BehaviorExecutor::~BehaviorExecutor()
	{
		std::cout << "Behavior executor destroyed!" << std::endl;
	}
	BehaviorExecutor& BehaviorExecutor::GetInstance()
	{
		static BehaviorExecutor instance;
		return instance;
	}
	void BehaviorExecutor::AddBehavior(std::unique_ptr<Behavior> behavior)
	{
		behaviors.push_back(std::move(behavior));
	}
	void BehaviorExecutor::ExecuteBehaviors(AeolusBot& bot) {
		if (behaviors.empty()) {
			std::cout << "Behavior executor: No behaviors to execute." << std::endl;
			return;
		}

		for (const auto& behavior : behaviors) {
			if (!behavior) {
				std::cerr << "Behavior executor: Encountered a null behavior!" << std::endl;
				continue;
			}
			try {
				behavior->execute(bot);
			}
			catch (const std::exception& e) {
				std::cerr << "Behavior executor: Exception during behavior execution: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "Behavior executor: Unknown exception during behavior execution!" << std::endl;
			}

			// Cleanup all behaviors after execution
			for (auto& behavior : behaviors) {
				behavior.reset(); // Explicitly destroy each behavior
			}

			behaviors.clear(); // Clear the list of unique_ptrs

		}
	}
}