#include "stdafx.h"
#include "TabletFilterTester.h"

#define LOG_MODULE "Tester"
#include "Logger.h"


// Constructor
TabletFilterTester::TabletFilterTester(TabletFilter *f, string input, string output) {
	this->filter = f;
	this->inputFilepath = input;
	this->outputFilepath = output;
}

// Destructor
TabletFilterTester::~TabletFilterTester() {
}


//
// Open files
//
bool TabletFilterTester::Open() {

	inputFile = ifstream(inputFilepath, ifstream::in);
	if(!inputFile) return false;

	outputFile = ofstream(outputFilepath, ofstream::out);
	if(!outputFile) return false;

	return false;
}

//
// Run filter test
//
void TabletFilterTester::Run() {

	double time;
	chrono::high_resolution_clock::time_point timeBegin;
	chrono::high_resolution_clock::time_point timeNow;
	Vector2D position;
	string line;
	bool firstReport;
	double distance;
	TabletState tabletState, outputState;

	firstReport = true;

	// Loop
	while(!inputFile.eof()) {

		// Read line from input file
		try {
			getline(inputFile, line);
		} catch(exception) {
			break;
		}

		// Parse input
		CommandLine cmd(line);
		if(cmd.is("position")) {
			time = cmd.GetDouble(0, 0) ;
			position.x = cmd.GetDouble(1, 0);
			position.y = cmd.GetDouble(2, 0);
			LOG_DEBUG("Input: %0.3f ms, %0.2f, %0.2f\n",
				time,
				position.x,
				position.y
			);

			// Init settings on first report
			if(firstReport) {
				timeBegin = chrono::high_resolution_clock::now();
				firstReport = false;
				outputState.position.Set(position);
				LOG_DEBUG("First report: %0.3f, %0.2f, %0.2f\n", time, position.x, position.y);
				
			} else {
				tabletState.time = timeNow;
				tabletState.position.Set(position);
				memcpy(&outputState, &tabletState, sizeof(TabletState));
				filter->SetTarget(&tabletState);
				filter->Update();
				filter->GetOutput(&outputState);
			}

			timeNow = timeBegin + chrono::microseconds((int)(time * 1000.0));
			distance = position.Distance(outputState.position);

			LOG_DEBUG("Output: %0.3f ms, %0.2f, %0.2f (%0.3f mm)\n",
				time,
				position.x,
				position.y,
				distance
			);

			outputFile << "position " << time << " " << outputState.position.x << " " << outputState.position.y << fixed << setprecision(2) << "\n";


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
