/*
	User Space 互傳訊息
*/
#pragma once

namespace umsg {
	/*
		IO_QUERY

		master->hidden

		IO 請求需要詢問使用者

		param[0]	= reason (1 = protect, 2 = 路徑差警告)
		ptr[0]		= SIORQ*
	*/
	const int IO_QUERY = 0;

	const int
		UNAUTHORIZE		= 1,
		PROTECT_PATH	= 2,
		PATH_WARNING	= 3;

	/*
		IO_REQUERY

		hover->hidden

		懸浮視窗消失，將 IO 請求放入 GUI 介面中

		param[0]	= reason (1 = protect, 2 = 路徑差警告)
		ptr[0]		= SIORQ*
	*/
	const int IO_REQUERY = 1;

	/*
		IO_REPLY

		hover->master / master->master / gui->master

		回復 IO
		delete ptr[0]

		param[0]	= reply value (1 = accept, 2 = reject)
		ptr[0]		= SIORQ*
	*/
	const int IO_REPLY = 10;

	const int
		IO_ACCEPT = 1,
		IO_REJECT = 2;

	/*
		GUI_CLOSE
		
		GUI->hidden
	*/
	const int GUI_CLOSE = 5;
	/*
		GUI_QUEUE_UPDATE

		hidden->GUI

		告知 GUI queue 內容有所變化，需要更新 list
	*/
	const int GUI_QUEUE_UPDATE = 6;
}
