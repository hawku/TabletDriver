#pragma once
#include "Vector2D.h"
#include "TabletFilter.h"
#include "TabletFilterAntiSmoothing.h"
#include "TabletFilterNoiseReduction.h"
#include "CommandLine.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>


class TabletFilterTester {
public:
	HANDLE timer;
	WAITORTIMERCALLBACK callback;

	TabletFilter *filter;
	string inputFilepath;
	string outputFilepath;

	ifstream inputFile;
	ofstream outputFile;

	bool firstReport;
	double inputTime;
	double inputTimeOffset;

	double nextInputTime;
	Vector2D nextInputPosition;



	chrono::high_resolution_clock::time_point timeBegin;
	chrono::high_resolution_clock::time_point timeNow;

	bool isRunning;

	TabletFilterTester(TabletFilter *filter, string inputFilepath, string outputFilepath);
	~TabletFilterTester();

	bool Start();
	bool Stop();
	void OnTimerTick();
	
private:
	
	static VOID CALLBACK TimerCallback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired) {
		TabletFilterTester *tester = (TabletFilterTester *)lpParameter;
		tester->OnTimerTick();
	}
};

