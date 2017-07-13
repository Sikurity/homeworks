#define _CRT_SECURE_NO_WARNINGS
#define ANSI

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "nr.h"

#define REPEAT_N 100000
#define INTERVAL 100

#define M 0.5f
#define S 1.25f

unsigned int g_nHist[INTERVAL + 1];

void nrerror(char error_text[])
{
	printf(error_text);
}

int main()
{
	int i, sum = 0;
	long seed = (long)time(NULL);
	memset(g_nHist, 0, sizeof(g_nHist));

	for(i = 0 ; i < REPEAT_N ; i++)
		g_nHist[(int)(INTERVAL * (1.0f + erff((gasdev(&seed)) / 2.0f)) / 2.0f)]++;

	i = 0;
	do
	{
		printf("hist[%f] : %d\t", M - i * S, g_nHist[(int)(49 - i)]);
		printf("hist[%f] : %d\n", M + i * S, g_nHist[(int)(50 + i)]);
	}
	while(++i < (INTERVAL / 2));

	return 0;
}