#pragma once
#include <spdlog/sinks/basic_file_sink.h>
#include "Foundation/RuntimeStatic.h"
#include "Foundation/NonCopyable.h"
#include "Foundation/NamespeceAlias.h"
#include "Foundation/Exception.h"
#include "Foundation/PreprocessorDirectives.h"

class Logger : public NonCopyable {
public:
	enum DebugLevel {
		EL_None		= 0,
        EL_Info		= 1 << 0,
		EL_Debug	= 1 << 1,
		EL_Warning	= 1 << 2,
		EL_Error	= 1 << 3,
		EL_All		= EL_Info | EL_Debug | EL_Warning | EL_Error,
    };
	ENUM_FLAGS_AS_MEMBER(DebugLevel);
private:
	friend class RuntimeStatic<Logger>;
    void Initialize();
	void Destroy();
	void LogMessage(DebugLevel level, const std::source_location &location, const std::string &message);
public:
	void SetLogPath(stdfs::path logPath);
	void SetLogLevelMask(DebugLevel mask);
	void SetPattern(const std::string &pattern);
	void StartLogging();
	auto GetLogLevelMask() const -> DebugLevel;

	template<typename ...Args>
	static void Info(FormatAndLocation fmtAndLoc, Args&&...args);

	template<typename ...Args>
	static void Debug(FormatAndLocation fmtAndLoc, Args&&...args);

	template<typename ...Args>
	static void Warning(FormatAndLocation fmtAndLoc, Args&&...args);

	template<typename ...Args>
	static void Error(FormatAndLocation fmtAndLoc, Args&&...args);
private:
	DebugLevel						_logLevelMask = EL_None;
	std::string						_pattern;
	stdfs::path						_logFilePath;
	std::shared_ptr<spdlog::logger> _pLogger;
};

inline RuntimeStatic<Logger> gLogger;

template<typename ... Args>
void Logger::Info(FormatAndLocation fmtAndLoc, Args &&... args) {
	if (gLogger == nullptr || !HasFlag(gLogger->GetLogLevelMask(), EL_Info)) {
	    return;
	}
	std::string message = fmt::vformat(fmtAndLoc.fmt, fmt::make_format_args(std::forward<Args>(args)...));
    gLogger->LogMessage(EL_Info, fmtAndLoc.location, message);		
}

template<typename ... Args>
void Logger::Debug(FormatAndLocation fmtAndLoc, Args &&... args) {
	if (gLogger == nullptr || !HasFlag(gLogger->GetLogLevelMask(), EL_Debug)) {
	    return;
	}
	std::string message = fmt::vformat(fmtAndLoc.fmt, fmt::make_format_args(std::forward<Args>(args)...));
    gLogger->LogMessage(EL_Debug, fmtAndLoc.location, message);	
}

template<typename ... Args>
void Logger::Warning(FormatAndLocation fmtAndLoc, Args &&... args) {
	if (gLogger == nullptr || !HasFlag(gLogger->GetLogLevelMask(), EL_Warning)) {
	    return;
	}
	std::string message = fmt::vformat(fmtAndLoc.fmt, fmt::make_format_args(std::forward<Args>(args)...));
    gLogger->LogMessage(EL_Warning, fmtAndLoc.location, message);	
}

template<typename ... Args>
void Logger::Error(FormatAndLocation fmtAndLoc, Args &&... args) {
	if (gLogger == nullptr || !HasFlag(gLogger->GetLogLevelMask(), EL_Error)) {
	    return;
	}
	std::string message = fmt::vformat(fmtAndLoc.fmt, fmt::make_format_args(std::forward<Args>(args)...));
    gLogger->LogMessage(EL_Error, fmtAndLoc.location, message);	
}
