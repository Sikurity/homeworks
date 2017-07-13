/**
*	@date		2016. 09. 12 15:00
*	@author		CSE 2011004040 이영식
*	@method		1 + 2^(-n)이 1과 같게 되는 최대 자연수 n값을 찾는 get_eps() 함수를 직접 작성하여 사용
*	@direction	float 형의 Machine Accuracy를 확인하기 위해서는 #define DOUBLE 매크로 제거 or 주석처리
*				double 형의 Machine Accuracy를 확인하기 위해서는 #define DOUBLE 매크로 추가
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