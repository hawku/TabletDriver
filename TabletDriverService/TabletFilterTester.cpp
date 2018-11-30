#include "stdafx.h"
#include "TabletFilterTester.h"

#define LOG_MODULE "Tester"
#include "Logger.h"


// Constructor
TabletFilterTester::TabletFilterTester(TabletFilter *f, string input, string output) {
	this->filter = f;
	this->inputFilepath = input;
	this->outputFilepath = output;
	isRunning = false;
}

// Destructor
TabletFilterTester::~TabletFilterTester() {
}

//
// Start test
//
bool TabletFilterTester::Start() {

	isRunning = false;
	callback = TimerCallback;


	inputFile = ifstream(inputFilepath, ifstream::in);
	if(!inputFile) return false;

	outputFile = ofstream(outputFilepath, ofstream::out);
	if(!outputFile) return false;


	BOOL result = CreateTimerQueueTimer(
		&timer,
		NULL, callback,
		this,
		0,
		1,
		WT_EXECUTEDEFAULT
	);
	if(!result) return false;

	firstReport = true;
	isRunning = true;

	timeNow = chrono::high_resolution_clock::now();
	timeBegin = timeNow;
	inputTimeOffset = 0;
	nextInputTime = 0;
	nextInputPosition.Set(0, 0);

	return true;
}


//
// Stop test
//
bool TabletFilterTester::Stop() {
	if(timer == NULL) return false;
	bool result = DeleteTimerQueueTimer(NULL, timer, NULL);
	if(result) {
		timer = NULL;
	}
	isRunning = false;

	if(inputFile && inputFile.is_open()) {
		inputFile.close();
	}
	if(outputFile && outputFile.is_open()) {
		outputFile.close();
	}
	return result;
}

//
// Timer tick
//
void TabletFilterTester::OnTimerTick() {
	if(!isRunning) return;

	timeNow = chrono::high_resolution_clock::now();

	double timePosition = round((timeNow - timeBegin).count() / 1000000.0);
	Vector2D newPosition;
	double distance;

	//LOG_DEBUG("TIME POSITION = %0.2f\n", timePosition);
	if(timePosition > 100) {
		//isRunning = false;
	}

	
	string line;

	// EOF?
	if(inputFile.eof()) {
		isRunning = false;
		return;
	}


	// Wait for the next report time
	if(!firstReport && timePosition < nextInputTime) {
		return;
	}

	// Time position reached the next input time
	if(!firstReport && timePosition >= nextInputTime) {
		
		filter->SetTarget(nextInputPosition);
		filter->Update();
		filter->GetPosition(&newPosition);

		distance = nextInputPosition.Distance(newPosition);

		LOG_DEBUG("Output: %0.0f/%0.0f ms, %0.2f, %0.2f (%0.3f mm)\n",
			nextInputTime,
			timePosition,
			newPosition.x,
			newPosition.y,
			distance
		);

		
		outputFile << "position " << nextInputTime << " " << newPosition.x << " " << newPosition.y << fixed << setprecision(2) << "\n";



	}



	// Read input
	try {
		getline(inputFile, line);
	} catch(exception) {
		isRunning = false;
		return;
	}

	// Parse input
	CommandLine cmd(line);
	if(cmd.is("position")) {
		Vector2D position, newPosition;
		double time;

		time = cmd.GetDouble(0, 0) - inputTimeOffset;
		position.x = cmd.GetDouble(1, 0);
		position.y = cmd.GetDouble(2, 0);
		LOG_DEBUG("Input: %0.0f ms, %0.2f, %0.2f\n",
			time,
			position.x,
			position.y
		);
		
		// Init settings on first report
		if(firstReport) {
			inputTimeOffset = time;
			timeBegin = timeNow;
			firstReport = false;

			nextInputTime = 0;
			nextInputPosition.Set(position);

			LOG_DEBUG("First report: %0.2f, %0.2f, %0.2f\n", inputTimeOffset, position.x, position.y);

		} else {
			nextInputTime = time;
			nextInputPosition.Set(position);
		}
	}
	

}