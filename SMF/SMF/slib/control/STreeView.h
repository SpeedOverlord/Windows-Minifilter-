#pragma once

#include "SCustomWnd.h"

#include "slib\SCharAry.h"
#include "slib\SPathLib.h"

const TCHAR STV_WND_CLS[] = TEXT("STVWND");
const DWORD STV_DEFAULT_STYLE = WS_VISIBLE | WS_CHILD |
TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

class STreeView;

namespace {
	enum CheckState {
		UNDEFINE = 0,
		UNCHECK = 1,
		CHECK = 2,
		HALF = 3
	};
	enum CheckUpdateDir {
		UNCHECK_UP = 1,
		UNCHECK_DOWN,
		CHECK_UP,
		CHECK_DOWN
	};
	struct ItemNode {
		~ItemNode() { Release(); }
		void Release() {
			for (int i = 0; i < child.size(); ++i) delete child[i];
			child.clear();
		}
		bool Once;
		CheckState state;	//1 = empty, 2 = full, 3 = not full
		SCharAry AbsPath;
		std::vector<ItemNode*> child;
		TCHAR name[MAX_PATH];
	};
	struct TreeViewCreateParam {
		STreeView* view_;
		std::vector<SCharAry>* config_;
		HINSTANCE inst_;
		DWORD style_;
		int width_, height_;
	};
	struct ProcParam {
		STreeView* view_;
		HTREEITEM parent_item_;
		ItemNode* parent_node_;
	};
}

namespace sis {
	namespace win32 {
		LRESULT CALLBACK DefSTreeViewProc(SIN HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		namespace treeview {
			bool DriveProc(SINOUT void* data, SIN const TCHAR* drive);
			bool FileProc(SINOUT void* data, SIN const WIN32_FIND_DATA* find_data, const TCHAR* path);
		}
	}
}

class STreeView : public SCustomWnd {
public:
	friend LRESULT CALLBACK sis::win32::DefSTreeViewProc(SIN HWND, UINT, WPARAM, LPARAM);
	friend bool sis::win32::treeview::DriveProc(SINOUT void*, SIN const TCHAR*);
	friend bool sis::win32::treeview::FileProc(SINOUT void*, SIN const WIN32_FIND_DATA*, const TCHAR*);
	//	reg
	static bool reg_;
	static HIMAGELIST image_, state_image_;
	static ATOM RegClass(SIN HINSTANCE inst) {
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = sis::win32::DefSTreeViewProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = inst;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = STV_WND_CLS;
		wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		reg_ = true;
		InitTreeViewImageLists();
		return RegisterClassEx(&wcex);
	}
	/*
		初始化圖片
	*/
	static void InitTreeViewImageLists()
	{
		image_ = ImageList_Create(24, 24, ILC_COLOR32, 3, 0);

		HINSTANCE dll = LoadLibrary(TEXT("SHELL32.dll"));
		HICON icon;

		//	folder
		icon = LoadIcon(dll, MAKEINTRESOURCE(4));
		ImageList_AddIcon(image_, icon);

		//	file
		icon = LoadIcon(dll, MAKEINTRESOURCE(1));
		ImageList_AddIcon(image_, icon);

		//	drive
		icon = LoadIcon(dll, MAKEINTRESOURCE(8));
		ImageList_AddIcon(image_, icon);

		FreeLibrary(dll);

		state_image_ = ImageList_Create(13, 13, ILC_COLOR32, 4, 0);

		HBITMAP bmp;

		bmp = (HBITMAP)LoadImage(NULL, TEXT("img\\uncheck.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		ImageList_Add(state_image_, bmp, (HBITMAP)NULL);
		ImageList_Add(state_image_, bmp, (HBITMAP)NULL);
		DeleteObject(bmp);

		bmp = (HBITMAP)LoadImage(NULL, TEXT("img\\check.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		ImageList_Add(state_image_, bmp, (HBITMAP)NULL);
		DeleteObject(bmp);

		bmp = (HBITMAP)LoadImage(NULL, TEXT("img\\inter.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		ImageList_Add(state_image_, bmp, (HBITMAP)NULL);
		DeleteObject(bmp);
	}
	/*
		在 parent 視窗的指定位置建立一個新的 TreeView
		preconfig 表示已經打勾的路徑
			* 資料夾路徑尾端保證沒有 back slash
			* 且所有路徑皆存在於磁碟中
	*/
	HWND Create(SIN int x, int y, int width, int height, HWND parent, HINSTANCE inst, std::vector<SCharAry>& preconfig,
				SIN DWORD style = STV_DEFAULT_STYLE) {
		if (!reg_) RegClass(inst);
		if (wnd_ != NULL) return wnd_;

		TreeViewCreateParam param;
		param.view_ = this;
		param.width_ = width; param.height_ = height;
		param.inst_ = inst;
		param.style_ = style;
		param.config_ = &preconfig;

		parent_ = parent;
		return wnd_ = CreateWindowEx(WS_EX_CLIENTEDGE, STV_WND_CLS, TEXT(""), WS_CHILD | WS_VISIBLE/*WS_OVERLAPPEDWINDOW*/,
			x, y, width, height,parent, NULL, inst, &param);
	}

	/*
		取得目前 TreeView 所有打勾的路徑，需要最精簡
			* 最精簡即表示若路徑 abc 和 abc/123 皆有打勾，list 只含 abc 而不含 abc/123 (因為 abc 已經包含 abc/123)
		
		取得之指標由 class 管理，只在 Destroy 前有效
	*/
	inline void GetList(SOUT std::vector<TCHAR*>& list) { CollectPath(list, &root_); }
protected:
	/*
		產生 tree view
	*/
	void CreateTreeView(SIN HWND parent, TreeViewCreateParam* param) {
		tree_ = CreateWindowEx(0, WC_TREEVIEW, TEXT(""), param->style_, 0, 0, param->width_, param->height_,
			parent, NULL, param->inst_, NULL);

		/*
			checkbox style
		*/
		SetWindowLongPtr(tree_, GWL_STYLE, param->style_ | TVS_CHECKBOXES);

		//InitTreeViewImageLists();
		TreeView_SetImageList(tree_, image_, TVSIL_NORMAL);
		TreeView_SetImageList(tree_, state_image_, TVSIL_STATE);

		/*
			設定樹節點
		*/
		sis::str::Copy(TEXT("root"), root_.name);
		root_.Once = true;
		root_.state = HALF;
		root_.AbsPath = root_.name;

		size_t config_cnt = param->config_->size();
		for (size_t i = 0; i < config_cnt; ++i) {
			AddPrePath(param->config_->at(i).Ptr());
		}

		InitTreeViewItems();
	}
	/*
	*/
	void CollectPath(std::vector<TCHAR*>& result, ItemNode* term) {
		//travel nodes and output
		for (int i = 0; i < term->child.size(); ++i) {
			if (term->child[i]->state == 1)continue;
			else if (term->child[i]->state == 2) {
				result.push_back(term->child[i]->AbsPath.Ptr());
				//MessageBox(NULL, term->child[i]->name, TEXT("RESULT"), MB_OK);
			}
			else if (term->child[i]->state == 3)
				CollectPath(result, term->child[i]);
		}
	}
	/*
		創建特定路徑的 item 時是否需要幫它加上 "+" 於左側
	*/
	inline bool NeedPlusIcon(const TCHAR* path) {
		return sis::path::GetSubItemCount(path, 3) == 3;
	}
	/*
		增加一個 preconfig 路徑
	*/
	void AddPrePath(const TCHAR* config) {
		if (!sis::path::FileExist(config)) return;

		std::vector<ItemNode*> stack;
		SCharAry config_copy(config);
		TCHAR *ptr = NULL, *tokbuf = NULL, full_path[MAX_PATH] = { 0 };
		ItemNode *node = NULL;
		size_t full_path_length = 0;
		bool exist;

		stack.push_back(&root_);
		ptr = _tcstok_s(config_copy.Ptr(), TEXT("\\"), &tokbuf);
		size_t ptr_length;
		while(ptr) {
			exist = false;
			ptr_length = sis::str::Length(ptr) + 1;
			node = stack.back();
			for (std::vector<ItemNode*>::iterator it = node->child.begin(); it != node->child.end(); ++it) {
				if (sis::str::Compare((*it)->name, ptr)==sis::str::EQUAL) {
					exist = true;
					stack.push_back(*it);
					if (full_path_length != 0) full_path[full_path_length++] = TEXT('\\');
					sis::str::Copy(ptr, ptr_length, full_path + full_path_length);	full_path_length += ptr_length - 1;
					break;
				}
			}
			if (!exist) {
				node = new ItemNode;
				sis::str::Copy(ptr, ptr_length, node->name);
				if (full_path_length != 0) full_path[full_path_length++] = TEXT('\\');
				sis::str::Copy(ptr, ptr_length, full_path + full_path_length);	full_path_length += ptr_length - 1;
				node->AbsPath = full_path;
				node->Once = false;
				node->state = UNDEFINE;

				stack.back()->child.push_back(node);
				stack.push_back(node);
			}
			ptr = _tcstok_s(NULL, TEXT("\\"), &tokbuf);
		}
		size_t cnt[2] = { 0, 0 };
		stack.back()->state = CHECK;
		stack.pop_back();

		while (stack.size() > 1) {
			node = stack.back();
			if (node->state != UNDEFINE) break;
			for (std::vector<ItemNode*>::iterator it = node->child.begin(); it != node->child.end(); ++it) {
				switch ((*it)->state) {
				case CHECK:
					++cnt[0];
					break;
				case HALF:
					++cnt[1];
					break;
				}
				if (cnt[1] != 0) break;
			}
			if (cnt[1] != 0) {
				node->state = HALF;
			}
			else {
				size_t child_cnt = node->child.size();
				if (sis::path::GetSubItemCount(node->AbsPath.Ptr(), child_cnt + 2 + 1) > child_cnt + 2) node->state = HALF;
				else node->state = CHECK;
			}
			stack.pop_back();
			if (node->state == HALF) {
				while (stack.size() > 1) {
					stack.back()->state = HALF;
					stack.pop_back();
				}
			}
		}
	}
	void UpdateState(HTREEITEM item) {
		if (item == NULL) return;
		CheckState org_state = GetItemCheckState(item), new_state;
		HTREEITEM child = TreeView_GetChild(tree_, item);
		size_t cnt[3] = { 0, 0, 0 };
		while (child) {
			switch (GetItemCheckState(child)) {
			case UNCHECK:
				++cnt[0];
				if (cnt[1] != 0 || cnt[2] != 0) break;
				break;
			case CHECK:
				++cnt[1];
				if (cnt[0] != 0 || cnt[2] != 0) break;
				break;
			case HALF:
				++cnt[2];
				if (cnt[0] != 0 || cnt[1] != 0) break;
				break;
			}
			child = TreeView_GetNextSibling(tree_, child);
		}
		if (cnt[2] == 0) {
			if (cnt[0] == 0) new_state = CHECK;
			else if (cnt[1] == 0) new_state = UNCHECK;
			else new_state = HALF;
		}
		else new_state = HALF;

		if (org_state != new_state) {
			SetItemCheckState(item, new_state);
			UpdateState(TreeView_GetParent(tree_, item));
		}
	}
	void UpdateStateDown(CheckState state, HTREEITEM root) {
		HTREEITEM item = TreeView_GetChild(tree_, root);
		while (item) {
			if (GetItemCheckState(item) != state)
			{
				SetItemCheckState(item, state);
				UpdateStateDown(state, item);
			}
			item = TreeView_GetNextSibling(tree_, item);
		}
	}
	/*
		取得 state image index
	*/
	CheckState GetItemCheckState(HTREEITEM item) {

		if (item != NULL) {
			TVITEM tvi;
			tvi.mask = TVIF_HANDLE | TVIF_STATE;
			tvi.hItem = item;
			tvi.stateMask = TVIS_STATEIMAGEMASK;
			TreeView_GetItem(tree_, &tvi);
			return (CheckState)(tvi.state >> 12);
		}
		else return UNDEFINE;
	}
	/*
		設定 state image
	*/
	void SetItemCheckState(HTREEITEM item, CheckState state) {
		if (item != NULL) {
			TVITEM tvi;
			tvi.mask = TVIF_PARAM;
			tvi.hItem = item;
			TreeView_GetItem(tree_, &tvi);
			ItemNode* set = (ItemNode*)tvi.lParam;
			set->state = state;

			//set the checkbox
			tvi.mask = TVIF_HANDLE | TVIF_STATE;
			tvi.hItem = item;
			tvi.stateMask = TVIS_STATEIMAGEMASK;
			tvi.state = INDEXTOSTATEIMAGEMASK(set->state);
			TreeView_SetItem(tree_, &tvi);
		}
	}
	/*
		展開 parent 時，新增 child 的 child
	*/
	void ExpandItem(HTREEITEM parent) {

		HTREEITEM child = TreeView_GetChild(tree_, parent);

		if (child != NULL) {
			TVITEM tvi;
			tvi.mask = TVIF_PARAM;
			tvi.hItem = child;
			TreeView_GetItem(tree_, &tvi);
			ItemNode* temp = (ItemNode*)tvi.lParam;
			delete temp;
			TreeView_DeleteItem(tree_, child);

			tvi.hItem = parent;
			TreeView_GetItem(tree_, &tvi);

			ProcParam param;
			param.view_ = this;
			param.parent_item_ = parent;
			param.parent_node_ = (ItemNode*)tvi.lParam;
			sis::path::EnumFile(&param, sis::win32::treeview::FileProc, NULL, param.parent_node_->AbsPath.Ptr(), 1);
			return;
		}
	}
	/*
		取得 item 路徑
	*/
	TCHAR* FindPath(HTREEITEM hitem) {
		TVITEM tvi;
		tvi.mask = TVIF_PARAM;
		tvi.hItem = hitem;
		if (TreeView_GetItem(tree_, &tvi)) return ((ItemNode*)tvi.lParam)->AbsPath.Ptr();
		return NULL;
	}
	/*
		用於產生 + 的子資料夾
	*/
	HTREEITEM AddPlusItem(HTREEITEM parent) {
		TVINSERTSTRUCT tvins;
		ItemNode* temp_node = new ItemNode;
		temp_node->state = UNCHECK;

		tvins.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE | TVIF_IMAGE;
		tvins.item.pszText = TEXT("temp");
		tvins.item.iImage = 1;
		tvins.item.lParam = (LPARAM)temp_node;
		tvins.hInsertAfter = TVI_LAST;
		tvins.hParent = parent;
		return TreeView_InsertItem(tree_, &tvins);
	}
	/*
		新增 item 到 treeview
	*/
	HTREEITEM AddItem(const TCHAR* name ,const TCHAR* full_path, HTREEITEM parent_item, ItemNode* parent_node, int image) {
		TVINSERTSTRUCT tvins;
		SCharAry name_copy(name);
		bool exist = false;

		tvins.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
		tvins.item.pszText = name_copy.Ptr();
		tvins.item.iImage = tvins.item.iSelectedImage = image;
		tvins.item.stateMask = TVIS_STATEIMAGEMASK;

		for (int i = 0; i < parent_node->child.size(); ++i) {
			if (sis::str::Compare(parent_node->child[i]->name, name) == sis::str::EQUAL) {
				exist = true;

				tvins.item.state = INDEXTOSTATEIMAGEMASK(parent_node->child[i]->state);
				tvins.item.lParam = (LPARAM)parent_node->child[i];
				break;
			}
		}
		if (!exist) {
			ItemNode *new_node = new ItemNode;
			sis::str::Copy(name, new_node->name);
			new_node->AbsPath = full_path;
			new_node->Once = false;
			new_node->state = parent_node->state == CHECK ? CHECK : UNCHECK;
			parent_node->child.push_back(new_node);

			tvins.item.state = INDEXTOSTATEIMAGEMASK(new_node->state);
			tvins.item.lParam = (LPARAM)new_node;
		}

		tvins.hInsertAfter = TVI_LAST;
		tvins.hParent = parent_item;
		return TreeView_InsertItem(tree_, &tvins);
	}
	/*
		初始化 item
	*/
	void InitTreeViewItems()
	{
		ProcParam param;
		param.view_ = this;
		param.parent_item_ = TVI_ROOT;
		param.parent_node_ = &root_;

		sis::path::EnumLogicalDrive(&param, sis::win32::treeview::DriveProc);
	}

	using SCustomWnd::Create;

	ItemNode root_;
	HWND tree_;
};

bool STreeView::reg_ = false;
HIMAGELIST STreeView::image_, STreeView::state_image_;

namespace sis {
	namespace win32 {
		LRESULT CALLBACK DefSTreeViewProc(SIN HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
			//	wnd data
			STreeView *data = (STreeView*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

			switch (message) {
			case WM_CREATE: {
				CREATESTRUCT *create_strcut = (CREATESTRUCT*)lParam;
				TreeViewCreateParam *create_param = (TreeViewCreateParam*)create_strcut->lpCreateParams;

				data = create_param->view_;
				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data);
				data->CreateTreeView(hWnd, (TreeViewCreateParam*)create_strcut->lpCreateParams);
				break;
			}
			case WM_NOTIFY: {
				//notify
				LPNMHDR lpnmh = (LPNMHDR)lParam;
				LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
				if (lpnmh->code == TVN_ITEMEXPANDED) {
					ItemNode* getnode = (ItemNode*)(pnmtv->itemNew.lParam);
					if (!getnode->Once) {
						getnode->Once = true;
						data->ExpandItem(pnmtv->itemNew.hItem);
					}
				}
				else if (lpnmh->code == NM_CLICK) {
					TVHITTESTINFO ht = { 0 };
					DWORD dwpos = GetMessagePos();
					ht.pt.x = GET_X_LPARAM(dwpos);
					ht.pt.y = GET_Y_LPARAM(dwpos);
					MapWindowPoints(HWND_DESKTOP, lpnmh->hwndFrom, &ht.pt, 1);
					TreeView_HitTest(lpnmh->hwndFrom, &ht);
					if (ht.flags & TVHT_ONITEMSTATEICON) PostMessage(hWnd, SWM_CHECKSTATECHANGE, 0, (LPARAM)ht.hItem);
				}
				break;
			}
			case SWM_CHECKSTATECHANGE: {
				HTREEITEM item = (HTREEITEM)lParam;
				switch (data->GetItemCheckState(item)) {
				case HALF:
					data->SetItemCheckState(item, UNCHECK);
					data->UpdateStateDown(UNCHECK, item);
					data->UpdateState(TreeView_GetParent(data->tree_, item));
					break;
				case CHECK:
				case UNCHECK:
					data->SetItemCheckState(item, CHECK);
					data->UpdateStateDown(CHECK, item);
					data->UpdateState(TreeView_GetParent(data->tree_, item));
					break;
				}
				break;
			}
			case WM_COMMAND: {
				break;
			}
			case WM_SIZE:
				MoveWindow(data->tree_, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
				break;
			case WM_PAINT: {
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);

				EndPaint(hWnd, &ps);
				break;
			}
			case WM_DESTROY:
				TreeView_DeleteAllItems(data->tree_);
				DestroyWindow(data->tree_);
				data->root_.Release();

				PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			return 0;
		}
		namespace treeview {
			bool DriveProc(SINOUT void* data, SIN const TCHAR* drive) {
				ProcParam* param = (ProcParam*)data;
				HTREEITEM item = param->view_->AddItem(drive, drive, param->parent_item_, param->parent_node_, 2);
				if (param->view_->NeedPlusIcon(drive)) param->view_->AddPlusItem(item);
				return true;
			}
			bool FileProc(SINOUT void* data, SIN const WIN32_FIND_DATA* find_data, const TCHAR* path) {
				ProcParam* param = (ProcParam*)data;
				HTREEITEM item = param->view_->AddItem(find_data->cFileName, path, param->parent_item_, param->parent_node_, ((find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) ? 1 : 0);
				if (param->view_->NeedPlusIcon(path)) param->view_->AddPlusItem(item);
				return true;
			}
		}
	}
}
