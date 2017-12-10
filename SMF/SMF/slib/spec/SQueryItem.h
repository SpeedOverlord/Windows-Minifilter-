#pragma once

#include "SIorq.h"

#include "slib\SPathLib.h"
#include "slib\control\item\SItemLib.h"
#include "slib\control\item\SDrawableItem.h"

namespace {
	const int CHECKBOX_WIDTH = 40;
	const int ACCESS_PADDING = 110, ACCESS_FONT_PADDING = 10;
}

class SQueryItem : public SDrawableItem {
public:
	virtual void DrawItem(SIN HDC hdc, RECT& rect, bool selected) override {
		TCHAR path_buffer[MAX_PATH];

		SItemLib::Erase(hdc, rect, selected);
		SItemLib::ButtonLine(hdc, rect);
		
		int height = rect.bottom - rect.top;
		RECT check_rect, text_rect;
		check_rect.left = rect.left;
		check_rect.right = rect.left + CHECKBOX_WIDTH;
		check_rect.top = rect.top + (height - CHECKBOX_WIDTH) / 2;
		check_rect.bottom = rect.bottom - (height - CHECKBOX_WIDTH) / 2;

		SItemLib::DrawCheckBox(hdc, check_rect, 3, selected);

		//	���o�r��
		SItemLib::CreateSizeFont(height / 3);
		HFONT old_font = (HFONT)SelectObject(hdc, SItemLib::font_);

		//	���o�r��һݪŶ�
		SIZE text_size;

		//	�e�X ACCESS
		check_rect.left = check_rect.right;
		check_rect.right += ACCESS_PADDING;
		check_rect.top = rect.top;
		check_rect.bottom = rect.bottom;

		int mode_old = SetBkMode(hdc, TRANSPARENT);
		//	str_ �O Allocate ���J�A�ݭ��s�p�����
		check_rect.top += ACCESS_FONT_PADDING;
		DrawText(hdc, str_.Ptr(), (int)sis::str::Length(str_.Ptr()), &check_rect, DT_CENTER);
		check_rect.top -= ACCESS_FONT_PADDING;

		//	check_rect ��s����ܸ��|���ϰ�
		check_rect.left = check_rect.right;
		check_rect.right = rect.right;

		if (!selected) {
			//	�S������ɥu��ܦs�����|
			sis::path::GetFileName(iorq_->file_.Ptr(), path_buffer);
			DrawText(hdc, path_buffer, (int)sis::str::Length(path_buffer), &check_rect, DT_SINGLELINE | DT_VCENTER | DT_PATH_ELLIPSIS);
			return;
		}

		if (iorq_->access_.IO_RENAME) {
			//	��ܨ�Ӹ��|

			//	font
			SelectObject(hdc, old_font);
			SItemLib::CreateSizeFont(height / 3);
			old_font = (HFONT)SelectObject(hdc, SItemLib::font_);

			//	source
			text_rect.left = check_rect.left;
			text_rect.right = check_rect.right;
			text_rect.top = check_rect.top;
			text_rect.bottom = check_rect.top + height / 3;

			DrawText(hdc, iorq_->file_.Ptr(), (int)iorq_->file_.Length(), &text_rect, DT_SINGLELINE | DT_VCENTER | DT_PATH_ELLIPSIS);

			//	�p��b�Y����
			SCharAry arrow(TEXT("=>"));
			GetTextExtentPoint(hdc, arrow.Ptr(), (int)arrow.Length(), &text_size);

			text_rect.top = text_rect.bottom;
			text_rect.bottom += height / 3;

			SetTextColor(hdc, RGB(255, 0, 0));
			DrawText(hdc, arrow.Ptr(), (int)arrow.Length(), &text_rect, DT_SINGLELINE | DT_VCENTER);
			SetTextColor(hdc, RGB(0, 0, 0));

			//	��ܥؼи��|
			text_rect.left += text_size.cx;

			DrawText(hdc, iorq_->des_.Ptr(), (int)iorq_->des_.Length(), &text_rect, DT_SINGLELINE | DT_VCENTER | DT_PATH_ELLIPSIS);

			//	��� exe ���|
			text_rect.left = check_rect.left;
			text_rect.top = text_rect.bottom;
			text_rect.bottom = check_rect.bottom;

			sis::path::GetFileName(iorq_->exe_.Ptr(), path_buffer);
			DrawText(hdc, path_buffer, (int)sis::str::Length(path_buffer), &text_rect, DT_SINGLELINE | DT_VCENTER | DT_PATH_ELLIPSIS);
		}
		else {
			//	��ܤ@�Ӹ��|

			//	font
			SelectObject(hdc, old_font);
			SItemLib::CreateSizeFont(height / 2);
			old_font = (HFONT)SelectObject(hdc, SItemLib::font_);

			//	��� exe ���|
			text_rect.left = check_rect.left;
			text_rect.right = check_rect.right;
			text_rect.top = (check_rect.top + check_rect.bottom) / 2;
			text_rect.bottom = check_rect.bottom;

			sis::path::GetFileName(iorq_->exe_.Ptr(), path_buffer);
			DrawText(hdc, path_buffer, (int)sis::str::Length(path_buffer), &text_rect, DT_SINGLELINE | DT_VCENTER | DT_PATH_ELLIPSIS);

			//	��� file ���g

			text_rect.bottom = text_rect.top;
			text_rect.top = check_rect.top;

			DrawText(hdc, iorq_->file_.Ptr(), (int)iorq_->file_.Length(), &text_rect, DT_SINGLELINE | DT_VCENTER | DT_PATH_ELLIPSIS);
		}

		SetBkColor(hdc, mode_old);
		SelectObject(hdc, old_font);
	}
	/*
		�]�w iorq ����t str_ �Ŷ��ö�J�A�_�h SelectList->AddString ���ɭ� str_.Ptr() �O NULL �|�z��
	*/
	void GetAccessString(SINOUT TCHAR* str) {
		bool slash = false;
		size_t index = 0;

		if (iorq_->access_.IO_READ) {
			if (slash) str[index++] = TEXT('/');
			wsprintf(str + index, TEXT("%s"), TEXT("Ū��"));
			index += 2;
			slash = true;
		}

		if (iorq_->access_.IO_WRITE) {
			if (slash) str[index++] = TEXT('/');
			wsprintf(str + index, TEXT("%s"), TEXT("�g�J"));
			index += 2;
			slash = true;
		}

		if (iorq_->access_.IO_DELETE) {
			if (slash) str[index++] = TEXT('/');
			wsprintf(str + index, TEXT("%s"), TEXT("�R��"));
			index += 2;
			slash = true;
		}

		if (iorq_->access_.IO_OVERWRITE) {
			if (slash) str[index++] = TEXT('/');
			wsprintf(str + index, TEXT("%s"), TEXT("�мg"));
			index += 2;
			slash = true;
		}

		if (iorq_->access_.IO_RENAME) {
			if (slash) str[index++] = TEXT('/');
			wsprintf(str + index, TEXT("%s"), TEXT("����"));
			index += 2;
			slash = true;
		}

		//str[index++] = 0;
		wsprintf(str + index, TEXT("\n%d%d:%d%d"),
			iorq_->time_.wHour / 10, iorq_->time_.wHour % 10,
			iorq_->time_.wMinute / 10, iorq_->time_.wMinute % 10);
	}
	SIORQ* iorq_;
};
