#include "precompiled.h"
#include "Logger.h"

//
// Constructor
//
Logger::Logger() {
	verbosity = LogLevelDebug;
	directPrint = false;
	isDebugOutputEnabled = false;
	writeCallback = NULL;
}


void Logger::OutputItem(LogItem *item) {

	char buffer[4096];
	int bufferSize = 4096;
	int index = 0;

	// Time
	index += strftime(buffer + index, bufferSize - index, "[%Y-%m-%d %H:%M:%S", &item->time);

	// Milliseconds
	if (index < bufferSize)
		index += sprintf_s(buffer + index, bufferSize - index, ".%03d] ", item->systemTime.wMilliseconds);

	// Module
	if (item->module.length() > 0) {
		if (index < bufferSize)
			index += sprintf_s(buffer + index, bufferSize - index, "[%s] ", item->module.c_str());
	}

	// Log level
	if (item->level != LogLevelInfo) {
		if (index < bufferSize)
			index += sprintf_s(buffer + index, bufferSize - index, "[%s] ", levelNames[item->level].c_str());
	}

	// Message
	if (index < bufferSize) {

		// Message to large for the buffer
		if ((int)item->message.size() > bufferSize - index) {
			memcpy(buffer + index, item->message.c_str(), (bufferSize - index) - 1);
			index = bufferSize;
		}

		// Copy message to buffer
		else
		{
			memcpy(buffer + index, item->message.c_str(), item->message.size());
			index += item->message.size();
		}
	}

	// Write to standard output
	try {
		std::cout.write(buffer, index);
		std::cout.flush();
	}
	catch (std::exception) {
	}

	// Write to output pipe
	try {
		if (writeCallback != NULL) {
			writeCallback(buffer, index);
		}
	}
	catch (std::exception) {
	}

	// Write to log file
	if (logFile && logFile.is_open()) {
		logFile.write(buffer, index);
		logFile.flush();
	}
}

//
// Process queue
//
void Logger::ProcessQueue() {

	std::queue<LogItem*> temporaryQueue;

	// Lock queue
	lockQueue.lock();

	// Loop through queue items and add to a temporary queue
	while (!logQueue.empty()) {
		LogItem *item = logQueue.front();
		temporaryQueue.push(item);
		logQueue.pop();
	}

	// Unlock queue
	lockQueue.unlock();

	// Process the temporary queue
	while (!temporaryQueue.empty()) {
		LogItem *item = temporaryQueue.front();

		// Output log item
		OutputItem(item);

		// Destroy log item
		delete item;

		temporaryQueue.pop();
	}

}


//
// Log message
//
void Logger::LogMessage(int level, std::string module, const char *fmt, ...) {
	char message[4096];
	int maxLength = sizeof(message) - 1;
	int index;
	time_t t;
	message[0] = 0;

	// Clamp level
	if (level < 2)
		level = 2;
	else if (level > 8)
		level = 8;

	if (level <= verbosity) {
		index = 0;
		time(&t);

		// Loop through arguments
		va_list ap;
		va_start(ap, fmt);
		if (index < maxLength) index += vsnprintf(message + index, maxLength - index, fmt, ap);
		va_end(ap);

		// New line at the end when the message is larger than max length
		if (index >= maxLength) {
			message[maxLength - 1] = '\n';
			message[maxLength] = 0;
		}

		// Add message
		LogItem *logItem = new LogItem();
		localtime_s(&logItem->time, &t);
		GetSystemTime(&logItem->systemTime);
		logItem->level = level;
		logItem->module = module;
		logItem->message = message;
		AddQueue(logItem);
	}
}

//
// Log buffer data
//
void Logger::LogBuffer(int level, std::string module, void *buffer, int length, const char *fmt, ...) {
	bool newLine = false;
	char message[4096];
	int maxLength = sizeof(message) - 1;
	time_t t;
	int index;
	message[0] = 0;

	// Clamp level
	if (level < 2)
		level = 2;
	else if (level > 8)
		level = 8;

	if (level <= verbosity) {
		index = 0;
		time(&t);

		// Loop through arguments
		va_list ap;
		va_start(ap, fmt);
		if (index < maxLength) index += vsnprintf(message + index, maxLength - index, fmt, ap);
		va_end(ap);

		// Detect new line
		newLine = (fmt[strlen(fmt) - 1] == '\n');
		if (newLine) {
			if (index < maxLength) index += snprintf(message + index, maxLength - index, "  { ");

		}
		else {
			if (index < maxLength) index += snprintf(message + index, maxLength - index, "{ ");
		}


		// Loop through buffer bytes
		for (int i = 0; i < length; i++) {

			// Last byte
			if (i == length - 1) {
				if (index < maxLength) index += snprintf(message + index, maxLength - index, "0x%02x", ((unsigned char*)buffer)[i]);

				//
			}
			else {
				if (index < maxLength) index += snprintf(message + index, maxLength - index, "0x%02x, ", ((unsigned char*)buffer)[i]);

			}
			// New line after every 12th byte
			if (newLine && (i + 1) % 12 == 0 && i != length - 1) {
				if (index < maxLength) index += snprintf(message + index, maxLength - index, "\n    ");
			}
		}

		// Ending bracket
		if (index < maxLength) index += snprintf(message + index, maxLength - index, " }\n");

		// New line at the end when the message is larger than max length
		if (index >= maxLength) {
			message[maxLength - 1] = '\n';
			message[maxLength] = 0;
		}

		// Add message
		LogItem *logItem = new LogItem();
		localtime_s(&logItem->time, &t);
		GetSystemTime(&logItem->systemTime);
		logItem->level = level;
		logItem->module = module;
		logItem->message = message;
		AddQueue(logItem);
	}
}

//
// Add item to queue
//
void Logger::AddQueue(LogItem *item) {

	if (!IsRunning()) return;

	// Direct print (skips the message buffer)
	if (directPrint) {

		// Output log item
		OutputItem(item);

		// Destroy log item
		delete item;

		return;
	}

	// Lock message list
	lockQueue.lock();

	// Add message
	logQueue.push(item);

	// Unlock message list
	lockQueue.unlock();

	// Notify new message
	NotifyNewItem();

}

//
// Notify new item
//
void Logger::NotifyNewItem()
{
	conditionNewItem.notify_all();
}

//
// Wait for a new message
//
void Logger::WaitNewItem()
{
	std::unique_lock<std::mutex> mlock(lockNewItem);

	// Check if the queue is empty
	bool isQueueEmpty = false;
	lockQueue.lock();
	isQueueEmpty = logQueue.empty();
	lockQueue.unlock();

	// Wait for items if queue is empty
	if (isQueueEmpty) {
		conditionNewItem.wait(mlock);
	}
}


//
// Logger thread
//
void Logger::run() {

	while (true) {

		// Shutdown the thread
		if (!IsRunning()) break;

		// Wait for the new item
		WaitNewItem();

		// Process item queue
		ProcessQueue();
	}

	printf("Logger thread exit!\n");
}

//
// Open log file
//
bool Logger::OpenLogFile(std::string filename) {
	if (logFile && logFile.is_open()) {
		logFile.close();
	}
	logFile = std::ofstream(filename, std::ofstream::out);
	if (!logFile) {
		return false;
	}
	logFilename = filename;
	return true;
}

//
// Close log file
//
bool Logger::CloseLogFile() {
	if (logFile && logFile.is_open()) {
		logFile.close();
		return true;
	}
	return false;
}


//
// Enable/disable debug output
//
void Logger::SetDebugOutput(bool enabled)
{
	lock.lock();
	isDebugOutputEnabled = enabled;
	lock.unlock();

}

//
// Check if the debug output is enabled
//
bool Logger::IsDebugOutputEnabled()
{
	bool enabled = false;
	lock.lock();
	enabled = isDebugOutputEnabled;
	lock.unlock();
	return enabled;
}



//
// Start logger thread
//
void Logger::Start() {
	if (!IsRunning()) {
		SetRunningState(true);
		threadLog = std::thread([this] { this->run(); });
	}
}

//
// Stop logger thread
//
void Logger::Stop() {
	if (IsRunning()) {
		SetRunningState(false);

		NotifyNewItem();

		printf("Logger thread join\n");
		try {
			threadLog.join();
		}
		catch (std::exception &e) {
			printf("Logger thread join exception: %s\n", e.what());
		}

		ProcessQueue();
		if (logFile && logFile.is_open()) {
			logFile.close();
		}
	}
}

Logger logger;
