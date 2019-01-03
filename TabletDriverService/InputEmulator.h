#pragma once

#include <map>

#include <psapi.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <audioendpoints.h>
#include <endpointvolume.h>

extern "C" NTSYSAPI NTSTATUS NTAPI NtSetTimerResolution(ULONG DesiredResolution, BOOLEAN SetResolution, PULONG CurrentResolution);
extern "C" NTSYSAPI NTSTATUS NTAPI NtQueryTimerResolution(OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG ActualResolution);


#define SAFE_RELEASE(resource) \
	if ((resource) != NULL) { \
		(resource)->Release(); \
		(resource) = NULL; \
	}

#define CHECK_HRESULT(hresult, resource) \
    if (FAILED(hresult)) { \
		SAFE_RELEASE(resource); \
		return false; \
	}

class InputEmulator
{
private:
	void CreateKeyMap();
	void CreateVirtualKeyMap();
	IAudioEndpointVolume *pAudioEndpointVolume = NULL;
	IMMDeviceEnumerator *pDeviceEnumerator = NULL;
	IMMDevice *pDefaultAudioDevice = NULL;
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
		MouseScrollBoth = 0x103,
		MediaVolumeControl = 0x110,
	};

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

	bool CreateEndpointVolume();
	bool ReleaseEndpointVolume();
	void VolumeSet(float volume);
	float VolumeGet();
	void VolumeChange(float delta);


};

