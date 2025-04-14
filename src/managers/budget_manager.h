#pragma once

#include "manager.h"

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief The budget manager handles the reservation of mineral / vespene for requests
	* from different managers / behaviors. Prevents the "crowding out" effect of resources.
	*/
	class BudgetManager : public Manager
	{
	public:
		BudgetManager(AeolusBot& aeolusbot) : m_bot(aeolusbot), m_mineralOffset(0), m_vespeneOffset(0) {}

		std::string_view GetName() const override {
			static const std::string name = "BudgetManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		void update(int iteration) override;

	private:
		AeolusBot& m_bot;

		int m_mineralOffset;

		int m_vespeneOffset;

		int _getMinerals();

		int _getVespene();

		void _reserveMinerals(int amount);

		void _reserveVespene(int amount);

		void _freeMinerals(int amount);

		void _freeVespene(int amount);
	};
}