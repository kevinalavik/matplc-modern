/*
 * (c) 2000 Mario de Sousa
 *
 * Offered to the public under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * This code is made available on the understanding that it will not be
 * used in safety-critical situations without a full and competent review.
 */

/*
 * filter.c
 *
 * this file implements the basic filter block
 */


  /* define __USE_GNU so we have the family of pow10() functions */
/* we could alternatively use */
#define pow10l(x) (powl(10, (x)))
/*
#define __USE_GNU
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sched.h>
#include <math.h>
#include <float.h>     /* required for LDBL_MAX */

#include <plc.h>
#include <misc/string_util.h>

#include "time_util.h"
#include "pid.h"




/*
 * This is a quick-n-dirty digital filter implementation.
 *
 * Limitations:
 *
 */


static const int debug = 0;





/***********************************************************/
/***********************************************************/
/**                                                       **/
/**                                                       **/
/**   I I R   F I L T E R   I M P L E M E N T A T I O N   **/
/**                                                       **/
/**                                                       **/
/***********************************************************/
/***********************************************************/




int filter_step(unsigned int step_count, dsp_time_t *dsp_time, fb_parm_t *parm)
/*
 *  The filter implements a series of the following second order canonical section:
 *
 *          |\
 *          | \
 *  in -----|C >-->(+)-->(+)----------------------->(+)-->(+)---> out
 *          | /     ^     ^               |          ^     |
 *          |/      |     |               |          |     |
 *                  |     |               |          |     |
 *                  |     |              z-1         |     |
 *                  |     |   /|    /|    |    |\    |     |
 *                  |     |  / |   / |    |    | \   |     |
 *                  |     --<-1|--<B1|<---|--->|A1>---     |
 *                  |        \ |   \ |    |    | /         |
 *                  |         \|    \|    |    |/          |
 *                  |                    z-1               |
 *                  |         /|    /|    |    |\          |
 *                  |        / |   / |    |    | \         |
 *                  |-------<-1|--<B2|<---|--->|A2>--------|
 *                           \ |   \ |         | /
 *                            \|    \|         |/
 *
 *
 *        where (z-1) is a delay block, i.e., in the z transform,
 *         z to the power of -1.
 *
 */
{
 f32 out_f32;
 long double tmp_ld, z0;
 int section_count;
 second_order_section_t *section;


 tmp_ld = plc_get_f32(parm->filter.in_pt);

 for (section_count = 0;
      section_count < parm->filter.num_sections;
      section_count++) {

   section = &(parm->filter.section[section_count]);

   z0 =     tmp_ld * section->C +
        section->z1 * -section->B1 +
        section->z2 * -section->B2;

   tmp_ld = z0 +
            section->z1 * section->A1 +
            section->z2 * section->A2;

   section->z2 = section->z1;
   section->z1 = z0;
 }

 out_f32 = (f32)tmp_ld;
 plc_set_f32 (parm->pid.out_pt, out_f32);

 return 0;
}










/***********************************************************/
/***********************************************************/
/**                                                       **/
/**                                                       **/
/**   C O M P L E X   N U M B E R   A R I T H M E T I C   **/
/**                                                       **/
/**                                                       **/
/***********************************************************/
/***********************************************************/




typedef struct {
	  long double re;
	  long double im;

	   /* indicates if the struct represents a valid pole/zero */
	   /* valid = 1, non-valid = 0 */
	  int valid;
	} complex;

#define sqr(x) ((x)*(x))

#define c_val(c) (c.valid)

static inline complex c_rect(long double re, long double im) {
  complex c;

  c.valid = 1;
  c.re = re;
  c.im = im;

  return c;
}


static inline complex c_polar(long double r, long double arg) {
  complex c;

  c.valid = 1;
  c.re = r * sinl(arg);
  c.im = r * cosl(arg);

  return c;
}


static inline complex c_add(complex c1, complex c2) {
  complex c;

  c.valid = c_val(c1) && c_val(c2);
  c.re = c1.re + c2.re;
  c.im = c1.im + c2.im;

  return c;
}


static inline complex c_sub(complex c1, complex c2) {
  complex c;

  c.valid = c_val(c1) && c_val(c2);
  c.re = c1.re - c2.re;
  c.im = c1.im - c2.im;

  return c;
}


static inline complex c_mul(complex c1, complex c2) {
  complex c;

  c.valid = c_val(c1) && c_val(c2);
  c.re = c1.re*c2.re - c1.im*c2.im;
  c.im = c1.re*c2.im + c1.im*c2.re;

  return c;
}


static inline complex c_div(complex c1, complex c2) {
  complex c;
  long double tmp_ld = sqr(c2.re) + sqr(c2.im);

  c.valid = c_val(c1) && c_val(c2);
  c.re = (c1.re*c2.re + c1.im*c2.im) / tmp_ld;
  c.im = (c1.im*c2.re - c1.re*c2.im) / tmp_ld;

  return c;
}


static inline complex c_sqrt(complex c1) {
  complex c;

  c.valid = c_val(c1);
  c.re = sqrtl( (sqrtl(sqr(c1.re) + sqr(c1.im)) + c1.re) / 2.0);
  c.im = sqrtl( (sqrtl(sqr(c1.re) + sqr(c1.im)) - c1.re) / 2.0);

  return c;
}


static inline complex c_sqr(complex c1) {
  return c_mul(c1, c1);
}


/***************************************************************************/
/***************************************************************************/
/**                                                                       **/
/**                                                                       **/
/**   A N A L O G   F I L T E R   P A R A M E T E R   C R U N C H I N G   **/
/**                                                                       **/
/**                                                                       **/
/***************************************************************************/
/***************************************************************************/



typedef enum {lowpass_fs,
	      highpass_fs,
	      bandpass_fs,
	      bandstop_fs
	     } filter_shape_t;


typedef enum {butterworth,
	      chebyshev,
	      elliptic
	     } filter_aprox_t;


/* Analog filter aproximation parameters. Used as the basis to calculate the */
/* IIR filter parameters                                                     */
typedef struct {
		/* parameters given by the user */
		filter_aprox_t filter_aprox;
		filter_shape_t filter_shape;
		f32 fp1;
		f32 fp2; /* only used for bandpass or bandstop filter shapes */
		f32 fs1;
		f32 fs2; /* only used for bandpass or bandstop filter shapes */
		f32 Ap;
		f32 As;
		f32 gain;
		/* parameters later determined by the program */
		int N; /* the order of the filter */
		complex *ns_poles;
		complex *ns_zeros;
		complex *s_poles;
		complex *s_zeros;
		complex *z_poles;
		complex *z_zeros;
		int num_ns_poles;
		int num_ns_zeros;
		int num_s_poles;
		int num_s_zeros;
		int num_z_poles;
		int num_z_zeros;
} analog_filter_parm_t;


/* helper function for butterworth, chebyshev and elliptic paramater crunching */
static long double filter_A(analog_filter_parm_t *fparm)
{
  return sqrtl((pow10l(fparm->As/10) - 1) / (pow10l(fparm->Ap/10) - 1));
}


/* helper function elliptic paramater crunching */
static long double filter_q(long double K) {
  long double q0 = (1 - powl(1 - K*K, 0.25)) / (2 * (1 + powl(1 - K*K, 0.25)));
  return  q0 + 2*powl(q0, 5) + 15*powl(q0, 9) + 150*powl(q0, 13);
}



/* helper function for butterworth, chebyshev and elliptic paramater crunching */
static long double filter_K(analog_filter_parm_t *fparm,
			    long double F /* sampling frequency */)
{
  long double Ka, Kb, Kc, K1, K2, tmp_ld;

  Ka = tanl(M_PI * fparm->fp2 / F) - tanl(M_PI * fparm->fp1 / F);
  Kb = tanl(M_PI * fparm->fp2 / F) * tanl(M_PI * fparm->fp1 / F);
  Kc = tanl(M_PI * fparm->fs2 / F) * tanl(M_PI * fparm->fs1 / F);
  tmp_ld = tanl(M_PI * fparm->fs1 / F);
  K1 = (Ka*tanl(M_PI * fparm->fs1 / F)) / (Kb - tmp_ld * tmp_ld);
  tmp_ld = tanl(M_PI * fparm->fs2 / F);
  K2 = (Ka*tanl(M_PI * fparm->fs2 / F)) / (tmp_ld * tmp_ld - Kb);

  switch (fparm->filter_shape) {
    case lowpass_fs:
      return tanl(M_PI * fparm->fp1 / F) / tanl(M_PI * fparm->fs1 / F);
    case highpass_fs:
      return tanl(M_PI * fparm->fs1 / F) / tanl(M_PI * fparm->fp1 / F);
    case bandpass_fs:
      if (Kc < Kb)
        return K2;
      else
        return K1;
    case bandstop_fs:
      if (Kc < Kb)
        return 1 / K1;
      else
        return 1 / K2;
  } /* switch */

  /* should never reach this point */
  return 0;
}



/* helper function for butterworth, chebyshev and elliptic paramater crunching */
/* Returns the minimum order of the transfer function                          */
static int filter_N(analog_filter_parm_t *fparm,
		    long double F /* sampling frequency */)
{
  long double A, K, q;

  A = filter_A(fparm);
  K = filter_K(fparm, F);
  q = filter_q(K);

  switch (fparm->filter_aprox) {
    case butterworth:
      return ceil(log10l(A) / log10l(1/K));
    case chebyshev:
      return ceil( acoshl(A) / acoshl(1/K));
    case elliptic:
      return ceil(log10l(16*A*A) / log10l(1/q));
    default:
      /* doesn't make sense to call this function for IIR and FIR filters */
      return -1;
  } /* switch */

  /* should never reach this point */
  return -1;
}



/* helper function for butterworth, chebyshev and elliptic paramater crunching */
/* Returns the frequency scaling parameter                                     */
/*
 * only valid for lowpass and highpass filter shapes
 */
static long double filter_alpha(analog_filter_parm_t *fparm,
				long double F /* sampling frequency */)
{
  long double pi;

  /* doesn't make sense to call this function for any other filters shape */
  if ((fparm->filter_shape != lowpass_fs) && (fparm->filter_shape != highpass_fs))
    return -1;

  pi = acosl(-1);

  switch (fparm->filter_aprox) {
    case butterworth:
      switch (fparm->filter_shape) {
        case lowpass_fs:
          return powl( pow10l(fparm->Ap / 10) - 1, -1.0L/(2*fparm->N)) * tanl(pi * fparm->fp1/ F);
        case highpass_fs:
          return powl( pow10l(fparm->Ap / 10) - 1,  1.0L/(2*fparm->N)) * tanl(pi * fparm->fp1/ F);
        default:
	  return -1;
      } /* switch */
    case chebyshev:
      return tanl(pi * fparm->fp1/ F);
    case elliptic:
      return sqrtl(tanl(pi * fparm->fp1/ F) * tanl(pi * fparm->fs1/ F));
    default:
      /* doesn't make sense to call this function for any other filters */
      return -1;
  } /* switch */

  /* should never reach this point */
  return -1;
}



/* helper function for butterworth, chebyshev and elliptic paramater crunching */
/* Returns the frequency scaling parameter                                     */
/*
 * only valid for bandpass and bandreject filter shapes
 */

static long double filter_beta(analog_filter_parm_t *fparm,
			       long double F /* sampling frequency */)
{
  /* doesn't make sense to call this function for any other filters shape */
  if ((fparm->filter_shape != bandpass_fs) && (fparm->filter_shape != bandstop_fs))
    return -1;

  switch (fparm->filter_aprox) {
    case butterworth:
      switch (fparm->filter_shape) {
        case bandpass_fs:
          return powl( pow10l(fparm->Ap / 10) - 1, -1.0L/(2*fparm->N));
        case bandstop_fs:
          return powl( pow10l(fparm->Ap / 10) - 1,  1.0L/(2*fparm->N));
        default:
	  return -1;
      } /* switch */
    case chebyshev:
      return 1;
    case elliptic:
      switch (fparm->filter_shape) {
        case bandpass_fs:
          return powl( filter_K(fparm, F), -0.5L);
        case bandstop_fs:
          return powl( filter_K(fparm, F),  0.5L);
        default:
	  return -1;
      } /* switch */
    default:
      /* doesn't make sense to call this function for any other filters */
      return -1;
  } /* switch */

  /* should never reach this point */
  return -1;
}




/* allocate memory for the pole and zero arrays */
static int filter_fparm_malloc(analog_filter_parm_t *fparm)
{
  complex *ptr;
  int i;
  int num_pole_zero_pairs = fparm->N;

  if ((fparm->filter_shape == bandpass_fs) || (fparm->filter_shape == bandstop_fs))
    num_pole_zero_pairs *= 2;

  if ((ptr = (complex *)malloc(6 * num_pole_zero_pairs * sizeof(complex))) == NULL)
    return -1;

  /* set every complex variable to invalid */
  for (i = 0; i < 6 * num_pole_zero_pairs; i++)
    c_val(ptr[i]) = 0;

  fparm->ns_poles = ptr; ptr += num_pole_zero_pairs;
  fparm->ns_zeros = ptr; ptr += num_pole_zero_pairs;
  fparm->s_poles  = ptr; ptr += num_pole_zero_pairs;
  fparm->s_zeros  = ptr; ptr += num_pole_zero_pairs;
  fparm->z_poles  = ptr; ptr += num_pole_zero_pairs;
  fparm->z_zeros  = ptr; ptr += num_pole_zero_pairs;

  return 0;
}


static int filter_fparm_mfree(analog_filter_parm_t *fparm)
{
  free(fparm->ns_poles);
  return 0;
}


/* Note: complex conjugate pairs must be on successive locations of the array
 *       In case of odd number of poles, the real pole with no complex conjugate pair
 *       must be placed in the last position of the array.
 */
static int filter_populate_ns_pole(analog_filter_parm_t *fparm,
				   long double F /* sampling frequency */)
{
  int i;

  switch (fparm->filter_aprox) {
    case butterworth: {
        long double pi_frac;
       	int num_pole_pairs = floor(fparm->N / 2.0);
	fparm->num_ns_poles = fparm->N;
        for (i = 0; i < num_pole_pairs; i++) {
	  pi_frac = acosl(-1) * ((2.0L*(i+1) - 1) / (2.0L*fparm->N));
          fparm->ns_poles[2*i    ].re = -sinl(pi_frac);
          fparm->ns_poles[2*i    ].im =  cosl(pi_frac);
          fparm->ns_poles[2*i + 1].re = -sinl(pi_frac);
          fparm->ns_poles[2*i + 1].im = -cosl(pi_frac);
        } /* for(;;) */

        /* if odd number of poles, then place the last real valued pole */
        if (fparm->N % 2 == 1) {
          fparm->ns_poles[fparm->N - 1].re = -1;
          fparm->ns_poles[fparm->N - 1].im = 0;
        };
      };
      break;

    case chebyshev: {
        long double pi_frac, sinh_y, cosh_y;
       	int num_pole_pairs = floor(fparm->N / 2.0);

	long double epsilon = sqrtl(pow10l(fparm->Ap/10.0) - 1);
	long double alpha = 1.0/epsilon + sqrtl(1.0/sqr(epsilon) + 1.0);;
	sinh_y = 0.5 * (powl(alpha, 1.0/fparm->N) - powl(alpha, -1.0/fparm->N));
	cosh_y = 0.5 * (powl(alpha, 1.0/fparm->N) + powl(alpha, -1.0/fparm->N));
	fparm->num_ns_poles = fparm->N;
        for (i = 0; i < num_pole_pairs; i++) {
	  pi_frac = acosl(-1)/*PI*/ * ((2.0L*(i+1) - 1) / (2.0L*fparm->N));
          fparm->ns_poles[2*i    ].re = -sinl(pi_frac) * sinh_y;
          fparm->ns_poles[2*i    ].im =  cosl(pi_frac) * cosh_y;
          fparm->ns_poles[2*i + 1].re = -sinl(pi_frac) * sinh_y;
          fparm->ns_poles[2*i + 1].im = -cosl(pi_frac) * cosh_y;
        } /* for(;;) */

        /* if odd number of poles, then place the last real valued pole */
        if (fparm->N % 2 == 1) {
          fparm->ns_poles[fparm->N - 1].re = -1.0 * sinh_y;
          fparm->ns_poles[fparm->N - 1].im = 0;
        };
      };
      break;

    case elliptic: {
	long double __FILTER_PRECISION = 1.0e-1;
       	int num_pole_pairs = floor(fparm->N / 2.0);
	int m;
	long double tmp_ld, sum1, sum2;
	long double sigma, W;
	long double u = ((fparm->N % 2) == 1)?0.0:0.5; /* 0 for N odd; 0.5 for N even */
	long double K = filter_K(fparm, F);
	long double q = filter_q(K);
	long double lambda = logl((pow10l(fparm->Ap*0.05)+1.0) / (pow10l(fparm->Ap*0.05)-1.0))
			      / (2.0 * fparm->N);

	for (m=0, sum1=0.0, tmp_ld = LDBL_MAX; /*fabsl(tmp_ld) >= fabsl(__FILTER_PRECISION*sum1)*/ m==0; m++) {
	  tmp_ld = powl(-1.0, m) * powl(q, m*(m+1.0)) * sinhl((2.0*m + 1.0) * lambda);
	  sum1 += tmp_ld;
	};
	for (m=1, sum2=0.0, tmp_ld = LDBL_MAX; /*fabsl(tmp_ld) >= fabsl(__FILTER_PRECISION*sum2)*/ m==0; m++) {
	  tmp_ld = powl(-1.0, m) * powl(q, sqr(m)) * coshl(2.0*m*lambda);
	  sum2 += tmp_ld;
	};
	sigma = - (2.0 * powl(q, 0.25) * sum1) / (1.0 + 2.0*sum2);

	W = sqrtl((1.0 + sqr(sigma)*K) * (1.0 + sqr(sigma)/K));

	fparm->num_ns_poles = fparm->N;
	fparm->num_ns_zeros = 2*num_pole_pairs;
        for (i = 0; i < num_pole_pairs; i++) {
	  long double omega, V;
  	  long double r = acosl(-1) /*PI*/ * (1.0 + i - u) / fparm->N;

	  for (m=0, sum1=0.0, tmp_ld = LDBL_MAX; fabsl(tmp_ld) >= fabsl(__FILTER_PRECISION*sum1); m++) {
	    tmp_ld = powl(-1.0, m) * powl(q, m*(m+1.0)) * sinl((2.0*m + 1.0) * r);
	    sum1 += tmp_ld;
	  };
	  for (m=1, sum2=0.0, tmp_ld = LDBL_MAX; fabsl(tmp_ld) >= fabsl(__FILTER_PRECISION*sum2); m++) {
	    tmp_ld = powl(-1.0, m) * powl(q, sqr(m)) * cosl(2.0*m*r);
	    sum2 += tmp_ld;
	  };
	  omega = (2.0 * powl(q, 0.25) * sum1) / (1.0 + 2.0*sum2);

	  V = sqrtl((1.0 - sqr(omega)*K) * (1.0 - sqr(omega)/K));

	  tmp_ld = 1.0 + sqr(sigma)*sqr(omega);

          fparm->ns_poles[2*i    ] = c_rect(sigma * V,   omega * W);
          fparm->ns_poles[2*i + 1] = c_rect(sigma * V, - omega * W);;
          fparm->ns_zeros[2*i    ] = c_rect(0,  1.0/omega);
          fparm->ns_zeros[2*i + 1] = c_rect(0, -1.0/omega);
	}

        /* if odd number of poles, then place the last real valued pole */
        if (fparm->N % 2 == 1) {
          fparm->ns_poles[fparm->N - 1] = c_rect(sigma, 0);
        };

      };
      break;

    default:
      return -1;
  } /* switch */
  return 0;
}


static int filter_populate_ns_zero(analog_filter_parm_t *fparm,
				   long double F /* sampling frequency */)
{
  switch (fparm->filter_aprox) {
    case butterworth:
       fparm->num_ns_zeros = 0;
       break;

    case chebyshev:
       fparm->num_ns_zeros = 0;
       break;

    default:
      return -1;
  } /* switch */

  return 0;
}




/* Note: includes prewarping! */
static int filter_ns_to_s(analog_filter_parm_t *fparm,
			  long double F  /* sampling frequency */
			 )
{
  int k;

  switch (fparm->filter_shape) {
    case lowpass_fs: {
        /* s = aplha * ns */
        complex alpha = c_rect(filter_alpha(fparm, F), 0.0);

        fparm->num_s_poles = fparm->num_ns_poles;
        fparm->num_s_zeros = fparm->num_ns_zeros;

        for (k = 0; k < fparm->num_s_poles; k++)
          fparm->s_poles[k] = c_mul(alpha, fparm->ns_poles[k]);

        for (k = 0; k < fparm->num_s_zeros; k++)
          fparm->s_zeros[k] = c_mul(alpha, fparm->ns_zeros[k]);
      }
      break;

    case highpass_fs: {
      /* s = aplha / ns */
        complex alpha = c_rect(filter_alpha(fparm, F), 0.0);

        fparm->num_s_poles = fparm->num_ns_poles;
        fparm->num_s_zeros = fparm->num_ns_zeros;
        /* For each pole, if no corresponding zero exists, then this transformation will create
           a new zero at the origin */
	if (fparm->num_s_zeros < fparm->num_s_poles)
          fparm->num_s_zeros = fparm->num_s_poles;

        for (k = 0; k < fparm->num_s_poles; k++) {
          fparm->s_poles[k] = c_div(fparm->ns_poles[k], alpha);

          if (c_val(fparm->s_zeros[k])) {
            fparm->s_zeros[k] = c_div(fparm->ns_zeros[k], alpha);
 	  } else {
	    /* create a new zero at the origin */
            fparm->s_zeros[k] = c_rect(0, 0);
	  }
        } /* for(;;) */
      }
      break;

    case bandstop_fs: {
	complex c_1 = c_rect( 1, 0);

        for (k = 0; k < fparm->num_ns_poles; k++)
	  fparm->ns_poles[k] = c_div(c_1, fparm->ns_poles[k]);
        for (k = 0; k < fparm->num_ns_zeros; k++)
	  fparm->ns_zeros[k] = c_div(c_1, fparm->ns_zeros[k]);
        }
        /* We *do not* stop here. Let it fall through to the bandpass_fs code... */
        /* break; */

    case bandpass_fs: {
	long double beta = filter_beta(fparm, F);
        long double re, im, c1, c2, c3, c4, c5;
	long double w0 = tanl(M_PI * fparm->fp2 / F) * tanl(M_PI * fparm->fp1 / F);
	long double wd = tanl(M_PI * fparm->fp2 / F) - tanl(M_PI * fparm->fp1 / F);
	int ofs;

        fparm->num_s_poles = 2 * fparm->num_ns_poles;
        fparm->num_s_zeros = 2 * fparm->num_ns_zeros;
	ofs = fparm->num_ns_poles;

        for (k = 0; k < fparm->num_ns_poles; k++) {
	/* this only works for poles with negative real values... */
/*          if (fparm->ns_poles[k].re > 0)
  	    return -1;
	  re = -fparm->ns_poles[k].re;
	  im =  fparm->ns_poles[k].im;
	  c1 = sqr(im) - sqr(re) + (4.0*w0) / (sqr(wd)*sqr(beta));
	  c2 = sqrtl(sqr(c1) + 4.0*sqr(re)*sqr(im));
	  c3 = sqrtl((c2 - c1) / 2.0);
	  c4 = sqrtl((c2 + c1) / 2.0);
  	  c5 = (beta * wd ) / 2.0;

          fparm->s_poles[k    ].re = c5 * (-fabsl(re) - c3);
          fparm->s_poles[k    ].im = c5 * (im + c4);
          fparm->s_poles[k+ofs].re = c5 * (-fabsl(re) + c3);
          fparm->s_poles[k+ofs].im = c5 * (im - c4);
*/
	  complex c_1p   = c_rect( 1, 0);
	  complex c_1n   = c_rect(-1, 0);
	  complex c_w0   = c_rect(sqrt(w0), 0);
	  complex c_wd   = c_mul(c_rect(wd, 0), c_rect(beta, 0));
	  complex r1 = c_mul(c_rect(0.5, 0), c_mul(fparm->ns_poles[k], c_wd));
	  complex r2 = c_sqr(c_div(c_w0, r1));
	  complex r3 = c_mul(c_1n, r2);
	  complex r4_p = c_sqrt(c_add(c_1p, r3));
	  complex r4_n = c_mul(c_1n, r4_p);
	  complex r5_p = c_add(c_1p, r4_p);
	  complex r5_n = c_add(c_1p, r4_n);
	  fparm->s_poles[k    ] = c_mul(r1, r5_p);
	  fparm->s_poles[k+ofs] = c_mul(r1, r5_n);

        } /* for(;;) */

        for (k = 0; k < fparm->num_ns_zeros; k++) {
	/* this only works for zeros with negative real values... */
          if (fparm->ns_zeros[k].re > 0)
  	    return -1;
	  re = -fparm->ns_zeros[k].re;
	  im =  fparm->ns_zeros[k].im;
	  c1 = sqr(im) - sqr(re) + (4.0*w0) / (sqr(wd)*sqr(beta));
	  c2 = sqrtl(sqr(c1) + 4.0*sqr(re)*sqr(im));
	  c3 = sqrtl((c2 - c1) / 2.0);
	  c4 = sqrtl((c2 + c1) / 2.0);
  	  c5 = (beta * wd ) / 2.0;

          fparm->s_zeros[k    ].re = c5 * (-fabsl(re) - c3);
          fparm->s_zeros[k    ].im = c5 * (im + c4);
          fparm->s_zeros[k+ofs].re = c5 * (-fabsl(re) + c3);
          fparm->s_zeros[k+ofs].im = c5 * (im - c4);
        } /* for(;;) */

	/* if no ns_zero exists for a ns_pole, then the bandpass transformation
	 * creates 1 new zero, at s = 0
	 */
	if (fparm->num_ns_zeros < fparm->num_ns_poles) {
          int tmp = fparm->num_s_zeros;
	  fparm->num_s_zeros += fparm->num_ns_poles - fparm->num_ns_zeros;
	  for (k = tmp; k < fparm->num_s_zeros; k++) {
	   fparm->s_zeros[k].re = 0;
	   fparm->s_zeros[k].im = 0;
	  }
	}
      };
      break;
  } /* switch */

  return 0;
}



/* s plane to z plane using the bilinear transformation */
/*
 *       1 + s
 *  z = -------
 *       1 - s
 *
 *  Note: s and z are complex variables.
 */
static int filter_bilinear_s_to_z(analog_filter_parm_t *fparm)
{
/*  long double tmp_ld, re, im; */
  complex c_1 = c_rect(1.0, 0);
  int k;

  fparm->num_z_poles = fparm->num_s_poles;
  fparm->num_z_zeros = fparm->num_s_zeros;
  /* For each pole, if no corresponding zero exists in the s plane,
     then the bilinear transformation creates a new zero, at z = -1
   */
  if (fparm->num_z_zeros < fparm->num_z_poles)
    fparm->num_z_zeros = fparm->num_z_poles;

  for (k = 0; k < fparm->num_s_poles; k++) {
/*
    re = fparm->s_poles[k].re;
    im = fparm->s_poles[k].im;
    tmp_ld = 1.0 - 2*re + sqr(re) + sqr(im);

    fparm->z_poles[k].re = (1.0 - sqr(re) - sqr(im)) / tmp_ld;
    fparm->z_poles[k].im = (2*im) / tmp_ld;
*/
    fparm->z_poles[k] = c_div(c_add(c_1, fparm->s_poles[k]), c_sub(c_1, fparm->s_poles[k]));


    if (c_val(fparm->s_zeros[k])) {
/*      re = fparm->s_zeros[k].re;
      im = fparm->s_zeros[k].im;
      tmp_ld = 1.0 - 2*re + sqr(re) + sqr(im);

      fparm->z_zeros[k].re = (1.0 - sqr(re) - sqr(im)) / tmp_ld;
      fparm->z_zeros[k].im = (2*im) / tmp_ld;
*/
    fparm->z_zeros[k] = c_div(c_add(c_1, fparm->s_zeros[k]), c_sub(c_1, fparm->s_zeros[k]));
    } else {
      /* insert a new zero at z = -1 */
      fparm->z_zeros[k] = c_rect(-1.0, 0);
    }
  } /* for(;;) */

  return 0;
}



static int filter_z_to_ABC(analog_filter_parm_t *fparm,
 			   long double F,  /* sampling frequency */
			   fb_filter_t *parm)
{
  int k, m, order;

  /* Note: the first poles actually represent a complex conjugate pair.
   *       if num_z_poles is odd, then the last is a real single pole...
   */
  parm->num_sections = ceil(fparm->num_z_poles / 2.0);
  parm->section = malloc(sizeof(second_order_section_t) * parm->num_sections);
  if (parm->section == NULL)
    return -1;

  for (k = 0; k < fparm->num_z_poles; k+=2) {
    order = ((k + 1) == fparm->num_z_poles)?1:2;
    m = k/2;

    /* initialize canonical filter 'memory' to zero. */
    parm->section[m].z1 = 0;
    parm->section[m].z2 = 0;

    /* set the section parameters */
    if (order == 1) {
      /* first order section */
      /* This means pole and zero are real numbers, i.e. have x_Im = 0 */
      if (fparm->z_zeros[k].im != 0)
        return -1;
      parm->section[m].A1 = - fparm->z_zeros[k].re;
      parm->section[m].A2 = 0;
      if (fparm->z_poles[k].im != 0)
        return -1;
      parm->section[m].B1 = - fparm->z_poles[k].re;
      parm->section[m].B2 = 0;
    };

    if (order == 2) {
      /* second order section */
        /* we only support complex conjugate pairs...*/
      if ((fparm->z_zeros[k].im + fparm->z_zeros[k+1].im) != 0)
        return -1;
      parm->section[m].A1 = - (fparm->z_zeros[k].re + fparm->z_zeros[k+1].re);
      parm->section[m].A2 = fparm->z_zeros[k].re*fparm->z_zeros[k+1].re -
			    fparm->z_zeros[k].im*fparm->z_zeros[k+1].im;
      if ((fparm->z_poles[k].im + fparm->z_poles[k+1].im) != 0)
        return -1;
      parm->section[m].B1 = - (fparm->z_poles[k].re + fparm->z_poles[k+1].re);
      parm->section[m].B2 = fparm->z_poles[k].re*fparm->z_poles[k+1].re -
			    fparm->z_poles[k].im*fparm->z_poles[k+1].im;
    };

    switch(fparm->filter_shape) {
      case lowpass_fs:
        parm->section[m].C  = (1 + parm->section[m].B1 + parm->section[m].B2) /
			      (1 + parm->section[m].A1 + parm->section[m].A2);
        break;
      case highpass_fs:
        parm->section[m].C  = (1 - parm->section[m].B1 + parm->section[m].B2) /
			      (1 - parm->section[m].A1 + parm->section[m].A2);
        break;
      case bandstop_fs: {
        long double C0 = (1 + parm->section[m].B1 + parm->section[m].B2) /
			 (1 + parm->section[m].A1 + parm->section[m].A2);
        long double C1 = (1 - parm->section[m].B1 + parm->section[m].B2) /
			 (1 - parm->section[m].A1 + parm->section[m].A2);
        parm->section[m].C  = sqrtl(C0*C1);
        }
        break;
      case bandpass_fs: {
	 /* ??? CORRECT THIS ??? */
	 /* fs1 and fs2 must be pre-warped !!!! */
	long double w = acosl(-1)/*PI*/ * (fparm->fs1/F + fparm->fs2/F);
	complex c_w1 = c_polar(1, -1.0*w);
	complex c_w2 = c_polar(1, -2.0*w);
	complex c_A1 = c_mul(c_w1, c_rect(parm->section[m].A1, 0));
	complex c_A2 = c_mul(c_w2, c_rect(parm->section[m].A2, 0));
	complex c_B1 = c_mul(c_w1, c_rect(parm->section[m].B1, 0));
	complex c_B2 = c_mul(c_w2, c_rect(parm->section[m].B2, 0));
	complex c_A  = c_add(c_rect(1,0), c_add(c_A1, c_A2));
	complex c_B  = c_add(c_rect(1,0), c_add(c_B1, c_B2));
	complex c_C  = c_div(c_B, c_A);
        parm->section[m].C  = sqrtl(sqr(c_C.re) + sqr(c_C.im));
	}
	break;
    } /* switch */

  } /* for(;;) */

  parm->section[0].C *= fparm->gain;

  return 0;
}




static int string_to_filter_shape(char *str_val, filter_shape_t *filter_shape)
{
  if ((filter_shape == NULL) || (str_val == NULL))
    return -1;

       if (strcmp(str_val, LOWPASS_FILTER_SHAPE_ID) == 0)
    *filter_shape = lowpass_fs;
  else if (strcmp(str_val, HIGHPASS_FILTER_SHAPE_ID) == 0)
    *filter_shape = highpass_fs;
  else if (strcmp(str_val, BANDPASS_FILTER_SHAPE_ID) == 0)
    *filter_shape = bandpass_fs;
  else if (strcmp(str_val, BANDSTOP_FILTER_SHAPE_ID) == 0)
    *filter_shape = bandstop_fs;
  else
    return -1;

  return 0;
}


static int string_to_filter_aprox(char *str_val, filter_aprox_t *filter_aprox)
{
  if ((filter_aprox == NULL) || (str_val == NULL))
    return -1;

       if (strcmp(str_val, BUTTERWORTH_FILTER_APROX_ID) == 0)
    *filter_aprox = butterworth;
  else if (strcmp(str_val, CHEBYSHEV_FILTER_APROX_ID) == 0)
    *filter_aprox = chebyshev;
  else if (strcmp(str_val, ELLIPTIC_FILTER_APROX_ID) == 0)
    *filter_aprox = elliptic;
  else
    return -1;

  return 0;
}


/* calculates the equivalent iir filter of the specified analog filter */
static int filter_analog_to_iir(fb_parm_t *parm,
				long double F,
				analog_filter_parm_t *fparm
			       )
{
  int k;

  if (debug)
    printf("filter_analog_to_iir():\n");

  if (parm == NULL)
    return -1;

   /* determine the order of the filter */
  fparm->N = filter_N(fparm, F);

plc_log_trcmsg(1, "N=%d\n",  fparm->N);

  filter_fparm_malloc(fparm);

  filter_populate_ns_pole(fparm, F);
  filter_populate_ns_zero(fparm, F);
  for (k = 0; k < fparm->num_ns_poles; k++)
    plc_log_trcmsg(1, "ns_pole_Re=%10.15Lf ns_pole_Im=%10.15Lf\n",
		   fparm->ns_poles[k].re, fparm->ns_poles[k].im);
  for (k = 0; k < fparm->num_ns_zeros; k++)
    plc_log_trcmsg(1, "ns_zero_Re=%10.15Lf ns_zero_Im=%10.15Lf\n",
		   fparm->ns_zeros[k].re, fparm->ns_zeros[k].im);

plc_log_trcmsg(1, "1N=%d\n",  fparm->N);

  filter_ns_to_s(fparm, F);
  for (k = 0; k < fparm->num_s_poles; k++)
    plc_log_trcmsg(1, "s_pole_Re=%10.15Lf s_pole_Im=%10.15Lf\n",
		   fparm->s_poles[k].re, fparm->s_poles[k].im);
  for (k = 0; k < fparm->num_s_zeros; k++)
    plc_log_trcmsg(1, "s_zero_Re=%10.15Lf s_zero_Im=%10.15Lf\n",
		   fparm->s_zeros[k].re, fparm->s_zeros[k].im);

plc_log_trcmsg(1, "2N=%d\n",  fparm->N);

  filter_bilinear_s_to_z(fparm);
  for (k = 0; k < fparm->num_z_poles; k++)
    plc_log_trcmsg(1, "z_pole_Re=%10.15Lf z_pole_Im=%10.15Lf\n",
		   fparm->z_poles[k].re, fparm->z_poles[k].im);
  for (k = 0; k < fparm->num_z_zeros; k++)
    plc_log_trcmsg(1, "z_zero_Re=%10.15Lf z_zero_Im=%10.15Lf\n",
		   fparm->z_zeros[k].re, fparm->z_zeros[k].im);

plc_log_trcmsg(1, "3N=%d\n",  fparm->N);

  filter_z_to_ABC(fparm, F, &(parm->filter));

plc_log_trcmsg(1, "4N=%d\n",  fparm->N);


/* if (debug)*/
    filter_dump_parm(*parm);

  filter_fparm_mfree(fparm);

  return 0;
}




/**************************************************************/
/**************************************************************/
/**                                                          **/
/**                                                          **/
/**   P A R A M E T E R   P A R S I N G   F U N C T I O N S  **/
/**                                                          **/
/**                                                          **/
/**************************************************************/
/**************************************************************/



/* helper function for iir_filter_parse_parm() */
static void read_parm(const char *parm_str,
		      int section_count,
		      int parm_count,
		      const char *table_name,
		      int table_index,
		      f32 *var) {

 /* read in the parameter */
 if (conffile_get_table_f32(table_name, table_index, 4 + parm_count + section_count*5,
                            var,-f32_MAX, f32_MAX, *var)
     < 0) {
   plc_log_wrnmsg(1,
                  "Invalid %s param. value for section %d of %s block %d. Reverting to default %f",
 	          parm_str,
                  section_count + 1,
		  FILTER_TABLE_ID,
		  table_index,
		  *var);
  }
}


/* parses the parameters for an iir filter */
/* this function reads the C, A1, A2, B1 and B2 parameters */
static int parse_iir_filter_parm (const char *table_name,
                                  int table_index,
                                  fb_parm_t *parm)
/*
 * filter function matplc.conf syntax:
 *   fblock filter iir [C [A1 [A2 [B1 [B2]]]] ...
 *
 * where:
 *   C, A1, A2, B1, B2 : second order iir filter section parameters
 *
 *
 *  The iir filter implements the following second order section:
 *
 *          |\
 *          | \
 *  in -----|C >-->(+)-->(+)----------------------->(+)-->(+)---> out
 *          | /     ^     ^               |          ^     |
 *          |/      |     |               |          |     |
 *                  |     |               |          |     |
 *                  |     |              z-1         |     |
 *                  |     |   /|    /|    |    |\    |     |
 *                  |     |  / |   / |    |    | \   |     |
 *                  |     --<-1|--<B1|<---|--->|A1>---     |
 *                  |        \ |   \ |    |    | /         |
 *                  |         \|    \|    |    |/          |
 *                  |                    z-1               |
 *                  |         /|    /|    |    |\          |
 *                  |        / |   / |    |    | \         |
 *                  |-------<-1|--<B2|<---|--->|A2>--------|
 *                           \ |   \ |         | /
 *                            \|    \|         |/
 *
 *
 *        where (z-1) is a delay block, i.e., in the z transform,
 *         z to the power of -1.
 *
 */
{
  int rowlen, section_count;
  second_order_section_t *section;

  if (debug)
    printf("iir_filter_parse_parm(): table=%s, index=%d\n",
            table_name, table_index);

  if (parm == NULL)
    return -1;

  /* Determine the number of second order sections. */
  rowlen = conffile_get_table_rowlen(table_name, table_index);
  parm->filter.num_sections = ceil((rowlen - 4) / 5);

  parm->filter.section = malloc(sizeof(second_order_section_t)
		                * parm->filter.num_sections);

  if (parm->filter.section == NULL) {
    /* not enough memory */
    plc_log_errmsg(1, "Not enough memory for function block %d.",
		   table_index);
    return -1;
  }

   /* get the section parameters */
  for (section_count = 0;
       section_count < parm->filter.num_sections;
       section_count++) {

    /* set the default parameters */
    section = &(parm->filter.section[section_count]);
    section->C  = FILTER_DEF_C;
    section->A1 = FILTER_DEF_A1;
    section->A2 = FILTER_DEF_A2;
    section->B1 = FILTER_DEF_B1;
    section->B2 = FILTER_DEF_B2;

    /* initialize canonical filter 'memory' to zero. */
    section->z2 = 0;
    section->z1 = 0;

    read_parm("C", section_count, 0,
	      table_name, table_index,
	      &(section->C));
    read_parm("A1", section_count, 1,
	      table_name, table_index,
	      &(section->A1));
    read_parm("A2", section_count, 2,
	      table_name, table_index,
	      &(section->A2));
    read_parm("B1", section_count, 3,
	      table_name, table_index,
	      &(section->B1));
    read_parm("B2", section_count, 4,
	      table_name, table_index,
	      &(section->B2));

  } /* for(section_count) */
  return 0;
}





/* helper function for chebyshev, butterworth, and elliptic filter aproximation */
/* This function reads in the filter shape and apropriate frequency and attenuation parameters */
/*
 * Syntax:
 * fblock filter filter_aprox filter_shape gain passband_tol stopband_tol f1 f2 [f3 f4]
 *
 *  gain:
 *   the gain of the filter for the passband frequencies.
 *   For low_pass,  this is the gain at f = 0 Hz
 *   For high_pass, this is the gain at f = F/2 Hz (F is sampling frequency)
 *   For band_pass, this is the gain at f = (fp1 + fp2) / 2 Hz
 *   For band_stop, this is the gain sqrt [ g(0)*g(0) + g(F/2)*g(F/2) ]
 *
 *  filter_aprox:
 *    butterworth | chebyshev | elliptic
 *
 *  filter_shape:
 *    lowpass | highpass | bandpass | bandstop
 *
 *  passband_tol (stopband_tol):
 *    maximum(minimum) atenuation in positive dB for the passband(stopband) frequencies
 *
 *  Where the interpretation of f1, f2, f3 and f4 depends on the filter shape.
 *  The idea is to have valid frequencies always in increasing value.
 *  for a LowPass:
 *            f1 - passband frequency limit
 *            f2 - stopband frequency limit
 *  for a HighPass:
 *            f1 - stopband frequency limit
 *            f2 - passband frequency limit
 *  for a BandPass:
 *            f1 - lower  stopband frequency limit
 *            f2 - lower  passband frequency limit
 *            f3 - higher passband frequency limit
 *            f4 - higher stopband frequency limit
 *  for a BandStop:
 *            f1 - lower  passband frequency limit
 *            f2 - lower  stopband frequency limit
 *            f3 - higher stopband frequency limit
 *            f4 - higher passband frequency limit
 *
 */
static int filter_parse_analog_parm(const char *table_name,
				    int table_index,
                                    fb_parm_t *parm,
/*
				   filter_aprox_t *filter_aprox,
				   filter_shape_t *filter_shape,
				   f32 *gain,
				   f32 *Ap,
				   f32 *As,
				   f32 *fp1,
				   f32 *fs1,
				   f32 *fs2,
				   f32 *fp2,*/
				    f32 F /* sampling Frequency */
				   )
{
  analog_filter_parm_t fparm;
  char *tmp_val;
  f32 *f1, *f2, *f3, *f4;

  /* get the filter aproximation */
  tmp_val = conffile_get_table(table_name, table_index, 3);
  if (string_to_filter_aprox(tmp_val, &(fparm.filter_aprox)) < 0) {
    plc_log_errmsg(1, "Invalid filter aproximation %s.", tmp_val);
    free(tmp_val);
    return -1;
  }
  free(tmp_val);

  /* get the filter shape */
  tmp_val = conffile_get_table(table_name, table_index, 4);
  if (string_to_filter_shape(tmp_val, &(fparm.filter_shape)) < 0) {
    plc_log_errmsg(1, "Invalid filter shape %s.", tmp_val);
    free(tmp_val);
    return -1;
  }
  free(tmp_val);

  /* get the filter gain */
  if (conffile_get_table_f32(table_name, table_index, 5,
			     &(fparm.gain), -f32_MAX, f32_MAX, FILTER_DEF_GAIN)
      < 0) {
    tmp_val = conffile_get_table(table_name, table_index, 5);
    plc_log_errmsg(1, "Invalid filter gain %s.", tmp_val);
    free(tmp_val);
    return -1;
  }

  /* get the Passband tolerance - Ap */
  if (conffile_get_table_f32(table_name, table_index, 6,
			     &(fparm.Ap), 0, f32_MAX, FILTER_DEF_PASSBAND_TOL)
      < 0) {
    tmp_val = conffile_get_table(table_name, table_index, 6);
    plc_log_errmsg(1, "Invalid passband tolerance %s.", tmp_val);
    free(tmp_val);
    return -1;
  }

  /* get the Stopband tolerance - As */
  if (conffile_get_table_f32(table_name, table_index, 7,
			     &(fparm.As), 0, f32_MAX, FILTER_DEF_STOPBAND_TOL)
      < 0) {
    tmp_val = conffile_get_table(table_name, table_index, 7);
    plc_log_errmsg(1, "Invalid stopband tolerance %s.", tmp_val);
    free(tmp_val);
    return -1;
  }

  /* get the corner frequencies */
  f1 = f2 = f3 = f4 = NULL;
  switch (fparm.filter_shape) {
    case lowpass_fs:  f1 = &(fparm.fp1);
		      f2 = &(fparm.fs1);
		      break;
    case highpass_fs: f1 = &(fparm.fs1);
		      f2 = &(fparm.fp1);
		      break;
    case bandpass_fs: f1 = &(fparm.fs1);
		      f2 = &(fparm.fp1);
		      f3 = &(fparm.fp2);
		      f4 = &(fparm.fs2);
		      break;
    case bandstop_fs: f1 = &(fparm.fp1);
		      f2 = &(fparm.fs1);
		      f3 = &(fparm.fs2);
		      f4 = &(fparm.fp2);
		      break;
  } /* switch */

  if (f1 != NULL)
    if (conffile_get_table_f32(table_name, table_index, 8,
			       f1, 0, F/2, F/8)
	< 0) {
    tmp_val = conffile_get_table(table_name, table_index, 8);
    plc_log_errmsg(1, "Invalid first corner frequency (%s).", tmp_val);
    free(tmp_val);
    return -1;
  }

  if (f2 != NULL)
    if (conffile_get_table_f32(table_name, table_index, 9,
			       f2, 0, F/2, 2 * F/8)
	< 0) {
    tmp_val = conffile_get_table(table_name, table_index, 9);
    plc_log_errmsg(1, "Invalid second corner frequency (%s).", tmp_val);
    free(tmp_val);
    return -1;
  }

  if ((f1 != NULL) && (f2 != NULL) && (*f2 <= *f1))
    return -1;

  if (f3 != NULL)
    if (conffile_get_table_f32(table_name, table_index, 10,
			       f3, 0, F/2, 3 * F/8)
	< 0) {
    tmp_val = conffile_get_table(table_name, table_index, 10);
    plc_log_errmsg(1, "Invalid third corner frequency (%s).", tmp_val);
    free(tmp_val);
    return -1;
  }

  if ((f2 != NULL) && (f3 != NULL) && (*f3 <= *f2))
    return -1;

  if (f4 != NULL)
    if (conffile_get_table_f32(table_name, table_index, 11,
			       f4, 0, F/2, F/2 /*(4 * F/8)*/)
	< 0) {
    tmp_val = conffile_get_table(table_name, table_index, 11);
    plc_log_errmsg(1, "Invalid fourth corner frequency (%s).", tmp_val);
    free(tmp_val);
    return -1;
  }

  if ((f3 != NULL) && (f4 != NULL) && (*f4 <= *f3))
    return -1;

  /* determine the equivalent iir filter parameters */
  return filter_analog_to_iir(parm, F, &fparm);
}





/* parses the parameters in the matplc.conf configuration table */
/* and initializes the parm variable.                             */
int filter_parse_parm (const char *table_name,
                       int table_index,
                       fb_parm_t *parm,
		       f32 T /* period this dsp module will work with */)

/*
 * filter function matplc.conf syntax:
 *   fblock filter iir {fir | iir | butterworth | chebyshev | elliptic} ...
 *
 * where:
 *   fir : finite impulse response filter
 *   iir : infinite impulse response filter
 *   butterworth : iir filter, with parameters given as frequency, attenuation, ...
 *   chebushev   : iir filter, with parameters given as frequency, attenuation, ...
 *   elliptic    : iir filter, with parameters given as frequency, attenuation, ...
 *
 */
/* NOTE: the "fblock" in the row is really the value    */
/* of the FBLOCK_TABLE_ID constant. (check in dsp.h )   */
/* NOTE: the "filter" in the row is really the value of */
/* the FILTER_TABLE_ID constant.                        */
{
  char *tmp_val;
  int rowlen;

  if (debug)
    printf("filter_parse_parm(): table=%s, index=%d\n",
            table_name, table_index);

  if (parm == NULL)
    return -1;

  /*
   * check that there's that many function blocks and that the block
   * in question has the right number of fields
   */
  rowlen = conffile_get_table_rowlen(table_name, table_index);
  if (rowlen < 3) {
    /* plc_log_wrnmsg("...) */
    return -1;
  }

  /* check that the block is really a filter block */
  tmp_val = conffile_get_table(table_name, table_index, 0);
  if ( strcmp (tmp_val, FILTER_TABLE_ID) != 0) {
    /* Row is not configuration of a filter function block */
    /* plc_log_wrnmsg("...) */
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the input point */
  tmp_val = conffile_get_table(table_name, table_index, 1);
  parm->filter.in_pt = plc_pt_by_name(tmp_val);
  if (parm->filter.in_pt.valid == 0) {
    /* invalid input point */
    /* plc_log_wrnmsg("...) */
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the output point */
  tmp_val = conffile_get_table(table_name, table_index, 2);
  parm->filter.out_pt = plc_pt_by_name(tmp_val);
  if (parm->filter.out_pt.valid == 0) {
    /* invalid output point */
    /* plc_log_wrnmsg("...) */
    free (tmp_val);
    return -1;
  }
  free (tmp_val);


plc_log_trcmsg(1, "T = %f\n", T);

  /* get the filter aproximation */
  tmp_val = conffile_get_table(table_name, table_index, 3);
  if (strcmp(tmp_val, IIR_FILTER_APROX_ID) == 0) {
    free (tmp_val);
    return parse_iir_filter_parm(table_name, table_index, parm);
  }
  else
    return filter_parse_analog_parm(table_name, table_index, parm, 1/T);

  /* never reaches this point */
  return -1;
}







int filter_dump_parm(fb_parm_t parm)
{
  int k, N;
  N = parm.filter.num_sections;

  plc_log_trcmsg(1,"Number of sections: %d\n", N);

  for (k = 0; k < N; k++) {
    plc_log_trcmsg(1,"C=%10.15f  A1=%10.15f, A2=%10.15f, B1=%10.15f, B2=%10.15f\n",
               parm.filter.section[k].C,
               parm.filter.section[k].A1,
               parm.filter.section[k].A2,
               parm.filter.section[k].B1,
               parm.filter.section[k].B2
              );
  }

 return 0;
}

