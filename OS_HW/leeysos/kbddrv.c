#include "kbddrv.h"

#define DEFAULT_STACK_SIZE				(64*1024) /* 64kbytes */

#define KBD_LED_SCROLLLOCK				0x01
#define KBD_LED_NUMLOCK					0x02
#define KBD_LED_CAPSLOCK				0x04

#define RAW_KEY_DATA_Q_SIZE				64
#define USER_KEY_DATA_Q_SIZE			(RAW_KEY_DATA_Q_SIZE/2)

//키보드에서 입력된 상태 그대로의 키 값을 저장하는 큐
typedef struct _RAW_DATA_QUEUE
{
	BYTE	cnt;
	BYTE	head;
	BYTE	tail;
	BYTE	queue[RAW_KEY_DATA_Q_SIZE];

} RAW_DATA_QUEUE, *PRAW_DATA_QUEUE;

//_RAW_DATA_QUEUE로부터 값을 읽어서 가공된 형태의 키 값을 저장하는 큐
typedef struct _USER_DATA_QUEUE
{
	BYTE			cnt;
	BYTE			head;
	BYTE			tail;
	KBD_KEY_DATA	queue[RAW_KEY_DATA_Q_SIZE];
} USER_DATA_QUEUE, *PUSER_DATA_QUEUE;

//키보드의 상태 및 입력된 키를 관리하기 위한 구조체
typedef struct _KBD_DATA {
	BYTE			indicator_status;
	BOOL			shift_key_pressed;
	BOOL			ctrl_key_pressed;
	BOOL			alt_key_pressed;
	RAW_DATA_QUEUE	raw_keydata_q;
	USER_DATA_QUEUE	user_keydata_q;
} KBD_DATA, *PKBD_DATA;

static DWORD KbdpTranslatorThread(PVOID StartContext); //Thread

static BOOL  KbdpPopRawKeyData(  IN RAW_DATA_QUEUE  *pKeyQ, OUT UCHAR *pKey);
static BOOL  KbdpPushRawKeyData( IN RAW_DATA_QUEUE  *pKeyQ, IN  UCHAR key);
static BOOL  KbdpPopUserKeyData( IN USER_DATA_QUEUE *pKeyQ, OUT KBD_KEY_DATA *pKeyData);
static BOOL  KbdpPushUserKeyData(IN USER_DATA_QUEUE *pKeyQ, IN  KBD_KEY_DATA *pKeyData);

static BOOL  KbdpResetIndicator(VOID);
static HANDLE m_ProcessHandle, m_TranslatorThreadHandle;
static KBD_DATA m_KbdData;


#define KBD_ASCII_ORG				0x00
#define KBD_ASCII_WITH_CAPSLOCK		0x01
#define KBD_ASCII_WITH_SHIFT		0x02
#define KBD_ASCII_CONTROL			0x04

//스캔 코드 변환 테이블
static BYTE m_AsciiCode[5][128] = {
	{ //ASCII TABLE(다른 특수 키카 눌러지지 않은 경우의 문자)
	0x00, 0x1b,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=', 0x08, 0x09,
	 'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', 0x0d, 0x00,  'a',  's',
	 'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', 0x27,  '`', 0x00, 0x5c,  'z',  'x',  'c',  'v',
	 'b',  'n',  'm',  ',',  '.',  '/', 0x00,  '*', 0x00,  ' ', 0x00, 0x03, 0x03, 0x03, 0x03, 0x08,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,  '-', 0x00, 0x00, 0x00,  '+', 0x00,
	0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x5c, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0d, 0x00,  '/', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  '/', 0x00, 0x00, 0x00, 0x00, 0x00,
	},
	{ // ASCII TABLE WITH CAPSLOCK
	0x00, 0x1b,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=', 0x08, 0x09,
	 'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '[',  ']', 0x0d, 0x00,  'A',  'S',
	 'D',  'F',  'G',  'H',  'J',  'K',  'L',  ';', 0x27,  '`', 0x00, 0x5c,  'Z',  'X',  'C',  'V',
	 'B',  'N',  'M',  ',',  '.',  '/', 0x00,  '*', 0x00,  ' ', 0x00, 0x03, 0x03, 0x03, 0x03, 0x08,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,  '-', 0x00, 0x00, 0x00,  '+', 0x00,
	0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x5c, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0d, 0x00,  '/', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  '/', 0x00, 0x00, 0x00, 0x00, 0x00,
	},
	{ //ASCII WITH SHIFT
	0x00, 0x1b,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '+', 0x7e, 0x7e,
	 'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', 0x7e, 0x00,  'A',  'S',
	 'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':', 0x27,  '~', 0x00,  '|',  'Z',  'X',  'C',  'V',
	 'B',  'N',  'M',  '<',  '>',  '?', 0x00,  '*', 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,  '-', 0x00, 0x00, 0x00,  '+', 0x00,
	0x00, 0x00, 0x01, 0x7f, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0d, 0x00,  '/', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  '/', 0x00, 0x00, 0x00, 0x00, 0x00,
	},
	{ // ASCII WITH CAPSLOCK & SHIFT
	0x00, 0x1b,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '+', 0x7e, 0x7e,
	 'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '{',  '}', 0x7e, 0x00,  'a',  's',
	 'd',  'f',  'g',  'h',  'j',  'k',  'l',  ':', 0x27,  '~', 0x00,  '|',  'z',  'x',  'c',  'v',
	 'b',  'n',  'm',  '<',  '>',  '?', 0x00,  '*', 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,	
 	0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,  '-', 0x00, 0x00, 0x00,  '+', 0x00,
	0x00, 0x00, 0x01, 0x7f, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0d, 0x00,  '/', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  '/', 0x00, 0x00, 0x00, 0x00, 0x00,
	},
	{ //ASCII WITH CONTROL
	0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x7f, 0x7f,
	0x11, 0x17, 0x05, 0x12, 0x14, 0x19, 0x15, 0x09, 0x0f, 0x10, 0x02, 0x02, 0x0a, 0x00, 0x01, 0x13,
	0x04, 0x06, 0x07, 0x08, 0x0a, 0x0b, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x18, 0x03, 0x16,
	0x02, 0x0e, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	}
};


BOOL KbdInitializeDriver(VOID)
{
	memset(&m_KbdData, 0, sizeof(m_KbdData));

	m_KbdData.indicator_status = KBD_LED_NUMLOCK;

	// 현재 indicator_status 필드 설정된 값을 키보드 하드웨어에 전송하는 함수
	// <Num Lock>, <Cabs Lock>, <Scroll Lock> 중에서 <Num Lock>을 제외한 나머지 값은 모두 OFF 상태로 하고
	// 키보드에 이 값을 반영하기 위한 함수

	KbdpResetIndicator();

	// 키보드 입력을 처리할 프로세스 생성
	if( !PsCreateProcess(&m_ProcessHandle) )
		return FALSE;

	// raw_key_data를 user_kay_data로 변형하는 기능을 하는 쓰레드 생성
	if( !PsCreateThread(&m_TranslatorThreadHandle, m_ProcessHandle, KbdpTranslatorThread, NULL, DEFAULT_STACK_SIZE, FALSE, HIGH) )
		return FALSE;

	PsSetThreadStatus(m_TranslatorThreadHandle, THREAD_STATUS_READY);

	return TRUE;
}

// 실질적으로 키보드 인터럽트를 처리하는 함수
VOID Kbd_IRQ_Handler(VOID)
{
	UCHAR keyCode;

	// User가 입력한 스캔 코드 값을 keycode 변수에 읽어옴
	keyCode = READ_PORT_UCHAR((PUCHAR)0x60);

	if( keyCode == 0xff || keyCode == 0x00 )	// Overrun
		return;
	else if( keyCode == 0xfa )					// Acknowledge
		return;

	// RAW Q에 Key Data를 넣고, IRQ 핸들러는 호출
	if( !KbdpPushRawKeyData(&(m_KbdData.raw_keydata_q), keyCode) )
		return;
}


/*
					
								* LED ASSOCIATED KEYs *
45-c5 : num-lock
3a-ba : capslock
46-c6 : scroll-lock

								* MAIN KEYs (LEFT-SIDED AREA) *
2a-aa : left-shift
36-b6 : right-shift
1d-9d : left-ctrl
e0-1d-e0-9d : right-ctrl
38-b8 : left-alt
e0-38-e0-b8 : right-alt

1c-9c : enter
e-8e : back-space
f-8f : tab

e0-71-e0-f1 : chinese
e0-5c-e0-dc : right-windows-key
e0-72 : kor/eng

1-81 : esc

3b-bb : f1
3c-bc : f2
3d-bd : f3
3e-be : f4
3f-bf : f5
40-c0 : f6
41-c1 : f7
42-c2 : f8
43-c3 : f9
44-c4 : f10
57-d7 : f11
58-d8 : f12

								* MIDDLE SIDE *
e0-2a-e0-37-e0-b7-e0-aa : print screen
e1-1d-45-e1-9d-c5 : pause

e0-2a-e0-52-e0-d2-e0-aa : insert
e0-2a-e0-47-e0-c7-e0-aa : home
e0-2a-e0-49-e0-c9-e0-aa : page up
e0-2a-e0-53-e0-d3-e0-aa : delete
e0-2a-e0-4f-e0-cf-e0-aa : end
e0-2a-e0-51-e0-d1-e0-aa : page down

e0-2a-e0-48-e0-c8-e0-aa : up arrow
e0-2a-e0-50-e0-d0-e0-aa : down arrow
e0-2a-e0-4b-e0-cb-e0-aa : left arrow
e0-2a-e0-4d-e0-cd-e0-aa : right arrow

								* NUM PAD *
e0-35-e0-b5 : /
37-b7 : *
4a-ca : -
4e-ce : +
e0-1c-e0-9c : enter
53-d3 : . (or Del)
52-d2 : 0 (or Ins)
4f-cf : 1
50-d0 : 2
51-d1 : 3
4b-cb : 4
4c-cc : 5
4d-cd : 6
47-c7 : 7
48-c8 : 8
c9-49 : 9

e0-21-e0-a1 : calculator
e0-18-e0-98 : X'fer

*/

//키보드로부터 입력받은 데이터를 가공하는 함수(raw_keydata_q -> user_keydata_q)
static DWORD KbdpTranslatorThread(PVOID StartContext)
{
	KBD_KEY_DATA key_data;
	BYTE raw_key, indicator, key_family;

	// 반복문을 통해 하나씩 raw_keydata값을 user_keydata로 변경

	while( 1 )
	{
		if( !KbdpPopRawKeyData(&(m_KbdData.raw_keydata_q), &raw_key) )
		{
			HalTaskSwitch(); // RAW 큐에 더이상 값이 없을 때, 다른 Task로 Task Switching
			continue;
		}

		if( raw_key & 0x80 )
		{
			if( raw_key == 0xAA || raw_key == 0xB6 ) // Shift Key Breaked
				m_KbdData.shift_key_pressed = FALSE;
			else if( raw_key == 0xB8 ) // Alt Key Breaked
				m_KbdData.alt_key_pressed = FALSE;
			else if( raw_key == 0x9D ) // Ctrl Key Breaked
				m_KbdData.ctrl_key_pressed = FALSE;
			continue;
		}

		// RESET INDICATORs
		if( raw_key == 0x45 ) // Num Lock
		{
			indicator = KBD_LED_NUMLOCK;
			goto $reset_indicator;
		}
		else if( raw_key == 0x3A ) // Caps Lock
		{
			indicator = KBD_LED_CAPSLOCK;
			goto $reset_indicator;
		}
		else if( raw_key == 0x46 ) // Scroll Lock
		{
			indicator = KBD_LED_SCROLLLOCK;
$reset_indicator:
			if( m_KbdData.indicator_status & indicator )
				m_KbdData.indicator_status &= (~indicator);
			else
				m_KbdData.indicator_status |= indicator;
			KbdpResetIndicator();
			continue;
		}

		// SHIFT, ALT, CTRL
		if( raw_key == 0x2A || raw_key == 0x36 )	// SHIFT
		{
			m_KbdData.shift_key_pressed = TRUE;
			continue;
		}
		else if( raw_key == 0x1D )	// CTRL
		{
			m_KbdData.ctrl_key_pressed = TRUE;
			continue;
		}
		else if( raw_key == 0x38 )	// ALT
		{
			m_KbdData.alt_key_pressed = TRUE;
			continue;
		}

		// GENERAL KEYs
		key_family = KBD_ASCII_ORG;
		key_family |= (m_KbdData.shift_key_pressed ? KBD_ASCII_WITH_SHIFT : 0);
		key_family |= (m_KbdData.indicator_status & KBD_LED_CAPSLOCK ? KBD_ASCII_WITH_CAPSLOCK : 0);

		key_data.type = KBD_KTYPE_GENERAL;
		key_data.key = m_AsciiCode[key_family][raw_key];

		KbdpPushUserKeyData(&(m_KbdData.user_keydata_q), &key_data);
	}
	
	return 0;
}

//키보드의 indicator의 상태를 컨트롤러에 전송하는 함수
static BOOL KbdpResetIndicator(VOID)
{
	UCHAR status;

	// 0x64번 포트에서 키보드의 상태를 확인
	// input buffer가 비어있으면 0xED 명령으로 입출력 포트인 0x60 포트에 넣어준다
	// 0xED : 현재 키보드의 Indicator LED 값을 설정하는 명령어

	do
	{
		status = READ_PORT_UCHAR((PUCHAR)0x64);
	}
	while( status & 0x02 );

	WRITE_PORT_UCHAR((PUCHAR)0x60, 0xED);

	// 이전에 우리가 설정했던 indicator의 값을 키보드로 보내준다.
	// 키보드가 LED를 모두 불빛이 사라지게 한다.
	do
	{
		status = READ_PORT_UCHAR((PUCHAR)0x64);
	}
	while( status & 0x02 );

	WRITE_PORT_UCHAR((PUCHAR)0x60, m_KbdData.indicator_status);

	return 0;
}

//Raw 큐로부터 DATA를 가져오는 함수
static BOOL KbdpPopRawKeyData( IN RAW_DATA_QUEUE *pKeyQ, OUT UCHAR *pKey)
{
	BOOL bResult = TRUE;

ENTER_CRITICAL_SECTION();
	{
		// count 체크
		if( pKeyQ->cnt == 0 )
		{
			bResult = FALSE;
			goto $exit;
		}

		pKeyQ->cnt--;
		*pKey = pKeyQ->queue[pKeyQ->head++];
		if( pKeyQ->head >= RAW_KEY_DATA_Q_SIZE )
			pKeyQ->head = 0;
	}
$exit :
EXIT_CRITICAL_SECTION();
	return bResult;
}

//Raw 큐에 DATA를 넣는 함수
static BOOL KbdpPushRawKeyData(IN RAW_DATA_QUEUE *pKeyQ, IN  UCHAR key)
{
	BOOL bResult = TRUE;

ENTER_CRITICAL_SECTION();
	{
		// 큐에 남은 공간 체크
		if( pKeyQ->cnt >= RAW_KEY_DATA_Q_SIZE )
		{
			bResult = FALSE;
			goto $exit;
		}

		pKeyQ->cnt++;
		pKeyQ->queue[pKeyQ->tail++] = key;
		if( pKeyQ->tail >= RAW_KEY_DATA_Q_SIZE )
			pKeyQ->tail = 0;
	}
$exit :
EXIT_CRITICAL_SECTION();
	return bResult;
}

//User 큐로부터 DATA를 가져오는 함수
static BOOL  KbdpPopUserKeyData( IN USER_DATA_QUEUE *pKeyQ, OUT KBD_KEY_DATA *pKeyData)
{
	BOOL bResult = TRUE;

ENTER_CRITICAL_SECTION();
	{
		// count 체크
		if( pKeyQ->cnt == 0 )
		{
			bResult = FALSE;
			goto $exit;
		}

		pKeyQ->cnt--;
		pKeyData->type = pKeyQ->queue[pKeyQ->head].type;
		pKeyData->key = pKeyQ->queue[pKeyQ->head].key;
		pKeyQ->head++;
		if( pKeyQ->head >= USER_KEY_DATA_Q_SIZE )
			pKeyQ->head = 0;
	}
$exit :
EXIT_CRITICAL_SECTION();
	return bResult;
}

//User 큐에 DATA를 넣는 함수
static BOOL  KbdpPushUserKeyData(IN USER_DATA_QUEUE *pKeyQ, IN  KBD_KEY_DATA *pKeyData)
{
	BOOL bResult = TRUE;

ENTER_CRITICAL_SECTION();
	{
		// 큐에 남은 공간 체크
		if( pKeyQ->cnt >= USER_KEY_DATA_Q_SIZE )
		{
			bResult = FALSE;
			goto $exit;
		}

		pKeyQ->cnt++;
		pKeyData->type = pKeyQ->queue[pKeyQ->tail].type;
		pKeyData->key = pKeyQ->queue[pKeyQ->tail].key;
		pKeyQ->tail++;
		if( pKeyQ->tail >= USER_KEY_DATA_Q_SIZE )
			pKeyQ->tail = 0;
	}
$exit :
EXIT_CRITICAL_SECTION();
	return bResult;
}