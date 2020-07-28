#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioToolkit"
#include "AudioToolkit.h"

#include "AudioDef.h"

#if defined(PC_EMULATION)
#include "windows.h"
#else
#include "unistd.h"
#include "pthread.h"
#endif

#include <utils/Log.h>
#include <utils/String8.h>

#ifdef SYS_IMPL
#include <media/AudioSystem.h>
#endif

#include <dirent.h>
#include <sys/stat.h>

namespace android {

static String8 keyMTK_GET_AUDIO_DUMP_FILE_LIST = String8("MTK_GET_AUDIO_DUMP_FILE_LIST");
static String8 keyMTK_GET_AUDIO_DUMP_FILE_CONTENT = String8("MTK_GET_AUDIO_DUMP_FILE_CONTENT");
static String8 keyMTK_DEL_AUDIO_DUMP_FILE = String8("MTK_DEL_AUDIO_DUMP_FILE");


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

status_t AudioToolKit_GetDecodedData(String8 strPara, size_t len, void *ptr)
{
    size_t sz_in = strPara.size();
    size_t sz_needed = Base64_OutputSize(false, sz_in);
    size_t sz_dec;
    status_t ret = NO_ERROR;

    if (sz_in <= 0)
        return NO_ERROR;

    ALOGD("%s in, len = %zu", __FUNCTION__, len);
    unsigned char *buf_dec = new unsigned char[sz_needed];
    sz_dec = Base64_Decode(strPara.string(), buf_dec, sz_in);

    if (sz_dec > sz_needed || sz_dec <= sz_needed -3) {
        ALOGE("%s(), Decode Error!!!after decode (%s), sz_in(%zu), sz_needed(%zu), sz_dec(%zu)",
            __FUNCTION__, buf_dec, sz_in, sz_needed, sz_dec);
    }  else {
        // sz_needed-3 < sz_dec <= sz_needed
    }

    if( (len == 0) || (len == sz_dec-sizeof(ret)) ) {
       if ( len ) {
           ret = (status_t) *(buf_dec);
           unsigned char *buff = (buf_dec+4);
           memcpy(ptr, buff, len);
       } else {
          const char * IntPtr = (char *)buf_dec;
          ret = atoi(IntPtr);
          ALOGD("%s len = 0 ret(%d)", __FUNCTION__, ret);
       }
    } else {
       ALOGD("%s decoded buffer isn't right format", __FUNCTION__);
    }

    if (buf_dec != NULL) {
        delete[] buf_dec;
    }

    return ret;
}

String8 base64Encode(const unsigned char *dataInput, size_t inputLength) {
    ALOGV("%s(), dataInput = %p, inputLength = %zu\n", __FUNCTION__, dataInput, inputLength);

    if (dataInput == NULL || inputLength == 0) {
        return String8("");
    }

    size_t outputLength = Base64_OutputSize(true, inputLength);

    char *bufEnc = new char[outputLength + 1];
    memset(bufEnc, 0, outputLength + 1);
    size_t encSize = Base64_Encode(dataInput, bufEnc, inputLength);

    if (encSize == 0) {
        ALOGW("%s(), Encode Error!!!after encode (%s), inputLength(%zu), outputLength(%zu), encSize(%zu)", __FUNCTION__, bufEnc, inputLength, outputLength, encSize);
    } else {
        ALOGV("%s(), after encode (0x%p), inputLength(%zu), encSize(%zu)", __FUNCTION__, bufEnc, inputLength, encSize);
    }

    //char to string8
    String8 encStr = String8(bufEnc, encSize);

    if (bufEnc != NULL) {
        delete[] bufEnc;
    }

    return encStr;
}

unsigned char* base64Decode(String8 encStr, size_t *decSize) {
    size_t encSize = encStr.length();

    ALOGV("%s(), encStr = %s, encSize = %zu\n", __FUNCTION__, encStr.string(), encSize);

    if (encSize == 0) {
        decSize = 0;
        return NULL;
    }

    *decSize = Base64_OutputSize(false, encSize);

    unsigned char *decBuffer = new unsigned char[(*decSize) + 1];
    memset(decBuffer, 0, (*decSize) + 1);

    *decSize = Base64_Decode(encStr.string(), decBuffer, encSize);

    if (*decSize == 0) {
        ALOGW("%s(), Decode Error!!!after decode (%p), encSize(%zu), decodeSize(%zu)", __FUNCTION__, decBuffer, encSize, *decSize);
        if (decBuffer != NULL) {
            delete[] decBuffer;
            decBuffer = NULL;
        }
    }

    return decBuffer;
}


#ifdef SYS_IMPL

String8 getAudioHalDumpFileList()
{
    String8 retKeyValPair = AudioSystem::getParameters(0, keyMTK_GET_AUDIO_DUMP_FILE_LIST);
    const char *val = strstr(retKeyValPair.string(), "=");
    if (val) {
        return String8(val + 1);
    } else {
        return String8("");
    }
}

size_t readAudioHalDumpFileContent(char* fileName, unsigned char** buf, size_t readFrom, size_t readSize)
{
    size_t retSize = 0;
    String8 readFromStr = String8(std::to_string(readFrom).c_str());
    String8 readSizeStr = String8(std::to_string(readSize).c_str());
    String8 queryStr = keyMTK_GET_AUDIO_DUMP_FILE_CONTENT + "#" + fileName + "#" + readFromStr + "#" + readSizeStr;

    String8 retKeyValPair = AudioSystem::getParameters(0, queryStr);

    const char* encStr = strstr(retKeyValPair.string(), "=");
    if (encStr) {
        encStr += 1;

        *buf = base64Decode(String8(encStr), &retSize);
        if (readSize != retSize) {
            ALOGW("%s(), retSize(%zu) is not readSize(%zu)!\n", __FUNCTION__, retSize, readSize);
            retSize = 0;
            delete[] (*buf);
            *buf = NULL;
        }
    } else {
        ALOGW("%s(), enc Str is NULL\n", __FUNCTION__);
    }

    return retSize;
}

bool delAudioHalDumpFile(const char* fileName) {
    String8 delFileCmd = String8(keyMTK_DEL_AUDIO_DUMP_FILE) + "=" + String8(fileName);
    AudioSystem::setParameters(0, delFileCmd);

    return true;
}

#else

String8 getAudioHalDumpFileList() {
    String8 fileList = String8("");
    struct dirent **nameList;
    int i = 0;
    int total = scandir(DUMP_PATH, &nameList, 0, alphasort);

    for (i = 0; i < total; i++) {
        if (!strcmp(nameList[i]->d_name, ".") || !strcmp(nameList[i]->d_name, "..")) {
            continue;
        }

        struct stat fileStat;
        String8 fileName = String8(nameList[i]->d_name);
        String8 filePath = String8(DUMP_PATH) + fileName;
        stat(filePath.string(), &fileStat);
        String8 fileSize = String8(std::to_string(fileStat.st_size).c_str());

        if (fileList == "") {
            fileList = fileName + String8(",") + fileSize;
        } else {
            fileList = fileList + String8(",") + fileName + String8(",") + fileSize;
        }
    }

    return fileList;
}

size_t readAudioHalDumpFileContent(char* fileName, unsigned char** buf, size_t readFrom, size_t readSize) {
    String8 filePath = String8 (DUMP_PATH) + fileName;
    size_t encSize = 0;
    unsigned char* buffer = NULL;
    char *bufEnc = NULL;

    if (readSize == 0) {
        ALOGW("%s(), readSize = 0\n", __FUNCTION__);
        return 0;
    }

    FILE* fp = fopen(filePath.string(), "rb");
    if (fp != NULL) {
        buffer = new unsigned char[readSize];
        if (buffer) {
            fseek(fp, readFrom, SEEK_SET);
            readSize = fread(buffer, 1, readSize, fp);

            // Base64 encoding
            size_t outputLength = Base64_OutputSize(true, readSize);
            bufEnc = new char[outputLength + 1];
            memset(bufEnc, 0, outputLength + 1);
            encSize = Base64_Encode(buffer, bufEnc, readSize);

            if (encSize == 0) {
                delete[] bufEnc;
                bufEnc = NULL;
                ALOGW("%s(), Encode Error!!! input size(%zu), encSize(%zu)", __FUNCTION__, readSize, encSize);
            } else {
                ALOGV("%s(), after encode (0x%p), input size(%zu), encSize(%zu)", __FUNCTION__, bufEnc, readSize, encSize);
            }
        } else {
            ALOGW("%s(), malloc fail (%zu)\n", __FUNCTION__, readSize);
        }
        fclose(fp);
    } else {
        ALOGW("%s(), fopen fail (file = %s, fp = %p)\n", __FUNCTION__, filePath.string(), fp);
    }

    ALOGV("%s(), Real readed file size = %zu, encodeSize = %zu, bufEnc = %p\n", __FUNCTION__, readSize, encSize, bufEnc);

    *buf = (unsigned char*)bufEnc;

    return encSize;
}


bool delAudioHalDumpFile(const char* fileName) {
    String8 filePath = String8(DUMP_PATH) + "/" + fileName;
    int ret = remove(filePath.string());
    if (ret == 0) {
        ALOGD("%s(), Remove file %s\n", __FUNCTION__, filePath.string());
    } else {
        ALOGW("%s(), Remove file %s fail (errno = %d)\n", __FUNCTION__, filePath.string(), errno);
    }
    return true;
}

#endif

}
