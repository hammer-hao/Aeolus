#pragma once

#include "manager.h"

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief The army composition manager is the single source of truth of the best current
	* army composition to use against the known enemy units.
	*/
	class ArmyCompositionManager : public Manager
	{
	public:
		ArmyCompositionManager(AeolusBot& aeolusbot) : m_bot(aeolusbot){}

		std::string_view GetName() const override {
			static const std::string name = "ArmyCompositionManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		void update(int iteration) override;

	private:
		AeolusBot& m_bot;
	};
}
