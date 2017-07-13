#define NRANSI

#include <stdio.h>
#include <math.h>
#include "nr.h"
#include "nrutil.h"

#define NPT		100
#define MA		9
#define SPREAD	0.001
#define MAX_ITER 100

int main(void)
{
	long idum = (-911);
	int i, *ia, iter, itst, j, k, mfit = MA;
	float alamda, chisq, ochisq, *x, *y, *sig, **covar, **alpha;
	static float a[MA + 1]		= {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0}; // 예측 결과
	static float gues[MA + 1]	= {0.0, 1.1, 2.2, 2.8, 3.7, 5.1, 5.8, 6.8, 8.3, 9.1}; // 초기 추측 값

	ia = ivector(1, MA);
	x = vector(1, NPT);
	y = vector(1, NPT);
	sig = vector(1, NPT);
	covar = matrix(1, MA, 1, MA);
	alpha = matrix(1, MA, 1, MA);

	for(i = 1; i <= NPT; i++) 
	{
		x[i] = 0.1*i;
		y[i] = 0.0;

		for(j = 1; j <= MA; j += 3)
			y[i] += a[j] * exp(-SQR((x[i] - a[j + 1]) / a[j + 2]));

		y[i] *= (1.0 + SPREAD * gasdev(&idum));
		sig[i] = SPREAD * y[i];
	}

	printf("원형 데이터 x\n");
	for(i = 1 ; i <= NPT ; i++)
	{
		printf("%.3e ", x[i]);
		if(i % 10 == 0)
			puts("");
	}

	/******************* Outlier 적용 *******************/
	//y[NPT / 2] += gasdev(&idum) * 0.1;		// 1%
	
	//for(i = 1 ; i <= NPT ; i += 20)			// 5%
	//	y[i] += gasdev(&idum) * 0.1;
	
	//for( i = 1 ; i <= NPT ; i += 5)			// 20%
	//	y[i] += gasdev(&idum) * 0.1;

	printf("/**************************************************************************************************/\n");
	printf("변환 데이터 x`\n");
	for(i = 1 ; i <= NPT ; i++)
	{
		printf("%.3e ", y[i]);
		if(i % 10 == 0)
			puts("");
	}

	for(i = 1; i <= mfit; i++) 
		ia[i] = 1;

	for(i = 1; i <= MA; i++) 
		a[i] = gues[i];

	alamda = -1;
	mrqmin(x, y, sig, NPT, a, ia, MA, covar, alpha, &chisq, fgauss, &alamda);
	k = 1;
	itst = 0;

	iter = 0;
	while( iter++ < MAX_ITER )
	{
		printf("\n%s %2d %17s %10.4f %10s %9.2e\n", "Iteration #", k, "chi-squared:", chisq, "alamda:", alamda);
		printf("%8s %8s %8s %8s %8s %8s %8s %8s %8s\n", "a[1]", "a[2]", "a[3]", "a[4]", "a[5]", "a[6]", "a[6]", "a[7]", "a[8]");

		for(i = 1; i <= MA; i++)
			printf("%9.4f", a[i]);
		printf("\n");

		k++;
		ochisq = chisq;
		mrqmin(x, y, sig, NPT, a, ia, MA, covar, alpha, &chisq, fgauss, &alamda);

		if(chisq > ochisq)
			itst = 0;
		else if(fabs(ochisq - chisq) < 0.1)
			itst++;

		if(itst < 4)
			continue;

		alamda = 0.0;
		mrqmin(x, y, sig, NPT, a, ia, MA, covar, alpha, &chisq, fgauss, &alamda);

		printf("\nExpected results:\n");
		printf(" %7.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f\n", 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);

		printf("\nUncertainties:\n");
		for(i = 1; i <= MA; i++)
			printf("%9.4f", sqrt(covar[i][i]));
		printf("\n");

		break;
	}

	free_matrix(alpha, 1, MA, 1, MA);	// 메모리 해제
	free_matrix(covar, 1, MA, 1, MA);	// 메모리 해제
	free_vector(sig, 1, NPT);			// 메모리 해제
	free_vector(y, 1, NPT);				// 메모리 해제
	free_vector(x, 1, NPT);				// 메모리 해제
	free_ivector(ia, 1, MA);			// 메모리 해제

	return 0;
}