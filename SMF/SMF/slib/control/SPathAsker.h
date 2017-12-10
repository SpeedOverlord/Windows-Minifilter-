#pragma once

#include "SCustomWnd.h"
#include "STreeView.h"

const TCHAR SPA_WND_CLS[] = TEXT("SPAWND");

namespace sis {
	namespace win32 {
		INT_PTR CALLBACK DefSPathAskerProc(SIN HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	}
}

struct AskResult {
	bool update_;					//	是否按下"確定"
	std::vector<SCharAry> result_;	//	新路徑設定
};

class SPathAsker;

namespace {
	const int CTL_PADDING = 10, BTN_WIDTH = 75, BTN_HEIGHT = 35;
	const int ACCEPT_ID = 300, CANCEL_ID = 301;
	struct PathAskerCreateParam {
		SPathAsker* asker_;
		std::vector<SCharAry>* config_;
		HINSTANCE inst_;
		const TCHAR* title_;
	};
}

class SPathAsker : public SCustomWnd, public AskResult {
public:
	friend INT_PTR CALLBACK sis::win32::DefSPathAskerProc(SIN HWND, UINT, WPARAM, LPARAM);
	static HBRUSH background_brush_;
	HWND Create(SIN const TCHAR* title, HWND parent, HINSTANCE instance, std::vector<SCharAry>& preconfig) {
		if (Valid()) return wnd_;

		update_ = false;

		PathAskerCreateParam param;

		param.asker_ = this;
		param.config_ = &preconfig;
		param.inst_ = instance;
		param.title_ = title;

		parent_ = parent;

		//
		char* buf = new char[1024];
		WORD *wdptr = NULL;
		LPDLGTEMPLATE dlg_template = (LPDLGTEMPLATE)buf;
		dlg_template->style = WS_POPUP | WS_THICKFRAME | WS_CAPTION;
		dlg_template->dwExtendedStyle = 0;
		dlg_template->cdit = 0;
		dlg_template->x = 10;	dlg_template->y = 10;
		dlg_template->cx = 200;	dlg_template->cy = 300;

		wdptr = (WORD*)(dlg_template + 1);
		*wdptr++ = 0;
		*wdptr++ = 0;
		sis::str::Copy(title, sis::str::Length(title) + 1, (TCHAR*)wdptr);

		DialogBoxIndirectParam(instance, (LPDLGTEMPLATE)buf, parent, sis::win32::DefSPathAskerProc, (LPARAM)&param);

		delete[]buf;

		return (HWND)INVALID_HANDLE_VALUE;
	}
private:
	using SCustomWnd::Create;
	
	STreeView tree_;
	HWND accept_, cancel_;
};

HBRUSH SPathAsker::background_brush_ = NULL;

namespace sis {
	namespace win32 {
		INT_PTR CALLBACK DefSPathAskerProc(SIN HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
			SPathAsker *data = (SPathAsker*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			switch (message) {
			case WM_INITDIALOG/*WM_CREATE*/: {
				PathAskerCreateParam *create_param = (PathAskerCreateParam*)lParam;
				RECT rect;

				GetClientRect(hWnd, &rect);

				data = create_param->asker_;
				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data);

				data->tree_.Create(CTL_PADDING, CTL_PADDING,
					rect.right - 2 * CTL_PADDING, rect.bottom - 3 * CTL_PADDING - BTN_HEIGHT,
					hWnd, create_param->inst_, *create_param->config_);
				data->accept_ = CreateWindow(WC_BUTTON, TEXT("確定"), WS_CHILD | WS_VISIBLE,
					rect.right - 2 * CTL_PADDING - 2 * BTN_WIDTH, rect.bottom - CTL_PADDING - BTN_HEIGHT,
					BTN_WIDTH, BTN_HEIGHT,
					hWnd, (HMENU)ACCEPT_ID, create_param->inst_, NULL);
				data->cancel_ = CreateWindow(WC_BUTTON, TEXT("取消"), WS_CHILD | WS_VISIBLE,
					rect.right - CTL_PADDING - BTN_WIDTH, rect.bottom - CTL_PADDING - BTN_HEIGHT,
					BTN_WIDTH, BTN_HEIGHT,
					hWnd, (HMENU)CANCEL_ID, create_param->inst_, NULL);
				break;
			}
			case WM_COMMAND: {
				int wm_event = HIWORD(wParam), wm_id = LOWORD(wParam);
				if (wm_event == BN_CLICKED) {
					switch (wm_id) {
					case ACCEPT_ID: {
						std::vector<TCHAR*> tv_result;
						size_t result_cnt;
						data->update_ = true;
						data->tree_.GetList(tv_result);
						result_cnt = tv_result.size();
						data->result_.resize(result_cnt);
						for (int i = 0; i < result_cnt; ++i) {
							data->result_[i] = tv_result[i];
						}
						DestroyWindow(hWnd);
						break;
					}
					case CANCEL_ID:
						data->update_ = false;
						DestroyWindow(hWnd);
						break;
					default:
						break;
					}
				}
				break;
			}
			case WM_SIZE: {
				MoveWindow(data->tree_.Handle(), CTL_PADDING, CTL_PADDING,
					LOWORD(lParam) - 2 * CTL_PADDING, HIWORD(lParam) - 3 * CTL_PADDING - BTN_HEIGHT, TRUE);
				MoveWindow(data->accept_, LOWORD(lParam) - 2 * CTL_PADDING - 2 * BTN_WIDTH, HIWORD(lParam) - CTL_PADDING - BTN_HEIGHT,
					BTN_WIDTH, BTN_HEIGHT, TRUE);
				MoveWindow(data->cancel_, LOWORD(lParam) - CTL_PADDING - BTN_WIDTH, HIWORD(lParam) - CTL_PADDING - BTN_HEIGHT, 
					BTN_WIDTH, BTN_HEIGHT, TRUE);
				break;
			}
			case WM_GETMINMAXINFO: {
				MINMAXINFO* minmaxinfo;
				minmaxinfo = (MINMAXINFO*)lParam;
				minmaxinfo->ptMinTrackSize.x = 300;
				minmaxinfo->ptMinTrackSize.y = 400;
				//minmaxinfo->ptMaxTrackSize.x = 800;
				//minmaxinfo->ptMaxTrackSize.y = 600;
				break;
			}
			case WM_PAINT: {
				//	draw
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				RECT rect;
				GetClientRect(hWnd, &rect);
				FillRect(hdc, &rect, SPathAsker::background_brush_);
				// TODO:  在此加入任何繪圖程式碼...
				EndPaint(hWnd, &ps);
				break;
			}
			case WM_DESTROY:
				data->tree_.Destroy();
				DestroyWindow(data->accept_);
				DestroyWindow(data->cancel_);

				EndDialog(hWnd, 0);
				break;
			default:
				return FALSE;
			}
			return TRUE;
		}
	}
}
