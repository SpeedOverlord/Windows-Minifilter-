#include "slib\spec\SMasterThread.h"

#include "slib\spec\SHiddenThread.h"
#include "slib\spec\SGuiThread.h"

DWORD AdjustPrivilege(const TCHAR* str) {
	HANDLE hToken;
	TOKEN_PRIVILEGES token, old_token;
	DWORD ret_size = sizeof(TOKEN_PRIVILEGES);
	LUID luid;
	DWORD status;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		goto ADJ_ERR;
	}

	if (!LookupPrivilegeValue(NULL, str, &luid)) {
		goto ADJ_ERR;
	}

	ZeroMemory(&token, sizeof(TOKEN_PRIVILEGES));
	token.PrivilegeCount = 1;
	token.Privileges[0].Luid = luid;
	token.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &token, sizeof(TOKEN_PRIVILEGES), &old_token, &ret_size)) {
		goto ADJ_ERR;
	}
	CloseHandle(hToken);
	return ERROR_SUCCESS;
ADJ_ERR:
	status = GetLastError();
	CloseHandle(hToken);
	return status;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	/*MSG msg;

	STreeView view;
	SPathAsker asker;
	std::vector<SCharAry> config;

	//view.Create(CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, hInstance, config);

	//ShowWindow(view.Handle(), nCmdShow);
	//UpdateWindow(view.Handle());


	asker.Create(TEXT("LOL"), NULL, hInstance, config);

	if (!asker.Valid()) MessageBox(NULL, TEXT("FAIL"), TEXT("FAIL"), MB_OK);

	ShowWindow(asker.Handle(), nCmdShow);
	UpdateWindow(asker.Handle());

	while (GetMessage(&msg, NULL, 0, 0)) {
		DispatchMessage(&msg);
	}

	return 0;*/
	/*
	*/
	sgl::inst = hInstance;

	/*
		Ω’æ„≈v≠≠
	*/
	DWORD st;
	if ((st = AdjustPrivilege(SE_LOAD_DRIVER_NAME)) != ERROR_SUCCESS) {
		wsprintf(SDEBUG, TEXT("Adjust Privilege Fail %d\n"), st);
		MSGB(SDEBUG);
		return 0;
	}

	//MasterProc(NULL);
	//HiddenProc(NULL);
	sgl::config.LoadFile();
	GuiProc(NULL);

	return 0;

	TCHAR temp_path[MAX_PATH];
	sis::path::GetExePathForPid(420, temp_path);
	std::map<TCHAR, SCharAry> path_map;
	sis::path::GetWin32PathForDosPath(path_map, temp_path, SDEBUG);
	MSGB(SDEBUG);

	return 0;
}
