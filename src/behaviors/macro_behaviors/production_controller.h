#include "macro_behavior.h"
#include <map>
#include <optional>
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	class AeolusBot;
	class ProductionController : public MacroBehavior
	{
	public:

		ProductionController(std::map<::sc2::UNIT_TYPEID, float> army_composition_map,
			std::pair<int, int> add_production_at_bank = { 300, 300 },
			float alpha = 0.9f,
			float unit_pending_progress = 0.75f,
			float ignore_below = 0.05f,
			bool should_repower = true) : 
			m_army_composition_map(army_composition_map),
			m_add_production_at_bank(add_production_at_bank),
			m_unit_pending_progress(unit_pending_progress),
			m_ignore_below(ignore_below),
			m_repower_structures(should_repower)
		{}

		~ProductionController() override = default;

		void execute(AeolusBot& aeolusbot) override;

	private:
		std::map<::sc2::UNIT_TYPEID, float> m_army_composition_map;
		std::pair<int, int> m_add_production_at_bank;
		float m_alpha;
		float m_unit_pending_progress;
		float m_ignore_below;
		bool m_repower_structures;

		static std::optional<::sc2::UNIT_TYPEID> _isTrainedFrom(::sc2::UNIT_TYPEID unit_type);
		bool _techUp(AeolusBot& aeolusbot, ::sc2::UNIT_TYPEID unit_type, int base_location = 0);

		bool _buildProductionDueToBank(AeolusBot& aeolusbot,
			::sc2::UNIT_TYPEID unit_type,
			float mineral_collection_rate,
			float gas_collection_rate,
			size_t existing_production_count,
			::sc2::UNIT_TYPEID production_structure_id, 
			float target_proportion);
	};
}