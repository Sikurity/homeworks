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
	
	// 시스템 콜이 호출 되었을 때, 사용하게 될 콜 게이트 설정
	if( !SyspSetupSysCallgate() )
	{
		DbgPrint("SyspSetupSysCallgate() returned an error.\r\n");
		return FALSE;
	}

	//시스템 콜 호출시에 커널 영역에서 사용될 스택을 설정
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


//시스템 콜을 실제로 처리하는 함수
_declspec(naked)  static VOID Sysp_SERVICE_CALL_MANAGER(VOID)
{
	static PSYSCALL_MSG call_msg;
	static KBD_KEY_DATA key_data;
	static int result = 0;

	// esp + 8 위치에 있는 값을 받아온다.
	// 이 값은 콜 게이트를 통해서 시스템 콜이 호출되면서 응용프로그램에서 넘어온 인자 값
	// 이를 통해 응용프로그램이 호출하고 싶은 서비스에 대한 값

	_asm
	{
		mov		eax, [esp + 8]
		mov		call_msg, eax

		pushad
	}

	// 각각 앞으로 우리가 필요한 시스템 콜 서비스를 구현
	// 앞으로 하나하나 서비스를 하나하나 추가할 예정

	switch( call_msg->syscall_type)
	{
		default:
			break;
	}

	// 스택좀 4바이트 만큼 POP
	_asm
	{
		popad
		mov		eax, result

		ret		4
	}
}


//Call Gate Descriptor를 설정하는 함수
static BOOL SyspSetupSysCallgate(VOID)
{
ENTER_CRITICAL_SECTION();
	m_GdtTable[SYSCALL_GATE >> 3].count		= 1;
	m_GdtTable[SYSCALL_GATE >> 3].type		= 0xEC;
	m_GdtTable[SYSCALL_GATE >> 3].selector	= KERNEL_CS;

	//Offset 값 설정
	m_GdtTable[SYSCALL_GATE >> 3].offset_1	= (BYTE)(((int)Sysp_SERVICE_CALL_MANAGER) & 0x000000ff);
	m_GdtTable[SYSCALL_GATE >> 3].offset_2	= (BYTE)((((int)Sysp_SERVICE_CALL_MANAGER) & 0x0000ff00) >> 8);
	m_GdtTable[SYSCALL_GATE >> 3].offset_3	= (BYTE)((((int)Sysp_SERVICE_CALL_MANAGER) & 0x00ff0000) >> 16);
	m_GdtTable[SYSCALL_GATE >> 3].offset_4	= (BYTE)((((int)Sysp_SERVICE_CALL_MANAGER) & 0xff000000) >> 24);
EXIT_CRITICAL_SECTION();

	return TRUE;
}
