#pragma once

#include <map>
#include <mutex>

#include <psapi.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <audioendpoints.h>
#include <endpointvolume.h>

#define SAFE_RELEASE(resource) \
	if ((resource) != NULL) { \
		(resource)->Release(); \
		(resource) = NULL; \
	}

#define CHECK_HRESULT(hresult, errorText) \
    if (FAILED(hresult)) { \
		throw std::runtime_error(errorText); \
	}

class InputEmulator
{
private:
	void CreateKeyMap();
	void CreateVirtualKeyMap();
	IAudioEndpointVolume *pAudioEndpointVolume = NULL;
	IMMDeviceEnumerator *pDeviceEnumerator = NULL;
	IMMDevice *pDefaultAudioDevice = NULL;
	IAudioClient3* pAudioClient3 = NULL;
	bool isLowLatencyAudioForced = false;
	std::mutex lockAudio;
public:
	enum MouseButtons {
		Mouse1 = 1,
		Mouse2 = 2,
		Mouse3 = 3,
		Mouse4 = 4,
		Mouse5 = 5,
		MouseScrollUp = 0x81,
		MouseScrollDown = 0x82,
		MouseScrollLeft = 0x83,
		MouseScrollRight = 0x84,
		MouseScrollVertical = 0x101,
		MouseScrollHorizontal = 0x102,
		MouseScrollBoth = 0x103
	};

	enum InputActionType {
		ActionTypeKeyboard,
		ActionTypeMouse,
		ActionTypeAudioVolumeControl,
		ActionTypeAudioBalanceControl,
		ActionTypeAudioVolumeChange,
		ActionTypeStartApplication
	};

	class InputAction {
	public:
		InputEmulator *emulator;
		InputActionType type;
		std::string description;
		WORD virtualKey;
		int mouseButton;
		double numericValue;
		std::string stringValue;
		InputAction();
		InputAction(InputActionType type, std::string description);
		void Execute(bool isPressed, bool isReleased, bool isDown);
	};

	class InputActionCollection {
	public:
		InputEmulator *emulator;
		std::vector<InputAction *> actions;
		InputActionCollection();
		InputActionCollection(InputEmulator *emulator);
		~InputActionCollection();
		void Add(InputAction *action);
		void Add(std::string actions);
		void Execute(bool isPressed, bool isReleased, bool isDown);
		void Clear();
		int Count();
		std::string ToString();
	};



	std::map<std::string, InputAction*> inputMap;
	std::vector<std::string> inputs;

	InputEmulator();
	~InputEmulator();


	void AddKey(std::string key, std::string keyName, WORD virtualCode);
	void AddMouse(std::string key, std::string keyName, int button);
	WORD GetKeyCode(std::string key);
	
	void MouseSet(int button, bool down);
	void MousePress(int button, int time);
	void MouseMove(int x, int y);
	void MouseMoveTo(int x, int y);
	void MouseScroll(int delta, bool vertical);
	void SetKeyState(std::string key, bool down);
	void SetKeyState(WORD vkCode, bool down);
	void SetInputStates(std::string inputs, bool down);
	void KeyPress(std::string keys, int time);

	bool CreateEndpointVolume();
	bool ReleaseEndpointVolume();
	void VolumeSet(float volume);
	void VolumeBalance(float leftRight);
	float VolumeGet();
	void VolumeChange(float delta);
	int ForceLowLatencyAudio();


};

