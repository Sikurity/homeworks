/**
  *	@author 
  *		2011004040 �̿���
  *
  * @update 
  *		process.c, leeysos.h, vsprintf.c, 6845crt.c, stdarg.h, leeysos.c
  *			<process.c>		: �� ���μ�����, ������ ť(pt_head_thread)�� �����, TIME / SW ���ͷ�Ʈ �ڵ鷯 �Լ� PspSetupTaskSWEnv�� PspFindNextThreadScheduled���� 
  *							  0~19�� ���� ���� current_priority_num�� ���� �����Ͽ� ����� �켱������ ť���� ���� �����带 ���� �´�. �켱������ �ٲ� ��,
  *							  �ٲ�� ���� �켱�������� ���������� ����� �����带 pt_last_thread�� ������ �� ���߿� ����Ѵ�. �����ϰ� ������ �ʱ� ���Ͽ�
  *							  �ý��� ���μ����� �� �켱���� ������ ť���� IDLE �����带 �־� ���Ҵ�. current_priority_num ���� PspFindNextThreadScheduled
  *							  �Լ��� ������ 1�� �����ǰ�, 20�� �Ǹ� 0���� �ٲپ� �ش�.
  *			<leeysos.h>		: ������ �켱���� ������ Ÿ�� enum _THREAD_PRIORITY�� ��������
  *			<vsprintf.c>	: ������ ������� ���ϰ� �����ֱ� ���� �ܼ� ��� �Լ�(ffmt) �ణ ����
  *			<6845crt.c>		: ������ ������� ���ϰ� �����ֱ� ���� �ܼ� ��� �Լ�(CrtPrintfXY) �߰�
  *			<stdarg.h>		: ���� ���ڿ� ��·� ���ϴ� ��� ���� �ʾ� ����
  *			<leeysos.c>		: �׽�Ʈ�� ���� ���� ���μ���(����, �߰�, ���� �켱���� ������ �� 1��, �� 3�� ����)�� ������Ű�� �Լ�(PspCreateUserProcess) ����
  *
  *	@postscript	
  *		Ư������ �Ѹ� �˻��ϸ� ���� �ۼ��� �ּ��� ���� ã�� �� �ֽ��ϴ�.
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

	THREAD_PRIORITY						priority;					// �� ������ �켱���� �� �Ӽ� �߰�
} THREAD_CONTROL_BLOCK, *PTHREAD_CONTROL_BLOCK;

typedef struct _PROCESS_CONTROL_BLOCK {
	DWORD								process_id;					/* process id */
	HANDLE								process_handle;				/* memory address */

	struct _PROCESS_CONTROL_BLOCK		*pt_next_process;			/* next process point used by RR-Scheduler */

	DWORD								thread_count;				/* number of threads */
    DWORD								next_thread_id;				/* next thread id used in this process */

	struct _THREAD_CONTROL_BLOCK		*pt_head_thread[3];			/* first thread point �� �� ���μ����� ��Ƽ���� ť�� ���� �ּ� ������ */
} PROCESS_CONTROL_BLOCK, *PPROCESS_CONTROL_BLOCK;


//Process ����Ʈ�� �����ϴ� ����ü
typedef struct _PROCESS_MANAGER_BLOCK 
{
	DWORD								process_count;				/* number of processes */
	DWORD								next_process_id;			/* next prodess id */
	struct _THREAD_CONTROL_BLOCK		*pt_current_thread;			/* running thread */
	struct _THREAD_CONTROL_BLOCK		*pt_last_thread[3];			// �� �� �켱���� ť�� ���������� ����� ������
	struct _PROCESS_CONTROL_BLOCK		*pt_head_process;			/* first process point */
} PROCESS_MANAGER_BLOCK, *PPROCESS_MANAGER_BLOCK;


//�ý��ۿ��� �����ؾ��ϴ� Process�� Thread�� ����Ʈ�� ���� ������ �����ϴ� ����ü
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

// �� ������Ѿ��� �켱���� ť�� �������� ��ȯ���ִ� �Լ�
static THREAD_PRIORITY GetNextRunPriority();

static PROCESS_MANAGER_BLOCK m_ProcMgrBlk;

static CUTTING_LIST m_ProcessCuttingList;
static CUTTING_LIST m_ThreadCuttingList;

static BOOL m_bShowTSWatchdogClock;			// �ٶ����� ���
static DWORD m_TickCount;					// �ý����� ƽ ���� �����ϴ� ����

extern SEGMENT_DESC *m_GdtTable;

static int current_priority_num;	// �� ���� OS ���� ���� ������ �켱���� ����
char	*thread_name[64];			// �� ��� ��������� �̸�, ��������� ������ ������ Ȯ���ϱ� ���� �߰�
char	*usr_thr_name[3];			// �� �׽�Ʈ�� ���� ���μ����� �� ��������� �̸��� �����ϱ� ���� ����
int		gTestNum[3];				// �� �׽�Ʈ�� ���� ���μ����� �� ��������� ���� Ƚ���� �����ϱ� ���� ����
int		bIsUsrProcessDead;			// �� �׽�Ʈ�� ���� ���μ����� ���� �Ǿ������� �˱� ���� ����

// �� ������Ѿ��� �켱���� ť�� �������� ��ȯ���ִ� �Լ�
static THREAD_PRIORITY GetNextRunPriority()
{
	return current_priority_num < 12 ? HIGH : (current_priority_num < 18 ? MID : LOW);
}

// �� ���� �켱���� �׽�Ʈ�� ���� ������ �Լ�, 0xff�� �ٴ����� ������ ����
static DWORD PspHighPriorityUserThread(PVOID StartContext)
{
	gTestNum[HIGH] = 0;

	while( ++gTestNum[HIGH] != 0xff )
		HalTaskSwitch();

	return 0;
}

// �� �߰� �켱���� �׽�Ʈ�� ���� ������ �Լ�, 0xff�� �ٴ����� ������ ����
static DWORD PspMidPriorityUserThread(PVOID StartContext)
{
	gTestNum[MID] = 0;

	while( ++gTestNum[MID] != 0xff )
		HalTaskSwitch();

	return 0;
}

// �� ���� �켱���� �׽�Ʈ�� ���� ������ �Լ�, 0xff�� �ٴ����� ������ ����
static DWORD PspLowPriorityUserThread(PVOID StartContext)
{
	gTestNum[LOW] = 0;

	while( ++gTestNum[LOW] != 0xff )
		HalTaskSwitch();

	return 0;
}

// �� ����� ���� ���μ���, leeysos.c���� ��� ��
KERNELAPI BOOL PspCreateUserProcess(VOID)
{
	HANDLE process_handle;
	HANDLE test_handle[3];

	//���� ���μ����� �������ִ� PSCreateProcess �Լ� ȣ��
	if(!PsCreateProcess(&process_handle)) 
		return FALSE;

	// �� �׽�Ʈ HIGH �ý��� ������ ����
	if( !PsCreateThread(&test_handle[0], process_handle, PspHighPriorityUserThread, NULL, DEFAULT_STACK_SIZE, TRUE, HIGH) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[0])->thread_id] = "USER_T_HIGH";
	PsSetThreadStatus(test_handle[0], THREAD_STATUS_READY);

	// �� �׽�Ʈ MID �ý��� ������ ����
	if( !PsCreateThread(&test_handle[1], process_handle, PspMidPriorityUserThread, NULL, DEFAULT_STACK_SIZE, TRUE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[1])->thread_id] = "USER_T_MID_";
	PsSetThreadStatus(test_handle[1], THREAD_STATUS_READY);

	// �� �׽�Ʈ LOW �ý��� ������ ����
	if( !PsCreateThread(&test_handle[2], process_handle, PspLowPriorityUserThread, NULL, DEFAULT_STACK_SIZE, TRUE, LOW) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[2])->thread_id] = "USER_T_LOW_";
	PsSetThreadStatus(test_handle[2], THREAD_STATUS_READY);

	return TRUE;
}

// ��� ���� �ʱ�ȭ �Լ�
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

	current_priority_num	= 0;		// �� ������ ���� �켱������ ������ �Ǵ� ���� 0���� �ʱ�ȭ
	bIsUsrProcessDead		= 0;		// �� ���� ���μ����� ������ �����̹Ƿ�, 0(�������)���� �ʱ�ȭ

	usr_thr_name[LOW]	= "USR_LOW__T"; // �� �׽�Ʈ�� ���� ���μ����� ���� �켱���� ������ �̸� ����
	usr_thr_name[MID]	= "USR_MID__T"; // �� �׽�Ʈ�� ���� ���μ����� �߰� �켱���� ������ �̸� ����
	usr_thr_name[HIGH]	= "USR_HIGH_T";	// �� �׽�Ʈ�� ���� ���μ����� ���� �켱���� ������ �̸� ����

	gTestNum[LOW]	= 0;				// �� �׽�Ʈ�� ���� ���μ����� ���� �켱���� ������ ����Ƚ�� 0���� �ʱ�ȭ
	gTestNum[MID]	= 0;				// �� �׽�Ʈ�� ���� ���μ����� �߰� �켱���� ������ ����Ƚ�� 0���� �ʱ�ȭ
	gTestNum[HIGH]	= 0;				// �� �׽�Ʈ�� ���� ���μ����� ���� �켱���� ������ ����Ƚ�� 0���� �ʱ�ȭ

	if(!PspCreateSystemProcess()) 
	{
		DbgPrint("PspCreateSystemProcess() returned an error.\r\n");
		return FALSE;
	}

	return TRUE;
}

//���μ��� ���� �Լ� (PID�Ҵ� �� ����)
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


// ������ ���� �Լ� �� �������� �켱���� ���� ���� �� �ֵ���, �Լ� ���� IN THREAD_PRIORITY Priority �߰�
KERNELAPI BOOL PsCreateThread(OUT PHANDLE ThreadHandle, IN HANDLE ProcessHandle, IN PKSTART_ROUTINE StartRoutine, 
					 IN PVOID StartContext, IN DWORD StackSize, IN BOOL AutoDelete, IN THREAD_PRIORITY Priority)
{
	PTHREAD_CONTROL_BLOCK pThread;
	int *pStack;
	
	//�޸��Ҵ�
	pThread = MmAllocateNonCachedMemory(sizeof(THREAD_CONTROL_BLOCK));
	if(pThread == NULL) return FALSE;
	//�����忡�� ����� ���� �Ҵ�
	pStack  = MmAllocateNonCachedMemory(StackSize);
	if(pStack == NULL) return FALSE;

	//�θ� ���μ����� �ڵ� ����
	pThread->parent_process_handle		= ProcessHandle;
	//Thread id �� handle �Ҵ�
	pThread->thread_id					= PspGetNextThreadID(ProcessHandle);
	pThread->thread_handle				= (HANDLE)pThread;
	pThread->thread_status				= THREAD_STATUS_STOP;		// Thread ���¸� STOP���� ����
	pThread->auto_delete				= AutoDelete; 
	pThread->pt_next_thread				= NULL;

	//�����尡 �����ؾ� �ϴ� �Լ�(StartRoutine), �Լ��� �Ѿ�� ����(StartContext), ���� ������ ����
	pThread->start_routine				= StartRoutine;
	pThread->start_context				= StartContext;
	pThread->pt_stack_base_address		= pStack;
	pThread->stack_size					= StackSize;
	pThread->priority					= Priority;					// �� �������� �켱���� �� ����

	//PspAddNewThread �Լ��� ���� Process�� ������ �����带 �߰�
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
	//PsCreateThread�Լ��� �ٸ��� auto_delete�� false
	pThread->auto_delete				= FALSE;
	pThread->pt_next_thread				= NULL;

	//PsCreateThread�Լ��� �ٸ��� argument�� StartRoutine�� �ٷ� �Ҵ�
	pThread->start_routine				= StartRoutine;
	pThread->start_context				= StartContext;
	pThread->pt_stack_base_address		= pStack;
	pThread->stack_size					= StackSize;
	pThread->priority					= HIGH;							// �� ���ͷ�Ʈ �������� �켱������ ���� ����
	if( !PspAddNewThread(ProcessHandle, (HANDLE)pThread) ) 
		return FALSE;

	HalSetupTSS(&pThread->thread_tss32, TRUE, (int)StartRoutine, pStack, StackSize);

	*ThreadHandle = pThread;

	return TRUE;
}
//�������� ���¸� �����ϴ� �Լ�
KERNELAPI BOOL PsSetThreadStatus(HANDLE ThreadHandle, THREAD_STATUS Status)
{
	PsGetThreadPtr(ThreadHandle)->thread_status = Status;

	return TRUE;
}

//���� ���μ������� ����ǰ� �ִ� �������� TCB�� ��ȯ�ϴ� �Լ�
KERNELAPI HANDLE PsGetCurrentThread(VOID)
{
	HANDLE thread;

ENTER_CRITICAL_SECTION();

	// m_ProcMgrBlk�� pt_current_thread�� ��ȯ
	thread = (HANDLE)(m_ProcMgrBlk.pt_current_thread);

EXIT_CRITICAL_SECTION();

	return thread;
}

// �� �̻� �ʿ����� ���� ���μ����� �����ϱ����� �Լ�
// �� ���μ����� ������Ű�� ���� �߰�(���μ����� �����Ǵ� ���� ���̱� ���� �߰�)
KERNELAPI BOOL PsDeleteProcess(IN HANDLE ProcessHandle)
{
	return PspPushCuttingItem(&m_ProcessCuttingList, ProcessHandle);
}

// �� �̻� �ʿ����� ���� �����带 �����ϱ����� �Լ�
KERNELAPI BOOL PsDeleteThread(IN HANDLE ThreadHandle)
{
	return PspPushCuttingItem(&m_ThreadCuttingList, ThreadHandle);
}

static BOOL PspPopCuttingItem(CUTTING_LIST *pCuttingList, HANDLE *pItem)
{
	BOOL bResult = TRUE;

ENTER_CRITICAL_SECTION();

	{
		// Ŀ�� ť�� ī��Ʈ üũ
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
		// Ŀ�� ť�� ���� ���� üũ
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


//Process ID ����
static DWORD PspGetNextProcessID(VOID)
{
	DWORD process_id;

ENTER_CRITICAL_SECTION();
	process_id = m_ProcMgrBlk.next_process_id++;
EXIT_CRITICAL_SECTION();

	return process_id;
}

//Thread ID ����
static DWORD PspGetNextThreadID(HANDLE ProcessHandle)
{
	DWORD thread_id;

ENTER_CRITICAL_SECTION();
	thread_id = PsGetProcessPtr(ProcessHandle)->next_thread_id++;
EXIT_CRITICAL_SECTION();

	return thread_id;
}

//���ο� ���μ����� ���� ������ ã�Ƽ� ����
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

//���ο� �����带 ���� ������ ã�Ƽ� ����
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

// ���� ���� ������ �����带 ã�� ���� �Լ�
static HANDLE PspFindNextThreadScheduled(VOID)
{
	int i;

	THREAD_PRIORITY			nextPriority;
	PTHREAD_CONTROL_BLOCK	pt_thread, pt_tmp_thread;
	PPROCESS_CONTROL_BLOCK	pt_process;

	if( m_ProcMgrBlk.process_count == 0 || m_ProcMgrBlk.pt_current_thread == NULL || m_ProcMgrBlk.pt_head_process == NULL )
		return NULL;

	nextPriority	= GetNextRunPriority();				// �� ���� ����� ��������� ���� �켱���� ��
	pt_thread		= m_ProcMgrBlk.pt_current_thread;
	
	if( pt_thread->priority != nextPriority)			// �� ���� �����尡 ���� ����� �켱������ ���ϴ��� Ȯ��
	{
		// �� ���� �����尡 ���� ����� �켱������ ������ �ʴ´ٸ�, m_ProcMgrBlk.pt_last_thread�� ���� �����带 �������ְ�,
		m_ProcMgrBlk.pt_last_thread[pt_thread->priority] = pt_thread;
		// �� ���� ����� �켱������ ������ �� �������� ����� �����带 �����´�.
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

	// �� ���� ���μ����� �����Ǿ����� ǥ��
	if( bIsUsrProcessDead )
		CrtPrintfXY(0, 3, "All Threads Are Terminated, The User Process Including Them is Removed\r\n");
	// �� ���� ���μ����� ������� ������ (����, �߰�, ���� �켱���� 3��)��������� ����� Ƚ�� ��, ������ ���� ���� ǥ��
	else
		CrtPrintfXY(0, 3, "%s : %00x, %s : %00x, %s : %00x\r\n", usr_thr_name[LOW], gTestNum[LOW], usr_thr_name[MID], gTestNum[MID], usr_thr_name[HIGH], gTestNum[HIGH]);
	
	// �� �� Current_priority_num �� ��, ����� ������ �̸��� ���
	CrtPrintfXY(0, current_priority_num + 4, "Currnt_Priority_Num : %00d, Thread Name : %s\r\n", current_priority_num, thread_name[pt_thread->thread_id]);	
	
	// �� �ʹ� �����Ƿ�, ������ ������ ������
	for( i = 0 ; i < 500000 ; i++ );

	return (HANDLE)pt_thread;
}

// �½�ũ ����Ī ��� �� �ϳ�
static VOID PspSetupTaskSWEnv(VOID)
{
	HANDLE current_thread, next_thread;

	// ���� ����ǰ� �ִ� �����带 ������
	current_thread = PsGetCurrentThread();

	// ���� ���� ������ �����带 ã�´�
	next_thread = PspFindNextThreadScheduled(); // At this time, current thread is changed with new thing

	if( ++current_priority_num >= 20 )	// �� � �켱���� ť���� �����带 ������� �Ǵ� ������ �Ǵ� ������ ����
		current_priority_num = 0;		// �� 0~11 : HIGH, 12~17 : MID, 18~19 : LOW

	// ���� �������� ���¸� Ȯ���Ͽ� THREAD_STATUS_TERMINATED(����) ������ ���, ������ ����
	if( PsGetThreadPtr(current_thread)->thread_status == THREAD_STATUS_TERMINATED )
	{
		// Auto Delete?
		if( PsGetThreadPtr(current_thread)->auto_delete )
			PsDeleteThread(current_thread);
	}
	// THREAD_STATUS_RUNNING(������) ������ ���, ���� �����ٸ� �� �� �簳�� �� �ֵ��� THREAD_STATUS_READY(������) ���·� ����
	else if( PsGetThreadPtr(current_thread)->thread_status == THREAD_STATUS_RUNNING )
		PsGetThreadPtr(current_thread)->thread_status = THREAD_STATUS_READY;

	// �½�ũ ����Ī
	if( current_thread != next_thread && next_thread != NULL )
	{
		HalWriteTssIntoGdt(&PsGetThreadPtr(next_thread)->thread_tss32, sizeof(TSS_32), TASK_SW_SEG, TRUE);
		PsGetThreadPtr(next_thread)->thread_status = THREAD_STATUS_RUNNING;
	}
}

// �� �߰� �켱���� �׽�Ʈ�� ������ �Լ�
static DWORD PspMidPriorityThread(PVOID StartContext)
{
	while( TRUE )
		HalTaskSwitch();

	return 0;
}

// �� ���� �켱���� �׽�Ʈ�� ������ �Լ�
static DWORD PspLowPriorityThread(PVOID StartContext)
{
	while( TRUE )
		HalTaskSwitch();

	return 0;
}

// IDLE �������� �ڵ鷯 �Լ�
// ����ؼ� �½�ũ ����Ī(HalTaskSwitch)�� �õ�
static DWORD PspIdleThread(PVOID StartContext)
{
	while( TRUE )
		HalTaskSwitch();

	return 0;
}

// ����� ���μ����� ����
static DWORD PspProcessCutterThread(PVOID StartContext)
{
	HANDLE ProcessHandle;
	PPROCESS_CONTROL_BLOCK	*pt_prev_process, *pt_cur_process;
	PTHREAD_CONTROL_BLOCK	*pt_cur_thread;

	while( 1 )
	{
		// ���μ����� cutting ����Ʈ Ȯ��
		if( !PspPopCuttingItem(&m_ProcessCuttingList, &ProcessHandle) )
		{
			HalTaskSwitch();
			continue;
		}

ENTER_CRITICAL_SECTION();
		// ������ ���μ����� �ý��� ���μ��� ���� Ȯ��
		if( ProcessHandle == PsGetThreadPtr(PsGetCurrentThread())->parent_process_handle )
		{
			goto $exit;
		}

		pt_prev_process = pt_cur_process = &(m_ProcMgrBlk.pt_head_process);

		while( *pt_cur_process != PsGetProcessPtr(ProcessHandle))
		{
			// ����Ʈ ������ ���� ���μ����� ������ ����Ʈ ���μ����� ��쿡 ����
			if( (*pt_cur_process)->pt_next_process == NULL )
			{
				goto $exit;
			}
			pt_prev_process = pt_cur_process;
			pt_cur_process = &((*pt_cur_process)->pt_next_process);
		}

		// ���� ���μ����� �޾ƿ´�
		(*pt_prev_process)->pt_next_process = (*pt_cur_process)->pt_next_process;
		m_ProcMgrBlk.process_count--;

		// ������ ���μ����� ã�Ҵٸ�, �ش� ���μ��� ���� ��� �������� �Ҵ�� �޸� ����

		// �� ��Ƽ���� ������ ť LOW �޸� ����
		pt_cur_thread = &(PsGetProcessPtr(ProcessHandle)->pt_head_thread[LOW]);
		while( *pt_cur_thread != NULL )
		{
			MmFreeNonCachedMemory((PVOID)(*pt_cur_thread)->pt_stack_base_address);
			MmFreeNonCachedMemory((PVOID)(*pt_cur_thread));
			pt_cur_thread = &((*pt_cur_thread)->pt_next_thread);
		}
		// �� ��Ƽ���� ������ ť MID �޸� ����
		pt_cur_thread = &(PsGetProcessPtr(ProcessHandle)->pt_head_thread[MID]);
		while( *pt_cur_thread != NULL )
		{
			MmFreeNonCachedMemory((PVOID)(*pt_cur_thread)->pt_stack_base_address);
			MmFreeNonCachedMemory((PVOID)(*pt_cur_thread));
			pt_cur_thread = &((*pt_cur_thread)->pt_next_thread);
		}
		// �� ��Ƽ���� ������ ť HIGH �޸� ����
		pt_cur_thread = &(PsGetProcessPtr(ProcessHandle)->pt_head_thread[HIGH]);
		while( *pt_cur_thread != NULL )
		{
			MmFreeNonCachedMemory((PVOID)(*pt_cur_thread)->pt_stack_base_address);
			MmFreeNonCachedMemory((PVOID)(*pt_cur_thread));
			pt_cur_thread = &((*pt_cur_thread)->pt_next_thread);
		}

		// �� ���μ����� �����Ǹ� 1(���ŵ�)���� ����, ��������� ���� ���μ����� �ϳ��̱� ������ ����...
		bIsUsrProcessDead = 1;
		// ������ ���μ��� ��ü�� �Ҵ�� �޸𸮵� ����
		MmFreeNonCachedMemory((PVOID)ProcessHandle);

$exit:
EXIT_CRITICAL_SECTION();
	}

	return 0;
}


//����� �������� ����
static DWORD PspThreadCutterThread(PVOID StartContext)
{
	HANDLE ProcessHandle, ThreadHandle;
	PTHREAD_CONTROL_BLOCK *pt_prev_thread, *pt_cur_thread;

	while( TRUE )
	{
		// Thread�� Cutting ����Ʈ Ȯ��
		if( !PspPopCuttingItem(&m_ThreadCuttingList, &ThreadHandle) )
		{
			HalTaskSwitch();
			continue;
		}

ENTER_CRITICAL_SECTION();

		ProcessHandle = PsGetThreadPtr(ThreadHandle)->parent_process_handle;

		// ������ �����尡 �����ִ� ���μ����� �ý��� ���μ������� Ȯ��
		if( ProcessHandle == PsGetThreadPtr(PsGetCurrentThread())->parent_process_handle )
			goto $exit;

		if( PsGetProcessPtr(ProcessHandle)->thread_count == 0 )
			goto $exit;
		// �����ִ� ���μ��� ������ �� ���� �����常 ������ ���
		else if( PsGetProcessPtr(ProcessHandle)->thread_count == 1 )
		{
			PsGetProcessPtr(ProcessHandle)->pt_head_thread[PsGetThreadPtr(ThreadHandle)->priority] = NULL;
			// �� ���μ����� ������ �����尡 �����Ǹ�, ���μ����� ������Ű�� ���� �߰�(���μ����� �����Ǵ� ���� ���̱� ���� �߰�)
			PsDeleteProcess(ProcessHandle);
		}
		else
		{
			pt_prev_thread = pt_cur_thread = &(PsGetProcessPtr(ProcessHandle)->pt_head_thread[PsGetThreadPtr(ThreadHandle)->priority]);	// ������ �������� �켱���� ť�� ���
			
			while( *pt_cur_thread != PsGetThreadPtr(ThreadHandle) )
			{
				if( (*pt_cur_thread)->pt_next_thread == NULL )
					goto $exit;
				pt_prev_thread = pt_cur_thread;
				pt_cur_thread = &((*pt_cur_thread)->pt_next_thread);
			}

			// ����Ʈ�� ���� �������� �����͸� �����´�
			(*pt_prev_thread)->pt_next_thread = (*pt_cur_thread)->pt_next_thread;
		}

		if( PsGetThreadPtr(ThreadHandle)->pt_stack_base_address >= (int *)0x00200000 )
			MmFreeNonCachedMemory((PVOID)(PsGetThreadPtr(ThreadHandle)->pt_stack_base_address)); // ���� ���� �Ҵ� ����
		
		// �� �����尡 �����Ǹ� 3�� ° �ٿ� �� �������� �̸��� "TERMINATED" �� ����
		usr_thr_name[PsGetThreadPtr(ThreadHandle)->priority] = "TERMINATED";

		MmFreeNonCachedMemory((PVOID)(PsGetThreadPtr(ThreadHandle))); // ������ ��ü �Ҵ� ����

		PsGetProcessPtr(ProcessHandle)->thread_count--;
$exit:
EXIT_CRITICAL_SECTION();
	}
}

// ����Ʈ���� ���ͷ�Ʈ �������� �ڵ鷯 �Լ�
static DWORD PspSoftTaskSW(PVOID StartContext)
{
	int cnt = 0, pos = 0;
	char *addr = (char *)TS_WATCHDOG_CLOCK_POS, status[] = {'-', '\\', '|', '/', '-', '\\', '|', '/'};
	
	// DbgPrint("PspSoftTaskSW START\r\n");

	while( TRUE )
	{
		_asm cli

		// ����ȭ�鿡 �ٶ����� ����� ȸ���ϴ� ����� ǥ��
		if( cnt++ >= TIMEOUT_PER_SECOND )
		{
			if( ++pos > 7 )
				pos = 0;
			cnt = 0;
			if( m_bShowTSWatchdogClock )
				*addr = status[pos];

			//DbgPrint("IT'S WINDY\r\n");
		}

		// �½�ũ ����Ī �Լ� ȣ��
		PspSetupTaskSWEnv();
		// ���ͷ�Ʈ ó���ÿ� ��� ó���� �Ϸ��ϰ� �ٽ� �½�ũ�� ����
		_asm iretd
	}

	return 0;
}


//Ÿ�̸� ���ͷ�Ʈ �ڵ鷯 �Լ�
static DWORD Psp_IRQ_SystemTimer(PVOID StartContxt)
{
	// DbgPrint("Psp_IRQ_SystemTimer START\r\n");

	while( TRUE )
	{
		_asm cli

		m_TickCount++;			// TickCount ���� 1�� ����
		PspSetupTaskSWEnv();	// Task Switching
		WRITE_PORT_UCHAR((PUCHAR)0x20, 0x20);	// EOI ��ȣ�� ����

		_asm iretd
	}

	return 0;
}

//�ʱ� ���μ����� �������� ������ ����
static BOOL PspCreateSystemProcess(VOID)
{
	HANDLE process_handle;
	HANDLE init_thread_handle, idle_thread_handle, process_cutter_handle, thread_cutter_handle, test_handle[10];
	HANDLE tmr_thread_handle, sw_task_sw_handle;

	//���� ���μ����� �������ִ� PSCreateProcess �Լ� ȣ��
	if(!PsCreateProcess(&process_handle)) 
		return FALSE;

	//���μ����� �����ϱ� ���� ���̽��� �� ���� ������(init ������) ���� 
	if(!PsCreateThread(&init_thread_handle, process_handle, NULL, NULL, DEFAULT_STACK_SIZE, FALSE, HIGH)) 
		return FALSE;
	// �� ���� �����忡 �̸� ����
	thread_name[PsGetThreadPtr(init_thread_handle)->thread_id] = "MAIN THREAD";

	//�ʱ� �������� �鸵ũ(Prev-Link) ����
	HalSetupTaskLink(&PsGetThreadPtr(init_thread_handle)->thread_tss32, TASK_SW_SEG);
	//�ʱ� �������� TSS�� GDT���� ����
	HalWriteTssIntoGdt(&PsGetThreadPtr(init_thread_handle)->thread_tss32, sizeof(TSS_32), INIT_TSS_SEG, FALSE);
	_asm 
	{
		push	ax
		mov		ax, INIT_TSS_SEG
		ltr		ax
		pop		ax
	}

	// ���ͷ�Ʈ Thread ���� �� Timer ó���� ���� ���ͷ�Ʈ ������ ����
	if( !PsCreateIntThread(&tmr_thread_handle, process_handle, Psp_IRQ_SystemTimer, NULL, DEFAULT_STACK_SIZE) )
		return FALSE;

	HalWriteTssIntoGdt(&PsGetThreadPtr(tmr_thread_handle)->thread_tss32, sizeof(TSS_32), TMR_TSS_SEG, FALSE);

	// ����Ʈ���� ���ͷ�Ʈ ó���� ���� ���ͷ�Ʈ ������ ����
	if( !PsCreateIntThread(&sw_task_sw_handle, process_handle, PspSoftTaskSW, NULL, DEFAULT_STACK_SIZE) )
		return FALSE;

	HalWriteTssIntoGdt(&PsGetThreadPtr(sw_task_sw_handle)->thread_tss32, sizeof(TSS_32), SOFT_TS_TSS_SEG, FALSE);

	// IDLE ������ ����
	if( !PsCreateThread(&idle_thread_handle, process_handle, PspIdleThread, NULL, DEFAULT_STACK_SIZE, FALSE, HIGH) )
		return FALSE;
	// �� IDLE Ŀ�� �����忡 �̸� ����
	thread_name[PsGetThreadPtr(idle_thread_handle)->thread_id] = "IDLE THREAD";
	PsSetThreadStatus(idle_thread_handle, THREAD_STATUS_RUNNING);

	HalWriteTssIntoGdt(&PsGetThreadPtr(idle_thread_handle)->thread_tss32, sizeof(TSS_32), TASK_SW_SEG, TRUE);

	m_ProcMgrBlk.pt_current_thread = idle_thread_handle;

	// ����� ���μ����� �����带 �����ϴ� cutter ������
	if( !PsCreateThread(&process_cutter_handle, process_handle, PspProcessCutterThread, NULL, DEFAULT_STACK_SIZE, FALSE, HIGH) )
		return FALSE;
	// �� ���μ��� Ŀ�� �����忡 �̸� ����
	thread_name[PsGetThreadPtr(process_cutter_handle)->thread_id] = "PCUT THREAD";
	PsSetThreadStatus(process_cutter_handle, THREAD_STATUS_READY);
	
	if( !PsCreateThread(&thread_cutter_handle, process_handle, PspThreadCutterThread, NULL, DEFAULT_STACK_SIZE, FALSE, HIGH) )
		return FALSE;
	// �� ������ Ŀ�� �����忡 �̸� ����
	thread_name[PsGetThreadPtr(thread_cutter_handle)->thread_id] = "TCUT THREAD";
	PsSetThreadStatus(thread_cutter_handle, THREAD_STATUS_READY);


	// �� �׽�Ʈ MID �ý��� ������ 1 ����
	if( !PsCreateThread(&test_handle[0], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[0])->thread_id] = "MID1 THREAD";
	PsSetThreadStatus(test_handle[0], THREAD_STATUS_READY);

	// �� �׽�Ʈ MID �ý��� ������ 2 ����
	if( !PsCreateThread(&test_handle[1], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[1])->thread_id] = "MID2 THREAD";
	PsSetThreadStatus(test_handle[1], THREAD_STATUS_READY);

	// �� �׽�Ʈ MID �ý��� ������ 3 ����
	if( !PsCreateThread(&test_handle[2], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[2])->thread_id] = "MID3 THREAD";
	PsSetThreadStatus(test_handle[2], THREAD_STATUS_READY);

	// �� �׽�Ʈ MID �ý��� ������ 4 ����
	if( !PsCreateThread(&test_handle[3], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[3])->thread_id] = "MID4 THREAD";
	PsSetThreadStatus(test_handle[3], THREAD_STATUS_READY);

	// �� �׽�Ʈ MID �ý��� ������ 5 ����
	if( !PsCreateThread(&test_handle[4], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[4])->thread_id] = "MID5 THREAD";
	PsSetThreadStatus(test_handle[4], THREAD_STATUS_READY);

	// �� �׽�Ʈ MID �ý��� ������ 6 ����
	if( !PsCreateThread(&test_handle[5], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[5])->thread_id] = "MID6 THREAD";
	PsSetThreadStatus(test_handle[5], THREAD_STATUS_READY);

	// �� �׽�Ʈ MID �ý��� ������ 7 ����
	if( !PsCreateThread(&test_handle[6], process_handle, PspMidPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, MID) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[6])->thread_id] = "MID7 THREAD";
	PsSetThreadStatus(test_handle[6], THREAD_STATUS_READY);



	// �� �׽�Ʈ LOW �ý��� ������ 1 ����
	if( !PsCreateThread(&test_handle[7], process_handle, PspLowPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, LOW) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[7])->thread_id] = "LOW1 THREAD";
	PsSetThreadStatus(test_handle[7], THREAD_STATUS_READY);

	// �� �׽�Ʈ LOW �ý��� ������ 2 ����
	if( !PsCreateThread(&test_handle[8], process_handle, PspLowPriorityThread, NULL, DEFAULT_STACK_SIZE, FALSE, LOW) )
		return FALSE;
	thread_name[PsGetThreadPtr(test_handle[8])->thread_id] = "LOW2 THREAD";
	PsSetThreadStatus(test_handle[8], THREAD_STATUS_READY);

	// �� �׽�Ʈ LOW �ý��� ������ 3 ����
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

	// ���� ����ǰ� �ִ� Thread�� TCB�� ������ ��
	current_thread = PsGetCurrentThread();

	// TCB�� start_routine �ݹ� �Լ��� start_context �����͸� �Ѱܼ� �ݹ� �Լ��� ȣ��
	start_routine = PsGetThreadPtr(current_thread)->start_routine;
	ret_value = start_routine(PsGetThreadPtr(current_thread)->start_context);

	// �������� ���¸� THREAD_STATUS_TERMINATED ����
	PsGetThreadPtr(current_thread)->thread_status = THREAD_STATUS_TERMINATED;

	// �½�ũ ����Ī �Լ� ȣ��
	HalTaskSwitch();

	while( TRUE );
}