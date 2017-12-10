#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
typedef struct ACCESS_ {
	unsigned int IO_CREATE : 1;
	unsigned int IO_SETINFO : 1;
	unsigned int IO_READ : 1;
	unsigned int IO_WRITE : 1;
	unsigned int IO_RENAME : 1;
	unsigned int IO_DELETE : 1;
	unsigned int IO_CLOSE : 1;
	unsigned int IO_CLEANUP : 1;
	unsigned int IO_OVERWRITE : 1;
}ACCESS;

#define SGPATH_RQ 1
#define DBPATH_RQ 2

#define INI_ACCESS(access) ((*((int*)access))=0)

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

PFLT_FILTER gFilterHandle;
PFLT_PORT gFilterPort, gClientPort;
ULONG_PTR OperationStatusCtx = 1;

HANDLE ClientID[2];
#define PTDBG_TRACE_ROUTINES            0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002

ULONG gTraceFlags = 0;

#define PT_DBG_PRINT( _dbgLevel, _string )          \
    (FlagOn(gTraceFlags,(_dbgLevel)) ?              \
        DbgPrint _string :                          \
        ((int)0))

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS
BTUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
BTCreatePreOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

FLT_PREOP_CALLBACK_STATUS
BTRWPreOperation(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
	);

FLT_PREOP_CALLBACK_STATUS
BTSetInfoPreOperation(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
	);

FLT_PREOP_CALLBACK_STATUS
BTCleanPreOperation(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
	);
NTSTATUS
BTConnectCallBack(
	IN PFLT_PORT ClientPort,
	IN PVOID ServerPortCookie,
	IN PVOID ConnectionContext,
	IN ULONG SizeOfContext,
	OUT PVOID *ConnectionPortCookie
	);

NTSTATUS
BTDisconnectCallBack(
	IN PVOID ConnectionCookie
	);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, BTUnload)
#endif

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
	{ IRP_MJ_CREATE,
	0,
	BTCreatePreOperation,
	NULL },

	{ IRP_MJ_READ,
	0,
	BTRWPreOperation,
	NULL },

	{ IRP_MJ_WRITE,
	0,
	BTRWPreOperation,
	NULL },

	{ IRP_MJ_SET_INFORMATION,
	0,
	BTSetInfoPreOperation,
	NULL },

    { IRP_MJ_OPERATION_END }
};

CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks

    BTUnload,                           //  MiniFilterUnload

    NULL,			                    //  InstanceSetup
    NULL,								//  InstanceQueryTeardown
    NULL,							    //  InstanceTeardownStart
    NULL,								//  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

};

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
	NTSTATUS status;
	OBJECT_ATTRIBUTES attr;
	PSECURITY_DESCRIPTOR psd;
	UNICODE_STRING port_name;

    UNREFERENCED_PARAMETER( RegistryPath );

	DbgPrint("BT!HelloWorld\n");

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("BT!DriverEntry: Entered\n") );

    status = FltRegisterFilter( DriverObject,
                                &FilterRegistration,
                                &gFilterHandle );

    FLT_ASSERT( NT_SUCCESS( status ) );

    if (NT_SUCCESS( status )) {
		gFilterPort = NULL;
		gClientPort = NULL;
		RtlInitUnicodeString(&port_name, L"\\BT_PORT");

		status = FltBuildDefaultSecurityDescriptor(&psd, FLT_PORT_ALL_ACCESS);
		if (!NT_SUCCESS(status)) goto ENTRY_INI_ERR;

		InitializeObjectAttributes(&attr, &port_name, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, psd);


		status = FltCreateCommunicationPort(gFilterHandle, &gFilterPort, &attr, NULL, BTConnectCallBack, BTDisconnectCallBack, NULL, 1);
		FltFreeSecurityDescriptor(psd);
		if (!NT_SUCCESS(status)) goto ENTRY_INI_ERR;
		DbgPrint("BT!CreatedCommunicationPort\n");

        status = FltStartFiltering( gFilterHandle );

		if (!NT_SUCCESS(status)) {
		ENTRY_INI_ERR:
			DbgPrint("Error %X",status);
            FltUnregisterFilter( gFilterHandle );
        }
    }

    return status;
}

NTSTATUS
BTUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("BT!BTUnload: Entered\n") );

	FltCloseCommunicationPort(gFilterPort);
    FltUnregisterFilter( gFilterHandle );

    return STATUS_SUCCESS;
}
int SendMsg(int rq_type, ACCESS* access, PFLT_FILE_NAME_INFORMATION info, PFILE_RENAME_INFORMATION rinfo) {
	DbgPrint("BT!%X %S\n\n", *access, info->Name.Buffer);

	PVOID send_buf = NULL, recv_buf = NULL;
	char result = 1, *ptr = NULL;
	ULONG send_length = sizeof(int), recv_length = 1;
	HANDLE pid = PsGetCurrentProcessId(), tid = PsGetCurrentThreadId();
	NTSTATUS status;

	if (gClientPort == NULL) return 1;
	if (pid == ClientID[0]) return 1;

	send_length += sizeof(HANDLE)* 2;
	send_length += sizeof(ACCESS);
	send_length += sizeof(USHORT);
	send_length += info->Name.Length*sizeof(WCHAR);

	if (rq_type == DBPATH_RQ) {
		send_length += sizeof(ULONG);
		send_length += rinfo->FileNameLength*sizeof(WCHAR);
	}

	send_buf = ExAllocatePoolWithTag(PagedPool, send_length, 'sFMS');
	if (send_buf == NULL) return 1;
	recv_buf = ExAllocatePoolWithTag(PagedPool, recv_length, 'rFMS');
	if (recv_buf == NULL) {
		ExFreePoolWithTag(send_buf, 'sFMS');
		return 1;
	}

	((char*)recv_buf)[0] = 0;

	ptr = send_buf;
	((int*)ptr)[0] = rq_type;					ptr += sizeof(int);
	((HANDLE*)ptr)[0] = pid;
	((HANDLE*)ptr)[1] = tid;					ptr += sizeof(HANDLE)* 2;
	((ACCESS*)ptr)[0] = (*access);				ptr += sizeof(ACCESS);
	((USHORT*)ptr)[0] = info->Name.Length;		ptr += sizeof(USHORT);

	for (USHORT i = 0; i < info->Name.Length; ++i)
		((WCHAR*)ptr)[i] = info->Name.Buffer[i];
	ptr += info->Name.Length*sizeof(WCHAR);

	if (rq_type == DBPATH_RQ) {
		((ULONG*)ptr)[0] = rinfo->FileNameLength;	ptr += sizeof(ULONG);

		for (ULONG i = 0; i < rinfo->FileNameLength; ++i)
			((WCHAR*)ptr)[i] = rinfo->FileName[i];
	}

	result = 1;

	if ((access->IO_CLEANUP == 1) || (access->IO_CLOSE == 1)) {
		recv_length = 0;

		status = FltSendMessage(gFilterHandle, &gClientPort, send_buf, send_length, NULL, NULL, NULL);
		ptr = recv_buf;
		*ptr = 1;
	}
	else {
		status = FltSendMessage(gFilterHandle, &gClientPort, send_buf, send_length, recv_buf, &recv_length, NULL);
	}


	if (!NT_SUCCESS(status)) goto FREE_MEM;

	ptr = recv_buf;
	result = *ptr;

FREE_MEM:
	ExFreePoolWithTag(send_buf, 'sFMS');
	ExFreePoolWithTag(recv_buf, 'rFMS');

	return result;
}

FLT_PREOP_CALLBACK_STATUS
BTCreatePreOperation(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
	) {
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	PFLT_FILE_NAME_INFORMATION info = NULL;
	ACCESS access;
	NTSTATUS status;
	ULONG option, disposition;
	UCHAR major_function = Data->Iopb->MajorFunction;
	int reply;

	if (major_function != IRP_MJ_CREATE) return FLT_PREOP_SUCCESS_NO_CALLBACK;

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &info);
	if (!NT_SUCCESS(status)) return FLT_PREOP_SUCCESS_NO_CALLBACK;

	INI_ACCESS(&access);
	option = Data->Iopb->Parameters.Create.Options;
	disposition = option >> 24;

	if ((option & FILE_DELETE_ON_CLOSE) == FILE_DELETE_ON_CLOSE) {
		access.IO_CREATE = 1;
		access.IO_DELETE = 1;
	}
	
	if (disposition == FILE_SUPERSEDE ||
		disposition == FILE_OVERWRITE ||
		disposition == FILE_OVERWRITE_IF) {
		access.IO_CREATE = 1;
		access.IO_OVERWRITE = 1;
	}
	
	reply = 1;

	if (access.IO_CREATE != 0) {
		reply = SendMsg(SGPATH_RQ, &access, info, NULL);
	}

	FltReleaseFileNameInformation(info);

	if (reply == 2) {
		Data->IoStatus.Status = STATUS_ACCESS_VIOLATION;
		return FLT_PREOP_COMPLETE;
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS
BTRWPreOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    ) {
    UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	PFLT_FILE_NAME_INFORMATION info = NULL;
	ACCESS access;
	NTSTATUS status;
	UCHAR major_function = Data->Iopb->MajorFunction;
	int reply;

	if ((major_function != IRP_MJ_READ &&
		major_function != IRP_MJ_WRITE)) return FLT_PREOP_SUCCESS_NO_CALLBACK;

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &info);
	if (!NT_SUCCESS(status)) return FLT_PREOP_SUCCESS_NO_CALLBACK;

	INI_ACCESS(&access);

	switch (major_function) {
	case IRP_MJ_READ:
		access.IO_READ = 1;
		break;
	case IRP_MJ_WRITE:
		access.IO_WRITE = 1;
		break;
	}

	reply = SendMsg(SGPATH_RQ, &access, info, NULL);

	FltReleaseFileNameInformation(info);

	if (reply == 2) {
		Data->IoStatus.Status = STATUS_ACCESS_VIOLATION;
		return FLT_PREOP_COMPLETE;
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS
BTSetInfoPreOperation(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
	) {
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	PFLT_FILE_NAME_INFORMATION info = NULL;
	FILE_INFORMATION_CLASS type;
	ACCESS access;
	NTSTATUS status;
	UCHAR major_function = Data->Iopb->MajorFunction;
	PVOID sinfo = NULL;
	int reply;

	if (major_function != IRP_MJ_SET_INFORMATION) return FLT_PREOP_SUCCESS_NO_CALLBACK;

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &info);
	if (!NT_SUCCESS(status)) return FLT_PREOP_SUCCESS_NO_CALLBACK;

	INI_ACCESS(&access);
	type = Data->Iopb->Parameters.SetFileInformation.FileInformationClass;
	sinfo = Data->Iopb->Parameters.SetFileInformation.InfoBuffer;

	reply = 1;

	switch (type) {
	case FileDispositionInformation:
		if (((PFILE_DISPOSITION_INFORMATION)sinfo)->DeleteFile == TRUE) {
			access.IO_DELETE = 1;
			reply = SendMsg(SGPATH_RQ, &access, info, NULL);
		}
		break;
	case FileRenameInformation:
		access.IO_RENAME = 1;
		if (((PFILE_RENAME_INFORMATION)sinfo)->ReplaceIfExists == TRUE) {
			access.IO_DELETE = 1;
		}
		reply = SendMsg(DBPATH_RQ, &access, info, sinfo);
		break;
	}

	FltReleaseFileNameInformation(info);

	if (reply == 2) {
		Data->IoStatus.Status = STATUS_ACCESS_VIOLATION;
		return FLT_PREOP_COMPLETE;
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS
BTCleanPreOperation(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
	) {
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	PFLT_FILE_NAME_INFORMATION info = NULL;
	ACCESS access;
	NTSTATUS status;
	UCHAR major_function = Data->Iopb->MajorFunction;

	if ((major_function != IRP_MJ_CLOSE && 
		major_function != IRP_MJ_CLEANUP)) return FLT_PREOP_SUCCESS_NO_CALLBACK;

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &info);
	if (!NT_SUCCESS(status)) return FLT_PREOP_SUCCESS_NO_CALLBACK;

	INI_ACCESS(&access);

	switch (major_function) {
	case IRP_MJ_CLOSE:
		access.IO_CLOSE = 1;
		break;
	case IRP_MJ_CLEANUP:
		access.IO_CLEANUP = 1;
		break;
	}

	SendMsg(SGPATH_RQ, &access, info, NULL);

	FltReleaseFileNameInformation(info);

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
NTSTATUS
BTConnectCallBack(
	IN PFLT_PORT ClientPort,
	IN PVOID ServerPortCookie,
	IN PVOID ConnectionContext,
	IN ULONG SizeOfContext,
	OUT PVOID *ConnectionPortCookie
	)
{
	UNREFERENCED_PARAMETER(ServerPortCookie);
	UNREFERENCED_PARAMETER(ConnectionPortCookie);

	PAGED_CODE();

	gClientPort = ClientPort;

	if (SizeOfContext != sizeof(HANDLE)* 2) {
		FltCloseClientPort(gFilterHandle, &gClientPort);

		DbgPrint("BT!Fail Get Client ID\n");
	}
	else {
		for (int i = 0; i < 2; ++i) {
			ClientID[i] = ((HANDLE*)(ConnectionContext))[i];
		}
		DbgPrint("BT!PID/TID %d %d\n", (int)ClientID[0], (int)ClientID[1]);
	}

	return STATUS_SUCCESS;
}

NTSTATUS
BTDisconnectCallBack(
	IN PVOID ConnectionCookie
) {
	UNREFERENCED_PARAMETER(ConnectionCookie);

	PAGED_CODE();

	if (gClientPort != NULL) FltCloseClientPort(gFilterHandle, &gClientPort);

	return STATUS_SUCCESS;
}
