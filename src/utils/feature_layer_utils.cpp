#ifdef BUILD_WITH_RENDERER

#include "feature_layer_utils.h"
#include <sc2renderer/sc2_renderer.h>

namespace Aeolus
{
	namespace utils
	{
		void DrawPathingGrid(
			const ::sc2::ImageData& pathing_grid,
			int offset_x,
			int offset_y
		)
		{
			int width{ pathing_grid.width };
			int height{ pathing_grid.height };
			if (pathing_grid.bits_per_pixel == 1)
			{
				::sc2::renderer::Matrix1BPP(pathing_grid.data.c_str(), width, height, 
				offset_x, offset_y, constants::PIXEL_DRAW_SIZE, constants::PIXEL_DRAW_SIZE);
			}
			else if (pathing_grid.bits_per_pixel == 8)
			{
				::sc2::renderer::Matrix8BPPPlayers(pathing_grid.data.c_str(), width, height,
				offset_x, offset_y, constants::PIXEL_DRAW_SIZE, constants::PIXEL_DRAW_SIZE);
			}
		}
	}
}

#endif