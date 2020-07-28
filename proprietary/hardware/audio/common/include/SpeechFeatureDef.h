#ifndef SPEECH_FEATURE_DEF_H
#define SPEECH_FEATURE_DEF_H

#include <stdint.h>
namespace android {

/*
 * =============================================================================
 *                     enum
 * =============================================================================
 */

enum TtyModeType {
    AUD_TTY_OFF  = 0,
    AUD_TTY_FULL = 1,
    AUD_TTY_VCO  = 2,
    AUD_TTY_HCO  = 4,
    AUD_TTY_ERR  = -1
};

/** speech feature type for switch on/off , max 15*/
enum SpeechFeatureType {
    SPEECH_FEATURE_LOOPBACK   = 0,
    SPEECH_FEATURE_BTNREC = 1,
    SPEECH_FEATURE_DMNR   = 2,
    SPEECH_FEATURE_LSPK_DMNR   = 3,
    SPEECH_FEATURE_HAC  = 4,
    SPEECH_FEATURE_SUPERVOLUME  = 5,
    NUM_SPEECH_FEATURE
};

}
#endif // end of SPEECH_FEATURE_DEF_H
