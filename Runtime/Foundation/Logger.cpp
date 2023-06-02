#include "Logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

void Logger::LogMessage(DebugLevel level, const std::source_location &location, const std::string &message) {
    std::string stringToWrite;
    const char *pFileName = location.file_name();
    const uint32_t line = location.line();
    const uint32_t column = location.column();
    std::string output;
    switch (level) {
    case EL_Info:
        output = fmt::format("{}({},{}) [info ]: {}", pFileName, line, column, message);
        _pLogger->info("{}", output);
        _pConsoleLogger->info("{}", output);
        break;
    case EL_Debug:
        output = fmt::format("{}({},{}) [Debug]: {}", pFileName, line, column, message);
        _pLogger->debug("{}", output);
        _pConsoleLogger->debug("{}", output);
        break;
    case EL_Warning:
        output = fmt::format("{}({},{}) [Warn ]: {}", pFileName, line, column, message);
        _pLogger->warn("{}", output);
        _pConsoleLogger->warn("{}", output);
        break;
    case EL_Error:
        output = fmt::format("{}({},{}) [Error]: {}", pFileName, line, column, message);
        _pLogger->error("{}", output);
        _pConsoleLogger->error("{}", output);
        break;
    case EL_None:
    default:
        return;
    }

    if (_logCallback != nullptr) {
        _logCallback(level, output);
    }

#if PLATFORM_WIN
    OutputDebugStringA(output.c_str());
#endif
}

void Logger::Initialize() {
    _logLevelMask = EL_Info | EL_Debug | EL_Warning | EL_Error;
    _pattern = "[%H:%M:%S] %v";
    _logFilePath = "log.txt";
}

void Logger::Destroy() {
    _pLogger = nullptr;
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

void Logger::SetLogCallBack(const LogCallBack &callback) {
    _logCallback = callback;
}

void Logger::StartLogging() {
    _pLogger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");
    _pConsoleLogger = spdlog::stdout_color_mt("console");
    spdlog::set_pattern(_pattern.c_str());
}

auto Logger::GetLogLevelMask() const -> DebugLevel {
    return _logLevelMask;
}
