/*
	Copyright © 2016 by Phillip Chang

	SPathLib

	*	提供較雜的路徑操作
*/
#pragma once

#include "SCharAry.h"

#include <Psapi.h>
#include <shellapi.h>

namespace sis {
	namespace path {
		/*
			回傳檔案是否存在
		*/
		inline bool FileExist(SIN const TCHAR* path) {
			return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
		}
		/*
			取得特定檔案所存在的目錄，不包含尾端 back slash
			若為根目錄，則回傳本身
		*/
		void GetParentDir(SIN const TCHAR* path, SOUT TCHAR* dir) {
			size_t length = str::Length(path), cur_pos = length - 1;
			if (path[cur_pos] == '\\') --cur_pos;
			while (cur_pos > 0 && path[cur_pos] != '\\') --cur_pos;
			if (cur_pos == 0) cur_pos = length;	//	根目錄
			str::Copy(path, cur_pos, dir);
			dir[cur_pos] = 0;
		}
		/*
			取得檔案名稱
		*/
		void GetFileName(SIN const TCHAR* path, SOUT TCHAR* file) {
			size_t length = str::Length(path), cur_pos = length - 1;
			if (path[cur_pos] == '\\') --cur_pos;
			while (cur_pos > 0 && path[cur_pos] != '\\') --cur_pos;
			if (cur_pos == 0 && path[cur_pos] != '\\') {
				str::Copy(path, length, file);
				file[length] = 0;
			}
			else {
				str::Copy(path + cur_pos + 1, length - cur_pos - 1, file);
				file[length - cur_pos - 1] = 0;
			}
		}
		/*
			使用檔案瀏覽器打開目標路徑
		*/
		inline void OpenExplorer(SIN const TCHAR* path) {
			TCHAR buffer[MAX_PATH];
			GetParentDir(path, buffer);
			ShellExecute(NULL, NULL, TEXT("explorer.exe"), buffer, NULL, SW_SHOWNORMAL);
		}
		/*
			利用 process id 取得執行檔路徑
		*/
		bool GetExePathForPid(SIN const DWORD pid, SOUT TCHAR* path) {
			HANDLE process = OpenProcess(/*PROCESS_QUERY_INFORMATION |*/ PROCESS_QUERY_LIMITED_INFORMATION/*| PROCESS_VM_READ*/, FALSE, pid);
			bool result = false;
			path[0] = 0;
			if (process != NULL) {
				DWORD path_length = MAX_PATH;
				//if (QueryFullProcessImageName(process, 0, path, &path_length) == 0) result = false;
				if (GetProcessImageFileName(process, path, MAX_PATH) == 0) result = false;
				//if (GetModuleFileNameEx(process, NULL, path, MAX_PATH) == 0) result = false;
				else {
					result = true;
					path[path_length] = 0;
				}
				CloseHandle(process);
			}
			return result;
		}
		/*
			窮舉時使用的函數指標類型
			
			void*			: custom data
			const TCHAR*	: 為字串指標

			回傳要不要繼續窮舉，false 表示停止
		*/
		typedef bool(*PATHPROC)(SINOUT void*, SIN const TCHAR*);
		/*
			取得所有邏輯磁碟機
			回傳以 <代號>: 為統一格式 Ex: "C:" "D:" "E:"
		*/
		void EnumLogicalDrive(SINOUT void* data, SIN PATHPROC proc) {
			DWORD logical_drive = GetLogicalDrives();
			size_t i;
			TCHAR device_name[] = TEXT("C:\0");
			SRP(i, sizeof(DWORD) << 3) {
				if (logical_drive & 1) {
					device_name[0] = (TCHAR)(TEXT('A') + i);
					if (!proc(data, device_name)) return;
				}
				logical_drive >>= 1;
			}
		}
		/*
			取得邏輯槽對應的 DOS 路徑
		*/
		namespace {
			struct DosPathData {
				bool result_;
				std::map<TCHAR, SCharAry>* path_map_;

				//	在 GetDosPathForLogicalDrive 分配，以增加效能
				SCharAry device_path_;
				TCHAR temp_path_[MAX_PATH];
			};
			bool DosPathProc(SINOUT void* data, SIN const TCHAR* path) {
				DosPathData* path_data = (DosPathData*)data;
				if (QueryDosDevice(path, path_data->temp_path_, MAX_PATH) == 0) {
					path_data->result_ = false;
					return false;
				}
				path_data->device_path_ = path_data->temp_path_;
				(*path_data->path_map_)[path[0]] = path_data->device_path_;
				return true;
			}
		}
		bool GetDosPathForLogicalDrive(SOUT std::map<TCHAR, SCharAry>& path_map) {
			DosPathData data;
			data.result_ = true;
			data.path_map_ = &path_map;

			EnumLogicalDrive(&data, DosPathProc);

			return data.result_;
		}
		/*
			從 DOS 路徑轉成 Win32 路徑
			|Device|HarddiskVolumeX|~

			如果 path 中找不到，會嘗試重建 path 再試一次
			還是找不到時回傳 false
		*/
		bool GetWin32PathForDosPath(SINOUT std::map<TCHAR, SCharAry>& path_map, const TCHAR* src, SOUT TCHAR* des) {
			std::map<TCHAR, SCharAry>::iterator it;
			size_t src_length = sis::str::Length(src), compare_length, copy_length;
			bool update = false, result = false;
		PATH_UPDATE:
			for (it = path_map.begin(); it != path_map.end(); ++it) {
				//wsprintf(SDEBUG, TEXT("COM %s(%d) %s(%d)"), src, (int)src_length, it->second.data_, (int)it->second.length_);
				//MSGB(SDEBUG);
				compare_length = it->second.Length();
				if (sis::str::IsPrefix(src, src_length, it->second.Ptr(), compare_length)) {
					result = true;
					copy_length = src_length - compare_length;
					des[0] = it->first;
					des[1] = TEXT(':');
					sis::str::Copy(src + compare_length, copy_length, des + 2);
					des[2 + copy_length] = 0;
					break;
				}
			}
			//	update path
			if (!result && !update) {
				update = true;
				path_map.clear();
				GetDosPathForLogicalDrive(path_map);
				goto PATH_UPDATE;
			}
			return result;
		}
		/*
			窮舉時使用的函數指標類型

			void*					: custom data
			const WIN32_FIND_DATA*	: 為 OS 搜尋結果
			const TCHAR*			: 為完整路徑

			回傳要不要繼續窮舉，false 表示停止
		*/
		typedef bool(*FILEPROC)(SINOUT void*, SIN const WIN32_FIND_DATA*, const TCHAR*);
		/*
			*	窮舉檔案目錄
			
				data		: PATHPROC 的 custom data
				pre_proc	: 向下遞迴前的子函數
				post_proc	: 向下遞迴結束後的子函數
				path		: 起始路徑 (NULL 結尾)
				level		: 檔案樹向下遞迴的層數 (不包含 path)

			*	回傳值為函數遞迴使用
				false 表示中途有 pre_proc 或 post_proc 停止遞迴

			*	當 pre_proc 或 post_proc 要求停止時(回傳 false)，函數會立即停止
				之前有 pre_proc 的路徑不會呼叫 post_proc
		*/
		bool EnumFile(SINOUT void* data, SIN FILEPROC pre_proc, FILEPROC post_proc, const TCHAR* path, const int level) {
			WIN32_FIND_DATA find_data;
			TCHAR find_path[MAX_PATH];
			HANDLE find_handle;
			bool result = true;	//	pre_proc 或 post_proc 是否要求繼續遞迴

			size_t path_length;

			if (level == 0) goto SKIP_FIND;

			wsprintf(find_path, TEXT("%s\\*"), path);
			path_length = str::Length(find_path) - 1;

			find_handle = FindFirstFile(find_path, &find_data);
			if (find_handle == INVALID_HANDLE_VALUE) goto SKIP_FIND;
			do {
				if (str::Equal(find_data.cFileName, TEXT(".")) || str::Equal(find_data.cFileName, TEXT(".."))) continue;

				str::Copy(find_data.cFileName, find_path + path_length);

				if (pre_proc) {
					if (!pre_proc(data, &find_data, find_path)) {
						result = false;
						break;
					}
				}
				if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (level > 1) {
						result = EnumFile(data, pre_proc, post_proc, find_path, level - 1);
						if (!result) break;
					}
				}
				if (post_proc) {
					if (!post_proc(data, &find_data, find_path)) {
						result = false;
						break;
					}
				}
			} while (FindNextFile(find_handle, &find_data));
			FindClose(find_handle);

		SKIP_FIND:
			return result;
		}
		/*
			回傳該路徑的子資料夾和子檔案數量
			回傳值不超過 stop_cnt
		*/
		size_t GetSubItemCount(const TCHAR* dir, const size_t stop_cnt = 0xFFFFFFFF) {
			WIN32_FIND_DATA find_data;
			TCHAR find_path[MAX_PATH];
			HANDLE find_handle;
			wsprintf(find_path, TEXT("%s\\*"), dir);
			find_handle = FindFirstFile(find_path, &find_data);
			if (find_handle == INVALID_HANDLE_VALUE) return 0;
			size_t cnt = 1;
			while (FindNextFile(find_handle, &find_data) && cnt != stop_cnt) ++cnt;
			FindClose(find_handle);
			return cnt;
		}
	}
}
