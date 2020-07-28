#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#include <utils/Log.h>
#include <utils/String8.h>

#include "audio_custom_exp.h"
#include <assert.h>
#include <cutils/properties.h>
#include "AudioSpeechEnhLayer.h"

#include "AudioUtility.h"

//#include <aee.h>
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioSPELayer"

/*
#define ASSERT(exp) \
    do { \
        if (!(exp)) { \
            ALOGE("ASSERT("#exp") fail: \""  __FILE__ "\", %uL", __LINE__); \
            aee_system_exception("[Audio]", NULL, DB_OPT_FTRACE, " %s, %uL", strrchr(__FILE__, '/') + 1, __LINE__); \
        } \
    } while(0)
*/

namespace android {

Word16 DefaultRecDMNR_cal_data[DMNRCalDataNum] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
Word16 DefaultRecCompen_filter[CompenFilterNum] = {
    32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
uWord32 DefaultRecEnhancePara[EnhanceParasNum] =
{96, 253, 16388, 32796, 49415, 0, 400, 0, 272, 4325, 99, 0, 0, 0, 0, 8192, 0, 0, 0, 10752, 32769, 0, 0, 0, 0, 0, 0, 0};

Word16 DefaultVoIPDMNR_cal_data[DMNRCalDataNum] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
Word16 DefaultVoIPCompen_filter[CompenFilterNum] = {
    26,    15,   -56,    27,   -17,    39,   -18,   -44,    40,     5, \
    38,   -63,    47,   -79,    52,   -22,    62,   -47,     4,    24, \
    -55,    46,   -28,   123,  -275,   241,  -208,   328,  -244,   -13, \
    98,   176,  -108,  -746,   476,   111,  1661, -2136,   206, -1707, \
    5461, -5885,  2762, -1354, 19656, 19656, -1354,  2762, -5885,  5461, \
    -1707,   206, -2136,  1661,   111,   476,  -746,  -108,   176,    98, \
    -13,  -244,   328,  -208,   241,  -275,   123,   -28,    46,   -55, \
    24,     4,   -47,    62,   -22,    52,   -79,    47,   -63,    38, \
    5,    40,   -44,   -18,    39,   -17,    27,   -56,    15,    26, \
    \
    26,    15,   -56,    27,   -17,    39,   -18,   -44,    40,     5, \
    38,   -63,    47,   -79,    52,   -22,    62,   -47,     4,    24, \
    -55,    46,   -28,   123,  -275,   241,  -208,   328,  -244,   -13, \
    98,   176,  -108,  -746,   476,   111,  1661, -2136,   206, -1707, \
    5461, -5885,  2762, -1354, 19656, 19656, -1354,  2762, -5885,  5461, \
    -1707,   206, -2136,  1661,   111,   476,  -746,  -108,   176,    98, \
    -13,  -244,   328,  -208,   241,  -275,   123,   -28,    46,   -55, \
    24,     4,   -47,    62,   -22,    52,   -79,    47,   -63,    38, \
    5,    40,   -44,   -18,    39,   -17,    27,   -56,    15,    26, \
    \
    -86,    73,  -153,   155,  -159,    46,    35,  -237,   309,  -476, \
    197,  -317,    -7,   -32,  -170,   -50,    44,   -50,  -172,   283, \
    -355,   226,  -380,   453, -1049,  1171, -1117,   733,   624, -1369, \
    3057, -3450,  3730, -1053,   478,  3304, -4044,  3533, -3125,  2856, \
    4304, -12328, 23197, -17817, 23197, 23197, -17817, 23197, -12328,  4304, \
    2856, -3125,  3533, -4044,  3304,   478, -1053,  3730, -3450,  3057, \
    -1369,   624,   733, -1117,  1171, -1049,   453,  -380,   226,  -355, \
    283,  -172,   -50,    44,   -50,  -170,   -32,    -7,  -317,   197, \
    -476,   309,  -237,    35,    46,  -159,   155,  -153,    73,   -86
};
uWord32 DefaultVoIPEnhancePara[EnhanceParasNum] =
{96, 253, 16388, 31, 57607, 31, 400, 64, 80, 4325, 611, 0, 16392, 0, 0, 8192, 0, 0, 0, 10752, 32769, 0, 0, 0, 0, 0, 0, 0};

#define MAX_DUMP_NUM (1024)
#define COMPENSATE_PCM_DATA (0xCC)
#define PREPARE_PROCESS_PCM_DATA (0x00)
#define BYPASS_DL_PCM_DATA (0x00)

int SPELayer::DumpFileNum = 0;
bool SPELayer::EPLDebugEnable = false;


int SPELayer::GetVoIPJitterTime(void) {
    char value[PROPERTY_VALUE_MAX];
    int ret = 0;
    ret = property_get("vendor.SetJitterTime", value, "0");
    int JitterTime = atoi(value);
    ALOGD("GetVoIPJitterTime JitterTime=%d,ret=%d", JitterTime, ret);

    return JitterTime;
}
int SPELayer::GetVoIPLatencyTime(void) {
    char value[PROPERTY_VALUE_MAX];
    int ret = 0;
    ret = property_get("vendor.SetLatencyTime", value, "0");
    int LatencyTime = atoi(value);
    ALOGD("GetVoIPLatencyTime LatencyTime=%d,ret=%d", LatencyTime, ret);

    return LatencyTime;
}

SPELayer::SPELayer() {
    Mutex::Autolock lock(mLock);
    ALOGD("%s+", __FUNCTION__);

    memset(&mSphEnhOps, 0, sizeof(mSphEnhOps));

    mSphCtrlBuffer = NULL;
    mpSPEBufferUL1 = NULL;
    mpSPEBufferUL2 = NULL;
    mpSPEBufferDL = NULL;
    mpSPEBufferFE = NULL;
    mpSPEBufferNE = NULL;
    mpSPEBufferDLDelay = NULL;
    mULInBufQLenTotal = 0;
    mDLInBufQLenTotal = 0;
    mULOutBufQLenTotal = 0;
    mDLOutBufQLenTotal = 0;
    mDLDelayBufQLenTotal = 0;
    mSPEProcessBufSize = 0;
    mNsecPerSample = 0;
    mMMI_ctrl_mask = MMI_CONTROL_MASK_ALL;

    //Record settings
    mRecordSampleRate = 48000;  // sample rate=48k HSR record   if sample rate=48k normal record
    mRecordFrameRate = 20;  // frame rate=20ms
    mRecordMICDigitalGain = 16; //MIC_DG for AGC
    mRecordULTotalGain = 184;   //uplink totol gain
    mRecordApp_table = 8;   //mode = "8" stereo record, "4" mono record
    mRecordFea_Cfg_table = 511; //without turing off anything

    //VoIP settings
    mPlatformOffsetTime = 0;
    mLatencyTime = 0;
    mVoIPSampleRate = 16000;    //sample rate=16k
    mLatencySampleCount = 0;    //Latency sample count, one channel
    mVoIPFrameRate = 20;    //frame rate=20ms
    mVoIPMICDigitalGain = 16;   //MIC_DG  for AGC
    mVoIPULTotalGain = 184;   //uplink totol gain
    mVoIPApp_table = 2; //mode=WB_VOIP
    mVoIPFea_Cfg_table = 511;   //without turning off anything
    mNeedDelayLatency = false;
    mLatencyDir = true;

    mNeedJitterBuffer = false;
    mDefaultJitterBufferTime = 0;
    mJitterBufferTime = GetVoIPJitterTime();
    mJitterSampleCount = mJitterBufferTime * mVoIPSampleRate / 1000; //Jitter buffer sample count, one channel

    //default record and VoIP parameters
    for (int i = 0; i < EnhanceParasNum; i++) {
        mRecordEnhanceParas[i] = DefaultRecEnhancePara[i];
        mVoIPEnhanceParas[i] = DefaultVoIPEnhancePara[i];
    }

    for (int i = 0; i < DMNRCalDataNum; i++) {
        mRecordDMNRCalData[i] = DefaultRecDMNR_cal_data[i];
        mVoIPDMNRCalData[i] = DefaultVoIPDMNR_cal_data[i];
    }

    for (int i = 0; i < CompenFilterNum; i++) {
        mRecordCompenFilter[i] = DefaultRecCompen_filter[i];
        mVoIPCompenFilter[i] = DefaultVoIPCompen_filter[i];
    }


    mMode = SPE_MODE_NONE;
    mRoute = ROUTE_NONE;
    mState = SPE_STATE_IDLE;
    mError = false;
    mVoIPRunningbefore = false;

    //for debug purpose
    mfpInDL = NULL;
    mfpInUL = NULL;
    mfpOutDL = NULL;
    mfpOutUL = NULL;
    mfpProcessedDL = NULL;
    mfpProcessedUL = NULL;
    mfpEPL = NULL;
    mfpVM = NULL;
    mVMDumpEnable = false;

    hDumpThread = 0;

    memset(&mSph_Enh_ctrl, 0, sizeof(SPH_ENH_ctrl_struct));

    mOutputStreamRunning = false;
    mNormalModeVoIP = false;
    mULDropTime = 0;
    mDLLatencyTime = 0;
    mCompensatedBufferSize = 0;
    mFirstVoIPUplink = true;
    mFirstVoIPDownlink = true;
    mPreULBufLen = 0;
    mPreDLBufLen = 0;
    mDLNewStart = false;
    mPrepareProcessDataReady = false;
    mDLPreQnum = 5 * 4;
    mDLPreQLimit = true;
    DLdataPrepareCount = 0;
    mNewReferenceBufferComes = false;
    memset(&mUplinkIntrStartTime, 0, sizeof(timespec));
    memset(&mPreUplinkEstTime, 0, sizeof(timespec));
    memset(&mULIntrDeltaTime, 0, sizeof(timespec));
    memset(&mDownlinkIntrStartTime, 0, sizeof(timespec));
    memset(&mPreDownlinkEstTime, 0, sizeof(timespec));
    memset(&mPreDownlinkQueueTime, 0, sizeof(timespec));

    mDLStreamAttribute.mBufferSize = 8192;
    mDLStreamAttribute.mChannels = 2;
    mDLStreamAttribute.mSampleRate = 44100;

    memset(mVMDumpFileName, 0, VM_DUMP_FILE_NAME_SIZE);

    /* dlopen */
    const char *error;
    const char *funName = NULL;

    mSphEnhOps.handle = dlopen("libspeech_enh_lib.so", RTLD_LAZY);
    if (!mSphEnhOps.handle) {
        ALOGE("%s(), dlopen fail! (%s)\n", __FUNCTION__, dlerror());
        ASSERT(0);
        return;
    }
    dlerror();    /* Clear any existing error */

    /* dlsym */
    funName = "ENH_API_Get_Memory";
    mSphEnhOps.ENH_API_Get_Memory = (Word32(*)(SPH_ENH_ctrl_struct * Sph_Enh_ctrl)) dlsym(mSphEnhOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        ASSERT(0);
        return;
    }

    funName = "ENH_API_Alloc";
    mSphEnhOps.ENH_API_Alloc = (Word16(*)(SPH_ENH_ctrl_struct * Sph_Enh_ctrl, Word32 * mem_ptr)) dlsym(mSphEnhOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        ASSERT(0);
        return;
    }

    funName = "ENH_API_Rst";
    mSphEnhOps.ENH_API_Rst = (Word16(*)(SPH_ENH_ctrl_struct * Sph_Enh_ctrl)) dlsym(mSphEnhOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        ASSERT(0);
        return;
    }

    funName = "ENH_API_Process";
    mSphEnhOps.ENH_API_Process = (void (*)(SPH_ENH_ctrl_struct * Sph_Enh_ctrl)) dlsym(mSphEnhOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        ASSERT(0);
        return;
    }

    funName = "ENH_API_Free";
    mSphEnhOps.ENH_API_Free = (Word16(*)(SPH_ENH_ctrl_struct * Sph_Enh_ctrl)) dlsym(mSphEnhOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        ASSERT(0);
        return;
    }

    funName = "ENH_API_Get_Version";
    mSphEnhOps.ENH_API_Get_Version = (Word16(*)(SPH_ENH_ctrl_struct * Sph_Enh_ctrl)) dlsym(mSphEnhOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        ASSERT(0);
        return;
    }

    Dump_Enalbe_Check();
    DumpFileNum++;
    DumpFileNum %= MAX_DUMP_NUM;
    ALOGD("%s-", __FUNCTION__);
}

SPELayer::~SPELayer() {
    Mutex::Autolock lock(mLock);
    AL_LOCK(mBufMutex);
    ALOGD("%s+", __FUNCTION__);

    uint32_t timeoutMs = 20;    //timeout time 20ms

    mMode = SPE_MODE_NONE;
    mRoute = ROUTE_NONE;
    //mState = SPE_STATE_IDLE;

    mError = false;

    Clear();

    FlushBufferQ();

    if (hDumpThread != 0) {
        hDumpThread = 0;
        if (AL_WAIT_MS(mDumpExitMutex, timeoutMs) != NO_ERROR) {
            ALOGD("%s, dumpthread close timeout?", __FUNCTION__);
        }
        AL_UNLOCK(mDumpExitMutex);
        DumpBufferClear();
    }

    if (mfpInDL) {
        fclose(mfpInDL);
        mfpInDL = NULL;
    }
    if (mfpInUL) {
        fclose(mfpInUL);
        mfpInUL = NULL;
    }
    if (mfpOutDL) {
        fclose(mfpOutDL);
        mfpOutDL = NULL;
    }
    if (mfpOutUL) {
        fclose(mfpOutUL);
        mfpOutUL = NULL;
    }
    if (mfpProcessedDL) {
        fclose(mfpProcessedDL);
        mfpProcessedDL = NULL;
    }
    if (mfpProcessedUL) {
        fclose(mfpProcessedUL);
        mfpProcessedUL = NULL;
    }
    if (mfpEPL) {
        fclose(mfpEPL);
        mfpEPL = NULL;
    }
    if (mfpVM) {
        fclose(mfpVM);
        mfpVM = NULL;
    }

    if (mSphEnhOps.handle) {
        dlclose(mSphEnhOps.handle);
        mSphEnhOps.handle = NULL;
    }

    AL_UNLOCK(mBufMutex);
    ALOGD("%s-", __FUNCTION__);
}

void SPELayer::FlushBufferQ(void) {
    ALOGD("%s+", __FUNCTION__);
    //clear the buffer queue, need mutex protect

    ALOGD("FlushBufferQ mULOutBufferQ size=%zu,mULInBufferQ.size=%zu,mDLOutBufferQ.size()=%zu,mDLInBufferQ.size()=%zu,mDLDelayBufferQ.size()=%zu", mULOutBufferQ.size(), mULInBufferQ.size(),
          mDLOutBufferQ.size(), mDLInBufferQ.size(), mDLDelayBufferQ.size());
    if (mULOutBufferQ.size() != 0) {
        while (mULOutBufferQ.size()) {
            free(mULOutBufferQ[0]->pBufBase);
            delete mULOutBufferQ[0];
            mULOutBufferQ.removeAt(0);
        }
        mULOutBufferQ.clear();
    }
    if (mULInBufferQ.size() != 0) {
        while (mULInBufferQ.size()) {
            free(mULInBufferQ[0]->pBufBase);
            delete mULInBufferQ[0];
            mULInBufferQ.removeAt(0);
        }
        mULInBufferQ.clear();
    }

    if (mDLOutBufferQ.size() != 0) {
        while (mDLOutBufferQ.size()) {
            free(mDLOutBufferQ[0]->pBufBase);
            delete mDLOutBufferQ[0];
            mDLOutBufferQ.removeAt(0);
        }
        mDLOutBufferQ.clear();
    }
    if (mDLInBufferQ.size() != 0) {
        while (mDLInBufferQ.size()) {
            if (mDLInBufferQ[0]->pBufBase) {
                ALOGD("mDLInBufferQ::pBufBase=%p", mDLInBufferQ[0]->pBufBase);
                //                free(mDLInBufferQ[0]->pBufBase);
                //                ALOGD("mDLInBufferQ::free");
                //                delete mDLInBufferQ[0];
                //                ALOGD("mDLInBufferQ::delete");
                mDLInBufferQ.removeAt(0);
                ALOGD("mDLInBufferQ::done, free at DLDelay buffer");
            }
        }
        mDLInBufferQ.clear();
    }

    if (mDLDelayBufferQ.size() != 0) {
        while (mDLDelayBufferQ.size()) {
            if (mDLDelayBufferQ[0]->pBufBase) {
                ALOGD("mDLDelayBufferQ::pBufBase=%p", mDLDelayBufferQ[0]->pBufBase);
                free(mDLDelayBufferQ[0]->pBufBase);
                ALOGD("mDLDelayBufferQ::free");
                delete mDLDelayBufferQ[0];
                ALOGD("mDLDelayBufferQ::delete");
                mDLDelayBufferQ.removeAt(0);
                ALOGD("mDLDelayBufferQ::done");
            }

        }
        mDLDelayBufferQ.clear();
    }

    mULInBufQLenTotal = 0;
    mDLInBufQLenTotal = 0;
    mULOutBufQLenTotal = 0;
    mDLOutBufQLenTotal = 0;
    mDLDelayBufQLenTotal = 0;
    mCompensatedBufferSize = 0;

    ALOGD("%s-", __FUNCTION__);
}

bool    SPELayer::MutexLock(void) {
    mLock.lock();
    return true;
}
bool    SPELayer::MutexUnlock(void) {
    mLock.unlock();
    return true;
}

bool SPELayer::DumpMutexLock(void) {
    mDumpLock.lock();
    return true;
}
bool SPELayer::DumpMutexUnlock(void) {
    mDumpLock.unlock();
    return true;
}

//set MMI table, dynamic on/off
bool SPELayer::SetDynamicFuncCtrl(const SPE_MMI_CONTROL_TABLE func, const bool enable) {
    Mutex::Autolock lock(mLock);
    const bool current_state = ((mMMI_ctrl_mask & func) > 0);
    ALOGD("%s(), SetDynamicFuncCtrl %x(%x), enable(%d) == current_state(%d)",
          __FUNCTION__, mMMI_ctrl_mask, func, enable, current_state);
    /*    if (current_state == enable) {
            return false;
        }*/
    if (enable == false) {
        mMMI_ctrl_mask &= (~func);
    } else {
        mMMI_ctrl_mask |= func;
        //normal/handsfree mode DMNR are exclusive
        if (func == HANDSFREE_DMNR) {
            mMMI_ctrl_mask &= (~NORMAL_DMNR);
        } else if (func == NORMAL_DMNR) {
            mMMI_ctrl_mask &= (~HANDSFREE_DMNR);
        }
    }

    mSph_Enh_ctrl.MMI_ctrl = mMMI_ctrl_mask;
    ALOGD("%s(), SetDynamicFuncCtrl %x", __FUNCTION__, mMMI_ctrl_mask);
    return true;
}

//parameters setting
bool    SPELayer::SetEnhPara(SPE_MODE mode, uWord32 *pEnhance_pars) {
    switch (mode) {
    case SPE_MODE_REC:
        memcpy(&mRecordEnhanceParas, pEnhance_pars, EnhanceParasNum * sizeof(uWord32));
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        memcpy(&mVoIPEnhanceParas, pEnhance_pars, EnhanceParasNum * sizeof(uWord32));
        break;
    default:
        ALOGD("%s, not support mode", __FUNCTION__);
        return false;
        break;
    }
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d", __FUNCTION__, mode);
    }
    /*  for(int i=0; i<EnhanceParasNum; i++)
        {
                ALOGD("mRecordEnhanceParas[%d] %d",i,mRecordEnhanceParas[i]);
                ALOGD("mSph_Enh_ctrl.enhance_pars[%d] %d",i,mSph_Enh_ctrl.enhance_pars[i]);
        }   */
    return true;
}

bool    SPELayer::SetDMNRPara(SPE_MODE mode, Word16 *pDMNR_cal_data) {
    switch (mode) {
    case SPE_MODE_REC:
        memcpy(&mRecordDMNRCalData, pDMNR_cal_data, DMNRCalDataNum * sizeof(short));
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        memcpy(&mVoIPDMNRCalData, pDMNR_cal_data, DMNRCalDataNum * sizeof(short));
        break;
    default:
        ALOGD("%s, not support mode", __FUNCTION__);
        return false;
        break;
    }
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d", __FUNCTION__, mode);
    }
    return true;
}

bool    SPELayer::SetCompFilter(SPE_MODE mode, Word16 *pCompen_filter) {
    switch (mode) {
    case SPE_MODE_REC:
        memcpy(&mRecordCompenFilter, pCompen_filter, CompenFilterNum * sizeof(short));
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        memcpy(&mVoIPCompenFilter, pCompen_filter, CompenFilterNum * sizeof(short));
        break;
    default:
        ALOGD("%s, not support mode", __FUNCTION__);
        return false;
        break;
    }

    ALOGD("%s, SPE_MODE=%d", __FUNCTION__, mode);

    return true;
}

bool    SPELayer::SetMICDigitalGain(SPE_MODE mode, Word32 gain) {
    switch (mode) {
    case SPE_MODE_REC:
        mRecordMICDigitalGain = gain;
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        mVoIPMICDigitalGain = gain;
        break;
    default:
        ALOGD("%s, not support mode", __FUNCTION__);
        return false;
        break;
    }

    ALOGD("%s, MIC_DG, SPE_MODE=%d, gain=%d", __FUNCTION__, mode, gain);

    return true;
}


bool SPELayer::SetUpLinkTotalGain(SPE_MODE mode, uint8_t gain) {
    switch (mode) {
    case SPE_MODE_REC:
        mRecordULTotalGain = gain;
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        mVoIPULTotalGain = gain;
        break;
    default:
        ALOGD("%s, not support mode", __FUNCTION__);
        return false;
        break;
    }

    ALOGD("%s, SPE_MODE=%d, gain=%d", __FUNCTION__, mode, gain);

    return true;
}

bool    SPELayer::SetSampleRate(SPE_MODE mode, long sample_rate) {

    switch (mode) {
    case SPE_MODE_REC:
        if (sample_rate != 16000 && sample_rate != 48000) {
            ALOGD("%s, Record only support 16k or 48k samplerate", __FUNCTION__);
            mRecordSampleRate = 48000;
            return false;
        }
        mRecordSampleRate = sample_rate;
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        if (sample_rate != 16000) {
            ALOGD("%s, VOIP only support 16k samplerate", __FUNCTION__);
        }

        mVoIPSampleRate = 16000;
        break;
    default:
        ALOGD("%s, not support mode", __FUNCTION__);
        return false;
        break;
    }
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d", __FUNCTION__, mode);
    }
    return true;
}

bool    SPELayer::SetFrameRate(SPE_MODE mode, long frame_rate) {

    if (frame_rate != 10 && frame_rate != 20 && frame_rate != 5 && frame_rate != 3 && frame_rate != 2 && frame_rate != 1) {
        ALOGD("%s, not supported framerate", __FUNCTION__);
        return false;
    }

    switch (mode) {
    case SPE_MODE_REC:
        mRecordFrameRate = frame_rate;
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        mVoIPFrameRate = frame_rate;
        break;
    default:
        ALOGD("%s, not support mode", __FUNCTION__);
        return false;
        break;
    }
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d, frame_rate=%lu", __FUNCTION__, mode, frame_rate);
    }
    return true;
}

bool    SPELayer::SetAPPTable(SPE_MODE mode, SPE_APP_TABLE App_table) {
    switch (mode) {
    case SPE_MODE_REC:
        mRecordApp_table = App_table;
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        mVoIPApp_table = App_table;
        break;
    default:
        ALOGD("%s, not support mode", __FUNCTION__);
        return false;
        break;
    }
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d, App_table=%x", __FUNCTION__, mode, App_table);
    }

    return true;
}

bool    SPELayer::SetFeaCfgTable(SPE_MODE mode, Word32 Fea_Cfg_table) {

    switch (mode) {
    case SPE_MODE_REC:
        mRecordFea_Cfg_table = Fea_Cfg_table;
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        mVoIPFea_Cfg_table = Fea_Cfg_table;
        break;
    default:
        ALOGD("%s, not support mode", __FUNCTION__);
        return false;
        break;
    }
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d,Fea_Cfg_table=%x", __FUNCTION__, mode, Fea_Cfg_table);
    }
    return true;
}

bool SPELayer::SetPlatfromTimeOffset(int ms) {
    ALOGD("%s, old=%d, new=%d", __FUNCTION__, mPlatformOffsetTime, ms);
    mPlatformOffsetTime = ms;
    return true;
}

bool    SPELayer::SetChannel(int channel) {
    if (channel == 1) { //mono
        mRecordApp_table = 4;    //mode = "8" stereo record, "4" mono record
    } else { //stereo
        mRecordApp_table = 8;
    }

    ALOGD("%s, only for recording, mRecordApp_table=%x", __FUNCTION__, mRecordApp_table);
    return true;
}

int SPELayer::GetChannel() {
    if (mRecordApp_table == 4) {
        return 1;
    } else {
        return 2;
    }
}
/*
bool    SPELayer::SetMode(SPE_MODE mode)
{
    ALOGD("SPELayer::SetMode %d",mode);

    if((mode!=SPE_MODE_REC) && (mode!=SPE_MODE_VOIP))
    {
        ALOGD("SPELayer::SetMode, SPE_MODE not correct");
        return false;
    }

    if (mMode == mode)
        return true;

    mMode = mode;
    return true;
}
*/

bool    SPELayer::SetRoute(SPE_ROUTE route) {
    ALOGD("%s, %d", __FUNCTION__, route);

    if ((route < ROUTE_NONE) || (route >= ROUTE_MAX)) {
        ALOGE("%s, route not correct", __FUNCTION__);
        return false;
    }

    if (mRoute == route) {
        return true;
    }

    mRoute = route;
    return true;
}

void SPELayer::SetStreamAttribute(bool direct, StreamAttribute SA) { //direct=0 =>downlink stream info, direct=1 =>uplink stream info
    ALOGD("%s(), direct=%d", __FUNCTION__, direct);
    if (direct) {
        ALOGD("%s(), not support uplink stream info yet", __FUNCTION__);
    } else {
        memcpy(&mDLStreamAttribute, &SA, sizeof(StreamAttribute));
    }
    ALOGD("%s(), mBufferSize=%d, mChannels=%d, mSampleRate=%d", __FUNCTION__, mDLStreamAttribute.mBufferSize, mDLStreamAttribute.mChannels, mDLStreamAttribute.mSampleRate);
    CalPreQNumber();
    return;
}

void SPELayer::CalPreQNumber(void) {
    int framecount = mDLStreamAttribute.mBufferSize / mDLStreamAttribute.mChannels / 2;
    int DLPreQfact = 200 / (framecount * 1000 / mDLStreamAttribute.mSampleRate);
    mDLPreQnum = DLPreQfact;
    if (EPLDebugEnable == true) {
        ALOGD("%s(), mDLPreQnum=%d, DLdataPrepareCount=%d", __FUNCTION__, mDLPreQnum, DLdataPrepareCount);
    }
}

void SPELayer::CalPrepareCount(void) {
    int framecount = mDLStreamAttribute.mBufferSize / mDLStreamAttribute.mChannels / 2;
    int DLPreparefact = 100 / (framecount * 1000 / mDLStreamAttribute.mSampleRate);
    DLdataPrepareCount = DLPreparefact;
    if (EPLDebugEnable == true) {
        ALOGD("%s(), mDLPreQnum=%d, DLdataPrepareCount=%d", __FUNCTION__, mDLPreQnum, DLdataPrepareCount);
    }
}

//Get parameters setting
int SPELayer::GetLatencyTime() {
    //only for VoIP
    return mLatencyTime;
}

SPE_MODE    SPELayer::GetMode() {
    return mMode;
}

SPE_ROUTE   SPELayer::GetRoute() {
    return mRoute;
}
bool    SPELayer::IsSPERunning() {
    if (mState == SPE_STATE_RUNNING) {
        return true;
    } else {
        return false;
    }
}


long    SPELayer::GetSampleRate(SPE_MODE mode) {
    long retSampleRate = 0;
    switch (mode) {
    case SPE_MODE_REC:
        retSampleRate = mRecordSampleRate;
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        retSampleRate = mVoIPSampleRate;
        break;
    default:
        retSampleRate = mSph_Enh_ctrl.sample_rate;
        break;
    }
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d, retSampleRate=%lu", __FUNCTION__, mode, retSampleRate);
    }
    return retSampleRate;
}

long    SPELayer::GetFrameRate(SPE_MODE mode) {
    long retFrameRate = 0;
    switch (mode) {
    case SPE_MODE_REC:
        retFrameRate = mRecordFrameRate;
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        retFrameRate = mVoIPFrameRate;
        break;
    default:
        retFrameRate = mSph_Enh_ctrl.frame_rate;
        break;
    }
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d, retFrameRate=%lu", __FUNCTION__, mode, retFrameRate);
    }
    return retFrameRate;
}

Word32    SPELayer::GetMICDigitalGain(SPE_MODE mode) {
    long retPGAGain = 0;
    switch (mode) {
    case SPE_MODE_REC:
        retPGAGain = mRecordMICDigitalGain;
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        retPGAGain = mVoIPMICDigitalGain;
        break;
    default:
        retPGAGain = mSph_Enh_ctrl.MIC_DG;
        break;
    }
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d, retPGAGain=%lu", __FUNCTION__, mode, retPGAGain);
    }
    return retPGAGain;
}

Word32    SPELayer::GetAPPTable(SPE_MODE mode) {
    Word32 retAPPTable = 0;
    switch (mode) {
    case SPE_MODE_REC:
        retAPPTable = mRecordApp_table;
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        retAPPTable = mVoIPApp_table;
        break;
    default:
        retAPPTable = mSph_Enh_ctrl.App_table;
        break;
    }
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d, retAPPTable=%x", __FUNCTION__, mode, retAPPTable);
    }
    return retAPPTable;
}

Word32    SPELayer::GetFeaCfgTable(SPE_MODE mode) {
    long retFeaCfgTable = 0;
    switch (mode) {
    case SPE_MODE_REC:
        retFeaCfgTable = mRecordFea_Cfg_table;
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        retFeaCfgTable = mVoIPFea_Cfg_table;
        break;
    default:
        retFeaCfgTable = mSph_Enh_ctrl.Fea_Cfg_table;
        break;
    }
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d, retFeaCfgTable=%lu", __FUNCTION__, mode, retFeaCfgTable);
    }
    return retFeaCfgTable;
}

bool    SPELayer::GetEnhPara(SPE_MODE mode, uWord32 *pEnhance_pars) {
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d", __FUNCTION__, mode);
    }
    switch (mode) {
    case SPE_MODE_REC:
        memcpy(pEnhance_pars, &mRecordEnhanceParas, EnhanceParasNum * sizeof(uWord32));
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        memcpy(pEnhance_pars, &mVoIPEnhanceParas, EnhanceParasNum * sizeof(uWord32));
        break;
    default:
        memcpy(pEnhance_pars, &mSph_Enh_ctrl.enhance_pars, EnhanceParasNum * sizeof(uWord32));
        break;
    }

    return true;
}

bool    SPELayer::GetDMNRPara(SPE_MODE mode, Word16 *pDMNR_cal_data) {
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d", __FUNCTION__, mode);
    }
    switch (mode) {
    case SPE_MODE_REC:
        memcpy(pDMNR_cal_data, &mRecordDMNRCalData, DMNRCalDataNum * sizeof(Word16));
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        memcpy(pDMNR_cal_data, &mVoIPDMNRCalData, DMNRCalDataNum * sizeof(Word16));
        break;
    default:
        memcpy(pDMNR_cal_data, &mSph_Enh_ctrl.DMNR_cal_data, DMNRCalDataNum * sizeof(Word16));
        break;
    }

    return true;
}

bool    SPELayer::GetCompFilter(SPE_MODE mode, Word16 *pCompen_filter) {
    if (EPLDebugEnable == true) {
        ALOGD("%s, SPE_MODE=%d", __FUNCTION__, mode);
    }
    switch (mode) {
    case SPE_MODE_REC:
        memcpy(pCompen_filter, &mRecordCompenFilter, CompenFilterNum * sizeof(short));
        break;
    case SPE_MODE_VOIP:
    case SPE_MODE_AECREC:
        memcpy(pCompen_filter, &mVoIPCompenFilter, CompenFilterNum * sizeof(short));
        break;
    default:
        memcpy(pCompen_filter, &mSph_Enh_ctrl.Compen_filter, CompenFilterNum * sizeof(short));
        break;
    }

    return true;
}

bool SPELayer::GetUPlinkIntrStartTime() {
    Mutex::Autolock lock(mLock);
    if (mState == SPE_STATE_RUNNING) {
        return false;
    }

    mUplinkIntrStartTime = GetSystemTime();
    if (EPLDebugEnable == true) {
        ALOGD("%s, sec=%lu, nsec=%lu", __FUNCTION__, mUplinkIntrStartTime.tv_sec, mUplinkIntrStartTime.tv_nsec);
    }
    mFirstVoIPUplink = true;
    mDLPreQLimit = false;
    return true;;
}

bool SPELayer::SetUPLinkIntrStartTime(struct timespec UPlinkStartTime) {
    Mutex::Autolock lock(mLock);
    if (mState == SPE_STATE_RUNNING) {
        return false;
    }

    mUplinkIntrStartTime = UPlinkStartTime;
    if (EPLDebugEnable == true) {
        ALOGD("%s, sec=%lu, nsec=%lu", __FUNCTION__, mUplinkIntrStartTime.tv_sec, mUplinkIntrStartTime.tv_nsec);
    }
    mFirstVoIPUplink = true;
    mDLPreQLimit = false;
    return true;;
}

bool SPELayer::GetDownlinkIntrStartTime() {
    Mutex::Autolock lock(mLock);
    AL_LOCK(mBufMutex);
    mDownlinkIntrStartTime = GetSystemTime();
    if (EPLDebugEnable == true) {
        ALOGD("%s, sec=%lu, nsec=%lu, size=%zu, mDLDelayBufferQ size()=%zu", __FUNCTION__, mDownlinkIntrStartTime.tv_sec, mDownlinkIntrStartTime.tv_nsec, mDLInBufferQ.size(), mDLDelayBufferQ.size());
    }
    if (mDLInBufferQ.size() > 0) {
        for (size_t i = 0; i < mDLInBufferQ.size() ; i++) {
            if (mDLInBufferQ[i]->DLfirstBuf == true) {
                if (EPLDebugEnable == true) {
                    ALOGD("%s, update estimate time", __FUNCTION__);
                }
                mDLInBufferQ[i]->time_stamp_estimate.tv_sec = mDownlinkIntrStartTime.tv_sec;
                mDLInBufferQ[i]->time_stamp_estimate.tv_nsec = mDownlinkIntrStartTime.tv_nsec;
                mPreDownlinkEstTime.tv_sec = mDownlinkIntrStartTime.tv_sec;
                mPreDownlinkEstTime.tv_nsec = mDownlinkIntrStartTime.tv_nsec;
                mPreDownlinkQueueTime.tv_sec = mDownlinkIntrStartTime.tv_sec;
                mPreDownlinkQueueTime.tv_nsec = mDownlinkIntrStartTime.tv_nsec;
            }
        }
        for (size_t i = 0; i < mDLDelayBufferQ.size() ; i++) {
            if (mDLDelayBufferQ[i]->DLfirstBuf == true) {
                if (EPLDebugEnable == true) {
                    ALOGD("%s, update estimate time mDLDelayBufferQ", __FUNCTION__);
                }
                mDLDelayBufferQ[i]->time_stamp_estimate.tv_sec = mDownlinkIntrStartTime.tv_sec;
                mDLDelayBufferQ[i]->time_stamp_estimate.tv_nsec = mDownlinkIntrStartTime.tv_nsec;
            }
        }
    }
    mDLNewStart = false;
    AL_UNLOCK(mBufMutex);
    return true;;
}

void SPELayer::SetEchoRefStartTime(struct timespec EchoRefStartTime) {
    Mutex::Autolock lock(mLock);
    mDownlinkIntrStartTime = EchoRefStartTime;
    if (EPLDebugEnable == true) {
        ALOGD("%s, sec=%lu, nsec=%lu, size=%zu, mDLDelayBufferQ size()=%zu", __FUNCTION__, mDownlinkIntrStartTime.tv_sec, mDownlinkIntrStartTime.tv_nsec, mDLInBufferQ.size(), mDLDelayBufferQ.size());
    }
    mDLNewStart = false;
}

//speech enhancement setting and process
bool    SPELayer::Start(SPE_MODE mode) { //for VOIP, both uplink/downlink
    if (EPLDebugEnable == true) {
        ALOGD("%s+, mode=%d", __FUNCTION__, mode);
    }
    Mutex::Autolock lock(mLock);
    if (mState == SPE_STATE_RUNNING) {
        ALOGD("%s already start!", __FUNCTION__);
        return false;
    }

    if ((mMode != SPE_MODE_NONE) && (mMode != mode)) {
        AL_LOCK(mBufMutex);
        FlushBufferQ();
        AL_UNLOCK(mBufMutex);
    }

    // set mSph_Enh_ctrl parameters
    if (mode == SPE_MODE_REC) {
        mSph_Enh_ctrl.sample_rate = mRecordSampleRate;
        mSph_Enh_ctrl.frame_rate = mRecordFrameRate;
        mSph_Enh_ctrl.MIC_DG = mRecordMICDigitalGain;
        mSph_Enh_ctrl.Fea_Cfg_table = mRecordFea_Cfg_table;
        mSph_Enh_ctrl.App_table = mRecordApp_table;
        mSph_Enh_ctrl.MMI_ctrl = mMMI_ctrl_mask;
        mSph_Enh_ctrl.MMI_MIC_GAIN = mRecordULTotalGain;
        memcpy(&mSph_Enh_ctrl.enhance_pars, &mRecordEnhanceParas, EnhanceParasNum * sizeof(uWord32));
        memcpy(&mSph_Enh_ctrl.DMNR_cal_data, &mRecordDMNRCalData, DMNRCalDataNum * sizeof(Word16));
        memcpy(&mSph_Enh_ctrl.Compen_filter, &mRecordCompenFilter, CompenFilterNum * sizeof(Word16));
        ALOGD("mRecordSampleRate=%d, mRecordFrameRate=%d, mRecordApp_table=%x", mRecordSampleRate, mRecordFrameRate, mRecordApp_table);
    } else if ((mode == SPE_MODE_VOIP) || (mode == SPE_MODE_AECREC)) {
        mSph_Enh_ctrl.sample_rate = mVoIPSampleRate;
        mSph_Enh_ctrl.frame_rate = mVoIPFrameRate;
        mSph_Enh_ctrl.MIC_DG = mVoIPMICDigitalGain;
        mSph_Enh_ctrl.Fea_Cfg_table = mVoIPFea_Cfg_table;
        mSph_Enh_ctrl.App_table = mVoIPApp_table;
        if (mode == SPE_MODE_AECREC) {
            mNormalModeVoIP = true;
        }

        mSph_Enh_ctrl.MMI_ctrl = mMMI_ctrl_mask;
        mSph_Enh_ctrl.MMI_MIC_GAIN = mVoIPULTotalGain;
        memcpy(&mSph_Enh_ctrl.enhance_pars, &mVoIPEnhanceParas, EnhanceParasNum * sizeof(uWord32));
        memcpy(&mSph_Enh_ctrl.DMNR_cal_data, &mVoIPDMNRCalData, DMNRCalDataNum * sizeof(Word16));
        memcpy(&mSph_Enh_ctrl.Compen_filter, &mVoIPCompenFilter, CompenFilterNum * sizeof(Word16));

        mLatencyTime = mPlatformOffsetTime + GetVoIPLatencyTime();
        if (mLatencyTime != 0) {
            mNeedDelayLatency = true;
        }

        if (mLatencyTime < 0) {
            mLatencyTime = abs(mLatencyTime);
            mLatencyDir = false;
        }
        ALOGD("mLatencyTime=%d,mLatencyDir=%x", mLatencyTime, mLatencyDir);
        mLatencySampleCount = mLatencyTime * mVoIPSampleRate / 1000; //Latency sample count

        mJitterSampleCount = mJitterBufferTime * mVoIPSampleRate / 1000; //Jitter buffer sample count, one channel
        if (mJitterBufferTime != 0) {
            mNeedJitterBuffer = true;
        }

    } else {
        ALOGD("%s, wrong mode", __FUNCTION__);
        return false;
    }

    mSph_Enh_ctrl.Device_mode = mRoute;

    if (mSphCtrlBuffer) {
        ALOGD("%s mSphCtrlBuffer already exist, delete and recreate", __FUNCTION__);
        mSphEnhOps.ENH_API_Free(&mSph_Enh_ctrl);
        free(mSphCtrlBuffer);
        mSphCtrlBuffer = NULL;
    }

    Word16 ver = mSphEnhOps.ENH_API_Get_Version(&mSph_Enh_ctrl);
    ALOGD("%s, SWIP ver = %d.%d\n", __FUNCTION__, (ver & 0x1FF8) >> 3, ver & 0x7);

    uint32_t mem_size;
    mem_size = mSphEnhOps.ENH_API_Get_Memory(&mSph_Enh_ctrl);
    mSphCtrlBuffer = (int *) malloc(mem_size);
    if (mSphCtrlBuffer == NULL) {
        ALOGD("%s, create fail", __FUNCTION__);
        return false;
    }
    ALOGD("%s, going to configure,mSphCtrlBuffer=%p,mem_size=%d", __FUNCTION__, mSphCtrlBuffer, mem_size);
    memset(mSphCtrlBuffer, 0, mem_size);

    dump();
    AL_LOCK(mBufMutex);
    mSphEnhOps.ENH_API_Alloc(&mSph_Enh_ctrl, (Word32 *)mSphCtrlBuffer);

    mSphEnhOps.ENH_API_Rst(&mSph_Enh_ctrl);


    mMode = mode;
    mState = SPE_STATE_START;

    //set the address
    ALOGD("mSph_Enh_ctrl.frame_rate %d", mSph_Enh_ctrl.frame_rate);
    if (mMode == SPE_MODE_REC) {
        if (mSph_Enh_ctrl.frame_rate == 20) { //frame rate = 20ms, buffer size 320*2, input/output use the same address
            mpSPEBufferUL1 = &mSph_Enh_ctrl.PCM_buffer[0];

            if (mSph_Enh_ctrl.sample_rate == 16000) {
                ALOGD("Start 16K record!!!");
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[Rec16KUL2BufStartAddr];
                mSPEProcessBufSize = RecBufSize16K20ms * 2 * sizeof(short); //for 16k samplerate with 20ms frame rate (stereo)
            } else { //48k sample rate
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[Rec48KUL2BufStartAddr];
                mSPEProcessBufSize = RecBufSize48K20ms * 2 * sizeof(short); //for 48k samplerate with 20ms frame rate (stereo)
            }

        } else if (mSph_Enh_ctrl.frame_rate == 10) { //frame rate = 10ms, buffer size 480*2

            mpSPEBufferUL1 = &mSph_Enh_ctrl.PCM_buffer[0];


            if (mSph_Enh_ctrl.sample_rate == 16000) {
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[Rec16KUL2BufStartAddr];
                mSPEProcessBufSize = RecBufSize16K10ms * 2 * sizeof(short); //for 16k samplerate with 10ms frame rate (stereo)
            } else { //48K samplerate
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[Rec48KUL2BufStartAddr];
                mSPEProcessBufSize = RecBufSize48K10ms * 2 * sizeof(short); //for 48k samplerate with 10ms frame rate (stereo)
            }

        } else if (mSph_Enh_ctrl.frame_rate == 5) { //frame rate = 5ms

            mpSPEBufferUL1 = &mSph_Enh_ctrl.PCM_buffer[0];


            if (mSph_Enh_ctrl.sample_rate == 16000) {
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[Rec16KUL2BufStartAddr];
                mSPEProcessBufSize = RecBufSize16K5ms * 2 * sizeof(short); //for 16k samplerate with 5ms frame rate (stereo)
            } else { //48K samplerate
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[Rec48KUL2BufStartAddr];
                mSPEProcessBufSize = RecBufSize48K5ms * 2 * sizeof(short); //for 48k samplerate with 5ms frame rate (stereo)
            }

        } else if (mSph_Enh_ctrl.frame_rate == 3) { //frame rate = 3ms

            mpSPEBufferUL1 = &mSph_Enh_ctrl.PCM_buffer[0];


            if (mSph_Enh_ctrl.sample_rate == 16000) {
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[Rec16KUL2BufStartAddr];
                mSPEProcessBufSize = RecBufSize16K3ms * 2 * sizeof(short); //for 16k samplerate with 3ms frame rate (stereo)
            } else { //48K samplerate
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[Rec48KUL2BufStartAddr];
                mSPEProcessBufSize = RecBufSize48K3ms * 2 * sizeof(short); //for 48k samplerate with 3ms frame rate (stereo)
            }

        } else if (mSph_Enh_ctrl.frame_rate == 2) { //frame rate = 2ms

            mpSPEBufferUL1 = &mSph_Enh_ctrl.PCM_buffer[0];


            if (mSph_Enh_ctrl.sample_rate == 16000) {
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[Rec16KUL2BufStartAddr];
                mSPEProcessBufSize = RecBufSize16K2ms * 2 * sizeof(short); //for 16k samplerate with 3ms frame rate (stereo)
            } else { //48K samplerate
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[Rec48KUL2BufStartAddr];
                mSPEProcessBufSize = RecBufSize48K2ms * 2 * sizeof(short); //for 48k samplerate with 3ms frame rate (stereo)
            }

        } else if (mSph_Enh_ctrl.frame_rate == 1) { //frame rate = 1ms

            mpSPEBufferUL1 = &mSph_Enh_ctrl.PCM_buffer[0];


            if (mSph_Enh_ctrl.sample_rate == 16000) {
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[Rec16KUL2BufStartAddr];
                mSPEProcessBufSize = RecBufSize16K1ms * 2 * sizeof(short); //for 16k samplerate with 3ms frame rate (stereo)
            } else { //48K samplerate
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[Rec48KUL2BufStartAddr];
                mSPEProcessBufSize = RecBufSize48K1ms * 2 * sizeof(short); //for 48k samplerate with 3ms frame rate (stereo)
            }

        } else {
            ALOGD("wrong mSph_Enh_ctrl.frame_rate setting %d", mSph_Enh_ctrl.frame_rate);
        }
    } else { //VoIP mode
        //only support 16K samplerate
        if (mSph_Enh_ctrl.frame_rate == 20) { //frame rate = 20ms, buffer size 320*4
            mpSPEBufferUL1 = &mSph_Enh_ctrl.PCM_buffer[0];
            mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[320];
            mpSPEBufferDL = &mSph_Enh_ctrl.PCM_buffer[640];
            mpSPEBufferDLDelay = &mSph_Enh_ctrl.PCM_buffer[960];

            mSPEProcessBufSize = 320 * 2 * sizeof(short); //for 16k samplerate with  20ms frame rate (stereo)
        } else { //frame rate = 10ms, buffer size 160*4
            mpSPEBufferUL1 = &mSph_Enh_ctrl.PCM_buffer[0];
            mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[160];
            mpSPEBufferDL = &mSph_Enh_ctrl.PCM_buffer[320];
            mpSPEBufferDLDelay = &mSph_Enh_ctrl.PCM_buffer[480];

            mSPEProcessBufSize = 160 * 2 * sizeof(short); //for 16k samplerate with  20ms frame rate (stereo)

        }
        mpSPEBufferNE = mpSPEBufferUL1;
        mpSPEBufferFE = mpSPEBufferDL;
    }

    mNsecPerSample = 1000000000 / mVoIPSampleRate;
    ALOGD("mNsecPerSample=%lld", mNsecPerSample);

    AL_UNLOCK(mBufMutex);
    /*
        mfpInDL = NULL;
        mfpInUL = NULL;
        mfpOutDL = NULL;
        mfpOutUL = NULL;
        mfpProcessedDL = NULL;
        mfpProcessedUL = NULL;
        mfpEPL = NULL;
        mfpVM = NULL;
    */
    ALOGD("mSPEProcessBufSize=%d", mSPEProcessBufSize);
    return true;
}

void SPELayer::WriteReferenceBuffer(struct InBufferInfo *Binfo) {
    struct timespec entertime;
    struct timespec leavetime;
    unsigned long long timediff = 0;
    mBufMutexWantLock.lock();
    entertime = GetSystemTime();
    mNewReferenceBufferComes = true;
    AL_LOCK(mBufMutex);
    mBufMutexWantLock.unlock();

    //normal VoIP is running case, and path routing case
    if (((mState == SPE_STATE_RUNNING) && ((mMode == SPE_MODE_VOIP) || (mMode == SPE_MODE_AECREC))) || mVoIPRunningbefore) {
        if (EPLDebugEnable == true) {
            ALOGD("WriteReferenceBuffer,inBufLength=%u", Binfo->BufLen);
        }
        AddtoInputBuffer(DOWNLINK, Binfo);
    }//prequeue happens before VoIP is running (mode is VoIP and state is START or mode is None and state is IDLE)
    else if ((mState != SPE_STATE_CLEANING) && (mMode != SPE_MODE_REC)) {
        if (EPLDebugEnable == true) {
            ALOGD("WriteDLQueue,inBufLength=%u", Binfo->BufLen);
        }
        AddtoInputBuffer(DOWNLINK, Binfo, true);
    }
    AL_UNLOCK(mBufMutex);
    mNewReferenceBufferComes = false;
    leavetime = GetSystemTime();

    timediff = TimeDifference(leavetime, entertime);
    if (timediff > 20000000) {
        ALOGD("WriteReferenceBuffer, process too long? %lld", timediff);
    }

}

void SPELayer::SetOutputStreamRunning(bool bRunning, bool bFromOutputStart) {
    Mutex::Autolock lock(mLock);
    AL_LOCK(mBufMutex);
    if (EPLDebugEnable == true) {
        ALOGD("%s, %d, %d, %d", __FUNCTION__, bRunning, mOutputStreamRunning, bFromOutputStart);
    }
    if ((bRunning == true) && (bFromOutputStart == true)) {
        mDLNewStart = true;
        mPrepareProcessDataReady = false;
    }

    if (bRunning == false) {
        mFirstVoIPDownlink = true;
    }
#if 1
    if ((mOutputStreamRunning == false) && (bRunning == true)) { //need to re-add the delay latency when input informed output restart running
        if (mLatencyTime != 0) {
            ALOGD("resync the latency delay time");
            mNeedDelayLatency = true;
        }

        if (mJitterBufferTime != 0) {
            mNeedJitterBuffer = true;
        }
    }
#endif
    mOutputStreamRunning = bRunning;
    AL_UNLOCK(mBufMutex);
}

void SPELayer::EnableNormalModeVoIP(bool bSet) {
    Mutex::Autolock lock(mLock);
    AL_LOCK(mBufMutex);
    ALOGD("%s, %d", __FUNCTION__, bSet);
    mNormalModeVoIP = bSet;
    AL_UNLOCK(mBufMutex);
}

void SPELayer::SetUPLinkDropTime(uint32_t droptime) {
    Mutex::Autolock lock(mLock);
    AL_LOCK(mBufMutex);
    mULDropTime = droptime; //ms
    ALOGD("%s, %d", __FUNCTION__, mULDropTime);
    AL_UNLOCK(mBufMutex);
}

void SPELayer::SetDownLinkLatencyTime(uint32_t latencytime) {
    Mutex::Autolock lock(mLock);
    AL_LOCK(mBufMutex);
    mDLLatencyTime = latencytime; //ms
    ALOGD("%s, %d", __FUNCTION__, mDLLatencyTime);
    AL_UNLOCK(mBufMutex);
}

void SPELayer::DropDownlinkData(uint32_t dropsample) {
    uint32_t diffBufLength = dropsample * 2;
    while (diffBufLength > 0) {
        if (mDLInBufferQ.isEmpty() || mDLDelayBufferQ.isEmpty()) {
            ALOGW("%s, no mDLInBufferQ data", __FUNCTION__);
            break;
        }
        if ((diffBufLength > (uint32_t)mDLInBufQLenTotal) || (diffBufLength > (uint32_t)mDLDelayBufQLenTotal)) {
            //time diff more than DL preQ data
            ALOGW("%s, something wrong happened?", __FUNCTION__);
            diffBufLength = mDLInBufQLenTotal;
            //break;
        }
        if (EPLDebugEnable == true) {
            ALOGD("%s, drop DL data diffBufLength=%d, mDLInBufferQ.size()=%zu, mDLInBufferQ[0]->BufLen=%d!!!", __FUNCTION__, diffBufLength, mDLInBufferQ.size(), mDLInBufferQ[0]->BufLen);
        }
        if (diffBufLength >= (uint32_t)mDLInBufferQ[0]->BufLen) {
            //drop DL data
            uint32_t droplength = mDLInBufferQ[0]->BufLen;
            diffBufLength -= mDLInBufferQ[0]->BufLen;
            mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
            mDLInBufferQ.removeAt(0);

            //drop DL delay data
            while (droplength > 0) {
                if (EPLDebugEnable == true) {
                    ALOGD("%s, drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%zu, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", __FUNCTION__, droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                }
                if (droplength < (uint32_t)mDLDelayBufferQ[0]->BufLen4Delay) {
                    if (EPLDebugEnable == true) {
                        ALOGD("%s, mDLDelayBufferQ[0]->pRead=%p", __FUNCTION__, mDLDelayBufferQ[0]->pRead);
                    }
                    mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                    mDLDelayBufQLenTotal -= droplength;
                    mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                    droplength = 0;
                    ALOGD("%s, after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", __FUNCTION__, mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                } else {
                    droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                    mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                    free(mDLDelayBufferQ[0]->pBufBase);
                    delete mDLDelayBufferQ[0];
                    mDLDelayBufferQ.removeAt(0);
                }
            }
        } else {
            //                    ALOGW("PrepareProcessData !!! break for test");
            //                    break;
            //drop DL data
            if (EPLDebugEnable == true) {
                ALOGD("%s, mDLInBufferQ[0]->pRead=%p , mDLInBufferQ[0]->BufLen=%d, sec %ld, nsec %ld", __FUNCTION__, mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, mDLInBufferQ[0]->time_stamp_estimate.tv_sec,
                      mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
            }
            uint32_t droplength = diffBufLength;
            mDLInBufferQ[0]->BufLen -= diffBufLength;   //record the buffer you consumed
            mDLInBufQLenTotal -= diffBufLength;
            mDLInBufferQ[0]->pRead = mDLInBufferQ[0]->pRead + diffBufLength / 2;

            //unsigned long long updateDLnsecdiff = (diffBufLength/2)*1000000000/mVoIPSampleRate;

            uint32_t adjustsample = diffBufLength / 2;
            unsigned long long updateDLnsecdiff = 0;
            updateDLnsecdiff = (adjustsample * (unsigned long long)1000000) / 16;

            mDLInBufferQ[0]->time_stamp_estimate.tv_sec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec + (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + updateDLnsecdiff) / 1000000000;
            mDLInBufferQ[0]->time_stamp_estimate.tv_nsec = (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + updateDLnsecdiff) % 1000000000;

            ALOGD("%s, after mDLInBufferQ[0]->pRead=%p, mDLInBufferQ[0]->BufLen=%d, updatensecdiff=%lld, sec=%ld, nsec=%ld", __FUNCTION__, mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, updateDLnsecdiff,
                  mDLInBufferQ[0]->time_stamp_estimate.tv_sec, mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
            diffBufLength = 0;

            //drop DL delay data
            while (droplength > 0) {
                if (EPLDebugEnable == true) {
                    ALOGD("%s, drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%zu, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", __FUNCTION__, droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                }
                if (droplength < (uint32_t)mDLDelayBufferQ[0]->BufLen4Delay) {
                    if (EPLDebugEnable == true) {
                        ALOGD("%s, mDLDelayBufferQ[0]->pRead=%p", __FUNCTION__, mDLDelayBufferQ[0]->pRead4Delay);
                    }
                    mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                    mDLDelayBufQLenTotal -= droplength;
                    mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                    droplength = 0;
                    ALOGD("%s, after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", __FUNCTION__, mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                } else {
                    droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                    mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                    free(mDLDelayBufferQ[0]->pBufBase);
                    delete mDLDelayBufferQ[0];
                    mDLDelayBufferQ.removeAt(0);
                }
            }

        }
    }

}

void SPELayer::AddtoInputBuffer(SPE_DATA_DIRECTION dir, struct InBufferInfo *BInputInfo, bool prequeue) {

    //pthread_mutex_lock(&mBufMutex );
    int inBufLen = BInputInfo->BufLen;
    short *inBufAddr = BInputInfo->pBufBase;
    bool bRemainInfo = BInputInfo->bHasRemainInfo;
    bool bPreQueue = prequeue;

    Dump_PCM_In(dir, inBufAddr, inBufLen);

    //    ALOGD("SPELayer::Process, dir=%x, inBuf=%p,inBufLength=%d,mMode=%x,copysize=%d",dir,inBuf,inBufLength,mMode,copysize);

    BufferInfo *newInBuffer = new BufferInfo;
    memset(newInBuffer, 0, sizeof(BufferInfo));
    struct timespec tstamp_queue;
    newInBuffer->pBufBase = (short *) malloc(inBufLen);
    ASSERT(newInBuffer->pBufBase != NULL);

    memcpy(newInBuffer->pBufBase, inBufAddr, inBufLen);

    //tstamp_queue = GetSystemTime(true, dir);  //modify to use read time
    tstamp_queue = BInputInfo->time_stamp_queued;

    newInBuffer->BufLen = inBufLen;
    newInBuffer->pRead = newInBuffer->pBufBase;
    newInBuffer->pWrite = newInBuffer->pBufBase;
    newInBuffer->time_stamp_queued = tstamp_queue;
    memset((void *) & (newInBuffer->time_stamp_process), 0, sizeof(newInBuffer->time_stamp_process));
    //newInBuffer->time_stamp_process = {0};
    newInBuffer->DLfirstBuf = false;
    /*
        if (prequeue)
        {
            ALOGD("AddtoInputBuffer, newInBuffer=%p, pBufBase=%p", newInBuffer, newInBuffer->pBufBase);
        }
    */
    if ((dir == UPLINK) && ((mMode == SPE_MODE_VOIP) || (mMode == SPE_MODE_AECREC))) {
        if (EPLDebugEnable == true) {
            ALOGD("uplink estimate time bRemainInfo=%d, pre tv_sec=%ld, pre nsec=%ld, mPreDLBufLen=%d, tv_sec=%ld, nsec=%ld",
                  bRemainInfo, mPreUplinkEstTime.tv_sec, mPreUplinkEstTime.tv_nsec, mPreDLBufLen, BInputInfo->time_stamp_predict.tv_sec, BInputInfo->time_stamp_predict.tv_nsec);
        }
        if (mFirstVoIPUplink) {
            mFirstVoIPUplink = false;
#if 1   //the first estimation time use the UL intr time, it is absolute time
            if (bRemainInfo) {
                mPreUplinkEstTime.tv_sec = BInputInfo->time_stamp_predict.tv_sec;
                mPreUplinkEstTime.tv_nsec = BInputInfo->time_stamp_predict.tv_nsec;
            } else {
                mPreUplinkEstTime.tv_sec = mUplinkIntrStartTime.tv_sec;
                if (mUplinkIntrStartTime.tv_nsec + mULDropTime * 1000000 >= 1000000000) {
                    mPreUplinkEstTime.tv_sec++;
                    mPreUplinkEstTime.tv_nsec = mUplinkIntrStartTime.tv_nsec + mULDropTime * 1000000 - 1000000000;
                } else {
                    mPreUplinkEstTime.tv_nsec = mUplinkIntrStartTime.tv_nsec + mULDropTime * 1000000;
                }

            }
#else //ULTR
            //the first estimation time use the first UL buffer queue time, it is corresponding time
            mPreUplinkEstTime.tv_sec = tstamp_queue.tv_sec;
            mPreUplinkEstTime.tv_nsec = tstamp_queue.tv_nsec;

            if (tstamp_queue.tv_sec >= mUplinkIntrStartTime.tv_sec) {
                if (tstamp_queue.tv_nsec >= mUplinkIntrStartTime.tv_nsec) {
                    mULIntrDeltaTime.tv_sec = tstamp_queue.tv_sec - mUplinkIntrStartTime.tv_sec;
                    mULIntrDeltaTime.tv_nsec = tstamp_queue.tv_nsec - mUplinkIntrStartTime.tv_nsec;
                } else {
                    if (tstamp_queue.tv_sec >= mUplinkIntrStartTime.tv_sec) {
                        ALOGW("first uplink estimate time error");
                    } else {
                        mULIntrDeltaTime.tv_sec = tstamp_queue.tv_sec - mUplinkIntrStartTime.tv_sec - 1;
                        mULIntrDeltaTime.tv_nsec = 1000000000 + tstamp_queue.tv_nsec - mUplinkIntrStartTime.tv_nsec;
                    }
                }
            }
#endif
            newInBuffer->time_stamp_estimate.tv_sec = mPreUplinkEstTime.tv_sec;
            newInBuffer->time_stamp_estimate.tv_nsec = mPreUplinkEstTime.tv_nsec;
            if (EPLDebugEnable == true) {
                ALOGD("first uplink estimate time bRemainInfo=%d, sec %ld nsec %ld, inBufLength=%d, mULIntrDeltaTime sec %ld nsec %ld",
                      bRemainInfo, mPreUplinkEstTime.tv_sec, mPreUplinkEstTime.tv_nsec, inBufLen, mULIntrDeltaTime.tv_sec, mULIntrDeltaTime.tv_nsec);
            }
            mPreULBufLen = inBufLen;
        } else {
            if (bRemainInfo) {
                struct timespec tempTime;
                tempTime.tv_sec = BInputInfo->time_stamp_predict.tv_sec;
                tempTime.tv_nsec = BInputInfo->time_stamp_predict.tv_nsec;
                if (TimeDifference(tempTime, mPreUplinkEstTime) > 40000000) {
                    ALOGD("AddtoInputBuffer uplink interval too long, need to do resync?");
                }

                mPreUplinkEstTime.tv_sec = BInputInfo->time_stamp_predict.tv_sec;
                mPreUplinkEstTime.tv_nsec = BInputInfo->time_stamp_predict.tv_nsec;

                newInBuffer->time_stamp_estimate.tv_sec = BInputInfo->time_stamp_predict.tv_sec;
                newInBuffer->time_stamp_estimate.tv_nsec = BInputInfo->time_stamp_predict.tv_nsec;
            } else {
                struct timespec Esttstamp;
                //unsigned long long ns = (((mPreULBufLen*1000000000)/2)/2)/mVoIPSampleRate;
                unsigned long long ns = ((mPreULBufLen * (unsigned long long)1000000) / 64);
                Esttstamp.tv_sec = mPreUplinkEstTime.tv_sec;
                //ALOGD("uplink estimate mPreUplinkEstTime, ns=%lld, tv_sec=%ld, nsec=%ld, mPreDLBufLen=%d", ns, mPreUplinkEstTime.tv_sec, mPreUplinkEstTime.tv_nsec, mPreULBufLen);
                if (mPreUplinkEstTime.tv_nsec + ns >= 1000000000) {
                    Esttstamp.tv_sec++;
                    Esttstamp.tv_nsec = mPreUplinkEstTime.tv_nsec + ns - 1000000000;
                } else {
                    Esttstamp.tv_nsec = mPreUplinkEstTime.tv_nsec + ns;
                }

                newInBuffer->time_stamp_estimate.tv_sec = Esttstamp.tv_sec;
                newInBuffer->time_stamp_estimate.tv_nsec = Esttstamp.tv_nsec;
                if (EPLDebugEnable == true) {
                    ALOGD("uplink estimate time, sec %ld nsec %ld, inBufLength=%d", Esttstamp.tv_sec, Esttstamp.tv_nsec, inBufLen);
                }
                mPreUplinkEstTime.tv_sec = Esttstamp.tv_sec;
                mPreUplinkEstTime.tv_nsec = Esttstamp.tv_nsec;
            }
            mPreULBufLen = inBufLen;
        }
    }

    if (dir == DOWNLINK) {
        if (EPLDebugEnable == true) {
            ALOGD("AddtoInputBuffer queue downlink sec %ld nsec %ld, downlink sec %ld nsec %ld",
                  BInputInfo->time_stamp_queued.tv_sec, BInputInfo->time_stamp_queued.tv_nsec, BInputInfo->time_stamp_predict.tv_sec, BInputInfo->time_stamp_predict.tv_nsec);
        }
        if (mFirstVoIPDownlink) {
            mFirstVoIPDownlink = false;
            if (mDLNewStart) { //downlink starts first time, the first DL buffer queue will earlier than interrupt enable, it happens when output starts after input stream create
                //mDLNewStart = false;
                newInBuffer->DLfirstBuf = true;

                //need to modify the estimate start time again when downlink Interrupt set.
                newInBuffer->time_stamp_estimate.tv_sec = BInputInfo->time_stamp_queued.tv_sec;
                newInBuffer->time_stamp_estimate.tv_nsec = BInputInfo->time_stamp_queued.tv_nsec;
                if (mDLLatencyTime * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec >= 1000000000) {
                    newInBuffer->time_stamp_estimate.tv_sec++;
                    newInBuffer->time_stamp_estimate.tv_nsec = mDLLatencyTime * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec - 1000000000;
                }
                mPreDownlinkEstTime.tv_sec = newInBuffer->time_stamp_estimate.tv_sec;
                mPreDownlinkEstTime.tv_nsec = newInBuffer->time_stamp_estimate.tv_nsec;
                if (EPLDebugEnable == true) {
                    ALOGD("downlink first time mDLNewStart queue estimate time, sec %ld nsec %ld, inBufLength=%d", mPreDownlinkEstTime.tv_sec, mPreDownlinkEstTime.tv_nsec, inBufLen);
                }
            } else { //the first DL buffer queue after downlink already start, it happens when input stream create after output is running
                if (bRemainInfo) {
                    newInBuffer->time_stamp_estimate.tv_sec = BInputInfo->time_stamp_predict.tv_sec;
                    newInBuffer->time_stamp_estimate.tv_nsec = BInputInfo->time_stamp_predict.tv_nsec;
                } else {
                    //use DL hardware buffer latency for estimate? or buffer length?
                    newInBuffer->time_stamp_estimate.tv_sec = BInputInfo->time_stamp_queued.tv_sec;
                    newInBuffer->time_stamp_estimate.tv_nsec = BInputInfo->time_stamp_queued.tv_nsec;
                    ALOGD("mDLLatencyTime=%d", mDLLatencyTime);
                    if ((mDLLatencyTime / 2) * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec >= 1000000000) {
                        newInBuffer->time_stamp_estimate.tv_sec++;
                        newInBuffer->time_stamp_estimate.tv_nsec = (mDLLatencyTime / 2) * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec - 1000000000;
                    } else {
                        newInBuffer->time_stamp_estimate.tv_nsec = (mDLLatencyTime / 2) * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec;
                    }
                }

                mPreDownlinkEstTime.tv_sec = newInBuffer->time_stamp_estimate.tv_sec;
                mPreDownlinkEstTime.tv_nsec = newInBuffer->time_stamp_estimate.tv_nsec;

                mPreDownlinkQueueTime.tv_sec = BInputInfo->time_stamp_queued.tv_sec;
                mPreDownlinkQueueTime.tv_nsec = BInputInfo->time_stamp_queued.tv_nsec;
                if (EPLDebugEnable == true) {
                    ALOGD("downlink first time queue estimate time, sec %ld nsec %ld, inBufLength=%d,bRemainInfo=%d", mPreDownlinkEstTime.tv_sec, mPreDownlinkEstTime.tv_nsec, inBufLen, bRemainInfo);
                }
            }
            mPreDLBufLen = inBufLen;
        } else { //not the first DL buffer queue, continuos queue
            if (EPLDebugEnable == true) {
                ALOGD("downlink estimate time bRemainInfo=%d, pre tv_sec=%ld, pre nsec=%ld, mPreDLBufLen=%d", bRemainInfo, mPreDownlinkEstTime.tv_sec, mPreDownlinkEstTime.tv_nsec, mPreDLBufLen);
            }
            if (bRemainInfo) {
                newInBuffer->time_stamp_estimate.tv_sec = BInputInfo->time_stamp_predict.tv_sec;
                newInBuffer->time_stamp_estimate.tv_nsec = BInputInfo->time_stamp_predict.tv_nsec;

                //ALOGD("mDLLatencyTime=%d, predict sec= %ld, nsec=%ld, mPreDownlinkEstTime sec=%ld, nsec=%ld" , mDLLatencyTime,BInputInfo->time_stamp_predict.tv_sec, BInputInfo->time_stamp_predict.tv_nsec, mPreDownlinkEstTime.tv_sec, mPreDownlinkEstTime.tv_nsec);

                if ((TimeDifference(BInputInfo->time_stamp_predict, mPreDownlinkEstTime) > (mDLLatencyTime * (unsigned long long)1000000))) {
                    //two downlink queue interval is larger than hardware buffer latency time, this buffer is playing directly since no previous data in the hardware buffer
                    if (EPLDebugEnable == true) {
                        ALOGD("downlink late time predict sec= %ld, nsec=%ld, mPreDownlinkQueueTime sec=%ld, nsec=%ld", BInputInfo->time_stamp_predict.tv_sec, BInputInfo->time_stamp_predict.tv_nsec,
                              mPreDownlinkQueueTime.tv_sec, mPreDownlinkQueueTime.tv_nsec);
                    }
                }

            } else {
                struct timespec Esttstamp;
                //uint32_t ns = mPreDLBufLen*1000000000/sizeof(short)/1/mVoIPSampleRate;   //downlink is mono data
                //unsigned long long ns = ((mPreDLBufLen*1000000000)/2)/mVoIPSampleRate;   //downlink is mono data
                unsigned long long diffns = 0;
                unsigned long long ns = ((mPreDLBufLen * (unsigned long long)1000000) / 32); //downlink is mono data
                //ALOGD("downlink estimate time ns=%lld, pre tv_sec=%ld, pre nsec=%ld, mPreDLBufLen=%d", ns, mPreDownlinkEstTime.tv_sec, mPreDownlinkEstTime.tv_nsec, mPreDLBufLen);

                newInBuffer->time_stamp_estimate.tv_sec = BInputInfo->time_stamp_queued.tv_sec;
                newInBuffer->time_stamp_estimate.tv_nsec = BInputInfo->time_stamp_queued.tv_nsec;

                //when the latest DL buffer queue and this DL buffer queue interval longer than hardware buffer latency, the new buffer will play directly
                //and if the next buffer queue longer than half hardware buffer latency, it will also play directly
                //if((TimeDifference(BInputInfo->time_stamp_queued,mPreDownlinkQueueTime)>(mDLLatencyTime*1000000))||
                //( (TimeDifference(BInputInfo->time_stamp_queued,mPreDownlinkQueueTime)>((mDLLatencyTime/2)*1000000))))
                if (TimeDifference(BInputInfo->time_stamp_queued, mPreDownlinkQueueTime) > (mDLLatencyTime * (unsigned long long)1000000)) {

                    //two downlink queue interval is larger than hardware buffer latency time, this buffer is playing directly since no previous data in the hardware buffer
                    if (EPLDebugEnable == true) {
                        ALOGD("downlink late time queue sec= %ld, nsec=%ld, mPreDownlinkQueueTime sec=%ld, nsec=%ld", BInputInfo->time_stamp_queued.tv_sec, BInputInfo->time_stamp_queued.tv_nsec,
                              mPreDownlinkQueueTime.tv_sec, mPreDownlinkQueueTime.tv_nsec);
                    }

                } else {
                    if ((mDLLatencyTime / 2) * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec >= 1000000000) {
                        newInBuffer->time_stamp_estimate.tv_sec++;
                        newInBuffer->time_stamp_estimate.tv_nsec = (mDLLatencyTime / 2) * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec - 1000000000;
                    } else {
                        newInBuffer->time_stamp_estimate.tv_nsec = (mDLLatencyTime / 2) * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec;
                    }
                }
            }

            mPreDownlinkQueueTime.tv_sec = BInputInfo->time_stamp_queued.tv_sec;
            mPreDownlinkQueueTime.tv_nsec = BInputInfo->time_stamp_queued.tv_nsec;
#if 1   //use queue time + HW buffer latency time           
            mPreDownlinkEstTime.tv_sec = newInBuffer->time_stamp_estimate.tv_sec;
            mPreDownlinkEstTime.tv_nsec = newInBuffer->time_stamp_estimate.tv_nsec;
            if (EPLDebugEnable == true) {
                ALOGD("downlink queue estimate time, sec %ld nsec %ld, inBufLength=%d", mPreDownlinkEstTime.tv_sec, mPreDownlinkEstTime.tv_nsec, inBufLen);
            }
#else
            //predict by previos time
            Esttstamp.tv_sec = mPreDownlinkEstTime.tv_sec;
            if (mPreDownlinkEstTime.tv_nsec + ns >= 1000000000) {
                Esttstamp.tv_sec++;
                Esttstamp.tv_nsec = mPreDownlinkEstTime.tv_nsec + ns - 1000000000;
            } else {
                Esttstamp.tv_nsec = mPreDownlinkEstTime.tv_nsec + ns;
            }
            newInBuffer->time_stamp_estimate.tv_sec = Esttstamp.tv_sec;
            newInBuffer->time_stamp_estimate.tv_nsec = Esttstamp.tv_nsec;
            ALOGD("downlink estimate time, sec %ld nsec %ld, inBufLength=%d", Esttstamp.tv_sec, Esttstamp.tv_nsec, inBufLen);

            mPreDownlinkEstTime.tv_sec = Esttstamp.tv_sec;
            mPreDownlinkEstTime.tv_nsec = Esttstamp.tv_nsec;
#endif

            mPreDLBufLen = inBufLen;
        }
    }


    //    ALOGD("inBufLength=%d,mULInBufQLenTotal=%d, Qsize=%d",newInBuffer->BufLen,mULInBufQLenTotal,mULInBufferQ.size());
    if (dir == UPLINK) {
        mULInBufferQ.add(newInBuffer);
        mULInBufQLenTotal += inBufLen;
        //          ALOGD("SPELayer::Process, mULInBufQLenTotal=%d, size=%d",mULInBufQLenTotal,mULInBufferQ.size());
    } else {
        //queue to the downlink input buffer queue, downlink data channel is mono

        mDLInBufferQ.add(newInBuffer);
        mDLInBufQLenTotal += inBufLen;
        if (EPLDebugEnable == true) {
            ALOGD("AddtoInputBuffer, mDLInBufferQ.size()=%zu, mDLPreQnum=%d,mDLPreQLimit=%d,mFirstVoIPUplink=%d,mDLInBufQLenTotal=%d", mDLInBufferQ.size(), mDLPreQnum, mDLPreQLimit, mFirstVoIPUplink,
                  mDLInBufQLenTotal);
        }
        //also add to delay buffer queue

        newInBuffer->BufLen4Delay = inBufLen;
        newInBuffer->pRead4Delay = newInBuffer->pBufBase;
        newInBuffer->pWrite4Delay = newInBuffer->pBufBase;
        mDLDelayBufferQ.add(newInBuffer);
        mDLDelayBufQLenTotal += inBufLen;
        //          ALOGD("SPELayer::Process, mDLDelayBufQLenTotal=%d, size=%d",mDLDelayBufQLenTotal, mDLDelayBufferQ.size());

        if (bPreQueue) {

            //ALOGD("AddtoInputBuffer, mDLInBufferQ.size()=%d, mDLPreQnum=%d,mDLPreQLimit=%d,mFirstVoIPUplink=%d",mDLInBufferQ.size(),mDLPreQnum,mDLPreQLimit,mFirstVoIPUplink);
#if 0
            for (int i; i < mDLInBufferQ.size(); i++) {
                ALOGD("mDLInBufferQ i=%d, length=%d, %p, Sec=%d, NSec=%ld", i, mDLInBufferQ[i]->BufLen, mDLInBufferQ[i]->pBufBase,
                      mDLInBufferQ[i]->time_stamp_estimate.tv_sec, mDLInBufferQ[i]->time_stamp_estimate.tv_nsec);
            }
#endif

            if ((mDLPreQLimit) || (!mDLPreQLimit && mFirstVoIPUplink)) { //wait for uplink comes, only queue five buffer for reference
                while (mDLInBufferQ.size() > (size_t)mDLPreQnum) {
                    //ALOGD("free over queue, mDLInBufferQ size=%d,mDLInBufQLenTotal=%d, BufLen=%d, %p, %p",mDLInBufferQ.size(),mDLInBufQLenTotal,mDLInBufferQ[0]->BufLen,mDLInBufferQ[0]->pBufBase,mDLInBufferQ[0]);
                    mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
                    mDLInBufferQ.removeAt(0);
                }
            } else { //uplink interrupt starts, remove previous queue
                //for(int i; i<mDLInBufferQ.size(); i++)
                while (!mDLInBufferQ.isEmpty()) {
                    uint32_t tempSec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec;
                    unsigned long long tempNSec = mDLInBufferQ[0]->time_stamp_estimate.tv_nsec;
                    uint32_t tempsample = mDLInBufferQ[0]->BufLen / 2;
                    unsigned long long tempdeltaNSec = tempsample * (unsigned long long)1000000 / 16;
                    unsigned long long tempEndNSec = tempNSec + tempdeltaNSec;
                    unsigned long long tempFinalNSec = 0;
                    uint32_t tempFinalSec = tempSec;
                    if (EPLDebugEnable == true) {
                        ALOGD("check to move? %p, tempSec=%d, tempNSec=%lld, tempsample=%d", mDLInBufferQ[0]->pBufBase, tempSec, tempNSec, tempsample);
                    }
                    if (tempEndNSec > 1000000000) {
                        tempFinalNSec = tempEndNSec - 1000000000;
                        tempFinalSec = tempFinalSec + 1;
                    } else {
                        tempFinalNSec = tempEndNSec;
                    }
                    if (EPLDebugEnable == true) {
                        ALOGD("tempFinalSec=%d, tempFinalNSec=%llu, tempdeltaNSec=%llu", tempFinalSec, tempFinalNSec, tempdeltaNSec);
                    }
                    if (mUplinkIntrStartTime.tv_sec > (long)tempFinalSec) {
                        mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
                        mDLInBufferQ.removeAt(0);
                    } else if (mUplinkIntrStartTime.tv_sec == (long)tempFinalSec) {
                        if ((unsigned long long)mUplinkIntrStartTime.tv_nsec >= tempFinalNSec) {
                            mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
                            mDLInBufferQ.removeAt(0);
                        } else {
                            //remove previous data in this buffer queue, will do it in the prepare data?
                            if (EPLDebugEnable == true) {
                                ALOGD("remove DL pre queue finish 1");
                            }
                            break;
                        }
                    } else {
                        if (EPLDebugEnable == true) {
                            ALOGD("remove DL pre queue finish 2");
                        }
                        break;
                    }

                }
            }
#if 0
            for (int i; i < mDLDelayBufferQ.size(); i++) {
                ALOGD("mDLDelayBufferQ i=%d, length=%d, %p", i, mDLDelayBufferQ[i]->BufLen, mDLDelayBufferQ[i]->pBufBase);
            }
#endif
            if (mDLPreQLimit || (!mDLPreQLimit && mFirstVoIPUplink)) { //wait for uplink comes, only queue five buffer for
                while (mDLDelayBufferQ.size() > (uint32_t)mDLPreQnum) {
                    //ALOGD("free over queue, mDLDelayBufferQ size=%d,mDLDelayBufQLenTotal=%d, BufLen=%d, %p, %p",mDLDelayBufferQ.size(),mDLDelayBufQLenTotal,mDLDelayBufferQ[0]->BufLen4Delay,mDLDelayBufferQ[0]->pBufBase,mDLDelayBufferQ[0]);
                    mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                    free(mDLDelayBufferQ[0]->pBufBase);
                    delete mDLDelayBufferQ[0];
                    mDLDelayBufferQ.removeAt(0);
                    //ALOGD("free mDLDelayBufferQ over queue done");
                }
            } else { //uplink interrupt starts, remove previous queue
                while (!mDLDelayBufferQ.isEmpty()) {
                    uint32_t tempSec = mDLDelayBufferQ[0]->time_stamp_estimate.tv_sec;
                    unsigned long long tempNSec = mDLDelayBufferQ[0]->time_stamp_estimate.tv_nsec;
                    uint32_t tempsample = mDLDelayBufferQ[0]->BufLen / 2;
                    unsigned long long tempdeltaNSec = tempsample * (unsigned long long)1000000 / 16;
                    unsigned long long tempEndNSec = tempNSec + tempdeltaNSec;
                    unsigned long long tempFinalNSec = 0;
                    uint32_t tempFinalSec = tempSec;
                    if (EPLDebugEnable == true) {
                        ALOGD("mDLDelayBufferQ check to move? %p, tempSec=%d, tempNSec=%lld, tempsample=%d", mDLDelayBufferQ[0]->pBufBase, tempSec, tempNSec, tempsample);
                    }
                    if (tempEndNSec > 1000000000) {
                        tempFinalNSec = tempEndNSec - 1000000000;
                        tempFinalSec = tempFinalSec + 1;
                    } else {
                        tempFinalNSec = tempEndNSec;
                    }
                    if (EPLDebugEnable == true) {
                        ALOGD("tempFinalSec=%d, tempFinalNSec=%llu, tempdeltaNSec=%llu", tempFinalSec, tempFinalNSec, tempdeltaNSec);
                    }
                    if (mUplinkIntrStartTime.tv_sec > (long)tempFinalSec) {
                        mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen;
                        free(mDLDelayBufferQ[0]->pBufBase);
                        delete mDLDelayBufferQ[0];
                        mDLDelayBufferQ.removeAt(0);
                    } else if (mUplinkIntrStartTime.tv_sec == (long)tempFinalSec) {
                        if ((unsigned long long)mUplinkIntrStartTime.tv_nsec >= tempFinalNSec) {
                            mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen;
                            free(mDLDelayBufferQ[0]->pBufBase);
                            delete mDLDelayBufferQ[0];
                            mDLDelayBufferQ.removeAt(0);
                        } else {
                            //remove previous data in this buffer queue, will do it in the prepare data?
                            if (EPLDebugEnable == true) {
                                ALOGD("remove DL delay pre queue finish 1");
                            }
                            break;
                        }
                    } else {
                        if (EPLDebugEnable == true) {
                            ALOGD("remove DL delay pre queue finish 2");
                        }
                        break;
                    }

                }
            }

        }
        AL_SIGNAL(mBufMutex);
    }

    //AL_SIGNAL(mBufMutex);
    //AL_UNLOCK(mBufMutex);
}

void SPELayer::CompensateBuffer(size_t BufLength, struct timespec CompenStartTime) {
    //    ALOGD("SPELayer::Process, dir=%x, inBuf=%p,inBufLength=%d,mMode=%x,copysize=%d",dir,inBuf,inBufLength,mMode,copysize);

    ALOGD("CompensateBuffer, BufLength=%zu, sec=%lu, nsec=%lu", BufLength, CompenStartTime.tv_sec, CompenStartTime.tv_nsec);
    BufferInfo *newInBuffer = new BufferInfo;
    struct timespec tstamp;
    newInBuffer->pBufBase = (short *) malloc(BufLength);
    ASSERT(newInBuffer->pBufBase != NULL);

    //memset(newInBuffer->pBufBase, 0, BufLength);
    memset(newInBuffer->pBufBase, COMPENSATE_PCM_DATA, BufLength);

    tstamp = GetSystemTime();

    newInBuffer->BufLen = BufLength;
    newInBuffer->pRead = newInBuffer->pBufBase;
    newInBuffer->pWrite = newInBuffer->pBufBase;
    newInBuffer->time_stamp_queued = tstamp;
    newInBuffer->time_stamp_estimate = CompenStartTime;  //need to check the previous eatimate time
    //newInBuffer->time_stamp_process = {0};
    memset((void *) & (newInBuffer->time_stamp_process), 0, sizeof(newInBuffer->time_stamp_process));
    //queue to the downlink input buffer queue, downlink data channel is mono
    mDLInBufferQ.add(newInBuffer);
    mDLInBufQLenTotal += BufLength;
    //      ALOGD("CompensateBuffer, mDLInBufQLenTotal=%d, size=%d",mDLInBufQLenTotal,mDLInBufferQ.size());


    newInBuffer->BufLen4Delay = BufLength;
    newInBuffer->pRead4Delay = newInBuffer->pBufBase;
    newInBuffer->pWrite4Delay = newInBuffer->pBufBase;
    mDLDelayBufferQ.add(newInBuffer);
    mDLDelayBufQLenTotal += BufLength;
    //    ALOGD("CompensateBuffer, mDLDelayBufQLenTotal=%d, size=%d",mDLDelayBufQLenTotal, mDLDelayBufferQ.size());
    //if(!mNeedDelayLatency && mOutputStreamRunning)  //compensate happen after DL first time queue, and only count when downlink is playback
    if (!mFirstVoIPDownlink && mOutputStreamRunning) { //compensate happen after DL first time queue, and only count when downlink is playback
        mCompensatedBufferSize += BufLength;
        ALOGD("%s, mCompensatedBufferSize=%zu", __FUNCTION__, mCompensatedBufferSize);
        if ((mCompensatedBufferSize >= (size_t)160) && mPrepareProcessDataReady) { //5ms data
            ReSync();
        }
    }

}

bool    SPELayer::WaitforDownlinkData(void) {
    bool bRet = true;
    uint32_t timeoutMs = 40;    //timeout time 20ms

    if (mNormalModeVoIP) { //normal mode VoIP
        if (!mOutputStreamRunning) { //no output running, might be normal record
            timeoutMs = 0;
        } else { //output is running
            if (!mPrepareProcessDataReady) {
                timeoutMs = 0;
            } else
                //if(mNeedDelayLatency) //first time output queue
                if (mFirstVoIPDownlink) {
                    timeoutMs = 10;
                }
        }
    } else {    //In-communication mode VoIP
        if (mRoute == ROUTE_BT || mRoute == ROUTE_BT_NSEC_OFF_PATH) {
            timeoutMs = 30; // BTCVSD DL interrupt rate is 20ms
        } else if (!mOutputStreamRunning) { //no output running, process uplink data directly
            timeoutMs = 0;
        } else { //output is running
            if (!mPrepareProcessDataReady) {
                timeoutMs = 0;
            } else {
                //if(mNeedDelayLatency) //if no DL played before, it is the first time UL earlier case, wait DL for a short time
                if (mFirstVoIPDownlink) {
                    timeoutMs = 60;
                }
            }
        }
    }
    if (EPLDebugEnable == true) {
        ALOGD("WaitforDownlinkData pthread_cond_timedwait_relative_np start %d,mOutputStreamRunning=%d,mFirstVoIPDownlink=%d,mNormalModeVoIP=%d,mPrepareProcessDataReady=%d", timeoutMs, mOutputStreamRunning,
              mFirstVoIPDownlink,
              mNormalModeVoIP, mPrepareProcessDataReady);
    }
    if (timeoutMs != 0) {
        if (AL_WAIT_MS(mBufMutex, timeoutMs) != NO_ERROR) {
            ALOGD("WaitforDownlinkData pthread_cond_timedwait_relative_np timeout");
            bRet = false;
        }
    } else {
        bRet = false;
    }

    return bRet;
}

bool    SPELayer::InsertDownlinkData(void) {
    bool bRet = true;
    uint32_t timeoutMs = 3; //timeout time 1ms

    if (EPLDebugEnable == true) {
        ALOGD("InsertDownlinkData pthread_cond_timedwait_relative_np start %d,mOutputStreamRunning=%d,mFirstVoIPDownlink=%d,mNormalModeVoIP=%d,mPrepareProcessDataReady=%d", timeoutMs, mOutputStreamRunning,
              mFirstVoIPDownlink, mNormalModeVoIP, mPrepareProcessDataReady);
    }
    if (timeoutMs != 0) {
        if (AL_WAIT_MS(mBufMutex, timeoutMs) != NO_ERROR) {
            ALOGD("InsertDownlinkData pthread_cond_timedwait_relative_np timeout");
            bRet = false;
        }
    }

    return bRet;

}

//return NS
unsigned long long SPELayer::TimeStampDiff(BufferInfo *BufInfo1, BufferInfo *BufInfo2) {
    unsigned long long diffns = 0;
    struct timespec tstemp1 = BufInfo1->time_stamp_estimate;
    struct timespec tstemp2 = BufInfo2->time_stamp_estimate;

    //    ALOGD("TimeStampDiff time1 sec= %ld, nsec=%ld, time2 sec=%ld, nsec=%ld" ,tstemp1.tv_sec, tstemp1.tv_nsec, tstemp2.tv_sec, tstemp2.tv_nsec);

    if (tstemp1.tv_sec > tstemp2.tv_sec) {
        if (tstemp1.tv_nsec >= tstemp2.tv_nsec) {
            diffns = ((tstemp1.tv_sec - tstemp2.tv_sec) * (unsigned long long)1000000000) + tstemp1.tv_nsec - tstemp2.tv_nsec;
        } else {
            diffns = ((tstemp1.tv_sec - tstemp2.tv_sec - 1) * (unsigned long long)1000000000) + tstemp1.tv_nsec + 1000000000 - tstemp2.tv_nsec;
        }
    } else if (tstemp1.tv_sec == tstemp2.tv_sec) {
        if (tstemp1.tv_nsec >= tstemp2.tv_nsec) {
            diffns = tstemp1.tv_nsec - tstemp2.tv_nsec;
        } else {
            diffns = tstemp2.tv_nsec - tstemp1.tv_nsec;
        }
    } else {
        if (tstemp2.tv_nsec >= tstemp1.tv_nsec) {
            diffns = ((tstemp2.tv_sec - tstemp1.tv_sec) * (unsigned long long)1000000000) + tstemp2.tv_nsec - tstemp1.tv_nsec;
        } else {
            diffns = ((tstemp2.tv_sec - tstemp1.tv_sec - 1) * (unsigned long long)1000000000) + tstemp2.tv_nsec + 1000000000 - tstemp1.tv_nsec;
        }
    }
    if (EPLDebugEnable == true) {
        ALOGD("%s, time1 sec= %ld, nsec=%ld, time2 sec=%ld, nsec=%ld, diffns=%lld", __FUNCTION__, tstemp1.tv_sec, tstemp1.tv_nsec, tstemp2.tv_sec, tstemp2.tv_nsec, diffns);
    }
    return diffns;

}
unsigned long long SPELayer::TimeDifference(struct timespec time1, struct timespec time2) {
    unsigned long long diffns = 0;
    struct timespec tstemp1 = time1;
    struct timespec tstemp2 = time2;

    //    ALOGD("TimeStampDiff time1 sec= %ld, nsec=%ld, time2 sec=%ld, nsec=%ld" ,tstemp1.tv_sec, tstemp1.tv_nsec, tstemp2.tv_sec, tstemp2.tv_nsec);

    if (tstemp1.tv_sec > tstemp2.tv_sec) {
        if (tstemp1.tv_nsec >= tstemp2.tv_nsec) {
            diffns = ((tstemp1.tv_sec - tstemp2.tv_sec) * (unsigned long long)1000000000) + tstemp1.tv_nsec - tstemp2.tv_nsec;
        } else {
            diffns = ((tstemp1.tv_sec - tstemp2.tv_sec - 1) * (unsigned long long)1000000000) + tstemp1.tv_nsec + 1000000000 - tstemp2.tv_nsec;
        }
    } else if (tstemp1.tv_sec == tstemp2.tv_sec) {
        if (tstemp1.tv_nsec >= tstemp2.tv_nsec) {
            diffns = tstemp1.tv_nsec - tstemp2.tv_nsec;
        } else {
            diffns = tstemp2.tv_nsec - tstemp1.tv_nsec;
        }
    } else {
        if (tstemp2.tv_nsec >= tstemp1.tv_nsec) {
            diffns = ((tstemp2.tv_sec - tstemp1.tv_sec) * (unsigned long long)1000000000) + tstemp2.tv_nsec - tstemp1.tv_nsec;
        } else {
            diffns = ((tstemp2.tv_sec - tstemp1.tv_sec - 1) * (unsigned long long)1000000000) + tstemp2.tv_nsec + 1000000000 - tstemp1.tv_nsec;
        }
    }
    //    ALOGD("TimeDifference time1 sec= %ld, nsec=%ld, time2 sec=%ld, nsec=%ld, diffns=%lld" ,tstemp1.tv_sec, tstemp1.tv_nsec, tstemp2.tv_sec, tstemp2.tv_nsec,diffns);
    return diffns;
}

bool SPELayer::TimeCompare(struct timespec time1, struct timespec time2) {
    bool bRet = 0;

    if (time1.tv_sec > time2.tv_sec) {
        bRet = true;
    } else if (time1.tv_sec == time2.tv_sec) {
        if (time1.tv_nsec >= time2.tv_nsec) {
            bRet = true;
        } else {
            bRet = false;
        }
    } else {
        bRet = false;
    }
    return bRet;
}

//endtime = false => compare the start time
//endtime = true => compare the end time
bool SPELayer::TimeStampCompare(BufferInfo *BufInfo1, BufferInfo *BufInfo2, bool Endtime) {
    bool bRet = 0;
    struct timespec tstemp1 = BufInfo1->time_stamp_estimate;
    struct timespec tstemp2 = BufInfo2->time_stamp_estimate;
    struct timespec tstemp2_end = tstemp2;
    int inSample = BufInfo2->BufLen / 2; //mono data since BufInfo2 is downlink data
    if (EPLDebugEnable == true) {
        ALOGD("%s, time1 sec= %ld, nsec=%ld, time2 sec=%ld, nsec=%ld, Endtime=%d, inSample=%d", __FUNCTION__, tstemp1.tv_sec, tstemp1.tv_nsec, tstemp2.tv_sec, tstemp2.tv_nsec, Endtime, inSample);
    }
    if (Endtime == 0) {
        if (tstemp1.tv_sec > tstemp2.tv_sec) {
            bRet = true;
        } else if (tstemp1.tv_sec == tstemp2.tv_sec) {
            if (tstemp1.tv_nsec >= tstemp2.tv_nsec) {
                bRet = true;
            } else {
                bRet = false;
            }
        } else {
            bRet = false;
        }
    } else {
        if (tstemp1.tv_sec > tstemp2.tv_sec) {
            bRet = true;
        } else if (tstemp1.tv_sec == tstemp2.tv_sec) {
            if (tstemp1.tv_nsec >= tstemp2.tv_nsec) {
                bRet = true;
            } else {
                bRet = false;
                return bRet;
            }
        } else {
            bRet = false;
            return bRet;
        }

        unsigned long long ns = ((inSample * (unsigned long long)1000000) / 16); //sample rate is 16000

        if (tstemp2.tv_nsec + ns >= 1000000000) {
            tstemp2.tv_sec++;
            tstemp2.tv_nsec = tstemp2.tv_nsec + ns - 1000000000;
        } else {
            tstemp2.tv_nsec = tstemp2.tv_nsec + ns;
        }
        if (EPLDebugEnable == true) {
            ALOGD("%s, tstemp2 sec=%ld, nsec=%ld, ns=%lld", __FUNCTION__, tstemp2.tv_sec, tstemp2.tv_nsec, ns);
        }
        if (tstemp1.tv_sec > tstemp2.tv_sec) {
            bRet = true;
        } else if (tstemp1.tv_sec == tstemp2.tv_sec) {
            if (tstemp1.tv_nsec >= tstemp2.tv_nsec) {
                bRet = true;
            } else {
                bRet = false;
            }
        } else {
            bRet = false;
        }
    }

    ALOGD("%s, bRet=%d", __FUNCTION__, bRet);
    return bRet;
}

void SPELayer::BypassDLBuffer(void) {
    BufferInfo *newInBuffer = new BufferInfo;
    struct timespec tstamp;
    int BufLength = mSPEProcessBufSize / 2;
    newInBuffer->pBufBase = (short *) malloc(BufLength);
    ASSERT(newInBuffer->pBufBase != NULL);
    //ALOGD("PrepareProcessData %p", newInBuffer->pBufBase);
    //memset(newInBuffer->pBufBase, 0, BufLength);
    memset(newInBuffer->pBufBase, BYPASS_DL_PCM_DATA, BufLength);

    tstamp = GetSystemTime();
    newInBuffer->BufLen = BufLength;
    newInBuffer->pRead = newInBuffer->pBufBase;
    newInBuffer->pWrite = newInBuffer->pBufBase;
    newInBuffer->time_stamp_queued = tstamp;
    memset((void *) & (newInBuffer->time_stamp_estimate), 0, sizeof(newInBuffer->time_stamp_estimate));
    memset((void *) & (newInBuffer->time_stamp_process), 0, sizeof(newInBuffer->time_stamp_process));

    //newInBuffer->time_stamp_estimate = {0};
    //newInBuffer->time_stamp_process = {0};
    newInBuffer->DLfirstBuf = false;

    //queue to the begging of the downlink input buffer queue, downlink data channel is mono
    //mDLInBufferQ.add(newInBuffer);
    mDLInBufferQ.push_front(newInBuffer);
    if (EPLDebugEnable == true) {
        ALOGD("BypassDLBuffer, size %zu, %p", mDLInBufferQ.size(), mDLInBufferQ[0]->pBufBase);
    }
    //mDLInBufferQ.insertVectorAt(newInBuffer,0);
    mDLInBufQLenTotal += BufLength;
    //      ALOGD("BypassDLBuffer, mDLInBufQLenTotal=%d, size=%d",mDLInBufQLenTotal,mDLInBufferQ.size());


    newInBuffer->BufLen4Delay = BufLength;
    newInBuffer->pRead4Delay = newInBuffer->pBufBase;
    newInBuffer->pWrite4Delay = newInBuffer->pBufBase;
    //mDLDelayBufferQ.add(newInBuffer);
    mDLDelayBufferQ.push_front(newInBuffer);
    mDLDelayBufQLenTotal += BufLength;
}

bool SPELayer::PrepareProcessData() {
    bool bRet = false;
    if (mPrepareProcessDataReady) {
        bRet = true;
        return bRet;
    }

    ALOGD("%s+", __FUNCTION__);
    if (mDLNewStart || (DLdataPrepareCount > 0)) { //the first queue downlink buffer is not ready to play yet, only need to process uplink data, so add zero data on downlink
        //compensate data to DL and DL delay as zero data for first uplink buffer process
        ALOGD("%s, DL data is not ready yet, size %zu", __FUNCTION__, mDLInBufferQ.size());

        BypassDLBuffer();
        if (mDLNewStart) {
            CalPrepareCount(); // 2 * 4;
        } else {
            DLdataPrepareCount--;
            ALOGD("prepare data DLdataPrepareCount=%d put infront of", DLdataPrepareCount);
        }
        bRet = false;
    } else { //when all data is ready, check the estimate time to let DL/UL could start together.
        if (mDLInBufferQ.isEmpty() || mDLDelayBufferQ.isEmpty()) {
            ALOGD("no downlink data, no need to sync");
            return bRet;
        }

        if (DLdataPrepareCount > 0) {
            DLdataPrepareCount--;
            ALOGD("prepare data DLdataPrepareCount=%d", DLdataPrepareCount);
            return bRet;
        }
#if 1
        if (TimeCompare(mDownlinkIntrStartTime, mULInBufferQ[0]->time_stamp_estimate)) { //if downlink data is later than uplink data, no need to process AEC and not need to do the sync
            //compensate data to DL and DL delay as zero data for first uplink buffer process
            ALOGD("%s, downlink data is not ready yet, no need to sync, size %zu, %p", __FUNCTION__, mDLInBufferQ.size(), mDLInBufferQ[0]->pBufBase);
            BypassDLBuffer();
            return bRet;
        }
#endif
        for (size_t i = 0; i < mDLInBufferQ.size(); i++) {
            ALOGD("mDLInBufferQ i=%zu, length=%d, %p, Sec=%lu, NSec=%lu", i, mDLInBufferQ[i]->BufLen, mDLInBufferQ[i]->pBufBase,
                  mDLInBufferQ[i]->time_stamp_estimate.tv_sec, mDLInBufferQ[i]->time_stamp_estimate.tv_nsec);
        }

        bool bULlate = false;
        //struct timespec deltatime;
        int deltaSec = 0;
        unsigned long long deltaNSec = 0;
        ALOGD("%s, time_stamp_estimate, mULInBufferQ[0].sec=%ld, mULInBufferQ[0].nsec=%ld", __FUNCTION__, mULInBufferQ[0]->time_stamp_estimate.tv_sec, mULInBufferQ[0]->time_stamp_estimate.tv_nsec);
        ALOGD("%s, time_stamp_estimate, mDLInBufferQ[0].sec=%ld, mDLInBufferQ[0].nsec=%ld", __FUNCTION__, mDLInBufferQ[0]->time_stamp_estimate.tv_sec, mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
        if (TimeStampCompare(mULInBufferQ[0], mDLInBufferQ[0], 0)) { //drop downlink and downlink delay data
            //remove previous queue downlink data, to match the nearlist DL buffer timestamp as uplink one
            while ((!mDLInBufferQ.isEmpty()) && (TimeStampCompare(mULInBufferQ[0], mDLInBufferQ[0], 1))) {
                //drop DL data
                uint32_t droplength = mDLInBufferQ[0]->BufLen;
                mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
                mDLInBufferQ.removeAt(0);

                //drop DL delay data
                while (droplength > 0) {
                    ALOGD("%s, 1 drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%zu, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", __FUNCTION__, droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                    if (droplength < (uint32_t)mDLDelayBufferQ[0]->BufLen4Delay) {
                        ALOGD("%s, 1 mDLDelayBufferQ[0]->pRead=%p", __FUNCTION__, mDLDelayBufferQ[0]->pRead);
                        mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                        mDLDelayBufQLenTotal -= droplength;
                        mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                        droplength = 0;
                        ALOGD("%s, 1 after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", __FUNCTION__, mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                    } else {
                        droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                        mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                        free(mDLDelayBufferQ[0]->pBufBase);
                        delete mDLDelayBufferQ[0];
                        mDLDelayBufferQ.removeAt(0);
                    }
                }

                if (mDLInBufferQ.isEmpty()) {
                    ALOGD("%s, something wrong? no DL buffer data, sync again", __FUNCTION__);
                    return false;
                    //break;
                }
            }

            if (TimeStampCompare(mULInBufferQ[0], mDLInBufferQ[0], 0)) {
                ALOGD("%s, calculate drop downlink data time", __FUNCTION__);
                bULlate = true; //calculate drop downlink data time
                if (mULInBufferQ[0]->time_stamp_estimate.tv_nsec >= mDLInBufferQ[0]->time_stamp_estimate.tv_nsec) {
                    deltaSec = mULInBufferQ[0]->time_stamp_estimate.tv_sec - mDLInBufferQ[0]->time_stamp_estimate.tv_sec;
                    deltaNSec = mULInBufferQ[0]->time_stamp_estimate.tv_nsec - mDLInBufferQ[0]->time_stamp_estimate.tv_nsec;
                } else {
                    deltaSec = mULInBufferQ[0]->time_stamp_estimate.tv_sec - mDLInBufferQ[0]->time_stamp_estimate.tv_sec - 1;
                    deltaNSec = 1000000000 + mULInBufferQ[0]->time_stamp_estimate.tv_nsec - mDLInBufferQ[0]->time_stamp_estimate.tv_nsec;
                }
            } else {
                bULlate = false;
                ALOGD("%s, actually uplink is earlier!!! need compensate downlink as zero", __FUNCTION__);
                if (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec >= mULInBufferQ[0]->time_stamp_estimate.tv_nsec) {
                    deltaSec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec - mULInBufferQ[0]->time_stamp_estimate.tv_sec;
                    deltaNSec = mDLInBufferQ[0]->time_stamp_estimate.tv_nsec - mULInBufferQ[0]->time_stamp_estimate.tv_nsec;
                } else {
                    deltaSec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec - mULInBufferQ[0]->time_stamp_estimate.tv_sec - 1;
                    deltaNSec = 1000000000 + mDLInBufferQ[0]->time_stamp_estimate.tv_nsec - mULInBufferQ[0]->time_stamp_estimate.tv_nsec;
                }
            }

        } else { //UL time is earlier
            //need compensate downlink data as zero
            bULlate = false;
            ALOGD("%s, 2 time_stamp_estimate,mDLInBufferQ[0].nsec = %ld, mULInBufferQ[0].nsec=%ld", __FUNCTION__, mDLInBufferQ[0]->time_stamp_estimate.tv_nsec, mULInBufferQ[0]->time_stamp_estimate.tv_nsec);
            ALOGD("%s, 2 time_stamp_estimate,mDLInBufferQ[0].sec = %ld, mULInBufferQ[0].sec=%ld", __FUNCTION__, mDLInBufferQ[0]->time_stamp_estimate.tv_sec, mULInBufferQ[0]->time_stamp_estimate.tv_sec);

            if (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec >= mULInBufferQ[0]->time_stamp_estimate.tv_nsec) {
                deltaSec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec - mULInBufferQ[0]->time_stamp_estimate.tv_sec;
                deltaNSec = mDLInBufferQ[0]->time_stamp_estimate.tv_nsec - mULInBufferQ[0]->time_stamp_estimate.tv_nsec;
            } else {
                deltaSec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec - mULInBufferQ[0]->time_stamp_estimate.tv_sec - 1;
                deltaNSec = 1000000000 + mDLInBufferQ[0]->time_stamp_estimate.tv_nsec - mULInBufferQ[0]->time_stamp_estimate.tv_nsec;
            }

        }
        ALOGD("%s, bULlate %d, deltaSec=%d, deltaNSec=%lld", __FUNCTION__, bULlate, deltaSec, deltaNSec);

        if (deltaSec < 0) {
            ALOGW("%s, deltaSec < 0? sync again", __FUNCTION__);
            return false;
        }
        unsigned long long diffnsec = deltaSec * 1000000000ULL + deltaNSec;
        //uint32_t diffSample = mVoIPSampleRate*diffnsec/1000000000;
        uint32_t diffSample = 16 * diffnsec / 1000000;
        uint32_t diffBufLength = diffSample * sizeof(short);

        ALOGD("%s, diffnsec %llu, diffSample=%d, diffBufLength=%d", __FUNCTION__, diffnsec, diffSample, diffBufLength);
#if 1
        while (diffBufLength > 0) {
            if (bULlate == true) { //drop DL data and DL delay data
                if (mDLInBufferQ.isEmpty() || mDLDelayBufferQ.isEmpty()) {
                    ALOGW("%s, no mDLInBufferQ data, something wrong? sync again", __FUNCTION__);
                    return false;
                    //break;
                }
                if ((diffBufLength > (uint32_t)mDLInBufQLenTotal) || (diffBufLength > (uint32_t)mDLDelayBufQLenTotal)) {
                    //time diff more than DL preQ data
                    ALOGW("%s, something wrong happened?, sync again", __FUNCTION__);
                    diffBufLength = mDLInBufQLenTotal;
                    return false;
                    //break;
                }
                ALOGD("%s, drop DL data diffBufLength=%d, mDLInBufferQ.size()=%zu, mDLInBufferQ[0]->BufLen=%d!!!", __FUNCTION__, diffBufLength, mDLInBufferQ.size(), mDLInBufferQ[0]->BufLen);
                if (diffBufLength >= (uint32_t)mDLInBufferQ[0]->BufLen) {
                    //drop DL data
                    uint32_t droplength = mDLInBufferQ[0]->BufLen;
                    diffBufLength -= mDLInBufferQ[0]->BufLen;
                    mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
                    mDLInBufferQ.removeAt(0);

                    //drop DL delay data
                    while (droplength > 0) {
                        ALOGD("%s, drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%zu, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", __FUNCTION__, droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                        if (droplength < (uint32_t)mDLDelayBufferQ[0]->BufLen4Delay) {
                            ALOGD("%s, mDLDelayBufferQ[0]->pRead=%p", __FUNCTION__, mDLDelayBufferQ[0]->pRead);
                            mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                            mDLDelayBufQLenTotal -= droplength;
                            mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                            droplength = 0;
                            ALOGD("%s, after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", __FUNCTION__, mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                        } else {
                            droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                            mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                            free(mDLDelayBufferQ[0]->pBufBase);
                            delete mDLDelayBufferQ[0];
                            mDLDelayBufferQ.removeAt(0);
                        }
                    }
                } else {
                    //                    ALOGW("PrepareProcessData !!! break for test");
                    //                    break;
                    //drop DL data
                    ALOGD("%s, mDLInBufferQ[0]->pRead=%p , mDLInBufferQ[0]->BufLen=%d, sec %ld, nsec %ld", __FUNCTION__, mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, mDLInBufferQ[0]->time_stamp_estimate.tv_sec,
                          mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
                    uint32_t droplength = diffBufLength;
                    mDLInBufferQ[0]->BufLen -= diffBufLength;   //record the buffer you consumed
                    mDLInBufQLenTotal -= diffBufLength;
                    mDLInBufferQ[0]->pRead = mDLInBufferQ[0]->pRead + diffBufLength / 2;

                    //unsigned long long updateDLnsecdiff = (diffBufLength/2)*1000000000/mVoIPSampleRate;

                    uint32_t adjustsample = diffBufLength / 2;
                    unsigned long long updateDLnsecdiff = 0;
                    updateDLnsecdiff = (adjustsample * (unsigned long long)1000000) / 16;

                    mDLInBufferQ[0]->time_stamp_estimate.tv_sec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec + (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + updateDLnsecdiff) / 1000000000;
                    mDLInBufferQ[0]->time_stamp_estimate.tv_nsec = (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + updateDLnsecdiff) % 1000000000;

                    ALOGD("%s, after mDLInBufferQ[0]->pRead=%p, mDLInBufferQ[0]->BufLen=%d, updatensecdiff=%lld, sec=%ld, nsec=%ld", __FUNCTION__, mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, updateDLnsecdiff,
                          mDLInBufferQ[0]->time_stamp_estimate.tv_sec, mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
                    diffBufLength = 0;

                    //drop DL delay data
                    while (droplength > 0) {
                        ALOGD("%s, drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%zu, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", __FUNCTION__, droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                        if (droplength < (uint32_t)mDLDelayBufferQ[0]->BufLen4Delay) {
                            ALOGD("%s, mDLDelayBufferQ[0]->pRead=%p", __FUNCTION__, mDLDelayBufferQ[0]->pRead4Delay);
                            mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                            mDLDelayBufQLenTotal -= droplength;
                            mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                            droplength = 0;
                            ALOGD("%s, after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", __FUNCTION__, mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                        } else {
                            droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                            mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                            free(mDLDelayBufferQ[0]->pBufBase);
                            delete mDLDelayBufferQ[0];
                            mDLDelayBufferQ.removeAt(0);
                        }
                    }

                }
            } else { //add DL zero data at the beginning
                BufferInfo *newInBuffer = new BufferInfo;
                struct timespec tstamp;
                uint32_t BufLength = diffBufLength;
                newInBuffer->pBufBase = (short *) malloc(BufLength);
                ASSERT(newInBuffer->pBufBase != NULL);
                ALOGD("%s, data is ready but need adjust", __FUNCTION__);
                //memset(newInBuffer->pBufBase, 0, BufLength);
                memset(newInBuffer->pBufBase, PREPARE_PROCESS_PCM_DATA, BufLength);

                tstamp = GetSystemTime();
                newInBuffer->BufLen = BufLength;
                newInBuffer->pRead = newInBuffer->pBufBase;
                newInBuffer->pWrite = newInBuffer->pBufBase;
                newInBuffer->time_stamp_queued = tstamp;
                newInBuffer->time_stamp_estimate = tstamp;
                memset((void *) & (newInBuffer->time_stamp_process), 0, sizeof(newInBuffer->time_stamp_process));

                //queue to the begging of the downlink input buffer queue, downlink data channel is mono
                //mDLInBufferQ.add(newInBuffer);
                mDLInBufferQ.push_front(newInBuffer);
                //ALOGD("PrepareProcessData, size %d, %p",mDLInBufferQ.size(), mDLInBufferQ[0]->pBufBase);
                //mDLInBufferQ.insertVectorAt(newInBuffer,0);
                mDLInBufQLenTotal += BufLength;
                //      ALOGD("CompensateBuffer, mDLInBufQLenTotal=%d, size=%d",mDLInBufQLenTotal,mDLInBufferQ.size());


                newInBuffer->BufLen4Delay = BufLength;
                newInBuffer->pRead4Delay = newInBuffer->pBufBase;
                newInBuffer->pWrite4Delay = newInBuffer->pBufBase;
                //mDLDelayBufferQ.add(newInBuffer);
                mDLDelayBufferQ.push_front(newInBuffer);
                mDLDelayBufQLenTotal += BufLength;
                diffBufLength = 0;
            }
        }
#endif
        ALOGD("%s, finish, mDLInBufferQ.size = %zu, mDLInBufQLenTotal=%d, mULInBufferQ.size = %zu, mULInBufQLenTotal=%d", __FUNCTION__, mDLInBufferQ.size(), mDLInBufQLenTotal, mULInBufferQ.size(),
              mULInBufQLenTotal); //going to start process
        ALOGD("%s, finish, mDLInBufferQ[0]->pRead=%p , mDLInBufferQ[0]->BufLen=%d, sec = %ld, nsec = %ld"
              , __FUNCTION__, mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, mDLInBufferQ[0]->time_stamp_estimate.tv_sec, mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);

        ALOGD("%s, finish, mULInBufferQ[0]->pRead=%p , mULInBufferQ[0]->BufLen=%d, sec = %ld, nsec = %ld"
              , __FUNCTION__, mULInBufferQ[0]->pRead, mULInBufferQ[0]->BufLen, mULInBufferQ[0]->time_stamp_estimate.tv_sec, mULInBufferQ[0]->time_stamp_estimate.tv_nsec);

        mPrepareProcessDataReady = true;
        bRet = true;

        if (mNeedJitterBuffer && (mJitterSampleCount != 0)) { //the first DL buffer, add the jitter buffer at the first downlink buffer queue and downlink delay buffer queue
            mNeedJitterBuffer = false;
            BufferInfo *newJitterBuffer = new BufferInfo;
            newJitterBuffer->pBufBase = (short *) malloc(mJitterSampleCount * sizeof(short)); //one channel, 16bits
            ASSERT(newJitterBuffer->pBufBase != NULL);
            newJitterBuffer->BufLen = mJitterSampleCount * sizeof(short);
            newJitterBuffer->pRead = newJitterBuffer->pBufBase;
            newJitterBuffer->pWrite = newJitterBuffer->pBufBase;
            newJitterBuffer->BufLen4Delay = mJitterSampleCount * sizeof(short);
            newJitterBuffer->pRead4Delay = newJitterBuffer->pBufBase;
            newJitterBuffer->pWrite4Delay = newJitterBuffer->pBufBase;
            memset(newJitterBuffer->pBufBase, 0, newJitterBuffer->BufLen);
            //newJitterBuffer->time_stamp_queued = tstamp;
            memset((void *) & (newJitterBuffer->time_stamp_process), 0, sizeof(newJitterBuffer->time_stamp_process));
            mDLInBufferQ.push_front(newJitterBuffer);
            mDLInBufQLenTotal += newJitterBuffer->BufLen;

            mDLDelayBufferQ.push_front(newJitterBuffer);
            mDLDelayBufQLenTotal += newJitterBuffer->BufLen;
            ALOGD("add jitter buffer,newDelayBuffer->BufLen=%d, size=%zu, mJitterSampleCount=%d, pBufBase=%p", newJitterBuffer->BufLen, mDLInBufferQ.size(), mJitterSampleCount, newJitterBuffer->pBufBase);
        }

        if (mNeedDelayLatency && (mLatencySampleCount != 0)) { //the first DL buffer, add the delay time buffer as first delay buffer queue
            //mNeedDelayLatency = false;
            ALOGD("%s, adjust downlink data mLatencyDir=%d,mLatencySampleCount=%d", __FUNCTION__, mLatencyDir, mLatencySampleCount);
            if (mLatencyDir == true) {
                BufferInfo *newDelayBuffer = new BufferInfo;
                newDelayBuffer->pBufBase = (short *) malloc(mLatencySampleCount * sizeof(short)); //one channel, 16bits
                ASSERT(newDelayBuffer->pBufBase != NULL);
                newDelayBuffer->BufLen = mLatencySampleCount * sizeof(short);
                newDelayBuffer->pRead = newDelayBuffer->pBufBase;
                newDelayBuffer->pWrite = newDelayBuffer->pBufBase;
                newDelayBuffer->BufLen4Delay = mLatencySampleCount * sizeof(short);
                newDelayBuffer->pRead4Delay = newDelayBuffer->pBufBase;
                newDelayBuffer->pWrite4Delay = newDelayBuffer->pBufBase;
                memset(newDelayBuffer->pBufBase, 0, newDelayBuffer->BufLen);
                //newDelayBuffer->time_stamp_queued = tstamp;
                //newDelayBuffer->time_stamp_process = {0};
                memset((void *) & (newDelayBuffer->time_stamp_process), 0, sizeof(newDelayBuffer->time_stamp_process));
                mDLDelayBufferQ.push_front(newDelayBuffer);
                mDLDelayBufQLenTotal += newDelayBuffer->BufLen;
                ALOGD("add delay latency buffer, newDelayBuffer->BufLen=%d, size=%zu, mLatencySampleCount=%d, pBufBase=%p", newDelayBuffer->BufLen, mDLDelayBufferQ.size(), mLatencySampleCount,
                      newDelayBuffer->pBufBase);
            } else {
                uint32_t diffLatencyBufLength = mLatencySampleCount * sizeof(short);
                while (diffLatencyBufLength > 0) {
                    if (mDLInBufferQ.isEmpty() || mDLDelayBufferQ.isEmpty()) {
                        ALOGW("adjust downlink data no mDLInBufferQ data");
                        break;
                    }
                    if ((diffLatencyBufLength > (uint32_t)mDLInBufQLenTotal) || (diffLatencyBufLength > (uint32_t)mDLDelayBufQLenTotal)) {
                        //time diff more than DL preQ data
                        ALOGW("adjust downlink data something wrong happened?");
                        diffLatencyBufLength = mDLInBufQLenTotal;
                        //break;
                    }
                    ALOGD("adjust downlink data drop DL data diffBufLength=%d, mDLInBufferQ.size()=%zu, mDLInBufferQ[0]->BufLen=%d!!!", diffBufLength, mDLInBufferQ.size(), mDLInBufferQ[0]->BufLen);
                    if (diffLatencyBufLength >= (uint32_t)mDLInBufferQ[0]->BufLen) {
                        //drop DL data
                        uint32_t droplength = mDLInBufferQ[0]->BufLen;
                        diffLatencyBufLength -= mDLInBufferQ[0]->BufLen;
                        mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
                        mDLInBufferQ.removeAt(0);

                        //drop DL delay data
                        while (droplength > 0) {
                            ALOGD("adjust downlink data drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%zu, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                            if (droplength < (uint32_t)mDLDelayBufferQ[0]->BufLen4Delay) {
                                ALOGD("adjust downlink data mDLDelayBufferQ[0]->pRead=%p", mDLDelayBufferQ[0]->pRead);
                                mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                                mDLDelayBufQLenTotal -= droplength;
                                mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                                droplength = 0;
                                ALOGD("adjust downlink data after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                            } else {
                                droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                                mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                                free(mDLDelayBufferQ[0]->pBufBase);
                                delete mDLDelayBufferQ[0];
                                mDLDelayBufferQ.removeAt(0);
                            }
                        }
                    } else {

                        //drop DL data
                        ALOGD("adjust downlink data mDLInBufferQ[0]->pRead=%p , mDLInBufferQ[0]->BufLen=%d, sec %ld, nsec %ld", mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, mDLInBufferQ[0]->time_stamp_estimate.tv_sec,
                              mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
                        uint32_t droplength = diffLatencyBufLength;
                        mDLInBufferQ[0]->BufLen -= diffLatencyBufLength;   //record the buffer you consumed
                        mDLInBufQLenTotal -= diffLatencyBufLength;
                        mDLInBufferQ[0]->pRead = mDLInBufferQ[0]->pRead + diffLatencyBufLength / 2;

                        //unsigned long long updateDLnsecdiff = (diffBufLength/2)*1000000000/mVoIPSampleRate;

                        uint32_t adjustsample = diffLatencyBufLength / 2;
                        unsigned long long updateDLnsecdiff = 0;
                        updateDLnsecdiff = (adjustsample * (unsigned long long)1000000) / 16;

                        mDLInBufferQ[0]->time_stamp_estimate.tv_sec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec + (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + updateDLnsecdiff) / 1000000000;
                        mDLInBufferQ[0]->time_stamp_estimate.tv_nsec = (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + updateDLnsecdiff) % 1000000000;

                        ALOGD("adjust downlink data after mDLInBufferQ[0]->pRead=%p, mDLInBufferQ[0]->BufLen=%d, updatensecdiff=%lld, sec=%ld, nsec=%ld", mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, updateDLnsecdiff,
                              mDLInBufferQ[0]->time_stamp_estimate.tv_sec, mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
                        diffLatencyBufLength = 0;

                        //drop DL delay data
                        while (droplength > 0) {
                            ALOGD("adjust downlink data drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%zu, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                            if (droplength < (uint32_t)mDLDelayBufferQ[0]->BufLen4Delay) {
                                ALOGD("adjust downlink data mDLDelayBufferQ[0]->pRead=%p", mDLDelayBufferQ[0]->pRead4Delay);
                                mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                                mDLDelayBufQLenTotal -= droplength;
                                mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                                droplength = 0;
                                ALOGD("adjust downlink data after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                            } else {
                                droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                                mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                                free(mDLDelayBufferQ[0]->pBufBase);
                                delete mDLDelayBufferQ[0];
                                mDLDelayBufferQ.removeAt(0);
                            }
                        }

                    }
                }
            }
        }

#if 0
        for (int i; i < mDLDelayBufferQ.size(); i++) {
            ALOGD("PrepareProcessData mDLDelayBufferQ i=%d, length=%d, %p", i, mDLDelayBufferQ[i]->BufLen, mDLDelayBufferQ[i]->pBufBase);
        }
#endif
    }
    return bRet;
}

int SPELayer::Process_Record(short *inBuf, int  inBufLength) {
    int retSize = inBufLength;
    //pthread_mutex_lock(&mBufMutex );
    //ALOGD("Process_Record, SPERecBufSize=%d,inBufLength=%d,mULInBufQLenTotal=%d, Insize=%d,Outsize=%d",SPERecBufSize,inBufLength,mULInBufQLenTotal,mULInBufferQ.size(),mULOutBufferQ.size());
    if ((mULInBufQLenTotal < mSPEProcessBufSize) && (mULOutBufferQ.size() == 0)) { //not enough UL buffer for process, and no processed uplink output buffer
        ALOGD("%s,going memset 0 inBuf=%p,inBufLength=%d", __FUNCTION__, inBuf, inBufLength);
        memset(inBuf, 0, inBufLength); //clear the buffer data as zero
        //AL_UNLOCK(mBufMutex);
        retSize = 0;
        return retSize; //return the processed buffer size
    }

    //   ALOGD("SPELayer::Process, enough mULInBufQLenTotal buffer,size=%d",mULInBufferQ.size());
    //process input data in the buffer queue
    while (mULInBufQLenTotal >= mSPEProcessBufSize) {
        int tmpSPEProcessBufSize = mSPEProcessBufSize;
        int indexIn = 0;
        int tempULIncopysize = mULInBufferQ[0]->BufLen >> 2;

        //      ALOGD("SPELayer::Process, mULInBufQLenTotal=%d, SPERecBufSize=%d,tempULIncopysize=%d",mULInBufQLenTotal,mSPEProcessBufSize,tempULIncopysize);
        //fill in the data to the process buffer
        while (tmpSPEProcessBufSize) {
            if (mULInBufferQ.isEmpty()) {
                ALOGD("%s, uplink input buffer queue is empty, something wrong!!", __FUNCTION__);
                mError = true;
                break;
            }

            //        ALOGD("SPELayer indexIn=%d, tmpSPERecBufSize=%d, mULInBufQLenTotal=%d,mULInBufferQ[0]->pRead=%p,mULInBufferQ[0]->pBufBase=%p,mULInBufferQ[0]->BufLen=%d,tempULIncopysize=%d",indexIn,tmpSPERecBufSize,mULInBufQLenTotal,mULInBufferQ[0]->pRead,mULInBufferQ[0]->pBufBase,mULInBufferQ[0]->BufLen,tempULIncopysize);
            if (tempULIncopysize > 0) { //get the buffer data from the first uplink input buffer queue
                *(mpSPEBufferUL1 + indexIn) = *(mULInBufferQ[0]->pRead);    //left channel
                *(mpSPEBufferUL2 + indexIn) = *(mULInBufferQ[0]->pRead + 1); //right channel
                //ALOGD("%d,%d",*(mULInBufferQ[0]->pRead),*(mULInBufferQ[0]->pRead +1));
                mULInBufferQ[0]->pRead += 2;
                tempULIncopysize--;
                indexIn++;
                tmpSPEProcessBufSize -= 4;
                mULInBufQLenTotal -= 4; //int and short transform
                mULInBufferQ[0]->BufLen -= 4; //record the buffer you consumed
            } else { //consume all the data in first queue buffer
                free(mULInBufferQ[0]->pBufBase);
                delete mULInBufferQ[0];
                mULInBufferQ.removeAt(0);
                tempULIncopysize = mULInBufferQ[0]->BufLen >> 2;
                //                ALOGD("UL in buffer consume finish, next BufferBase=%p",mULInBufferQ[0]->pBufBase);
            }
        }

        if (mError) {
            ALOGD("%s, error!!", __FUNCTION__);
            break;
        }

        //process the fill in buffer
        mSphEnhOps.ENH_API_Process(&mSph_Enh_ctrl);

        Dump_EPL(&mSph_Enh_ctrl.EPL_buffer, EPLBufSize * sizeof(short));
        EPLTransVMDump();

        BufferInfo *newOutBuffer = new BufferInfo;

        newOutBuffer->pBufBase = (short *) malloc(mSPEProcessBufSize);
        ASSERT(newOutBuffer->pBufBase != NULL);
        newOutBuffer->BufLen = mSPEProcessBufSize;

        newOutBuffer->pRead = newOutBuffer->pBufBase;
        newOutBuffer->pWrite = newOutBuffer->pBufBase;
        //        ALOGD("newOutBuffer->pBufBase=%p,newOutBuffer->pRead=%p,newOutBuffer->pWrite=%p,newOutBuffer->BufLen=%d",newOutBuffer->pBufBase,newOutBuffer->pRead,newOutBuffer->pWrite,newOutBuffer->BufLen);

        int indexOut = 0;
        int copysize = newOutBuffer->BufLen >> 2;
        while (copysize) {
            //            ALOGD("newOutBuffer->pWrite=%p, indexOut=%d,copysizetest=%d",newOutBuffer->pWrite,indexOut,copysizetest);
            *(newOutBuffer->pWrite) = *(mpSPEBufferUL1 + indexOut); //left channel
            *(newOutBuffer->pWrite + 1) = *(mpSPEBufferUL2 + indexOut); //right channel
            newOutBuffer->pWrite += 2;
            indexOut++;
            copysize--;
        }

        Dump_PCM_Process(UPLINK, newOutBuffer->pBufBase, newOutBuffer->BufLen);

        mULOutBufferQ.add(newOutBuffer);
        mULOutBufQLenTotal += newOutBuffer->BufLen;

        //        ALOGD("mULOutBufQLenTotal=%d, indexOut=%d,newOutBuffer->pWrite=%p, mULOutBufferQsize=%d",mULOutBufQLenTotal,indexOut,newOutBuffer->pWrite,mULOutBufferQ.size());
    }


    //process the processed output buffer queue

    //    ALOGD("mULOutBufferQ=%d, mULOutBufQLenTotal=%d",mULOutBufferQ.size(),mULOutBufQLenTotal);
    if (mULOutBufferQ.isEmpty()) {
        ALOGD("%s, not enought UL output buffer", __FUNCTION__);
        memset(inBuf, 0, inBufLength); //return in same input buffer address
        //AL_UNLOCK(mBufMutex);
        retSize = 0;
        return retSize; //return the processed buffer size
    }

    int tmpInBufLength = inBufLength;
    if (mULOutBufQLenTotal < inBufLength) {
        tmpInBufLength = mULOutBufQLenTotal;
    }
    retSize = tmpInBufLength;
    int count = 0;
    int tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
    while (tmpInBufLength) {
        if (mULOutBufferQ.isEmpty()) {
            ALOGD("%s, uplink Output buffer queue is empty, something wrong!!", __FUNCTION__);
            mError = true;
            break;
        }

        //        ALOGD("mULOutBufferQ.size = %d,tempULCopy=%d",mULOutBufferQ.size(),tempULCopy);

        if (tempULCopy > 0) { //get the buffer data from the first uplink input buffer queue
            //            ALOGD("mULOutBufferQ[0]->pRead = %p,mULOutBufferQ[0]->pBufBase=%p,mULOutBufferQ[0]->BufLen=%d",mULOutBufferQ[0]->pRead,mULOutBufferQ[0]->pBufBase,mULOutBufferQ[0]->BufLen);
            //            ALOGD("tmpInBufLength=%d,count=%d,mULOutBufQLenTotal=%d,tempULCopy=%d",tmpInBufLength,count,mULOutBufQLenTotal,tempULCopy);
            *(inBuf + count) = *(mULOutBufferQ[0]->pRead);
            *(inBuf + count + 1) = *(mULOutBufferQ[0]->pRead + 1);
            mULOutBufferQ[0]->pRead += 2;
            tmpInBufLength -= 4; //int and short transform
            tempULCopy--;
            count += 2;
            mULOutBufQLenTotal -= 4; //int and short transform
            mULOutBufferQ[0]->BufLen -= 4;
        } else { //consume all the data in first queue buffer
            free(mULOutBufferQ[0]->pBufBase);
            delete mULOutBufferQ[0];
            mULOutBufferQ.removeAt(0);
            tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
            //            ALOGD("SPELayer::uplink Output buffer consumed");
        }
    }
    //AL_UNLOCK(mBufMutex);
    return retSize;
}

int SPELayer::Process_VoIP(short *inBuf, int  inBufLength) {
    int retSize = inBufLength;
    //    ALOGD("SPELayer::process VoIP");
    //pthread_mutex_lock(&mBufMutex );
    //       ALOGD("SPELayer::Process, SPERecBufSize=%d,inBufLength=%d,mULInBufQLenTotal=%d, Insize=%d,Outsize=%d",SPERecBufSize,inBufLength,mULInBufQLenTotal,mULInBufferQ.size(),mULOutBufferQ.size());

    if (mULInBufQLenTotal < mSPEProcessBufSize) { //not enough UL input buffer for process
        int tmpInBufLength = inBufLength;

        if (mULOutBufferQ.isEmpty() || mULOutBufQLenTotal < inBufLength) { //TODO:fixme, return data we have?
            ALOGD("not enough UL output buffer, inBuf=%p,inBufLength=%d", inBuf, inBufLength);
            memset(inBuf, 0, inBufLength); //reset data
            //AL_UNLOCK(mBufMutex);
            //return true;
            retSize = 0;
            return retSize; //return the processed buffer size
        }


        int count = 0;
        int tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
        while (tmpInBufLength) {
            if (mULOutBufferQ.isEmpty()) {
                ALOGD("%s(), Output buffer queue is empty, return size mULOutBufQLenTotal", __FUNCTION__);
                break;
            }

            //                ALOGD("mDLOutBufferQ.size = %d,tempDLCopy=%d",mDLOutBufferQ.size(),tempDLCopy);

            if (tempULCopy > 0) { //get the buffer data from the first downlink input buffer queue
                //                       ALOGD("mULOutBufferQ[0]->pRead = %p,mULOutBufferQ[0]->pBufBase=%p,mULOutBufferQ[0]->BufLen=%d",mULOutBufferQ[0]->pRead,mULOutBufferQ[0]->pBufBase,mULOutBufferQ[0]->BufLen);
                //                       ALOGD("tmpInBufLength=%d,count=%d,mULOutBufQLenTotal=%d,tempULCopy=%d",tmpInBufLength,count,mULOutBufQLenTotal,tempULCopy);
                *(inBuf + count) = *(mULOutBufferQ[0]->pRead);
                *(inBuf + count + 1) = *(mULOutBufferQ[0]->pRead + 1);
                mULOutBufferQ[0]->pRead += 2;
                tmpInBufLength -= 4; //int and short transform
                tempULCopy--;
                count += 2;
                mULOutBufQLenTotal -= 4; //int and short transform
                mULOutBufferQ[0]->BufLen -= 4;
            } else { //consume all the data in first queue buffer
                free(mULOutBufferQ[0]->pBufBase);
                delete mULOutBufferQ[0];
                mULOutBufferQ.removeAt(0);
                //need check if still have ULOutbuffer
                if (!mULOutBufferQ.isEmpty()) {
                    ALOGD("%s, mULOutBufferQ not empty, get next one, size=%zu", __FUNCTION__, mULOutBufferQ.size());
                    tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
                } else {
                    ALOGD("%s, mULOutBufferQ is empty!!! size=%zu", __FUNCTION__, mULOutBufferQ.size());
                }
                //                  ALOGD("SPELayer::uplink Output buffer consumed a");
            }
        }
        ALOGD("%s, has UL Output buffer but not enough UL Input buffer, size=%zu, mULOutBufQLenTotal=%d", __FUNCTION__, mULOutBufferQ.size(), mULOutBufQLenTotal);
        //AL_UNLOCK(mBufMutex);
        //AL_UNLOCK(mBufMutex);
        //return true;
        return retSize;
    }

    //fix me!!: process when UL data is enough, DL data need to compensated as zero
    //processing if have enough input UL data (UL is stereo, DL is mono data)
    if (mULInBufQLenTotal >= mSPEProcessBufSize) {    //&&(mDLInBufQLenTotal>= mSPEProcessBufSize/2))
        while (mULInBufQLenTotal >= mSPEProcessBufSize) { //&&(mDLInBufQLenTotal>= mSPEProcessBufSize/2))  //TODO:fixme!!! has problem
            if (mDLInBufQLenTotal < mSPEProcessBufSize / 2) { //not enough downlink data to process, wait for a while
                if (mULOutBufQLenTotal >= inBufLength) {
                    if (EPLDebugEnable == true) {
                        ALOGD("Process_VoIP have enough uplink processed data, skip this time");
                    }
                    break;
                }
                //WaitforDownlinkData();
            }

            if (PrepareProcessData()) { //sync ok, could start process
                if (mDLInBufQLenTotal < mSPEProcessBufSize / 2) { //not enough downlink data to process, wait for a while
                    if (WaitforDownlinkData()) { //got new DL data queue
                        if (mDLInBufQLenTotal < mSPEProcessBufSize / 2) { //but still not enough data to process
                            if (EPLDebugEnable == true) {
                                ALOGD("got new DL buffer, but still not enough data to process");
                            }
                            continue;
                        }
                    } else { //no new DL data queue
                        ALOGD("no new DL buffer queue, process directly");
                    }
                } else { //has enough downlink data to  process, but still check if there has downlink buffer queue wait
                    if (mNewReferenceBufferComes) { //if there is new DL buffer comes, let it add the queue first (To not block the downlink process)
                        InsertDownlinkData();
                    }
                }
            } else { //no sync yet, no need to check or wait for downlink data
                if (mNewReferenceBufferComes) { //if there is new DL buffer comes, let it add the queue first (To not block the downlink process)
                    ALOGD("also check if new downlink data comes even the sync is not ready");
                    InsertDownlinkData();
                }
            }

            //fill in the data to process buffer
            int tmpSPEProcessBufSize = mSPEProcessBufSize;
            int indexIn = 0;
            int ULIncopysize = mULInBufferQ[0]->BufLen >> 2;

            struct timespec tstamp_process;
            struct timespec DLtstamp_compen;

            if (mDLInBufferQ.isEmpty()) { //there is no DL data before process, compensate time use the uplink time, else use the DL previous time
                DLtstamp_compen = mULInBufferQ[0]->time_stamp_estimate;
            }
            if (EPLDebugEnable == true) {
                //ALOGD("%s, mULInBufQLenTotal=%d, mDLInBufQLenTotal=%d, SPERecBufSize=%d,ULIncopysize=%d", __FUNCTION__, mULInBufQLenTotal, mDLInBufQLenTotal, mSPEProcessBufSize, ULIncopysize);
                ALOGD("%s, mULInBufQLenTotal=%d, mDLInBufQLenTotal=%d, SPERecBufSize=%d,ULIncopysize=%d, mULOutBufQLenTotal=%d, mULOutBufferQ.size=%zu, bypass process",
                      __FUNCTION__, mULInBufQLenTotal, mDLInBufQLenTotal, mSPEProcessBufSize, ULIncopysize, mULOutBufQLenTotal, mULOutBufferQ.size());
            }
            //if(!mDLDelayBufferQ.isEmpty())
            //ALOGD("SPELayer::Process_VoIP, mDLDelayBufferQ size=%d,mDLDelayBufQLenTotal=%d, SPERecBufSize=%d, %p",mDLDelayBufferQ.size(),mDLDelayBufQLenTotal,mSPEProcessBufSize,mDLDelayBufferQ[0]->pBufBase);
            //if(!mDLInBufferQ.isEmpty())
            //ALOGD("SPELayer::Process_VoIP, mDLInBufferQ size=%d,mDLInBufQLenTotal=%d,ULIncopysize=%d, %p",mDLInBufferQ.size(),mDLInBufQLenTotal,ULIncopysize,mDLInBufferQ[0]->pBufBase);

            while (tmpSPEProcessBufSize) {
                if (mULInBufferQ.isEmpty()) { //||mDLInBufferQ.isEmpty()||mDLDelayBufferQ.isEmpty())
                    ALOGD("%s,input buffer queue is empty, something wrong!!", __FUNCTION__);
                    mError = true;
                    break;
                }

                tstamp_process = GetSystemTime();

                //                      ALOGD("SPELayer indexIn=%d, tmpSPERecBufSize=%d, mULInBufQLenTotal=%d,mULInBufferQ[0]->pRead=%p,mULInBufferQ[0]->pBufBase=%p,mULInBufferQ[0]->BufLen=%d,ULIncopysize=%d",indexIn,tmpSPERecBufSize,mULInBufQLenTotal,mULInBufferQ[0]->pRead,mULInBufferQ[0]->pBufBase,mULInBufferQ[0]->BufLen,ULIncopysize);
                if (ULIncopysize > 0) { //get the buffer data from the first uplink input buffer queue
                    //fill in uplink data
                    *(mpSPEBufferUL1 + indexIn) = *(mULInBufferQ[0]->pRead);
                    *(mpSPEBufferUL2 + indexIn) = *(mULInBufferQ[0]->pRead + 1);
                    mULInBufferQ[0]->pRead += 2;
                    mULInBufQLenTotal -= 4; //int and short transform
                    mULInBufferQ[0]->BufLen -= 4; //record the buffer you consumed

                    mULInBufferQ[0]->time_stamp_process = tstamp_process;


#if 1   //INTR
                    //update estimate time, when use the corresponding time
                    mULInBufferQ[0]->time_stamp_estimate.tv_sec = mULInBufferQ[0]->time_stamp_estimate.tv_sec + (mULInBufferQ[0]->time_stamp_estimate.tv_nsec + mNsecPerSample) / 1000000000;
                    mULInBufferQ[0]->time_stamp_estimate.tv_nsec = (mULInBufferQ[0]->time_stamp_estimate.tv_nsec + mNsecPerSample) % 1000000000;
#endif
                    //fill in downlink data
                    if (mDLInBufferQ.isEmpty()) {
                        ALOGD("no DL buffer, need compensate 1, tmpSPEProcessBufSize=%d", tmpSPEProcessBufSize);
                        CompensateBuffer(tmpSPEProcessBufSize / 2, DLtstamp_compen);
                    }
                    if (mDLInBufferQ[0]->BufLen <= 0) { //run out of DL queue0 buffer
                        //not to free the buffer here due to the data still queue in the DLDelay buffer
                        //free(mDLInBufferQ[0]->pBufBase);  //just remove the queue but not delete buffer since it also queue in the delay queue
                        //delete mDLInBufferQ[0];
                        mDLInBufferQ.removeAt(0);
                        //                            ALOGD("get next DLInBufferQ, size=%d, mDLInBufQLenTotal=%d",mDLInBufferQ.size(),mDLInBufQLenTotal);
                        if (mDLInBufferQ.isEmpty()) {
                            ALOGD("no DL buffer, need compensate, tmpSPEProcessBufSize=%d", tmpSPEProcessBufSize);
#if 1//def DOWNLINK_MONO                            
                            CompensateBuffer(tmpSPEProcessBufSize / 2, DLtstamp_compen);
#else
                            CompensateBuffer(tmpSPEProcessBufSize);
#endif
                        }
                        //                            ALOGD("DL in buffer consume finish, next BufferBase=%p",mDLInBufferQ[0]->pBufBase);
                    }

                    mDLInBufferQ[0]->time_stamp_process = tstamp_process;
#if 1//def DOWNLINK_MONO                     
                    //*(mpSPEBufferDL + indexIn) = (*(mDLInBufferQ[0]->pRead)>>1) + (*(mDLInBufferQ[0]->pRead+1)>>1); //only mono data
                    *(mpSPEBufferDL + indexIn) = *(mDLInBufferQ[0]->pRead); //already mono data
                    mDLInBufferQ[0]->pRead++;
                    mDLInBufQLenTotal -= 2; //int and short transform
                    mDLInBufferQ[0]->BufLen -= 2; //record the buffer you consumed


                    mDLInBufferQ[0]->time_stamp_estimate.tv_sec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec + (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + mNsecPerSample) / 1000000000;
                    mDLInBufferQ[0]->time_stamp_estimate.tv_nsec = (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + mNsecPerSample) % 1000000000;
                    DLtstamp_compen = mDLInBufferQ[0]->time_stamp_estimate;

                    //check to remove the first DL buffer to avoid compare the wrong estimate time next time if the buffer is compensated buffer
                    if (mDLInBufferQ[0]->BufLen <= 0) { //run out of DL queue0 buffer
                        //not to free the buffer here due to the data still queue in the DLDelay buffer
                        mDLInBufferQ.removeAt(0);
                    }
#else
                    *(mpSPEBufferDL + indexIn) = (*(mDLInBufferQ[0]->pRead) >> 1) + (*(mDLInBufferQ[0]->pRead + 1) >> 1); //only mono data
                    mDLInBufferQ[0]->pRead += 2;
                    mDLInBufQLenTotal -= 4; //int and short transform
                    mDLInBufferQ[0]->BufLen -= 4; //record the buffer you consumed
#endif

                    //fill in delay latency data
                    if (mDLDelayBufferQ[0]->BufLen4Delay <= 0) { //run out of DL  delay queue0 buffer
                        //ALOGD("DL delay consume");
                        free(mDLDelayBufferQ[0]->pBufBase);
                        delete mDLDelayBufferQ[0];
                        mDLDelayBufferQ.removeAt(0);
                        if (mDLDelayBufferQ.isEmpty()) {
                            ALOGD("no DL delay buffer, should already compensate something wrong");
                            mError = true;
                            break;
                        }
                        //                            ALOGD("DL delay in buffer consume finish, next BufferBase=%p, size=%d",mDLDelayBufferQ[0]->pBufBase,mDLDelayBufferQ.size());
                    }
                    mDLDelayBufferQ[0]->time_stamp_process = tstamp_process;
#if 1//def DOWNLINK_MONO 
                    //*(mpSPEBufferDLDelay + indexIn) = (*(mDLDelayBufferQ[0]->pRead4Delay)>>1) + (*(mDLDelayBufferQ[0]->pRead4Delay+1)>>1); //only mono data
                    *(mpSPEBufferDLDelay + indexIn) = *(mDLDelayBufferQ[0]->pRead4Delay); //already mono data
                    mDLDelayBufferQ[0]->pRead4Delay++;
                    mDLDelayBufQLenTotal -= 2; //int and short transform
                    mDLDelayBufferQ[0]->BufLen4Delay -= 2; //record the buffer you consumed
                    //ALOGD("%d,%d",*(mULInBufferQ[0]->pRead),*(mULInBufferQ[0]->pRead +1));
#else
                    *(mpSPEBufferDLDelay + indexIn) = (*(mDLDelayBufferQ[0]->pRead4Delay) >> 1) + (*(mDLDelayBufferQ[0]->pRead4Delay + 1) >> 1); //only mono data
                    mDLDelayBufferQ[0]->pRead4Delay += 2;
                    mDLDelayBufQLenTotal -= 4; //int and short transform
                    mDLDelayBufferQ[0]->BufLen4Delay -= 4; //record the buffer you consumed
#endif

                    if (mDLDelayBufferQ[0]->BufLen4Delay <= 0) { //run out of DL  delay queue0 buffer
                        free(mDLDelayBufferQ[0]->pBufBase);
                        delete mDLDelayBufferQ[0];
                        mDLDelayBufferQ.removeAt(0);
                    }

                    ULIncopysize--;
                    indexIn++;
                    tmpSPEProcessBufSize -= 4;

                } else { //consume all the data in first queue buffer
                    free(mULInBufferQ[0]->pBufBase);
                    delete mULInBufferQ[0];
                    mULInBufferQ.removeAt(0);
                    ULIncopysize = mULInBufferQ[0]->BufLen >> 2;
                    //ALOGD("UL in buffer consume finish, next BufferBase=%p, size=%d,mULInBufQLenTotal=%d",mULInBufferQ[0]->pBufBase,mULInBufferQ.size(),mULInBufQLenTotal);
                }
            }

            if (mError) {
                ALOGE("error happened!!");
                break;
            }

            //after fill buffer, process
            mSphEnhOps.ENH_API_Process(&mSph_Enh_ctrl);

            Dump_EPL(&mSph_Enh_ctrl.EPL_buffer, EPLBufSize * sizeof(short));
            EPLTransVMDump();

            //record to the outputbuffer queue, no need processed downlink data

            //BufferInfo *newDLOutBuffer = new BufferInfo;
            BufferInfo *newULOutBuffer = new BufferInfo;

            //newDLOutBuffer->pBufBase = (short*) malloc(mSPEProcessBufSize/2);
            //newDLOutBuffer->BufLen= mSPEProcessBufSize/2;

            //newDLOutBuffer->pRead = newDLOutBuffer->pBufBase;
            //newDLOutBuffer->pWrite= newDLOutBuffer->pBufBase;

            newULOutBuffer->pBufBase = (short *) malloc(mSPEProcessBufSize);
            ASSERT(newULOutBuffer->pBufBase != NULL);
            newULOutBuffer->BufLen = mSPEProcessBufSize;

            newULOutBuffer->pRead = newULOutBuffer->pBufBase;
            newULOutBuffer->pWrite = newULOutBuffer->pBufBase;
            //                ALOGD("newDLOutBuffer->pBufBase=%p,newDLOutBuffer->pRead=%p,newDLOutBuffer->pWrite=%p,newDLOutBuffer->BufLen=%d",newDLOutBuffer->pBufBase,newDLOutBuffer->pRead,newDLOutBuffer->pWrite,newDLOutBuffer->BufLen);
            int indexOut = 0;

            int copysizetest = newULOutBuffer->BufLen >> 2;
            while (copysizetest) {
                //ALOGD("newOutBuffer->pWrite=%p, indexOut=%d,copysizetest=%d",newOutBuffer->pWrite,indexOut,copysizetest);
                //*(newDLOutBuffer->pWrite) = *(mpSPEBufferFE + indexOut);
                //*(newDLOutBuffer->pWrite+1) = *(mpSPEBufferFE + indexOut);

                *(newULOutBuffer->pWrite) = *(mpSPEBufferNE + indexOut);
                *(newULOutBuffer->pWrite + 1) = *(mpSPEBufferNE + indexOut);

                //                      ALOGD("indexOut=%d,mpSPEBufferFE =%d, mpSPEBufferNE=%d",indexOut,*(mpSPEBufferFE + indexOut),*(mpSPEBufferNE + indexOut));
                //newDLOutBuffer->pWrite+=2;
                newULOutBuffer->pWrite += 2;
                indexOut++;
                copysizetest--;
            }

            //mDLOutBufferQ.add(newDLOutBuffer);
            //mDLOutBufQLenTotal += newDLOutBuffer->BufLen;
            //                  ALOGD("queue to DLOut mDLOutBufQLenTotal=%d, size=%d",mDLOutBufQLenTotal,mDLOutBufferQ.size());
            //Dump_PCM_Process(DOWNLINK,newDLOutBuffer->pBufBase,newDLOutBuffer->BufLen);
            mULOutBufferQ.add(newULOutBuffer);
            mULOutBufQLenTotal += newULOutBuffer->BufLen;
            //            Dump_PCM_Process(UPLINK,newULOutBuffer->pBufBase,newULOutBuffer->BufLen);
        }
        //            ALOGD("return SPELayer::Process, mDLInBufferQ size=%d,mDLInBufQLenTotal=%d,mDLInBufferQ[0]->BufLen=%d",mDLInBufferQ.size(),mDLInBufQLenTotal,mDLInBufferQ[0]->BufLen);

    } else { //not enough UL  data, not process
        ALOGD("not enough uplink data, not process");
    }

    //process the uplink output processed buffer queue
    memset(inBuf, 0, inBufLength); //clean the buffer will be used
#if 1
    if (mULOutBufferQ.isEmpty())  //fixme, return data we have?
#else
    if (mULOutBufferQ.isEmpty() || mULOutBufQLenTotal < inBufLength) //fixme, return data we have?
#endif
    {
        ALOGD("SPELayer not enought UL output buffer return size");
        //AL_UNLOCK(mBufMutex);
        //return true;
        retSize = 0;
        return retSize; //return the processed buffer size
    } else {
        int tmpInBufLength = inBufLength;
        if (mULOutBufQLenTotal < inBufLength) {
            ALOGD("Process_VoIP mULOutBufQLenTotal<inBufLength");
            tmpInBufLength = mULOutBufQLenTotal;
            retSize = mULOutBufQLenTotal;
        }
        int count = 0;
        int tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
        while (tmpInBufLength) {
            if (mULOutBufferQ.isEmpty()) {
                ALOGD("Process_VoIP run out of  output buffer queue");
                break;
            }

            //   ALOGD("mULOutBufferQ.size = %d,tempULCopy=%d",mULOutBufferQ.size(),tempULCopy);

            if (tempULCopy > 0) { //get the buffer data from the first uplink input buffer queue
                //            ALOGD("mULOutBufferQ[0]->pRead = %p,mULOutBufferQ[0]->pBufBase=%p,mULOutBufferQ[0]->BufLen=%d",mULOutBufferQ[0]->pRead,mULOutBufferQ[0]->pBufBase,mULOutBufferQ[0]->BufLen);
                //            ALOGD("tmpInBufLength=%d,count=%d,mULOutBufQLenTotal=%d,tempULCopy=%d",tmpInBufLength,count,mULOutBufQLenTotal,tempULCopy);
                *(inBuf + count) = *(mULOutBufferQ[0]->pRead);
                *(inBuf + count + 1) = *(mULOutBufferQ[0]->pRead + 1);
                mULOutBufferQ[0]->pRead += 2;
                tmpInBufLength -= 4; //int and short transform
                tempULCopy--;
                count += 2;
                mULOutBufQLenTotal -= 4; //int and short transform
                mULOutBufferQ[0]->BufLen -= 4;
            } else { //consume all the data in first queue buffer
                free(mULOutBufferQ[0]->pBufBase);
                delete mULOutBufferQ[0];
                mULOutBufferQ.removeAt(0);
                if (!mULOutBufferQ.isEmpty()) {
                    //ALOGD("Process_VoIP mULOutBufferQ not empty, get next one 2, size=%d",mULOutBufferQ.size());
                    tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
                } else {
                    ALOGD("Process_VoIP mULOutBufferQ empty no more data 2, size=%zu", mULOutBufferQ.size());
                }
                //                      ALOGD("SPELayer::uplink Output buffer consumed");
            }
        }
    }

    //AL_UNLOCK(mBufMutex);
    //return true;
    return retSize;
}

//normal record + VoIP, new interface
int SPELayer::Process(InBufferInfo *InBufinfo) {
    if (mError == true) {
        ReStart();
        mError = false;
    }
    Mutex::Autolock lock(mLock);
    //ALOGD("SPELayer::Process going to take mBufMutex new interface");
    mBufMutexWantLock.lock();
    AL_LOCK(mBufMutex);
    mBufMutexWantLock.unlock();
    if (mState == SPE_STATE_IDLE) {
        ALOGD("%s, wrong state, mState=%d,mMode=%d", __FUNCTION__, mState, mMode);
        AL_UNLOCK(mBufMutex);
        return false;
    }
    //    if(mMode == SPE_MODE_REC)
    {
        if ((mULInBufferQ.size() > 20) || (mULOutBufferQ.size() > 20)) {
            ALOGD("no service? mULInBufferQ.size=%zu, mULOutBufferQ.size=%zu", mULInBufferQ.size(), mULOutBufferQ.size());
        }
    }
    mState = SPE_STATE_RUNNING;

    AddtoInputBuffer(UPLINK, InBufinfo);
    //AddUplinkBuffer(InBufinfo);

    int inBufLength = InBufinfo->BufLen;
    short *inBuf = InBufinfo->pBufBase;
    int retSize = inBufLength;

    //process the input buffer queue
    if (mMode == SPE_MODE_REC) { //record
        mVoIPRunningbefore = false;
        retSize = Process_Record(inBuf, inBufLength);
    } else { //VoIP
        mVoIPRunningbefore = true;
        Process_VoIP(inBuf, inBufLength);
    }

    Dump_PCM_Out(UPLINK, inBuf, retSize);

    if (mPrepareProcessDataReady && (mDLInBufQLenTotal > (inBufLength * 10))) { //if there has too many echoref data, it means the uplink data may not get continuously. Might have buffer overflow in kernel. (uplink data is stereo\echoref data is mono)
        //need to resync
        ReSync();
        ALOGD("%s, uplink data might uncontinuous, resync", __FUNCTION__);
    }

    AL_UNLOCK(mBufMutex);
    //ALOGD("%s, inBufLength=%d,retSize=%d", __FUNCTION__, inBufLength, retSize);
    return retSize;

}

bool    SPELayer::Stop() {
    ALOGD("%s", __FUNCTION__);
    Mutex::Autolock lock(mLock);
    AL_LOCK(mBufMutex);
    if (mState == SPE_STATE_IDLE) {
        ALOGD("not start before");
        AL_UNLOCK(mBufMutex);
        return false;
    }
    mState = SPE_STATE_CLEANING;
    Clear();
    AL_UNLOCK(mBufMutex);
    return true;
}


void SPELayer::ReStart() {
    ALOGD("%s, State=%d, mode=%d", __FUNCTION__, mState, mMode);
    Stop();
    Start(mMode);
}

void SPELayer::Clear() {
    ALOGD("%s", __FUNCTION__);
    //pthread_mutex_lock(&mBufMutex );
    if (mSphCtrlBuffer) {
        ALOGD("free mSphCtrlBuffer %p", mSphCtrlBuffer);
        mSphEnhOps.ENH_API_Free(&mSph_Enh_ctrl);
        free(mSphCtrlBuffer);
        mSphCtrlBuffer = NULL;
        ALOGD("~free mSphCtrlBuffer");
    }

    mpSPEBufferUL1 = NULL;
    mpSPEBufferUL2 = NULL;
    mpSPEBufferDL = NULL;
    mpSPEBufferDLDelay = NULL;

    mNeedDelayLatency = false;
    mNeedJitterBuffer = false;
    mCompensatedBufferSize = 0;

#if 0
    //clear the buffer queue

    ALOGD("SPELayer::mULOutBufferQ size=%d,mULInBufferQ.size=%d,mDLOutBufferQ.size()=%d,mDLInBufferQ.size()=%d,mDLDelayBufferQ.size()=%d", mULOutBufferQ.size(), mULInBufferQ.size(), mDLOutBufferQ.size(),
          mDLInBufferQ.size(), mDLDelayBufferQ.size());
    if (mULOutBufferQ.size() != 0) {
        while (mULOutBufferQ.size()) {
            free(mULOutBufferQ[0]->pBufBase);
            delete mULOutBufferQ[0];
            mULOutBufferQ.removeAt(0);
        }
        mULOutBufferQ.clear();
    }
    if (mULInBufferQ.size() != 0) {
        while (mULInBufferQ.size()) {
            free(mULInBufferQ[0]->pBufBase);
            delete mULInBufferQ[0];
            mULInBufferQ.removeAt(0);
        }
        mULInBufferQ.clear();
    }

    if (mDLOutBufferQ.size() != 0) {
        while (mDLOutBufferQ.size()) {
            free(mDLOutBufferQ[0]->pBufBase);
            delete mDLOutBufferQ[0];
            mDLOutBufferQ.removeAt(0);
        }
        mDLOutBufferQ.clear();
    }
    if (mDLInBufferQ.size() != 0) {
        while (mDLInBufferQ.size()) {
            if (mDLInBufferQ[0]->pBufBase) {
                ALOGD("mDLInBufferQ::pBufBase=%d", mDLInBufferQ[0]->pBufBase);
                //                free(mDLInBufferQ[0]->pBufBase);
                ALOGD("mDLInBufferQ::free");
                //                delete mDLInBufferQ[0];
                ALOGD("mDLInBufferQ::delete");
                mDLInBufferQ.removeAt(0);
                ALOGD("mDLInBufferQ::done, free at DLDelay buffer");
            }
        }
        mDLInBufferQ.clear();
    }

    if (mDLDelayBufferQ.size() != 0) {
        while (mDLDelayBufferQ.size()) {
            if (mDLDelayBufferQ[0]->pBufBase) {
                ALOGD("mDLDelayBufferQ::pBufBase=%d", mDLDelayBufferQ[0]->pBufBase);
                free(mDLDelayBufferQ[0]->pBufBase);
                ALOGD("mDLDelayBufferQ::free");
                delete mDLDelayBufferQ[0];
                ALOGD("mDLDelayBufferQ::delete");
                mDLDelayBufferQ.removeAt(0);
                ALOGD("mDLDelayBufferQ::done");
            }

        }
        mDLDelayBufferQ.clear();
    }

    mULInBufQLenTotal = 0;
    mDLInBufQLenTotal = 0;
    mULOutBufQLenTotal = 0;
    mDLOutBufQLenTotal = 0;
    mDLDelayBufQLenTotal = 0;
    mCompensatedBufferSize = 0;
#endif
    mState = SPE_STATE_IDLE;

    ALOGD("~Clear");
    //AL_UNLOCK(mBufMutex);
}

bool SPELayer::Standby() {
    ALOGD("%s+", __FUNCTION__);
    bool bRet = true;

    Mutex::Autolock lock(mLock);
    AL_LOCK(mBufMutex);

    mState = SPE_STATE_CLEANING;
    Clear();

    mMode = SPE_MODE_NONE;
    mRoute = ROUTE_NONE;

    mError = false;

    FlushBufferQ();

    mFirstVoIPUplink = true;
    mFirstVoIPDownlink = true;
    mDLNewStart = false;
    mPrepareProcessDataReady = false;
    mDLPreQLimit = true;

    mVoIPRunningbefore = false;
    DLdataPrepareCount = 0;
    mOutputStreamRunning = false;

    mLatencyDir = true;
    mNeedJitterBuffer = false;
    mNormalModeVoIP = false;
    mPreULBufLen = 0;
    mPreDLBufLen = 0;

    memset(&mUplinkIntrStartTime, 0, sizeof(timespec));
    memset(&mPreUplinkEstTime, 0, sizeof(timespec));

    //    memset(&mDownlinkIntrStartTime, 0, sizeof(timespec));
    //    memset(&mPreDownlinkEstTime, 0, sizeof(timespec));
    //    memset(&mPreDownlinkQueueTime, 0, sizeof(timespec));

    AL_UNLOCK(mBufMutex);
    ALOGD("%s-", __FUNCTION__);
    return bRet;
}


void SPELayer::ReSync() {
    if (mPrepareProcessDataReady) {
        ALOGD("%s", __FUNCTION__);
        //ASSERT(0);
        mPrepareProcessDataReady = false;
        mCompensatedBufferSize = 0;
    }
}

void SPELayer::dump() {
    ALOGD("%s, State=%d, mode=%d", __FUNCTION__, mState, mMode);
    //dump normal record parameters
    ALOGD("Record:Samplerate = %d, FrameRate=%d,App_table=%x, Fea_Cfg_table=%x", mRecordSampleRate, mRecordFrameRate, mRecordApp_table, mRecordFea_Cfg_table);
    ALOGD("Record:EnhanceParas");
    for (int i = 0; i < (EnhanceParasNum / 7); i++) {
        ALOGD("[index %d] %d,%d,%d,%d,%d,%d,%d", i, mRecordEnhanceParas[i * 7], mRecordEnhanceParas[i * 7 + 1], mRecordEnhanceParas[i * 7 + 2],
              mRecordEnhanceParas[i * 7 + 3], mRecordEnhanceParas[i * 7 + 4], mRecordEnhanceParas[i * 7 + 5], mRecordEnhanceParas[i * 7 + 6]);
    }

    /*   ALOGD("Record:DMNRCalData");
       for(int i=0; i<DMNRCalDataNum; i++)
           ALOGD("%d",mRecordDMNRCalData[i]);
       ALOGD("Record:CompenFilter");
       for(int i=0; i<CompenFilterNum; i++)
           ALOGD("%d",mRecordCompenFilter[i]);


       //dump VoIP parameters
       ALOGD("VoIP:Samplerate = %d, FrameRate=%d,PGAGain=%d, App_table=%x, Fea_Cfg_table=%x",mVoIPSampleRate,mVoIPFrameRate,mVoIPPGAGain,mVoIPApp_table,mVoIPFea_Cfg_table);
       ALOGD("VoIP:EnhanceParas");
       for(int i=0; i<EnhanceParasNum; i++)
           ALOGD("%d",mVoIPEnhanceParas[i]);
       ALOGD("VoIP:DMNRCalData");
       for(int i=0; i<DMNRCalDataNum; i++)
           ALOGD("%d",mVoIPDMNRCalData[i]);
       ALOGD("VoIP:CompenFilter");
       for(int i=0; i<CompenFilterNum; i++)
           ALOGD("%d",mVoIPCompenFilter[i]);
    */
    //dump using parameters
    ALOGD("Using:Samplerate = %d, FrameRate=%d,MIC_DG=%d, App_table=%x, Fea_Cfg_table=%x, MMI_table=%x, Device_mode=%x, MMI_MIC_GAIN=%d",
          mSph_Enh_ctrl.sample_rate, mSph_Enh_ctrl.frame_rate, mSph_Enh_ctrl.MIC_DG, mSph_Enh_ctrl.App_table, mSph_Enh_ctrl.Fea_Cfg_table, mSph_Enh_ctrl.MMI_ctrl,
          mSph_Enh_ctrl.Device_mode, mSph_Enh_ctrl.MMI_MIC_GAIN);
    ALOGD("Using:EnhanceParas");
    for (int i = 0; i < (EnhanceParasNum / 7); i++)
        ALOGD("[index %d] %d,%d,%d,%d,%d,%d,%d", i, mSph_Enh_ctrl.enhance_pars[i * 7], mSph_Enh_ctrl.enhance_pars[i * 7 + 1], mSph_Enh_ctrl.enhance_pars[i * 7 + 2]
              , mSph_Enh_ctrl.enhance_pars[i * 7 + 3], mSph_Enh_ctrl.enhance_pars[i * 7 + 4], mSph_Enh_ctrl.enhance_pars[i * 7 + 5], mSph_Enh_ctrl.enhance_pars[i * 7 + 6]);

    ALOGD("Using:DMNRCalData");
    for (int i = 0; i < (DMNRCalDataNum / 19); i++)
        ALOGD("[index %d] %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
              i, mSph_Enh_ctrl.DMNR_cal_data[i * 19], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 1], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 2]
              , mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 3], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 4], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 5], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 6]
              , mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 7], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 8], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 9], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 10]
              , mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 11], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 12], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 13], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 14]
              , mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 15], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 16], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 17], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 18]);

    /*  ALOGD("Using:CompenFilter");
        for(int i=0; i<CompenFilterNum; i++)
            ALOGD("%d",mSph_Enh_ctrl.Compen_filter[i]);
        */
}

static int checkAndCreateDirectory(const char *pC) {
    char tmp[PATH_MAX];
    int i = 0;

    while (*pC) {
        tmp[i] = *pC;

        if (*pC == '/' && i) {
            tmp[i] = '\0';
            if (access(tmp, F_OK) != 0) {
                if (mkdir(tmp, 0770) == -1) {
                    ALOGE("AudioDumpPCM: mkdir error! %s\n", (char *)strerror(errno));
                    return -1;
                }
            }
            tmp[i] = '/';
        }
        i++;
        pC++;
    }
    return 0;

}

bool SPELayer::HasBufferDump() {
    DumpMutexLock();
    bool bret = true;
    if (mDumpDLInBufferQ.size() == 0 && mDumpDLOutBufferQ.size() == 0 && mDumpULInBufferQ.size() == 0 && mDumpULOutBufferQ.size() == 0
        && mDumpEPLBufferQ.size() == 0) {
        bret = false;
    }

    DumpMutexUnlock();
    return bret;
}

void SPELayer::DumpBufferClear(void) {
    DumpMutexLock();
    ALOGD("DumpBufferClear, %zu %zu %zu %zu %zu", mDumpDLInBufferQ.size(), mDumpDLOutBufferQ.size(), mDumpULInBufferQ.size(), mDumpULOutBufferQ.size(), mDumpEPLBufferQ.size());
    if (mDumpDLInBufferQ.size() != 0) {
        while (mDumpDLInBufferQ.size()) {
            free(mDumpDLInBufferQ[0]->pBufBase);
            delete mDumpDLInBufferQ[0];
            mDumpDLInBufferQ.removeAt(0);
        }
        mDumpDLInBufferQ.clear();
    }

    if (mDumpDLOutBufferQ.size() != 0) {
        while (mDumpDLOutBufferQ.size()) {
            free(mDumpDLOutBufferQ[0]->pBufBase);
            delete mDumpDLOutBufferQ[0];
            mDumpDLOutBufferQ.removeAt(0);
        }
        mDumpDLOutBufferQ.clear();
    }

    if (mDumpULInBufferQ.size() != 0) {
        while (mDumpULInBufferQ.size()) {
            free(mDumpULInBufferQ[0]->pBufBase);
            delete mDumpULInBufferQ[0];
            mDumpULInBufferQ.removeAt(0);
        }
        mDumpULInBufferQ.clear();
    }

    if (mDumpULOutBufferQ.size() != 0) {
        while (mDumpULOutBufferQ.size()) {
            free(mDumpULOutBufferQ[0]->pBufBase);
            delete mDumpULOutBufferQ[0];
            mDumpULOutBufferQ.removeAt(0);
        }
        mDumpULOutBufferQ.clear();
    }

    if (mDumpEPLBufferQ.size() != 0) {
        while (mDumpEPLBufferQ.size()) {
            free(mDumpEPLBufferQ[0]->pBufBase);
            delete mDumpEPLBufferQ[0];
            mDumpEPLBufferQ.removeAt(0);
        }
        mDumpEPLBufferQ.clear();
    }
    DumpMutexUnlock();
    ALOGD("DumpBufferClear---");
}

void *DumpThread(void *arg) {
    SPELayer *pSPEL = (SPELayer *)arg;
    ALOGD("DumpThread");
    struct timespec timeout;
    timeout.tv_sec  = 0;
    timeout.tv_nsec = 10 * 1000000;
    AL_LOCK(pSPEL->mDumpExitMutex);

    while (1) {
        if (pSPEL->hDumpThread == 0) {
            ALOGD("DumpThread hDumpThread null");
            //pSPEL->DumpBufferClear();
            break;
#if 0
            if (!pSPEL->HasBufferDump()) {
                pSPEL->DumpMutexLock();
                pSPEL->mDumpDLInBufferQ.clear();
                pSPEL->mDumpDLOutBufferQ.clear();
                pSPEL->mDumpULInBufferQ.clear();
                pSPEL->mDumpULOutBufferQ.clear();
                pSPEL->mDumpEPLBufferQ.clear();
                pSPEL->DumpMutexUnlock();
                ALOGD("DumpThread exit");
                break;
            } else {
                ALOGD("DumpThread still has buffer need consume");
            }
#endif
        }
        if (!pSPEL->HasBufferDump()) {
            AL_UNLOCK(pSPEL->mDumpExitMutex);
            AL_SIGNAL(pSPEL->mDumpExitMutex);
            usleep(3 * 1000);
            AL_LOCK(pSPEL->mDumpExitMutex);
            continue;
        }
        //ALOGD( "DumpThread,mDumpULInBufferQ=%d, mDumpULOutBufferQ=%d, mDumpEPLBufferQ=%d",pSPEL->mDumpULInBufferQ.size(),pSPEL->mDumpULOutBufferQ.size(),pSPEL->mDumpEPLBufferQ.size());
        if (pSPEL->mDumpDLInBufferQ.size() > 0) {
            fwrite(pSPEL->mDumpDLInBufferQ[0]->pBufBase, pSPEL->mDumpDLInBufferQ[0]->BufLen, 1, pSPEL->mfpInDL);
            pSPEL->DumpMutexLock();
            if (pSPEL->hDumpThread != 0) {
                free(pSPEL->mDumpDLInBufferQ[0]->pBufBase);
                delete pSPEL->mDumpDLInBufferQ[0];
                pSPEL->mDumpDLInBufferQ.removeAt(0);
            }
            pSPEL->DumpMutexUnlock();
        }

        if (pSPEL->mDumpDLOutBufferQ.size() > 0) {
            fwrite(pSPEL->mDumpDLOutBufferQ[0]->pBufBase, pSPEL->mDumpDLOutBufferQ[0]->BufLen, 1, pSPEL->mfpOutDL);
            pSPEL->DumpMutexLock();
            if (pSPEL->hDumpThread != 0) {
                free(pSPEL->mDumpDLOutBufferQ[0]->pBufBase);
                delete pSPEL->mDumpDLOutBufferQ[0];
                pSPEL->mDumpDLOutBufferQ.removeAt(0);
            }
            pSPEL->DumpMutexUnlock();
        }

        if (pSPEL->mDumpULInBufferQ.size() > 0) {
            fwrite(pSPEL->mDumpULInBufferQ[0]->pBufBase, pSPEL->mDumpULInBufferQ[0]->BufLen, 1, pSPEL->mfpInUL);
            pSPEL->DumpMutexLock();
            if (pSPEL->hDumpThread != 0) {
                free(pSPEL->mDumpULInBufferQ[0]->pBufBase);
                delete pSPEL->mDumpULInBufferQ[0];
                pSPEL->mDumpULInBufferQ.removeAt(0);
            }
            pSPEL->DumpMutexUnlock();
        }

        if (pSPEL->mDumpULOutBufferQ.size() > 0) {
            fwrite(pSPEL->mDumpULOutBufferQ[0]->pBufBase, pSPEL->mDumpULOutBufferQ[0]->BufLen, 1, pSPEL->mfpOutUL);
            pSPEL->DumpMutexLock();
            if (pSPEL->hDumpThread != 0) {
                free(pSPEL->mDumpULOutBufferQ[0]->pBufBase);
                delete pSPEL->mDumpULOutBufferQ[0];
                pSPEL->mDumpULOutBufferQ.removeAt(0);
            }
            pSPEL->DumpMutexUnlock();
        }

        if (pSPEL->mDumpEPLBufferQ.size() > 0) {
            fwrite(pSPEL->mDumpEPLBufferQ[0]->pBufBase, pSPEL->mDumpEPLBufferQ[0]->BufLen, 1, pSPEL->mfpEPL);
            pSPEL->DumpMutexLock();
            //            ALOGD("DumpThread %p, %p",pSPEL->mDumpEPLBufferQ[0],pSPEL->mDumpEPLBufferQ[0]->pBufBase);
            if (pSPEL->hDumpThread != 0) {
                free(pSPEL->mDumpEPLBufferQ[0]->pBufBase);
                delete pSPEL->mDumpEPLBufferQ[0];
                pSPEL->mDumpEPLBufferQ.removeAt(0);
            }
            pSPEL->DumpMutexUnlock();
        }
    }
    AL_UNLOCK(pSPEL->mDumpExitMutex);
    AL_SIGNAL(pSPEL->mDumpExitMutex);
    ALOGD("DumpThread exit!!");
    pthread_exit(NULL);
    return 0;
}

bool SPELayer::CreateDumpThread() {
#if defined(PC_EMULATION)
    hDumpThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DumpThread, this, 0, 0);
    if (hDumpThread == 0) { return false; }
    return true;
#else
    //create PCM data dump thread here
    int ret;
    ret = pthread_create(&hDumpThread, NULL, DumpThread, (void *)this);
    if (ret != 0) { return false; }

    ALOGD("-CreateDumpThread \n");
    return true;
#endif

}

void SPELayer::Dump_Enalbe_Check(void) {
    int ret;
    char Buf[10];
    snprintf(Buf, sizeof(Buf), "%d.pcm", DumpFileNum);
    char value[PROPERTY_VALUE_MAX];
    char value1[PROPERTY_VALUE_MAX];

    //Dump_PCM_In check
    property_get("vendor.SPEIn.pcm.dump", value, "0");
    int bflag = atoi(value);
    if (bflag) {
        if (hDumpThread == 0) {
            CreateDumpThread();
        }

        //uplink
        String8 DumpFileNameUpIn;
        DumpFileNameUpIn.appendFormat("%s%s%s", audio_dump_path, "SPEIn_Uplink", Buf);
        ret = checkAndCreateDirectory(DumpFileNameUpIn);
        if (ret < 0) {
            ALOGE("SPELayer::Dump_PCM_In UPLINK checkAndCreateDirectory() fail!!!");
        } else {
            if (mfpInUL == NULL) {
                mfpInUL = fopen(DumpFileNameUpIn, "ab+");
                if (mfpInUL == NULL) {
                    ALOGD("open  SPEIn_Uplink.pcm fail");
                } else {
                    ALOGD("open SPEIn_Uplink.pcm");
                }
            }
        }

        //downlink
        String8 DumpFileNameDownIn;
        DumpFileNameDownIn.appendFormat("%s%s%s", audio_dump_path, "SPEIn_Downlink", Buf);
        ret = checkAndCreateDirectory(DumpFileNameDownIn);
        if (ret < 0) {
            ALOGE("SPELayer::Dump_PCM_In DOWNLINK checkAndCreateDirectory() fail!!!");
        } else {
            if (mfpInDL == NULL) {
                mfpInDL = fopen(DumpFileNameDownIn, "ab+");
                if (mfpInDL == NULL) {
                    ALOGD("open  SPEIn_Downlink.pcm fail");
                } else {
                    ALOGD("open SPEIn_Downlink.pcm");
                }
            }
        }
    }

    //Dump_PCM_Process check
    property_get("vendor.SPE.pcm.dump", value, "0");
    bflag = atoi(value);
    if (bflag) {
        //if(hDumpThread == NULL)
        //CreateDumpThread();

        //uplink
        String8 DumpFileNameUpPro;
        DumpFileNameUpPro.appendFormat("%s%s%s", audio_dump_path, "SPE_Uplink", Buf);
        ret = checkAndCreateDirectory(DumpFileNameUpPro);
        if (ret < 0) {
            ALOGE("SPELayer::Dump_PCM_Process UPLINK checkAndCreateDirectory() fail!!!");
        } else {
            if (mfpProcessedUL == NULL) {
                mfpProcessedUL = fopen(DumpFileNameUpPro, "ab+");
                if (mfpProcessedUL == NULL) {
                    ALOGD("open  SPE_Uplink.pcm fail");
                } else {
                    ALOGD("open SPE_Uplink.pcm");
                }
            }
        }

        //downlink
        String8 DumpFileNameDownPro;
        DumpFileNameDownPro.appendFormat("%s%s%s", audio_dump_path, "SPE_Downlink", Buf);
        ret = checkAndCreateDirectory(DumpFileNameDownPro);
        if (ret < 0) {
            ALOGE("SPELayer::Dump_PCM_Process DOWNLINK checkAndCreateDirectory() fail!!!");
        } else {
            if (mfpProcessedDL == NULL) {
                mfpProcessedDL = fopen(DumpFileNameDownPro, "ab+");
                if (mfpProcessedDL == NULL) {
                    ALOGD("open  SPE_Downlink.pcm fail");
                } else {
                    ALOGD("open SPE_Downlink.pcm");
                }
            }
        }
    }

    //Dump_PCM_Out check
    property_get("vendor.SPEOut.pcm.dump", value, "0");
    bflag = atoi(value);
    if (bflag) {
        if (hDumpThread == 0) {
            CreateDumpThread();
        }

        //uplink
        String8 DumpFileNameUpOut;
        DumpFileNameUpOut.appendFormat("%s%s%s", audio_dump_path, "SPEOut_Uplink", Buf);
        ret = checkAndCreateDirectory(DumpFileNameUpOut);
        if (ret < 0) {
            ALOGE("SPELayer::Dump_PCM_Out UPLINK checkAndCreateDirectory() fail!!!");
        } else {
            if (mfpOutUL == NULL) {
                mfpOutUL = fopen(DumpFileNameUpOut, "ab+");
                if (mfpOutUL == NULL) {
                    ALOGD("open  SPEOut_Uplink.pcm fail");
                } else {
                    ALOGD("open SPEOut_Uplink.pcm");
                }
            }
        }

        //downlink
        String8 DumpFileNameDownOut;
        DumpFileNameDownOut.appendFormat("%s%s%s", audio_dump_path, "SPEOut_Downlink", Buf);
        ret = checkAndCreateDirectory(DumpFileNameDownOut);
        if (ret < 0) {
            ALOGE("SPELayer::Dump_PCM_Out DOWNLINK checkAndCreateDirectory() fail!!!");
        } else {
            if (mfpOutDL == NULL) {
                mfpOutDL = fopen(DumpFileNameDownOut, "ab+");
                if (mfpOutDL == NULL) {
                    ALOGD("open  SPEOut_Downlink.pcm fail");
                } else {
                    ALOGD("open SPEOut_Downlink.pcm");
                }
            }
        }
    }

    //Dump_EPL check
    property_get("vendor.SPE_EPL", value, "0");
    property_get("vendor.streamin.epl.dump", value1, "0");
    bflag = atoi(value);
    int bflag1 = atoi(value1);
    if (bflag || bflag1) {
        EPLDebugEnable = true;
        if (hDumpThread == 0) {
            CreateDumpThread();
        }

        snprintf(Buf, sizeof(Buf), "%d.EPL", DumpFileNum);
        String8 DumpFileNameEPL;
        DumpFileNameEPL.appendFormat("%s%s%s", audio_dump_path, "SPE_EPL", Buf);

        ALOGD("Dump_EPL DumpFileNameEPL = %s", DumpFileNameEPL.string());

        ret = checkAndCreateDirectory(DumpFileNameEPL);
        if (ret < 0) {
            ALOGE("SPELayer::Dump_EPL checkAndCreateDirectory() fail!!!");
        } else {
            if (mfpEPL == NULL) {
                mfpEPL = fopen(DumpFileNameEPL, "ab+");
                if (mfpEPL == NULL) {
                    ALOGD("open SPE_EPL.EPL fail");
                } else {
                    ALOGD("open SPE_EPL.EPL");
                }
            }
        }
    } else {
        EPLDebugEnable = false;
    }
}

void SPELayer::Dump_PCM_In(SPE_DATA_DIRECTION dir, void *buffer, int bytes) {
    if (hDumpThread == 0) {
        return;
    }
    if (dir == UPLINK) {
        if (mfpInUL != NULL) {
            BufferInfo *newInBuffer = new BufferInfo;
            newInBuffer->pBufBase = (short *) malloc(bytes);
            ASSERT(newInBuffer->pBufBase != NULL);
            memcpy(newInBuffer->pBufBase, buffer, bytes);

            newInBuffer->BufLen = bytes;
            newInBuffer->pRead = newInBuffer->pBufBase;
            newInBuffer->pWrite = newInBuffer->pBufBase;
            DumpMutexLock();
            mDumpULInBufferQ.add(newInBuffer);
            DumpMutexUnlock();
        }
    } else {
        if (mfpInDL != NULL) {
            BufferInfo *newInBuffer = new BufferInfo;
            newInBuffer->pBufBase = (short *) malloc(bytes);
            ASSERT(newInBuffer->pBufBase != NULL);
            memcpy(newInBuffer->pBufBase, buffer, bytes);

            newInBuffer->BufLen = bytes;
            newInBuffer->pRead = newInBuffer->pBufBase;
            newInBuffer->pWrite = newInBuffer->pBufBase;
            DumpMutexLock();
            mDumpDLInBufferQ.add(newInBuffer);
            DumpMutexUnlock();
        }
    }

}

void SPELayer::Dump_PCM_Process(SPE_DATA_DIRECTION dir, void *buffer, int bytes) {
    if (dir == UPLINK) {
        if (mfpProcessedUL != NULL) {
            fwrite(buffer, bytes, 1, mfpProcessedUL);
        }
    } else {
        if (mfpProcessedDL != NULL) {
            fwrite(buffer, bytes, 1, mfpProcessedDL);
        }
    }
}

void SPELayer::Dump_PCM_Out(SPE_DATA_DIRECTION dir, void *buffer, int bytes) {
    if (hDumpThread == 0) {
        return;
    }
    if (dir == UPLINK) {
        if (mfpOutUL != NULL) {
            BufferInfo *newInBuffer = new BufferInfo;
            newInBuffer->pBufBase = (short *) malloc(bytes);
            ASSERT(newInBuffer->pBufBase != NULL);
            memcpy(newInBuffer->pBufBase, buffer, bytes);

            newInBuffer->BufLen = bytes;
            newInBuffer->pRead = newInBuffer->pBufBase;
            newInBuffer->pWrite = newInBuffer->pBufBase;
            DumpMutexLock();
            mDumpULOutBufferQ.add(newInBuffer);
            DumpMutexUnlock();
        }
    } else {
        if (mfpOutDL != NULL) {
            BufferInfo *newInBuffer = new BufferInfo;
            newInBuffer->pBufBase = (short *) malloc(bytes);
            ASSERT(newInBuffer->pBufBase != NULL);
            memcpy(newInBuffer->pBufBase, buffer, bytes);

            newInBuffer->BufLen = bytes;
            newInBuffer->pRead = newInBuffer->pBufBase;
            newInBuffer->pWrite = newInBuffer->pBufBase;
            DumpMutexLock();
            mDumpDLOutBufferQ.add(newInBuffer);
            DumpMutexUnlock();
        }
    }
}

void SPELayer::Dump_EPL(void *buffer, int bytes) {
    if (hDumpThread == 0) {
        return;
    }
    if (mfpEPL != NULL) {
        BufferInfo *newInBuffer = new BufferInfo;
        newInBuffer->pBufBase = (short *) malloc(bytes);
        ASSERT(newInBuffer->pBufBase != NULL);
        memcpy(newInBuffer->pBufBase, buffer, bytes);

        newInBuffer->BufLen = bytes;
        newInBuffer->pRead = newInBuffer->pBufBase;
        newInBuffer->pWrite = newInBuffer->pBufBase;
        DumpMutexLock();
        //        ALOGD("Dump_EPL %p, %p",newInBuffer,newInBuffer->pBufBase);
        mDumpEPLBufferQ.add(newInBuffer);
        DumpMutexUnlock();
    }
}

void SPELayer::EPLTransVMDump() {

    char value[PROPERTY_VALUE_MAX];
    property_get("vendor.APVM.dump", value, "0");
    int bflag = atoi(value);
    if (bflag || mVMDumpEnable) {
        int ret;
        String8 filename;
        filename.appendFormat("%s%s", audio_dump_path, "SPE.VM");
        if (bflag) {
            audio_strncpy(mVMDumpFileName, filename.string(), VM_DUMP_FILE_NAME_SIZE);
        }
        if (mVMDumpFileName[0] == '\0') {
            ALOGE("no mVMDumpFileName name?");
        }

        ret = checkAndCreateDirectory(mVMDumpFileName);
        if (ret < 0) {
            ALOGE("EPLTransVMDump checkAndCreateDirectory() fail!!!");
        } else {
            if (mfpVM == NULL && mVMDumpFileName[0] != '\0') {
                mfpVM = fopen(mVMDumpFileName, "ab+");
            }
        }

        if (mfpVM != NULL) {
            if (mSph_Enh_ctrl.sample_rate == 48000) {
                /*        memcpy(mVM, mSph_Enh_ctrl.EPL_buffer, RecBufSize48K20ms*2*sizeof(short));
                        mVM[MaxVMSize-2] = mSph_Enh_ctrl.EPL_buffer[VMAGC1];
                        mVM[MaxVMSize-1] = mSph_Enh_ctrl.EPL_buffer[VMAGC2];
                */
                ALOGD("EPLTransVMDump 48k write to /data/vendor/audiohal/");
                for (int i = 0; i < MaxVMSize; i++) {
                    if (i == (MaxVMSize - 2)) {
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[VMAGC1];
                    } else if (i == (MaxVMSize - 1)) {
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[VMAGC2];
                    } else {
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[i];
                    }
                }
                //                ALOGE("EPLTransVMDump write to /data/vendor/audiohal");
                fwrite(mVM, MaxVMSize * sizeof(short), 1, mfpVM);
            } else { //suppose only 16K
                /*        memcpy(mVM, &mSph_Enh_ctrl.EPL_buffer[160*4], 320*2*sizeof(short));
                        mVM[640] = mSph_Enh_ctrl.EPL_buffer[VMAGC1];
                        mVM[641] = mSph_Enh_ctrl.EPL_buffer[VMAGC2];
                */
                ALOGD("EPLTransVMDump 16k write to /data/vendor/audiohal");
                for (int i = 0; i < 642; i++) { //320*2+2
                    if (i == 640) {
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[VMAGC1];
                    } else if (i == 641) {
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[VMAGC2];
                    } else {
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[640 + i];
                    }
                }
                fwrite(mVM, 642 * sizeof(short), 1, mfpVM);
            }
        } else {
            ALOGD("open vendor.APVM.dump fail");
        }
    }
}

void SPELayer::SetVMDumpEnable(bool bEnable) {
    ALOGD("%s(), %d", __FUNCTION__, bEnable);
    mVMDumpEnable = bEnable;
}

void SPELayer::SetVMDumpFileName(const char *VMFileName) {
    ALOGD("%s()+, VMFileName=%s", __FUNCTION__, VMFileName);
    audio_strncpy(mVMDumpFileName, VMFileName, VM_DUMP_FILE_NAME_SIZE);
    ALOGD("%s()-, VMFileName=%s, mVMDumpFileName=%s", __FUNCTION__, VMFileName, mVMDumpFileName);
}

// ----------------------------------------------------------------------------
}

