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

	TabletFilterTester(TabletFilter *filter, string inputFilepath, string outputFilepath);
	~TabletFilterTester();

	bool Open();
	void Run();
	bool Close();

};

