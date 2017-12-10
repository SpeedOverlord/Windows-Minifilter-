/*
	User Space ���ǰT��
*/
#pragma once

namespace umsg {
	/*
		IO_QUERY

		master->hidden

		IO �ШD�ݭn�߰ݨϥΪ�

		param[0]	= reason (1 = protect, 2 = ���|�tĵ�i)
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

		�a�B���������A�N IO �ШD��J GUI ������

		param[0]	= reason (1 = protect, 2 = ���|�tĵ�i)
		ptr[0]		= SIORQ*
	*/
	const int IO_REQUERY = 1;

	/*
		IO_REPLY

		hover->master / master->master / gui->master

		�^�_ IO
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

		�i�� GUI queue ���e�����ܤơA�ݭn��s list
	*/
	const int GUI_QUEUE_UPDATE = 6;
}
