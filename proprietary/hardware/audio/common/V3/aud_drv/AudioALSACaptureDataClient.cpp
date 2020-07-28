#include "AudioALSACaptureDataClient.h"

#include "AudioUtility.h"

#include "AudioType.h"
#include <AudioLock.h>

#include "AudioALSACaptureDataProviderBase.h"
#include "AudioALSAHardware.h"
#include "AudioALSAHardwareResourceManager.h"
#include "AudioVolumeFactory.h"
#include "SpeechEnhancementController.h"

#include "audio_custom_exp.h"

//BesRecord+++
#include "AudioCustParamClient.h"
#if (defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT) && (MTK_AUDIO_TUNING_TOOL_V2_PHASE >= 2))
#include "AudioParamParser.h"
#include <string>
#endif
//BesRecord---
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#ifdef MTK_LATENCY_DETECT_PULSE
#include "AudioDetectPulse.h"
#endif

#define LOG_TAG "AudioALSACaptureDataClient"


namespace android {
#define MTK_STREAMIN_VOLUEM_MAX (0x1000)
#define MTK_STREAMIN_VOLUME_VALID_BIT (12)

//BesRecord+++
#define VOICE_RECOGNITION_RECORD_SAMPLE_RATE (16000)
#define HD_RECORD_SAMPLE_RATE (48000)
#define NORMAL_RECORDING_DEFAULT_MODE    (1)
#define VOICE_REC_RECORDING_DEFAULT_MODE (0)
#define VOICE_UnLock_RECORDING_DEFAULT_MODE (6)
#define UNPROCESSED_MODE (26)
#define FAST_CAPTURE_MODE (28)
#define SRC_DROP_DATA
#define ECHOREF_TIME_OFFSET (-4)
#define DROP_HW_PALSE_TIMES (1)
#ifndef DROP_MS_AFTER_BESRECORD_PROCESS
#define DROP_MS_AFTER_BESRECORD_PROCESS 25
#endif
#define DROP_MS_FOR_UNPROCESSED_AUDIO_SOURCE 260

//BesRecord---

/*==============================================================================
 *                     Constant
 *============================================================================*/

static const uint32_t kClientBufferSize = 0x8000; // 32k

/* Audio Type Declaration */
#define VOIP_AUDIO_TYPE             "VoIP"
#define VOIPDMNR_AUDIO_TYPE         "VoIPDMNR"
#define VOIPGENERAL_AUDIO_TYPE      "VoIPGeneral"
#define RECORD_AUDIO_TYPE           "Record"
#define RECORDFIR_AUDIO_TYPE        "RecordFIR"
#define RECORDDMNR_AUDIO_TYPE       "RecordDMNR"

/* VoIP Categories Decleration */
#define VOIP_HANDSET_DMNR_PATH      "Profile,Handset 2mic NR"
#define VOIP_HANDSET_NO_DMNR_PATH   "Profile,Handset no 2mic NR"
#define VOIP_3POLE_HEADSET_PATH     "Profile,3-pole headset"
#define VOIP_4POLE_HEADSET_PATH     "Profile,4-pole headset"
#define VOIP_5POLE_HEADSET_PATH     "Profile,5-pole headset"
#define VOIP_5POLE_HEADSET_ANC_PATH "Profile,5-pole headset+ANC"
#define VOIP_BT_PATH                "Profile,BT earphone"
#define VOIP_BT_NSEC_OFF_PATH       "Profile,BT_NREC_Off"
#define VOIP_HANDSFREE_NR_PATH      "Profile,Hands-free 1mic NR"
#define VOIP_HANDSFREE_NO_NR_PATH   "Profile,Hands-free no 1mic NR"
#define VOIP_COMMON_PATH            "CategoryLayer,Common"
#define VOIP_NO_DMNR_PATH           ""

/* VoIP Parameter Decleration */
#define VOIP_PARAM                  "voip_mode_para"
#define VOIP_IN_FIR_PARAM           "sph_in_fir"
#define VOIP_OUT_FIR_PARAM          "sph_out_fir"
#define VOIP_DMNR_PARAM             "dmnr_para"

/* VoIPGeneral Category & Parameter Decleration*/
#define VOIPGENERAL_COMMON_PATH     "CategoryLayer,Common"
#define VOIPGENERAL_PARAM_NAME      "voip_common_para"

/* Record Categories Decleration */
#define RECORD_HANDSET_PATH         "Profile,Handset"
#define RECORD_4POLE_HEADSET_PATH   "Profile,4-pole HS"
#define RECORD_5POLE_HEADSET_PATH   "Profile,5-pole HS"
#define RECORD_5POLE_HEADSET_ANC_PATH "Profile,5-pole HS+ANC"
#define RECORD_BT_PATH              "Profile,BT earphone"
#define RECORD_VR_PATH              "Application,VR"
#define RECORD_VOICE_UNLOCK_PATH    "Application,VoiceUnLk"
#define RECORD_ASR_PATH             "Application,ASR"
#define RECORD_SND_REC_NORMAL_PATH  "Application,SndRecNormal"
#define RECORD_SND_REC_LECTURE_PATH "Application,SndRecLecture"
#define RECORD_SND_REC_MEETING_PATH "Application,SndRecMeeting"
#define RECORD_CAM_REC_NORMAL_PATH  "Application,CamRecNormal"
#define RECORD_CAM_REC_MEETING_PATH "Application,CamRecMeeting"
#define RECORD_CUSTOMIZATION2_PATH  "Application,Customization2"
#define RECORD_FAST_RECORD_PATH     "Application,FastRecord"
#define RECORD_UNPROCESSED_PATH     "Application,Unprocessed"
#define RECORD_NO_DMNR_PATH         ""

/* Record param */
#define RECORD_PARAM                "record_mode_para"
#define RECORD_IN_FIR1_PARAM        "sph_in_fir1"
#define RECORD_IN_FIR2_PARAM        "sph_in_fir2"
#define RECORD_DMNR_PARAM           "dmnr_para"

/* Feature options */
#define VOIP_NORMAL_DMNR_SUPPORT_FO     "VIR_VOIP_NORMAL_DMNR_SUPPORT"
#define VOIP_HANDSFREE_DMNR_SUPPORT_FO  "VIR_VOIP_HANDSFREE_DMNR_SUPPORT"

/* Key of parameters */
static String8 keyANC_runing = String8("ANC_running");

//debug++
static bool bTempDebug = false;
//debug--
/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioALSACaptureDataClient::AudioALSACaptureDataClient(AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target) :
    mCaptureDataProvider(pCaptureDataProvider),
    mRawStartFrameCount(0),
    mAudioSpeechEnhanceInfoInstance(AudioSpeechEnhanceInfo::getInstance()),
    mStreamAttributeSource(mCaptureDataProvider->getStreamAttributeSource()),
    mStreamAttributeTarget(stream_attribute_target),
    mBliSrc(NULL),
    mMicMute(false),
    mMuteTransition(false),
    mSPELayer(NULL),
    mAudioALSAVolumeController(AudioVolumeFactory::CreateAudioVolumeController()),
    mChannelRemixOp(CHANNEL_REMIX_NOP),
    mBesRecTuningEnable(false),
    //echoref+++
    mCaptureDataProviderEchoRef(NULL),
    mStreamAttributeSourceEchoRef(NULL),
    mStreamAttributeTargetEchoRef(NULL),
    mBliSrcEchoRef(NULL),
    mBliSrcEchoRefBesRecord(NULL)
    //echoref---
{
    ALOGD("%s()", __FUNCTION__);

    // init member struct
    memset((void *)&mEchoRefRawDataBuf, 0, sizeof(mEchoRefRawDataBuf));
    memset((void *)&mEchoRefSrcDataBuf, 0, sizeof(mEchoRefSrcDataBuf));

    // raw data
    memset((void *)&mRawDataBuf, 0, sizeof(mRawDataBuf));
    mRawDataBuf.pBufBase = new char[kClientBufferSize];
    mRawDataBuf.bufLen   = kClientBufferSize;
    mRawDataBuf.pRead    = mRawDataBuf.pBufBase;
    mRawDataBuf.pWrite   = mRawDataBuf.pBufBase;
    ASSERT(mRawDataBuf.pBufBase != NULL);

    // src data
    memset((void *)&mSrcDataBuf, 0, sizeof(mSrcDataBuf));
    mSrcDataBuf.pBufBase = new char[kClientBufferSize];
    mSrcDataBuf.bufLen   = kClientBufferSize;
    mSrcDataBuf.pRead    = mSrcDataBuf.pBufBase;
    mSrcDataBuf.pWrite   = mSrcDataBuf.pBufBase;
    ASSERT(mSrcDataBuf.pBufBase != NULL);

    // processed data
    memset((void *)&mProcessedDataBuf, 0, sizeof(mProcessedDataBuf));
    mProcessedDataBuf.pBufBase = new char[kClientBufferSize];
    mProcessedDataBuf.bufLen   = kClientBufferSize;
    mProcessedDataBuf.pRead    = mProcessedDataBuf.pBufBase;
    mProcessedDataBuf.pWrite   = mProcessedDataBuf.pBufBase;
    ASSERT(mProcessedDataBuf.pBufBase != NULL);

    //TODO: Sam, move here for temp
    //BesRecord+++
#if (!defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT) || (MTK_AUDIO_TUNING_TOOL_V2_PHASE == 1))
    memset((void *)&mBesRecordSceneTable, 0, sizeof(mBesRecordSceneTable));
    memset((void *)&mBesRecordParam, 0, sizeof(mBesRecordParam));

    mBesRecordModeIndex = -1;
    mBesRecordSceneIndex = -1;
#endif
    mBesRecordStereoMode = false;
    mBypassBesRecord = false;
    mNeedBesRecordSRC = false;
    mBliSrcHandler1 = NULL;
    mBliSrcHandler2 = NULL;
    mBesRecSRCSizeFactor = 1;
    mBesRecSRCSizeFactor2 = 1;
    dropBesRecordDataSize = 0;
    mFirstSRC = true;
    mFirstEchoSRC = true;
    mDropMs = 0;

    mSpeechProcessMode = SPE_MODE_REC;
    mVoIPSpeechEnhancementMask = mStreamAttributeTarget->BesRecord_Info.besrecord_dynamic_mask;

    //BesRecord Config
    mSPELayer = new SPELayer();
    if (!mSPELayer) {
        ALOGE("new SPELayer() FAIL");
        ASSERT(mSPELayer != NULL);
    }
    mAudioCustParamClient = NULL;
    mAudioCustParamClient = AudioCustParamClient::GetInstance();

    SetCaptureGain();
    ALOGD("%s(), besrecord_enable=%d, besrecord_scene=%d", __FUNCTION__, mStreamAttributeTarget->BesRecord_Info.besrecord_enable,
          mStreamAttributeTarget->BesRecord_Info.besrecord_scene);
    if (mStreamAttributeTarget->BesRecord_Info.besrecord_enable) {
        LoadBesRecordParams();

        mSPELayer->SetVMDumpEnable(mStreamAttributeTarget->BesRecord_Info.besrecord_tuningEnable || mStreamAttributeTarget->BesRecord_Info.besrecord_dmnr_tuningEnable);
        mSPELayer->SetVMDumpFileName(mStreamAttributeTarget->BesRecord_Info.besrecord_VMFileName);
        mSPELayer->SetPlatfromTimeOffset(ECHOREF_TIME_OFFSET); //Default -4ms EchoRef data

        CheckBesRecordBypass();
        CheckBesRecordMode();
        ConfigBesRecordParams();
        StartBesRecord();
        if (stream_attribute_target->BesRecord_Info.besrecord_voip_enable == true) {
            // drop data size need to align interrupt rate
            if (getLatencyTime() < DROP_MS_AFTER_BESRECORD_PROCESS) {
                mDropMs = (DROP_MS_AFTER_BESRECORD_PROCESS % getLatencyTime() == 0) ?  DROP_MS_AFTER_BESRECORD_PROCESS : ((DROP_MS_AFTER_BESRECORD_PROCESS / getLatencyTime()) + DROP_HW_PALSE_TIMES) * getLatencyTime();
            }
            dropBesRecordDataSize = (stream_attribute_target->sample_rate / 1000) * mDropMs *  stream_attribute_target->num_channels * audio_bytes_per_sample(stream_attribute_target->audio_format);
        } else {
            mDropMs = DROP_MS_AFTER_BESRECORD_PROCESS;
            dropBesRecordDataSize = (stream_attribute_target->sample_rate / 1000) * DROP_MS_AFTER_BESRECORD_PROCESS *  stream_attribute_target->num_channels * audio_bytes_per_sample(stream_attribute_target->audio_format);
        }

        ALOGD("sample rate = %d, drop ms = %d, channels = %d, byts per sample = %zu, dropBesRecordDataSize = %d\n",
              stream_attribute_target->sample_rate, DROP_MS_AFTER_BESRECORD_PROCESS, stream_attribute_target->num_channels, audio_bytes_per_sample(stream_attribute_target->audio_format), dropBesRecordDataSize);
    } else {
        if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_UNPROCESSED)
        {
            /* For unprocess test, we have to drop these glitch data to avoid the max RMS over the upper bound */
            dropBesRecordDataSize = (stream_attribute_target->sample_rate / 1000) * DROP_MS_FOR_UNPROCESSED_AUDIO_SOURCE *  stream_attribute_target->num_channels * audio_bytes_per_sample(stream_attribute_target->audio_format);
        }
        CheckBesRecordStereoModeEnable();
    }

    //BesRecord---
    //Android Native Preprocess effect +++
    mAudioPreProcessEffect = NULL;
    mAudioPreProcessEffect = new AudioPreProcess(mStreamAttributeTarget);
    if (!mAudioPreProcessEffect) {
        ALOGE("new mAudioPreProcessEffect() FAIL");
        ASSERT(mAudioPreProcessEffect != NULL);
    }
    CheckNativeEffect();
    //Android Native Preprocess effect ---

    // attach client to capture data provider
    ALOGV("mCaptureDataProvider=%p", mCaptureDataProvider);
    mCaptureDataProvider->configStreamAttribute(mStreamAttributeTarget); // config attribute for input device
    mCaptureDataProvider->attach(this); // mStreamAttributeSource will be updated when first client attached

    //assume starts after PCM open
    mSPELayer->SetUPLinkDropTime(CAPTURE_DROP_MS);

    // init SRC
    if (mStreamAttributeSource->sample_rate != mStreamAttributeTarget->sample_rate) {
        ALOGD("sample_rate: %d => %d, num_channels: %d => %d, audio_format: 0x%x => 0x%x",
              mStreamAttributeSource->sample_rate, mStreamAttributeTarget->sample_rate,
              mStreamAttributeSource->num_channels, mStreamAttributeSource->num_channels,
              mStreamAttributeSource->audio_format, mStreamAttributeTarget->audio_format);

        SRC_PCM_FORMAT  SrcFormat = mStreamAttributeTarget->audio_format == AUDIO_FORMAT_PCM_16_BIT ? SRC_IN_Q1P15_OUT_Q1P15 : SRC_IN_Q1P31_OUT_Q1P31;
        mBliSrc = newMtkAudioSrc(
                      mStreamAttributeSource->sample_rate, mStreamAttributeSource->num_channels,
                      mStreamAttributeTarget->sample_rate, mStreamAttributeSource->num_channels,
                      SrcFormat);
        mBliSrc->open();
    }
    if (mStreamAttributeTarget->BesRecord_Info.besrecord_enable) {
        //move CheckNeedBesRecordSRC to here for mStreamAttributeSource info
        CheckNeedBesRecordSRC();
    }

    CheckChannelRemixOp();
    //debug++
    bTempDebug = mAudioSpeechEnhanceInfoInstance->GetDebugStatus();
    //debug--
}

AudioALSACaptureDataClient::~AudioALSACaptureDataClient() {
    ALOGD("%s()", __FUNCTION__);

    //EchoRef+++
    if (mCaptureDataProviderEchoRef != NULL) {
        ALOGD("%s(), remove EchoRef data provider,mCaptureDataProviderEchoRef=%p", __FUNCTION__, mCaptureDataProviderEchoRef);
        mSPELayer->SetOutputStreamRunning(false, true);
        mCaptureDataProviderEchoRef->detach(this);
        if (mEchoRefRawDataBuf.pBufBase != NULL) { delete[] mEchoRefRawDataBuf.pBufBase; }

        if (mEchoRefSrcDataBuf.pBufBase != NULL) { delete[] mEchoRefSrcDataBuf.pBufBase; }
    }

    if (mBliSrcEchoRef != NULL) {
        mBliSrcEchoRef->close();
        deleteMtkAudioSrc(mBliSrcEchoRef);
        mBliSrcEchoRef = NULL;
    }

    if (mBliSrcEchoRefBesRecord != NULL) {
        mBliSrcEchoRefBesRecord->close();
        deleteMtkAudioSrc(mBliSrcEchoRefBesRecord);
        mBliSrcEchoRefBesRecord = NULL;
    }
    //EchoRef---

    mCaptureDataProvider->detach(this);

    if (mRawDataBuf.pBufBase != NULL) { delete[] mRawDataBuf.pBufBase; }

    if (mSrcDataBuf.pBufBase != NULL) { delete[] mSrcDataBuf.pBufBase; }

    if (mProcessedDataBuf.pBufBase != NULL) { delete[] mProcessedDataBuf.pBufBase; }

    if (mBliSrc != NULL) {
        mBliSrc->close();
        deleteMtkAudioSrc(mBliSrc);
        mBliSrc = NULL;
    }

    //TODO: Sam, add here for temp
    //BesRecord+++
    StopBesRecord();
    if (mBliSrcHandler1) {
        mBliSrcHandler1->close();
        deleteMtkAudioSrc(mBliSrcHandler1);
        mBliSrcHandler1 = NULL;
    }

    if (mBliSrcHandler2) {
        mBliSrcHandler2->close();
        deleteMtkAudioSrc(mBliSrcHandler2);
        mBliSrcHandler2 = NULL;
    }
    if (mSPELayer != NULL) { delete mSPELayer; }
    //BesRecord---

    //Android Native Preprocess effect +++
    if (mAudioPreProcessEffect != NULL) { delete mAudioPreProcessEffect; }
    //Android Native Preprocess effect ---

    ALOGD("-%s()", __FUNCTION__);
}


uint32_t AudioALSACaptureDataClient::copyCaptureDataToClient(RingBuf pcm_read_buf) {
    ALOGV("+%s()", __FUNCTION__);
    //debug++
    if (bTempDebug) {
        ALOGD("%s()+", __FUNCTION__);
    }
    //debug--

    AL_LOCK(mLock);

    uint32_t freeSpace = RingBuf_getFreeSpace(&mRawDataBuf);
    uint32_t dataSize = RingBuf_getDataCount(&pcm_read_buf);
    if (freeSpace < dataSize) {
        ALOGE("%s(), mRawDataBuf <= pcm_read_buf, freeSpace(%u) < dataSize(%u), buffer overflow!!", __FUNCTION__, freeSpace, dataSize);
        RingBuf_copyFromRingBuf(&mRawDataBuf, &pcm_read_buf, freeSpace);
    } else {
        RingBuf_copyFromRingBuf(&mRawDataBuf, &pcm_read_buf, dataSize);
    }

    // SRC
    uint32_t kNumRawData = RingBuf_getDataCount(&mRawDataBuf);    //mRawDataBuf has data with mStreamAttributeSource sample rate
    uint32_t num_free_space = RingBuf_getFreeSpace(&mSrcDataBuf);   //mSrcDataBuf has data with mStreamAttributeTarget sample rate

    //BesRecord PreProcess effect
    if (((mStreamAttributeTarget->BesRecord_Info.besrecord_enable) && !mBypassBesRecord)) {
        char *pRawDataLinearBuf = new char[kNumRawData];
        RingBuf_copyToLinear(pRawDataLinearBuf, &mRawDataBuf, kNumRawData);
        if (mStreamAttributeSource->audio_format != AUDIO_FORMAT_PCM_16_BIT) {
            kNumRawData = TransferFormat(pRawDataLinearBuf, mStreamAttributeSource->audio_format, AUDIO_FORMAT_PCM_16_BIT, kNumRawData);
        }
#ifdef MTK_LATENCY_DETECT_PULSE
        AudioDetectPulse::doDetectPulse(TAG_CAPTURE_DATA_CLIENT3, PULSE_LEVEL, 0, (void *)pRawDataLinearBuf,
                                        kNumRawData, mStreamAttributeSource->audio_format,
                                        mStreamAttributeSource->num_channels, mStreamAttributeSource->sample_rate);
#endif
        uint32_t ProcesseddataSize = kNumRawData;
        uint32_t SRC1outputLength = kNumRawData * mBesRecSRCSizeFactor;
        char *pSRC1DataLinearBuf = new char[SRC1outputLength];

        char *p_read = pRawDataLinearBuf;
        uint32_t num_raw_data_left = kNumRawData;
        uint32_t num_converted_data = SRC1outputLength;
        uint32_t consumed = num_raw_data_left;

        //transform data format to BesRecord needed
        if (mNeedBesRecordSRC && (mBliSrcHandler1 != 0)) {
            mBliSrcHandler1->process((int16_t *)p_read, &num_raw_data_left,
                                     (int16_t *)pSRC1DataLinearBuf, &num_converted_data);
            consumed -= num_raw_data_left;

            p_read += consumed;
            ALOGV("%s(), num_raw_data_left = %u, num_converted_data = %u",
                  __FUNCTION__, num_raw_data_left, num_converted_data);
#ifdef SRC_DROP_DATA
            //Drop first SRC data to let buffer align
            if ((mStreamAttributeTarget->BesRecord_Info.besrecord_voip_enable) && mFirstSRC) {
                mFirstSRC = false;
                delete[] pRawDataLinearBuf;
                delete[] pSRC1DataLinearBuf;
                AL_SIGNAL(mLock);
                AL_UNLOCK(mLock);

                return 0;
            }
#endif

            //ASSERT(num_raw_data_left == 0);
            if (num_raw_data_left > 0) {
                ALOGW("%s(), BesRecordSRC1 num_raw_data_left(%u) > 0", __FUNCTION__, num_raw_data_left);
            }

            ProcesseddataSize = BesRecordPreprocess(pSRC1DataLinearBuf, num_converted_data);

            if (ProcesseddataSize == 0) {
                delete[] pRawDataLinearBuf;
                delete[] pSRC1DataLinearBuf;
                AL_SIGNAL(mLock);
                AL_UNLOCK(mLock);

                ALOGD("-%s(), no BesRecordPreprocess data", __FUNCTION__);
                return 0;
            }

            //transform data format back to StreamAttribue needed after BesRecord process
            if (mBliSrcHandler2 != 0) {
                uint32_t SRC2outputLength = ProcesseddataSize * mBesRecSRCSizeFactor2;
                char *pSRC2DataLinearBuf = new char[SRC2outputLength];

                p_read = pSRC1DataLinearBuf;
                num_raw_data_left = ProcesseddataSize;
                consumed = ProcesseddataSize;

                mBliSrcHandler2->process((int16_t *)p_read, &num_raw_data_left,
                                         (int16_t *)pSRC2DataLinearBuf, &SRC2outputLength);

                consumed -= num_raw_data_left;

                p_read += consumed;
                ALOGV("%s(), num_raw_data_left = %u, SRC2outputLength = %u",
                      __FUNCTION__, num_raw_data_left, SRC2outputLength);

                //ASSERT(num_raw_data_left == 0);
                if (num_raw_data_left > 0) {
                    ALOGW("%s(), BesRecord1 SRC2 num_raw_data_left(%u) > 0", __FUNCTION__, num_raw_data_left);
                }

                if (num_free_space < SRC2outputLength) {
                    ALOGE("%s(), BesRecord1 SRC2outputLength <= mSrcDataBuf num_free_space, num_free_space(%u) < SRC2outputLength(%u), buffer overflow!!", __FUNCTION__, num_free_space, SRC2outputLength);
                    RingBuf_copyFromLinear(&mSrcDataBuf, pSRC2DataLinearBuf, num_free_space);
                } else {
                    RingBuf_copyFromLinear(&mSrcDataBuf, pSRC2DataLinearBuf, SRC2outputLength);
                }

                delete[] pSRC2DataLinearBuf;
            } else {
                if (num_free_space < ProcesseddataSize) {
                    ALOGE("%s(), BesRecord1 mProcessedDataBuf <= mSrcDataBuf, num_free_space(%u) < ProcesseddataSize(%u), buffer overflow!!", __FUNCTION__, num_free_space, ProcesseddataSize);
                    RingBuf_copyFromLinear(&mSrcDataBuf, pSRC1DataLinearBuf, num_free_space);
                } else {
                    RingBuf_copyFromLinear(&mSrcDataBuf, pSRC1DataLinearBuf, ProcesseddataSize);
                }
            }
        } else { //no need to tranform data format to BesRecord needed
            ProcesseddataSize = BesRecordPreprocess(pRawDataLinearBuf, kNumRawData);

            if (ProcesseddataSize == 0) {
                delete[] pRawDataLinearBuf;
                delete[] pSRC1DataLinearBuf;
                AL_SIGNAL(mLock);
                AL_UNLOCK(mLock);
                ALOGD("-%s(), no BesRecordPreprocess 2 data", __FUNCTION__);
                return 0;
            }

            //transform data format back to StreamAttribue needed after BesRecord processed
            if (mBliSrcHandler2 != 0) {
                uint32_t SRC2outputLength = ProcesseddataSize * mBesRecSRCSizeFactor2;
                char *pSRC2DataLinearBuf = new char[SRC2outputLength];

                p_read = pRawDataLinearBuf;
                num_raw_data_left = ProcesseddataSize;
                consumed = ProcesseddataSize;

                mBliSrcHandler2->process((int16_t *)p_read, &num_raw_data_left,
                                         (int16_t *)pSRC2DataLinearBuf, &SRC2outputLength);

                consumed -= num_raw_data_left;

                p_read += consumed;
                ALOGV("%s(), num_raw_data_left = %u, SRC2outputLength = %u",
                      __FUNCTION__, num_raw_data_left, SRC2outputLength);

                //ASSERT(num_raw_data_left == 0);
                if (num_raw_data_left > 0) {
                    ALOGW("%s(), BesRecord2 SRC2 num_raw_data_left(%u) > 0", __FUNCTION__, num_raw_data_left);
                }

                if (num_free_space < SRC2outputLength) {
                    ALOGE("%s(), BesRecord2 SRC2outputLength <= mSrcDataBuf num_free_space, num_free_space(%u) < SRC2outputLength(%u), buffer overflow!!", __FUNCTION__, num_free_space, SRC2outputLength);
                    RingBuf_copyFromLinear(&mSrcDataBuf, pSRC2DataLinearBuf, num_free_space);
                } else {
                    RingBuf_copyFromLinear(&mSrcDataBuf, pSRC2DataLinearBuf, SRC2outputLength);
                }

                delete[] pSRC2DataLinearBuf;
            } else {
                if (num_free_space < ProcesseddataSize) {
                    ALOGE("%s(), BesRecord2 mProcessedDataBuf <= mSrcDataBuf, num_free_space(%u) < ProcesseddataSize(%u), buffer overflow!!", __FUNCTION__, num_free_space, ProcesseddataSize);
                    RingBuf_copyFromLinear(&mSrcDataBuf, pRawDataLinearBuf, num_free_space);
                } else {
                    RingBuf_copyFromLinear(&mSrcDataBuf, pRawDataLinearBuf, ProcesseddataSize);
                }
            }
        }

        delete[] pRawDataLinearBuf;
        delete[] pSRC1DataLinearBuf;
    } else { //no need to do BesRecord PreProcess, transform data to mStreamAttributeTarget format
        if (mBliSrc == NULL) { // No need SRC
            if (mStreamAttributeTarget->audio_format != mStreamAttributeSource->audio_format) {
                char *pRawDataLinearBuf = new char[kNumRawData];
                RingBuf_copyToLinear(pRawDataLinearBuf, &mRawDataBuf, kNumRawData);
                kNumRawData = TransferFormat(pRawDataLinearBuf, mStreamAttributeSource->audio_format, mStreamAttributeTarget->audio_format, kNumRawData);
#ifdef MTK_LATENCY_DETECT_PULSE
                AudioDetectPulse::doDetectPulse(TAG_CAPTURE_DATA_CLIENT2, PULSE_LEVEL, 0, (void *)pRawDataLinearBuf,
                                                kNumRawData, mStreamAttributeSource->audio_format,
                                                mStreamAttributeSource->num_channels, mStreamAttributeSource->sample_rate);
#endif
                //ASSERT(num_free_space >= kNumRawData);
                if (num_free_space < kNumRawData) {
                    ALOGW("%s(), num_free_space(%u) < kNumRawData(%u)", __FUNCTION__, num_free_space, kNumRawData);
                    RingBuf_copyFromLinear(&mSrcDataBuf, pRawDataLinearBuf, num_free_space);
                } else {
                    RingBuf_copyFromLinear(&mSrcDataBuf, pRawDataLinearBuf, kNumRawData);
                }
                delete[] pRawDataLinearBuf;
            } else {
                if (num_free_space < kNumRawData) {
                    ALOGW("%s(), num_free_space(%u) < kNumRawData(%u)", __FUNCTION__, num_free_space, kNumRawData);
                    RingBuf_copyFromRingBuf(&mSrcDataBuf, &mRawDataBuf, num_free_space);
                } else {
                    RingBuf_copyFromRingBuf(&mSrcDataBuf, &mRawDataBuf, kNumRawData);
                }
            }
        } else { // Need SRC
            char *pRawDataLinearBuf = new char[kNumRawData];
            RingBuf_copyToLinear(pRawDataLinearBuf, &mRawDataBuf, kNumRawData);

            char *pSrcDataLinearBuf = new char[num_free_space];
            char *p_read = pRawDataLinearBuf;
            uint32_t num_raw_data_left = kNumRawData;
            uint32_t num_converted_data = num_free_space; // max convert num_free_space
            uint32_t consumed = num_raw_data_left;

            if (mStreamAttributeTarget->audio_format == AUDIO_FORMAT_PCM_16_BIT) {
                if (mStreamAttributeSource->audio_format != mStreamAttributeTarget->audio_format) { //8+24 to 16 bit
                    num_raw_data_left = TransferFormat(pRawDataLinearBuf, mStreamAttributeSource->audio_format, mStreamAttributeTarget->audio_format, num_raw_data_left);
                }
                mBliSrc->process((int16_t *)p_read, &num_raw_data_left,
                                 (int16_t *)pSrcDataLinearBuf, &num_converted_data);
            } else {
                mBliSrc->process((int32_t *)p_read, &num_raw_data_left,
                                 (int32_t *)pSrcDataLinearBuf, &num_converted_data);

                if (mStreamAttributeSource->audio_format != mStreamAttributeTarget->audio_format) { //8+24 to 24 bit
                    num_converted_data = TransferFormat(pSrcDataLinearBuf, mStreamAttributeSource->audio_format, mStreamAttributeTarget->audio_format, num_converted_data);
                }
            }

            consumed -= num_raw_data_left;
            p_read += consumed;
            ALOGV("%s(), num_raw_data_left = %u, num_converted_data = %u", __FUNCTION__, num_raw_data_left, num_converted_data);

            //ASSERT(num_raw_data_left == 0);
            if (num_raw_data_left > 0) {
                ALOGW("%s(), num_raw_data_left(%u) > 0", __FUNCTION__, num_raw_data_left);
            }


#ifdef MTK_LATENCY_DETECT_PULSE
            AudioDetectPulse::doDetectPulse(TAG_CAPTURE_DATA_CLIENT1, PULSE_LEVEL, 0, (void *)pSrcDataLinearBuf,
                                            num_converted_data, mStreamAttributeSource->audio_format,
                                            mStreamAttributeSource->num_channels, mStreamAttributeSource->sample_rate);
#endif
            RingBuf_copyFromLinear(&mSrcDataBuf, pSrcDataLinearBuf, num_converted_data);
            ALOGV("%s(), dataCount:%u", __FUNCTION__, RingBuf_getDataCount(&mSrcDataBuf));

            delete[] pRawDataLinearBuf;
            delete[] pSrcDataLinearBuf;
        }
    }

    freeSpace = RingBuf_getFreeSpace(&mProcessedDataBuf);
    dataSize = RingBuf_getDataCount(&mSrcDataBuf);
    uint32_t ProcessdataSize = dataSize;

    //android native effect, use the same sample rate as mStreamAttributeTarget
    //Native Preprocess effect+++
    if ((mAudioPreProcessEffect->num_preprocessors > 0) && (IsVoIPEnable() == false)) {
        char *pSrcDataLinearBuf = new char[dataSize];
        uint32_t native_processed_byte = 0;
        RingBuf_copyToLinear(pSrcDataLinearBuf, &mSrcDataBuf, dataSize);

        if (IsNeedChannelRemix()) {
            ProcessdataSize = ApplyChannelRemix((short *)pSrcDataLinearBuf, dataSize);
        }

        native_processed_byte = NativePreprocess(pSrcDataLinearBuf, ProcessdataSize);

        if (freeSpace < native_processed_byte) {
            ALOGE("%s(), NativeProcess mProcessedDataBuf <= mSrcDataBuf, freeSpace(%u) < native_processed size(%u), buffer overflow!!", __FUNCTION__, native_processed_byte, dataSize);
            RingBuf_copyFromLinear(&mProcessedDataBuf, pSrcDataLinearBuf, freeSpace);
        } else {
            RingBuf_copyFromLinear(&mProcessedDataBuf, pSrcDataLinearBuf, native_processed_byte);
        }

        delete[] pSrcDataLinearBuf;
    }
    //Native Preprocess effect---
    else {  //no need to do native effect, copy data from mSrcDataBuf to mProcessedDataBuf directly
        if (IsNeedChannelRemix()) {
            ApplyChannelRemixWithRingBuf(&mSrcDataBuf, &mProcessedDataBuf);
        } else {
            if (freeSpace < dataSize) {
                ALOGE("%s(), mProcessedDataBuf <= mSrcDataBuf, freeSpace(%u) < dataSize(%u), buffer overflow!!",
                      __FUNCTION__, freeSpace, dataSize);
                RingBuf_copyFromRingBuf(&mProcessedDataBuf, &mSrcDataBuf, freeSpace);
            } else {
                RingBuf_copyFromRingBuf(&mProcessedDataBuf, &mSrcDataBuf, dataSize);
            }
        }
    }

    AL_SIGNAL(mLock);
    AL_UNLOCK(mLock);

    ALOGV("-%s()", __FUNCTION__);
    //debug++
    if (bTempDebug) {
        ALOGD("%s()-", __FUNCTION__);
    }
    //debug--
    return 0;
}

ssize_t AudioALSACaptureDataClient::read(void *buffer, ssize_t bytes) {
    ALOGV("+%s(), bytes=%zu", __FUNCTION__, bytes);
    //debug++
    if (bTempDebug) {
        ALOGD("+%s(), bytes=%zu", __FUNCTION__, bytes);
    }
    //debug--

    char *pWrite = (char *)buffer;
    char *pStart = (char *)buffer;
    uint32_t RingBufferSize = 0;
    uint32_t ReadDataBytes = bytes;

    int TryCount = 8;


    // clean buffer
    memset(buffer, 0, bytes);

    do {
        AL_LOCK(mLock);
        //debug++
        if (bTempDebug) {
            ALOGD("%s(), TryCount=%d", __FUNCTION__, TryCount);
        }
        //debug--

        CheckNativeEffect();    //add here for alsaStreamIn lock holding
        CheckDynamicSpeechMask();

        if (dropBesRecordDataSize > 0) {
            /* Drop distortion data */
            RingBufferSize = RingBuf_getDataCount(&mProcessedDataBuf);
            if (RingBufferSize >= dropBesRecordDataSize) {
                // Drop dropBesRecordDataSize bytes from RingBuffer
                while (dropBesRecordDataSize > 0) {
                    uint32_t dropSize = dropBesRecordDataSize > ReadDataBytes ? ReadDataBytes : dropBesRecordDataSize;
                    RingBuf_copyToLinear((char *)pWrite, &mProcessedDataBuf, dropSize);
                    dropBesRecordDataSize -= dropSize;
                }
            } else {
                // Drop RingBufferSize from RingBuffer
                while (RingBufferSize > 0 && dropBesRecordDataSize > 0) {
                    uint32_t dropSize = dropBesRecordDataSize > ReadDataBytes ? ReadDataBytes : dropBesRecordDataSize;
                    dropSize = dropSize > RingBufferSize ? RingBufferSize : dropSize;
                    RingBuf_copyToLinear((char *)pWrite, &mProcessedDataBuf, dropSize);
                    RingBufferSize -= dropSize;
                    dropBesRecordDataSize -= dropSize;
                }
            }
        }

        if (dropBesRecordDataSize == 0) {
            RingBufferSize = RingBuf_getDataCount(&mProcessedDataBuf);
            if (RingBufferSize >= ReadDataBytes) { // ring buffer is enough, copy & exit
                RingBuf_copyToLinear((char *)pWrite, &mProcessedDataBuf, ReadDataBytes);
                ReadDataBytes = 0;
                AL_UNLOCK(mLock);
                break;
            } else { // ring buffer is not enough, copy all data
                RingBuf_copyToLinear((char *)pWrite, &mProcessedDataBuf, RingBufferSize);
                ReadDataBytes -= RingBufferSize;
                pWrite += RingBufferSize;
            }
        }

        // wait for new data
        if (AL_WAIT_MS(mLock, 300) != NO_ERROR) {
            ALOGW("%s(), waitRelative fail", __FUNCTION__);
            AL_UNLOCK(mLock);
            break;
        }

        AL_UNLOCK(mLock);
        TryCount--;
    } while (ReadDataBytes > 0 && (TryCount != 0 || dropBesRecordDataSize != 0));

    if (IsNeedApplyVolume()) {
        ApplyVolume(buffer, bytes);
    }

    if (ReadDataBytes > 0) {
        ALOGW("-%s(),(Not enough data) ReadDataBytes %d!!", __FUNCTION__, ReadDataBytes);
    } else {
        ALOGV("-%s(), ReadDataBytes=%d", __FUNCTION__, ReadDataBytes);
    }

    //debug++
    if (bTempDebug) {
        ALOGD("-%s(), ReadDataBytes=%d", __FUNCTION__, ReadDataBytes);
    }
    //debug--
    return bytes - ReadDataBytes;
}

#include <inttypes.h>

int AudioALSACaptureDataClient::getCapturePosition(int64_t *frames, int64_t *time) {
    if (mCaptureDataProvider == NULL || frames == NULL || time == NULL) {
        return -EINVAL;
    }

    /* Convert provider sample rate to streamin sample rate*/
    int ret = mCaptureDataProvider->getCapturePosition(frames, time);
    *frames = (*frames) * mStreamAttributeTarget->sample_rate / mStreamAttributeSource->sample_rate;

    /* Compensate drop ms to frames */
    *frames = (*frames) - ((int64_t)mDropMs * (int64_t)mStreamAttributeTarget->sample_rate / 1000);

    ALOGV("%s(), frames = %" PRIu64 ", tar sample = %d, src sample = %d", __FUNCTION__, *frames, mStreamAttributeTarget->sample_rate, mStreamAttributeSource->sample_rate);
    return ret;
}

uint32_t AudioALSACaptureDataClient::TransferFormat(char *linear_buffer, audio_format_t src_format, audio_format_t des_format, uint32_t bytes) {
    uint32_t *ptr_src_bit_r = (uint32_t *)linear_buffer;
    size_t src_bit = audio_bytes_per_sample(src_format);
    size_t des_bit = audio_bytes_per_sample(des_format);
    bool formatchanged = false;

    if (src_bit == 0 || des_bit == 0) {
        ALOGE("Cannot get bytes per sample for audio_format_t (src_format = %d, des_format = %d)\n", src_format, des_format);
        return 0;
    }

    if (des_format == AUDIO_FORMAT_PCM_24_BIT_PACKED) { //convert 8+24 to 24 bit
        char *ptr_des_bit_r = linear_buffer;
        int32_t *ptr_des_bit_w = 0;
        if (src_format == AUDIO_FORMAT_PCM_8_24_BIT) {
            for (uint32_t i = 1; i < (bytes / src_bit); i++) {
                ptr_des_bit_r = ptr_des_bit_r + 3;
                ptr_des_bit_w = (int32_t *)ptr_des_bit_r;
                memcpy(ptr_des_bit_w, (ptr_src_bit_r + i), sizeof(int));
            }
            formatchanged = true;
        }
    }
    if (des_format == AUDIO_FORMAT_PCM_16_BIT) { //convert 8+24 to 16 bit
        int16_t *ptr_des_bit_w = (int16_t *)linear_buffer;
        if (src_format == AUDIO_FORMAT_PCM_8_24_BIT) {
            for (uint32_t i = 0; i < (bytes / src_bit); i++) {
                *(ptr_des_bit_w + i) = (int16_t)(*(ptr_src_bit_r + i) >> 8);
            }
            formatchanged = true;
        }
    }
    ASSERT(formatchanged == true);
    ASSERT(src_bit != 0);

    bytes = bytes * des_bit / src_bit;
    return bytes;
}

bool AudioALSACaptureDataClient::IsNeedApplyVolume() {
    /* Only real input CaptureDataprovider need to apply volume for mic mute */
    switch (mCaptureDataProvider->getCaptureDataProviderType()) {
    case CAPTURE_PROVIDER_NORMAL:
    case CAPTURE_PROVIDER_BT_SCO:
    case CAPTURE_PROVIDER_BT_CVSD:
    case CAPTURE_PROVIDER_TDM_RECORD:
    case CAPTURE_PROVIDER_EXTERNAL:
        return true;
    default :
        return false;
    }

    return false;
}

status_t AudioALSACaptureDataClient::ApplyVolume(void *Buffer, uint32_t BufferSize) {
    // check if need apply mute
    if (mMicMute != mStreamAttributeTarget->micmute) {
        mMicMute =  mStreamAttributeTarget->micmute ;
        mMuteTransition = false;
    }

    if (mMicMute == true) {
        // do ramp down
        if (mMuteTransition == false) {
            uint32_t count = BufferSize >> 1;
            float Volume_inverse = (float)(MTK_STREAMIN_VOLUEM_MAX / count) * -1;
            short *pPcm = (short *)Buffer;
            int ConsumeSample = 0;
            int value = 0;
            while (count) {
                value = *pPcm * (MTK_STREAMIN_VOLUEM_MAX + (Volume_inverse * ConsumeSample));
                *pPcm = clamp16(value >> MTK_STREAMIN_VOLUME_VALID_BIT);
                pPcm++;
                count--;
                ConsumeSample ++;
                //ALOGD("ApplyVolume Volume_inverse = %f ConsumeSample = %d",Volume_inverse,ConsumeSample);
            }
            mMuteTransition = true;
        } else {
            memset(Buffer, 0, BufferSize);
        }
    } else if (mMicMute == false) {
        // do ramp up
        if (mMuteTransition == false) {
            uint32_t count = BufferSize >> 1;
            float Volume_inverse = (float)(MTK_STREAMIN_VOLUEM_MAX / count);
            short *pPcm = (short *)Buffer;
            int ConsumeSample = 0;
            int value = 0;
            while (count) {
                value = *pPcm * (Volume_inverse * ConsumeSample);
                *pPcm = clamp16(value >> MTK_STREAMIN_VOLUME_VALID_BIT);
                pPcm++;
                count--;
                ConsumeSample ++;
                //ALOGD("ApplyVolume Volume_inverse = %f ConsumeSample = %d",Volume_inverse,ConsumeSample);
            }
            mMuteTransition = true;
        }
    }
    return NO_ERROR;
}

void AudioALSACaptureDataClient::CheckChannelRemixOp(void) {
    uint32_t targetChannel = mStreamAttributeTarget->num_channels;
    uint32_t sourceChannel = mStreamAttributeSource->num_channels;

    if (mStreamAttributeTarget->BesRecord_Info.besrecord_enable) {
        if (targetChannel == 1) {
            mChannelRemixOp = CHANNEL_STEREO_DOWNMIX_L_ONLY;
        } else if (targetChannel == 2 && !mBesRecordStereoMode) {
            // speech enhancement output data is mono, need to convert to stereo
            mChannelRemixOp = CHANNEL_STEREO_CROSSMIX_L2R;
        } else {
            mChannelRemixOp = CHANNEL_REMIX_NOP;
        }
    } else {
        if (targetChannel == 1 && sourceChannel == 2) {
            /* For unprocessed audio source, the down channel should refer to L ch only, don't do the channel mix */
            if (mBesRecordStereoMode && mStreamAttributeTarget->input_source != AUDIO_SOURCE_UNPROCESSED) {
                mChannelRemixOp = CHANNEL_STEREO_DOWNMIX;
            } else {
                mChannelRemixOp = CHANNEL_STEREO_DOWNMIX_L_ONLY;
            }
        } else if (targetChannel == 2 && sourceChannel == 1) {
            mChannelRemixOp = CHANNEL_MONO_TO_STEREO;
        } else if (targetChannel == 2 && sourceChannel == 2) {
            if (mBesRecordStereoMode) {
                mChannelRemixOp = CHANNEL_REMIX_NOP;
            } else {
                mChannelRemixOp = CHANNEL_STEREO_CROSSMIX_L2R;
            }
        } else {
            mChannelRemixOp = CHANNEL_REMIX_NOP;
        }
    }

    ALOGD("%s(), ch %d->%d (mBesRecordStereoMode = %d, BesRecord enable = %d), ChannelRemixOp = %d",
          __FUNCTION__,
          sourceChannel,
          targetChannel,
          mBesRecordStereoMode,
          mStreamAttributeTarget->BesRecord_Info.besrecord_enable,
          mChannelRemixOp);
}

ssize_t AudioALSACaptureDataClient::ApplyChannelRemix(short *buffer, size_t bytes) {
    ssize_t remixSize = 0;
    uint32_t remixOp = mChannelRemixOp;
    int frameCount;

    if (remixOp == CHANNEL_STEREO_CROSSMIX_L2R) {
        frameCount = bytes >> 2;

        for (int i = 0; i < frameCount; i++) {
            *(buffer + 1) = *buffer;
            buffer += 2;
        }

        remixSize = bytes;
    } else if (remixOp == CHANNEL_STEREO_CROSSMIX_R2L) {
        frameCount = bytes >> 2;

        for (int i = 0; i < frameCount; i++) {
            *buffer = *(buffer + 1);
            buffer += 2;
        }

        remixSize = bytes;
    } else if (remixOp == CHANNEL_STEREO_DOWNMIX) {
        short mix;
        short *monoBuffer = buffer;
        frameCount = bytes >> 2;

        for (int i = 0; i < frameCount; i++) {
            mix = (*buffer + * (buffer + 1)) >> 1;
            *monoBuffer = mix;
            monoBuffer++;
            buffer += 2;
        }

        remixSize = bytes >> 1;
    } else if (remixOp == CHANNEL_STEREO_DOWNMIX_L_ONLY) {
        short *monoBuffer = buffer;
        frameCount = bytes >> 2;

        for (int i = 0; i < frameCount; i++) {
            *monoBuffer = *buffer;
            monoBuffer++;
            buffer += 2;
        }

        remixSize = bytes >> 1;
    } else if (remixOp == CHANNEL_STEREO_DOWNMIX_R_ONLY) {
        short *monoBuffer = buffer;
        frameCount = bytes >> 2;

        for (int i = 0; i < frameCount; i++) {
            *monoBuffer = *(buffer + 1);
            monoBuffer++;
            buffer += 2;
        }

        remixSize = bytes >> 1;
    } else if (remixOp == CHANNEL_MONO_TO_STEREO) {
        frameCount = bytes >> 1;
        short *monoBuffer = buffer + frameCount - 1;
        short *stereoBuffer = buffer + (frameCount * 2) - 1;
        short data;

        for (int i = 0; i < frameCount; i++) {
            data = *monoBuffer--;
            *stereoBuffer-- = data;
            *stereoBuffer-- = data;
        }

        remixSize = bytes << 1;
    }

    return remixSize;
}

ssize_t AudioALSACaptureDataClient::ApplyChannelRemixWithRingBuf(RingBuf *srcBuffer, RingBuf *dstBuffer) {
    ssize_t remixSize = 0;
    size_t dataSize = RingBuf_getDataCount(srcBuffer);
    size_t availSize = RingBuf_getFreeSpace(dstBuffer);
    size_t dataSizeAfterProcess;
    char *tempBuffer = NULL;
    size_t tempBufferSize;
    /*
    if (dataSize < 0)
        dataSize = 0;
    */
    if (mChannelRemixOp == CHANNEL_MONO_TO_STEREO) {
        dataSizeAfterProcess = dataSize << 1;
    } else if (mChannelRemixOp == CHANNEL_STEREO_DOWNMIX ||
               mChannelRemixOp == CHANNEL_STEREO_DOWNMIX_L_ONLY ||
               mChannelRemixOp == CHANNEL_STEREO_DOWNMIX_R_ONLY) {
        dataSizeAfterProcess = dataSize >> 1;
    } else {
        dataSizeAfterProcess = dataSize;
    }

    if (dataSizeAfterProcess > availSize) {
        ALOGE("%s() availSize(%zu) < dataSizeAfterProcess(%zu), buffer overflow!",
              __FUNCTION__, availSize, dataSizeAfterProcess);
        dataSizeAfterProcess = availSize;
        dataSizeAfterProcess &= 3;

        if (mChannelRemixOp == CHANNEL_MONO_TO_STEREO) {
            dataSize = dataSizeAfterProcess >> 1;
        } else if (mChannelRemixOp == CHANNEL_STEREO_DOWNMIX ||
                   mChannelRemixOp == CHANNEL_STEREO_DOWNMIX_L_ONLY ||
                   mChannelRemixOp == CHANNEL_STEREO_DOWNMIX_R_ONLY) {
            dataSize = dataSizeAfterProcess << 1;
        } else {
            dataSize = dataSizeAfterProcess;
        }
    }
    /*
        if (!dataSizeAfterProcess || dataSize < 0)
            return 0;
    */
    if (!dataSizeAfterProcess) {
        return 0;
    }

    tempBufferSize = (dataSizeAfterProcess > dataSize) ? dataSizeAfterProcess : dataSize;
    tempBuffer = new char[tempBufferSize];
    if (!tempBuffer) {
        return 0;
    }

    RingBuf_copyToLinear(tempBuffer, srcBuffer, dataSize);

    remixSize = ApplyChannelRemix((short *)tempBuffer, dataSize);

    RingBuf_copyFromLinear(dstBuffer, tempBuffer, remixSize);

    if (tempBuffer) {
        delete[] tempBuffer;
    }

    return remixSize;
}

bool AudioALSACaptureDataClient::IsLowLatencyCapture(void) {
    bool low_latency_on = false;
    bool voip_on = mStreamAttributeTarget->BesRecord_Info.besrecord_voip_enable;

#ifdef UPLINK_LOW_LATENCY
    if ((voip_on == false) &&
        (mStreamAttributeTarget->mAudioInputFlags & AUDIO_INPUT_FLAG_FAST)) {
        low_latency_on = true;
    }
#endif

    ALOGD("%s(), low_latency_on: %d, voip_on: %d", __FUNCTION__, low_latency_on, voip_on);
    return low_latency_on;
}

//TODO: Move here for temp solution, need to move to DataProcess
//BesRecord+++

void AudioALSACaptureDataClient::LoadBesRecordParams(void) {
#if ((!defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)) || (MTK_AUDIO_TUNING_TOOL_V2_PHASE == 1))
    uint8_t total_num_scenes = MAX_HD_REC_SCENES;
    ALOGD("+%s()", __FUNCTION__);
    // get scene table
    mAudioSpeechEnhanceInfoInstance->GetPreLoadBesRecordSceneTable(&mBesRecordSceneTable);

    // get hd rec param
    mAudioSpeechEnhanceInfoInstance->GetPreLoadBesRecordParam(&mBesRecordParam);

    //get VoIP param
    mAudioSpeechEnhanceInfoInstance->GetPreLoadAudioVoIPParam(&mVOIPParam);

    //get DMNR param
    if ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0) {
        mAudioSpeechEnhanceInfoInstance->GetPreLoadDualMicSpeechParam(&mDMNRParam);
    }

#if 0 // Debug print
    for (int i = 0; i < total_num_scenes; i++)
        for (int j = 0; j < NUM_HD_REC_DEVICE_SOURCE; j++) {
            ALOGD("scene_table[%d][%d] = %d", i, j, mBesRecordSceneTable.scene_table[i][j]);
        }
#endif
    ALOGD("-%s()", __FUNCTION__);
#endif
}

int AudioALSACaptureDataClient::SetCaptureGain(void) {

    if (mAudioALSAVolumeController != NULL) {
        mAudioALSAVolumeController->SetCaptureGain(mStreamAttributeTarget->audio_mode, mStreamAttributeTarget->input_source,
                                                   mStreamAttributeTarget->input_device, mStreamAttributeTarget->output_devices);
    }
    return 0;
}

int AudioALSACaptureDataClient::CheckBesRecordMode(void) {
#if ((!defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)) || (MTK_AUDIO_TUNING_TOOL_V2_PHASE == 1))
    //check the BesRecord mode and scene
    ALOGD("+%s()", __FUNCTION__);
    uint8_t modeIndex = 0;
    int32_t u4SceneIdx = 0;
    bool RecordModeGet = false;

    if (IsVoIPEnable() == true) {
        mSpeechProcessMode = SPE_MODE_VOIP;
    } else if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1) //MagiASR need AEC
               || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION2)) { //Normal Record + AEC
        mSpeechProcessMode = SPE_MODE_AECREC;
    } else {
        mSpeechProcessMode = SPE_MODE_REC;
    }

    u4SceneIdx = mStreamAttributeTarget->BesRecord_Info.besrecord_scene;
    mBesRecordStereoMode = false;

    //special input source case
    if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_RECOGNITION) || mStreamAttributeTarget->BesRecord_Info.besrecord_tuning16K) {
        ALOGD("voice recognition case");
        u4SceneIdx = VOICE_REC_RECORDING_DEFAULT_MODE;
    } else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_UNLOCK) {
        ALOGD("voice unlock case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE;
    } else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1) {
        ALOGD("CUSTOMIZATION1 case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE + 1;
    } else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION2) {
        ALOGD("CUSTOMIZATION2 case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE + 2;
    } else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION3) {
        ALOGD("CUSTOMIZATION3 case");
        u4SceneIdx = VOICE_UnLock_RECORDING_DEFAULT_MODE + 3;
    }

    //for BT record case, use specific params, whether what input source it is.
    if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET) {
        ALOGD("%s, is BT device", __FUNCTION__);
        if (mBesRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_BT_EARPHONE] != 0xFF) {
            modeIndex = mBesRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_BT_EARPHONE];
            ALOGD("%s, get specific BT modeIndex = %d", __FUNCTION__, modeIndex);
        } else {
            modeIndex = mBesRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_HANDSET];
        }
        mBesRecordSceneIndex = NORMAL_RECORDING_DEFAULT_MODE;
        RecordModeGet = true;
    }
    //Get the BesRecord Mode from the Record Scene
    else if ((u4SceneIdx >= 0) && (u4SceneIdx < MAX_HD_REC_SCENES)) {
        // get mode index
        if ((mBesRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET] != 0xFF)
            && (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_WIRED_HEADSET)) {
            modeIndex = mBesRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET];
            ALOGD("%s, is HEADSET device, u4SceneIdx=%d, modeIndex=%d", __FUNCTION__, u4SceneIdx, modeIndex);
        }
        // Handset Mic
        else if (mBesRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET] != 0xFF) {
            modeIndex = mBesRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET];
            ALOGD("%s, is HANDSET device, u4SceneIdx=%d, modeIndex=%d", __FUNCTION__, u4SceneIdx, modeIndex);
            /* only stereo flag is true, the stereo record preprocess is enabled */
            if ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0) {
                if (mBesRecordParam.hd_rec_map_to_stereo_flag[modeIndex] != 0) {
                    mBesRecordStereoMode = true;
                }
            }
        } else {
            ALOGD("%s, Handset mode index shoule not be -1, u4SceneIdx=%d, modeIndex=%d", __FUNCTION__, u4SceneIdx, modeIndex);
        }

#ifdef UPLINK_LOW_LATENCY
        if (mStreamAttributeTarget->mAudioInputFlags & AUDIO_INPUT_FLAG_FAST) { //(fast_capture)
            modeIndex = FAST_CAPTURE_MODE;
        }
#endif

        if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_UNPROCESSED) {
            modeIndex = UNPROCESSED_MODE;
        }

        // Debug print
        ALOGD("GetBesRecordModeInfo: map_fir_ch1=%d, map_fir_ch2=%d",
              mBesRecordParam.hd_rec_map_to_fir_for_ch1[modeIndex],
              mBesRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex]);

        mBesRecordSceneIndex = u4SceneIdx;
        RecordModeGet = true;
    }

    //No correct mode get, use default mode parameters
    if (RecordModeGet == false) {
        //ALOGD("%s, use default mode mdevices=%x, mAttribute.mPredevices=%x, mHDRecordSceneIndex = %d ", __FUNCTION__,
        //    mAttribute.mdevices, mAttribute.mPredevices, mHDRecordSceneIndex);
        ALOGD("%s, use default mode input_device=%x, mBesRecordSceneIndex = %d ", __FUNCTION__, mStreamAttributeTarget->input_device, mBesRecordSceneIndex);

        //can not get match HD record mode, use the default one
        // check if 3rd party camcorder
        if (mStreamAttributeTarget->input_source != AUDIO_SOURCE_CAMCORDER) { //not camcorder
#if 0   //TODO:Sam, if Capture handler is reopen when routing, no needed.
            if (mAttribute.mdevices != mAttribute.mPredevices) { //device changed, use previous scene (since scene not changed), (headset plug in/out during recording case)
                if (mHDRecordSceneIndex == -1) {
                    mHDRecordSceneIndex = NORMAL_RECORDING_DEFAULT_MODE;
                }
                if (mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET) { //headset
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HEADSET];
                } else {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HANDSET];
                }
            } else
#endif
            {
                if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) { //headset
                    modeIndex = mBesRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_HEADSET];
                } else { //default use internal one
                    modeIndex = mBesRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_HANDSET];
                }
                mBesRecordSceneIndex  = NORMAL_RECORDING_DEFAULT_MODE;
            }
        } else { //camcoder
            u4SceneIdx = mBesRecordSceneTable.num_voice_rec_scenes + NORMAL_RECORDING_DEFAULT_MODE;//1:cts verifier offset
#if 0 //TODO:Sam, if Capture handler is reopen when routing, no needed.
            if (mAttribute.mdevices != mAttribute.mPredevices) { //device changed, use previous scene
                if (mHDRecordSceneIndex == -1) {
                    mHDRecordSceneIndex = u4SceneIdx;
                }
                if (mAttribute.mdevices == AUDIO_DEVICE_IN_WIRED_HEADSET) { //headset
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HEADSET];
                } else {
                    modeIndex = mhdRecordSceneTable.scene_table[mHDRecordSceneIndex][HD_REC_DEVICE_SOURCE_HANDSET];
                }
            } else
#endif
            {
                if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) { //headset
                    modeIndex = mBesRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET];
                } else { //default use internal one
                    modeIndex = mBesRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET];
                }
                mBesRecordSceneIndex  = u4SceneIdx;
            }
        }

        //also need to configure the channel when use default mode
        /* only stereo flag is true, the stereo record is enabled */
        if ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0) {
            if (mStreamAttributeTarget->input_device ==  AUDIO_DEVICE_IN_BUILTIN_MIC) { //handset
                if (mBesRecordParam.hd_rec_map_to_stereo_flag[modeIndex] != 0) {
                    mBesRecordStereoMode = true;
                }
            }
        }

#ifdef UPLINK_LOW_LATENCY
        if (mStreamAttributeTarget->mAudioInputFlags & AUDIO_INPUT_FLAG_FAST) { //(fast_capture)
            modeIndex = FAST_CAPTURE_MODE;
        }
#endif

        if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_UNPROCESSED) {
            modeIndex = UNPROCESSED_MODE;
        }

    }

    ALOGD("-%s(), mBesRecordSceneIndex=%d, modeIndex=%d", __FUNCTION__, mBesRecordSceneIndex, modeIndex);
    mBesRecordModeIndex = modeIndex;
    return modeIndex;
#else
    return 0;
#endif
}

void AudioALSACaptureDataClient::CheckBesRecordStereoModeEnable() {
    if ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0) {
        if (mStreamAttributeTarget->input_device ==  AUDIO_DEVICE_IN_BUILTIN_MIC) {
            mBesRecordStereoMode = true;
            ALOGD("%s(), set the mBesRecordStereoMode = true\n", __FUNCTION__);
        }
    }

    if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_CALL) {
        /* For voice call incall record, the valid channel is stereo */
        mBesRecordStereoMode = true;
    }

    ALOGD("%s(), mBesRecordStereoMode = %d", __FUNCTION__, mBesRecordStereoMode);
}

void AudioALSACaptureDataClient::ConfigBesRecordParams(void) {
    ALOGD("+%s()", __FUNCTION__);

#if (defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT) && (MTK_AUDIO_TUNING_TOOL_V2_PHASE >= 2))
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return;
    }

    AppHandle *pAppHandle = appOps->appHandleGetInstance();
    AudioType *VoIPAudioType = appOps->appHandleGetAudioTypeByName(pAppHandle, VOIP_AUDIO_TYPE);
    AudioType *VoIPDmnrAudioType = appOps->appHandleGetAudioTypeByName(pAppHandle, VOIPDMNR_AUDIO_TYPE);
    AudioType *VoIPGeneralAudioType = appOps->appHandleGetAudioTypeByName(pAppHandle, VOIPGENERAL_AUDIO_TYPE);
    AudioType *RecordAudioType = appOps->appHandleGetAudioTypeByName(pAppHandle, RECORD_AUDIO_TYPE);
    AudioType *RecordFirAudioType = appOps->appHandleGetAudioTypeByName(pAppHandle, RECORDFIR_AUDIO_TYPE);
    AudioType *RecordDmnrAudioType = appOps->appHandleGetAudioTypeByName(pAppHandle, RECORDDMNR_AUDIO_TYPE);
    ParamUnit *pParamUnit;
    Param     *pSpeciParam;
    Param     *pCommonParam;
    Param     *pInFirParam;
    Param     *pOutFirParam;
    Param     *pInFir1Param;
    Param     *pInFir2Param;
    Param     *pDmnrParam;
    std::string categoryPath = "";
    uWord32 BesRecordEnhanceParas[EnhanceParasNum] = {0};
    Word16 BesRecordCompenFilter[CompenFilterNum] = {0};
    Word16 BesRecordDMNRParam[DMNRCalDataNum] = {0};
    bool bVoIPEnable = IsVoIPEnable();
    int RoutePath = GetBesRecordRoutePath();
    SPE_MODE mode = mSpeechProcessMode;

    mBesRecordStereoMode = false;

    // Get mSpeechProcessMode
    if (bVoIPEnable) {
        mode = SPE_MODE_VOIP;
        mSpeechProcessMode = mode;
    } else if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1) //MagiASR need AEC
               || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION2)) { //Normal Record + AEC
        mode = SPE_MODE_AECREC;
        mSpeechProcessMode = mode;
    } else {
        mode = SPE_MODE_REC;
        mSpeechProcessMode = mode;
    }

    if (bVoIPEnable) {
        // Get VoIP category path
        if (RoutePath == ROUTE_BT) {
            categoryPath = VOIP_BT_PATH;
        } else if (RoutePath == ROUTE_BT_NSEC_OFF_PATH) {
            categoryPath = VOIP_BT_NSEC_OFF_PATH;
        } else if (RoutePath == ROUTE_EARPHONE) {
            categoryPath = VOIP_3POLE_HEADSET_PATH;
        } else if (RoutePath == ROUTE_HEADSET) {
            switch (AudioALSAHardwareResourceManager::getInstance()->getNumOfHeadsetPole()) {
            case 4:
                categoryPath = VOIP_4POLE_HEADSET_PATH;
                break;
            case 5:
                if (AudioALSAHardware::GetInstance()->getParameters(keyANC_runing) == "ANC_running=true") {
                    categoryPath += "," VOIP_5POLE_HEADSET_ANC_PATH;
                } else {
                    categoryPath += "," VOIP_5POLE_HEADSET_PATH;
                }
                break;
            }
        } else if (RoutePath == ROUTE_SPEAKER) {
            if (appOps->appHandleIsFeatureOptionEnabled(pAppHandle, VOIP_HANDSFREE_DMNR_SUPPORT_FO) == 1) {
                categoryPath = VOIP_HANDSFREE_NR_PATH;
            } else {
                categoryPath = VOIP_HANDSFREE_NO_NR_PATH;
            }
        } else {
            if (appOps->appHandleIsFeatureOptionEnabled(pAppHandle, VOIP_NORMAL_DMNR_SUPPORT_FO) == 1) {
                categoryPath = VOIP_HANDSET_DMNR_PATH;
            } else {
                categoryPath = VOIP_HANDSET_NO_DMNR_PATH;
            }
        }
    } else {
        // Get Record category path
#ifdef UPLINK_LOW_LATENCY
        if (mStreamAttributeTarget->mAudioInputFlags & AUDIO_INPUT_FLAG_FAST) {
            categoryPath += RECORD_FAST_RECORD_PATH;
        } else
#endif
            if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_RECOGNITION) {
                categoryPath += RECORD_VR_PATH;
            } else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_UNLOCK) {
                categoryPath += RECORD_VOICE_UNLOCK_PATH;
                CheckBesRecordStereoModeEnable();
            } else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1) {
                categoryPath += RECORD_ASR_PATH;
            } else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION2) {
                categoryPath += RECORD_CUSTOMIZATION2_PATH;
            } else if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_UNPROCESSED) {
                categoryPath += RECORD_UNPROCESSED_PATH;
            } else {
                // Sound/Video recording, Get application from besrecord_scene
                switch (mStreamAttributeTarget->BesRecord_Info.besrecord_scene) {
                case 1:
                    categoryPath += RECORD_SND_REC_NORMAL_PATH;
                    CheckBesRecordStereoModeEnable();
                    break;
                case 2:
                    categoryPath += RECORD_SND_REC_MEETING_PATH;
                    CheckBesRecordStereoModeEnable();
                    break;
                case 3:
                    categoryPath += RECORD_SND_REC_LECTURE_PATH;
                    CheckBesRecordStereoModeEnable();
                    break;
                case 4:
                    categoryPath += RECORD_CAM_REC_NORMAL_PATH;
                    CheckBesRecordStereoModeEnable();
                    break;
                case 5:
                    categoryPath += RECORD_CAM_REC_MEETING_PATH;
                    break;
                default:
                    if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CAMCORDER) {
                        categoryPath += RECORD_CAM_REC_NORMAL_PATH;
                        CheckBesRecordStereoModeEnable();
                    } else {
                        categoryPath += RECORD_SND_REC_NORMAL_PATH;
                        CheckBesRecordStereoModeEnable();
                    }
                    break;
                }
            }

        if (RoutePath == ROUTE_BT) {
            categoryPath += "," RECORD_BT_PATH;
        } else if (RoutePath == ROUTE_HEADSET) {
            switch (AudioALSAHardwareResourceManager::getInstance()->getNumOfHeadsetPole()) {
            case 4:
                categoryPath += "," RECORD_4POLE_HEADSET_PATH;
                break;
            case 5:
                if (AudioALSAHardware::GetInstance()->getParameters(keyANC_runing) == "ANC_running=true") {
                    categoryPath += "," RECORD_5POLE_HEADSET_ANC_PATH;
                } else {
                    categoryPath += "," RECORD_5POLE_HEADSET_PATH;
                }
                break;
            }
        } else if (RoutePath == ROUTE_SPEAKER) {
            categoryPath += "," RECORD_HANDSET_PATH;
        } else {
            categoryPath += "," RECORD_HANDSET_PATH;
        }
    }

    ALOGD("%s(), categoryPath = %s, mBesRecordStereoMode = %d, input_source = %d, input_devices = %x, bVoIPEnable = %d, bypassDualProcess = %d",
          __FUNCTION__,
          categoryPath.c_str(),
          mBesRecordStereoMode,
          mStreamAttributeTarget->input_source,
          mStreamAttributeTarget->input_device,
          bVoIPEnable,
          mStreamAttributeTarget->BesRecord_Info.besrecord_bypass_dualmicprocess);

    appOps->audioTypeReadLock(VoIPAudioType, __FUNCTION__);
    appOps->audioTypeReadLock(VoIPDmnrAudioType, __FUNCTION__);
    appOps->audioTypeReadLock(VoIPGeneralAudioType, __FUNCTION__);
    appOps->audioTypeReadLock(RecordAudioType, __FUNCTION__);
    appOps->audioTypeReadLock(RecordFirAudioType, __FUNCTION__);
    appOps->audioTypeReadLock(RecordDmnrAudioType, __FUNCTION__);

    // set speech parameters+++
    if (mode == SPE_MODE_VOIP) {
        pParamUnit = appOps->audioTypeGetParamUnit(VoIPAudioType, categoryPath.c_str());
        pSpeciParam = appOps->paramUnitGetParamByName(pParamUnit, VOIP_PARAM);

    } else {
        // record case
        pParamUnit = appOps->audioTypeGetParamUnit(RecordAudioType, categoryPath.c_str());
        pSpeciParam = appOps->paramUnitGetParamByName(pParamUnit, RECORD_PARAM);
    }

    // For DMNR tuning, mode param 47[15] should be set 1
    if (mStreamAttributeTarget->BesRecord_Info.besrecord_dmnr_tuningEnable) {
        ((uWord32 *)pSpeciParam->data)[47] |= 0x8000;
    }

    //common parameters as same as VoIP's
    pParamUnit = appOps->audioTypeGetParamUnit(VoIPGeneralAudioType, VOIP_COMMON_PATH);
    pCommonParam = appOps->paramUnitGetParamByName(pParamUnit, VOIPGENERAL_PARAM_NAME);

    ASSERT(pSpeciParam != NULL && pCommonParam != NULL);
    ASSERT((pSpeciParam->arraySize + pCommonParam->arraySize) <= EnhanceParasNum);

    //pSpeciParam + pCommonParam
    memcpy(BesRecordEnhanceParas, (uWord32 *)pSpeciParam->data, pSpeciParam->arraySize * sizeof(uWord32));
    memcpy(&BesRecordEnhanceParas[pSpeciParam->arraySize], (uWord32 *)pCommonParam->data, pCommonParam->arraySize * sizeof(uWord32));

    //AEC off
    if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION) &&
        (mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_AECOn == false) &&
        (mStreamAttributeTarget->input_device != AUDIO_DEVICE_IN_ALL_SCO)) {
        BesRecordEnhanceParas[0] = 0;
        BesRecordEnhanceParas[1] = 479;
    }

    //NS off
    if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION) &&
        (mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_NSOn == false) &&
        (mStreamAttributeTarget->input_device != AUDIO_DEVICE_IN_ALL_SCO)) {
        //Dual mic
        BesRecordEnhanceParas[13] = 0;
        BesRecordEnhanceParas[14] = 0;
        //1-mic
        BesRecordEnhanceParas[4] &= ~1;
    }

    //AGC off
    if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION) &&
        (mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_AGCOn == false) &&
        (mStreamAttributeTarget->input_device != AUDIO_DEVICE_IN_ALL_SCO)) {
        BesRecordEnhanceParas[3] |= 0x4000;
    }
    mSPELayer->SetEnhPara(mode, BesRecordEnhanceParas);
    //speech parameters---

    //FIR parameters+++
    if (mStreamAttributeTarget->BesRecord_Info.besrecord_dmnr_tuningEnable == true) {
        pParamUnit = appOps->audioTypeGetParamUnit(VoIPAudioType, categoryPath.c_str());
        pInFirParam = appOps->paramUnitGetParamByName(pParamUnit, VOIP_IN_FIR_PARAM);

        ASSERT(pInFirParam != NULL);
        ASSERT((pInFirParam->arraySize + pInFirParam->arraySize) <= CompenFilterNum);

        memcpy(BesRecordCompenFilter, (Word16 *)pInFirParam->data, pInFirParam->arraySize * sizeof(Word16));  // UL1
        memcpy(&BesRecordCompenFilter[pInFirParam->arraySize], (Word16 *)pInFirParam->data, pInFirParam->arraySize * sizeof(Word16));  // UL2
    } else if (mode == SPE_MODE_VOIP) {
        pParamUnit = appOps->audioTypeGetParamUnit(VoIPAudioType, categoryPath.c_str());
        pInFirParam = appOps->paramUnitGetParamByName(pParamUnit, VOIP_IN_FIR_PARAM);
        pOutFirParam = appOps->paramUnitGetParamByName(pParamUnit, VOIP_OUT_FIR_PARAM);

        ASSERT(pInFirParam != NULL && pOutFirParam != NULL);
        ASSERT((pInFirParam->arraySize + pInFirParam->arraySize + pOutFirParam->arraySize) <= CompenFilterNum);

        // VoIP FIR parameter have 3 FIR parameter, but 1 FIR parameter work for SWIP limitation.
        memcpy(BesRecordCompenFilter, (Word16 *)pInFirParam->data, pInFirParam->arraySize * sizeof(Word16));  // UL1
        memcpy(&BesRecordCompenFilter[pInFirParam->arraySize], (Word16 *)pInFirParam->data, pInFirParam->arraySize * sizeof(Word16));  // UL2
        memcpy(&BesRecordCompenFilter[pInFirParam->arraySize * 2], (Word16 *)pOutFirParam->data, pOutFirParam->arraySize * sizeof(Word16)); // DL
    } else {
        // Record 2 FIR param is work
        pParamUnit = appOps->audioTypeGetParamUnit(RecordFirAudioType, categoryPath.c_str());
        pInFir1Param = appOps->paramUnitGetParamByName(pParamUnit, RECORD_IN_FIR1_PARAM);
        pInFir2Param = appOps->paramUnitGetParamByName(pParamUnit, RECORD_IN_FIR2_PARAM);

        ASSERT(pInFir1Param != NULL && pInFir2Param != NULL);
        ASSERT((pInFir1Param->arraySize + pInFir2Param->arraySize) <= CompenFilterNum);

        memcpy(BesRecordCompenFilter, (Word16 *)pInFir1Param->data, pInFir1Param->arraySize * sizeof(Word16));  // UL1
        memcpy(&BesRecordCompenFilter[pInFir1Param->arraySize], (Word16 *)pInFir2Param->data, pInFir2Param->arraySize * sizeof(Word16));  // UL2
    }

    mSPELayer->SetCompFilter(mode, BesRecordCompenFilter);
    //FIR parameters---

    //DMNR parameters+++
    if (((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0) && (mStreamAttributeTarget->BesRecord_Info.besrecord_bypass_dualmicprocess == false)) {
        //DMNR parameters
        //google default input source AUDIO_SOURCE_VOICE_RECOGNITION not using DMNR (on/off by parameters)
        if (((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_RECOGNITION) || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1)
             || mStreamAttributeTarget->BesRecord_Info.besrecord_tuning16K || mStreamAttributeTarget->BesRecord_Info.besrecord_dmnr_tuningEnable)
            && ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_ASR) > 0)) {
            pParamUnit = appOps->audioTypeGetParamUnit(RecordDmnrAudioType, categoryPath.c_str());
            pDmnrParam = appOps->paramUnitGetParamByName(pParamUnit, RECORD_DMNR_PARAM);

            ASSERT(pDmnrParam != NULL);
            ASSERT(pDmnrParam->arraySize <= DMNRCalDataNum);

            memcpy(BesRecordDMNRParam, (Word16 *)pDmnrParam->data, pDmnrParam->arraySize * sizeof(Word16));
        } else if (mode == SPE_MODE_VOIP) {
            //receiver path
            if ((RoutePath == ROUTE_NORMAL) && ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_VOIP_NORMAL_DMNR) > 0)
                && CheckDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR)) {
                pParamUnit = appOps->audioTypeGetParamUnit(VoIPDmnrAudioType, categoryPath.c_str());
                pDmnrParam = appOps->paramUnitGetParamByName(pParamUnit, VOIP_DMNR_PARAM);

                ASSERT(pDmnrParam != NULL);
                ASSERT(pDmnrParam->arraySize <= DMNRCalDataNum);

                memcpy(BesRecordDMNRParam, (Word16 *)pDmnrParam->data, pDmnrParam->arraySize * sizeof(Word16));
                SetDMNREnable(DMNR_NORMAL, true);
            }
            //speaker path
            else if ((RoutePath == ROUTE_SPEAKER) && ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_VOIP_HANDSFREE_DMNR) > 0)
                     && CheckDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR)) {
                pParamUnit = appOps->audioTypeGetParamUnit(VoIPDmnrAudioType, categoryPath.c_str());
                pDmnrParam = appOps->paramUnitGetParamByName(pParamUnit, VOIP_DMNR_PARAM);

                ASSERT(pDmnrParam != NULL);
                ASSERT(pDmnrParam->arraySize <= DMNRCalDataNum);

                memcpy(BesRecordDMNRParam, (Word16 *)pDmnrParam->data, pDmnrParam->arraySize * sizeof(Word16));
                SetDMNREnable(DMNR_HANDSFREE, true);
            } else {
                pParamUnit = appOps->audioTypeGetParamUnit(VoIPDmnrAudioType, VOIP_NO_DMNR_PATH);
                pDmnrParam = appOps->paramUnitGetParamByName(pParamUnit, VOIP_DMNR_PARAM);

                ASSERT(pDmnrParam != NULL);
                ASSERT(pDmnrParam->arraySize <= DMNRCalDataNum);

                memcpy(BesRecordDMNRParam, (Word16 *)pDmnrParam->data, pDmnrParam->arraySize * sizeof(Word16));
                SetDMNREnable(DMNR_DISABLE, false);
            }
        } else {
            pParamUnit = appOps->audioTypeGetParamUnit(RecordDmnrAudioType, RECORD_NO_DMNR_PATH);
            pDmnrParam = appOps->paramUnitGetParamByName(pParamUnit, RECORD_DMNR_PARAM);

            ASSERT(pDmnrParam != NULL);
            ASSERT(pDmnrParam->arraySize <= DMNRCalDataNum);

            memcpy(BesRecordDMNRParam, (Word16 *)pDmnrParam->data, pDmnrParam->arraySize * sizeof(Word16));
        }
        mSPELayer->SetDMNRPara(mode, BesRecordDMNRParam);
    } else {
        // no DMNR support DMNR disabled
        pParamUnit = appOps->audioTypeGetParamUnit(RecordDmnrAudioType, RECORD_NO_DMNR_PATH);
        pDmnrParam = appOps->paramUnitGetParamByName(pParamUnit, RECORD_DMNR_PARAM);

        ASSERT(pDmnrParam != NULL);
        ASSERT(pDmnrParam->arraySize <= DMNRCalDataNum);

        memcpy(BesRecordDMNRParam, (Word16 *)pDmnrParam->data, pDmnrParam->arraySize * sizeof(Word16));
        mSPELayer->SetDMNRPara(mode, BesRecordDMNRParam);
        SetDMNREnable(DMNR_DISABLE, false);
    }
    //DMNR parameters---

    appOps->audioTypeUnlock(VoIPAudioType);
    appOps->audioTypeUnlock(VoIPDmnrAudioType);
    appOps->audioTypeUnlock(VoIPGeneralAudioType);
    appOps->audioTypeUnlock(RecordAudioType);
    appOps->audioTypeUnlock(RecordFirAudioType);
    appOps->audioTypeUnlock(RecordDmnrAudioType);
#else /* Get VoIP/Record parameter from NVRam struct */
    uWord32 BesRecordEnhanceParas[EnhanceParasNum] = {0};
    Word16 BesRecordCompenFilter[CompenFilterNum] = {0};
    Word16 BesRecordDMNRParam[DMNRCalDataNum] = {0};

    bool bVoIPEnable = IsVoIPEnable();
    int RoutePath = GetBesRecordRoutePath();
    SPE_MODE mode = mSpeechProcessMode;
    bool bIsMICInverse = AudioALSAHardwareResourceManager::getInstance()->getMicInverse();

    ALOGD("%s(),mBesRecordStereoMode=%d, input_source= %d, input_devices=%x,mBesRecordModeIndex=%d, bVoIPEnable=%d, mode=%d, bypassDualProcess=%d, bIsMICInverse=%d", __FUNCTION__, mBesRecordStereoMode, mStreamAttributeTarget->input_source,
          mStreamAttributeTarget->input_device, mBesRecordModeIndex, bVoIPEnable, mode, mStreamAttributeTarget->BesRecord_Info.besrecord_bypass_dualmicprocess, bIsMICInverse);

    //set speech parameters+++
    for (int i = 0; i < EnhanceParasNum; i++) { //EnhanceParasNum = (16+32)+12(common parameters)
        if (i < (SPEECH_PARA_NUM + EnhanceModeParasExtNum)) {
            // Mode parameters
            if (i < SPEECH_PARA_NUM) {
                if (mStreamAttributeTarget->BesRecord_Info.besrecord_dmnr_tuningEnable == true) {
                    //specific parameters
                    BesRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_NORMAL][i];     //use loud speaker mode speech params
                } else if (mode == SPE_MODE_VOIP) {
                    //specific parameters
                    if (RoutePath == ROUTE_BT) {
                        BesRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_BT][i];
                    } else if (RoutePath == ROUTE_HEADSET) {
                        BesRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_HEADSET][i];
                    } else if (RoutePath == ROUTE_SPEAKER) {
                        BesRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_SPEAKER][i];
                    } else { //normal receiver case
                        BesRecordEnhanceParas[i] = mVOIPParam.speech_mode_para[AUDIO_VOIP_DEVICE_NORMAL][i];
                    }

                    //AEC off
                    if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION) &&
                        (mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_AECOn == false) &&
                        (mStreamAttributeTarget->input_device != AUDIO_DEVICE_IN_ALL_SCO)) {
                        BesRecordEnhanceParas[0] = 0;
                        BesRecordEnhanceParas[1] = 479;
                    }

                    //NS off
                    if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION) &&
                        (mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_NSOn == false) &&
                        (mStreamAttributeTarget->input_device != AUDIO_DEVICE_IN_ALL_SCO)) {
                        //Dual mic
                        BesRecordEnhanceParas[13] = 0;
                        BesRecordEnhanceParas[14] = 0;
                        //1-mic
                        BesRecordEnhanceParas[4] &= ~1;
                    }

                    //AGC off
                    if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION) &&
                        (mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_AGCOn == false) &&
                        (mStreamAttributeTarget->input_device != AUDIO_DEVICE_IN_ALL_SCO)) {
                        BesRecordEnhanceParas[3] |= 0x4000;
                    }
                } else {
                    BesRecordEnhanceParas[i] = mBesRecordParam.hd_rec_speech_mode_para[mBesRecordModeIndex][i];
                }
            } else {
                // Fill with 0 for new mode parameter extension
                BesRecordEnhanceParas[i] = 0;

                // For DMNR tuning, mode param 47[15] should be set 1
                if (mStreamAttributeTarget->BesRecord_Info.besrecord_dmnr_tuningEnable) {
                    BesRecordEnhanceParas[47] |= 0x8000;
                }
            }
        } else {
            //common parameters also use VoIP's
            BesRecordEnhanceParas[i] = mVOIPParam.speech_common_para[i - (SPEECH_PARA_NUM + EnhanceModeParasExtNum)];
        }
        ALOGV("BesRecordEnhanceParas[%u]=%u", i, BesRecordEnhanceParas[i]);
    }

    mSPELayer->SetEnhPara(mode, BesRecordEnhanceParas);
    //speech parameters---

    //FIR parameters+++
    for (int i = 0; i < WB_FIR_NUM; i++) {
        if (mStreamAttributeTarget->BesRecord_Info.besrecord_dmnr_tuningEnable == true) {
            BesRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_NORMAL][i];
            //ALOGD("BesRecordCompenFilter[%d]=%d", i, BesRecordCompenFilter[i]);
            BesRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_NORMAL][i];
        } else if (mode == SPE_MODE_VOIP) {
            if (RoutePath == ROUTE_BT) {
                BesRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_BT][i];   //UL1 params
                BesRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_BT][i];  //UL2 params
                BesRecordCompenFilter[i + WB_FIR_NUM * 2] = mVOIPParam.out_fir[AUDIO_VOIP_DEVICE_BT][i]; //DL params
            } else if (RoutePath == ROUTE_HEADSET) {
                BesRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_HEADSET][i];   //UL1 params
                BesRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_HEADSET][i];  //UL2 params
                BesRecordCompenFilter[i + WB_FIR_NUM * 2] = mVOIPParam.out_fir[AUDIO_VOIP_DEVICE_HEADSET][i]; //DL params
            } else if (RoutePath == ROUTE_SPEAKER) {
                BesRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_SPEAKER][i];   //UL1 params
                BesRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_SPEAKER][i];  //UL2 params
                BesRecordCompenFilter[i + WB_FIR_NUM * 2] = mVOIPParam.out_fir[AUDIO_VOIP_DEVICE_SPEAKER][i]; //DL params
            } else { //normal receiver case
                BesRecordCompenFilter[i] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_NORMAL][i];   //UL1 params
                BesRecordCompenFilter[i + WB_FIR_NUM] = mVOIPParam.in_fir[AUDIO_VOIP_DEVICE_NORMAL][i];  //UL2 params
                BesRecordCompenFilter[i + WB_FIR_NUM * 2] = mVOIPParam.out_fir[AUDIO_VOIP_DEVICE_NORMAL][i]; //DL params
            }
        } else {
            if (bIsMICInverse == false) {
                BesRecordCompenFilter[i] = mBesRecordParam.hd_rec_fir[mBesRecordParam.hd_rec_map_to_fir_for_ch1[mBesRecordModeIndex]][i];
                //ALOGD("BesRecordCompenFilter[%d]=%d", i, BesRecordCompenFilter[i]);
                if (mBesRecordStereoMode) { //stereo, UL2 use different FIR filter
                    BesRecordCompenFilter[i + WB_FIR_NUM] = mBesRecordParam.hd_rec_fir[mBesRecordParam.hd_rec_map_to_fir_for_ch2[mBesRecordModeIndex]][i];
                } else { //mono, UL2 use the same FIR filter
                    BesRecordCompenFilter[i + WB_FIR_NUM] = mBesRecordParam.hd_rec_fir[mBesRecordParam.hd_rec_map_to_fir_for_ch1[mBesRecordModeIndex]][i];
                }
            } else {
                // Mic inversed, main mic using the reference mic settings
                BesRecordCompenFilter[i] = mBesRecordParam.hd_rec_fir[mBesRecordParam.hd_rec_map_to_fir_for_ch2[mBesRecordModeIndex]][i];
                if (mBesRecordStereoMode) {
                    BesRecordCompenFilter[i + WB_FIR_NUM] = mBesRecordParam.hd_rec_fir[mBesRecordParam.hd_rec_map_to_fir_for_ch1[mBesRecordModeIndex]][i];
                } else { //mono, UL2 use the same FIR filter
                    BesRecordCompenFilter[i + WB_FIR_NUM] = mBesRecordParam.hd_rec_fir[mBesRecordParam.hd_rec_map_to_fir_for_ch2[mBesRecordModeIndex]][i];
                }
            }
        }
    }

    mSPELayer->SetCompFilter(mode, BesRecordCompenFilter);
    //FIR parameters---

    //DMNR parameters+++
    if (((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0) && (mStreamAttributeTarget->BesRecord_Info.besrecord_bypass_dualmicprocess == false)) {
        //DMNR parameters
        //google default input source AUDIO_SOURCE_VOICE_RECOGNITION not using DMNR (on/off by parameters)
        if (((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_RECOGNITION) || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1)
             || mStreamAttributeTarget->BesRecord_Info.besrecord_tuning16K || mStreamAttributeTarget->BesRecord_Info.besrecord_dmnr_tuningEnable)
            && ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_ASR) > 0)) {
            for (int i = 0; i < NUM_ABFWB_PARAM; i++) {
                BesRecordDMNRParam[i] = mDMNRParam.ABF_para_VR[i];
            }
        } else if (mode == SPE_MODE_VOIP) { //VoIP case
            //receiver path
            if ((RoutePath == ROUTE_NORMAL) && ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_VOIP_NORMAL_DMNR) > 0)
                && CheckDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR)) {
                //enable corresponding DMNR flag
                for (int i = 0; i < NUM_ABFWB_PARAM; i++) {
                    BesRecordDMNRParam[i] = mDMNRParam.ABF_para_VOIP[i];
                }
                SetDMNREnable(DMNR_NORMAL, true);
            }
            //speaker path
            else if ((RoutePath == ROUTE_SPEAKER) && ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_VOIP_HANDSFREE_DMNR) > 0)
                     && CheckDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR)) {
                for (int i = 0; i < NUM_ABFWB_PARAM; i++) {
                    BesRecordDMNRParam[i] = mDMNRParam.ABF_para_VOIP_LoudSPK[i];
                }
                SetDMNREnable(DMNR_HANDSFREE, true);
            } else {
                memset(BesRecordDMNRParam, 0, sizeof(BesRecordDMNRParam));
                SetDMNREnable(DMNR_DISABLE, false);
            }
        } else {
            memset(BesRecordDMNRParam, 0, sizeof(BesRecordDMNRParam));
        }

        mSPELayer->SetDMNRPara(mode, BesRecordDMNRParam);
    } else {
        memset(BesRecordDMNRParam, 0, sizeof(BesRecordDMNRParam));
        mSPELayer->SetDMNRPara(mode, BesRecordDMNRParam);
        SetDMNREnable(DMNR_DISABLE, false);
    }
    //DMNR parameters---
#endif

    //need to config as 16k sample rate for voice recognition(Google's will use 48K preprocess instead) or VoIP or REC+AEC
    if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1)
        || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION2)
        || (mStreamAttributeTarget->BesRecord_Info.besrecord_tuning16K == true) || (IsVoIPEnable() == true)) {
        if (mode == SPE_MODE_VOIP) { //VoIP case
            mSPELayer->SetSampleRate(mode, VOICE_RECOGNITION_RECORD_SAMPLE_RATE);
            mSPELayer->SetAPPTable(mode, WB_VOIP);
        } else { //voice recognition case
            mSPELayer->SetSampleRate(mode, VOICE_RECOGNITION_RECORD_SAMPLE_RATE);
            if (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION2) {
                mSPELayer->SetAPPTable(mode, MONO_AEC_RECORD);   //set library do AEC Record
            } else {
                mSPELayer->SetAPPTable(mode, SPEECH_RECOGNITION);   //set library do voice recognition process or MagiASR
            }
        }
    } else if ((mode == SPE_MODE_REC) && (mStreamAttributeTarget->input_source == AUDIO_SOURCE_UNPROCESSED)) {
        mSPELayer->SetSampleRate(mode, HD_RECORD_SAMPLE_RATE);
        mSPELayer->SetAPPTable(mode, LOW_LATENCY_RECORD);
    } else { //normal record  use 48k
        mSPELayer->SetSampleRate(mode, HD_RECORD_SAMPLE_RATE);
        if (mBesRecordStereoMode) {
            mSPELayer->SetAPPTable(mode, STEREO_RECORD);    //set library do stereo process
        } else {
            mSPELayer->SetAPPTable(mode, MONO_RECORD);    //set library do mono process
        }
    }

    mSPELayer->SetRoute((SPE_ROUTE)RoutePath);

    //set MIC digital gain to library
    long gain = mAudioALSAVolumeController->GetSWMICGain();
    uint8_t TotalGain = mAudioALSAVolumeController->GetULTotalGain();
    if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET) {
        gain = 0;
        TotalGain = 0;
        ALOGD("BT path set Digital MIC gain = 0");
    }
    mSPELayer->SetMICDigitalGain(mode, gain);
    mSPELayer->SetUpLinkTotalGain(mode, TotalGain);

#ifdef UPLINK_LOW_LATENCY
    if ((mode == SPE_MODE_REC) && (mStreamAttributeTarget->mAudioInputFlags & AUDIO_INPUT_FLAG_FAST)) { //normal record case && if(fast capture)
        // audio low latency param - record - swip
        mSPELayer->SetFrameRate(mode, UPLINK_LOW_LATENCY_MS);   //can support 1/2/3/5/10ms frame rate for low latency
        mSPELayer->SetAPPTable(mode, LOW_LATENCY_RECORD);   //only for low latency record!!
    }
#endif

    ALOGD("-%s()", __FUNCTION__);
}

void AudioALSACaptureDataClient::StartBesRecord(void) {
    ALOGD("+%s()", __FUNCTION__);
    mSPELayer->Start(mSpeechProcessMode);
    ALOGD("-%s()", __FUNCTION__);
}

void AudioALSACaptureDataClient::StopBesRecord(void) {
    ALOGD("+%s()", __FUNCTION__);
    mSPELayer->Stop();
    ALOGD("-%s()", __FUNCTION__);
}

uint32_t AudioALSACaptureDataClient::BesRecordPreprocess(void *buffer, uint32_t bytes) {
    uint32_t retSize = bytes;

    //ALOGD("+%s()", __FUNCTION__);
    if (!mBypassBesRecord) {
        struct InBufferInfo InBufinfo;
        InBufinfo.pBufBase = (short *)buffer;
        InBufinfo.BufLen = bytes;
        InBufinfo.time_stamp_queued = GetSystemTime(false);
        InBufinfo.bHasRemainInfo = true;
        InBufinfo.time_stamp_predict = GetCaptureTimeStamp();

        retSize = mSPELayer->Process(&InBufinfo);
    }
    //ALOGD("-%s(), bytes=%d, retSize=%d", __FUNCTION__, bytes, retSize);

    return retSize;
}

int AudioALSACaptureDataClient::GetBesRecordRoutePath(void) {
    int RoutePath;
    ALOGD("+%s(), output device = 0x%x, input device = 0x%x", __FUNCTION__, mStreamAttributeTarget->output_devices, mStreamAttributeTarget->input_device);

    if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET) {
        RoutePath = ROUTE_BT;
    } else if (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_WIRED_HEADSET) {
        RoutePath = ROUTE_HEADSET;
    } else if (mStreamAttributeTarget->output_devices == AUDIO_DEVICE_OUT_WIRED_HEADPHONE
               && (mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_BUILTIN_MIC || mStreamAttributeTarget->input_device == AUDIO_DEVICE_IN_BACK_MIC)) {
        RoutePath = ROUTE_EARPHONE;
    } else if (mStreamAttributeTarget->output_devices & AUDIO_DEVICE_OUT_SPEAKER) { //speaker path
        RoutePath = ROUTE_SPEAKER;
    } else {
        RoutePath = ROUTE_NORMAL;
    }

    return RoutePath;
}


bool AudioALSACaptureDataClient::CheckBesRecordBypass() {
#if 0   //these input sources will not enable BesRecord while capture handle create (BesRecord_Info.besrecord_enable), keep this function for other purpose in the future
    if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_VOICE_UNLOCK) || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_FM_TUNER)) {
        mBypassBesRecord = true;
    } else {
        mBypassBesRecord = false;
    }
#endif
    ALOGD("%s() %d", __FUNCTION__, mBypassBesRecord);
    return mBypassBesRecord;
}

bool AudioALSACaptureDataClient::CheckNeedBesRecordSRC() {
    uint32_t BesRecord_usingsamplerate = HD_RECORD_SAMPLE_RATE;

    if (mStreamAttributeTarget->BesRecord_Info.besrecord_enable == true) {
        //BesRecord need 16K sample rate data (Google's voice recognition will use 48K process due to new CTS test case)
        if ((mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION1)
            || (mStreamAttributeTarget->input_source == AUDIO_SOURCE_CUSTOMIZATION2)
            || (mStreamAttributeTarget->BesRecord_Info.besrecord_tuning16K == true) || (IsVoIPEnable() == true)) {
            BesRecord_usingsamplerate = VOICE_RECOGNITION_RECORD_SAMPLE_RATE;
            //need src if the stream source sample rate are not the same with BesRecord needed
            if ((mStreamAttributeSource->sample_rate  != VOICE_RECOGNITION_RECORD_SAMPLE_RATE) || (mStreamAttributeSource->num_channels != 2)) {
                mNeedBesRecordSRC = true;
            } else {
                mNeedBesRecordSRC = false;
            }
        } else { //BesRecord need 48K sample rate data
            //need src if the stream source sample rate are not the same with BesRecord needed
            if ((mStreamAttributeSource->sample_rate  != HD_RECORD_SAMPLE_RATE)  || (mStreamAttributeSource->num_channels != 2)) {
                mNeedBesRecordSRC = true;
                BesRecord_usingsamplerate = HD_RECORD_SAMPLE_RATE;
            } else {
                mNeedBesRecordSRC = false;
            }
        }
        ALOGD("%s(), mStreamAttributeSource->sample_rate=%d, mStreamAttributeSource->num_channels=%d, mStreamAttributeTarget->sample_rate=%d,mStreamAttributeTarget->num_channels=%d, BesRecord_usingsamplerate=%d"
              , __FUNCTION__, mStreamAttributeSource->sample_rate, mStreamAttributeSource->num_channels, mStreamAttributeTarget->sample_rate, mStreamAttributeTarget->num_channels, BesRecord_usingsamplerate);

        //if need to do BesRecord SRC
        //if (mNeedBesRecordSRC)
        {
            if ((mStreamAttributeSource->sample_rate == 0) || (mStreamAttributeSource->num_channels == 0)) {
                ASSERT(0);
            }
            // Need SRC from stream target to BesRecord needed
            if ((mStreamAttributeSource->sample_rate != BesRecord_usingsamplerate) || (mStreamAttributeSource->num_channels != 2)) {

                mBliSrcHandler1 = newMtkAudioSrc(mStreamAttributeSource->sample_rate, mStreamAttributeSource->num_channels,
                                                 BesRecord_usingsamplerate, 2, SRC_IN_Q1P15_OUT_Q1P15);
                mBliSrcHandler1->open();
            }

            mBesRecSRCSizeFactor = ((BesRecord_usingsamplerate * 2) / (mStreamAttributeSource->sample_rate * mStreamAttributeSource->num_channels)) + 1;

            // Need SRC from BesRecord to stream target needed
            if (mStreamAttributeTarget->sample_rate != BesRecord_usingsamplerate) {
                mBliSrcHandler2 = newMtkAudioSrc(BesRecord_usingsamplerate, 2,
                                                 mStreamAttributeTarget->sample_rate, 2, SRC_IN_Q1P15_OUT_Q1P15);
                mBliSrcHandler2->open();
            }

            mBesRecSRCSizeFactor2 = ((mStreamAttributeTarget->sample_rate * 2) / (BesRecord_usingsamplerate * 2)) + 1;
        }
    } else {
        mNeedBesRecordSRC = false;
    }

    ALOGD("%s(), %d, %d, mBesRecSRCSizeFactor=%d", __FUNCTION__, mNeedBesRecordSRC, BesRecord_usingsamplerate, mBesRecSRCSizeFactor);
    return mNeedBesRecordSRC;
}

bool AudioALSACaptureDataClient::IsVoIPEnable(void) {
    //ALOGV("%s() %d", __FUNCTION__, mStreamAttributeTarget->BesRecord_Info.besrecord_voip_enable);
    return mStreamAttributeTarget->BesRecord_Info.besrecord_voip_enable;
}

status_t AudioALSACaptureDataClient::UpdateBesRecParam() {
    ALOGD("+%s() besrecord_voip_enable %d, besrecord_enable=%d", __FUNCTION__,
          mStreamAttributeTarget->BesRecord_Info.besrecord_voip_enable, mStreamAttributeTarget->BesRecord_Info.besrecord_enable);
    if (mStreamAttributeTarget->BesRecord_Info.besrecord_voip_enable && mStreamAttributeTarget->BesRecord_Info.besrecord_enable) {
        if (mSPELayer->IsSPERunning()) {
            StopBesRecord();
            ConfigBesRecordParams();
            mSPELayer->Standby();   //for doing the time resync
            StartBesRecord();
        } else {
            ConfigBesRecordParams();
        }
    }
    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

void AudioALSACaptureDataClient::CheckDynamicSpeechMask(void) {
    if (mStreamAttributeTarget->BesRecord_Info.besrecord_dynamic_mask.dynamic_func != mVoIPSpeechEnhancementMask.dynamic_func) {  //need update dynamic mask
        UpdateDynamicFunction();
        mVoIPSpeechEnhancementMask = mStreamAttributeTarget->BesRecord_Info.besrecord_dynamic_mask;
    }
}

void AudioALSACaptureDataClient::UpdateDynamicFunction(void) {
    ALOGD("+%s()", __FUNCTION__);
    int RoutePath = GetBesRecordRoutePath();
    SPE_MODE mode = mSpeechProcessMode;
    short DMNRParam[DMNRCalDataNum] = {0};
    ALOGD("%s(), RoutePath %d, mode %d", __FUNCTION__, RoutePath, mode);
    //DMNR function update
    if ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0) {
#if (defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT) && (MTK_AUDIO_TUNING_TOOL_V2_PHASE >= 2))
        AppOps *appOps = appOpsGetInstance();
        if (appOps == NULL) {
            ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
            ASSERT(0);
            return;
        }

        AudioType *VoIPDmnrAudioType = appOps->appHandleGetAudioTypeByName(appOps->appHandleGetInstance(), VOIPDMNR_AUDIO_TYPE);
        appOps->audioTypeReadLock(VoIPDmnrAudioType, __FUNCTION__);

        if ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DMNR_3_0) > 0) {
            if (mode == SPE_MODE_VOIP) {
                //receiver path & receiver DMNR is enabled
                if ((RoutePath == ROUTE_NORMAL) && CheckDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR) &&
                    ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_VOIP_NORMAL_DMNR) > 0)) {
                    ALOGD("enable normal mode DMNR");

                    ParamUnit *pParamUnit = appOps->audioTypeGetParamUnit(VoIPDmnrAudioType, VOIP_HANDSET_DMNR_PATH);
                    Param *pDmnrParam = appOps->paramUnitGetParamByName(pParamUnit, VOIP_DMNR_PARAM);

                    ASSERT(pDmnrParam != NULL);

                    mSPELayer->SetDMNRPara(mode, (Word16 *)pDmnrParam->data);
                    SetDMNREnable(DMNR_NORMAL, true);
                } else if ((RoutePath == ROUTE_SPEAKER) && CheckDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) &&
                           ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_VOIP_HANDSFREE_DMNR) > 0)) { //speaker path
                    ALOGD("enable loudspeaker mode DMNR");
                    ParamUnit *pParamUnit = appOps->audioTypeGetParamUnit(VoIPDmnrAudioType, VOIP_HANDSFREE_NR_PATH);
                    Param *pDmnrParam = appOps->paramUnitGetParamByName(pParamUnit, VOIP_DMNR_PARAM);

                    ASSERT(pDmnrParam != NULL);

                    mSPELayer->SetDMNRPara(mode, (Word16 *)pDmnrParam->data);
                    SetDMNREnable(DMNR_HANDSFREE, true);
                } else {
                    ALOGD("disable DMNR");
                    ParamUnit *pParamUnit = appOps->audioTypeGetParamUnit(VoIPDmnrAudioType, VOIP_NO_DMNR_PATH);
                    Param *pDmnrParam = appOps->paramUnitGetParamByName(pParamUnit, VOIP_DMNR_PARAM);

                    ASSERT(pDmnrParam != NULL);

                    mSPELayer->SetDMNRPara(mode, (Word16 *)pDmnrParam->data);
                    SetDMNREnable(DMNR_DISABLE, false);
                }
            }
        } else {
            ALOGD("%s(),disable DMNR due to not support", __FUNCTION__);

            ParamUnit *pParamUnit = appOps->audioTypeGetParamUnit(VoIPDmnrAudioType, VOIP_NO_DMNR_PATH);
            Param *pDmnrParam = appOps->paramUnitGetParamByName(pParamUnit, VOIP_DMNR_PARAM);

            ASSERT(pDmnrParam != NULL);

            mSPELayer->SetDMNRPara(mode, (Word16 *)pDmnrParam->data);
            SetDMNREnable(DMNR_DISABLE, false);
        }
        appOps->audioTypeUnlock(VoIPDmnrAudioType);
#else
        if ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DMNR_3_0) > 0) {
            if (mode == SPE_MODE_VOIP) {
                //receiver path & receiver DMNR is enabled
                if ((RoutePath == ROUTE_NORMAL) && CheckDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR) &&
                    ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_VOIP_NORMAL_DMNR) > 0)) {
                    ALOGD("enable normal mode DMNR");
                    //enable corresponding DMNR flag
                    for (int i = 0; i < NUM_ABFWB_PARAM; i++) {
                        DMNRParam[i] = mDMNRParam.ABF_para_VOIP[i];
                    }
                    SetDMNREnable(DMNR_NORMAL, true);
                } else if ((RoutePath == ROUTE_SPEAKER) && CheckDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) &&
                           ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_VOIP_HANDSFREE_DMNR) > 0)) { //speaker path
                    ALOGD("enable loudspeaker mode DMNR");
                    for (int i = 0; i < NUM_ABFWB_PARAM; i++) {
                        DMNRParam[i] = mDMNRParam.ABF_para_VOIP_LoudSPK[i];
                    }
                    SetDMNREnable(DMNR_HANDSFREE, true);
                } else {
                    ALOGD("disable DMNR");
                    memset(DMNRParam, 0, sizeof(DMNRParam));
                    SetDMNREnable(DMNR_DISABLE, false);
                }
                mSPELayer->SetDMNRPara(mode, DMNRParam);
            }
        } else {
            ALOGD("%s(),disable DMNR due to not support", __FUNCTION__);
            memset(DMNRParam, 0, sizeof(DMNRParam));
            SetDMNREnable(DMNR_DISABLE, false);
            mSPELayer->SetDMNRPara(mode, DMNRParam);
        }
#endif
    }
    ALOGD("-%s()", __FUNCTION__);
}

bool AudioALSACaptureDataClient::CheckDynamicSpeechEnhancementMaskOnOff(const voip_sph_enh_dynamic_mask_t dynamic_mask_type) {
    ALOGV("%s() %x, %x", __FUNCTION__, mStreamAttributeTarget->BesRecord_Info.besrecord_dynamic_mask.dynamic_func, dynamic_mask_type);
    bool bret = false;

    if ((mStreamAttributeTarget->BesRecord_Info.besrecord_dynamic_mask.dynamic_func & dynamic_mask_type) > 0) {
        bret = true;
    }

    return bret;
}

//0: disable DMNR
//1: normal mode DMNR
//2: handsfree mode DMNR
void AudioALSACaptureDataClient::SetDMNREnable(DMNR_TYPE type, bool enable) {
    ALOGD("%s(), type=%d, bypassDMNR=%d", __FUNCTION__, type, mStreamAttributeTarget->BesRecord_Info.besrecord_bypass_dualmicprocess);

    if (((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DUAL_MIC) > 0) && (mStreamAttributeTarget->BesRecord_Info.besrecord_bypass_dualmicprocess == false)) {
        if ((mAudioCustParamClient->QueryFeatureSupportInfo()& SUPPORT_DMNR_3_0) > 0) {
            switch (type) {
            case DMNR_DISABLE :
                mSPELayer->SetDynamicFuncCtrl(NORMAL_DMNR, false);
                mSPELayer->SetDynamicFuncCtrl(HANDSFREE_DMNR, false);
                break;
            case DMNR_NORMAL :
                mSPELayer->SetDynamicFuncCtrl(NORMAL_DMNR, enable);
                break;
            case DMNR_HANDSFREE :
                mSPELayer->SetDynamicFuncCtrl(HANDSFREE_DMNR, enable);
                break;
            default:
                mSPELayer->SetDynamicFuncCtrl(NORMAL_DMNR, false);
                mSPELayer->SetDynamicFuncCtrl(HANDSFREE_DMNR, false);
                break;
            }
        } else {

            ALOGD("%s(), turn off due to not support", __FUNCTION__);
            mSPELayer->SetDynamicFuncCtrl(NORMAL_DMNR, false);
            mSPELayer->SetDynamicFuncCtrl(HANDSFREE_DMNR, false);
        }
    }
}

timespec AudioALSACaptureDataClient::GetCaptureTimeStamp(void) {
    struct timespec capturetime;

    long ret_ns;
    capturetime.tv_sec  = 0;
    capturetime.tv_nsec = 0;

    if ((mStreamAttributeSource->Time_Info.timestamp_get.tv_sec == 0) && (mStreamAttributeSource->Time_Info.timestamp_get.tv_nsec == 0)) {
        ALOGE("%s fail", __FUNCTION__);
    } else {
        capturetime = mStreamAttributeSource->Time_Info.timestamp_get;
        ret_ns = mStreamAttributeSource->Time_Info.kernelbuffer_ns;
        if ((capturetime.tv_nsec - ret_ns) >= 0) {
            capturetime.tv_nsec -= ret_ns;
        } else {
            capturetime.tv_sec -= 1;
            capturetime.tv_nsec = 1000000000 + capturetime.tv_nsec - ret_ns;
        }
        ALOGV("%s, sec= %ld, nsec=%ld, ret_ns = %ld\n", __FUNCTION__, capturetime.tv_sec, capturetime.tv_nsec, ret_ns);
    }

    return capturetime;
}

timespec AudioALSACaptureDataClient::GetEchoRefTimeStamp(void) {
    struct timespec echoreftime;

    long ret_ns;
    echoreftime.tv_sec  = 0;
    echoreftime.tv_nsec = 0;

    if ((mStreamAttributeSourceEchoRef->Time_Info.timestamp_get.tv_sec == 0) && (mStreamAttributeSourceEchoRef->Time_Info.timestamp_get.tv_nsec == 0)) {
        ALOGE("%s fail", __FUNCTION__);
    } else {
        echoreftime = mStreamAttributeSourceEchoRef->Time_Info.timestamp_get;
        ret_ns = mStreamAttributeSourceEchoRef->Time_Info.kernelbuffer_ns;
        if ((echoreftime.tv_nsec - ret_ns) >= 0) {
            echoreftime.tv_nsec -= ret_ns;
        } else {
            echoreftime.tv_sec -= 1;
            echoreftime.tv_nsec = 1000000000 + echoreftime.tv_nsec - ret_ns;
        }
        ALOGV("%s, sec= %ld, nsec=%ld, ret_ns = %ld\n", __FUNCTION__, echoreftime.tv_sec, echoreftime.tv_nsec, ret_ns);
    }

    return echoreftime;
}

//BesRecord---
//#endif    //MTK_AUDIO_HD_REC_SUPPORT

//Android Native Preprocess effect +++
void AudioALSACaptureDataClient::CheckNativeEffect(void) {

    if (mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Update == true) {
        ALOGD("+%s() %d", __FUNCTION__, mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Count);

        if (mAudioPreProcessEffect != NULL) {
#if 0
            for (int i = 0; i < mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Count; i++) {
                //mAudioPreProcessEffect->removeAudioEffect(mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Record[i]);
                //mAudioPreProcessEffect->addAudioEffect(mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Record[i]);
            }
#endif
            mAudioPreProcessEffect->CheckNativeEffect();
        }

        mStreamAttributeTarget->NativePreprocess_Info.PreProcessEffect_Update = false;
        ALOGD("-%s()", __FUNCTION__);
    }
}

uint32_t AudioALSACaptureDataClient::NativePreprocess(void *buffer, uint32_t bytes) {
    uint32_t retsize = bytes;
    retsize = mAudioPreProcessEffect->NativePreprocess(buffer, bytes, &mStreamAttributeSource->Time_Info);
    return retsize;
}

//Android Native Preprocess effect ---

//EchoRef+++
void AudioALSACaptureDataClient::AddEchoRefDataProvider(AudioALSACaptureDataProviderBase *pCaptureDataProvider, stream_attribute_t *stream_attribute_target) {
    ALOGD("+%s()", __FUNCTION__);
    mStreamAttributeTargetEchoRef = stream_attribute_target;
    mCaptureDataProviderEchoRef = pCaptureDataProvider;//AudioALSACaptureDataProviderEchoRef::getInstance();
    mStreamAttributeSourceEchoRef = mCaptureDataProviderEchoRef->getStreamAttributeSource();

    // fix the channel count of echo reference data to stereo since native echo_reference_itfe supports stereo only
    mStreamAttributeTargetEchoRef->num_channels = 2;
    mStreamAttributeTargetEchoRef->audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;

    //check SRC needed and created
    // raw data
    memset((void *)&mEchoRefRawDataBuf, 0, sizeof(mEchoRefRawDataBuf));
    mEchoRefRawDataBuf.pBufBase = new char[kClientBufferSize];
    mEchoRefRawDataBuf.bufLen   = kClientBufferSize;
    mEchoRefRawDataBuf.pRead    = mEchoRefRawDataBuf.pBufBase;
    mEchoRefRawDataBuf.pWrite   = mEchoRefRawDataBuf.pBufBase;
    ASSERT(mEchoRefRawDataBuf.pBufBase != NULL);

    // src data
    memset((void *)&mEchoRefSrcDataBuf, 0, sizeof(mEchoRefSrcDataBuf));
    mEchoRefSrcDataBuf.pBufBase = new char[kClientBufferSize];
    mEchoRefSrcDataBuf.bufLen   = kClientBufferSize;
    mEchoRefSrcDataBuf.pRead    = mEchoRefSrcDataBuf.pBufBase;
    mEchoRefSrcDataBuf.pWrite   = mEchoRefSrcDataBuf.pBufBase;
    ASSERT(mEchoRefSrcDataBuf.pBufBase != NULL);

    // attach client to capture EchoRef data provider
    ALOGD("%s(), mCaptureDataProviderEchoRef=%p", __FUNCTION__, mCaptureDataProviderEchoRef);
    mCaptureDataProviderEchoRef->configStreamAttribute(mStreamAttributeTarget);
    mCaptureDataProviderEchoRef->attach(this); // mStreamAttributeSource will be updated when first client attached

    // init SRC, this SRC is for Android Native.
    if (mStreamAttributeSourceEchoRef->sample_rate  != mStreamAttributeTargetEchoRef->sample_rate  ||
        mStreamAttributeSourceEchoRef->num_channels != mStreamAttributeTargetEchoRef->num_channels ||
        mStreamAttributeSourceEchoRef->audio_format != mStreamAttributeTargetEchoRef->audio_format) {
        mBliSrcEchoRef = newMtkAudioSrc(
                             mStreamAttributeSourceEchoRef->sample_rate, mStreamAttributeSourceEchoRef->num_channels,
                             mStreamAttributeTargetEchoRef->sample_rate, mStreamAttributeTargetEchoRef->num_channels,
                             SRC_IN_Q1P15_OUT_Q1P15); // TODO(Harvey, Ship): 24bit
        mBliSrcEchoRef->open();
    }

    // init SRC, this SRC is for MTK VoIP
    if ((mStreamAttributeTargetEchoRef->sample_rate != 16000) || (mStreamAttributeTargetEchoRef->num_channels != 1)) {
        mBliSrcEchoRefBesRecord = newMtkAudioSrc(
                                      mStreamAttributeTargetEchoRef->sample_rate, mStreamAttributeTargetEchoRef->num_channels,
                                      16000, 1,
                                      SRC_IN_Q1P15_OUT_Q1P15);
        mBliSrcEchoRefBesRecord->open();
    }

    //assume starts after PCM open
    mSPELayer->SetDownLinkLatencyTime(mStreamAttributeSourceEchoRef->latency);

    // pcm start if need
    while (mCaptureDataProvider->getReadThreadReady() == false ||
           mCaptureDataProviderEchoRef->getReadThreadReady() == false)
    {
        usleep(2 * 1000);
    }
    ALOGD("readthread all ready! pcm start");
    mCaptureDataProviderEchoRef->signalPcmStart();
    mSPELayer->SetOutputStreamRunning(true, true);
    mSPELayer->SetEchoRefStartTime(GetSystemTime(false));
    mCaptureDataProvider->signalPcmStart();
    mSPELayer->SetUPLinkIntrStartTime(GetSystemTime(false));



    ALOGD("%s(), Source sample_rate=%d, num_channels=%d, audio_format=%d", __FUNCTION__
          , mStreamAttributeSourceEchoRef->sample_rate, mStreamAttributeSourceEchoRef->num_channels, mStreamAttributeSourceEchoRef->audio_format);
    ALOGD("%s(), Target sample_rate=%d, num_channels=%d, audio_format=%d", __FUNCTION__
          , mStreamAttributeTargetEchoRef->sample_rate, mStreamAttributeTargetEchoRef->num_channels, mStreamAttributeTargetEchoRef->audio_format);

    ALOGD("-%s()", __FUNCTION__);
}


//EchoRef data no need to provide to Capture handler
uint32_t AudioALSACaptureDataClient::copyEchoRefCaptureDataToClient(RingBuf pcm_read_buf) {
    ALOGV("+%s()", __FUNCTION__);

    uint32_t freeSpace = RingBuf_getFreeSpace(&mEchoRefRawDataBuf);
    uint32_t dataSize = RingBuf_getDataCount(&pcm_read_buf);
    if (freeSpace < dataSize) {
        ALOGE("%s(), mRawDataBuf <= pcm_read_buf, freeSpace(%u) < dataSize(%u), buffer overflow!!", __FUNCTION__, freeSpace, dataSize);
        RingBuf_copyFromRingBuf(&mEchoRefRawDataBuf, &pcm_read_buf, freeSpace);
    } else {
        RingBuf_copyFromRingBuf(&mEchoRefRawDataBuf, &pcm_read_buf, dataSize);
    }

    // SRC to to Native AEC need format (as StreaminTarget format since AWB data might be the same as DL1 before)
    const uint32_t kNumRawData = RingBuf_getDataCount(&mEchoRefRawDataBuf);
    uint32_t num_free_space = RingBuf_getFreeSpace(&mEchoRefSrcDataBuf);

    if (mBliSrcEchoRef == NULL) { // No need SRC
        //ASSERT(num_free_space >= kNumRawData);
        if (num_free_space < kNumRawData) {
            ALOGW("%s(), num_free_space(%u) < kNumRawData(%u)", __FUNCTION__, num_free_space, kNumRawData);
            RingBuf_copyFromRingBuf(&mEchoRefSrcDataBuf, &mEchoRefRawDataBuf, num_free_space);
        } else {
            RingBuf_copyFromRingBuf(&mEchoRefSrcDataBuf, &mEchoRefRawDataBuf, kNumRawData);
        }
    } else { // Need SRC
        char *pEchoRefRawDataLinearBuf = new char[kNumRawData];
        RingBuf_copyToLinear(pEchoRefRawDataLinearBuf, &mEchoRefRawDataBuf, kNumRawData);

        char *pEchoRefSrcDataLinearBuf = new char[num_free_space];

        char *p_read = pEchoRefRawDataLinearBuf;
        uint32_t num_raw_data_left = kNumRawData;
        uint32_t num_converted_data = num_free_space; // max convert num_free_space

        uint32_t consumed = num_raw_data_left;
        mBliSrcEchoRef->process((int16_t *)p_read, &num_raw_data_left,
                                (int16_t *)pEchoRefSrcDataLinearBuf, &num_converted_data);
        consumed -= num_raw_data_left;

        p_read += consumed;
        ALOGV("%s(), num_raw_data_left = %u, num_converted_data = %u",
              __FUNCTION__, num_raw_data_left, num_converted_data);

        //ASSERT(num_raw_data_left == 0);
        if (num_raw_data_left > 0) {
            ALOGW("%s(), num_raw_data_left(%u) > 0", __FUNCTION__, num_raw_data_left);
        }

        RingBuf_copyFromLinear(&mEchoRefSrcDataBuf, pEchoRefSrcDataLinearBuf, num_converted_data);
        ALOGV("%s(), dataCount:%u", __FUNCTION__, RingBuf_getDataCount(&mEchoRefSrcDataBuf));

        delete[] pEchoRefRawDataLinearBuf;
        delete[] pEchoRefSrcDataLinearBuf;
    }


    //for Preprocess
    const uint32_t kNumEchoRefSrcData = RingBuf_getDataCount(&mEchoRefSrcDataBuf);
    char *pEchoRefProcessDataLinearBuf = new char[kNumEchoRefSrcData];
    RingBuf_copyToLinear(pEchoRefProcessDataLinearBuf, &mEchoRefSrcDataBuf, kNumEchoRefSrcData);

#ifdef BOOST_ECHOREF
    if ((mStreamAttributeTarget->output_devices & AUDIO_DEVICE_OUT_SPEAKER)) {
        for (uint32_t i = 0; i < kNumEchoRefSrcData / 2 ; i++) {
            // over flow protection
            int16_t temp = *((int16_t *)(pEchoRefProcessDataLinearBuf + (i * 2)));
            if (temp > 8191) {
                temp = 8191;
            } else if (temp < -8192) {
                temp = -8192;
            }
            // enhance 12dB
            temp = temp << 2;

            pEchoRefProcessDataLinearBuf[2 * i] = (char)temp;
            pEchoRefProcessDataLinearBuf[2 * i + 1] = (char)(temp >> 8);
        }
    }
#endif

    //here to queue the EchoRef data to Native effect, since it doesn't need to SRC here
    if ((mAudioPreProcessEffect->num_preprocessors > 0)) {  //&& echoref is enabled
        //copy pEchoRefProcessDataLinearBuf to native preprocess for echo ref
        mAudioPreProcessEffect->WriteEchoRefData(pEchoRefProcessDataLinearBuf, kNumEchoRefSrcData, &mStreamAttributeSourceEchoRef->Time_Info);
    }

    //If need MTK VoIP process
    if ((mStreamAttributeTarget->BesRecord_Info.besrecord_enable) && !mBypassBesRecord) {
        struct InBufferInfo BufInfo;
        //for MTK native SRC
        if (mBliSrcEchoRefBesRecord == NULL) { // No need SRC
            //TODO(Sam),copy pEchoRefProcessDataLinearBuf to MTK echoref data directly

            BufInfo.pBufBase = (short *)pEchoRefProcessDataLinearBuf;
            BufInfo.BufLen = kNumEchoRefSrcData;
            BufInfo.time_stamp_queued = GetSystemTime(false);
            BufInfo.bHasRemainInfo = true;
            BufInfo.time_stamp_predict = GetEchoRefTimeStamp();
#ifdef SRC_DROP_DATA
            if (mFirstEchoSRC == true) {
                mFirstEchoSRC = false;
                delete[] pEchoRefProcessDataLinearBuf;
                return 0;
            }
#endif
            mSPELayer->WriteReferenceBuffer(&BufInfo);

        } else { // Need SRC
            char *pEchoRefProcessSRCDataLinearBuf = new char[kNumEchoRefSrcData];

            char *p_read = pEchoRefProcessDataLinearBuf;
            uint32_t num_raw_data_left = kNumEchoRefSrcData;
            uint32_t num_converted_data = kNumEchoRefSrcData; // max convert num_free_space
            uint32_t consumed = num_raw_data_left;

            mBliSrcEchoRefBesRecord->process((int16_t *)p_read, &num_raw_data_left,
                                             (int16_t *)pEchoRefProcessSRCDataLinearBuf, &num_converted_data);
            consumed -= num_raw_data_left;

            p_read += consumed;
            ALOGV("%s(), num_raw_data_left = %u, num_converted_data = %u",
                  __FUNCTION__, num_raw_data_left, num_converted_data);

            //ASSERT(num_raw_data_left == 0);
            if (num_raw_data_left > 0) {
                ALOGW("%s(), num_raw_data_left(%u) > 0", __FUNCTION__, num_raw_data_left);
            }

            //TODO: (Sam )copy pEchoRefSrcDataLinearBuf to MTK VoIP write echo ref data
            BufInfo.pBufBase = (short *)pEchoRefProcessSRCDataLinearBuf;
            BufInfo.BufLen = num_converted_data;
            BufInfo.time_stamp_queued = GetSystemTime(false);
            BufInfo.bHasRemainInfo = true;
            BufInfo.time_stamp_predict = GetEchoRefTimeStamp();
#ifdef SRC_DROP_DATA
            if (mFirstEchoSRC == true) {
                mFirstEchoSRC = false;
                delete[] pEchoRefProcessSRCDataLinearBuf;
                delete[] pEchoRefProcessDataLinearBuf;
                return 0;
            }
#endif
            mSPELayer->WriteReferenceBuffer(&BufInfo);

            delete[] pEchoRefProcessSRCDataLinearBuf;
        }
    }

    delete[] pEchoRefProcessDataLinearBuf;
    return 0;
}

//EchoRef---
uint32_t AudioALSACaptureDataClient::getLatencyTime() {
    uint32_t mlatency = UPLINK_NORMAL_LATENCY_MS ; //20ms
#ifdef UPLINK_LOW_LATENCY
    if (IsLowLatencyCapture()) {
        mlatency = UPLINK_LOW_LATENCY_MS; //5ms
    }
#endif
    return mlatency;
}

} // end of namespace android

