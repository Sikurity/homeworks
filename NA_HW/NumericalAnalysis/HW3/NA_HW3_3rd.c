#define ANSI
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nr.h"
#include "gamma.h"

#define A0		(0.1f)
#define A1		(1.0f)

#define B0		(0.0f)
#define B1		(1.0f)

#define C0		(-2.0f)
#define C1		(-1.0f)

#define D0		(9.0f)
#define D1		(10.0f)

#define XACC	(0.000001f)

void nrerror(char error_text[])
{
	printf("%s\n", error_text);
}

float A_Func(float x)
{
	return 10 * exp(-x) * sin(2 * M_PI * x) - 2;
}

float B_Func(float x)
{
	return x * x + (exp(-x) - 2 * x) * exp(-x);
}

float C_Func(float x)
{
	return cos(x + sqrt(2.0f)) + x * (sqrt(2.0f) + x / 2);
}

float D_Func(float x)
{
	return gamma(x) - 100000.0f;
}

void A_Func_Derivative(float x, float *fn, float *df)
{
	*fn = A_Func(x);
	*df = 20 * M_PI * cos(2 * M_PI * x) - 10 * sin(2 * M_PI * x) * exp(-x);
}

void B_Func_Derivative(float x, float *fn, float *df)
{
	*fn = B_Func(x);
	*df = exp(-x) * (2 * x - 2 - exp(-x)) + 2 * x;
}

void C_Func_Derivative(float x, float *fn, float *df)
{
	*fn = C_Func(x);
	*df = x - sin(x + sqrt(2.0f)) + sqrt(2.0f);
}

void D_Func_Derivative(float x, float *fn, float *df)
{
	*fn = D_Func(x);
	*df = gamma(x) * digamma(x);
}

int main()
{
	printf("1. 10e^(-x) * sin(2¥ðx) - 2= 0, on [0.1, 1]\n");
	printf("TRUE VALUE = %f\n", 0.44926083f);
	printf("X when f(x) = 0 : %f\n\n", rtsafe(A_Func_Derivative, A0, A1, XACC));

	printf("2. x^2 - 2xe^(-x) + e^(-2x) = 0, on [0, 1]\n");
	printf("TRUE VALUE = %f\n", 0.56751098f);
	printf("X when f(x) = 0 : %f\n\n", rtsafe(B_Func_Derivative, B0, B1, XACC));

	printf("3. cos(x + sqrt(2)) + x ( x / 2 + sqrt(2)) = 0, on [-2, -1]\n");
	printf("TRUE VALUE = %f\n", -1.41421356f);
	printf("X when f(x) = 0 : %f\n\n", rtsafe(C_Func_Derivative, C0, C1, XACC));

	printf("4. ¥Ã(x) - 100000 = 0, on [9, 10]\n");
	printf("TRUE VALUE = %f\n", 9.41956817);
	printf("X when f(x) = 0 : %f\n\n", rtsafe(D_Func_Derivative, D0, D1, XACC));	

	return 0;
}