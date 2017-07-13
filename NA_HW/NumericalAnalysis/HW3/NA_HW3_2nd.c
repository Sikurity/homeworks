#define ANSI

#include <Windows.h>
#include <stdio.h>
#include "nr.h"
#include "muller.h"

#define CHECK_TIME_START if (QueryPerformanceFrequency((LARGE_INTEGER*)&freq)) {QueryPerformanceCounter((LARGE_INTEGER*)&start);
#define CHECK_TIME_END(a,b) QueryPerformanceCounter((LARGE_INTEGER*)&end); a=(float)((double)(end - start)/freq*1000); b=TRUE; } else b=FALSE;

#define X1		(1.0f)
#define X2		(10.0f)
#define N		(100)
#define	MAX		(20)
#define XACC	(0.000001f)

void nrerror(char error_text[])
{
	printf("%s\n", error_text);
}

void Bessel_J_Func(float x, float *fn, float *df)
{
	*fn = bessj0(x);
	*df = -bessj1(x);
}

int main()
{
	float xb1[MAX + 1], xb2[MAX + 1];
	int i, nb = MAX;
	float time;
	BOOL flag;
	__int64 freq, start, end;

	zbrak(bessj0, X1, X2, N, xb1, xb2, &nb);

	printf("(a) Bisection\n");
	CHECK_TIME_START;
	for(i = 1 ; i <= nb ; i++)
		printf(" - %d번째 근 : %f\n", i, rtbis(bessj0, xb1[i], xb2[i], XACC));
	CHECK_TIME_END(time, flag);
	if(flag) printf("걸린 시간 : %8.6fms\n\n", time);

	printf("(b) Linear Interpolation\n");
	CHECK_TIME_START;
	for(i = 1 ; i <= nb ; i++)
		printf(" - %d번째 근 : %f\n", i, rtflsp(bessj0, xb1[i], xb2[i], XACC));
	CHECK_TIME_END(time, flag);
	if(flag) printf("걸린 시간 : %8.6fms\n\n", time);


	printf("(c) Secant\n");
	CHECK_TIME_START;
	for(i = 1 ; i <= nb ; i++)
		printf(" - %d번째 근 : %f\n", i, rtsec(bessj0, xb1[i], xb2[i], XACC));
	CHECK_TIME_END(time, flag);
	if(flag) printf("걸린 시간 : %8.6fms\n\n", time);


	printf("(d) Newton-Raphson\n");
	CHECK_TIME_START;
	for(i = 1 ; i <= nb ; i++)
		printf(" - %d번째 근 : %f\n", i, rtnewt(Bessel_J_Func, xb1[i], xb2[i], XACC));
	CHECK_TIME_END(time, flag);
	if(flag) printf("걸린 시간 : %8.6fms\n\n", time);


	printf("(e) Newton With Bracketing\n");
	CHECK_TIME_START;
	for(i = 1 ; i <= nb ; i++)
		printf(" - %d번째 근 : %f\n", i, rtsafe(Bessel_J_Func, xb1[i], xb2[i], XACC));
	CHECK_TIME_END(time, flag);
	if(flag) printf("걸린 시간 : %8.6fms\n\n", time);


	printf("※ Muller(Made By Myself)\n");
	CHECK_TIME_START;
	for(i = 1 ; i <= nb ; i++)
		printf(" - %d번째 근 : %f\n", i, rtmuller(bessj0, xb1[i], (xb1[i] + xb2[i]) / 2.0f, xb2[i], XACC));
	CHECK_TIME_END(time, flag);
	if(flag) printf("걸린 시간 : %8.6fms\n\n", time);

	return 0;
}