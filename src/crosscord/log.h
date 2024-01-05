#pragma once

#pragma warning(push, 0)
#include <fmt/format.h>
#pragma warning(pop)

enum class ELogType {
	Verbose,
	Info,
	Warning,
	Error
};

template <typename... T>
inline std::string FormatLog(const char* cFmt, T&&... Arguments) {
	return fmt::vformat(cFmt, fmt::make_format_args(Arguments...));
}

void Log(ELogType eLogType, std::string sMessage, bool bNewline = true);

void LogInit(const char* cLogName, const char* cLogLocation);

#ifdef _DEBUG
template<typename... Args>
inline void LogVerbose(const char* cFmt, Args... Arguments) { Log(ELogType::Verbose, FormatLog(cFmt, std::forward<Args>(Arguments)...)); }
#else
#define LogVerbose(...)
#endif

template<typename... Args>
inline void LogInfo(const char* cFmt, Args... Arguments) { Log(ELogType::Info, FormatLog(cFmt, std::forward<Args>(Arguments)...)); }

template<typename... Args>
inline void LogWarning(const char* cFmt, Args... Arguments) { Log(ELogType::Warning, FormatLog(cFmt, std::forward<Args>(Arguments)...)); }

template<typename... Args>
inline void LogError(const char* cFmt, Args... Arguments) { Log(ELogType::Error, FormatLog(cFmt, std::forward<Args>(Arguments)...)); }