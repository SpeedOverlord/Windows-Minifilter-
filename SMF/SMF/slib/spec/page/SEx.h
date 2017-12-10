/*
	Menu 的控制項
*/
#pragma once

#include "slib\spec\SGlobal.h"
#include "slib\spec\SConfig.h"
#include "slib\spec\SGuiImport.h"

#include "slib\control\SPushButton.h"

namespace gui {
	namespace ex {
		namespace {
			const int TEXT_MAX_LENGTH = 20,
				CFG_HEIGHT = 28, CFG_PADDING = 3;

			const int BTN_CNT = 4;
			const TCHAR *BTN_TEXT[BTN_CNT] = {
				TEXT("讀取直接通過"),
				TEXT("未定義路徑直接通過"),
				TEXT("將 C:\\Program File 下的子目錄視為沙盒路徑"),
				TEXT("不阻擋請求，僅將警告儲存於檔案中")
			};
			HWND button[BTN_CNT];
			SPushButton apply_button, reset_button, recover_button;

			SImage apply_img[2], reset_img[2], recover_img[2];

			void Set(SCheckConfig& ccfg) {
				Button_SetCheck(button[0], ccfg.read_pass_);
				Button_SetCheck(button[1], ccfg.block_only_warning_);
				Button_SetCheck(button[2], ccfg.program_file_);
				Button_SetCheck(button[3], ccfg.log_only_);
			}
			void Apply() {
				SCheckConfig ccfg;
				sgl::config_mutex.lock();
				sgl::config.GetCheckConfig((ccfg));

				ccfg.read_pass_ = Button_GetCheck(button[0]);
				ccfg.block_only_warning_ = Button_GetCheck(button[1]);
				ccfg.program_file_ = Button_GetCheck(button[2]);
				ccfg.log_only_ = Button_GetCheck(button[3]);

				sgl::config_mutex.unlock();
			}
			void Reset() {
				SCheckConfig ccfg;
				Set(ccfg);
			}
			void Recover() {
				SCheckConfig ccfg;
				sgl::config_mutex.lock();
				sgl::config.GetCheckConfig((ccfg));
				sgl::config_mutex.unlock();
				
				Set(ccfg);
			}
		}
		void Initialize(SIN HWND wnd) {
			HDC hdc = GetDC(wnd);

			img::ShortLoad(TEXT("apply"), hdc, OPER_WIDTH, OPER_HEIGHT, apply_img);
			img::ShortLoad(TEXT("reset"), hdc, OPER_WIDTH, OPER_HEIGHT, reset_img);
			img::ShortLoad(TEXT("recover"), hdc, OPER_WIDTH, OPER_HEIGHT, recover_img);

			ReleaseDC(wnd, hdc);

			RECT rect;
			GetClientRect(wnd, &rect);

			ctl::ShortSetPushButton(apply_img,
				rect.right - OPER_PADDING - OPER_WIDTH / 2 - OPER_WIDTH * 2, rect.bottom - OPER_PADDING - OPER_HEIGHT / 2,
				ID_APPLY,
				&apply_button);
			ctl::ShortSetPushButton(reset_img,
				rect.right - OPER_PADDING - OPER_WIDTH / 2 - OPER_WIDTH, rect.bottom - OPER_PADDING - OPER_HEIGHT / 2,
				ID_RESET,
				&reset_button);
			ctl::ShortSetPushButton(recover_img,
				rect.right - OPER_PADDING - OPER_WIDTH / 2, rect.bottom - OPER_PADDING - OPER_HEIGHT / 2,
				ID_RECOVER,
				&recover_button);
		}
		void Create(SIN HWND wnd) {
			int i, y = OPER_PADDING * 2 + BACK_HEIGHT;

			RECT rect;
			GetClientRect(wnd, &rect);
			rect.right -= OPER_PADDING * 2;

			SRP(i, BTN_CNT) {
				button[i] = CreateWindow(WC_BUTTON, BTN_TEXT[i],
					WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_TEXT,
					OPER_PADDING, y, rect.right, CFG_HEIGHT, wnd, (HMENU)SWM_SBTN_CLK, sgl::inst, NULL);
				y += CFG_HEIGHT + CFG_PADDING;
			}

			apply_button.Create(wnd, sgl::inst);
			reset_button.Create(wnd, sgl::inst);
			recover_button.Create(wnd, sgl::inst);

			Recover();
		}
		void Destroy() {
			int i;
			SRP(i, BTN_CNT) {
				DestroyWindow(button[i]);
			}
			apply_button.Destroy();
			reset_button.Destroy();
			recover_button.Destroy();
		}
		void ButtonProc(SIN CTLID id) {
			switch (id) {
			case ID_APPLY:
				Apply();
				break;
			case ID_RESET:
				Reset();
				break;
			case ID_RECOVER:
				Recover();
				break;
			}
		}
	}
}
