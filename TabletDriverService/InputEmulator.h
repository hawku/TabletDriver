#pragma once

#include <map>

class InputEmulator
{
private:
	void CreateKeyMap();
	void CreateVirtualKeyMap();
public:

	class KeyMapValue {
	public:
		string name;
		WORD virtualKey;
		int mouseButton;
		KeyMapValue();
		KeyMapValue(string name, WORD key, int button);
		KeyMapValue(string name, WORD key);
	};


	map<string, KeyMapValue*> keyMap;
	vector<string> keys;

	InputEmulator();
	~InputEmulator();


	void AddKey(string key, string keyName, WORD virtualCode);
	void AddKey(string key, string keyName, WORD virtualCode, int button);
	WORD GetKeyCode(string key);
	
	void MouseSet(int button, bool down);
	void MousePress(int button, int time);
	void MouseMove(int x, int y);
	void MouseMoveTo(int x, int y);
	void MouseScroll(int delta, bool vertical);
	void SetKeyState(string key, bool down);
	void SetKeyState(WORD vkCode, bool down);
	void SetInputStates(string inputs, bool down);
	void KeyPress(string keys, int time);


};

