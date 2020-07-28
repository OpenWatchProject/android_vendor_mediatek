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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

// Standard C header file

// Android system/core header file
#include <cutils/properties.h>
// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "DCMFIspPipeFlowUtility.h"

// logging
#undef PIPE_CLASS_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "DCMFIspPipeFlowUtility"
#include <featurePipe/core/include/PipeLog.h>

class scoped_tracer
{
public:
    scoped_tracer(const char* functionName)
    : mFunctionName(functionName)
    {
        CAM_LOGD("[%s] +", mFunctionName);
    }
    ~scoped_tracer()
    {
        CAM_LOGD("[%s] -", mFunctionName);
    }
private:
    const char* const mFunctionName;
};
#define SCOPED_TRACER() scoped_tracer ___scoped_tracer(__FUNCTION__ );
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {


MBOOL
DCMFIspPipeFlowUtility::
imgMemoryCopy(IImageBuffer* pDstYUVImg, const IImageBuffer* pSrcYUVImg, MRect crop)
{
    SCOPED_TRACER();

    using NSCam::NSIoPipe::NSSImager::IImageTransform;

    MBOOL bRet = MTRUE;
    if(::property_get_int32("vendor.debug.stereo.cpumcpy", 0) == 1)
    {
        MY_LOGD("cpu cpy");
        for (size_t i = 0; i < pSrcYUVImg->getPlaneCount(); i++)
        {
            MVOID* pSrcYUVImgVa = (MVOID*) (pSrcYUVImg->getBufVA(i));
            MVOID* pDstYUVImgVa = (MVOID*) (pDstYUVImg->getBufVA(i));
            MUINT32 uBufSize = pSrcYUVImg->getBufSizeInBytes(i);
            memcpy(pDstYUVImgVa, pSrcYUVImgVa, uBufSize);
        }
    }
    else
    {
        MY_LOGD("mdp cpy");
        // use IImageTransform to handle image cropping
        std::unique_ptr<IImageTransform, std::function<MVOID(IImageTransform*)>> transform(
                IImageTransform::createInstance(), // constructor
                [](IImageTransform *p){ if (p) p->destroyInstance(); } // deleter
                );

        if (transform.get() == nullptr)
        {
            MY_LOGE("IImageTransform is nullptr, cannot generate output");
            bRet = MFALSE;
        }
        else
        {
            MRect crop = MRect(pSrcYUVImg->getImgSize().w, pSrcYUVImg->getImgSize().h);
            bRet = transform->execute(
                pSrcYUVImg,     // src
                pDstYUVImg,     // dst1
                nullptr,        // dst2
                crop,           // ratio crop
                0,              // no transform
                3000            // timeout
            );

            if(!bRet)
            {
                MY_LOGE("Failed doIImageTransform!");
            }
        }
    }
    pDstYUVImg->syncCache(eCACHECTRL_FLUSH);
    return MTRUE;
}

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam
