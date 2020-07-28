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

#ifndef _MTKCAM_FEATURE_UTILS_P2_PLAT_INFO_H_
#define _MTKCAM_FEATURE_UTILS_P2_PLAT_INFO_H_

#include <mtkcam/def/common.h>
#include <mtkcam/drv/iopipe/PortMap.h>

// === PQ related: start ===
//!!NOTES: the following structures are same as those defined in [custom]/common/hal/inc/camera_custom_pq.h
typedef enum P2_PQ_INDEX
{
    P2_PQ_DEFAULT           = 0,
    P2_PQ_NORMAL            = 1,
    // EIS
    P2_PQ_EIS12_VIDEO_4k    = 2,
    P2_PQ_EIS30_VIDEO       = 3,
    P2_PQ_EIS30_VIDEO_4k    = 4,
    P2_PQ_EIS35_VIDEO       = 5,
    P2_PQ_EIS35_VIDEO_4k    = 6,
    P2_PQ_EIS35_NO_VIDEO    = 7,
    // SLOW MOTION
    P2_PQ_SMVRCONSTRAINT    = 8,
    P2_PQ_SMVRBATCH         = 9,

    //!!NOTES: do not change FEATURE_PQ_NUM
    P2_PQ_MAX_NUM,
} eP2_PQ_INDEX;

// --- PQ PATH ---
typedef enum P2_PQ_PATH
{
    P2_PQ_PATH_DISPLAY   = 0 ,
    P2_PQ_PATH_RECORD    = 1,
    P2_PQ_PATH_VSS       = 2,
    P2_PQ_PATH_OTHER     = 3,
} eP2_PQ_PATH;

#define P2_PQ_PATH_ENABLE_DISPLAY(x)              (x) |= (1<<P2_PQ_PATH_DISPLAY)
#define P2_PQ_PATH_DISABLE_DISPLAY(x)             (x) &=~(1<<P2_PQ_PATH_DISPLAY)
#define P2_PQ_PATH_IS_DISPLAY_ENABLED(x)          ((x& (1<<P2_PQ_PATH_DISPLAY))?true:false)

#define P2_PQ_PATH_ENABLE_RECORD(x)               (x) |= (1<<P2_PQ_PATH_RECORD)
#define P2_PQ_PATH_DISABLE_RECORD(x)              (x) &=~(1<<P2_PQ_PATH_RECORD)
#define P2_PQ_PATH_IS_RECORD_ENABLED(x)           ((x& (1<<P2_PQ_PATH_RECORD))?true:false)

#define P2_PQ_PATH_ENABLE_VSS(x)                  (x) |= (1<<P2_PQ_PATH_VSS)
#define P2_PQ_PATH_DISABLE_VSS(x)                 (x) &=~(1<<P2_PQ_PATH_VSS)
#define P2_PQ_PATH_IS_VSS_ENABLED(x)              ((x& (1<<P2_PQ_PATH_VSS))?true:false)

typedef struct P2PQCtrl
{
    // ref: CUSTOM_PQ_PATH: DISPLAY=0, RECORD=1, VSS=2
    uint32_t czEnableMask = 0;
    uint32_t dreEnableMask = 0;
    uint32_t hfgEnableMask = 0;
} P2PQCtrl;
// === PQ related: end ===
namespace NSCam {
namespace Feature {
namespace P2Util {

class P2PlatInfo
{
public:
    enum FeatureID {
        FID_NONE,
        FID_CLEARZOOM,
        FID_DRE,
        FID_HFG,
        FID_DSDN,
    };

    class NVRamData {
    public:
        void* mLowIsoData = NULL;
        void* mHighIsoData = NULL;
        uint32_t mLowIsoIndex = 0;
        uint32_t mHighIsoIndex = 0;
        uint32_t mLowerIso = 0;
        uint32_t mUpperIso = 0;
    };

    class NVRamDSDN {
    public:
        uint32_t mIsoThreshold = 3200;
        uint32_t mRatioMultiple = 1;
        uint32_t mRatioDivider = 3;
    };

public:
    static const P2PlatInfo* getInstance(MUINT32 sensorID);

public:
    virtual ~P2PlatInfo() {}
    virtual MBOOL isDip50() const = 0;
    virtual MBOOL supportDefaultPQ() const = 0;
    virtual MBOOL supportClearZoom() const = 0;
    virtual MBOOL supportDRE() const = 0;
    virtual MBOOL supportHFG() const = 0;
    virtual MBOOL supportSecure3DNR() const = 0;
    virtual MRect getActiveArrayRect() const = 0;
    virtual P2PlatInfo::NVRamDSDN queryDSDN(MINT32 magic3A, MUINT8 ispProfile) const = 0;
    virtual P2PlatInfo::NVRamData queryNVRamData(FeatureID fID, MINT32 magic3A, MUINT8 ispProfile) const = 0;
    virtual MUINT32 getImgoAlignMask() const = 0;

    virtual NSCam::NSIoPipe::PortID getLsc2Port() const = 0;
    virtual NSCam::NSIoPipe::PortID getBpc2Port() const = 0;
    virtual NSCam::NSIoPipe::PortID getYnrFaceiPort() const = 0;
    virtual NSCam::NSIoPipe::PortID getYnrLceiPort() const = 0;
    virtual MBOOL hasYnrFacePort() const = 0;
    virtual MBOOL hasYnrLceiPort() const = 0;

    virtual MBOOL supportDefaultPQPath(eP2_PQ_PATH /* pqPath */ ) const { return MFALSE; }
    virtual MBOOL supportCZPath(MUINT32 /* pqIdx */, eP2_PQ_PATH /* pqPath */ ) const  { return MFALSE; }
    virtual MBOOL supportDREPath(MUINT32 /* pqIdx */ , eP2_PQ_PATH /* pqPath*/ ) const { return MFALSE; }
    virtual MBOOL supportHFGPath(MUINT32 /* pqIdx */, eP2_PQ_PATH /* pqPath */ ) const { return MFALSE; }

protected:
    P2PlatInfo() {}
};

} // namespace P2Util
} // namespace Feature
} // namespace NSCam

#endif // _MTKCAM_FEATURE_UTILS_P2_PLAT_INFO_H_
