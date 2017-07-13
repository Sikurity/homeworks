#ifndef _LEEYSOS_H_
#define _LEEYSOS_H_

/* common */
#include "types.h"
#include "debug.h"

/* libraries */
#include "string.h"
#include "vsprintf.h"

/* device drivers */
#include "sys_desc.h"
#include "6845crt.h"

/* x86 system speific */
// PUSHFD�� ���� ���� EFLAGS �������͸� ���ÿ� �ӽ� ����
// EFLAGS �������ʹ� �ý��� ���� ������ �������
// CLI�� ���� ���ͷ�Ʈ ���� �÷��� �����Ͽ� �ٸ� �������� Context Switching ����
// �ٸ� OS ������ ��Ÿ ���ͷ�Ʈ ����� ����ϵ��� �ϱ� ���� �ٸ� CPU ��ɾ� ���

#define ENTER_CRITICAL_SECTION() __asm PUSHFD __asm CLI

#define EXIT_CRITICAL_SECTION() __asm POPFD

extern KERNELAPI UCHAR	READ_PORT_UCHAR(IN PUCHAR Port);
extern KERNELAPI VOID	WRITE_PORT_UCHAR(IN PUCHAR Port, IN UCHAR Value);
extern KERNELAPI VOID	HalpEnableInterrupt(PIDTR_DESC idtr);
extern KERNELAPI VOID	HalTaskSwitch(VOID);

// ���μ��� ������
typedef DWORD (*PKSTART_ROUTINE)(PVOID StartContext);

typedef enum _THREAD_STATUS
{
	THREAD_STATUS_STOP,			// ���� ����
	THREAD_STATUS_TERMINATED,	// �������� ���� ����
	THREAD_STATUS_READY,		// �������� ���� �غ� �Ϸ� ����
	THREAD_STATUS_WAITING,		// �������� ��� ����
	THREAD_STATUS_RUNNING		// �������� ���� ����
} THREAD_STATUS;

// �� �����忡 �켱���� �Ӽ��� �ο��ϱ� ���� ���� ������ Ÿ��
typedef enum _THREAD_PRIORITY
{
	LOW, 				// �� ���� �켱����
	MID, 				// �� �߰� �켱����
	HIGH				// �� ���� �켱����(�ý��� ���μ����� ��������� ����)
} THREAD_PRIORITY;		// �� ������ �켱���� ������ Ÿ�� ����

KERNELAPI BOOL PsCreateProcess(OUT HANDLE ProcessHandle);
KERNELAPI BOOL PsDeleteProcess(IN HANDLE ProcessHandle);
KERNELAPI BOOL PsCreateThread(OUT PHANDLE ThreadHandle, IN HANDLE ProcessHandle, IN PKSTART_ROUTINE StartRoutine, IN PVOID StartContext, IN DWORD StackSize, IN BOOL AutoDelete, THREAD_PRIORITY);
KERNELAPI BOOL PsCreateIntThread(OUT PHANDLE ThreadHandle, IN HANDLE ProcessHandle, IN PKSTART_ROUTINE StartRoutine, IN PVOID StartContext, IN DWORD StackSize);
KERNELAPI BOOL PsDeleteProcess(IN HANDLE ProcessHandle);
KERNELAPI BOOL PsDeleteThread(IN HANDLE ThreadHandle);
KERNELAPI HANDLE PsGetCurrentThread(VOID);
KERNELAPI BOOL PsSetThreadStatus(IN HANDLE ThreadHandle, IN THREAD_STATUS Status);
KERNELAPI BOOL PspCreateUserProcess(VOID);

// �޸� �Ҵ� �� ����
KERNELAPI VOID *MmAllocateNonCachedMemory(IN ULONG NumberOfBytes);
KERNELAPI VOID MmFreeNonCachedMemory(IN PVOID BaseAddress);

#endif /* _LEEYSOS_H_ */