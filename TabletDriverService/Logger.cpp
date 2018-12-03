#include "stdafx.h"
#include "Logger.h"

//
// Constructor
//
Logger::Logger() {
	verbosity = LogLevelDebug;
	newMessage = false;
	directPrint = false;
	debugEnabled = false;
}


void Logger::OutputMessage(LogItem *message) {
	char timeBuffer[64];
	char moduleBuffer[128];

	strftime(timeBuffer, 64, "%Y-%m-%d %H:%M:%S", &message->time);
	if(message->module.length() > 0) {
		sprintf_s(moduleBuffer, " [%s]", message->module.c_str());
	} else {
		moduleBuffer[0] = 0;
	}

	try {
		cout << timeBuffer << moduleBuffer << " [" << levelNames[message->level] << "] " << message->text << flush;
	} catch(exception) {
		exit(1);
	}
	if(logFile && logFile.is_open()) {
		logFile << timeBuffer << moduleBuffer << " [" << levelNames[message->level] << "] " << message->text << flush;
	}
}

//
// Process messages
//
void Logger::ProcessMessages() {

	// Lock message list
	lockMessages.lock();

	// Copy message list to a temporary list
	vector<LogItem> tmp(messages);
	messages.clear();

	// Unlock message list
	lockMessages.unlock();

	// Loop through messages
	for(auto message : tmp) {
		if(!directPrint) {
			OutputMessage(&message);
		}
	}
}


//
// Log message
//
void Logger::LogMessage(int level, string module, const char *fmt, ...) {
	char message[4096];
	int maxLength = sizeof(message) - 1;
	int index;
	time_t t;
	message[0] = 0;

	// Clamp level
	if(level < 2)
		level = 2;
	else if(level > 8)
		level = 8;

	if(level <= verbosity) {
		index = 0;
		time(&t);

		// Loop through arguments
		va_list ap;
		va_start(ap, fmt);
		if(index < maxLength) index += vsnprintf(message + index, maxLength - index, fmt, ap);
		va_end(ap);

		// New line at the end when the message is larger than max length
		if(index >= maxLength) {
			message[maxLength - 1] = '\n';
			message[maxLength] = 0;
		}

		// Add message
		LogItem logItem;
		localtime_s(&logItem.time, &t);
		logItem.level = level;
		logItem.module = module;
		logItem.text = message;
		AddMessage(&logItem);
	}
}

//
// Log buffer data
//
void Logger::LogBuffer(int level, string module, void *buffer, int length, const char *fmt, ...) {
	bool newLine = false;
	char message[4096];
	int maxLength = sizeof(message) - 1;
	time_t t;
	int index;
	message[0] = 0;

	// Clamp level
	if(level < 2)
		level = 2;
	else if(level > 8)
		level = 8;

	if(level <= verbosity) {
		index = 0;
		time(&t);

		// Loop through arguments
		va_list ap;
		va_start(ap, fmt);
		if(index < maxLength) index += vsnprintf(message + index, maxLength - index, fmt, ap);
		va_end(ap);

		// Detect new line
		newLine = (fmt[strlen(fmt) - 1] == '\n');
		if(newLine) {
			if(index < maxLength) index += snprintf(message + index, maxLength - index, "  { ");

		} else {
			if(index < maxLength) index += snprintf(message + index, maxLength - index, "{ ");
		}


		// Loop through buffer bytes
		for(int i = 0; i < length; i++) {

			// Last byte
			if(i == length - 1) {
				if(index < maxLength) index += snprintf(message + index, maxLength - index, "0x%02x", ((unsigned char*)buffer)[i]);

				//
			} else {
				if(index < maxLength) index += snprintf(message + index, maxLength - index, "0x%02x, ", ((unsigned char*)buffer)[i]);

			}
			// New line after every 12th byte
			if(newLine && (i + 1) % 12 == 0 && i != length - 1) {
				if(index < maxLength) index += snprintf(message + index, maxLength - index, "\n    ");
			}
		}

		// Ending bracket
		if(index < maxLength) index += snprintf(message + index, maxLength - index, " }\n");

		// New line at the end when the message is larger than max length
		if(index >= maxLength) {
			message[maxLength - 1] = '\n';
			message[maxLength] = 0;
		}

		// Add message
		LogItem logItem;
		localtime_s(&logItem.time, &t);
		logItem.level = level;
		logItem.module = module;
		logItem.text = message;
		AddMessage(&logItem);
	}
}

//
// Add message to list
//
void Logger::AddMessage(LogItem *message) {

	// Direct print (skips the message buffer)
	if(directPrint) {
		OutputMessage(message);
	}

	// Lock message list
	lockMessages.lock();

	// Add message
	messages.push_back(*message);

	// Unlock message list
	lockMessages.unlock();

	newMessage = true;
}


//
// Logger thread
//
void Logger::run() {
	while(true) {

		// Wait for messages
		if(newMessage) {

		// Set new message flag to false
			newMessage = false;

			// Process messages
			ProcessMessages();
		}

		// Shutdown the thread
		if(!isRunning && !newMessage) break;

		// Sleep 10ms
		Sleep(10);
	}
}

//
// Open log file
//
bool Logger::OpenLogFile(string filename) {
	if(logFile && logFile.is_open()) {
		logFile.close();
	}
	logFile = ofstream(filename, ofstream::out);
	if(!logFile) {
		return false;
	}
	logFilename = filename;
	return true;
}

//
// Close log file
//
bool Logger::CloseLogFile() {
	if(logFile && logFile.is_open()) {
		logFile.close();
		return true;
	}
	return false;
}


//
// Start logger thread
//
void Logger::Start() {
	if(!isRunning) {
		isRunning = true;
		threadLog = thread([this] { this->run(); });
	}
}

//
// Stop logger thread
//
void Logger::Stop() {
	if(isRunning) {
		isRunning = false;
		newMessage = true;
		threadLog.join();
		if(logFile && logFile.is_open()) {
			logFile.close();
		}
	}
}

Logger logger;
