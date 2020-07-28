#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioToolkit"
#include "AudioToolkit.h"

#if defined(PC_EMULATION)
#include "windows.h"
#else
#include "unistd.h"
#include "pthread.h"
#endif

#include <utils/Log.h>
#include <utils/String8.h>

namespace android {
//---------- implementation of base64 encode/decode--------------
// function for encode/decode string
static const char table_base64[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                    'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                    '4', '5', '6', '7', '8', '9', '+', '/'
                                   };
static const int table_mod[] = {0, 2, 1};
static char *decoding_table = NULL;

//Base64_Encode: output_length = 4 * ((input_length + 2) / 3);
//Base64_Decode: output_length =  input_length / 4 * 3;
size_t Base64_OutputSize(bool bEncode, size_t input_length) {
    size_t output_length = 0;

    if (bEncode) {
        output_length = 4 * ((input_length + 2) / 3);
    } else {
        output_length =  input_length / 4 * 3;
    }
    ALOGV("-%s(), bEncode(%d), input_length= %zu, output_length=%zu", __FUNCTION__, bEncode, input_length, output_length);
    return output_length;
}

size_t Base64_Encode(const unsigned char *data_input, char *data_encoded, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    ALOGV("+%s(), data_input(%p), data_encoded(%p), input_length= %zu", __FUNCTION__, data_input, data_encoded, input_length);

    //    char *encoded_data = malloc(*output_length);
    if (data_encoded == NULL) {return 0;}

    for (size_t i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char)data_input[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data_input[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data_input[i++] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        data_encoded[j++] = table_base64[(triple >> 3 * 6) & 0x3F];
        data_encoded[j++] = table_base64[(triple >> 2 * 6) & 0x3F];
        data_encoded[j++] = table_base64[(triple >> 1 * 6) & 0x3F];
        data_encoded[j++] = table_base64[(triple >> 0 * 6) & 0x3F];
        ALOGV("%s(), i(%zu), j(%zu)", __FUNCTION__, i, j);
    }
    for (int i = 0; i < table_mod[input_length % 3]; i++) {
        data_encoded[output_length - 1 - i] = '-';
    }
    ALOGV("-%s(), output_length=%zu", __FUNCTION__, output_length);
    return output_length;
}

void build_decoding_table() {
    if (decoding_table == NULL) {
        decoding_table = new char [256];

        for (int i = 0; i < 64; i++) {
            decoding_table[(unsigned char) table_base64[i]] = i;
        }
    } else {
        ALOGV("-%s(), decoding_table already exist", __FUNCTION__);
    }
}

void base64_cleanup() {
    ALOGV("+%s()", __FUNCTION__);
    if (decoding_table != NULL) {
        delete[] decoding_table;
        decoding_table = NULL;
    }
    ALOGV("-%s()", __FUNCTION__);
}

size_t Base64_Decode(const char *data_input, unsigned char *data_decoded, size_t input_length) {
    ALOGV("+%s(), data_input(%p), data_decoded(%p), input_length= %zu", __FUNCTION__, data_input, data_decoded, input_length);
    if ((input_length % 4 != 0) || (data_decoded == NULL)) {
        return 0;
    }
    build_decoding_table();

    size_t output_length = input_length / 4 * 3;
    if (data_input[input_length - 1] == '-') { (output_length)--; }
    if (data_input[input_length - 2] == '-') { (output_length)--; }

    //unsigned char *decoded_data = malloc(*output_length);

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t sextet_a = data_input[i] == '-' ? 0 & i++ : decoding_table[(unsigned char)data_input[i++]];
        uint32_t sextet_b = data_input[i] == '-' ? 0 & i++ : decoding_table[(unsigned char)data_input[i++]];
        uint32_t sextet_c = data_input[i] == '-' ? 0 & i++ : decoding_table[(unsigned char)data_input[i++]];
        uint32_t sextet_d = data_input[i] == '-' ? 0 & i++ : decoding_table[(unsigned char)data_input[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
                          + (sextet_b << 2 * 6)
                          + (sextet_c << 1 * 6)
                          + (sextet_d << 0 * 6);

        if (j < output_length) { data_decoded[j++] = (triple >> 2 * 8) & 0xFF; }
        if (j < output_length) { data_decoded[j++] = (triple >> 1 * 8) & 0xFF; }
        if (j < output_length) { data_decoded[j++] = (triple >> 0 * 8) & 0xFF; }
        ALOGV("%s(), i(%zu), j(%zu)", __FUNCTION__, i, j);
    }
    base64_cleanup();
    ALOGV("-%s(), output_length=%zu", __FUNCTION__, output_length);

    return output_length;
}

}
