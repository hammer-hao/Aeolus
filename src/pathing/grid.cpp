#include "grid.h"

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
}