#pragma once

#include <memory>
#include "manager.h"
#include "../thirdparty/libvoxelbot/combat/simulator.h"
#include <sc2api/sc2_typeenums.h>

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief the combat sim manager serves as an adapter to the combat sim logic in libvoxelbot,
	* it will handle requests for simulating combat and return the simulated results.
	*/
	class CombatSimManager : public Manager
	{
	public:
		
		CombatSimManager(AeolusBot& aeolusbot);

		std::string_view GetName() const override
		{
			static const std::string_view name = "CombatSimManager";
			return name;
		}

		void update(int iteration) override;

		void Initialize();

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

	private:

		AeolusBot& m_bot;

		std::unique_ptr<CombatPredictor> m_simulator;

		bool _predictEngagement(std::vector<::sc2::UNIT_TYPEID> own_army, std::vector<::sc2::UNIT_TYPEID> opponent_army);
	};
}