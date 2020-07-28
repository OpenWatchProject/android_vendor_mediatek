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

#if (CAM3_3A_ISP_40_EN)
#include "../common/hal3a/v1.0/buf_mgr/IBufMgr.h"
#else
#include "IBufMgr.h"
#endif

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
#include "lens_drv.h"

//
#include <dbg_aaa_param.h>
#include <dbg_af_param.h>
#include <camera_custom_nvram.h>
#include <aaa/af_param.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_mgr_if.h>
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
#define _GET_TIME_STAMP_US_() ({\
        MINT64 _ts = 0;\
        struct timespec t;\
        t.tv_sec = t.tv_nsec = 0;\
        clock_gettime(CLOCK_MONOTONIC, &t);\
        _ts = ((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000;\
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
        MINT32            mHeapID;
        MINT32            mLensPos;

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
            , mHeapID(0)
            , mLensPos(-1)
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
    virtual MVOID reset() {};

protected:
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
    Hal3ASttCtrl*          m_p3ASttCtrl;
    MBOOL                  m_bPreStop;
    //Condition              mP1DeqWaitCond;
    sem_t                  m_semP1Deq;
    Mutex                  m_P1DeqWaitLock;
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
#if (CAM3_3A_ISP_50_EN)
PDOBufMgr*
PDOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, STT_CFG_INFO_T const sttInfo __unused)
{
    PDOBufMgrImp* pObj = new PDOBufMgrImp(i4SensorDev,i4SensorIdx);
    return pObj;
}
#else
PDOBufMgr*
PDOBufMgr::
createInstance(MINT32 const i4SensorDev, MINT32 const i4SensorIdx)
{
    PDOBufMgrImp* pObj = new PDOBufMgrImp(i4SensorDev,i4SensorIdx);
    return pObj;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
PDOBufMgrImp::
destroyInstance()
{
    delete this;
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
    , m_bPreStop(MFALSE)
    , m_i4IonDevFd(-1)
    , m_i4LensPosCur(-1)
    , m_i4LensPosPre(-1)
{
    //
    m_vecPureRawBufInfo.clear();
    m_vecPureRawBufInfo.reserve(4);

    //
    m_bDebugEnable       = property_get_int32("vendor.debug.pdo_mgr.enable", 0);
    m_bDumpBufEnable     = property_get_int32("vendor.pd.dump.enable", 0);
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
            m_pCamsvSttPipe = ICamsvStatisticPipe::createInstance(m_i4SensorIdx, LOG_TAG, EPIPE_CAMSV_FEATURE_PDAF);
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
    // release resource
    m_vecPureRawBufInfo.clear();

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
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
PDOBufMgrImp::
dequeueHwBuf()
{
    MINT32 ret = -1;

    CAM_LOGD_IF(m_bDebugEnable,"[%s] dev(%d) +", __FUNCTION__, m_i4SensorDev);

    //
    if(m_bAbort || m_bPreStop)
    {
        CAM_LOGD_IF(m_bDebugEnable,"[%s] dev(%d) abort(%d) preStop(%d) -", __FUNCTION__, m_i4SensorDev, m_bAbort, m_bPreStop);
        return ret;
    }

    /*********************************************
     * deque HW buffer from driver.(blocking code)
     *********************************************/
    PDO_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "PD data deque %d", m_i4PDDataPath);
    MBOOL bDequed = MFALSE;
    MBOOL bBufRdy = MFALSE;
    QBufInfo    rDQBuf;
    StatisticBufInfo rHwBuf;
    switch( m_i4PDDataPath)
    {
        case FEATURE_PDAF_SUPPORT_BNR_PDO:
        case FEATURE_PDAF_SUPPORT_PBN_PDO:
        {
            if(m_pSttPipe)
            {
                bDequed = m_pSttPipe->deque( m_rPort, rDQBuf, ESTT_CacheInvalidByRange);

                //
                if( bDequed && rDQBuf.mvOut.size())
                {
                    //
                    mcuMotorInfo rMotorInfo;
                    LensDrv::getInstance()->getMCUInfo( &rMotorInfo, m_i4SensorDev);

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
                    bBufRdy                 = (m_bAbort||m_bPreStop) ? MFALSE : MTRUE;
                }
            }
        }
        break;

        case FEATURE_PDAF_SUPPORT_LEGACY:
        {
            if(m_i4IonDevFd)
            {
                bDequed = deque_pure_raw(rHwBuf);

                if(rHwBuf.mVa && rHwBuf.mSize)
                {
                    bBufRdy = (m_bAbort||m_bPreStop) ? MFALSE : MTRUE;

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
                    mcuMotorInfo rMotorInfo;
                    LensDrv::getInstance()->getMCUInfo( &rMotorInfo, m_i4SensorDev);

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
                    bBufRdy                 = (m_bAbort||m_bPreStop) ? MFALSE : MTRUE;
                }
            }
        }
        break;

        default:
            break;

    }
    PDO_TRACE_FMT_END(TRACE_LEVEL_DEF);

    //
    if( bDequed && bBufRdy)
    {
        //
        CAM_LOGD_IF( m_bDebugEnable,
                     "[%s] dev(%d), data path(%d), pdo port index(%d), HW Buffer info : magic number(%d)/heapID(%d)/va(0x%p)/size(0x%x)/stride(0x%x BYTE)/Lens(%d)",
                     __FUNCTION__,
                     m_i4SensorDev,
                     m_i4PDDataPath,
                     m_rPort.index,
                     rHwBuf.mMagicNumber,
                     rHwBuf.mImgHeapID,
                     (void*)(rHwBuf.mVa),
                     rHwBuf.mSize,
                     rHwBuf.mStride,
                     rHwBuf.mLensPosition);

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
                        (unsigned long long)(_GET_TIME_STAMP_US_()),
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
        if( !m_bAbort && !m_bPreStop)
        {
            PDO_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "process buf#%d", rHwBuf.mMagicNumber);
            CAM_LOGD_IF(m_bDebugEnable, "pass PD buffer to AF manager(%d) +", rHwBuf.mMagicNumber);
            IAfMgr::getInstance().passPDBuffer(m_i4SensorDev, reinterpret_cast<MVOID *>(&rHwBuf));
            CAM_LOGD_IF(m_bDebugEnable, "pass PD buffer to AF manager(%d) -", rHwBuf.mMagicNumber);
            PDO_TRACE_FMT_END(TRACE_LEVEL_DBG);

            //
            ret = MTRUE;
        }


        /*********************************************
         * enque HW buffer back driver
         *********************************************/
        MBOOL bEnqueDn = MFALSE;
        PDO_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "PD data enque %d", m_i4PDDataPath);
        CAM_LOGD_IF(m_bDebugEnable, "enque Hw buffer back driver (%d) +", rHwBuf.mMagicNumber);
        switch( m_i4PDDataPath)
        {
            case FEATURE_PDAF_SUPPORT_BNR_PDO:
            case FEATURE_PDAF_SUPPORT_PBN_PDO:
            {
                if(m_pSttPipe)
                {
                    bEnqueDn = m_pSttPipe->enque(rDQBuf);
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
                if(m_pCamsvSttPipe)
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
        CAM_LOGD_IF(m_bDebugEnable, "enque Hw buffer back driver (%d), EnqueDn(%d) -", rHwBuf.mMagicNumber, bEnqueDn);
        PDO_TRACE_FMT_END(TRACE_LEVEL_DEF);
    }
    else
    {
        if( !m_bAbort && !m_bPreStop)
        {
            CAM_LOGE("[%s] dev(%d), data path(%d), Deque HW buffer fail : bDequed(%d) bBufRdy(%d) SttPipe(%p) CamsvSttPipe(%p) ionDevFD(%d)",
                     __FUNCTION__,
                     m_i4SensorDev,
                     m_i4PDDataPath,
                     bDequed,
                     bBufRdy,
                     m_pSttPipe,
                     m_pCamsvSttPipe,
                     m_i4IonDevFd);
        }
        ret = -1;
    }


    //
    CAM_LOGD_IF(m_bDebugEnable,"[%s] dev(%d) ret(%d) abort(%d) preStop(%d) -", __FUNCTION__, m_i4SensorDev, ret, m_bAbort, m_bPreStop);
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
    CAM_LOGD("[%s] dev(%d), PD data path(%d)",
             __FUNCTION__,
             m_i4SensorDev,
             m_i4PDDataPath);

    //
    m_bAbort = MTRUE;

    //
    switch( m_i4PDDataPath)
    {
        case FEATURE_PDAF_SUPPORT_BNR_PDO:
        case FEATURE_PDAF_SUPPORT_PBN_PDO:
        {
            if(m_pSttPipe)
            {
                m_pSttPipe->abortDma(m_rPort,LOG_TAG);
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
            ::sem_post( &(m_semP1Deq));
        }
        break;

        case FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL:
        {
            if(m_pCamsvSttPipe)
            {
                PortID _dq_portID = NSImageio::NSIspio::EPortIndex_CAMSV_IMGO;
                m_pCamsvSttPipe->abortDma(_dq_portID,LOG_TAG);
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
    CAM_LOGD("[%s] -", __FUNCTION__);
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
    else if(m_bAbort)
    {
        CAM_LOGD_IF( m_bDebugEnable, "[%s] abort!!", __FUNCTION__);
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
                oSttBufInfo.mImgHeapID        = pureRawBufInfo->mHeapID;
                oSttBufInfo.mLensPosition     = pureRawBufInfo->mLensPos;

                //pop front
                m_vecPureRawBufInfo.erase( m_vecPureRawBufInfo.begin());

                //
                ret = MTRUE;
            }
        }

        //
        if(ret)
        {
            PDO_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "mmap#%d", oSttBufInfo.mMagicNumber);
            MINTPTR va = (MUINTPTR)::ion_mmap(m_i4IonDevFd, NULL, oSttBufInfo.mSize, PROT_READ, MAP_SHARED, oSttBufInfo.mImgHeapID, 0);
            PDO_TRACE_FMT_END(TRACE_LEVEL_DEF);
            oSttBufInfo.mVa = va;

            CAM_LOGD_IF( m_bDebugEnable,
                         "[%s] frmNum(%d) w(%d) h(%d) stride(0x%x BYTE) heapID(%d) va(%p) format(%d), sz(0x%x BYTE), bit depth per pixel(%d)",
                         __FUNCTION__,
                         oSttBufInfo.mMagicNumber,
                         oSttBufInfo.mImgoSizeW,
                         oSttBufInfo.mImgoSizeH,
                         oSttBufInfo.mStride,
                         oSttBufInfo.mImgHeapID,
                         reinterpret_cast<void *>(va),
                         oSttBufInfo.mImgoFmt,
                         oSttBufInfo.mSize,
                         oSttBufInfo.mImgoBitsPerPixel);

            //
            ret = va ? MTRUE : MFALSE;
        }
    }

    CAM_LOGD_IF( m_bDebugEnable, "[%s] ret(%d) -", __FUNCTION__, ret);
    return ret;
#else
    CAM_LOGE("[%s] - : MTK_ION_SUPPORT is not defined. buffer can not be ready", __FUNCTION__);
    return MFALSE;
#endif
}

MBOOL
PDOBufMgrImp::
enque_pure_raw(StatisticBufInfo &iSttBufInfo)
{
#if defined(MTK_ION_SUPPORT)
    if(m_i4IonDevFd<0)
    {
        CAM_LOGE("[%s] Ion device is not opened", __FUNCTION__);
    }
    else
    {
        CAM_LOGD_IF( m_bDebugEnable,
                     "[%s] ion_munmap : va(%p) size(0x%x)",
                     __FUNCTION__,
                     reinterpret_cast<void *>(iSttBufInfo.mVa),
                     iSttBufInfo.mSize);

        PDO_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "munmap");
        ::ion_munmap(m_i4IonDevFd, (void *)(iSttBufInfo.mVa), iSttBufInfo.mSize);
        PDO_TRACE_FMT_END(TRACE_LEVEL_DEF);
    }
    return MTRUE;
#else
    CAM_LOGE("not defined: MTK_ION_SUPPORT");
    return MFALSE;
#endif
}

NSCam::NSIoPipe::IoPipeEventCtrl
PDOBufMgrImp::
onP1Dequed(PDOBufMgrImp * user,
           NSCam::NSIoPipe::NormalPipeDequedEvent & evt)
{
    if(user == NULL)
    {
        CAM_LOGE("%s user is NULL", __FUNCTION__);
        evt.setResult((NSCam::NSIoPipe::IoPipeEvent::ResultType)
                      NSCam::NSIoPipe::IoPipeEvent::RESULT_ERROR);
        return NSCam::NSIoPipe::IoPipeEventCtrl::STOP_BROADCASTING;
    }

    //
    mcuMotorInfo rMotorInfo;
    LensDrv::getInstance()->getMCUInfo( &rMotorInfo, user->m_i4SensorDev);

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
            MUINT32 magicNum = evt.getMagicNum();

            //
            MINT64 bufFmt = eIMAGE_DESC_RAW_TYPE_INVALID;
            imgBuf->getImgDesc(eIMAGE_DESC_ID_RAW_TYPE, bufFmt);

            //
            CAM_LOGD_IF( user->m_bDebugEnable,
                         "[%s] receive deque event from ioPipeEvent system, user(%p), frmNum(%d)",
                         __FUNCTION__,
                         user,
                         magicNum);


            // receive pure raw buffer. post to pd calculation thread.
            if( (ImageDescRawType)(bufFmt)==eIMAGE_DESC_RAW_TYPE_PURE)
            {
                //
                ImgoInfo pureRawBufInfo;
                pureRawBufInfo.mImgSize           = imgBuf->getImgSize();
                pureRawBufInfo.mImgBitsPerPixel   = imgBuf->getImgBitsPerPixel();
                pureRawBufInfo.mPlaneBitsPerPixel = imgBuf->getPlaneBitsPerPixel(0);
                pureRawBufInfo.mPlaneCount        = imgBuf->getPlaneCount();
                pureRawBufInfo.mBufSizeInBytes    = imgBufHeap->getBufSizeInBytes(0);
                pureRawBufInfo.mBufStridesInBytes = imgBufHeap->getBufStridesInBytes(0);
                pureRawBufInfo.mHeapID            = imgBufHeap->getHeapID();
                pureRawBufInfo.mMagicNum          = magicNum;
                pureRawBufInfo.mSensorIdx         = evt.getSensorIndex();
                pureRawBufInfo.mFmt               = (ImageDescRawType)(bufFmt);
                pureRawBufInfo.mLensPos           = user->m_i4LensPosPre;

                {
                    Mutex::Autolock lock(user->m_P1DeqWaitLock);

                    user->m_vecPureRawBufInfo.push_back(pureRawBufInfo);

                    //
                    /* CAM_LOGD_IF( user->m_bDebugEnable,
                                 "[%s] size of pure raw buffer information queue(%d). current pure raw buffer information[frmNum(%d) heapID(%d) format(%d) plane(%d) lens(%d)]",
                                 __FUNCTION__,
                                 user->m_vecPureRawBufInfo.size(),
                                 user->m_vecPureRawBufInfo.back().mMagicNum,
                                 user->m_vecPureRawBufInfo.back().mHeapID,
                                 user->m_vecPureRawBufInfo.back().mFmt,
                                 user->m_vecPureRawBufInfo.back().mPlaneCount,
                                 user->m_vecPureRawBufInfo.back().mLensPos); */
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
    return NSCam::NSIoPipe::IoPipeEventCtrl::OK;
}

#endif
