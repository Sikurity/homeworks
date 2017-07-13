#ifndef _6845_CRT_DRIVER_HEADRE_FILE_
#define _6845_CRT_DRIVER_HEADRE_FILE_

#include "leeysos.h"

KERNELAPI VOID	CrtClearScreen(VOID);
KERNELAPI VOID	CrtGetCursorPos(BYTE *pX, BYTE *pY);
KERNELAPI BOOL	CrtPrintText(LPCSTR pText);
KERNELAPI BOOL	CrtPrintTextXYWithAttr(LPCSTR pText, WORD x, WORD y, UCHAR attr);
KERNELAPI int	CrtPrintf(const char *fmt, ...);
KERNELAPI int	CrtPrintfXY(int x, int y, const char *fmt, ...);				// ☞ 이번 과제를 위해 만든 X, Y 위치부터 원하는 포맷문자열을 출력하게 하는 함수

#endif /* _6845_CRT_DRIVER_HEADRE_FILE_ */