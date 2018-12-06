#pragma once

#include <string>
#include <iomanip>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <algorithm>

using namespace std;

class CommandLine {
public:
	string line;
	string command;
	vector<string> values;
	int valueCount;
	bool isValid;

	CommandLine(string text);
	~CommandLine();
	bool is(string command);
	string GetCommandLowerCase();
	string GetParameterString();
	int Parse(string text);
	string ParseHex(string str);
	string GetString(int index, string defaultValue);
	string GetStringLower(int index, string defaultValue);
	int GetInt(int index, int defaultValue);
	long GetLong(int index, long defaultValue);
	double GetDouble(int index, double defaultValue);
	float GetFloat(int index, float defaultValue);
	bool GetBoolean(int index, bool defaultValue);
};

