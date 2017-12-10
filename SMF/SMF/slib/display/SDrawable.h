/*
	Copyright © 2016 by Phillip Chang

	SDrawable

	定義可繪圖物件
*/
#pragma once

#include "slib\SBase.h"

class SDrawable {
public:
	//	initialize
	SDrawable() {
		x_ = y_ = width_ = height_ = 0;
		visible_ = true;
	}

	//	attribute
	inline void SetX(SIN const int x) { x_ = x; }
	inline void SetY(SIN const int y) { y_ = y; }
	inline void SetWidth(SIN const int width) { width_ = width; }
	inline void SetHeight(SIN const int height) { height_ = height; }
	inline void SetVisible(SIN const bool visible) { visible_ = visible; }

	inline int X() const { return x_; }
	inline int Y() const { return y_; }
	inline int Width() const { return width_; }
	inline int Height() const { return height_; }
	inline bool Visible() const { return visible_; }


	/*
		在 x, y 座標上以原大小畫在 hdc 上
	*/
	virtual void DrawXY(SIN HDC main_hdc, HDC hdc, const int x, const int y) const = 0;
	/*
		使用 x_, y_ 以原大小畫在 hdc 上
	*/
	inline void Draw(SIN HDC main_hdc, HDC hdc) const {
		DrawXY(main_hdc, hdc, x_, y_);
	}
	/*
		以 width*height 大小畫在 hdc 上，座標為 x_, y_
	*/
	void Draw(SIN HDC main_hdc, HDC hdc, const int width, const int height) const {
		//	畫在 width_*height 的 temp hdc 在 scale
		HDC temp_hdc = CreateCompatibleDC(main_hdc);
		HBITMAP temp_bmp = CreateCompatibleBitmap(main_hdc, width_, height_), old_bmp;

		old_bmp = (HBITMAP)SelectObject(temp_hdc, temp_bmp);
		DrawXY(main_hdc, temp_hdc, 0, 0);
		SetStretchBltMode(hdc, STRETCH_DELETESCANS);
		StretchBlt(hdc, x_, y_, width, height, temp_hdc, 0, 0, width_, height_, SRCCOPY);
		SelectObject(temp_hdc, old_bmp);

		DeleteObject(temp_bmp);
		DeleteDC(temp_hdc);
	}
protected:
	int x_, y_, width_, height_;
	bool visible_;
};
