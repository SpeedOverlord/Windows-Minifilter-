/*
	Copyright © 2016 by Phillip Chang

	SDrawableItem

	相容於 windows control 的繪畫物件
*/
#pragma once

#include "slib\SCharAry.h"

class SDrawableItem {
public:
	/*
		畫出 item 時使用
	*/
	virtual void DrawItem(SIN HDC hdc, RECT& rect, bool selected) = 0;

	SCharAry str_;
};
