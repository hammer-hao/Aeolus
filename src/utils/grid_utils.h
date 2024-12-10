#pragma once

#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_map_info.h"

namespace Aeolus
{
	class Grid;
	namespace utils
	{
		void SetDestructableStatus(Grid& grid, const ::sc2::Unit* destructable, int status);

		::sc2::ImageData To8BPPImageData(Grid& grid, int color);

		::sc2::ImageData To1BPPImageData(Grid& grid);
	}
}