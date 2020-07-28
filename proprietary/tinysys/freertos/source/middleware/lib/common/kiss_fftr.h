#ifndef KISS_FTR_H
#define KISS_FTR_H

#include "kiss_fft.h"
#ifdef __cplusplus
extern "C" {
#endif


/* ------------------------  data format ------------------------------------*/
/* fftr data structure */
struct kiss_fftr_state {
    kiss_fft_cpx *tmpbuf;
    kiss_fft_cpx *super_twiddles;
    kiss_fft_cfg substate; /* substate will point to kiss_fft_state */
#ifdef STATIC_FFT_MEMORY
    struct kiss_fft_state fft_cfg;
#endif
#ifdef USE_SIMD
    void * pad;
#endif
};
typedef struct kiss_fftr_state *kiss_fftr_cfg;


/*--------------------------- external functions ---------------------------- */
/*
 Real optimized version can save about 45% cpu time vs. complex fft of a real seq.
 */


#ifdef STATIC_FFT_MEMORY
void kiss_fftr_static_alloc(int32_t, int32_t, kiss_fftr_cfg);
#else
kiss_fftr_cfg kiss_fftr_alloc(int32_t nfft, int32_t inverse_fft, void *mem, size_t *lenmem);
#endif
/*
 nfft must be even

 If you don't care to allocate space, use mem = lenmem = NULL
*/


void kiss_fftr(kiss_fftr_cfg cfg, const kiss_fft_scalar *timedata, kiss_fft_cpx *freqdata);
/*
 input timedata has nfft scalar points
 output freqdata has nfft/2+1 complex points
*/

void kiss_fftri(kiss_fftr_cfg cfg, const kiss_fft_cpx *freqdata, kiss_fft_scalar *timedata);
/*
 input freqdata has  nfft/2+1 complex points
 output timedata has nfft scalar points
*/

#define kiss_fftr_free free

#ifdef __cplusplus
}
#endif
#endif
