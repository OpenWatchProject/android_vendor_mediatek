#ifndef __BASE64_H__
#define __BASE64_H__

#include <stdlib.h>
#include <utils/Vector.h>

#define TCHAR char    //Not unicode
#define TEXT(x) x     //Not unicode
#define DWORD long
#define BYTE unsigned char

#define DEBUG_BASE64
#ifdef DEBUG_BASE64
#include <android/log.h>
#define LOG_TAG "Base64"
#define BASE64_ALOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define BASE64_IN() BASE64_ALOG("+ %s", __func__)
#define BASE64_OUT() BASE64_ALOG("- %s", __func__)
#define BASE64_PROP() BASE64_ALOG(" --> %s: %d", __func__, __LINE__)
#else
#define BASE64_ALOG(...)
#define BASE64_IN()
#define BASE64_OUT()
#define BASE64_PROP()
#endif

namespace MtkVenc {

static inline void dumpBase64Result(char* input, android::Vector<unsigned char> decodedBytes)
{
    char dump[1024];
    int offset = 0;
    dump[0] = 0;
    for(int i=0; i<decodedBytes.size(); i++)
    {
        offset = snprintf(dump+offset, sizeof(dump)-offset, ",%X", decodedBytes[i]);
        if(offset < 0 || offset >= sizeof(dump)) break;
    }
    BASE64_ALOG("[DUMP] Base64 %s -> [%s]", input, dump+1);
}

const static TCHAR padCharacter = TEXT('=');
static inline android::Vector<unsigned char> base64Decode(int inputLength, char* input)
{
    BASE64_IN();
    //Setup a vector to hold the result
    android::Vector<unsigned char> decodedBytes;

    if(input == NULL) return decodedBytes;
    if(inputLength == 0) return decodedBytes;

    if (inputLength % 4) //Sanity check
    {
        BASE64_ALOG("[ERROR] input.length %zu not 4x", inputLength);
        decodedBytes.clear();
        return decodedBytes;
    }
    size_t padding = 0;
    if (inputLength)
    {
        if (input[inputLength-1] == padCharacter)
            padding++;
        if (input[inputLength-2] == padCharacter)
            padding++;
    }

    BASE64_ALOG("[DEBUG] reserve len %zu (inputLen %d padding %zu)",
        ((inputLength/4)*3) - padding, inputLength, padding);

    decodedBytes.reserve(((inputLength/4)*3) - padding);
    DWORD temp=0; //Holds decoded quanta
    char* cursor = &input[0];
    while (cursor < &input[inputLength-1])
    {
        for (size_t quantumPosition = 0; quantumPosition < 4; quantumPosition++)
        {
            temp <<= 6;
            if       (*cursor >= 0x41 && *cursor <= 0x5A) // This area will need tweaking if
                temp |= *cursor - 0x41;                   // you are using an alternate alphabet
            else if  (*cursor >= 0x61 && *cursor <= 0x7A)
                temp |= *cursor - 0x47;
            else if  (*cursor >= 0x30 && *cursor <= 0x39)
                temp |= *cursor + 0x04;
            else if  (*cursor == 0x2B)
                temp |= 0x3E; //change to 0x2D for URL alphabet
            else if  (*cursor == 0x2F)
                temp |= 0x3F; //change to 0x5F for URL alphabet
            else if  (*cursor == padCharacter) //pad
            {
                switch( &input[inputLength] - cursor )
                {
                case 1: //One pad character
                    BASE64_ALOG("[DEBUG] padding 1");
                    decodedBytes.push_back((temp >> 16) & 0x000000FF);
                    decodedBytes.push_back((temp >> 8 ) & 0x000000FF);
                    dumpBase64Result(input, decodedBytes);
                    return decodedBytes;
                case 2: //Two pad characters
                    BASE64_ALOG("[DEBUG] padding 2");
                    decodedBytes.push_back((temp >> 10) & 0x000000FF);
                    dumpBase64Result(input, decodedBytes);
                    return decodedBytes;
                default:
                    BASE64_ALOG("[ERROR] input.end() - cursor != 1,2");
                    decodedBytes.clear();
                    dumpBase64Result(input, decodedBytes);
                    return decodedBytes;
                }
            }
            else
            {
                BASE64_ALOG("[ERROR] cursor not correct %x", *cursor);
                decodedBytes.clear();
                return decodedBytes;
            }
            cursor++;
        }
        decodedBytes.push_back((temp >> 16) & 0x000000FF);
        decodedBytes.push_back((temp >> 8 ) & 0x000000FF);
        decodedBytes.push_back((temp      ) & 0x000000FF);
    }

    dumpBase64Result(input, decodedBytes);

    BASE64_OUT();
    return decodedBytes;
}

}; // NS MtkVenc

#endif //__BASE64_H__