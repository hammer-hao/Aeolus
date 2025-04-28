#pragma once

#include "manager.h"
#include <sc2api/sc2_unit.h>
#include <tuple>
#include <any>

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief A manager for "door" units for protoss. Responsible for selecting a door unit
	* and issueing the hold position command when there are threats near the "door"
	*/
	class DoorManager : public Manager
	{
	public:
		DoorManager(AeolusBot& aeolusbot) : m_bot(aeolusbot) 
		{
			m_doorPos = { 0.0f, 0.0f };
			m_doorUnit = nullptr;
			m_enabled = false;
		}

		std::string_view GetName() const override {
			static const std::string name = "DoorManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		void update(int iteration) override;

	private:
		bool m_enabled;

		AeolusBot& m_bot;

		std::pair<float, float> m_doorPos;

		const ::sc2::Unit* m_doorUnit;

		void enable();

		void setDoorPos(std::pair<float, float> doorPos);

		void catchNewDoorUnit();

		void onUnitDestroyed(const ::sc2::Unit* unit);

		void closeDoor();

		void openDoor();
	};
}
