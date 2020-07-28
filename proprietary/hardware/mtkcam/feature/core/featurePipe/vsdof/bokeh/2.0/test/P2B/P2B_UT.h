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
#ifndef _MTK_CAMERA_FEATURE_PIPE_P2B_UT_H_
#define _MTK_CAMERA_FEATURE_PIPE_P2B_UT_H_
//
#include <utils/RefBase.h>
#include <mtkcam/feature/effectHalBase/IEffectHal.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
#include <ImageBufferPool.h>
#include <vector>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace std;
//
namespace VSDOF
{
namespace Bokeh
{
namespace UT
{
struct P2B_UT_Params
{
    MINT32 scenario = 0;
    MBOOL is3DNREnable = MFALSE;
    MINT32 gmv_x = -1;
    MINT32 gmv_y = -1;
    MINT32 bokehLevel = 0;
    MBOOL isAFTrigger = MFALSE;
    MINT32 ptAF_x = -1;
    MINT32 ptAF_y = -1;
    MINT32 iGSensor = -1;
};
//
class P2B_UT
{
    public:
        P2B_UT() = default;
        ~P2B_UT();
        MBOOL init(P2B_UT_Params params);
        MBOOL uninit();
        MBOOL generate(
                    sp<EffectRequest>& effect/*,
                    SmartImageBuffer& dmg,
                    SmartImageBuffer& dmbg*/);
    private:
        MBOOL preareInputData();
        MVOID setImageBufferValue(
                    SmartImageBuffer& buffer,
                    MINT32 width,
                    MINT32 height,
                    MINT32 value);//
        MBOOL createBufferPool(
                android::sp<ImageBufferPool>& pPool,
                MUINT32 width,
                MUINT32 height,
                NSCam::EImageFormat format,
                MUINT32 bufCount,
                const char* caller,
                MUINT32 bufUsage = ImageBufferPool::USAGE_HW);
        sp<IImageBuffer>            loadImgFromFile(
                                    const IImageBufferAllocator::ImgParam imgParam,
                                    const char* path,
                                    const char* name,
                                    MINT usage);
        sp<IImageBuffer>            createEmptyImageBuffer(
                                    const IImageBufferAllocator::ImgParam imgParam,
                                    const char* name,
                                    MINT usage);
    private:
        vector<sp<IImageBuffer> >   mvMainImageBuffer;
        vector<sp<IImageBuffer> >   mvMainImageBuffer_Cap;
        //sp<ImageBufferPool>         mpDMGBufPool = nullptr;
        //sp<ImageBufferPool>         mpDMBGBufPool= nullptr;
        vector<sp<IImageBuffer> >   mvDMGImageBuffer;
        vector<sp<IImageBuffer> >   mvDMBGImageBuffer;
        vector<sp<IImageBuffer> >   mvWDMAImageBuffer;
        vector<sp<IImageBuffer> >   mvWROTImageBuffer;
        P2B_UT_Params               mParams;
        MINT32                      miIndex = 0;
};
};
};
};
#endif // _MTK_CAMERA_FEATURE_PIPE_P2B_UT_H_