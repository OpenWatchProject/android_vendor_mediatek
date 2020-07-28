/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#define LOG_TAG "MfllCore/Strategy"

#include "MfllStrategy.h"

// MFNR public headers
#include <MfllProperty.h>
#include <MfllLog.h>

// MTKCAM
#include <mtkcam/utils/hw/GyroCollector.h>
#include <mtkcam/utils/hw/HwInfoHelper.h> // NSCamHw::HwInfoHelper
#include <mtkcam/drv/IHalSensor.h> // NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE
#if MTK_CAM_NEW_NVRAM_SUPPORT
#include "MfllNvram.h"
#endif
#include <camera_custom_nvram.h>
#include <custom/feature/mfnr/camera_custom_mfll.h> // CUST_MFLL_ENABLE_CONTENT_AWARE_AIS

// AOPS
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
#include <cutils/compiler.h>

// STL
#include <chrono>
#include <fstream>

using android::sp;
using namespace mfll;

//-----------------------------------------------------------------------------
IMfllStrategy* IMfllStrategy::createInstance()
{
    return (IMfllStrategy*)new MfllStrategy;
}
//-----------------------------------------------------------------------------
void IMfllStrategy::destroyInstance()
{
    decStrong((void*)this);
}
//-----------------------------------------------------------------------------
// MfllStrategy implementation
//-----------------------------------------------------------------------------
MfllStrategy::MfllStrategy()
: m_mtkAisInited(false)
, m_nvramChunk(NULL)
, m_captureFrameNum(MFLL_CAPTURE_FRAME)
, m_blendFrameNum(MFLL_BLEND_FRAME)
{
    mfllTraceCall();

    mfllTraceBegin("create MTKAis instance");
    m_mtkAis = std::shared_ptr<MTKAis>(
            MTKAis::createInstance(DRV_AIS_OBJ_SW),
            [this](auto *obj)->void {
                mfllTraceBegin("del_MTKAis");
                if (CC_LIKELY(obj)) {
                    // check inited or not.
                    if (CC_LIKELY( this->m_mtkAisInited.load() )) {
                        obj->AisReset();
                    }
                    obj->destroyInstance();
                }
                mfllTraceEnd();
            }
    );
    mfllTraceEnd();

    if (m_mtkAis.get() == NULL) {
        mfllLogE("create MTKAis failed");
        return;
    }

    AIS_INIT_PARAM _initParam;
    _initParam.eProcID = AIS_PROC2;
    mfllTraceBegin("init MTKAis");

    // If AisInit return S_AIS_OK, we MUST invoke MTKAis::AisReset for avoiding
    // memory leakage, but if AisInit returns failed, we CANNOT invoke
    // MTKAis::AisReset, or an exception will happen.
    auto result = m_mtkAis->AisInit(&_initParam, NULL);
    mfllTraceEnd();
    if (CC_UNLIKELY( result != S_AIS_OK )) {
        mfllLogE("init MTKAis failed with code %#x", result);
        m_mtkAis = nullptr;
        return;
    }
    else {
        m_mtkAisInited.store(true);
    }
}
//-----------------------------------------------------------------------------
MfllStrategy::~MfllStrategy()
{
    // wait future done (timeout 1 second)
    if (CC_LIKELY( m_futureWorkingBuffer.valid() )) {
        try {
            auto _status = m_futureWorkingBuffer.wait_for(std::chrono::seconds(1));
            if (CC_UNLIKELY( _status != std::future_status::ready )) {
                mfllLogE("wait future status is not ready");
            }
        }
        catch (std::exception&) {
            mfllLogE("m_futureWorkingBuffer::wait_for throws exception");
        }
    }

    m_mtkAis = nullptr;
}
//-----------------------------------------------------------------------------
enum MfllErr MfllStrategy::init(sp<IMfllNvram> &nvramProvider)
{
    mfllTraceCall();

    if (nvramProvider.get() == NULL) {
        mfllLogE("%s: nvram provider is NULL", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    if (m_mtkAis.get() == NULL) {
        mfllLogE("%s: MTKAis instance is NULL", __FUNCTION__);
        return MfllErr_NullPointer;
    }

    /* read NVRAM */
    size_t chunkSize = 0;
    const char *pChunk = nvramProvider->getChunk(&chunkSize);
    if (pChunk == NULL) {
        mfllLogE("%s: read NVRAM failed", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }
    m_nvramChunk = pChunk;
    char *pMutableChunk = const_cast<char*>(pChunk);

    /* reading NVRAM */
    NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_MFLL_STRUCT*>(pMutableChunk);
    m_captureFrameNum = pNvram->capture_frame_number;
    m_blendFrameNum = pNvram->blend_frame_number;
    m_isFullSizeMc =  pNvram->full_size_mc;

    /* ask AIS working buffer size */
    {
        mfllTraceName("create_thd_ais_buf");
        AIS_GET_PROC2_INFO _getParam = {0};
        auto _result = m_mtkAis->AisFeatureCtrl(
                AIS_FTCTRL_GET_PROC2_INFO,
                (void*)&_getParam,
                nullptr);
        if (CC_UNLIKELY( _result != S_AIS_OK )) {
            mfllLogE("%s: AisFeatureCtrl GET_PROC2_INFO failed", __FUNCTION__);
            return MfllErr_UnexpectedError;
        }

        /* async prepare working buffer */
        m_futureWorkingBuffer = std::async( std::launch::async, [this](MUINT32 __bufSize) -> int{
            MFLL_THREAD_NAME("strategy_alloc_buf");
            mfllTraceName("allocate_ais_wk_buf");

            /* create working buffer */
            m_imgWorkingBuf = IMfllImageBuffer::createInstance();
            if (CC_UNLIKELY( m_imgWorkingBuf.get() == nullptr )) {
                mfllLogE("create AIS working buffer instance failed");
                return -1;
            }

            /* align working buffer size */
            __bufSize = (__bufSize + 1) >> 1;
            m_imgWorkingBuf->setResolution(__bufSize, 2);
            m_imgWorkingBuf->setImageFormat(ImageFormat_Y8);
            mfllLogD3("init AIS working buffer with size %u", __bufSize << 1);

            auto r = m_imgWorkingBuf->initBuffer();
            if (CC_UNLIKELY( r != MfllErr_Ok )) {
                mfllLogE("init AIS working buffer failed");
                return -1;
            }
            return 0; // OK!
        }, _getParam.u4WorkingBufSize );
    }


    return MfllErr_Ok;
}
//-----------------------------------------------------------------------------
enum MfllErr MfllStrategy::queryStrategy(
        const MfllStrategyConfig_t &cfg,
        MfllStrategyConfig_t *out)
{
    mfllTraceCall();

    if (m_mtkAis.get() == NULL) {
        mfllLogE("%s: MTKAis instance doesn't exist", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }
    if (m_nvramChunk == NULL) {
        mfllLogE("%s: NVRAM chunk is NULL", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }

    /* default AISConfig */
    MfllStrategyConfig_t finalCfg = cfg;

    finalCfg.frameCapture   = m_captureFrameNum;
    finalCfg.frameBlend     = m_blendFrameNum;
    finalCfg.isFullSizeMc   = m_isFullSizeMc;

    // check content-award ais
    auto adbEnableGyro = MfllProperty::readProperty(
        mfll::Property_Gyro, MFLL_CONTENT_AWARE_AIS_ENABLE);

    // check customer folder
    bool bCustomEnable = [&cfg](){
        if (CC_UNLIKELY( cfg.sensor_id < 0 )) {
            mfllLogE("sensor id < 0, something wrong, disable content-aware AIS");
            return false;
        }

        bool b = !! (CUST_MFLL_ENABLE_CONTENT_AWARE_AIS & (1 << cfg.sensor_id));
        if (CC_UNLIKELY( !b )) {
            mfllLogI("disable Content-Aware AIS by custom");
        }
        return b;
    }();

    //
    bool bEnableContentAwareAis = adbEnableGyro && (cfg.isAis != 0) && bCustomEnable;

    if (CC_LIKELY( bEnableContentAwareAis )) {
        mfllLogD("enable content-aware AIS");
    }
    else {
        mfllLogD("disabled content-aware AIS");
    }

    //-------------------------------------------------------------------------
    // query frame number
    //-------------------------------------------------------------------------
    if (bEnableContentAwareAis) { // AIS only
    // {{{
        mfllTraceName("procssing_ais");
        mfllAutoLog("processing_ais");

        /* wait working buffer ready */
        if (CC_LIKELY( m_futureWorkingBuffer.valid() )) {
            mfllTraceName("wait_ais_wk_buf");
            mfllAutoLog("wait_ais_wk_buf");

            if (CC_UNLIKELY( m_futureWorkingBuffer.get() != 0 )) {
                mfllLogE("allocate working buffer failed");
                return MfllErr_UnexpectedError;
            }

            if (CC_UNLIKELY( m_imgWorkingBuf.get() == nullptr )) {
                mfllLogE("m_imgWorkingBuf is nullptr");
                return MfllErr_UnexpectedError;
            }
        }
        else {
            mfllLogE("allocate working buffer thread is invalid");
            return MfllErr_UnexpectedError;
        }

        /* prepare information to MTKAis */
        // {{{
        {
            mfllTraceName("AIS_SET_PROC2_INFO");
            mfllLogD3("AIS_SET_PROC2_INFO");

            /* NVRAM block */
            const NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvram =
                reinterpret_cast<decltype(pNvram)>(m_nvramChunk);

            AIS_SET_PROC2_INFO      param;
            mfllLogD3("original mfll_iso_th=%d", pNvram->mfll_iso_th);

            param.u2IsoLvl1         = pNvram->iso_level1;
            param.u2IsoLvl2         = pNvram->iso_level2;
            param.u2IsoLvl3         = pNvram->iso_level3;
            param.u2IsoLvl4         = pNvram->iso_level4;
            param.u2IsoLvl5         = pNvram->iso_level5;
            /* u1FrmNum represents frame number for blending */
            param.u1FrmNum1         = pNvram->frame_num1;
            param.u1FrmNum2         = pNvram->frame_num2;
            param.u1FrmNum3         = pNvram->frame_num3;
            param.u1FrmNum4         = pNvram->frame_num4;
            param.u1FrmNum5         = pNvram->frame_num5;
            param.u1FrmNum6         = pNvram->frame_num6;
            /* u1SrcNum represents frame number for capture */
            param.u1SrcNum1         = pNvram->frame_num1;
            param.u1SrcNum2         = pNvram->frame_num2;
            param.u1SrcNum3         = pNvram->frame_num3;
            param.u1SrcNum4         = pNvram->frame_num4;
            param.u1SrcNum5         = pNvram->frame_num5;
            param.u1SrcNum6         = pNvram->frame_num6;
            /* gyro */
            param.fRsc              = pNvram->fRsc;
            param.fBiasX            = pNvram->fBiasX;
            param.fBiasY            = pNvram->fBiasY;
            param.fBiasZ            = pNvram->fBiasZ;
            param.fTofst            = pNvram->fTofst;
            param.u4MotionBase      = static_cast<MUINT32>(pNvram->uMotionBase);
            param.u4MVThLow         = static_cast<MUINT32>(pNvram->uMVThLow);
            param.u4MVThHigh        = static_cast<MUINT32>(pNvram->uMVThHigh);
            param.u4MinExp          = pNvram->u4MinExpUs;
            param.u4MaxIso          = pNvram->u4MaxIso;
            /* working buffer */
            param.pu1WorkingBuf     = static_cast<MUINT8*>(m_imgWorkingBuf->getVa());
            param.u4WorkingBufSize  = m_imgWorkingBuf->getRealBufferSize();

            /* debug info */
            // {{{
            mfllLogD3("%s: iso_level1    = %hu",  __FUNCTION__, param.u2IsoLvl1);
            mfllLogD3("%s: iso_level2    = %hu",  __FUNCTION__, param.u2IsoLvl2);
            mfllLogD3("%s: iso_level3    = %hu",  __FUNCTION__, param.u2IsoLvl3);
            mfllLogD3("%s: iso_level4    = %hu",  __FUNCTION__, param.u2IsoLvl4);
            mfllLogD3("%s: iso_level5    = %hu",  __FUNCTION__, param.u2IsoLvl5);
            //
            mfllLogD3("%s: u1FrmNum1     = %hhu", __FUNCTION__, param.u1FrmNum1);
            mfllLogD3("%s: u1FrmNum2     = %hhu", __FUNCTION__, param.u1FrmNum2);
            mfllLogD3("%s: u1FrmNum3     = %hhu", __FUNCTION__, param.u1FrmNum3);
            mfllLogD3("%s: u1FrmNum4     = %hhu", __FUNCTION__, param.u1FrmNum4);
            mfllLogD3("%s: u1FrmNum5     = %hhu", __FUNCTION__, param.u1FrmNum5);
            mfllLogD3("%s: u1FrmNum6     = %hhu", __FUNCTION__, param.u1FrmNum6);
            //
            mfllLogD3("%s: u1SrcNum1     = %hhu", __FUNCTION__, param.u1SrcNum1);
            mfllLogD3("%s: u1SrcNum2     = %hhu", __FUNCTION__, param.u1SrcNum2);
            mfllLogD3("%s: u1SrcNum3     = %hhu", __FUNCTION__, param.u1SrcNum3);
            mfllLogD3("%s: u1SrcNum4     = %hhu", __FUNCTION__, param.u1SrcNum4);
            mfllLogD3("%s: u1SrcNum5     = %hhu", __FUNCTION__, param.u1SrcNum5);
            mfllLogD3("%s: u1SrcNum6     = %hhu", __FUNCTION__, param.u1SrcNum6);
            //
            mfllLogD3("%s: fRsc          = %f",   __FUNCTION__, param.fRsc);
            mfllLogD3("%s: fBiasX        = %f",   __FUNCTION__, param.fBiasX);
            mfllLogD3("%s: fBiasY        = %f",   __FUNCTION__, param.fBiasY);
            mfllLogD3("%s: fBiasZ        = %f",   __FUNCTION__, param.fBiasZ);
            mfllLogD3("%s: fTofst        = %f",   __FUNCTION__, param.fTofst);
            mfllLogD3("%s: u4MotionBase  = %u",   __FUNCTION__, param.u4MotionBase);
            mfllLogD3("%s: u4MVThLow     = %u",   __FUNCTION__, param.u4MVThLow);
            mfllLogD3("%s: u4MVThHigh    = %u",   __FUNCTION__, param.u4MVThHigh);
            mfllLogD3("%s: u4MinExp      = %u",   __FUNCTION__, param.u4MinExp);
            mfllLogD3("%s: u4MaxIso      = %u",   __FUNCTION__, param.u4MaxIso);
            // }}}

            auto result = m_mtkAis->AisFeatureCtrl(
                    AIS_FTCTRL_SET_PROC2_INFO,
                    (void*)&param,
                    NULL);

            if (result != S_AIS_OK) {
                mfllLogE("%s: AisFeatureCtrl failed with code %#x", __FUNCTION__, result);
            }
        }
        // }}}

        /* collect gyro info */
        mfllTraceBegin("collect_gyro_info");
        int64_t currentTs = android::uptimeMillis() * 1000000L; // ms-->ns
        constexpr static int64_t const A_SECOND = 1000000000L;

        // retrieve gyro info
        auto gyroInfo = NSCam::Utils::GyroCollector::getData(
                currentTs - A_SECOND, // i
                currentTs
                );


        mfllLogD3("gyro info size= %zu", gyroInfo.size());

        if (CC_UNLIKELY(MfllProperty::readProperty(Property_DumpGyro) > 0)) {
            static int serial_ = 0;
            char filename[512];
            snprintf(
                    filename,
                    sizeof(filename),
                    "%s%04d_%s.txt",
                    "/sdcard/DCIM/Camera/MFLL",
                    serial_++,
                    "gyro");
            std::ofstream wt(filename, std::ofstream::out);
            for (size_t i = 0; i < gyroInfo.size(); i++) {
                const auto& itr = gyroInfo[i];
                wt  << itr.x << ", "
                    << itr.y << ", "
                    << itr.z << ", "
                    << itr.timestamp
                    << std::endl;
            }
        }

        AIS_PROC2_PARA_IN   paramIn;
        AIS_PROC2_PARA_OUT  paramOut;

        constexpr bool bGyroNeedMemcpy =
            sizeof(AIS_GYRO_INFO) != sizeof(NSCam::Utils::GyroCollector::GyroInfo);

        std::unique_ptr<AIS_GYRO_INFO> gyroDataChunk_ = [bGyroNeedMemcpy, &gyroInfo]{
            return CC_UNLIKELY(bGyroNeedMemcpy)
                ? std::unique_ptr<AIS_GYRO_INFO>(new AIS_GYRO_INFO[gyroInfo.size()])
                : nullptr
                ;
        }();

        // gyro information
        paramIn.u4GyroNum = gyroInfo.size();
        paramIn.u4GyroIntervalMS = static_cast<MUINT32>(
                NSCam::Utils::GyroCollector::INTERVAL);

        // copy gyro info if necessary
        if (CC_UNLIKELY(bGyroNeedMemcpy)) {
            mfllLogW("%s: AIS_GYRO_INFO doesn't equals to GyroInfo, need copy. " \
                     "Please consider using the same structure w/o wasting copy " \
                     "operations which are expensive.",
                     __FUNCTION__);

            for (size_t i = 0; i < gyroInfo.size(); i++) {
                if (CC_UNLIKELY(gyroDataChunk_.get() == nullptr)) {
                    mfllLogE("%s: Gyro data chunk is null", __FUNCTION__);
                    break;
                }

                gyroDataChunk_.get()[i].fX = static_cast<MFLOAT>(gyroInfo[i].x);
                gyroDataChunk_.get()[i].fY = static_cast<MFLOAT>(gyroInfo[i].y);
                gyroDataChunk_.get()[i].fZ = static_cast<MFLOAT>(gyroInfo[i].z);
            }

            paramIn.prGyroInfo = gyroDataChunk_.get();
        }
        else {
            // data structures between middleware and algorithm are the same,
            // we can use memory chunk directly.
            paramIn.prGyroInfo = static_cast<AIS_GYRO_INFO*>(
                    static_cast<void*>(
                    const_cast<NSCam::Utils::GyroCollector::GyroInfo*>(gyroInfo.data())));
        }

        paramIn.u4CurrentIso = cfg.iso;
        paramIn.u4CurrentExp = cfg.exp;

        /* get param width/height from NVRAM */
        {
            const NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvram =
                reinterpret_cast<decltype(pNvram)>(m_nvramChunk);

            paramIn.u4paramWidth   = pNvram->u4ParamWidth;
            paramIn.u4paramHeight  = pNvram->u4ParamHeight;
        }

        /* query sensor size */
        do {
            mfllTraceName("get_sensor_size");

            NSCam::MSize _sensorSize;

            /* default value here */
            paramIn.u4sensorWidth = 0;
            paramIn.u4sensorHeight = 0;

            NSCamHW::HwInfoHelper _helper(cfg.sensor_id);
            if (CC_UNLIKELY( !_helper.updateInfos() ))
            {
                mfllLogE("update sensor static information failed");
                break;
            }

            if (CC_UNLIKELY( !_helper.getSensorSize(NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE, _sensorSize) ))
            {
                mfllLogE("cannot get params about sensor");
                break;
            }

            mfllLogD3("set sensor(%d) size to %dx%d", cfg.sensor_id, _sensorSize.w, _sensorSize.h);

            paramIn.u4sensorWidth = static_cast<MUINT32>(_sensorSize.w);
            paramIn.u4sensorHeight = static_cast<MUINT32>(_sensorSize.h);

        } while(0);

        mfllTraceEnd(); // collect_gyro_info

        // debug message
        // {{{
        mfllLogD3("%s: AIS_PROC2_PARA_IN.u4CurrentIso     = %u", __FUNCTION__, paramIn.u4CurrentIso);
        mfllLogD3("%s: AIS_PROC2_PARA_IN.u4CurrentExp(us) = %u", __FUNCTION__, paramIn.u4CurrentExp);
        mfllLogD3("%s: AIS_PROC2_PARA_IN.u4gyroNum        = %u", __FUNCTION__, paramIn.u4GyroNum);
        mfllLogD3("%s: AIS_PROC2_PARA_IN.prgyroInfo       = %p", __FUNCTION__, paramIn.prGyroInfo);
        mfllLogD3("%s: AIS_PROC2_PARA_IN.u4gyroIntervalMS = %u", __FUNCTION__, paramIn.u4GyroIntervalMS);
        mfllLogD3("%s: AIS_PROC2_PARA_IN.u4paramWidth     = %u", __FUNCTION__, paramIn.u4paramWidth);
        mfllLogD3("%s: AIS_PROC2_PARA_IN.u4paramHeight    = %u", __FUNCTION__, paramIn.u4paramHeight);
        mfllLogD3("%s: AIS_PROC2_PARA_IN.u4sensorWidth    = %u", __FUNCTION__, paramIn.u4sensorWidth);
        mfllLogD3("%s: AIS_PROC2_PARA_IN.u4sensorHeight   = %u", __FUNCTION__, paramIn.u4sensorHeight);
        // }}}

        /* Do AIS algorithm */
        {
            mfllTraceName("AIS_PROC2");
            mfllAutoLog("AIS_PROC2");
            auto result = m_mtkAis->AisMain(AIS_PROC2, &paramIn, &paramOut);
            if (result != S_AIS_OK) {
                mfllLogE("%s: MTKAis::AisMain with AIS_PROC returns fail(%#x)", __FUNCTION__, result);
            }
            else {
                finalCfg.frameCapture = static_cast<int>(paramOut.u1ReqSrcNum);
                finalCfg.frameBlend = static_cast<int>(paramOut.u1ReqFrmNum);
                finalCfg.iso = static_cast<int>(paramOut.u4NewIso);
                finalCfg.exp = static_cast<int>(paramOut.u4NewExp);
                mfllLogD3("%s: AIS_PROC2_PARA_OUT.u1ReqSrcNum   = %d", __FUNCTION__, finalCfg.frameCapture);
                mfllLogD3("%s: AIS_PROC2_PARA_OUT.u1ReqFrmNum   = %d", __FUNCTION__, finalCfg.frameBlend);
                mfllLogD3("%s: AIS_PROC2_PARA_OUT.u4NewIso      = %d", __FUNCTION__, finalCfg.iso);
                mfllLogD3("%s: AIS_PROC2_PARA_OUT.u4NewExp      = %d", __FUNCTION__, finalCfg.exp);
            }
        }

        /* clear working buffer */
        m_imgWorkingBuf = nullptr;

    // }}}
    }
    else {
        /* update capture num/blend num by NVRAM only */
        /* NVRAM block */
        const NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvram =
            reinterpret_cast<decltype(pNvram)>(m_nvramChunk);

        /* check ISO range */
        int __frame_num = [&cfg, &pNvram](){
            if      (cfg.iso < pNvram->iso_level1)
                return pNvram->frame_num1;
            else if (cfg.iso < pNvram->iso_level2)
                return pNvram->frame_num2;
            else if (cfg.iso < pNvram->iso_level3)
                return pNvram->frame_num3;
            else if (cfg.iso < pNvram->iso_level4)
                return pNvram->frame_num4;
            else if (cfg.iso < pNvram->iso_level5)
                return pNvram->frame_num5;
            else if (cfg.iso < pNvram->iso_level6)
                return pNvram->frame_num6;
            else if (cfg.iso < pNvram->iso_level7)
                return pNvram->frame_num7;
            else
                return pNvram->frame_num8;
        }();

        if (CC_LIKELY( pNvram->cap_num_fix == 0 ))
            finalCfg.frameCapture = __frame_num;
        finalCfg.frameBlend   = __frame_num;
    }

    //-------------------------------------------------------------------------
    // query if it's necessary to do MFNR
    //-------------------------------------------------------------------------
    /* checks ISO only */
    {
        const NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvram =
            reinterpret_cast<decltype(pNvram)>(m_nvramChunk);

        if (finalCfg.isAis != 0) { // AIS (MFNR with changing pline mechanism)

            bool isPlineTrigger = false;
            bool isMfnrTrigger = false;

            // current (preview) iso threshold for pline change
            if (finalCfg.original_iso < pNvram->pline_iso_th) {
                // original iso is good enough, no need to change PLine.
                finalCfg.iso = cfg.original_iso;
                finalCfg.exp = cfg.original_exp;
            }
            else{
                isPlineTrigger = true;
            }

            // change (capture) iso threshold for mfll trigger
            if (finalCfg.iso < pNvram->mfll_iso_th) {
                // low capture iso, only capture single frame (mfll will be bypass).
                finalCfg.frameCapture = 1;
                finalCfg.frameBlend = 1;
            }
            else {
                isMfnrTrigger = true;
            }

            // continue to execute the flow if PLine changing or MFNR triggered
            if (isPlineTrigger || isMfnrTrigger) {
                finalCfg.enableMfb = 1;
            }
            else {
                finalCfg.enableMfb = 0;
            }

            mfllLogI("%s:[AIS] Set enableMfb to %d due to Pline(%d)/MFNR(%d) will be triggered",
                    __FUNCTION__, finalCfg.enableMfb, isPlineTrigger, isMfnrTrigger);
        }
        else { // MFNR (without changing pline mechanism), on/off by iso threshold
            // no changing Pline, capture iso = preview iso
            if (cfg.iso > pNvram->mfll_iso_th)
                finalCfg.enableMfb = 1;
            else
                finalCfg.enableMfb = 0;
        }

        // query downscale tuning setting: iso threshold to trigger downscale yuv and downscale ratio
        finalCfg.downscaleRatio  = pNvram->downscale_ratio;
        if (finalCfg.downscaleRatio > 0) {
            if (finalCfg.iso > pNvram->downscale_iso_th) {
                finalCfg.enableDownscale = 1;
            }
        }

        // debug by adb setting
        int bForceDownscale = MfllProperty::readProperty(Property_ForceDownscale);
        if (CC_UNLIKELY( bForceDownscale == 0 )) {
            mfllLogD("Force disable downscale yuv");
            finalCfg.enableDownscale = 0;
            finalCfg.downscaleRatio  = 0;
        }
        else if (CC_UNLIKELY(bForceDownscale > 0)) {
            mfllLogD("Force to enable downscale yuv (bForceDownscale:%d)", bForceDownscale);
            finalCfg.enableDownscale = 1;
            finalCfg.downscaleRatio  = 2;
        }

        // TODO: MEMC algo only support downscale ratio to be x2.
        if (finalCfg.downscaleRatio > 2) {
            mfllLogI("%s: only support downscale ratio < x2 (invalid downscale nvram setting = x%d)",
                    __FUNCTION__, finalCfg.downscaleRatio);
            finalCfg.downscaleRatio  = 2;
        }

        mfllLogI("%s: iso:%u, origin_iso:%u, pline_iso_th:%d, mfll_iso_th:%d, downscale(iso_th:%d, enabled:%d, ratio:%d), finalCfg(enableMfb:%d, frameCapture:%d)",
                __FUNCTION__, cfg.iso, cfg.original_iso, pNvram->pline_iso_th, pNvram->mfll_iso_th, pNvram->downscale_iso_th,
                finalCfg.enableDownscale, finalCfg.downscaleRatio, finalCfg.enableMfb, finalCfg.frameCapture);
    }


    //-------------------------------------------------------------------------
    // always force on MFB
    //-------------------------------------------------------------------------
#if MFLL_MFB_ALWAYS_ON
    finalCfg.enableMfb = 1;
#endif

    //-------------------------------------------------------------------------
    // check properties.
    //-------------------------------------------------------------------------
    {
        int bForceMfb = MfllProperty::isForceMfll();

        if (CC_UNLIKELY( bForceMfb == 0 )) {
            mfllLogD("Force disable MFNR");
            finalCfg.enableMfb = 0;
        }
        else if (CC_UNLIKELY(bForceMfb > 0)) {
            mfllLogD("Force MFNR (bForceMfb:%d)", bForceMfb);
            finalCfg.enableMfb = 1;
        }

        if (finalCfg.enableMfb) {
            int forceExp        = MfllProperty::getExposure();
            int forceIso        = MfllProperty::getIso();
            int forceCaptureNum = MfllProperty::getCaptureNum();
            int forceBlendNum   = MfllProperty::getBlendNum();
            int forceFullSizeMc = MfllProperty::getFullSizeMc();

            if (CC_UNLIKELY(forceExp > 0)) finalCfg.exp = forceExp * 1000;
            if (CC_UNLIKELY(forceIso > 0)) finalCfg.iso = forceIso;
            if (CC_UNLIKELY(forceCaptureNum > 0)) finalCfg.frameCapture = forceCaptureNum;
            if (CC_UNLIKELY(forceBlendNum > 0)) finalCfg.frameBlend = forceBlendNum;
            if (CC_UNLIKELY(forceFullSizeMc >= 0)) finalCfg.isFullSizeMc = forceFullSizeMc;

            /* calculate either auto iso or auto exposure */
            if (CC_LIKELY(forceExp <= 0 && forceIso <= 0)) {
                // general case, do nothing.
            }
            else if (forceExp > 0 && forceIso > 0) {
                // manual set both exposure and ISO, do nothing.
            }
            else if (forceExp > 0) {
                /* calculate auto ISO */
                int total = cfg.exp * cfg.iso;
                finalCfg.iso = total / finalCfg.exp;
            }
            else if (forceIso > 0) {
                int total = cfg.exp * cfg.iso;
                finalCfg.exp = total / finalCfg.iso;
            }
            else;
        }
    }

    *out = finalCfg;
    return MfllErr_Ok;
}
