/*
	Copyright © 2016 by Phillip Chang

	SDrawable

	定義圖像
*/
#pragma once

#include "SDrawable.h"

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

class SGdiPlusData {
public:
	SGdiPlusData() {
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	}
	~SGdiPlusData() {
		Gdiplus::GdiplusShutdown(gdiplusToken);
	}
protected:
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
}std_gdiplus;

class SImage : public SDrawable {
public:
	SImage() {
		data_ = NULL;
	}
	~SImage() {
		Clear();
	}
	void Clear() {
		if (data_ != NULL) {
			DeleteObject(data_);
			data_ = NULL;
		}
	}
	bool Load(SIN const TCHAR* path) {
		Gdiplus::Bitmap bitmap(path);

		width_ = (int)bitmap.GetWidth();
		height_ = (int)bitmap.GetHeight();

		return bitmap.GetHBITMAP(RGB(255, 255, 255), &data_) == Gdiplus::Ok;
	}
	void Scale(SIN HDC main_hdc, const int width, const int height) {
		HDC temp_hdc = CreateCompatibleDC(main_hdc);
		HBITMAP data = CreateCompatibleBitmap(main_hdc, width, height);
		HBITMAP old_bmp = (HBITMAP)SelectObject(temp_hdc, data);

		int org_x = x_, org_y = y_;
		x_ = y_ = 0;

		Draw(main_hdc, temp_hdc, width, height);

		x_ = org_x;	y_ = org_y;

		SelectObject(temp_hdc, old_bmp);
		DeleteDC(temp_hdc);

		DeleteObject(data_);
		data_ = data;
		width_ = width;
		height_ = height;
	}

	//	copy
	void Copy(SIN HDC main_hdc, SImage& img) {
		Clear();

		width_ = img.width_;
		height_ = img.height_;

		HDC temp_hdc[2] = { CreateCompatibleDC(main_hdc), CreateCompatibleDC(main_hdc) };
		data_ = CreateCompatibleBitmap(main_hdc, width_, height_);
		HBITMAP old_bmp[2] = { (HBITMAP)SelectObject(temp_hdc[0], data_), (HBITMAP)SelectObject(temp_hdc[1], img.data_) };

		BitBlt(temp_hdc[0], 0, 0, width_, height_, temp_hdc[1], 0, 0, SRCCOPY);

		SelectObject(temp_hdc[0], old_bmp[0]);
		SelectObject(temp_hdc[1], old_bmp[1]);
		DeleteDC(temp_hdc[0]);
		DeleteDC(temp_hdc[1]);
	}

	//	virtual
	virtual void DrawXY(SIN HDC main_hdc, HDC hdc, const int x, const int y) const override {
		HDC temp_hdc = CreateCompatibleDC(main_hdc);
		HBITMAP old_bmp = (HBITMAP)SelectObject(temp_hdc, data_);

		BitBlt(hdc, x, y, width_, height_, temp_hdc, 0, 0, SRCCOPY);

		SelectObject(temp_hdc, old_bmp);
		DeleteDC(temp_hdc);
	}
protected:
	HBITMAP data_;
};
