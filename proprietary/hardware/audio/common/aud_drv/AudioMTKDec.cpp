
#include <stdlib.h>
#include "AudioMTKDec.h"
#include "mp3dec_exp.h"

#define LOG_TAG "AudioMTKDec"

namespace android {


AudioMTKDecHandlerBase::AudioMTKDecHandlerBase() {
    ALOGD("%s()", __FUNCTION__);
}

AudioMTKDecHandlerBase::~AudioMTKDecHandlerBase() {
    ALOGD("%s()", __FUNCTION__);
}


AudioMTKDecHandlerMP3::AudioMTKDecHandlerMP3() :
    AudioMTKDecHandlerBase(),
    mMp3Dec(NULL),
    mMp3InitFlag(false) {
    ALOGD("%s()", __FUNCTION__);
}

AudioMTKDecHandlerMP3::~AudioMTKDecHandlerMP3() {
    if (mMp3Dec) {
        if (mMp3Dec->working_buf1) {
            free(mMp3Dec->working_buf1);
            mMp3Dec->working_buf1 = NULL;
        }

        if (mMp3Dec->working_buf2) {
            free(mMp3Dec->working_buf2);
            mMp3Dec->working_buf2 = NULL;
        }

        free(mMp3Dec);
        mMp3Dec = NULL;
    }
}

bool AudioMTKDecHandlerMP3::InitAudioDecoder() {
    if (!mMp3InitFlag) {
        ALOGD("+%s()", __FUNCTION__);
        mMp3Dec = (mp3DecEngine *)malloc(sizeof(mp3DecEngine));
        if (mMp3Dec == NULL) {
            ALOGD("%s() allocate engine fail", __FUNCTION__);
        }
        memset(mMp3Dec, 0, sizeof(mp3DecEngine));

        MP3Dec_GetMemSize(&mMp3Dec->min_bs_size, &mMp3Dec->pcm_size, &mMp3Dec->workingbuf_size1, &mMp3Dec->workingbuf_size2);
        ALOGD("%s >> min_bs_size=%u, pcm_size=%u, workingbuf_size1=%u,workingbuf_size2=%u", __FUNCTION__,
              mMp3Dec->min_bs_size, mMp3Dec->pcm_size, mMp3Dec->workingbuf_size1, mMp3Dec->workingbuf_size2);

        mMp3Dec->working_buf1 = malloc(mMp3Dec->workingbuf_size1);
        mMp3Dec->working_buf2 = malloc(mMp3Dec->workingbuf_size2);
        mMp3Dec->pcm_buf      = malloc(mMp3Dec->pcm_size);

        if ((NULL == mMp3Dec->working_buf1) || (NULL == mMp3Dec->working_buf2)) {
            ALOGD("%s() allocate working buf fail", __FUNCTION__);
            return false;
        }

        memset(mMp3Dec->working_buf1, 0, mMp3Dec->workingbuf_size1);
        memset(mMp3Dec->working_buf2, 0, mMp3Dec->workingbuf_size2);
        memset(mMp3Dec->pcm_buf, 0, mMp3Dec->pcm_size);

        if (mMp3Dec->handle == NULL) {
            mMp3Dec->handle = MP3Dec_Init(mMp3Dec->working_buf1, mMp3Dec->working_buf2);

            if (mMp3Dec->handle == NULL) {
                ALOGD("%s() Init Decoder Fail", __FUNCTION__);

                if (mMp3Dec->working_buf1) {
                    free(mMp3Dec->working_buf1);
                    mMp3Dec->working_buf1 = NULL;
                }

                if (mMp3Dec->working_buf2) {
                    free(mMp3Dec->working_buf2);
                    mMp3Dec->working_buf2 = NULL;
                }

                if (mMp3Dec->pcm_buf) {
                    free(mMp3Dec->pcm_buf);
                    mMp3Dec->pcm_buf = NULL;
                }

                free(mMp3Dec);
                mMp3Dec = NULL;
                return false;
            }
        }

        mMp3InitFlag = true;
        mEndFlag = false;
        ALOGD("-%s()", __FUNCTION__);
    }

    return true;
}

void AudioMTKDecHandlerMP3::DeinitAudioDecoder() {
    ALOGD("+%s()", __FUNCTION__);

    if ((mMp3InitFlag == true) && (mMp3Dec != NULL)) {
        if (mMp3Dec->working_buf1) {
            free(mMp3Dec->working_buf1);
            mMp3Dec->working_buf1 = NULL;
        }

        if (mMp3Dec->working_buf2) {
            free(mMp3Dec->working_buf2);
            mMp3Dec->working_buf2 = NULL;
        }

        if (mMp3Dec->pcm_buf) {
            free(mMp3Dec->pcm_buf);
            mMp3Dec->pcm_buf = NULL;
        }

        free(mMp3Dec);
        mMp3Dec = NULL;
        mMp3InitFlag = false;
    }
    ALOGD("-%s()", __FUNCTION__);
}

int32_t AudioMTKDecHandlerMP3::ParseAudioHeader(int8_t *inputBsbuf) {
    int32_t ver_idx, mp3_version, layer, bit_rate_idx, sample_rate_idx, channel_idx;
    struct mp3_header header;
    ALOGD("+%s()", __FUNCTION__);

    memcpy((void *)&header, (void *)inputBsbuf, sizeof(header));

    /* check sync bits */
    if ((header.sync & MP3_SYNC) != MP3_SYNC) {
        ALOGD("%s(), Can't find sync word", __FUNCTION__);
        return -1;
    }
    ver_idx = (header.sync >> 11) & 0x03;
    mp3_version = ver_idx == 0 ? MPEG25 : ((ver_idx & 0x1) ? MPEG1 : MPEG2);
    layer = 4 - ((header.sync >> 9) & 0x03);
    bit_rate_idx = ((header.format1 >> 4) & 0x0f);
    sample_rate_idx = ((header.format1 >> 2) & 0x03);
    channel_idx = ((header.format2 >> 6) & 0x03);

    if (sample_rate_idx == 3 || layer == 4 || bit_rate_idx == 15) {
        ALOGD("%s(), Error: Can't find valid header", __FUNCTION__);
        return -1;
    }
    mChannel = (channel_idx == MONO ? 1 : 2);
    mSampleRate = mp3_sample_rates[mp3_version][sample_rate_idx];
    mBitRate = (mp3_bit_rates[mp3_version][layer - 1][bit_rate_idx]) * 1000;

    ALOGD("-%s()", __FUNCTION__);
    return 0;
}

int32_t AudioMTKDecHandlerMP3::BsbufferSize() {
    return mMp3Dec->min_bs_size;
}

int32_t AudioMTKDecHandlerMP3::PcmbufferSize() {
    return mMp3Dec->pcm_size;
}

int32_t AudioMTKDecHandlerMP3::DecodeAudio(int8_t *inputBsbuf, int8_t *outputPcmbuf, int32_t BsbufSize) {
    ALOGV("%s():%p %p", __FUNCTION__, inputBsbuf, outputPcmbuf);
    int32_t ret;
    ret = MP3Dec_Decode(mMp3Dec->handle, (void *)mMp3Dec->pcm_buf, (void *)inputBsbuf, BsbufSize, (void *)inputBsbuf);
    memcpy(outputPcmbuf, mMp3Dec->pcm_buf, mMp3Dec->pcm_size);

    return ret;
}

}
