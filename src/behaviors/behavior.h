#pragma once

namespace Aeolus
{
	class AeolusBot;
	class ManagerMediator;

	class Behavior
	{
		/* Interface that all behavior should adhere to.
		Namely, all behavior should feature the public execute()
		interface, such that it can be called by the behavior
		executor. Each behavior will have access to the manager
		mediator for information retrieval*/

	public:
		virtual bool execute(AeolusBot& aeolusbot) = 0;

		/* Execute the implemented behavior.
		Parameters:
			ai: Aeolus bot object running the game
			mediator: Manager mediator used for getting information */

		virtual ~Behavior() = default;
	};
};
