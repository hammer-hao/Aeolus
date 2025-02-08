#pragma once
#include <fstream>
#include <filesystem>
#include <string>

namespace Aeolus
{
	namespace utils
	{
		inline void recordMatchResult(const std::string& opponent, const std::string& strategy, bool isWin)
		{
			// create the data directory if it does not exist
			namespace fs = std::filesystem;
			fs::create_directories("data");

			// open the file we write to
			std::ofstream outFile("data/match_history.csv", std::ios::app);
			if (!outFile) return;

			std::string resultText = isWin ? "Win" : "Loss";

			outFile << opponent << ','
				<< strategy << ','
				<< resultText << '\n';

			outFile.close();
		}
	}
}