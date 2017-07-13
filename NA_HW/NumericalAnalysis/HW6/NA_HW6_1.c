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

#define A -2
#define B 3

unsigned int g_nHist[INTERVAL];

void nrerror(char error_text[])
{
	printf(error_text);
}

int main()
{
	int i;
	long seed = (long)time(NULL);

	memset(g_nHist, 0, sizeof(g_nHist));

	for(i = 0 ; i < REPEAT_N ; i++)
		g_nHist[(int)(ran1(&seed) * INTERVAL)]++;

	i = 0;
	do
	{
		printf("hist[%f] : %d\t", (49 - i) / 20.0f - 3.0f, g_nHist[(int)(49 - i)]);
		printf("hist[%f] : %d\n", (50 + i) / 20.0f - 3.0f, g_nHist[(int)(50 + i)]);
	}
	while(++i < (INTERVAL / 2));

	return 0;
}