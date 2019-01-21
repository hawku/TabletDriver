#pragma once
#include "Output.h"
class OutputDummy : public Output {
public:

	std::chrono::high_resolution_clock::time_point timeBegin;

	void Init() override;
	bool Set(TabletState *tabletState);
	bool Write();
	bool Reset();

	OutputDummy();
	~OutputDummy();
};

