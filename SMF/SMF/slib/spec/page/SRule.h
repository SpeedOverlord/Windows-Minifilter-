/*
	Rule Page 的控制項
*/
#pragma once

#include "slib\spec\SGlobal.h"
#include "slib\spec\SGuiImport.h"

#include "slib\control\SSelectList.h"
#include "slib\control\SPushButton.h"
#include "slib\control\SStateButton.h"
#include "slib\control\SPathAsker.h"

#include "slib\control\item\SPathItem.h"

namespace gui {
	namespace rule {
		namespace {
			/*
				data
			*/
			//	const
			const int
				PATH_ITEM_HEIGHT = 30,
				PATH_LIST_PADDING = 2;

			const TCHAR *ASKER_TITLE[] = { TEXT("編輯保護路徑"), TEXT("編輯共用路徑"), TEXT("編輯沙盒路徑"), TEXT("編輯工作區路徑") };
			//	control
			SSelectList path_list[2];
			SStateButton<2> path_button[4];
			SPushButton add_button[2], delete_button[2];
			SPathAsker asker;
			//	config
			namespace config_cache {
				std::vector<SCharAry>
					protect_path,	//	保護路徑
					share_path,	//	共用路徑
					exe_path;		//	應用程式沙盒路徑

				std::map<SCharAry, std::vector<SCharAry>> workspace_path;	//	應用程式工作區路徑

				SCheckConfig check_config;
			}
			//	image
			SImage path_img[4][4], add_img[2], delete_img[2];
			/*
				function
			*/
			/*
				建立一個空的 select list
			*/
			void CreatePathList(SIN HWND parent, SOUT SSelectList* list, SIN DWORD style = SSL_DEFAULT_STYLE) {
				std::vector<SDrawableItem*> vec;
				list->Create(vec, PATH_ITEM_HEIGHT, 0, 0, 100, 100, parent, sgl::inst, style);
			}
			/*
				取得目前所選擇的路徑類別 => 檢查 state button 的狀態

				0 = protect path
				1 = share path
				2 = exe path
				3 = workspace
			*/
			int GetCurrentSelectType() {
				int select_index = -1;
				for (int i = 0; i < 4; ++i)
					if (path_button[i].state_ == 0) {
						select_index = i;
						break;
					}
				return select_index;
			}
			/*
				將 SCharAry 的 vector 設定到某個 select list 裡面
			*/
			void SetVectorList(SIN std::vector<SCharAry>& vec, SOUT SSelectList* list) {
				std::vector<SDrawableItem*> item;
				size_t i, cnt = vec.size();
				item.resize(cnt);
				SRP(i, cnt) {
					item[i] = new SPathItem;
					item[i]->str_ = vec[i];
				}
				list->ClearItem();
				list->Add(item);
			}
			/*
				將 workspace 設定裡面所有的 exe 路徑取出成一個 SCharAry 的 vector
			*/
			void GetWorkSpaceVector(SOUT std::vector<SCharAry>& vec) {
				std::map<SCharAry, std::vector<SCharAry>>::iterator it;
				size_t i = 0;
				vec.resize(config_cache::workspace_path.size());
				for (it = config_cache::workspace_path.begin(); it != config_cache::workspace_path.end(); ++it) {
					vec[i++] = it->first;
				}
			}
			/*
				更新左側的 path list
			*/
			void UpdatePathList0() {
				int select_index = GetCurrentSelectType();
				if (select_index == -1) return;

				path_list[0].ClearItem();
				if (path_list[1].Valid()) path_list[1].ClearItem();

				switch (select_index) {
				case 0:
					SetVectorList(config_cache::protect_path, path_list);
					break;
				case 1:
					SetVectorList(config_cache::share_path, path_list);
					break;
				case 2:
					SetVectorList(config_cache::exe_path, path_list);
					break;
				case 3: {
					std::vector<SCharAry> vec;
					GetWorkSpaceVector(vec);
					SetVectorList(vec, path_list);
					break;
				}
				default:
					//	never
					break;
				}
			}
			/*
				更新右側的 path list
			*/
			void UpdatePathList1() {
				//	檢查 workspace 狀態
				if (path_button[3].state_ == 1) return;

				std::vector<SDrawableItem*> list;
				path_list[0].GetSelectList(list);
				path_list[1].ClearItem();
				if (list.size() == 1) {
					SetVectorList(config_cache::workspace_path[list[0]->str_], path_list + 1);
				}
			}
			/*
				更新特定種類的設定檔
			*/
			void UpdateConfig(SIN ConfigType type) {
				sgl::config_mutex.lock();
				switch (type) {
				case PROTECT_CONFIG:
					sgl::config.SetProtectPath(config_cache::protect_path);
					break;
				case SHARE_CONFIG:
					sgl::config.SetSharePath(config_cache::share_path);
					break;
				case EXE_CONFIG:
					sgl::config.SetExePath(config_cache::exe_path);
					break;
				case WORKSPACE_CONFIG:
					sgl::config.SetWorkspace(config_cache::workspace_path);
					break;
				}
				sgl::config.SaveFile(type);
				sgl::config_mutex.unlock();
			}
			/*
				將 select list 所選取之項目的字串取出成 vector
			*/
			void GetSelectListVector(SIN SSelectList* list, SOUT std::vector<SCharAry>& vec) {
				std::vector<SDrawableItem*> item;
				size_t i, cnt;

				list->GetList(item);

				cnt = item.size();
				vec.resize(cnt);
				SRP(i, cnt) vec[i] = item[i]->str_;
			}
		}
		void Initialize(SIN HWND wnd) {
			HDC hdc = GetDC(wnd);

			img::ShortLoad(TEXT("protect_path_on"), hdc, OPER_WIDTH, OPER_HEIGHT, path_img[0]);
			img::ShortLoad(TEXT("protect_path_off"), hdc, OPER_WIDTH, OPER_HEIGHT, path_img[0] + 2);

			img::ShortLoad(TEXT("share_path_on"), hdc, OPER_WIDTH, OPER_HEIGHT, path_img[1]);
			img::ShortLoad(TEXT("share_path_off"), hdc, OPER_WIDTH, OPER_HEIGHT, path_img[1] + 2);

			img::ShortLoad(TEXT("exe_path_on"), hdc, OPER_WIDTH, OPER_HEIGHT, path_img[2]);
			img::ShortLoad(TEXT("exe_path_off"), hdc, OPER_WIDTH, OPER_HEIGHT, path_img[2] + 2);

			img::ShortLoad(TEXT("workspace_path_on"), hdc, OPER_WIDTH, OPER_HEIGHT, path_img[3]);
			img::ShortLoad(TEXT("workspace_path_off"), hdc, OPER_WIDTH, OPER_HEIGHT, path_img[3] + 2);

			img::ShortLoad(TEXT("add"), hdc, OPER_WIDTH, OPER_HEIGHT, add_img);
			img::ShortLoad(TEXT("delete"), hdc, OPER_WIDTH, OPER_HEIGHT, delete_img);

			ReleaseDC(wnd, hdc);

			RECT rect;
			GetClientRect(wnd, &rect);

			//	path
			ctl::ShortSetStateButton(
				path_img[0],
				BACK_PADDING + OPER_WIDTH / 2, BACK_PADDING * 2 + BACK_HEIGHT + OPER_HEIGHT / 2,
				ID_PROTECT_PATH,
				path_button
				);

			ctl::ShortSetStateButton(
				path_img[1],
				BACK_PADDING + OPER_WIDTH / 2 + OPER_WIDTH, BACK_PADDING * 2 + BACK_HEIGHT + OPER_HEIGHT / 2,
				ID_SHARE_PATH,
				path_button + 1
				);

			ctl::ShortSetStateButton(
				path_img[2],
				BACK_PADDING + OPER_WIDTH / 2 + OPER_WIDTH * 2, BACK_PADDING * 2 + BACK_HEIGHT + OPER_HEIGHT / 2,
				ID_EXE_PATH,
				path_button + 2
				);

			ctl::ShortSetStateButton(
				path_img[3],
				BACK_PADDING + OPER_WIDTH / 2 + OPER_WIDTH * 3, BACK_PADDING * 2 + BACK_HEIGHT + OPER_HEIGHT / 2,
				ID_WORKSPACE,
				path_button + 3
				);

			//	add / delete 0
			ctl::ShortSetPushButton(
				add_img,
				(rect.right - OPER_WIDTH - OPER_PADDING) / 2 - OPER_WIDTH - OPER_PADDING,
				rect.bottom - OPER_HEIGHT / 2 - OPER_PADDING,
				ID_ADD0,
				add_button);

			ctl::ShortSetPushButton(
				delete_img,
				(rect.right - OPER_WIDTH - OPER_PADDING) / 2,
				rect.bottom - OPER_HEIGHT / 2 - OPER_PADDING,
				ID_DELETE0,
				delete_button);

			//	add / delete 1

			ctl::ShortSetPushButton(
				add_img,
				(rect.right + OPER_WIDTH + OPER_PADDING) / 2,
				rect.bottom - OPER_HEIGHT / 2 - OPER_PADDING,
				ID_ADD1,
				add_button + 1);

			ctl::ShortSetPushButton(
				delete_img,
				(rect.right + OPER_WIDTH + OPER_PADDING) / 2 + OPER_WIDTH + OPER_PADDING,
				rect.bottom - OPER_HEIGHT / 2 - OPER_PADDING,
				ID_DELETE1,
				delete_button + 1);
		}
		void Create(SIN HWND wnd) {
			int i;
			back_button.Create(wnd, sgl::inst);

			SRP(i, 4) {
				path_button[i].Create(wnd, sgl::inst);
				path_button[i].state_ = 1;				//	off as default
			}
			add_button[0].Create(wnd, sgl::inst);
			delete_button[0].Create(wnd, sgl::inst);

			path_button[0].ButtonUp();

			//	cache config
			sgl::config_mutex.lock();

			sgl::config.GetCheckConfig(config_cache::check_config);
			sgl::config.GetProtectPath(config_cache::protect_path);
			sgl::config.GetSharePath(config_cache::share_path);
			sgl::config.GetExePath(config_cache::exe_path);
			sgl::config.GetWorkspace(config_cache::workspace_path);

			sgl::config_mutex.unlock();
		}
		void Destroy() {
			int i;
			back_button.Destroy();

			SRP(i, 4) path_button[i].Destroy();
			SRP(i, 2) {
				path_list[i].Destory();
				add_button[i].Destroy();
				delete_button[i].Destroy();
			}
		}
		namespace {
			void PathButton(SIN CTLID id) {
				int i, click_index = 0;
				switch (id) {
				case ID_PROTECT_PATH:
					break;
				case ID_SHARE_PATH:
					click_index = 1;
					break;
				case ID_EXE_PATH:
					click_index = 2;
					break;
				case ID_WORKSPACE:
					click_index = 3;
					break;
				}
				//	只可以選擇沒有被選擇的 path button
				if (path_button[click_index].state_ == 1) {
					path_button[click_index].state_ = 0;
					InvalidateRect(path_button[click_index].Handle(), NULL, TRUE);
					return;
				}
				//	取得被取消的按鈕，更新他
				SRP(i, 4) {
					if (i == click_index) continue;
					if (path_button[i].state_ == 0) {
						path_button[i].state_ = 1;
						InvalidateRect(path_button[i].Handle(), NULL, TRUE);
					}
				}
				//	調整操作按鈕
				RECT rect;
				int base_y = path_button[0].Y() + OPER_HEIGHT + OPER_PADDING;
				GetClientRect(path_button[0].Parent(), &rect);
				if (id == ID_WORKSPACE) {
					add_button[1].Create(add_button[0].Parent(), sgl::inst);
					delete_button[1].Create(delete_button[0].Parent(), sgl::inst);

					path_list[0].Destory();
					CreatePathList(path_button[0].Parent(), path_list, SSL_DEFAULT_STYLE_SINGLE);
					CreatePathList(path_button[0].Parent(), path_list + 1);

					MoveWindow(path_list[0].Handle(),
						path_button[0].X(), base_y,
						rect.right / 2 - path_button[0].X() - PATH_LIST_PADDING, add_button[0].Y() - OPER_HEIGHT - base_y,
						TRUE);
					MoveWindow(path_list[1].Handle(),
						rect.right / 2 + PATH_LIST_PADDING, base_y,
						rect.right / 2 - path_button[0].X() - PATH_LIST_PADDING, add_button[0].Y() - OPER_HEIGHT - base_y,
						FALSE);
				}
				else {
					add_button[1].Destroy();
					delete_button[1].Destroy();

					path_list[0].Destory();
					CreatePathList(path_button[0].Parent(), path_list, SSL_DEFAULT_STYLE);
					MoveWindow(path_list[0].Handle(),
						path_button[0].X(), base_y,
						rect.right - 2 * path_button[0].X(), add_button[0].Y() - OPER_HEIGHT - base_y,
						TRUE);
					path_list[1].Destory();
				}
				//	更新 list item
				UpdatePathList0();
			}
			void PathEditButton(SIN CTLID id) {
				int select_index = GetCurrentSelectType();
				if (select_index == -1) return;

				std::vector<SDrawableItem*> item;
				switch (id) {
				case ID_ADD0: {
					std::vector<SCharAry> mp_config, *config = &mp_config;
					switch (select_index) {
					case 0:
						config = &config_cache::protect_path;
						break;
					case 1:
						config = &config_cache::share_path;
						break;
					case 2:
						config = &config_cache::exe_path;
						break;
					case 3:
						GetWorkSpaceVector(mp_config);
						break;
					default:
						break;
					}
					asker.Create(ASKER_TITLE[select_index], path_list[0].Parent(), sgl::inst, *config);
					SetFocus(path_list[0].Parent());
					if (asker.update_) {
						wsprintf(SDEBUG, TEXT("YES %d"), (int)asker.result_.size());
						MSGB(SDEBUG);
						switch (select_index) {
						case 0:
							config_cache::protect_path = asker.result_;
							UpdateConfig(PROTECT_CONFIG);
							break;
						case 1:
							config_cache::share_path = asker.result_;
							UpdateConfig(SHARE_CONFIG);
							break;
						case 2:
							config_cache::exe_path = asker.result_;
							UpdateConfig(EXE_CONFIG);
							break;
						case 3: {
							size_t i, cnt = asker.result_.size();
							std::map<SCharAry, std::vector<SCharAry>> mp = config_cache::workspace_path;
							std::map<SCharAry, std::vector<SCharAry>>::iterator it;
							std::vector<SCharAry> empty;
							config_cache::workspace_path.clear();

							SRP(i, cnt) {
								it = mp.find(asker.result_[i]);
								if (it == mp.end()) {
									config_cache::workspace_path[asker.result_[i]] = empty;
								}
								else {
									config_cache::workspace_path[asker.result_[i]] = it->second;
								}
							}
							UpdateConfig(WORKSPACE_CONFIG);
							break;
						}
						default:
							//	never
							break;
						}
						UpdatePathList0();
						UpdatePathList1();
					}
					break;
				}
				case ID_ADD1:
					if (select_index != 3) break;
					path_list[0].GetSelectList(item);
					if (item.size() != 1) break;
					asker.Create(ASKER_TITLE[select_index], path_list[0].Parent(), sgl::inst, config_cache::workspace_path[item[0]->str_]);
					SetFocus(path_list[0].Parent());
					if (asker.update_) {
						config_cache::workspace_path[item[0]->str_] = asker.result_;
						UpdateConfig(WORKSPACE_CONFIG);
						UpdatePathList1();
					}
					break;
				case ID_DELETE0: {
					std::vector<SCharAry> result;

					path_list[0].DeleteSelect();

					GetSelectListVector(path_list, result);

					switch (select_index) {
					case 0:
						config_cache::protect_path = result;
						UpdateConfig(PROTECT_CONFIG);
						break;
					case 1:
						config_cache::share_path = result;
						UpdateConfig(SHARE_CONFIG);
						break;
					case 2:
						config_cache::exe_path = result;
						UpdateConfig(EXE_CONFIG);
						break;
					case 3: {
						size_t i, cnt = result.size();
						std::map<SCharAry, std::vector<SCharAry>> mp = config_cache::workspace_path;
						std::map<SCharAry, std::vector<SCharAry>>::iterator it;
						std::vector<SCharAry> empty;
						config_cache::workspace_path.clear();

						SRP(i, cnt) {
							it = mp.find(result[i]);
							if (it == mp.end()) {
								config_cache::workspace_path[result[i]] = empty;
							}
							else {
								config_cache::workspace_path[result[i]] = it->second;
							}
						}
						UpdateConfig(WORKSPACE_CONFIG);
						UpdatePathList1();
						break;
					}
					default:
						//	never
						break;
					}
					break;
				}
				case ID_DELETE1: {
					if (select_index != 3) break;
					path_list[0].GetSelectList(item);
					if (item.size() != 1) break;

					std::vector<SCharAry> result;

					path_list[1].DeleteSelect();

					GetSelectListVector(path_list + 1, result);

					config_cache::workspace_path[item[0]->str_] = result;

					UpdateConfig(WORKSPACE_CONFIG);
					UpdatePathList1();
					break;
				}
				default:
					//	never
					break;
				}
			}
		}
		void ButtonProc(SIN CTLID id) {
			switch (id) {
			case ID_ADD0:
			case ID_ADD1:
			case ID_DELETE0:
			case ID_DELETE1:
				PathEditButton(id);
				break;
			default:
				PathButton(id);
				break;
			}
		}
		inline void SelectChang(SIN SSelectList* list) {
			if (list == path_list) UpdatePathList1();
		}
	}
}
