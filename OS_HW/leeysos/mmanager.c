#include "leeysos.h"

// Memory Pool 시작 주소
#define MEMORY_POOL_START_ADDRESS	0x00200000

#define MEM_BLK_SIZE				512
#define MEM_BLK_FREE				0
#define MEM_BLK_USED				1

// Memory Descriptor 구조체
typedef struct _MEM_BLK_DESC
{
	DWORD					status;
	DWORD					block_size;
	struct	_MEM_BLK_DESC	*pNext;
} MEM_BLK_DESC, *PMEM_BLK_DESC;

//모든 Memory 정보를 관리하는 구조체
typedef struct _MEM_BLK_MANAGER
{
	DWORD				nBlocks;
	DWORD				nUsedBlocks;
	DWORD				nFreeBlocks;
	MEM_BLK_DESC		*pDescEntry;
	int					*pPoolEntry;
} MEM_BLK_MANAGER, *PMEM_BLK_MANAGER;


static DWORD			m_MemSize;
static MEM_BLK_MANAGER	m_MemBlkManager;

static BOOL			MmpCheckMemorySize(VOID);
static BOOL			MmpCreateMemPoolBlk(VOID);
static DWORD		MmpGetRequiredBlocksFromBytes(DWORD bytes);
static VOID			*MmpGetPoolAddrFromDescAddr(MEM_BLK_DESC *pDescAddr);
static MEM_BLK_DESC	*MmpGetDescAddrFromPoolAddr(VOID *pPoolAddr);

BOOL MmkInitializeMemoryManager(VOID)
{
	if( !MmpCheckMemorySize() )
	{
		DbgPrint("MmpCheckMemorySize() returned an error.\r\n");
		return FALSE;
	}

	// 메모리 초기화 모듈
	if( !MmpCreateMemPoolBlk() )
	{
		DbgPrint("MmpCreateMemPoolBlk() returned an error.\r\n");
		return FALSE;
	}
	DbgPrint("Memory Manager is initialized!!\r\n");

	return TRUE;
}

static BOOL MmpCheckMemorySize(VOID)
{
	BOOL bResult;
	DWORD *pAddr = (DWORD *)0x00000000, tmp;

	// 메모리 최대값 확인

	while( 1 )
	{
		//4MB 단위로 메모리 위치 변경
		pAddr += (4 * 1024 * 1024);

		//tmp에 이전 메모리 위치에 있던 값 저장

		tmp = *pAddr;
		*pAddr = 0x11223344;

		if( *pAddr != 0x11223344 )
			break;

		*pAddr = tmp;
	}

ENTER_CRITICAL_SECTION();

	m_MemSize = (DWORD)pAddr;
	bResult = (m_MemSize < MEMORY_POOL_START_ADDRESS + (1 * 1024 * 1024) ? FALSE : TRUE);

EXIT_CRITICAL_SECTION();

	return bResult;
}

static BOOL	MmpCreateMemPoolBlk(VOID)
{
	DWORD dwUsableMemSize;
	DWORD dwBlksOfUsableMem;
	DWORD dwBlksOfAllocatableMem;
	DWORD dwBlksOfDescs;
	DWORD i;

	int *pPoolEntry;
	MEM_BLK_DESC *pPrev, *pCur;

	// 각 변수들 초기화
	// 사용 가능한 Memory의 Byte 단위 크기
ENTER_CRITICAL_SECTION();
	dwUsableMemSize = m_MemSize - MEMORY_POOL_START_ADDRESS;
EXIT_CRITICAL_SECTION();

	// 사용 가능한 Memory block수를 계산
	dwBlksOfUsableMem	= MmpGetRequiredBlocksFromBytes(dwUsableMemSize);

	// 사용 가능한 Memory block을 관리하는데 필요한 Memory Descriptor의 수
	dwBlksOfDescs		= MmpGetRequiredBlocksFromBytes(dwBlksOfUsableMem * sizeof(MEM_BLK_DESC));
	
	// 할당 가능한 Memory block수를 계산
	dwBlksOfAllocatableMem = dwBlksOfUsableMem - dwBlksOfDescs;

	// 할당 가능한 Memory block을 관리하는데 필요한 Memory Descriptor의 수
	dwBlksOfDescs = MmpGetRequiredBlocksFromBytes(dwBlksOfAllocatableMem * sizeof(MEM_BLK_DESC));
	// Memory block의 시작위치를 지정

	pPoolEntry = (int *)(MEMORY_POOL_START_ADDRESS + dwBlksOfDescs * MEM_BLK_SIZE);

ENTER_CRITICAL_SECTION();

	m_MemBlkManager.nBlocks		= dwBlksOfAllocatableMem;
	m_MemBlkManager.nUsedBlocks	= 0;
	m_MemBlkManager.nFreeBlocks	= dwBlksOfAllocatableMem;
	m_MemBlkManager.pDescEntry	= (MEM_BLK_DESC *)MEMORY_POOL_START_ADDRESS;
	m_MemBlkManager.pPoolEntry	= pPoolEntry;

	pPrev = m_MemBlkManager.pDescEntry;
	pPrev->status = MEM_BLK_FREE;

	// Memory Block을 전부 초기화
	for( i = 1 ; i < dwBlksOfAllocatableMem ; i++ )
	{
		pCur = (MEM_BLK_DESC *)(MEMORY_POOL_START_ADDRESS + sizeof(MEM_BLK_DESC) * i);
		pCur->status = MEM_BLK_FREE;
		pCur->block_size = 0;
		pPrev->pNext = pCur;
		pPrev = pCur;
	}

	pCur->pNext = NULL;
EXIT_CRITICAL_SECTION();

	return TRUE;
}

static DWORD MmpGetRequiredBlocksFromBytes(DWORD bytes)
{
	DWORD dwBlocks = 0;

	dwBlocks = bytes / MEM_BLK_SIZE;

	if( bytes % MEM_BLK_SIZE )
		dwBlocks++;

	return dwBlocks;
}



// 메모리 할당 함수
KERNELAPI VOID *MmAllocateNonCachedMemory(IN ULONG NumberOfBytes)
{
	DWORD dwRequiredBlocks, dwCurBlocks, i;
	MEM_BLK_DESC *pAllocStart, *pCur;

	// 요청된 바이트가 0일 경우 NULL을 리턴
	if( NumberOfBytes == 0 )
		return (VOID *)NULL;

	// 요청된 바이트를 블록 단위로 변경해서 시스템에 남아있는 블록의 수와 비교
	// 이 후, 할당 가능한 블록이 부족할 경우 NULL을 리턴
	dwRequiredBlocks = MmpGetRequiredBlocksFromBytes(NumberOfBytes);

ENTER_CRITICAL_SECTION();

	if( m_MemBlkManager.nFreeBlocks < dwRequiredBlocks )
	{

EXIT_CRITICAL_SECTION();
	
		return (VOID *)NULL;
	}

	// 메모리 디스크립터 리스트를 검색하여 할당 가능한 공간을 찾는다
	pAllocStart = pCur = m_MemBlkManager.pDescEntry;
	dwCurBlocks = 0;
	while( dwCurBlocks < dwRequiredBlocks )
	{
		if( pCur->pNext == NULL )
		{
			
EXIT_CRITICAL_SECTION();
	
			return (VOID *)NULL;
		}

		if( pCur->status == MEM_BLK_USED )
		{
			dwCurBlocks = 0;
			pAllocStart = pCur = pCur + pCur->block_size;
			continue;
		}

		dwCurBlocks++;
		pCur = pCur->pNext;
	}

	// 할당 가능한 공간을 찾은 후, 해당 메모리 블록의 상태를 USED로 변경
	pCur = pAllocStart;

	for( i = 0 ; i < dwRequiredBlocks ; i++ )
	{
		pCur->status = MEM_BLK_USED;
		pCur = pCur->pNext;
	}

	pAllocStart->block_size = dwRequiredBlocks;

	m_MemBlkManager.nFreeBlocks -= dwRequiredBlocks;
	m_MemBlkManager.nUsedBlocks += dwRequiredBlocks;

	// 할당된 메모리 블록의 실제 주소를 받아온다
	pAllocStart = (MEM_BLK_DESC *)MmpGetPoolAddrFromDescAddr(pAllocStart);

EXIT_CRITICAL_SECTION();

	return (VOID *)pAllocStart;
}

// 메모리 디스크립터를 가지고 블록의 주소를 찾는 함수
static VOID *MmpGetPoolAddrFromDescAddr(MEM_BLK_DESC *pDescAddr)
{
	int ResultAddr;

ENTER_CRITICAL_SECTION();

	// 메모리 풀에서 메모리 블록의 시작 주소 가지고온다.
	ResultAddr = (int)(m_MemBlkManager.pPoolEntry);
	// 할당된 메모리 블록의 실제주소를 구한다.
	ResultAddr += (int)((pDescAddr - m_MemBlkManager.pDescEntry) * MEM_BLK_SIZE);

EXIT_CRITICAL_SECTION();

	return (VOID *)ResultAddr;
}

KERNELAPI VOID MmFreeNonCachedMemory(IN PVOID BaseAddress)
{
	MEM_BLK_DESC *pCur;
	DWORD i, dwBlockSize;

	if( BaseAddress == NULL )
		return;

	// 해제하고자 하는 메모리 블록의 디스크립터 리스트를 검색
	pCur = (MEM_BLK_DESC *)MmpGetDescAddrFromPoolAddr(BaseAddress);

ENTER_CRITICAL_SECTION();

	if( pCur->status != MEM_BLK_USED )
	{

EXIT_CRITICAL_SECTION();

		return;
	}

	// 검색 후, 해제하고자 하는 메모리 블록의 상태를 FREE로 변경
	dwBlockSize = pCur->block_size;
	for( i = 0 ; i < dwBlockSize ; i++ )
	{
		pCur->status = MEM_BLK_FREE;
		pCur->block_size = 0;
		pCur = pCur->pNext;
	}

	m_MemBlkManager.nFreeBlocks += dwBlockSize;
	m_MemBlkManager.nUsedBlocks -= dwBlockSize;

EXIT_CRITICAL_SECTION();
}

// 메모리 블록 주소를 가지고 메모리 디스크립터를 찾는 함수
static MEM_BLK_DESC *MmpGetDescAddrFromPoolAddr(VOID *pPoolAddr)
{
	int ResultAddr;

ENTER_CRITICAL_SECTION();

	// 실제 메모리 블록 주소에서 메모리풀에서 메모리풀의 시작 주소를 뺀 값
	ResultAddr = (int)pPoolAddr - (int)(m_MemBlkManager.pDescEntry);
	
	// 블록 단위로 나누고, 해당 메모리 디스크리터를 찾는다
	ResultAddr = ResultAddr / MEM_BLK_SIZE * sizeof(MEM_BLK_DESC);
	ResultAddr += (int)(m_MemBlkManager.pDescEntry);

EXIT_CRITICAL_SECTION();

	return (MEM_BLK_DESC *)ResultAddr;
}