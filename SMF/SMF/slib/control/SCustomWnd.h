/*
	Copyright © 2016 by Phillip Chang

	SCustomWnd

	*	定義自訂視窗訊息
	*	自訂視窗的基底
*/
#pragma once

#include "slib\SBase.h"

//	lParam = SLogWnd pointer

/*	告訴 parent 視窗 log 視窗需要更新
	PostMessage
	wParam	: not use
	lParam	: SSLogWnd*
*/
#define SWM_SLOG_UPDATE	(WM_USER+1)

/*	告訴 parent 視窗 edit 視窗傳送了一個指令
	PostMessage
	wParam	: SSEnterEdit*
	lParam	: SSCharAry*
*/
#define SWM_SEDIT_TEXT	(WM_USER+2)

/*	告訴 ASK_WND 一個新的 IO請求
	SendMessage
	wParam	: not use
	lParam	: SSAskItem*
*/
#define SWM_SASK_POST	(WM_USER+3)

/*	告訴視窗一個 button 被按下
	PostMessage/SendMessage
	wParam	: button id
	lParam	: button pointer
*/
#define SWM_SBTN_CLK	(WM_USER+10)

/*	告訴視窗選擇更變 (SSelectList)
	PostMessage
	wParam	: not use
	lParam	: SSelectList
*/
#define SWM_SSL_CHANGE	(WM_USER+20)

/*
	
*/
#define SWM_CHECKSTATECHANGE (WM_USER + 100)


/*
	wnd_	- 視窗本體
	parent_	- 母視窗
*/
class SCustomWnd {
public:
	SCustomWnd() :wnd_(NULL), parent_(NULL) {
		;
	}
	inline HWND Handle() const {
		return wnd_;
	}
	inline HWND Parent() const {
		return parent_;
	}
	inline bool Valid() const {
		return wnd_ != NULL;
	}
	//  重複 Create 會得到原本的 HWND
	HWND Create(SIN	const TCHAR* class_name, const TCHAR* wnd_name, DWORD style, int x, int y, int width, int height,
		HWND parent, HMENU menu, HINSTANCE instance, void* param) {
		if (Valid()) return wnd_;
		parent_ = parent;
		return wnd_ = CreateWindow(class_name, wnd_name, style, x, y, width, height, parent, menu, instance, param);
	}
	void Destroy() {
		if (Valid()) {
			DestroyWindow(wnd_);
			wnd_ = NULL;
			parent_ = NULL;
		}
	}
protected:
	HWND wnd_, parent_;
};
