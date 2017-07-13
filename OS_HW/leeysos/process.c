/**
  *	@author 
  *		2011004040 이영식
  *
  * @update 
  *		process.c, leeysos.h, vsprintf.c, 6845crt.c, stdarg.h, leeysos.c
  *			<process.c>		: 각 프로세스에, 스레드 큐(pt_head_thread)를 만들어, TIME / SW 인터럽트 핸들러 함수 PspSetupTaskSWEnv의 PspFindNextThreadScheduled에서 
  *							  0~19의 값을 갖는 current_priority_num의 값을 참고하여 사용할 우선순위의 큐에서 다음 스레드를 갖고 온다. 우선순위가 바뀔 때,
  *							  바뀌기 전의 우선순위에서 마지막으로 실행된 스레드를 pt_last_thread에 저장한 후 나중에 사용한다. 복잡하게 만들지 않기 위하여
  *							  시스템 프로세스의 각 우선순위 스레드 큐에는 IDLE 스레드를 넣어 놓았다. current_priority_num 값은 PspFindNextThreadScheduled
  *							  함수가 끝나면 1씩 증가되고, 20이 되면 0으로 바꾸어 준다.
  *			<leeysos.h>		: 스레드 우선순위 데이터 타입 enum _THREAD_PRIORITY을 선언해줌
  *			<vsprintf.c>	: 과제의 결과물을 편하게 보여주기 위해 콘솔 출력 함수(ffmt) 약간 수정
  *			<6845crt.c>		: 과제의 결과물을 편하게 보여주기 위해 콘솔 출력 함수(CrtPrintfXY) 추가
  *			<stdarg.h>		: 포맷 문자열 출력력 원하는 대로 되지 않아 수정
  *			<leeysos.c>		: 테스트를 위한 유저 프로세스(낮은, 중간, 높은 우선순위 스레드 각 1개, 총 3개 갖음)를 생성시키는 함수(PspCreateUserProcess) 실행
  *
  *	@postscript	
  *		특수문자 ☞를 검색하면 제가 작성한 주석을 쉽게 찾을 수 있습니다.
  */

#include "leeysos.h"
#include "sys_desc.h"

#define DEFAULT_STACK_SIZE			(64*1024) 
#define TS_WATCHDOG_CLOCK_POS		(0xb8000+(80-1)*2)

#define PsGetProcessPtr(handle)		((PPROCESS_CONTROL_BLOCK)handle)
#define PsGetThreadPtr(handle)		((PTHREAD_CONTROL_BLOCK)handle)

#define MAX_CUTTING_ITEM				30

typedef struct _THREAD_CONTROL_BLOCK 
{
	HANDLE								parent_process_handle;		/* memory address */

	DWORD								thread_id;					/* thread id */
	HANDLE								thread_handle;				/* memory address */
	THREAD_STATUS						thread_status;				/* thread status */
	BOOL								auto_delete;

	struct _THREAD_CONTROL_BLOCK		*pt_next_thread;			/* next thread point */

	PKSTART_ROUTINE						start_routine;				/* program entry point */
	PVOID								start_context;				/* context to be passed into the entry routine */
	int									*pt_stack_base_address;		/* stack base address */
	DWORD								stack_size;					/* stack size */
	TSS_32								thread_tss32;				/* TSS32 BLOCK */

	THREAD_PRIORITY						priority;					// ☞ 스레드 우선순위 값 속성 추가
} THREAD_CONTROL_BLOCK, *PTHREAD_CONTROL_BLOCK;

typedef struct _PROCESS_CONTROL_BLOCK {
	DWORD								process_id;					/* process id */
	HANDLE								process_handle;				/* memory address */

	struct _PROCESS_CONTROL_BLOCK		*pt_next_process;			/* next process point used by RR-Scheduler */

	DWORD								thread_count;				/* number of threads */
    DWORD								next_thread_id;				/* next thread id used in this process */

	struct _THREAD_CONTROL_BLOCK		*pt_head_thread[3];			/* first thread point ☞ 각 프로세스의 멀티레벨 큐의 시작 주소 포인터 */
} PROCESS_CONTROL_BLOCK, *PPROCESS_CONTROL_BLOCK;


//Process 리스트를 관리하는 구조체
typedef struct _PROCESS_MANAGER_BLOCK 
{
	DWORD								process_count;				/* number of processes */
	DWORD								next_process_id;			/* next prodess id */
	struct _THREAD_CONTROL_BLOCK		*pt_current_thread;			/* running thread */
	struct _THREAD_CONTROL_BLOCK		*pt_last_thread[3];			// ☞ 각 우선순위 큐별 마지막으로 실행된 스레드
	struct _PROCESS_CONTROL_BLOCK		*pt_head_process;			/* first process point */
} PROCESS_MANAGER_BLOCK, *PPROCESS_MANAGER_BLOCK;


//시스템에서 제거해야하는 Process와 Thread의 리스트에 대한 정보를 관리하는 구조체
typedef struct _CUTTING_LIST 
{

	BYTE								count;
	BYTE								head;
	BYTE								tail;
	HANDLE								handle_list[MAX_CUTTING_ITEM];

} CUTTING_LIST, *PCUTTING_LIST;

extern VOID		*SysGetSyscallStackPtr(VOID);
extern BOOL		HalSetupTSS(TSS_32 *pTss32, BOOL IsKernelTSS, int EntryPoint, int *pStackBase, DWORD StackSize);
extern BOOL		HalWriteTssIntoGdt(TSS_32 *pTss32, DWORD TssSize, DWORD TssNumber, BOOL SetBusy);
extern BOOL		HalSetupTaskLink(TSS_32 *pTss32, WORD TaskLink);
 
static BOOL		PspCreateSystemProcess(VOID);
static DWORD	PspGetNextProcessID(VOID);
static BOOL		PspAddNewProcess(HANDLE ProcessHandle);
static DWORD	PspGetNextThreadID(HANDLE ProcessHandle);
static BOOL		PspAddNewThread(HANDLE ProcessHandle, HANDLE ThreadHandle);
static VOID		PspSetupTaskSWEnv(VOID);
static VOID		PspTaskEntryPoint(VOID);
static BOOL		PspPopCuttingItem(CUTTING_LIST *pCuttingList, HANDLE *pItem);
static BOOL		PspPushCuttingItem(CUTTING_LIST *pCuttingList, HANDLE Item);

// ☞ 실행시켜야할 우선순위 큐가 무엇인지 반환해주는 함수
static THREAD_PRIORITY GetNextRunPriority();

static PROCESS_MANAGER_BLOCK m_ProcMgrBlk;

static CUTTING_LIST m_ProcessCuttingList;
static CUTTING_LIST m_ThreadCuttingList;

static BOOL m_bShowTSWatchdogClock;			// 바람개비 모양
static DWORD m_TickCount;					// 시스템의 틱 값을 저장하는 변수

extern SEGMENT_DESC *m_GdtTable;

static int current_priority_num;	// ☞ 현재 OS 에서 사용될 스레드 우선순위 정도
char	*thread_name[64];			// ☞ 모든 스레드들의 이름, 스레드들의 실행을 눈으로 확인하기 위해 추가
char	*usr_thr_name[3];			// ☞ 테스트용 유저 프로세스의 각 스레드들의 이름을 저장하기 위한 변수
int		gTestNum[3];				// ☞ 테스트용 유저 프로세스의 각 스레드들의 실행 횟수를 저장하기 위한 변수
int		bIsUsrProcessDead;			// ☞ 테스트용 유저 프로세스가 종료 되었는지를 알기 위한 변수

// ☞ 실행시켜야할 우선순위 큐가 무엇인지 반환해주는 함수
static THREAD_PRIORITY GetNextRunPriority()
{
	return current_priority_num < 12 ? HIGH : (current_priority_num < 18 ? MID : LOW);
}

// ☞ 높은 우선순위 테스트용 유저 스레드 함수, 0xff에 다달으면 스레드 종료
static DWORD PspHighPriorityUserThread(PVOID StartContext)
{
	gTestNum[HIGH] = 0;

	while( ++gTestNum[HIGH] != 0xff )
		HalTaskSwitch();

	return 0;
}

// ☞ 중간 우선순위 테스트용 유저 스레드 함수, 0xff에 다달으면 스레드 종료
static DWORD PspMidPriorityUserThread(PVOID StartContext)
{
	gTestNum[MID] = 0;

	while( ++gTestNum[MID] != 0xff )
		HalTaskSwitch();

	return 0;
}

// ☞ 낮은 우선순위 테스트용 유저 스레드 함수, 0xff에 다달으면 스레드 종료
static DWORD PspLowPriorityUserThread(PVOID StartContext)
{
	gTestNum[LOW] = 0;

	while( ++gTestNum[LOW] != 0xff )
		HalTaskSwitch();

	return 0;
}

// ☞ 실험용 유저 프로세스, leeysos.c에서 사용 됨
KERNELAPI BOOL PspCreateUserProcess(VOID)
{
	HANDLE process_handle;
	HANDLE test_handle[3];

	//유저 프로세스를 생성해주는 PSCreateProcess 함수 호출
	if(!PsCreateProcess(&process_handle)) 
		return FALSE;

	// ☞ 테스트 HIGH 시스템 스레드 생성
	if( !PsCreateThread(&test_handle[0], process_handle, PspHighPriorityUserThread, NULL, DEFAULT_STACK_SIZE, TRUE, HIGH) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[0])->thread_id] = "USER_T_HIGH";
	PsSetThreadStatus(test_handle[0], THREAD_STATUS_READY);

	// ☞ 테스트 MID 시스템 스레드 생성
	if( !PsCreateThread(&test_handle[1], process_handle, PspMidPriorityUserThread, NULL, DEFAULT_STACK_SIZE, TRUE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[1])->thread_id] = "USER_T_MID_";
	PsSetThreadStatus(test_handle[1], THREAD_STATUS_READY);

	// ☞ 테스트 LOW 시스템 스레드 생성
	if( !PsCreateThread(&test_handle[2], process_handle, PspLowPriorityUserThread, NULL, DEFAULT_STACK_SIZE, TRUE, LOW) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[2])->thread_id] = "USER_T_LOW_";
	PsSetThreadStatus(test_handle[2], THREAD_STATUS_READY);

	return TRUE;
}

// 모든 변수 초기화 함수
BOOL PskInitializeProcessManager(VOID)
{
	m_ProcMgrBlk.process_count			= 0;
	m_ProcMgrBlk.next_process_id		= 0;
	m_ProcMgrBlk.pt_current_thread		= 0;
	m_ProcMgrBlk.pt_last_thread[LOW]	= NULL;
	m_ProcMgrBlk.pt_last_thread[MID]	= NULL;
	m_ProcMgrBlk.pt_last_thread[HIGH]	= NULL;
	m_ProcMgrBlk.pt_head_process		= NULL;

	m_ProcessCuttingList.count		= 0;
	m_ProcessCuttingList.head		= 0;
	m_ProcessCuttingList.tail		= 0;

	m_ThreadCuttingList.count		= 0;
	m_ThreadCuttingList.head		= 0;
	m_ThreadCuttingList.tail		= 0;

	m_bShowTSWatchdogClock			= TRUE;
	m_TickCount						= 0;

	current_priority_num	= 0;		// ☞ 스레드 실행 우선순위의 기준이 되는 값을 0으로 초기화
	bIsUsrProcessDead		= 0;		// ☞ 유저 프로세스는 생성될 예정이므로, 0(살아있음)으로 초기화

	usr_thr_name[LOW]	= "USR_LOW__T"; // ☞ 테스트용 유저 프로세스의 낮은 우선순위 스레드 이름 대입
	usr_thr_name[MID]	= "USR_MID__T"; // ☞ 테스트용 유저 프로세스의 중간 우선순위 스레드 이름 대입
	usr_thr_name[HIGH]	= "USR_HIGH_T";	// ☞ 테스트용 유저 프로세스의 높은 우선순위 스레드 이름 대입

	gTestNum[LOW]	= 0;				// ☞ 테스트용 유저 프로세스의 높은 우선순위 스레드 실행횟수 0으로 초기화
	gTestNum[MID]	= 0;				// ☞ 테스트용 유저 프로세스의 중간 우선순위 스레드 실행횟수 0으로 초기화
	gTestNum[HIGH]	= 0;				// ☞ 테스트용 유저 프로세스의 낮은 우선순위 스레드 실행횟수 0으로 초기화

	if(!PspCreateSystemProcess()) 
	{
		DbgPrint("PspCreateSystemProcess() returned an error.\r\n");
		return FALSE;
	}

	return TRUE;
}

//프로세스 생성 함수 (PID할당 등 관리)
KERNELAPI BOOL PsCreateProcess(OUT PHANDLE ProcessHandle)
{
	PPROCESS_CONTROL_BLOCK pProcess;

	pProcess = MmAllocateNonCachedMemory(sizeof(PROCESS_CONTROL_BLOCK));
	if(pProcess == NULL) return FALSE;

	pProcess->process_id		= PspGetNextProcessID();
	pProcess->process_handle	= (HANDLE)pProcess;
	pProcess->pt_next_process	= NULL;
	pProcess->thread_count		= 0;
	pProcess->next_thread_id	= 0;
	pProcess->pt_head_thread[LOW]	= NULL;
	pProcess->pt_head_thread[MID]	= NULL;
	pProcess->pt_head_thread[HIGH]	= NULL;
	
	if(!PspAddNewProcess((HANDLE)pProcess)) 
		return FALSE;

	*ProcessHandle = pProcess;

	return TRUE;
}


// 쓰레드 생성 함수 ☞ 스레드의 우선순위 값을 정할 수 있도록, 함수 인자 IN THREAD_PRIORITY Priority 추가
KERNELAPI BOOL PsCreateThread(OUT PHANDLE ThreadHandle, IN HANDLE ProcessHandle, IN PKSTART_ROUTINE StartRoutine, 
					 IN PVOID StartContext, IN DWORD StackSize, IN BOOL AutoDelete, IN THREAD_PRIORITY Priority)
{
	PTHREAD_CONTROL_BLOCK pThread;
	int *pStack;
	
	//메모리할당
	pThread = MmAllocateNonCachedMemory(sizeof(THREAD_CONTROL_BLOCK));
	if(pThread == NULL) return FALSE;
	//쓰레드에서 사용할 스택 할당
	pStack  = MmAllocateNonCachedMemory(StackSize);
	if(pStack == NULL) return FALSE;

	//부모 프로세스의 핸들 설정
	pThread->parent_process_handle		= ProcessHandle;
	//Thread id 및 handle 할당
	pThread->thread_id					= PspGetNextThreadID(ProcessHandle);
	pThread->thread_handle				= (HANDLE)pThread;
	pThread->thread_status				= THREAD_STATUS_STOP;		// Thread 상태를 STOP으로 설정
	pThread->auto_delete				= AutoDelete; 
	pThread->pt_next_thread				= NULL;

	//쓰레드가 실행해야 하는 함수(StartRoutine), 함수에 넘어가는 인자(StartContext), 스택 사이즈 설정
	pThread->start_routine				= StartRoutine;
	pThread->start_context				= StartContext;
	pThread->pt_stack_base_address		= pStack;
	pThread->stack_size					= StackSize;
	pThread->priority					= Priority;					// ☞ 스레드의 우선순위 값 설정

	//PspAddNewThread 함수를 통해 Process에 생성된 쓰레드를 추가
	if( !PspAddNewThread(ProcessHandle, (HANDLE)pThread) ) 
		return FALSE;

	HalSetupTSS(&pThread->thread_tss32, TRUE, (int)PspTaskEntryPoint, pStack, StackSize);

	*ThreadHandle = pThread;

	return TRUE;
}

KERNELAPI BOOL PsCreateIntThread(OUT PHANDLE ThreadHandle, IN HANDLE ProcessHandle, IN PKSTART_ROUTINE StartRoutine,
					   IN PVOID StartContext, IN DWORD StackSize)
{
	PTHREAD_CONTROL_BLOCK pThread;
	int *pStack;

	pThread = MmAllocateNonCachedMemory(sizeof(THREAD_CONTROL_BLOCK));
	if(pThread == NULL) 
		return FALSE;
	
	pStack  = MmAllocateNonCachedMemory(StackSize);
	if(pStack == NULL) 
		return FALSE;

	pThread->parent_process_handle		= ProcessHandle;
	pThread->thread_id					= PspGetNextThreadID(ProcessHandle);
	pThread->thread_handle				= (HANDLE)pThread;
	pThread->thread_status				= THREAD_STATUS_STOP;
	//PsCreateThread함수와 다르게 auto_delete가 false
	pThread->auto_delete				= FALSE;
	pThread->pt_next_thread				= NULL;

	//PsCreateThread함수와 다르게 argument의 StartRoutine을 바로 할당
	pThread->start_routine				= StartRoutine;
	pThread->start_context				= StartContext;
	pThread->pt_stack_base_address		= pStack;
	pThread->stack_size					= StackSize;
	pThread->priority					= HIGH;							// ☞ 인터럽트 스레드의 우선순위는 높게 설정
	if( !PspAddNewThread(ProcessHandle, (HANDLE)pThread) ) 
		return FALSE;

	HalSetupTSS(&pThread->thread_tss32, TRUE, (int)StartRoutine, pStack, StackSize);

	*ThreadHandle = pThread;

	return TRUE;
}
//쓰레드의 상태를 설정하는 함수
KERNELAPI BOOL PsSetThreadStatus(HANDLE ThreadHandle, THREAD_STATUS Status)
{
	PsGetThreadPtr(ThreadHandle)->thread_status = Status;

	return TRUE;
}

//현재 프로세스에서 실행되고 있는 쓰레드의 TCB를 반환하는 함수
KERNELAPI HANDLE PsGetCurrentThread(VOID)
{
	HANDLE thread;

ENTER_CRITICAL_SECTION();

	// m_ProcMgrBlk의 pt_current_thread를 반환
	thread = (HANDLE)(m_ProcMgrBlk.pt_current_thread);

EXIT_CRITICAL_SECTION();

	return thread;
}

// 더 이상 필요하지 않은 프로세스를 삭제하기위한 함수
// ☞ 프로세스를 삭제시키기 위해 추가(프로세스가 삭제되는 것을 보이기 위해 추가)
KERNELAPI BOOL PsDeleteProcess(IN HANDLE ProcessHandle)
{
	return PspPushCuttingItem(&m_ProcessCuttingList, ProcessHandle);
}

// 더 이상 필요하지 않은 쓰레드를 삭제하기위한 함수
KERNELAPI BOOL PsDeleteThread(IN HANDLE ThreadHandle)
{
	return PspPushCuttingItem(&m_ThreadCuttingList, ThreadHandle);
}

static BOOL PspPopCuttingItem(CUTTING_LIST *pCuttingList, HANDLE *pItem)
{
	BOOL bResult = TRUE;

ENTER_CRITICAL_SECTION();

	{
		// 커터 큐에 카운트 체크
		if( pCuttingList->count == 0 )
		{
			bResult = FALSE;
			goto $exit;
		}

		// POP
		pCuttingList->count--;
		*pItem = pCuttingList->handle_list[pCuttingList->head++];

		if( pCuttingList->head >= MAX_CUTTING_ITEM )
			pCuttingList->head = 0;
	}

$exit:
EXIT_CRITICAL_SECTION();

	return bResult;
}

static BOOL PspPushCuttingItem(CUTTING_LIST *pCuttingList, HANDLE item)
{
	BOOL bResult = TRUE;

ENTER_CRITICAL_SECTION();

	{
		// 커터 큐에 남은 공간 체크
		if( pCuttingList->count >= MAX_CUTTING_ITEM )
		{
			bResult = FALSE;
			goto $exit;
		}

		// PUSH
		pCuttingList->count++;
		pCuttingList->handle_list[pCuttingList->tail++] = item;

		if( pCuttingList->tail >= MAX_CUTTING_ITEM )
			pCuttingList->tail = 0;
	}

$exit:
EXIT_CRITICAL_SECTION();

	return bResult;
}


//Process ID 생성
static DWORD PspGetNextProcessID(VOID)
{
	DWORD process_id;

ENTER_CRITICAL_SECTION();
	process_id = m_ProcMgrBlk.next_process_id++;
EXIT_CRITICAL_SECTION();

	return process_id;
}

//Thread ID 생성
static DWORD PspGetNextThreadID(HANDLE ProcessHandle)
{
	DWORD thread_id;

ENTER_CRITICAL_SECTION();
	thread_id = PsGetProcessPtr(ProcessHandle)->next_thread_id++;
EXIT_CRITICAL_SECTION();

	return thread_id;
}

//새로운 프로세스를 넣을 공간을 찾아서 생성
static BOOL PspAddNewProcess(HANDLE ProcessHandle)
{
	PPROCESS_CONTROL_BLOCK *pt_next_process;

ENTER_CRITICAL_SECTION();
	pt_next_process = &m_ProcMgrBlk.pt_head_process;
	while(*pt_next_process)
		pt_next_process = &(*pt_next_process)->pt_next_process;
	*pt_next_process = PsGetProcessPtr(ProcessHandle);
	m_ProcMgrBlk.process_count++;
EXIT_CRITICAL_SECTION();

	return TRUE;
}

//새로운 쓰레드를 넣을 공간을 찾아서 생성
static BOOL PspAddNewThread(HANDLE ProcessHandle, HANDLE ThreadHandle)
{
	THREAD_PRIORITY priority;
	PTHREAD_CONTROL_BLOCK *pt_next_thread;

ENTER_CRITICAL_SECTION();
	priority = PsGetThreadPtr(ThreadHandle)->priority;

	pt_next_thread = &PsGetProcessPtr(ProcessHandle)->pt_head_thread[priority];
	while(*pt_next_thread)
		pt_next_thread = &(*pt_next_thread)->pt_next_thread;
	*pt_next_thread = PsGetThreadPtr(ThreadHandle);
	PsGetProcessPtr(ProcessHandle)->thread_count++;
EXIT_CRITICAL_SECTION();

	return TRUE;
}

// 다음 실행 가능한 쓰레드를 찾기 위한 함수
static HANDLE PspFindNextThreadScheduled(VOID)
{
	int i;

	THREAD_PRIORITY			nextPriority;
	PTHREAD_CONTROL_BLOCK	pt_thread, pt_tmp_thread;
	PPROCESS_CONTROL_BLOCK	pt_process;

	if( m_ProcMgrBlk.process_count == 0 || m_ProcMgrBlk.pt_current_thread == NULL || m_ProcMgrBlk.pt_head_process == NULL )
		return NULL;

	nextPriority	= GetNextRunPriority();				// ☞ 다음 실행될 스레드들이 속한 우선순위 값
	pt_thread		= m_ProcMgrBlk.pt_current_thread;
	
	if( pt_thread->priority != nextPriority)			// ☞ 현재 스레드가 다음 실행될 우선순위에 속하는지 확인
	{
		// ☞ 현재 스레드가 다음 실행될 우선순위에 속하지 않는다면, m_ProcMgrBlk.pt_last_thread에 현재 스레드를 저장해주고,
		m_ProcMgrBlk.pt_last_thread[pt_thread->priority] = pt_thread;
		// ☞ 다음 실행될 우선순위의 스레드 중 마지막에 실행된 스레드를 가져온다.
		pt_tmp_thread = m_ProcMgrBlk.pt_last_thread[nextPriority];

		if( pt_tmp_thread )
			pt_thread = pt_tmp_thread;
		else
			pt_thread = m_ProcMgrBlk.pt_last_thread[HIGH];
	}

$find_thread:
	if(	pt_thread && pt_thread->pt_next_thread != NULL )
		pt_thread = pt_thread->pt_next_thread;
	else
	{
		while(1)
		{
			pt_process = PsGetProcessPtr(pt_thread->parent_process_handle)->pt_next_process;
$find_process:
			if( pt_process == NULL )
				pt_process = m_ProcMgrBlk.pt_head_process;
			if( pt_process->pt_head_thread[nextPriority] == NULL )
			{
				pt_process = pt_process->pt_next_process;
				goto $find_process;
			}
			else
			{
				pt_thread = pt_process->pt_head_thread[nextPriority];
				break;
			}
		}
	}
	
	if( pt_thread->thread_status != THREAD_STATUS_READY && pt_thread->thread_status != THREAD_STATUS_RUNNING )
		goto $find_thread;

	m_ProcMgrBlk.pt_current_thread = pt_thread;

	// ☞ 유저 프로세스가 삭제되었음을 표시
	if( bIsUsrProcessDead )
		CrtPrintfXY(0, 3, "All Threads Are Terminated, The User Process Including Them is Removed\r\n");
	// ☞ 유저 프로세스가 살아있을 때에는 (낮은, 중간, 높은 우선순위 3개)스레드들의 실행된 횟수 및, 스레드 종료 여부 표시
	else
		CrtPrintfXY(0, 3, "%s : %00x, %s : %00x, %s : %00x\r\n", usr_thr_name[LOW], gTestNum[LOW], usr_thr_name[MID], gTestNum[MID], usr_thr_name[HIGH], gTestNum[HIGH]);
	
	// ☞ 각 Current_priority_num 일 때, 실행된 스레드 이름을 출력
	CrtPrintfXY(0, current_priority_num + 4, "Currnt_Priority_Num : %00d, Thread Name : %s\r\n", current_priority_num, thread_name[pt_thread->thread_id]);	
	
	// ☞ 너무 빠르므로, 강제로 느리게 만들음
	for( i = 0 ; i < 500000 ; i++ );

	return (HANDLE)pt_thread;
}

// 태스크 스위칭 방법 중 하나
static VOID PspSetupTaskSWEnv(VOID)
{
	HANDLE current_thread, next_thread;

	// 현재 실행되고 있는 쓰레드를 가져옴
	current_thread = PsGetCurrentThread();

	// 다음 실행 가능한 쓰레드를 찾는다
	next_thread = PspFindNextThreadScheduled(); // At this time, current thread is changed with new thing

	if( ++current_priority_num >= 20 )	// ☞ 어떤 우선순위 큐에서 스레드를 갖어올지 판단 기준이 되는 변수를 증가
		current_priority_num = 0;		// ☞ 0~11 : HIGH, 12~17 : MID, 18~19 : LOW

	// 현재 쓰레드의 상태를 확인하여 THREAD_STATUS_TERMINATED(종료) 상태일 경우, 스레드 제거
	if( PsGetThreadPtr(current_thread)->thread_status == THREAD_STATUS_TERMINATED )
	{
		// Auto Delete?
		if( PsGetThreadPtr(current_thread)->auto_delete )
			PsDeleteThread(current_thread);
	}
	// THREAD_STATUS_RUNNING(실행중) 상태일 경우, 다음 스케줄링 될 때 재개될 수 있도록 THREAD_STATUS_READY(실행대기) 상태로 변경
	else if( PsGetThreadPtr(current_thread)->thread_status == THREAD_STATUS_RUNNING )
		PsGetThreadPtr(current_thread)->thread_status = THREAD_STATUS_READY;

	// 태스크 스위칭
	if( current_thread != next_thread && next_thread != NULL )
	{
		HalWriteTssIntoGdt(&PsGetThreadPtr(next_thread)->thread_tss32, sizeof(TSS_32), TASK_SW_SEG, TRUE);
		PsGetThreadPtr(next_thread)->thread_status = THREAD_STATUS_RUNNING;
	}
}

// ☞ 중간 우선순위 테스트용 스레드 함수
static DWORD PspMidPriorityThread(PVOID StartContext)
{
	while( TRUE )
		HalTaskSwitch();

	return 0;
}

// ☞ 낮은 우선순위 테스트용 스레드 함수
static DWORD PspLowPriorityThread(PVOID StartContext)
{
	while( TRUE )
		HalTaskSwitch();

	return 0;
}

// IDLE 쓰레드의 핸들러 함수
// 계속해서 태스크 스위칭(HalTaskSwitch)을 시도
static DWORD PspIdleThread(PVOID StartContext)
{
	while( TRUE )
		HalTaskSwitch();

	return 0;
}

// 종료된 프로세스의 삭제
static DWORD PspProcessCutterThread(PVOID StartContext)
{
	HANDLE ProcessHandle;
	PPROCESS_CONTROL_BLOCK	*pt_prev_process, *pt_cur_process;
	PTHREAD_CONTROL_BLOCK	*pt_cur_thread;

	while( 1 )
	{
		// 프로세스의 cutting 리스트 확인
		if( !PspPopCuttingItem(&m_ProcessCuttingList, &ProcessHandle) )
		{
			HalTaskSwitch();
			continue;
		}

ENTER_CRITICAL_SECTION();
		// 삭제할 프로세스가 시스템 프로세스 인지 확인
		if( ProcessHandle == PsGetThreadPtr(PsGetCurrentThread())->parent_process_handle )
		{
			goto $exit;
		}

		pt_prev_process = pt_cur_process = &(m_ProcMgrBlk.pt_head_process);

		while( *pt_cur_process != PsGetProcessPtr(ProcessHandle))
		{
			// 리스트 내에서 현재 프로세스가 마지막 리스트 프로세스일 경우에 종료
			if( (*pt_cur_process)->pt_next_process == NULL )
			{
				goto $exit;
			}
			pt_prev_process = pt_cur_process;
			pt_cur_process = &((*pt_cur_process)->pt_next_process);
		}

		// 다음 프로세스를 받아온다
		(*pt_prev_process)->pt_next_process = (*pt_cur_process)->pt_next_process;
		m_ProcMgrBlk.process_count--;

		// 삭제할 프로세스를 찾았다면, 해당 프로세스 내에 모든 쓰레드의 할당된 메모리 해제

		// ☞ 멀티레벨 스레드 큐 LOW 메모리 해제
		pt_cur_thread = &(PsGetProcessPtr(ProcessHandle)->pt_head_thread[LOW]);
		while( *pt_cur_thread != NULL )
		{
			MmFreeNonCachedMemory((PVOID)(*pt_cur_thread)->pt_stack_base_address);
			MmFreeNonCachedMemory((PVOID)(*pt_cur_thread));
			pt_cur_thread = &((*pt_cur_thread)->pt_next_thread);
		}
		// ☞ 멀티레벨 스레드 큐 MID 메모리 해제
		pt_cur_thread = &(PsGetProcessPtr(ProcessHandle)->pt_head_thread[MID]);
		while( *pt_cur_thread != NULL )
		{
			MmFreeNonCachedMemory((PVOID)(*pt_cur_thread)->pt_stack_base_address);
			MmFreeNonCachedMemory((PVOID)(*pt_cur_thread));
			pt_cur_thread = &((*pt_cur_thread)->pt_next_thread);
		}
		// ☞ 멀티레벨 스레드 큐 HIGH 메모리 해제
		pt_cur_thread = &(PsGetProcessPtr(ProcessHandle)->pt_head_thread[HIGH]);
		while( *pt_cur_thread != NULL )
		{
			MmFreeNonCachedMemory((PVOID)(*pt_cur_thread)->pt_stack_base_address);
			MmFreeNonCachedMemory((PVOID)(*pt_cur_thread));
			pt_cur_thread = &((*pt_cur_thread)->pt_next_thread);
		}

		// ☞ 프로세스가 삭제되면 1(제거됨)으로 설정, 시험용으로 만든 프로세스가 하나이기 때문에 가능...
		bIsUsrProcessDead = 1;
		// 삭제할 프로세스 자체에 할당된 메모리도 해제
		MmFreeNonCachedMemory((PVOID)ProcessHandle);

$exit:
EXIT_CRITICAL_SECTION();
	}

	return 0;
}


//종료된 쓰레드의 삭제
static DWORD PspThreadCutterThread(PVOID StartContext)
{
	HANDLE ProcessHandle, ThreadHandle;
	PTHREAD_CONTROL_BLOCK *pt_prev_thread, *pt_cur_thread;

	while( TRUE )
	{
		// Thread의 Cutting 리스트 확인
		if( !PspPopCuttingItem(&m_ThreadCuttingList, &ThreadHandle) )
		{
			HalTaskSwitch();
			continue;
		}

ENTER_CRITICAL_SECTION();

		ProcessHandle = PsGetThreadPtr(ThreadHandle)->parent_process_handle;

		// 삭제할 쓰레드가 속해있는 프로세스가 시스템 프로세스인지 확인
		if( ProcessHandle == PsGetThreadPtr(PsGetCurrentThread())->parent_process_handle )
			goto $exit;

		if( PsGetProcessPtr(ProcessHandle)->thread_count == 0 )
			goto $exit;
		// 속해있는 프로세스 내에서 한 개의 쓰레드만 존재할 경우
		else if( PsGetProcessPtr(ProcessHandle)->thread_count == 1 )
		{
			PsGetProcessPtr(ProcessHandle)->pt_head_thread[PsGetThreadPtr(ThreadHandle)->priority] = NULL;
			// ☞ 프로세스의 마지막 스레드가 삭제되면, 프로세스를 삭제시키기 위해 추가(프로세스가 삭제되는 것을 보이기 위해 추가)
			PsDeleteProcess(ProcessHandle);
		}
		else
		{
			pt_prev_thread = pt_cur_thread = &(PsGetProcessPtr(ProcessHandle)->pt_head_thread[PsGetThreadPtr(ThreadHandle)->priority]);	// ☞지울 스레드의 우선순위 큐를 사용
			
			while( *pt_cur_thread != PsGetThreadPtr(ThreadHandle) )
			{
				if( (*pt_cur_thread)->pt_next_thread == NULL )
					goto $exit;
				pt_prev_thread = pt_cur_thread;
				pt_cur_thread = &((*pt_cur_thread)->pt_next_thread);
			}

			// 리스트의 다음 쓰레드의 포인터를 가져온다
			(*pt_prev_thread)->pt_next_thread = (*pt_cur_thread)->pt_next_thread;
		}

		if( PsGetThreadPtr(ThreadHandle)->pt_stack_base_address >= (int *)0x00200000 )
			MmFreeNonCachedMemory((PVOID)(PsGetThreadPtr(ThreadHandle)->pt_stack_base_address)); // 스택 영역 할당 해제
		
		// ☞ 스레드가 삭제되면 3번 째 줄에 각 스레드의 이름을 "TERMINATED" 로 변경
		usr_thr_name[PsGetThreadPtr(ThreadHandle)->priority] = "TERMINATED";

		MmFreeNonCachedMemory((PVOID)(PsGetThreadPtr(ThreadHandle))); // 쓰레드 자체 할당 해제

		PsGetProcessPtr(ProcessHandle)->thread_count--;
$exit:
EXIT_CRITICAL_SECTION();
	}
}

// 소프트웨어 인터럽트 쓰레드의 핸들러 함수
static DWORD PspSoftTaskSW(PVOID StartContext)
{
	int cnt = 0, pos = 0;
	char *addr = (char *)TS_WATCHDOG_CLOCK_POS, status[] = {'-', '\\', '|', '/', '-', '\\', '|', '/'};
	
	// DbgPrint("PspSoftTaskSW START\r\n");

	while( TRUE )
	{
		_asm cli

		// 실행화면에 바람개비 모양이 회전하는 모습을 표시
		if( cnt++ >= TIMEOUT_PER_SECOND )
		{
			if( ++pos > 7 )
				pos = 0;
			cnt = 0;
			if( m_bShowTSWatchdogClock )
				*addr = status[pos];

			//DbgPrint("IT'S WINDY\r\n");
		}

		// 태스크 스위칭 함수 호출
		PspSetupTaskSWEnv();
		// 인터럽트 처리시에 모든 처리를 완료하고 다시 태스크로 복귀
		_asm iretd
	}

	return 0;
}


//타이머 인터럽트 핸들러 함수
static DWORD Psp_IRQ_SystemTimer(PVOID StartContxt)
{
	// DbgPrint("Psp_IRQ_SystemTimer START\r\n");

	while( TRUE )
	{
		_asm cli

		m_TickCount++;			// TickCount 값을 1씩 증가
		PspSetupTaskSWEnv();	// Task Switching
		WRITE_PORT_UCHAR((PUCHAR)0x20, 0x20);	// EOI 신호를 전송

		_asm iretd
	}

	return 0;
}

//초기 프로세스와 쓰레드의 생성과 설정
static BOOL PspCreateSystemProcess(VOID)
{
	HANDLE process_handle;
	HANDLE init_thread_handle, idle_thread_handle, process_cutter_handle, thread_cutter_handle, test_handle[10];
	HANDLE tmr_thread_handle, sw_task_sw_handle;

	//메인 프로세스를 생성해주는 PSCreateProcess 함수 호출
	if(!PsCreateProcess(&process_handle)) 
		return FALSE;

	//프로세스를 생성하기 위해 베이스가 될 메인 쓰레드(init 쓰레드) 생성 
	if(!PsCreateThread(&init_thread_handle, process_handle, NULL, NULL, DEFAULT_STACK_SIZE, FALSE, HIGH)) 
		return FALSE;
	// ☞ 메인 스레드에 이름 설정
	thread_name[PsGetThreadPtr(init_thread_handle)->thread_id] = "MAIN THREAD";

	//초기 쓰레드의 백링크(Prev-Link) 설정
	HalSetupTaskLink(&PsGetThreadPtr(init_thread_handle)->thread_tss32, TASK_SW_SEG);
	//초기 쓰레드의 TSS를 GDT내에 설정
	HalWriteTssIntoGdt(&PsGetThreadPtr(init_thread_handle)->thread_tss32, sizeof(TSS_32), INIT_TSS_SEG, FALSE);
	_asm 
	{
		push	ax
		mov		ax, INIT_TSS_SEG
		ltr		ax
		pop		ax
	}

	// 인터럽트 Thread 수행 시 Timer 처리를 위한 인터럽트 쓰레드 생성
	if( !PsCreateIntThread(&tmr_thread_handle, process_handle, Psp_IRQ_SystemTimer, NULL, DEFAULT_STACK_SIZE) )
		return FALSE;

	HalWriteTssIntoGdt(&PsGetThreadPtr(tmr_thread_handle)->thread_tss32, sizeof(TSS_32), TMR_TSS_SEG, FALSE);

	// 소프트웨어 인터럽트 처리를 위한 인터럽트 스레드 생성
	if( !PsCreateIntThread(&sw_task_sw_handle, process_handle, PspSoftTaskSW, NULL, DEFAULT_STACK_SIZE) )
		return FALSE;

	HalWriteTssIntoGdt(&PsGetThreadPtr(sw_task_sw_handle)->thread_tss32, sizeof(TSS_32), SOFT_TS_TSS_SEG, FALSE);

	// IDLE 스레드 생성
	if( !PsCreateThread(&idle_thread_handle, process_handle, PspIdleThread, NULL, DEFAULT_STACK_SIZE, FALSE, HIGH) )
		return FALSE;
	// ☞ IDLE 커터 스레드에 이름 설정
	thread_name[PsGetThreadPtr(idle_thread_handle)->thread_id] = "IDLE THREAD";
	PsSetThreadStatus(idle_thread_handle, THREAD_STATUS_RUNNING);

	HalWriteTssIntoGdt(&PsGetThreadPtr(idle_thread_handle)->thread_tss32, sizeof(TSS_32), TASK_SW_SEG, TRUE);

	m_ProcMgrBlk.pt_current_thread = idle_thread_handle;

	// 종료된 프로세스와 쓰레드를 삭제하는 cutter 스레드
	if( !PsCreateThread(&process_cutter_handle, process_handle, PspProcessCutterThread, NULL, DEFAULT_STACK_SIZE, FALSE, HIGH) )
		return FALSE;
	// ☞ 프로세스 커터 스레드에 이름 설정
	thread_name[PsGetThreadPtr(process_cutter_handle)->thread_id] = "PCUT THREAD";
	PsSetThreadStatus(process_cutter_handle, THREAD_STATUS_READY);
	
	if( !PsCreateThread(&thread_cutter_handle, process_handle, PspThreadCutterThread, NULL, DEFAULT_STACK_SIZE, FALSE, HIGH) )
		return FALSE;
	// ☞ 스레드 커터 스레드에 이름 설정
	thread_name[PsGetThreadPtr(thread_cutter_handle)->thread_id] = "TCUT THREAD";
	PsSetThreadStatus(thread_cutter_handle, THREAD_STATUS_READY);


	// ☞ 테스트 MID 시스템 스레드 1 생성
	if( !PsCreateThread(&test_handle[0], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[0])->thread_id] = "MID1 THREAD";
	PsSetThreadStatus(test_handle[0], THREAD_STATUS_READY);

	// ☞ 테스트 MID 시스템 스레드 2 생성
	if( !PsCreateThread(&test_handle[1], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[1])->thread_id] = "MID2 THREAD";
	PsSetThreadStatus(test_handle[1], THREAD_STATUS_READY);

	// ☞ 테스트 MID 시스템 스레드 3 생성
	if( !PsCreateThread(&test_handle[2], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[2])->thread_id] = "MID3 THREAD";
	PsSetThreadStatus(test_handle[2], THREAD_STATUS_READY);

	// ☞ 테스트 MID 시스템 스레드 4 생성
	if( !PsCreateThread(&test_handle[3], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[3])->thread_id] = "MID4 THREAD";
	PsSetThreadStatus(test_handle[3], THREAD_STATUS_READY);

	// ☞ 테스트 MID 시스템 스레드 5 생성
	if( !PsCreateThread(&test_handle[4], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[4])->thread_id] = "MID5 THREAD";
	PsSetThreadStatus(test_handle[4], THREAD_STATUS_READY);

	// ☞ 테스트 MID 시스템 스레드 6 생성
	if( !PsCreateThread(&test_handle[5], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[5])->thread_id] = "MID6 THREAD";
	PsSetThreadStatus(test_handle[5], THREAD_STATUS_READY);

	// ☞ 테스트 MID 시스템 스레드 7 생성
	if( !PsCreateThread(&test_handle[6], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[6])->thread_id] = "MID7 THREAD";
	PsSetThreadStatus(test_handle[6], THREAD_STATUS_READY);



	// ☞ 테스트 LOW 시스템 스레드 1 생성
	if( !PsCreateThread(&test_handle[7], process_handle, PspLowPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, LOW) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[7])->thread_id] = "LOW1 THREAD";
	PsSetThreadStatus(test_handle[7], THREAD_STATUS_READY);

	// ☞ 테스트 LOW 시스템 스레드 2 생성
	if( !PsCreateThread(&test_handle[8], process_handle, PspLowPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, LOW) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[8])->thread_id] = "LOW2 THREAD";
	PsSetThreadStatus(test_handle[8], THREAD_STATUS_READY);

	// ☞ 테스트 LOW 시스템 스레드 3 생성
	if( !PsCreateThread(&test_handle[9], process_handle, PspLowPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, LOW) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[9])->thread_id] = "LOW3 THREAD";
	PsSetThreadStatus(test_handle[9], THREAD_STATUS_READY);

	return TRUE;
}

static VOID PspTaskEntryPoint(VOID)
{
	PKSTART_ROUTINE start_routine;
	HANDLE current_thread;
	DWORD ret_value;

	// 현재 실행되고 있는 Thread의 TCB를 가지고 옴
	current_thread = PsGetCurrentThread();

	// TCB의 start_routine 콜백 함수에 start_context 포인터를 넘겨서 콜백 함수를 호출
	start_routine = PsGetThreadPtr(current_thread)->start_routine;
	ret_value = start_routine(PsGetThreadPtr(current_thread)->start_context);

	// 쓰레드의 상태를 THREAD_STATUS_TERMINATED 설정
	PsGetThreadPtr(current_thread)->thread_status = THREAD_STATUS_TERMINATED;

	// 태스크 스위칭 함수 호출
	HalTaskSwitch();

	while( TRUE );
}