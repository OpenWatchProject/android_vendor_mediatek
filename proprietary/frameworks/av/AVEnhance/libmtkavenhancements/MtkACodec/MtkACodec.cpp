/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkACodec"

#ifdef __LP64__
#define OMX_ANDROID_COMPILE_AS_32BIT_ON_64BIT_PLATFORMS
#endif

#include <utils/Log.h>
#include <ui/GraphicBuffer.h>
#include <ui/Fence.h>
#include <media/stagefright/omx/OMXUtils.h>
#include "MtkACodec.h"
#include "Utils_MTK.h"
#include "stagefright/MediaDefs_MTK.h"
#include <media/openmax/OMX_VideoExt.h>
#include <media/stagefright/foundation/ByteUtils.h>


#define MP3_MULTI_FRAME_COUNT_IN_ONE_OUTPUTBUFFER_FOR_PURE_AUDIO 20

namespace android {

//   set AvSyncRefTime to omx +
static inline bool IsWhoIAm(const char* who, const char* me)
{
    bool ret = false;
    if (!strncmp(who, me, strlen(me))){
        ret = true;
    }
    return ret;
}

static inline bool IsMTKVideoDecoderComponent(const char* componentName)
{
    #define IAM_MTK_VDEC "OMX.MTK.VIDEO.DECODER"
    return IsWhoIAm(componentName, IAM_MTK_VDEC);
}
//   set AvSyncRefTime to omx -

//////////////////////////////////////////////

MtkACodec :: MtkACodec() {
    //set AvSyncRefTime to omx
    mAnchorTimeRealUs = 0;

#ifdef MTK_ROI_SUPPORT
    mRoi = new MtkRoi();
#endif
}

MtkACodec::~MtkACodec() {
}

status_t MtkACodec::setupAudioCodec(
            status_t err, const char *mime, bool encoder, const sp<AMessage> &msg) {
    ALOGV("setupAudioCodec: mime %s, encoder %d, msg.get() %p", mime, encoder, msg.get());

#ifdef MSSI_MTK_WMA_PLAYBACK_SUPPORT
    if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_WMA) ||
            !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_WMAPRO)) {
        int32_t numChannels;
        int32_t sampleRate;
        if (!msg->findInt32("channel-count", &numChannels)
                || !msg->findInt32("sample-rate", &sampleRate)) {
            err = INVALID_OPERATION;
        } else {
            err = setupWMACodec(encoder, numChannels, sampleRate);
        }
    }
#endif

#ifdef MSSI_MTK_AUDIO_ADPCM_SUPPORT
    if(!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MS_ADPCM) ||
                    !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_DVI_IMA_ADPCM)) {
        err = setupADPCMCodec(mime, msg);
    }
#endif

#ifdef MSSI_MTK_AUDIO_ALAC_SUPPORT
    if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_ALAC)) {
        int32_t numChannels;
        int32_t sampleRate;
        if (!msg->findInt32("channel-count", &numChannels)
            || !msg->findInt32("sample-rate", &sampleRate)) {
            err = INVALID_OPERATION;
        } else {
            err = setupAlacCodec(mime, msg);
        }
    }
#endif

#ifdef MSSI_MTK_AUDIO_APE_SUPPORT
    if (!encoder && !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_APE)) {
        err = setupApeCodec(mime, msg);
    }
#endif //MSSI_MTK_AUDIO_APE_SUPPORT

    if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MSGSM)) {
        int32_t numChannels;
        int32_t sampleRate;
        if (!msg->findInt32("channel-count", &numChannels)
                || !msg->findInt32("sample-rate", &sampleRate)) {
            err = INVALID_OPERATION;
        } else {
            err = setupRawAudioFormat(kPortIndexInput, sampleRate, numChannels);
        }
    }

    return err;
}

status_t MtkACodec::getPortFormat(OMX_U32 portIndex, sp<AMessage> &notify) {
    status_t err = ACodec::getPortFormat(portIndex, notify);
    if (err != BAD_TYPE) {
        return err;
    }

    /* only Google's ACodec::getPortFormat() return BAD_VALUE(
     * Unsupported audio coding/Unsupported domain),
     * then try to get mtk port format.
     */
    const char *niceIndex = portIndex == kPortIndexInput ? "input" : "output";
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = portIndex;

    err = mOMXNode->getParameter(OMX_IndexParamPortDefinition, &def, sizeof(def));
    if (err != OK) {
        return err;
    }

    if (def.eDir != (portIndex == kPortIndexOutput ? OMX_DirOutput : OMX_DirInput)) {
        ALOGE("unexpected dir: %s(%d) on %s port", asString(def.eDir), def.eDir, niceIndex);
        return BAD_VALUE;
    }

    switch (def.eDomain) {
        case OMX_PortDomainAudio:
        {
            OMX_AUDIO_PORTDEFINITIONTYPE *audioDef = &def.format.audio;

            switch ((int)audioDef->eEncoding) {
#ifdef MSSI_MTK_WMA_PLAYBACK_SUPPORT
                case OMX_AUDIO_CodingWMA:
                {
                    OMX_AUDIO_PARAM_WMATYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    CHECK_EQ((status_t)OK, mOMXNode->getParameter(
                        (OMX_INDEXTYPE)OMX_IndexParamAudioWma,
                        &params,
                        sizeof(params)));

                    notify->setInt32("channel-count", params.nChannels);
                    notify->setInt32("sample-rate", params.nSamplingRate);
                    break;
                }
#endif
#ifdef MSSI_MTK_AUDIO_ADPCM_SUPPORT
                case OMX_AUDIO_CodingADPCM:
                {
                    OMX_AUDIO_PARAM_ADPCMTYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    CHECK_EQ((status_t)OK, mOMXNode->getParameter(
                        (OMX_INDEXTYPE)OMX_IndexParamAudioAdpcm,
                        &params,
                        sizeof(params)));

                    notify->setString("mime", params.nFormatTag == WAVE_FORMAT_MS_ADPCM ?
                        MEDIA_MIMETYPE_AUDIO_MS_ADPCM : MEDIA_MIMETYPE_AUDIO_DVI_IMA_ADPCM);
                    notify->setInt32("channel-count", params.nChannelCount);
                    notify->setInt32("sample-rate", params.nSamplesPerSec);
                    notify->setInt32("block-align", params.nBlockAlign);
                    notify->setInt32("bits-per-sample", params.nBitsPerSample);
                    break;
                }
#endif
#ifdef MSSI_MTK_AUDIO_ALAC_SUPPORT
                case OMX_AUDIO_CodingALAC:
                {
                    OMX_AUDIO_PARAM_ALACTYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    CHECK_EQ((status_t)OK, mOMXNode->getParameter(
                        (OMX_INDEXTYPE)OMX_IndexParamAudioAlac,
                        &params,
                        sizeof(params)));

                    notify->setString("mime", MEDIA_MIMETYPE_AUDIO_ALAC);
                    notify->setInt32("channel-count", params.nChannels);
                    notify->setInt32("sample-rate", params.nSampleRate);
                    break;
                }
#endif
#ifdef MSSI_MTK_AUDIO_APE_SUPPORT
                case OMX_AUDIO_CodingAPE:
                {
                    OMX_AUDIO_PARAM_APETYPE params;
                    InitOMXParams(&params);
                    params.nPortIndex = portIndex;

                    CHECK_EQ((status_t)OK, mOMXNode->getParameter(
                        (OMX_INDEXTYPE)OMX_IndexParamAudioApe,
                        &params,
                        sizeof(params)));

                    notify->setString("mime", MEDIA_MIMETYPE_AUDIO_APE);
                    notify->setInt32("channel-count", params.channels);
                    notify->setInt32("sample-rate", params.SampleRate);
                    break;
                }
#endif

                default:
                    ALOGE("Unsupported audio coding: %s(%d)\n",
                            asString(audioDef->eEncoding), audioDef->eEncoding);
                    return BAD_TYPE;
            }
            break;
        }

        default:
            ALOGE("Unsupported domain: %s(%d)", asString(def.eDomain), def.eDomain);
            return BAD_TYPE;
    }

    return getVendorParameters(portIndex, notify);
}

#ifdef MSSI_MTK_AUDIO_ADPCM_SUPPORT
status_t MtkACodec::setupADPCMCodec(const char *mime, const sp<AMessage> &msg) {
    int32_t encoder;
    if (!msg->findInt32("encoder", &encoder)) {
        encoder = false;
    }

    int32_t numChannels;
    int32_t sampleRate;
    CHECK(msg->findInt32("channel-count", &numChannels));
    CHECK(msg->findInt32("sample-rate", &sampleRate));

    status_t err = setupRawAudioFormat(
        encoder ? kPortIndexInput : kPortIndexOutput, sampleRate, numChannels);

    if (err != OK) {
        return err;
    }

    OMX_AUDIO_PARAM_ADPCMTYPE def;

    if (encoder) {
        InitOMXParams(&def);
        def.nPortIndex = kPortIndexOutput;
        //uint32_t type;

        err = mOMXNode->getParameter((OMX_INDEXTYPE)OMX_IndexParamAudioAdpcm, &def, sizeof(def));
        if (err != OK) {
            return err;
        }

        def.nFormatTag = (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MS_ADPCM)) ? WAVE_FORMAT_MS_ADPCM : WAVE_FORMAT_DVI_IMA_ADPCM;
        def.nChannelCount = numChannels;
        def.nSamplesPerSec = sampleRate;

        return mOMXNode->setParameter(
            (OMX_INDEXTYPE)OMX_IndexParamAudioAdpcm, &def, sizeof(def));
    } else {
        OMX_AUDIO_ADPCMPARAM def;
        InitOMXParams(&def);
        def.nPortIndex = kPortIndexInput;
        //uint32_t type;
        sp<ABuffer> buffer;
        int32_t bitsPerSample = 0;
        err = mOMXNode->getParameter(
            (OMX_INDEXTYPE)OMX_IndexParamAudioAdpcm, &def, sizeof(def));
        if (err != OK) {
            return err;
        }

        def.nFormatTag = (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MS_ADPCM)) ? WAVE_FORMAT_MS_ADPCM : WAVE_FORMAT_DVI_IMA_ADPCM;
        def.nChannelCount = numChannels;
        def.nSamplesPerSec = sampleRate;
        if (msg->findInt32("bits-per-sample", &bitsPerSample) && bitsPerSample > 0)
        {
            def.nBitsPerSample  = bitsPerSample;
        }
        return mOMXNode->setParameter(
            (OMX_INDEXTYPE)OMX_IndexParamAudioAdpcm, &def, sizeof(def));
    }
}
#endif

#ifdef MSSI_MTK_AUDIO_ALAC_SUPPORT
status_t MtkACodec::setupAlacCodec(const char *mime, const sp<AMessage> &msg) {
    UNUSED(mime);
    int32_t numChannels = 0, sampleRate = 0, bitWidth = 0;

    CHECK(msg->findInt32("channel-count", &numChannels));
    CHECK(msg->findInt32("sample-rate", &sampleRate));
    if (!msg->findInt32("bits-per-sample", &bitWidth)) {
        ALOGD("can not find bits-per-sample");
    }

    /*
        Work arround. some 3rd app provide wrong sampleRate or bitWidth(even not provide).
        Here use csd to parse correct values.
     */
    sp<ABuffer> csd;
    if (msg->findBuffer("csd-0", &csd)) {
        ALOGV("find csd-0 buffer, size = %zu", csd->size());
        if (csd->size() == 24) { //csd buffer size is always 24
            uint8_t * data = (uint8_t *) csd->data();
            numChannels = data[9];
            sampleRate = U32_AT(data + 20);
            bitWidth = data[5];
        }
    }

    status_t err = setupRawAudioFormat(kPortIndexOutput, sampleRate, numChannels);
    if (err != OK) {
        return err;
    }

    OMX_AUDIO_PARAM_ALACTYPE profileAlac;
    InitOMXParams(&profileAlac);
    profileAlac.nPortIndex = kPortIndexInput;

    err = mOMXNode->getParameter(
        (OMX_INDEXTYPE)OMX_IndexParamAudioAlac, &profileAlac, sizeof(profileAlac));
    CHECK_EQ((status_t)OK, err);

    profileAlac.nChannels   = numChannels;
    profileAlac.nSampleRate = sampleRate;
    profileAlac.nSamplesPerPakt = 4096; //always 4096
    profileAlac.nBitsWidth  = bitWidth;
    ALOGV("bitWidth = %d, numChannels = %d, sampleRate = %d, profileAlac.nBitsWidth = %d",
        bitWidth, numChannels, sampleRate, profileAlac.nBitsWidth);
    err = mOMXNode->setParameter(
        (OMX_INDEXTYPE)OMX_IndexParamAudioAlac, &profileAlac, sizeof(profileAlac));
    CHECK_EQ((status_t)OK, err);

    OMX_PARAM_PORTDEFINITIONTYPE inputdef, outputdef;

    InitOMXParams(&inputdef);
    inputdef.nPortIndex = OMX_DirInput;

    err = mOMXNode->getParameter(
        (OMX_INDEXTYPE)OMX_IndexParamPortDefinition, &inputdef, sizeof(inputdef));
    CHECK_EQ((status_t)OK, err);

    inputdef.nBufferSize = profileAlac.nChannels * (profileAlac.nBitsWidth >> 3) * profileAlac.nSamplesPerPakt;
    int32_t maxInputSize = 0;
    if (msg->findInt32("max-input-size", &maxInputSize) && maxInputSize > 0) {
        if (inputdef.nBufferSize < maxInputSize) {
            inputdef.nBufferSize = maxInputSize;
        }
    }
    err = mOMXNode->setParameter(
        (OMX_INDEXTYPE)OMX_IndexParamPortDefinition, &inputdef, sizeof(inputdef));
    CHECK_EQ((status_t)OK, err);

    InitOMXParams(&outputdef);
    outputdef.nPortIndex = OMX_DirOutput;

    err = mOMXNode->getParameter(
        (OMX_INDEXTYPE)OMX_IndexParamPortDefinition, &outputdef, sizeof(outputdef));
    CHECK_EQ((status_t)OK, err);
    outputdef.nBufferSize = profileAlac.nChannels * 2 * profileAlac.nSamplesPerPakt;

    if (profileAlac.nBitsWidth > 16)
    {
        outputdef.nBufferSize <<= 1;
    }

    err = mOMXNode->setParameter(
        (OMX_INDEXTYPE)OMX_IndexParamPortDefinition, &outputdef, sizeof(outputdef));
    CHECK_EQ((status_t)OK, err);
    return err;
}
#endif

#ifdef MSSI_MTK_WMA_PLAYBACK_SUPPORT
status_t MtkACodec::setupWMACodec(
        bool encoder, int32_t numChannels, int32_t sampleRate) {
    status_t err = setupRawAudioFormat(
            encoder ? kPortIndexInput : kPortIndexOutput, sampleRate, numChannels);

    if (err != OK) {
        return err;
    }

    if (encoder) {
        ALOGW("WMA encoding is not supported.");
        return INVALID_OPERATION;
    }
#ifdef MSSI_MTK_SWIP_WMAPRO
    int32_t channelMask = 0;
    mOMXNode->getParameter(OMX_IndexParamAudioWmaProfile, &channelMask, sizeof(channelMask));
    mChannelMaskPresent = true;
    mChannelMask = channelMask;
    ALOGD("WMAPro channelMask is 0x%x", channelMask);
#endif

    OMX_AUDIO_PARAM_WMATYPE def;
    InitOMXParams(&def);
    def.nPortIndex = kPortIndexInput;

    err = mOMXNode->getParameter(
            (OMX_INDEXTYPE)OMX_IndexParamAudioWma,
            &def,
            sizeof(def));

    if (err != OK) {
        return err;
    }

    def.nChannels = numChannels;
    def.nSamplingRate = sampleRate;

    return mOMXNode->setParameter((OMX_INDEXTYPE)OMX_IndexParamAudioWma, &def, sizeof(def));
}
#endif

#ifdef MSSI_MTK_AUDIO_APE_SUPPORT
status_t MtkACodec::setupApeCodec(const char *mime, const sp<AMessage> &msg) {
    OMX_AUDIO_PARAM_APETYPE profile;
    InitOMXParams(&profile);
    profile.nPortIndex = OMX_DirInput;
    ALOGD("setupApeCodec mime %s", mime);
    status_t err = mOMXNode->getParameter(
            OMX_IndexParamAudioApe, &profile, sizeof(profile));

    int32_t bps = 0, maxInputSize = 0, bitRate = 0;
    int32_t channelCount = 0;
    int32_t sampleRate = 0;
    if(msg->findInt32("channel-count", &channelCount) && channelCount > 0) {
        profile.channels = channelCount;
    }
    if(msg->findInt32("sample-rate", &sampleRate) && sampleRate > 0) {
        profile.SampleRate = sampleRate;
    }
    if (msg->findInt32("bitrate", (int32_t *)&bitRate) && bitRate > 0) {
        profile.Bitrate = bitRate;
    }
    if (msg->findInt32("max-input-size", (int32_t *)&maxInputSize) && maxInputSize > 0) {
        profile.SourceBufferSize = maxInputSize;
    }
    if (msg->findInt32("bits-per-sample", (int32_t *)&bps) && bps > 0) {
        profile.bps = bps;
    }

    err = mOMXNode->setParameter(
           OMX_IndexParamAudioApe, &profile, sizeof(profile));
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = OMX_DirInput;
    err = mOMXNode->getParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    if(profile.SourceBufferSize != 0)
    {
        def.nBufferSize = profile.SourceBufferSize;
    }
    err = mOMXNode->setParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));

    return err;
}
#endif

status_t MtkACodec::setViLTEParameters(const sp<IOMXNode> &omxNode, const sp<AMessage> &msg, bool fgCheckResolutionChange)
{
    int32_t width = 0;
    int32_t height = 0;
    int32_t ViLTE = 0;

    if (msg->findInt32("vilte-mode", &ViLTE) && ViLTE)
    {
        ALOGI("setMTKVTMode, value: %d", ViLTE);
    }

    if (msg->findInt32("width", &width))
    {
        ALOGI("setMTKParameters, width: %d", width);
    }

    if (msg->findInt32("height", &height))
    {
        ALOGI("setMTKParameters, height: %d", height);
    }
	status_t err;
	{
        int32_t rotation = 0;
        if (msg->findInt32("rotation-degrees", &rotation)) {
            ALOGI("set rotation-degrees %d",rotation);
            OMX_CONFIG_ROTATIONTYPE     rotationType;
            InitOMXParams(&rotationType);
            rotationType.nPortIndex =kPortIndexOutput;// kPortIndexInput;
            rotationType.nRotation = rotation;
            err = omxNode->setConfig(
                    (OMX_INDEXTYPE)OMX_IndexConfigCommonRotate,
                    &rotationType, sizeof(rotationType));

            if (err != OK) {
                ALOGE("setConfig(OMX_CONFIG_ROTATIONTYPE) "
                      "returned error 0x%08x", err);
                return err;
            }
        }
    }

    // set vilte mode to encoder
    int32_t Mode = 0;
    if (msg->findInt32("setViLTEMode", &Mode) && Mode){
        OMX_VIDEO_PARAM_BITRATETYPE bitrateType;
        InitOMXParams(&bitrateType);
        bitrateType.nPortIndex = kPortIndexOutput;

        status_t err = mOMXNode->getParameter(
                OMX_IndexParamVideoBitrate, &bitrateType, sizeof(bitrateType));

        if (err != OK) {
            ALOGE("set encode VT mode fail get OMX_IndexParamVideoBitrate");
            return err;
        }

        int32_t video_bitrate = 4800000;
        msg->findInt32("bitrate", &video_bitrate);

        bitrateType.eControlRate = OMX_Video_ControlRateConstantSkipFrames;
        bitrateType.nTargetBitrate = video_bitrate;

        ALOGE("set encode VT mode OMX_Video_ControlRateConstantSkipFrames bitrate=%d", video_bitrate);

        err = mOMXNode->setParameter(
              OMX_IndexParamVideoBitrate, &bitrateType, sizeof(bitrateType));

        if (err != OK) {
          ALOGE("set encode VT mode fail set OMX_IndexParamVideoBitrate");
          return err;
        }
   }

    if (ViLTE == 1 && fgCheckResolutionChange && width != 0 && height!= 0) {
        OMX_VIDEO_PARAM_RESOLUTION config;
        InitOMXParams(&config);
        config.nFrameWidth = (OMX_U32)width;
        config.nFrameHeight = (OMX_U32)height;

        ALOGI("ACodec::ResolutionChange");

        status_t temp = omxNode->setConfig(
                (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVencSeResolutionChange,
                &config, sizeof(config));
        if (temp != OK) {
            ALOGI("codec does not support config resolution change (err %d)", temp);
            return BAD_VALUE;
        }
    }

    return OK;
}

status_t MtkACodec::setOmxReadMultiFrame(const sp<IOMXNode> &omxNode,
        const sp<AMessage> &msg) {
    ALOGD("setOmxReadMultiFrame");
    status_t err = BAD_VALUE;

    OMX_AUDIO_PARAM_MP3TYPE profileMp3;
    InitOMXParams(&profileMp3);
    profileMp3.nPortIndex = kPortIndexInput;
    int32_t ch = 0, saR = 0;
    if (msg->findInt32("channel-count", &ch) && msg->findInt32("sample-rate", &saR)) {
        profileMp3.nChannels=ch;
        profileMp3.nSampleRate=saR;
        err = omxNode->getParameter(
                OMX_IndexParamAudioMp3, &profileMp3, sizeof(profileMp3));

        if (err == OK) {
            err = omxNode->setParameter(
                    OMX_IndexParamAudioMp3, &profileMp3, sizeof(profileMp3));
        }
    }

    int32_t  isMtkMp3 = 0;
    int32_t  app_pid = 0;
    if (!msg->findInt32("mtk-mp3extractor", &isMtkMp3) ||
        !msg->findInt32("app-pid", &app_pid)){
        ALOGW("Not expected one.");
        return BAD_VALUE;
    }

    bool isSetMtkMp3Codec = isMtkMp3Music((uint32_t)app_pid) && (isMtkMp3 == 1);

    OMX_PARAM_U32TYPE defmp3;
    InitOMXParams(&defmp3);
    defmp3.nPortIndex = kPortIndexOutput;
    if (isSetMtkMp3Codec) {
        err = omxNode->getParameter(
                OMX_IndexVendorMtkMP3Decode, &defmp3, sizeof(defmp3));
        if (err == OK) {
            defmp3.nU32 = (OMX_U32)MP3_MULTI_FRAME_COUNT_IN_ONE_OUTPUTBUFFER_FOR_PURE_AUDIO;
            err = omxNode->setParameter(
                    OMX_IndexVendorMtkMP3Decode, &defmp3, sizeof(defmp3));
        }

        if (err == OK) {
            ALOGD("Turn on MP3-Enhance, set mp3FrameCountInBuffer %d", defmp3.nU32);
        }
    } else {
        err = BAD_VALUE;
    }

    return err;
}

status_t MtkACodec::setMtkParameters(const sp<IOMXNode> & omxNode,
            const sp<AMessage> & params, bool isEncoder) {
    //for mtk video decoder seekmode
    int64_t seekTimeUs = 0;
    if (params->findInt64("seekTimeUs", &seekTimeUs)){
        if (isEncoder){
            return ERROR_UNSUPPORTED;
        }
        ALOGD("set seekTimeUs %lld", (long long)seekTimeUs);

        OMX_PARAM_S64TYPE pOmxTicksInfo;
        InitOMXParams(&pOmxTicksInfo);
        pOmxTicksInfo.nPortIndex = kPortIndexInput;
        pOmxTicksInfo.nS64 = seekTimeUs;

        status_t err = omxNode->setConfig(
                OMX_IndexVendorMtkOmxVdecSeekMode,
                &pOmxTicksInfo,
                sizeof(pOmxTicksInfo));

        if (err != OK) {
            ALOGE("setConfig(OOMX_IndexVendorMtkOmxVdecSeekMode) "
                  "returned error 0x%08x", err);
            return err;
        }
    }

    //for mtk set WFD mode
    int32_t WFDMode = 0;
    if (params->findInt32("isWfdVideo", &WFDMode) && WFDMode && isEncoder){
        OMX_VIDEO_PARAM_BITRATETYPE bitrateType;
        InitOMXParams(&bitrateType);
        bitrateType.nPortIndex = kPortIndexOutput;

        status_t err = mOMXNode->getParameter(
                OMX_IndexParamVideoBitrate, &bitrateType, sizeof(bitrateType));

        if (err != OK) {
            ALOGE("set WFD mode fail get OMX_IndexParamVideoBitrate");
            return err;
        }

        int32_t video_bitrate = 4800000;
        params->findInt32("bitrate", &video_bitrate);

        bitrateType.eControlRate = OMX_Video_ControlRateMtkWFD;
        bitrateType.nTargetBitrate = video_bitrate;

        ALOGE("set WFD mode OMX_Video_ControlRateMtkWFD bitrate=%d", video_bitrate);

        err = mOMXNode->setParameter(
              OMX_IndexParamVideoBitrate, &bitrateType, sizeof(bitrateType));

        if (err != OK) {
          ALOGE("set WFD mode fail set OMX_IndexParamVideoBitrate");
          return err;
        }
   }

#ifdef MSSI_MTK_AUDIO_APE_SUPPORT
    int32_t newframe = 0,firstbyte = 0; //for ape seek on acodec
    if (params->findInt32("nwfrm", &newframe) &&
        params->findInt32("sekbyte", &firstbyte))
    {
        OMX_AUDIO_PARAM_APETYPE profile;
        InitOMXParams(&profile);
        profile.nPortIndex = kPortIndexInput;

        status_t err = mOMXNode->getParameter((OMX_INDEXTYPE)OMX_IndexParamAudioApe, &profile, sizeof(profile));
        if (err != OK) {
            return err;
        }

        profile.seekbyte = firstbyte;
        profile.seekfrm = newframe;
        ALOGI("APE setParameter nwfrm:%d sekbyte:%d",(int)newframe,(int)firstbyte);
        err = mOMXNode->setParameter((OMX_INDEXTYPE)OMX_IndexParamAudioApe, &profile, sizeof(profile));
        if (err != OK) {
            return err;
        }
    }
#endif

#ifdef MTK_ROI_SUPPORT
    if(isEncoder) mRoi->setRoiParameters(omxNode, params);
#endif
    return OK;
}

//   set AvSyncRefTime to omx +
status_t MtkACodec::setAVSyncTime(const sp<IOMXNode> & omxNode, int64_t time)
{
    OMX_PARAM_S64TYPE AVSyncTimeInfo;
    InitOMXParams(&AVSyncTimeInfo);
    AVSyncTimeInfo.nPortIndex = kPortIndexOutput;
    AVSyncTimeInfo.nS64 = time;
    status_t err = omxNode->setConfig(
        OMX_IndexVendorMtkOmxVdecAVSyncTime,
        &AVSyncTimeInfo,
        sizeof(AVSyncTimeInfo));
    if (err != OK){
        ALOGE("Failed to set OMX_IndexVendorMtkOmxVdecAVSyncTime");
    }
    return err;
}

status_t MtkACodec::setAVSyncTime(const char* componentName,
        const sp<AMessage> bufferMeta,
        const sp<IOMXNode> & omxNode,
        const sp<AMessage> & msg) {
    int64_t avSyncTimeUs = 0;
    if (!msg->findInt64("AvSyncRefTimeUs", &avSyncTimeUs)){
        if (bufferMeta->findInt64("AvSyncRefTimeUs", &avSyncTimeUs)) {
            if (avSyncTimeUs != -1){
                if (avSyncTimeUs < mAnchorTimeRealUs){
                    //TODO: Need to reset mAnchorTimeRealUs at proper place to avoid false alarm
                    ALOGW("Got smaller av sync time. New:%lld < Old:%lld",
                        (long long)avSyncTimeUs, (long long)mAnchorTimeRealUs);
                }
                mAnchorTimeRealUs = avSyncTimeUs;
                if(IsMTKVideoDecoderComponent(componentName)){
                    return setAVSyncTime(omxNode,mAnchorTimeRealUs);
                }
            }
        }
    }
    return BAD_VALUE;
}
//   set AvSyncRefTime to omx -
status_t MtkACodec::setThumbnailMode(const char* componentName,
        const sp<IOMXNode> &omxNode,
        const sp<AMessage> &msg,
        bool IsEncoder) {
    if (IsMTKVideoDecoderComponent(componentName)) {
        ALOGV("MTK codec set param");
        OMX_PARAM_U32TYPE param;
        InitOMXParams(&param);
        param.nPortIndex = IsEncoder ? kPortIndexOutput : kPortIndexInput;
        status_t err = OK;
        int32_t enableThumbnailOptimzation = 0;
        if (msg->findInt32("enableThumbnailOptimzation", &enableThumbnailOptimzation)
            && (enableThumbnailOptimzation == 1)) {
            param.nU32 = (OMX_U32)enableThumbnailOptimzation;
            err = omxNode->setParameter(
                    OMX_IndexVendorMtkOmxVdecThumbnailMode,
                    &param, sizeof(param));
            if (err != OK) {
                ALOGW("ThumbnailOptimzation parameters set failed: %d", err);
                return err;
            }
        }
    }
    return OK;
}

}  // namespace android
