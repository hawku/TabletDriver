#pragma once
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <ctime>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <Windows.h>

#include "Runnable.h"

#ifndef LOG_MODULE
#define LOG_MODULE "Logger"
#endif

#define LOGGER_FILE(file) logger.OpenLogFile(file)
#define LOGGER_START(...) logger.Start()
#define LOGGER_STOP(...) logger.Stop()
#define LOGGER_PROCESS(...) logger.ProcessQueue()
#define LOGGER_DIRECT logger.directPrint

#define LOG_DIE(...)  (logger.LogMessage(logger.LogLevelCritical, LOG_MODULE,  __VA_ARGS__), LOGGER_PROCESS(); exit(1))
#define LOG_DIEBUFFER(buf, len, ...) (logger.LogBuffer(LogLevelCritical, LOG_MODULE, buf, len, __VA_ARGS__), LOGGER_PROCESS(), exit(1))

#define LOG_ERROR(...)   logger.LogMessage(logger.LogLevelError, LOG_MODULE, __VA_ARGS__)
#define LOG_ERRORBUFFER(buf, len, ...) logger.LogBuffer(logger.LogLevelError, LOG_MODULE, buf, len, __VA_ARGS__)

#define LOG_WARNING(...)  logger.LogMessage(logger.LogLevelWarning, LOG_MODULE, __VA_ARGS__)
#define LOG_WARNINGBUFFER(buf, len, ...) logger.LogBuffer(4, LOG_MODULE, buf, len, __VA_ARGS__)

#define LOG_NOTE(...)  logger.LogMessage(logger.LogLevelNote, LOG_MODULE, __VA_ARGS__)
#define LOG_NOTEBUFFER(buf, len, ...) logger.LogBuffer(logger.LogLevelNote, LOG_MODULE, buf, len, __VA_ARGS__)

#define LOG_STATUS(...)  logger.LogMessage(logger.LogLevelStatus, LOG_MODULE, __VA_ARGS__)
#define LOG_STATUSBUFFER(buf, len, ...) logger.LogBuffer(logger.LogLevelStatus, LOG_MODULE, buf, len, __VA_ARGS__)

#define LOG_INFO(...)  logger.LogMessage(logger.LogLevelInfo, LOG_MODULE, __VA_ARGS__)
#define LOG_INFOBUFFER(buf, len, ...) logger.LogBuffer(logger.LogLevelInfo, LOG_MODULE, buf, len, __VA_ARGS__)

#define LOG_DEBUG(...) logger.LogMessage(logger.LogLevelDebug, LOG_MODULE, __VA_ARGS__)
#define LOG_DEBUGBUFFER(buf, len, ...) logger.LogBuffer(logger.LogLevelDebug, LOG_MODULE, buf, len, __VA_ARGS__)


class Logger : public Runnable {
private:
	std::thread threadLog;
	void run();
	std::mutex lockQueue;
	std::mutex lockNewItem;
	std::condition_variable conditionNewItem;
	std::ofstream logFile;
	bool logToFile;
	bool isDebugOutputEnabled;

public:
	enum LogLevels {
		LogLevelUnknown,
		LogLevelUnknown2,
		LogLevelCritical,
		LogLevelError,
		LogLevelWarning,
		LogLevelNote,
		LogLevelStatus,
		LogLevelInfo,
		LogLevelDebug
	};
	typedef struct {
		tm time;
		SYSTEMTIME systemTime;
		int level;
		std::string module;
		std::string message;
	} LogItem;
	std::queue<LogItem*> logQueue;

	std::string levelNames[9] = {
		"",
		"",
		"CRITICAL",
		"ERROR",
		"WARNING",
		"NOTE",
		"STATUS",
		"INFO",
		"DEBUG"
	};
	bool directPrint;
	std::string logFilename = "";
	std::function<void(void *buffer, int length)> writeCallback = NULL;
	std::mutex lock;

	void OutputItem(LogItem *item);
	void ProcessQueue();
	void LogMessage(int level, std::string module, const char *fmt, ...);
	void LogBuffer(int level, std::string module, void *buffer, int length, const char *fmt, ...);
	int verbosity;
	Logger();
	void AddQueue(LogItem *item);
	void NotifyNewItem();
	void WaitNewItem();
	void Start();
	void Stop();
	bool OpenLogFile(std::string filename);
	bool CloseLogFile();

	void SetDebugOutput(bool enabled);
	bool IsDebugOutputEnabled();

};
extern Logger logger;

