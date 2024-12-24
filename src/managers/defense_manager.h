#pragma once

#include "manager.h"
#include "../constants.h"

namespace Aeolus
{
	class AeolusBot;

	class DefenseManager : public Manager
	{
	public:
		DefenseManager(AeolusBot& aeolusbot) : m_bot(aeolusbot)
		{
		}

		std::string_view GetName() const override {
			static const std::string name = "DefenseManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		void update(int iteration) override;

	private:
		AeolusBot& m_bot;
	};
}