#include "leeysos.h"

static void halt(char *pMsg);

extern BOOL KrnInitializeKernel(VOID);
extern BOOL CrtInitializeDriver(VOID);
extern BOOL KbdInitializeDriver(VOID);
extern BOOL FddInitializeDriver(VOID);

int leeysos_init(void)
{
	// 콘솔 시스템 초기화 함수 호출
	if( !CrtInitializeDriver() )
		halt(NULL);

	// 커널 초기화 루틴의 호출
	if( !KrnInitializeKernel() )
	{
		halt("KrnInitializeKernel() returned an error.\r\n");
	}

	if( !KbdInitializeDriver() )
	{
		halt("KbdInitializeDriver() returned an error.\r\n");
	}
	CrtPrintf("Keyboard Driver is initialized!! ");

	if( !FddInitializeDriver() )
	{
		halt("FddInitializeDriver() returned an error.\r\n");
	}
	CrtPrintf("Floppy Disk Driver is initialized!!\r\n");

	PspCreateUserProcess();		// ☞ 테스트 용 유저 프로세스 생성

	_asm
	{
		push	eax
		
		pushfd
		pop		eax
		or		ah, 40h	; nested
		push	eax
		popfd

		pop		eax
		iretd
	}

	// 만약 초기화 실패 시, 이 부분 실행(전체 시스템 정지)
	halt("Booting Error!\r\n");

	return 0;
}

// 커널 정지 함수, 디버그 모드일 때는 메시지 표시.
static void halt(char *pMsg)
{
	if( pMsg != NULL )
	{
		DbgPrint(pMsg);
		DbgPrint("Halting System.\r\n");
	}

	while( 1 );
}