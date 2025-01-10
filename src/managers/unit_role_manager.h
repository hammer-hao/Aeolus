#pragma once
#include "manager.h"
#include "../constants.h"
#include <sc2api/sc2_unit.h>
#include <unordered_map>

namespace Aeolus {
	class AeolusBot;

	class UnitRoleManager : public Manager
	{
	public:
		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		std::string_view GetName() const override {
			static const std::string name = "UnitRoleManager";
			return name;
		}

		// member function to assign a role to a unit
		void AssignRole(const ::sc2::Unit* unit, constants::UnitRole unitrole);

		// to clear the role from a unit
		void ClearRole(const ::sc2::Unit* unit);

		// to assign an unassign unit an assumed role
		void CatchUnit(const ::sc2::Unit* unit);

		// member function to get a unit from a role
		::sc2::Units GetUnitsFromRole(constants::UnitRole) const;

		// Implement the update method.
		void update(int iteration) override {
			// std::cout << "Updating UnitRoleManager at iteration: " << iteration << std::endl;
		}

	private:
		std::unordered_map<constants::UnitRole, ::sc2::Units> unit_role_map_;
		std::unordered_set<const ::sc2::Unit*> m_assigned_units;
	};
}