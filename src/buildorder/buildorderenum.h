#pragma once

#include <string>
#include <optional>
#include <iostream>

namespace Aeolus
{
	enum class BuildOrderEnum
	{
		MACRO_STALKERS,
		STALKER_IMMORTAL,
		BLINK_STALKERS,
		MASS_ROBO,
		STALKER_STARGATE,
		FORGE_EXPAND,
		COUNT // helper for randomization, do not use!
	};

	inline std::string buildOrderToString(BuildOrderEnum build_order)
	{
		switch (build_order)
		{
		case (BuildOrderEnum::MACRO_STALKERS):
		{
			return "MACRO_STALKERS";
		}
		case (BuildOrderEnum::BLINK_STALKERS):
		{
			return "BLINK_STALKERS";
		}
		case (BuildOrderEnum::STALKER_IMMORTAL):
		{
			return "STALKER_IMMORTAL";
		}
		case (BuildOrderEnum::MASS_ROBO):
		{
			return "MASS_ROBO";
		}
		case (BuildOrderEnum::STALKER_STARGATE):
		{
			return "STALKER_STARGATE";
		}
		case (BuildOrderEnum::FORGE_EXPAND):
			return "FORGE_EXPAND";
		}
	}

	inline std::optional<BuildOrderEnum> stringToBuildOrder(const std::string& string)
	{
		if (string == "MACRO_STALKERS") return BuildOrderEnum::MACRO_STALKERS;
		if (string == "STALKER_IMMORTAL") return BuildOrderEnum::STALKER_IMMORTAL;
		if (string == "BLINK_STALKERS") return BuildOrderEnum::BLINK_STALKERS;
		if (string == "MASS_ROBO") return BuildOrderEnum::MASS_ROBO;
		if (string == "STALKER_STARGATE") return BuildOrderEnum::STALKER_STARGATE;
		if (string == "FORGE_EXPAND") return BuildOrderEnum::FORGE_EXPAND;
		std::cout << "Invalid build order name:" << string << std::endl;
		return std::nullopt;
	}
}