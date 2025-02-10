#pragma once

#include "file_io_utils.h"
#include "../build_order_enum.h"
#include <iostream>
#include <map>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <set>

namespace Aeolus
{
	namespace utils
	{
		// A simple multi-armed bandit model using an epsilon-greedy approach with weight decay.
		class EpsilonGreedyMAB
		{
		public:
			/**
			* @brief Constructs a new MAB epsilon-greedy model for choosing our strategy
			* @param strategies: vector of available strategies (arms)
			* @param epsilon: probability of exploring (choosing a random strategy)
			* @param alpha: learning rate for the chosen arm
			* @param decay: multiplicative decay factor fir all other arms (0 < decay < 1)
			*/
			EpsilonGreedyMAB(const std::vector<BuildOrderEnum>& strategies,
				double epsilon,
				double alpha,
				double decay) : m_strategies(strategies), m_epsilon(epsilon), m_alpha(alpha), m_decay(decay)
			{
				std::cout << "MAB: initializing Q-table... " << std::endl;
				for (const auto& strategy : m_strategies)
				{
					m_Q_table[strategy] = 0.0f;
				}
				std::cout << "MAB: seeding random number..." << std::endl;
				std::srand(static_cast<unsigned int>(std::time(nullptr)));
				std::cout << "MAB: initialization complete." << std::endl;
			}

			/**
			* @brief Choose a strategy using the epsilon-greedy method.
			*/
			BuildOrderEnum chooseStrategy()
			{
				std::cout << "MAB: all matches recorded. now choosing a strategy... " << std::endl;
				double rand_val = static_cast<double>(std::rand()) / RAND_MAX; // random value between 0 and 1
				BuildOrderEnum chosenStrategy;

				if (rand_val < m_epsilon)
				{
					// exploration: choose a random strategy
					std::cout << "MAB: choosing a random strategy... " << std::endl;
					int idx = std::rand() % m_strategies.size();
					chosenStrategy = m_strategies[idx];
				}
				else
				{
					std::cout << "MAB: choosing a strategy with maximum q-value... " << std::endl;
					auto max_it = std::max_element(
						m_Q_table.begin(), m_Q_table.end(),
						[](const std::pair<const BuildOrderEnum, double>& a,
							const std::pair<const BuildOrderEnum, double>& b)
						{
							return a.second < b.second;
						}
					);

					if (max_it != m_Q_table.end())
					{
						// found best strategy, return it
						std::cout << "MAB: found best strategy: " << buildOrderToString(max_it->first) << std::endl;
						chosenStrategy = max_it->first;
					}
					else
					{
						// somehow no best strategy, return the first one
						std::cout << "MAB: no best strategy found. " << std::endl;
						chosenStrategy = m_strategies.front();
					}
				}

				// Nicely formatted console output of the Q-table summary.
				std::cout << "================ Strategy Q-Value Summary ================\n";
				for (const auto& entry : m_Q_table)
				{
					// Assuming you have a function toString() that converts BuildOrderEnum to a string.
					std::cout << "Strategy: " << buildOrderToString(entry.first)
						<< " | Q-value: " << entry.second << "\n";
				}
				std::cout << "==========================================================\n";
				std::cout << "<<< Chosen strategy: " << buildOrderToString(chosenStrategy) << " >>>" << std::endl;

				return chosenStrategy;
			}

			/**
			* @brief update our model for a give nstrategy based on the observed reward.
			* The chosen strategy's Q value is updated towards the new reward (with learning rate alpha),
			* While the q values for all other strategies are decayed by the decay factor
			*/
			void update(const BuildOrderEnum strategy, double reward)
			{
				auto it = m_Q_table.find(strategy);
				if (it == m_Q_table.end()) return; // unknown strategy, do nothing

				// go into the Q-table
				for (auto it = m_Q_table.begin(); it != m_Q_table.end(); ++it)
				{
					if (it->first != strategy)
					{
						// decay the q-value of non-chosen strategies
						it->second *= m_decay;
					}
					else
					{
						// update the q-value of this strategy
						it->second = ((1 - m_alpha) * it->second) + reward * m_alpha;
					}
				}
			}

			/**
			* @brief Parse a single CSV line
			* In the format: opponent_id, strategy, Win/Loss
			* Here we convert "Win" to a reward of 1.0 and any other result (e.g. "Loss") to 0.0.
			*/
			void updateFromRecord(const std::string& record)
			{
				// Find commas to split the line.
				size_t first_comma = record.find(',');
				if (first_comma == std::string::npos)
					return;
				size_t second_comma = record.find(',', first_comma + 1);
				if (second_comma == std::string::npos)
					return;
				
				// Extract the strategy
				auto strategy = stringToBuildOrder(record.substr(first_comma + 1,
					second_comma - first_comma - 1));
				if (!strategy.has_value()) return;

				std::string outcome = record.substr(second_comma + 1);

				double reward = (outcome == "Win") ? 1.0f : 0.0f;
				update(strategy.value(), reward);
			}

		private:
			std::vector<BuildOrderEnum> m_strategies;
			std::map<BuildOrderEnum, double> m_Q_table;
			double m_epsilon;
			double m_alpha;
			double m_decay;
		};

		/**
		* @brief Given a CSV string containing historical match data,
		* (opponent,strategy,Win/Loss) in individual lines, builds a MAB
		* model based on the unique strategies found, updates it with
		* all the records, and then returns a strategy selected via the 
		* epsilon-greedy approach.
		* @param loaded_data: loaded lines in the csv file
		* @param epsilon: the probability of selecting a random strategy
		* @param alpha: the rate at which the strategy is learned
		* @param decay: number between 0 and 1 to update decay all win rates with after a match
		*/
		inline BuildOrderEnum chooseBestStrateGyFromHistory(const std::vector<std::string>& records,
			double epsilon = 0.1,
			double alpha = 0.2,
			double decay = 0.99)
		{
			std::set<BuildOrderEnum> unique_strategies
			{
				BuildOrderEnum::MACRO_STALKERS, 
				BuildOrderEnum::STALKER_IMMORTAL
			};

			//// parse the input string line by line.
			//for (const auto& line : records)
			//{
			//	if (line.empty()) continue;

			//	// Extract the strategy fro meach record.
			//	size_t first_comma = line.find(',');
			//	if (first_comma == std::string::npos)
			//		continue;
			//	size_t second_comma = line.find(',', first_comma + 1);
			//	if (second_comma == std::string::npos)
			//		continue;

			//	// Extract the strategy
			//	auto strategy = stringToBuildOrder(line.substr(first_comma + 1,
			//		second_comma - first_comma - 1));
			//	if (strategy.has_value())
			//		unique_strategies.insert(strategy.value());
			//}

			if (unique_strategies.empty())
			{
				std::cout << "failed to parse strategies, resorting to default macro stalkers. " << std::endl;
				return BuildOrderEnum::MACRO_STALKERS;
			}

			// found a set of strategies, build a vector if them
			std::cout << "Initializing strategy vector... " << std::endl;
			std::vector<BuildOrderEnum> strategies(unique_strategies.begin(), unique_strategies.end());

			// Create the MAB model.
			std::cout << "Creating the MAB model" << std::endl;
			EpsilonGreedyMAB mab(strategies, epsilon, alpha, decay);

			// with each record, update the model.
			for (const auto& rec : records)
			{
				std::cout << "updating the MAB with: \n" << rec << std::endl;
				mab.updateFromRecord(rec);
			}

			// return the chosen srtategy
			return mab.chooseStrategy();
		}
	} // namespace utils
} // namespace Aeolus