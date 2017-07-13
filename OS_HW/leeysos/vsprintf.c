#include "vsprintf.h"

/*
 *	DEFINITIONS
 */

#define SIGN	1	/* unsigned, signed long */
#define LARGE	2	/* converts lower alphabet to higher one */

/*
 *	DECLARES INTERAL FUNCTIONS 
 */

static int ffmt(char *buf, const char *fmt, va_list args);

/*
 *	FUNCTIONS EXPORTED
 */
int vsprintf(char *buf, const char *fmt, va_list args)
{
	return ffmt(buf, fmt, args);
}

// ☞ 과제 진행 도중, 포맷 출력이 원하는대로 되지 않아, 수정
static int ffmt(char *buf, const char *fmt, va_list args)
{
	char *str, temp, numtemp[33], *numptr, prefix;
	unsigned int number, base, flag, width;

	str = buf;
	numtemp[32] = 0;

	for( ; *fmt ; fmt++ )
	{
		/* normal character */

		if( *fmt != '%' )
		{
			*str = *fmt;
			++str;

			continue;
		}

		/* flag process */
		fmt++;
		flag = 0;
		prefix = ' ';
		width = 0;

		if( *fmt == '0' )
			prefix = '0';

		while( *fmt >= '0' && *fmt <= '9' )
		{
			width++;
			fmt++;
		}
		
		switch( *fmt )
		{
		case 'c':
			*str++ = va_arg(args, char);
			continue;
		case 's':
			numptr = va_arg(args, char *);
			while ( *numptr != 0 )
				*str++ = *numptr++;
			continue;
		case '%':
			*str++ = '%';
			continue;
		case 'b':
			base = 2;
			continue;
		case 'd': case 'i':
			base = 10;
			flag |= SIGN;
			break;
		case 'u':
			base = 10;
			break;
		case 'o':
			base = 8;
			break;
		case 'X':
			base = 16;
			flag |= LARGE;
			break;
		case 'x':
			base = 16;
			break;
		default:
			continue;
		}
		
		/* number */
		numptr = &numtemp[31];
		number = va_arg(args, unsigned int);

		if( (flag & SIGN ) && (number & 0x80000000 ) )
		{
			number = ~number + 1;	/* 2's complement */
			*str++ = '-';			/* '-'[NUM] */
		}

		do 
		{
			temp = number % base;

			if( temp > 9 )
				temp += ( ( flag & LARGE ? 'A' : 'a' ) - 10 );
			else
				temp += '0';

			*numptr-- = temp;

			if( width > 0 )
				width--;
		}
		while( number /= base );
		
		if( width > 0 )
		{
			while( width-- != 0 )
				*numptr-- = prefix;
		}

		if( *fmt == 'x' || *fmt == 'X' )
		{
			*numptr-- = 'x';
			*numptr-- = '0';
		}

		while( temp = *(++numptr) )
				*str++ = temp;
	}

	*str = 0;

	return (int)(str - buf + 1);
}