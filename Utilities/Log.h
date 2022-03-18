#pragma once
#include <stdio.h>
#include <fstream>
#include <filesystem>

class Logger {
public:

	Logger(std::string FileName, bool append = false)
	{
		std::filesystem::path Path = std::filesystem::current_path();
		Path += FileName;
		
		LogFile.open(Path, append ? std::fstream::out : std::fstream::out | std::fstream::app);

		printf("[LOG] Log has been created at location %ws\n\n", Path.c_str());
	}

	inline void CloseLog() {
		LogFile.close();
	}

	void LogToFile(std::string Information) {
		LogFile << Information << std::endl;
	}

private:
	std::ofstream LogFile;
};


