#include "unitbuildorderstep.h"

#include "buildorderstep.h"
#include "sc2api/sc2_typeenums.h"

#include "../Aeolus.h"
#include "../utils/unit_utils.h"
#include <stdexcept>


namespace Aeolus
{
	UnitBuildOrderStep::UnitBuildOrderStep(int supply_threshold, ::sc2::UNIT_TYPEID unit_id) :
		m_supply_threshold(supply_threshold), m_to_train(unit_id)
	{
	}

	bool UnitBuildOrderStep::execute(AeolusBot& aeolusbot)
	{
		auto result = utils::_isTrainedFrom(m_to_train);
		if (!result.has_value()) throw std::invalid_argument("Invalid unit type to train!");

		auto trainedFrom = result.value();

		auto all_structures = ManagerMediator::getInstance().GetAllOwnStructures(aeolusbot);
		::sc2::Units canTrain;

		std::copy_if(all_structures.begin(), all_structures.end(), std::back_inserter(canTrain),
			[trainedFrom](const ::sc2::Unit* unit) { return unit->unit_type == trainedFrom; });

		if (canTrain.empty()) return false; // no building available to train

		// need to make sure we have required tech
		::sc2::UNIT_TYPEID required_tech = ManagerMediator::getInstance().GetRequiredTech(aeolusbot, m_to_train);
		if (!std::any_of(all_structures.begin(), all_structures.end(),
			[required_tech](const ::sc2::Unit* structure) {return structure->unit_type == required_tech; }))
			return false; // does not have required tech yet.

		auto unitCost = ManagerMediator::getInstance().GetUnitCost(aeolusbot, m_to_train);
		if (aeolusbot.Observation()->GetMinerals() < unitCost.first
			|| aeolusbot.Observation()->GetVespene() < unitCost.second)
			return false; // not enough resources to afford this unit.

		for (const auto& structure : canTrain)
		{
			{
				// found available structure
				aeolusbot.Actions()->UnitCommand(structure,
					ManagerMediator::getInstance().GetCreationAbility(aeolusbot, m_to_train));
				return true;
			}
		}
		return false;
	}

	int UnitBuildOrderStep::getSupplyThreshold()
	{
		return m_supply_threshold;
	}

	std::string_view UnitBuildOrderStep::toString()
	{
		return ::sc2::UnitTypeToName(m_to_train);
	}
}