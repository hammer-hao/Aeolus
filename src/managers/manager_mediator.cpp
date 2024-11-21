#include "manager_mediator.h"
#include "resource_manager.h"
#include "unit_role_manager.h"
#include "path_manager.h"


#include <iostream>
#include <vector>
#include <typeinfo>

#include <sc2api/sc2_unit.h>

namespace Aeolus
{
    class Manager;

    void ManagerMediator::AddManagers(std::vector<std::unique_ptr<Manager>>& managers)
    {
        for (auto& manager : managers)
        {
            std::string manager_name{ typeid(*manager).name() };
            m_managers[manager_name] = std::move(manager);
        }
        managers.clear();
    }


    /*
     void ManagerMediator::Populate(AeolusBot& aeolusbot)
    {
        std::vector<std::unique_ptr<Manager>> managers;
        managers.push_back(std::make_unique<UnitRoleManager>());
        managers.push_back(std::make_unique<ResourceManager>());
        managers.push_back(std::make_unique<PathManager>(aeolusbot));
        AddManagers(managers);
    }   
    */

}