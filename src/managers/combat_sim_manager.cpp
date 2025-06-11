#include "combat_sim_manager.h"
#include "../thirdparty/libvoxelbot/combat/simulator.h"

#include "../Aeolus.h"

namespace Aeolus
{
	std::any CombatSimManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		default:
			return 0;
		}
	}

 	CombatSimManager::CombatSimManager(AeolusBot& aeolusbot) : m_bot(aeolusbot) {
	}

	void CombatSimManager::Initialize()
	{
		initMappings();
		m_simulator = std::make_unique<CombatPredictor>();
	}

	void CombatSimManager::update(int iteration)
	{
	}
}