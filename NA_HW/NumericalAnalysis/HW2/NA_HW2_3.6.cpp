#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>

#define TRUE_VALUE 6.737947e-3

double R[2][21];

double abs(double num)
{
	return num < 0.0e0 ? -num : num;
}

void EvaluteValue1()
{
	double tmp, ret;
	int i;

	tmp = ret = 1.0e0;

	for(i = 1 ; i <= 20 ; i++)
	{
		tmp *= -5.0e0 / (double)i;
		ret += tmp;

		R[0][i] = ret;
	}
}

void EvaluteValue2()
{
	double tmp, ret;
	int i;

	tmp = ret = 1.0e0;

	for(i = 1 ; i <= 20 ; i++)
	{
		tmp *= 5.0e0 / (double)i;
		ret += tmp;

		R[1][i] = 1.0e0 / ret;
	}
}

int main()
{
	int i;

	R[0][0] = R[1][0] = 1.0e0;

	EvaluteValue1();
	EvaluteValue2();

	for(i = 0 ; i <= 20 ; i++)
	{
		if(i)
			printf("%d\t%e\t%e\t%e\t   %e\t%e\t%e\n", i, R[0][i], 100 * abs(R[0][i] / TRUE_VALUE), 100 * abs((R[0][i] - R[0][i - 1]) / R[0][i]), R[1][i], 100 * abs(R[1][i] / TRUE_VALUE), 100 * abs((R[1][i] - R[1][i - 1]) / R[1][i]));
		else
			printf("Term\tMethod 1\tTrue Erros 1(%%)\tApx Rel Erros 1(%%) Method 2\tTrueErros 2(%%)\tApxRelErros 2(%%)\n");
	}

	return 0;
}