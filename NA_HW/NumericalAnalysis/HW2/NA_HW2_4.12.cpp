#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#include <cstdio>
#include <cmath>

#define A		(1.00001e0)
#define B		(1.0e1)
#define C		(3.0e2)
#define D		(1.0e-3)
#define E		(1.0001e0 * M_PI)

#define PROBLEM_A_ORIGIN (sqrt(abs(A - 1)) + 1)
#define PROBLEM_B_ORIGIN (exp(-B))
#define PROBLEM_C_ORIGIN (sqrt(C * C + 1) - C)
#define PROBLEM_D_ORIGIN ((exp(-D) - 1) / D)
#define PROBLEM_E_ORIGIN (sin(E) / (1 + cos(E)))

#define PROBLEM_A_DERIVATIVE ((A - 1) / (2 * sqrt(abs(A - 1)) * abs(A - 1)))
#define PROBLEM_B_DERIVATIVE (-exp(-B))
#define PROBLEM_C_DERIVATIVE (C / sqrt(C * C + 1) - 1)
#define PROBLEM_D_DERIVATIVE ((exp(-D) * (-D + exp(D) - 1) / (D * D)))
#define PROBLEM_E_DERIVATIVE (1 / (1 + cos(E)))

#define TRUE_VALUE (554)

char *GetStatus(double cnum)
{
	return abs(cnum) < 1.0e0 ? "error reduction" : "ill-conditioned";
}

int main()
{
	double a_cnum = A * PROBLEM_A_DERIVATIVE / PROBLEM_A_ORIGIN;
	double b_cnum = B * PROBLEM_B_DERIVATIVE / PROBLEM_B_ORIGIN;
	double c_cnum = C * PROBLEM_C_DERIVATIVE / PROBLEM_C_ORIGIN;
	double d_cnum = D * PROBLEM_D_DERIVATIVE / PROBLEM_D_ORIGIN;
	double e_cnum = E * PROBLEM_E_DERIVATIVE / PROBLEM_E_ORIGIN;

	printf("(a) Condition Number at X = %lf is %lf, %s\n", A, a_cnum, GetStatus(a_cnum));
	printf("(b) Condition Number at X = %lf is %lf, %s\n", B, b_cnum, GetStatus(b_cnum));
	printf("(c) Condition Number at X = %lf is %lf, %s\n", C, c_cnum, GetStatus(c_cnum));
	printf("(d) Condition Number at X = %lf is %lf, %s\n", D, d_cnum, GetStatus(d_cnum));
	printf("(e) Condition Number at X = %lf is %lf, %s\n", E, e_cnum, GetStatus(e_cnum));

	return 0;
}