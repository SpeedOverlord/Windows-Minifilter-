#pragma once

#include "SDriverMsg.h"

#include "slib\SCharAry.h"

/*
	�з� IO �ШD
*/
struct SIORQ {
	//	�^�Ǯ����� driver ���Ѽ�
	ULONG reply_length_;
	ULONGLONG msg_id_;

	//	�s���v��
	dmsg::ACCESS access_;
	HANDLE pid_, tid_;
	SCharAry file_, exe_, des_;
	bool update_cache_;		//	�O�_��s cache
	SYSTEMTIME time_;

	void GetAccessString(SINOUT TCHAR* str) {
		bool slash = false;
		size_t index = 0;

		if (access_.IO_READ) {
			if (slash) str[index++] = TEXT('/');
			wsprintf(str + index, TEXT("%s"), TEXT("Ū��"));
			index += 2;
			slash = true;
		}

		if (access_.IO_WRITE) {
			if (slash) str[index++] = TEXT('/');
			wsprintf(str + index, TEXT("%s"), TEXT("�g�J"));
			index += 2;
			slash = true;
		}

		if (access_.IO_DELETE) {
			if (slash) str[index++] = TEXT('/');
			wsprintf(str + index, TEXT("%s"), TEXT("�R��"));
			index += 2;
			slash = true;
		}

		if (access_.IO_OVERWRITE) {
			if (slash) str[index++] = TEXT('/');
			wsprintf(str + index, TEXT("%s"), TEXT("�мg"));
			index += 2;
			slash = true;
		}

		if (access_.IO_RENAME) {
			if (slash) str[index++] = TEXT('/');
			wsprintf(str + index, TEXT("%s"), TEXT("����"));
			index += 2;
			slash = true;
		}
	}

	void Dbg() {
		static TCHAR s[MAX_PATH];
		wsprintf(s, TEXT("RQ : %s\n"), file_.Ptr());
		DPRT(s);

		if ((*((int*)&access_)) == 0) DPRT(TEXT("ACCESS : NULL\n"));
		else {
			wsprintf(s, TEXT("ACCESS : %s%s%s%s%s%s%s%s\n"),
				access_.IO_CREATE ? TEXT("Create ") : TEXT(""),
				access_.IO_READ ? TEXT("Read ") : TEXT(""),
				access_.IO_WRITE ? TEXT("Write ") : TEXT(""),
				access_.IO_RENAME ? TEXT("Rename ") : TEXT(""),
				access_.IO_DELETE ? TEXT("Delete ") : TEXT(""),
				access_.IO_CLOSE ? TEXT("Close ") : TEXT(""),
				access_.IO_CLEANUP ? TEXT("Cleanup ") : TEXT(""),
				access_.IO_OVERWRITE ? TEXT("Overwrite ") : TEXT("")
				);
			DPRT(s);
		}

		wsprintf(s, TEXT("EXE : %s (%d %d)\n"), exe_.Ptr(), (int)pid_, (int)tid_);
		DPRT(s);

		if (access_.IO_RENAME) {
			wsprintf(s, TEXT("DES : %s\n"), des_.Ptr());
			DPRT(s);
		}

		DPRT(TEXT("\n\n"));
	}
};
