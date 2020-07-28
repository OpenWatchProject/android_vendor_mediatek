#ifndef AUDIO_MTK_DEC_H
#define AUDIO_MTK_DEC_H

#include <sys/types.h>
#include "mp3dec_exp.h"
#include "AudioType.h"
#include <AudioLock.h>

#ifndef uint32_t
typedef unsigned int        uint32;
#endif


namespace android {



#if defined(__cplusplus)
extern "C" {
#endif

#define MP3_SYNC 0xe0ff

const int mp3_sample_rates[3][3] = {
    {44100, 48000, 32000},        /* MPEG-1 */
    {22050, 24000, 16000},        /* MPEG-2 */
    {11025, 12000,  8000},        /* MPEG-2.5 */
};

const int mp3_bit_rates[3][3][15] = {
    {
        /* MPEG-1 */
        {  0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448}, /* Layer 1 */
        {  0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384}, /* Layer 2 */
        {  0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320}, /* Layer 3 */
    },
    {
        /* MPEG-2 */
        {  0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256}, /* Layer 1 */
        {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}, /* Layer 2 */
        {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}, /* Layer 3 */
    },
    {
        /* MPEG-2.5 */
        {  0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256}, /* Layer 1 */
        {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}, /* Layer 2 */
        {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}, /* Layer 3 */
    },
};

enum mpeg_version {
    MPEG1  = 0,
    MPEG2  = 1,
    MPEG25 = 2
};

enum mp3_stereo_mode {
    STEREO = 0x00,
    JOINT = 0x01,
    DUAL = 0x02,
    MONO = 0x03
};

#if defined(__cplusplus)
}
#endif


typedef struct {
Mp3dec_handle *handle;
int32_t workingbuf_size1;
int32_t workingbuf_size2;
int32_t min_bs_size;
int32_t pcm_size;
void *working_buf1;
void *working_buf2;
void *pcm_buf;
} mp3DecEngine;

typedef struct mp3_header {
uint16_t sync;
uint8_t format1;
uint8_t format2;
};

class AudioMTKDecHandlerBase {
public:
AudioMTKDecHandlerBase();
virtual ~AudioMTKDecHandlerBase() = 0;

virtual bool InitAudioDecoder() = 0;
virtual void DeinitAudioDecoder() = 0;
virtual int32_t ParseAudioHeader(int8_t *inputBsbuf) = 0;
virtual int32_t DecodeAudio(int8_t *inputBsbuf, int8_t *outputPcmbuf, int32_t BsbufSize) = 0;
virtual int32_t PcmbufferSize() = 0;
virtual int32_t BsbufferSize() = 0;
};


#define MTK_OUTPUT_BUFFER_SIZE_MP3 4608


class AudioMTKDecHandlerMP3 : public AudioMTKDecHandlerBase {
public:
AudioMTKDecHandlerMP3();
~AudioMTKDecHandlerMP3();
virtual bool InitAudioDecoder();
virtual void DeinitAudioDecoder();
virtual int32_t ParseAudioHeader(int8_t *inputBsbuf);
virtual int32_t DecodeAudio(int8_t *inputBsbuf, int8_t *outputPcmbuf, int32_t BsbufSize);
virtual int32_t PcmbufferSize();
virtual int32_t BsbufferSize();


private:
mp3DecEngine *mMp3Dec;
uint32_t mBitRate;
uint32_t mChannel;
bool mMp3InitFlag;
uint32_t mSampleRate;
int32_t mBSBuffSize;
uint32_t mBufferLength;
bool mEndFlag;
bool mIsEndOfStream;
};

}
#endif
