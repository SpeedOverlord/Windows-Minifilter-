/*
	Copyright © 2017 by Phillip Chang

	S2StateRoundButton
*/
#pragma once

#include "SStateButton.h"

class S2StateRoundButton : public SStateButton<2> {
public:
	virtual bool Enter(SIN const int x, const int y) const {
		int dx = x - (width_ >> 1), dy = y - (height_ >> 1);
		dx = dx*dx + dy*dy;

		dy = (width_ >> 1);
		dy *= dy;
		return dx <= dy;
	}
};
