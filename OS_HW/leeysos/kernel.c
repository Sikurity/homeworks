#include "leeysos.h"

BOOL KrnInitializeKernel(VOID);

extern BOOL HalInitializeHal(VOID);
extern BOOL MmkInitializeMemoryManager(VOID);
extern BOOL PskInitializeProcessManager(VOID);
extern BOOL SysInitializeSyscall(VOID);

// 커널 초기화를 하기 위한 함수 추가
BOOL KrnInitializeKernel(VOID)
{
	if( !HalInitializeHal() )
	{
		DbgPrint("HalInitializeHal() returned an error.\r\n");
		return FALSE;
	}

	// 메모리 초기화 모듈
	if( !MmkInitializeMemoryManager() )
	{
		DbgPrint("MmkInitializeMemoryManager() returned an error.\r\n");
		return FALSE;
	}
	
	// 프로세스 초기화 모듈
	if( !PskInitializeProcessManager() )
	{
		DbgPrint("PskInitializeProcessManager() returned an error.\r\n");
		return FALSE;
	}

	// 시스템 콜 초기화 함수 호출
	if( !SysInitializeSyscall() )
	{
		DbgPrint("SysInitializeSyscall() returned an error.\r\n");
		return FALSE;
	}

	/*
		여러 프로세스들을 다양한 우선순위로 추가해주어 Multilevel Queue Scheduling이 잘 되는지 확인


	*/

	return TRUE;
}