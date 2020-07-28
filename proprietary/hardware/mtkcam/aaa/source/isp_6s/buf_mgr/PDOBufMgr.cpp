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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "pdo_buf_mgr"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <mtkcam/utils/std/Trace.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <string.h>

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/drv/IHalSensor.h>

#include "IBufMgr.h"
#include "IResultPool.h"

#if CAM3_FAKE_SENSOR_DRV
#include "fake_sensor_drv/IStatisticPipe.h"
#include "fake_sensor_drv/ICamsvStatisticPipe.h"
#include "fake_sensor_drv/IHalCamIO.h"
#else
#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>
#include <mtkcam/drv/iopipe/CamIO/ICamsvStatisticPipe.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#endif

#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <private/IopipeUtils.h>
#include <mtkcam/drv/iopipe/Event/IoPipeEvent.h>

//
#include "mcu_drv.h"

//
#include <dbg_aaa_param.h>
#include <dbg_af_param.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <pd_mgr_if.h>
#include <aaa_hal_sttCtrl.h>
#include <vector>

//ion
//#include <ion.h>
#include <sys/mman.h>
#include <ion/ion.h>
#include <libion_mtk/include/ion.h>


using namespace android;
using namespace NS3Av3;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

//
#define _GET_TIME_STAMP_NS_() ({\
        MINT64 _ts = 0;\
        struct timespec t;\
        t.tv_sec = t.tv_nsec = 0;\
        clock_gettime(CLOCK_MONOTONIC, &t);\
        _ts = (t.tv_sec) * 1000000000LL + t.tv_nsec;\
        _ts;\
    })

/************************************************************************/
/* Systrace                                                             */
/************************************************************************/

#define TRACE_LEVEL_DBG 1
#define TRACE_LEVEL_DEF 0

#ifdef PDO_TRACE_FMT_BEGIN
#undef PDO_TRACE_FMT_BEGIN
#endif
#define PDO_TRACE_FMT_BEGIN(lv, fmt, arg...)      \
    do {                                         \
        if (m_i4DbgSystraceLevel >= lv) {        \
            CAM_TRACE_FMT_BEGIN(fmt, ##arg);     \
        }                                        \
    } while(0)

#ifdef PDO_TRACE_FMT_END
#undef PDO_TRACE_FMT_END
#endif
#define PDO_TRACE_FMT_END(lv)                     \
    do {                                         \
        if (m_i4DbgSystraceLevel >= lv) {        \
            CAM_TRACE_FMT_END();                 \
        }                                        \
    } while(0)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class PDOBufMgrImp : public PDOBufMgr
{
    struct ImgoInfo
    {
        MSize             mImgSize;
        size_t            mImgBitsPerPixel;
        size_t            mPlaneBitsPerPixel;
        size_t            mPlaneCount;
        size_t            mBufSizeInBytes;
        size_t            mBufStridesInBytes;
        MUINT32           mMagicNum;
        MUINT32           mSensorIdx;
        ImageDescRawType  mFmt;
        ion_user_handle_t mIonBuf_handle;
        MINT32            mIonBuf_share_fd;
        MINT32            mImgHeapID;
        MINT32            mLensPos;
        MINT64            mTimeStamp;

        ImgoInfo()
            : mImgSize()
            , mImgBitsPerPixel(0)
            , mPlaneBitsPerPixel(0)
            , mPlaneCount(0)
            , mBufSizeInBytes(0)
            , mBufStridesInBytes(0)
            , mMagicNum(0)
            , mSensorIdx(0)
            , mFmt(eIMAGE_DESC_RAW_TYPE_INVALID)
            , mIonBuf_handle(0)
            , mIonBuf_share_fd(0)
            , mImgHeapID(0)
            , mLensPos(-1)
            , mTimeStamp(0)
        {}

    };


public:
    virtual MBOOL destroyInstance();
    PDOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx);
    virtual      ~PDOBufMgrImp();

    virtual MBOOL dequeueHwBuf();
    virtual MBOOL enqueueHwBuf();
    virtual StatisticBufInfo* dequeueSwBuf();
    virtual MVOID abortDequeue();
    virtual MINT32 waitDequeue()
    {
        return 0;
    };
    virtual MVOID notifyPreStop();
    virtual MVOID reset();

protected:
    /**
    * @brief abort for waiting dma done.
    */
    virtual MBOOL abort_dequeue_dma();
    /**
    * @brief deque pure raw buffer for normalpipe wrapper. (mmap)
    */
    virtual MBOOL deque_pure_raw(StatisticBufInfo &oSttBufInfo);
    /**
    * @brief enque pure raw buffer.(munmap)
    */
    virtual MBOOL enque_pure_raw(StatisticBufInfo &iSttBufInfo);
    /**
    * @brief get P1 callback event and acquire image buffer from NormalPipeDequedEvent
    */
    static NSCam::NSIoPipe::IoPipeEventCtrl onP1Dequed(PDOBufMgrImp *user, NSCam::NSIoPipe::NormalPipeDequedEvent &evt);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32           const m_i4SensorDev;
    MINT32           const m_i4SensorIdx;
    MINT32                 m_i4DbgSystraceLevel;
    MBOOL                  m_bDebugEnable;
    MBOOL                  m_bDumpBufEnable;
    IStatisticPipe*        m_pSttPipe;
    ICamsvStatisticPipe*   m_pCamsvSttPipe;
    PortID                 m_rPort;
    FEATURE_PDAF_STATUS    m_i4PDDataPath;
    MBOOL                  m_bAbort;
    MBOOL                  m_bSkipEnq;
    Hal3ASttCtrl*          m_p3ASttCtrl;
    MBOOL                  m_bPreStop;
    //Condition              mP1DeqWaitCond;
    sem_t                  m_semP1Deq;
    Mutex                  m_P1DeqWaitLock;
    Mutex                  m_FlowLock;
    android::sp<IoPipeEventHandle>  mspIoPipeEvtHandleDequed;
    MINT32                 m_i4IonDevFd;
    vector<ImgoInfo>       m_vecPureRawBufInfo;
    MINT32                 m_i4LensPosCur;
    MINT32                 m_i4LensPosPre;
};

#if (CAM3_3ATESTLVL > CAM3_3AUT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PDOBufMgr*
PDOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo __unused)
{
    PDOBufMgrImp* pObj = new PDOBufMgrImp(i4SensorDev,i4SensorIdx);
    return pObj;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PDOBufMgrImp::
destroyInstance()
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    delete this;
    CAM_LOGD("[%s] -", __FUNCTION__);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PDOBufMgrImp::
PDOBufMgrImp(MINT32 i4SensorDev, MINT32 i4SensorIdx)
    : m_i4SensorDev(i4SensorDev)
    , m_i4SensorIdx(i4SensorIdx)
    , m_i4DbgSystraceLevel(TRACE_LEVEL_DEF)
    , m_bDebugEnable(MFALSE)
    , m_pSttPipe(NULL)
    , m_pCamsvSttPipe(NULL)
    , m_rPort(PORT_PDO)
    , m_i4PDDataPath(FEATURE_PDAF_UNSUPPORT)
    , m_bAbort(MFALSE)
    , m_bSkipEnq(MFALSE)
    , m_bPreStop(MFALSE)
    , m_i4IonDevFd(-1)
    , m_i4LensPosCur(-1)
    , m_i4LensPosPre(-1)
{
    //
    m_vecPureRawBufInfo.clear();
    m_vecPureRawBufInfo.reserve(4);

    //
    m_bDebugEnable   = property_get_int32("vendor.debug.pdo_mgr.enable", 0);
    m_bDumpBufEnable = property_get_int32("vendor.pd.dump.enable", 0);
    m_i4DbgSystraceLevel = property_get_int32("vendor.debug.pdsystrace", TRACE_LEVEL_DEF);

    //
    m_p3ASttCtrl = Hal3ASttCtrl::getInstance(m_i4SensorDev);

    //
    m_i4PDDataPath = m_p3ASttCtrl->quertPDAFStatus();

    // initial driver
    switch( m_i4PDDataPath)
    {
        case FEATURE_PDAF_SUPPORT_BNR_PDO:
        case FEATURE_PDAF_SUPPORT_PBN_PDO:
        {
            m_pSttPipe = IStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG);
        }
        break;

        case FEATURE_PDAF_SUPPORT_LEGACY:
        {
            // Subscribe IoPipeEventSystem
            NSCam::NSIoPipe::IoPipeEventSystem& evtSystem = NSCam::NSIoPipe::IoPipeEventSystem::getGlobal();

            if(mspIoPipeEvtHandleDequed!= NULL)
            {
                mspIoPipeEvtHandleDequed->unsubscribe();
                mspIoPipeEvtHandleDequed = NULL;
            }

            mspIoPipeEvtHandleDequed = evtSystem.subscribe(NSCam::NSIoPipe::EVT_NORMAL_PIPE_DEQUED, onP1Dequed, this);
            if (mspIoPipeEvtHandleDequed == NULL)
            {
                CAM_LOGW("[%s] IoPipeEventSystem subscribe EVT_NORMAL_PIPE_DEQUED fail", __FUNCTION__);
            }

            //create ion device FD
            m_i4IonDevFd = ::mt_ion_open(LOG_TAG);
        }
        break;

        case FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL:
        {
            m_pCamsvSttPipe = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, VC_PDAF_STATS);
        }
        break;

        default:
            break;

    }

    sem_init( &m_semP1Deq, 0, 0);

    //create folder for saving debug information as m_i4DbgPdDump is set.
    if(m_bDumpBufEnable&2)
    {
        FILE *fp = fopen("/sdcard/pdo/pd_mgr_info", "w");
        if( NULL == fp)
        {
            MINT32 err = mkdir( "/sdcard/pdo", S_IRWXU | S_IRWXG | S_IRWXO);
            CAM_LOGD( "create folder /sdcard/pdo (%d)", err);
        }
        else
        {
            fprintf( fp, "folder /sdcard/pdo is exist");
            fclose( fp);
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PDOBufMgrImp::
~PDOBufMgrImp()
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] dev(%d) +", __FUNCTION__, m_i4SensorDev);

    //
    switch( m_i4PDDataPath)
    {
        case FEATURE_PDAF_SUPPORT_BNR_PDO:
        case FEATURE_PDAF_SUPPORT_PBN_PDO:
        {
            if(m_pSttPipe != NULL)
            {
                m_pSttPipe->destroyInstance(LOG_TAG);
                m_pSttPipe = NULL;
            }
        }
        break;

        case FEATURE_PDAF_SUPPORT_LEGACY:
        {
            // Unsubscribe IoPipeEventSystem
            if (mspIoPipeEvtHandleDequed != NULL)
            {
                mspIoPipeEvtHandleDequed->unsubscribe();
                mspIoPipeEvtHandleDequed = NULL;
            }


            // release resource
            {
                Mutex::Autolock lock(m_P1DeqWaitLock);

                if( m_vecPureRawBufInfo.size())
                {
                    for(vector<ImgoInfo>::iterator itr = m_vecPureRawBufInfo.begin(); itr !=m_vecPureRawBufInfo.end(); itr++)
                    {
                        if( /* for decreasing handle ref count */
                            ::ion_free(m_i4IonDevFd, itr->mIonBuf_handle))
                        {
                            CAM_LOGE("ion_free failed : ion_info[handle(%d)]", itr->mIonBuf_handle);
                        }
                        ::ion_share_close(m_i4IonDevFd, itr->mIonBuf_share_fd);
                    }

                    m_vecPureRawBufInfo.clear();
                }
            }

            //close ion device FD
            ::ion_close(m_i4IonDevFd);
            m_i4IonDevFd = -1;
        }
        break;

        case FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL:
        {
            if(m_pCamsvSttPipe != NULL)
            {
                m_pCamsvSttPipe->destroyInstance(LOG_TAG);
                m_pCamsvSttPipe = NULL;
            }
        }
        break;

        default:
            break;

    }


    CAM_LOGD_IF(m_bDebugEnable,"[%s] dev(%d) -", __FUNCTION__, m_i4SensorDev);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
PDOBufMgrImp::
dequeueHwBuf()
{
    Mutex::Autolock lock(m_FlowLock);

    MINT32 ret = -1;

    CAM_LOGD_IF(m_bDebugEnable,"[%s] + dev(%d) abort(%d) preStop(%d)", __FUNCTION__, m_i4SensorDev, m_bAbort, m_bPreStop);

    //
    if( m_bPreStop || m_bAbort)
    {
        CAM_LOGD_IF(m_bDebugEnable,"[%s] dev(%d) abort(%d) preStop(%d) -", __FUNCTION__, m_i4SensorDev, m_bAbort, m_bPreStop);
        return ret;
    }

    /*********************************************
     * deque HW buffer from driver.(blocking code)
     *********************************************/
    PDO_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "PD data deque %d", m_i4PDDataPath);
    //
    MBOOL bDequed = MFALSE;
    //
    QBufInfo         rDQBuf;
    StatisticBufInfo rHwBuf;
    //
    mcuMotorInfo  rMotorInfo;
    mcuMotorInfo *pMotorInfo = nullptr;
    switch( m_i4PDDataPath)
    {
        case FEATURE_PDAF_SUPPORT_BNR_PDO:
        case FEATURE_PDAF_SUPPORT_PBN_PDO:
        {
            if(m_pSttPipe)
            {
                m_bSkipEnq = MFALSE;
                bDequed = m_pSttPipe->deque( m_rPort, rDQBuf, ESTT_CacheInvalidByRange);

                //
                if( bDequed && rDQBuf.mvOut.size())
                {
                    //
                    pMotorInfo = &rMotorInfo;
                    MCUDrv::getInstance(m_i4SensorDev)->getMCUInfo( pMotorInfo);

                    m_i4LensPosPre = m_i4LensPosCur;
                    m_i4LensPosCur = rMotorInfo.u4CurrentPosition;

                    //
                    int idx = rDQBuf.mvOut.size()-1;
                    BufInfo rLastDQBuf = rDQBuf.mvOut.at(idx);

                    // Prepare buffer information for PD manager
                    rHwBuf.mMagicNumber     = rLastDQBuf.mMetaData.mMagicNum_hal;
                    rHwBuf.mSize            = rLastDQBuf.mSize;
                    rHwBuf.mVa              = rLastDQBuf.mVa;
                    rHwBuf.mPa_offset       = rLastDQBuf.mPa_offset;
                    rHwBuf.mPrivateData     = rLastDQBuf.mMetaData.mPrivateData;
                    rHwBuf.mPrivateDataSize = rLastDQBuf.mMetaData.mPrivateDataSize;
                    rHwBuf.mStride          = rLastDQBuf.mStride;
                    rHwBuf.mLensPosition    = m_i4LensPosPre;
                    rHwBuf.mTimeStamp       = rLastDQBuf.mMetaData.mTimeStamp; /* nano second */
                }
            }
        }
        break;

        case FEATURE_PDAF_SUPPORT_LEGACY:
        {
            if(m_i4IonDevFd)
            {
                if(deque_pure_raw(rHwBuf))
                {
                    bDequed = (rHwBuf.mVa && rHwBuf.mSize) ? MTRUE : MFALSE;
                }
            }
        }
        break;

        case FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL:
        {
            if(m_pCamsvSttPipe)
            {
                PortID _dq_portID;
                bDequed = m_pCamsvSttPipe->deque(_dq_portID, rDQBuf, ESTT_CacheInvalidByRange);

                if( bDequed && rDQBuf.mvOut.size())
                {
                    //
                    pMotorInfo = &rMotorInfo;
                    MCUDrv::getInstance(m_i4SensorDev)->getMCUInfo( pMotorInfo);

                    m_i4LensPosPre = m_i4LensPosCur;
                    m_i4LensPosCur = rMotorInfo.u4CurrentPosition;

                    //
                    int idx = rDQBuf.mvOut.size()-1;
                    BufInfo rLastDQBuf = rDQBuf.mvOut.at(idx);

                    // Prepare buffer information for PD manager
                    rHwBuf.mMagicNumber     = rLastDQBuf.mMetaData.mMagicNum_hal;
                    rHwBuf.mSize            = rLastDQBuf.mSize;
                    rHwBuf.mVa              = rLastDQBuf.mVa;
                    rHwBuf.mPa_offset       = rLastDQBuf.mPa_offset;
                    rHwBuf.mPrivateData     = rLastDQBuf.mMetaData.mPrivateData;
                    rHwBuf.mPrivateDataSize = rLastDQBuf.mMetaData.mPrivateDataSize;
                    rHwBuf.mStride          = rLastDQBuf.mStride;
                    rHwBuf.mLensPosition    = m_i4LensPosPre;
                    rHwBuf.mTimeStamp       = rLastDQBuf.mMetaData.mTimeStamp; /* nano second */
                }
            }
        }
        break;

        default:
            break;

    }

    // update flash state to statistic information.
    IResultPool* pResultPoolObj = IResultPool::getInstance(m_i4SensorDev);
    AllResult_T *pAllResult = pResultPoolObj->getResultByReqNum(rHwBuf.mMagicNumber, __FUNCTION__);
    if(pAllResult)
    {
        FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)(pAllResult->ModuleResult[E_FLASH_RESULTTOMETA]->read());
        if(pFLASHResult != NULL)
            rHwBuf.mFlashState = pFLASHResult->u1FlashState;
    }
    pResultPoolObj->returnResult(pAllResult, __FUNCTION__);

    PDO_TRACE_FMT_END(TRACE_LEVEL_DEF);

    //
    if( bDequed)
    {
        //
        if( pMotorInfo)
        {
            CAM_LOGD_IF( m_bDebugEnable,
                         "[%s] dev(%d), data path(%d), pdo port index(%d), HW Buffer info : time stamp(%lld)/magic number(%d)/heapID(%d)/va(0x%p)/size(0x%x)/stride(0x%x BYTE)/Lens[pre(DAC:%d, TS:%d) cur(DAC:%d, TS:%d)]",
                         __FUNCTION__,
                         m_i4SensorDev,
                         m_i4PDDataPath,
                         m_rPort.index,
                         rHwBuf.mTimeStamp,
                         rHwBuf.mMagicNumber,
                         rHwBuf.mImgoBuf_share_fd,
                         (void*)(rHwBuf.mVa),
                         rHwBuf.mSize,
                         rHwBuf.mStride,
                         pMotorInfo->u4PreviousPosition,
                         pMotorInfo->u8PreviousTimestamp,
                         pMotorInfo->u4CurrentPosition,
                         pMotorInfo->u8CurrentTimestamp);
        }
        else
        {
            CAM_LOGD_IF( m_bDebugEnable,
                         "[%s] dev(%d), data path(%d), pdo port index(%d), HW Buffer info : time stamp(%lld)/magic number(%d)/heapID(%d)/va(0x%p)/size(0x%x)/stride(0x%x BYTE)/Lens(%d)",
                         __FUNCTION__,
                         m_i4SensorDev,
                         m_i4PDDataPath,
                         m_rPort.index,
                         rHwBuf.mTimeStamp,
                         rHwBuf.mMagicNumber,
                         rHwBuf.mImgoBuf_share_fd,
                         (void*)(rHwBuf.mVa),
                         rHwBuf.mSize,
                         rHwBuf.mStride,
                         rHwBuf.mLensPosition);

        }

        //
        if(m_bDumpBufEnable&2)
        {
            static MUINT32 dump_raw_data_cnt = 0;

            if( /**/
                ((m_i4PDDataPath!=FEATURE_PDAF_SUPPORT_LEGACY)) ||
                ((m_i4PDDataPath==FEATURE_PDAF_SUPPORT_LEGACY) && (rHwBuf.mImgoFmt==eIMAGE_DESC_RAW_TYPE_PURE) && (dump_raw_data_cnt%10)))
            {
                char fileName[256];
                FILE *fp = nullptr;

                //
                sprintf(fileName,
                        "/sdcard/pdo/%llu_%05d_hwBuf_path_%d_va_0x%p_size_0x%x_stride_0x%x_w_%d_h_%d_fmt_%d.raw",
                        _GET_TIME_STAMP_NS_(),
                        rHwBuf.mMagicNumber,
                        m_i4PDDataPath,
                        reinterpret_cast<void *>(rHwBuf.mVa),
                        rHwBuf.mSize,
                        rHwBuf.mStride,
                        rHwBuf.mImgoSizeW,
                        rHwBuf.mImgoSizeH,
                        rHwBuf.mImgoFmt);

                fp = fopen(fileName, "w");
                if( fp)
                {
                    CAM_LOGD( "dump file : %s", fileName);
                    fwrite(reinterpret_cast<void *>(rHwBuf.mVa), 1, rHwBuf.mSize, fp);
                    fclose(fp);
                }
            }

            dump_raw_data_cnt += rHwBuf.mImgoFmt==eIMAGE_DESC_RAW_TYPE_PURE ? 1 : 0;
        }


        /*********************************************
         * process buffer
         *********************************************/
        if( !m_bPreStop && !m_bAbort)
        {
            PDO_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "process buf#%d", rHwBuf.mMagicNumber);
            CAM_LOGD_IF(m_bDebugEnable, "[%s] pass PD buffer to mgr + (%d)", __FUNCTION__, rHwBuf.mMagicNumber);
            IPDMgr::getInstance().postToPDTask( m_i4SensorDev, &rHwBuf, pMotorInfo);
            CAM_LOGD_IF(m_bDebugEnable, "[%s] pass PD buffer to mgr -", __FUNCTION__, rHwBuf.mMagicNumber);
            PDO_TRACE_FMT_END(TRACE_LEVEL_DBG);
        }

        //
        ret = MTRUE;


        /*********************************************
         * enque HW buffer back driver
         *********************************************/
        MBOOL bEnqueDn = MFALSE;
        PDO_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "PD data enque %d", m_i4PDDataPath);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] enque Hw buffer back driver + (%d)", __FUNCTION__, rHwBuf.mMagicNumber);
        switch( m_i4PDDataPath)
        {
            case FEATURE_PDAF_SUPPORT_BNR_PDO:
            case FEATURE_PDAF_SUPPORT_PBN_PDO:
            {
                if(m_pSttPipe && !m_bAbort && !m_bPreStop)
                {
                    if (!m_bSkipEnq)
                    {
                        bEnqueDn = m_pSttPipe->enque(rDQBuf);
                    }
                    else
                    {
                        CAM_LOGW("[%s] dev(%d), skip sttPipe enque",
                             __FUNCTION__,
                             m_i4SensorDev);
                    }
                }
                else
                {
                    CAM_LOGE("[%s] dev(%d), PD data path(%d), sttPipe is not exist!!",
                             __FUNCTION__,
                             m_i4SensorDev,
                             m_i4PDDataPath);
                }
            }
            break;

            case FEATURE_PDAF_SUPPORT_LEGACY:
            {
                bEnqueDn = enque_pure_raw(rHwBuf);
            }
            break;

            case FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL:
            {
                if(m_pCamsvSttPipe && !m_bAbort && !m_bPreStop)
                {
                    bEnqueDn = m_pCamsvSttPipe->enque(rDQBuf);
                }
                else
                {
                    CAM_LOGE("[%s] dev(%d), PD data path(%d), camsvSttPipe is not exist!!",
                             __FUNCTION__,
                             m_i4SensorDev,
                             m_i4PDDataPath);
                }
            }
            break;

            default:
                break;

        }
        CAM_LOGD_IF(m_bDebugEnable, "[%s] enque Hw buffer back driver - EnqueDn(%d)", __FUNCTION__, bEnqueDn);
        PDO_TRACE_FMT_END(TRACE_LEVEL_DEF);
    }
    else
    {
        CAM_LOGE("[%s] dev(%d), data path(%d), Deque HW buffer fail : bDequed(%d) SttPipe(%p) CamsvSttPipe(%p) ionDevFD(%d)",
                 __FUNCTION__,
                 m_i4SensorDev,
                 m_i4PDDataPath,
                 bDequed,
                 m_pSttPipe,
                 m_pCamsvSttPipe,
                 m_i4IonDevFd);

        ret = -1;
    }


    //
    CAM_LOGD_IF(m_bDebugEnable,"[%s] - dev(%d) ret(%d) abort(%d) preStop(%d)", __FUNCTION__, m_i4SensorDev, ret, m_bAbort, m_bPreStop);
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PDOBufMgrImp::
enqueueHwBuf()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatisticBufInfo*
PDOBufMgrImp::
dequeueSwBuf()
{
    // Unused code, beacuse PDAF function executed when deque and enque statistic data.
    return NULL;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
PDOBufMgrImp::
abortDequeue()
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    m_bAbort = MTRUE;

    // In order to avoid waiting dma signal during preview end stage.
    // Abort waiting dma bufer ready after setting flag(m_bAbort) as true.
    MBOOL status = abort_dequeue_dma();

    CAM_LOGD("[%s] - dev(%d), PD data path(%d), abort dequeue dma status(%d)",
             __FUNCTION__,
             m_i4SensorDev,
             m_i4PDDataPath,
             status);

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
PDOBufMgrImp::
notifyPreStop()
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    m_bPreStop = MTRUE;

    MBOOL status = abort_dequeue_dma();

    Mutex::Autolock lock(m_FlowLock);

    CAM_LOGD("[%s] - dev(%d), PD data path(%d), abort dequeue dma status(%d)",
             __FUNCTION__,
             m_i4SensorDev,
             m_i4PDDataPath,
             status);
}

MVOID
PDOBufMgrImp::
reset()
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    /* W+T susprned/resume flow , skip enque after deque done when reset() is called */
    m_bSkipEnq = MTRUE;

    CAM_LOGD("[%s] - dev(%d), PD data path(%d)",
             __FUNCTION__,
             m_i4SensorDev,
             m_i4PDDataPath);
}

MBOOL
PDOBufMgrImp::
abort_dequeue_dma()
{
    MBOOL ret = MTRUE;

    //
    switch( m_i4PDDataPath)
    {
        case FEATURE_PDAF_SUPPORT_BNR_PDO:
        case FEATURE_PDAF_SUPPORT_PBN_PDO:
        {
            if(m_pSttPipe)
            {
                if(m_bAbort || m_bPreStop)
                {
                    m_pSttPipe->abortDma(m_rPort,LOG_TAG);
                }
            }
            else
            {
                ret = MFALSE;
                CAM_LOGE("[%s] dev(%d), PD data path(%d), sttPipe is not exist!!",
                         __FUNCTION__,
                         m_i4SensorDev,
                         m_i4PDDataPath);
            }
        }
        break;

        case FEATURE_PDAF_SUPPORT_LEGACY:
        {
            // Unsubscribe IoPipeEventSystem
            if (mspIoPipeEvtHandleDequed != NULL)
            {
                mspIoPipeEvtHandleDequed->unsubscribe();
                mspIoPipeEvtHandleDequed = NULL;
            }

            // release resource
            {
                Mutex::Autolock lock(m_P1DeqWaitLock);

                if( m_vecPureRawBufInfo.size())
                {
                    for(vector<ImgoInfo>::iterator itr = m_vecPureRawBufInfo.begin(); itr !=m_vecPureRawBufInfo.end(); itr++)
                    {
                        if( /* for decreasing handle ref count */
                            ::ion_free(m_i4IonDevFd, itr->mIonBuf_handle))
                        {
                            CAM_LOGE("ion_free failed : ion_info[handle(%d)]", itr->mIonBuf_handle);
                        }
                        ::ion_share_close(m_i4IonDevFd, itr->mIonBuf_share_fd);
                    }

                    m_vecPureRawBufInfo.clear();
                    sem_init( &(m_semP1Deq), 0, 0);
                }
            }

            //
            ::sem_post( &(m_semP1Deq));
        }
        break;

        case FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL:
        {
            if(m_pCamsvSttPipe)
            {
                if(m_bAbort || m_bPreStop)
                {
                    PortID _dq_portID = NSImageio::NSIspio::EPortIndex_CAMSV_IMGO;
                    m_pCamsvSttPipe->abortDma(_dq_portID,LOG_TAG);
                }
            }
            else
            {
                ret = MFALSE;
                CAM_LOGE("[%s] dev(%d), PD data path(%d), camsvSttPipe is not exist!!",
                         __FUNCTION__,
                         m_i4SensorDev,
                         m_i4PDDataPath);
            }
        }
        break;

        default:
            break;

    }

    return ret;

}


/******************************************************************************
* PD Thread P1 Event Control
*******************************************************************************/
MBOOL
PDOBufMgrImp::
deque_pure_raw(StatisticBufInfo &oSttBufInfo)
{
    MBOOL ret = MFALSE;

    CAM_LOGD_IF( m_bDebugEnable, "[%s] +", __FUNCTION__);

    //
    memset( &oSttBufInfo, 0, sizeof(StatisticBufInfo));

    //
    PDO_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "P1DeqWaitSem");
    ::sem_wait( &m_semP1Deq);
    if( m_bDebugEnable)
    {
        int Val;
        ::sem_getvalue( &m_semP1Deq, &Val);
        CAM_LOGD_IF( m_bDebugEnable, "[%s] semP1Deq(%d)", __FUNCTION__, Val);
    }
    PDO_TRACE_FMT_END(TRACE_LEVEL_DEF);

    //
    /*
    //status_t res = mP1DeqWaitCond.waitRelative(mP1DeqWaitLock, (long long int)200000000);
    if(res != 0)
    {
        CAM_LOGE("[%s] wait deque event from ioPipeEvent system TIMEOUT!!", __FUNCTION__);
        return MFALSE;
    }//*/

    //
#if defined(MTK_ION_SUPPORT)
    if(m_i4IonDevFd<0)
    {
        CAM_LOGE("Ion device is not opened");
        ret = MFALSE;
    }
    else
    {
        {
            Mutex::Autolock lock(m_P1DeqWaitLock);

            if(m_vecPureRawBufInfo.size())
            {
                vector<ImgoInfo>::iterator pureRawBufInfo = m_vecPureRawBufInfo.begin();

                //Output
                oSttBufInfo.mMagicNumber      = pureRawBufInfo->mMagicNum;
                oSttBufInfo.mSize             = pureRawBufInfo->mBufSizeInBytes;
                oSttBufInfo.mStride           = pureRawBufInfo->mBufStridesInBytes;
                oSttBufInfo.mImgoSizeW        = pureRawBufInfo->mImgSize.w;
                oSttBufInfo.mImgoSizeH        = pureRawBufInfo->mImgSize.h;
                oSttBufInfo.mImgoBitsPerPixel = pureRawBufInfo->mImgBitsPerPixel;
                oSttBufInfo.mImgoFmt          = pureRawBufInfo->mFmt;
                oSttBufInfo.mImgoBuf_handle   = pureRawBufInfo->mIonBuf_handle;
                oSttBufInfo.mImgoBuf_share_fd = pureRawBufInfo->mIonBuf_share_fd;
                oSttBufInfo.mImgHeapID        = pureRawBufInfo->mImgHeapID;
                oSttBufInfo.mLensPosition     = pureRawBufInfo->mLensPos;
                oSttBufInfo.mTimeStamp        = pureRawBufInfo->mTimeStamp;

                //pop front
                m_vecPureRawBufInfo.erase( m_vecPureRawBufInfo.begin());

                //
                ret = MTRUE;
            }
            else
            {
                if(m_bAbort||m_bPreStop)
                {
                    CAM_LOGD( "[%s] abort from waiting pure raw buffer. PreStop(%d), Abort(%d)",
                              __FUNCTION__,
                              m_bPreStop,
                              m_bAbort);
                }
                else
                {
                    CAM_LOGE( "[%s] should not be happened!! PreStop(%d), Abort(%d)",
                              __FUNCTION__,
                              m_bPreStop,
                              m_bAbort);
                }
            }
        }

        //
        if(ret)
        {
            PDO_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "mmap#%d", oSttBufInfo.mMagicNumber);
            MINTPTR va = (MUINTPTR)::ion_mmap(m_i4IonDevFd, NULL, oSttBufInfo.mSize, PROT_READ, MAP_SHARED, oSttBufInfo.mImgoBuf_share_fd, 0);
            ret = ((va) && ((void *)va!=MAP_FAILED)) ? MTRUE : MFALSE;
            PDO_TRACE_FMT_END(TRACE_LEVEL_DEF);

            if(ret)
            {
                oSttBufInfo.mVa = va;

                //
                CAM_LOGD_IF( m_bDebugEnable,
                             "[%s] frmNum(%d) w(%d) h(%d) stride(0x%x BYTE) share_fd(%d) va(%p) format(%d), sz(0x%x BYTE), bit depth per pixel(%d)",
                             __FUNCTION__,
                             oSttBufInfo.mMagicNumber,
                             oSttBufInfo.mImgoSizeW,
                             oSttBufInfo.mImgoSizeH,
                             oSttBufInfo.mStride,
                             oSttBufInfo.mImgoBuf_share_fd,
                             reinterpret_cast<void *>(va),
                             oSttBufInfo.mImgoFmt,
                             oSttBufInfo.mSize,
                             oSttBufInfo.mImgoBitsPerPixel);
            }
            else
            {
                CAM_LOGW("[%s] ion_mmap fail !! : frmNum(%d) w(%d) h(%d) stride(0x%x BYTE) share_fd(%d) va(%p) format(%d), sz(0x%x BYTE), bit depth per pixel(%d)",
                         __FUNCTION__,
                         oSttBufInfo.mMagicNumber,
                         oSttBufInfo.mImgoSizeW,
                         oSttBufInfo.mImgoSizeH,
                         oSttBufInfo.mStride,
                         oSttBufInfo.mImgoBuf_share_fd,
                         reinterpret_cast<void *>(va),
                         oSttBufInfo.mImgoFmt,
                         oSttBufInfo.mSize,
                         oSttBufInfo.mImgoBitsPerPixel);

            }

        }
    }
#else
    CAM_LOGE("[%s] - : MTK_ION_SUPPORT is not defined. buffer can not be ready", __FUNCTION__);
    ret = MFALSE;
#endif

    CAM_LOGD_IF( m_bDebugEnable, "[%s] ret(%d) -", __FUNCTION__, ret);
    return ret;
}

MBOOL
PDOBufMgrImp::
enque_pure_raw(StatisticBufInfo &iSttBufInfo)
{
    MBOOL ret = MFALSE;

    CAM_LOGD_IF( m_bDebugEnable, "[%s] +", __FUNCTION__);

#if defined(MTK_ION_SUPPORT)
    if(m_i4IonDevFd<0)
    {
        CAM_LOGE("[%s] Ion device is not opened", __FUNCTION__);
    }
    else
    {
        if( /* for decreasing handle ref count */
            ::ion_free(m_i4IonDevFd, iSttBufInfo.mImgoBuf_handle))
        {
            CAM_LOGE("ion_free failed : ion_info[handle(%d)]", iSttBufInfo.mImgoBuf_handle);
        }

        CAM_LOGD_IF( m_bDebugEnable,
                     "[%s] ion_munmap : va(%p) size(0x%x) heapID(%d) share_fd(%d)",
                     __FUNCTION__,
                     reinterpret_cast<void *>(iSttBufInfo.mVa),
                     iSttBufInfo.mSize,
                     iSttBufInfo.mImgHeapID,
                     iSttBufInfo.mImgoBuf_share_fd);

        PDO_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "munmap");
        ::ion_munmap(m_i4IonDevFd, (void *)(iSttBufInfo.mVa), iSttBufInfo.mSize);
        ::ion_share_close(m_i4IonDevFd, iSttBufInfo.mImgoBuf_share_fd);
        PDO_TRACE_FMT_END(TRACE_LEVEL_DEF);

    }
    ret = MTRUE;
#else
    CAM_LOGE("not defined: MTK_ION_SUPPORT");
    ret = MFALSE;
#endif

    CAM_LOGD_IF( m_bDebugEnable, "[%s] ret(%d) -", __FUNCTION__, ret);
    return ret;

}

NSCam::NSIoPipe::IoPipeEventCtrl
PDOBufMgrImp::
onP1Dequed(PDOBufMgrImp * user,
           NSCam::NSIoPipe::NormalPipeDequedEvent & evt)
{
    if(user == NULL)
    {
        CAM_LOGE("[%s] user is NULL", __FUNCTION__);
        evt.setResult((NSCam::NSIoPipe::IoPipeEvent::ResultType)
                      NSCam::NSIoPipe::IoPipeEvent::RESULT_ERROR);
        return NSCam::NSIoPipe::IoPipeEventCtrl::STOP_BROADCASTING;
    }

    //
    MUINT32 magicNum = evt.getMagicNum();

    CAM_LOGD_IF(user->m_bDebugEnable, "[%s] + #%d", __FUNCTION__, magicNum);

    //
    mcuMotorInfo rMotorInfo;

    if(user->m_i4DbgSystraceLevel >= TRACE_LEVEL_DEF)
    {
        CAM_TRACE_FMT_BEGIN("getLensPos");
    }
    CAM_LOGD_IF(user->m_bDebugEnable, "[%s] get lens info from MCUDrv +", __FUNCTION__);
    MCUDrv::getInstance(user->m_i4SensorDev)->getMCUInfo( &rMotorInfo);
    CAM_LOGD_IF(user->m_bDebugEnable, "[%s] get lens info from MCUDrv - Current lens position(%d)", __FUNCTION__, rMotorInfo.u4CurrentPosition);
    if(user->m_i4DbgSystraceLevel >= TRACE_LEVEL_DEF)
    {
        CAM_TRACE_FMT_END();
    }

    user->m_i4LensPosPre = user->m_i4LensPosCur;
    user->m_i4LensPosCur = rMotorInfo.u4CurrentPosition;


    //
    //evt.getRrzoBuffer(); //No use here
    //
    IImageBuffer *imgBuf = evt.getImgoBuffer();

    if(imgBuf)
    {
        IImageBufferHeap *imgBufHeap = imgBuf->getImageBufferHeap();

        if(imgBufHeap)
        {
            //
            MINT64 bufFmt = eIMAGE_DESC_RAW_TYPE_INVALID;
            imgBuf->getImgDesc(eIMAGE_DESC_ID_RAW_TYPE, bufFmt);

            // receive pure raw buffer. post to pd calculation thread.
            if( (ImageDescRawType)(bufFmt)==eIMAGE_DESC_RAW_TYPE_PURE)
            {

                //
                ImgoInfo pureRawBufInfo;

                pureRawBufInfo.mImgHeapID = imgBufHeap->getHeapID();

                //
                if( /* get ion buffer handle from current ion shared fd and handle ref count is increased*/
                    ::ion_import(user->m_i4IonDevFd, pureRawBufInfo.mImgHeapID, &(pureRawBufInfo.mIonBuf_handle))==0)
                {

                    if(::ion_share(user->m_i4IonDevFd, pureRawBufInfo.mIonBuf_handle, &(pureRawBufInfo.mIonBuf_share_fd))==0)
                    {
                        pureRawBufInfo.mImgSize           = imgBuf->getImgSize();
                        pureRawBufInfo.mImgBitsPerPixel   = imgBuf->getImgBitsPerPixel();
                        pureRawBufInfo.mPlaneBitsPerPixel = imgBuf->getPlaneBitsPerPixel(0);
                        pureRawBufInfo.mPlaneCount        = imgBuf->getPlaneCount();
                        pureRawBufInfo.mBufSizeInBytes    = imgBufHeap->getBufSizeInBytes(0);
                        pureRawBufInfo.mBufStridesInBytes = imgBufHeap->getBufStridesInBytes(0);
                        pureRawBufInfo.mMagicNum          = magicNum;
                        pureRawBufInfo.mSensorIdx         = evt.getSensorIndex();
                        pureRawBufInfo.mFmt               = (ImageDescRawType)(bufFmt);
                        pureRawBufInfo.mLensPos           = user->m_i4LensPosPre;
                        pureRawBufInfo.mTimeStamp         = _GET_TIME_STAMP_NS_();

                        {
                            //
                            Mutex::Autolock lock(user->m_P1DeqWaitLock);

                            if( /* 1. queue one pure raw information only : remove all elements */
                                user->m_vecPureRawBufInfo.size())
                            {
                                for(vector<ImgoInfo>::iterator itr = user->m_vecPureRawBufInfo.begin(); itr !=user->m_vecPureRawBufInfo.end(); itr++)
                                {
                                    if( /* for decreasing handle ref count */
                                        ::ion_free(user->m_i4IonDevFd, itr->mIonBuf_handle))
                                    {
                                        CAM_LOGE("ion_free failed : ion_info[handle(%d)]", itr->mIonBuf_handle);
                                    }
                                    ::ion_share_close(user->m_i4IonDevFd, itr->mIonBuf_share_fd);
                                }

                                user->m_vecPureRawBufInfo.clear();
                                sem_init( &(user->m_semP1Deq), 0, 0);
                            }


                            // 2. queue pure raw information
                            user->m_vecPureRawBufInfo.push_back(pureRawBufInfo);

                            //
                            CAM_LOGD("[%s] receive deque event from ioPipeEvent system : User(%p), Queue size(%d), Current pure raw buffer information[frmNum(%d) share_fd(%d) format(%d) plane(%d) lens(%d)], ION_info[handle(%d) shared_fd(%d)]",
                                     __FUNCTION__,
                                     user,
                                     user->m_vecPureRawBufInfo.size(),
                                     user->m_vecPureRawBufInfo.back().mMagicNum,
                                     user->m_vecPureRawBufInfo.back().mImgHeapID,
                                     user->m_vecPureRawBufInfo.back().mFmt,
                                     user->m_vecPureRawBufInfo.back().mPlaneCount,
                                     user->m_vecPureRawBufInfo.back().mLensPos,
                                     user->m_vecPureRawBufInfo.back().mIonBuf_handle,
                                     user->m_vecPureRawBufInfo.back().mIonBuf_share_fd);
                        }

                        //
                        if(user->m_i4DbgSystraceLevel >= TRACE_LEVEL_DEF)
                        {
                            CAM_TRACE_FMT_BEGIN("postSem#%d", magicNum);
                        }
                        //user->mP1DeqWaitCond.broadcast();
                        ::sem_post( &(user->m_semP1Deq));
                        if(user->m_i4DbgSystraceLevel >= TRACE_LEVEL_DEF)
                        {
                            CAM_TRACE_FMT_END();
                        }
                    }
                    else
                    {
                        CAM_LOGE("ion_share failed : pure raw buffer information[share_fd(%d)] ion_info[handle(%d) shared_fd(%d)]",
                                 user->m_vecPureRawBufInfo.back().mImgHeapID,
                                 pureRawBufInfo.mIonBuf_handle,
                                 user->m_vecPureRawBufInfo.back().mIonBuf_share_fd);
                    }


                }
                else
                {
                    CAM_LOGE("ion_import fail : handle(%d) shared fd(%d)",
                             pureRawBufInfo.mIonBuf_handle,
                             user->m_vecPureRawBufInfo.back().mImgHeapID);
                }
            }
            else
            {
                CAM_LOGD_IF( user->m_bDebugEnable,
                             "[%s] IMGO is not pure raw. (%d)",
                             __FUNCTION__,
                             bufFmt);
            }
        }
        else
        {
            CAM_LOGE("%s imgBufHeap is NULL", __FUNCTION__);
        }
    }
    else
    {
        CAM_LOGE("%s imgBuf is NULL", __FUNCTION__);
    }


    CAM_LOGD_IF(user->m_bDebugEnable, "[%s] -", __FUNCTION__);
    return NSCam::NSIoPipe::IoPipeEventCtrl::OK;
}

#endif
