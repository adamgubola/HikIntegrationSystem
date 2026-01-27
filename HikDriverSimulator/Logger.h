#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <sstream>

enum class LogLevel {
	INFO,
	WARNING,
	ERROR_LOG,
	NETWORK
};

class Logger {
private:
	static std::mutex logMutex;
	static std::ofstream logFile;
	static bool consolOutput;
	static std::string GetCurrentTime();
	static void LogInternal(LogLevel level, const std::string& message);

public:
	static void Init(const std::string& filename, bool consoleOutput = true);
	static void Info(const std::string& message);
	static void Warning(const std::string& message);
	static void Error(const std::string& message);
	static void Network(const std::string& message);
};