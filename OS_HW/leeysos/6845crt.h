#ifndef _6845_CRT_DRIVER_HEADRE_FILE_
#define _6845_CRT_DRIVER_HEADRE_FILE_

#include "leeysos.h"

KERNELAPI VOID	CrtClearScreen(VOID);
KERNELAPI VOID	CrtGetCursorPos(BYTE *pX, BYTE *pY);
KERNELAPI BOOL	CrtPrintText(LPCSTR pText);
KERNELAPI BOOL	CrtPrintTextXYWithAttr(LPCSTR pText, WORD x, WORD y, UCHAR attr);
KERNELAPI int	CrtPrintf(const char *fmt, ...);
KERNELAPI int	CrtPrintfXY(int x, int y, const char *fmt, ...);				// �� �̹� ������ ���� ���� X, Y ��ġ���� ���ϴ� ���˹��ڿ��� ����ϰ� �ϴ� �Լ�

#endif /* _6845_CRT_DRIVER_HEADRE_FILE_ */