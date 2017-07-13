#ifndef _STDARG_H_
#define _STDARG_H_

/* codes from Windows */
typedef char	*va_list;

// ap로 받은 argument의 첫 주소를 지정.
#define va_start(ap, v) ( ap = (va_list)(&(v)) + 4)	// ☞ 과제 진행 도중, 포맷 출력이 원하는대로 되지 않아, 수정
#define va_arg(ap, t) ((t)*((int *)(ap += 4) - 1 )) // ☞ 과제 진행 도중, 포맷 출력이 원하는대로 되지 않아, 수정
#define va_end(ap) ( ap = (va_list)NULL )			// ☞ 과제 진행 도중, 포맷 출력이 원하는대로 되지 않아, 수정

#endif // _STDARG_H_