#pragma once

#include <memory>
#include <vector>

#include "manager.h"
#include "manager_mediator.h"
#include "unit_role_manager.h"
#include "path_manager.h"
#include "resource_manager.h"
#include "neutral_unit_manager.h"

namespace Aeolus
{
	class AeolusBot;
	class Hub
	{
		// main class for the population and update of 
		// individual managers
	public:
		Hub(AeolusBot& aeolusbot)
		{

			// Create managers list for transfering ownership to ManagerMediator
			// In the order of dependencies!
			std::vector<std::unique_ptr<Manager>> managers;
			managers.push_back(std::make_unique<UnitRoleManager>());
			managers.push_back(std::make_unique<ResourceManager>());
			managers.push_back(std::make_unique<NeutralUnitManager>(aeolusbot));
			managers.push_back(std::make_unique<PathManager>(aeolusbot));

			// Store raw pointers before transferring ownership
			m_unit_role_manager_ref = static_cast<UnitRoleManager*>(managers[0].get());
			m_resource_manager_ref = static_cast<ResourceManager*>(managers[1].get());
			m_neutral_unit_manager_ref = static_cast<NeutralUnitManager*>(managers[2].get());
			m_path_manager_ref = static_cast<PathManager*>(managers[3].get());

			m_managers.push_back(m_unit_role_manager_ref);
			m_managers.push_back(m_resource_manager_ref);
			m_managers.push_back(m_neutral_unit_manager_ref);
			m_managers.push_back(m_path_manager_ref);

			ManagerMediator::getInstance().AddManagers(managers);

			_initializeManagers();
		}

		void UpdateManagers()
		{
			for (Manager* manager : m_managers)
			{
				if (manager)
				{
					manager->update(0);
				}
			}
		}

		void _initializeManagers()
		{
			UpdateManagers();
		}

	private:
		// Unique pointers for ownership
		std::vector<Manager*> m_managers;

		UnitRoleManager* m_unit_role_manager_ref;
		ResourceManager* m_resource_manager_ref;
		PathManager* m_path_manager_ref;
		NeutralUnitManager* m_neutral_unit_manager_ref;
	};
}
