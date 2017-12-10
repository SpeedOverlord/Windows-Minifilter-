/*
	Menu ªº±±¨î¶µ
*/
#pragma once

#include "slib\spec\SGlobal.h"
#include "slib\spec\SGuiImport.h"

#include "slib\control\SPushButton.h"
#include "slib\control\SStateButton.h"
#include "slib\control\S2StateRoundButton.h"

namespace gui {
	namespace menu {
		namespace {
			/*
				data
			*/
			//	const
			const int
				ACTIVE_WIDTH = 300, SAFE_WIDTH = 50, ACTIVE_PADDING = 30,
				BTN_WIDTH = 250, BTN_HEIGHT = 125, BTN_TOP_PADDING = 20;
			//	control
			SPushButton query_button, rule_button, ex_button;
			S2StateRoundButton active, safe_mode;
			//	image
			SImage active_img[4], safe_mode_img[4],
				query_img[2], rule_img[2], ex_img[2];
		}
		void Initialize(SIN HWND wnd) {
			HDC hdc = GetDC(wnd);

			img::ShortLoad(TEXT("query"), hdc, BTN_WIDTH, BTN_HEIGHT, query_img);
			img::ShortLoad(TEXT("rule"), hdc, BTN_WIDTH, BTN_HEIGHT, rule_img);
			img::ShortLoad(TEXT("ex"), hdc, BTN_WIDTH, BTN_HEIGHT, ex_img);

			img::ShortLoad(TEXT("active_on"), hdc, ACTIVE_WIDTH, ACTIVE_WIDTH, active_img);
			img::ShortLoad(TEXT("active_off"), hdc, ACTIVE_WIDTH, ACTIVE_WIDTH, active_img + 2);

			img::ShortLoad(TEXT("safe_on"), hdc, SAFE_WIDTH, SAFE_WIDTH, safe_mode_img);
			img::ShortLoad(TEXT("safe_off"), hdc, SAFE_WIDTH, SAFE_WIDTH, safe_mode_img + 2);

			ReleaseDC(wnd, hdc);

			RECT rect;
			GetClientRect(wnd, &rect);

			//	query
			ctl::ShortSetPushButton(
				query_img,
				rect.right / 2 - BTN_WIDTH, BTN_HEIGHT / 2 + BTN_TOP_PADDING,
				ID_QUERY,
				&query_button);

			//	rule
			ctl::ShortSetPushButton(
				rule_img,
				rect.right / 2, BTN_HEIGHT / 2 + BTN_TOP_PADDING,
				ID_RULE,
				&rule_button);

			//	ex
			ctl::ShortSetPushButton(
				ex_img,
				rect.right / 2 + BTN_WIDTH, BTN_HEIGHT / 2 + BTN_TOP_PADDING,
				ID_EX,
				&ex_button);

			//	active
			ctl::ShortSetStateButton(
				active_img,
				rect.right / 2, rect.bottom / 2 + ACTIVE_PADDING,
				ID_ACTIVE,
				&active
				);

			//	safe mode
			ctl::ShortSetStateButton(
				safe_mode_img,
				(rect.right + ACTIVE_WIDTH + SAFE_WIDTH) / 2, (rect.bottom) / 2 + ACTIVE_PADDING * 2,
				ID_SAFE_MODE,
				&safe_mode
				);
		}
		void Create(SIN HWND wnd) {
			query_button.Create(wnd, sgl::inst);
			rule_button.Create(wnd, sgl::inst);
			ex_button.Create(wnd, sgl::inst);

			active.Create(wnd, sgl::inst);
			safe_mode.Create(wnd, sgl::inst);

			sgl::config_mutex.lock();
			active.state_ = sgl::config.Active() ? 0 : 1;
			safe_mode.state_ = sgl::config.SafeMode() ? 0 : 1;
			sgl::config_mutex.unlock();
		}
		void Destroy() {
			query_button.Destroy();
			rule_button.Destroy();
			ex_button.Destroy();

			active.Destroy();
			safe_mode.Destroy();
		}
	}
}
