#pragma once
#include "OutputSettings.h"
#include "TabletState.h"

class Output {
public:
	OutputSettings *settings = NULL;

	virtual void Init();
	virtual bool Set(TabletState *tabletState) = 0;
	virtual bool Write() = 0;
	virtual bool Reset() = 0;
};
