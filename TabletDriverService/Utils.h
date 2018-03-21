#pragma once

#include <vector>
#include <string>
#include <sstream>

class Utils
{
public:
	Utils();
	~Utils();

	template<typename Out>
	static void							split(const std::string &s, char delim, Out result, bool addEmpty = false);
	static std::vector<std::string>		split(const std::string &s, char delim, bool addEmpty = false);

	static std::string					join(std::vector<std::string>, std::string);
};

