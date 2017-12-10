#pragma once

#include "slib\SCharAry.h"
#include "slib\spec\SIorq.h"

class SQueryView {
public:
	/*
		取得目前 QueryView 的 window handle
	*/
	HWND Hwnd() const;

	/*
		在 parent 視窗的指定位置建立一個新的 QueryView
		list 為當前 IO 請求
			* list 中的元素由 master thread 分配(new)、釋放(delete)
			* SQueryView 需要管理 list 的新增及移除 (移除時不須對 SIORQ 做 delete，只需將其指標從 vector 中移除)
	*/
	bool Create(SIN int x, int y, int width, int height, HWND parent, HINSTANCE inst, std::vector<SIORQ*>& list);

	/*
		新增 IO 請求
	*/
	void AddRequest(SIN std::vector<SIORQ*>& list);

	/*
		取得當前以選取的 SIORQ list
	*/
	void GetSelect(SOUT std::vector<SIORQ*>& list);

	/*
		將當前選取的 item 刪除
			* 僅將 SIORQ 指標從 vector 中移除，不需要 delete
	*/
	void DeleteSelect();

	/*
		取得整個 io_query 的資料
	*/
	void GetList(SOUT std::vector<SIORQ*>& list);

	/*
		銷毀 QueryView
	*/
	void Destroy();
private:
	std::vector<SIORQ*> io_query_;
};
