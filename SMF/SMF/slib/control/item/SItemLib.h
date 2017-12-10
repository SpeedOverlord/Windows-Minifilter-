/*
	Copyright © 2016 by Phillip Chang

	SSItemLib

	顯示路徑的物件
*/
#pragma once

#include "slib\SBase.h"

class SItemLib {
public:
	static HFONT font_;
	static int font_size_;

	static bool erase_brush_initialize_;
	static HBRUSH erase_brush_[2];

	static void Erase(SIN HDC hdc, RECT& rect, bool selected) {
		if (!erase_brush_initialize_) {
			erase_brush_[0] = CreateSolidBrush(RGB(51, 143, 255));
			erase_brush_[1] = CreateSolidBrush(RGB(255, 255, 255));

			erase_brush_initialize_ = true;
		}
		FillRect(hdc, &rect, selected ? erase_brush_[0] : erase_brush_[1]);
	}
	static void ButtonLine(SIN HDC hdc, RECT& rect) {
		//	item 之間會有 1 pixel 重疊，所以上下都畫
		MoveToEx(hdc, rect.left, rect.top, NULL);
		LineTo(hdc, rect.right, rect.top);

		MoveToEx(hdc, rect.left, rect.bottom, NULL);
		LineTo(hdc, rect.right, rect.bottom);
	}
	static void CreateSizeFont(SIN int height) {
		if (font_size_ == height) return;
		if (font_ != NULL) DeleteObject(font_);
		font_ = CreateFont(height, 0, 0, 0
			, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			FIXED_PITCH | FF_DONTCARE, NULL);
		font_size_ = height;
	}
	static void DrawCheckBox(SIN HDC hdc, RECT& rect, int line_width, bool check) {
		int width = rect.right - rect.left, height = rect.bottom - rect.top;
		int size_w = width >> 1, size_h = height >> 1;
		int x = rect.left + (width - size_w) / 2, y = rect.top + (height - size_h) / 2;

		HPEN pen = CreatePen(PS_SOLID, line_width, RGB(0, 0, 0)), pen_old = (HPEN)SelectObject(hdc, pen);
		//	rect
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, x + size_w, y);
		LineTo(hdc, x + size_w, y + size_h);
		LineTo(hdc, x, y + size_h);
		LineTo(hdc, x, y);

		SelectObject(hdc, pen_old);
		DeleteObject(pen);

		//	check
		if (check) {
			pen = CreatePen(PS_SOLID, line_width * 4 / 3, RGB(0, 0, 0));
			pen_old = (HPEN)SelectObject(hdc, pen);

			MoveToEx(hdc, x + size_w / 6, y + size_h / 6 + size_h / 3, NULL);
			LineTo(hdc, x + size_w / 2, y + size_h / 2 + size_h / 3);
			LineTo(hdc, x + size_w + size_w / 4, y - size_h / 4);

			SelectObject(hdc, pen_old);
			DeleteObject(pen);
		}
	}
};

HFONT SItemLib::font_ = NULL;
int SItemLib::font_size_ = 0;

bool SItemLib::erase_brush_initialize_ = false;
HBRUSH SItemLib::erase_brush_[2] = { (HBRUSH)INVALID_HANDLE_VALUE, (HBRUSH)INVALID_HANDLE_VALUE };
