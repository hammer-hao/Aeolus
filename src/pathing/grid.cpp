#include "grid.h"
#include <vector>
#include <utility>
#include <cmath>

namespace Aeolus
{
	Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> Grid::GetGrid()
	{
		// Get the value in Eigen Matrix form
		return m_grid;
	}

	void Grid::SetValue(int x, int y, double value)
	{
		// Set the value of the grid at position (x, y)
		m_grid(y, x) = value;
	}

	double Grid::GetValue(int x, int y)
	{
		// get the value of the grid at positio (x, y)
		return m_grid(y, x);
	}

	/**
	 * @brief Sets a specific value for a rectangular block within the grid.
	 *
	 * This method updates a block of grid cells, defined by its starting
	 * coordinates and dimensions, to a constant value.
	 *
	 * @param x_start The starting x-coordinate of the block.
	 * @param y_start The starting y-coordinate of the block.
	 * @param width The width of the block.
	 * @param height The height of the block.
	 * @param val The value to assign to all cells within the block.
	 */
	void Grid::SetBlockValue(int x_start, int y_start, int width, int height, int val)
	{
		// Set the value of a block in grid
		m_grid.block(y_start, x_start, height, width).setConstant(val);
	}

	void Grid::FlipValues()
	{
		// Flip 0s to 1s and 1s to 0s in the entire grid
		m_grid = m_grid.unaryExpr([](int val) -> int {
			return (val == 0) ? 1 : (val == 1 ? 0 : val);
			});
	}

	void Grid::AddCost(double pos_x, double pos_y, double radius, double weight, bool safe, double initial_default_weight)
	{
		// Calculate the affected disk (set of grid cells)
		std::vector<std::pair<int, int>> disk = _drawCircle(pos_x, pos_y, radius);
		
		// Apply the disk to the grid
		_applyDiskToGrid(pos_x, pos_y, disk, weight, safe, initial_default_weight);
	}

	std::vector<std::pair<int, int>> Grid::_drawCircle(const double& pos_x, const double& pos_y, const double& radius) const
	{
		std::vector<std::pair<int, int>> disk;

		int centerX = static_cast<int>(pos_x);
		int centerY = static_cast<int>(pos_y);
		int r = static_cast<int>(std::ceil(radius));

		for (int y = -r; y <= r; ++y)
		{
			for (int x = -r; x <= r; ++x)
			{
				if (x * x + y * y < radius * radius)
				{
					int gridX = centerX + x;
					int gridY = centerY + y;
					
					if (gridX >= 0 && gridX <= m_width && gridY >= 0 && gridY <= m_height)
					{
						disk.emplace_back(gridX, gridY);
					}
				}
			}
		}

		return disk;
	}

	void Grid::_applyDiskToGrid(const double& pos_x, const double& pos_y,
		const std::vector<std::pair<int, int>>& disk, const double& weight,
		bool safe, const double& initial_default_Weight)
	{
		for (const auto& cell : disk)
		{
			int x = cell.first;
			int y = cell.second;

			if (x >= 0 && x < m_width && y >= 0 && y < m_height)
			{
				if (initial_default_Weight > 0 && m_grid(y, x) == 1) {
					m_grid(y, x) = 1 + initial_default_Weight;
				}

				m_grid(y, x) += weight;
				if (safe && m_grid(y, x) < 1)
				{
					std::cerr << "Warning: Value below 1. Setting to minimum (1).\n";
					m_grid(y, x) = 1;
				}
			}
		}
	}
}