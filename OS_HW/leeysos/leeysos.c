#include "leeysos.h"

static void halt(char *pMsg);

extern BOOL KrnInitializeKernel(VOID);
extern BOOL CrtInitializeDriver(VOID);
extern BOOL KbdInitializeDriver(VOID);
extern BOOL FddInitializeDriver(VOID);

int leeysos_init(void)
{
	// �ܼ� �ý��� �ʱ�ȭ �Լ� ȣ��
	if( !CrtInitializeDriver() )
		halt(NULL);

	// Ŀ�� �ʱ�ȭ ��ƾ�� ȣ��
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

	PspCreateUserProcess();		// �� �׽�Ʈ �� ���� ���μ��� ����

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

	// ���� �ʱ�ȭ ���� ��, �� �κ� ����(��ü �ý��� ����)
	halt("Booting Error!\r\n");

	return 0;
}

// Ŀ�� ���� �Լ�, ����� ����� ���� �޽��� ǥ��.
static void halt(char *pMsg)
{
	if( pMsg != NULL )
	{
		DbgPrint(pMsg);
		DbgPrint("Halting System.\r\n");
	}

	while( 1 );
}