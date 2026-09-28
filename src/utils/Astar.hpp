#pragma once

#include "Eigen/Dense"
#include <sc2api/sc2_common.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

namespace Aeolus
{
    // Contract: matrix(row, col) is the cost of entering the unit-square cell
    // [col, col+1) x [row, row+1). Finite positive costs are traversable;
    // nonpositive and nonfinite costs are blocked. Encode ALL movement blockers,
    // including cells outside playable bounds, before calling this function.
    // This is a point-agent grid search: unit radius/clearance is an input-grid
    // responsibility, not something this function infers from SC2 observations.
    inline const double kAStarDiagonalCost = std::sqrt(2.0);

    struct Node
    {
        int row;
        int col;
        double gCost;
        double fCost;
    };

    struct AStarWorkspace
    {
        std::vector<double> gCost;
        std::vector<int> cameFrom;

        void Reset(std::size_t totalCells, double infinity)
        {
            if (gCost.size() != totalCells)
                gCost.resize(totalCells);

            if (cameFrom.size() != totalCells)
                cameFrom.resize(totalCells);

            std::fill(gCost.begin(), gCost.end(), infinity);
            std::fill(cameFrom.begin(), cameFrom.end(), -1);
        }
    };

    struct CompareNode
    {
        bool operator()(const Node& a, const Node& b) const
        {
            return a.fCost > b.fCost;
        }
    };

    // Octile distance for this exact eight-neighbor movement model.
    inline double heuristic(int y1, int x1, int y2, int x2)
    {
        const int dx = std::abs(x1 - x2);
        const int dy = std::abs(y1 - y2);
        return static_cast<double>(std::max(dx, dy)) +
            (kAStarDiagonalCost - 1.0) * std::min(dx, dy);
    }

    inline bool isValid(const Eigen::MatrixXd& grid, int y, int x)
    {
        if (y < 0 || y >= grid.rows() || x < 0 || x >= grid.cols())
            return false;
        const double cost = grid(y, x);
        return std::isfinite(cost) && cost > 0.0;
    }

    inline std::vector<sc2::Point2D> reconstructPath(
        const std::vector<int>& cameFrom,
        int goalRow,
        int goalCol,
        int cols)
    {
        std::vector<sc2::Point2D> path;

        int current = goalRow * cols + goalCol;

        while (current != -1)
        {
            const int row = current / cols;
            const int col = current % cols;

            path.emplace_back(
                static_cast<float>(col) + 0.5f,
                static_cast<float>(row) + 0.5f);

            current = cameFrom[current];
        }

        std::reverse(path.begin(), path.end());
        return path;
    }

    // Conservative smoothing: only merge consecutive steps in the SAME
    // direction. It preserves the exact grid route, its turns, and its costs.
    // Both endpoints, including the start, remain in the result.
    // sensitivity <= 1 leaves the path unchanged, including zero/negative input.
    inline std::vector<sc2::Point2D> smoothPath(
        const std::vector<sc2::Point2D>& path, int sensitivity)
    {
        if (path.size() <= 2 || sensitivity <= 1)
            return path;

        std::vector<sc2::Point2D> result;
        result.push_back(path.front());
        std::size_t begin = 0;
        while (begin + 1 < path.size())
        {
            std::size_t end = begin + 1;
            const float dx = path[end].x - path[begin].x;
            const float dy = path[end].y - path[begin].y;
            while (end + 1 < path.size() &&
                end - begin < static_cast<std::size_t>(sensitivity) &&
                path[end + 1].x - path[end].x == dx &&
                path[end + 1].y - path[end].y == dy)
            {
                ++end;
            }
            result.push_back(path[end]);
            begin = end;
        }
        return result;
    }

    inline std::vector<sc2::Point2D> AStarPathFind(
        sc2::Point2D start,
        sc2::Point2D goal,
        const Eigen::MatrixXd& grid,
        AStarWorkspace& workspace,
        bool smoothing = false,
        int sensitivity = 5,
        double cached_min_weight = -1.0)
    {
        const auto inBounds = [&grid](const sc2::Point2D& p)
            {
                return std::isfinite(p.x) &&
                    std::isfinite(p.y) &&
                    p.x >= 0.0f &&
                    p.y >= 0.0f &&
                    static_cast<double>(p.x) < grid.cols() &&
                    static_cast<double>(p.y) < grid.rows();
            };

        if (grid.rows() <= 0 ||
            grid.cols() <= 0 ||
            grid.rows() > std::numeric_limits<int>::max() ||
            grid.cols() > std::numeric_limits<int>::max() ||
            !inBounds(start) ||
            !inBounds(goal))
        {
            return {};
        }

        const int start_x = static_cast<int>(std::floor(start.x));
        const int start_y = static_cast<int>(std::floor(start.y));
        const int goal_x = static_cast<int>(std::floor(goal.x));
        const int goal_y = static_cast<int>(std::floor(goal.y));

        if (!isValid(grid, start_y, start_x) ||
            !isValid(grid, goal_y, goal_x))
        {
            return {};
        }

        const int rows = static_cast<int>(grid.rows());
        const int cols = static_cast<int>(grid.cols());

        const double infinity =
            std::numeric_limits<double>::infinity();

        /*
         * If PathManager supplies the minimum grid weight, we avoid
         * scanning the entire map for every A* request.
         *
         * Keep the fallback for other callers.
         */
        double minWeight = cached_min_weight;

        if (!(minWeight > 0.0) || !std::isfinite(minWeight))
        {
            minWeight = infinity;

            for (int y = 0; y < rows; ++y)
            {
                for (int x = 0; x < cols; ++x)
                {
                    if (isValid(grid, y, x))
                    {
                        minWeight =
                            std::min(minWeight, grid(y, x));
                    }
                }
            }

            if (!std::isfinite(minWeight))
                return {};
        }

        /*
         * Contiguous gCost buffer.
         *
         * vector<vector<double>> performs rows+1 allocations and has
         * worse cache locality. This performs one allocation.
         */
        const std::size_t totalCells =
            static_cast<std::size_t>(rows) *
            static_cast<std::size_t>(cols);

        const auto index = [cols](int row, int col) -> std::size_t
            {
                return static_cast<std::size_t>(row) *
                    static_cast<std::size_t>(cols) +
                    static_cast<std::size_t>(col);
            };

        workspace.Reset(totalCells, infinity);

        auto& gCost = workspace.gCost;
        auto& cameFrom = workspace.cameFrom;

        /*
         * Reserve some storage for the priority queue so small/medium
         * searches don't repeatedly grow its backing vector.
         */
        std::vector<Node> heapStorage;
        heapStorage.reserve(512);

        std::priority_queue<
            Node,
            std::vector<Node>,
            CompareNode>
            openSet(
                CompareNode{},
                std::move(heapStorage));

        gCost[index(start_y, start_x)] = 0.0;

        openSet.push({
            start_y,
            start_x,
            0.0,
            minWeight *
                heuristic(
                    start_y,
                    start_x,
                    goal_y,
                    goal_x)
            });

        constexpr std::array<std::pair<int, int>, 8> directions = { {
            {-1,  0},
            { 1,  0},
            { 0, -1},
            { 0,  1},
            {-1, -1},
            { 1,  1},
            {-1,  1},
            { 1, -1}
        } };

        while (!openSet.empty())
        {
            const Node current = openSet.top();
            openSet.pop();

            const std::size_t currentIndex =
                index(current.row, current.col);

            // Ignore stale priority-queue entries.
            if (current.gCost > gCost[currentIndex])
                continue;

            if (current.row == goal_y &&
                current.col == goal_x)
            {
                auto path =
                    reconstructPath(
                        cameFrom,
                        goal_y,
                        goal_x,
                        cols);

                return smoothing
                    ? smoothPath(path, sensitivity)
                    : path;
            }

            for (const auto& direction : directions)
            {
                const int dr = direction.first;
                const int dc = direction.second;

                const int nr = current.row + dr;
                const int nc = current.col + dc;

                if (!isValid(grid, nr, nc))
                    continue;

                const bool diagonal =
                    dr != 0 && dc != 0;

                // Prevent diagonal corner cutting.
                if (diagonal &&
                    (!isValid(
                        grid,
                        current.row + dr,
                        current.col) ||
                        !isValid(
                            grid,
                            current.row,
                            current.col + dc)))
                {
                    continue;
                }

                const std::size_t neighborIndex =
                    index(nr, nc);

                const double movementCost =
                    diagonal
                    ? kAStarDiagonalCost
                    : 1.0;

                const double stepCost =
                    grid(nr, nc) * movementCost;

                const double tentativeG =
                    current.gCost + stepCost;

                if (tentativeG >= gCost[neighborIndex])
                    continue;

                gCost[neighborIndex] = tentativeG;

                cameFrom[neighborIndex] =
                    static_cast<int>(currentIndex);

                const double h =
                    minWeight *
                    heuristic(
                        nr,
                        nc,
                        goal_y,
                        goal_x);

                openSet.push({
                    nr,
                    nc,
                    tentativeG,
                    tentativeG + h
                    });
            }
        }

        return {};
    }
}