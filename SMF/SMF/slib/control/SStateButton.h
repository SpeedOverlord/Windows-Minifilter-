/*
	Copyright © 2017 by Phillip Chang

	SStateButton
*/
#pragma once

#include "SButton.h"

#include "slib\display\SImage.h"

template<int ST>
class SStateButton : public SButton {
public:
	virtual bool Hover() override {
		return true;
	}
	virtual bool ButtonDown() override {
		return true;
	}

	virtual bool ButtonUp() override {
		++state_;
		if (state_ >= ST) state_ = 0;

		PostMessage(parent_, SWM_SBTN_CLK, id_, (LPARAM)this);
		return true;
	}

	virtual void DrawXY(SIN HDC main_hdc, HDC hdc, const int x, const int y) const override {
		if (hover_) {
			if (button_down_) img_[state_*button::IMAGE_CNT + button::DOWN]->DrawXY(main_hdc, hdc, 0, 0);
			else img_[state_*button::IMAGE_CNT + button::HOVER]->DrawXY(main_hdc, hdc, 0, 0);
		}
		else {
			img_[state_*button::IMAGE_CNT + button::UNHOVER]->DrawXY(main_hdc, hdc, 0, 0);
		}
	}

	HWND Create(SIN HWND parent, HINSTANCE instance) {
		state_ = 0;
		return SButton::Create(parent, instance);
	}

	int state_;
	SImage* img_[button::IMAGE_CNT*ST];
};
