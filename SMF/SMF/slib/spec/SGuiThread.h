/*
	負責
	1.	產生 GUI 視窗
	2.	
	3.	
*/
#pragma once

#include "SGlobal.h"
#include "SUserMsg.h"

#include "page\SMenu.h"
#include "page\SQuery.h"
#include "page\SRule.h"
#include "page\SEx.h"

namespace ggl {
	const int GUI_WIDTH = 800, GUI_HEIGHT = 600;
	const TCHAR CLS_NAME[] = TEXT("GGL_GUI_WND");	//	unregister 使用

	HWND gui_wnd;
	bool gui_wnd_exit;
	HBRUSH gui_bkgnd;

	void CreateGuiWnd();
	void ProcMsg(SIN SMSG& msg);
	LRESULT CALLBACK GuiMsgProc(SIN HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	namespace dfa {
		/*
			0 - main
			1 - query
			2 - rule
			3 - ex
		*/
		const int
			MAIN_STATE = 0,
			QUERY_STATE = 1,
			RULE_STATE = 2,
			EX_STATE = 3;
		int state;

		//	使用 set state 而不是 enter/leave state
		void SetState(int new_state, HWND wnd);
		void EnterState(HWND wnd);
		void LeaveState(HWND wnd);
	}

}

unsigned int __stdcall GuiProc(SIN void* data) {
	ggl::gui_wnd_exit = false;

	ggl::gui_bkgnd = CreateSolidBrush(RGB(220, 220, 220));

	ggl::CreateGuiWnd();

	MSG msg;
	SMSG smsg;

	msg.message = 0;
	while (!ggl::gui_wnd_exit || msg.message != WM_QUIT) {
		/*
			window msg
		*/
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			DispatchMessage(&msg);
		}

		/*
			queue msg
		*/
		while (sgl::gui_queue.PeekMsg(sgl::TIMEOUT, smsg)) {
			ggl::ProcMsg(smsg);
		}

		Sleep(sgl::IDLE_TIME);
	}
	//	釋放資源
	UnregisterClass(ggl::CLS_NAME, sgl::inst);

	DeleteObject(ggl::gui_bkgnd);

	smsg.type_ = umsg::GUI_CLOSE;
	smsg.param_[0] = 0;
	smsg.ptr_[0] = NULL;

	sgl::hidden_queue.PushMsg(smsg);

	return 0;
}

namespace ggl {
	void CreateGuiWnd() {
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = GuiMsgProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = sgl::inst;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = gui_bkgnd;
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = CLS_NAME;
		wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		RegisterClassEx(&wcex);

		gui_wnd = CreateWindow(CLS_NAME, TEXT("SMF"), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, 0, 0, GUI_WIDTH, GUI_HEIGHT, NULL, NULL, sgl::inst, NULL);

		ShowWindow(gui_wnd, SW_SHOW);
		UpdateWindow(gui_wnd);
	}
	void ProcMsg(SIN SMSG& msg) {
		switch (msg.type_) {
		case umsg::GUI_QUEUE_UPDATE:
			/*
				如果 DFA = QUERY_STATE，更新 SelectList
			*/
			if (dfa::state == dfa::QUERY_STATE) {
				gui::query::UpdateList();
			}
			break;
		}
	}
	LRESULT CALLBACK GuiMsgProc(SIN HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_CREATE:
			gui::Initialize(hWnd);
			gui::menu::Initialize(hWnd);
			gui::query::Initialize(hWnd);
			gui::rule::Initialize(hWnd);
			gui::ex::Initialize(hWnd);

			dfa::state = dfa::MAIN_STATE;
			dfa::EnterState(hWnd);	//	唯一直接呼叫 EnterState 的時機

			break;
		case WM_COMMAND: {
			int wmId = LOWORD(wParam), wmEvent = HIWORD(wParam);
			if (wmEvent == BN_CLICKED && wmId==SWM_SBTN_CLK) {
				HWND button = (HWND)lParam;
				Button_SetCheck(button, (Button_GetCheck(button) == 1 ? 0 : 1));
			}
			break;
		}
		case SWM_SSL_CHANGE:
			gui::rule::SelectChang((SSelectList*)lParam);
			break;
		case SWM_SBTN_CLK: {
			gui::CTLID id = (gui::CTLID)wParam;
			switch (id) {
			case gui::ID_ACTIVE: {
				SCheckConfig check_config;
				sgl::config_mutex.lock();
				sgl::config.GetCheckConfig(check_config);
				check_config.active_ = !check_config.active_;
				sgl::config.SetCheckConfig(check_config);
				sgl::config.SaveFile(CHECK_CONFIG);
				sgl::config_mutex.unlock();
				break;
			}
			case gui::ID_SAFE_MODE: {
				SCheckConfig check_config;
				sgl::config_mutex.lock();
				sgl::config.GetCheckConfig(check_config);
				check_config.safe_mode_ = !check_config.safe_mode_;
				sgl::config.SetCheckConfig(check_config);
				sgl::config.SaveFile(CHECK_CONFIG);
				sgl::config_mutex.unlock();
				break;
			}
			case gui::ID_QUERY:
				dfa::SetState(dfa::QUERY_STATE, hWnd);
				break;
			case gui::ID_RULE:
				dfa::SetState(dfa::RULE_STATE, hWnd);
				break;
			case gui::ID_EX:
				dfa::SetState(dfa::EX_STATE, hWnd);
				break;
			case gui::ID_BACK:
				dfa::SetState(dfa::MAIN_STATE, hWnd);
				break;
			case gui::ID_ACCEPT:
			case gui::ID_REJECT:
			case gui::ID_OPENFILE:
			case gui::ID_OPENEXE:
				gui::query::ButtonProc(id);
				break;
			case gui::ID_PROTECT_PATH:
			case gui::ID_SHARE_PATH:
			case gui::ID_EXE_PATH:
			case gui::ID_WORKSPACE:
			case gui::ID_ADD0:
			case gui::ID_ADD1:
			case gui::ID_DELETE0:
			case gui::ID_DELETE1:
				gui::rule::ButtonProc(id);
				break;
			case gui::ID_APPLY:
			case gui::ID_RESET:
			case gui::ID_RECOVER:
				gui::ex::ButtonProc(id);
				break;
			default:
				break;
			}
			break;
		}
		case WM_PAINT: {
			RECT client_rect;
			HFONT old_font;
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			/*if (dfa::state == dfa::MAIN_STATE) {
				GetClientRect(hWnd, &client_rect);
				client_rect.right -= client_rect.left;
				client_rect.right /= 2;

				MoveToEx(hdc, client_rect.right - ctl::BTN_WIDTH / 2, ctl::BTN_TOP_PADDING, NULL);
				LineTo(hdc, client_rect.right - ctl::BTN_WIDTH / 2, ctl::BTN_TOP_PADDING + ctl::BTN_HEIGHT);

				MoveToEx(hdc, client_rect.right + ctl::BTN_WIDTH / 2, ctl::BTN_TOP_PADDING, NULL);
				LineTo(hdc, client_rect.right + ctl::BTN_WIDTH / 2, ctl::BTN_TOP_PADDING + ctl::BTN_HEIGHT);
				}
				else if (dfa::state == dfa::QUERY_STATE) {
				GetClientRect(hWnd, &client_rect);
				SItemLib::CreateSizeFont(40);
				old_font = (HFONT)SelectObject(hdc, SItemLib::font_);
				SetBkMode(hdc, TRANSPARENT);

				client_rect.left = ctl::BACK_PADDING;
				client_rect.right -= ctl::BACK_PADDING;
				client_rect.top = ctl::BACK_PADDING;
				client_rect.bottom = ctl::BACK_PADDING + ctl::BACK_HEIGHT;
				DrawText(hdc, gui::QUERY_TEXT.Ptr(), gui::QUERY_TEXT.Length(), &client_rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

				SelectObject(hdc, old_font);
				}*/

			EndPaint(hWnd, &ps);
			break;
		}
		case WM_GETMINMAXINFO: {
			MINMAXINFO* minmaxinfo;
			minmaxinfo = (MINMAXINFO*)lParam;
			minmaxinfo->ptMinTrackSize.x = GUI_WIDTH;
			minmaxinfo->ptMinTrackSize.y = GUI_HEIGHT;
			minmaxinfo->ptMaxTrackSize.x = GUI_WIDTH;
			minmaxinfo->ptMaxTrackSize.y = GUI_HEIGHT;
			break;
		}
		case WM_DESTROY:
			gui_wnd_exit = true;
			//	不回收會導致下次無法 create
			dfa::LeaveState(hWnd);		//	唯一直接呼叫 LeaveState 的時機

			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

	namespace dfa {
		//	使用 set state 而不是 enter/leave state
		void SetState(SIN int new_state, HWND wnd) {
			LeaveState(wnd);
			state = new_state;
			EnterState(wnd);
			InvalidateRect(wnd, NULL, TRUE);
		}
		void EnterState(SIN HWND wnd) {
			switch (state) {
			case MAIN_STATE:
				gui::menu::Create(wnd);
				break;
			case QUERY_STATE:
				gui::back_button.Create(wnd, sgl::inst);
				gui::query::Create(wnd);
				break;
			case RULE_STATE:
				gui::back_button.Create(wnd, sgl::inst);
				gui::rule::Create(wnd);
				break;
			case EX_STATE:
				gui::back_button.Create(wnd, sgl::inst);
				gui::ex::Create(wnd);
				break;
			}
		}
		void LeaveState(SIN HWND wnd) {
			int i;
			switch (state) {
			case MAIN_STATE:
				gui::menu::Destroy();
				break;
			case QUERY_STATE:
				gui::back_button.Destroy();
				gui::query::Destroy();
				break;
			case RULE_STATE:
				gui::back_button.Destroy();
				gui::rule::Destroy();
				break;
			case EX_STATE:
				gui::back_button.Destroy();
				gui::ex::Destroy();
				break;
			}
		}
	}

}
