/*
	Query Page ªº±±¨î¶µ
*/
#pragma once

#include "slib\spec\SUserMsg.h"
#include "slib\spec\SQueryItem.h"

#include "slib\spec\SGlobal.h"
#include "slib\spec\SGuiImport.h"

#include "slib\control\SSelectList.h"
#include "slib\control\SPushButton.h"

namespace gui {
	namespace query {
		namespace {
			/*
				data
			*/
			//	const
			const int QUERY_ITEM_HEIGHT = 60;

			//	control
			SSelectList query_list;
			SPushButton accept_button, reject_button, open_file_button, open_exe_button;
			//	image
			SImage accept_img[2], reject_img[2], open_file_img[2], open_exe_img[2];
			/*
				function
			*/
			void GetIorqList(SOUT std::vector<SDrawableItem*>& vec) {
				SQueryItem* item = NULL;

				for (size_t i = 0; i < sgl::io_query.size(); ++i) {
					item = new SQueryItem;
					item->iorq_ = (SIORQ*)(sgl::io_query[i].ptr_[0]);
					item->str_.Allocate(20);
					item->GetAccessString(item->str_.Ptr());
					vec.push_back(item);
				}
			}
		}
		void Initialize(SIN HWND wnd) {
			HDC hdc = GetDC(wnd);

			img::ShortLoad(TEXT("accept"), hdc, OPER_WIDTH, OPER_HEIGHT, accept_img);
			img::ShortLoad(TEXT("reject"), hdc, OPER_WIDTH, OPER_HEIGHT, reject_img);
			img::ShortLoad(TEXT("openfile"), hdc, OPER_WIDTH, OPER_HEIGHT, open_file_img);
			img::ShortLoad(TEXT("openexe"), hdc, OPER_WIDTH, OPER_HEIGHT, open_exe_img);

			ReleaseDC(wnd, hdc);

			RECT rect;
			GetClientRect(wnd, &rect);

			//	accept
			ctl::ShortSetPushButton(
				accept_img,
				(rect.right - OPER_WIDTH - OPER_PADDING) / 2 - OPER_WIDTH - OPER_PADDING,
				rect.bottom - OPER_HEIGHT / 2 - OPER_PADDING,
				ID_ACCEPT,
				&accept_button);

			//	reject
			ctl::ShortSetPushButton(
				reject_img,
				(rect.right - OPER_WIDTH - OPER_PADDING) / 2,
				rect.bottom - OPER_HEIGHT / 2 - OPER_PADDING,
				ID_REJECT,
				&reject_button);

			//	open_file
			ctl::ShortSetPushButton(
				open_file_img,
				(rect.right + OPER_WIDTH + OPER_PADDING) / 2,
				rect.bottom - OPER_HEIGHT / 2 - OPER_PADDING,
				ID_OPENFILE,
				&open_file_button);

			//	open_exe
			ctl::ShortSetPushButton(
				open_exe_img,
				(rect.right + OPER_WIDTH + OPER_PADDING) / 2 + OPER_WIDTH + OPER_PADDING,
				rect.bottom - OPER_HEIGHT / 2 - OPER_PADDING,
				ID_OPENEXE,
				&open_exe_button);
		}
		void Create(SIN HWND wnd) {
			//	button
			back_button.Create(wnd, sgl::inst);

			accept_button.Create(wnd, sgl::inst);
			reject_button.Create(wnd, sgl::inst);
			open_file_button.Create(wnd, sgl::inst);
			open_exe_button.Create(wnd, sgl::inst);

			//	query list
			std::vector<SDrawableItem*> vec;
			RECT rect;
			GetClientRect(wnd, &rect);

			sgl::io_query_mutex.lock();

			GetIorqList(vec);
			sgl::io_query.clear();

			query_list.Create(vec, QUERY_ITEM_HEIGHT,
				BACK_PADDING, BACK_PADDING + BACK_HEIGHT + BACK_PADDING,
				rect.right - BACK_PADDING * 2, rect.bottom - BACK_PADDING * 2 - OPER_PADDING * 2 - BACK_HEIGHT - OPER_HEIGHT,
				wnd, sgl::inst);

			sgl::io_query_mutex.unlock();
		}
		void Destroy() {
			//	button
			back_button.Destroy();

			accept_button.Destroy();
			reject_button.Destroy();
			open_file_button.Destroy();
			open_exe_button.Destroy();

			//	query list
			std::vector<SDrawableItem*> vec;
			SMSG msg;

			sgl::io_query_mutex.lock();

			query_list.GetList(vec);

			msg.type_ = umsg::IO_REQUERY;
			msg.param_[0] = 0;
			for (size_t i = 0; i < vec.size(); ++i) {
				msg.ptr_[0] = ((SQueryItem*)vec[i])->iorq_;
				sgl::io_query.push_back(msg);
			}

			query_list.Destory();

			sgl::io_query_mutex.unlock();
		}
		void UpdateList() {
			std::vector<SDrawableItem*> vec;

			sgl::io_query_mutex.lock();

			GetIorqList(vec);
			sgl::io_query.clear();

			query_list.Add(vec);

			sgl::io_query_mutex.unlock();
		}
		void ButtonProc(SIN CTLID id) {
			std::vector<SDrawableItem*> list;
			query_list.GetSelectList(list);

			SMSG msg;
			size_t i;
			switch (id) {
			case ID_ACCEPT:
			case ID_REJECT:
				msg.type_ = umsg::IO_REPLY;
				msg.param_[0] = id == ID_ACCEPT ? umsg::IO_ACCEPT : umsg::IO_REJECT;
				SRP(i, list.size()) {
					msg.ptr_[0] = ((SQueryItem*)list[i])->iorq_;
					sgl::master_queue.PushMsg(msg);
				}
				query_list.DeleteSelect();
				break;
			case ID_OPENFILE:
				SRP(i, list.size()) {
					sis::path::OpenExplorer(((SQueryItem*)list[i])->iorq_->file_.Ptr());
				}
				break;
			case ID_OPENEXE:
				SRP(i, list.size()) {
					sis::path::OpenExplorer(((SQueryItem*)list[i])->iorq_->exe_.Ptr());
				}
				break;
			default:
				break;
			}
		}
	}
}
