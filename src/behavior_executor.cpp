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
	void BehaviorExecutor::ExecuteBehaviors(AeolusBot& aeolusbot)
	{
		// std::cout << "Aeolus: Executing all behaviors!!" << std::endl;
		for (const auto& behavior : behaviors)
		{
			behavior->execute(aeolusbot);
		}
		behaviors.clear();
	}
}