/* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * Copyright Statement:
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */

/*
* Description:
*   This file implements Mtk Audio Loudness Header
*/
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <assert.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG  "MtkAudioLoud"

#ifdef CONFIG_MT_ENG_BUILD
#define LOG_NDEBUG 0
#endif

#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <assert.h>

#include "AudioCompFltCustParam.h"
#include "MtkAudioLoud.h"
#if defined(SYS_IMPL)
#include "AudioToolkit.h"
#endif
//#define VERY_VERBOSE_LOGGING
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif
#define BLOCK_SIZE 512
#define BLOCK_SIZE_FOR_LOW_LATENCY_DRC 256
#define BLOCK_SIZE_LOW_LATENCY  64
#define DRC_NOISEFILTER_MIN 80 // means -80db/-75db
#define NOISE_FILTER_BASE -256
#define NOISE_FILTER_STEP 5

#define HAVE_SWIP
#define AUDIO_ACF_PARAM_USE_CACHE

#define MIN_MULTI_BAND_COUNT 5
#define MAX_MULTI_BAND_COUNT 10
#define MUSIC_UP_BOUND 11   // mMusicCount > MUSIC_UP_BOUND, use MIN_MULTI_BAND_COUNT
#define MUSIC_LOW_BOUND 5   // mMusicCount < MUSIC_LOW_BOUND, use MAX_MULTI_BAND_COUNT
namespace android {
Mutex mMusicCountMutex;
int MtkAudioLoud::mMusicCount = 0;
int MtkAudioLoud::mMaxMusicCount = 10;
Mutex mFastMusicCountMutex;
int MtkAudioLoud::mFastMusicCount = 0;
int MtkAudioLoud::mMaxFastMusicCount = 3;

const int ParamCacheIndexMap[AUDIO_COMP_FLT_NUM] = {PARAM_CACHE_ACF_IDX, PARAM_CACHE_UNSUPPORT_IDX, PARAM_CACHE_UNSUPPORT_IDX, PARAM_CACHE_UNSUPPORT_IDX, PARAM_CACHE_UNSUPPORT_IDX, PARAM_CACHE_DRC_MUSIC_IDX, PARAM_CACHE_DRC_RINGTONE_IDX};
MtkAudioCustParamCache *MtkAudioCustParamCache::mAudioCustParamCache = NULL;
static Mutex InstanceLock;
MtkAudioCustParamCache *MtkAudioCustParamCache::getInstance() {
    Mutex::Autolock _l(InstanceLock);

    if (mAudioCustParamCache == NULL) {
        mAudioCustParamCache = new MtkAudioCustParamCache();
    }
    assert(mAudioCustParamCache != NULL);
    return mAudioCustParamCache;
}

ACE_ERRID MtkAudioCustParamCache::destroyInstance() {
    Mutex::Autolock _l(InstanceLock);

    if (mAudioCustParamCache == NULL) {
        ALOGW("destroyInstance Fail: mAudioCustParamCache = NULL");
        return ACE_NOT_INIT;
    } else {
        ALOGD("Doing destroyInstance");
        delete mAudioCustParamCache;
        mAudioCustParamCache = NULL;
        ALOGD("Done destroyInstance");
        return ACE_SUCCESS;
    }
}

MtkAudioCustParamCache::MtkAudioCustParamCache() {
    ALOGD("MtkAudioCustParamCache");
    memset(mValid, 0, PARAM_CACHE_VALID_NUM * sizeof(bool));
    memset(mAudioParam, 0, PARAM_CACHE_VALID_NUM * sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    memset(&mTemp, 0, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    memset(mCustScene, 0, PARAM_CACHE_VALID_NUM * SCENE_NAME_SIZE * sizeof(char));   // 0 means '\0'
}

MtkAudioCustParamCache::~MtkAudioCustParamCache() {
    ALOGD("~MtkAudioCustParamCache");
}

ACE_ERRID MtkAudioCustParamCache::loadParameter(uint32_t FilterType, AUDIO_ACF_CUSTOM_PARAM_STRUCT *AudioParam, const char *custScene) {
    if (FilterType >= AUDIO_COMP_FLT_NUM) {
        ALOGE("%s FilterType %d >= AUDIO_COMP_FLT_NUM %d", __FUNCTION__, FilterType, AUDIO_COMP_FLT_NUM);
        return ACE_INVALIDE_PARAMETER;
    } else if (ParamCacheIndexMap[FilterType] == PARAM_CACHE_UNSUPPORT_IDX) {
        ALOGE("%s We don't support the cache for FilterType %d", __FUNCTION__, FilterType);
        return ACE_INVALIDE_PARAMETER;
    } else if (AudioParam == NULL) {
        ALOGE("%s AudioParam is null", __FUNCTION__);
        return ACE_INVALIDE_PARAMETER;
    }
    int idx = ParamCacheIndexMap[FilterType];
    char tmpSceneName[SCENE_NAME_SIZE];
    memset(tmpSceneName, 0, SCENE_NAME_SIZE * sizeof(char)); // must clear mem
    if (custScene == NULL) {
        // do nothing
    } else {
        unsigned int rec_str_len = SCENE_NAME_STRLEN;
        unsigned int str_len = strlen(custScene);
        if (str_len > rec_str_len) {
            str_len = rec_str_len;
        }
        strncpy(tmpSceneName, custScene, str_len);
    }
    ALOGV("%s Type [%d] idx [%d], mValid [%d], tmpSceneName [%s], mCustScene [%s]", __FUNCTION__, FilterType, idx, mValid[idx], tmpSceneName, mCustScene[idx]);
    Mutex::Autolock _l(mLock[idx]);
    if (mValid[idx] && !strncmp(tmpSceneName, mCustScene[idx], SCENE_NAME_STRLEN)) {
        memcpy((void *)AudioParam, (void *)&mAudioParam[idx], sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        ALOGD("%s HIT Cache Type [%d], Scene [%s]", __FUNCTION__, FilterType, tmpSceneName);
    } else {
#if !defined(SYS_IMPL)
        getAudioCompFltCustParamFromStorage((AudioCompFltType_t)FilterType, AudioParam, tmpSceneName);
        memcpy((void *)&mAudioParam[idx], (void *)AudioParam, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        mValid[idx] = true;
        strncpy(mCustScene[idx], tmpSceneName, SCENE_NAME_STRLEN);
        ALOGW("Warning %s Update Cache Type [%d], Scene [%s]", __FUNCTION__, FilterType, tmpSceneName);
        return ACE_WARN_MISMATCH_SCENE_NAME;
#else
        memcpy((void *)AudioParam, (void *)&mAudioParam[idx], sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        ALOGW("Warning %s, Unsupport the cache for FilterType [%d], custScene [%s] RecordValid [%d] RecordScene [%s]", __FUNCTION__, FilterType, tmpSceneName, mValid[idx], mCustScene[idx]);
        return ACE_WARN_MISMATCH_SCENE_NAME;
#endif
    }
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioCustParamCache::saveParameter(uint32_t FilterType, AUDIO_ACF_CUSTOM_PARAM_STRUCT *AudioParam, const char *custScene) {
    if (FilterType >= AUDIO_COMP_FLT_NUM) {
        ALOGE("%s FilterType %d >= AUDIO_COMP_FLT_NUM %d", __FUNCTION__, FilterType, AUDIO_COMP_FLT_NUM);
        return ACE_INVALIDE_PARAMETER;
    } else if (ParamCacheIndexMap[FilterType] == PARAM_CACHE_UNSUPPORT_IDX) {
        ALOGE("%s We don't support the cache for FilterType %d", __FUNCTION__, FilterType);
        return ACE_INVALIDE_PARAMETER;
    } else if (AudioParam == NULL) {
        ALOGE("%s AudioParam is null", __FUNCTION__);
        return ACE_INVALIDE_PARAMETER;
    }
    int idx = ParamCacheIndexMap[FilterType];
    char tmpSceneName[SCENE_NAME_SIZE];
    memset(tmpSceneName, 0, SCENE_NAME_SIZE * sizeof(char)); // must clear mem
    if (custScene == NULL) {
        // do nothing
        } else {
        unsigned int rec_str_len = SCENE_NAME_STRLEN;
        unsigned int str_len = strlen(custScene);
        if (str_len > rec_str_len) {
            str_len = rec_str_len;
        }
        strncpy(tmpSceneName, custScene, str_len);
    }
    ALOGV("%s Type [%d] idx [%d], mValid [%d], tmpSceneName [%s], mCustScene [%s]", __FUNCTION__, FilterType, idx, mValid[idx], tmpSceneName, mCustScene[idx]);
    Mutex::Autolock _l(mLock[idx]);
    memcpy((void *)&mAudioParam[idx], (void *)AudioParam, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    mValid[idx] = true;
    strncpy(mCustScene[idx], tmpSceneName, SCENE_NAME_STRLEN);
    ALOGD("%s Update Cache Type [%d], Scene [%s]",__FUNCTION__ , FilterType, tmpSceneName);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioCustParamCache::saveEncodedParameter(uint32_t FilterType, String8 keys, const char *custScene) {
#if defined(SYS_IMPL)
    if (FilterType >= AUDIO_COMP_FLT_NUM) {
        ALOGE("%s FilterType %d >= AUDIO_COMP_FLT_NUM %d", __FUNCTION__, FilterType, AUDIO_COMP_FLT_NUM);
        return ACE_INVALIDE_PARAMETER;
    } else if (ParamCacheIndexMap[FilterType] == PARAM_CACHE_UNSUPPORT_IDX) {
        ALOGE("%s We don't support the cache for FilterType %d", __FUNCTION__, FilterType);
        return ACE_INVALIDE_PARAMETER;
    }

    int idx = ParamCacheIndexMap[FilterType];
    char tmpSceneName[SCENE_NAME_SIZE];
    memset(tmpSceneName, 0, SCENE_NAME_SIZE * sizeof(char)); // must clear mem
    if (custScene == NULL) {
        // do nothing
    } else {
        unsigned int rec_str_len = SCENE_NAME_STRLEN;
        unsigned int str_len = strlen(custScene);
        if (str_len > rec_str_len) {
            str_len = rec_str_len;
        }
        strncpy(tmpSceneName, custScene, str_len);
    }
    ALOGV("%s Type [%d] idx [%d], mValid [%d], tmpSceneName [%s], mCustScene [%s]", __FUNCTION__, FilterType, idx, mValid[idx], tmpSceneName, mCustScene[idx]);
    Mutex::Autolock _l(mLock[idx]);
    status_t ret = AudioToolKit_GetDecodedData(keys, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT), &mTemp);
    if (ret == NO_ERROR) {
        memcpy((void *)&mAudioParam[idx], (void *)&mTemp, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        mValid[idx] = true;
        strncpy(mCustScene[idx], tmpSceneName, SCENE_NAME_STRLEN);
        ALOGD("%s Update Cache Type [%d], Scene [%s]", __FUNCTION__ , FilterType, tmpSceneName);
    } else {
        ALOGE("%s ret [%d] Fail: Cache Type [%d], Scene [%s]", __FUNCTION__, ret, FilterType, tmpSceneName);
        return ACE_INVALIDE_PARAMETER;
    }

    return ACE_SUCCESS;
#else
    (void) FilterType;
    (void) keys;
    (void) custScene;
    ALOGE("%s Vendor Process doesn't support the function", __FUNCTION__);
    return ACE_INVALIDE_OPERATION;
#endif
}

bool MtkAudioCustParamCache::isCacheValid(uint32_t FilterType, const char *custScene)
{
    if (FilterType >= AUDIO_COMP_FLT_NUM) {
        ALOGE("%s FilterType %d >= AUDIO_COMP_FLT_NUM %d", __FUNCTION__, FilterType, AUDIO_COMP_FLT_NUM);
        return false;
    } else if (ParamCacheIndexMap[FilterType] == PARAM_CACHE_UNSUPPORT_IDX) {
        ALOGE("%s We don't support the cache for FilterType %d", __FUNCTION__, FilterType);
        return false;
    }
    int idx = ParamCacheIndexMap[FilterType];
    char tmpSceneName[SCENE_NAME_SIZE];
    memset(tmpSceneName, 0, SCENE_NAME_SIZE * sizeof(char)); // must clear mem
    if (custScene == NULL) {
        // do nothing
    } else {
        unsigned int rec_str_len = SCENE_NAME_STRLEN;
        unsigned int str_len = strlen(custScene);
        if (str_len > rec_str_len) {
            str_len = rec_str_len;
        }
        strncpy(tmpSceneName, custScene, str_len);
    }
    ALOGV("%s Type [%d] idx [%d], mValid [%d], tmpSceneName [%s], mCustScene [%s]", __FUNCTION__, FilterType, idx, mValid[idx], tmpSceneName, mCustScene[idx]);
    Mutex::Autolock _l(mLock[idx]);
    if (mValid[idx] && !strncmp(tmpSceneName, mCustScene[idx], SCENE_NAME_STRLEN)) {
        ALOGD("%s, Support the cache for FilterType [%d], custScene [%s]", __FUNCTION__, FilterType, tmpSceneName);
        return true;
    } else {
        ALOGD("%s, Unsupport the cache for FilterType [%d], custScene [%s] RecordValid [%d] RecordScene [%s]", __FUNCTION__, FilterType, tmpSceneName, mValid[idx], mCustScene[idx]);
        return false;
    }
}

bool MtkAudioLoud::mAudioCompFltXmlRegCallback = false;

MtkAudioLoud::MtkAudioLoud() :
    mPcmFormat(0), mFilterType(0), mWorkMode(0), mTempBufSize(0), mInternalBufSize(0),
    mTempBufSize_cache(0), mInternalBufSize_cache(0), mpTempBuf(NULL), mpInternalBuf(NULL),
    mIsSepLR_Filter(false), mIsZeroCoeff(false), mNoiseFilter(0), mBlockSize(BLOCK_SIZE), isFastTrack(false) {
    init();
}

MtkAudioLoud::MtkAudioLoud(uint32_t eFLTtype, bool bFastTrack) :
    mPcmFormat(0), mFilterType(eFLTtype), mWorkMode(0), mTempBufSize(0), mInternalBufSize(0),
    mTempBufSize_cache(0), mInternalBufSize_cache(0), mpTempBuf(NULL), mpInternalBuf(NULL),
    mIsSepLR_Filter(false), mIsZeroCoeff(false), mNoiseFilter(0), mBlockSize(BLOCK_SIZE), isFastTrack(bFastTrack) {
    init();
    setParameter(BLOUD_PAR_SET_FILTER_TYPE, (void *)((long)eFLTtype));
    if (mFilterType == AUDIO_COMP_FLT_DRC_FOR_MUSIC) {
        if (!isFastTrack) {
            Mutex::Autolock _l(mMusicCountMutex);
            mMusicCount ++;
        } else {
            Mutex::Autolock _l(mFastMusicCountMutex);
            mFastMusicCount ++;
        }
        //ALOGD("mMusicCount ++ = %d", mMusicCount );
    }
}
void MtkAudioLoud::init() {
    memset(&mBloudHandle, 0, sizeof(BS_HD_Handle));

    memset(&mInitParam, 0, sizeof(BLOUD_HD_InitParam));

    memset(&mAudioParam, 0, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));

    memset(&mV4ToV5Use, 0, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT_FILTER_PARAM));

    memset(&mParamFormatUse, 0, sizeof(BLOUD_HD_IIR_Design_Param));

    mInitParam.pMode_Param = new BLOUD_HD_ModeParam;
    memset(mInitParam.pMode_Param, 0, sizeof(BLOUD_HD_ModeParam));

    mInitParam.pMode_Param->pFilter_Coef_L = new BLOUD_HD_FilterCoef;
    memset(mInitParam.pMode_Param->pFilter_Coef_L, 0, sizeof(BLOUD_HD_FilterCoef));

    mInitParam.pMode_Param->pFilter_Coef_R = new BLOUD_HD_FilterCoef;
    memset(mInitParam.pMode_Param->pFilter_Coef_R, 0, sizeof(BLOUD_HD_FilterCoef));

    mInitParam.pMode_Param->pCustom_Param = new BLOUD_HD_CustomParam;
    memset(mInitParam.pMode_Param->pCustom_Param, 0, sizeof(BLOUD_HD_CustomParam));

    mState = ACE_STATE_INIT;

    mInitParam.Frame_Length = mBlockSize;

    initParser();

    memset(mCustScene, 0, sizeof(mCustScene));

    ALOGV("MtkAudioLoud Constructor\n");
}

MtkAudioLoud::~MtkAudioLoud() {
    ALOGV("+%s()\n", __FUNCTION__);
    if (mFilterType == AUDIO_COMP_FLT_DRC_FOR_MUSIC) {
        if (!isFastTrack) {
            Mutex::Autolock _l(mMusicCountMutex);
            mMusicCount --;
        } else {
            Mutex::Autolock _l(mFastMusicCountMutex);
            mFastMusicCount --;
        }
        //ALOGD("mMusicCount -- = %d", mMusicCount );
    }
    if (mInitParam.pMode_Param != NULL) {
        if (mInitParam.pMode_Param->pFilter_Coef_L != NULL) {
            delete mInitParam.pMode_Param->pFilter_Coef_L;
            mInitParam.pMode_Param->pFilter_Coef_L = NULL;
        }
        if (mInitParam.pMode_Param->pFilter_Coef_R != NULL) {
            delete mInitParam.pMode_Param->pFilter_Coef_R;
            mInitParam.pMode_Param->pFilter_Coef_R = NULL;
        }
        if (mInitParam.pMode_Param->pCustom_Param != NULL) {
            delete mInitParam.pMode_Param->pCustom_Param;
            mInitParam.pMode_Param->pCustom_Param = NULL;
        }
        delete mInitParam.pMode_Param;
        mInitParam.pMode_Param = NULL;
    }

    if (mpTempBuf != NULL) {
        delete[] mpTempBuf;
        mpTempBuf = NULL;
    }
    if (mpInternalBuf != NULL) {
        delete[] mpInternalBuf;
        mpInternalBuf = NULL;
    }
    ALOGV("-%s()\n", __FUNCTION__);
}

ACE_ERRID MtkAudioLoud::setParameter(uint32_t paramID, void *param) {
    ALOGV("+%s(), paramID %d, param 0x%lx\n", __FUNCTION__, paramID, (long)param);
    Mutex::Autolock _l(mLock);
    uint32_t Curparam = (uint32_t)((long)param);
    //Add constraint to limit the use after open.
    switch (paramID) {
    case BLOUD_PAR_SET_FILTER_TYPE: {
        mFilterType = Curparam;
        break;
    }
    case BLOUD_PAR_SET_WORK_MODE: {
        mWorkMode = Curparam;
        // mBlockSize = BLOCK_SIZE;
        switch (mWorkMode) {
        case AUDIO_CMP_FLT_LOUDNESS_BASIC:     // basic Loudness mode
            mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_LOUD_FLT;
            mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_BASIC;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_ENHANCED:     // enhancement(1) Loudness mode
            mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_LOUD_FLT;
            mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_ENHANCED;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_AGGRESSIVE:     // enhancement(2) Loudness mode
            mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_LOUD_FLT;
            mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_AGGRESSIVE;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_LITE:     // Only DRC, no filtering
            mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_NONE;
            mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_BASIC;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP:     // Audio Compensation Filter mode (No DRC)
            mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_COMP_FLT;
            mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_NONE;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP_BASIC:     // Audio Compensation Filter mode + DRC
            mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_COMP_FLT;
            mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_BASIC;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP_HEADPHONE:     //HCF
            mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_COMP_HDP;
            mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_NONE;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP_AUDENH:
            mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_AUD_ENH;
            mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_NONE;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP_LOW_LATENCY_WITHOUT_LIMITER:
            mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_LOW_LATENCY_ACF;
            mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_NONE;
            mBlockSize = BLOCK_SIZE_LOW_LATENCY;
            mInitParam.Frame_Length = mBlockSize;
            mInitParam.pMode_Param->pCustom_Param->Disable_Limiter = 1;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP_LOW_LATENCY_WITH_LIMITER:
            mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_LOW_LATENCY_ACF;
            mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_NONE;
            mBlockSize = BLOCK_SIZE_LOW_LATENCY;
            mInitParam.Frame_Length = mBlockSize;
            mInitParam.pMode_Param->pCustom_Param->Disable_Limiter = 0;
            break;
        default:
            ALOGW("%s() invalide workmode %d\n", __FUNCTION__, mWorkMode);
            break;
        }
        break;
    }
    case BLOUD_PAR_SET_SAMPLE_RATE: {
        mInitParam.Sampling_Rate = Curparam;
        break;
    }
    case BLOUD_PAR_SET_PCM_FORMAT: {
        mPcmFormat = Curparam;
        mInitParam.PCM_Format = mPcmFormat;
        break;
    }
    case BLOUD_PAR_SET_CHANNEL_NUMBER: {
        uint32_t chNum = Curparam;
        if (chNum > 0 && chNum < 3) {
            // chnum should be 1 or 2
            mInitParam.Channel = chNum;
        } else {
            return ACE_INVALIDE_PARAMETER;
        }
        break;
    }

    case BLOUD_PAR_SET_USE_DEFAULT_PARAM: {
#if defined(AUDIO_ACF_PARAM_USE_CACHE)
        MtkAudioCustParamCache::getInstance()->loadParameter(mFilterType, &mAudioParam);
#else
        if (mFilterType == AUDIO_COMP_FLT_AUDENH) {
            getDefaultAudioCompFltParam((AudioCompFltType_t)mFilterType, &mAudioParam);
        } else {
            getAudioCompFltCustParamFromStorage((AudioCompFltType_t)mFilterType, &mAudioParam);
        }
#endif
        copyParam();
        break;
    }
    case BLOUD_PAR_SET_PREVIEW_PARAM: {
#if defined(AUDIO_ACF_PARAM_USE_CACHE)
        MtkAudioCustParamCache::getInstance()->saveParameter(mFilterType, (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)param);
#endif
        memcpy((void *)&mAudioParam, (void *)param, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
        copyParam();
        break;
    }
    case BLOUD_PAR_SET_USE_DEFAULT_PARAM_FORCE_RELOAD: {
        if (mFilterType == AUDIO_COMP_FLT_AUDENH) {
            getDefaultAudioCompFltParam((AudioCompFltType_t)mFilterType, &mAudioParam);
        } else {
            getAudioCompFltCustParamFromStorage((AudioCompFltType_t)mFilterType, &mAudioParam, mCustScene);
        }
#if defined(AUDIO_ACF_PARAM_USE_CACHE)
        MtkAudioCustParamCache::getInstance()->saveParameter(mFilterType, &mAudioParam);
#endif
        copyParam();
        break;
    }
    case BLOUD_PAR_SET_USE_DEFAULT_PARAM_SUB: {
        break;
    }
    case BLOUD_PAR_SET_PREVIEW_PARAM_SUB: {
        break;
    }
    case BLOUD_PAR_SET_SEP_LR_FILTER: {
        break;
    }
    case BLOUD_PAR_SET_STEREO_TO_MONO_MODE: {
        mInitParam.pMode_Param->S2M_Mode = Curparam;
        if (mState == ACE_STATE_OPEN) {
            BLOUD_HD_RuntimeParam runtime_param;
            runtime_param.Command = BLOUD_HD_CHANGE_MODE;
            runtime_param.pMode_Param = mInitParam.pMode_Param;
            mBloudHandle.SetParameters(&mBloudHandle, &runtime_param);
        } else {
            return ACE_INVALIDE_OPERATION;
        }
        break;
    }
    case BLOUD_PAR_SET_UPDATE_PARAM_TO_SWIP: {
        if (mState != ACE_STATE_OPEN) {
            return ACE_INVALIDE_OPERATION;
        }
        BLOUD_HD_RuntimeParam runtime_param;
        runtime_param.Command = BLOUD_HD_CHANGE_MODE;
        runtime_param.pMode_Param = (BLOUD_HD_ModeParam *) mInitParam.pMode_Param;
        mBloudHandle.SetParameters(&mBloudHandle, &runtime_param);
        break;
    }
    case BLOUD_PAR_SET_RAMP_UP: {
        if (Curparam == 0) {
            mInitParam.Initial_State = BLOUD_HD_NORMAL_STATE;
        } else {
            mInitParam.Initial_State = BLOUD_HD_BYPASS_STATE;
        }

        ALOGD("%s %d mInitParam.Initial_State %d", __FUNCTION__, __LINE__, mInitParam.Initial_State);
        break;
    }
    case BLOUD_PAR_SET_NOISE_FILTER: {
        if (Curparam == 1) {
            mNoiseFilter = DRC_NOISEFILTER_MIN;
        } else {
            mNoiseFilter = 0;
        }
        break;
    }
    case BLOUD_PAR_SET_LOAD_CACHE_PARAM: {
#if defined(AUDIO_ACF_PARAM_USE_CACHE)
        ACE_ERRID ret;
        ret = MtkAudioCustParamCache::getInstance()->loadParameter(mFilterType, &mAudioParam, mCustScene);
        if (ret != ACE_SUCCESS) {
            ALOGE("-%s(), Fail paramID %d, param 0x%lx ret = %d\n", __FUNCTION__, paramID, (long)param, ret);
            return ret;
        }
#else
        return ACE_INVALIDE_OPERATION;
#endif
        copyParam();
        break;
    }
    case BLOUD_PAR_SET_FRAME_LENGTH: {
        ALOGD("mBlockSize %d, mInitParam.Frame_Length %d", mBlockSize, mInitParam.Frame_Length);
        if ((Curparam == BLOCK_SIZE || Curparam == BLOCK_SIZE_FOR_LOW_LATENCY_DRC) && mBlockSize != BLOCK_SIZE_LOW_LATENCY) {
            mBlockSize = Curparam;
            mInitParam.Frame_Length = Curparam;
            ALOGD("-%s(), paramID %d, param %d\n", __FUNCTION__, paramID, Curparam);
        } else {
            ALOGE("-%s(), Fail paramID %d, param %d\n", __FUNCTION__, paramID, Curparam);
        }
        break;
    }
    default:
        ALOGD("-%s() Error\n", __FUNCTION__);
        return ACE_INVALIDE_PARAMETER;
    }
    ALOGV("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::getParameter(uint32_t paramID, void *param) {
    ALOGV("+%s(), paramID %d, param %p\n", __FUNCTION__, paramID, param);
    Mutex::Autolock _l(mLock);
    ALOGV("-%s(), paramID %d, param %p\n", __FUNCTION__, paramID, param);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::open(void) {
    int32_t result;

    ALOGD("+%s()\n", __FUNCTION__);
    Mutex::Autolock _l(mLock);
    if (mState != ACE_STATE_INIT) {
        return ACE_INVALIDE_OPERATION;
    }
#if defined(HAVE_SWIP)
    BLOUD_HD_SetHandle(&mBloudHandle);
#endif
    mBloudHandle.GetBufferSize(&mInternalBufSize, &mTempBufSize, mPcmFormat);

    ALOGV("AudLdSz %d/%d %d/%d fmt %d", mInternalBufSize, mInternalBufSize_cache, mTempBufSize, mTempBufSize_cache, mPcmFormat);

    if (mInternalBufSize > mInternalBufSize_cache) {
        if (mpInternalBuf != NULL) {
            ALOGD("AudLdSz Del mpInternalBuf");
            delete[] mpInternalBuf;
        }

        mpInternalBuf = new char[mInternalBufSize];
        mInternalBufSize_cache = mInternalBufSize;
        ALOGV("AudLdSz New mpInternalBuf Pass");
    }

    if (mTempBufSize > mTempBufSize_cache) {
        if (mpTempBuf != NULL) {
            ALOGD("AudLdSz Del mpTempBuf");
            delete[] mpTempBuf;
        }

        mpTempBuf = new char[mTempBufSize];
        mTempBufSize_cache = mTempBufSize;
        ALOGV("AudLdSz New mpTempBuf Pass");
    }

    //ALOGD("Filter_Mode [%d] Loudness_Mode [%d] Pte [%x]",mInitParam.pMode_Param->Filter_Mode,mInitParam.pMode_Param->Loudness_Mode,(int)mInitParam.pMode_Param->pFilter_Coef_L);

    result = mBloudHandle.Open(&mBloudHandle, mpInternalBuf, (const void *)&mInitParam);
    mState = ACE_STATE_OPEN;
    ALOGD("-%s() result %d\n", __FUNCTION__, result);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::close(void) {
    ALOGD("+%s()\n", __FUNCTION__);
    Mutex::Autolock _l(mLock);
    if (mState != ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }
#if 0
    if (mpTempBuf != NULL) {
        delete[] mpTempBuf;
        mpTempBuf = NULL;
    }
    if (mpInternalBuf != NULL) {
        delete[] mpInternalBuf;
        mpInternalBuf = NULL;
    }
#endif
    mState = ACE_STATE_INIT;
    ALOGD("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::resetBuffer(void) {
    ALOGV("+%s()\n", __FUNCTION__);
    Mutex::Autolock _l(mLock);
    if (mState != ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }
    BLOUD_HD_RuntimeParam runtime_param;
    runtime_param.Command = BLOUD_HD_RESET;
    mBloudHandle.SetParameters(&mBloudHandle, &runtime_param);
    ALOGV("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::setWorkMode(uint32_t chNum, uint32_t smpRate, uint32_t workMode, bool bRampUpEnable) {
    ACE_ERRID ret;
    ALOGV("+%s()\n", __FUNCTION__);
    if (mState != ACE_STATE_OPEN) {
        ALOGD("%s(), chNum %d, sampleRate %d, workMode %d RampupEnable %d\n", __FUNCTION__, chNum, smpRate, workMode, bRampUpEnable);

        if ((ret = setParameter(BLOUD_PAR_SET_CHANNEL_NUMBER, (void *)((long)chNum))) != ACE_SUCCESS) {
            return ret;
        }
        if ((ret = setParameter(BLOUD_PAR_SET_SAMPLE_RATE, (void *)((long)smpRate))) != ACE_SUCCESS) {
            return ret;
        }
        if ((ret = setParameter(BLOUD_PAR_SET_WORK_MODE, (void *)((long)workMode))) != ACE_SUCCESS) {
            return ret;
        }
        if ((ret = setParameter(BLOUD_PAR_SET_RAMP_UP, (void *)bRampUpEnable)) != ACE_SUCCESS) {
            return ret;
        }
    }
    ALOGV("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

int MtkAudioLoud::getBesLoudSamplingRateIndex(unsigned int sampling_rate) {
    int sr_idx;

    switch (sampling_rate) {
    case 48000:
        sr_idx = 0;
        break;
    case 44100:
        sr_idx = 1;
        break;
    case 32000:
        sr_idx = 2;
        break;
    case 24000:
        sr_idx = 3;
        break;
    case 22050:
        sr_idx = 4;
        break;
    case 16000:
        sr_idx = 5;
        break;
    case 12000:
        sr_idx = 6;
        break;
    case 11025:
        sr_idx = 7;
        break;
    case  8000:
        sr_idx = 8;
        break;
    default:
        sr_idx = -1;
        break;
    }

    return sr_idx;
}

int MtkAudioLoud::convertBesLoudCoeffV4ToV5(unsigned int sampling_rate, AUDIO_ACF_CUSTOM_PARAM_STRUCT_FILTER_PARAM *p_V4, BLOUD_HD_FilterCoef *p_V5) {
    int result = 0;
    int sr_idx = getBesLoudSamplingRateIndex(sampling_rate);


    memset(p_V5->HPF_COEF, 0, 2 * 5 * sizeof(unsigned int));
    memset(p_V5->LPF_COEF, 0,     3 * sizeof(unsigned int));
    memset(p_V5->BPF_COEF, 0, 8 * 6 * sizeof(unsigned int));

    if (p_V4 == NULL || p_V5 == NULL) {
        result = -1;
    } else if (sr_idx < 0 || sr_idx > 8) {
        result = -2;
    } else {
        int flt_idx;

        for (flt_idx = 0; flt_idx < 2; flt_idx++) {
            memcpy(p_V5->HPF_COEF[flt_idx], p_V4->bes_loudness_hsf_coeff[flt_idx][sr_idx], sizeof(unsigned int) * 5);
        }

        if (sr_idx < 6) {
            memcpy(p_V5->LPF_COEF, p_V4->bes_loudness_lpf_coeff[sr_idx], sizeof(unsigned int) * 3);

            for (flt_idx = 0; flt_idx < 8; flt_idx++) {
                //memcpy(p_V5->BPF_COEF, p_V4->bes_loudness_bpf_coeff[sr_idx], sizeof(unsigned int) * 3);
                memset(p_V5->BPF_COEF[flt_idx], 0, sizeof(unsigned int) * 6);
                memcpy(p_V5->BPF_COEF[flt_idx], p_V4->bes_loudness_bpf_coeff[flt_idx][sr_idx], sizeof(unsigned int) * 3);
            }
        }
    }

    return result;

}

void MtkAudioLoud::useDefaultFullband(BLOUD_HD_InitParam *pInitParam) {
    if (pInitParam) {
        unsigned int Att_Time_Default[6] = {64, 64, 64, 64, 64, 64};
        unsigned int Rel_Time_Default[6] = {6400, 6400, 6400, 6400, 6400, 6400};
        int          Hyst_Th_Default[6]  = {256, 256, 256, 256, 256, 256};
        int          DRC_Th_Default[5]   = { -15360, -12800, -10240, -7680, 0,};
        int          DRC_Gn_Default[5]   = {6144, 6144, 6144, 6144, 0};

        pInitParam->pMode_Param->pCustom_Param->Num_Bands = 1;
        pInitParam->pMode_Param->pCustom_Param->SB_Mode[0] = 0;
        pInitParam->pMode_Param->pCustom_Param->SB_Gn[0] = 0;
        pInitParam->pMode_Param->pCustom_Param->Lim_Th = 0x7FFF;
        pInitParam->pMode_Param->pCustom_Param->Lim_Gn = 0x7FFF;
        pInitParam->pMode_Param->pCustom_Param->Lim_Const = 4;
        pInitParam->pMode_Param->pCustom_Param->Sep_LR_Filter = 0;

        memcpy((void *)pInitParam->pMode_Param->pCustom_Param->Att_Time, (void *)Att_Time_Default, 6 * sizeof(unsigned int));
        memcpy((void *)pInitParam->pMode_Param->pCustom_Param->Rel_Time, (void *)Rel_Time_Default, 6 * sizeof(unsigned int));
        memcpy((void *)pInitParam->pMode_Param->pCustom_Param->Hyst_Th, (void *)Hyst_Th_Default,  6 * sizeof(int));
        memcpy((void *)pInitParam->pMode_Param->pCustom_Param->DRC_Th, (void *)DRC_Th_Default,   5 * sizeof(int));
        memcpy((void *)pInitParam->pMode_Param->pCustom_Param->DRC_Gn, (void *)DRC_Gn_Default,   5 * sizeof(int));
    }
}

void MtkAudioLoud::useNoiseFilter(BLOUD_HD_InitParam *pInitParam) {
    if (pInitParam) {
        int NoiseGap1 = NOISE_FILTER_BASE * mNoiseFilter;
        int NoiseGap2 = NOISE_FILTER_BASE * (mNoiseFilter - NOISE_FILTER_STEP);

        int DRC_Th_Default[2]   = {NoiseGap1, NoiseGap2};
        int DRC_Gn_Default[1]   = {0};

        for (unsigned int i = 0; i < pInitParam->pMode_Param->pCustom_Param->Num_Bands; i++) {
            if (pInitParam->pMode_Param->pCustom_Param->DRC_Th[i][0] > DRC_Th_Default[0]) {
                pInitParam->pMode_Param->pCustom_Param->DRC_Th[i][0] = DRC_Th_Default[0];
            }
            if (pInitParam->pMode_Param->pCustom_Param->DRC_Th[i][1] > DRC_Th_Default[1]) {
                pInitParam->pMode_Param->pCustom_Param->DRC_Th[i][1] = DRC_Th_Default[1];
            }

            pInitParam->pMode_Param->pCustom_Param->DRC_Gn[i][0] = DRC_Gn_Default[0];
        }
    }
}

void MtkAudioLoud::copyParam(void) {
    bool ZeroFlag = true;
    int dCheckStatus;
    mInitParam.pMode_Param->pCustom_Param->WS_Gain_Max = mAudioParam.bes_loudness_WS_Gain_Max;
    mInitParam.pMode_Param->pCustom_Param->WS_Gain_Min = mAudioParam.bes_loudness_WS_Gain_Min;
    mInitParam.pMode_Param->pCustom_Param->Filter_First = mAudioParam.bes_loudness_Filter_First;
    mInitParam.pMode_Param->pCustom_Param->Num_Bands = mAudioParam.bes_loudness_Num_Bands;
    mInitParam.pMode_Param->pCustom_Param->Flt_Bank_Order = mAudioParam.bes_loudness_Flt_Bank_Order;
    mInitParam.pMode_Param->pCustom_Param->DRC_Delay = mAudioParam.DRC_Delay;
    mInitParam.pMode_Param->pCustom_Param->Lim_Th = mAudioParam.Lim_Th;
    mInitParam.pMode_Param->pCustom_Param->Lim_Gn = mAudioParam.Lim_Gn;
    mInitParam.pMode_Param->pCustom_Param->Lim_Const = mAudioParam.Lim_Const;
    mInitParam.pMode_Param->pCustom_Param->Lim_Delay = mAudioParam.Lim_Delay;
    mInitParam.pMode_Param->pCustom_Param->Sep_LR_Filter = mIsSepLR_Filter = mAudioParam.bes_loudness_Sep_LR_Filter;
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->Att_Time, (void *)mAudioParam.Att_Time, 48 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->Rel_Time, (void *)mAudioParam.Rel_Time, 48 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->Cross_Freq, (void *)mAudioParam.bes_loudness_Cross_Freq, 7 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->SB_Mode, (void *)mAudioParam.SB_Mode, 8 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->SB_Gn, (void *)mAudioParam.SB_Gn, 8 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->Hyst_Th, (void *)mAudioParam.Hyst_Th, 48 * sizeof(int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->DRC_Th, (void *)mAudioParam.DRC_Th, 40 * sizeof(int));
    memcpy((void *)mInitParam.pMode_Param->pCustom_Param->DRC_Gn, (void *)mAudioParam.DRC_Gn, 40 * sizeof(int));

    if (mFilterType == AUDIO_COMP_FLT_DRC_FOR_MUSIC) {
        if (mNoiseFilter != 0) {
            // Only VOIP will use noise filter.
            // We suppose the track number will not exceed MUSIC_UP_BOUND.
            useNoiseFilter(&mInitParam);
        } else {
            if (!isFastTrack) {
                Mutex::Autolock _l(mMusicCountMutex);
                if (mMusicCount > MUSIC_UP_BOUND) {
                    mMaxMusicCount = MIN_MULTI_BAND_COUNT;
                } else if (mMusicCount < MUSIC_LOW_BOUND) {
                    mMaxMusicCount = MAX_MULTI_BAND_COUNT;
                }

                if (mMusicCount > mMaxMusicCount) {
                    //ALOGD("mMusicCount = %d", mMusicCount );
                    useDefaultFullband(&mInitParam);
                }
            } else {
                Mutex::Autolock _l(mFastMusicCountMutex);
                if (mFastMusicCount > mMaxFastMusicCount) {
                    //ALOGD("mMusicCount = %d", mMusicCount );
                    useDefaultFullband(&mInitParam);
                }
            }
        }
    }

    if (mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_hpf_fc != 0
        || mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_hpf_order != 0
        || mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_lpf_fc != 0
        || mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_lpf_order != 0
        || mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_hpf_fc != 0
        || mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_hpf_order != 0
        || mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_lpf_fc != 0
        || mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_lpf_order != 0
       ) {
        ZeroFlag = false;
    }

    if (ZeroFlag) {
        for (int i = 0; i < 8; i++) {
            if (mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_bpf_fc[i] != 0
                || mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_bpf_bw[i] != 0
                || mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_bpf_gain[i] != 0
                || mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_bpf_fc[i] != 0
                || mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_bpf_bw[i] != 0
                || mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_bpf_gain[i] != 0
               ) {
                ZeroFlag = false;
                break;
            }
        }
    }

    mIsZeroCoeff = ZeroFlag;

    if (mFilterType == AUDIO_COMP_FLT_VIBSPK) {
        memset((void *)&mV4ToV5Use, 0x00, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT_FILTER_PARAM));
        memcpy((void *)mV4ToV5Use.bes_loudness_bpf_coeff, (void *)mAudioParam.bes_loudness_f_param.V5ViVSPK.bes_loudness_bpf_coeff, 36 * sizeof(unsigned int));
        dCheckStatus = convertBesLoudCoeffV4ToV5(mInitParam.Sampling_Rate, &mV4ToV5Use, mInitParam.pMode_Param->pFilter_Coef_L);
        if (dCheckStatus) {
            ALOGE("%s %d Return pFilter_Coef_L %d", __FUNCTION__, __LINE__, dCheckStatus);
        }
        dCheckStatus = convertBesLoudCoeffV4ToV5(mInitParam.Sampling_Rate, &mV4ToV5Use, mInitParam.pMode_Param->pFilter_Coef_R);
        if (dCheckStatus) {
            ALOGE("%s %d Return pFilter_Coef_R %d", __FUNCTION__, __LINE__, dCheckStatus);
        }
    } else {
        mParamFormatUse.hpf_fc = mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_hpf_fc;
        mParamFormatUse.hpf_order = mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_hpf_order;
        mParamFormatUse.lpf_fc = mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_lpf_fc;
        mParamFormatUse.lpf_order = mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_lpf_order;
        memcpy((void *)mParamFormatUse.bpf_fc, (void *)mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_bpf_fc, 8 * sizeof(unsigned int));
        memcpy((void *)mParamFormatUse.bpf_bw, (void *)mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_bpf_bw, 8 * sizeof(unsigned int));
        memcpy((void *)mParamFormatUse.bpf_gain, (void *)mAudioParam.bes_loudness_f_param.V5F.bes_loudness_L_bpf_gain, 8 * sizeof(int));

        dCheckStatus = BLOUD_HD_Filter_Design(mInitParam.pMode_Param->Filter_Mode, mInitParam.Sampling_Rate, &mParamFormatUse, mInitParam.pMode_Param->pFilter_Coef_L);

        if (dCheckStatus) {
            ALOGE("%s %d Return pFilter_Coef_L %d", __FUNCTION__, __LINE__, dCheckStatus);
        }

        mParamFormatUse.hpf_fc = mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_hpf_fc;
        mParamFormatUse.hpf_order = mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_hpf_order;
        mParamFormatUse.lpf_fc = mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_lpf_fc;
        mParamFormatUse.lpf_order = mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_lpf_order;
        memcpy((void *)mParamFormatUse.bpf_fc, (void *)mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_bpf_fc, 8 * sizeof(unsigned int));
        memcpy((void *)mParamFormatUse.bpf_bw, (void *)mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_bpf_bw, 8 * sizeof(unsigned int));
        memcpy((void *)mParamFormatUse.bpf_gain, (void *)mAudioParam.bes_loudness_f_param.V5F.bes_loudness_R_bpf_gain, 8 * sizeof(int));

        dCheckStatus = BLOUD_HD_Filter_Design(mInitParam.pMode_Param->Filter_Mode, mInitParam.Sampling_Rate, &mParamFormatUse, mInitParam.pMode_Param->pFilter_Coef_R);

        if (dCheckStatus) {
            ALOGE("%s %d Return pFilter_Coef_R %d", __FUNCTION__, __LINE__, dCheckStatus);
        }
    }
    //memcpy((void *)mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF, (void *)mAudioParam.bes_loudness_hsf_coeff_L, 10 * sizeof(unsigned int));
    //memcpy((void *)mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF, (void *)mAudioParam.bes_loudness_bpf_coeff_L, 24 * sizeof(unsigned int));
    //memcpy((void *)mInitParam.pMode_Param->pFilter_Coef_L->LPF_COEF, (void *)mAudioParam.bes_loudness_lpf_coeff_L, 3 * sizeof(unsigned int));

    //memcpy((void *)mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF, (void *)mAudioParam.bes_loudness_hsf_coeff_R, 10 * sizeof(unsigned int));
    //memcpy((void *)mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF, (void *)mAudioParam.bes_loudness_bpf_coeff_R, 24 * sizeof(unsigned int));
    //memcpy((void *)mInitParam.pMode_Param->pFilter_Coef_R->LPF_COEF, (void *)mAudioParam.bes_loudness_lpf_coeff_R, 3 * sizeof(unsigned int));
    if (mIsSepLR_Filter == 0) {
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF, 0, 10 * sizeof(unsigned int));
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF, 0, 48 * sizeof(unsigned int));
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_R->LPF_COEF, 0, 3 * sizeof(unsigned int));
    }

    if (mInitParam.pMode_Param->pCustom_Param->Disable_Limiter) {
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF, 0, 48 * sizeof(unsigned int));
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_L->LPF_COEF, 0, 3 * sizeof(unsigned int));
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF, 0, 48 * sizeof(unsigned int));
        memset((void *)mInitParam.pMode_Param->pFilter_Coef_R->LPF_COEF, 0, 3 * sizeof(unsigned int));
    }
    /*
        bool ZeroFlag = true;

        for(int i=0;i<2;i++)
            for(int j=0;j<5;j++)
            {
                if (mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF[i][j])
                {
                    ZeroFlag = false;
                    break;
                }
            }

        if (ZeroFlag)
        {
            for(int i=0;i<8;i++)
            for(int j=0;j<6;j++)
            {
                if (mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[i][j])
                {
                    ZeroFlag = false;
                    break;
                }
            }
        }

        if (ZeroFlag)
        {
            for(int i=0;i<8;i++)
            {
                if (mInitParam.pMode_Param->pFilter_Coef_L->LPF_COEF[i])
                {
                    ZeroFlag = false;
                    break;
                }
            }
        }

        if (mIsSepLR_Filter)
        {
            for(int i=0;i<2;i++)
            for(int j=0;j<5;j++)
            {
                if (mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF[i][j])
                {
                    ZeroFlag = false;
                    break;
                }
            }

            if (ZeroFlag)
            {
                for(int i=0;i<8;i++)
                for(int j=0;j<6;j++)
                {
                    if (mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[i][j])
                    {
                        ZeroFlag = false;
                        break;
                    }
                }
            }

            if (ZeroFlag)
            {
                for(int i=0;i<8;i++)
                {
                    if (mInitParam.pMode_Param->pFilter_Coef_R->LPF_COEF[i])
                    {
                        ZeroFlag = false;
                        break;
                    }
                }
            }
        }

        mIsZeroCoeff = ZeroFlag;
        */
    ALOGD("mIsZeroCoeff %d mFilterType %d", mIsZeroCoeff, mFilterType);
    ALOGV("Channel %d", mInitParam.Channel);
    ALOGV("Sampling_Rate %d", mInitParam.Sampling_Rate);
    ALOGV("PCM_Format %d", mInitParam.PCM_Format);

    ALOGV("copyParam mIsSepLR_Filter [%d]", mIsSepLR_Filter);
    ALOGV("LHSF_Coeff [0][0]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF[0][0], &mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF[0][0]);
    ALOGV("LHSF_Coeff [0][1]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF[0][1], &mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF[0][1]);
    ALOGV("LBPF_Coeff [0][0]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[0][0], &mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[0][0]);
    ALOGV("LBPF_Coeff [0][1]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[0][1], &mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[0][1]);
    ALOGV("RHSF_Coeff [0][0]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF[0][0], &mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF[0][0]);
    ALOGV("RHSF_Coeff [0][1]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF[0][1], &mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF[0][1]);
    ALOGV("RBPF_Coeff [0][0]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[0][0], &mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[0][0]);
    ALOGV("RBPF_Coeff [0][1]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[0][1], &mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[0][1]);

    ALOGV("WS_Gain_Max %d", mInitParam.pMode_Param->pCustom_Param->WS_Gain_Max);
    ALOGV("WS_Gain_Min %d", mInitParam.pMode_Param->pCustom_Param->WS_Gain_Min);
    ALOGV("Filter_First %d", mInitParam.pMode_Param->pCustom_Param->Filter_First);
    ALOGV("Num_Bands %d", mInitParam.pMode_Param->pCustom_Param->Num_Bands);
    ALOGV("Flt_Bank_Order %d", mInitParam.pMode_Param->pCustom_Param->Flt_Bank_Order);
    ALOGV("DRC_Delay %d", mInitParam.pMode_Param->pCustom_Param->DRC_Delay);
    ALOGV("Lim_Th %d", mInitParam.pMode_Param->pCustom_Param->Lim_Th);
    ALOGV("Lim_Gn %d", mInitParam.pMode_Param->pCustom_Param->Lim_Gn);
    ALOGV("Lim_Const %d", mInitParam.pMode_Param->pCustom_Param->Lim_Const);
    ALOGV("Lim_Delay %d", mInitParam.pMode_Param->pCustom_Param->Lim_Delay);
    ALOGV("Sep_LR_Filter %d", mInitParam.pMode_Param->pCustom_Param->Sep_LR_Filter);

    for (int i = 0; i < 6; i++) {
        ALOGV("Att_Time[0][%d] = %d", i, mInitParam.pMode_Param->pCustom_Param->Att_Time[0][i]);
        ALOGV("Rel_Time[0][%d] = %d", i, mInitParam.pMode_Param->pCustom_Param->Rel_Time[0][i]);
    }

    for (int i = 0; i < 5; i++) {
        ALOGV("DRC_Th[0][%d] = %d", i, mInitParam.pMode_Param->pCustom_Param->DRC_Th[0][i]);
        ALOGV("DRC_Gn[0][%d] = %d", i, mInitParam.pMode_Param->pCustom_Param->DRC_Gn[0][i]);
    }
}

void MtkAudioLoud::copyParamSub(void) {
}

/* Return: consumed input buffer size(byte)                             */
ACE_ERRID MtkAudioLoud::process(void *pInputBuffer,   /* Input, pointer to input buffer */
                                uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                                /* Output, length(byte) left in the input buffer after conversion */
                                void *pOutputBuffer,               /* Input, pointer to output buffer */
                                uint32_t *OutputSampleCount)       /* Input, length(byte) of output buffer */
/* Output, output data length(byte) */
{
    ALOGVV("+%s(), inputCnt %d, outputCnt %d mBlockSize %d\n", __FUNCTION__, *InputSampleCount, *OutputSampleCount, mBlockSize);
    Mutex::Autolock _l(mLock);
    uint32_t dOutputMaxBufSize = *OutputSampleCount;
    uint32_t block_size_byte, offset_bit, loop_cnt, i, TotalConsumedSample = 0, TotalOuputSample = 0, ConsumedSampleCount;
    int32_t result;
    if (mState != ACE_STATE_OPEN) {
        ALOGD("Error");
        return ACE_INVALIDE_OPERATION;
    }
    //Simplify handle (BLOCK_SIZE x N) Samples
    if (mPcmFormat == BLOUDHD_IN_Q1P15_OUT_Q1P15) { // 16 bits
        // 2-byte, mono
        if (mInitParam.Channel == 1) {
            offset_bit = 1;
        }// 2-byte, L/R
        else {
            offset_bit = 2;
        }
    } else { //32 bits
        if (mInitParam.Channel == 1) {
            // 4-byte, Mono
            offset_bit = 2;
        } else {
            // 4-byte, L/R
            offset_bit = 3;
        }
    }
    block_size_byte = mBlockSize * (1 << offset_bit);
    if (((*InputSampleCount & (block_size_byte - 1)) != 0) || ((*OutputSampleCount & (block_size_byte - 1)) != 0)/* || (*InputSampleCount != *OutputSampleCount)*/) {
        ALOGW("-%s(), inputCnt %d, outputCnt %d block_size_byte %d\n", __FUNCTION__, *InputSampleCount, *OutputSampleCount, block_size_byte);
        assert(0);
    }
    loop_cnt = *InputSampleCount / block_size_byte;
    //ALOGV("+%s(), loop_cnt %d, block_size_byte %d, sample %d %d\n",__FUNCTION__, loop_cnt, block_size_byte, *(int *)pInputBuffer, (*(int *)pInputBuffer)>>16);
    for (i = 0; i < loop_cnt; i++) {
        ConsumedSampleCount = block_size_byte;
        *OutputSampleCount = block_size_byte;
        if (dOutputMaxBufSize < TotalConsumedSample + ConsumedSampleCount) {
            ALOGW("Warning for input [%d] > output [%d], and skip process", *InputSampleCount, dOutputMaxBufSize);
            break;
        }
#ifdef ENABLE_PROCESS_PROFILING
        struct timespec systemtime_start, systemtime_end;
        unsigned long total_nano_sec = 0;
        int rc;
        rc = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &systemtime_start);
#endif
        char *pInputBufferChar = (char *) pInputBuffer;
        char *pOutputBufferChar = (char *) pOutputBuffer;
        result = mBloudHandle.Process(&mBloudHandle,
                                      mpTempBuf,
                                      (int *)(pInputBufferChar + TotalConsumedSample),
                                      (int *)&ConsumedSampleCount,
                                      (int *)(pOutputBufferChar + TotalOuputSample),
                                      (int *)OutputSampleCount);
#ifdef ENABLE_PROCESS_PROFILING
        rc = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &systemtime_end);
        total_nano_sec = systemtime_end.tv_nsec - systemtime_start.tv_nsec;
        ALOGD("FLT[%d] nano [%d] SampleCount [%d] Ch [%d] SP [%d] PCM [%d]", mFilterType, total_nano_sec, ConsumedSampleCount, mInitParam.Channel, mInitParam.Sampling_Rate, mInitParam.PCM_Format);
#endif
        ALOGVV("result [%d] ConsumedSampleCount [%d] i [%d] loop_cnt [%d]", result, ConsumedSampleCount, i, loop_cnt);
        TotalConsumedSample += ConsumedSampleCount;

        TotalOuputSample += *OutputSampleCount;
    }
    //ALOGV("+%s(), result = %d, loop_cnt %d, block_size_byte %d, sample %d %d\n",__FUNCTION__, result, loop_cnt, block_size_byte, *(int *)pOutputBuffer, (*(int *)pOutputBuffer)>>16);
    *OutputSampleCount = TotalOuputSample;
    *InputSampleCount = TotalConsumedSample;
    ALOGVV("-%s(), inputCnt %d, outputCnt %d\n", __FUNCTION__, *InputSampleCount, *OutputSampleCount);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::change2ByPass(void) {
    ALOGD("+%s()\n", __FUNCTION__);
    Mutex::Autolock _l(mLock);
    if (mState != ACE_STATE_OPEN) {
        ALOGW("-%s() Line [%d]\n", __FUNCTION__, __LINE__);
        return ACE_INVALIDE_OPERATION;
    }

    BLOUD_HD_RuntimeStatus runtime_status;

    if (mBloudHandle.GetStatus(&mBloudHandle, &runtime_status) < 0) {
        ALOGW("-%s() Line [%d]\n", __FUNCTION__, __LINE__);
        return ACE_INVALIDE_OPERATION;
    } else if (runtime_status.State == BLOUD_HD_SWITCHING_STATE) {
        ALOGW("-%s() Line [%d]\n", __FUNCTION__, __LINE__);
        return ACE_INVALIDE_OPERATION;
    }

    BLOUD_HD_RuntimeParam runtime_param;
    runtime_param.Command = BLOUD_HD_TO_BYPASS_STATE;
    mBloudHandle.SetParameters(&mBloudHandle, &runtime_param);
    ALOGD("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::change2Normal(void) {
    ALOGD("+%s()\n", __FUNCTION__);
    Mutex::Autolock _l(mLock);
    if (mState != ACE_STATE_OPEN) {
        ALOGW("-%s() Line [%d]\n", __FUNCTION__, __LINE__);
        return ACE_INVALIDE_OPERATION;
    }
    BLOUD_HD_RuntimeStatus runtime_status;

    if (mBloudHandle.GetStatus(&mBloudHandle, &runtime_status) < 0) {
        ALOGW("-%s() Line [%d]\n", __FUNCTION__, __LINE__);
        return ACE_INVALIDE_OPERATION;
    } else if (runtime_status.State == BLOUD_HD_SWITCHING_STATE) {
        ALOGW("-%s() Line [%d]\n", __FUNCTION__, __LINE__);
        return ACE_INVALIDE_OPERATION;
    }
    BLOUD_HD_RuntimeParam runtime_param;
    runtime_param.Command = BLOUD_HD_TO_NORMAL_STATE;
    mBloudHandle.SetParameters(&mBloudHandle, &runtime_param);
    ALOGD("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

bool MtkAudioLoud::isZeroCoeffFilter(void) {
    return mIsZeroCoeff;
}

ACE_ERRID MtkAudioLoud::setOutputGain(int32_t gain, uint32_t ramp_sample_cnt) {
    ALOGD("+%s() gain %d rampeSample %d\n", __FUNCTION__, gain, ramp_sample_cnt);
    Mutex::Autolock _l(mLock);
    if (mState == ACE_STATE_OPEN) {
        BLOUD_HD_RuntimeParam runtime_param;
        runtime_param.Command = BLOUD_HD_SET_OUTPUT_GAIN;
        runtime_param.pMode_Param = mInitParam.pMode_Param;
        runtime_param.pMode_Param->pFilter_Coef_L->output_gain = gain;
        runtime_param.pMode_Param->pFilter_Coef_L->ramp_smpl_cnt = ramp_sample_cnt;
        runtime_param.pMode_Param->pFilter_Coef_R->output_gain = gain;
        runtime_param.pMode_Param->pFilter_Coef_R->ramp_smpl_cnt = ramp_sample_cnt;
        mBloudHandle.SetParameters(&mBloudHandle, &runtime_param);
    }
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::setNotchFilterParam(uint32_t fc, uint32_t bw, int32_t th) {
    ALOGD("+%s() fc= %d bw= %d th= %d, state %d\n", __FUNCTION__, fc, bw, th, mState);
    Mutex::Autolock _l(mLock);
    if (mState == ACE_STATE_OPEN) {
        BLOUD_HD_RuntimeParam runtime_param;
        runtime_param.Command = BLOUD_HD_CHANGE_NOTCH;
        runtime_param.pMode_Param = mInitParam.pMode_Param;
        runtime_param.pMode_Param->pFilter_Coef_L->notch_fc = fc;
        runtime_param.pMode_Param->pFilter_Coef_L->notch_bw = bw;
        runtime_param.pMode_Param->pFilter_Coef_L->notch_th = th;
        runtime_param.pMode_Param->pFilter_Coef_R->notch_fc = fc;
        runtime_param.pMode_Param->pFilter_Coef_R->notch_bw = bw;
        runtime_param.pMode_Param->pFilter_Coef_R->notch_th = th;
        mBloudHandle.SetParameters(&mBloudHandle, &runtime_param);
    }
    return ACE_SUCCESS;
}

int MtkAudioLoud::getBesSoundVer(void) {
    const int ParameterVer = 5;

#if defined(HAVE_SWIP)
    BS_HD_Handle mTempBloudHandle;
    BS_HD_EngineInfo mEngineInfo;
    memset((void *)(&mEngineInfo), 0x00, sizeof(BS_HD_EngineInfo));
    BLOUD_HD_SetHandle(&mTempBloudHandle);
    mTempBloudHandle.GetEngineInfo(&mEngineInfo);
    ALOGD("BesSound Ver : 0x%x", mEngineInfo.Version);
    ALOGD("BesSound FlushOutSampleCount : 0x%x", mEngineInfo.FlushOutSampleCount);
    ALOGD("NvRam Format  : V%d", ParameterVer);
    return mEngineInfo.Version;
#else
    ALOGD("BesSound Ver : 0 [Unsupport]");
    ALOGD("NvRam Format  : V%d", ParameterVer);
    return 0;
#endif
}

bool MtkAudioLoud::initParser(void) {
    if (mAudioCompFltXmlRegCallback == false) {
        mAudioCompFltXmlRegCallback = true;
        audioComFltCustParamInit();
        return true;
    }
    return false;
}
ACE_ERRID MtkAudioLoud::setCustSceneName(const char* sceneName) {
    unsigned int rec_str_len = SCENE_NAME_STRLEN;
    unsigned int arr_len = SCENE_NAME_SIZE;
    if (sceneName == NULL) {
        return ACE_INVALIDE_PARAMETER;
    } else {
        unsigned int str_len = strlen(sceneName);
        if (str_len > rec_str_len) {
            str_len = rec_str_len;
        }
        memset(mCustScene, 0, arr_len);
        strncpy(mCustScene, sceneName, str_len);
    }
    ALOGD("mCustScene = %s, size = %d len = %zu", mCustScene, arr_len, strlen(mCustScene));
    return ACE_SUCCESS;
}

uint32_t MtkAudioLoud::getFilterType(void) {
    return mFilterType;
}
ACE_ERRID MtkAudioLoud::getCustSceneName(char* sceneName) {
    unsigned int str_len;
    if (sceneName == NULL) {
        return ACE_INVALIDE_PARAMETER;
    } else {
        str_len = strlen(mCustScene);
        strncpy(sceneName, mCustScene, str_len);
    }
    ALOGD("mCustScene = %s,len = %zu", mCustScene, strlen(mCustScene));
    return ACE_SUCCESS;
}

}//namespace android

