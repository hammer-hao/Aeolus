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

        if (!f.is_open())
        {
            throw std::runtime_error("Failed to open data/config/contingency_plans.json");
        }

        json config = json::parse(f);
        _loadContingencyPlans(config);
        
        std::ifstream f_natural_wall_placements("data/config/natural_wall_positions.json");
        if (!f_natural_wall_placements.is_open())
        {
            throw std::runtime_error("Failed to open data/config/natural_wall_positions.json");
        }
        json natural_wall_data = json::parse(f_natural_wall_placements);
        _loadNaturalWallPlacements(natural_wall_data);
	}

    std::any ConfigManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
    {
        switch (request)
        {
        case constants::ManagerRequestType::GET_CONTINGENCY_PLANS:
        {
            return _getContingencyPlans();
        }
        case constants::ManagerRequestType::GET_NATURAL_BUILDING_PLACEMENTS:
        {
            if (m_natual_pylon_position.has_value() &&
                m_natural_door_position.has_value() &&
                m_natural_three_by_three_positions)
            {
                std::vector<::sc2::Point2D> positions;
                positions.reserve(
                    2 + m_natural_three_by_three_positions->size());

                positions.push_back(*m_natual_pylon_position);

                positions.insert(
                    positions.end(),
                    m_natural_three_by_three_positions->begin(),
                    m_natural_three_by_three_positions->end());

                positions.push_back(*m_natural_door_position);

                return std::optional<std::vector<::sc2::Point2D>>{
                    std::move(positions)
                };
            }
            else 
            {
                return std::optional<std::vector<::sc2::Point2D>>{};
            }
        }
        default:
            return 0;
        }
    }

    void ConfigManager::update(int iteration)
    {
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

                plan.check_no_enemy_expansion =
                    plan_json.value("check_no_enemy_expansion", false);

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
        std::cout << "Parsing unit: [" << name << "]" << std::endl;
        static const std::unordered_map<
            std::string,
            ::sc2::UNIT_TYPEID> unit_map =
        {
            {"SPAWNING_POOL", ::sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL},
            {"STALKER", ::sc2::UNIT_TYPEID::PROTOSS_STALKER},
            {"ZEALOT", ::sc2::UNIT_TYPEID::PROTOSS_ZEALOT},
            {"ZERGLING", ::sc2::UNIT_TYPEID::ZERG_ZERGLING},
            {"GATEWAY", ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY},
            {"ADEPT", ::sc2::UNIT_TYPEID::PROTOSS_ADEPT},
            {"ROACH_WARREN", ::sc2::UNIT_TYPEID::ZERG_ROACHWARREN},
            {"PROBE", ::sc2::UNIT_TYPEID::PROTOSS_PROBE},
            {"SCV", ::sc2::UNIT_TYPEID::TERRAN_SCV},
            {"DRONE", ::sc2::UNIT_TYPEID::ZERG_DRONE},
            {"IMMORTAL", ::sc2::UNIT_TYPEID::PROTOSS_IMMORTAL},
            {"MARINE", ::sc2::UNIT_TYPEID::TERRAN_MARINE},
            {"BARRACKS", ::sc2::UNIT_TYPEID::TERRAN_BARRACKS}
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

    void ConfigManager::_loadNaturalWallPlacements(const json& placement_data)
    {
        auto* observation = m_bot.Observation();

        std::string map_name =
            observation->GetGameInfo().map_name;

        const auto& all_wall_positions =
            placement_data.at("natural_wall_positions");

        if (!all_wall_positions.contains(map_name))
        {
            std::cerr << "No natural wall placement data found for map: "
                << map_name << std::endl;
            return;
        }

        const auto& map_wall_positions =
            all_wall_positions.at(map_name);

        ::sc2::Point2D own_start =
            observation->GetStartLocation();

        auto parsePoint = [](const json& point_json)
            {
                return ::sc2::Point2D(
                    point_json.at(0).get<float>(),
                    point_json.at(1).get<float>());
            };

        const json* best_wall_json = nullptr;
        float best_distance = std::numeric_limits<float>::max();

        for (const auto& wall_json : map_wall_positions)
        {
            ::sc2::Point2D starting_point =
                parsePoint(wall_json.at("starting_point"));

            float distance =
                ::sc2::DistanceSquared2D(own_start, starting_point);

            if (distance < best_distance)
            {
                best_distance = distance;
                best_wall_json = &wall_json;
            }
        }

        if (best_wall_json == nullptr)
        {
            std::cerr << "Natural wall placement data exists for map "
                << map_name
                << " but contains no placements."
                << std::endl;
            return;
        }

        m_natual_pylon_position =
            parsePoint(best_wall_json->at("natural_pylon"));

        std::vector<::sc2::Point2D> three_by_three_positions;

        for (const auto& point_json :
            best_wall_json->at("natural_three_by_threes"))
        {
            three_by_three_positions.push_back(
                parsePoint(point_json));
        }

        m_natural_three_by_three_positions =
            three_by_three_positions;

        m_natural_door_position =
            parsePoint(best_wall_json->at("natural_door_unit"));
    }
}