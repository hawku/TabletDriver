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

	std::vector<TabletFilter *> filters;
	std::string inputFilepath;
	std::string outputFilepath;

	std::ifstream inputFile;
	std::ofstream outputFile;

	TabletFilterTester(std::string inputFilepath, std::string outputFilepath);
	~TabletFilterTester();

	void AddFilter(TabletFilter *filter);

	bool Open();
	void Run();
	bool Close();

};

