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
#include "placement_manager.h"
#include "building_manager.h"

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
			managers.push_back(std::make_unique<PlacementManager>(aeolusbot));
			managers.push_back(std::make_unique<BuildingManager>(aeolusbot));

			// Store raw pointers before transferring ownership
			m_unit_filter_manager_ref = static_cast<UnitFilterManager*>(managers[0].get());
			m_unit_role_manager_ref = static_cast<UnitRoleManager*>(managers[1].get());
			m_resource_manager_ref = static_cast<ResourceManager*>(managers[2].get());
			m_path_manager_ref = static_cast<PathManager*>(managers[3].get());
			m_unit_property_manager_ref = static_cast<UnitPropertyManager*>(managers[4].get());
			m_defense_manager_ref = static_cast<DefenseManager*>(managers[5].get());
			m_placement_manager_ref = static_cast<PlacementManager*>(managers[6].get());
			m_building_manager_ref = static_cast<BuildingManager*>(managers[7].get());

			m_managers.push_back(m_unit_filter_manager_ref);
			m_managers.push_back(m_unit_role_manager_ref);
			m_managers.push_back(m_resource_manager_ref);
			m_managers.push_back(m_path_manager_ref);
			m_managers.push_back(m_unit_property_manager_ref);
			m_managers.push_back(m_defense_manager_ref);
			m_managers.push_back(m_placement_manager_ref);
			m_managers.push_back(m_building_manager_ref);

			ManagerMediator::getInstance().AddManagers(managers);

			_initializeManagers();
		}

		void UpdateManagers(int iter)
		{
			for (Manager* manager : m_managers)
			{
				if (!manager)
					continue;

				// Record the time before calling update()
				// auto start_time = std::chrono::high_resolution_clock::now();

				manager->update(iter);

				// Record the time after update()
				// auto end_time = std::chrono::high_resolution_clock::now();

				// Calculate elapsed time in milliseconds
				// auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

				// Print the manager name and how long update() took
				//if (iter % 100 == 0) std::cout << manager->GetName() << " took " << elapsed_ms << " ms to update." << std::endl;

				// std::cout << manager->GetName() << " updated. " << std::endl;
			}
		}

		void OnUnitDestroyed(const ::sc2::Unit* unit)
		{
			m_unit_role_manager_ref->ClearRole(unit);
			m_resource_manager_ref->OnUnitDestroyed(unit);
		}

		void _initializeManagers()
		{
			UpdateManagers(0);
		}

		void Initialize()
		{
			m_placement_manager_ref->Initialize();
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
		PlacementManager* m_placement_manager_ref;
		BuildingManager* m_building_manager_ref;
	};
}
