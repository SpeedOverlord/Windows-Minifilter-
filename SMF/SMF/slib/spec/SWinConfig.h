/*
	Copyright © 2016 by Phillip Chang

	SWinConfig

	設定預設保護路徑
*/
#pragma once

#include "SConfig.h"
#include "slib/SPathLib.h"

#include <ShlObj.h>
#include <KnownFolders.h>
#include <VersionHelpers.h>

namespace winconfig {
	namespace {
		enum WinVer {
			WIN7,
			WIN8,
			WIN10
		};
		/*
			取得作業系統版本
		*/
		WinVer GetVersion() {
			if (IsWindowsVersionOrGreater(10, 0, 0)) return WIN10;
			if (IsWindows8OrGreater()) return WIN8;
			return WIN7;
		}
		/*
			取得當前 user 路徑
		*/
		SCharAry user_path, program_file[2];	//	global
		void GetUserPath() {
			TCHAR *ptr = NULL, buf[MAX_PATH];
			SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &ptr);
			sis::path::GetParentDir(ptr, buf);
			CoTaskMemFree(ptr);
			user_path = buf;
		}
		void SetCommonConfig(SINOUT SConfig& config) {
			std::map<SCharAry, std::vector<SCharAry>> mp;
			std::vector<SCharAry> vec;
			SCharAry str;
			TCHAR buf[MAX_PATH];

			config.GetProtectPath(vec);
			//	protect
			str = TEXT("C:\\Windows");		vec.push_back(str);
			//	end
			config.SetProtectPath(vec);

			config.GetSharePath(vec);
			//	share
			str = TEXT("C:\\ProgramData");	vec.push_back(str);
			str = TEXT("C:\\Downloads");	vec.push_back(str);
			str = TEXT("C:\\User\\Public");	vec.push_back(str);
			wsprintf(buf, TEXT("%s\\AppData"), user_path.Ptr());	str = buf;	vec.push_back(str);
			wsprintf(buf, TEXT("%s\\Downloads"), user_path.Ptr());	str = buf;	vec.push_back(str);
			wsprintf(buf, TEXT("%s\\Desktop"), user_path.Ptr());	str = buf;	vec.push_back(str);
			//	end
			config.SetSharePath(vec);

			config.GetExePath(vec);
			//	exe
			//	end
			config.SetExePath(vec);

			config.GetWorkspace(mp);
			//	work space
			//	end
			config.SetWorkspace(mp);
		}
		void SetWin7Config(SINOUT SConfig& config) {

		}
		void SetWin8Config(SINOUT SConfig& config) {

		}
		void SetWin10Config(SINOUT SConfig& config) {

		}
	}
	void SetDefaultConfig(SINOUT SConfig& config) {
		WinVer ver = GetVersion();
		GetUserPath();

		SetCommonConfig(config);
		switch (ver) {
		case WIN7:
			SetWin7Config(config);
			break;
		case WIN8:
			SetWin8Config(config);
			break;
		case WIN10:
			SetWin10Config(config);
			break;
		}
	}
	/*
		取得 program file 資料夾路徑
		回傳是否在 program file 底下


	*/
	void InitializeProgramFilePath() {
		if (sis::path::FileExist(TEXT("C:\\Program Files"))) program_file[0] = TEXT("C:\\Program Files");
		if (sis::path::FileExist(TEXT("C:\\Program Files (x86)"))) program_file[1] = TEXT("C:\\Program Files (x86)");
	}
	bool GetProgramFilePath(SIN SCharAry& file, SOUT SCharAry& dir) {
		for (size_t i = 0; i < 2; ++i) {
			if (program_file[i].Length() == 0) continue;
			if (!sis::str::IsParentPath(file.Ptr(), file.Length(), program_file[i].Ptr(), program_file[i].Length())) continue;
			size_t slash_index = program_file[i].Length() + 1;
			while (slash_index < file.Length() && file[slash_index] != TEXT('\\')) ++slash_index;
			if (slash_index == file.Length()) return false;	//	是 prefix 卻沒有 slash
			dir.Copy(file.Ptr(), slash_index);
			return true;
		}
		return false;
	}
}
