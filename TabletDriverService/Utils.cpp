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

void		Utils::keyboardShortcutPress(std::vector<int> const& keys)
{
	std::vector<INPUT> keyPress;

	for (auto it : keys)
	{
		INPUT ip;

		ip.type = INPUT_KEYBOARD;
		ip.ki.wScan = 0;
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;

		ip.ki.wVk = it;
		ip.ki.dwFlags = 0;

		keyPress.push_back(ip);
	}
	for (auto it : keyPress)
	{
		SendInput(1, &it, sizeof(INPUT));
	}
	for (int i = 0; i < keyPress.size(); ++i)
	{
		keyPress[i].ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &keyPress[i], sizeof(INPUT));
	}
}