#ifndef _VSPRINTF_H_
#define _VSPRINTF_H_

/*
 *	INCLUDE FILES
 */

#include "stdarg.h"

/*
 *	Library Functions
 */

int vsprintf(char *buf, const char *fmt, va_list args);

#endif /* _VSPRINTF_H_ */