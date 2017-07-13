#include "fdddrv.h"

#define DEFAULT_STACK_SIZE				(64*1024) /* 64kbytes */

//플로피 디스크에서 사용되는 포트
#define FDD_DOR_PORT		0x3f2
#define FDD_STATUS_PORT		0x3f4
#define FDD_CMD_PORT		0x3f4
#define FDD_DATA_PORT		0x3f5

typedef enum _FDD_JOB_TYPE {
	FDD_READ_SECTOR,
	FDD_WRITE_SECTOR,
} FDD_JOB_TYPE;

//플로피 디스크의 작업을 저장하기 위한 큐 구조체
typedef struct _FDD_JOB_ITEM {
		
	FDD_JOB_TYPE	type;				//_FDD_JOB_TYPE : 수행해야할 작업의 종류
	WORD			sector;				// 읽거나 써야 할 섹터의 번호
	BYTE			numbers_of_sectors;	// 읽거나 써야 할 섹터의 수
	BYTE			*pt_data;			// 실제 읽거나 써야할 데이터

	HANDLE			thread;				// 이 작업을 수행하게 되는 쓰레드의 핸들
} FDD_JOB_ITEM, *PFDD_JOB_ITEM;

#define FDD_JOB_ITEM_Q_SIZE		32

//큐 관리하는 구조체
typedef struct _FDD_JOB_ITEM_Q {
	BYTE			cnt;
	BYTE			head;
	BYTE			tail;
	FDD_JOB_ITEM	queue[FDD_JOB_ITEM_Q_SIZE];
} FDD_JOB_ITEM_Q, *PFDD_JOB_ITEM_Q;


static BOOL  FddpPopJobItem(FDD_JOB_ITEM *pJobItem);
static BOOL  FddpPushJobItem(FDD_JOB_ITEM *pJobItem);
static DWORD FddpJobProcessThread(PVOID StartContext);

static BOOL FddpReadWriteSector(FDD_JOB_TYPE JobType, WORD Sector, BYTE NumbersOfSectors, BYTE *pData);

static BOOL FddpTurnOnMotor(VOID);
static BOOL FddpTurnOffMotor(VOID);
static BOOL FddpSetupDMA(FDD_JOB_TYPE JobType);
static BOOL FddpWriteFdcData(BYTE Data);
static BOOL FddpReadFdcData(BYTE *pData);



static FDD_JOB_ITEM_Q m_JobItemQ;
static HANDLE m_ProcessHandle, m_ThreadHandle;

static BOOL m_FddInterruptOccurred;

//플로피 디스크 디바이스 드라이버 초기화 함수
BOOL FddInitializeDriver(VOID)
{
	// 플로피 디스크의 모터를 OFF
	if( !FddpTurnOffMotor() )
	{
		DbgPrint("FddpTurnOffMotor() returned an error.\n\r");
		return FALSE;
	}

	// 플로피 디스크에서 수행해야 할 동작들을 처리하기 위한 프로세스 생성
	if( !PsCreateProcess(&m_ProcessHandle) )
		return FALSE;

	// 플로피 디스크에서 수행해야 할 동작들을 처리하기 위한 쓰레드 생성
	if( !PsCreateThread(&m_ThreadHandle, m_ProcessHandle, FddpJobProcessThread, NULL, DEFAULT_STACK_SIZE, FALSE, HIGH) )
		return FALSE;

	PsSetThreadStatus(m_ThreadHandle, THREAD_STATUS_READY);

	return TRUE;
}

//플로피 디스크 인터럽트가 발생했음을 알리는 변수를 설정
VOID Fdd_IRQ_Handler(VOID)
{

	
		///////////내용 작성해 주세요///////////////////////
}


#define BYTES_PER_SECTOR			512
#define SECTORS_PER_TRACK			18

//플로피 디스크의 모터 ON
static BOOL FddpTurnOnMotor(VOID)
{
	// Drive A, DMA 활성화, RESET, A드라이브 모터 가동
	WRITE_PORT_UCHAR((PUCHAR)FDD_DOR_PORT, 0x1c);	// Drive A, DMA 활성화, RESET, A 드라이브 모터 가동

	return TRUE;
}

//플로피 디스크의 모터 OFF
static BOOL FddpTurnOffMotor(VOID)
{
	WRITE_PORT_UCHAR((PUCHAR)FDD_DOR_PORT, 0x00);

	return TRUE;
}


//플로피 디스크 컨트롤러에서 데이터를 쓰는 함수
static BOOL FddpWriteFdcData(BYTE Data)
{
	UCHAR status;

	do
	{
		status = READ_PORT_UCHAR((PUCHAR)FDD_STATUS_PORT);
	}
	while( (status & 0xc0) != 0x80 );
	WRITE_PORT_UCHAR((PUCHAR)FDD_DATA_PORT, Data);

	return TRUE;
}
//플로피 디스크 컨트롤러에서 데이터를 읽는 함수
static BOOL FddpReadFdcData(BYTE *pData)
{
	UCHAR status;

	do
	{
		status = READ_PORT_UCHAR((PUCHAR)FDD_STATUS_PORT);
	}
	while( !(status & 0x80) );
	*pData = READ_PORT_UCHAR((PUCHAR)FDD_DATA_PORT);

	return TRUE;
}

//데이터 읽기 명령을 전송하기 전에 DMA를 설정하는 함수
static BOOL FddpSetupDMA(FDD_JOB_TYPE JobType)
{
	WORD count = BYTES_PER_SECTOR * SECTORS_PER_TRACK - 1;

	// 읽기 모드인지 쓰기 모드인지 설정
	if( JobType == FDD_READ_SECTOR )
		WRITE_PORT_UCHAR((PUCHAR)0x0b, 0x46);	// single mode, WRITE, Channel 2
	else
		WRITE_PORT_UCHAR((PUCHAR)0x0b, 0x4a);	// single mode, READ, Channel 2

	// ADDRESS
	WRITE_PORT_UCHAR((PUCHAR)0x04, 0x00);	// address register에 플로피에서 읽은 데이터를 저장해야할 주소 입력
	WRITE_PORT_UCHAR((PUCHAR)0x04, 0x00);	
	WRITE_PORT_UCHAR((PUCHAR)0x81, 0x00);	// Channel 2를 사용하기 위한 명령어

	WRITE_PORT_UCHAR((PUCHAR)0x05, (BYTE)count);		// count register에 데이터의 크기를 입력
	WRITE_PORT_UCHAR((PUCHAR)0x05, (BYTE)(count >> 8));	

	WRITE_PORT_UCHAR((PUCHAR)0x0a, 0x02);	// channel mask register에 channel 2번 활성화

	return TRUE;
}

//플로피 디스크와 직접적으로 통신하며 읽기/쓰기를 처리하는 함수
static BOOL FddpReadWriteSector(FDD_JOB_TYPE JobType, WORD Sector, BYTE NumbersOfSectors, BYTE *pData)
{
	BYTE *pDMAAddr = (BYTE *)0x00000000;
	BYTE drive, head, track, sector;

	int i;

	// 디스크에 쓰기는 구현 X
	if( JobType == FDD_WRITE_SECTOR )
		return FALSE;

	drive	= 0;
	head	= ((Sector % (SECTORS_PER_TRACK * 2)) / SECTORS_PER_TRACK);
	track	= (Sector / (SECTORS_PER_TRACK * 2));
	sector	= (Sector % SECTORS_PER_TRACK) + 1;

	// 모터 ON - 1st Interrupt
	m_FddInterruptOccurred = FALSE;
	{
		FddpTurnOnMotor();
	}
	while( !m_FddInterruptOccurred );

	// calibrate drive - 2st INTERRUPT
	m_FddInterruptOccurred = FALSE;
	{
		FddpWriteFdcData(0x07);	// calibrate command
		FddpWriteFdcData(0x07);	// drive 선택(00 : A, 01 : B, 10 : C, 11 : D)
	}
	while( !m_FddInterruptOccurred );

	// seek - 3st INTERRUPT
	m_FddInterruptOccurred = FALSE;
	{
		FddpWriteFdcData(0x0f);					// seek command
		FddpWriteFdcData((head << 2) + drive);	// Head Number
		FddpWriteFdcData(track);				// Disk의 Head를 위치 시킬 트랙번호
	}
	while( !m_FddInterruptOccurred );

	// DMA를 설정하는 함수
	FddpSetupDMA(JobType);

	m_FddInterruptOccurred = FALSE;
	{
		FddpWriteFdcData(0xe6);
		FddpWriteFdcData((head << 2) + drive);
		FddpWriteFdcData(track);
		FddpWriteFdcData(head);
		FddpWriteFdcData(1);
		FddpWriteFdcData(2);
		FddpWriteFdcData(SECTORS_PER_TRACK);
		FddpWriteFdcData(27);
		FddpWriteFdcData(0xff);				
	}
	while( !m_FddInterruptOccurred );

	pDMAAddr += (BYTES_PER_SECTOR * (sector - 1));

	if( JobType == FDD_READ_SECTOR )
	{
		for( i = 0 ; i < (BYTES_PER_SECTOR * NumbersOfSectors) ; i++ )
		{
			*(pData + i ) = *(pDMAAddr + i);
		}
	}

	// 모터 OFF
	FddpTurnOffMotor();

	return TRUE;

}

//플로피 디스크 드라이버의 처리를 담당하는 함수
static DWORD FddpJobProcessThread(PVOID StartContext)
{

	FDD_JOB_ITEM job_item;

	while( 1 )
	{
		if( !FddpPopJobItem(&job_item) )
		{
			HalTaskSwitch();
			continue;
		}

		FddpReadWriteSector(job_item.type, job_item.sector, job_item.numbers_of_sectors, job_item.pt_data);
		PsSetThreadStatus(job_item.thread, THREAD_STATUS_READY);
	}

	return 0;
}

//작업 큐에서 플로피 디스크의 작업을 받아오는 함수
static BOOL FddpPopJobItem(FDD_JOB_ITEM *pJobItem)
{
	BOOL bResult = TRUE;

ENTER_CRITICAL_SECTION();
	{
		// 큐 관리 함수로 부터 카운트 확인
		if( m_JobItemQ.cnt == 0 )
		{
			bResult = FALSE;
			goto $exit;
		}

		m_JobItemQ.cnt--;
		pJobItem->type					= m_JobItemQ.queue[m_JobItemQ.head].type;
		pJobItem->sector				= m_JobItemQ.queue[m_JobItemQ.head].sector;
		pJobItem->numbers_of_sectors	= m_JobItemQ.queue[m_JobItemQ.head].numbers_of_sectors;
		pJobItem->pt_data				= m_JobItemQ.queue[m_JobItemQ.head].pt_data;
		pJobItem->thread				= m_JobItemQ.queue[m_JobItemQ.head].thread;
		m_JobItemQ.head++;
		if( m_JobItemQ.head >= FDD_JOB_ITEM_Q_SIZE )
			m_JobItemQ.head = 0;
	}
$exit:
EXIT_CRITICAL_SECTION();

	return bResult;
}

//작업 큐에서 플로피 디스크의 작업을 저장하는 함수
static BOOL FddpPushJobItem(FDD_JOB_ITEM *pJobItem)
{
	BOOL bResult = TRUE;

ENTER_CRITICAL_SECTION();
	{
		// 큐 관리 함수로 부터 카운트 확인
		if( m_JobItemQ.cnt >= FDD_JOB_ITEM_Q_SIZE )
		{
			bResult = FALSE;
			goto $exit;
		}

		m_JobItemQ.cnt++;
		m_JobItemQ.queue[m_JobItemQ.tail].type					= pJobItem->type;
		m_JobItemQ.queue[m_JobItemQ.tail].sector				= pJobItem->sector;
		m_JobItemQ.queue[m_JobItemQ.tail].numbers_of_sectors	= pJobItem->numbers_of_sectors;
		m_JobItemQ.queue[m_JobItemQ.tail].pt_data				= pJobItem->pt_data;
		m_JobItemQ.queue[m_JobItemQ.tail].thread				= pJobItem->thread;
		m_JobItemQ.tail++;
		if( m_JobItemQ.tail >= FDD_JOB_ITEM_Q_SIZE )
			m_JobItemQ.tail = 0;
	}
$exit:
EXIT_CRITICAL_SECTION();

	return bResult;
}