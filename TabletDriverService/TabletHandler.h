#pragma once
#include <thread>
#include <mutex>
#include "Tablet.h"
#include "InputEmulator.h"

class TabletHandler {
public:
	Tablet *tablet;
	thread *tabletInputThread;
	thread *auxInputThread;
	HANDLE timer;
	double timerInterval;
	mutex lock;
	chrono::high_resolution_clock::time_point timeBegin;
	TabletState outputState;
	InputEmulator inputEmulator;

	bool isRunning;

	TabletHandler();
	~TabletHandler();

	bool Start();
	bool Stop();
	bool StartTimer();
	bool StopTimer();
	void ChangeTimerInterval(int newInterval);
	void RunTabletInputThread();
	void RunAuxInputThread();
	void OnTimerTick();
	void WriteOutputState(TabletState *outputState);


private:
	static VOID CALLBACK TimerCallback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired) {
		TabletHandler *tabletHandler = (TabletHandler *)lpParameter;
		tabletHandler->OnTimerTick();
	}
};

