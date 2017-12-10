/*
	Log 檔寫入
*/

#pragma once

#include "SIorq.h"
#include "SGlobal.h"
#include "slib\SPathLib.h"

namespace slog {
	/*
		開啟數字序最小且不超過 1024 KB 的檔案
	*/
	namespace {
		/*
			const
		*/
		const TCHAR LOG_NAME[] = TEXT("io_log"),
			LOG_EXT[] = TEXT("log");
		const LONGLONG LOG_MAX_SIZE = 1024 * 1024;
		const TCHAR ENTER_TSTR[] = { 0x0D, 0x0A, 0x00 };
		/*
			data
		*/
		std::mutex log_mutex;
		TCHAR write_buf[MAX_PATH * 4], access_buf[20], rename_buf[MAX_PATH * 2];	//	1000+ TCHAR
		int log_index = 1;
		/*
			function
		*/
		bool OpenLog(SOUT HANDLE& file) {
			LARGE_INTEGER file_size;
			HANDLE temp_file;
			TCHAR buf[MAX_PATH];
		CHECK_EXIST:
			wsprintf(buf, TEXT("%s\\%s_%d.%s"), sgl::exe_dir, LOG_NAME, log_index, LOG_EXT);
			if (sis::path::FileExist(buf)) {
				temp_file = CreateFile(buf, GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (temp_file == INVALID_HANDLE_VALUE) {
					++log_index;
					goto CHECK_EXIST;
				}
				GetFileSizeEx(temp_file, &file_size);
				if (file_size.QuadPart >= LOG_MAX_SIZE) {
					++log_index;
					goto CHECK_EXIST;
				}
			}
			else {
				temp_file = CreateFile(buf, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (temp_file == INVALID_HANDLE_VALUE) {
					++log_index;
					goto CHECK_EXIST;
				}
			}
			file = temp_file;
			return true;
		}
		void WriteLog(SIN HANDLE file, SIORQ* iorq) {
			DWORD write_length;
			iorq->GetAccessString(access_buf);
			rename_buf[0] = 0;
			if (iorq->access_.IO_RENAME) {
				wsprintf(rename_buf, TEXT("RENAME = %s%s"), iorq->des_.Ptr(), ENTER_TSTR);
			}
			wsprintf(write_buf, TEXT("ACCESS = %s%sEXE = %s%sFILE = %s%s%s%s"),
				access_buf, ENTER_TSTR,
				iorq->exe_.Ptr(), ENTER_TSTR,
				iorq->file_.Ptr(), ENTER_TSTR,
				rename_buf,
				ENTER_TSTR);
			WriteFile(file, write_buf, sis::str::Length(write_buf)*sizeof(TCHAR), &write_length, NULL);
		}
	}
	bool EZLog(SIN SIORQ* iorq) {
		std::unique_lock<std::mutex> um(log_mutex);
		HANDLE file;
		if (!OpenLog(file)) return false;
		LARGE_INTEGER pos, org_pos;
		pos.QuadPart = 0;
		SetFilePointerEx(file, pos, &org_pos, FILE_END);
		WriteLog(file, iorq);
		CloseHandle(file);
		return true;
	}
}
