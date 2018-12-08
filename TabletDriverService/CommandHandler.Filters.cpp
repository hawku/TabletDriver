#include "stdafx.h"
#include "CommandHandler.h"

#define LOG_MODULE ""
#include "Logger.h"

//
// Create filter commands
//
void CommandHandler::CreateFilterCommands() {

	//
	// Command: SmoothingFilter, Smoothing
	//
	// Sets smoothing filter parameters
	//
	AddAlias("Smoothing", "SmoothingFilter");
	AddCommand(new Command("SmoothingFilter", [&](CommandLine *cmd) {
		if(!ExecuteCommand("TabletValid")) return false;

		double latency = cmd->GetDouble(0, tablet->smoothing.GetLatency());
		double threshold = cmd->GetDouble(1, tablet->smoothing.threshold * 100);

		threshold /= 100;

		string stringValue = cmd->GetStringLower(0, "");

		// Off / False
		if(stringValue == "off" || stringValue == "false") {
			latency = 0;
		}

		// Limits
		if(latency < 0) latency = 1;
		if(latency > 1000) latency = 1000;
		if(threshold < 0.1) threshold = 0.1;
		if(threshold > 0.99) threshold = 0.99;

		// Set threshold
		tablet->smoothing.threshold = threshold;

		// Set smoothing filter latency
		tablet->smoothing.SetLatency(latency);

		// Print output
		if(tablet->smoothing.weight < 1.0) {
			tablet->smoothing.isEnabled = true;
			LOG_INFO("Smoothing = %0.2f ms to reach %0.0f%% (weight = %f)\n", latency, tablet->smoothing.threshold * 100, tablet->smoothing.weight);
		}
		else {
			tablet->smoothing.isEnabled = false;
			LOG_INFO("Smoothing = off\n");
		}

		return true;
	}));


	//
	// Command: GravityFilter, Gravity
	//
	// Sets gravity filter parameters
	//
	AddAlias("Gravity", "GravityFilter");
	AddCommand(new Command("GravityFilter", [&](CommandLine *cmd) {

		if(!ExecuteCommand("TabletValid")) return false;
		double gravity = cmd->GetDouble(0, tablet->gravityFilter.gravity);
		double friction = cmd->GetDouble(1, 0);
		double pressureGravity = cmd->GetDouble(2, 0);
		double pressureFriction = cmd->GetDouble(3, 0);


		// Limits
		if(gravity < 0) gravity = 0.0;
		if(gravity > 100) gravity = 100;
		if(friction < 0.1) friction = 0.1;
		if(friction > 1000) friction = 1000;

		if(gravity > 0.1) {
			tablet->gravityFilter.isEnabled = true;
			tablet->gravityFilter.gravity = gravity;
			tablet->gravityFilter.friction = friction;
			tablet->gravityFilter.pressureGravity = pressureGravity;
			tablet->gravityFilter.pressureFriction = pressureFriction;
		}
		else {
			tablet->gravityFilter.isEnabled = false;
			tablet->gravityFilter.gravity = 0;
			tablet->gravityFilter.friction = 0;
			tablet->gravityFilter.pressureGravity = 0;
			tablet->gravityFilter.pressureFriction = 0;

		}

		LOG_INFO("Gravity = %s (%0.2f gravity, %0.2f friction, %0.2f pressure gravity, %0.2f pressure friction)\n",
			tablet->gravityFilter.isEnabled ? "True" : "False",
			tablet->gravityFilter.gravity,
			tablet->gravityFilter.friction,
			tablet->gravityFilter.pressureGravity,
			tablet->gravityFilter.pressureFriction
		);
		return true;
	}));


	//
	// Command: NoiseReduction, Noise
	//
	// Sets noise reduction filter parameters
	//
	AddAlias("Noise", "NoiseReduction");
	AddCommand(new Command("NoiseReduction", [&](CommandLine *cmd) {

		string stringValue = cmd->GetStringLower(0, "");

		// Off / False
		if(stringValue == "off" || stringValue == "false") {
			tablet->noiseFilter.isEnabled = false;
			LOG_INFO("Noise Reduction = off\n");

			// Enable
		}
		else {

			// Position buffer length
			int length = cmd->GetInt(0, tablet->noiseFilter.buffer.length);

			// Threshold where the noise filter starts to reduce the amount of filtering.
			double distanceThreshold = cmd->GetDouble(1, tablet->noiseFilter.distanceThreshold);

			// Distance where the amount of filtering will be zero. Default maximum is 2 times the threshold.
			double distanceMaximum = cmd->GetDouble(2, distanceThreshold * 2.0);

			// Geometric median calculation iteration count
			int iterations = cmd->GetInt(3, tablet->noiseFilter.iterations);

			// Limits
			if(length < 0) length = 0;
			else if(length > 50) length = 50;

			if(distanceThreshold < 0.0) distanceThreshold = 0.0;
			else if(distanceThreshold > 1000) distanceThreshold = 1000;

			if(distanceMaximum < 0.1) distanceMaximum = 0.1;
			else if(distanceMaximum > 1000) distanceMaximum = 1000;

			if(distanceThreshold > distanceMaximum) distanceThreshold = distanceMaximum;

			if(iterations < 1) iterations = 1;
			else if(iterations > 100) iterations = 100;

			// Set noise filter values
			tablet->noiseFilter.buffer.SetLength(length);
			tablet->noiseFilter.distanceThreshold = distanceThreshold;
			tablet->noiseFilter.distanceMaximum = distanceMaximum;
			tablet->noiseFilter.iterations = iterations;

			// Enable filter
			if(tablet->noiseFilter.buffer.length > 0) {
				tablet->noiseFilter.isEnabled = true;
				LOG_INFO("Noise Reduction = [\n");
				LOG_INFO("  %d samples\n", length);
				LOG_INFO("  %0.2f mm threshold (Wacom %0.2f mm/s)\n", distanceThreshold, distanceThreshold * 133.0);
				LOG_INFO("  %0.2f mm maximum (Wacom %0.2f mm/s)\n", distanceMaximum, distanceMaximum * 133.0);
				LOG_INFO("  %d iterations\n", iterations);
				LOG_INFO("]\n");
			}
			else {
				tablet->noiseFilter.isEnabled = false;
				LOG_INFO("Noise Reduction = off\n");
			}

		}
		return true;
	}));


	//
	// Command: AntiSmoothingFilter, AntiSmoothing, Anti
	//
	// Sets anti-smoothing filter parameters
	//
	AddAlias("AntiSmoothing", "AntiSmoothingFilter");
	AddAlias("Anti", "AntiSmoothingFilter");
	AddCommand(new Command("AntiSmoothingFilter", [&](CommandLine *cmd) {

		string stringValue = cmd->GetStringLower(0, "");

		// Off / False
		if(stringValue == "off" || stringValue == "false") {
			tablet->antiSmoothing.isEnabled = false;
			LOG_INFO("Anti-smoothing = off\n");

			// Enable
		}
		else {

			double shape = cmd->GetDouble(0, 1.0);
			double compensation = cmd->GetDouble(1, 1.0);
			bool ignoreWhenDragging = cmd->GetBoolean(2, false);

			if(shape <= 0) {
				tablet->antiSmoothing.isEnabled = false;
				LOG_INFO("Anti-smoothing = off\n");
			}
			else {
				tablet->antiSmoothing.shape = shape;
				tablet->antiSmoothing.compensation = compensation;
				tablet->antiSmoothing.ignoreWhenDragging = ignoreWhenDragging;
				tablet->antiSmoothing.isEnabled = true;
				LOG_INFO("Anti-smoothing = Shape:%0.2f Compensation:%0.2f DragIgnore:%s\n",
					tablet->antiSmoothing.shape,
					tablet->antiSmoothing.compensation,
					tablet->antiSmoothing.ignoreWhenDragging ? "true" : "false"
				);
			}
		}
		return true;
	}));


	//
	// Command: FilterTimerInterval
	//
	// Sets filter timer interval
	//
	AddCommand(new Command("FilterTimerInterval", [&](CommandLine *cmd) {
		if(tabletHandler == NULL) return true;
		int oldInterval = (int)round(tabletHandler->timerInterval);

		int interval = cmd->GetInt(0, oldInterval);

		// 10 Hz
		if(interval > 100) interval = 100;

		// 1000 Hz
		if(interval < 1) interval = 1;

		// Change interval
		tabletHandler->ChangeTimerInterval(interval);

		LOG_INFO("Filter Timer Interval = %0.0f (%0.2f Hz)\n",
			tabletHandler->timerInterval,
			1000.0 / tabletHandler->timerInterval
		);
		return true;
	}));


	//
	// Command: FilterTester, Tester
	//
	AddAlias("Tester", "FilterTester");
	AddCommand(new Command("FilterTester", [&](CommandLine *cmd) {

		string inputFilepath = cmd->GetString(0, "tester_input.txt");
		string outputFilepath = cmd->GetString(1, "tester_output.txt");
		TabletFilterAntiSmoothing *filterAntiSmoothing = NULL;
		TabletFilterNoiseReduction *filterNoise = NULL;
		TabletFilterTester *tester = NULL;
		char settingsString[1024];
		int settingsStringSize = 1024;
		int settingsStringIndex = 0;

		// Anti-smoothing
		if(tablet->antiSmoothing.isEnabled) {
			filterAntiSmoothing = new TabletFilterAntiSmoothing();
			filterAntiSmoothing->shape = tablet->antiSmoothing.shape;
			filterAntiSmoothing->compensation = tablet->antiSmoothing.compensation;
			filterAntiSmoothing->ignoreWhenDragging = tablet->antiSmoothing.ignoreWhenDragging;
			settingsStringIndex += sprintf_s((settingsString + settingsStringIndex), settingsStringSize - settingsStringIndex,
				"settings AntiSmoothing shape=%0.3f compensation=%0.3f dragignore=%s\r\n",
				tablet->antiSmoothing.shape,
				tablet->antiSmoothing.compensation,
				tablet->antiSmoothing.ignoreWhenDragging ? "true" : "false"
			);
		}

		// Noise reduction
		if(tablet->noiseFilter.isEnabled) {
			filterNoise = new TabletFilterNoiseReduction();
			filterNoise->buffer.SetLength(tablet->noiseFilter.buffer.length);
			filterNoise->distanceThreshold = tablet->noiseFilter.distanceThreshold;
			filterNoise->distanceMaximum = tablet->noiseFilter.distanceMaximum;
			filterNoise->iterations = tablet->noiseFilter.iterations;
			settingsStringIndex += sprintf_s((settingsString + settingsStringIndex), settingsStringSize - settingsStringIndex,
				"settings NoiseReduction buffer=%d threshold=%0.3f maximum=%0.3f iterations=%d\r\n",
				tablet->noiseFilter.buffer.length,
				tablet->noiseFilter.distanceThreshold,
				tablet->noiseFilter.distanceMaximum,
				tablet->noiseFilter.iterations
			);
		}
		if(filterAntiSmoothing != NULL || filterNoise != NULL) {

			logger.directPrint = true;

			LOG_INFO("Filter test starting!\n");

			tester = new TabletFilterTester(inputFilepath, outputFilepath);

			// Add filters
			if(filterAntiSmoothing != NULL)
				tester->AddFilter(filterAntiSmoothing);
			if(filterNoise != NULL)
				tester->AddFilter(filterNoise);

			bool result = tester->Open();
			if(!result) {
				LOG_ERROR("Filter tester can't open files!\n");
				return false;
			}
			tester->outputFile << settingsString;
			tester->Run();
			LOG_INFO("Filter test ended!\n");

			logger.ProcessMessages();
			logger.directPrint = false;

		}
		else {
			LOG_INFO("No report filters enabled!\n");
		}

		if(filterAntiSmoothing != NULL) {
			delete filterAntiSmoothing;
		}

		if(filterNoise != NULL) {
			delete filterNoise;
		}

		if(tester != NULL) {
			delete tester;
		}

		return true;
	}));


}
