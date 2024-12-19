#pragma once

#include "macro_behavior.h"

#include <unordered_map>

namespace Aeolus
{
	class AeolusBot;

	class Mining : public MacroBehavior
	{
		/* Mining behavior, ideally registered during every step
		Executing this behavior will instruct workers to mine
		minerals and vespene gas, with the option for mineral
		boosting 
		TODO: Add mineral boost
		TODO: Add worker evasion */
	public:
		Mining
		(
			double flee_at_health_perc = 0.5,
			bool keep_safe = true,
			bool mineral_boost = true,
			bool self_defence_active = true
		) :
			m_flee_at_health_perc(flee_at_health_perc),
			m_keep_safe(keep_safe),
			m_mineral_boost(mineral_boost),
			m_self_defence_active(self_defence_active) {}

		~Mining() override = default;
		void execute(AeolusBot& aeolusbot) override;

	private:
		double m_flee_at_health_perc;
		bool m_keep_safe;
		bool m_mineral_boost;
		bool m_self_defence_active;

		void DoMiningBoost(
			const ::sc2::Unit* patch, 
			const ::sc2::Unit* worker,
			const std::unordered_map<const ::sc2::Unit*, ::sc2::Point2D>& patch_target_map,
			AeolusBot& aeolusbot
			);

		std::unordered_map<const ::sc2::Unit*, ::sc2::Point2D> m_patch_map;

		::sc2::Units m_town_halls;
	};
}