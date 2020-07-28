#define LOG_TAG  "AudioMTKFilter"

#define MTK_LOG_ENABLE 1
#include <cutils/compiler.h>

#include "audio_custom_exp.h"
#include "AudioCustParamClient.h"
#include"AudioMTKFilter.h"
#include <log/log.h>

#include "AudioType.h"
#include "AudioUtility.h"
//#if defined(MTK_VIBSPK_SUPPORT)
#include "AudioCompFltCustParam.h"
#include "AudioVIBSPKControl.h"
//#endif
#include "AudioFtm.h"
#include "AudioMTKFilter.h"

#ifndef VIBSPK_DEFAULT_FREQ
#define VIBSPK_DEFAULT_FREQ     (156) //141~330 Hz
#endif

//#define MERGE_HCF_AUDENH_WITH_UI_OPTION

namespace {

#define AUDIO_COMPENSATION_FLT_MODE AUDIO_CMP_FLT_LOUDNESS_COMP
#define AUDIO_COMPENSATION_FLT_MODE_LOW_LATENCY_WITH_LIMITER AUDIO_CMP_FLT_LOUDNESS_COMP_LOW_LATENCY_WITH_LIMITER
#define AUDIO_COMPENSATION_FLT_MODE_LOW_LATENCY_WITHOUT_LIMITER AUDIO_CMP_FLT_LOUDNESS_COMP_LOW_LATENCY_WITHOUT_LIMITER



}


namespace android {
#define PROPERTY_KEY_AUDENH_SWITCH_STATE "persist.vendor.audiohal.audenh_state";
#define AUDENH_SWITCH_DEFAULT_STATE (0)

AudioMTKFilter::AudioMTKFilter(
    AudioCompFltType_t type,
    AudioComFltMode_t mode,
    uint32_t sampleRate,
    uint32_t channel,
    uint32_t format,
    size_t bufferSize) :
    bIsZeroCoeff(false),
    mType(type),
    mMode(mode),
    mSampleTate(sampleRate),
    mChannel(channel),
    mFormat(format),
    mBufferSize(bufferSize),
    mFilter(NULL),
    mStart(false),
    mActive(false) {
}

AudioMTKFilter::~AudioMTKFilter() {
    if (mFilter) {
        mFilter->close();
        deleteMtkAudioLoud(mFilter);
        mFilter = NULL;
    }
}

status_t AudioMTKFilter::init() {
    Mutex::Autolock _l(&mLock);

    if (mType < AUDIO_COMP_FLT_NUM && mFilter == NULL) {
        mFilter = newMtkAudioLoud(mType);

        if (NULL != mFilter) {
            int format = (mFormat == AUDIO_FORMAT_PCM_32_BIT ? BLOUD_IN_Q1P31_OUT_Q1P31 : BLOUD_IN_Q1P15_OUT_Q1P15);
            mFilter->setParameter(BLOUD_PAR_SET_PCM_FORMAT, (void *)((long)format));
            mFilter->setParameter(BLOUD_PAR_SET_CHANNEL_NUMBER, (void *)((long)mChannel));
            mFilter->setParameter(BLOUD_PAR_SET_SAMPLE_RATE, (void *)((long)mSampleTate));
            mFilter->setParameter(BLOUD_PAR_SET_WORK_MODE, (void *)((long)mMode));
            mFilter->setParameter(BLOUD_PAR_SET_USE_DEFAULT_PARAM, (void *)NULL);
#if defined(ENABLE_STEREO_SPEAKER)&&defined(MTK_STEREO_SPK_ACF_TUNING_SUPPORT)
            if (AUDIO_COMP_FLT_AUDIO == mType) {
                mFilter->setParameter(BLOUD_PAR_SET_USE_DEFAULT_PARAM_SUB, (void *)NULL);
            }
#else
            if (AUDIO_COMP_FLT_AUDIO == mType) {
                mFilter->setParameter(BLOUD_PAR_SET_USE_DEFAULT_PARAM_SUB, (void *)NULL);
            }
#endif
            bIsZeroCoeff = mFilter->isZeroCoeffFilter();
            return NO_ERROR;
        } else {
            ALOGE("Error: %s Line#%d mType %d mFilter %p", __FUNCTION__, __LINE__, mType, mFilter);
            return NO_INIT;
        }
    } else {
        ALOGE("Error: %s Line#%d mType %d mFilter %p", __FUNCTION__, __LINE__, mType, mFilter);
        return INVALID_OPERATION;
    }
}

void AudioMTKFilter::start(bool bFirstDataWrite) {
    Mutex::Autolock _l(mLock);
    if (mFilter && !mActive) {
        ALOGD("AudioMTKFilter::start() type %d mode %d bFirstDataWrite %d", mType, mMode, bFirstDataWrite);

        mFilter->setWorkMode(mChannel, mSampleTate, mMode, bFirstDataWrite ? false : true);
        mFilter->open();
        mStart  = true;
        mActive = true;
    }
    return;
}

void AudioMTKFilter::stop() {
    Mutex::Autolock _l(mLock);
    if (mFilter && mActive) {
        ALOGD("AudioMTKFilter::stop() type %d mode %d", mType, mMode);
        //mFilter->Stop();
        mFilter->resetBuffer();
        mFilter->close();
        mStart  = false;
        mActive = false;
    }
    return;
}

void AudioMTKFilter::pause() {
    Mutex::Autolock _l(mLock);
    if (mFilter && mActive) {
        ALOGD("AudioMTKFilter::pause() type %d mode %d", mType, mMode);
        if (mFilter->change2ByPass() == ACE_SUCCESS) {
            mActive = false;
        }
    }
}

void AudioMTKFilter::resume() {
    Mutex::Autolock _l(mLock);
    if (mFilter && !mActive) {
        ALOGD("AudioMTKFilter::resume() type %d mode %d", mType, mMode);
        if (mFilter->change2Normal() == ACE_SUCCESS) {
            mActive = true;
        }
    }
}

bool AudioMTKFilter::isStart() {
    Mutex::Autolock _l(mLock);
    return mStart;
}

bool AudioMTKFilter::isActive() {
    Mutex::Autolock _l(mLock);
    return mActive;
}

void AudioMTKFilter::setParameter(void *param) {
    Mutex::Autolock _l(mLock);
    if (mFilter) {
        ALOGD("AudioMTKFilter::setParameter type %d mode %d mActive %d", mType, mMode, mActive);
        mFilter->resetBuffer();
        mFilter->close();
        mFilter->setParameter(BLOUD_PAR_SET_CHANNEL_NUMBER, (void *)((long)mChannel));
        mFilter->setParameter(BLOUD_PAR_SET_SAMPLE_RATE, (void *)((long)mSampleTate));
        mFilter->setParameter(BLOUD_PAR_SET_WORK_MODE, (void *)((long)mMode));
        mFilter->setParameter(BLOUD_PAR_SET_PREVIEW_PARAM, (void *)param);
        mFilter->open();
        if (!mActive && mStart) {
            mFilter->change2ByPass();
        }

        bIsZeroCoeff = mFilter->isZeroCoeffFilter();
    }
}

void AudioMTKFilter::setOutputGain(int32_t gain, uint32_t ramp_sample_cnt) {
    Mutex::Autolock _l(mLock);
    if (mFilter != NULL) {
        mFilter->setOutputGain(gain, ramp_sample_cnt);
    }
    return;
}

void AudioMTKFilter::setFilterParam(unsigned int fc, unsigned int bw, int th) {
    Mutex::Autolock _l(mLock);
    if (mFilter != NULL) {
        mFilter->setNotchFilterParam(fc, bw, th);
    }
    return;
}

void AudioMTKFilter::setParameter2Sub(void *param) {
#if (defined(ENABLE_STEREO_SPEAKER)&&defined(MTK_STEREO_SPK_ACF_TUNING_SUPPORT))

    Mutex::Autolock _l(mLock);
    if (mFilter) {
        ALOGV("AudioMTKFilter::setParameter2Sub type %d mode %d mActive %d", mType, mMode, mActive);
        mFilter->resetBuffer();
        mFilter->close();
        mFilter->setParameter(BLOUD_PAR_SET_CHANNEL_NUMBER, (void *)((long)mChannel));
        mFilter->setParameter(BLOUD_PAR_SET_SAMPLE_RATE, (void *)((long)mSampleTate));
        mFilter->setParameter(BLOUD_PAR_SET_WORK_MODE, (void *)((long)mMode));
        mFilter->setParameter(BLOUD_PAR_SET_PREVIEW_PARAM_SUB, (void *)param);
        mFilter->open();
        if (!mActive && mStart) {
            mFilter->change2ByPass();
        }
    }
#else
  if(!param)
      ALOGV("AudioMTKFilter:: param is NULL");
  else
    ALOGD("UnSupport Stereo Speaker.");
#endif
}


uint32_t AudioMTKFilter::process(void *inBuffer, uint32_t bytes, void *outBuffer, uint32_t outBytes) {
    // if return 0, means CompFilter can't do anything. Caller should use input buffer to write to Hw.
    // do post process
    Mutex::Autolock _l(mLock);
    if (mFilter && mStart) {
        //SXLOGD("AudioMTKFilter::process type %d mode %d", mType, mMode);
        uint32_t inBytes =  bytes;
        uint32_t outBytes2 =  outBytes;

        mFilter->process((short *)inBuffer, &inBytes, (short *)outBuffer, &outBytes2);
        //SXLOGD("AudioMTKFilter::process type %d mode %d", mType, mMode);
        return outBytes2;
    }
    return 0;
}


//filter manager
#undef  LOG_TAG
#define LOG_TAG  "AudioMTKFilterManager"

AudioMTKFilterManager::AudioMTKFilterManager(
    uint32_t sampleRate,
    uint32_t channel,
    uint32_t format,
    size_t bufferSize)
    : mSamplerate(sampleRate),
      mChannel(channel),
      mFormat(format),
      mBufferSize(bufferSize),
      mFixedParam(false),
      mSpeakerFilter(NULL),
      mHeadphoneFilter(NULL),
      mEnhanceFilter(NULL),
      //#if defined(MTK_VIBSPK_SUPPORT)
      mVIBSPKFilter(NULL),
      //#endif
      mVibspkAddToneFilter(NULL),
      mBuffer(NULL),
      mDevices(0) {
}

AudioMTKFilterManager::~AudioMTKFilterManager() {
    deinit();
}
void AudioMTKFilterManager::deinit() {
    if (mSpeakerFilter) {
        mSpeakerFilter->stop();
        delete mSpeakerFilter;
        mSpeakerFilter = NULL;
    }
    if (mHeadphoneFilter) {
        mHeadphoneFilter->stop();
        delete mHeadphoneFilter;
        mHeadphoneFilter = NULL;
    }
    if (mEnhanceFilter) {
        mEnhanceFilter->stop();
        delete mEnhanceFilter;
        mEnhanceFilter = NULL;
    }
    if (mVIBSPKFilter) {
        mVIBSPKFilter->stop();
        delete mVIBSPKFilter;
        mVIBSPKFilter = NULL;
    }
    if (mVibspkAddToneFilter) {
        delete mVibspkAddToneFilter;
        mVibspkAddToneFilter = NULL;
    }
    if (mBuffer) {
        delete[] mBuffer;
        mBuffer = NULL;
    }
}
status_t AudioMTKFilterManager::init(uint32_t flags) {
    bool bInitFail = false;
    do {
#if defined(ENABLE_AUDIO_COMPENSATION_FILTER)
        AudioComFltMode_t mSpeakerFilterMode;
        if (flags & AUDIO_OUTPUT_FLAG_FAST) {
            if (flags & AUDIO_OUTPUT_FLAG_PRIMARY) {
                mSpeakerFilterMode = AUDIO_COMPENSATION_FLT_MODE_LOW_LATENCY_WITH_LIMITER;   // with limiter
            } else {
                mSpeakerFilterMode = AUDIO_COMPENSATION_FLT_MODE_LOW_LATENCY_WITHOUT_LIMITER;   // without limiter
            }
        } else {
            mSpeakerFilterMode = AUDIO_COMPENSATION_FLT_MODE;
        }
        mSpeakerFilter = new AudioMTKFilter(AUDIO_COMP_FLT_AUDIO, mSpeakerFilterMode,
                                            mSamplerate, mChannel, mFormat, mBufferSize);
        if (mSpeakerFilter == NULL) {
            ALOGE("Error: %s Line#%d", __FUNCTION__, __LINE__);
            bInitFail = true;
            break;
        } else {
            if (mSpeakerFilter->init() != NO_ERROR) {
                ALOGE("Error: %s Line#%d", __FUNCTION__, __LINE__);
                bInitFail = true;
                break;
            }
        }
#endif

#if defined(ENABLE_HEADPHONE_COMPENSATION_FILTER)
        mHeadphoneFilter = new AudioMTKFilter(AUDIO_COMP_FLT_HEADPHONE, AUDIO_CMP_FLT_LOUDNESS_COMP_HEADPHONE,
                                              mSamplerate, mChannel, mFormat, mBufferSize);
        if (mHeadphoneFilter == NULL) {
            ALOGE("Error: %s Line#%d", __FUNCTION__, __LINE__);
            bInitFail = true;
            break;
        } else {
            if (mHeadphoneFilter->init() != NO_ERROR) {
                ALOGE("Error: %s Line#%d", __FUNCTION__, __LINE__);
                bInitFail = true;
                break;
            }
        }
#endif

#if defined(MTK_AUDENH_SUPPORT) //For reduce resource
        mEnhanceFilter = new AudioMTKFilter(AUDIO_COMP_FLT_AUDENH, AUDIO_CMP_FLT_LOUDNESS_COMP_AUDENH,
                                            mSamplerate, mChannel, mFormat, mBufferSize);

        if (mEnhanceFilter == NULL) {
            ALOGE("Error: %s Line#%d", __FUNCTION__, __LINE__);
            bInitFail = true;
            break;
        } else {
            if (mEnhanceFilter->init() != NO_ERROR) {
                ALOGE("Error: %s Line#%d", __FUNCTION__, __LINE__);
                bInitFail = true;
                break;
            }
        }

        mBuffer = new uint8_t[mBufferSize];
        if (mBuffer == NULL) {
            ALOGE("Error: %s Line#%d", __FUNCTION__, __LINE__);
            bInitFail = true;
            break;
        }
        char property_value[PROPERTY_VALUE_MAX];
        property_get(PROPERTY_KEY_AUDENH_SWITCH_STATE, property_value, AUDENH_SWITCH_DEFAULT_STATE ? "1":"0");
        int audenh_enable = atoi(property_value);
        if (audenh_enable) {
            mFixedParam = true;
        } else {
            mFixedParam = false;
        }
#endif

        if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER)) {
            mVibspkAddToneFilter = new AudioMTKFilter_VibSpkAddTone(mSamplerate, mChannel, mFormat, mBufferSize);
            if (mVibspkAddToneFilter == NULL) {
                ALOGE("Error: %s Line#%d", __FUNCTION__, __LINE__);
                bInitFail = true;
                break;
            } else {
                if (mVibspkAddToneFilter->init() != NO_ERROR) {
                    ALOGE("Error: %s Line#%d", __FUNCTION__, __LINE__);
                    bInitFail = true;
                    break;
                }
            }

            mVIBSPKFilter = new AudioMTKFilter(AUDIO_COMP_FLT_VIBSPK, AUDIO_CMP_FLT_LOUDNESS_COMP,
                                               mSamplerate, mChannel, mFormat, mBufferSize);

            if (mVIBSPKFilter == NULL) {
                ALOGE("Error: %s Line#%d", __FUNCTION__, __LINE__);
                bInitFail = true;
                break;
            } else {
                if (mVIBSPKFilter->init() != NO_ERROR) {
                    ALOGE("Error: %s Line#%d", __FUNCTION__, __LINE__);
                    bInitFail = true;
                    break;
                }
            }

            if (mBuffer == NULL) {
                mBuffer = new uint8_t[mBufferSize];
                if (mBuffer == NULL) {
                    ALOGE("Error: %s Line#%d", __FUNCTION__, __LINE__);
                    bInitFail = true;
                    break;
                }
            }
        }
    } while (0);

    if (bInitFail) {
        deinit();
        ALOGE("Error: %s Line#%d Allocate Fail", __FUNCTION__, __LINE__);
        return INVALID_OPERATION;
    } else {
        ALOGD("mFixedParam %d", mFixedParam);
        return NO_ERROR;
    }
}

void AudioMTKFilterManager::start(bool bFirstDataWrite) {
    uint32_t device = mDevices;
#if defined(CONFIG_MT_ENG_BUILD)
    ALOGV("start() device 0x%x", device);
#endif

    if (device & AUDIO_DEVICE_OUT_SPEAKER) {
        //stop hcf & enhangce
        if (mHeadphoneFilter) { mHeadphoneFilter->stop(); }
        if (mEnhanceFilter) { mEnhanceFilter->stop(); }
        // start acf
        if (mSpeakerFilter) { mSpeakerFilter->start(bFirstDataWrite); }
        //#if defined(MTK_VIBSPK_SUPPORT)
        if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER))
            if (mVIBSPKFilter) { mVIBSPKFilter->start(bFirstDataWrite); }
        //#endif

    } else if ((device & AUDIO_DEVICE_OUT_WIRED_HEADSET) || (device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
        //#if defined(MTK_VIBSPK_SUPPORT)
        if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER))
            if (mVIBSPKFilter) { mVIBSPKFilter->stop(); }
        //#endif
        // stop acf
        if (mSpeakerFilter) { mSpeakerFilter->stop(); }
        // start hcf
#ifdef MERGE_HCF_AUDENH_WITH_UI_OPTION
        if (mHeadphoneFilter && mHeadphoneFilter->bIsZeroCoeff != true) {
            if (false == mFixedParam) {
                if (mHeadphoneFilter->isStart()) { mHeadphoneFilter->pause(); }
            } else {
                if (!mHeadphoneFilter->isStart()) {
                    mHeadphoneFilter->start(bFirstDataWrite);
                } else {
                    mHeadphoneFilter->resume();
                }
            }
        } else if (mEnhanceFilter) {
            if (false == mFixedParam) {
                if (mEnhanceFilter->isStart()) { mEnhanceFilter->pause(); }
            } else {
                if (!mEnhanceFilter->isStart()) {
                    mEnhanceFilter->start(bFirstDataWrite);
                } else {
                    mEnhanceFilter->resume();
                }
            }
        }
#else
        if (mHeadphoneFilter) { mHeadphoneFilter->start(bFirstDataWrite); }

        if (mEnhanceFilter) {
            if (false == mFixedParam) {
                if (mEnhanceFilter->isStart()) { mEnhanceFilter->pause(); }
            } else {
                if (!mEnhanceFilter->isStart()) {
                    mEnhanceFilter->start(bFirstDataWrite);
                } else {
                    mEnhanceFilter->resume();
                }
            }
        }
#endif
    }
    //#if defined(MTK_VIBSPK_SUPPORT)
    else if (device & AUDIO_DEVICE_OUT_EARPIECE) {
        if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER) && IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER)) {
            if (mHeadphoneFilter) { mHeadphoneFilter->stop(); }
            if (mEnhanceFilter) { mEnhanceFilter->stop(); }
            if (mSpeakerFilter) { mSpeakerFilter->stop(); }
            if (mVIBSPKFilter) { mVIBSPKFilter->start(bFirstDataWrite); }
        }
    }
    //#endif

}

void AudioMTKFilterManager::stop() {
    ALOGV("stop()");
    //#if defined(MTK_VIBSPK_SUPPORT)
    if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER))
        if (mVIBSPKFilter) { mVIBSPKFilter->stop(); }
    //#endif
    if (mSpeakerFilter) { mSpeakerFilter->stop(); }
    if (mHeadphoneFilter) { mHeadphoneFilter->stop(); }
    if (mEnhanceFilter) { mEnhanceFilter->stop(); }
}

bool  AudioMTKFilterManager::isFilterStart(uint32_t type) {
    if (type == AUDIO_COMP_FLT_AUDIO && mSpeakerFilter) {
        return mSpeakerFilter->isStart();
    } else if (type == AUDIO_COMP_FLT_HEADPHONE && mHeadphoneFilter) {
        return mHeadphoneFilter->isStart();
    } else if (type == AUDIO_COMP_FLT_AUDENH && mEnhanceFilter) {
        return mEnhanceFilter->isStart();
    }
    //#if defined(MTK_VIBSPK_SUPPORT)
    else if (type == AUDIO_COMP_FLT_VIBSPK && mVIBSPKFilter && (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER))) {
        return mVIBSPKFilter->isStart();
    }
    //#endif

    return false;
}

void AudioMTKFilterManager::setParamFixed(bool flag) {
    ALOGD("setParamFixed() flag %d", flag);

#if defined(MTK_AUDENH_SUPPORT)
    mFixedParam = flag;
    property_set(PROPERTY_KEY_AUDENH_SWITCH_STATE, (mFixedParam) ? "1" : "0");
#else
    ALOGW("Unsupport AudEnh Feature");
#endif
}

bool AudioMTKFilterManager::isParamFixed() {
    ALOGD("mFixedParam [%d]", mFixedParam);
    return mFixedParam;
}

void AudioMTKFilterManager::setDevice(uint32_t devices) {
    mDevices = devices;
    return;
}

uint32_t  AudioMTKFilterManager::process(void *inBuffer, uint32_t bytes, void *outBuffer, uint32_t outSize) {
#if defined(CONFIG_MT_ENG_BUILD)
    ALOGV("+process() insize %u", bytes);
#endif
    uint32_t outputSize = 0;
    //#ifndef MTK_BASIC_PACKAGE
#if 1
    uint32_t device = mDevices;
    if (device & AUDIO_DEVICE_OUT_SPEAKER) {
        if (mSpeakerFilter) {
            if (mSpeakerFilter->isStart()) {
                outputSize = mSpeakerFilter->process(inBuffer, bytes, outBuffer, outSize);
            }
        }
        //#if defined(MTK_VIBSPK_SUPPORT)
        if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER)) {
            if (mVIBSPKFilter) { //Notch filter
                if (mVIBSPKFilter->isStart()) {
                    void *out;
                    void *in;
                    if (CC_UNLIKELY(outputSize == 0)) {
                        in = inBuffer;
                    } else {
                        in = outBuffer;

                    }

                    out = mBuffer;
                    outputSize = mVIBSPKFilter->process(in, bytes, out, outSize);

                    //Temp Use mSwapBufferVoIP for memory reduce
                    void *vibspkin = out;
                    void *vibspkout = outBuffer;
                    mVibspkAddToneFilter->DoVibSignal2DLProcess(vibspkout, vibspkin, outputSize);


                }

            }
        }
        //#endif
    } else if ((device & AUDIO_DEVICE_OUT_WIRED_HEADSET) || (device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) {
        if (mEnhanceFilter) {
            if (mEnhanceFilter->isStart()) {
                outputSize = mEnhanceFilter->process(inBuffer, bytes, outBuffer, outSize);
            }
        }
        if (mHeadphoneFilter) {
            if (mHeadphoneFilter->isStart()) {
                if (CC_UNLIKELY(outputSize == 0)) {
                    outputSize = mHeadphoneFilter->process(inBuffer, bytes, outBuffer, outSize);
                } else {
                    void *in = outBuffer;
                    void *out = mBuffer;
                    outputSize = mHeadphoneFilter->process(in, outputSize, out, outSize);
                    if (outputSize > 0) { memcpy(outBuffer, out, outputSize); }
                }
            }
        }
    }
    //#if defined(MTK_VIBSPK_SUPPORT)
    else if (device & AUDIO_DEVICE_OUT_EARPIECE) {
        if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER) && IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER)) {
            if (mVIBSPKFilter) {
                if (mVIBSPKFilter->isStart()) {
                    outputSize = mVIBSPKFilter->process(inBuffer, bytes, outBuffer, outSize);
                }
            }
        }
    }
    //#endif
#endif
#if defined(CONFIG_MT_ENG_BUILD)
    ALOGV("-process() outsize %u", outputSize);
#endif
    return outputSize;

}

void AudioMTKFilterManager::setParameter(uint32_t type, void *param) {
    ALOGV("setParameter() type %u", type);

    if (type == AUDIO_COMP_FLT_AUDIO && mSpeakerFilter) {
        return mSpeakerFilter->setParameter(param);
    } else if (type == AUDIO_COMP_FLT_HEADPHONE && mHeadphoneFilter) {
        return mHeadphoneFilter->setParameter(param);
    } else if (type == AUDIO_COMP_FLT_AUDENH && mEnhanceFilter) {
        return mEnhanceFilter->setParameter(param);
    } else if (type == AUDIO_COMP_FLT_AUDIO_SUB && mSpeakerFilter) {
        return mSpeakerFilter->setParameter2Sub(param);
    } else {
        // MTKFilter doesn't process DRC LID
        ALOGW("MTKFilter doesn't process FLDID [%d]", type);
        return;
    }
}

void AudioMTKFilterManager::setSpkOutputGain(int32_t gain, uint32_t ramp_sample_cnt) {
    ALOGV("setSpkMntrGain() target gain %d, sample %d", gain, ramp_sample_cnt);
    if (mSpeakerFilter != NULL) {
        return mSpeakerFilter->setOutputGain(gain, ramp_sample_cnt);
    }
    //Only speaker, need to update others in the future?
}

void AudioMTKFilterManager::setSpkFilterParam(unsigned int fc, unsigned int bw, int th) {
    if (mSpeakerFilter != NULL) {
        return mSpeakerFilter->setFilterParam(fc, bw, th);
    }
}

//#if defined(MTK_VIBSPK_SUPPORT)
const unsigned int AUD_VIBR_FILTER_COEF_Table[VIBSPK_FILTER_NUM][36] = {
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_141,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_144,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_147,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_150,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_153,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_156,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_159,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_162,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_165,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_168,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_171,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_174,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_177,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_180,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_183,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_186,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_189,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_192,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_195,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_198,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_201,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_204,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_207,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_210,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_213,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_216,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_219,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_222,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_225,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_228,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_231,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_234,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_237,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_240,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_243,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_246,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_249,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_252,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_255,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_258,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_261,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_264,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_267,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_270,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_273,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_276,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_279,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_282,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_285,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_288,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_291,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_294,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_297,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_300,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_303,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_306,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_309,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_312,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_315,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_318,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_321,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_324,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_327,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_330
};


AudioMTKFilter_VibSpkAddTone::AudioMTKFilter_VibSpkAddTone(
    uint32_t sampleRate,
    uint32_t channel,
    uint32_t format,
    size_t bufferSize) {
    //#if defined(MTK_VIBSPK_SUPPORT)
    ALOGD("+Init AudioMTKFilter_VibSpkAddTone");
    ALOGD("sampleRate [%d] channel [%d] format [0x%x] bufferSize [%zu]", sampleRate, channel, format, bufferSize);
    mChannels = channel;
    mSampleRate = sampleRate;
    mFormat = format;
    mBufferSize = bufferSize;
    mShifter_to_1_31_VIBSPK = NULL;
    mVibSpk = NULL;
    mVibSpkFreq = 0;
    mVibSpkEnable = false;
    mVIBSPKToneBuffer = NULL;
    mVIBSPKToneWorkBuffer = NULL;
    ALOGD("-Init AudioMTKFilter_VibSpkAddTone");
}

status_t AudioMTKFilter_VibSpkAddTone::init() {
    bool bInitFail = false;
    do {
        mVibSpk      = AudioVIBSPKControl::getInstance();
        mVibSpkFreq = AudioFtm::getInstance()->GetVibSpkCalibrationStatus();
        ALOGD("VibSpkReadFrequency:%x", mVibSpkFreq);
        if (mVibSpkFreq == 0) {
            AUDIO_ACF_CUSTOM_PARAM_STRUCT *pCali_param = new AUDIO_ACF_CUSTOM_PARAM_STRUCT;
            ASSERT(pCali_param != NULL);
            memset(pCali_param, 0, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
            memcpy(&(pCali_param->bes_loudness_f_param.V5ViVSPK.bes_loudness_bpf_coeff), (void *)&AUD_VIBR_FILTER_COEF_Table[(VIBSPK_DEFAULT_FREQ - VIBSPK_FREQ_LOWBOUND + 1) / VIBSPK_FILTER_FREQSTEP], sizeof(uint32_t)*VIBSPK_AUD_PARAM_SIZE);
            pCali_param->bes_loudness_WS_Gain_Min = VIBSPK_DEFAULT_FREQ;
            pCali_param->bes_loudness_WS_Gain_Max = VIBSPK_SETDEFAULT_VALUE;
            setAudioCompFltCustParam(AUDIO_COMP_FLT_VIBSPK, pCali_param);
            ALOGD("[VibSpk] SetDefaultFreq");
            mVibSpkFreq = VIBSPK_DEFAULT_FREQ;
            delete pCali_param;
        }

        mVibSpk->setParameters(mSampleRate/*44100*/, mVibSpkFreq, MOD_FREQ, DELTA_FREQ);
        mVibSpkEnable = false;

        mVIBSPKToneBuffer = NULL;
        mVIBSPKToneBuffer = new uint8_t[mBufferSize];
        if (mVIBSPKToneBuffer == NULL) {
            ALOGE("mVIBSPKToneBuffer for VIBSPK allocate fail Size %zu !!! \n", mBufferSize);
            ASSERT(1);
        }

        mVIBSPKToneWorkBuffer = NULL;
        mVIBSPKToneWorkBuffer = new uint8_t[mBufferSize];
        if (mVIBSPKToneWorkBuffer == NULL) {
            ALOGE("mVIBSPKToneWorkBuffer for VIBSPK allocate fail Size %zu !!! \n", mBufferSize);
            ASSERT(1);
        }

        if (mFormat == AUDIO_FORMAT_PCM_32_BIT) {
            mShifter_to_1_31_VIBSPK = newMtkAudioBitConverter(mSampleRate, mChannels, BCV_IN_Q1P15_OUT_Q1P31);
            if (mShifter_to_1_31_VIBSPK == NULL) {
                ALOGE("Error: %s Line#%d", __FUNCTION__, __LINE__);
                bInitFail = true;
                break;
            } else {
                mShifter_to_1_31_VIBSPK->open();
            }
        } else {
            mShifter_to_1_31_VIBSPK = NULL;
        }

    } while (0);

    if (bInitFail) {
        deinit();
        ALOGE("Error: %s Line#%d Allocate Fail", __FUNCTION__, __LINE__);
        return INVALID_OPERATION;
    }
    return NO_ERROR;
}

void AudioMTKFilter_VibSpkAddTone::deinit() {
    if (mShifter_to_1_31_VIBSPK) {
        deleteMtkAudioBitConverter(mShifter_to_1_31_VIBSPK);
        mShifter_to_1_31_VIBSPK = NULL;
    }

    if (mVIBSPKToneBuffer) {
        delete []mVIBSPKToneBuffer;
        mVIBSPKToneBuffer = NULL;
    }

    if (mVIBSPKToneWorkBuffer) {
        delete []mVIBSPKToneWorkBuffer;
        mVIBSPKToneWorkBuffer = NULL;
    }
}

AudioMTKFilter_VibSpkAddTone::~AudioMTKFilter_VibSpkAddTone() {
    deinit();
}


size_t  AudioMTKFilter_VibSpkAddTone::DoVibSignal2DLProcess(void *outbuffer, void *src2DLbuffer, size_t bytes) {
    bool bSkipVibTone = false;
    size_t dToneSize;
    dToneSize = (mShifter_to_1_31_VIBSPK == NULL) ? bytes : (bytes >> 1);
    void *bToneTempWrokBuf;

    if (mVibSpk->getVibSpkEnable()) {

        if (mVibSpkEnable == false) {
            mVibSpkEnable = true;
            if (mShifter_to_1_31_VIBSPK) { mShifter_to_1_31_VIBSPK->resetBuffer(); }
            mVibSpk->VibSpkRampControl(2);
        }

        mVibSpk->VibSpkProcess(dToneSize, mVIBSPKToneBuffer, mChannels);//Gen Tone
        //dumpPcm(mVIBsignalDumpFile, mVIBSPKToneBuffer, dToneSize);
    } else {
        if (mVibSpkEnable == true) {
            mVibSpkEnable = false;
            mVibSpk->VibSpkRampControl(1);
            mVibSpk->VibSpkProcess(dToneSize, mVIBSPKToneBuffer, mChannels);
            //dumpPcm(mVIBsignalDumpFile, mVIBSPKToneBuffer, dToneSize);
        } else {
            bSkipVibTone = true;
        }
    }

    if (mShifter_to_1_31_VIBSPK && !bSkipVibTone) {
        unsigned int in_size = (unsigned int)dToneSize;
        unsigned int out_size = bytes;
        mShifter_to_1_31_VIBSPK->process((void *)mVIBSPKToneBuffer, (unsigned int *)&in_size, (void *)mVIBSPKToneWorkBuffer, (unsigned int *)&out_size);
        bToneTempWrokBuf = mVIBSPKToneWorkBuffer;
    } else {
        bToneTempWrokBuf = mVIBSPKToneBuffer;
    }

    if (mShifter_to_1_31_VIBSPK) {
        int dAudioGain = 0x7FFF - mVibSpk->getVibSpkGain();
        uint32_t dSampleCount = bytes >> 2;
        int *pVibToneData = (int *)bToneTempWrokBuf;
        int *pAudioData = (int *)src2DLbuffer;
        int *pOutputData = (int *)outbuffer;

        while (dSampleCount) {
            if (bSkipVibTone) {
                *pOutputData = (*pAudioData);//(int)(((int64_t)(*pAudioData) * dAudioGain) >> 15);
            } else {
                *pOutputData = (*pVibToneData) + (int)(((int64_t)(*pAudioData) * dAudioGain) >> 15);
            }

            pOutputData++;
            pVibToneData++;
            pAudioData++;
            dSampleCount--;
        }
    } else {
        int dAudioGain = 0x7FFF - mVibSpk->getVibSpkGain();
        uint32_t dSampleCount = bytes >> 1;
        short *pVibToneData = (short *)bToneTempWrokBuf;
        short *pAudioData = (short *)src2DLbuffer;
        short *pOutputData = (short *)outbuffer;

        while (dSampleCount) {
            if (bSkipVibTone) {
                *pOutputData = (*pAudioData);//(((*pAudioData) * dAudioGain) >> 15);
            } else {
                *pOutputData = (*pVibToneData) + (((*pAudioData) * dAudioGain) >> 15);
            }

            pOutputData++;
            pVibToneData++;
            pAudioData++;
            dSampleCount--;
        }
    }


    return bytes;

}
//#endif


}

