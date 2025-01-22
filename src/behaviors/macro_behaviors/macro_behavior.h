#pragma once

#include "../behavior.h"
/*
#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
*/

namespace Aeolus
{
	class AeolusBot;
	class ManagerMediator;
	class MacroBehavior : public Behavior
	{
		/* Interface all macro behaviors should adhere to.
		All macro behaviors must have the execute() member function,
		which takes in constant reference of the bot and mediator
		for infromation retrieval only. */
	public:
		~MacroBehavior() override = default;

		virtual bool execute(AeolusBot& aeolusbot) = 0;
	};
}