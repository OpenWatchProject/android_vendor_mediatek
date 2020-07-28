#ifndef AURISYS_SCENARIO_DSP_H
#define AURISYS_SCENARIO_DSP_H

#include <stdint.h>

enum {
    /* playback */
    AURISYS_SCENARIO_DSP_PRIMARY            = 0,
    AURISYS_SCENARIO_DSP_OFFLOAD            = 1,
    AURISYS_SCENARIO_DSP_DEEP_BUF           = 2,
    AURISYS_SCENARIO_DSP_FAST               = 3,

    AURISYS_SCENARIO_DSP_PLAYBACK           = 4, /* last module before hardware */


    /* record */
    AURISYS_SCENARIO_DSP_RECORD             = 5,
    AURISYS_SCENARIO_DSP_RECORD_FAST        = 6,


    /* call & voip */
    AURISYS_SCENARIO_DSP_PHONE_CALL         = 7,
    AURISYS_SCENARIO_DSP_VOIP               = 8,
    AURISYS_SCENARIO_DSP_CALL_FINAL         = 9,
    AURISYS_SCENARIO_DSP_PLAYBACK_SMARTPA   = 10,
    AURISYS_SCENARIO_DSP_KTV                = 11,


    /* control */
    AURISYS_SCENARIO_DSP_SIZE,
    AURISYS_SCENARIO_DSP_ALL,

    AURISYS_SCENARIO_DSP_INVALID = 0xFFFFFFFF
};



#endif /* end of AURISYS_SCENARIO_DSP_H */

