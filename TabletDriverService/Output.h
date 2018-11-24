#pragma once
#include "OutputSettings.h"

class Output {
public:

	OutputSettings * settings;
	bool isEnabled;
	bool debugEnabled;


	virtual bool Set(unsigned char buttons, double x, double y, double pressure) = 0;
	virtual bool Write() = 0;
	virtual bool Reset() = 0;

	Output();

};
