/**
*	@date		2016. 09. 12 01:30
*	@author		CSE 2011004040 이영식
*	@method		Open Source machar 함수 사용
*	@direction	float 형의 Machine Accuracy를 확인하기 위해서는 #define DOUBLE 매크로 제거 or 주석처리
*				double 형의 Machine Accuracy를 확인하기 위해서는 #define DOUBLE 매크로 추가
*/

#include <stdio.h>
#include "machar.h"

#define DOUBLE

#ifdef DOUBLE
#define REAL		double
#define ZERO		0.0e0
#define ONE			1.0e0
#define PREC		"Double "
#define REALSIZE	2
#define machar		r8_machar
#else
#define REAL		float
#define ZERO		0.0
#define ONE			1.0
#define PREC		"Single "
#define REALSIZE	1
#define machar		r4_machar
#endif

#define DISPLAY(s, x) \
	{ \
		uval.xbig = x; \
		printf(s); \
		printf(" %24.16e ",(REAL)x); \
		for( i = 0 ; i < REALSIZE ; i++ ) printf(" %9X ", uval.inMemory[i]); \
		printf("\n"); \
	}

int main()
{
	int i;
	long int ibeta, it, irnd, ngrd, machep, negep, iexp, minexp, maxexp;
	REAL eps, epsneg, xmin, xmax; 

	union _uval
	{
		long int inMemory[REALSIZE];
		REAL xbig;
	} uval;

	machar(&ibeta, &it, &irnd, &ngrd, &machep, &negep, &iexp, &minexp, &maxexp, &eps, &epsneg, &xmin, &xmax);

	printf("ibeta : %d \n", ibeta);
	printf("it : %d \n", it);
	printf("irnd : %d \n", irnd);
	printf("ngrd : %d \n", ngrd);
	printf("machep : %d \n", machep);
	printf("negep : %d \n", negep);
	printf("iexp : %d \n", iexp);
	printf("minexp : %d \n", minexp);
	printf("maxexp : %d \n", maxexp); 
	
	puts("");

	DISPLAY("eps   ", eps);
	DISPLAY("epsneg", epsneg);
	DISPLAY("xmin  ", xmin);
	DISPLAY("xmax  ", xmax);

	puts("");

	return 0;
}