#pragma once

#include <iostream>
#include <any>
#include "../constants.h"

namespace Aeolus
{
	class AeolusBot;
	class Manager
	{
		// Abstract base class for managers
		// Includes interface all managers should adhere to
	public:

		Manager() = default;
		
		virtual void initialise()
		{
			std::cout << "Manager Initialised" << std::endl;

		}

		virtual std::any ProcessRequest(
			AeolusBot& aeolusbot,
			constants::ManagerRequestType request,
			std::any args
		) = 0;

		// Pure virtual method to update the manager at each iteration. Must be implemented by derived classes.
		virtual void update(int iteration) = 0;
	};
}