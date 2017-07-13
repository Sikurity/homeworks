#define _CRT_SECURE_NO_WARNINGS
#define ANSI

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nr.h"

#define MAX_LEN 10
#define EPS		1.0e-6

float **A;
float **B;
float **X;
int N;

int main()
{
	int i, j;

	FILE *fp;

	fp = fopen("lineq3.dat", "r");
	fscanf(fp, "%d %d", &N, &N);

	A = (float **)malloc(sizeof(float *) * (N + 1));
	for( i = 1 ; i <= N ; i++)
		A[i] = (float *)malloc(sizeof(float) * (N + 1));

	B = (float **)malloc(sizeof(float *) * (N + 1));
	for(i = 1 ; i <= N ; i++)
		B[i] = (float *)malloc(sizeof(float) * 2);

	X = (float **)malloc(sizeof(float *) * (N + 1));
	for(i = 1 ; i <= N ; i++)
		X[i] = (float *)malloc(sizeof(float) * 2);

	for( i = 1 ; i <= N ; i++)
		for(j = 1 ; j <= N ; j++)
			fscanf(fp, "%f", &A[i][j]);

	for(i = 1 ; i <= N ; i++)
		fscanf(fp, "%f", &B[i][1]);

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
	printf("\nÇØ : <");
	for(i = 1 ; i <= N ; i++)
		printf("%f ", X[i][1]);
	printf("> T\n");

	for(i = 1 ; i <= N ; i++)
	{
		free(A[i]);
		free(B[i]);
		free(X[i]);
	}
	free(A);
	free(B);
	free(X);

	fclose(fp);

	return 0;
}