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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MTKCAM_HWNODE_P2_UTIL_H_
#define _MTKCAM_HWNODE_P2_UTIL_H_

#include <mtkcam3/feature/featurePipe/IStreamingFeaturePipe.h>

#include "P2_Request.h"

#include <mtkcam/utils/hw/IFDContainer.h>

namespace P2
{

class P2Util
{
public:
    enum FindOutMask {  FIND_NO_ROTATE  = 0x01,
                        FIND_ROTATE     = 0x02,
                        FIND_DISP       = 0x04,
                        FIND_VIDEO      = 0x08, };
    enum ReleaseMask { RELEASE_ALL      = 0x01,
                       RELEASE_DISP     = 0x02,
                       RELEASE_FD       = 0x04,
                       RELEASE_RSSO     = 0x08, };
    enum P2PortFlag { USE_VENC = 0x01, };

    class SimpleIO
    {
    public:
        SimpleIO();
        MVOID setUseLMV(MBOOL useLMV);
        MBOOL hasInput() const;
        MBOOL hasOutput() const;
        MBOOL isResized() const;
        MSize getInputSize() const;
        MVOID updateResult(MBOOL result) const;
        MVOID dropRecord() const;
        MBOOL findDisplayAndRelease(MBOOL result);
        MVOID earlyRelease(MUINT32 mask, MBOOL result);
        sp<P2Img> getMDPSrc() const;
        sp<P2Img> getLcso() const;
        P2IOPack toP2IOPack(const P2Pack &p2Pack) const;
        MVOID printIO(const ILog &log) const;
    private:
        MBOOL mResized;
        MBOOL mUseLMV;
        sp<P2Img> mIMGI;
        sp<P2Img> mLCEI;
        sp<P2Img> mLCSHO;
        sp<P2Img> mIMG2O;
        sp<P2Img> mIMG3O;
        sp<P2Img> mWROTO;
        sp<P2Img> mWDMAO;
        friend class P2Util;
    };

    class SimpleIn
    {
    public:
        SimpleIn(MUINT32 sensorId, sp<P2Request> request);
        MUINT32 getSensorId() const;
        MVOID setUseLMV(MBOOL useLMV);
        MVOID setISResized(MBOOL isResized);
        MBOOL isResized() const;
        MBOOL useLMV() const;
        MBOOL useCropRatio() const;
        MSize getInputSize() const;
        sp<P2Img> getLcso() const;
        MVOID addCropRatio(const char* name, const float cropRatio);
        MVOID addCropRatio(const char* name, const float widthCropRatio, const float heightCropRatio);
        MBOOL hasCropRatio() const;
        MSizeF getCropRatio() const;
        MVOID releaseAllImg();
    public:
        sp<P2Img>     mIMGI     = nullptr;
        sp<P2Img>     mLCEI     = nullptr;
        sp<P2Img>     mLCSHO    = nullptr;
        sp<P2Img>     mRSSO     = nullptr;
        sp<P2Img>     mPreRSSO  = nullptr;
        sp<P2Img>     mFullYuv  = nullptr;
        sp<P2Img>     mRrzYuv1  = nullptr;
        sp<P2Img>     mRrzYuv2  = nullptr;
        sp<P2Request> mRequest  = nullptr; // for metadata
        TuningParam   mTuning;
        NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam mFeatureParam;
    private:
        MUINT32       mSensorId  = 0;
        MBOOL         mResized   = MFALSE;
        MBOOL         mUseLMV    = MFALSE;
        MBOOL         mUseCropRatio = MFALSE;
        MSizeF        mCropRatio = MSizeF(1.0f, 1.0f);
        friend class P2Util;
    };

    class SimpleOut
    {
    public:
        SimpleOut(MUINT32 sensorId, sp<P2Request> request, sp<P2Img>& pImg);
        MUINT32 getSensorId() const;
        MVOID setIsFD(MBOOL isFDBuffer);
        MBOOL isDisplay() const;
        MBOOL isRecord() const;
        MBOOL isFD() const;
        MBOOL isMDPOutput() const;
    public:
        sp<P2Img>     mImg;
        sp<P2Request> mRequest; // for metadata
        MRectF        mCrop     = MRectF(0,0);
        MUINT32       mDMAConstrain = 0;
        P2Obj         mP2Obj;   // for isp tuning
    private:
        MUINT32       mSensorId = 0;
        MBOOL         mFD       = MFALSE;
        friend class P2Util;
    };

public:
    static SimpleIO extractSimpleIO(const sp<P2Request> &request, MUINT32 portFlag = 0);
    static MVOID releaseTuning(TuningParam &tuning);

    static sp<P2Img> extractOut(const sp<P2Request> &request, MUINT32 target = 0);


public:
    static TuningParam xmakeTuning(const P2Pack &p2Pack, const SimpleIO &io, NS3Av3::IHalISP *halISP, P2MetaSet &metaSet);
    static DIPParams xmakeDIPParams(const P2Pack &p2Pack, const SimpleIO &io, const TuningParam &tuning, const P2ObjPtr &p2ObjPtr);

    static MVOID xmakeDpPqParam(const P2Pack &p2Pack, const SimpleOut &out, FD_DATATYPE * &pFdData);
    static const char *p2PQIdx2String(const MUINT32 pqIdx);
};

class Pool
{
public:
    Pool();
    ~Pool();
    MVOID create(const char *name, NSCam::EImageFormat fmt, const MSize &size);
    MVOID destroy();
    sp<IImageBuffer> getBuffer() const;

private:
    std::string mName;
    sp<IImageBuffer> mBuffer;
};

MBOOL copyImageBuffer(IImageBuffer *src, IImageBuffer *dst);

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_UTIL_H_
