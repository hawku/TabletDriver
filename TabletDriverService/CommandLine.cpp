#include "precompiled.h"
#include "CommandLine.h"

#define LOG_MODULE "CommandLine"
#include "Logger.h"

//
// Constructor
//
CommandLine::CommandLine(std::string text) {
	this->line = text;
	this->Parse(text);
}

//
// Destructor
//
CommandLine::~CommandLine() {
}

//
// Command matcher
//
bool CommandLine::is(std::string command) {

	std::string match = this->command;
	std::transform(command.begin(), command.end(), command.begin(), ::tolower);
	std::transform(match.begin(), match.end(), match.begin(), ::tolower);

	if (command.compare(match) == 0) {
		return true;
	}
	return false;
}

//
// Get command in lower case
//
std::string CommandLine::GetCommandLowerCase() {
	std::transform(command.begin(), command.end(), command.begin(), ::tolower);
	return command;
}

//
// Get command parameters as as string
//
std::string CommandLine::GetParameterString() {
	if (command.size() < line.size())
		return line.substr(command.size() + 1);
	return "";
}


//
// Parse
//
int CommandLine::Parse(std::string line) {

	std::string item = "";
	std::vector<std::string> items;

	int lineLength = line.size();
	int itemLength = 0;
	int itemCount = 0;
	int index = 0;

	char currentChar;
	char previousChar = 0;
	char splitChars[] = " ,:(){}[]=";
	char endChars[] = { '\r', '\n', ';', 0 };
	char commentChar = '#';
	char escapeChar = '\\';
	char enclosingChar = '"';

	bool isSplitChar = false;
	bool isEndChar = false;
	bool isEnclosingChar = false;
	bool isLastChar = false;
	bool isEnclosed = false;

	for (std::string::iterator it = line.begin(); it != line.end(); ++it) {
		currentChar = *it;

		// Comment char
		if (!isEnclosed && currentChar == commentChar) {
			if (itemLength > 0) {
				items.push_back(item);
			}
			break;
		}

		// Is split char?
		isSplitChar = false;
		for (int i = 0; i < (int)sizeof(splitChars); i++) {
			if (splitChars[i] && currentChar == splitChars[i]) {
				isSplitChar = true;
				break;
			}
		}

		// Is end char?
		isEndChar = false;
		for (int i = 0; i < (int)sizeof(endChars); i++) {
			if (currentChar == endChars[i]) {
				isEndChar = true;
				break;
			}
		}

		// Is last char?
		isLastChar = false;
		if (index == lineLength - 1) {
			isLastChar = true;
		}


		// Toggle enclosing
		isEnclosingChar = false;
		if (currentChar == enclosingChar && previousChar != escapeChar) {
			isEnclosed = !isEnclosed;
			isEnclosingChar = true;
		}

		// New item
		if (
			!isEnclosed &&
			(
				isSplitChar ||
				isEndChar ||
				(itemCount == 0 && currentChar == '=') ||
				(itemCount == 1 && itemLength == 0 && currentChar == '=') ||
				isLastChar ||
				(isLastChar && isEnclosingChar)
				)
			) {

			//INFO("char: %c\n", currentChar);
			//INFO("itemCount = %d\n", itemCount);
			//INFO("itemLength = %d\n", itemLength);

			// Last char
			if (isLastChar && !isEndChar && !isSplitChar) {
				if (!isEnclosingChar) {
					item.push_back(currentChar);
					itemLength = 1;
				}
			}

			// Create new item
			if (itemLength > 0) {
				items.push_back(item);
				item = "";
				itemLength = 0;
				itemCount++;
			}
			else {
				item = "";
				itemLength = 0;
			}

			// Stop parsing at end of the line
			if (isEndChar) {
				break;
			}

			// Add text to item
		}
		else if (currentChar >= 32) {
			if (itemCount == 0 && currentChar == '=') {
			}
			else if (isEnclosingChar) {
			}
			else if (currentChar == escapeChar && !isEnclosed) {
			}
			else {
				item.push_back(currentChar);
				itemLength++;
			}
		}
		index++;
		previousChar = currentChar;
	}

	// Set command
	if (itemCount > 0) {
		command = items[0];
		isValid = true;
	}
	else {
		isValid = false;
	}

	// Set values
	values.clear();
	for (int i = 1; i < (int)items.size(); i++) {
		values.push_back(items[i]);
	}

	valueCount = values.size();
	return valueCount;
}


//
// Parse hex string
//
std::string CommandLine::ParseHex(std::string str) {
	if (str.size() >= 3 && str[0] == '0' && str[1] == 'x') {
		try {
			std::string tmp = str.substr(2, str.size() - 2);
			return std::to_string(stol(tmp, 0, 16));
		}
		catch (std::exception) {
		}
	}
	return str;
}

//
// Parse bit string (e.g. 0b10100101)
//
std::string CommandLine::ParseBits(std::string str) {
	if (str.size() >= 3 && str[0] == '0' && str[1] == 'b') {
		int length = str.size();
		unsigned int output = 0;
		try {
			for (int i = 2; i < length; i++) {
				char c = str.at(i);
				if (c == '1') {
					output |= 1 << (length - (i + 1));
				}
			}
			return std::to_string(output);
		}
		catch (std::exception) {
		}
	}
	return str;
}

//
// Parse hex and bits strings
//
std::string CommandLine::ParseHexBits(std::string str)
{
	return ParseBits(ParseHex(str));
}


//
// Get string value
//
std::string CommandLine::GetString(int index, std::string defaultValue) {
	if (index < valueCount) {
		return values[index];
	}
	return defaultValue;
}


//
// Get lowercase string value
//
std::string CommandLine::GetStringLower(int index, std::string defaultValue) {
	std::string str = GetString(index, defaultValue);
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}


//
// Get integer
//
int CommandLine::GetInt(int index, int defaultValue) {
	if (index < valueCount) {
		try {
			auto value = stoi(ParseHexBits(values[index]));
			return value;
		}
		catch (std::exception) {}
	}
	return defaultValue;
}


//
// Get long integer
//
long CommandLine::GetLong(int index, long defaultValue) {
	if (index < valueCount) {
		try {
			auto value = stol(ParseHexBits(values[index]));
			return value;
		}
		catch (std::exception) {}
	}
	return defaultValue;
}


//
// Get double precision floating point number
//
double CommandLine::GetDouble(int index, double defaultValue) {
	if (index < valueCount) {
		try {
			auto value = stod(ParseHexBits(values[index]));
			return value;
		}
		catch (std::exception) {}
	}
	return defaultValue;
}


//
// Get floating point number
//
float CommandLine::GetFloat(int index, float defaultValue) {
	if (index < valueCount) {
		try {
			auto value = stof(ParseHexBits(values[index]));
			return value;
		}
		catch (std::exception) {}
	}
	return defaultValue;
}


//
// Get boolean
//
bool CommandLine::GetBoolean(int index, bool defaultValue) {
	if (GetInt(index, 0) > 0) return true;
	std::string str = GetStringLower(index, "");
	if (str == "true") return true;
	if (str == "on") return true;
	if (str == "false") return false;
	if (str == "off") return false;
	if (str == "0") return false;
	return defaultValue;
}
