#include "precompiled.h"
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
	if (isLowLatencyAudioForced && pAudioClient3 != NULL) {
		try {
			pAudioClient3->Stop();
			SAFE_RELEASE(pAudioClient3);
			printf("Low latency audio client stopped!\n");
		}
		catch (std::exception &e) {
			printf("ERROR: Couldn't stop low latency audio client! %s\n", e.what());
			SAFE_RELEASE(pAudioClient3);
		}
		isLowLatencyAudioForced = false;
	}
}

//
// Create key map
//
void InputEmulator::CreateKeyMap()
{
	// Disabled
	AddKey("DISABLED", "Disabled", 0);

	// Mouse buttons
	AddMouse("MOUSE1", "Mouse 1", MouseButtons::Mouse1);
	AddMouse("MOUSE2", "Mouse 2", MouseButtons::Mouse2);
	AddMouse("MOUSE3", "Mouse 3", MouseButtons::Mouse3);
	AddMouse("MOUSE4", "Mouse 4", MouseButtons::Mouse4);
	AddMouse("MOUSE5", "Mouse 5", MouseButtons::Mouse5);

	// Mouse scroll
	AddMouse("SCROLLUP", "Mouse Scroll Up", MouseButtons::MouseScrollUp);
	AddMouse("SCROLLDOWN", "Mouse Scroll Down", MouseButtons::MouseScrollDown);
	AddMouse("SCROLLLEFT", "Mouse Scroll Left", MouseButtons::MouseScrollLeft);
	AddMouse("SCROLLRIGHT", "Mouse Scroll Right", MouseButtons::MouseScrollRight);


	// Mouse scroll
	AddMouse("MOUSESCROLLV", "Mouse Scroll Vertical", MouseButtons::MouseScrollVertical);
	AddMouse("MOUSESCROLLH", "Mouse Scroll Horizontal", MouseButtons::MouseScrollHorizontal);
	AddMouse("MOUSESCROLLB", "Mouse Scroll Both", MouseButtons::MouseScrollBoth);

	// Media volume control
	inputMap["VOLUMECONTROL"] = new InputAction(InputActionType::ActionTypeAudioVolumeControl, "Media Volume Control");
	inputMap["BALANCECONTROL"] = new InputAction(InputActionType::ActionTypeAudioBalanceControl, "Media Balance Control");

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
// Add key to input map 
//
void InputEmulator::AddKey(std::string inputName, std::string description, WORD virtualCode)
{
	std::transform(inputName.begin(), inputName.end(), inputName.begin(), ::toupper);
	if (inputMap.count(inputName) <= 0) {
		inputMap[inputName] = new InputAction(InputActionType::ActionTypeKeyboard, description);
		inputMap[inputName]->virtualKey = virtualCode;
		inputs.push_back(inputName);
	}
}


//
// Add mouse action to input map 
//
void InputEmulator::AddMouse(std::string inputName, std::string description, int button)
{
	std::transform(inputName.begin(), inputName.end(), inputName.begin(), ::toupper);
	if (inputMap.count(inputName) <= 0) {
		inputMap[inputName] = new InputAction(InputActionType::ActionTypeMouse, description);
		inputMap[inputName]->mouseButton = button;
		inputs.push_back(inputName);
	}
}



//
// Get keycode
//
WORD InputEmulator::GetKeyCode(std::string key) {

	WORD vkCode = 0;

	// Convert the key string to uppercase
	std::transform(key.begin(), key.end(), key.begin(), ::toupper);

	if (inputMap.count(key) > 0 && inputMap[key]->virtualKey > 0) {
		vkCode = inputMap[key]->virtualKey;
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
	if (button == 1) {
		if (down)
			input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		else
			input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	}
	else if (button == 2) {
		if (down)
			input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
		else
			input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	}
	else if (button == 3) {
		if (down)
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
	if (vertical) {
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
void InputEmulator::SetKeyState(std::string key, bool down) {
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
	if (down) {
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
void InputEmulator::SetInputStates(std::string inputs, bool down)
{
	InputActionCollection collection;
	collection.emulator = this;
	collection.Add(inputs);
	collection.Execute(true, true, down);
}


//
// Press keyboard key
//
void InputEmulator::KeyPress(std::string keys, int time) {
	SetInputStates(keys, true);
	Sleep(time);
	SetInputStates(keys, false);
}


//
// Create audio endpoint volume
//
bool InputEmulator::CreateEndpointVolume()
{
	HRESULT hresult;

	try {
		// Device enumerator
		hresult = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDeviceEnumerator));
		CHECK_HRESULT(hresult, "MMDeviceEnumerator");

		// Default audio endpoint
		hresult = pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDefaultAudioDevice);
		CHECK_HRESULT(hresult, "GetDefaultAudioEndpoint");

		// Endpoint volume
		hresult = pDefaultAudioDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&pAudioEndpointVolume);
		CHECK_HRESULT(hresult, "IAudioEndpointVolume");
	}
	catch (std::exception &e) {
		LOG_ERROR("Couldn't create audio endpoint volume! Error at %s\n", e.what());
		SAFE_RELEASE(pDeviceEnumerator);
		SAFE_RELEASE(pDefaultAudioDevice);
		SAFE_RELEASE(pAudioEndpointVolume);
		return false;
	}
	return true;
}

//
// Release audio endpoint volume
//
bool InputEmulator::ReleaseEndpointVolume()
{
	SAFE_RELEASE(pAudioEndpointVolume);
	SAFE_RELEASE(pDefaultAudioDevice);
	SAFE_RELEASE(pDeviceEnumerator);
	return true;
}

//
// Set main audio device volume
//
void InputEmulator::VolumeSet(float volume)
{
	// Lock audio control
	std::unique_lock<std::mutex> mlock(lockAudio);

	// Get endpoint volume
	if (CreateEndpointVolume()) {
		if (volume < 0) volume = 0.0f;
		else if (volume > 1) volume = 1.0f;
		pAudioEndpointVolume->SetMasterVolumeLevelScalar(volume, &GUID_NULL);
	}
	ReleaseEndpointVolume();
}

//
// Set main audio device left/right balance
//
void InputEmulator::VolumeBalance(float newBalance)
{
	// Lock audio control
	std::unique_lock<std::mutex> mlock(lockAudio);

	UINT channelCount = 0;
	float levelLeft = -1;
	float levelRight = -1;

	// Get endpoint volume
	if (CreateEndpointVolume()) {

		// Channel count
		try {
			pAudioEndpointVolume->GetChannelCount(&channelCount);
			if (channelCount >= 2) {

				// Get channel levels
				pAudioEndpointVolume->GetChannelVolumeLevelScalar(0, &levelLeft);
				pAudioEndpointVolume->GetChannelVolumeLevelScalar(1, &levelRight);

				// Levels valid?
				if (levelLeft >= 0 || levelRight >= 0) {

					// Calculate balance
					float levelSum = levelLeft + levelRight;

					// Limit resolution
					newBalance = round(newBalance * 100.0f) / 100.0f;
					if (newBalance < 0.0f) newBalance = 0.0f;
					else if (newBalance > 1.0f) newBalance = 1.0f;

					// Set channel level values
					levelLeft = levelSum * newBalance;
					levelRight = levelSum * (1 - newBalance);

					// Limit levels
					if (levelLeft < 0) levelLeft = 0;
					else if (levelLeft > 1) levelLeft = 1;
					if (levelRight < 0) levelRight = 0;
					else if (levelRight > 1) levelRight = 1;

					// Debug message
					if (logger.IsDebugOutputEnabled()) {
						LOG_DEBUG("Audio balance: Left %0.5f <- %0.3f -> %0.5f Right\n", levelLeft, newBalance, levelRight);
					}

					// Set endpoint volume channel levels
					pAudioEndpointVolume->SetChannelVolumeLevelScalar(0, levelLeft, &GUID_NULL);
					pAudioEndpointVolume->SetChannelVolumeLevelScalar(1, levelRight, &GUID_NULL);

					// Set other channels to an average level between left and right
					for (int i = 2; i < (int)channelCount; i++) {
						pAudioEndpointVolume->SetChannelVolumeLevelScalar(i, levelSum / 2.0f, &GUID_NULL);
					}
				}
			}
		}
		catch (std::exception &e) {
			LOG_ERROR("Can't set audio balance! %s\n", e.what());
		}
	}

	ReleaseEndpointVolume();
}

//
// Get main audio device volume
//
float InputEmulator::VolumeGet()
{
	// Lock audio control
	std::unique_lock<std::mutex> mlock(lockAudio);

	float volume = 0;
	if (CreateEndpointVolume()) {
		pAudioEndpointVolume->GetMasterVolumeLevelScalar(&volume);
	}
	ReleaseEndpointVolume();
	return volume;
}

//
// Change main audio device volume
//
void InputEmulator::VolumeChange(float delta)
{
	// Lock audio control
	std::unique_lock<std::mutex> mlock(lockAudio);

	float volume;
	if (CreateEndpointVolume()) {
		pAudioEndpointVolume->GetMasterVolumeLevelScalar(&volume);
		volume += delta;
		if (volume < 0) volume = 0.0f;
		else if (volume > 1) volume = 1.0f;
		pAudioEndpointVolume->SetMasterVolumeLevelScalar(volume, &GUID_NULL);
	}
	ReleaseEndpointVolume();
}


//
// Force low latency audio (Windows 10)
//
int InputEmulator::ForceLowLatencyAudio()
{
	// Lock audio control
	std::unique_lock<std::mutex> mlock(lockAudio);

	if (isLowLatencyAudioForced) return 0;

	HRESULT hresult;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	WAVEFORMATEX* pFormat = NULL;

	UINT32 defaultPeriodInFrames = 0;
	UINT32 fundamentalPeriodInFrames = 0;
	UINT32 minPeriodInFrames = 0;
	UINT32 maxPeriodInFrames = 0;
	UINT32 beforePeriodInFrames = 0;
	UINT32 nowPeriodInFrames = 0;

	bool debug = logger.IsDebugOutputEnabled();

	try {

		// Create MMDeviceEnumerator
		hresult = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&pEnumerator));
		CHECK_HRESULT(hresult, "MMDeviceEnumerator");

		// Get default audio endpoint
		hresult = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
		CHECK_HRESULT(hresult, "GetDefaultAudioEndpoint");

		// Create AudioClient3
		hresult = pDevice->Activate(__uuidof(IAudioClient3), CLSCTX_ALL, NULL, reinterpret_cast<void**>(&pAudioClient3));
		CHECK_HRESULT(hresult, "IAudioClient3");

		// Get audio format
		hresult = pAudioClient3->GetMixFormat(&pFormat);
		CHECK_HRESULT(hresult, "GetMixFormat");

		if (debug) {
			LOG_DEBUG("Format.nChannels = %d\n", pFormat->nChannels);
			LOG_DEBUG("Format.nSamplesPerSec = %d\n", pFormat->nSamplesPerSec);
			LOG_DEBUG("Format.wBitsPerSample = %d\n", pFormat->wBitsPerSample);
			LOG_DEBUG("Format.nAvgBytesPerSec = %d\n", pFormat->nAvgBytesPerSec);
			LOG_DEBUG("Format.nBlockAlign = %d\n", pFormat->nBlockAlign);
			LOG_DEBUG("Format.wFormatTag = %d\n", pFormat->wFormatTag);
		}


		// Get shared mode period settings
		hresult = pAudioClient3->GetSharedModeEnginePeriod(
			pFormat,
			&defaultPeriodInFrames,
			&fundamentalPeriodInFrames,
			&minPeriodInFrames,
			&maxPeriodInFrames);
		CHECK_HRESULT(hresult, "GetSharedModeEnginePeriod");

		if (debug) {
			LOG_DEBUG("DefaultPeriodInFrames = %d\n", defaultPeriodInFrames);
			LOG_DEBUG("FundamentalPeriodInFrames = %d\n", fundamentalPeriodInFrames);
			LOG_DEBUG("MinPeriodInFrames = %d\n", minPeriodInFrames);
			LOG_DEBUG("MaxPeriodInFrames = %d\n", maxPeriodInFrames);
		}

		beforePeriodInFrames = 0;
		hresult = pAudioClient3->GetCurrentSharedModeEnginePeriod(&pFormat, &beforePeriodInFrames);
		CHECK_HRESULT(hresult, "GetCurrentSharedModeEnginePeriod");
		if (debug) {
			LOG_DEBUG("BeforePeriodInFrames = %d\n", beforePeriodInFrames);
		}

		hresult = pAudioClient3->InitializeSharedAudioStream(
			0,
			minPeriodInFrames,
			pFormat,
			NULL);
		CHECK_HRESULT(hresult, "InitializeSharedAudioStream");


		hresult = pAudioClient3->Start();
		CHECK_HRESULT(hresult, "AudioClient3 start");

		nowPeriodInFrames = 0;
		hresult = pAudioClient3->GetCurrentSharedModeEnginePeriod(&pFormat, &nowPeriodInFrames);
		CHECK_HRESULT(hresult, "GetCurrentSharedModeEnginePeriod");

		if (debug) {
			LOG_DEBUG("NowPeriodInFrames = %d\n", nowPeriodInFrames);
			LOG_DEBUG("Latency Min = %0.3f ms\n",
				(double)minPeriodInFrames / (double)pFormat->nSamplesPerSec * 1000.0
			);
			LOG_DEBUG("Latency Max = %0.3f ms\n",
				(double)maxPeriodInFrames / (double)pFormat->nSamplesPerSec * 1000.0
			);
			LOG_DEBUG("Latency Before = %0.3f ms\n",
				(double)beforePeriodInFrames / (double)pFormat->nSamplesPerSec * 1000.0
			);
			LOG_DEBUG("Latency Now = %0.3f ms\n",
				(double)nowPeriodInFrames / (double)pFormat->nSamplesPerSec * 1000.0
			);
		}

		SAFE_RELEASE(pEnumerator);
		SAFE_RELEASE(pDevice);
		if (pFormat != NULL) {
			CoTaskMemFree(pFormat);
		}

		isLowLatencyAudioForced = true;
	}
	catch (std::exception &e) {
		isLowLatencyAudioForced = false;
		LOG_ERROR("Couldn't force low latency audio! %s failed!\n", e.what());
		SAFE_RELEASE(pEnumerator);
		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pAudioClient3);
		if (pFormat != NULL) {
			CoTaskMemFree(pFormat);
		}
		return -1;
	}

	return 1;
}


//
// InputAction constructor
//
InputEmulator::InputAction::InputAction()
{
	this->virtualKey = 0;
	this->mouseButton = 0;
	this->numericValue = 0;
	this->stringValue = "";
}
InputEmulator::InputAction::InputAction(InputActionType type, std::string description) : InputAction()
{
	this->type = type;
	this->description = description;
}

//
// InputAction execute
//
void InputEmulator::InputAction::Execute(bool isPressed, bool isReleased, bool isDown)
{

	// Do not run if not button is not pressed or released
	if (!isPressed && !isReleased) {
		return;
	}

	// Debug message
	if (logger.IsDebugOutputEnabled()) {
		LOG_DEBUG("Execute '%s' P=%s, R=%s, D=%s, T=%d, K=%d, M=%d, N=%0.2f\n",
			description.c_str(),
			isPressed ? "True" : "False",
			isReleased ? "True" : "False",
			isDown ? "True" : "False",
			type,
			virtualKey,
			mouseButton,
			numericValue
		);
	}


	//
	// Keyboard action
	//
	if (type == InputActionType::ActionTypeKeyboard && virtualKey > 0) {
		emulator->SetKeyState(virtualKey, isDown);
	}

	//
	// Mouse button
	//
	else if (type == InputActionType::ActionTypeMouse &&
		mouseButton >= MouseButtons::Mouse1 &&
		mouseButton <= MouseButtons::Mouse5) {
		emulator->MouseSet(mouseButton, isDown);
	}

	//
	// Mouse scroll
	//
	else if (
		type == InputActionType::ActionTypeMouse &&
		mouseButton >= MouseButtons::MouseScrollUp &&
		mouseButton <= MouseButtons::MouseScrollRight &&
		isPressed) {

		switch (mouseButton)
		{
		case MouseButtons::MouseScrollUp: emulator->MouseScroll(1, true); break;
		case MouseButtons::MouseScrollDown: emulator->MouseScroll(-1, true); break;
		case MouseButtons::MouseScrollLeft: emulator->MouseScroll(-1, false); break;
		case MouseButtons::MouseScrollRight: emulator->MouseScroll(1, false); break;
		default:
			break;
		}
	}

	//
	// Audio volume change
	//
	else if (type == InputActionType::ActionTypeAudioVolumeChange && numericValue != 0 && isPressed) {
		try {
			emulator->VolumeChange((float)numericValue);
		}
		catch (std::exception& e) {
			LOG_ERROR("Volume control error! %s\n", e.what());
		}
	}

	//
	// Start application
	//
	else if (type == InputActionType::ActionTypeStartApplication && stringValue.size() > 0 && isPressed) {

		try {
			SHELLEXECUTEINFOA executeInfo;
			executeInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
			executeInfo.fMask = NULL;
			executeInfo.lpParameters = NULL;
			executeInfo.lpDirectory = NULL;
			executeInfo.hInstApp = NULL;
			executeInfo.hwnd = GetDesktopWindow();
			executeInfo.lpVerb = "open";
			executeInfo.lpFile = stringValue.c_str();
			executeInfo.nShow = SW_SHOWDEFAULT;

			bool result = ShellExecuteExA(&executeInfo);
			DWORD errorCode = GetLastError();

			// Errors
			if (!result && errorCode > 0) {

				// Get error text
				LPSTR errorTextBuffer = nullptr;
				size_t size = FormatMessageA(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					errorCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPSTR)&errorTextBuffer, 0,
					NULL
				);
				std::string errorText(errorTextBuffer, size);
				LocalFree(errorTextBuffer);

				// Throw
				throw std::exception(errorText.c_str());
			}
		}
		catch (std::exception& e) {
			LOG_ERROR("Can't open \"%s\". Error: %s\n", stringValue.c_str(), e.what());
		}
	}

}



//
// InputActionCollection constructor
//
InputEmulator::InputActionCollection::InputActionCollection()
{
}
InputEmulator::InputActionCollection::InputActionCollection(InputEmulator *emulator)
{
	this->emulator = emulator;
}

//
// InputActionCollection destructor
//
InputEmulator::InputActionCollection::~InputActionCollection()
{
	Clear();
}

//
// InputActionCollection add action
//
void InputEmulator::InputActionCollection::Add(InputAction *action)
{
	action->emulator = emulator;
	actions.push_back(action);
}

//
// InputActionCollection add action by using a string
//
void InputEmulator::InputActionCollection::Add(std::string actions)
{
	std::string originalText = actions;
	std::transform(actions.begin(), actions.end(), actions.begin(), ::toupper);
	std::stringstream stringStream(actions);
	std::string input;
	InputAction *inputAction = NULL;
	InputAction *newInputAction = NULL;

	while (getline(stringStream, input, '+')) {

		// Input in the map?
		if (emulator->inputMap.count(input) > 0) {
			inputAction = emulator->inputMap[input];

			newInputAction = new InputAction(inputAction->type, inputAction->description);
			newInputAction->mouseButton = inputAction->mouseButton;
			newInputAction->virtualKey = inputAction->virtualKey;
			newInputAction->numericValue = inputAction->numericValue;
			newInputAction->stringValue = inputAction->stringValue;
			Add(newInputAction);
		}

		//
		// Precise volume control
		//
		else if (input.compare(0, 8, "VOLUMEUP") == 0 && input.size() > 8) {
			double value = 0;
			try {
				// Parse number from end of the string
				value = stod(input.substr(8, input.size() - 8));

				// Create input action
				newInputAction = new InputAction(InputActionType::ActionTypeAudioVolumeChange, "Volume Up");
				newInputAction->numericValue = value / 100.0;
				Add(newInputAction);
			}
			catch (std::exception& e) {
				LOG_ERROR("Volume control error! %s\n", e.what());
			}
		}
		else if (input.compare(0, 10, "VOLUMEDOWN") == 0 && input.size() > 10) {
			float value = 0;
			try {
				// Parse number from end of the string
				value = stof(input.substr(10, input.size() - 10));

				// Create input action
				newInputAction = new InputAction(InputActionType::ActionTypeAudioVolumeChange, "Volume Down");
				newInputAction->numericValue = -value / 100.0;
				Add(newInputAction);
			}
			catch (std::exception& e) {
				LOG_ERROR("Volume control error! %s\n", e.what());
			}
		}


		//
		// Run application or open a file/folder
		//
		else if (input.compare(0, 4, "RUN ") == 0) {

			// Get path
			std::string path = originalText.substr(4, originalText.size() - 4);

			// Create input action
			newInputAction = new InputAction(InputActionType::ActionTypeStartApplication, "Run: " + path);
			newInputAction->stringValue = path;
			Add(newInputAction);

		}
	}

}


//
// InputActionCollection execute actions
//
void InputEmulator::InputActionCollection::Execute(bool isPressed, bool isReleased, bool isDown)
{
	// Do not run if not button is not pressed or released
	if (!isPressed && !isReleased) {
		return;
	}

	// Execute in normal order when pressed
	if (isPressed) {
		for (InputAction *inputAction : actions) {
			inputAction->Execute(isPressed, isReleased, isDown);
		}
	}

	// Execute in reverse order when released
	else if (isReleased) {
		int startIndex = actions.size() - 1;
		for (int i = startIndex; i >= 0; i--) {
			actions[i]->Execute(isPressed, isReleased, isDown);
		}
	}

}

//
// InputActionCollection clear
//
void InputEmulator::InputActionCollection::Clear()
{
	// Destroy actions
	for (InputAction *action : actions) {
		if (action != NULL)
			delete action;
	}
	actions.clear();
}


//
// InputActionCollection count
//
int InputEmulator::InputActionCollection::Count()
{
	return actions.size();
}


//
// InputActionCollection to string
//
std::string InputEmulator::InputActionCollection::ToString()
{
	std::string str = "";
	bool first = true;
	for (InputAction *action : actions) {
		if (first) {
			first = false;
		}
		else {
			str += " + ";
		}
		str += action->description;
	}
	return str;
}
