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

	double Grid::GetValue(int x, int y)
	{
		if (!IsCellValid(x, y))
		{
			return std::numeric_limits<double>::infinity();
		}

		return m_grid(y, x);
	}

	void Grid::SetValue(int x, int y, double value)
	{
		if (!IsCellValid(x, y))
		{
			return;
		}

		m_grid(y, x) = value;
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

	::sc2::Point2D Grid::FindFurthestSafeSpotTowards(::sc2::Point2D position, ::sc2::Point2D target, const double& radius)
	{
		// get all the points within the radius
		std::vector<std::pair<int, int>> candidates = _drawCircle(position.x, position.y, radius);

		::sc2::Point2D best_position = position;
		double max_dot_product = -std::numeric_limits<double>::infinity();
		double pos_to_target_sq = ::sc2::DistanceSquared2D(position, target);

		double target_vec_x = target.x - position.x;
		double target_vec_y = target.y - position.y;

		for (const auto& cell : candidates)
		{
			int x = cell.first;
			int y = cell.second;

			::sc2::Point2D candidate(x, y);

			if (!IsPositionSafe(candidate)) continue;

			double distance_to_target_sq =
				(x - target.x) * (x - target.x) +
				(y - target.y) * (y - target.y);

			if (distance_to_target_sq > pos_to_target_sq) continue;

			double move_vec_x = x - position.x;
			double move_vec_y = y - position.y;

			double dot_product =
				move_vec_x * target_vec_x +
				move_vec_y * target_vec_y;

			if (dot_product > max_dot_product)
			{
				max_dot_product = dot_product;
				best_position = candidate;
			}
		}
		return best_position;
	}

	::sc2::Point2D Grid::FindClosestSafeSpotTowards(::sc2::Point2D position, ::sc2::Point2D target, const double& radius)
	{
		std::vector<std::pair<int, int>> candidates =
			_drawCircle(position.x, position.y, radius);

		::sc2::Point2D best_safe_position = position;
		::sc2::Point2D safest_position = position;

		double best_safe_score = std::numeric_limits<double>::infinity();
		double best_safety_score = std::numeric_limits<double>::infinity();

		bool found_safe = false;

		constexpr double retreat_target_bias = 0.75;

		for (const auto& cell : candidates)
		{
			int x = cell.first;
			int y = cell.second;

			::sc2::Point2D candidate(
				static_cast<float>(x),
				static_cast<float>(y));

			double distance_from_unit =
				::sc2::Distance2D(position, candidate);

			double distance_to_target =
				::sc2::Distance2D(candidate, target);

			double target_biased_score =
				distance_from_unit +
				retreat_target_bias * distance_to_target;

			if (IsPositionSafe(candidate))
			{
				found_safe = true;

				if (target_biased_score < best_safe_score)
				{
					best_safe_score = target_biased_score;
					best_safe_position = candidate;
				}
			}

			double safety_score = m_grid(y, x);

			if (safety_score < best_safety_score)
			{
				best_safety_score = safety_score;
				safest_position = candidate;
			}
		}

		return found_safe ? best_safe_position : safest_position;
	}

	bool Grid::isSpotSaferThan(::sc2::Point2D posA, ::sc2::Point2D posB)
	{
		if (!IsPositionValid(posA)) return false;
		if (!IsPositionValid(posB)) return true;

		return m_grid(static_cast<int>(posA.y), static_cast<int>(posA.x)) <=
			m_grid(static_cast<int>(posB.y), static_cast<int>(posB.x));
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

		if (!IsPositionValid(position))
		{
			return false;
		}

		int x = static_cast<int>(position.x);
		int y = static_cast<int>(position.y);

		return m_grid(y, x) <= weight_safety_limit;
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
					
					if (IsCellValid(gridX, gridY))
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

			if (!IsCellValid(x, y))
			{
				continue;
			}

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

	void Grid::SetPlayableBounds(::sc2::Point2D playable_min, ::sc2::Point2D playable_max)
	{
		m_playable_min_x = static_cast<int>(std::floor(playable_min.x));
		m_playable_min_y = static_cast<int>(std::floor(playable_min.y));

		m_playable_max_x = static_cast<int>(std::ceil(playable_max.x));
		m_playable_max_y = static_cast<int>(std::ceil(playable_max.y));

		m_has_playable_bounds = true;
	}

	bool Grid::IsPositionValid(::sc2::Point2D position) const
	{
		int x = static_cast<int>(position.x);
		int y = static_cast<int>(position.y);

		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
		{
			return false;
		}

		if (!m_has_playable_bounds)
		{
			return true;
		}

		return x >= m_playable_min_x &&
			x < m_playable_max_x &&
			y >= m_playable_min_y &&
			y < m_playable_max_y;
	}

	bool Grid::IsCellValid(int x, int y) const
	{
		return IsPositionValid(::sc2::Point2D(
			static_cast<float>(x),
			static_cast<float>(y)));
	}
}