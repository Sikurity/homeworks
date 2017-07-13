#ifndef _STDARG_H_
#define _STDARG_H_

/* codes from Windows */
typedef char	*va_list;

// ap�� ���� argument�� ù �ּҸ� ����.
#define va_start(ap, v) ( ap = (va_list)(&(v)) + 4)	// �� ���� ���� ����, ���� ����� ���ϴ´�� ���� �ʾ�, ����
#define va_arg(ap, t) ((t)*((int *)(ap += 4) - 1 )) // �� ���� ���� ����, ���� ����� ���ϴ´�� ���� �ʾ�, ����
#define va_end(ap) ( ap = (va_list)NULL )			// �� ���� ���� ����, ���� ����� ���ϴ´�� ���� �ʾ�, ����

#endif // _STDARG_H_