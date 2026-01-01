#pragma once

#include "macro_behavior.h"

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief Builds detection if we detect invisible enemy units.
	* Can be overriden with force = true to build detection anyways
	*/
	class BuildDetection : public MacroBehavior
	{
	public:
		BuildDetection(bool force = false, int max = 1) : m_force(force), m_max(max) {}
		~BuildDetection() override = default;

		bool execute(AeolusBot& aeolusbot) override;

	private:
		bool m_force;
		bool m_max;
	};

}