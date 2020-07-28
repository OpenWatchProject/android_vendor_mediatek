#ifndef SPEECH_PARSER_TYPE_H
#define SPEECH_PARSER_TYPE_H

#include <stdint.h>
#include <system/audio.h>
#include "SpeechFeatureDef.h"

namespace android {

/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */
#define MAX_SPEECH_PARSER_KEY_LEN      (512)
#define SPEECH_PARSER_SET_KEY_PREFIX      "SPEECH_PARSER_SET_PARAM"
#define SPEECH_PARSER_GET_KEY_PREFIX      "SPEECH_PARSER_GET_PARAM"
/** driver usage */
#define SPEECH_PARSER_MD_VERSION     "MDVERSION"
/** parser usage */
#define SPEECH_PARSER_PARAMBUF_SIZE      "PARAMBUF_SIZE"

/*
 * =============================================================================
 *                     enum
 * =============================================================================
 */

enum SpeechScenario {
    SPEECH_SCENARIO_SPEECH_ON = 0,
    SPEECH_SCENARIO_PARAM_CHANGE = 1,
    SPEECH_SCENARIO_DEVICE_CHANGE = 2,
    SPEECH_SCENARIO_VOLUME_CHANGE = 3,
    SPEECH_SCENARIO_FEATURE_CHANGE = 4,
    NUM_SPEECH_SCENARIO
};

/*
 * =============================================================================
 *                     struct definition
 * =============================================================================
 */

/** char string buffer */
typedef struct {
    uint32_t memorySize;       /* 0 <= string_size < memory_size */
    uint32_t stringSize;       /* string_size = strlen(p_string); */
    union {
        char    *stringAddr;      /* string address */
        uint32_t dummy[2];      /* reserve for 64 bits pointer only */
    };
} SpeechStringBufType; /* sizeof(): 16 bytes */


/** data buffer (speech parameters */
typedef struct {
    uint32_t memorySize;       /* buffer size (memory) */
    uint32_t dataSize;         /* 0 <= data_size <= memory_size */
    union {
        void    *bufferAddr;      /* memory address */
        uint32_t dummy[2];      /* reserve for 64 bits pointer only */
    };
} SpeechDataBufType; /* sizeof(): 16 bytes */


/** driver environment attribute for speech parser */
typedef struct {
    audio_devices_t inputDevice;
    audio_devices_t outputDevice;
    unsigned int idxVolume;
    SpeechScenario driverScenario;
    TtyModeType ttyMode;
    uint16_t speechFeatureOn;
    uint8_t  __reserve_at_48byte[16];
} SpeechParserAttribute; /* sizeof(): 48 bytes */

}
#endif // end of SPEECH_PARSER_TYPE_H
