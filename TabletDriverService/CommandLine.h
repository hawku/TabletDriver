#pragma once

#include <string>
#include <iomanip>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <algorithm>


class CommandLine {
public:
	std::string line;
	std::string command;
	std::vector<std::string> values;
	int valueCount;
	bool isValid;

	CommandLine(std::string text);
	~CommandLine();
	bool is(std::string command);
	std::string GetCommandLowerCase();
	std::string GetParameterString();
	int Parse(std::string text);
	std::string ParseHex(std::string str);
	std::string ParseBits(std::string str);
	std::string ParseHexBits(std::string str);
	std::string GetString(int index, std::string defaultValue);
	std::string GetStringLower(int index, std::string defaultValue);
	int GetInt(int index, int defaultValue);
	long GetLong(int index, long defaultValue);
	double GetDouble(int index, double defaultValue);
	float GetFloat(int index, float defaultValue);
	bool GetBoolean(int index, bool defaultValue);
};

