/*
	Copyright © 2016 by Phillip Chang

	SButton

	按鍵接口
*/
#pragma once

#include "SCustomWnd.h"

#include "slib\display\SDrawable.h"

const TCHAR SSBTN_WND_CLS[] = TEXT("SSBTNWND");

namespace sis {
	namespace win32 {
		LRESULT CALLBACK DefSSButtonProc(SIN HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	}
}

/*
	按鈕被按下的時候會傳給母視窗 SWM_SBT_CLK

	訊息應藉由 ButtonGroup 傳遞
	Hover->Unhover
	(Hover->)Down->(Leave Rect)->Undown(->Unhover)
	(Hover->)Down->(Release)->Undown->Up(->Unhover)

	當滑鼠是按下的狀態，ButtonGroup 會傳送 MouseDown 代替 Move，因此 MouseDown 應在必要時呼叫 Move

	繼承 SDrawable 的屬性
	x_, y_ 是相對於 parent 視窗左上角 (0, 0) 的座標
*/
class SButton : public SDrawable, public SCustomWnd {
public:
	//	reg
	static bool reg_;
	static ATOM RegClass(SIN HINSTANCE inst) {
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = sis::win32::DefSSButtonProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = inst;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = SSBTN_WND_CLS;
		wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		reg_ = true;
		return RegisterClassEx(&wcex);
	}

	//	initialize
	SButton() :id_(0) { ; }

	//	ID
	inline void SetID(SIN const int id) { id_ = id; }
	inline int ID() const { return id_; }

	/*
		CreateWindows
	*/
	HWND Create(SIN HWND parent, HINSTANCE instance) {
		if (!reg_) RegClass(instance);
		hover_ = button_down_ = false;
		return Create(SSBTN_WND_CLS, TEXT(""), WS_CHILD | WS_VISIBLE, x_, y_, width_, height_, parent, NULL, instance, this);
	}

	/*	
		判斷滑鼠是否進入按鈕區
		回傳是否需要重畫
		
		x, y	- 按鈕座標
	*/
	bool Move(SIN const int x, const int y) {
		if (Enter(x, y)) {
			if (!hover_) {
				hover_ = true;
				return Hover();
			}
		}
		else {
			if (hover_) {
				hover_ = false;
				return Hover();
			}
		}
		return false;
	}
	/*
		判斷滑鼠是否按到按鈕
		回傳是否需要重畫

		x, y	- 按鈕座標
	*/
	bool MouseDown(SIN const int x, const int y) {
		bool result = false;
		if (Enter(x, y)) {
			if (!button_down_) {
				button_down_ = true;
				if (!hover_) result |= Move(x, y);
				return result | ButtonDown();
			}
		}
		else {
			if (button_down_) {
				button_down_ = false;
				result |= ButtonDown();
				return result | Move(x, y);
			}
		}
		return false;
	}
	bool MouseUp(SIN const int x, const int y) {
		bool result = false;
		if (Enter(x, y)) {
			button_down_ = false;
			result |= ButtonDown();
			return result | ButtonUp();
		}
		else {
			return false;
		}
	}
	/*
		滑鼠離開
		將狀態全部移除
	*/
	bool MouseLeave() {
		bool result = false;
		result |= Move(width_, height_);
		result |= MouseUp(width_, height_);
		StopTrack();
		return result;
	}
	/*	
		判斷座標是否在按鈕中

		預設使用普通長方形 button

		x, y	- 按鈕座標
	*/
	virtual bool Enter(SIN const int x, const int y) const {
		return x >= 0 && x < width_ && y >= 0 && y < height_;
	}
	/*
		滑鼠進入/離開按鈕區
		使用 hover_ 得知目前狀態
		回傳是否要重畫

		預設不理會 hover 訊息
	*/
	virtual bool Hover() {
		return false;
	}
	/*
		按鈕被按住，或是取消按住狀態
		回傳是否需要重畫

		預設不需要重繪
	*/
	virtual bool ButtonDown() {
		return false;
	}
	/*	
		按鈕被放開
		回傳是否需要重畫

		預設傳送按鈕訊息，不重繪
	*/
	virtual bool ButtonUp() {
		PostMessage(parent_, SWM_SBTN_CLK, id_, (LPARAM)this);
		return false;
	}

	/*
		track
	*/
	void StartTrack() {
		if (track_) return;
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = wnd_;
		tme.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&tme);

		track_ = true;
	}
	void StopTrack() {
		if (!track_) return;
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_CANCEL | TME_LEAVE;
		tme.hwndTrack = wnd_;
		tme.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&tme);

		track_ = false;
	}
	void Destroy() {
		if (track_) StopTrack();
		SCustomWnd::Destroy();
	}

	bool track_;	//	only for wnd proc
protected:
	using SCustomWnd::Create;
	using SCustomWnd::Destroy;

	int id_;	//	用於識別按鈕
	bool hover_, button_down_;
};

bool SButton::reg_ = false;

namespace sis {
	namespace win32 {
		LRESULT CALLBACK DefSSButtonProc(SIN HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
			//	draw
			PAINTSTRUCT ps;
			HDC hdc;
			//	wnd data
			SButton *data = (SButton*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			//	create param
			CREATESTRUCT* create_strcut = NULL;
			//	mouse
			int x, y;

			switch (message) {
			case WM_CREATE:
				//	use construct param instead of data from GetWindowLongPtr
				create_strcut = (CREATESTRUCT*)lParam;
				data = (SButton*)create_strcut->lpCreateParams;
				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data);
				break;
			case WM_MOUSELEAVE:
				if (data->MouseLeave()) InvalidateRect(hWnd, NULL, true);
				break;
			case WM_MOUSEMOVE:
				x = GET_X_LPARAM(lParam);
				y = GET_Y_LPARAM(lParam);
				data->StartTrack();
				if (data->Move(x, y)) InvalidateRect(hWnd, NULL, true);
				break;
			case WM_LBUTTONDOWN:
				x = GET_X_LPARAM(lParam);
				y = GET_Y_LPARAM(lParam);
				if (data->MouseDown(x, y)) InvalidateRect(hWnd, NULL, true);
				break;
			case WM_LBUTTONUP:
				x = GET_X_LPARAM(lParam);
				y = GET_Y_LPARAM(lParam);
				if (data->MouseUp(x, y)) InvalidateRect(hWnd, NULL, true);
				break;
			case WM_ERASEBKGND:
				return 1;
			case WM_PAINT:
				hdc = BeginPaint(hWnd, &ps);
				// TODO:  在此加入任何繪圖程式碼...
				data->DrawXY(hdc, hdc, 0, 0);
				EndPaint(hWnd, &ps);
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			return 0;
		}
	}
}
