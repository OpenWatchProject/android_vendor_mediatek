/* NOTE: if you want to use the static-memory fft, you must abandon some flexibility
 * e.g. the fft length must be known before you start to do fft and the multiple fft length
 * doesn't be supported in this version since the arrangement of twiddle factors
 * doesn't be optimized. */

/* although this customer head/source file have defined the requirement for fft,
 * the choices (FIXED_POINT/STATIC_FFT_MEMORY) must be defined in the kiss_fft.h */

#ifndef _KISS_FFT_CUST_H_
#define _KISS_FFT_CUST_H_

#include "kiss_fft.h"

/* window setting */
#define NUM_FFTR_WIN 1u
extern const uint32_t fftr_window_len[NUM_FFTR_WIN];

/* twiddle factors */
#define NUM_FFT_TWI 64u /* = max(fftr_win_len) / 2*/
#define NUM_FFTR_TWI 32u /* = NUM_FFT_TWI / 2*/
#define NUM_FFT_FAC 6u /* based on the offline factor computation */

/* tmp buffer while doing fftr */
#define MAX_LEN_TMPBUF 64u /* = max(fftr_win_len) / 2*/

/* fft twiddle factors */
extern kiss_fft_cpx fft_twiddles[NUM_FFT_TWI];
extern kiss_fft_cpx fftr_twiddles[NUM_FFTR_TWI];
extern int32_t fft_factors[NUM_FFT_FAC];

#ifdef STATIC_FFT_GLOBAL
/* tmp buff */
extern kiss_fft_cpx tmpbuf[MAX_LEN_TMPBUF];
#endif

#endif /* _KISS_FFT_CUST_H_ */
