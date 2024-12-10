#include "grid_utils.h"
#include "../constants.h"
#include "../pathing/grid.h"

namespace Aeolus
{
	namespace utils
	{
		void SetDestructableStatus(Grid& grid, const ::sc2::Unit* destructable, int status)
		{
			/*
			Set destructable positions to status, modifies the grid in place
			grid: grid to be modified
			destructable: destructable to be registered
			status: status to set the position of destructable in the grid
			*/

			int x = static_cast<int>(destructable->pos.x);
			int y = static_cast<int>(destructable->pos.y);
			int w = 0, h = 0;

			std::cout << "processing destructable of unit type " << destructable->unit_type << std::endl;

			if (constants::DESTRUCTABLE2X2_IDS.find(destructable->unit_type) != constants::DESTRUCTABLE2X2_IDS.end())
			{
				w = 2;
				h = 2;
			}
			else if (constants::DESTRUCTABLE2X4_IDS.find(destructable->unit_type) != constants::DESTRUCTABLE2X4_IDS.end())
			{
				w = 2;
				h = 4;
			}
			else if (constants::DESTRUCTABLE4X2_IDS.find(destructable->unit_type) != constants::DESTRUCTABLE4X2_IDS.end())
			{
				w = 4;
				h = 2;
			}
			else if (constants::DESTRUCTABLE4X4_IDS.find(destructable->unit_type) != constants::DESTRUCTABLE4X4_IDS.end())
			{
				w = 4;
				h = 4;
			}
			else if (constants::DESTRUCTABLE2X6_IDS.find(destructable->unit_type) != constants::DESTRUCTABLE2X6_IDS.end())
			{
				w = 2;
				h = 6;
			}
			else if (constants::DESTRUCTABLE6X2_IDS.find(destructable->unit_type) != constants::DESTRUCTABLE6X2_IDS.end())
			{
				w = 6;
				h = 2;
			}
			else if (constants::DESTRUCTABLE6X6_IDS.find(destructable->unit_type) != constants::DESTRUCTABLE6X6_IDS.end())
			{
				w = 6;
				h = 6;
			}
			else if (constants::DESTRUCTABLE12X4_IDS.find(destructable->unit_type) != constants::DESTRUCTABLE12X4_IDS.end())
			{
				w = 12;
				h = 4;
			}
			else if (constants::DESTRUCTABLE4X12_IDS.find(destructable->unit_type) != constants::DESTRUCTABLE4X12_IDS.end())
			{
				w = 4;
				h = 12;
			}

			// implementing change of status logic
			if (w > 0 && h > 0)
			{
				std::cout << "modified destructable of " << w << " by " << h << std::endl;
				x = x - w / 2;
				y = y - h / 2;
				if (x >= 0 && y >= 0 && x + w <= grid.GetGrid().rows() && y + h <= grid.GetGrid().cols())
				{
					grid.SetBlockValue(x, y, w, h, status);
				}
				else {
					std::cerr << "Warning: Attempted to modify grid out of bounds.\n";
				}
				return;
			}
			else if (constants::DESTRUCTABLEBLUR_IDS.find(destructable->unit_type) != constants::DESTRUCTABLEBLUR_IDS.end())
			{
				int x_ref = static_cast<int>(destructable->pos.x - 5);
				int y_pos = static_cast<int>(destructable->pos.y);
				grid.SetBlockValue(x_ref + 6, y_pos + 4, 2, 1, status);
				grid.SetBlockValue(x_ref + 5, y_pos + 3, 4, 1, status);
				grid.SetBlockValue(x_ref + 4, y_pos + 2, 6, 1, status);
				grid.SetBlockValue(x_ref + 3, y_pos + 1, 7, 1, status);
				grid.SetBlockValue(x_ref + 2, y_pos + 0, 7, 1, status);
				grid.SetBlockValue(x_ref + 1, y_pos - 1, 7, 1, status);
				grid.SetBlockValue(x_ref + 0, y_pos - 2, 7, 1, status);
				grid.SetBlockValue(x_ref + 0, y_pos - 3, 6, 1, status);
				grid.SetBlockValue(x_ref + 1, y_pos - 4, 4, 1, status);
				grid.SetBlockValue(x_ref + 2, y_pos - 5, 2, 1, status);
			}
			else if (constants::DESTRUCTABLEULBR_IDS.find(destructable->unit_type) != constants::DESTRUCTABLEULBR_IDS.end())
			{
				int x_ref = static_cast<int>(destructable->pos.x - 5);
				int y_pos = static_cast<int>(destructable->pos.y);
				grid.SetBlockValue(x_ref + 6, y_pos - 5, 2, 1, status);
				grid.SetBlockValue(x_ref + 5, y_pos - 4, 4, 1, status);
				grid.SetBlockValue(x_ref + 4, y_pos - 3, 6, 1, status);
				grid.SetBlockValue(x_ref + 3, y_pos - 2, 7, 1, status);
				grid.SetBlockValue(x_ref + 2, y_pos - 1, 7, 1, status);
				grid.SetBlockValue(x_ref + 1, y_pos + 0, 7, 1, status);
				grid.SetBlockValue(x_ref + 0, y_pos + 1, 7, 1, status);
				grid.SetBlockValue(x_ref + 0, y_pos + 2, 6, 1, status);
				grid.SetBlockValue(x_ref + 1, y_pos + 3, 4, 1, status);
				grid.SetBlockValue(x_ref + 2, y_pos + 4, 2, 1, status);
			}
		}

		::sc2::ImageData To8BPPImageData(Grid& grid, int color)
		{
			// 1 = Green, 2 = Red, 3 = Blue, 4 = Yellow, 5 = Cyan, other = white
			::sc2::ImageData image_data;
			image_data.width = grid.GetWidth();
			image_data.height = grid.GetHeight();
			image_data.bits_per_pixel = 8;
			image_data.data.resize(image_data.width * image_data.height);

			for (int y = 0; y < grid.GetHeight(); ++y)
			{
				for (int x = 0; x < grid.GetWidth(); ++x)
				{
					double value = grid.GetValue(x, y);
					uint8_t pixel_value = static_cast<uint8_t>(value * color);

					// Store in ImageData's flat data array
					image_data.data[y * image_data.width + x] = pixel_value;
				}
			}
			return image_data;
		}
	}
}