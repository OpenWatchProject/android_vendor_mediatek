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
* @file lmv_hal.cpp
*
* LMV Hal Source File
*
*/

#include <cstdio>
#include <queue>
#include <string>
#include <cutils/atomic.h>
#include <utils/threads.h>
#include <utils/SystemClock.h>
#include <utils/Trace.h>
#include <utils/String8.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>

#include "lmv_drv.h"

#include <android/sensor.h>
#include <mtkcam/utils/sys/SensorProvider.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam/utils/imgbuf/IDummyImageBufferHeap.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam3/feature/eis/eis_ext.h>
#include <camera_custom_nvram.h>
#include <camera_custom_eis.h>

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::Utils;

#include "lmv_hal_imp.h"



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
#include <mtkcam/utils/std/ULog.h>

#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif


CAM_ULOG_DECLARE_MODULE_ID(MOD_LMV_HAL);

#undef LMV_LOG_LV1
#undef LMV_LOG_LV2
#undef LMV_LOG_LV3
#undef LMV_LOG
#undef LMV_S_LOG
#undef LMV_P_LOG
#undef LMV_DO
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_S_LOGD
#undef MY_S_LOGI
#undef MY_S_LOGW
#undef MY_S_LOGE
#undef MY_LOGD0
#undef MY_LOGD1
#undef MY_LOGD2
#undef MY_LOGD3
#undef MY_TRACE_API_LIFE
#undef MY_TRACE_FUNC_LIFE
#undef MY_TRACE_TAG_LIFE
#undef MY_LOGA


#define LMV_LOG_LV1                  ( 1 <= mDebugDump )
#define LMV_LOG_LV2                  ( 2 <= mDebugDump )
#define LMV_LOG_LV3                  ( 3 <= mDebugDump )

#define LMV_LOG(lv, fmt, arg...)      CAM_ULOGM##lv("[%s]" fmt, __FUNCTION__, ##arg)
#define LMV_S_LOG(lv, fmt, arg...)    CAM_ULOGM##lv("[%s][Cam::%d]" fmt, __FUNCTION__, mSensorIdx, ##arg)

#define LMV_DO(cmd) do { cmd; } while(0)

#define MY_LOGD(fmt, arg...)          LMV_DO( LMV_LOG(D, fmt, ##arg))
#define MY_LOGI(fmt, arg...)          LMV_DO( LMV_LOG(I, fmt, ##arg))
#define MY_LOGW(fmt, arg...)          LMV_DO( LMV_LOG(W, fmt, ##arg))
#define MY_LOGE(fmt, arg...)          LMV_DO( LMV_LOG(E, fmt, ##arg))

#define MY_S_LOGD(fmt, arg...)        LMV_DO( LMV_S_LOG(D, fmt, ##arg))
#define MY_S_LOGI(fmt, arg...)        LMV_DO( LMV_S_LOG(I, fmt, ##arg))
#define MY_S_LOGW(fmt, arg...)        LMV_DO( LMV_S_LOG(W, fmt, ##arg))
#define MY_S_LOGE(fmt, arg...)        LMV_DO( LMV_S_LOG(E, fmt, ##arg))
#define MY_S_LOGD_IF(c, fmt, arg...)  LMV_DO(if(c) LMV_S_LOG(D, fmt, ##arg))

#define MY_LOGD0(fmt, arg...)         MY_S_LOGD(fmt, ##arg)
#define MY_LOGD1(fmt, arg...)         MY_S_LOGD_IF(LMV_LOG_LV1, fmt, ##arg)
#define MY_LOGD2(fmt, arg...)         MY_S_LOGD_IF(LMV_LOG_LV2, fmt, ##arg)
#define MY_LOGD3(fmt, arg...)         MY_S_LOGD_IF(LMV_LOG_LV3, fmt, ##arg)

#define MY_TRACE_API_LIFE()           CAM_ULOGM_APILIFE()
#define MY_TRACE_FUNC_LIFE()          CAM_ULOGM_FUNCLIFE()
#define MY_TRACE_TAG_LIFE(name)       CAM_ULOGM_TAGLIFE(name)


#if (MTKCAM_HAVE_AEE_FEATURE == 1)
static const unsigned int LMV_AEE_DB_FLAGS = DB_OPT_NE_JBT_TRACES | DB_OPT_PROCESS_COREDUMP | DB_OPT_PROC_MEM | DB_OPT_PID_SMAPS |
                                             DB_OPT_LOW_MEMORY_KILLER | DB_OPT_DUMPSYS_PROCSTATS | DB_OPT_FTRACE;
#define MY_LOGA(fmt, arg...)                                                            \
        do {                                                                            \
            android::String8 const str = android::String8::format(fmt, ##arg);          \
            MY_LOGE("ASSERT(%s)", str.c_str());                                         \
            aee_system_exception(LOG_TAG, NULL, LMV_AEE_DB_FLAGS, str.c_str());         \
            raise(SIGKILL);                                                             \
        } while(0)
#else
#define MY_LOGA(fmt, arg...)                                                            \
        do {                                                                            \
            android::String8 const str = android::String8::format(fmt, ##arg);          \
            MY_LOGE("ASSERT(%s)", str.c_str());                                         \
        } while(0)
#endif

#endif

#define LMV_HAL_NAME            "LMVHal"
#define LMV_HAL_DUMP            "vendor.debug.lmv.dump"
#define LMV_HAL_GYRO_INTERVAL   "vendor.debug.lmv.setinterval"

#define LMV_DUMP_PATH           "/data/vendor/dump"

#define intPartShift            (8)
#define floatPartShift          (31 - intPartShift)
#define DEBUG_DUMP_FRAMW_NUM    (10)

template <> LMVHalObj<0>* LMVHalObj<0>::spInstance = 0;
template <> LMVHalObj<1>* LMVHalObj<1>::spInstance = 0;
template <> LMVHalObj<2>* LMVHalObj<2>::spInstance = 0;
template <> LMVHalObj<3>* LMVHalObj<3>::spInstance = 0;
template <> LMVHalObj<4>* LMVHalObj<4>::spInstance = 0;
template <> LMVHalObj<5>* LMVHalObj<5>::spInstance = 0;
template <> LMVHalObj<6>* LMVHalObj<6>::spInstance = 0;
template <> LMVHalObj<7>* LMVHalObj<7>::spInstance = 0;
template <> LMVHalObj<8>* LMVHalObj<8>::spInstance = 0;
template <> LMVHalObj<9>* LMVHalObj<9>::spInstance = 0;

template <> Mutex LMVHalObj<0>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex LMVHalObj<1>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex LMVHalObj<2>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex LMVHalObj<3>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex LMVHalObj<4>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex LMVHalObj<5>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex LMVHalObj<6>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex LMVHalObj<7>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex LMVHalObj<8>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex LMVHalObj<9>::s_instMutex(::Mutex::PRIVATE);


const MUINT32 GyroInterval_ms = 20;

MINT32 LMVHalImp::mDebugDump = 0;


#define LMVO_BUFFER_NUM (60)

LMVHal *LMVHal::CreateInstance(char const *userName, const MUINT32 &aSensorIdx)
{
    MY_LOGI("user(%s),sensorId(%d)", userName, aSensorIdx);
    return LMVHalImp::GetInstance(aSensorIdx);
}

LMVHal *LMVHalImp::GetInstance(const MUINT32 &aSensorIdx)
{
    MY_LOGD("sensorIdx(%u)", aSensorIdx);

    switch( aSensorIdx )
    {
        case 0 : return LMVHalObj<0>::GetInstance();
        case 1 : return LMVHalObj<1>::GetInstance();
        case 2 : return LMVHalObj<2>::GetInstance();
        case 3 : return LMVHalObj<3>::GetInstance();
        case 4 : return LMVHalObj<4>::GetInstance();
        case 5 : return LMVHalObj<5>::GetInstance();
        case 6 : return LMVHalObj<6>::GetInstance();
        case 7 : return LMVHalObj<7>::GetInstance();
        case 8 : return LMVHalObj<8>::GetInstance();
        case 9 : return LMVHalObj<9>::GetInstance();
        default :
            MY_LOGW("Current limit is 10 sensors, use 0");
            return LMVHalObj<0>::GetInstance();
    }
}

MVOID LMVHalImp::DestroyInstance(char const *userName)
{
    MY_LOGD0("user(%s)", userName);
}

LMVHalImp::LMVHalImp(const MUINT32 &aSensorIdx)
    : LMVHal()
    , mSensorIdx(aSensorIdx)
{
    mName = LMV_HAL_NAME"_" + std::to_string(aSensorIdx);

    memset(&mSensorStaticInfo, 0, sizeof(mSensorStaticInfo));
    memset(&mSensorDynamicInfo, 0, sizeof(mSensorDynamicInfo));
    memset(&mLmvAlgoProcData, 0, sizeof(mLmvAlgoProcData));
    memset(&mLmvLastData2EisPlus, 0, sizeof(mLmvLastData2EisPlus));

    while( !mLMVOBufferList.empty() )
    {
        mLMVOBufferList.pop();
    }
}

MINT32 LMVHalImp::Init(const MUINT32 eisFactor)
{
    MY_TRACE_API_LIFE();

    Mutex::Autolock lock(mLock);

    if( mUsers > 0 )
    {
        android_atomic_inc(&mUsers);
        MY_LOGD0("has %d users", mUsers);
        return LMV_RETURN_NO_ERROR;
    }

    mDebugDump = ::property_get_int32(LMV_HAL_DUMP, mDebugDump);
    mEisPlusCropRatio = eisFactor > 100 ? eisFactor : EISCustom::getEIS12Factor();

    m_pHalSensorList = MAKE_HalSensorList();
    if( m_pHalSensorList == NULL )
    {
        MY_LOGE("IHalSensorList::get fail");
        goto create_fail_exit;
    }
    if( GetSensorInfo() != LMV_RETURN_NO_ERROR )
    {
        MY_LOGE("GetSensorInfo fail");
        goto create_fail_exit;
    }

    m_pLMVDrv = LMVDrv::CreateInstance(mSensorIdx);
    if( m_pLMVDrv == NULL )
    {
        MY_LOGE("LMVDrv::createInstance fail");
        goto create_fail_exit;
    }
    if( m_pLMVDrv->Init() != LMV_RETURN_NO_ERROR )
    {
        MY_LOGE("LMVDrv::Init fail");
        goto create_fail_exit;
    }

    MY_LOGD0("LMVHal(%p)CropRatio(%u)", this, mEisPlusCropRatio);

    CreateMultiMemBuf(LMVO_MEMORY_SIZE, (LMVO_BUFFER_NUM+1), m_pLMVOMainBuffer, m_pLMVOSliceBuffer);
    if( !m_pLMVOSliceBuffer[0]->getBufVA(0) )
    {
        MY_LOGE("LMVO slice buf create ImageBuffer fail!");
        MY_LOGD0("-");
        return LMV_RETURN_MEMORY_ERROR;
    }
    {
        Mutex::Autolock lock(mLMVOBufferListLock);
        for( int index = 0; index < LMVO_BUFFER_NUM; index++ )
        {
            mLMVOBufferList.push(m_pLMVOSliceBuffer[index]);
        }
    }

    android_atomic_inc(&mUsers);

    if( UNLIKELY(mDebugDump >= 3) )
    {
        std::string filename = LMV_DUMP_PATH"/" + std::string(mName);
        mDumpFile = fopen(filename.c_str(), "w+");

        if( mDumpFile == NULL )
        {
            MY_LOGW("Open file(%s) fail! (errno=%d)", filename.c_str(), errno);
        }
    }

    MY_LOGD0("-");
    return LMV_RETURN_NO_ERROR;

create_fail_exit:

    if( m_pLMVDrv != NULL )
    {
        m_pLMVDrv->Uninit();
        m_pLMVDrv->DestroyInstance();
        m_pLMVDrv = NULL;
    }

    if( m_pHalSensorList != NULL )
    {
        m_pHalSensorList = NULL;
    }
    MY_LOGD0("-");
    return LMV_RETURN_NULL_OBJ;
}

MINT32 LMVHalImp::Uninit()
{
    MY_TRACE_API_LIFE();

    Mutex::Autolock lock(mLock);

    if( mUsers <= 0 )
    {
        MY_LOGD0("has 0 user");
        return LMV_RETURN_NO_ERROR;
    }

    android_atomic_dec(&mUsers);

    if( mUsers == 0 )
    {
        MY_LOGD0("TG(%d)", mSensorDynamicInfo.TgInfo);

        if( m_pLMVDrv != NULL )
        {
            MY_LOGD0("m_pLMVDrv uninit");

            m_pLMVDrv->Uninit();
            m_pLMVDrv->DestroyInstance();
            m_pLMVDrv = NULL;
        }

        if( UNLIKELY(mDebugDump >= 2) )
        {
            if( mSensorDynamicInfo.TgInfo != CAM_TG_NONE )
            {
                MINT32 err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SAVE_LOG, NULL, NULL);
                if( err != S_EIS_OK )
                {
                    MY_LOGE("EisFeatureCtrl(EIS_FEATURE_SAVE_LOG) fail(0x%x)", err);
                }
            }
        }

        if( m_pEisAlg != NULL )
        {
            MY_LOGD0("m_pEisAlg uninit");
            m_pEisAlg->EisReset();
            m_pEisAlg->destroyInstance(m_pEisAlg);
            m_pEisAlg = NULL;
        }
        if( mpSensorProvider != NULL )
        {
            mpSensorProvider->disableSensor(SENSOR_TYPE_GYRO);
            mpSensorProvider = NULL;
        }
        if( m_pHalSensorList != NULL )
        {
            m_pHalSensorList = NULL;
        }

        if( UNLIKELY(mDebugDump >= 2) )
        {
            m_pLmvDbgBuf->unlockBuf("LMVDbgBuf");
            DestroyMemBuf(m_pLmvDbgBuf);
        }
        DestroyMultiMemBuf((LMVO_BUFFER_NUM+1), m_pLMVOMainBuffer, m_pLMVOSliceBuffer);

        mLmvInput_W = 0;
        mLmvInput_H = 0;
        mP1Target_W = 0;
        mP1Target_H = 0;
        mFrameCnt = 0;
        mEisPass1Enabled = 0;
        mIsLmvConfig = 0;
        mCmvX_Int = 0;
        mDoLmvCount = 0;
        mCmvX_Flt = 0;
        mCmvY_Int = 0;
        mMVtoCenterX = 0;
        mMVtoCenterY = 0;
        mCmvY_Flt = 0;
        mCmvTmpX_Int = 0;
        mCmvTmpX_Flt = 0;
        mCmvTmpY_Int = 0;
        mCmvTmpY_Flt = 0;
        mGMV_X = 0;
        mGMV_Y = 0;
        mMAX_GMV = LMV_MAX_GMV_DEFAULT;
        mVideoW = 0;
        mVideoH = 0;
        mMemAlignment = 0;
        mBufIndex = 0;
        mDebugDump = 0;
        mPrev_X = 0;
        mPrev_Y = 0;
        mCur_X = 0;
        mCur_Y = 0;

        {
            Mutex::Autolock lock(mLMVOBufferListLock);
            while( !mLMVOBufferList.empty() )
            {
                mLMVOBufferList.pop();
            }
        }

        if( mDumpFile != NULL )
        {
            fflush(mDumpFile);
            fclose(mDumpFile);
            MY_LOGD0("Close file successfully");
            mDumpFile = NULL;
        }

    }
    else
    {
        MY_LOGD0("has %d users", mUsers);
    }
    return LMV_RETURN_NO_ERROR;
}

MINT32 LMVHalImp::CreateMultiMemBuf(MUINT32 memSize, MUINT32 num, android::sp<IImageBuffer>& spMainImageBuf, android::sp<IImageBuffer> spImageBuf[MAX_LMV_MEMORY_SIZE])
{
    MY_TRACE_FUNC_LIFE();

    MINT32 err = LMV_RETURN_NO_ERROR;
    MUINT32 totalSize = memSize*num;

    if( num >= MAX_LMV_MEMORY_SIZE )
    {
        MY_LOGE("num of image buffer is larger than MAX_LMV_MEMORY_SIZE(%d)", MAX_LMV_MEMORY_SIZE);
        return LMV_RETURN_MEMORY_ERROR;
    }

    IImageBufferAllocator::ImgParam imgParam(totalSize, 0);

    sp<IIonImageBufferHeap> pHeap = IIonImageBufferHeap::create(mName.c_str(), imgParam);
    if( pHeap == NULL )
    {
        MY_LOGE("image buffer heap create fail");
        return LMV_RETURN_MEMORY_ERROR;
    }

    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN |
                        GRALLOC_USAGE_SW_WRITE_OFTEN | // ISP3 is software-write
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    spMainImageBuf = pHeap->createImageBuffer();
    if( spMainImageBuf == NULL )
    {
        MY_LOGE("mainImage buffer create fail");
        return LMV_RETURN_MEMORY_ERROR;
    }
    if( !(spMainImageBuf->lockBuf(mName.c_str(), usage)) )
    {
        MY_LOGE("image buffer lock fail");
        return LMV_RETURN_MEMORY_ERROR;
    }
    MUINTPTR const iVAddr = pHeap->getBufVA(0);
    MUINTPTR const iPAddr = pHeap->getBufPA(0);
    MINT32 const iHeapId = pHeap->getHeapID();

    MY_LOGD0("IIonImageBufferHeap iVAddr:%p, iPAddr:%p, iHeapId:%d. spMainImageBuf iVAddr:%p, iPAddr:%p",
            (void*)iVAddr, (void*)iPAddr, iHeapId,
            (void*)spMainImageBuf->getBufVA(0), (void*)spMainImageBuf->getBufPA(0));

    for( MUINT32 index = 0; index < num; index++)
    {
        MUINTPTR const cVAddr = iVAddr + ((index)*(memSize));
        MUINTPTR const virtAddr[] = {cVAddr, 0, 0};
        MUINTPTR const cPAddr = iPAddr + ((index)*(memSize));
        MUINTPTR const phyAddr[] = {cPAddr, 0, 0};
        IImageBufferAllocator::ImgParam imgParam_t = IImageBufferAllocator::ImgParam(memSize, 0);
        PortBufInfo_dummy portBufInfo = PortBufInfo_dummy(iHeapId, virtAddr, phyAddr, 1);
        sp<IImageBufferHeap> imgBufHeap = IDummyImageBufferHeap::create(mName.c_str(), imgParam_t, portBufInfo, false);
        if( imgBufHeap == NULL )
        {
            MY_LOGE("acquire LMV_HAL - image buffer heap create fail");
            return LMV_RETURN_MEMORY_ERROR;
        }
        sp<IImageBuffer> imgBuf = imgBufHeap->createImageBuffer();
        if( imgBuf == NULL )
        {
            MY_LOGE("acquire LMV_HAL - image buffer create fail");
            return LMV_RETURN_MEMORY_ERROR;
        }
        if( !(imgBuf->lockBuf(mName.c_str(), usage)) )
        {
            MY_LOGE("acquire LMV_HAL - image buffer lock fail");
            return LMV_RETURN_MEMORY_ERROR;
        }

        spImageBuf[index] = imgBuf;
    }
    MY_LOGD0("-");
    return err;
}

MINT32 LMVHalImp::CreateMemBuf(MUINT32 memSize, android::sp<IImageBuffer>& spImageBuf)
{
    MY_TRACE_FUNC_LIFE();

    MINT32 err = LMV_RETURN_NO_ERROR;
    IImageBufferAllocator* pImageBufferAlloc = IImageBufferAllocator::getInstance();

    MY_LOGD0("Size(%u)", memSize);
    IImageBufferAllocator::ImgParam bufParam((size_t)memSize, 0);
    spImageBuf = pImageBufferAlloc->alloc(mName.c_str(), bufParam);
    MY_LOGD0("-");
    return err;
}

MINT32 LMVHalImp::DestroyMultiMemBuf(MUINT32 num, android::sp<IImageBuffer>& spMainImageBuf, android::sp<IImageBuffer> spImageBuf[MAX_LMV_MEMORY_SIZE])
{
    MY_TRACE_FUNC_LIFE();

    MINT32 err = LMV_RETURN_NO_ERROR;
    for( MUINT32 index = 0; index < num; index++)
    {
        spImageBuf[index]->unlockBuf(mName.c_str());
        spImageBuf[index] = NULL;
    }

    spMainImageBuf->unlockBuf(mName.c_str());
    spMainImageBuf = NULL;

    MY_LOGD0("-");
    return err;
}

MINT32 LMVHalImp::DestroyMemBuf(android::sp<IImageBuffer>& spImageBuf)
{
    MY_TRACE_FUNC_LIFE();

    MINT32 err = LMV_RETURN_NO_ERROR;
    IImageBufferAllocator* pImageBufferAlloc = IImageBufferAllocator::getInstance();

    if( spImageBuf != NULL )
    {
        pImageBufferAlloc->free(spImageBuf.get());
        spImageBuf = NULL;
    }

    MY_LOGD0("-");
    return err;
}

MINT32 LMVHalImp::GetSensorInfo()
{
    MY_TRACE_FUNC_LIFE();

    mSensorDev = m_pHalSensorList->querySensorDevIdx(mSensorIdx);
    m_pHalSensorList->querySensorStaticInfo(mSensorDev, &mSensorStaticInfo);
    m_pHalSensor = m_pHalSensorList->createSensor(mName.c_str(), 1, &mSensorIdx);
    if( m_pHalSensor == NULL )
    {
        MY_LOGE("m_pHalSensorList->createSensor fail");
        return LMV_RETURN_API_FAIL;
    }
    if( m_pHalSensor->querySensorDynamicInfo(mSensorDev, &mSensorDynamicInfo) == MFALSE )
    {
        MY_LOGE("querySensorDynamicInfo fail");
        return LMV_RETURN_API_FAIL;
    }

    m_pHalSensor->destroyInstance(mName.c_str());
    m_pHalSensor = NULL;

    return LMV_RETURN_NO_ERROR;
}

MVOID LMVHalImp::EnableGyroSensor()
{
    if( mpSensorProvider == NULL )
    {
        {
            MY_TRACE_TAG_LIFE("SensorProvider::createInstance");
            mpSensorProvider = SensorProvider::createInstance(mName.c_str());
        }
        MUINT32 interval = ::property_get_int32(LMV_HAL_GYRO_INTERVAL, GyroInterval_ms);

        MBOOL isEnabledGyro = MFALSE;
        {
            MY_TRACE_TAG_LIFE("SensorProvider::enableSensor");
            isEnabledGyro = mpSensorProvider->enableSensor(SENSOR_TYPE_GYRO, interval);
        }

        if( isEnabledGyro )
        {
            MY_LOGD0("Enable SensorProvider success");
        }
        else
        {
            MY_LOGE("Enable SensorProvider fail");
        }
    }

}

MINT32 LMVHalImp::ConfigLMV(const LMV_HAL_CONFIG_DATA &aLmvConfig)
{
    MY_TRACE_API_LIFE();

    if( mLmvSupport == MFALSE )
    {
        MY_LOGI("mSensorIdx(%u) not support LMV", mSensorIdx);
        return LMV_RETURN_NO_ERROR;
    }

    MINT32 err = LMV_RETURN_NO_ERROR;

    static EIS_SET_ENV_INFO_STRUCT eisAlgoInitData;

    LMV_SENSOR_ENUM sensorType;
    switch( aLmvConfig.sensorType )
    {
        case NSCam::NSSensorType::eRAW:
            sensorType = LMV_RAW_SENSOR;
            break;
        case NSCam::NSSensorType::eYUV:
            sensorType = LMV_YUV_SENSOR;
            break;
        default:
            MY_LOGE("not support sensor type(%u), use RAW setting", aLmvConfig.sensorType);
            sensorType = LMV_RAW_SENSOR;
            break;
    }

    MY_LOGD1("mIsLmvConfig(%u)", mIsLmvConfig);

    if( mIsLmvConfig == 0 )
    {
        GetEisCustomize(&eisAlgoInitData.eis_tuning_data);
        eisAlgoInitData.Eis_Input_Path = EIS_PATH_RAW_DOMAIN;   // RAW domain

        if( mSensorDynamicInfo.TgInfo == CAM_TG_NONE )
        {
            // Reget sensor information
            if( GetSensorInfo() != LMV_RETURN_NO_ERROR )
            {
                MY_LOGE("GetSensorInfo fail");
            }
            MY_LOGD0("TG(%d)",mSensorDynamicInfo.TgInfo);
        }

        if( m_pEisAlg == NULL )
        {
            m_pEisAlg = MTKEis::createInstance();
            if( m_pEisAlg == NULL )
            {
                MY_LOGE("MTKEis::createInstance fail");
                return LMV_RETURN_UNKNOWN_ERROR;
            }
        }
        if( m_pEisAlg->EisInit(&eisAlgoInitData) != S_EIS_OK )
        {
            MY_LOGE("EisInit fail(0x%x)",err);
            return LMV_RETURN_API_FAIL;
        }

        if( UNLIKELY(mDebugDump >= 2) )
        {
            MUINT32 lmvMemSize = 0;
            EIS_SET_LOG_BUFFER_STRUCT eisAlgoLogInfo;

            if( m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_DEBUG_INFO, NULL, &lmvMemSize) != S_EIS_OK ||
                lmvMemSize == 0 )
            {
                MY_LOGE("EisFeatureCtrl(EIS_FEATURE_GET_DEBUG_INFO) fail(0x%x)",err);
                eisAlgoLogInfo.Eis_Log_Buf_Addr = NULL;
                eisAlgoLogInfo.Eis_Log_Buf_Size = 0;
            }
            else
            {
                CreateMemBuf(lmvMemSize, m_pLmvDbgBuf);
                m_pLmvDbgBuf->lockBuf("LMVDbgBuf", eBUFFER_USAGE_SW_MASK);
                if( !m_pLmvDbgBuf->getBufVA(0) )
                {
                    MY_LOGE("mLmvDbgBuf create ImageBuffer fail");
                    return LMV_RETURN_MEMORY_ERROR;
                }

                MY_LOGD0("mLmvDbgBuf : size(%u),virAdd(0x%p)", lmvMemSize, (void*)m_pLmvDbgBuf->getBufVA(0));
                eisAlgoLogInfo.Eis_Log_Buf_Addr = (MVOID *)m_pLmvDbgBuf->getBufVA(0);
                eisAlgoLogInfo.Eis_Log_Buf_Size = lmvMemSize;
            }
            if( m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SET_DEBUG_INFO, &eisAlgoLogInfo, NULL) != S_EIS_OK )
            {
                MY_LOGE("EisFeatureCtrl(EIS_FEATURE_SET_DEBUG_INFO) fail(0x%x)",err);
            }
        }

        mTsForAlgoDebug = 0;

        if( UNLIKELY(mDebugDump >= 1) )
        {
            String8 str;
            str += String8::format("EIS tuning_data:sensitivity(%d),filter_small_motion(%u),adv_shake_ext(%u),"
                                   "stabilization_strength(%f),new_tru_th(%u),vot_th(%u),votb_enlarge_size(%u),"
                                   "min_s_th(%u),vec_th(%u),spr_offset(%u),spr_gain1(%u),spr_gain2(%u),"
                                   "vot_his_method(%d),smooth_his_step(%u),eis_debug(%u)",
                                    eisAlgoInitData.eis_tuning_data.sensitivity,
                                    eisAlgoInitData.eis_tuning_data.filter_small_motion,
                                    eisAlgoInitData.eis_tuning_data.adv_shake_ext,
                                    eisAlgoInitData.eis_tuning_data.stabilization_strength,
                                    eisAlgoInitData.eis_tuning_data.advtuning_data.new_tru_th,
                                    eisAlgoInitData.eis_tuning_data.advtuning_data.vot_th,
                                    eisAlgoInitData.eis_tuning_data.advtuning_data.votb_enlarge_size,
                                    eisAlgoInitData.eis_tuning_data.advtuning_data.min_s_th,
                                    eisAlgoInitData.eis_tuning_data.advtuning_data.vec_th,
                                    eisAlgoInitData.eis_tuning_data.advtuning_data.spr_offset,
                                    eisAlgoInitData.eis_tuning_data.advtuning_data.spr_gain1,
                                    eisAlgoInitData.eis_tuning_data.advtuning_data.spr_gain2,
                                    eisAlgoInitData.eis_tuning_data.advtuning_data.vot_his_method,
                                    eisAlgoInitData.eis_tuning_data.advtuning_data.smooth_his_step,
                                    eisAlgoInitData.eis_tuning_data.advtuning_data.eis_debug );
            String8 str_gmv_pan(",gmv_pan");
            String8 str_gmv_sm(",gmv_sm");
            String8 str_cmv_pan(",cmv_pan");
            String8 str_cmv_sm(",cmv_sm");
            const int MV_ARRAY_NUM = 4;
            for( int i = 0; i < MV_ARRAY_NUM; ++i)
            {
                str_gmv_pan += String8::format("(%u)", eisAlgoInitData.eis_tuning_data.advtuning_data.gmv_pan_array[i]);
                str_gmv_sm  += String8::format("(%u)", eisAlgoInitData.eis_tuning_data.advtuning_data.gmv_sm_array[i]);
                str_cmv_pan += String8::format("(%u)", eisAlgoInitData.eis_tuning_data.advtuning_data.cmv_pan_array[i]);
                str_cmv_sm  += String8::format("(%u)", eisAlgoInitData.eis_tuning_data.advtuning_data.cmv_sm_array[i]);
            }
            str += str_gmv_pan + str_gmv_sm + str_cmv_pan + str_cmv_sm;
            MY_LOGD0("%s", str.string());
        }

        if( m_pLMVDrv->ConfigLMVReg(mSensorDynamicInfo.TgInfo) != LMV_RETURN_NO_ERROR )
        {
            MY_LOGE("ConfigLMVReg fail(0x%x)", err);
            return LMV_RETURN_API_FAIL;
        }

        mIsLmvConfig = 1;
        {
            Mutex::Autolock lock(mLock);
            mEisPass1Enabled = 1;
        }
    }
    return LMV_RETURN_NO_ERROR;
}

MINT32 LMVHalImp::PrepareLMV(const LMV_HAL_ENQUEUE_DATA &aLmvEnqueData __unused)
{
    // TODO:
    return LMV_RETURN_NO_ERROR;
}

MINT32 LMVHalImp::DoLMVCalc(QBufInfo const &pBufInfo, const MUINT32 rrzIdx, const MUINT32 lmvIdx)
{
    MY_TRACE_API_LIFE();

    MINT32 err = LMV_RETURN_NO_ERROR;
    const MUINT64 aTimeStamp = pBufInfo.mvOut[0].mMetaData.mTimeStamp; //Maybe framedone

    if( mLmvSupport == MFALSE )
    {
        MY_LOGD0("not support LMV");
        return LMV_RETURN_EISO_MISS;
    }

    if( UNLIKELY(mTsForAlgoDebug == 0) )
    {
        mTsForAlgoDebug = aTimeStamp;
    }

    MY_LOGD1("mEisPass1Enabled(%u)", mEisPass1Enabled);

    if( aTimeStamp <= 0 )
    {
        MY_LOGD0("DoP1Eis aTimeStamp is not reasonable(%" PRIi64 ")",aTimeStamp);
    }
    else
    {
        EIS_RESULT_INFO_STRUCT eisCMVResult;

        if( UNLIKELY( (rrzIdx == NSImageio::NSIspio::EPortIndex_UNKNOW) || (lmvIdx == NSImageio::NSIspio::EPortIndex_UNKNOW) ) )
        {
            MY_LOGA("Error: Unknwon buffer index(rrz:%d/lmv:%d)"
                    "\nCRDISPATCH_KEY:LMV unknown buffer index",
                    rrzIdx, lmvIdx);
        }
        //crop region
        mP1ResizeIn_W = mLmvInput_W = pBufInfo.mvOut.at(rrzIdx).mMetaData.mDstSize.w;
        mP1ResizeIn_H = mLmvInput_H = pBufInfo.mvOut.at(rrzIdx).mMetaData.mDstSize.h;

        MINTPTR lmvoBufferVA = pBufInfo.mvOut.at(lmvIdx).mBuffer->getBufVA(0);

        {
            Mutex::Autolock lock(mP1Lock);

            if( m_pLMVDrv->Get2PixelMode() == 1 )
            {
                mLmvInput_W >>= 1;
            }
            else if( m_pLMVDrv->Get2PixelMode() == 2 )
            {
                mLmvInput_W >>= 2;
            }

            mLmvInput_W -= 4;   //ryan wang request to -4
            mLmvInput_H -= 4;   //ryan wang request to -4

            mP1Target_W = (mLmvInput_W / (mEisPlusCropRatio / 100.0));
            mP1Target_H = (mLmvInput_H / (mEisPlusCropRatio / 100.0));

            mP1ResizeOut_W = (mP1ResizeIn_W / (mEisPlusCropRatio / 100.0));
            mP1ResizeOut_H = (mP1ResizeIn_H / (mEisPlusCropRatio / 100.0));

            mLmvAlgoProcData.eis_image_size_config.InputWidth   = mLmvInput_W;
            mLmvAlgoProcData.eis_image_size_config.InputHeight  = mLmvInput_H;
            mLmvAlgoProcData.eis_image_size_config.TargetWidth  = mP1Target_W;
            mLmvAlgoProcData.eis_image_size_config.TargetHeight = mP1Target_H;
        }

        //> get LMV HW statistic
        if( m_pLMVDrv->GetLmvHwStatistic(lmvoBufferVA, &mLmvAlgoProcData.eis_state) == LMV_RETURN_EISO_MISS )
        {
            MY_LOGW("EISO data miss");
            return LMV_RETURN_NO_ERROR;
        }

        mLmvAlgoProcData.DivH      = m_pLMVDrv->GetLMVDivH();
        mLmvAlgoProcData.DivV      = m_pLMVDrv->GetLMVDivV();
        mLmvAlgoProcData.EisWinNum = m_pLMVDrv->GetLMVMbNum();

        // get gyro/acceleration data
        SensorData gyroData;
        SensorData accData;
        MBOOL gyroValid = false;
        MBOOL accValid = false;
        {
            MY_TRACE_TAG_LIFE("SensorProvider::getLatestSensorData");
            gyroValid = mpSensorProvider->getLatestSensorData(SENSOR_TYPE_GYRO, gyroData);
            //accValid = mpSensorProvider->getLatestSensorData(SENSOR_TYPE_ACCELERATION, accData);
        }

        {
            Mutex::Autolock lock(mP1Lock);

            mLmvAlgoProcData.sensor_info.GyroValid = gyroValid;
            mLmvAlgoProcData.sensor_info.Gvalid = accValid;

            if( gyroValid )
            {
                mLmvAlgoProcData.sensor_info.GyroInfo[0] = gyroData.gyro[0];
                mLmvAlgoProcData.sensor_info.GyroInfo[1] = gyroData.gyro[1];
                mLmvAlgoProcData.sensor_info.GyroInfo[2] = gyroData.gyro[2];
            }
        }

        MY_LOGD1("EN:(Acc,Gyro)=(%d,%d)/Acc(%f,%f,%f)/Gyro(%f,%f,%f)", accValid, gyroValid,
                mLmvAlgoProcData.sensor_info.AcceInfo[0], mLmvAlgoProcData.sensor_info.AcceInfo[1], mLmvAlgoProcData.sensor_info.AcceInfo[2],
                mLmvAlgoProcData.sensor_info.GyroInfo[0],mLmvAlgoProcData.sensor_info.GyroInfo[1],mLmvAlgoProcData.sensor_info.GyroInfo[2]);

        if( m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SET_PROC_INFO, &mLmvAlgoProcData, NULL) != S_EIS_OK )
        {
            MY_LOGE("EisAlg:LMV_FEATURE_SET_PROC_INFO fail(0x%x)",err);

            return LMV_RETURN_API_FAIL;
        }
        if( m_pEisAlg->EisMain(&eisCMVResult) != S_EIS_OK )
        {
            MY_LOGE("EisAlg:EisMain fail(0x%x), mSensorIdx=%u",err, mSensorIdx);
            return LMV_RETURN_API_FAIL;
        }

        EIS_GET_PLUS_INFO_STRUCT eisData2EisPlus;
        if( m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_EIS_PLUS_DATA, NULL, &eisData2EisPlus) != S_EIS_OK )
        {
            MY_LOGE("EisAlg:LMV_FEATURE_GET_LMV_PLUS_DATA fail(0x%x)",err);
            return LMV_RETURN_API_FAIL;
        }

        {
            Mutex::Autolock lock(mP2Lock);

            if( m_pLMVDrv->Get2PixelMode() == 1 )
            {
                MY_LOGD1("eisData2EisPlus.GMVx *= 2");
                eisData2EisPlus.GMVx *= 2.0f;
            }
            else if( m_pLMVDrv->Get2PixelMode() == 2 )
            {
                MY_LOGD1("eisData2EisPlus.GMVx *= 4");
                eisData2EisPlus.GMVx *= 4.0f;
            }

            mLmvLastData2EisPlus.GMVx  = eisData2EisPlus.GMVx;
            mLmvLastData2EisPlus.GMVy  = eisData2EisPlus.GMVy;
            mLmvLastData2EisPlus.ConfX = eisData2EisPlus.ConfX;
            mLmvLastData2EisPlus.ConfY = eisData2EisPlus.ConfY;
        }

        EIS_GMV_INFO_STRUCT lmvGMVResult;

        if( m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_ORI_GMV, NULL, &lmvGMVResult) != S_EIS_OK )
        {
            MY_LOGE("EisAlg:LMV_FEATURE_GET_ORI_GMV fail(0x%x)", err);
            return LMV_RETURN_API_FAIL;
        }

        if( m_pLMVDrv->Get2PixelMode() == 1 )
        {
            MY_LOGD1("eisGMVResult.LMV_GMVx *= 2");
            lmvGMVResult.EIS_GMVx *= 2.0f;
        }
        else if( m_pLMVDrv->Get2PixelMode() == 2 )
        {
            MY_LOGD1("eisGMVResult.LMV_GMVx *= 4");
            lmvGMVResult.EIS_GMVx *= 4.0f;
        }

        mGMV_X = lmvGMVResult.EIS_GMVx;
        mGMV_Y = lmvGMVResult.EIS_GMVy;
        mMAX_GMV = m_pLMVDrv->GetLMVMaxGmv();

        PrepareLmvResult(eisCMVResult.CMV_X,eisCMVResult.CMV_Y);

        mFrameCnt = m_pLMVDrv->GetFirstFrameInfo();

        MY_LOGD1("Crop(%4u),LMVIn(%5u,%5u),P1Target(%5u,%5u)=>GMV(%6d,%6d),CMV(%4d,%4d),CMVCenter(%5d,%5d)",
                mEisPlusCropRatio, mLmvInput_W,mLmvInput_H,mP1Target_W,mP1Target_H,
                mGMV_X, mGMV_Y, mCmvX_Int, mCmvY_Int, mMVtoCenterX, mMVtoCenterY);

        if( UNLIKELY(mDebugDump >= 2) )
        {
            DumpStatistic(mLmvAlgoProcData.eis_state);
        }

        MY_LOGD1("mFrameCnt(%u)",mFrameCnt);

        if( mFrameCnt == 0 )
        {
            MY_LOGD0("not first frame");
            mFrameCnt = 1;
        }

    }

    mDoLmvCount++;

    return LMV_RETURN_NO_ERROR;
}

MVOID LMVHalImp::PrepareLmvResult(const MINT32 &cmvX, const MINT32 &cmvY)
{
    MY_TRACE_FUNC_LIFE();

    #define LMV_FRACTION_MAX (0x100000000)

    MY_LOGD1("cmvX(%d),cmvY(%d)", cmvX, cmvY);
    Mutex::Autolock lock(mP1Lock);

    //====== Boundary Checking ======
    if( cmvX < 0 )
    {
        MY_LOGE("cmvX should not be negative(%u), fix to 0",cmvX);
        mCmvX_Int = mCmvX_Flt = 0;
    }
    else
    {
        MFLOAT tempCMV_X = cmvX / 256.0;
        MINT32 tempFinalCmvX = cmvX;
        mMVtoCenterX = cmvX;

        if( (tempCMV_X + (MFLOAT)mP1ResizeOut_W) > (MFLOAT)mP1ResizeIn_W )
        {
            MY_LOGD0("cmvX too large(%u), fix to %u",cmvX,(mP1ResizeIn_W - mP1ResizeOut_W));
            tempFinalCmvX = (mP1ResizeIn_W - mP1ResizeOut_W);
        }

        mMVtoCenterX -=  ((mLmvInput_W-mP1Target_W)<<(intPartShift-1)); //Make mv for the top-left of center

        if( m_pLMVDrv->Get2PixelMode() == 1 )
        {
            MY_LOGD1("tempFinalCmvX *= 2");
            tempFinalCmvX *= 2;
            mMVtoCenterX *= 2;
        }
        else if( m_pLMVDrv->Get2PixelMode() == 2 )
        {
            MY_LOGD1("tempFinalCmvX *= 4");
            tempFinalCmvX *= 4;
            mMVtoCenterX *= 4;
        }

        mPrev_X = mCur_X;

        mCmvTmpX_Int = (tempFinalCmvX & (~0xFF)) >> intPartShift;
        mCmvTmpX_Flt = (tempFinalCmvX & (0xFF)) << floatPartShift;

        mCur_X = mCmvTmpX_Int + (float)mCmvTmpX_Flt / LMV_FRACTION_MAX;
    }

    if( cmvY < 0 )
    {
        MY_LOGE("cmvY should not be negative(%u), fix to 0",cmvY);

        mCmvY_Int = mCmvY_Flt = 0;
    }
    else
    {
        MFLOAT tempCMV_Y = cmvY / 256.0;
        MINT32 tempFinalCmvY = cmvY;
        mMVtoCenterY = cmvY;

        if( (tempCMV_Y + (MFLOAT)mP1ResizeOut_H) > (MFLOAT)mP1ResizeIn_H )
        {
            MY_LOGD0("cmvY too large(%u), fix to %u",cmvY,(mP1ResizeIn_H - mP1ResizeOut_H));

            tempFinalCmvY = (mP1ResizeIn_H - mP1ResizeOut_H);
        }
        mMVtoCenterY -=  ((mP1ResizeIn_H-mP1ResizeOut_H)<<(intPartShift-1)); //Make mv for the top-left of center

        mPrev_Y = mCur_Y;

        mCmvTmpY_Int = (tempFinalCmvY & (~0xFF)) >> intPartShift;
        mCmvTmpY_Flt = (tempFinalCmvY & (0xFF)) << floatPartShift;

        mCur_Y = mCmvTmpY_Int + (float)mCmvTmpY_Flt / LMV_FRACTION_MAX;
    }

    if (fabs(mCur_X - mPrev_X) > 0.8 || fabs(mCur_Y - mPrev_Y) > 0.8)
    {
        mCmvX_Int = mCmvTmpX_Int;
        mCmvX_Flt = mCmvTmpX_Flt;
        mCmvY_Int = mCmvTmpY_Int;
        mCmvY_Flt = mCmvTmpY_Flt;
    }

    MY_LOGD1("X(%u,%u),Y(%u,%u),MVtoCenter(%d,%d)", mCmvX_Int, mCmvX_Flt,
                                                    mCmvY_Int, mCmvY_Flt, mMVtoCenterX, mMVtoCenterY);
}

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
    MY_TRACE_API_LIFE();

    if( mLmvSupport == MFALSE )
    {
        MY_LOGD0("not support LMV");
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
        a_TarWidth  = mP1ResizeOut_W;
        a_TarHeight = mP1ResizeOut_H;
        a_MVtoCenterX = mMVtoCenterX;
        a_MVtoCenterY = mMVtoCenterY;
        a_isFromRRZ = 1; //Hardcode MUST be fix later!!!!
    }

    MY_LOGD1("X(%u,%u),Y(%u,%u)", a_CMV_X_Int, a_CMV_X_Flt, a_CMV_Y_Int, a_CMV_Y_Flt);

}

MVOID LMVHalImp::GetGmv(MINT32 &aGMV_X, MINT32 &aGMV_Y, MUINT32 *confX, MUINT32 *confY, MUINT32 *MAX_GMV)
{
    MY_TRACE_API_LIFE();

    if( mLmvSupport == MFALSE )
    {
        MY_LOGD("mSensorIdx(%u) not support LMV",mSensorIdx);
        return;
    }

    aGMV_X = mGMV_X;
    aGMV_Y = mGMV_Y;

    if( MAX_GMV != NULL )
    {
        *MAX_GMV = mMAX_GMV;
    }

    {
        Mutex::Autolock lock(mP1Lock);

        if( confX != NULL )
        {
            *confX = mLmvLastData2EisPlus.ConfX;
        }

        if( confY != NULL )
        {
            *confY = mLmvLastData2EisPlus.ConfY;
        }
    }

    if( UNLIKELY(mDebugDump >= 1) )
    {
        if( confX && confY )
        {
            MY_LOGD0("GMV(%d,%d),Conf(%d,%d)", aGMV_X, aGMV_Y, *confX, *confY);
        }
        else
        {
            MY_LOGD0("GMV(%d,%d)", aGMV_X, aGMV_Y);
        }
    }
}

MBOOL LMVHalImp::GetLMVSupportInfo(const MUINT32 &aSensorIdx)
{
    mLmvSupport = m_pLMVDrv->GetLMVSupportInfo(aSensorIdx);
    return mLmvSupport;
}

MVOID LMVHalImp::GetLMVInputSize(MUINT32 *aWidth, MUINT32 *aHeight)
{
    if( NULL != m_pLMVDrv )
    {
        m_pLMVDrv->GetLMVInputSize(aWidth, aHeight);
    }
    else
    {
        MY_LOGE("m_pLMVDrv is NULL");
    }
}

MVOID LMVHalImp::GetRegSetting(void *data)
{
    if( NULL != m_pLMVDrv )
    {
        m_pLMVDrv->GetRegSetting(data);
    }
    else
    {
        MY_LOGE("m_pLMVDrv is NULL");
    }
}

MUINT32 LMVHalImp::GetLMVStatus()
{
    MY_TRACE_API_LIFE();

    Mutex::Autolock lock(mLock);

    MUINT32 retVal = mEisPass1Enabled;
    return retVal;
}

MINT32 LMVHalImp::GetBufLMV(android::sp<IImageBuffer>& spBuf)
{
    MY_TRACE_API_LIFE();

    Mutex::Autolock lock(mLMVOBufferListLock);

    if( !mLMVOBufferList.empty() )
    {
        spBuf = mLMVOBufferList.front();
        mLMVOBufferList.pop();

        MY_LOGD1("GetBufLMV : %zu", mLMVOBufferList.size());
    }
    else
    {
        spBuf = m_pLMVOSliceBuffer[LMVO_BUFFER_NUM];
        MY_LOGE("Error: LMVO buffer pool is empty!");
    }
    return LMV_RETURN_NO_ERROR;
}

MINT32 LMVHalImp::NotifyLMV(QBufInfo& pBufInfo)
{
    MY_TRACE_API_LIFE();

    android::sp<IImageBuffer> retBuf;
    for( size_t i = 0; i < pBufInfo.mvOut.size(); i++ )
    {
        if( pBufInfo.mvOut[i].mPortID.index == PORT_EISO.index )
        {

            Mutex::Autolock lock(mLMVOBufferListLock);

            retBuf = (pBufInfo.mvOut.at(i).mBuffer);
            mLMVOBufferList.push(retBuf);

            MY_LOGD1("NotifyLMV : %zu", mLMVOBufferList.size());
        }
    }

    return LMV_RETURN_NO_ERROR;
}

MINT32 LMVHalImp::NotifyLMV(android::sp<NSCam::IImageBuffer>& spBuf)
{
    MY_TRACE_API_LIFE();

    Mutex::Autolock lock(mLMVOBufferListLock);

    if( spBuf != 0 )
    {
        mLMVOBufferList.push(spBuf);
        MY_LOGD1("NotifyLMV : %zu - Drop", mLMVOBufferList.size());
    }

    return LMV_RETURN_NO_ERROR;
}

MVOID LMVHalImp::GetLMVStatistic(EIS_STATISTIC_STRUCT *a_pLMV_Stat)
{
    for( MINT32 i = 0; i < LMV_MAX_WIN_NUM; ++i )
    {
        a_pLMV_Stat->i4LMV_X[i]    = mLmvAlgoProcData.eis_state.i4LMV_X[i];
        a_pLMV_Stat->i4LMV_Y[i]    = mLmvAlgoProcData.eis_state.i4LMV_Y[i];
        a_pLMV_Stat->i4LMV_X2[i]   = mLmvAlgoProcData.eis_state.i4LMV_X2[i];
        a_pLMV_Stat->i4LMV_Y2[i]   = mLmvAlgoProcData.eis_state.i4LMV_Y2[i];
        a_pLMV_Stat->NewTrust_X[i] = mLmvAlgoProcData.eis_state.NewTrust_X[i];
        a_pLMV_Stat->NewTrust_Y[i] = mLmvAlgoProcData.eis_state.NewTrust_Y[i];
        a_pLMV_Stat->SAD[i]        = mLmvAlgoProcData.eis_state.SAD[i];
        a_pLMV_Stat->SAD2[i]       = mLmvAlgoProcData.eis_state.SAD2[i];
        a_pLMV_Stat->AVG_SAD[i]    = mLmvAlgoProcData.eis_state.AVG_SAD[i];
    }
}

MVOID LMVHalImp::GetEisCustomize(EIS_TUNING_PARA_STRUCT *a_pDataOut)
{
    MY_TRACE_FUNC_LIFE();

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

}

MVOID LMVHalImp::DumpStatistic(const EIS_STATISTIC_STRUCT &aLmvStat)
{
    MY_TRACE_FUNC_LIFE();

    MINT32 totalMVX = 0;
    MINT32 totalMVY = 0;

    for( int i = 0; i < LMV_MAX_WIN_NUM; ++i )
    {
        totalMVX += aLmvStat.i4LMV_X[i];
        totalMVY += aLmvStat.i4LMV_Y[i];
    }

    MPoint rawLMV(totalMVX/LMV_MAX_WIN_NUM, totalMVY/LMV_MAX_WIN_NUM);

    MY_LOGD0("GMV(%6d,%6d),CMV(%4d,%4d):Raw(%6d,%6d),Center(%5d,%5d)",
            mGMV_X, mGMV_Y, mCmvX_Int, mCmvY_Int, rawLMV.x, rawLMV.y, mMVtoCenterX, mMVtoCenterY);

    if( mDebugDump >= 3 )
    {
        for( int i = 0; i < LMV_MAX_WIN_NUM; ++i )
        {
            MY_LOGD0("MB%d%d,(LMV_X,LMV_Y)=(%d,%d),(LMV_X2,LMV_Y2)=(%d,%d),MinSAD(%u),(NewTrust_X,NewTrust_Y)=(%u,%u),MinSAD2(%u),AvgSAD(%u)",
                    (i/4), (i%4),
                    aLmvStat.i4LMV_X[i], aLmvStat.i4LMV_Y[i],
                    aLmvStat.i4LMV_X2[i], aLmvStat.i4LMV_Y2[i],
                    aLmvStat.SAD[i],
                    aLmvStat.NewTrust_X[i], aLmvStat.NewTrust_Y[i],
                    aLmvStat.SAD2[i],
                    aLmvStat.AVG_SAD[i]);
        }

        if( mDumpFile != NULL )
        {
            fprintf(mDumpFile, "GMV(%6d,%6d),CMV(%4d,%4d):Raw(%6d,%6d),Center(%4d,%4d)\n",
                    mGMV_X, mGMV_Y, mCmvX_Int, mCmvY_Int, rawLMV.x, rawLMV.y, mMVtoCenterX, mMVtoCenterY);
        }
    }
}

LMVData LMVHalImp::getDefaultData(QBufInfo const &pBufInfo, const MUINT32 rrzIdx)
{
    if( UNLIKELY( rrzIdx == NSImageio::NSIspio::EPortIndex_UNKNOW ) )
    {
        MY_LOGA("Error: Unknwon buffer index(rrz:%d)"
                "\nCRDISPATCH_KEY:LMV unknown buffer index",
                rrzIdx);
    }

    MSize srcSize = pBufInfo.mvOut.at(rrzIdx).mMetaData.mDstSize;
    MSize dstSize = MSize(srcSize.w / (mEisPlusCropRatio / 100.0), srcSize.h / (mEisPlusCropRatio / 100.0));
    MPoint offset = MPoint((srcSize.w - dstSize.w) / 2.0, (srcSize.h - dstSize.h) / 2.0);

    const MINT64 timestamp = pBufInfo.mvOut.at(rrzIdx).mMetaData.mTimeStamp;

    MINT32 hwTS = (timestamp >> 32) & 0xFFFFFFFF; //High word
    MINT32 lwTS = (timestamp & 0xFFFFFFFF);     //Low word

    LMVData lmvDefaultData(offset.x, 0, offset.y, 0, dstSize.w, dstSize.h, 0, 0,
                           1, 0, 0, 0, 0, timestamp, hwTS, lwTS, 0, 0);
    MY_LOGD1("LMV default data(%d):CMV(%d-%d,%d-%d),Target(%dx%d),CMVCenter(%d,%d),GMV(%d,%d),Conf(%d,%d),MaxGMV(%d)."
             "Src(%dx%d),Dst(%dx%d),Factor(%d)",
             rrzIdx, lmvDefaultData.cmv_x_int, lmvDefaultData.cmv_x_float, lmvDefaultData.cmv_y_int, lmvDefaultData.cmv_y_float,
             lmvDefaultData.width, lmvDefaultData.height, lmvDefaultData.cmv_x_center, lmvDefaultData.cmv_y_center,
             lmvDefaultData.gmv_x, lmvDefaultData.gmv_y, lmvDefaultData.conf_x, lmvDefaultData.conf_y, lmvDefaultData.maxGMV,
             srcSize.w, srcSize.h, dstSize.w, dstSize.h, mEisPlusCropRatio);
    return lmvDefaultData;
}

