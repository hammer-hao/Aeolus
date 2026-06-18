#include "hug_corner_towards.h"

#include "../../Aeolus.h"
#include "move.h"

#include <algorithm>
#include <cmath>

namespace Aeolus
{
	bool HugCornerTowards::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		if (unit == nullptr)
		{
			return false;
		}

		constexpr float targetReachedEps = 1.0f;

		if (::sc2::Distance2D(unit->pos, m_target) < targetReachedEps)
		{
			return false;
		}

		auto* observation = aeolusbot.Observation();

		const ::sc2::Point2D minPlayable = observation->GetGameInfo().playable_min;
		const ::sc2::Point2D maxPlayable = observation->GetGameInfo().playable_max;

		const float xMinRaw = minPlayable.x;
		const float xMaxRaw = maxPlayable.x;
		const float yMinRaw = minPlayable.y;
		const float yMaxRaw = maxPlayable.y;

		// Stay slightly inside the playable bounds instead of ordering exactly
		// onto the border, which can sometimes be awkward/pathing-hostile.
		constexpr float edgeInset = 0.1f;
		constexpr float edgeEps = 0.4f;
		constexpr float alignEps = 2.0f;

		const float xMin = xMinRaw + edgeInset;
		const float xMax = xMaxRaw - edgeInset;
		const float yMin = yMinRaw + edgeInset;
		const float yMax = yMaxRaw - edgeInset;

		const ::sc2::Point2D unitPos = unit->pos;

		const float distToLeft = std::abs(unitPos.x - xMin);
		const float distToRight = std::abs(unitPos.x - xMax);
		const float distToBottom = std::abs(unitPos.y - yMin);
		const float distToTop = std::abs(unitPos.y - yMax);

		const float distanceToNearestEdge = std::min({
			distToLeft,
			distToRight,
			distToBottom,
			distToTop
			});

		if (distanceToNearestEdge > m_maxDistanceFromEdge)
		{
			return false;
		}

		auto clampX = [&](float x)
			{
				return std::clamp(x, xMin, xMax);
			};

		auto clampY = [&](float y)
			{
				return std::clamp(y, yMin, yMax);
			};
		const ::sc2::Point2D clampedTarget{ clampX(m_target.x), clampY(m_target.y) };

		auto moveTo = [&](const ::sc2::Point2D& point)
			{
				Move move(point);
				return move.execute(aeolusbot, unit);
			};

		const float xMid = (xMin + xMax) * 0.5f;
		const float yMid = (yMin + yMax) * 0.5f;

		// Target-side corner.
		const bool targetIsLeft = m_target.x < xMid;
		const bool targetIsBottom = m_target.y < yMid;

		const float targetEdgeX = targetIsLeft ? xMin : xMax;
		const float targetEdgeY = targetIsBottom ? yMin : yMax;

		const float antiTargetEdgeX = targetIsLeft ? xMax : xMin;
		const float antiTargetEdgeY = targetIsBottom ? yMax : yMin;

		const bool onLeft = distToLeft < edgeEps;
		const bool onRight = distToRight < edgeEps;
		const bool onBottom = distToBottom < edgeEps;
		const bool onTop = distToTop < edgeEps;

		const bool onAnyEdge = onLeft || onRight || onBottom || onTop;

		const bool onTargetVerticalEdge =
			(targetIsLeft && onLeft) || (!targetIsLeft && onRight);

		const bool onTargetHorizontalEdge =
			(targetIsBottom && onBottom) || (!targetIsBottom && onTop);

		const bool onAntiTargetVerticalEdge =
			(targetIsLeft && onRight) || (!targetIsLeft && onLeft);

		const bool onAntiTargetHorizontalEdge =
			(targetIsBottom && onTop) || (!targetIsBottom && onBottom);

		const bool unitOnSameXSideAsTarget =
			targetIsLeft ? unitPos.x <= xMid : unitPos.x >= xMid;

		const bool unitOnSameYSideAsTarget =
			targetIsBottom ? unitPos.y <= yMid : unitPos.y >= yMid;

		const bool closelyAlignedWithTarget =
			std::abs(unitPos.x - m_target.x) < alignEps ||
			std::abs(unitPos.y - m_target.y) < alignEps;

		// Case 1:
		// Unit is not currently hugging any edge.
		if (!onAnyEdge)
		{
			// Exception:
			// If already on the target side of the map and almost lined up,
			// do not force an artificial edge path.
			if (unitOnSameXSideAsTarget && unitOnSameYSideAsTarget && closelyAlignedWithTarget)
			{
				return moveTo(clampedTarget);
			}

			// Move to the physically closest edge first.
			float bestDist = distToLeft;
			::sc2::Point2D closestEdgePoint{ xMin, clampY(unitPos.y) };

			if (distToRight < bestDist)
			{
				bestDist = distToRight;
				closestEdgePoint = { xMax, clampY(unitPos.y) };
			}

			if (distToBottom < bestDist)
			{
				bestDist = distToBottom;
				closestEdgePoint = { clampX(unitPos.x), yMin };
			}

			if (distToTop < bestDist)
			{
				closestEdgePoint = { clampX(unitPos.x), yMax };
			}

			return moveTo(closestEdgePoint);
		}

		// Case 2:
		// Unit is on the opposite vertical edge. Move along that edge toward
		// the corner whose y-value matches the target-side corner.
		if (onAntiTargetVerticalEdge && !onTargetHorizontalEdge)
		{
			return moveTo({ antiTargetEdgeX, targetEdgeY });
		}

		// Case 3:
		// Unit is on the opposite horizontal edge. Move along that edge toward
		// the corner whose x-value matches the target-side corner.
		if (onAntiTargetHorizontalEdge && !onTargetVerticalEdge)
		{
			return moveTo({ targetEdgeX, antiTargetEdgeY });
		}

		// Case 4:
		// Unit is already on a good vertical edge. Match target y first.
		if (onTargetVerticalEdge)
		{
			if (std::abs(unitPos.y - m_target.y) > alignEps)
			{
				return moveTo({ targetEdgeX, clampY(m_target.y) });
			}

			return moveTo(clampedTarget);
		}

		// Case 5:
		// Unit is already on a good horizontal edge. Match target x first.
		if (onTargetHorizontalEdge)
		{
			if (std::abs(unitPos.x - m_target.x) > alignEps)
			{
				return moveTo({ clampX(m_target.x), targetEdgeY });
			}

			return moveTo(clampedTarget);
		}

		// Fallback. Should rarely happen, but prevents undefined behavior.
		return moveTo(clampedTarget);
	}
}