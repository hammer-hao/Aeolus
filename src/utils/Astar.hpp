#pragma once

#include "../pathing/grid.h"
#include <sc2api/sc2_common.h>
#include <cmath>
#include <queue>
#include <limits>

namespace Aeolus
{
	struct Node
	{
		int row;
		int col;
		double gCost; // cost from the start to this node
		double fCost; // gCost + heuristic
	};

	struct CompareNode 
	{
		bool operator()(const Node& a, const Node& b) const
		{
			return a.fCost > b.fCost;
		}
	};

	/**
	 * @brief Compute heuristic cost from current node to goal.
	 */
	inline double heuristic(int y1, int x1, int y2, int x2)
	{
		// euclidean distance
		double dx = static_cast<double>(x1 - x2);
		double dy = static_cast<double>(y1 - y2);
		return std::sqrt(dx * dx + dy * dy);
	}

	/**
	 * @brief Check if (x, y) is valid (in range) and not blocked in the costMap.
	 * @param costMap Eigen::MatrixXd that holds cost for each cell.
	 * @param x Cow index.
	 * @param y Rol index.
	 */
	inline bool isValid(const Eigen::MatrixXd& costMap, int y, int x)
	{
		if (y < 0 || y >= costMap.rows()) return false;
		if (x < 0 || x >= costMap.cols()) return false;
		// If cost is infinite, treat as blocked.
		if (std::isinf(costMap(y, x))) return false;
		return true;
	}

	/**
	 * @brief Reconstruct the path by backtracking from goal to start.
	 * @param cameFrom 2D array of parent indices: cameFrom[row][col] = (parentRow, parentCol).
	 * @param goal
	 * @return Vector of (row, col) from start to goal.
	 */
	inline std::vector<::sc2::Point2D> reconstructPath(
		const std::vector<std::vector<std::pair<int, int>>>& cameFrom,
		int goalRow, int goalCol)
	{
		std::vector<::sc2::Point2D> path;
		int currentRow = goalRow;
		int currentCol = goalCol;

		while (cameFrom[currentRow][currentCol].first != -1 &&
			cameFrom[currentRow][currentCol].second != -1)
		{
			path.push_back(::sc2::Point2D(currentCol, currentRow));
			auto parent = cameFrom[currentRow][currentCol];
			currentRow = parent.first;
			currentCol = parent.second;
		}
		// Finally add the start node
		path.push_back(::sc2::Point2D(currentCol, currentRow));

		// Reverse the path so it's from start -> goal
		std::reverse(path.begin(), path.end());
		return path;
	}

	inline std::vector<sc2::Point2D> smoothPath(const std::vector<sc2::Point2D>& path, int sensitivity) {
		// If the path is empty or has only 1 point, there's nothing to skip or smooth
		if (path.size() <= 1) {
			return path;
		}

		// Complete path: in Python, we did "list(map(Point2, path))"
		// but here we already have sc2::Point2D, so just copy:
		std::vector<sc2::Point2D> complete_path(path);

		// skipped_path = complete_path[0:-1:sensitivity]
		std::vector<sc2::Point2D> skipped_path;
		// Loop up to the second-last element in steps of 'sensitivity'
		// so i goes 0, sensitivity, 2*sensitivity, ..., up to < (size - 1)
		for (int i = 0; i < static_cast<int>(complete_path.size()) - 1; i += sensitivity) {
			skipped_path.push_back(complete_path[i]);
		}

		// if skipped_path not empty, pop(0) in Python => erase the front
		if (!skipped_path.empty()) {
			skipped_path.erase(skipped_path.begin()); // Remove first element
		}

		// Append the very last node from the complete path
		skipped_path.push_back(complete_path.back());

		return skipped_path;
	}


	/*
	@brief Given a start point, a goal point, and a grid with enemy influence,
	find the shortest safe path from start point to the goal point and returns
	the next point to move to in that path.
	*/
	inline std::vector<::sc2::Point2D> AStarPathFind(::sc2::Point2D start, ::sc2::Point2D goal,
		const Eigen::MatrixXd& grid, bool smoothing = false, int sensitivity = 5)
	{
		int start_x = static_cast<int>(std::round(start.x));
		int start_y = static_cast<int>(std::round(start.y));
		int goal_x = static_cast<int>(std::round(goal.x));
		int goal_y = static_cast<int>(std::round(goal.y));

		// Sanity checks
		if (!isValid(grid, start_y, start_x)) {
			std::cerr << "Start is invalid or blocked.\n";
			return {};
		}
		if (!isValid(grid, goal_y, goal_x)) {
			std::cerr << "Goal is invalid or blocked.\n";
			return {};
		}

		int rows = grid.rows();
		int cols = grid.cols();

		// We store gCosts in a 2D array initialized to infinity.
		std::vector<std::vector<double>> gCost(rows, std::vector<double>(cols, std::numeric_limits<double>::infinity()));
		// We store the node’s parent for path reconstruction
		std::vector<std::vector<std::pair<int, int>>> cameFrom(rows, std::vector<std::pair<int, int>>(
			cols, std::make_pair(-1, -1)));

		// Min-heap for open set
		std::priority_queue<Node, std::vector<Node>, CompareNode> openSet;

		// Initialize the start node
		Node startNode;
		startNode.row = start_y;
		startNode.col = start_x;
		startNode.gCost = 0.0;
		startNode.fCost = heuristic(start_y, start_x, static_cast<int>(goal.y), static_cast<int>(goal.x));
		gCost[start_y][start_x] = 0.0;

		openSet.push(startNode);

		std::vector<std::pair<int, int>> directions = {
		{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, 1}, {-1, 1}, {1, -1}
		};

		while (!openSet.empty())
		{
			Node current = openSet.top();
			openSet.pop();

			// If we reached the goal, reconstruct path
			if (current.row == goal_y && current.col == goal_x) 
			{
				// Reconstruct and optionally smooth
				auto path = reconstructPath(cameFrom, static_cast<int>(goal_y), static_cast<int>(goal_x));
				if (path.size() > 1) {
					return smoothPath(path, sensitivity); // the first step beyond the start, adjusted for sensitivity
				}
				return path; // if path has only one point, start==goal
			}

			// If this node's gCost is already worse than what's recorded, skip
			if (current.gCost > gCost[current.row][current.col]) 
			{
				continue;
			}

			// Check neighbors
			for (auto& dir : directions)
			{
				int nr = current.row + dir.first;
				int nc = current.col + dir.second;

				if (!isValid(grid, nr, nc))
				{
					continue;
				}

				// The cost to move to neighbor depends on costMap + gCost
				double tentativeG = gCost[current.row][current.col] + grid(nr, nc);
				// If we found a better way to get to neighbor
				if (tentativeG < gCost[nr][nc]) {
					gCost[nr][nc] = tentativeG;
					cameFrom[nr][nc] = { current.row, current.col };

					Node neighbor;
					neighbor.row = nr;
					neighbor.col = nc;
					neighbor.gCost = tentativeG;
					neighbor.fCost = tentativeG +
						heuristic(nr, nc, goal_y, goal_x);
					openSet.push(neighbor);
				}
			}
		}

		// If we exit the loop, no path was found
		std::cerr << "No path found from start to goal.\n";
		return {};
	}
}