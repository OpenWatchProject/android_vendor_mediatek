#ifndef VOWE_DSP_TESTING
#define VOWE_DSP_TESTING

#include "VowEngine_types.h"
#include "VowEngine_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
    ParsedData dspParsedModelData;
} VOWE_DSP_testing_init_parameters;

int VOWE_DSP_testing_init(const VOWE_DSP_testing_init_parameters* parameters);
    /* initialize testing part of VOWE, passing VOWE_DSP_testing_init_parameters pointer as parameter wrapper
     * [dspParsedModelData] a buffer for enrolled model data structures on DSP (after enrollment)
     *
     * return
     * [vowe_ok] init succeeds
     * [vowe_bad] init fails, ex: unacceptable parameters, memory alloc fails ...
    */

int VOWE_DSP_testing_reset();
    /* reset testing part of VOWE; just like VOWE_DSP_testing_init without reloading model
     *
     * return
     * [vowe_ok] reset succeeds
     * [vowe_bad] reset fails
    */

RecognitionResults VOWE_DSP_testing_recognize(const short* signal, const int sampleNumber);
    /* diagnose the signals inputted
     * NOTICE: calling requires number of inputting samples equaling to (frameShift) for each channel
     * [signal] pointer to the buffer of input signal
     * [sampleNumber] total sample number of signal
     *
     * return
     * [RecognitionResults]
     *     [status]
     *         [vowe_hold_on] no recognition result; need more signals for further recognition
     *         [vowe_accept] current utterance is keyword
     *         [vowe_reject] current utterance is not keyword
     *         [vowe_bad] problem happens when recognize, ex: insufficient input samples, wrong calling flow ...
     *     [score]    the score indicates the likelihood of whether the recent input contains a keyword
     *     [startIdx] the most likely start frame index of a keyword
     *     [endIdx]   the most likely end frame index of a keyword
    */

int VOWE_DSP_testing_release();
    /* release the testing part
     *
     * return
     * [vowe_ok] release succeeds
     * [vowe_bad] release fails
     *
    */

const char* VOWE_DSP_testing_version();
    /* get current VOWE testing version, in string style
    */

int VOWE_DSP_testing_setArgument(const int argumentId, const int value);
    /* set arguments of VOWE testing to control corresponding flow
     * [vowe_argid_printLog]
     *   control log printing or not
     *   0: log print off (default)
     *   1: log print on
     *
     *
     * return
     * [vowe_ok] set succeeds
     * [vowe_bad] set fails, ex: wrong id, unacceptable value ...
    */




#ifdef __cplusplus
}
#endif

#endif // VOWE_DSP_TESTING

