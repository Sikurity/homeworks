#include "6845crt.h"

#define SCREEN_CX				80
#define SCREEN_CY				25
#define VIDEO_BUFFER_ADDRESS	0xb8000
#define VIDEO_BUFFER_SIZE		((SCREEN_CX * SCREEN_CY) * 2)

// 문자열 출력 함, char * 및 기타 가변인수를 받아서 출력
// 가변 인수는 va_list, va_start, va_end 관련 정보 참조

#define DEFAULT_ATTRIBUTE		7

#define KEY_LF					'\n'
#define KEY_CR					'\r'
#define KEY_TAB					'\t'
#define DEFAULT_TAB_SIZE		4

static WORD m_CursorXPos, m_CursorYPos;

static VOID		CrtpSetCursorPos(WORD x, WORD y);
static VOID		CrtpUpdateCursorPosByPtr(UCHAR *pPtr);
static UCHAR	*CrtpGetNextVideoPtr(VOID);
static UCHAR	*CrtpGetNextVideoPtrWithPos(WORD x, WORD y);
static VOID		CrtpKeyLF(UCHAR **pVideoPtr);
static VOID		CrtpKeyCR(UCHAR **pVideoPtr);
static VOID		CrtpKeyBackspace(UCHAR **pVideoPtr);
static VOID		CrtpScrollOneLine(VOID);
static int		CrtpPrintfFmt(UCHAR Attr, WORD x, WORD y, const char *fmt, va_list args);

KERNELAPI int CrtPrintf(const char *fmt, ...)
{
	int i;

	va_list args;

	va_start(args, fmt);

	i = CrtpPrintfFmt(DEFAULT_ATTRIBUTE, 0xffff, 0xffff, fmt, args);

	va_end(args);

	return i;
}

// ☞ 이번 과제를 위해 만든 X, Y 위치부터 원하는 포맷문자열을 출력하게 하는 함수
KERNELAPI int CrtPrintfXY(int x, int y, const char *fmt, ...)
{
	int i;

	va_list args;

	va_start(args, fmt);

	i = CrtpPrintfFmt(DEFAULT_ATTRIBUTE, x, y, fmt, args);

	va_end(args);

	return i;
}

// 가변 인수로 받은 문자랑 출력 문자열 조합해서 버퍼에 저장
static int CrtpPrintfFmt(UCHAR Attr, WORD x, WORD y, const char *fmt, va_list args)
{
	char buf[256];
	int i;

	i = vsprintf(buf, fmt, args);
	CrtPrintTextXYWithAttr(buf, x, y, Attr);

	return i;
}

KERNELAPI BOOL CrtPrintTextXYWithAttr(LPCSTR pText, WORD x, WORD y, UCHAR Attr)
{
	int i;
	UCHAR *pVideoBuf;

	if( pText == NULL )
		return FALSE;

	if( x == 0xffff && y == 0xffff )
		pVideoBuf = CrtpGetNextVideoPtr();
	else
		pVideoBuf = CrtpGetNextVideoPtrWithPos(x, y);

	while( *pText != NULL )
	{
		if( pVideoBuf >= (UCHAR *)(VIDEO_BUFFER_ADDRESS + VIDEO_BUFFER_SIZE) )
		{
			CrtpScrollOneLine();
			pVideoBuf -= (SCREEN_CX * 2);
		}

		if( *pText == KEY_LF )
		{
			;
		}
		else if( *pText == KEY_CR )
		{
			CrtpKeyCR(&pVideoBuf);
			CrtpKeyLF(&pVideoBuf);
		}
		else if( *pText == '\b' )
			CrtpKeyBackspace(&pVideoBuf);
		else if( *pText == KEY_TAB )
		{
			for( i = 0 ; i < DEFAULT_TAB_SIZE ; i++ )
			{
				*pVideoBuf++ = ' ';
				*pVideoBuf++ = Attr;
			}
		}
		else
		{
			*pVideoBuf++ = *pText;
			*pVideoBuf++ = Attr;
		}

		pText++;
	}

	CrtpUpdateCursorPosByPtr(pVideoBuf);

	return TRUE;
}

static UCHAR *CrtpGetNextVideoPtr(VOID)
{
	return (UCHAR *)(VIDEO_BUFFER_ADDRESS + (m_CursorXPos + m_CursorYPos * SCREEN_CX) * 2);
}

static UCHAR *CrtpGetNextVideoPtrWithPos(WORD x, WORD y)
{
	return (UCHAR *)(VIDEO_BUFFER_ADDRESS + (x + y * SCREEN_CX) * 2);
}

static VOID CrtpScrollOneLine(VOID)
{
	UCHAR *pVideoBase = (UCHAR *)VIDEO_BUFFER_ADDRESS;
	UCHAR *pSecondLine = (UCHAR *)(VIDEO_BUFFER_ADDRESS + SCREEN_CX * 2);

	while( pSecondLine < (UCHAR *)(VIDEO_BUFFER_ADDRESS + VIDEO_BUFFER_SIZE) )
		*pVideoBase++ = (*pSecondLine++);

	while( pSecondLine < (UCHAR *)(VIDEO_BUFFER_ADDRESS + VIDEO_BUFFER_SIZE) )
	{
		*pVideoBase++ = ' ';
		*pVideoBase++ = DEFAULT_ATTRIBUTE;
	}
}

static VOID CrtpKeyLF(UCHAR **pVideoPtr)
{
	WORD x, y;

	UCHAR *pVideoBase = (UCHAR *)VIDEO_BUFFER_ADDRESS;

	int videoPos = ( (int)*pVideoPtr - VIDEO_BUFFER_ADDRESS ) / 2;

	y = (WORD)(videoPos / SCREEN_CX);
	x = (WORD)(videoPos % SCREEN_CX);

	if( y == (SCREEN_CY - 1 ) )
	{
		CrtpScrollOneLine();
		*pVideoPtr = (UCHAR *)(VIDEO_BUFFER_ADDRESS + VIDEO_BUFFER_SIZE - (SCREEN_CX) * 2);
	}
	else
		*pVideoPtr += (SCREEN_CX * 2);

	return;
}

static VOID CrtpKeyCR(UCHAR **pVideoPtr)
{
	WORD x;

	int videoPos = ((int)*pVideoPtr - VIDEO_BUFFER_ADDRESS) / 2;

	x = (WORD)(videoPos % SCREEN_CX);
	*pVideoPtr -= x * 2;

	return;
}

static VOID CrtpKeyBackspace(UCHAR **pVideoPtr)
{
	if( *pVideoPtr < (UCHAR *)(VIDEO_BUFFER_ADDRESS + 2) )
		return;
	
	*pVideoPtr -= 2;
	**pVideoPtr = ' ';

}

static VOID CrtpUpdateCursorPosByPtr(UCHAR *pPtr)
{
	WORD x, y;
	int bufPos = ( (int)pPtr - VIDEO_BUFFER_ADDRESS ) / 2;

	y = (WORD)(bufPos / SCREEN_CX);
	x = (WORD)(bufPos % SCREEN_CX);

	CrtpSetCursorPos(x, y);
}

static VOID CrtpSetCursorPos(WORD x, WORD y)
{
	UCHAR data;
	// 커서 위치 설정 시 다른 곳에서 커서 위치를 동시에 수정하는 것을 막기 위해
	// 크리티컬 섹션을 지정하여 방지

ENTER_CRITICAL_SECTION();

	m_CursorXPos = x;
	m_CursorYPos = y;

EXIT_CRITICAL_SECTION();

	data = (UCHAR)( (x + y * SCREEN_CX) >> 8 );
	WRITE_PORT_UCHAR( (PUCHAR)0x3d4, 0x0e );
	WRITE_PORT_UCHAR( (PUCHAR)0x3d5, data );

	data = (UCHAR)( (x + y * SCREEN_CX) & 0xff );
	WRITE_PORT_UCHAR( (PUCHAR)0x3d4, 0x0f );
	WRITE_PORT_UCHAR( (PUCHAR)0x3d5, data );
}

BOOL CrtInitializeDriver(VOID)
{
	CrtClearScreen();

	return TRUE;
}

KERNELAPI VOID CrtClearScreen(VOID)
{
	UCHAR *pVideoBuf;

	pVideoBuf = (UCHAR *)VIDEO_BUFFER_ADDRESS;

	while( pVideoBuf < (UCHAR *)(VIDEO_BUFFER_ADDRESS + VIDEO_BUFFER_SIZE) )
	{
		*pVideoBuf++ = ' ';
		*pVideoBuf++ = DEFAULT_ATTRIBUTE;
	}

	CrtpSetCursorPos(0, 0);
}