/*	
	Copyright © 2016 by Phillip Chang
	
	SBase

	*	引入 win32 標頭，C/C++ 標準標頭檔
	*	定義函數參數修飾符
			排序應為 SINOUT SIN SOUT [SINOUT SIN SOUT](有預設參數)
	*	定義迴圈縮減符
	*	宣告全域 debug 字串
*/
#pragma once

//	win32
#include <Windows.h>
#include <windowsx.h>
#include <CommCtrl.h>

#pragma comment(lib,"ComCtl32.lib")

#pragma comment(linker,"/manifestdependency:\"type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <tchar.h>

//	c++
#include <mutex>
#include <vector>
#include <queue>
#include <map>
#include <process.h>

#ifndef SIN
#define SIN
#endif

#ifndef SOUT
#define SOUT
#endif

#ifndef SINOUT
#define SINOUT
#endif

#define SRP(iterator,iter_time) for(iterator=0;i<iter_time;++i)

TCHAR SDEBUG[MAX_PATH];
#define DPRT(str) OutputDebugString(str)
#define MSGB(str) MessageBox(NULL,str,TEXT("Note"),MB_OK)

namespace serial {
	/*
		自動移動 ptr
	*/
	template<class Data>
	void PopData(SINOUT char*& ptr, SOUT Data& data) {
		data = *((Data*)ptr);
		ptr += sizeof(Data);
	}
	template<class Data>
	void PushDataDef(SINOUT char*& ptr, SIN const Data& data) {
		*((Data*)ptr) = data;
		ptr += sizeof(Data);
	}
	template<class Data>
	void PushData(SINOUT char*& ptr, SIN const Data data) {
		return PushDataDef(ptr, data);
	}
}
