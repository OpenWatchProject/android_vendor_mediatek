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

#define LOG_TAG "ABF/UT"
#include <mtkcam/utils/std/Log.h>

#include "AbfAdpUtil.h"
#include "AbfAdapterUT.h"


#define utAssert(_cond, _statement) \
    do { \
        if (!_cond) { \
            CAM_LOGE("[ASSERT FAIL] %s: %s : %s", __func__, #_cond, _statement); \
        } \
    } while (0)

#define utAssertRet(_cond, _statement, _ret) \
    do { \
        if (!_cond) { \
            CAM_LOGE("[ASSERT FAIL] %s: %s : %s", __func__, #_cond, _statement); \
            return _ret; \
        } \
    } while (0)


// Copied from Standalone_SWABF.cpp
static constexpr ABFTuningInfo DEFAULT_TUNING_INFO = {
    240, 240, 240, 8, 15,
    190, -190,
    0, 1, 236, 252,
    96, 108, 196, 210,
    122, 130, 184, 205,
    64, -24,
    56, -36,
    511, -8,
    0, 0,
    16, 22, 20, 4,
    0, 100,
    0xabf,
    1
};

static_assert(DEFAULT_TUNING_INFO.ABF_CHECK == 0xabf, "ABFTuningInfo was modified");


// --------------------------------------------------------------------------
//  MTKABFMock
// --------------------------------------------------------------------------

MTKABF* MTKABFMock::createInstance(DrvABFObject_e )
{
    return new MTKABFMock();
}

void MTKABFMock::destroyInstance(MTKABF *obj)
{
    delete obj;
}

MTKABFMock::MTKABFMock() :
    isInited(false), isMainCalled(false), isReset(false)
{
    mResult.RetCode = E_ABF_WRONG_STATE;
}

MTKABFMock::~MTKABFMock()
{
    utAssert(isInited == isReset, "ABFInit() & ABFReset() should be called in pair");
}


MRESULT MTKABFMock::ABFInit(void *)
{
    FUNCTION_IN_OUT_LOG();

    isInited = true;
    return S_ABF_OK;
}

MRESULT MTKABFMock::ABFMain(void)
{
    FUNCTION_IN_OUT_LOG();

    utAssertRet(isInited, "", E_ABF_ERR);
    utAssertRet(isInCtrlHist(ABF_FEATURE_ADD_IMG), "", E_ABF_ERR);
    isMainCalled = true;
    mResult.RetCode = S_ABF_OK;
    return S_ABF_OK;
}

MRESULT MTKABFMock::ABFReset(void)
{
    FUNCTION_IN_OUT_LOG();

    utAssertRet(isInited, "", E_ABF_WRONG_STATE);
    isReset = true;
    return S_ABF_OK;
}

MRESULT MTKABFMock::ABFFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut)
{
    FUNCTION_IN_OUT_LOG();
    CAM_LOGD("ABFFeatureCtrl(%u, %p, %p)", FeatureID, pParaIn, pParaOut);

    addCtrlHist(FeatureID, pParaIn);
    return doABFFeatureCtrl(FeatureID, pParaIn, pParaOut);
}


MRESULT MTKABFMock::doABFFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut)
{
    FUNCTION_IN_OUT_LOG();

    switch (FeatureID) {
    case ABF_FEATURE_CHECK_ENABLE:
        {
        MINT32 *abfSwitch = static_cast<MINT32*>(pParaOut);
        *abfSwitch = 1;
        }
        break;

    case ABF_FEATURE_GET_WORKBUF_SIZE:
        {
        const ABFImageInfo *imageInfo = static_cast<const ABFImageInfo*>(pParaIn);
        MUINT32 *pBufferSize = static_cast<MUINT32*>(pParaOut);
        *pBufferSize = imageInfo->Width * imageInfo->Height * 4;
        }
        break;

    case ABF_FEATURE_SET_WORKBUF_ADDR:
        utAssertRet(isInCtrlHist(ABF_FEATURE_GET_WORKBUF_SIZE), "", E_ABF_ERR);
        break;

    case ABF_FEATURE_ADD_IMG:
        {
        const ABFImageInfo *imageInfo = static_cast<const ABFImageInfo*>(pParaIn);
        for (int i = 0; i < 3; i++)
            mResult.pImg[i] = imageInfo->pImg[i];
        mResult.ImgNum = 3;
        mResult.ImgFmt = imageInfo->ImgFmt;
        mResult.Width = imageInfo->Width;
        mResult.Height = imageInfo->Height;
        }
        break;

    case ABF_FEATURE_GET_RESULT:
        {
        utAssertRet(isMainCalled, "", E_ABF_WRONG_STATE);
        ABFResultInfo *out = static_cast<ABFResultInfo*>(pParaOut);
        *out = mResult;
        return mResult.RetCode;
        }
    }

    return S_ABF_OK;
}


// --------------------------------------------------------------------------
//  AbfAdapterUT
// --------------------------------------------------------------------------

AbfAdapterUT::~AbfAdapterUT()
{
    if (mOwnTuningInfo)
        delete mTuningInfo;
}


void AbfAdapterUT::setMock(MTKABF *mock)
{
    mpAbfAlgo->destroyInstance(mpAbfAlgo);

    if (mock != NULL)
        mpAbfAlgo = mock;
    else
        mpAbfAlgo = MTKABFMock::createInstance();
}


void* AbfAdapterUT::getTuningData(const ProcessParam &)
{
    return const_cast<void*>(static_cast<const void*>(&DEFAULT_TUNING_INFO));
}


