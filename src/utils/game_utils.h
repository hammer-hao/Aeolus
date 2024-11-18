#pragma once
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_interfaces.h>

namespace Aeolus
{
	namespace utils
	{
		::sc2::Units GetOwnedTownHalls(const sc2::ObservationInterface* observation);

		// Iterate through all buffs on the worker to see if any buff indicates resource carrying
		bool IsWorkerCarryingResource(const sc2::Unit* worker);

		bool HasAbilityQueued(const sc2::Unit* unit, sc2::ABILITY_ID ability);
	}
}