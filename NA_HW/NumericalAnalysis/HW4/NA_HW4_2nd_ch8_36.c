#define ANSI
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <math.h>
#include "nr.h"
#include "muller.h"

#define N		(100)
#define	MAX		(20)

#define W0	0.0f
#define W1	M_PI / 2.0f

#define X	35.0f
#define Y0	2.0f
#define Y	1.0f
#define V0	20.0f
#define G	9.81f

#define XACC	0.00001f

void nrerror(char error_text[])
{
	printf("%s\n", error_text);
}

float Func(float theta)
{
	double tanTheta = tan(theta);
	double cosTheta = cos(theta);

	return X * (tanTheta - G * X / (2 * V0 * V0 * cosTheta * cosTheta)) + Y0 - Y;
}

float Deriv(float theta)
{
	double tanTheta = tan(theta);
	double secTheta = 1.0f / cos(theta);

	return X * (1 - 0.858375 * tanTheta) * (secTheta * secTheta);
}

void FuncWithDeriv(float theta, float *fn, float *df)
{
	*fn = Func(theta);
	*df = Deriv(theta);
}

int main()
{
	float xb1[MAX + 1], xb2[MAX + 1];
	int i, nb = MAX;

	zbrak(Func, W0, W1, N, xb1, xb2, &nb);

	printf("(a) Bisection\n");
	for(i = 1 ; i <= nb ; i++)
		printf("%d 번째 - 근 : %f\n", i, rtbis(Func, xb1[i], xb2[i], XACC));

	printf("(b) Linear Interpolation\n");
	for(i = 1 ; i <= nb ; i++)
		printf("%d 번째 - 근 : %f\n", i, rtflsp(Func, xb1[i], xb2[i], XACC));

	printf("(c) Secant\n");
	for(i = 1 ; i <= nb ; i++)
		printf("%d 번째 - 근 : %f\n", i, rtsec(Func, xb1[i], xb2[i], XACC));

	printf("(d) Newton-Raphson\n");
	for(i = 1 ; i <= nb ; i++)
		printf("%d 번째 - 근 : %f\n", i, rtnewt(FuncWithDeriv, xb1[i], xb2[i], XACC));

	printf("(e) Newton With Bracketing\n");
	for(i = 1 ; i <= nb ; i++)
		printf("%d 번째 - 근 : %f\n", i, rtsafe(FuncWithDeriv, xb1[i], xb2[i], XACC));

	printf("(*) My Muller\n");
	for(i = 1 ; i <= nb ; i++)
		printf("%d 번째 - 근 : %f\n", i, rtmuller(Func, xb1[i], (xb1[i] + xb2[i]) / 2.0f, xb2[i], XACC));

	return 0;
}