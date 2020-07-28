/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#ifndef __MfllMfb_H__
#define __MfllMfb_H__

#include <mtkcam3/feature/mfnr/IMfllMfb.h>

// MTKCAM
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/aaa/IHal3A.h>

// CUSTOM (platform)
#include <isp_tuning/isp_tuning.h>
using namespace NSIspTuning;

// AOSP
#include <utils/Mutex.h> // android::Mutex

// STL
#include <deque> // std::deque
#include <unordered_map> // std::unordered_map
#include <cstdint> // intptr_t

#include "MfllIspProfiles.h"

using android::sp;
using android::Mutex;
using NSCam::IImageBuffer;
using NSCam::MRect;
using NSCam::MSize;
using NSCam::MPoint;
using NS3Av3::MetaSet_T;
using NSCam::NSIoPipe::NSPostProc::INormalStream;
using NS3Av3::IHal3A;
using NSCam::IMetadata;

namespace mfll {

class MfllMfb : public IMfllMfb {
public:
    MfllMfb(void);
    virtual ~MfllMfb(void);

/* implementations */
public:
    virtual enum MfllErr init(int sensorId);
    virtual void setMfllCore(IMfllCore *c) { m_pCore = c; }
    virtual void setShotMode(const enum MfllMode &mode) { m_shotMode = mode; }
    virtual void setPostNrType(const enum NoiseReductionType &type) { m_nrType = type; }
    virtual enum MfllErr blend(IMfllImageBuffer *base, IMfllImageBuffer *ref, IMfllImageBuffer *out, IMfllImageBuffer *wt_in, IMfllImageBuffer *wt_out);
    virtual enum MfllErr blend(
                             IMfllImageBuffer* base,
                             IMfllImageBuffer* ref,
                             IMfllImageBuffer* conf,
                             IMfllImageBuffer* out,
                             IMfllImageBuffer* wt_in,
                             IMfllImageBuffer* wt_out
                         );

    virtual enum MfllErr mix(IMfllImageBuffer *base, IMfllImageBuffer *ref, IMfllImageBuffer *out, IMfllImageBuffer *wt);
    virtual enum MfllErr mix(
            IMfllImageBuffer *base,
            IMfllImageBuffer *ref,
            IMfllImageBuffer *out_main,
            IMfllImageBuffer *out_thumbnail,
            IMfllImageBuffer *wt,
            const MfllRect_t& output_thumb_crop
            );
    virtual enum MfllErr setSyncPrivateData(const std::deque<void*>& dataset);

    /* Raw to Yuv */
    virtual enum MfllErr encodeRawToYuv(IMfllImageBuffer *input, IMfllImageBuffer *output, const enum YuvStage &s);
    virtual enum MfllErr encodeRawToYuv(
            IMfllImageBuffer *input,
            IMfllImageBuffer *output,
            IMfllImageBuffer *output_q,
            const MfllRect_t& output_crop,
            const MfllRect_t& output_q_crop,
            enum YuvStage s = YuvStage_RawToYuy2);

    virtual enum MfllErr encodeRawToYuv(
            IMfllImageBuffer *input,
            IMfllImageBuffer *output,
            IMfllImageBuffer *output2,
            IMfllImageBuffer *output3,
            const MfllRect_t& output2_crop,
            const MfllRect_t& output3_crop,
            enum YuvStage s = YuvStage_RawToYuy2);

    virtual enum MfllErr convertYuvFormatByMdp(
            IMfllImageBuffer* input,
            IMfllImageBuffer* output,
            IMfllImageBuffer* output_q,
            const MfllRect_t& output_crop,
            const MfllRect_t& output_q_crop,
            enum YuvStage s = YuvStage_Unknown);


/* private interface */
protected:
    enum MfllErr encodeRawToYuv(
            IImageBuffer *src,
            IImageBuffer *dst,
            IImageBuffer *dst2,
            IImageBuffer *dst3,
            const MfllRect_t& output_crop,
            const MfllRect_t& output_q_crop,
            const MfllRect_t& output_3_crop,
            const enum YuvStage &s);

    enum MfllErr convertYuvFormatByMdp(
            IImageBuffer* src,
            IImageBuffer* dst,
            IImageBuffer* dst2,
            const MfllRect_t& output_crop,
            const MfllRect_t& output_q_crop,
            const enum YuvStage s);

    enum MfllErr blend(IImageBuffer *base, IImageBuffer *ref, IImageBuffer *out, IImageBuffer *wt_in, IImageBuffer *wt_out, IImageBuffer* confmap = nullptr);

/* attributes */
private:
    /* thread-safe protector */
    mutable Mutex m_mutex;

private:
    int m_sensorId;
    enum MfllMode m_shotMode;
    enum NoiseReductionType m_nrType;
    void *m_syncPrivateData;
    size_t m_syncPrivateDataSize;

    std::deque<MetaSet_T>       m_vMetaSet;
    std::deque<IMetadata*>      m_vMetaApp;
    std::deque<IMetadata*>      m_vMetaHal;
    std::deque<IImageBuffer*>   m_vImgLcsoRaw;

    IMfllCore *m_pCore;

    IHal3A          *m_p3A;
    INormalStream   *m_pNormalStream;
    INormalStream   *m_pNormalStream_vss;

    volatile int m_encodeYuvCount;
    volatile int m_blendCount;
    volatile bool m_bIsInited;

    bool m_bExifDumpped[STAGE_SIZE];
};
};/* namespace mfll */
#endif//__MfllMfb_H__

