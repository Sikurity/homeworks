#define _CRT_SECURE_NO_WARNINGS
#define ANSI

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nr.h"

#define MAX_LEN 10
#define EPS		1.0e-6

float **A;
float *B;
float *X;
int N;

int main()
{ 
	int i, j, *index;
	long idum = -13;
	float **alud, d;

	FILE *fp;

	fp = fopen("lineq3.dat", "r");
	fscanf(fp, "%d %d", &N, &N);

	A = (float **)malloc(sizeof(float *) * (N + 1));
	for(i = 1 ; i <= N ; i++)
		A[i] = (float *)malloc(sizeof(float) * (N + 1));

	alud = (float **)malloc(sizeof(float *) * (N + 1));
	for(i = 1 ; i <= N ; i++)
		alud[i] = (float *)malloc(sizeof(float) * (N + 1));

	B = (float *)malloc(sizeof(float) * (N + 1));
	X = (float *)malloc(sizeof(float) * (N + 1));

	for(i = 1 ; i <= N ; i++)
		for(j = 1 ; j <= N ; j++)
		{
			fscanf(fp, "%f", &A[i][j]);
			alud[i][j] = A[i][j];
		}

	for(i = 1 ; i <= N ; i++)
	{
		fscanf(fp, "%f", &B[i]);
		X[i] = B[i];
	}

	index = (int *)malloc(sizeof(int) * (N + 1));
	ludcmp(alud, N, index, &d);
	lubksb(alud, N, index, X);

	d = 1.0f;
	for(i = 1 ; i <= N ; i++)
		d *= alud[i][i];

	if(fabs(d) < EPS)
		nrerror("Singular Matrix");

	for(i = 1 ; i <= N ; i++)
		X[i] *= (1.0 + 0.2 * ran3(&idum));

	mprove(A, alud, N, index, B, X);

	printf("Nonsingular Matrix");
	printf("\nÇØ : <");
	for(i = 1 ; i <= N ; i++)
		printf("%f ", X[i]);
	printf("> T\n");

	for(i = 1 ; i <= N ; i++)
	{
		free(A[i]);
		free(alud[i]);
	}
	free(A);
	free(B);
	free(X);
	free(index);
	free(alud);

	fclose(fp);

	return 0;
}