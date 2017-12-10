/*
	Copyright © 2016 by Phillip Chang

	SGlobal

	宣告全域變數
*/
#pragma once

#include "SConfig.h"
#include "SWinConfig.h"
#include "SCache.h"
#include "SDriverMsg.h"
#include "SIorq.h"
#include "SSimpleHash.h"

#include "slib\SBase.h"
#include "slib\SMsgQueue.h"

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0x00000000
#endif

/*
	標準訊息
*/
struct SMSG {
	SMSG() {
		type_ = param_[0] = param_[1] = 0;
		ptr_[0] = ptr_[1] = NULL;
	}

	int type_, param_[2];
	void* ptr_[2];
};

//	sister global
namespace sgl {
	const DWORD TIMEOUT = 50, IDLE_TIME = 3;
	//	queue 自帶同步控制
	SMsgQueue<SMSG> master_queue, hidden_queue, gui_queue;

	//	hash
	SHasher hasher;

	//	視窗控制代碼
	HINSTANCE inst;

	//	錯誤發生，需要退出
	bool fatal_error = false;

	//	cache
	//	master 使用和新增，hidden 釋放
	std::mutex cache_mutex;
	SCache cache(&hasher);

	//	只有 master/hidden 可以控制 hidden/gui
	HANDLE hidden_thread, gui_thread;
	bool gui_active = false;

	/*	master/gui 可以存
		winconfig 不需要 mutex，因為他只在一開始設定一次後就再也不更改
	*/
	std::mutex config_mutex;
	SConfig config, win_config;

	//	GUI 中 IO 存取的佇列
	//	hidden 和 gui 共同管理
	std::mutex io_query_mutex;
	std::vector<SMSG> io_query;

	/*
		儲存防毒路徑
		用於產生 log file
	*/
	TCHAR exe_dir[MAX_PATH];
}
