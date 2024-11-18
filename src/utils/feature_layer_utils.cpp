#include "feature_layer_utils.h"
#include <sc2renderer/sc2_renderer.h>

namespace Aeolus
{
	namespace utils
	{
		void DrawPathingGrid(
			::sc2::ImageData pathing_grid,
			int offset_x,
			int offset_y
		)
		{
			assert(pathing_grid.bits_per_pixel == 1);
			int width{ pathing_grid.width };
			int height{ pathing_grid.height };
			::sc2::renderer::Matrix1BPP(pathing_grid.data.c_str(), width, height, 
				offset_x, offset_y, constants::PIXEL_DRAW_SIZE, constants::PIXEL_DRAW_SIZE);
		}
	}
}