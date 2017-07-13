#define ANSI

#include <stdio.h>
#include <math.h>
#include "nr.h"

#define N		(100)
#define	MAX		(20)

#define W0	1.0f
#define W1	1000.0f

#define Z	75.0
#define R	225.0
#define C	6.0e-7
#define L	5.0e-1

#define XACC	0.001f

void nrerror(char error_text[])
{
	printf("%s\n", error_text);
}

float Func(float w)
{
	double tmp = w * C - 1.0f / (w * L);

	return sqrt(tmp * tmp + 1.0f / (R * R)) - (1 / Z);
}

int main()
{
	float xb1[MAX + 1], xb2[MAX + 1];
	int i, nb = MAX;

	printf("(a) Bisection\n");
	printf("±Ù : %f\n", rtbis(Func, W0, W1, XACC));

	//printf("(b) Linear Interpolation\n");
	//printf(" - ±Ù : %f\n", rtflsp(Func, W0, W1, XACC));

	zbrak(Func, W0, W1, N, xb1, xb2, &nb);
	printf("(b) Linear Interpolation\n");
	for(i = 1 ; i <= nb ; i++)
		printf("±Ù : %f\n", rtflsp(Func, xb1[i], xb2[i], XACC));

	return 0;
}