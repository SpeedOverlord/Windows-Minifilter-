#pragma once

#include "slib\SCharAry.h"
#include "slib\spec\SIorq.h"

class SQueryView {
public:
	/*
		���o�ثe QueryView �� window handle
	*/
	HWND Hwnd() const;

	/*
		�b parent ���������w��m�إߤ@�ӷs�� QueryView
		list ����e IO �ШD
			* list ���������� master thread ���t(new)�B����(delete)
			* SQueryView �ݭn�޲z list ���s�W�β��� (�����ɤ����� SIORQ �� delete�A�u�ݱN����бq vector ������)
	*/
	bool Create(SIN int x, int y, int width, int height, HWND parent, HINSTANCE inst, std::vector<SIORQ*>& list);

	/*
		�s�W IO �ШD
	*/
	void AddRequest(SIN std::vector<SIORQ*>& list);

	/*
		���o��e�H����� SIORQ list
	*/
	void GetSelect(SOUT std::vector<SIORQ*>& list);

	/*
		�N��e����� item �R��
			* �ȱN SIORQ ���бq vector �������A���ݭn delete
	*/
	void DeleteSelect();

	/*
		���o��� io_query �����
	*/
	void GetList(SOUT std::vector<SIORQ*>& list);

	/*
		�P�� QueryView
	*/
	void Destroy();
private:
	std::vector<SIORQ*> io_query_;
};
