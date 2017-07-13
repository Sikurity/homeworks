#define ANSI

#include <stdio.h>
#include <math.h>
#include "muller.h"

#define MAXIT 100

float sign(float num)
{
	return (num <= 0.0f) ? -1.0f : 1.0f;
}

float rtmuller(float (*func)(float), float p0, float p1, float p2, float xacc)
{
	void nrerror(char error_text[]);
	float a, b, c, p3;
	float p0_1, p1_2, p0_2, p2_3;
	float f0, f1, f2, f3;
	int i;

	f0 = (*func)(p0);
	f1 = (*func)(p1);
	f2 = (*func)(p2);

	if(f0 == 0.0f)
	{
		printf("Iteration È½¼ö : 0 - ");
		return p0;
	}

	if(f1 == 0.0f)
	{
		printf("Iteration È½¼ö : 0 - ");
		return p1;
	}

	if(f2 == 0.0f)
	{
		printf("Iteration È½¼ö : 0 - ");
		return p2;
	}

	p0_1 = p0 - p1;
	p1_2 = p1 - p2;

	c = f2;

	for( i = 1 ; i <= MAXIT ; i++ )
	{
		p0_2 = p0 - p2;

		b = (p0_2 * p0_2 * (f1 - f2) - p1_2 * p1_2 * (f0 - f2)) / (p0_2 * p1_2 * p0_1);
		a = (p1_2 * p1_2 * (f0 - f2) - p0_2 * p0_2 * (f1 - f2)) / (p0_2 * p1_2 * p0_1);

		p3 = p2 - 2.0f * c / (b + sign(b) * (float)sqrt(b * b - 4.0f * a * c));
		f3 = func(p3);

		p2_3 = p2 - p3;
		if(fabs(p2_3) < xacc || f3 == 0.0)
		{
			printf("Iteration È½¼ö : %d - ", i);
			return p3;
		}

		f0 = f1;
		f1 = f2;
		f2 = f3;

		p0 = p1;
		p1 = p2;
		p2 = p3;

		p0_1 = p1_2;
		p1_2 = p2_3;

		c = f3;
	}
	nrerror("Maximum number of iterations exceeded in rtmuller");
	return 0.0;
}
#undef JMAX