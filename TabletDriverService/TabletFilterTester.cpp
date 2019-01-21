#include "precompiled.h"
#include "TabletFilterTester.h"

#define LOG_MODULE "Tester"
#include "Logger.h"


// Constructor
TabletFilterTester::TabletFilterTester(std::string input, std::string output) {
	this->inputFilepath = input;
	this->outputFilepath = output;
}

// Destructor
TabletFilterTester::~TabletFilterTester() {
}

//
// Add filter
//
void TabletFilterTester::AddFilter(TabletFilter *filter)
{
	filters.push_back(filter);
}


//
// Open files
//
bool TabletFilterTester::Open() {

	inputFile = std::ifstream(inputFilepath, std::ifstream::in);
	if(!inputFile) return false;


	outputFile = std::ofstream(outputFilepath, std::ofstream::out);
	if(!outputFile) return false;

	return true;
}

//
// Run filter test
//
void TabletFilterTester::Run() {

	double time;
	std::chrono::high_resolution_clock::time_point timeBegin;
	std::chrono::high_resolution_clock::time_point timeNow;
	Vector2D position;
	std::string line;
	bool firstReport;
	double distance;
	TabletState tabletState, outputState;

	firstReport = true;

	// Loop
	while(!inputFile.eof()) {

		// Read line from input file
		try {
			getline(inputFile, line);
		} catch(std::exception) {
			break;
		}

		// Parse input
		CommandLine cmd(line);
		if(cmd.is("position")) {
			time = cmd.GetDouble(0, 0);
			position.x = cmd.GetDouble(1, 0);
			position.y = cmd.GetDouble(2, 0);

			// Debug message
			if(logger.IsDebugOutputEnabled()) {
				LOG_DEBUG("IN : %0.3f ms, %0.2f, %0.2f\n",
					time,
					position.x,
					position.y
				);
			}

			// Init settings on first report
			if(firstReport) {
				timeBegin = std::chrono::high_resolution_clock::now();
				firstReport = false;
				outputState.position.Set(position);

				// Debug message
				if(logger.IsDebugOutputEnabled()) {
					LOG_DEBUG("First report: %0.3f, %0.2f, %0.2f\n", time, position.x, position.y);
				}

			}
			else {
				tabletState.time = timeNow;
				tabletState.position.Set(position);
				memcpy(&outputState, &tabletState, sizeof(TabletState));

				// Process filters
				for(TabletFilter *filter : filters) {
					filter->SetTarget(&outputState);
					filter->Update();
					filter->GetOutput(&outputState);
				}
			}

			timeNow = timeBegin + std::chrono::microseconds((int)(time * 1000.0));
			distance = position.Distance(outputState.position);

			/*
			if(distance > 5) {
				LOG_DEBUG("LONG DELTA: %0.2f ms, %0.3f,%0.3f -> %0.3f,%0.3f\n",
					time,
					position.x,
					position.y,
					outputState.position.x,
					outputState.position.y
				);
			}
			*/

			// Debug message
			if(logger.IsDebugOutputEnabled()) {
				LOG_DEBUG("OUT: %0.3f ms, %0.2f, %0.2f (%0.3f mm)\n",
					time,
					position.x,
					position.y,
					distance
				);
			}

			outputFile << "position " << time << " " << outputState.position.x << " " << outputState.position.y << std::fixed << std::setprecision(2) << "\n";


		}

	}

}

//
// Close files
//
bool TabletFilterTester::Close() {

	if(inputFile && inputFile.is_open()) {
		inputFile.close();
	}
	if(outputFile && outputFile.is_open()) {
		outputFile.close();
	}

	return true;
}
