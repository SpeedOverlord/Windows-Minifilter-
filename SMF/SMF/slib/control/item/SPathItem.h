/*
	Copyright © 2016 by Phillip Chang

	SPathItem

	顯示路徑的物件
*/
#pragma once

#include "SDrawableItem.h"
#include "SItemLib.h"

class SPathItem : public SDrawableItem {
public:
	virtual void DrawItem(SIN HDC hdc, RECT& rect, bool selected) override {
		SItemLib::Erase(hdc, rect, selected);

		RECT check_rect;
		check_rect.left = rect.left;
		check_rect.right = rect.left + rect.bottom - rect.top;
		check_rect.top = rect.top;
		check_rect.bottom = rect.bottom;

		SItemLib::DrawCheckBox(hdc, check_rect, 3, selected);

		check_rect.left = check_rect.right;
		check_rect.right = rect.right;

		SItemLib::CreateSizeFont(rect.bottom - rect.top);
		HFONT old_font = (HFONT)SelectObject(hdc, SItemLib::font_);

		int mode_old = SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, str_.Ptr(), (int)str_.Length(), &check_rect, DT_VCENTER | DT_PATH_ELLIPSIS);
		SetBkMode(hdc, mode_old);

		SelectObject(hdc, old_font);
	}
};