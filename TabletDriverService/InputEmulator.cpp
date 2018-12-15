#include "stdafx.h"
#include "InputEmulator.h"

#define LOG_MODULE "InputEmulator"
#include "Logger.h"

InputEmulator::InputEmulator()
{
	CreateKeyMap();
	CreateVirtualKeyMap();
}


InputEmulator::~InputEmulator()
{
}

//
// Create key map
//
void InputEmulator::CreateKeyMap()
{
	// Disabled
	AddKey("DISABLED", "Disabled", 0, 0);

	// Mouse buttons
	AddKey("MOUSE1", "Mouse 1", 0, 1);
	AddKey("MOUSE2", "Mouse 2", 0, 2);
	AddKey("MOUSE3", "Mouse 3", 0, 3);
	AddKey("MOUSE4", "Mouse 4", 0, 4);
	AddKey("MOUSE5", "Mouse 5", 0, 5);

	// Mouse scroll
	AddKey("MOUSESCROLLV", "Mouse Scroll Vertical", 0, 0x101);
	AddKey("MOUSESCROLLH", "Mouse Scroll Horizontal", 0, 0x102);
	AddKey("MOUSESCROLLB", "Mouse Scroll Both", 0, 0x103);

	// Shift
	AddKey("SHIFT", "Shift", VK_SHIFT);
	AddKey("LSHIFT", "Left shift", VK_LSHIFT);
	AddKey("RSHIFT", "Right shift", VK_RSHIFT);

	// Control
	AddKey("CTRL", "Control", VK_CONTROL);
	AddKey("LCTRL", "Left control", VK_LCONTROL);
	AddKey("RCTRL", "Right control", VK_RCONTROL);

	// Alt
	AddKey("ALT", "Alt", VK_MENU);
	AddKey("LALT", "Left alt", VK_LMENU);
	AddKey("RALT", "Right alt", VK_RMENU);

	// Windows keys
	AddKey("LWIN", "Left Windows key", VK_LWIN);
	AddKey("RWIN", "Right Windows key", VK_RWIN);
	AddKey("MENU", "Windows menu key", VK_APPS);

	// 
	AddKey("BACK", "Backspace", VK_BACK);
	AddKey("TAB", "Tab", VK_TAB);
	AddKey("ENTER", "Enter", VK_RETURN);
	AddKey("ESC", "Escape", VK_ESCAPE);
	AddKey("SPACE", "Space", VK_SPACE);
	AddKey("PAGEUP", "Page up", VK_PRIOR);
	AddKey("PAGEDOWN", "Page down", VK_NEXT);
	AddKey("END", "End", VK_END);
	AddKey("HOME", "Home", VK_HOME);
	AddKey("DEL", "Delete", VK_DELETE);
	AddKey("INS", "Insert", VK_INSERT);
	AddKey("CAPSLOCK", "Caps lock", VK_CAPITAL);
	AddKey("NUMLOCK", "Numlock", VK_NUMLOCK);
	AddKey("SCROLLLOCK", "Scroll lock", VK_SCROLL);

	// Arrow keys
	AddKey("UP", "Up", VK_UP);
	AddKey("DOWM", "Down", VK_DOWN);
	AddKey("LEFT", "Left", VK_LEFT);
	AddKey("RIGHT", "Right", VK_RIGHT);

	// Function keys
	AddKey("F1", "F1", VK_F1);
	AddKey("F2", "F2", VK_F2);
	AddKey("F3", "F3", VK_F3);
	AddKey("F4", "F4", VK_F4);
	AddKey("F5", "F5", VK_F5);
	AddKey("F6", "F6", VK_F6);
	AddKey("F7", "F7", VK_F7);
	AddKey("F8", "F8", VK_F8);
	AddKey("F9", "F9", VK_F9);
	AddKey("F10", "F10", VK_F10);
	AddKey("F11", "F11", VK_F11);
	AddKey("F12", "F12", VK_F12);
	AddKey("F13", "F13", VK_F13);
	AddKey("F14", "F14", VK_F14);
	AddKey("F15", "F15", VK_F15);
	AddKey("F16", "F16", VK_F16);
	AddKey("F17", "F17", VK_F17);
	AddKey("F18", "F18", VK_F18);
	AddKey("F19", "F19", VK_F19);
	AddKey("F20", "F20", VK_F20);
	AddKey("F21", "F21", VK_F21);
	AddKey("F22", "F22", VK_F22);
	AddKey("F23", "F23", VK_F23);
	AddKey("F24", "F24", VK_F24);

	// Numbpad
	AddKey("NUMPAD0", "Numpad 0", VK_NUMPAD0);
	AddKey("NUMPAD1", "Numpad 1", VK_NUMPAD1);
	AddKey("NUMPAD2", "Numpad 2", VK_NUMPAD2);
	AddKey("NUMPAD3", "Numpad 3", VK_NUMPAD3);
	AddKey("NUMPAD4", "Numpad 4", VK_NUMPAD4);
	AddKey("NUMPAD5", "Numpad 5", VK_NUMPAD5);
	AddKey("NUMPAD6", "Numpad 6", VK_NUMPAD6);
	AddKey("NUMPAD7", "Numpad 7", VK_NUMPAD7);
	AddKey("NUMPAD8", "Numpad 8", VK_NUMPAD8);
	AddKey("NUMPAD9", "Numpad 9", VK_NUMPAD9);

	// Math
	AddKey("MULTIPLY", "Multiply (*)", VK_MULTIPLY);
	AddKey("DIVIDE", "Divide (/)", VK_DIVIDE);
	AddKey("PLUS", "Plus (+)", VK_OEM_PLUS);
	AddKey("MINUS", "Minus (-)", VK_OEM_MINUS);
	AddKey("COMMA", "Comma (,)", VK_OEM_COMMA);
	AddKey("PERIOD", "Period (.)", VK_OEM_PERIOD);

	// Media keys
	AddKey("VOLUMEUP", "Volume up", VK_VOLUME_UP);
	AddKey("VOLUMEDOWN", "Volume down", VK_VOLUME_DOWN);
	AddKey("VOLUMEMUTE", "Volume mute", VK_VOLUME_MUTE);
	AddKey("MEDIANEXT", "Media next", VK_MEDIA_NEXT_TRACK);
	AddKey("MEDIAPREV", "Media previous", VK_MEDIA_PREV_TRACK);
	AddKey("MEDIASTOP", "Media stop", VK_MEDIA_STOP);
	AddKey("MEDIAPLAY", "Media play/pause", VK_MEDIA_PLAY_PAUSE);

}

//
// Create virtual key map
//
void InputEmulator::CreateVirtualKeyMap() {
	AddKey("BACK", "Back", 8);
	AddKey("TAB", "Tab", 9);
	AddKey("LINEFEED", "LineFeed", 10);
	AddKey("CLEAR", "Clear", 12);
	AddKey("ENTER", "Enter", 13);
	AddKey("SHIFTKEY", "ShiftKey", 16);
	AddKey("CONTROLKEY", "ControlKey", 17);
	AddKey("MENU", "Menu", 18);
	AddKey("PAUSE", "Pause", 19);
	AddKey("CAPITAL", "Capital", 20);
	AddKey("KANAMODE", "KanaMode", 21);
	AddKey("JUNJAMODE", "JunjaMode", 23);
	AddKey("FINALMODE", "FinalMode", 24);
	AddKey("HANJAMODE", "HanjaMode", 25);
	AddKey("ESCAPE", "Escape", 27);
	AddKey("IMECONVERT", "IMEConvert", 28);
	AddKey("IMENONCONVERT", "IMENonconvert", 29);
	AddKey("IMEACEEPT", "IMEAceept", 30);
	AddKey("IMEMODECHANGE", "IMEModeChange", 31);
	AddKey("SPACE", "Space", 32);
	AddKey("PGUP", "PgUp", 33);
	AddKey("PGDN", "PgDn", 34);
	AddKey("END", "End", 35);
	AddKey("HOME", "Home", 36);
	AddKey("LEFT", "Left", 37);
	AddKey("UP", "Up", 38);
	AddKey("RIGHT", "Right", 39);
	AddKey("DOWN", "Down", 40);
	AddKey("SELECT", "Select", 41);
	AddKey("PRINT", "Print", 42);
	AddKey("EXECUTE", "Execute", 43);
	AddKey("PRINTSCREEN", "PrintScreen", 44);
	AddKey("INS", "Ins", 45);
	AddKey("DEL", "Del", 46);
	AddKey("HELP", "Help", 47);
	AddKey("0", "0", 48);
	AddKey("1", "1", 49);
	AddKey("2", "2", 50);
	AddKey("3", "3", 51);
	AddKey("4", "4", 52);
	AddKey("5", "5", 53);
	AddKey("6", "6", 54);
	AddKey("7", "7", 55);
	AddKey("8", "8", 56);
	AddKey("9", "9", 57);
	AddKey("A", "A", 65);
	AddKey("B", "B", 66);
	AddKey("C", "C", 67);
	AddKey("D", "D", 68);
	AddKey("E", "E", 69);
	AddKey("F", "F", 70);
	AddKey("G", "G", 71);
	AddKey("H", "H", 72);
	AddKey("I", "I", 73);
	AddKey("J", "J", 74);
	AddKey("K", "K", 75);
	AddKey("L", "L", 76);
	AddKey("M", "M", 77);
	AddKey("N", "N", 78);
	AddKey("O", "O", 79);
	AddKey("P", "P", 80);
	AddKey("Q", "Q", 81);
	AddKey("R", "R", 82);
	AddKey("S", "S", 83);
	AddKey("T", "T", 84);
	AddKey("U", "U", 85);
	AddKey("V", "V", 86);
	AddKey("W", "W", 87);
	AddKey("X", "X", 88);
	AddKey("Y", "Y", 89);
	AddKey("Z", "Z", 90);
	AddKey("LWIN", "LWin", 91);
	AddKey("RWIN", "RWin", 92);
	AddKey("APPS", "Apps", 93);
	AddKey("SLEEP", "Sleep", 95);
	AddKey("NUMPAD0", "NumPad0", 96);
	AddKey("NUMPAD1", "NumPad1", 97);
	AddKey("NUMPAD2", "NumPad2", 98);
	AddKey("NUMPAD3", "NumPad3", 99);
	AddKey("NUMPAD4", "NumPad4", 100);
	AddKey("NUMPAD5", "NumPad5", 101);
	AddKey("NUMPAD6", "NumPad6", 102);
	AddKey("NUMPAD7", "NumPad7", 103);
	AddKey("NUMPAD8", "NumPad8", 104);
	AddKey("NUMPAD9", "NumPad9", 105);
	AddKey("MULTIPLY", "Multiply", 106);
	AddKey("ADD", "Add", 107);
	AddKey("SEPARATOR", "Separator", 108);
	AddKey("SUBTRACT", "Subtract", 109);
	AddKey("DECIMAL", "Decimal", 110);
	AddKey("DIVIDE", "Divide", 111);
	AddKey("F1", "F1", 112);
	AddKey("F2", "F2", 113);
	AddKey("F3", "F3", 114);
	AddKey("F4", "F4", 115);
	AddKey("F5", "F5", 116);
	AddKey("F6", "F6", 117);
	AddKey("F7", "F7", 118);
	AddKey("F8", "F8", 119);
	AddKey("F9", "F9", 120);
	AddKey("F10", "F10", 121);
	AddKey("F11", "F11", 122);
	AddKey("F12", "F12", 123);
	AddKey("F13", "F13", 124);
	AddKey("F14", "F14", 125);
	AddKey("F15", "F15", 126);
	AddKey("F16", "F16", 127);
	AddKey("F17", "F17", 128);
	AddKey("F18", "F18", 129);
	AddKey("F19", "F19", 130);
	AddKey("F20", "F20", 131);
	AddKey("F21", "F21", 132);
	AddKey("F22", "F22", 133);
	AddKey("F23", "F23", 134);
	AddKey("F24", "F24", 135);
	AddKey("NUMLOCK", "NumLock", 144);
	AddKey("SCROLL", "Scroll", 145);
	AddKey("LSHIFTKEY", "LShiftKey", 160);
	AddKey("RSHIFTKEY", "RShiftKey", 161);
	AddKey("LCONTROLKEY", "LControlKey", 162);
	AddKey("RCONTROLKEY", "RControlKey", 163);
	AddKey("LMENU", "LMenu", 164);
	AddKey("RMENU", "RMenu", 165);
	AddKey("BROWSERBACK", "BrowserBack", 166);
	AddKey("BROWSERFORWARD", "BrowserForward", 167);
	AddKey("BROWSERREFRESH", "BrowserRefresh", 168);
	AddKey("BROWSERSTOP", "BrowserStop", 169);
	AddKey("BROWSERSEARCH", "BrowserSearch", 170);
	AddKey("BROWSERFAVORITES", "BrowserFavorites", 171);
	AddKey("BROWSERHOME", "BrowserHome", 172);
	AddKey("VOLUMEMUTE", "VolumeMute", 173);
	AddKey("VOLUMEDOWN", "VolumeDown", 174);
	AddKey("VOLUMEUP", "VolumeUp", 175);
	AddKey("MEDIANEXTTRACK", "MediaNextTrack", 176);
	AddKey("MEDIAPREVIOUSTRACK", "MediaPreviousTrack", 177);
	AddKey("MEDIASTOP", "MediaStop", 178);
	AddKey("MEDIAPLAYPAUSE", "MediaPlayPause", 179);
	AddKey("LAUNCHMAIL", "LaunchMail", 180);
	AddKey("SELECTMEDIA", "SelectMedia", 181);
	AddKey("LAUNCHAPPLICATION1", "LaunchApplication1", 182);
	AddKey("LAUNCHAPPLICATION2", "LaunchApplication2", 183);
	AddKey("OEM1", "Oem1", 186);
	AddKey("OEMPLUS", "Oemplus", 187);
	AddKey("OEMCOMMA", "Oemcomma", 188);
	AddKey("OEMMINUS", "OemMinus", 189);
	AddKey("OEMPERIOD", "OemPeriod", 190);
	AddKey("OEMQUESTION", "OemQuestion", 191);
	AddKey("OEMTILDE", "Oemtilde", 192);
	AddKey("OEMOPENBRACKETS", "OemOpenBrackets", 219);
	AddKey("OEM5", "Oem5", 220);
	AddKey("OEM6", "Oem6", 221);
	AddKey("OEM7", "Oem7", 222);
	AddKey("OEM8", "Oem8", 223);
	AddKey("OEMBACKSLASH", "OemBackslash", 226);
	AddKey("PROCESSKEY", "ProcessKey", 229);
	AddKey("PACKET", "Packet", 231);
	AddKey("ATTN", "Attn", 246);
	AddKey("CRSEL", "Crsel", 247);
	AddKey("EXSEL", "Exsel", 248);
	AddKey("ERASEEOF", "EraseEof", 249);
	AddKey("PLAY", "Play", 250);
	AddKey("ZOOM", "Zoom", 251);
	AddKey("NONAME", "NoName", 252);
	AddKey("PA1", "Pa1", 253);
	AddKey("OEMCLEAR", "OemClear", 254);

}


//
// Add key to key map
//
void InputEmulator::AddKey(string key, string keyName, WORD virtualCode)
{
	AddKey(key, keyName, virtualCode, 0);
}


//
// Add key to key map 
//
void InputEmulator::AddKey(string key, string keyName, WORD virtualCode, int button)
{
	std::transform(key.begin(), key.end(), key.begin(), ::toupper);
	if(keyMap.count(key) <= 0) {
		keyMap[key] = new KeyMapValue(keyName, virtualCode, button);
		keys.push_back(key);
	}
}


//
// Get keycode
//
WORD InputEmulator::GetKeyCode(string key) {

	WORD vkCode = 0;

	// Convert the key string to uppercase
	std::transform(key.begin(), key.end(), key.begin(), ::toupper);

	if(keyMap.count(key) > 0 && keyMap[key]->virtualKey > 0) {
		vkCode = keyMap[key]->virtualKey;
	}

	return vkCode;
}


//
// Set mouse button state
//
void InputEmulator::MouseSet(int button, bool down) {
	INPUT input = { 0 };

	input.type = INPUT_MOUSE;
	input.mi.mouseData = 0;
	input.mi.dx = 0;
	input.mi.dy = 0;
	if(button == 1) {
		if(down)
			input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		else
			input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	}
	else if(button == 2) {
		if(down)
			input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
		else
			input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	}
	else if(button == 3) {
		if(down)
			input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
		else
			input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
	}
	SendInput(1, &input, sizeof(INPUT));
}


//
// Press mouse button
//
void InputEmulator::MousePress(int button, int time) {
	MouseSet(button, true);
	Sleep(time);
	MouseSet(button, false);
}


//
// Mouse scroll
//
void InputEmulator::MouseScroll(int delta, bool vertical) {

	INPUT input = { 0 };

	input.type = INPUT_MOUSE;
	input.mi.dx = 0;
	input.mi.dy = 0;
	input.mi.time = 0;
	input.mi.dwExtraInfo = 0;

	// Vertical or horizontal?
	if(vertical) {
		input.mi.dwFlags = MOUSEEVENTF_WHEEL;
	}
	else {
		input.mi.dwFlags = MOUSEEVENTF_HWHEEL;
	}

	// 120 == One scroll
	input.mi.mouseData = delta * 120;
	SendInput(1, &input, sizeof(INPUT));
}


//
// Relative mouse move
//
void InputEmulator::MouseMove(int x, int y) {
	INPUT input = { 0 };

	input.type = INPUT_MOUSE;
	input.mi.mouseData = 0;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	SendInput(1, &input, sizeof(INPUT));

}


//
// Absolute mouse move
//
void InputEmulator::MouseMoveTo(int x, int y) {
	INPUT input = { 0 };

	input.type = INPUT_MOUSE;
	input.mi.mouseData = 0;
	input.mi.dx = (x * 65536) / GetSystemMetrics(SM_CXSCREEN);
	input.mi.dy = (y * 65536) / GetSystemMetrics(SM_CYSCREEN);
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	SendInput(1, &input, sizeof(INPUT));

}


//
// Set keyboard key state with key name
//
void InputEmulator::SetKeyState(string key, bool down) {
	INPUT input = { 0 };
	WORD vkCode = 0;
	vkCode = GetKeyCode(key);
	SetKeyState(vkCode, down);
}


//
// Set keyboard key state with virtual code
//
void InputEmulator::SetKeyState(WORD vkCode, bool down)
{
	INPUT input = { 0 };
	if(down) {
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = vkCode;
		SendInput(1, &input, sizeof(INPUT));
	}
	else {
		input.type = INPUT_KEYBOARD;
		input.ki.dwFlags = KEYEVENTF_KEYUP;
		input.ki.wVk = vkCode;
		SendInput(1, &input, sizeof(INPUT));
	}
}


//
// Set mouse or keyboard input states
//
void InputEmulator::SetInputStates(string inputs, bool down)
{
	string input;
	std::transform(inputs.begin(), inputs.end(), inputs.begin(), ::toupper);
	stringstream stringStream(inputs);
	KeyMapValue *keyMapValue;

	while(getline(stringStream, input, '+')) {

		// Key in map?
		if(keyMap.count(input) > 0) {

			keyMapValue = keyMap[input];

			// Virtual key?
			if(keyMapValue->virtualKey > 0) {

				// Set keyboard key state
				SetKeyState(keyMapValue->virtualKey, down);
			}

			// Mouse button?
			else if(keyMapValue->mouseButton > 0) {

				// Set mouse button state
				MouseSet(keyMapValue->mouseButton, down);
			}
		}
	}

}


//
// Press keyboard key
//
void InputEmulator::KeyPress(string keys, int time) {
	SetInputStates(keys, true);
	Sleep(time);
	SetInputStates(keys, false);
}


//
// Key map value constructors
//
InputEmulator::KeyMapValue::KeyMapValue()
{
}
InputEmulator::KeyMapValue::KeyMapValue(string name, WORD key, int button)
{
	this->name = name;
	this->virtualKey = key;
	this->mouseButton = button;
}
InputEmulator::KeyMapValue::KeyMapValue(string name, WORD key)
{
	this->name = name;
	this->virtualKey = key;
	this->mouseButton = 0;
}
