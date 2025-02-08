#pragma once

#include <string>

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
}