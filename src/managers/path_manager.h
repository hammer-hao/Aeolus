#pragma once

#include "manager.h"

namespace Aeolus
{
	class AeolusBot;

	class PathManager : public Manager
	{
	public:
		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		// Implement the update method.
		void update(int iteration) override {
			std::cout << "Updating PathManager at iteration: " << iteration << std::endl;
		}

	private:
	};
}