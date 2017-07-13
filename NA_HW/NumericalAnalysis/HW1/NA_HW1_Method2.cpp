/**
*	@date		2016. 09. 12 15:00
*	@author		CSE 2011004040 �̿���
*	@method		1 + 2^(-n)�� 1�� ���� �Ǵ� �ִ� �ڿ��� n���� ã�� get_eps() �Լ��� ���� �ۼ��Ͽ� ���
*	@direction	float ���� Machine Accuracy�� Ȯ���ϱ� ���ؼ��� #define DOUBLE ��ũ�� ���� or �ּ�ó��
*				double ���� Machine Accuracy�� Ȯ���ϱ� ���ؼ��� #define DOUBLE ��ũ�� �߰�
*/

#define _CRT_NO_SECURE_WARNINGS

#include <stdio.h>

#ifdef DOUBLE
#define REAL	double
#define ONE		1.0e0
#define SIZE	2
#else
#define REAL	float
#define ONE		1.0f
#define SIZE	1
#endif

typedef union _MEMORY{
	int memory[SIZE];
	REAL value;
} MEMORY;

void get_eps(int *n, MEMORY *delta)
{
	delta->value = ONE;

	*n = 0;

	while((ONE + (delta->value *= 0.5f)) != ONE)
		(*n)++;
}

int main()
{
	int i, n;
	MEMORY delta;

	get_eps(&n, &delta);
	printf("n : %d, eps :%24.16e", n, delta.value);
	for( i = 0 ; i < SIZE ; i++ )
		printf(" %9X", delta.memory[i]);

	puts("\n");

	return 0;
}