#pragma once
#include <fstream>
#include <filesystem>
#include <string>

namespace Aeolus
{
	namespace utils
	{
		inline void recordMatchResult(const std::string& opponent, const std::string& strategy, bool isWin, bool override_last = false)
		{
			namespace fs = std::filesystem;
			// Ensure the "data" directory exists.
			fs::create_directories("data");

			const std::string filePath = "data/match_history.csv";
			// Prepare the new entry.
			std::string newEntry = opponent + "," + strategy + "," + (isWin ? "Win" : "Loss");

            if (override_last)
            {
                // Read the existing file contents.
                std::vector<std::string> lines;
                {
                    std::ifstream inFile(filePath);
                    std::string line;
                    while (std::getline(inFile, line))
                    {
                        lines.push_back(line);
                    }
                    // inFile is closed automatically when it goes out of scope.
                }

                // Remove the last line if the file is not empty.
                if (!lines.empty())
                {
                    lines.pop_back();
                }

                // Open the file in truncation mode to overwrite it.
                std::ofstream outFile(filePath, std::ios::trunc);
                if (!outFile)
                    return;

                // Write back the remaining lines.
                for (const auto& existingLine : lines)
                {
                    outFile << existingLine << "\n";
                }
                // Append the new entry.
                outFile << newEntry << "\n";
            }
            else
            {
                // Just append the new entry to the file.
                std::ofstream outFile(filePath, std::ios::app);
                if (!outFile)
                    return;
                outFile << newEntry << "\n";
            }
		}
	}
}