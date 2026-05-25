#include "config_manager.h"
#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include <fstream>
#include <sc2api/sc2_common.h>
#include <nlohman/json.hpp>

namespace Aeolus
{
	using json = nlohmann::json;

	ConfigManager::ConfigManager(AeolusBot& aeolusbot) : m_bot(aeolusbot)
	{
		std::ifstream f("data/config/contingency_plans.json");
		json contingency_plans = json::parse(f)["contingency_plans"];
        _loadContingencyPlans(contingency_plans);
	}

    std::any ConfigManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
    {
        switch (request)
        {
        case constants::ManagerRequestType::GET_CONTINGENCY_PLANS:
        {
            return _getContingencyPlans();
        }
        default:
            return 0;
        }
    }

    std::vector<ContingencyPlan> ConfigManager::_getContingencyPlans() {
        auto& mediator = ManagerMediator::getInstance();
        ::sc2::Race opponentRace = mediator.getOpponentRace(m_bot);

        switch (opponentRace)
        {
        case ::sc2::Race::Terran:
            return m_contingency_plans[::sc2::Race::Terran];
            break;
        case ::sc2::Race::Zerg:
            return m_contingency_plans[::sc2::Race::Zerg];
            break;
        case ::sc2::Race::Protoss:
            return m_contingency_plans[::sc2::Race::Protoss];
            break;
        case ::sc2::Race::Random:
            return _flattenContingencyPlans(m_contingency_plans);
            break;
        default:
            return _flattenContingencyPlans(m_contingency_plans);
            break;
        }
    }

    void ConfigManager::_loadContingencyPlans(const json& j)
    {
        const auto& plans_json = j["contingency_plans"];

        for (const auto& [race_name, race_plans_json]
            : plans_json.items())
        {
            ::sc2::Race race;

            if (race_name == "zerg")
            {
                race = ::sc2::Race::Zerg;
            }
            else if (race_name == "terran")
            {
                race = ::sc2::Race::Terran;
            }
            else if (race_name == "protoss")
            {
                race = ::sc2::Race::Protoss;
            }
            else
            {
                continue;
            }

            std::vector<Aeolus::ContingencyPlan> plans;

            for (const auto& plan_json : race_plans_json)
            {
                Aeolus::ContingencyPlan plan;

                plan.name =
                    plan_json["name"].get<std::string>();

                plan.move_out_supply =
                    plan_json["move_out_supply"].get<int>();

                plan.cannons_to_add =
                    plan_json["cannons_to_add"].get<int>();

                //
                // Parse scouting conditions
                //
                for (const auto& condition_json
                    : plan_json["conditions"])
                {
                    Aeolus::ScoutingCondition condition;

                    std::string unit_name =
                        condition_json["name"]
                        .get<std::string>();

                    condition.unitType =
                        _unitNameToType(unit_name);

                    condition.count =
                        condition_json["count"].get<int>();

                    condition.before_seconds =
                        condition_json["before_seconds"]
                        .get<int>();

                    condition.is_proxied =
                        condition_json["is_proxied"]
                        .get<bool>();

                    plan.conditions.push_back(condition);
                }

                //
                // Parse army composition
                //
                for (const auto& [unit_name, weight_json]
                    : plan_json["army_composition"].items())
                {
                    ::sc2::UNIT_TYPEID unit_type =
                        _unitNameToType(unit_name);

                    float weight =
                        weight_json.get<float>();

                    plan.army_composition[unit_type] =
                        weight;
                }

                plans.push_back(plan);
            }

            m_contingency_plans[race] = plans;
        }
    }

    ::sc2::UNIT_TYPEID ConfigManager::_unitNameToType(const std::string& name)
    {
        static const std::unordered_map<
            std::string,
            ::sc2::UNIT_TYPEID> unit_map =
        {
            {"SPAWNING_POOL", ::sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL},
            {"STALKER", ::sc2::UNIT_TYPEID::PROTOSS_STALKER},
        };

        auto it = unit_map.find(name);

        if (it == unit_map.end())
        {
            throw std::runtime_error(
                "Unknown unit type: " + name);
        }

        return it->second;
    }

    std::vector<ContingencyPlan> ConfigManager::_flattenContingencyPlans(
            const std::unordered_map<
            ::sc2::Race,
            std::vector<ContingencyPlan>>&plans)
    {
        std::vector<ContingencyPlan> result;

        for (const auto& [race, vec] : plans)
        {
            result.insert(
                result.end(),
                vec.begin(),
                vec.end());
        }

        return result;
    }
}