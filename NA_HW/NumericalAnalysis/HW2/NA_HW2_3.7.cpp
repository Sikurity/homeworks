#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <math.h>

#define TRUE_VALUE 2.3529107926e6
#define X 5.77e-1

double abs(double num)
{
	return num < 0.0e0 ? -num : num;
}

int main()
{
	int ret;
	double relativeError;
	double numerator, denominator;

	printf("��ȿ���� �ٻ簪\t\t������\n");
	
	numerator = 6 * X;
	numerator = floor(numerator * 100) / 100;

	denominator = X * X;
	denominator = floor(denominator * 1000) / 1000;

	denominator *= 3;
	denominator = 1 - denominator;

	ret = numerator / (denominator * denominator);
	relativeError = 100 * abs(TRUE_VALUE - ret) / TRUE_VALUE;

	printf("   3��\t %d\t\t%.2lf%%\n", ret, relativeError);

	numerator = 6 * X;
	numerator = floor(numerator * 1000) / 1000;

	denominator = X * X;
	denominator = floor(denominator * 10000) / 10000;

	denominator *= 3;
	denominator = 1 - denominator;

	ret = numerator / (denominator * denominator);
	relativeError = 100 * abs(TRUE_VALUE - ret) / TRUE_VALUE;

	printf("   4��\t %d\t%.2lf%%\n", ret, relativeError);

	return 0;
}