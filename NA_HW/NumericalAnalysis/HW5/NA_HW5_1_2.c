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
int N;

int main()
{
	int i, j, *index;
	float d;

	FILE *fp;

	fp = fopen("lineq1.dat", "r");
	fscanf(fp, "%d %d", &N, &N);

	A = (float **)malloc(sizeof(float *) * (N + 1));
	for(i = 1 ; i <= N ; i++)
		A[i] = (float *)malloc(sizeof(float) * (N + 1));

	B = (float *)malloc(sizeof(float) * (N + 1));

	for(i = 1 ; i <= N ; i++)
		for(j = 1 ; j <= N ; j++)
			fscanf(fp, "%f", &A[i][j]);

	for(i = 1 ; i <= N ; i++)
		fscanf(fp, "%f", &B[i]);

	index = (int *)malloc(sizeof(int) * (N + 1));
	ludcmp(A, N, index, &d);
	lubksb(A, N, index, B);

	d = 1.0f;
	for(i = 1 ; i <= N ; i++)
		d *= A[i][i];

	if(fabs(d) < EPS)
		nrerror("Singular Matrix");

	printf("Nonsingular Matrix");
	printf("\nÇØ : <");
	for(i = 1 ; i <= N ; i++)
		printf("%f ", B[i]);
	printf("> T\n");

	for(i = 1 ; i <= N ; i++)
		free(A[i]);
	
	free(A);
	free(B);
	free(index);

	fclose(fp);

	return 0;
}