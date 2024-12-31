#include "grid.h"
#include <vector>
#include <utility>
#include <cmath>

namespace Aeolus
{
	Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> Grid::GetGrid() const
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

	void Grid::AddCost(double pos_x, double pos_y, double radius, double weight, bool safe, double initial_default_weight)
	{
		// Calculate the affected disk (set of grid cells)
		std::vector<std::pair<int, int>> disk = _drawCircle(pos_x, pos_y, radius);
		
		// Apply the disk to the grid
		_applyDiskToGrid(pos_x, pos_y, disk, weight, safe, initial_default_weight);
	}

	void Grid::InitializeWeights(double default_weight)
	{
		m_grid = m_grid.unaryExpr([default_weight](double val) {
			return (val != 0.0) ? default_weight : std::numeric_limits<double>::infinity();
			});
		UpdateCache(); // sync the cached grid
	}

	::sc2::Point2D Grid::FindClosestSafeSpot(::sc2::Point2D position, const double& radius)
	{
		// get all the points within the radius
		std::vector<std::pair<int, int>> candidates = _drawCircle(position.x, position.y, radius);

		double min_cost = std::numeric_limits<double>::infinity();
		::sc2::Point2D best_position = position;
		double min_distance_sq = std::numeric_limits<double>::infinity();

		for (const auto& cell : candidates)
		{
			int x = cell.first;
			int y = cell.second;
			double cost = m_grid(y, x);
			double distance_sq = (x - position.x) * (x - position.x) + (y - position.y) * (y - position.y);
			if (cost < min_cost || cost == min_cost && distance_sq < min_distance_sq)
			{
				min_cost = cost;
				min_distance_sq = distance_sq;
				best_position = ::sc2::Point2D(x, y);
			}
		}
		return best_position;
	}

	/**
		 * @brief Check if the given position is considered safe on the grid.
		 *
		 * @param position_x The x-coordinate of the position to check.
		 * @param position_y The y-coordinate of the position to check.
		 * @param weight_safety_limit The maximum value the point can have on the grid to be considered safe.
		 * @return True if the position is considered safe, False otherwise.
		 */
	bool Grid::IsPositionSafe(::sc2::Point2D position, double weight_safety_limit) const 
	{
		/*
		// Ensure the position is within bounds
		if (position.x < 0 || position.x >= m_grid.cols() ||
			position.y < 0 || position.y >= m_grid.rows()) {
			throw std::out_of_range("Position is out of grid bounds.");
		}
		*/

		// Check if the value at the grid position is below the safety limit
		return m_grid(static_cast<int>(position.y), static_cast<int>(position.x)) <= weight_safety_limit;
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