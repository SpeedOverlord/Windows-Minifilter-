/*
	驅動和 User Space 互傳的訊息
*/
#pragma once

namespace dmsg {
	struct ACCESS {
		unsigned int IO_CREATE : 1;
		unsigned int IO_SETINFO : 1;
		unsigned int IO_READ : 1;
		unsigned int IO_WRITE : 1;
		unsigned int IO_RENAME : 1;
		unsigned int IO_DELETE : 1;
		unsigned int IO_CLOSE : 1;
		unsigned int IO_CLEANUP : 1;
		unsigned int IO_OVERWRITE : 1;
	};
	/*
		SGPATH_RQ

		driver->user

		IO 存取要求進入

		HANDLE	pid
		HANDLE	tid
		char	access
		USHORT	file_path_length
		TCHAR[]	file_path
	*/
	const int SGPATH_RQ = 1;
	/*
		DBPATH_RQ

		driver->user

		IO 存取要求進入

		HANDLE	pid
		HANDLE	tid
		char	access
		USHORT	file_path_length
		TCHAR[]	file_path
		ULONG	rename_path_length
		TCHAR[]	rename_path
	*/
	const int DBPATH_RQ = 2;
}
