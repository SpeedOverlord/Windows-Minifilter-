/*
	Copyright © 2016 by Phillip Chang

	SSSelectList

	按鍵接口
*/
#pragma once

#include "SCustomWnd.h"
#include "item\SDrawableItem.h"

#include "slib\SCharAry.h"

const TCHAR SSSEL_WND_CLS[] = TEXT("SSSELWND");

namespace sis {
	namespace win32 {
		LRESULT CALLBACK DefSSSelectListProc(SIN HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	}
}

const DWORD SSL_DEFAULT_STYLE = WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_MULTIPLESEL | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT;

class SSSelectList;

/*
	建立 control 視窗的參數
*/
struct SelectListCreateParem {
	SSSelectList* list_;
	HINSTANCE inst_;
	DWORD style_;
	int width_, height_;
};

class SSSelectList : public SCustomWnd {
public:
	friend LRESULT CALLBACK sis::win32::DefSSSelectListProc(SIN HWND, UINT, WPARAM, LPARAM);
	//	reg
	static bool reg_;
	static ATOM RegClass(SIN HINSTANCE inst) {
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = sis::win32::DefSSSelectListProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = inst;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = SSSEL_WND_CLS;
		wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		reg_ = true;
		return RegisterClassEx(&wcex);
	}

	~SSSelectList() {
		//	解構時視窗應該都要已經刪除(不再存取 item_)
		ReleaseItem();
	}
	/*
		initialize

		data 是資料指標陣列，裡面的資料都是 new 出來的，將向量傳入 Create 之後，只要沒有失敗，就不應該繼續使用那些資料
		指標釋放由此視窗進行
	*/
	HWND Create(SIN std::vector<SDrawableItem*>& data, int item_height, int x, int y, int width, int height, HWND parent, HINSTANCE inst,
				SIN DWORD style = SSL_DEFAULT_STYLE) {
		if (!reg_) RegClass(inst);
		if (Valid()) return wnd_;
		
		SelectListCreateParem param;
		param.list_ = this;
		param.width_ = width;	param.height_ = height;
		param.inst_ = inst;
		param.style_ = style;

		ReleaseItem();
		item_ = data;
		
		item_height_ = item_height;

		parent_ = parent;
		return wnd_ = CreateWindow(SSSEL_WND_CLS, TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, x, y, width, height, parent, NULL, inst, &param);
	}
	void Destory() {
		if (Valid()) {
			DestroyWindow(list_box_);
			DestroyWindow(wnd_);

			list_box_ = wnd_ = NULL;
		}
		ReleaseItem();
	}
	/*
		List Box
	*/
	void DeleteSelect() {
		std::vector<SDrawableItem*> item;
		SDrawableItem* cur = NULL;
		int index = 0;
		while (index < ListBox_GetCount(list_box_)) {
			cur = (SDrawableItem*)ListBox_GetItemData(list_box_, index);
			if (ListBox_GetSel(list_box_, index) > 0) {
				delete cur;
				ListBox_DeleteString(list_box_, index);
			}
			else {
				item.push_back(cur);
				++index;
			}
		}
		item_.clear();
		item_ = item;
	}
	inline void ClearItem() {
		ListBox_ResetContent(list_box_);
		ReleaseItem();
	}
	/*
		執行完後不應該繼續使用 data 的資料

		資料不應該跟原有資料重複
	*/
	void Add(SIN std::vector<SDrawableItem*>& data) {
		size_t i, length = data.size();
		int index;
		SRP(i, length) {
			index = ListBox_AddString(list_box_, data[i]->str_.Ptr());
			ListBox_SetItemData(list_box_, index, data[i]);
		}

		item_.insert(item_.end(), data.begin(), data.end());
	}	
	/*
		取得所有資料

		應該在 list 資料被改變前使用完 data
	*/
	void GetList(SOUT std::vector<SDrawableItem*>& data) {
		data.clear();
		int i, cnt = ListBox_GetCount(list_box_);
		data.resize(cnt);
		SRP(i, cnt) {
			data[i] = (SDrawableItem*)ListBox_GetItemData(list_box_, i);
		}
	}
	/*
		取得所有選取資料

		應該在 list 資料被改變前使用完 data
	*/
	void GetSelectList(SOUT std::vector<SDrawableItem*>& data) {
		data.clear();
		int i = 0, cnt = ListBox_GetSelCount(list_box_), index = 0;
		data.resize(cnt);
		while (i < cnt) {
			if (ListBox_GetSel(list_box_, index) > 0) {
				data[i] = (SDrawableItem*)ListBox_GetItemData(list_box_, index);
				++i;
			}
			++index;
		}
	}
protected:
	/*
		釋放 item 指標
	*/
	void ReleaseItem() {
		size_t i, length = item_.size();
		SRP(i, length) delete item_[i];
		item_.clear();
	}
	/*
		List Box
	*/
	void CreateListBox(SIN HWND parent, SelectListCreateParem& param) {
		//	這裡的 parent 就是 SSSEL_WND_CLS 視窗(SSCustomWnd::wnd_)，但是這個函式會在 WM_CREATE 裡面執行，因此 wnd_ 的值還沒更新
		list_box_ = CreateWindowEx(0, WC_LISTBOX, TEXT(""), SSL_DEFAULT_STYLE,
			0, 0, param.width_, param.height_, parent, NULL, param.inst_, NULL);

		//wsprintf(SDEBUG, TEXT("Fail %d"), GetLastError());
		//if (list_box_ == NULL) MessageBox(parent, SDEBUG, TEXT("CreateListBox"), MB_OK);
		size_t i, length = item_.size();
		int index;
		SRP(i, length) {
			//wsprintf(SDEBUG, TEXT("Add %s"), item_[i]->str_.Ptr());
			//MessageBox(parent, SDEBUG, TEXT("Note"), MB_OK);
			index = ListBox_AddString(list_box_, item_[i]->str_.Ptr());
				//(int)SendMessage(list_box_, LB_ADDSTRING, 0, (LPARAM)(item_[i]->str_.data_));
			ListBox_SetItemData(list_box_, index, item_[i]);
			//SendMessage(list_box_, LB_SETITEMDATA, index, (LPARAM)(item_[i]));
		}

		InvalidateRect(wnd_, NULL, TRUE);
	}

	//	data
	SCustomWnd::Create;
	SCustomWnd::Destroy;

	HWND list_box_;
	std::vector<SDrawableItem*> item_;
	int item_height_;
};

bool SSSelectList::reg_ = false;

namespace sis {
	namespace win32 {
		LRESULT CALLBACK DefSSSelectListProc(SIN HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
			//	draw
			PAINTSTRUCT ps;
			HDC hdc;
			//	wnd data
			SSSelectList *data = (SSSelectList*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			//	create param
			CREATESTRUCT* create_strcut = NULL;
			SelectListCreateParem* create_param = NULL;
			//	item
			PMEASUREITEMSTRUCT measure;
			PDRAWITEMSTRUCT draw_item;
			SDrawableItem* item = NULL;

			switch (message) {
			case WM_CREATE:
				create_strcut = (CREATESTRUCT*)lParam;
				create_param = (SelectListCreateParem*)create_strcut->lpCreateParams;
				data = create_param->list_;
				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data);	//	先設置，避免 CreateListBox 的 MEASUREITEM 重入時 data = NULL
				data->CreateListBox(hWnd, *create_param);
				//MessageBox(hWnd, TEXT("create"), TEXT("note"), MB_OK);
				break;
			case WM_MEASUREITEM:
				//wsprintf(SDEBUG, TEXT("Measure %d"), data->item_height_);
				//MessageBox(hWnd, SDEBUG, TEXT("Note"), MB_OK);

				measure = (PMEASUREITEMSTRUCT)lParam;
				measure->itemHeight = data->item_height_;
				return TRUE;
			case WM_DRAWITEM:
				draw_item = (PDRAWITEMSTRUCT)lParam;

				//wsprintf(SDEBUG, TEXT("Draw %d"), draw_item->itemID);
				//MessageBox(hWnd, SDEBUG, TEXT("Note"), MB_OK);
				//	不是空的 list
				if (draw_item->itemID != -1) {
					switch (draw_item->itemAction) {
					case ODA_DRAWENTIRE:
					case ODA_SELECT:
						item = (SDrawableItem*)ListBox_GetItemData(data->list_box_, draw_item->itemID);
						item->DrawItem(draw_item->hDC, draw_item->rcItem, ((draw_item->itemState & ODS_SELECTED) > 0) ? true : false);
						
						break;
					default:
						break;
					}
				}
				return TRUE;
			case WM_PAINT:
				hdc = BeginPaint(hWnd, &ps);
				// TODO:  在此加入任何繪圖程式碼...
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
