#ifndef AUDIO_TOOLKIT_H
#define AUDIO_TOOLKIT_H
#include <utils/String8.h>

/*
    this function should implement the basic function for debug information
    or basic function proviede to audio hardware modules
*/

namespace android {
//-------Base64 operation
size_t Base64_OutputSize(bool bEncode, size_t input_length);
size_t Base64_Encode(const unsigned char *data_input, char *data_encoded, size_t input_length);
size_t Base64_Decode(const char *data_input, unsigned char *data_decoded, size_t input_length);

}

#endif
