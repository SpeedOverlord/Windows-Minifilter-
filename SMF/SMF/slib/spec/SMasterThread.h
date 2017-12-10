/*
	�D�{��
*/
#pragma once

#include "SGlobal.h"
#include "SDriverMsg.h"
#include "SUserMsg.h"
#include "SMFlt.h"
#include "SHiddenThread.h"

#include "slib\SCharAry.h"
#include "slib\SPathLib.h"

#pragma comment(lib,"fltlib.lib")

const WCHAR DRIVER_PORT[] = L"\\BT_PORT";

//	master global
namespace mgl {
	//	port
	HANDLE driver_port;

	//	path
	std::map<TCHAR, SCharAry> dos_path_map;
	TCHAR config_path[MAX_PATH], exe_path[MAX_PATH];

	//	recv
	const DWORD RECV_SIZE = sizeof(FILTER_MESSAGE_HEADER)+MAX_PATH * 2;
	char recv_buf[RECV_SIZE];
	TCHAR temp_path[2][MAX_PATH];
	OVERLAPPED ovlp;

	//	proc
	void ProcUserMsg(SIN SMSG& msg);
	void ProcIORQ(SIN SIORQ* iorq);
}

bool ConnectDriver(SOUT HRESULT& result) {
	HANDLE id[2] = { (HANDLE)GetCurrentProcessId(), (HANDLE)GetCurrentThreadId() };
	//	���� 5 ��
	for (int i = 0; i < 5; ++i) {
		result = FilterConnectCommunicationPort(DRIVER_PORT, 0, id, sizeof(HANDLE)* 2, NULL, &mgl::driver_port);
		if (result == S_OK) break;
	}
	return result == S_OK;
}

unsigned int __stdcall MasterProc(SIN void* data) {
	/*
		���J Filter
	*/
	HRESULT result;

	result = FilterLoad(TEXT("BT"));
	if (result != S_OK) {
		MSGB(TEXT("Driver Load Fail"));
		return 0;
	}
	EnumAttachDrive();

	//system("fltmc load BT");
	//system("fltmc attach BT E:\\ -a 500000");
	/*
		��l�� overlap
	*/
	ZeroMemory(&mgl::ovlp, sizeof(OVERLAPPED));
	mgl::ovlp.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (mgl::ovlp.hEvent == NULL) {
		MSGB(TEXT("��l�� Overlap ����"));
		return 0;
	}
	/*
		�Ұ� hidden thread
	*/
	sgl::hidden_thread = (HANDLE)_beginthreadex(NULL, 0, HiddenProc, NULL, CREATE_SUSPENDED, NULL);
	if (sgl::hidden_thread == NULL) {
		MSGB(TEXT("��l�� HiddenThread ����"));
		return 0;
	}
	ResumeThread(sgl::hidden_thread);
	

	/*
		�}�l�P driver ���q
	*/
	if (!ConnectDriver(result)) {
		wsprintf(SDEBUG, TEXT("�s�u�� driver ���� %X\n"), (int)result);
		MSGB(SDEBUG);
		return 0;
	}
	/*
		���ͷ�e�����ɸ��|�� config �ɸ��|
	*/
	sis::path::GetExePathForPid(GetCurrentProcessId(), mgl::temp_path[0]);
	sis::path::GetWin32PathForDosPath(mgl::dos_path_map, mgl::temp_path[0], mgl::exe_path);
	sis::path::GetParentDir(mgl::exe_path, mgl::temp_path[0]);
	sis::str::Copy(mgl::temp_path[0], sgl::exe_dir);
	wsprintf(mgl::config_path, TEXT("%s\\%s"), mgl::temp_path[0], DEFAULT_CONFIG_DIR);

	MSGB(mgl::exe_path);
	MSGB(mgl::config_path);

	/*
		Ū���]�w
	*/
	if (!sgl::config.LoadFile()) {
		MSGB(TEXT("Ū���]�w���~"));
		return 0;
	}
	winconfig::SetDefaultConfig(sgl::win_config);

	DWORD wait_result;
	SMSG msg;
	PFILTER_MESSAGE_HEADER mh;
	char* ptr = NULL;
	int type;
	SIORQ *iorq = NULL;
	USHORT path_length;
	ULONG des_length;
	while (!sgl::fatal_error) {
		result = FilterGetMessage(mgl::driver_port, (PFILTER_MESSAGE_HEADER)mgl::recv_buf, mgl::RECV_SIZE, &mgl::ovlp);

		if (result != S_OK && (result&ERROR_IO_PENDING) != ERROR_IO_PENDING) {
			wsprintf(SDEBUG, TEXT("���� driver �T������ %X\n"), (int)result);
			MSGB(SDEBUG);
			break;
		}

		do {
			wait_result = WaitForSingleObject(mgl::ovlp.hEvent, sgl::TIMEOUT);

			while (sgl::master_queue.PeekMsg(sgl::TIMEOUT, msg)) {
				//	proc msg
				mgl::ProcUserMsg(msg);
			}
		} while (wait_result == WAIT_TIMEOUT);

		if (wait_result == WAIT_FAILED) {
			wsprintf(SDEBUG, TEXT("���� driver �T������\n"));
			MSGB(SDEBUG);
			break;
		}

		mh = (PFILTER_MESSAGE_HEADER)mgl::recv_buf;
		ptr = mgl::recv_buf;	ptr += sizeof(FILTER_MESSAGE_HEADER);
		/*
			�ѪR�ʥ]
		*/
		serial::PopData(ptr, type);
		switch (type) {
		case dmsg::SGPATH_RQ:
		case dmsg::DBPATH_RQ:
			iorq = new SIORQ;

			iorq->msg_id_ = mh->MessageId;
			iorq->reply_length_ = mh->ReplyLength;

			serial::PopData(ptr, iorq->pid_);
			serial::PopData(ptr, iorq->tid_);
			serial::PopData(ptr, iorq->access_);

			//	file path
			serial::PopData(ptr, path_length);

			sis::str::Copy((TCHAR*)ptr, path_length, mgl::temp_path[0]);	ptr += path_length*sizeof(TCHAR);
			mgl::temp_path[0][path_length] = 0;
			sis::path::GetWin32PathForDosPath(mgl::dos_path_map, mgl::temp_path[0], mgl::temp_path[1]);
			iorq->file_ = mgl::temp_path[1];

			//	rename path
			if (type == dmsg::DBPATH_RQ) {
				serial::PopData(ptr, des_length);

				sis::str::Copy((TCHAR*)ptr, des_length, mgl::temp_path[0]);
				mgl::temp_path[0][des_length] = 0;
				sis::path::GetWin32PathForDosPath(mgl::dos_path_map, mgl::temp_path[0], mgl::temp_path[1]);
				iorq->des_ = mgl::temp_path[1];
			}

			//	exe path
			sis::path::GetExePathForPid((DWORD)iorq->pid_, mgl::temp_path[0]);
			sis::path::GetWin32PathForDosPath(mgl::dos_path_map, mgl::temp_path[0], mgl::temp_path[1]);
			iorq->exe_ = mgl::temp_path[1];

			iorq->update_cache_ = false;

			GetLocalTime(&iorq->time_);

			mgl::ProcIORQ(iorq);
			break;
		default:
			MSGB(TEXT("���w�q�T������"));
			break;
		}
		
	}
	sgl::fatal_error = true;

	return 0;
}

namespace mgl {
	/*
	*/
	void ProcReply(SIN SMSG& msg) {
		const size_t reply_length = sizeof(FILTER_REPLY_HEADER)+1;
		char reply_buf[reply_length];
		SIORQ* iorq = (SIORQ*)msg.ptr_[0];
		PFILTER_REPLY_HEADER rh;

		rh = (PFILTER_REPLY_HEADER)reply_buf;
		rh->MessageId = iorq->msg_id_;
		rh->Status = STATUS_SUCCESS;

		reply_buf[reply_length - 1] = (char)(msg.param_[0] & 0xFF);

		//	��s cache
		if (iorq->update_cache_ && msg.param_[0] == umsg::IO_ACCEPT) {
			sgl::cache_mutex.lock();
			sgl::cache.Cache(iorq);
			sgl::cache_mutex.unlock();
		}

		//	�^���귽
		delete iorq;

		if (FilterReplyMessage(mgl::driver_port, rh, reply_length) != S_OK) {
			MSGB(TEXT("�^�_ driver �T������"));
		}
	}

	/*
		�B�z��L thread post ���T��
	*/
	void ProcUserMsg(SIN SMSG& msg) {
		switch (msg.type_) {
		case umsg::IO_REPLY:
			ProcReply(msg);
			break;
		}
	}
	/*
		�Ĥ@���q�B�z IO �ШD
	*/
	void ProcIORQ(SIN SIORQ* iorq) {
		SCharAry dir;
		SMSG msg;
		unsigned int config_value;
		/*
			�p�G�O close�A�վ� cache
			���ݭn�^�ǰT��
		*/
		if (iorq->access_.IO_CLOSE || iorq->access_.IO_CLEANUP) {
			//	���O�ڥؿ��~�վ� cache
			//if (iorq->file_.Length() != 3) mgl::cache.CleanIORQ(iorq);
			delete iorq;
			return;
		}
		/*
			�ǳ� msg
		*/
		msg.ptr_[0] = iorq;

		/*
			�ڥؿ��ާ@�����q�L
			C:/ D:/ etc + NULL
		*/
		if (iorq->file_.Length() == 3) {
			goto POST_ACCEPT;
		}

		/*
			�t�Υ��ҰʡA�����q�L
		*/
		sgl::config_mutex.lock();
		config_value = sgl::config.Active();
		sgl::config_mutex.unlock();
		if (config_value == 0) {
			goto POST_ACCEPT;
		}

		/*
			Ū���q�L
		*/
		sgl::config_mutex.lock();
		config_value = sgl::config.ReadPass();
		sgl::config_mutex.unlock();
		if (config_value && iorq->access_.IO_READ) {
			goto POST_ACCEPT;
		}

		/*
			�ˬd�O�_�� config �ɮסA������
		*/
		if (sis::str::IsParentPath(iorq->file_.Ptr(), iorq->file_.Length(), mgl::config_path, sis::str::Length(mgl::config_path)) ||
			sis::str::Equal(iorq->file_.Ptr(), mgl::exe_path)) {
			goto POST_REJECT;
		}

		/*
			�ˬd cache
		*/
		sgl::cache_mutex.lock();
		config_value = sgl::cache.IsCached(iorq);
		sgl::cache_mutex.unlock();
		if (config_value) {
			goto POST_ACCEPT;
		}

		/*
			�ˬd Config
			
			config_value
			0 = QUERY
			1 = ACCEPT
			2 = UNDEFINE

		*/
		config_value = 2;
		sgl::config_mutex.lock();
		if (sgl::config.IsProtectPath(iorq->file_) ||
			sgl::win_config.IsProtectPath(iorq->file_)) {
			msg.param_[0] = umsg::PROTECT_PATH;
			config_value = 0;
		}
		else if (sgl::config.IsSharePath(iorq->file_) ||
			sgl::win_config.IsSharePath(iorq->file_)) {
			config_value = 1;
		}
		else if ((sgl::config.IsExePath(iorq->exe_) && sgl::config.IsExePath(iorq->file_)) ||
			(sgl::win_config.IsExePath(iorq->exe_) && sgl::win_config.IsExePath(iorq->file_))) {
			config_value = 1;
		}
		else if (sgl::config.IsWorkspace(iorq->exe_, iorq->file_) ||
			sgl::win_config.IsWorkspace(iorq->exe_, iorq->file_)) {
			config_value = 1;
		}
		else if (sgl::config.UseProgramFile() &&
			winconfig::GetProgramFilePath(iorq->exe_, dir) &&
			sis::str::IsParentPath(iorq->file_.Ptr(), iorq->file_.Length(), dir.Ptr(), dir.Length())) {
			//	program file
			config_value = 1;
		}
		sgl::config_mutex.unlock();
		if (config_value == 1) {
			goto POST_ACCEPT;
		}
		else if (config_value == 0) {
			goto POST_QUERY;
		}

		/*
			���]�w�a�a
		*/
		sgl::config_mutex.lock();
		config_value = sgl::config.BlockOnlyWarning();
		sgl::config_mutex.unlock();
		if (config_value) {
			/*
				����|�i��p��
				�o�̴��X���|�tĵ�i

				���|�tĵ�i�z�פW�u�� block_warning = true �~�|�ҥ�
			*/

			/*
				�ˬd���S�����Xĵ�i���ܡA�ШD�q�L
			*/
			goto POST_ACCEPT;
		}

		msg.param_[0] = umsg::UNAUTHORIZE;
		goto POST_QUERY;

	POST_ACCEPT:
		msg.type_ = umsg::IO_REPLY;
		msg.param_[0] = umsg::IO_ACCEPT;

		sgl::master_queue.PushMsg(msg);
		return;
	POST_QUERY:
		sgl::config_mutex.lock();
		config_value = sgl::config.SafeMode();
		sgl::config_mutex.unlock();
		if (config_value) goto POST_REJECT;

		msg.type_ = umsg::IO_QUERY;
		iorq->update_cache_ = true;

		sgl::hidden_queue.PushMsg(msg);
		return;
	POST_REJECT:
		msg.type_ = umsg::IO_REPLY;
		msg.param_[0] = umsg::IO_REJECT;

		sgl::master_queue.PushMsg(msg);
	}
}
