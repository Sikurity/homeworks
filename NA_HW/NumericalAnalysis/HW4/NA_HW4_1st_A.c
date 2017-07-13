#define ANSI

#include <stdio.h>
#include <math.h>
#include "nr.h"

#define R0		0.0f
#define R1		400.0f
#define XACC	0.0001f

void nrerror(char error_text[])
{
	printf("%s\n", error_text);
}

float Func(float r)
{
	return exp(-0.005 * r) * cos(sqrt(2000 - 0.01 * r * r ) * 0.05) - 0.01;
}

float Deriv(float r)
{
	return (0.0005 * exp(-0.005 * r) * r * sin(0.05 * sqrt(2000 - 0.01 * r * r))) / sqrt(2000 - 0.01 * r * r) - 0.005 * exp(-0.005 * r) * cos(0.05 * sqrt(2000 - 0.01 * r * r));
}

void FuncWithDeriv(float x, float *fn, float *df)
{
	*fn = Func(x);
	*df = Deriv(x);
}

int main()
{
	printf("(a) Bisection\n");
	printf("�� : %f\n", rtbis(Func, R0, R1, XACC));

	printf("(b) Linear Interpolation\n");
	printf("�� : %f\n", rtflsp(Func, R0, R1 , XACC));

	printf("(c) Secant\n");
	printf("�� : %f\n", rtsec(Func, R0, R1 , XACC));

	printf("(d) Newton-Raphson\n");
	printf("�� : %f\n", rtnewt(FuncWithDeriv, R0, R1 , XACC));

	printf("(e) Newton With Bracketing\n");
	printf("�� : %f\n", rtsafe(FuncWithDeriv, R0, R1 , XACC));

	return 0;
}