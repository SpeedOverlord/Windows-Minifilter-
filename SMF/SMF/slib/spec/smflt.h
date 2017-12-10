#pragma once

#include "slib\SPathLib.h"

#include <fltUser.h>

namespace {
	bool available[30];
	bool attached[30];	//	A~Z
	bool ScanDrive(SINOUT void* param, SIN const TCHAR* drive) {
		available[drive[0] - TEXT('A')] = true;
		return 0;
	}
}

void EnumAttachDrive() {
	size_t i;
	SRP(i, 30) {
		available[i] = false;
	}
	sis::path::EnumLogicalDrive(NULL, ScanDrive);
	SRP(i, 30) {
		if (!available[i]) attached[i] = false;
	}

	NTSTATUS result;
	TCHAR volume[10];
	SRP(i, 30) {
		if (!available[i]) continue;
		if (attached[i]) continue;
		wsprintf(volume, TEXT("%c\\"), TEXT('A') + i);

		result = FilterAttachAtAltitude(TEXT("BT"), volume, TEXT("500000"), NULL, 0, NULL);
		if (result == S_OK) {
			attached[i] = true;
		}
		/*char buf[MAX_PATH];
		sprintf_s(buf, "fltmc attack BT %c:\\ -a 500000", drive);
		system(buf);*/
	}
}
