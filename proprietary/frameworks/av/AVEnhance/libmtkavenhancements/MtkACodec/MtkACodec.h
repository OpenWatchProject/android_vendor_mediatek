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

#ifndef MTK_A_CODEC_H_
#define MTK_A_CODEC_H_

#include <media/stagefright/ACodec.h>

#include "roi/MtkRoi.h"

namespace android {

struct MtkACodec : public ACodec {
    MtkACodec();

protected:
    virtual ~MtkACodec();

    virtual status_t setupAudioCodec(
         status_t err, const char *mime, bool encoder, const sp<AMessage> &msg);
    virtual status_t getPortFormat(OMX_U32 portIndex, sp<AMessage> &notify);

    virtual status_t setMtkParameters(const sp<IOMXNode> & omxNode,
            const sp<AMessage> & params, bool isEncoder);

    virtual status_t setAVSyncTime(const char* componentName,
            const sp<AMessage> bufferMeta,
            const sp<IOMXNode> & omxNode,
            const sp<AMessage> & msg);

private:
    int64_t mAnchorTimeRealUs;
    status_t setAVSyncTime(const sp<IOMXNode> & omxNode, int64_t time);
    // for MTK_WMA_PLAYBACK_SUPPORT
    status_t setupWMACodec(bool encoder, int32_t numChannels, int32_t sampleRate);

    // #ifdef MTK_AUDIO_ADPCM_SUPPORT
    status_t setupADPCMCodec(const char *mime, const sp<AMessage> &msg);
    // #endif
    // #ifdef MTK_AUDIO_ALAC_SUPPORT
    status_t setupAlacCodec(const char *mime, const sp<AMessage> &msg);
    // #endif

    //#ifdef MTK_AUDIO_APE_SUPPORT
    status_t setupApeCodec(const char *mime, const sp<AMessage> &msg);
    // #endif

    //for mp3 lowpower
    virtual status_t setOmxReadMultiFrame(const sp<IOMXNode> &omxNode,
                const sp<AMessage> &msg);

private:

    enum {
        kPortIndexInput  = 0,
        kPortIndexOutput = 1
    };

    //for ViLTE
    virtual status_t setViLTEParameters(const sp<IOMXNode> &omxNode, const sp<AMessage> &msg, bool fgCheckResolutionChange);

    //  for ThumbnailMode
    virtual status_t setThumbnailMode(const char* componentName,
        const sp<IOMXNode>&omxNode,
        const sp<AMessage> &msg,
        bool IsEncoder);

    // for ROI
    sp<MtkRoi> mRoi;

    DISALLOW_EVIL_CONSTRUCTORS(MtkACodec);
};

}  // namespace android

#endif  // MTK_A_CODEC_H_
