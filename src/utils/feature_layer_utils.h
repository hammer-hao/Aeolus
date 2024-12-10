#pragma once

#include "sc2api/sc2_api.h"
#include <sc2renderer/sc2_renderer.h>
#include "sc2utils/sc2_manage_process.h"
#include "../constants.h"


namespace Aeolus
{
	namespace utils
	{
		void DrawPathingGrid(
			const ::sc2::ImageData& pathing_grid,
			int offset_x = 0,
			int offset_y = 0
		);
	}
}