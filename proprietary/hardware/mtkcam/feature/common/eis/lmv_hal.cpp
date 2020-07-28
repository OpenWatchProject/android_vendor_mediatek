/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
*      TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file eis_hal.cpp
*
* EIS Hal Source File
*
*/

#include <cstdio>
#include <queue>
#include <cutils/atomic.h>
#include <utils/threads.h>
#include <utils/SystemClock.h>
#include <utils/Trace.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include "lmv_drv.h"

#include <android/sensor.h>
#include <mtkcam/utils/sys/SensorProvider.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam/utils/imgbuf/IDummyImageBufferHeap.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <camera_custom_nvram.h>
#include <camera_custom_eis.h>

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::Utils;

#include "lmv_hal_imp.h"

#if 1
#undef  ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_ALWAYS
#define DP_TRACE_CALL()                 ATRACE_CALL()
#define DP_TRACE_BEGIN(name)            ATRACE_BEGIN(name)
#define DP_TRACE_END()                  ATRACE_END()

#else

#define DP_TRACE_CALL()
#define DP_TRACE_BEGIN(name)
#define DP_TRACE_END()

#endif  // CONFIG_FOR_SYSTRACE


/*******************************************************************************
*
********************************************************************************/
#define LMV_HAL_DEBUG

#ifdef LMV_HAL_DEBUG

#undef __func__
#define __func__ __FUNCTION__

#undef  LOG_TAG
#define LOG_TAG "LMVHal"
#include <mtkcam/utils/std/Log.h>

#define LMV_LOG(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define LMV_INF(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define LMV_WRN(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define LMV_ERR(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else
#define LMV_LOG(a,...)
#define LMV_INF(a,...)
#define LMV_WRN(a,...)
#define LMV_ERR(a,...)
#endif

#define LMV_HAL_NAME "LMVHal"
#define intPartShift 8
#define floatPartShift (31 - intPartShift)
#define DEBUG_DUMP_FRAMW_NUM 10

template <> LMVHalObj<0>* LMVHalObj<0>::spInstance = 0;
template <> LMVHalObj<1>* LMVHalObj<1>::spInstance = 0;
template <> LMVHalObj<2>* LMVHalObj<2>::spInstance = 0;
template <> LMVHalObj<3>* LMVHalObj<3>::spInstance = 0;

template <> Mutex LMVHalObj<0>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex LMVHalObj<1>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex LMVHalObj<2>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex LMVHalObj<3>::s_instMutex(::Mutex::PRIVATE);


MINT32 LMVHalImp::mDebugDump = 0;
const MUINT32 GyroInterval_ms = 20;


#define EISO_BUFFER_NUM (30)

/*******************************************************************************
*
********************************************************************************/
LMVHal *LMVHal::CreateInstance(char const *userName,const MUINT32 &aSensorIdx)
{
    LMV_LOG("%s",userName);
    return LMVHalImp::GetInstance(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
LMVHal *LMVHalImp::GetInstance(const MUINT32 &aSensorIdx)
{
    LMV_LOG("sensorIdx(%u)",aSensorIdx);

    switch(aSensorIdx)
    {
        case 0 : return LMVHalObj<0>::GetInstance();
        case 1 : return LMVHalObj<1>::GetInstance();
        case 2 : return LMVHalObj<2>::GetInstance();
        case 3 : return LMVHalObj<3>::GetInstance();
        default :
            LMV_WRN("Current limit is 4 sensors, use 0");
            return LMVHalObj<0>::GetInstance();
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID LMVHalImp::DestroyInstance(char const *userName)
{
    LMV_LOG("%s",userName);
}

/*******************************************************************************
*
********************************************************************************/
LMVHalImp::LMVHalImp(const MUINT32 &aSensorIdx) : LMVHal()
{
    mUsers = 0;

    //> EIS driver object
    m_pLMVDrv = NULL;

    //> member variable
    mEisInput_W = 0;
    mEisInput_H = 0;
    mP1Target_W = 0;
    mP1Target_H = 0;
    mDoEisCount = 0;    //Vent@20140427: Add for EIS GMV Sync Check.
    mCmvX_Int = 0;
    mCmvX_Flt = 0;
    mCmvY_Int = 0;
    mCmvY_Flt = 0;
    mGMV_X = 0;
    mGMV_Y = 0;
    mMAX_GMV = LMV_MAX_GMV_DEFAULT;
    mMVtoCenterX = 0;
    mMVtoCenterY = 0;
    mFrameCnt = 0;
    mEisPass1Enabled = 0;
    mIsEisConfig = 0;
    mSensorIdx = aSensorIdx;
    mSensorDev = 0;
    mEisSupport = MTRUE;
    mMemAlignment = 0;
    mEisPlusCropRatio = 100;
    mVideoW = 0;
    mVideoH = 0;
    mEisPlusResult.GridX = 0;
    mEisPlusResult.GridY = 0;
    mEisPlusResult.ClipX = 0;
    mEisPlusResult.ClipY = 0;

    memset(&mSensorStaticInfo, 0, sizeof(mSensorStaticInfo));
    memset(&mSensorDynamicInfo, 0, sizeof(mSensorDynamicInfo));

    mDebugDump = 0;

    m_pEisAlg = NULL;

    memset(&mEisAlgoProcData, 0, sizeof(mEisAlgoProcData));

    mEisLastData2EisPlus.ConfX = mEisLastData2EisPlus.ConfY = 0;
    mEisLastData2EisPlus.GMVx = mEisLastData2EisPlus.GMVy = 0.0;
    mBufIndex = 0;
    mpSensorProvider = NULL;
    while(!mEISOBufferList.empty())
    {
        mEISOBufferList.pop();
    }

    m_pEisDbgBuf = NULL;
    // imagesensor
    m_pHalSensorList = NULL;

    // eis result data
    while(!mEis2EisPlusGmvTS.empty())
    {
        mEis2EisPlusGmvTS.pop();
    }

    while(!mEis2EisPlusGmvX.empty())
    {
        mEis2EisPlusGmvX.pop();
    }

    while(!mEis2EisPlusGmvY.empty())
    {
        mEis2EisPlusGmvY.pop();
    }

    while(!mEis2EisPlusConfX.empty())
    {
        mEis2EisPlusConfX.pop();
    }

    while(!mEis2EisPlusConfY.empty())
    {
        mEis2EisPlusConfY.pop();
    }
}

/*******************************************************************************
*
********************************************************************************/
MINT32 LMVHalImp::Init(const MUINT32 eisFactor)
{
    DP_TRACE_CALL();

    //====== Check Reference Count ======
    Mutex::Autolock lock(mLock);
    MUINT32 index;
    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        LMV_LOG("snesorIdx(%u) has %d users",mSensorIdx,mUsers);
        return LMV_RETURN_NO_ERROR;
    }

    MINT32 err = LMV_RETURN_NO_ERROR;

    //====== Dynamic Debug ======
    mDebugDump = 0;
    mDebugDump = property_get_int32("vendor.debug.lmv.dump", mDebugDump);

    mBufIndex = 0;
    mVideoW = 0;
    mVideoH = 0;
    mEisPass1Enabled = 0;

    LMV_LOG("(%p)  mSensorIdx(%u) init", this, mSensorIdx);

    //====== Create Sensor Object ======

    m_pHalSensorList = MAKE_HalSensorList();
    if(m_pHalSensorList == NULL)
    {
        LMV_ERR("IHalSensorList::get fail");
        goto create_fail_exit;
    }

    if(LMV_RETURN_NO_ERROR != GetSensorInfo())
    {
        LMV_ERR("GetSensorInfo fail");
        goto create_fail_exit;
    }

    //====== Create EIS Driver ======

    m_pLMVDrv = LMVDrv::CreateInstance(mSensorIdx);

    if(m_pLMVDrv == NULL)
    {
        LMV_ERR("LMVDrv::createInstance fail");
        goto create_fail_exit;
    }

    err = m_pLMVDrv->Init();
    if(err != LMV_RETURN_NO_ERROR)
    {
        LMV_ERR("LMVDrv::Init fail");
        goto create_fail_exit;
    }

    //====== Create EIS Algorithm Object ======

    LMV_LOG("TG(%d)",mSensorDynamicInfo.TgInfo);

    DP_TRACE_BEGIN("CreateMultiMemBuf");

    //Create EISO output buffer
    CreateMultiMemBuf(EISO_MEMORY_SIZE, (EISO_BUFFER_NUM+1), m_pEISOMainBuffer, m_pEISOSliceBuffer);
    if (!m_pEISOSliceBuffer[0]->getBufVA(0))
    {
        LMV_ERR("EISO slice buf create ImageBuffer fail");
        return LMV_RETURN_MEMORY_ERROR;
    }
    {

        Mutex::Autolock lock(mEISOBufferListLock);
        for (index =0; index<EISO_BUFFER_NUM; index++)
        {
            mEISOBufferList.push(m_pEISOSliceBuffer[index]);
        }
    }
    DP_TRACE_END();
    // set EIS12Factor as default for CMV calculation
    mEisPlusCropRatio = eisFactor > 100 ? eisFactor : EISCustom::getEIS12Factor();
    LMV_LOG("mEisPlusCropRatio(%u)", mEisPlusCropRatio);

    //====== Increase User Count ======

    android_atomic_inc(&mUsers);

    LMV_LOG("-");
    return LMV_RETURN_NO_ERROR;

create_fail_exit:

    if (m_pLMVDrv != NULL)
    {
        m_pLMVDrv->Uninit();
        m_pLMVDrv->DestroyInstance();
        m_pLMVDrv = NULL;
    }

    if (m_pHalSensorList != NULL)
    {
        m_pHalSensorList = NULL;
    }
    LMV_LOG("-");
    return LMV_RETURN_NULL_OBJ;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 LMVHalImp::Uninit()
{
    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers <= 0)
    {
        LMV_LOG("mSensorIdx(%u) has 0 user",mSensorIdx);
        return LMV_RETURN_NO_ERROR;
    }

    //====== Uninitialize ======

    android_atomic_dec(&mUsers);    //decrease referebce count

    if(mUsers == 0)    // there is no user
    {
        MINT32 err = LMV_RETURN_NO_ERROR;

        LMV_LOG("mSensorIdx(%u) uninit", mSensorIdx);

        if(m_pLMVDrv != NULL)
        {
            LMV_LOG("m_pLMVDrv uninit");

            m_pLMVDrv->Uninit();
        	m_pLMVDrv->DestroyInstance();
        	m_pLMVDrv = NULL;
        }

        //======  Release EIS Algo Object ======

#if EIS_ALGO_READY

        if (UNLIKELY(mDebugDump >= 2))
        {
            if(mSensorDynamicInfo.TgInfo == CAM_TG_1)
            {
                err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SAVE_LOG, NULL, NULL);
        	    if(err != S_EIS_OK)
        	    {
        	        LMV_ERR("EisFeatureCtrl(EIS_FEATURE_SAVE_LOG) fail(0x%x)",err);
        	    }
            }
        }


        LMV_LOG("TG(%d)",mSensorDynamicInfo.TgInfo);

        if (m_pEisAlg != NULL)
        {
            LMV_LOG("m_pEisAlg uninit");
            m_pEisAlg->EisReset();
            m_pEisAlg->destroyInstance(m_pEisAlg);
            m_pEisAlg = NULL;
        }

#endif

        // Next-Gen EIS

        if (mpSensorProvider != NULL)
        {
            mpSensorProvider->disableSensor(SENSOR_TYPE_GYRO);
            mpSensorProvider = NULL;
        }


        //====== Destroy Sensor Object ======

        if(m_pHalSensorList != NULL)
        {
            m_pHalSensorList = NULL;
        }

        //======  Release Memory and IMem Object ======


        if (UNLIKELY(mDebugDump >= 2))
        {
            //>  free EIS debug buffer
            m_pEisDbgBuf->unlockBuf("EISDbgBuf");
            DestroyMemBuf(m_pEisDbgBuf);
        }

        DestroyMultiMemBuf((EISO_BUFFER_NUM+1), m_pEISOMainBuffer, m_pEISOSliceBuffer);

        //======  Reset Member Variable ======

        mEisInput_W = 0;
        mEisInput_H = 0;
        mP1Target_W = 0;
        mP1Target_H = 0;
        mFrameCnt = 0; // first frmae
        mEisPass1Enabled = 0;
        mIsEisConfig = 0;
        mCmvX_Int = 0;
        mDoEisCount = 0;
        mCmvX_Flt = 0;
        mCmvY_Int = 0;
        mMVtoCenterX = 0;
        mMVtoCenterY = 0;
        mCmvY_Flt = 0;
        mGMV_X = 0;
        mGMV_Y = 0;
        mMAX_GMV = LMV_MAX_GMV_DEFAULT;
        mVideoW = 0;
        mVideoH = 0;
        mMemAlignment = 0;
        mEisLastData2EisPlus.ConfX = mEisLastData2EisPlus.ConfY = 0;
        mEisLastData2EisPlus.GMVx = mEisLastData2EisPlus.GMVy = 0.0;

        mBufIndex = 0;

        mDebugDump = 0;

        {
            Mutex::Autolock lock(mEISOBufferListLock);
            while(!mEISOBufferList.empty())
            {
                mEISOBufferList.pop();
            }
        }

        while(!mEis2EisPlusGmvTS.empty())
        {
            mEis2EisPlusGmvTS.pop();
        }

        while(!mEis2EisPlusGmvX.empty())
        {
            mEis2EisPlusGmvX.pop();
        }

        while(!mEis2EisPlusGmvY.empty())
        {
            mEis2EisPlusGmvY.pop();
        }

        while(!mEis2EisPlusConfX.empty())
        {
            mEis2EisPlusConfX.pop();
        }

        while(!mEis2EisPlusConfY.empty())
        {
            mEis2EisPlusConfY.pop();
        }

    }
    else
    {
        LMV_LOG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }
    return LMV_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 LMVHalImp::CreateMultiMemBuf(MUINT32 memSize, MUINT32 num, android::sp<IImageBuffer>& spMainImageBuf, android::sp<IImageBuffer> spImageBuf[MAX_MEMORY_SIZE])
{
    MINT32 err = LMV_RETURN_NO_ERROR;
    MUINT32 totalSize = memSize*num;

    if (num >= MAX_MEMORY_SIZE)
    {
        LMV_ERR("num of image buffer is larger than MAX_MEMORY_SIZE(%d)",MAX_MEMORY_SIZE);
        return LMV_RETURN_MEMORY_ERROR;
    }

    IImageBufferAllocator::ImgParam imgParam(totalSize, 0);

    sp<IIonImageBufferHeap> pHeap = IIonImageBufferHeap::create("LMV_HAL", imgParam);
    if (pHeap == NULL)
    {
        LMV_ERR("image buffer heap create fail");
        return LMV_RETURN_MEMORY_ERROR;
    }

    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN |
                        GRALLOC_USAGE_SW_WRITE_OFTEN | // ISP3 is software-write
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    spMainImageBuf = pHeap->createImageBuffer();
    if (spMainImageBuf == NULL)
    {
        LMV_ERR("mainImage buffer create fail");
        return LMV_RETURN_MEMORY_ERROR;
    }
    if (!(spMainImageBuf->lockBuf("LMV_HAL", usage)))
    {
        LMV_ERR(" image buffer lock fail");
        return LMV_RETURN_MEMORY_ERROR;
    }
    MUINTPTR const iVAddr = pHeap->getBufVA(0);
    MUINTPTR const iPAddr = pHeap->getBufPA(0);
    MINT32 const iHeapId = pHeap->getHeapID();

    LMV_LOG("IIonImageBufferHeap iVAddr:%p, iPAddr:%p, iHeapId:%d\n", (void*)iVAddr, (void*)iPAddr, iHeapId);
    LMV_LOG("spMainImageBuf iVAddr:%p, iPAddr:%p\n",(void*)spMainImageBuf->getBufVA(0),(void*)spMainImageBuf->getBufPA(0));

    MUINT32 index;
    size_t bufStridesInBytes[] = {memSize, 0, 0};
    size_t bufBoundaryInBytes[] = {0, 0, 0};

    for (index = 0; index < num; index++)
    {
        MUINTPTR const cVAddr = iVAddr + ((index)*(memSize));
        MUINTPTR const virtAddr[] = {cVAddr, 0, 0};
        MUINTPTR const cPAddr = iPAddr + ((index)*(memSize));
        MUINTPTR const phyAddr[] = {cPAddr, 0, 0};
        IImageBufferAllocator::ImgParam imgParam_t = IImageBufferAllocator::ImgParam(memSize, 0);
        PortBufInfo_dummy portBufInfo = PortBufInfo_dummy(iHeapId, virtAddr, phyAddr, 1);
        sp<IImageBufferHeap> imgBufHeap = IDummyImageBufferHeap::create("LMV_HAL", imgParam_t, portBufInfo, false);
        if (imgBufHeap == NULL)
        {
            LMV_ERR("acquire LMV_HAL - image buffer heap create fail");
            return LMV_RETURN_MEMORY_ERROR;
        }

        sp<IImageBuffer> imgBuf = imgBufHeap->createImageBuffer();
        if (imgBuf == NULL)
        {
            LMV_ERR("acquire LMV_HAL - image buffer create fail");
            return LMV_RETURN_MEMORY_ERROR;
        }

        if (!(imgBuf->lockBuf("LMV_HAL", usage)))
        {
            LMV_ERR("acquire LMV_HAL - image buffer lock fail");
            return LMV_RETURN_MEMORY_ERROR;
        }

        spImageBuf[index] = imgBuf;
    }
    LMV_LOG("X");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 LMVHalImp::CreateMemBuf(MUINT32 memSize, android::sp<IImageBuffer>& spImageBuf)
{
    MINT32 err = LMV_RETURN_NO_ERROR;
    IImageBufferAllocator* pImageBufferAlloc = IImageBufferAllocator::getInstance();

    LMV_LOG("Size(%u)",memSize);
    IImageBufferAllocator::ImgParam bufParam((size_t)memSize, 0);
    spImageBuf = pImageBufferAlloc->alloc("LMV_HAL", bufParam);
    LMV_LOG("X");
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 LMVHalImp::DestroyMultiMemBuf(MUINT32 num, android::sp<IImageBuffer>& spMainImageBuf, android::sp<IImageBuffer> spImageBuf[MAX_MEMORY_SIZE])
{
    MINT32 err = LMV_RETURN_NO_ERROR;
    MUINT32 index;
    for (index = 0; index < num; index++)
    {
        spImageBuf[index]->unlockBuf("LMV_HAL");
        spImageBuf[index] = NULL;
    }

    spMainImageBuf->unlockBuf("LMV_HAL");
    spMainImageBuf = NULL;

    LMV_LOG("X");
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 LMVHalImp::DestroyMemBuf(android::sp<IImageBuffer>& spImageBuf)
{
    MINT32 err = LMV_RETURN_NO_ERROR;
    IImageBufferAllocator* pImageBufferAlloc = IImageBufferAllocator::getInstance();
    LMV_LOG("DestroyMemBuf");


    if (spImageBuf != NULL)
    {
        pImageBufferAlloc->free(spImageBuf.get());
        spImageBuf = NULL;
    }

    LMV_LOG("X");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 LMVHalImp::GetSensorInfo()
{
    LMV_LOG("mSensorIdx(%u)",mSensorIdx);

    mSensorDev = m_pHalSensorList->querySensorDevIdx(mSensorIdx);
    m_pHalSensorList->querySensorStaticInfo(mSensorDev,&mSensorStaticInfo);

    m_pHalSensor = m_pHalSensorList->createSensor(LMV_HAL_NAME,1,&mSensorIdx);
    if(m_pHalSensor == NULL)
    {
        LMV_ERR("m_pHalSensorList->createSensor fail");
        return LMV_RETURN_API_FAIL;
    }

    if(MFALSE == m_pHalSensor->querySensorDynamicInfo(mSensorDev,&mSensorDynamicInfo))
    {
        LMV_ERR("querySensorDynamicInfo fail");
        return LMV_RETURN_API_FAIL;
    }

    m_pHalSensor->destroyInstance(LMV_HAL_NAME);
    m_pHalSensor = NULL;

    return LMV_RETURN_NO_ERROR;
}

MVOID LMVHalImp::EnableGyroSensor()
{
    //====== Create SensorProvider Object ======
    DP_TRACE_BEGIN("SensorProvider");
    if (mpSensorProvider == NULL)
    {
        mpSensorProvider = SensorProvider::createInstance(LMV_HAL_NAME);
        MUINT32 interval = property_get_int32("vendor.eisrecord.setinterval", GyroInterval_ms);

        if (mpSensorProvider->enableSensor(SENSOR_TYPE_GYRO, interval))
        {
            LMV_LOG("Enable SensorProvider success");
        }
        else
        {
            LMV_ERR("Enable SensorProvider fail");
        }
    }

    DP_TRACE_END();
}


/*******************************************************************************
*
********************************************************************************/
MINT32 LMVHalImp::ConfigLMV(const LMV_HAL_CONFIG_DATA &aEisConfig)
{
    if(mEisSupport == MFALSE)
    {
        LMV_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return LMV_RETURN_NO_ERROR;
    }

    MINT32 err = LMV_RETURN_NO_ERROR;

#if EIS_ALGO_READY
    static EIS_SET_ENV_INFO_STRUCT eisAlgoInitData;
#endif
    //====== Check Sensor Type ======

    EIS_SENSOR_ENUM sensorType;

    switch(aEisConfig.sensorType)
    {
        case NSCam::NSSensorType::eRAW:
            sensorType = EIS_RAW_SENSOR;
            break;
        case NSCam::NSSensorType::eYUV:
            sensorType = EIS_YUV_SENSOR;
            break;
        default:
            LMV_ERR("not support sensor type(%u), use RAW setting",aEisConfig.sensorType);
            sensorType = EIS_RAW_SENSOR;
            break;
    }

#if EIS_ALGO_READY
    //====== EIS Algo Init ======

    if (UNLIKELY(mDebugDump >= 1))
    {
        LMV_LOG("mIsEisConfig(%u)",mIsEisConfig);
    }

    if (mIsEisConfig == 0)
    {
        //> get EIS customize setting

        GetEisCustomize(&eisAlgoInitData.eis_tuning_data);

        eisAlgoInitData.Eis_Input_Path = EIS_PATH_RAW_DOMAIN;   // RAW domain

        //> init EIS algo
        if ( mSensorDynamicInfo.TgInfo == CAM_TG_NONE)
        {
            //Reget sensor information
            if(LMV_RETURN_NO_ERROR != GetSensorInfo())
            {
                LMV_ERR("GetSensorInfo fail");
            }

            LMV_LOG("TG(%d)",mSensorDynamicInfo.TgInfo);

        }

        if (m_pEisAlg == NULL)
        {
            m_pEisAlg = MTKEis::createInstance();
            if(m_pEisAlg == NULL)
            {
                LMV_ERR("MTKEis::createInstance fail");
                return LMV_RETURN_UNKNOWN_ERROR;
            }
        }

        err = m_pEisAlg->EisInit(&eisAlgoInitData);
        if (err != S_EIS_OK)
        {
            LMV_ERR("EisInit fail(0x%x)",err);
            return LMV_RETURN_API_FAIL;
        }

        if (UNLIKELY(mDebugDump >= 2))
        {
            //====== EIS Debug Buffer ======

            MUINT32 eisMemSize = 0;
            EIS_SET_LOG_BUFFER_STRUCT eisAlgoLogInfo;

            err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_DEBUG_INFO,NULL,&eisMemSize);
            if (err != S_EIS_OK || eisMemSize == 0)
            {
                LMV_ERR("EisFeatureCtrl(EIS_FEATURE_GET_DEBUG_INFO) fail(0x%x)",err);
                eisAlgoLogInfo.Eis_Log_Buf_Addr = NULL;
                eisAlgoLogInfo.Eis_Log_Buf_Size = 0;

            }
            else
            {
                CreateMemBuf(eisMemSize, m_pEisDbgBuf);
                m_pEisDbgBuf->lockBuf("EISDbgBuf", eBUFFER_USAGE_SW_MASK);
                if (!m_pEisDbgBuf->getBufVA(0))
                {
                    LMV_ERR("mEisDbgBuf create ImageBuffer fail");
                    return LMV_RETURN_MEMORY_ERROR;
                }

                LMV_LOG("mEisDbgBuf : size(%u),virAdd(0x%p)", eisMemSize, (void*)m_pEisDbgBuf->getBufVA(0));
                eisAlgoLogInfo.Eis_Log_Buf_Addr = (MVOID *)m_pEisDbgBuf->getBufVA(0);
                eisAlgoLogInfo.Eis_Log_Buf_Size = eisMemSize;

            }
            err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SET_DEBUG_INFO, &eisAlgoLogInfo, NULL);
            if(err != S_EIS_OK)
            {
                LMV_ERR("EisFeatureCtrl(EIS_FEATURE_SET_DEBUG_INFO) fail(0x%x)",err);
            }
        }

        mTsForAlgoDebug = 0;


        if (UNLIKELY(mDebugDump >= 1))
        {
            LMV_LOG("EIS tuning_data");
            LMV_LOG("sensitivity(%d)",eisAlgoInitData.eis_tuning_data.sensitivity);
            LMV_LOG("filter_small_motion(%u)",eisAlgoInitData.eis_tuning_data.filter_small_motion);
            LMV_LOG("adv_shake_ext(%u)",eisAlgoInitData.eis_tuning_data.adv_shake_ext);
            LMV_LOG("stabilization_strength(%f)",eisAlgoInitData.eis_tuning_data.stabilization_strength);
            LMV_LOG("advtuning_data.new_tru_th(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.new_tru_th);
            LMV_LOG("advtuning_data.vot_th(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.vot_th);
            LMV_LOG("advtuning_data.votb_enlarge_size(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.votb_enlarge_size);
            LMV_LOG("advtuning_data.min_s_th(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.min_s_th);
            LMV_LOG("advtuning_data.vec_th(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.vec_th);
            LMV_LOG("advtuning_data.spr_offset(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.spr_offset);
            LMV_LOG("advtuning_data.spr_gain1(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.spr_gain1);
            LMV_LOG("advtuning_data.spr_gain2(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.spr_gain2);
            LMV_LOG("advtuning_data.vot_his_method(%d)",eisAlgoInitData.eis_tuning_data.advtuning_data.vot_his_method);
            LMV_LOG("advtuning_data.smooth_his_step(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.smooth_his_step);
            LMV_LOG("advtuning_data.eis_debug(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.eis_debug);

            for(MINT32 i = 0; i < 4; ++i)
            {
                LMV_LOG("gmv_pan_array[%d]=%u",i,eisAlgoInitData.eis_tuning_data.advtuning_data.gmv_pan_array[i]);
                LMV_LOG("gmv_sm_array[%d]=%u",i,eisAlgoInitData.eis_tuning_data.advtuning_data.gmv_sm_array[i]);
                LMV_LOG("cmv_pan_array[%d]=%u",i,eisAlgoInitData.eis_tuning_data.advtuning_data.cmv_pan_array[i]);
                LMV_LOG("cmv_sm_array[%d]=%u",i,eisAlgoInitData.eis_tuning_data.advtuning_data.cmv_sm_array[i]);
            }
        }
#endif

        //===== EIS HW Config ======

        err = m_pLMVDrv->ConfigLMVReg(mSensorDynamicInfo.TgInfo);
        if (err != LMV_RETURN_NO_ERROR)
        {
            LMV_ERR("ConfigEisReg fail(0x%x)",err);
            return LMV_RETURN_API_FAIL;
        }

        //====== Turn on Eis Configure One Time Flag ======
        mIsEisConfig = 1;
        {
            Mutex::Autolock lock(mLock);
            mEisPass1Enabled = 1;
        }
    }
    return LMV_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 LMVHalImp::DoLMVCalc(const EIS_PASS_ENUM &aEisPass, QBufInfo const &pBufInfo)
{
    MINT32 err = LMV_RETURN_NO_ERROR;
    const MUINT64 aTimeStamp = pBufInfo.mvOut[0].mMetaData.mTimeStamp; //Maybe framedone

    if (mEisSupport == MFALSE)
    {
        LMV_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return LMV_RETURN_EISO_MISS;
    }

    if (UNLIKELY(mTsForAlgoDebug == 0))
    {
        mTsForAlgoDebug = aTimeStamp;
    }

    if (UNLIKELY(mDebugDump >= 1))
    {
        LMV_LOG("mSensorIdx=%u, aEisPass(%d),mEisPass1Enabled(%u)", mSensorIdx, aEisPass,mEisPass1Enabled);
    }

    if (aTimeStamp <= 0)
    {
        LMV_LOG("DoP1Eis aTimeStamp is not reasonable(%" PRIi64 ")",aTimeStamp);
    }
    else
    {
#if EIS_ALGO_READY

        //====== Setting EIS Algo Process Data ======

        EIS_RESULT_INFO_STRUCT   eisCMVResult;
        MINTPTR eisoBufferVA = 0;

        //rrzo
        for (size_t i = 0; i < pBufInfo.mvOut.size(); i++)
        {
            if (pBufInfo.mvOut[i].mPortID.index == PORT_RRZO.index)
            {
                //crop region
                mEisInput_W = pBufInfo.mvOut.at(i).mMetaData.mDstSize.w;
                mEisInput_H = pBufInfo.mvOut.at(i).mMetaData.mDstSize.h;
            }

            if (pBufInfo.mvOut[i].mPortID.index == PORT_EISO.index)
            {
                eisoBufferVA= pBufInfo.mvOut.at(i).mBuffer->getBufVA(0);
            }
        }


        {
            //> EIS input image size
            Mutex::Autolock lock(mP1Lock);

            if(m_pLMVDrv->Get2PixelMode() == 1)
            {
                mEisInput_W >>= 1;
            }else if(m_pLMVDrv->Get2PixelMode() == 2)
            {
                mEisInput_W >>= 2;
            }

            mEisInput_W -= 4;   //ryan wang request to -4
            mEisInput_H -= 4;   //ryan wang request to -4

            mP1Target_W = (mEisInput_W / (mEisPlusCropRatio / 100.0));
            mP1Target_H = (mEisInput_H / (mEisPlusCropRatio / 100.0));

            mEisAlgoProcData.eis_image_size_config.InputWidth   = mEisInput_W;
            mEisAlgoProcData.eis_image_size_config.InputHeight  = mEisInput_H;
            mEisAlgoProcData.eis_image_size_config.TargetWidth  = mP1Target_W;
            mEisAlgoProcData.eis_image_size_config.TargetHeight = mP1Target_H;
        }

        if (UNLIKELY(mDebugDump >= 0))
        {
            LMV_LOG("mEisPlusCropRatio(%u), mSensorIdx=%u, EisIn(%u,%u),P1T(%u,%u)", mEisPlusCropRatio, mSensorIdx, mEisInput_W,mEisInput_H,mP1Target_W,mP1Target_H);
        }

        //> get EIS HW statistic
        if (LMV_RETURN_EISO_MISS == m_pLMVDrv->GetEisHwStatistic(eisoBufferVA, &mEisAlgoProcData.eis_state))
        {
            LMV_WRN("EISO data miss");

            //> use latst data

            mEis2EisPlusGmvTS.push(aTimeStamp);
            mEis2EisPlusGmvX.push(mEisLastData2EisPlus.GMVx);
            mEis2EisPlusGmvY.push(mEisLastData2EisPlus.GMVy);
            mEis2EisPlusConfX.push(mEisLastData2EisPlus.ConfX);
            mEis2EisPlusConfY.push(mEisLastData2EisPlus.ConfY);

            return LMV_RETURN_NO_ERROR;
        }

        if (UNLIKELY(mDebugDump == 3))
        {
            DumpStatistic(mEisAlgoProcData.eis_state);
        }

        //> get EIS HW setting of eis_op_vert and eis_op_hori

        mEisAlgoProcData.DivH = m_pLMVDrv->GetLMVDivH();
        mEisAlgoProcData.DivV = m_pLMVDrv->GetLMVDivV();

        //> get MB number

        mEisAlgoProcData.EisWinNum = m_pLMVDrv->GetLMVMbNum();

        // get gyro/acceleration data
        SensorData gyroData;
        SensorData accData;
        MBOOL gyroValid = mpSensorProvider->getLatestSensorData(SENSOR_TYPE_GYRO, gyroData);
        //MBOOL accValid = mpSensorProvider->getLatestSensorData(SENSOR_TYPE_ACCELERATION, accData);
        MBOOL accValid = false;

        {
            Mutex::Autolock lock(mP1Lock);

            mEisAlgoProcData.sensor_info.GyroValid = gyroValid;
            mEisAlgoProcData.sensor_info.Gvalid = accValid;

            if (gyroValid)
            {
                mEisAlgoProcData.sensor_info.GyroInfo[0] = gyroData.gyro[0];
                mEisAlgoProcData.sensor_info.GyroInfo[1] = gyroData.gyro[1];
                mEisAlgoProcData.sensor_info.GyroInfo[2] = gyroData.gyro[2];
            }
        }

        if (UNLIKELY(mDebugDump >= 1))
        {
            LMV_LOG("EN:(Acc,Gyro)=(%d,%d)",accValid,gyroValid);
            LMV_LOG("EIS:Acc(%f,%f,%f)",mEisAlgoProcData.sensor_info.AcceInfo[0],mEisAlgoProcData.sensor_info.AcceInfo[1],mEisAlgoProcData.sensor_info.AcceInfo[2]);
            LMV_LOG("EIS:Gyro(%f,%f,%f)",mEisAlgoProcData.sensor_info.GyroInfo[0],mEisAlgoProcData.sensor_info.GyroInfo[1],mEisAlgoProcData.sensor_info.GyroInfo[2]);
        }

        //====== EIS Algorithm ======

        err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SET_PROC_INFO, &mEisAlgoProcData, NULL);
        if (err != S_EIS_OK)
        {
            LMV_ERR("EisAlg:LMV_FEATURE_SET_PROC_INFO fail(0x%x)",err);
            err = LMV_RETURN_API_FAIL;
            return err;
        }

        err = m_pEisAlg->EisMain(&eisCMVResult);
        if (err != S_EIS_OK)
        {
            LMV_ERR("EisAlg:EisMain fail(0x%x), mSensorIdx=%u",err, mSensorIdx);
            err = LMV_RETURN_API_FAIL;
            return err;
        }

        //====== Get EIS Result to EIS Plus ======

        EIS_GET_PLUS_INFO_STRUCT eisData2EisPlus;

        err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_EIS_PLUS_DATA, NULL, &eisData2EisPlus);
        if (err != S_EIS_OK)
        {
            LMV_ERR("EisAlg:LMV_FEATURE_GET_LMV_PLUS_DATA fail(0x%x)",err);
            err = LMV_RETURN_API_FAIL;
            return err;
        }

        {
            Mutex::Autolock lock(mP2Lock);

            if(m_pLMVDrv->Get2PixelMode() == 1)
            {
                if(mDebugDump > 0)
                {
                    LMV_LOG("eisData2EisPlus.GMVx *= 2");
                }
                eisData2EisPlus.GMVx *= 2.0f;
            }else if(m_pLMVDrv->Get2PixelMode() == 2)
            {
                if(mDebugDump > 0)
                {
                    LMV_LOG("eisData2EisPlus.GMVx *= 4");
                }
                eisData2EisPlus.GMVx *= 4.0f;
            }

            //> keep lateset result for EisPlus

            mEisLastData2EisPlus.GMVx  = eisData2EisPlus.GMVx;
            mEisLastData2EisPlus.GMVy  = eisData2EisPlus.GMVy;
            mEisLastData2EisPlus.ConfX = eisData2EisPlus.ConfX;
            mEisLastData2EisPlus.ConfY = eisData2EisPlus.ConfY;

            //> save to queue
            mEis2EisPlusGmvTS.push(aTimeStamp);
            mEis2EisPlusGmvX.push(eisData2EisPlus.GMVx);
            mEis2EisPlusGmvY.push(eisData2EisPlus.GMVy);
            mEis2EisPlusConfX.push(eisData2EisPlus.ConfX);
            mEis2EisPlusConfY.push(eisData2EisPlus.ConfY);

            while (mEis2EisPlusGmvTS.size() > 30)
            {
                mEis2EisPlusGmvTS.pop();
            }

            while (mEis2EisPlusGmvX.size() > 30)
            {
                mEis2EisPlusGmvX.pop();
            }

            while (mEis2EisPlusGmvY.size() > 30)
            {
                mEis2EisPlusGmvY.pop();
            }

            while (mEis2EisPlusConfX.size() > 30)
            {
                mEis2EisPlusConfX.pop();
            }

            while (mEis2EisPlusConfY.size() > 30)
            {
                mEis2EisPlusConfY.pop();
            }
        }

        //====== Get GMV ======

        EIS_GMV_INFO_STRUCT eisGMVResult;

        err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_ORI_GMV, NULL, &eisGMVResult);
        if(err != S_EIS_OK)
        {
            LMV_ERR("EisAlg:LMV_FEATURE_GET_ORI_GMV fail(0x%x)",err);
            err = LMV_RETURN_API_FAIL;
            return err;
        }

        //====== Save EIS CMV and GMV =======

        if(m_pLMVDrv->Get2PixelMode() == 1)
        {
            if(mDebugDump > 0)
            {
                LMV_LOG("eisGMVResult.LMV_GMVx *= 2");
            }

            eisGMVResult.EIS_GMVx *= 2.0f;
        }else if(m_pLMVDrv->Get2PixelMode() == 2)
        {
            if(mDebugDump > 0)
            {
                LMV_LOG("eisGMVResult.LMV_GMVx *= 4");
            }

            eisGMVResult.EIS_GMVx *= 4.0f;
        }

        mGMV_X = eisGMVResult.EIS_GMVx;
        mGMV_Y = eisGMVResult.EIS_GMVy;
        mMAX_GMV = m_pLMVDrv->GetLMVMaxGmv();

        //====== Prepare EIS Result ======

        PrepareEisResult(eisCMVResult.CMV_X,eisCMVResult.CMV_Y);

        //====== Get First Frame Info ======

        mFrameCnt = m_pLMVDrv->GetFirstFrameInfo();

        if (UNLIKELY(mDebugDump >= 1))
        {
            LMV_LOG("mFrameCnt(%u)",mFrameCnt);
        }

        //====== Not The First Frame ======

        if (mFrameCnt == 0)
        {
            LMV_LOG("not first frame");
            mFrameCnt = 1;
        }
#endif
    }

    mDoEisCount++;

    return LMV_RETURN_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MVOID LMVHalImp::PrepareEisResult(const MINT32 &cmvX, const MINT32 &cmvY)
{
    if (UNLIKELY(mDebugDump >= 1))
    {
        LMV_LOG("cmvX(%d),cmvY(%d)",cmvX,cmvY);
    }

    Mutex::Autolock lock(mP1Lock);

    //====== Boundary Checking ======

    if(cmvX < 0)
    {
        LMV_ERR("cmvX should not be negative(%u), fix to 0",cmvX);

        mCmvX_Int = mCmvX_Flt = 0;
    }
    else
    {
        MFLOAT tempCMV_X = cmvX / 256.0;
        MINT32 tempFinalCmvX = cmvX;
        mMVtoCenterX = cmvX;

        if((tempCMV_X + (MFLOAT)mP1Target_W) > (MFLOAT)mEisInput_W)
        {
            LMV_LOG("cmvX too large(%u), fix to %u",cmvX,(mEisInput_W - mP1Target_W));

            tempFinalCmvX = (mEisInput_W - mP1Target_W);
        }

        mMVtoCenterX -=  ((mEisInput_W-mP1Target_W)<<(intPartShift-1)); //Make mv for the top-left of center

        if(m_pLMVDrv->Get2PixelMode() == 1)
        {
            if(mDebugDump > 0)
            {
                LMV_LOG("tempFinalCmvX *= 2");
            }

            tempFinalCmvX *= 2;
            mMVtoCenterX *= 2;
        }else if(m_pLMVDrv->Get2PixelMode() == 2)
        {
            if(mDebugDump > 0)
            {
                LMV_LOG("tempFinalCmvX *= 4");
            }

            tempFinalCmvX *= 4;
            mMVtoCenterX *= 4;
        }

        mCmvX_Int = (tempFinalCmvX & (~0xFF)) >> intPartShift;
        mCmvX_Flt = (tempFinalCmvX & (0xFF)) << floatPartShift;
    }

    if(cmvY < 0)
    {
        LMV_ERR("cmvY should not be negative(%u), fix to 0",cmvY);

        mCmvY_Int = mCmvY_Flt = 0;
    }
    else
    {
        MFLOAT tempCMV_Y = cmvY / 256.0;
        MINT32 tempFinalCmvY = cmvY;
        mMVtoCenterY = cmvY;

        if((tempCMV_Y + (MFLOAT)mP1Target_H) > (MFLOAT)mEisInput_H)
        {
            LMV_LOG("cmvY too large(%u), fix to %u",cmvY,(mEisInput_H - mP1Target_H));

            tempFinalCmvY = (mEisInput_H - mP1Target_H);
        }
        mMVtoCenterY -=  ((mEisInput_H-mP1Target_H)<<(intPartShift-1)); //Make mv for the top-left of center

        mCmvY_Int = (tempFinalCmvY & (~0xFF)) >> intPartShift;
        mCmvY_Flt = (tempFinalCmvY & (0xFF)) << floatPartShift;
    }

    if (mDebugDump > 0)
    {
        LMV_LOG("X(%u,%u),Y(%u,%u),MVtoCenter (%d,%d)",mCmvX_Int, mCmvX_Flt,
                                                   mCmvY_Int, mCmvY_Flt, mMVtoCenterX, mMVtoCenterY);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID LMVHalImp::GetLMVResult(MUINT32 &a_CMV_X_Int,
                                 MUINT32 &a_CMV_X_Flt,
                                 MUINT32 &a_CMV_Y_Int,
                                 MUINT32 &a_CMV_Y_Flt,
                                 MUINT32 &a_TarWidth,
                                 MUINT32 &a_TarHeight,
                                 MINT32  &a_MVtoCenterX,
                                 MINT32  &a_MVtoCenterY,
                                 MUINT32 &a_isFromRRZ)
{
    if(mEisSupport == MFALSE)
    {
        LMV_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        a_CMV_X_Int = 0;
        a_CMV_X_Flt = 0;
        a_CMV_Y_Int = 0;
        a_CMV_Y_Flt = 0;
        a_TarWidth  = 0;
        a_TarHeight = 0;
        a_MVtoCenterX = 0;
        a_MVtoCenterY = 0;
        a_isFromRRZ = 0;
        return;
    }

    {
        Mutex::Autolock lock(mP1Lock);

        a_CMV_X_Int = mCmvX_Int;
        a_CMV_X_Flt = mCmvX_Flt;
        a_CMV_Y_Int = mCmvY_Int;
        a_CMV_Y_Flt = mCmvY_Flt;
        a_TarWidth  = mP1Target_W;
        a_TarHeight = mP1Target_H;
        a_MVtoCenterX = mMVtoCenterX;
        a_MVtoCenterY = mMVtoCenterY;
        a_isFromRRZ = 1; //Hardcode MUST be fix later!!!!
    }

    if(mDebugDump >= 1)
    {
        LMV_LOG("X(%u,%u),Y(%u,%u)",a_CMV_X_Int,a_CMV_X_Flt,a_CMV_Y_Int,a_CMV_Y_Flt);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID LMVHalImp::GetGmv(MINT32 &aGMV_X, MINT32 &aGMV_Y, MUINT32 *confX, MUINT32 *confY, MUINT32 *MAX_GMV)
{
    if(mEisSupport == MFALSE)
    {
        LMV_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return;
    }

    aGMV_X = mGMV_X;
    aGMV_Y = mGMV_Y;

    if(MAX_GMV != NULL)
    {
        *MAX_GMV = mMAX_GMV;
    }

    {
        Mutex::Autolock lock(mP1Lock);

        if(confX != NULL)
        {
            *confX = mEisLastData2EisPlus.ConfX;
        }

        if(confY != NULL)
        {
            *confY = mEisLastData2EisPlus.ConfY;
        }
    }

    if (UNLIKELY(mDebugDump >= 1))
    {
        if (confX && confY)
        {
            LMV_LOG("GMV(%d,%d),Conf(%d,%d)", aGMV_X, aGMV_Y, *confX, *confY);
        }else
        {
            LMV_LOG("GMV(%d,%d)", aGMV_X, aGMV_Y);
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
MBOOL LMVHalImp::GetLMVSupportInfo(const MUINT32 &aSensorIdx)
{
    mEisSupport = m_pLMVDrv->GetLMVSupportInfo(aSensorIdx);
    return mEisSupport;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 LMVHalImp::GetEisPlusCropRatio()
{
#if RECORD_WITHOUT_LMV_ENABLE
    if (g_AIMDump == 1)
    {
        return LMV_NONE_FACTOR;
    }
#endif
    if(mDebugDump >= 1)
    {
        LMV_LOG("mEisPlusCropRatio(%d)",mEisPlusCropRatio);
    }
    return mEisPlusCropRatio;
}


/*******************************************************************************
*
********************************************************************************/
MVOID LMVHalImp::GetLMVInputSize(MUINT32 *aWidth, MUINT32 *aHeight)
{
    if(NULL != m_pLMVDrv)
    {
        m_pLMVDrv->GetLMVInputSize(aWidth, aHeight);
    }else
    {
        LMV_ERR("m_pLMVDrv is NULL");
    }
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 LMVHalImp::GetEisDivH()
{
    if(NULL != m_pLMVDrv)
    {
        return m_pLMVDrv->GetLMVDivH();
    }else
    {
        LMV_ERR("m_pLMVDrv is NULL");
    }
    return 1;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 LMVHalImp::GetEisDivV()
{
    if(NULL != m_pLMVDrv)
    {
        return m_pLMVDrv->GetLMVDivV();
    }else
    {
        LMV_ERR("m_pLMVDrv is NULL");
    }
     return 1;
}

/*******************************************************************************
*
********************************************************************************/
MVOID LMVHalImp::GetRegSetting(void *data)
{
    if(NULL != m_pLMVDrv)
    {
        m_pLMVDrv->GetRegSetting(data);
    }else
    {
        LMV_ERR("m_pLMVDrv is NULL");
    }
}

MUINT32 LMVHalImp::GetLMVStatus()
{
    Mutex::Autolock lock(mLock);

    MUINT32 retVal = mEisPass1Enabled;
    return retVal;
}

MINT32 LMVHalImp::GetBufLMV(android::sp<IImageBuffer>& spBuf)
{
    Mutex::Autolock lock(mEISOBufferListLock);

    if (!mEISOBufferList.empty())
    {
        spBuf = mEISOBufferList.front();
        mEISOBufferList.pop();
        if (UNLIKELY(mDebugDump >= 1))
        {
            LMV_LOG("GetBufEis : %zu",mEISOBufferList.size());
        }
    }else
    {
        spBuf = m_pEISOSliceBuffer[EISO_BUFFER_NUM];

        LMV_WRN("GetBufEis empty!!");
    }
    return LMV_RETURN_NO_ERROR;
}

MSize LMVHalImp::QueryMinSize(MBOOL isEISOn, MSize sensorSize, NSCam::MSize outputSize, MSize requestSize, MSize FovMargin)
{
    MSize retSize;
    MINT32 out_width;
    MINT32 out_height;
    if (MFALSE == isEISOn)
    {
        out_width = (requestSize.w <= 160)? 160 : requestSize.w;
        out_height = (requestSize.h <= 160)? 160 : requestSize.h;
    }else
    {
        if ( mVideoW == 0 && outputSize.w != 0)
            mVideoW = outputSize.w;

        if ( mVideoH == 0 && outputSize.h != 0)
            mVideoH = outputSize.h;

        if ( (mVideoW  < VR_UHD_W) && (mVideoH  < VR_UHD_H))
        {
            out_width = (requestSize.w <= (EIS_FE_MAX_INPUT_W + FovMargin.w))? (EIS_FE_MAX_INPUT_W + FovMargin.w) : requestSize.w;
            out_height = (requestSize.h <= (EIS_FE_MAX_INPUT_H + FovMargin.h))? (EIS_FE_MAX_INPUT_H + FovMargin.h) : requestSize.h;
        }else
        {
            MSize EISPlusFOV;
            EISPlusFOV.w = (VR_UHD_W*mEisPlusCropRatio/100.0f) + FovMargin.w;
            EISPlusFOV.h = (VR_UHD_H*mEisPlusCropRatio/100.0f) + FovMargin.h;
            out_width  = (requestSize.w <= EISPlusFOV.w)?
                         EISPlusFOV.w : requestSize.w;
            out_height = (requestSize.h <= EISPlusFOV.h)?
                         EISPlusFOV.h : requestSize.h;
            out_width = (out_width <= sensorSize.w)? out_width : sensorSize.w;
            out_height = (out_height <= sensorSize.h)? out_height : sensorSize.h;
            if (((out_width*9)>>4) < out_height)
            {
                //Align video view angle
                out_height = (out_width*9)>>4;
            }
        }

        if (UNLIKELY(mDebugDump >= 1))
        {
            LMV_LOG("eis(%d), sensor: %d/%d, outputSize: %d/%d, videoSize: %d/%d, ret: %d/%d, crop %d",
                    isEISOn, sensorSize.w, sensorSize.h, outputSize.w, outputSize.h, mVideoW, mVideoH , out_width, out_height, mEisPlusCropRatio);
        }
    }
    retSize = MSize(out_width,out_height);


    return retSize;
}


MINT32 LMVHalImp::NotifyLMV(QBufInfo&   pBufInfo)
{
    android::sp<IImageBuffer> retBuf;
    for (size_t i = 0; i < pBufInfo.mvOut.size(); i++)
    {
        if (pBufInfo.mvOut[i].mPortID.index == PORT_EISO.index)
        {

            Mutex::Autolock lock(mEISOBufferListLock);

            retBuf = (pBufInfo.mvOut.at(i).mBuffer);
            mEISOBufferList.push(retBuf);
            if (UNLIKELY(mDebugDump >= 1))
            {
                LMV_LOG("NotifyLMV : %zu",mEISOBufferList.size());
            }
        }
    }

    return LMV_RETURN_NO_ERROR;
}

MINT32 LMVHalImp::NotifyLMV(android::sp<NSCam::IImageBuffer>& spBuf)
{
    Mutex::Autolock lock(mEISOBufferListLock);

    if (spBuf != 0)
    {
        mEISOBufferList.push(spBuf);
        if (UNLIKELY(mDebugDump >= 1))
        {
            LMV_LOG("NotifyLMV : %zu - Drop",mEISOBufferList.size());
        }
    }

    return LMV_RETURN_NO_ERROR;
}


#if EIS_ALGO_READY

/*******************************************************************************
*
********************************************************************************/
MVOID LMVHalImp::GetEisStatistic(EIS_STATISTIC_STRUCT *a_pLMV_Stat)
{
    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        a_pLMV_Stat->i4LMV_X[i]    = mEisAlgoProcData.eis_state.i4LMV_X[i];
        a_pLMV_Stat->i4LMV_Y[i]    = mEisAlgoProcData.eis_state.i4LMV_Y[i];
        a_pLMV_Stat->i4LMV_X2[i]   = mEisAlgoProcData.eis_state.i4LMV_X2[i];
        a_pLMV_Stat->i4LMV_Y2[i]   = mEisAlgoProcData.eis_state.i4LMV_Y2[i];
        a_pLMV_Stat->NewTrust_X[i] = mEisAlgoProcData.eis_state.NewTrust_X[i];
        a_pLMV_Stat->NewTrust_Y[i] = mEisAlgoProcData.eis_state.NewTrust_Y[i];
        a_pLMV_Stat->SAD[i]        = mEisAlgoProcData.eis_state.SAD[i];
        a_pLMV_Stat->SAD2[i]       = mEisAlgoProcData.eis_state.SAD2[i];
        a_pLMV_Stat->AVG_SAD[i]    = mEisAlgoProcData.eis_state.AVG_SAD[i];
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID LMVHalImp::GetEisCustomize(EIS_TUNING_PARA_STRUCT *a_pDataOut)
{
    if(mDebugDump >= 1)
    {
        LMV_LOG("+");
    }

    EIS_Customize_Para_t customSetting;

    EISCustom::getEISData(&customSetting);

    a_pDataOut->sensitivity            = (EIS_SENSITIVITY_ENUM)customSetting.sensitivity;
    a_pDataOut->filter_small_motion    = customSetting.filter_small_motion;
    a_pDataOut->adv_shake_ext          = customSetting.adv_shake_ext;  // 0 or 1
    a_pDataOut->stabilization_strength = customSetting.stabilization_strength;  // 0.5~0.95

    a_pDataOut->advtuning_data.new_tru_th        = customSetting.new_tru_th;         // 0~100
    a_pDataOut->advtuning_data.vot_th            = customSetting.vot_th;             // 1~16
    a_pDataOut->advtuning_data.votb_enlarge_size = customSetting.votb_enlarge_size;  // 0~1280
    a_pDataOut->advtuning_data.min_s_th          = customSetting.min_s_th;           // 10~100
    a_pDataOut->advtuning_data.vec_th            = customSetting.vec_th;             // 0~11   should be even
    a_pDataOut->advtuning_data.spr_offset        = customSetting.spr_offset;         //0 ~ MarginX/2
    a_pDataOut->advtuning_data.spr_gain1         = customSetting.spr_gain1;          // 0~127
    a_pDataOut->advtuning_data.spr_gain2         = customSetting.spr_gain2;          // 0~127

    a_pDataOut->advtuning_data.gmv_pan_array[0] = customSetting.gmv_pan_array[0];   //0~5
    a_pDataOut->advtuning_data.gmv_pan_array[1] = customSetting.gmv_pan_array[1];   //0~5
    a_pDataOut->advtuning_data.gmv_pan_array[2] = customSetting.gmv_pan_array[2];   //0~5
    a_pDataOut->advtuning_data.gmv_pan_array[3] = customSetting.gmv_pan_array[3];   //0~5

    a_pDataOut->advtuning_data.gmv_sm_array[0] = customSetting.gmv_sm_array[0];    //0~5
    a_pDataOut->advtuning_data.gmv_sm_array[1] = customSetting.gmv_sm_array[1];    //0~5
    a_pDataOut->advtuning_data.gmv_sm_array[2] = customSetting.gmv_sm_array[2];    //0~5
    a_pDataOut->advtuning_data.gmv_sm_array[3] = customSetting.gmv_sm_array[3];    //0~5

    a_pDataOut->advtuning_data.cmv_pan_array[0] = customSetting.cmv_pan_array[0];   //0~5
    a_pDataOut->advtuning_data.cmv_pan_array[1] = customSetting.cmv_pan_array[1];   //0~5
    a_pDataOut->advtuning_data.cmv_pan_array[2] = customSetting.cmv_pan_array[2];   //0~5
    a_pDataOut->advtuning_data.cmv_pan_array[3] = customSetting.cmv_pan_array[3];   //0~5

    a_pDataOut->advtuning_data.cmv_sm_array[0] = customSetting.cmv_sm_array[0];    //0~5
    a_pDataOut->advtuning_data.cmv_sm_array[1] = customSetting.cmv_sm_array[1];    //0~5
    a_pDataOut->advtuning_data.cmv_sm_array[2] = customSetting.cmv_sm_array[2];    //0~5
    a_pDataOut->advtuning_data.cmv_sm_array[3] = customSetting.cmv_sm_array[3];    //0~5

    a_pDataOut->advtuning_data.vot_his_method  = (EIS_VOTE_METHOD_ENUM)customSetting.vot_his_method; //0 or 1
    a_pDataOut->advtuning_data.smooth_his_step = customSetting.smooth_his_step; // 2~6

    a_pDataOut->advtuning_data.eis_debug = customSetting.eis_debug;

    if(mDebugDump >= 1)
    {
        LMV_LOG("-");
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID LMVHalImp::DumpStatistic(const EIS_STATISTIC_STRUCT &aEisStat)
{
    LMV_LOG("+");

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        LMV_INF("MB%d%d,(LMV_X,LMV_Y)=(%d,%d)",(i/4),(i%4),aEisStat.i4LMV_X[i],aEisStat.i4LMV_Y[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        LMV_INF("MB%d%d,(LMV_X2,LMV_Y2)=(%d,%d)",(i/4),(i%4),aEisStat.i4LMV_X2[i],aEisStat.i4LMV_Y2[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        LMV_INF("MB%d%d,MinSAD(%u)",(i/4),(i%4),aEisStat.SAD[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        LMV_INF("MB%d%d,(NewTrust_X,NewTrust_Y)=(%u,%u)",(i/4),(i%4),aEisStat.NewTrust_X[i],aEisStat.NewTrust_Y[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        LMV_INF("MB%d%d,MinSAD2(%u)",(i/4),(i%4),aEisStat.SAD2[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        LMV_INF("MB%d%d,AvgSAD(%u)",(i/4),(i%4),aEisStat.AVG_SAD[i]);
    }

    LMV_LOG("-");
}

#endif

