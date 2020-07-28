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

#ifndef _MTK_CAMERA_FEATURE_PIPE_I_STREAMING_FEATURE_PIPE_H_
#define _MTK_CAMERA_FEATURE_PIPE_I_STREAMING_FEATURE_PIPE_H_

//#include <effectHal/EffectRequest.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>

#include <utils/RefBase.h>

#include <mtkcam/feature/featurePipe/util/VarMap.h>
#include <mtkcam/feature/3dnr/3dnr_defs.h>
#include "./IStreamingFeaturePipe_var.h"
#include <mtkcam/feature/eis/EisInfo.h>
#include <mtkcam/utils/imgbuf/ISecureImageBufferHeap.h>

#ifdef FEATURE_MASK
#error FEATURE_MASK macro redefine
#endif

#define FEATURE_MASK(name) (1 << OFFSET_##name)

#define MAKE_FEATURE_MASK_FUNC(name, tag)             \
    const MUINT32 MASK_##name = (1 << OFFSET_##name); \
    inline MBOOL HAS_##name(MUINT32 feature)          \
    {                                                 \
        return (feature & FEATURE_MASK(name));        \
    }                                                 \
    inline MVOID ENABLE_##name(MUINT32 &feature)      \
    {                                                 \
        feature |= FEATURE_MASK(name);                \
    }                                                 \
    inline MVOID DISABLE_##name(MUINT32 &feature)     \
    {                                                 \
        feature &= ~FEATURE_MASK(name);               \
    }                                                 \
    inline const char* TAG_##name()                   \
    {                                                 \
        return tag;                                   \
    }

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum FEATURE_MASK_OFFSET
{
    OFFSET_EIS,
    OFFSET_EIS_25,
    OFFSET_EIS_30,
    OFFSET_VFB,
    OFFSET_VFB_EX,
    OFFSET_VHDR,
    OFFSET_3DNR,
    OFFSET_EIS_QUEUE,
    OFFSET_VENDOR,
    OFFSET_FOV,
    OFFSET_N3D,
    OFFSET_VENDOR_FOV,
    OFFSET_3DNR_RSC,
    OFFSET_FSC,
};

MAKE_FEATURE_MASK_FUNC(EIS, "EIS22");
MAKE_FEATURE_MASK_FUNC(EIS_25, "EIS25");
MAKE_FEATURE_MASK_FUNC(EIS_30, "EIS30");
MAKE_FEATURE_MASK_FUNC(VFB, "");
MAKE_FEATURE_MASK_FUNC(VFB_EX, "");
MAKE_FEATURE_MASK_FUNC(VHDR, "");
MAKE_FEATURE_MASK_FUNC(3DNR, "3DNR");
MAKE_FEATURE_MASK_FUNC(EIS_QUEUE, "Q");
MAKE_FEATURE_MASK_FUNC(VENDOR, "Vendor");
MAKE_FEATURE_MASK_FUNC(FOV, "FOV");
MAKE_FEATURE_MASK_FUNC(N3D, "N3D");
MAKE_FEATURE_MASK_FUNC(VENDOR_FOV, "Vendor_FOV");
MAKE_FEATURE_MASK_FUNC(3DNR_RSC, "3DNR_RSC");
MAKE_FEATURE_MASK_FUNC(FSC, "FSC");


class FeaturePipeParam;

class IStreamingFeaturePipe : public virtual android::RefBase
{
public:
    enum eAppMode
    {
        APP_PHOTO_PREVIEW = 0,
        APP_VIDEO_PREVIEW = 1,
        APP_VIDEO_RECORD  = 2,
        APP_VIDEO_STOP    = 3,
    };

    enum eUsageMode
    {
        USAGE_DEFAULT,
        USAGE_P2A_PASS_THROUGH,
        USAGE_P2A_PASS_THROUGH_TIME_SHARING,
        USAGE_P2A_FEATURE,
        USAGE_STEREO_EIS,
        USAGE_FULL,
    };

    enum eUsageMask
    {
        PIPE_USAGE_EIS              = 1 << 0,
        PIPE_USAGE_3DNR             = 1 << 1,
        PIPE_USAGE_VENDOR           = 1 << 2,
        PIPE_USAGE_EARLY_DISPLAY    = 1 << 3,
    };

    class UsageHint
    {
    public:
        eUsageMode mMode = USAGE_FULL;
        MSize mStreamingSize;
        MSize mVendorCusSize;
        NSCam::EIS::EisInfo mEISInfo;
        MUINT32 mVendorMode = 0;
        MUINT32 m3DNRMode = 0;
        MUINT32 mFSCMode = 0;
        MUINT32 mDualMode = 0;
        MUINT32 mVendorFOVMode = 0;
        MBOOL   mUseTSQ = MFALSE;
        SecType mSecType = SecType::mem_normal;

        UsageHint();
        UsageHint(eUsageMode mode, const MSize &streamingSize);

        MVOID enable3DNRModeMask(NSCam::NR3D::E3DNR_MODE_MASK mask)
        {
            m3DNRMode |= mask;
        }
    };

public:
    virtual ~IStreamingFeaturePipe() {}

public:
    // interface for PipelineNode
    static IStreamingFeaturePipe* createInstance(MUINT32 openSensorIndex, const UsageHint &usageHint);
    MBOOL destroyInstance(const char *name=NULL);

    virtual MBOOL init(const char *name=NULL) = 0;
    virtual MBOOL uninit(const char *name=NULL) = 0;
    virtual MBOOL enque(const FeaturePipeParam &param) = 0;
    virtual MBOOL flush() = 0;
    virtual MBOOL setJpegParam(NSCam::NSIoPipe::NSPostProc::EJpgCmd cmd, int arg1, int arg2) = 0;
    virtual MBOOL setFps(MINT32 fps) = 0;
    virtual MUINT32 getRegTableSize() = 0;
    virtual MBOOL sendCommand(NSCam::NSIoPipe::NSPostProc::ESDCmd cmd, MINTPTR arg1=0, MINTPTR arg2=0, MINTPTR arg3=0) = 0;
    virtual MBOOL addMultiSensorId(MUINT32 sensorId) = 0;

public:
    // sync will block until all data are processed,
    // use with caution and avoid deadlock !!
    virtual MVOID sync() = 0;

    virtual IImageBuffer* requestBuffer() = 0;
    virtual MBOOL returnBuffer(IImageBuffer *buffer) = 0;

protected:
    IStreamingFeaturePipe() {}
};

class FeaturePipeParam
{
public:
    enum MSG_TYPE { MSG_FRAME_DONE, MSG_DISPLAY_DONE, MSG_RSSO_DONE, MSG_FD_DONE, MSG_P2B_SET_3A, MSG_N3D_SET_SHOTMODE };
    typedef MBOOL (*CALLBACK_T)(MSG_TYPE, FeaturePipeParam&);

    VarMap mVarMap;
    MUINT32 mFeatureMask;
    CALLBACK_T mCallback;
    NSCam::NSIoPipe::QParams mQParams;

    FeaturePipeParam()
        : mFeatureMask(0)
        , mCallback(NULL)
    {
    }

    FeaturePipeParam(CALLBACK_T callback)
        : mFeatureMask(0)
        , mCallback(callback)
    {
    }

    MVOID setFeatureMask(MUINT32 mask, MBOOL enable)
    {
        if( enable )
        {
            mFeatureMask |= mask;
        }
        else
        {
            mFeatureMask &= (~mask);
        }
    }

    MVOID setQParams(NSCam::NSIoPipe::QParams &qparams)
    {
        mQParams = qparams;
    }

    NSCam::NSIoPipe::QParams getQParams() const
    {
        return mQParams;
    }

    DECLARE_VAR_MAP_INTERFACE(mVarMap, setVar, getVar, tryGetVar, clearVar);
};

}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#undef MAKE_FEATURE_MASK_FUNC

#endif // _MTK_CAMERA_FEATURE_PIPE_I_STREAMING_FEATURE_PIPE_H_
