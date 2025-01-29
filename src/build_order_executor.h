#pragma once

#include <cstddef>

namespace Aeolus
{
	class AeolusBot;
	/**
	* @brief The build order executor in charge of converting build orders into
	* macro behaviors and register them for execution.
	*/

	enum class BuildOrderEnum
	{
		MACRO_STALKERS
	};

	class BuildOrderExecutor
	{
	public:
		static BuildOrderExecutor& GetInstance();

		void execute(AeolusBot& aeolusbot, BuildOrderEnum build_order);

	private:
		//constructor
		BuildOrderExecutor() : build_order_step(0)
		{
		}
		//destructor
		~BuildOrderExecutor() = default;

		size_t build_order_step;
	};
}