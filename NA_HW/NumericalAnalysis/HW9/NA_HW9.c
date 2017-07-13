#define _CRT_SECURE_NO_WARNINGS
#define ANSI

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "nr.h"

#define FLT_MIN		1E-10
#define POINT_NUM	77

float **NewMat(int rownum, int colnum)
{
	int i;
	float **ret;

	ret = (float **)malloc((rownum + 1) * sizeof(float*));

	for(i = 0; i <= rownum; i++)
		ret[i] = (float *)calloc(sizeof(float), colnum + 1);

	return ret;
}

float *NewVec(int num)
{
	float *ret;

	ret = (float *)calloc(sizeof(float), num + 1);

	return ret;
}

void MulMat(float **left, float **right, int a, int b, int c, float **ret)
{
	int i, j, k;

	for(i = 1 ; i <= a ; i++) 
	{
		for(j = 1; j <= c ; j++) 
		{
			ret[i][j] = 0.0f;

			for(k = 1 ; k <= b ; k++)
				ret[i][j] += left[i][k] * right[k][j];
		}
	}
}

void FreeMat(float **m, int rownum)
{
	int i;

	for( i = 0 ; i <= rownum ; i++ )
		free(m[i]);

	free(m);
}

void FreeVec(float *m)
{
	free(m);
}

void GetAffineMatrix(float **from, float **to, int point_num, float **ret)
{
	int i, j;
	float **tmp1, **tmp2, **F, **T, **V, **V_T, **INV_SIGMA, *SV;

	F			= NewMat(point_num, 3);
	T			= NewMat(3, point_num);
	V			= NewMat(3, 3);
	V_T			= NewMat(3, 3);
	INV_SIGMA	= NewMat(3, 3);
	SV			= NewVec(3);
	tmp1		= NewMat(3, 3);
	tmp2		= NewMat(3, 3);

	// 초기화
	for(i = 1; i <= 3; i++)
	{
		for(j = 1; j <= point_num; j++)
		{
			F[j][i] = from[i][j];
			T[i][j] = to[i][j];
		}
	}

	// U -> F
	svdcmp(F, point_num, 3, SV, V);

	// inv(SIGMA) -> INV_SIGMA
	for(i = 1; i <= 3; i++)
	{
		for(j = 1; j <= 3; j++)
		{
			if( i == j && SV[i] > FLT_MIN )
				INV_SIGMA[i][i] = 1.0f / SV[i];
			else
				INV_SIGMA[i][j] = 0.0f;
		}
	}

	// Transpose V -> V_T
	for(i = 1; i <= 3; i++)
		for(j = 1; j <= 3; j++)
			V_T[i][j] = V[j][i];

	// ret = T * U * INV_SIGMA * V_T
	MulMat(INV_SIGMA, V_T, 3, 3, 3, tmp1);
	MulMat(T, F, 3, point_num, 3, tmp2);
	MulMat(tmp2, tmp1, 3, 3, 3, ret);

	ret[3][1] = 0.0f;
	ret[3][2] = 0.0f;
	ret[3][3] = 1.0f;

	// 메모리 해제
	FreeMat(tmp1, 3);
	FreeMat(tmp2, 3);
	FreeMat(F, point_num);
	FreeMat(T, 3);
	FreeMat(V, 3);
	FreeMat(V_T, 3);
	FreeMat(INV_SIGMA, 3);
	FreeVec(SV);
}

int main()
{
	int i;
	float **from, **to, **ret, **fromXret;

	FILE *fp = fopen("fitdata1.dat", "r"); // 파일 읽어오는 부분
	from = NewMat(3, POINT_NUM);
	to = NewMat(3, POINT_NUM);

	printf("/*********************************************************/\n");
	printf(" * FROM -> TO\n");
	for(i = 1 ; i <= POINT_NUM ; i++)
	{
		fscanf(fp, "%f %f %f %f\n", &from[1][i], &from[2][i], &to[1][i], &to[2][i]);
		from[3][i] = to[3][i] = 1.0f;
		printf("(%.2f, %.2f, %.1f) -> (%.2f, %.2f, %.1f)\n", from[1][i], from[2][i], from[3][i], to[1][i], to[2][i], to[3][i]);
	}

	ret = NewMat(3, 3);
	GetAffineMatrix(from, to, POINT_NUM, ret); // 과제 핵심 구현함수 GetAffineMatrix

	printf("\n/*********************************************************/\n");
	printf(" * Affine Matrix\n");
	for(i = 1 ; i <= 3 ; i++)
		printf("%f %f %f\n", ret[i][1], ret[i][2], ret[i][3]);

	fromXret = NewMat(3, POINT_NUM);
	MulMat(ret, from, 3, 3, 77, fromXret);

	printf("\n/*********************************************************/\n");
	printf(" * FROM x AFFINE :: TO\n");
	for(i = 1 ; i <= POINT_NUM ; i++)
		printf("(%.2f, %.2f, %.1f) :: (%.2f, %.2f, %.1f)\n", fromXret[1][i], fromXret[2][i], fromXret[3][i], to[1][i], to[2][i], to[3][i]);

	fclose(fp);
	
	return 0;
}