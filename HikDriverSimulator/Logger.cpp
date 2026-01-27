#pragma once
#include "Logger.h"

std::mutex Logger::logMutex;
std::ofstream Logger::logFile;
bool Logger::consolOutput = true;

void Logger::Init(const std::string& filename, bool consolOut) {
	std::lock_guard<std::mutex> lock(logMutex);
	consolOutput = consolOut;

	logFile.open(filename, std::ios::app);
	if (!logFile.is_open()) {
		std::cerr << "[CRITICAL] Failed to open log file: " << filename << std::endl;
	}
}
std::string Logger::GetCurrentTime() {

	auto now = std::time(nullptr);
	std::tm tmNow;
	localtime_s(&tmNow, &now);
	std::ostringstream oss;
	oss << std::put_time(&tmNow, "%Y-%m-%d %H:%M:%S");
	return oss.str();
}
void Logger::LogInternal(LogLevel level, const std::string& message) {
	std::lock_guard<std::mutex> lock(logMutex);
	std::string levelStr;
	std::string colorCode = "";
	std::string resetColor = "\033[0m";

	switch (level) {
	case LogLevel::INFO:
		levelStr = "INFO";
		colorCode = "\033[32m"; // Green
		break;
	case LogLevel::WARNING:
		levelStr = "WARNING";
		colorCode = "\033[33m"; // Yellow
		break;
	case LogLevel::ERROR_LOG:
		levelStr = "ERROR";
		colorCode = "\033[31m"; // Red
		break;
	case LogLevel::NETWORK:
		levelStr = "NETWORK";
		colorCode = "\033[34m"; // Blue
		break;
	}
	std::string timeStamp = GetCurrentTime();
	std::string finalLog = "[" + timeStamp + "] [" + levelStr + "] " + message;
	if (logFile.is_open()) {
		logFile << finalLog << std::endl;
	}
	if (consolOutput) {
		std::cout << colorCode << finalLog << resetColor << std::endl;
	}
}
void Logger::Info(const std::string& message) {
	LogInternal(LogLevel::INFO, message);
}
void Logger::Warning(const std::string& message) {
	LogInternal(LogLevel::WARNING, message);
}
void Logger::Error(const std::string& message) {
	LogInternal(LogLevel::ERROR_LOG, message);
}
void Logger::Network(const std::string& message) {
	LogInternal(LogLevel::NETWORK, message);
}

