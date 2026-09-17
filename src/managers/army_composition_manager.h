#pragma once

#include "manager.h"
#include "../enums.h"
#include "../managers/manager_mediator.h"
#include <sc2api/sc2_common.h>
#include <unordered_map>
#include <map>

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

        inline static const std::unordered_map<::sc2::UNIT_TYPEID, CompositionWeights> ARMY_COMPOSITION_LOOKUP = {
            // stalker; immortal; tempest; colossus; archon;
            {::sc2::UNIT_TYPEID::ZERG_ZERGLING, {0.35f, 0.00f, 0.00f, 0.30f, 0.25f}},
            {::sc2::UNIT_TYPEID::ZERG_BANELING, {0.45f, 0.00f, 0.00f, 0.15f, 0.40f}},
            {::sc2::UNIT_TYPEID::ZERG_ROACH, {0.50f, 0.40f, 0.10f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::ZERG_RAVAGER, {0.75f, 0.15f, 0.00f, 0.00f, 0.10f}},
            {::sc2::UNIT_TYPEID::ZERG_HYDRALISK, {0.40f, 0.00f, 0.00f, 0.60f, 0.00f}},
            {::sc2::UNIT_TYPEID::ZERG_LURKERMP, {0.00f, 0.00f, 1.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::ZERG_MUTALISK, {1.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::ZERG_CORRUPTOR, {1.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::ZERG_BROODLORD, {0.00f, 0.00f, 1.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::ZERG_ULTRALISK, {0.30f, 0.40f, 0.00f, 0.00f, 0.30f}},
            {::sc2::UNIT_TYPEID::ZERG_INFESTOR, {1.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::ZERG_VIPER, {1.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::ZERG_SWARMHOSTMP, {1.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::ZERG_QUEEN, {0.50f, 0.30f, 0.00f, 0.00f, 0.20f}},

            {::sc2::UNIT_TYPEID::TERRAN_MARINE, {0.40f, 0.00f, 0.00f, 0.50f, 0.10f}},
            {::sc2::UNIT_TYPEID::TERRAN_MARAUDER, {0.00f, 0.60f, 0.10f, 0.00f, 0.30f}},
            {::sc2::UNIT_TYPEID::TERRAN_REAPER, {0.80f, 0.00f, 0.00f, 0.20f, 0.00f}},
            {::sc2::UNIT_TYPEID::TERRAN_GHOST, {0.70f, 0.30f, 0.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::TERRAN_HELLION, {0.70f, 0.00f, 0.00f, 0.30f, 0.00f}},
            {::sc2::UNIT_TYPEID::TERRAN_HELLIONTANK, {0.70f, 0.00f, 0.00f, 0.30f, 0.00f}},
            {::sc2::UNIT_TYPEID::TERRAN_WIDOWMINE, {0.50f, 0.00f, 0.00f, 0.50f, 0.00f}},
            {::sc2::UNIT_TYPEID::TERRAN_CYCLONE, {0.00f, 0.60f, 0.00f, 1.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::TERRAN_SIEGETANK, {0.00f, 0.40f, 0.00f, 0.60f, 0.00f}},
            {::sc2::UNIT_TYPEID::TERRAN_THOR, {0.00f, 0.60f, 0.00f, 0.40f, 0.00f}},
            {::sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER, {1.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::TERRAN_MEDIVAC, {0.50f, 0.00f, 0.30f, 0.00f, 0.20f}},
            {::sc2::UNIT_TYPEID::TERRAN_LIBERATOR, {0.00f, 0.00f, 1.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::TERRAN_BANSHEE, {0.50f, 0.00f, 0.50f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::TERRAN_RAVEN, {0.50f, 0.00f, 0.50f, 0.0f, 0.00f}},
            {::sc2::UNIT_TYPEID::TERRAN_BATTLECRUISER, {0.00f, 0.00f, 1.00f, 0.00f, 0.00f}},

            {::sc2::UNIT_TYPEID::PROTOSS_ZEALOT, {0.20f, 0.00f, 0.00f, 0.30f, 0.50f}},
            {::sc2::UNIT_TYPEID::PROTOSS_ADEPT, {0.20f, 0.00f, 0.00f, 0.30f, 0.50f}},
            {::sc2::UNIT_TYPEID::PROTOSS_STALKER, {0.40f, 0.60f, 0.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::PROTOSS_SENTRY, {1.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::PROTOSS_ARCHON, {1.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::PROTOSS_IMMORTAL, {0.00f, 0.00f, 1.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::PROTOSS_COLOSSUS, {0.00f, 0.50f, 0.50f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::PROTOSS_DISRUPTOR, {0.00f, 0.50f, 0.50f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::PROTOSS_PHOENIX, {1.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::PROTOSS_ORACLE, {1.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::PROTOSS_VOIDRAY, {0.40f, 0.00f, 0.00f, 0.00f, 0.60f}},
            {::sc2::UNIT_TYPEID::PROTOSS_CARRIER, {0.00f, 0.00f, 0.00f, 1.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::PROTOSS_TEMPEST, {0.50f, 0.00f, 0.00f, 0.00f, 0.50f}},
            {::sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIP, {0.00f, 0.00f, 1.00f, 0.00f, 0.00f}},
            {::sc2::UNIT_TYPEID::PROTOSS_HIGHTEMPLAR, {1.00f, 0.00f, 0.00f, 0.00f, 0.00f}}, 
            {::sc2::UNIT_TYPEID::PROTOSS_DARKTEMPLAR, {1.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
        };

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		void update(int iteration) override;

	private:
		AeolusBot& m_bot;

        std::map<::sc2::UNIT_TYPEID, float> m_best_army_composition;
	};
}
