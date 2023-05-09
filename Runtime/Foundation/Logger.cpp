#include "Logger.h"
#include <spdlog/spdlog.h>

void Logger::Initialize() {
	_logLevelMask = EL_Info | EL_Debug | EL_Warning | EL_Error;
	_pattern = "[%H:%M:%S] %v";
	_logFilePath = "log.txt";
}

void Logger::Destroy() {
	_pLogger = nullptr;
}

void Logger::LogMessage(DebugLevel level, const std::source_location &location, const std::string &message) {
	std::string stringToWrite;
	const char *pFileName = location.file_name();
	const uint32_t line = location.line();
	const uint32_t column = location.column();
	switch (level) {
	case EL_Info:
		_pLogger->info("{}({},{}) [info-]: {}", pFileName, line, column, message);
		break;
	case EL_Debug:
		_pLogger->debug("{}({},{}) [Debug]: {}", pFileName, line, column, message);
		break;
	case EL_Warning:
		_pLogger->warn("{}({},{}) [Warn-]: {}", pFileName, line, column, message);
		break;
	case EL_Error:
		_pLogger->error("{}({},{}) [Error]: {}", pFileName, line, column, message);
		break;
	case EL_None:
	default:
		break;
	}
}

void Logger::SetLogPath(stdfs::path logPath) {
	_logFilePath = logPath;
}

void Logger::SetLogLevelMask(DebugLevel mask) {
	_logLevelMask = mask;
}

void Logger::SetPattern(const std::string &pattern) {
	_pattern = pattern;
}

void Logger::StartLogging() {
	_pLogger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");
	spdlog::set_pattern(_pattern.c_str());
}

auto Logger::GetLogLevelMask() const -> DebugLevel {
	return _logLevelMask;
}

