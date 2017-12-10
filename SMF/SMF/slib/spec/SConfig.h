/*
	Copyright © 2016 by Phillip Chang

	SConfig

	設定結構
*/
#pragma once

#include "slib\SCharAry.h"

#include <algorithm>

#define ENCRYPT 0xB6

const TCHAR DEFAULT_CONFIG_DIR[] = TEXT("cfg");
const TCHAR DEFAULT_CONFIG_CHECK[] = TEXT("check.scfg");
const TCHAR DEFAULT_CONFIG_PROTECT[] = TEXT("protect.scfg");
const TCHAR DEFAULT_CONFIG_SHARE[] = TEXT("share.scfg");
const TCHAR DEFAULT_CONFIG_EXE[] = TEXT("exe.scfg");
const TCHAR DEFAULT_CONFIG_WORKSP[] = TEXT("worksp.scfg");

enum ConfigType {
	CHECK_CONFIG = 0,
	PROTECT_CONFIG,
	SHARE_CONFIG,
	EXE_CONFIG,
	WORKSPACE_CONFIG
};

struct SCheckConfig {
	unsigned int active_ : 1;				//	系統是否啟動
	unsigned int safe_mode_ : 1;			//	安全模式使否開啟
	unsigned int read_pass_ : 1;			//	讀取操作直接通過
	unsigned int block_only_warning_ : 1;	//	只阻擋保護目錄和警告
	unsigned int program_file_ : 1;			//	將 program file 目錄下所有資料夾視為沙盒路徑
	unsigned int log_only_ : 1;				//	僅將警告儲存於 log 檔中

	SCheckConfig() {
		active_ = 1;
		safe_mode_ = 0;
		read_pass_ = 1;
		block_only_warning_ = 0;
		program_file_ = 1;
		log_only_ = 0;
	}
};

/*
	內部所有 vector 都有 sort 過
*/
class SConfig {
public:
	/*
		file access
	*/
	bool LoadFile();
	bool SaveFile(SIN ConfigType type);
	//	只儲存 check config
	bool SaveCheckFile(SIN const TCHAR* path);
	/*
		path
	*/
	inline bool IsProtectPath(SIN const SCharAry& path) const {
		return InVector(protect_path_, path);
	}
	inline bool IsSharePath(SIN const SCharAry& path) const {
		return InVector(share_path_, path);
	}
	bool IsExePath(SIN const SCharAry& path) const {
		return InVector(exe_path_, path);
	}
	bool IsWorkspace(SIN const SCharAry& exe_path, const SCharAry& file_path) const {
		//	只要在目錄底下的 exe 皆可存取 workspace
		for (MCIT it = workspace_path_.begin(); it != workspace_path_.end(); ++it) {
			if (sis::str::IsParentPath(exe_path.Ptr(), exe_path.Length(), it->first.Ptr(), it->first.Length())) {
				return InVector(it->second, file_path);
			}
		}
		return false;
		//	唯一識別 exe
		/*MCIT cit = workspace_path_.find(exe_path);
		if (cit == workspace_path_.end()) return false;
		return InVector(cit->second, file_path);*/
	}
	void GetProtectPath(SOUT std::vector<SCharAry>& path) const {
		path = protect_path_;
	}
	void GetSharePath(SOUT std::vector<SCharAry>& path) const {
		path = share_path_;
	}
	void GetExePath(SOUT std::vector<SCharAry>& path) const {
		path = exe_path_;
	}
	void GetWorkspace(SOUT std::map<SCharAry, std::vector<SCharAry>>& path) const {
		path = workspace_path_;
	}
	void SetProtectPath(SIN std::vector<SCharAry>& path) {
		protect_path_ = path;
	}
	void SetSharePath(SIN std::vector<SCharAry>& path) {
		share_path_ = path;
	}
	void SetExePath(SIN std::vector<SCharAry>& path) {
		exe_path_ = path;
	}
	void SetWorkspace(SIN std::map<SCharAry, std::vector<SCharAry>>& path) {
		workspace_path_ = path;
	}
	/*
		check
	*/
	unsigned int Active() const {
		return check_config_.active_;
	}
	unsigned int SafeMode() const {
		return check_config_.safe_mode_;
	}
	unsigned int ReadPass() const {
		return check_config_.read_pass_;
	}
	unsigned int BlockOnlyWarning() const {
		return check_config_.block_only_warning_;
	}
	unsigned int UseProgramFile() const {
		return check_config_.program_file_;
	}
	unsigned int LogOnly() const {
		return check_config_.log_only_;
	}
	void GetCheckConfig(SOUT SCheckConfig& config) const {
		config = check_config_;
	}
	void SetCheckConfig(SIN SCheckConfig& config) {
		check_config_ = config;
	}
private:
	typedef std::vector<SCharAry>::iterator VIT;
	typedef std::vector<SCharAry>::const_iterator VCIT;
	typedef std::map<SCharAry, std::vector<SCharAry>>::iterator MIT;
	typedef std::map<SCharAry, std::vector<SCharAry>>::const_iterator MCIT;

	bool InVector(SIN const std::vector<SCharAry>& vec, const SCharAry& path) const {
		size_t i;
		SRP(i, vec.size()) {
			/*
				A:/abcdef/a.cpp 不應該是 A:/abc 的子目錄，因此需要新增第二個條件
			*/
			if (sis::str::IsParentPath(path.Ptr(), path.Length(), vec[i].Ptr(), vec[i].Length())) return true;
		}
		return false;
	}

	std::vector<SCharAry>
		protect_path_,	//	保護路徑
		share_path_,	//	共用路徑
		exe_path_;		//	應用程式沙盒路徑

	std::map<SCharAry, std::vector<SCharAry>> workspace_path_;	//	應用程式工作區路徑

	SCheckConfig check_config_;
};

namespace {
	/*
		將 ary 資料儲存到 ptr 指向的位址
		完成後將 ptr 移動到下一個寫入資料的位址
	*/
	void Push(SINOUT char*& ptr, SIN const SCharAry& ary) {
		serial::PushDataDef(ptr, ary.Length());
		sis::str::Copy(ary.Ptr(), ary.Length(), (TCHAR*)ptr);
		ptr += ary.Length()*sizeof(TCHAR);
	}
	/*
		將 ary 資料從 ptr 指向的位址取出
		完成後將 ptr 移動到下一個讀出資料的位址
	*/
	void Pop(SINOUT char*& ptr, SOUT SCharAry& ary) {
		size_t ary_length;
		serial::PopData(ptr, ary_length);
		ary.Copy((TCHAR*)ptr, ary_length);	ptr += ary_length*sizeof(TCHAR);
	}
	/*
		計算儲存一個 vector 所需空間
	*/
	size_t VectorStoreLength(SIN const std::vector<SCharAry>& vec) {
		size_t i, vec_length = vec.size(), result = sizeof(size_t);
		SRP(i, vec_length) {
			result += sizeof(size_t);
			result += vec[i].Length()*sizeof(TCHAR);
		}
		return result;
	}
	/*
		將 vec 資料寫入
		更新 ptr
	*/
	void Push(SINOUT char*& ptr, SIN const std::vector<SCharAry>& vec) {
		size_t i, vec_length = vec.size();
		serial::PushData(ptr, (size_t)vec.size());
		SRP(i, vec_length) Push(ptr, vec[i]);
	}
	/*
		將 vec 從 buf 指向的位址讀出
		更新 ptr
	*/
	void Pop(SINOUT char*& buf, SOUT std::vector<SCharAry>& vec) {
		size_t i, vec_length = 0;
		vec.clear();

		serial::PopData(buf, vec_length);
		if (vec_length == 0) return;
		vec.resize(vec_length);

		SRP(i, vec_length) Pop(buf, vec[i]);
	}
	/*
		map
	*/
	size_t MapStoreLength(SIN const std::map<SCharAry, std::vector<SCharAry>>& mp) {
		size_t result = sizeof(size_t);
		for (std::map<SCharAry, std::vector<SCharAry>>::const_iterator it = mp.begin(); it != mp.end(); ++it) {
			result += sizeof(size_t);
			result += it->first.Length()*sizeof(TCHAR);

			result += VectorStoreLength(it->second);
		}
		return result;
	}
	void Push(SINOUT char*& ptr, SIN const std::map<SCharAry, std::vector<SCharAry>>& mp) {
		serial::PushData(ptr, mp.size());
		for (std::map<SCharAry, std::vector<SCharAry>>::const_iterator it = mp.begin(); it != mp.end(); ++it) {
			Push(ptr, it->first);
			Push(ptr, it->second);
		}
	}
	void Pop(SINOUT char*& ptr, SOUT std::map<SCharAry, std::vector<SCharAry>>& mp) {
		size_t i, mp_length = 0;
		SCharAry key;
		std::vector<SCharAry> value;
		mp.clear();

		serial::PopData(ptr, mp_length);
		SRP(i, mp_length) {
			Pop(ptr, key);
			Pop(ptr, value);

			mp[key] = value;
		}
	}
	/*
		上層函數
	*/
	void LoadVector(SIN HANDLE file, SOUT std::vector<SCharAry>& vec) {
		LARGE_INTEGER size;
		DWORD read_length;
		GetFileSizeEx(file, &size);

		char *buf = new char[size.QuadPart], *ptr = buf;
		ReadFile(file, buf, (DWORD)size.QuadPart, &read_length, NULL);
		Pop(ptr, vec);
		delete[]buf;
	}
	void LoadVecConfig(SIN const TCHAR* config_file, SOUT std::vector<SCharAry>& vec) {
		HANDLE file;
		TCHAR buf[MAX_PATH];
		wsprintf(buf, TEXT("%s\\%s"), DEFAULT_CONFIG_DIR, config_file);
		file = CreateFile(buf, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (file != INVALID_HANDLE_VALUE) {
			LoadVector(file, vec);
			CloseHandle(file);
		}
	}
}

bool SConfig::LoadFile() {
	HANDLE file;
	TCHAR buf[MAX_PATH];
	DWORD read_length;

	wsprintf(buf, TEXT("%s\\%s"), DEFAULT_CONFIG_DIR, DEFAULT_CONFIG_CHECK);
	file = CreateFile(buf, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE) {
		ReadFile(file, &check_config_, sizeof(SCheckConfig), &read_length, NULL);
		CloseHandle(file);
	}

	LoadVecConfig(DEFAULT_CONFIG_PROTECT, protect_path_);
	LoadVecConfig(DEFAULT_CONFIG_SHARE, share_path_);
	LoadVecConfig(DEFAULT_CONFIG_EXE, exe_path_);

	wsprintf(buf, TEXT("%s\\%s"), DEFAULT_CONFIG_DIR, DEFAULT_CONFIG_WORKSP);
	file = CreateFile(buf, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER size;
		GetFileSizeEx(file, &size);
		
		char *temp = new char[size.QuadPart], *ptr = temp;
		ReadFile(file, temp, (DWORD)size.QuadPart, &read_length, NULL);

		Pop(ptr, workspace_path_);
		delete[]temp;

		CloseHandle(file);
	}

	return true;
}
bool SConfig::SaveFile(SIN ConfigType type) {
	HANDLE file;
	TCHAR buf[MAX_PATH];
	DWORD write_length;
	char *write_buf = NULL, *ptr = NULL;

	//MSGB(TEXT(""))
	//	create directory
	DWORD attr = GetFileAttributes(DEFAULT_CONFIG_DIR);
	if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		CreateDirectory(DEFAULT_CONFIG_DIR, NULL);
	}

	switch (type) {
	case CHECK_CONFIG:
		wsprintf(buf, TEXT("%s\\%s"), DEFAULT_CONFIG_DIR, DEFAULT_CONFIG_CHECK);
		file = CreateFile(buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_NOTIFY_CHANGE_ATTRIBUTES, NULL);
		if (file != INVALID_HANDLE_VALUE) {
			WriteFile(file, &check_config_, sizeof(SCheckConfig), &write_length, NULL);
			CloseHandle(file);
		}
		break;
	case PROTECT_CONFIG:
		wsprintf(buf, TEXT("%s\\%s"), DEFAULT_CONFIG_DIR, DEFAULT_CONFIG_PROTECT);
		file = CreateFile(buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_NOTIFY_CHANGE_ATTRIBUTES, NULL);
		if (file != INVALID_HANDLE_VALUE) {
			write_length = (DWORD)VectorStoreLength(protect_path_);
			ptr = write_buf = new char[write_length];

			Push(ptr, protect_path_);
			WriteFile(file, write_buf, write_length, &write_length, NULL);

			delete[]write_buf;
			CloseHandle(file);
		}
		break;
	case SHARE_CONFIG:
		wsprintf(buf, TEXT("%s\\%s"), DEFAULT_CONFIG_DIR, DEFAULT_CONFIG_SHARE);
		file = CreateFile(buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_NOTIFY_CHANGE_ATTRIBUTES, NULL);
		if (file != INVALID_HANDLE_VALUE) {
			write_length = (DWORD)VectorStoreLength(share_path_);
			ptr = write_buf = new char[write_length];

			Push(ptr, share_path_);
			WriteFile(file, write_buf, write_length, &write_length, NULL);

			delete[]write_buf;
			CloseHandle(file);
		}
		break;
	case EXE_CONFIG:
		wsprintf(buf, TEXT("%s\\%s"), DEFAULT_CONFIG_DIR, DEFAULT_CONFIG_EXE);
		file = CreateFile(buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_NOTIFY_CHANGE_ATTRIBUTES, NULL);
		if (file != INVALID_HANDLE_VALUE) {
			write_length = (DWORD)VectorStoreLength(exe_path_);
			ptr = write_buf = new char[write_length];

			Push(ptr, exe_path_);
			WriteFile(file, write_buf, write_length, &write_length, NULL);

			delete[]write_buf;
			CloseHandle(file);
		}
		break;
	case WORKSPACE_CONFIG:
		wsprintf(buf, TEXT("%s\\%s"), DEFAULT_CONFIG_DIR, DEFAULT_CONFIG_WORKSP);
		file = CreateFile(buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_NOTIFY_CHANGE_ATTRIBUTES, NULL);
		if (file != INVALID_HANDLE_VALUE) {
			write_length = (DWORD)MapStoreLength(workspace_path_);
			ptr = write_buf = new char[write_length];

			Push(ptr, workspace_path_);
			WriteFile(file, write_buf, write_length, &write_length, NULL);

			delete[]write_buf;
			CloseHandle(file);
		}
		break;
	}

	return true;
}
