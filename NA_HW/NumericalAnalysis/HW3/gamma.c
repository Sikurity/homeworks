#define _USE_MATH_DEFINES
#include <math.h>

long double frac(long double x)
{
	long double result;

	if(x >= 0.0)
		result = fabs(x) - floor(x);
	else
		result = -(fabs(x) - floor(x));

	return result;
}

float gamma(float y)
{
	long double pi = 3.1415926535897932385;
	long double sq2pi = 2.50662827463100050241577;  // sqrt(2Pi)
	long double g = 607. / 128;   // best resu'ts when 4<=g<=5
	long double t, w, gam, x = y - 1.0;
	int i, cg = 14;
	long double c[16];

	// Lanczos approximation for the complex plane
	// calculated using vpa digits(256)
	// the best set of coeffs was selected from
	// a solution space of g=0 to 32 with 1 to 32 terms
	// these coeffs really give superb performance
	// of 15 sig. digits for 0<=real(z)<=171
	// coeffs should sum to about g*g/2+23/24

	// http://www.numericana.com/answer/info/godfrey.htm

	c[1] = 0.99999999999999709182;   // thiBasic arrays start at 1 ?
	c[2] = 57.156235665862923517;
	c[3] = -59.597960355475491248;
	c[4] = 14.136097974741747174;
	c[5] = -0.49191381609762019978;
	c[6] = 0.33994649984811888699 / 10000;
	c[7] = 0.46523628927048575665 / 10000;
	c[8] = -0.98374475304879564677 / 10000;
	c[9] = 0.15808870322491248884 / 1000;
	c[10] = -0.21026444172410488319 / 1000;
	c[11] = 0.21743961811521264320 / 1000;
	c[12] = -0.16431810653676389022 / 1000;
	c[13] = 0.84418223983852743293 / 10000;
	c[14] = -0.26190838401581408670 / 10000;
	c[15] = 0.36899182659531622704 / 100000;

	// printf(" y = %LG\n", y);
	//  printf(" x = %LG\n", x);

	if(x < 0.0) {
		x = -x;
		if(frac(x) == 0.0) {
			gam = pow(10.0, 4932);
		}
		else {
			t = c[1];
			for(i = 1; i <= cg; i++) {
				t = t + c[i + 1] / (x + i);
			}
			w = x + g + 0.5;
			gam = pow(w, x + 0.5)*exp(-w)*sq2pi*t;
			gam = pi*x / (gam*sin(pi*x));
		}
	}
	else {
		t = c[1];
		for(i = 1; i <= cg; i++) {
			t = t + c[i + 1] / (x + i);
		}
		w = x + g + 0.5;
		// printf("   w = %LG\n", w);
		gam = pow(w, x + 0.5)*exp(-w)*sq2pi*t;
	}

	return (float)gam;
}

/*
*  Mathlib : A C Library of Special Functions
*  Copyright (C) 1998 Ross Ihaka
*  Copyright (C) 2000-2015 The R Core Team
*  Copyright (C) 2004-2009 The R Foundation
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, a copy is available at
*  https://www.R-project.org/Licenses/
*
*  AUTHOR
*
*    Amos, D. E.	(Fortran)
*    Ross Ihaka	(C Translation)
*    Martin Maechler   (x < 0, and psigamma())
*
*  REFERENCES
*
*    Handbook of Mathematical Functions,
*    National Bureau of Standards Applied Mathematics Series 55,
*    Edited by M. Abramowitz and I. A. Stegun, equations 6.3.5,
*    6.3.18, 6.4.6, 6.4.9 and 6.4.10, pp.258-260, 1964.
*
*    D. E. Amos, (1983). "A Portable Fortran Subroutine for
*    Derivatives of the Psi Function", Algorithm 610,
*    TOMS 9(4), pp. 494-502.
*
*    Routines called: Rf_d1mach, Rf_i1mach.
*/

#define N_MAX	(100)
#define EPS		(1.0e-18)
/* From R, currently only used for kode = 1, m = 1 : */
void dpsifn(double x, int n, int kode, int m, double *ans, int *nz, int *ierr)
{
	const static double bvalues[] = {	/* Bernoulli Numbers */
		1.00000000000000000e+00,
		-5.00000000000000000e-01,
		1.66666666666666667e-01,
		-3.33333333333333333e-02,
		2.38095238095238095e-02,
		-3.33333333333333333e-02,
		7.57575757575757576e-02,
		-2.53113553113553114e-01,
		1.16666666666666667e+00,
		-7.09215686274509804e+00,
		5.49711779448621554e+01,
		-5.29124242424242424e+02,
		6.19212318840579710e+03,
		-8.65802531135531136e+04,
		1.42551716666666667e+06,
		-2.72982310678160920e+07,
		6.01580873900642368e+08,
		-1.51163157670921569e+10,
		4.29614643061166667e+11,
		-1.37116552050883328e+13,
		4.88332318973593167e+14,
		-1.92965793419400681e+16
	};

	int i, j, k, mm, mx, nn, np, nx, fn;
	double arg, den, elim, eps, fln, fx, rln, rxsq,
		r1m4, r1m5, s, slope, t, ta, tk, tol, tols, tss, tst,
		tt, t1, t2, wdtol, xdmln, xdmy, xinc, xln = 0.0 /* -Wall */,
		xm, xmin, xq, yint;
	double trm[23], trmr[N_MAX + 1];

	*ierr = 0;
	if(n < 0 || kode < 1 || kode > 2 || m < 1) {
		*ierr = 1;
		return;
	}
	if(x <= 0.) {
		/* use	Abramowitz & Stegun 6.4.7 "Reflection Formula"
		*	psi(k, x) = (-1)^k psi(k, 1-x)	-  pi^{n+1} (d/dx)^n cot(x)
		*/
		if(x == round(x)) {
			/* non-positive integer : +Inf or NaN depends on n */
			for(j = 0; j < m; j++) /* k = j + n : */
				ans[j] = ((j + n) % 2) ? INFINITY : NAN;
			return;
		}
		/* This could cancel badly */
		dpsifn(1. - x, n, /*kode = */ 1, m, ans, nz, ierr);
		/* ans[j] == (-1)^(k+1) / gamma(k+1) * psi(k, 1 - x)
		*	     for j = 0:(m-1) ,	k = n + j
		*/

		/* Cheat for now: only work for	 m = 1, n in {0,1,2,3} : */
		if(m > 1 || n > 3) {/* doesn't happen for digamma() .. pentagamma() */
							/* not yet implemented */
			*ierr = 4; return;
		}
		x *= M_PI; /* pi * x */
		if(n == 0)
			tt = cos(x) / sin(x);
		else if(n == 1)
			tt = -1 / pow(sin(x), 2);
		else if(n == 2)
			tt = 2 * cos(x) / pow(sin(x), 3);
		else if(n == 3)
			tt = -2 * (2 * pow(cos(x), 2) + 1.) / pow(sin(x), 4);
		else /* can not happen! */
			tt = NAN;
		/* end cheat */

		s = (n % 2) ? -1. : 1.;/* s = (-1)^n */
							   /* t := pi^(n+1) * d_n(x) / gamma(n+1)	, where
							   *		   d_n(x) := (d/dx)^n cot(x)*/
		t1 = t2 = s = 1.;
		for(k = 0, j = k - n; j < m; k++, j++, s = -s) {
			/* k == n+j , s = (-1)^k */
			t1 *= M_PI;/* t1 == pi^(k+1) */
			if(k >= 2)
				t2 *= k;/* t2 == k! == gamma(k+1) */
			if(j >= 0) /* by cheat above,  tt === d_k(x) */
				ans[j] = s*(ans[j] + t1 / t2 * tt);
		}
		if(n == 0 && kode == 2) /* unused from R, but "wrong": xln === 0 :*/
			ans[0] += xln;
		return;
	} /* x <= 0 */

	  /* else :  x > 0 */
	*nz = 0;
	xln = log(x);
	if(kode == 1 && m == 1) {/* the R case  ---  for very large x: */
		double lrg = 1 / (2. * EPS);
		if(n == 0 && x * xln > lrg) {
			ans[0] = -xln;
			return;
		}
		else if(n >= 1 && x > n * lrg) {
			ans[0] = exp(-n * xln) / n; /* == x^-n / n  ==  1/(n * x^n) */
			return;
		}
	}
	mm = m;
	nx = 1021;
	r1m5 = log10l(2.0e0);
	r1m4 = EPS * 0.5;
	wdtol = fmax(r1m4, 0.5e-18); /* 1.11e-16 */

								  /* elim = approximate exponential over and underflow limit */
	elim = 2.302 * (nx * r1m5 - 3.0);/* = 700.6174... */
	for(;;) {
		nn = n + mm - 1;
		fn = nn;
		t = (fn + 1) * xln;

		/* overflow and underflow test for small and large x */

		if(fabs(t) > elim) {
			if(t <= 0.0) {
				*nz = 0;
				*ierr = 2;
				return;
			}
		}
		else {
			if(x < wdtol) {
				ans[0] = pow(x, -n - 1);
				if(mm != 1) {
					for(k = 1; k < mm ; k++)
						ans[k] = ans[k - 1] / x;
				}
				if(n == 0 && kode == 2)
					ans[0] += xln;
				return;
			}

			/* compute xmin and the number of terms of the series,  fln+1 */

			rln = r1m5 * 16;
			rln = fmin(rln, 18.06);
			fln = fmax(rln, 3.0) - 3.0;
			yint = 3.50 + 0.40 * fln;
			slope = 0.21 + fln * (0.0006038 * fln + 0.008677);
			xm = yint + slope * fn;
			mx = (int)xm + 1;
			xmin = mx;
			if(n != 0) {
				xm = -2.302 * rln - fmin(0.0, xln);
				arg = xm / n;
				arg = fmin(0.0, arg);
				eps = exp(arg);
				xm = 1.0 - eps;
				if(fabs(arg) < 1.0e-3)
					xm = -arg;
				fln = x * xm / eps;
				xm = xmin - x;
				if(xm > 7.0 && fln < 15.0)
					break;
			}
			xdmy = x;
			xdmln = xln;
			xinc = 0.0;
			if(x < xmin) {
				nx = (int)x;
				xinc = xmin - nx;
				xdmy = x + xinc;
				xdmln = log(xdmy);
			}

			/* generate w(n+mm-1, x) by the asymptotic expansion */

			t = fn * xdmln;
			t1 = xdmln + xdmln;
			t2 = t + xdmln;
			tk = fmax(fabs(t), fmax(fabs(t1), fabs(t2)));
			if(tk <= elim) /* for all but large x */
				goto L10;
		}
		nz++; /* underflow */
		mm--;
		ans[mm] = 0.;
		if(mm == 0)
			return;
	} /* end{for()} */
	nn = (int)fln + 1;
	np = n + 1;
	t1 = (n + 1) * xln;
	t = exp(-t1);
	s = t;
	den = x;
	for(i = 1; i <= nn; i++) {
		den += 1.;
		trm[i] = pow(den, (double)-np);
		s += trm[i];
	}
	ans[0] = s;
	if(n == 0 && kode == 2)
		ans[0] = s + xln;

	if(mm != 1) { /* generate higher derivatives, j > n */

		tol = wdtol / 5.0;
		for(j = 1; j < mm; j++) {
			t /= x;
			s = t;
			tols = t * tol;
			den = x;
			for(i = 1; i <= nn; i++) {
				den += 1.;
				trm[i] /= den;
				s += trm[i];
				if(trm[i] < tols)
					break;
			}
			ans[j] = s;
		}
	}
	return;

L10:
	tss = exp(-t);
	tt = 0.5 / xdmy;
	t1 = tt;
	tst = wdtol * tt;
	if(nn != 0)
		t1 = tt + 1.0 / fn;
	rxsq = 1.0 / (xdmy * xdmy);
	ta = 0.5 * rxsq;
	t = (fn + 1) * ta;
	s = t * bvalues[2];
	if(fabs(s) >= tst) {
		tk = 2.0;
		for(k = 4; k <= 22; k++) {
			t = t * ((tk + fn + 1) / (tk + 1.0))*((tk + fn) / (tk + 2.0)) * rxsq;
			trm[k] = t * bvalues[k - 1];
			if(fabs(trm[k]) < tst)
				break;
			s += trm[k];
			tk += 2.;
		}
	}
	s = (s + t1) * tss;
	if(xinc != 0.0) {

		/* backward recur from xdmy to x */

		nx = (int)xinc;
		np = nn + 1;
		if(nx > N_MAX) {
			*nz = 0;
			*ierr = 3;
			return;
		}
		else {
			if(nn == 0)
				goto L20;
			xm = xinc - 1.0;
			fx = x + xm;

			/* this loop should not be changed. fx is accurate when x is small */
			for(i = 1; i <= nx; i++) {
				trmr[i] = pow(fx, (double)-np);
				s += trmr[i];
				xm -= 1.;
				fx = x + xm;
			}
		}
	}
	ans[mm - 1] = s;
	if(fn == 0)
		goto L30;

	/* generate lower derivatives,  j < n+mm-1 */

	for(j = 2; j <= mm; j++) {
		fn--;
		tss *= xdmy;
		t1 = tt;
		if(fn != 0)
			t1 = tt + 1.0 / fn;
		t = (fn + 1) * ta;
		s = t * bvalues[2];
		if(fabs(s) >= tst) {
			tk = 4 + fn;
			for(k = 4; k <= 22; k++) {
				trm[k] = trm[k] * (fn + 1) / tk;
				if(fabs(trm[k]) < tst)
					break;
				s += trm[k];
				tk += 2.;
			}
		}
		s = (s + t1) * tss;
		if(xinc != 0.0) {
			if(fn == 0)
				goto L20;
			xm = xinc - 1.0;
			fx = x + xm;
			for(i = 1 ; i <= nx ; i++) {
				trmr[i] = trmr[i] * fx;
				s += trmr[i];
				xm -= 1.;
				fx = x + xm;
			}
		}
		ans[mm - j] = s;
		if(fn == 0)
			goto L30;
	}
	return;

L20:
	for(i = 1; i <= nx; i++)
		s += 1. / (x + (nx - i)); /* avoid disastrous cancellation, PR#13714 */

L30:
	if(kode != 2) /* always */
		ans[0] = s - xdmln;
	else if(xdmy != x) {
		xq = xdmy / x;
		ans[0] = s - log(xq);
	}
	return;
} /* dpsifn() */

#ifdef MATHLIB_STANDALONE
# define ML_TREAT_psigam(_IERR_)	\
    if(_IERR_ != 0) {			\
	errno = EDOM;			\
	return NAN;			\
    }
#else
#define ML_TREAT_psigam(_IERR_)	\
    if(_IERR_ != 0)			\
	return NAN
#endif

float digamma(float x)
{
	double ans;
	int nz, ierr;
	if(x == NAN)
		return x;
	dpsifn(x, 0, 1, 1, &ans, &nz, &ierr);
	ML_TREAT_psigam(ierr);

	return (float)-ans;
}