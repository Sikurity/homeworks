#define _CRT_SECURE_NO_WARNINGS
#define ANSI

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nr.h"

#define MAX_LEN 10
#define EPS		1.0e-6

float **A, **DA, **B, *DB, **X;
int N;

int main()
{
	int i, j, *index;
	float d;

	FILE *fp;

	fp = fopen("lineq3.dat", "r");
	fscanf(fp, "%d %d", &N, &N);

	A = (float **)malloc(sizeof(float *) * (N + 1));
	for(i = 1 ; i <= N ; i++)
		A[i] = (float *)malloc(sizeof(float) * (N + 1));

	DA = (float **)malloc(sizeof(float *) * (N + 1));
	for(i = 1 ; i <= N ; i++)
		DA[i] = (float *)malloc(sizeof(float) * (N + 1));

	B = (float **)malloc(sizeof(float *) * (N + 1));
	for(i = 1 ; i <= N ; i++)
		B[i] = (float *)malloc(sizeof(float) * 2);

	DB = (float *)malloc(sizeof(float) * (N + 1));

	X = (float **)malloc(sizeof(float *) * (N + 1));
	for(i = 1 ; i <= N ; i++)
		X[i] = (float *)malloc(sizeof(float) * 2);

	for(i = 1 ; i <= N ; i++)
		for(j = 1 ; j <= N ; j++)
		{
			fscanf(fp, "%f", &A[i][j]);
			DA[i][j] = A[i][j];
		}

	for(i = 1 ; i <= N ; i++)
	{
		fscanf(fp, "%f", &B[i][1]);
	}

	for(i = 1 ; i <= N ; i++)
		X[i][1] = B[i][1];

	gaussj(A, N, B, 1);

	for(i = 1 ; i <= N ; i++)
		B[i][1] = X[i][1];

	for(i = 1 ; i <= N ; i++)
	{
		X[i][1] = 0.0f;
		for(j = 1 ; j <= N ; j++)
			X[i][1] += A[i][j] * B[j][1];
	}

	printf("Nonsingular Matrix");
	printf("\n해 : <");
	for(i = 1 ; i <= N ; i++)
		printf("%f ", X[i][1]);
	printf("> T\n");

	index = (int *)malloc(sizeof(int) * (N + 1));
	ludcmp(DA, N, index, &d);
	lubksb(DA, N, index, DB);

	printf("\n");
	d = 1.0f;
	for(i = 1 ; i <= N ; i++)
		d *= DA[i][i];
	printf("행렬식 : %.0f\n", d);

	printf("\n역행렬\n");
	for(i = 1 ; i <= N ; i++)
	{
		for(j = 1 ; j <= N ; j++)
		{
			if( A[i][j] >= 0 )
				printf(" ");
			printf("%f ", A[i][j]);
		}
		printf("\n");
	}

	for(i = 1 ; i <= N ; i++)
	{
		free(A[i]);
		free(B[i]);
		free(DA[i]);
		free(X[i]);
	}

	free(A);
	free(B);
	free(DA);
	free(DB);
	free(X);
	free(index);

	fclose(fp);

	return 0;
}