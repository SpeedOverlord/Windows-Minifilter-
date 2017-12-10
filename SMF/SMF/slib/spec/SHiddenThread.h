/*
	�t�d
	1.	���U�k�U���u��C���p�ϥ�
	2.	�����a�B����
	3.	���� GUI ����
*/
#pragma once

#include "SGlobal.h"
#include "SUserMsg.h"
#include "SMFlt.h"
#include "SGuiThread.h"
#include "SEZLog.h"

#include <TlHelp32.h>

namespace hgl {
	//	notify setting
	const UINT NOTIFY_ID = 100, SWM_NOTIFY = WM_USER + 100;
	const TCHAR NOTIFY_TIP[] = TEXT("SISTEM");

	HWND msg_wnd;

	void CreateHiddenWnd();
	void CreateNotifyIcon();
	void UpdateNotifyBalloon(SIN const TCHAR* title, const TCHAR* info);
	void ProcMsg(SIN SMSG& msg);
	void UpdateCache();
	LRESULT CALLBACK NotifyProc(SIN HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
}

unsigned int __stdcall HiddenProc(SIN void* data) {
	/*
		�Ы����æ���
	*/
	hgl::CreateHiddenWnd();

	/*
		���U�p�ϥ�
	*/
	hgl::CreateNotifyIcon();

	/*
		�T���B�z
	*/
	MSG msg;
	SMSG smsg;
	int cache_update_cnt = 0;

	msg.message = 0;
	while (!sgl::fatal_error) {
		/*
			window msg
		*/
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			DispatchMessage(&msg);
		}

		/*
			queue msg
		*/
		while (sgl::hidden_queue.PeekMsg(sgl::TIMEOUT, smsg)) {
			hgl::ProcMsg(smsg);
		}

		Sleep(sgl::IDLE_TIME);

		/*
			���� cache �귽
		*/
		++cache_update_cnt;
		if (cache_update_cnt == 2000) {
			hgl::UpdateCache();
			cache_update_cnt = 0;

			EnumAttachDrive();
		}
	}

	return 0;
}

namespace hgl {
	void CreateHiddenWnd() {
		static const TCHAR CLS_NAME[] = TEXT("HGL_HIDDEN_WND");

		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = NotifyProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = sgl::inst;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = CLS_NAME;
		wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		RegisterClassEx(&wcex);

		msg_wnd = CreateWindowEx(NULL, CLS_NAME, TEXT(""), WS_CHILD, 0, 0, 10, 10, HWND_MESSAGE, NULL, sgl::inst, NULL);
	}
	void CreateNotifyIcon() {
		NOTIFYICONDATA ndata;

		ndata.cbSize = sizeof(NOTIFYICONDATA);
		ndata.hWnd = hgl::msg_wnd;
		ndata.uID = NOTIFY_ID;
		ndata.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP;
		ndata.uCallbackMessage = SWM_NOTIFY;
		LoadIconMetric(NULL, MAKEINTRESOURCE(IDI_APPLICATION), LIM_LARGE, &ndata.hIcon);
		sis::str::Copy(NOTIFY_TIP, ndata.szTip);
		ndata.uVersion = NOTIFYICON_VERSION_4;

		Shell_NotifyIcon(NIM_ADD, &ndata);

		Shell_NotifyIcon(NIM_SETVERSION, &ndata);

		UpdateNotifyBalloon(TEXT("SISTEM"), TEXT("IO �ʱ��w�g�ҥ�"));
	}
	void UpdateNotifyBalloon(SIN const TCHAR* title, const TCHAR* info) {
		NOTIFYICONDATA ndata;

		ndata.cbSize = sizeof(NOTIFYICONDATA);
		ndata.hWnd = hgl::msg_wnd;
		ndata.uID = NOTIFY_ID;
		ndata.uFlags = NIF_INFO | NIF_TIP | NIF_SHOWTIP;
		sis::str::Copy(title, ndata.szInfoTitle);
		sis::str::Copy(info, ndata.szInfo);
		sis::str::Copy(NOTIFY_TIP, ndata.szTip);
		ndata.dwInfoFlags = NIIF_NONE;

		Shell_NotifyIcon(NIM_MODIFY, &ndata);
	}
	void ProcMsg(SIN SMSG& msg) {
		switch (msg.type_) {
		case umsg::IO_QUERY: {
			/*
				�����a�B����
			*/
			//	log only
			unsigned int config_value;
			sgl::config_mutex.lock();
			config_value = sgl::config.LogOnly();
			sgl::config_mutex.unlock();

			if (config_value) {
				//	log here
				slog::EZLog((SIORQ*)msg.ptr_[0]);

				//	reply
				msg.type_ = umsg::IO_REPLY;
				msg.param_[0] = umsg::IO_ACCEPT;
				sgl::master_queue.PushMsg(msg);

				break;
			}
			msg.type_ = umsg::IO_REQUERY;
			sgl::hidden_queue.PushMsg(msg);
			break;
		}
		case umsg::IO_REQUERY:
			/*
				��s queue
			*/
			sgl::io_query_mutex.lock();
			sgl::io_query.push_back(msg);
			sgl::io_query_mutex.unlock();
			UpdateNotifyBalloon(TEXT(""), TEXT("�s�������ШD�i�J"));
			/*
				�p�G GUI �����s�b�A�����L��s����
			*/
			if (sgl::gui_active) {
				msg.type_ = umsg::GUI_QUEUE_UPDATE;
				sgl::hidden_queue.PushMsg(msg);
			}
			break;
		case umsg::GUI_CLOSE:
			/*
				���� thread ����
			*/
			if (sgl::gui_active) {
				WaitForSingleObject(sgl::gui_thread, INFINITE);
				CloseHandle(sgl::gui_thread);
				sgl::gui_active = false;
			}
			break;
		}
	}
	namespace {
		void GetCurrentExeList(SIN std::vector<SStrItem>& list) {
			HANDLE psnap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

			if (psnap_shot == INVALID_HANDLE_VALUE)  return;
			PROCESSENTRY32 pentry = { sizeof(PROCESSENTRY32) };

			DWORD result = -1;
			SStrItem item;
			if (Process32First(psnap_shot, &pentry)) {
				do {
					item.str_ = pentry.szExeFile;
					item.Hash(sgl::hasher);

					SCache::VIT vit = std::lower_bound(list.begin(), list.end(), item);
					if (vit == list.end() || *vit != item) {
						//	�S�����ƪ��ܡA���J
						list.insert(vit, item);
					}
				} while (Process32Next(psnap_shot, &pentry));
			}
			else {
				return;
			}
			CloseHandle(psnap_shot);
			return;
		}
	}
	void UpdateCache() {
		std::vector<SStrItem> cache_list, exe_list, delete_list;

		sgl::cache_mutex.lock();
		sgl::cache.GetExeList(cache_list);
		sgl::cache_mutex.unlock();

		if (cache_list.size() == 0) return;

		GetCurrentExeList(exe_list);

		size_t index[2] = { 0, 0 };
		while (index[0] < cache_list.size() && index[1] < exe_list.size()) {
			if (cache_list[index[0]] > exe_list[index[1]]) {
				++index[1];
			}
			else if (cache_list[index[0]] < exe_list[index[1]]) {
				//	index[0] �� process ���s�b
				delete_list.push_back(cache_list[index[0]]);
				++index[0];
			}
			else {
				++index[0];
				++index[1];
			}
		}

		sgl::cache_mutex.lock();
		sgl::cache.DeleteExeList(delete_list);
		sgl::cache_mutex.unlock();
	}
	LRESULT CALLBACK NotifyProc(SIN HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		PAINTSTRUCT ps;
		HDC hdc;
		SMSG msg;

		switch (message) {
		case SWM_NOTIFY:
			switch (LOWORD(lParam)) {
			case WM_LBUTTONUP:
				/*
					GUI thread
				*/
				//	gui �w�ҰʡA��������
				if (sgl::gui_active) break;

				//	�M�ŰT��
				sgl::gui_queue.Clear();

				sgl::gui_thread = (HANDLE)_beginthreadex(NULL, 0, GuiProc, NULL, CREATE_SUSPENDED, NULL);

				if (!sgl::gui_thread) {
					MessageBox(NULL, TEXT("Create GUI window fail"), TEXT("Notice"), MB_OK);
					break;
				}
				ResumeThread(sgl::gui_thread);

				sgl::gui_active = true;
				break;
			default:
				break;
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
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
