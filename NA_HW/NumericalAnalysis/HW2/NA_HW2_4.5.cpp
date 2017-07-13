#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <math.h>

#define BASE	1
#define X		3

#define ORIGINAL_FUNC	(((25 * BASE - 6) * BASE + 7) * BASE - 88)
#define DERIVATIVE_FUNC	((75 * BASE - 12) * BASE + 7)
#define SECOND_DER_FINC (150 * BASE - 12)
#define THIRD_DER_FUNC	(150)

#define TRUE_VALUE (554)

double abs(double num)
{
	return num < 0.0e0 ? -num : num;
}

int main()
{
	int i, cnt, square_part;
	double cur, next, error;

	square_part = 1;
	cnt = 0;
	cur = error = 0.0e0;

	printf("TERM\t근사값\t\t오차\n", cnt, cur);

	do
	{
		switch(cnt)
		{
		case 0:
			next = ORIGINAL_FUNC;
			break;
		case 1:
			next = DERIVATIVE_FUNC / (1);
			square_part *= X - BASE;
			break;
		case 2:
			next = SECOND_DER_FINC / (2 * 1);
			square_part *= X - BASE;
			break;
		case 3:
			next = THIRD_DER_FUNC / (3 * 2 * 1);
			square_part *= X - BASE;
			break;
		default:
			printf("Somethine Wring...\n");
			return 0;
		}

		cur += next * square_part;
		error = 100 * abs(TRUE_VALUE - cur) / TRUE_VALUE;

		printf("%d\t%lf\t%2.2lf%\n", cnt, cur, error);

		cnt++;
	}
	while(error);

	return 0;
}