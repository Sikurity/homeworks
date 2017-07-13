#include "leeysos.h"
#include "sys_desc.h"
#include "syscall_type.h"
#include "key_def.h"

#define DEFAULT_STACK_SIZE			(64*1024) /* 64kbytes */

static BOOL SyspSetupSysCallgate(VOID);
static BYTE *m_pSyscallStack;

extern CALLGATE_DESC m_GdtTable[NUMBERS_OF_GDT_ENTRIES];

BOOL SysInitializeSyscall(VOID)
{
	
	// �ý��� ���� ȣ�� �Ǿ��� ��, ����ϰ� �� �� ����Ʈ ����
	if( !SyspSetupSysCallgate() )
	{
		DbgPrint("SyspSetupSysCallgate() returned an error.\r\n");
		return FALSE;
	}

	//�ý��� �� ȣ��ÿ� Ŀ�� �������� ���� ������ ����
	m_pSyscallStack = MmAllocateNonCachedMemory(DEFAULT_STACK_SIZE);
	if(m_pSyscallStack == NULL) return FALSE;

	return TRUE;
}

VOID *SysGetSyscallStackPtr(VOID)
{
	return (VOID *)m_pSyscallStack;
}

DWORD SysGetSyscallStackSize(VOID)
{
	return DEFAULT_STACK_SIZE;
}


//�ý��� ���� ������ ó���ϴ� �Լ�
_declspec(naked)  static VOID Sysp_SERVICE_CALL_MANAGER(VOID)
{
	static PSYSCALL_MSG call_msg;
	static KBD_KEY_DATA key_data;
	static int result = 0;

	// esp + 8 ��ġ�� �ִ� ���� �޾ƿ´�.
	// �� ���� �� ����Ʈ�� ���ؼ� �ý��� ���� ȣ��Ǹ鼭 �������α׷����� �Ѿ�� ���� ��
	// �̸� ���� �������α׷��� ȣ���ϰ� ���� ���񽺿� ���� ��

	_asm
	{
		mov		eax, [esp + 8]
		mov		call_msg, eax

		pushad
	}

	// ���� ������ �츮�� �ʿ��� �ý��� �� ���񽺸� ����
	// ������ �ϳ��ϳ� ���񽺸� �ϳ��ϳ� �߰��� ����

	switch( call_msg->syscall_type)
	{
		default:
			break;
	}

	// ������ 4����Ʈ ��ŭ POP
	_asm
	{
		popad
		mov		eax, result

		ret		4
	}
}


//Call Gate Descriptor�� �����ϴ� �Լ�
static BOOL SyspSetupSysCallgate(VOID)
{
ENTER_CRITICAL_SECTION();
	m_GdtTable[SYSCALL_GATE >> 3].count		= 1;
	m_GdtTable[SYSCALL_GATE >> 3].type		= 0xEC;
	m_GdtTable[SYSCALL_GATE >> 3].selector	= KERNEL_CS;

	//Offset �� ����
	m_GdtTable[SYSCALL_GATE >> 3].offset_1	= (BYTE)(((int)Sysp_SERVICE_CALL_MANAGER) & 0x000000ff);
	m_GdtTable[SYSCALL_GATE >> 3].offset_2	= (BYTE)((((int)Sysp_SERVICE_CALL_MANAGER) & 0x0000ff00) >> 8);
	m_GdtTable[SYSCALL_GATE >> 3].offset_3	= (BYTE)((((int)Sysp_SERVICE_CALL_MANAGER) & 0x00ff0000) >> 16);
	m_GdtTable[SYSCALL_GATE >> 3].offset_4	= (BYTE)((((int)Sysp_SERVICE_CALL_MANAGER) & 0xff000000) >> 24);
EXIT_CRITICAL_SECTION();

	return TRUE;
}
