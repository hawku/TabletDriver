#include "precompiled.h"
#include "OutputDummy.h"


#define LOG_MODULE "Dummy"
#include "Logger.h"


void OutputDummy::Init() {
	timeBegin = std::chrono::high_resolution_clock::now();
}

bool OutputDummy::Set(TabletState *tabletState) {

	double timeDelta = (tabletState->time - timeBegin).count() / 1000000.0;

	if(logger.IsDebugOutputEnabled()) {
		LOG_DEBUG("T=%0.3f B=%d X=%0.2f Y=%0.2f P=%0.4f\n",
			timeDelta,
			tabletState->buttons,
			tabletState->position.x,
			tabletState->position.y,
			tabletState->pressure
		);
	}


	return true;
}

bool OutputDummy::Write() {
	return true;
}

bool OutputDummy::Reset() {
	return true;
}

OutputDummy::OutputDummy() {
}


OutputDummy::~OutputDummy() {
}
