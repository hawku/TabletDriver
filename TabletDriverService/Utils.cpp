#include "stdafx.h"
#include "Utils.h"


Utils::Utils()
{

}


Utils::~Utils()
{

}

template<typename Out>
void Utils::split(const std::string &s, char delim, Out result, bool addEmpty) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		if (item != "" || addEmpty)
			*(result++) = item;
	}
}

std::vector<std::string> Utils::split(const std::string &s, char delim, bool addEmpty) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems), addEmpty);
	return elems;
}

std::string   Utils::join(std::vector<std::string> v, std::string j) {
	std::string result = "";
	int count = 0;
	int size = v.size();
	for (auto it = v.begin(); it != v.end(); it++) {
		result += *it;
		if (count < size - 1)
			result += j;
		count++;
	}
	return result;
}