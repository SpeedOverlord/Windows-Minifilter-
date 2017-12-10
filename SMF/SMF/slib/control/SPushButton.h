/*
	Copyright © 2017 by Phillip Chang

	SPushButton
*/
#pragma once

#include "SButton.h"

#include "slib\display\SImage.h"

namespace button {
	const int IMAGE_CNT = 3,
		UNHOVER = 0, HOVER = 1, DOWN = 2;
}

class SPushButton : public SButton {
public:
	virtual bool Hover() override {
		return true;
	}
	virtual bool ButtonDown() override {
		return true;
	}

	virtual void DrawXY(SIN HDC main_hdc, HDC hdc, const int x, const int y) const override {
		if (hover_) {
			if (button_down_) img_[button::DOWN]->DrawXY(main_hdc, hdc, 0, 0);
			else img_[button::HOVER]->DrawXY(main_hdc, hdc, 0, 0);
		}
		else {
			img_[button::UNHOVER]->DrawXY(main_hdc, hdc, 0, 0);
		}
	}

	void SetImage(SIN SImage* img1, SImage* img2, SImage* img3) {
		img_[0] = img1;
		img_[1] = img2;
		img_[2] = img3;
	}

	SImage* img_[button::IMAGE_CNT];
};
