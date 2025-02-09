#pragma once

#include <string>
#include <optional>

namespace Aeolus
{
	enum class BuildOrderEnum
	{
		MACRO_STALKERS,
		STALKER_IMMORTAL,
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
		case (BuildOrderEnum::STALKER_IMMORTAL):
		{
			return "STALKER_IMMORTAL";
		}
		}
	}

	inline std::optional<BuildOrderEnum> stringToBuildOrder(const std::string& string)
	{
		if (string == "MACRO_STALKERS") return BuildOrderEnum::MACRO_STALKERS;
		if (string == "STALKER_IMMORTAL") return BuildOrderEnum::STALKER_IMMORTAL;
		std::cout << "Invalid build order name:" << string << std::endl;
		return std::nullopt;
	}
}