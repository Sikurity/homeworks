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
// PUSHFD를 통해 현재 EFLAGS 레지스터를 스택에 임시 저장
// EFLAGS 레지스터는 시스템 설정 사항이 들어있음
// CLI를 통해 인터럽트 금지 플래그 설정하여 다른 쓰레드의 Context Switching 방지
// 다른 OS 에서는 기타 인터럽트 사용은 계속하도록 하기 위해 다른 CPU 명령어 사용

#define ENTER_CRITICAL_SECTION() __asm PUSHFD __asm CLI

#define EXIT_CRITICAL_SECTION() __asm POPFD

extern KERNELAPI UCHAR	READ_PORT_UCHAR(IN PUCHAR Port);
extern KERNELAPI VOID	WRITE_PORT_UCHAR(IN PUCHAR Port, IN UCHAR Value);
extern KERNELAPI VOID	HalpEnableInterrupt(PIDTR_DESC idtr);
extern KERNELAPI VOID	HalTaskSwitch(VOID);

// 프로세스 관리자
typedef DWORD (*PKSTART_ROUTINE)(PVOID StartContext);

typedef enum _THREAD_STATUS
{
	THREAD_STATUS_STOP,			// 정지 상태
	THREAD_STATUS_TERMINATED,	// 쓰레드의 종료 상태
	THREAD_STATUS_READY,		// 쓰레드의 실행 준비 완료 상태
	THREAD_STATUS_WAITING,		// 쓰레드의 대기 상태
	THREAD_STATUS_RUNNING		// 쓰레드의 실행 상태
} THREAD_STATUS;

// ☞ 스레드에 우선순위 속성을 부여하기 위해 만든 데이터 타입
typedef enum _THREAD_PRIORITY
{
	LOW, 				// ☞ 낮은 우선순위
	MID, 				// ☞ 중간 우선순위
	HIGH				// ☞ 높은 우선순위(시스템 프로세스의 스레드들이 속함)
} THREAD_PRIORITY;		// ☞ 스레드 우선순위 데이터 타입 정의

KERNELAPI BOOL PsCreateProcess(OUT HANDLE ProcessHandle);
KERNELAPI BOOL PsDeleteProcess(IN HANDLE ProcessHandle);
KERNELAPI BOOL PsCreateThread(OUT PHANDLE ThreadHandle, IN HANDLE ProcessHandle, IN PKSTART_ROUTINE StartRoutine, IN PVOID StartContext, IN DWORD StackSize, IN BOOL AutoDelete, THREAD_PRIORITY);
KERNELAPI BOOL PsCreateIntThread(OUT PHANDLE ThreadHandle, IN HANDLE ProcessHandle, IN PKSTART_ROUTINE StartRoutine, IN PVOID StartContext, IN DWORD StackSize);
KERNELAPI BOOL PsDeleteProcess(IN HANDLE ProcessHandle);
KERNELAPI BOOL PsDeleteThread(IN HANDLE ThreadHandle);
KERNELAPI HANDLE PsGetCurrentThread(VOID);
KERNELAPI BOOL PsSetThreadStatus(IN HANDLE ThreadHandle, IN THREAD_STATUS Status);
KERNELAPI BOOL PspCreateUserProcess(VOID);

// 메모리 할당 및 해제
KERNELAPI VOID *MmAllocateNonCachedMemory(IN ULONG NumberOfBytes);
KERNELAPI VOID MmFreeNonCachedMemory(IN PVOID BaseAddress);

#endif /* _LEEYSOS_H_ */