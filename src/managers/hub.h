#pragma once

#include <memory>
#include <vector>

#include "manager.h"
#include "manager_mediator.h"
#include "unit_role_manager.h"
#include "path_manager.h"
#include "resource_manager.h"
#include "unit_filter_manager.h"
#include "unit_property_manager.h"
#include "defense_manager.h"

namespace Aeolus
{
	class AeolusBot;
	class Hub
	{
		// main class for the population and update of 
		// individual managers
	public:
		Hub() = default;

		Hub(AeolusBot& aeolusbot)
		{

			// Create managers list for transfering ownership to ManagerMediator
			// In the order of dependencies!
			std::vector<std::unique_ptr<Manager>> managers;
			managers.push_back(std::make_unique<UnitFilterManager>(aeolusbot));
			managers.push_back(std::make_unique<UnitRoleManager>());
			managers.push_back(std::make_unique<ResourceManager>(aeolusbot));
			managers.push_back(std::make_unique<PathManager>(aeolusbot));
			managers.push_back(std::make_unique<UnitPropertyManager>(aeolusbot));
			managers.push_back(std::make_unique<DefenseManager>(aeolusbot));

			// Store raw pointers before transferring ownership
			m_unit_filter_manager_ref = static_cast<UnitFilterManager*>(managers[0].get());
			m_unit_role_manager_ref = static_cast<UnitRoleManager*>(managers[1].get());
			m_resource_manager_ref = static_cast<ResourceManager*>(managers[2].get());
			m_path_manager_ref = static_cast<PathManager*>(managers[3].get());
			m_unit_property_manager_ref = static_cast<UnitPropertyManager*>(managers[4].get());
			m_defense_manager_ref = static_cast<DefenseManager*>(managers[5].get());

			m_managers.push_back(m_unit_filter_manager_ref);
			m_managers.push_back(m_unit_role_manager_ref);
			m_managers.push_back(m_resource_manager_ref);
			m_managers.push_back(m_path_manager_ref);
			m_managers.push_back(m_unit_property_manager_ref);
			m_managers.push_back(m_defense_manager_ref);

			ManagerMediator::getInstance().AddManagers(managers);

			_initializeManagers();
		}

		void UpdateManagers(int iter)
		{
			for (Manager* manager : m_managers)
			{
				if (manager)
				{
					manager->update(iter);
				}
			}
		}

		void OnUnitDestroyed(const ::sc2::Unit* unit)
		{
			m_unit_role_manager_ref->ClearRole(unit);
			m_resource_manager_ref->ClearAssignment(unit);
		}

		void _initializeManagers()
		{
			UpdateManagers(0);
		}

	private:
		// Unique pointers for ownership
		std::vector<Manager*> m_managers;

		UnitRoleManager* m_unit_role_manager_ref;
		ResourceManager* m_resource_manager_ref;
		PathManager* m_path_manager_ref;
		UnitFilterManager* m_unit_filter_manager_ref;
		UnitPropertyManager* m_unit_property_manager_ref;
		DefenseManager* m_defense_manager_ref;
	};
}
