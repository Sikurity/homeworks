#define _CRT_SECURE_NO_WARNINGS
#define ANSI

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nr.h"

#define MAX_LEN 10
#define EPS		1.0e-5

float **A;
float *W, *B, *X;
float **V;
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

	W = (float *)malloc(sizeof(float) * (N + 1));
	B = (float *)malloc(sizeof(float) * (N + 1));
	X = (float *)malloc(sizeof(float) * (N + 1));

	V = (float **)malloc(sizeof(float *) * (N + 1));
	for(i = 1 ; i <= N ; i++)
		V[i] = (float *)malloc(sizeof(float) * (N + 1));

	for(i = 1 ; i <= N ; i++)
		for(j = 1 ; j <= N ; j++)
			fscanf(fp, "%f", &A[i][j]);

	for(i = 1 ; i <= N ; i++)
		fscanf(fp, "%f", &B[i]);

	svdcmp(A, N, N, W, V);
	svbksb(A, W, V, N, N, B, X);

	d = 1.0f;
	for(i = 1 ; i <= N ; i++)
		d *= W[i];

	if( abs(d) < EPS )
		nrerror("Singular Matrix");

	printf("Nonsingular Matrix");
	printf("\nÇØ : <");
	for(i = 1 ; i <= N ; i++)
		printf("%f ", X[i]);
	printf("> T\n");

	for(i = 1 ; i <= N ; i++)
	{
		free(A[i]);
		free(V[i]);
	}

	free(A);
	free(B);
	free(V);
	free(W);
	free(X);

	fclose(fp);

	return 0;
}