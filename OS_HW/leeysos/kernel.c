#include "leeysos.h"

BOOL KrnInitializeKernel(VOID);

extern BOOL HalInitializeHal(VOID);
extern BOOL MmkInitializeMemoryManager(VOID);
extern BOOL PskInitializeProcessManager(VOID);
extern BOOL SysInitializeSyscall(VOID);

// Ŀ�� �ʱ�ȭ�� �ϱ� ���� �Լ� �߰�
BOOL KrnInitializeKernel(VOID)
{
	if( !HalInitializeHal() )
	{
		DbgPrint("HalInitializeHal() returned an error.\r\n");
		return FALSE;
	}

	// �޸� �ʱ�ȭ ���
	if( !MmkInitializeMemoryManager() )
	{
		DbgPrint("MmkInitializeMemoryManager() returned an error.\r\n");
		return FALSE;
	}
	
	// ���μ��� �ʱ�ȭ ���
	if( !PskInitializeProcessManager() )
	{
		DbgPrint("PskInitializeProcessManager() returned an error.\r\n");
		return FALSE;
	}

	// �ý��� �� �ʱ�ȭ �Լ� ȣ��
	if( !SysInitializeSyscall() )
	{
		DbgPrint("SysInitializeSyscall() returned an error.\r\n");
		return FALSE;
	}

	/*
		���� ���μ������� �پ��� �켱������ �߰����־� Multilevel Queue Scheduling�� �� �Ǵ��� Ȯ��


	*/

	return TRUE;
}