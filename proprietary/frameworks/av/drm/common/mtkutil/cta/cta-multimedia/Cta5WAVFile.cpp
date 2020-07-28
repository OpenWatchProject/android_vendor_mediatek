#define LOG_NDEBUG 0
#define LOG_TAG "Cta5WAVFile"
#include <utils/Log.h>

#include <Cta5WAVFile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <cutils/bitops.h>


#define CHANNEL_MASK_USE_CHANNEL_ORDER 0

using namespace android;

enum {
    WAVE_FORMAT_PCM        = 0x0001,
    WAVE_FORMAT_ALAW       = 0x0006,
    WAVE_FORMAT_MULAW      = 0x0007,
    WAVE_FORMAT_MSGSM      = 0x0031,
    WAVE_FORMAT_EXTENSIBLE = 0xFFFE,
    WAVE_FORMAT_MSADPCM    = 0x0002,
    WAVE_FORMAT_DVI_IMAADCPM = 0x0011
};

static const char* WAVEEXT_SUBFORMAT = "\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71";

static uint32_t U32_LE_AT(const uint8_t *ptr) {
    return ptr[3] << 24 | ptr[2] << 16 | ptr[1] << 8 | ptr[0];
}

static uint16_t U16_LE_AT(const uint8_t *ptr) {
    return ptr[1] << 8 | ptr[0];
}

Cta5WAVFile::Cta5WAVFile(int fd, String8 key) : Cta5CommonMultimediaFile(fd, key)
{
    ALOGD("Cta5WAVFile(fd, key)");
}

//This constructor is useful when you want to get a Cta5 file format
//To convert a normal file to a CTA5 file, you may need this version
Cta5WAVFile::Cta5WAVFile(String8 mimeType, String8 cid, String8 dcfFlHeaders,
        uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, cid, dcfFlHeaders, datatLen, key)
{
    ALOGD("Cta5CommonMultimediaFile(mimeType, cid, dcfFlHeaders, datatLen, key)");
}

Cta5WAVFile::Cta5WAVFile(String8 mimeType, uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, datatLen, key)
{
    ALOGD("Cta5MPEGFile(mimeType, datatLen, key)");
}

bool Cta5WAVFile::parseHeaders(int fd) {
    ALOGD("parseHeaders()");
     uint8_t header[12];
     if (DrmCtaMultiMediaUtil::readAt(
                 fd, 0, header, sizeof(header)) < (ssize_t)sizeof(header)) {
         return false;
     }

     if (memcmp(header, "RIFF", 4) || memcmp(&header[8], "WAVE", 4)) {
         return false;
     }

     size_t totalSize = U32_LE_AT(&header[4]);

     off64_t offset = 12;
     size_t remainingSize = totalSize;
     while (remainingSize >= 8) {
         uint8_t chunkHeader[8];
         if (DrmCtaMultiMediaUtil::readAt(fd, offset, chunkHeader, 8) < 8) {
             return false;
         }

         remainingSize -= 8;
         offset += 8;

         uint32_t chunkSize = U32_LE_AT(&chunkHeader[4]);

         if (chunkSize > remainingSize) {
             return false;
         }

         if (!memcmp(chunkHeader, "fmt ", 4)) {
             if (chunkSize < 16) {
                 return false;
             }

             uint8_t formatSpec[40];
             if (DrmCtaMultiMediaUtil::readAt(fd, offset, formatSpec, 2) < 2) {
                 return false;
             }

             mWaveFormat = U16_LE_AT(formatSpec);
             if (mWaveFormat != WAVE_FORMAT_PCM
                     && mWaveFormat != WAVE_FORMAT_ALAW
                     && mWaveFormat != WAVE_FORMAT_MULAW
 #ifdef AUDIO_ADPCM_SUPPORT
                     && mWaveFormat != WAVE_FORMAT_MSADPCM
                     && mWaveFormat != WAVE_FORMAT_DVI_IMAADCPM
 #endif
                     && mWaveFormat != WAVE_FORMAT_MSGSM
                     && mWaveFormat != WAVE_FORMAT_EXTENSIBLE) {
                 return false;
             }


             uint8_t fmtSize = 16;
             if (mWaveFormat == WAVE_FORMAT_EXTENSIBLE) {
                 fmtSize = 40;
             }
             if (DrmCtaMultiMediaUtil::readAt(fd, offset, formatSpec, fmtSize) < fmtSize) {
                 return false;
             }

             mNumChannels = U16_LE_AT(&formatSpec[2]);
             if (mWaveFormat != WAVE_FORMAT_EXTENSIBLE) {
                 if (mNumChannels != 1 && mNumChannels != 2) {
                     ALOGW("More than 2 channels (%d) in non-WAVE_EXT, unknown channel mask",
                             mNumChannels);
                     if(mNumChannels == 0){
                         return false;
                     }
                 }
             } else {
                 if (mNumChannels < 1 && mNumChannels > 8) {
                     return false;
                 }
                 if (mNumChannels < 1 || mNumChannels > 8) {
                         ALOGW("channel count is %d", mNumChannels);
                         return false;
                 }
             }

             mSampleRate = U32_LE_AT(&formatSpec[4]);

             if (mSampleRate == 0) {
                 return false;
             }
             ALOGD("mNumChannels is %d, mSampleRate is %d", mNumChannels, mSampleRate);
             mAvgBytesPerSec = U32_LE_AT(&formatSpec[8]);
             if(mAvgBytesPerSec <= 0)
             {
                 return false;
             }

             mBlockAlign = U16_LE_AT(&formatSpec[12]);
             if(mBlockAlign <= 0)
             {
                 return false;
             }

             if(mWaveFormat == WAVE_FORMAT_MSADPCM || mWaveFormat == WAVE_FORMAT_DVI_IMAADCPM || mWaveFormat == WAVE_FORMAT_MSGSM)
                 ALOGD("mBlockAlign is %d", mBlockAlign);

             mBitsPerSample = U16_LE_AT(&formatSpec[14]);

             if (mWaveFormat == WAVE_FORMAT_PCM
                     || mWaveFormat == WAVE_FORMAT_EXTENSIBLE) {
                 if (mBitsPerSample != 8 && mBitsPerSample != 16
                     && mBitsPerSample != 24
 #ifdef AUDIO_RAW_SUPPORT
                     && mBitsPerSample != 32
 #endif
                 ) {
                     return false;
                 }
             }
 #ifdef AUDIO_ADPCM_SUPPORT
             else if(mWaveFormat == WAVE_FORMAT_MSADPCM || mWaveFormat == WAVE_FORMAT_DVI_IMAADCPM)
             {
                 if(mBitsPerSample != 4)
                 {
                     return false;
                 }
             }
 #endif

             else if(mWaveFormat == WAVE_FORMAT_MSGSM)
             {
                 if(mBitsPerSample != 0)
                     return false;
             }
             else {
                if(!(mWaveFormat == WAVE_FORMAT_MULAW
                         || mWaveFormat == WAVE_FORMAT_ALAW)) {
                    return false;
                }
                 if (mBitsPerSample != 8) {
                     return false;
                 }
             }

 #ifdef AUDIO_ADPCM_SUPPORT
             if(mWaveFormat == WAVE_FORMAT_MSADPCM || mWaveFormat == WAVE_FORMAT_DVI_IMAADCPM)
             {
                 uint8_t extraData[2];
                 if (DrmCtaMultiMediaUtil::readAt(fd, offset+16, extraData, 2) < 2)
                 {
                        return false;
                 }

                 mExtraDataSize = U16_LE_AT(extraData);
                 if(mExtraDataSize < 2)
                 {
                     return false;

                 }
                 mpExtraData = (uint8_t*)malloc(mExtraDataSize);
                 if(NULL == mpExtraData)
                 {
                     ALOGD("ADPCM malloc extraDataSize failed !!!");
                     return false;
                 }
                 else
                 {
                     ALOGD("ADPCM mExtraDataSize is %d", mExtraDataSize);
                     if(DrmCtaMultiMediaUtil::readAt(fd, offset+18, mpExtraData, mExtraDataSize) < mExtraDataSize)
                     {
                         return false;
                     }
                 }
                 mSamplesPerBlock = U16_LE_AT(mpExtraData);
                 if(mExtraDataSize < 0)
                 {
                     return false;
                 }

             }
 #endif
             if (mWaveFormat == WAVE_FORMAT_EXTENSIBLE) {
                 uint16_t validBitsPerSample = U16_LE_AT(&formatSpec[18]);
                 if (validBitsPerSample != mBitsPerSample) {
                     if (validBitsPerSample != 0) {
                         ALOGE("[ERROR][CTA5]validBits(%d) != bitsPerSample(%d) are not supported",
                                 validBitsPerSample, mBitsPerSample);
                         return false;
                     } else {
                         // we only support valitBitsPerSample == bitsPerSample but some WAV_EXT
                         // writers don't correctly set the valid bits value, and leave it at 0.
                         ALOGW("WAVE_EXT has 0 valid bits per sample, ignoring");
                     }
                 }

                 mChannelMask = U32_LE_AT(&formatSpec[20]);
                 ALOGV("numChannels=%d channelMask=0x%x", mNumChannels, mChannelMask);
                 if ((mChannelMask >> 18) != 0) {
                     ALOGE("[ERROR][CTA5]invalid channel mask 0x%x", mChannelMask);
                     return false;
                 }

                 if ((mChannelMask != CHANNEL_MASK_USE_CHANNEL_ORDER)
                         && (popcount(mChannelMask) != mNumChannels)) {
                     ALOGE("[ERROR][CTA5]invalid number of channels (%d) in channel mask (0x%x)",
                             popcount(mChannelMask), mChannelMask);
                     return false;
                 }

                 // In a WAVE_EXT header, the first two bytes of the GUID stored at byte 24 contain
                 // the sample format, using the same definitions as a regular WAV header
                 mWaveFormat = U16_LE_AT(&formatSpec[24]);
                 if (mWaveFormat != WAVE_FORMAT_PCM
                         && mWaveFormat != WAVE_FORMAT_ALAW
                         && mWaveFormat != WAVE_FORMAT_MULAW
 #ifdef AUDIO_ADPCM_SUPPORT
                         && mWaveFormat != WAVE_FORMAT_MSADPCM
                         && mWaveFormat != WAVE_FORMAT_DVI_IMAADCPM
 #endif
                         && mWaveFormat != WAVE_FORMAT_MSGSM
                 )
                         {
                     return false;
                 }
                 if (memcmp(&formatSpec[26], WAVEEXT_SUBFORMAT, 14)) {
                     ALOGE("[ERROR][CTA5]unsupported GUID");
                     return false;
                 }
             }

             mValidFormat = true;
         }
         else if(!memcmp(chunkHeader, "fact", 4))
         {
             if(chunkSize != 4)
             {
                 ALOGD("fact chunk size is invailed, chunkSize is %d !!!", chunkSize);
 //                return ERROR_MALFORMED;
             }
             uint8_t factChunkData[4];
             if(DrmCtaMultiMediaUtil::readAt(fd, offset, factChunkData, 4) < 4)
             {
 //                return ERROR_MALFORMED;
             }
             mSamplesNumberPerChannel = U32_LE_AT(factChunkData);
             if(mSamplesNumberPerChannel < 0)
             {
 //                return ERROR_MALFORMED;
             }
             ALOGD("fact chunk mChannelCount is %d, mSamplesNumberPerChannel is %d, mSamplesPerBlock is %d", mNumChannels, mSamplesNumberPerChannel, mSamplesPerBlock);
         }
         else if (!memcmp(chunkHeader, "data", 4)) {
             if (mValidFormat) {
                 mDataOffset = offset;
                 mDataSize = chunkSize;

                // remove set metadata

                // get offset
                ALOGD("found wav header, offset[0], size[%ld]", (long) offset);

                // stored header information into Vector<header>
                Header * wav_header = new Header();
                wav_header->clear_header_offset = 0;
                wav_header->clear_header_size = offset;

                mHeaders.push_back(wav_header);

                return true;
             }
         }

         offset += chunkSize;
     }
    return false;
}


