#define ANSI

#include <stdio.h>
#include "nr.h"
#include "muller.h"

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

	zbrak(bessj0, X1, X2, N, xb1, xb2, &nb);

	printf("(a) Bisection\n");
	for(i = 1 ; i <= nb ; i++)
		printf(" - %d번째 근 : %lf\n", i, rtbis(bessj0, xb1[i], xb2[i], XACC));

	printf("(b) Linear Interpolation\n");
	for(i = 1 ; i <= nb ; i++)
		printf(" - %d번째 근 : %lf\n", i, rtflsp(bessj0, xb1[i], xb2[i], XACC));

	printf("(c) Secant\n");
	for(i = 1 ; i <= nb ; i++)
		printf(" - %d번째 근 : %lf\n", i, rtsec(bessj0, xb1[i], xb2[i], XACC));

	printf("(d) Newton-Raphson\n");
	for(i = 1 ; i <= nb ; i++)
		printf(" - %d번째 근 : %lf\n", i, rtnewt(Bessel_J_Func, xb1[i], xb2[i], XACC));

	printf("(e) Newton With Bracketing\n");
	for(i = 1 ; i <= nb ; i++)
		printf(" - %d번째 근 : %lf\n", i, rtsafe(Bessel_J_Func, xb1[i], xb2[i], XACC));

	printf("※ Muller(Made By Myself)\n");
	for(i = 1 ; i <= nb ; i++)
		printf(" - %d번째 근 : %lf\n", i, rtmuller(bessj0, xb1[i], (xb1[i] + xb2[i]) / 2.0, xb2[i], XACC));

	return 0;
}