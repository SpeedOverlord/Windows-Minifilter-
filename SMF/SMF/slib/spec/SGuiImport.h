/*
	���Ѱ�¦�]�w���
*/
#pragma once

#include "slib\\SCharAry.h"

#include "slib\control\SPushButton.h"
#include "slib\control\SStateButton.h"

namespace gui {
	enum CTLID {
		ID_QUERY = 0,
		ID_RULE,
		ID_EX,
		ID_ACTIVE,
		ID_SAFE_MODE,

		ID_BACK,

		ID_ACCEPT,
		ID_REJECT,
		ID_OPENFILE,
		ID_OPENEXE,

		ID_PROTECT_PATH,
		ID_SHARE_PATH,
		ID_EXE_PATH,
		ID_WORKSPACE,

		ID_ADD0,
		ID_ADD1,
		ID_DELETE0,
		ID_DELETE1,

		ID_APPLY,
		ID_RESET,
		ID_RECOVER
	};
	/*
		common const
	*/
	const int OPER_WIDTH = 125, OPER_HEIGHT = 50, OPER_PADDING = 20;
	/*
		common button
	*/
	const int
		BACK_WIDTH = 125, BACK_HEIGHT = 50, BACK_PADDING = 20;
	SPushButton back_button;
	SImage back_img[2];
	/*
		special text for display page
	*/
	const SCharAry QUERY_TEXT(TEXT("�i�ö���"));
	/*
		function
	*/
	namespace ctl {
		/*
			�]�w�ؼХ��W���ؼСA�H�F�줤�߬� x, y
		*/
		void SetCenter(SINOUT SDrawable* drawable, SIN int x, int y) {
			drawable->SetX(x - drawable->Width() / 2);
			drawable->SetY(y - drawable->Height() / 2);
		}
		/*
			�ֳt�N push button �Ѽƶ�J
		*/
		void ShortSetPushButton(SIN SImage* image_set, int center_x, int center_y, int id, SOUT SPushButton* btn) {
			btn->SetImage(image_set, image_set + 1, image_set + 1);
			btn->SetWidth(image_set[0].Width());
			btn->SetHeight(image_set[0].Height());
			SetCenter(btn, center_x, center_y);
			btn->SetID(id);
		}
		/*
			�ֳt�N state button �Ѽƶ�J
		*/
		template<int ST>
		void ShortSetStateButton(SIN SImage* image_set, int center_x, int center_y, int id, SOUT SStateButton<ST>* btn) {
			int i;
			SRP(i, ST) {
				btn->img_[i*button::IMAGE_CNT] = &(image_set[i * 2]);
				btn->img_[i*button::IMAGE_CNT + 1] = &(image_set[i * 2 + 1]);
				btn->img_[i*button::IMAGE_CNT + 2] = &(image_set[i * 2 + 1]);
			}
			btn->SetWidth(image_set[0].Width());
			btn->SetHeight(image_set[0].Height());
			SetCenter(btn, center_x, center_y);
			btn->SetID(id);
		}
	}
	namespace img {
		/*
			�^�ǫO�� org_width, org_height ��Ҫ�����U�A�����R fit_width, fit_height �����e
		*/
		void AdjustSize(SIN int fit_width, int fit_height, int org_width, int org_height, SOUT int& result_width, int& result_height) {
			int temp = org_height*fit_width / org_width;
			if (temp <= fit_height) {
				result_width = fit_width;
				result_height = temp;
			}
			else {
				result_width = org_width*fit_height / org_height;
				result_height = fit_height;
			}
		}
		/*
			�N�Ϥ��Y��H�ŦX���w���e
		*/
		void ScaleFit(SINOUT SImage& img, SIN HDC main_hdc, int width, int height) {
			int scale_width = 1, scale_height = 1;
			AdjustSize(width, height, img.Width(), img.Height(), scale_width, scale_height);
			img.Scale(main_hdc, scale_width, scale_height);
		}
		/*
			�ֳt�N�Ϥ������`�M hover����Ū����Ϥ��}�C��
		*/
		void ShortLoad(SIN const TCHAR* name, HDC main_hdc, int width, int height, SOUT SImage* image_set) {
			static TCHAR LD_BUF[MAX_PATH];
			wsprintf(LD_BUF, TEXT("img\\%s.bmp"), name);
			image_set[0].Load(LD_BUF);
			wsprintf(LD_BUF, TEXT("img\\%s_hover.bmp"), name);
			image_set[1].Load(LD_BUF);

			int i;
			SRP(i, 2) ScaleFit(image_set[i], main_hdc, width, height);
		}
	}
	void Initialize(SIN HWND wnd) {
		HDC hdc = GetDC(wnd);

		img::ShortLoad(TEXT("back"), hdc, BACK_WIDTH, BACK_HEIGHT, back_img);

		ReleaseDC(wnd, hdc);

		ctl::ShortSetPushButton(
			back_img,
			BACK_PADDING + back_img[0].Width() / 2, BACK_PADDING + back_img[0].Height() / 2,
			ID_BACK,
			&back_button);
	}
}
