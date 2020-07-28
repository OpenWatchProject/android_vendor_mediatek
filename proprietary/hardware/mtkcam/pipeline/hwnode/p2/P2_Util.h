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

#include "P2_Request.h"

#ifdef HAL3A_SIMULATOR_IMP
typedef NSCam::v3::IHal3ASimulator IHal3A_T;
#ifdef MAKE_Hal3A
#undef MAKE_Hal3A
#endif
#define MAKE_Hal3A(...) IHal3A_T::createInstance(__VA_ARGS__)
#else
typedef NS3Av3::IHal3A IHal3A_T;
#endif

namespace P2
{

class P2Util
{
public:
    enum FindOutMask {  FIND_NO_ROTATE  = 0x01,
                        FIND_ROTATE     = 0x02,
                        FIND_DISP       = 0x04,
                        FIND_VIDEO      = 0x08, };
    enum ReleaseMask { RELEASE_DISP = 0x01,
                       RELEASE_FD   = 0x02, };
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
        MVOID addMargin(const char* name, const MSize &margin);
        MBOOL hasMargin() const;
        MVOID updateResult(MBOOL result) const;
        MVOID dropRecord() const;
        MVOID earlyRelease(MUINT32 mask, MBOOL result);
        sp<P2Img> getMDPSrc() const;
    private:
        MBOOL mResized;
        MBOOL mUseLMV;
        MBOOL mUseMargin;
        MSize mMargin;
        sp<P2Img> mIMGI;
        sp<P2Img> mLCEI;
        sp<P2Img> mIMG2O;
        sp<P2Img> mIMG3O;
        sp<P2Img> mWROTO;
        sp<P2Img> mWDMAO;
        friend class P2Util;
    };

public:
    static auto getDebugExif();
    static MBOOL is4K2K(const MSize &size);
    static MUINT32 getDipVersion();
    static MBOOL isDip50(MUINT32 version);

    static SimpleIO extractSimpleIO(const sp<P2Request> &request, MUINT32 portFlag = 0);
    static MBOOL getActiveArrayRect(MUINT32 sensorID, MRect &rect, const Logger &logger = Logger());
    static MVOID process3A(const SimpleIO &io, MUINT32 tuningSize, IHal3A_T *hal3A, TuningParam &tuning, P2MetaSet &metaSet, const Logger &logger = Logger());
    static TuningParam prepareTuning(const SimpleIO &io, MUINT32 tuningSize, IHal3A_T *hal3A, MetaSet_T inMetaSet, MetaSet_T *pOutMetaSet, const Logger &logger = Logger());
    static MVOID releaseTuning(TuningParam &tuning, const Logger &logger = Logger());

    static sp<P2Img> extractOut(const sp<P2Request> &request, MUINT32 target = 0);

    static QParams makeSimpleQParams(ENormalStreamTag tag, const SimpleIO &io, const TuningParam &tuning, const Cropper &cropper, const Logger &logger = Logger());
    static MVOID prepareClearZoom(QParams &param, const Logger &logger = Logger());
    static MVOID printQParams(const QParams &params, const Logger &logger = Logger());
    static MVOID prepareExtraModule(QParams &qparams, const P2ExtraData &extraData, const Logger &logger = Logger());
    static MVOID releaseExtraModule(QParams &qparams, const Logger &logger = Logger());

    static MVOID updateDebugExif(const IMetadata &inHal, IMetadata &outHal, const Logger &logger = Logger());
    static MVOID updateExtraMeta(const P2ExtraData &extraData, IMetadata &outHal, const Logger &logger = Logger());

private:
    static EPortCapbility toCapability(MUINT32 usage);
    static MVOID prepareIn(QFRAME_T &frame, const PortID &portID, IImageBuffer *buffer);
    static MVOID prepareOut(QFRAME_T &frame, const PortID &portID, const sp<P2Img> &img);
    static MVOID prepareCrop(QFRAME_T &frame, MUINT32 cropID, const MCropRect &crop, const MSize size);
    static MBOOL updateCropRegion(IMetadata *meta, const Cropper &cropper);
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_UTIL_H_
