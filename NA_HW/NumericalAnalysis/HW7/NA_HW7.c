#define _CRT_SECURE_NO_WARNINGS
#define ANSI

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "nr.h"

#define SIZE 9

float **sym_M;
float **EVec;
float EVal[SIZE + 1];

int main()
{
	int i, j, nrot;
	long idum;

	sym_M = (float **)malloc(sizeof(float *) * (SIZE + 1));
	EVec = (float **)malloc(sizeof(float *) * (SIZE + 1));

	for(i = 1 ; i <= SIZE ; i++)	
	{
		sym_M[i] = (float *)calloc(SIZE + 1, sizeof(float));
		EVec[i] = (float *)calloc(SIZE + 1, sizeof(float));
	}

	for( i = 1 ; i <= SIZE ; i++ )
		for(j = i ; j <= SIZE ; j++)
		{
			sym_M[i][j] = gasdev(&idum);
			if( i != j )
				sym_M[j][i] = sym_M[i][j];
		}

	printf("������ ��Ī ���\n");
	for(i = 1 ; i <= SIZE ; i++)
	{
		for(j = 1 ; j <= SIZE ; j++)
		{
			if(sym_M[i][j] >= 0.0f )
				printf(" ");
			printf("%f ", sym_M[i][j]);
		}
		printf("\n");
	}
	printf("\n");

	jacobi(sym_M, SIZE, EVal, EVec, &nrot);
	eigsrt(EVal, EVec, SIZE);

	printf("%dȸ ȸ�� ���\n", nrot);
	for(i = 1 ; i <= SIZE ; i++)
	{
		printf("������ : %f\t�������� : [ ", EVal[i]);
		for(j = 1 ; j <= SIZE ; j++)
		{
			if(EVec[i][j] >= 0.0f)
				printf(" ");
			printf("%f ", EVec[i][j]);
		}
		printf("]\n");
	}

	printf("\n");

	return 0;
}