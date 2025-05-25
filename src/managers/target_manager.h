#pragma once

#include "manager.h"
#include <tuple>
#include <any>

namespace Aeolus
{
	class AeolusBot;

	/**
	* Manager for calculating and bookkeeping the bot's target locations
	*/
	class TargetManager : public Manager
	{
	public:
		TargetManager(AeolusBot& aeolusbot) : m_bot(aeolusbot) 
		{
			m_attackTarget = { 0.0f, 0.0f };
			m_prismTarget = { 0.0f, 0.0f };
			m_defenseTarget = {};
			m_currentBaseTarget = 0;
		}

		std::string_view GetName() const override {
			static const std::string name = "TargetManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		void update(int iteration) override;

		void Initialize();

	private:
		AeolusBot& m_bot;

		::sc2::Point2D m_attackTarget;

		std::vector<::sc2::Point2D> m_defenseTarget;

		::sc2::Point2D m_prismTarget;

		::sc2::Point2D getAttackTarget();

		::sc2::Point2D getDefenseTarget(int baseLocation);

		int m_currentBaseTarget;
	};
}