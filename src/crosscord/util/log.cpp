#include "log.h"

#pragma warning(push, 0)
#include <fmt/color.h>
#include <fmt/chrono.h>
#pragma warning(pop)

#include <map>
#include <time.h>
#include <mutex>
#include <fstream>
#include <filesystem>

#ifdef _DEBUG
#include <debugapi.h>
#endif

#define LOG_MAX_ENTRIES 4

bool bLogInitialized = false;

// from: minwindef.h
#define MAX_PATH 260

std::mutex LogFileLock;
std::ofstream LogFile;
std::ofstream LatestFile;

void LogInit(const char* cLogName, const char* cLogLocation) {
    if (bLogInitialized)
        return;

    /*
        define log filename and setup log path
    */
    std::string sLogFilename = fmt::format("{} - {:%Y-%m-%d %H.%M.%S}.txt", cLogName, fmt::localtime(std::time(nullptr)));

    std::string sLogName = fmt::format("{}{}", cLogLocation, sLogFilename);
    std::string sLatestName = fmt::format("{}{}", cLogLocation, "latest.txt");

    std::filesystem::create_directories(cLogLocation);

    /*
        make sure there's as many files as LOG_MAX_ENTRIES allows
    */
    std::multimap<time_t, std::filesystem::directory_entry> LogFiles;
    for (auto& Entry : std::filesystem::directory_iterator(cLogLocation)) {
        time_t EntryTime = Entry.last_write_time().time_since_epoch().count();
        LogFiles.insert(std::pair(EntryTime, Entry));
    }

    if (LogFiles.size() >= LOG_MAX_ENTRIES) {
        size_t nToDelete = LogFiles.size() - LOG_MAX_ENTRIES;
        unsigned int i = 0;

        for (auto const& [Time, File] : LogFiles) {
            i++;
            if (i > nToDelete)
                break;

            std::filesystem::remove(File);
        }
    }
    

    LogFile.open(sLogName, std::ios::out);
    LatestFile.open(sLatestName, std::ios::out);

    bLogInitialized = true;
}

void Log(ELogType eLogType, std::string sText, bool bNewline) {
    fmt::color LogColor = fmt::color::white;
    std::string sLogType;

    switch (eLogType) {
        case ELogType::Verbose:
            sLogType = "VERBOSE";
            LogColor = fmt::color::green;
            break;
        case ELogType::Info:
            sLogType = "INFO";
            LogColor = fmt::color::blue;
            break;
        case ELogType::Warning:
            sLogType = "WARNING";
            LogColor = fmt::color::yellow;
            break;
        case ELogType::Error:
            sLogType = "ERROR";
            LogColor = fmt::color::red;
            break;
    }

    if (bLogInitialized) {
        LogFileLock.lock();
        LogFile << fmt::format("{:%Y-%m-%d - %H:%M:%S} - [{}] - {}", fmt::localtime(std::time(nullptr)), sLogType, sText) << std::endl;
        LatestFile << fmt::format("{:%Y-%m-%d - %H:%M:%S} - [{}] - {}", fmt::localtime(std::time(nullptr)), sLogType, sText) << std::endl;
        LogFileLock.unlock();
    }

#ifndef _DEBUG
    if (eLogType == ELogType::Verbose) return;
#endif

    fmt::print(fg(LogColor), "[{}]", sLogType);
    if (bNewline)
        fmt::print(fg(fmt::color::white), " {}\n", sText);
    else
        fmt::print(fg(fmt::color::white), " {}", sText);
}
