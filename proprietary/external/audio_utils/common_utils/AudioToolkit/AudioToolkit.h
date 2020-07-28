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
status_t AudioToolKit_GetDecodedData(String8 strPara, size_t len, void *ptr);
unsigned char* base64Decode(String8 encStr, size_t *decSize);
String8 base64Encode(const unsigned char *dataInput, size_t inputLength);

// APIs for audio dump management
String8 getAudioHalDumpFileList();
size_t readAudioHalDumpFileContent(char* fileName, unsigned char** buf, size_t readFrom, size_t readSize);
bool delAudioHalDumpFile(const char* fileName);
}

#endif
