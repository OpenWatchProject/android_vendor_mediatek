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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_VNR_HAL_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_VNR_HAL_H_

#include "StreamingFeatureData.h"
//#include "vnr_lib/include/MTKDSDN.h"
#include <libdsdn/MTKDSDN.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class VNRHal
{
public:
    static MBOOL isSimulate();
    static MBOOL isSupport(EImageFormat format);

public:
    VNRHal();
    ~VNRHal();

    MBOOL init(const MSize &maxSize);
    MBOOL uninit();
    MBOOL process(MUINT32 fps, const ImgBuffer &inFullImg, const ImgBuffer &inDS1Img, const ImgBuffer &inDS2Img, const ImgBuffer &outFullImg);

private:
    MBOOL checkState() const;
    MBOOL checkInput(const IImageBuffer *full, const IImageBuffer *ds1, const IImageBuffer *ds2, const IImageBuffer *out) const;

    MBOOL cleanUp();
    MBOOL createDSDN();
    MBOOL initDSDN();
    MBOOL initDSDNBuffer();
    MBOOL uninitDSDN();
    MBOOL uninitDSDNBuffer();

    IImageBuffer* getBuffer(const ImgBuffer &img) const;
    MBOOL prepareProcInfo(struct DSDNProcInfo &info, MUINT32 fps, const IImageBuffer* full, const IImageBuffer *ds1, const IImageBuffer *ds2, const IImageBuffer *out) const;
    MVOID printProcInfo(const struct DSDNProcInfo &info) const;
    MBOOL addBufferInfo(struct DSDNProcInfo &info, DSDN_BUFFER_TYPE_ENUM type, const IImageBuffer *img) const;
    MBOOL doDSDN(const struct DSDNProcInfo &info);

    DSDN_BUFFER_FMT_ENUM toDSDNFormat(EImageFormat fmt) const;
    const char* toString(DSDN_BUFFER_FMT_ENUM) const;

private:
    MTKDSDN *mDSDN = NULL;
    MSize mMaxSize;
    MUINT32 mWorkBufSize = 0;
    MUINT8 *mWorkBuf = NULL;
    MUINT32 mPrintIO = 0;
};

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_VNR_HAL_H_
