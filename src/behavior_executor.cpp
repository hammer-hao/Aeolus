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
			behavior->execute(bot); // Ensure this call does not throw exceptions.
		}
	}
}