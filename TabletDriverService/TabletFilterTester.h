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
#include <vector>


class TabletFilterTester {
public:
	HANDLE timer;
	WAITORTIMERCALLBACK callback;

	vector<TabletFilter *> filters;
	string inputFilepath;
	string outputFilepath;

	ifstream inputFile;
	ofstream outputFile;

	TabletFilterTester(string inputFilepath, string outputFilepath);
	~TabletFilterTester();

	void AddFilter(TabletFilter *filter);

	bool Open();
	void Run();
	bool Close();

};

