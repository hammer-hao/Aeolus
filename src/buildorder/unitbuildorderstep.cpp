#include "unitbuildorderstep.h"

#include "buildorderstep.h"
#include "sc2api/sc2_typeenums.h"

#include "../Aeolus.h"
#include "../utils/unit_utils.h"
#include <stdexcept>


namespace Aeolus
{
	UnitBuildOrderStep::UnitBuildOrderStep(int supply_threshold, ::sc2::UNIT_TYPEID unit_id) :
		m_supply_threshold(supply_threshold), m_to_train(unit_id), m_started(false)
	{
	}

	bool UnitBuildOrderStep::execute(AeolusBot& aeolusbot)
	{
		ManagerMediator& mediator = ManagerMediator::getInstance();

		// std::cout << "Training " << sc2::UnitTypeToName(m_to_train) << std::endl;
		auto result = utils::_isTrainedFrom(m_to_train);
		if (!result.has_value()) throw std::invalid_argument("Invalid unit type to train!");

		auto trainedFrom = result.value();

		auto all_structures = mediator.GetAllOwnStructures(aeolusbot);
		::sc2::Units canTrain;

		std::copy_if(all_structures.begin(), all_structures.end(), std::back_inserter(canTrain),
			[trainedFrom](const ::sc2::Unit* unit) { 
				return unit->unit_type == trainedFrom && unit->build_progress >= 1.0 && unit->orders.empty(); 
			});

		if (canTrain.empty())
		{
			// std::cout << "no building available to train!" << std::endl;
			return false; // no building available to train
		}

		// need to make sure we have required tech
		::sc2::UNIT_TYPEID required_tech = mediator.GetRequiredTech(aeolusbot, m_to_train);
		if (required_tech != ::sc2::UNIT_TYPEID::INVALID &&
			!std::any_of(all_structures.begin(), all_structures.end(),
			[required_tech](const ::sc2::Unit* structure) {return structure->unit_type == required_tech; }))
		{
			// std::cout << "we do not have the required tech to train " << ::sc2::UnitTypeToName(m_to_train) << std::endl;
			// std::cout << "required tech: " << ::sc2::UnitTypeToName(required_tech) << std::endl;
			return false; // does not have required tech yet.
		}

		auto unitCost = mediator.GetUnitCost(aeolusbot, m_to_train);
		if (mediator.GetMinerals(aeolusbot) < unitCost.first
			|| mediator.GetVespene(aeolusbot) < unitCost.second)
		{
			// std::cout << "Not enough resources! current minerals: " << mediator.GetMinerals(aeolusbot)
			// << " current gas: " << mediator.GetVespene(aeolusbot) << std::endl;
			return false; // not enough resources to afford this unit.
		}

		auto supplyCost = mediator.GetUnitSupplyCost(aeolusbot, m_to_train);
		if (aeolusbot.Observation()->GetFoodCap() - aeolusbot.Observation()->GetFoodUsed() < supplyCost)
		{
			return false;
		}

		for (const auto& structure : canTrain)
		{
			{
				// found available structure
			    std::cout << "Issueing training command to " << ::sc2::UnitTypeToName(structure->unit_type)
					<< " to train " << ::sc2::UnitTypeToName(m_to_train) << std::endl;
				aeolusbot.Actions()->UnitCommand(structure,
					ManagerMediator::getInstance().GetCreationAbility(aeolusbot, m_to_train));
				m_started = true;
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

	bool UnitBuildOrderStep::isDone(AeolusBot& aeolusbot)
	{
		auto trainedFrom = utils::_isTrainedFrom(m_to_train);
		if (!trainedFrom.has_value()) return true;

		::sc2::ABILITY_ID creationAbility = ManagerMediator::getInstance().GetCreationAbility(aeolusbot, m_to_train);
		for (const auto& structure : ManagerMediator::getInstance().GetAllOwnStructures(aeolusbot))
		{
			for (const auto& order : structure->orders)
			{
				if (order.ability_id == creationAbility) return true;
			}
		}
		return false;
	}

	bool UnitBuildOrderStep::started()
	{
		return m_started;
	}
}