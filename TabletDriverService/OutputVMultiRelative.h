#pragma once

#include "Vector2D.h"

class OutputVMultiRelative : public Output {
public:

	// Relative mouse vmulti report
	struct {
		BYTE vmultiId;
		BYTE reportLength;
		BYTE reportId;
		BYTE buttons;
		signed char x;
		signed char y;
		BYTE wheel;
	} report;

	bool Set(unsigned char buttons, double x, double y, double pressure);
	bool Write();
	bool Reset();

	OutputVMultiRelative();
	~OutputVMultiRelative();
};

