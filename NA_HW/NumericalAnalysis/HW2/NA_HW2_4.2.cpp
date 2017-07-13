#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <math.h>

#define TRUE_VALUE 8.660254e-1

double abs(double num)
{
	return num < 0.0e0 ? -num : num;
}

int main()
{
	int cnt, i, n;
	double prev, cur, next, error;

	cnt = 0;
	prev = 0.0e0;
	cur = 0.0e0;
	error = 0.0e0;

	printf("TERM\t근사값\t\t상대근사오차\n", cnt, cur);

	do
	{
		next = 1.0e0;
		n = 2 * ++cnt - 1;
		for(i = 1 ; i <= n ; i++)
			next *= M_PI / (i * 3.0e0);

		cur += (cnt % 2 ? next : -next);

		if(prev)
		{
			error = 100 * abs(prev - cur) / prev;
			printf("%d\t%e\t%2.2lf%\n", cnt, cur, error);
		}
		else
		{
			error = INFINITY;
			printf("%d\t%e\tNONE\n", cnt, cur);
		}

		prev = cur;
	}
	while(error > 5.0e-1);

	return 0;
}