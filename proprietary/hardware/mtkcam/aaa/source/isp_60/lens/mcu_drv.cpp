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

#define LOG_TAG "LensDrv"

#include <fcntl.h>
#include <utils/threads.h>  // For android::Mutex::Autolock.
#include <aaa_types.h>       /* DataType, Ex : MINT32 */
#include <mtkcam/utils/std/Log.h>
#include <isp_tuning.h>      /* Enum, Ex : ESensorDev_T */
#include <mtkcam/drv/mem/cam_cal_drv.h> /* AF calibration data */
#include <mtkcam/drv/IHalSensor.h>

#include <af_cxu_param.h>
#if defined(MTKCAM_CCU_AF_SUPPORT)
#include <cutils/properties.h>
#include "iccu_ctrl_af.h"
#endif

#include "camera_custom_msdk.h"

#include "mcu_drv.h"
#include "mcu_info.h"

#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>



static android::Mutex& g_VCMDevLock = *new android::Mutex(); // VCM drivers only use one API due to custom_drv setting.

#define GETTIMESTAMP(time) {      \
        struct timespec t;        \
        MINT64 timestamp;        \
                                  \
        t.tv_sec = t.tv_nsec = 0; \
        timestamp = 0;            \
        clock_gettime(CLOCK_MONOTONIC, &t); \
        timestamp = ((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000; \
        time = timestamp;         \
        }

#define MY_INST_MCUDRV NS3Av3::INST_T<MCUDrv>
static std::array<MY_INST_MCUDRV, SENSOR_IDX_MAX> gMultitonMCUDrv;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MCUDrv::MCUDrv(MINT32 const eSensorDev)
{
    m_u4SensorDev = eSensorDev;
    m_u4LensIdx   = 0;
    m_McuDevIdx   = 0;
    memset(m_LensInitFunc, 0, sizeof(MSDK_LENS_INIT_FUNCTION_STRUCT)*MAX_NUM_OF_SUPPORT_LENS);

    // the related kernel
    m_fdMCU       = -1;
    m_userCnt     = 0;
    m_CurPos      = 0;
    m_CurTS       = 0;
    m_PrvPos      = 0;
    m_PrvTS       = 0;
    m_InitPos     = 0;
    m_InitDone    = 0;
    m_SetInitPos  = 0;
    m_CalInfPos   = 0;
    m_McuThreadEn = 0;

    m_i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    if (m_i4SensorIdx >= 0 && m_i4SensorIdx < 5)
    {
        m_McuDevIdx = m_i4SensorIdx;
    }
    else
    {
        m_McuDevIdx = 0;
        CAM_LOGE("[%s] Unsupport MCU Driver Index: %d(%d)\n", __FUNCTION__, m_McuDevIdx, m_i4SensorIdx);
    }

    memset(m_u1MotorFolderName, 0, sizeof(MUINT8) * 32);
    m_u1MotorFolderName[0] = '\0';

#if defined(MTKCAM_CCU_AF_SUPPORT)
    m_ptrCcu = NULL;
    m_CurTS_GPT_CCU = 0;
    m_PrvTS_GPT_CCU = 0;
    m_CurTS_CCU = 0;
    m_PrvTS_CCU = 0;
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MCUDrv* MCUDrv::getInstance(MINT32 const i4SensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(i4SensorDev);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
    }

    MY_INST_MCUDRV& rSingleton = gMultitonMCUDrv[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<MCUDrv>(i4SensorDev);
    } );

    // CAM_LOGD("[%s] SensorDev(%d), SensorIdx(%d) instance(%p)\n", __FUNCTION__, i4SensorDev, i4SensorIdx, rSingleton.instance.get());

    return rSingleton.instance.get();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::lensSearch(MUINT32 a_u4SensorId, MUINT32 a_u4ModuleId)
{
    android::Mutex::Autolock lock(g_VCMDevLock);

    MUINT32 i = 0;

    CAM_LOGD("[%s] Dev(%d) , SensorId(0x%04x), ModuleId(0x%04x), m_McuDevIdx(0x%04x)", __FUNCTION__, m_u4SensorDev, a_u4SensorId, a_u4ModuleId, m_McuDevIdx);

    LensCustomInit(m_u4SensorDev);

    LensCustomGetInitFunc(m_LensInitFunc);

    m_u4LensIdx = 0;

    for (i = 0; i < MAX_NUM_OF_SUPPORT_LENS; i++)
    {
        if (m_LensInitFunc[i].LensId == DUMMY_LENS_ID)
        {
            m_u4LensIdx = i;
        }
    }

    // force assign LensIdx if SensorId != DUMMY_SENSOR_ID (to support backup lens/new lens driver)
    for (i = 0; i < MAX_NUM_OF_SUPPORT_LENS; i++)
    {
        if ((m_LensInitFunc[i].SensorId == a_u4SensorId) && (a_u4SensorId != 0xFFFF) && (a_u4SensorId != 0x0))
        {
            CAM_LOGD("[%s] Dev(%d) , ListIdx(0x%04x), ListSensorID(0x%04x), ListModuleID(0x%04x), ListLensName(%s)",
                     __FUNCTION__, m_u4SensorDev, i, m_LensInitFunc[i].SensorId, m_LensInitFunc[i].ModuleId, m_LensInitFunc[i].LensDrvName);

#if 1 // use module ID directly
            if ((m_LensInitFunc[i].ModuleId == DUMMY_MODULE_ID) || (m_LensInitFunc[i].ModuleId == a_u4ModuleId))
            {
                m_u4LensIdx = i;
                break;
            }
#else // use kernel driver to read moduel ID
            int fdMCU = open(MCU_DRV_NAME[m_McuDevIdx], O_RDWR);
            if (fdMCU >= 0)
            {
                mcuMotorName motorName;
                memcpy(motorName.uMotorName, m_LensInitFunc[i].LensDrvName, 32);
                int err = ioctl(fdMCU, mcuIOC_S_SETDRVNAME, &motorName);
                close(fdMCU);
                if (err > 0)
                {
                    m_u4LensIdx = i;
                    break;
                }
            }
#endif
        }
    }
    LensCustomSetIndex(m_u4LensIdx);

    CAM_LOGD("[%s] Dev(%d) , LensIdx(0x%04x)\n",
             __FUNCTION__, m_u4SensorDev, m_u4LensIdx);

    return 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::getCurrLensID()
{
    return m_LensInitFunc[m_u4LensIdx].LensId;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::setLensNvramIdx()
{
    android::Mutex::Autolock lock(g_VCMDevLock);

    LensCustomInit(m_u4SensorDev);

    LensCustomGetInitFunc(m_LensInitFunc);

    LensCustomSetIndex(m_u4LensIdx);

    CAM_LOGD("[%s] Dev(%d) , LensIdx(0x%04x)\n",
             __FUNCTION__, m_u4SensorDev, m_u4LensIdx);

    return 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::isLensSupport(MUINT32 a_u4SensorId)
{
    android::Mutex::Autolock lock(g_VCMDevLock);

    int ret = 0;

    CAM_LOGD("[%s] Dev(%d) , SensorId(0x%04x)", __FUNCTION__, m_u4SensorDev, a_u4SensorId);

    if ((a_u4SensorId == 0xFFFF) || (a_u4SensorId == 0x0))
    {
        return 0;
    }

    LensCustomInit(m_u4SensorDev);

    LensCustomGetInitFunc(m_LensInitFunc);

    for (MUINT32 i = 0; i < MAX_NUM_OF_SUPPORT_LENS; i++)
    {
        if ((m_LensInitFunc[i].SensorId == a_u4SensorId) && (m_LensInitFunc[i].LensId != DUMMY_LENS_ID))
        {
            ret = 1;
            break;
        }
    }

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::init(MUINT32 a_u4InitPos, MUINT32 a_u4SensorIdx, MINT32 forceCPU)
{
    android::Mutex::Autolock lock(mLock);

    CAM_LOGD("[%s] Dev(%d) Idx(%d) , InitPos(0x%04x)", __FUNCTION__, m_u4SensorDev, a_u4SensorIdx, a_u4InitPos);

    MUINT32 ret = 0;

    if (m_McuThreadEn == 0 && m_InitDone == 0)
    {
        m_InitPos     = a_u4InitPos;
        ret = pthread_create(&m_InitThread, NULL, initMCUThread, this);
        CAM_LOGD("[%s] Dev(%d) , pthread_create(%d)", __FUNCTION__, m_u4SensorDev, ret);
        if (ret)
        {
            CAM_LOGD("[%s] Dev(%d) , pthread_create fail", __FUNCTION__, m_u4SensorDev);
            m_McuThreadEn = 0;
        }
        else
        {
            CAM_LOGD("[%s] Dev(%d) , pthread_create sucess", __FUNCTION__, m_u4SensorDev);
            m_McuThreadEn = 1;
            m_SetInitPos  = 1;
        }
    }

#if defined(MTKCAM_CCU_AF_SUPPORT)
    if (m_ptrCcu == NULL)
    {
        MINT32 isForceCPU = property_get_int32("vendor.debug.forceCPUAF.enable", FORCE_CPUAF_ENABLE);
        if (isForceCPU == 0)
        {
            MINT32 isCCUAlive = 0;
            m_ptrCcu = static_cast<MVOID *>(NSCcuIf::ICcuCtrlAf::createInstance((const uint8_t *)"LensDrv", (ESensorDev_T)m_u4SensorDev));
            NSCcuIf::ICcuCtrlAf *ptrCcu = static_cast<NSCcuIf::ICcuCtrlAf *>(m_ptrCcu);
            isCCUAlive = ptrCcu->init(a_u4SensorIdx, (ESensorDev_T)m_u4SensorDev);
            if (NSCcuIf::CCU_CTRL_SUCCEED != isCCUAlive)
            {
                ptrCcu->uninit();
                ptrCcu->destroyInstance();
                m_ptrCcu = NULL;
            }
        }
    }
    m_CurTS_GPT_CCU = 0;
    m_PrvTS_GPT_CCU = 0;
    m_CurTS_CCU = 0;
    m_PrvTS_CCU = 0;
#endif

    return (ret == 0) ? 1 : 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* MCUDrv::initMCUThread(MVOID *arg)
{
    MCUDrv *_this = reinterpret_cast<MCUDrv*>(arg);

    _this->initMCU();

    return NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::initMCU()
{
    CAM_LOGD("[%s] Dev(%d) , [Lens Driver] %s [m_userCnt] %d +", __FUNCTION__,
             m_u4SensorDev, m_LensInitFunc[m_u4LensIdx].LensDrvName, m_userCnt);

    if (m_userCnt == 0)
    {
        if (m_fdMCU == -1)
        {
            m_fdMCU = open(MCU_DRV_NAME[m_McuDevIdx], O_RDWR);

            if (m_fdMCU < 0)
            {
                m_CurPos   = 0;
                m_CurTS    = 0;
                m_PrvPos   = 0;
                m_PrvTS    = 0;
                m_InitDone = 1;

                if (m_u4LensIdx == 0)
                {
                    // no error log for dummy lens
                    CAM_LOGD("[%s] Dev(%d) , dummy lens", __FUNCTION__, m_u4SensorDev);
                    return 1;
                }
                else
                {
                    CAM_LOGD("[%s] Dev(%d) , Lens error opening (%s)", __FUNCTION__, m_u4SensorDev, strerror(errno));
                    return 0;
                }
            }

            mcuMotorName motorName;
            memcpy(motorName.uMotorName, m_LensInitFunc[m_u4LensIdx].LensDrvName, 32);

            int err = ioctl(m_fdMCU, mcuIOC_S_SETDRVNAME, &motorName);
            if (err <= 0)
            {
                m_CurPos = 0;
                m_InitDone = 1;
                CAM_LOGD("[%s] Dev(%d) , please check kernel driver ", __FUNCTION__, m_u4SensorDev);
                close(m_fdMCU);
                m_fdMCU = -1;
                return 0;
            }

            if (m_CalInfPos == 0)
            {
                CAM_CAL_DATA_STRUCT GetCamCalData;
                CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
                MINT32 i4SensorDevID = NS3Av3::mapSensorIdxToDev(m_i4SensorIdx);

                if (i4SensorDevID != -1)
                {
                    MINT32 i4InfPos = 0;
                    pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, CAMERA_CAM_CAL_DATA_3A_GAIN, (void *)&GetCamCalData);
                    i4InfPos  = GetCamCalData.Single2A.S2aAf[0];
                    m_CalInfPos = (0 < i4InfPos && i4InfPos < 1023) ? i4InfPos : m_CalInfPos;
                }
            }

            {
                android::Mutex::Autolock lock(mLock_InitPos);
                m_InitPos = (m_InitPos == 0) ? m_CalInfPos : m_InitPos;
                err = ioctl(m_fdMCU, mcuIOC_T_MOVETO, (unsigned long)m_InitPos);

                m_PrvPos = m_CurPos;
                m_PrvTS  = m_CurTS;
                m_CurPos = m_InitPos;
                GETTIMESTAMP(m_CurTS);
            }

            if (err < 0) {
                m_CurPos = 0;
                CAM_LOGD("[%s] Dev(%d) , ioctl - mcuIOC_T_MOVETO, error %s\n", __FUNCTION__, m_u4SensorDev, strerror(errno));
            }

            m_InitDone = 1;
            CAM_LOGD("[%s] Dev(%d) , Init Pos(%d) Done\n", __FUNCTION__, m_u4SensorDev, m_InitPos);
        }
    }

    m_userCnt++;
    m_InitDone = 1;

    CAM_LOGD("[%s] Dev(%d) , [m_InitDone] %d [m_userCnt] %d -", __FUNCTION__,
             m_u4SensorDev, m_InitDone, m_userCnt);

    return 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::waitInitDone()
{
    android::Mutex::Autolock lock(mLock);

    CAM_LOGD("[%s] Dev(%d) -", __FUNCTION__, m_u4SensorDev);

    if (m_McuThreadEn == 1)
    {
        CAM_LOGD("[%s] Dev(%d) , pthread_join", __FUNCTION__, m_u4SensorDev);
        pthread_join(m_InitThread, NULL);
        m_McuThreadEn = 0;
    }

    CAM_LOGD("[%s] Dev(%d) -", __FUNCTION__, m_u4SensorDev);

    return 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::setInitPos(MUINT32 a_u4InitPos)
{
    android::Mutex::Autolock lock(mLock);
    MUINT32 ret = 0;
    CAM_LOGD("[%s] Dev(%d) +", __FUNCTION__, m_u4SensorDev);

    if (m_SetInitPos == 1)
    {
        android::Mutex::Autolock lock(mLock_InitPos);
        int err;

        m_InitPos = a_u4InitPos;

        if (m_InitDone == 1)
        {
            err = ioctl(m_fdMCU, mcuIOC_T_MOVETO, (unsigned long)m_InitPos);

            m_PrvPos = m_CurPos;
            m_PrvTS  = m_CurTS;
            m_CurPos = m_InitPos;
            GETTIMESTAMP(m_CurTS);

            if (err < 0) {
                CAM_LOGD("[%s] Dev(%d) , ioctl - mcuIOC_T_MOVETO, error %s\n", __FUNCTION__, m_u4SensorDev, strerror(errno));
            }
        }

        m_SetInitPos = 0;
        ret = 1;
    }

    CAM_LOGD("[%s] Dev(%d) -", __FUNCTION__, m_u4SensorDev);

    return ret;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::uninit()
{
    android::Mutex::Autolock lock(mLock);

    MUINT32 ret = 0;

#if defined(MTKCAM_CCU_AF_SUPPORT)
    if (m_ptrCcu != NULL)
    {
        NSCcuIf::ICcuCtrlAf *ptrCcu = static_cast<NSCcuIf::ICcuCtrlAf *>(m_ptrCcu);
        ptrCcu->uninit();
        ptrCcu->destroyInstance();
        m_ptrCcu = NULL;
    }
#endif

    if (m_McuThreadEn == 1)
    {
        CAM_LOGD("[%s] Dev(%d) , pthread_join", __FUNCTION__, m_u4SensorDev);
        pthread_join(m_InitThread, NULL);
        m_McuThreadEn = 0;
    }

    m_InitDone = 0;
    m_CurPos   = 0;
    m_CurTS    = 0;
    m_PrvPos   = 0;
    m_PrvTS    = 0;
    m_SetInitPos = 0;

    CAM_LOGD("[%s] Dev(%d) , [m_userCnt]%d [fdMCU_main]%d +", __FUNCTION__, m_u4SensorDev, m_userCnt, m_fdMCU);

    if (m_userCnt == 1)
    {
        if (m_fdMCU > 0)
            close(m_fdMCU);

        m_fdMCU = -1;
    }
    m_userCnt--;

    CAM_LOGD("[%s] Dev(%d) , [m_userCnt]%d [fdMCU_main]%d -", __FUNCTION__, m_u4SensorDev, m_userCnt, m_fdMCU);

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::moveMCU(MUINT32 a_i4FocusPos)
{
    android::Mutex::Autolock lock(mLock);

    int err;

    if (m_InitDone == 0)
    {
        CAM_LOGD("[%s] Dev(%d) , driver init (%d:%d)", __FUNCTION__, m_u4SensorDev, m_McuThreadEn, m_InitDone);
        return 0;
    }

    if (m_fdMCU < 0)
    {
        if (m_u4LensIdx == 0)
        {   // no error log for dummy lens
            return 0;
        }
        else
        {
            CAM_LOGD("[%s] Dev(%d) , invalid m_fdMCU = %d\n", __FUNCTION__, m_u4SensorDev, m_fdMCU);
            return 0;
        }
    }

    err = ioctl(m_fdMCU, mcuIOC_T_MOVETO, (unsigned long)a_i4FocusPos);

    m_PrvPos = m_CurPos;
    m_PrvTS  = m_CurTS;
    m_CurPos = a_i4FocusPos;
    GETTIMESTAMP(m_CurTS);

    if (err < 0)
    {
        CAM_LOGD("[%s] Dev(%d) , ioctl - mcuIOC_T_MOVETO, error %s\n", __FUNCTION__, m_u4SensorDev, strerror(errno));
        return m_CurTS;
    }

    return m_CurTS;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::setMCUParam(MUINT32 a_CmdId, MUINT32 a_Param)
{
    int err;

    if (m_fdMCU < 0)
    {
        if (m_u4LensIdx == 0)
        {   // no error log for dummy lens
            return 1;
        }
        else
        {
            CAM_LOGD("[%s] Dev(%d) , invalid m_fdMCU = %d\n", __FUNCTION__, m_u4SensorDev, m_fdMCU);
            return 0;
        }
    }

    mcuMotorCmd motorCmd;

    motorCmd.u4CmdID = (unsigned int)a_CmdId;
    motorCmd.u4Param = (unsigned int)a_Param;

    err = ioctl(m_fdMCU, mcuIOC_S_SETPARA, &motorCmd);

    if (err < 0)
    {
        CAM_LOGD("[%s] Dev(%d) , ioctl - mcuIOC_S_SETPARA, error %s\n", __FUNCTION__, m_u4SensorDev, strerror(errno));
        return 0;
    }

    return 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::ctrlMCUParam(MUINT64 a_CmdId, MUINT64 *a_Param)
{
    int err;

    if (m_fdMCU < 0)
    {
        if (m_u4LensIdx == 0)
        {   // no error log for dummy lens
            return 1;
        }
        else
        {
            CAM_LOGD("[%s] Dev(%d) , invalid m_fdMCU = %d\n", __FUNCTION__, m_u4SensorDev, m_fdMCU);
            return 0;
        }
    }

    mcuCtrlCmd CtrlCmd;

    CtrlCmd.i8CmdID = (long long)a_CmdId;
    CtrlCmd.i8Param[0] = (long long)(a_Param[0]);
    CtrlCmd.i8Param[1] = (long long)(a_Param[1]);

    err = ioctl(m_fdMCU, mcuIOC_X_CTRLPARA, &CtrlCmd);

    if (err < 0)
    {
        CAM_LOGD("[%s] Dev(%d) , ioctl - mcuIOC_S_SETPARA, error %s\n", __FUNCTION__, m_u4SensorDev, strerror(errno));
        return 0;
    }

    a_Param[0] = (MUINT64)CtrlCmd.i8Param[0];
    a_Param[1] = (MUINT64)CtrlCmd.i8Param[1];

    return 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::getMCUInfo(mcuMotorInfo *a_pMotorInfo)
{
#if defined(MTKCAM_CCU_AF_SUPPORT)
    if (m_ptrCcu != NULL && m_InitDone == 1)
    {
        MINT32 CurPos = 0;
        MINT32 CurTS_L = 0;
        MINT32 CurTS_H = 0;
        MINT32 PrvPos = 0;
        MINT32 PreTS_L = 0;
        MINT32 PreTS_H = 0;
        NSCcuIf::ICcuCtrlAf *ptrCcu = static_cast<NSCcuIf::ICcuCtrlAf *>(m_ptrCcu);
        ptrCcu->ccu_af_get_curposition(&CurPos, &CurTS_H, &CurTS_L, &PrvPos, &PreTS_H, &PreTS_L);
        MUINT64 CurTS_clk = (((MUINT64)CurTS_H) << 32) + (MUINT32)CurTS_L;
        MUINT64 PreTS_clk = (((MUINT64)PreTS_H) << 32) + (MUINT32)PreTS_L;
        if ((m_CurTS_GPT_CCU != CurTS_clk) || (m_PrvTS_GPT_CCU != PreTS_clk))
        {
            android::Mutex::Autolock lock(mLock);
            MUINT64 u8Param[2];

            u8Param[0] = CurTS_clk;
            u8Param[1] = PreTS_clk;
            ctrlMCUParam(CONVERT_CCU_TIMESTAMP, u8Param);
            if (CurTS_clk != 0)
                m_CurTS_CCU = u8Param[0] / 1000;
            if (PreTS_clk != 0)
                m_PrvTS_CCU = u8Param[1] / 1000;
        }

        if (CurTS_clk != 0) /* if ccu move lens, CurTs will not to be 0 */
        {
            if (PreTS_clk == 0) /* CCU can't store initial position */
            {
                m_PrvPos = (MUINT32)m_CurPos;
                m_PrvTS  = (MUINT64)m_CurTS;
            }
            else
            {
                m_PrvPos = (MUINT32)PrvPos;
                m_PrvTS  = (MUINT64)m_PrvTS_CCU;
            }
            m_CurPos = (MUINT32)CurPos;
            m_CurTS  = (MUINT64)m_CurTS_CCU;
        }

        m_CurTS_GPT_CCU = CurTS_clk;
        m_PrvTS_GPT_CCU = PreTS_clk;
    }
#endif

    a_pMotorInfo->bIsMotorOpen        = m_InitDone;
    a_pMotorInfo->bIsMotorMoving      = 1;
    a_pMotorInfo->u4CurrentPosition   = m_CurPos;
    a_pMotorInfo->u8CurrentTimestamp  = m_CurTS;
    a_pMotorInfo->u4PreviousPosition  = m_PrvPos;
    a_pMotorInfo->u8PreviousTimestamp = m_PrvTS;
    a_pMotorInfo->u4MacroPosition     = 1023;
    a_pMotorInfo->u4InfPosition       = 0;
    a_pMotorInfo->bIsSupportSR        = 1;

    return 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::setMCUInfPos(int a_i4FocusPos)
{
    int err;

    if (m_fdMCU < 0)
    {
        if (m_u4LensIdx == 0)
        {   // no error log for dummy lens
            return 1;
        }
        else
        {
            CAM_LOGD("[%s] Dev(%d) , invalid m_fdMCU = %d\n", __FUNCTION__, m_u4SensorDev, m_fdMCU);
            return 0;
        }
    }

    err = ioctl(m_fdMCU, mcuIOC_T_SETINFPOS, (unsigned long)a_i4FocusPos);
    if (err < 0)
    {
        CAM_LOGD("[%s] Dev(%d) , ioctl - mcuIOC_T_SETINFPOS, error %s\n", __FUNCTION__, m_u4SensorDev, strerror(errno));
        return 0;
    }

    return 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::setMCUMacroPos(int a_i4FocusPos)
{
    int err;

    if (m_fdMCU < 0)
    {
        if (m_u4LensIdx == 0)
        {   // no error log for dummy lens
            return 1;
        }
        else
        {
            CAM_LOGD("[%s] Dev(%d) , invalid m_fdMCU = %d\n", __FUNCTION__, m_u4SensorDev, m_fdMCU);
            return 0;
        }
    }

    err = ioctl(m_fdMCU, mcuIOC_T_SETMACROPOS, (unsigned long)a_i4FocusPos);
    if (err < 0)
    {
        CAM_LOGD("[%s] Dev(%d) , ioctl - mcuIOC_T_SETMACROPOS, error %s\n", __FUNCTION__, m_u4SensorDev, strerror(errno));
        return 0;
    }

    return 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::getOISPosInfo(mcuOISPosInfo *a_pOISPosInfo)
{
    int err;

    memset(a_pOISPosInfo, 0, sizeof(mcuOISPosInfo));

    if (m_fdMCU < 0)
    {
        if (m_u4LensIdx == 0)
        {   // no error log for dummy lens
            return 1;
        }
        else
        {
            CAM_LOGD("[%s] Dev(%d) , invalid m_fdMCU = %d\n", __FUNCTION__, m_u4SensorDev, m_fdMCU);
            return 0;
        }
    }

    err = ioctl(m_fdMCU, mcuIOC_G_OISPOSINFO, a_pOISPosInfo);
    if (err < 0)
    {
        CAM_LOGD("[%s] Dev(%d) , ioctl - mcuIOC_G_OISPOSINFO, error %s\n", __FUNCTION__, m_u4SensorDev, strerror(errno));
        return 0;
    }

    return 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::getMCUOISInfo(mcuMotorOISInfo *a_pMotorOISInfo)
{
    int err;

    mcuMotorOISInfo motorOISInfo;
    memset(&motorOISInfo, 0, sizeof(motorOISInfo));

    a_pMotorOISInfo->i4OISHallPosXum  = 0;
    a_pMotorOISInfo->i4OISHallPosYum  = 0;
    a_pMotorOISInfo->i4OISHallFactorX = 0;
    a_pMotorOISInfo->i4OISHallFactorY = 0;

    if (m_fdMCU < 0)
    {
        if (m_u4LensIdx == 0)
        {   // no error log for dummy lens
            return 1;
        }
        else
        {
            CAM_LOGD("[%s] Dev(%d) , invalid m_fdMCU = %d\n", __FUNCTION__, m_u4SensorDev, m_fdMCU);
            return 0;
        }
    }

    err = ioctl(m_fdMCU, mcuIOC_G_MOTOROISINFO, &motorOISInfo);
    if (err < 0)
    {
        CAM_LOGD("[%s] Dev(%d) , ioctl - mcuIOC_G_MOTOROISINFO, error %s\n", __FUNCTION__, m_u4SensorDev, strerror(errno));
        return 0;
    }

    a_pMotorOISInfo->i4OISHallPosXum  = motorOISInfo.i4OISHallPosXum;
    a_pMotorOISInfo->i4OISHallPosYum  = motorOISInfo.i4OISHallPosYum;
    a_pMotorOISInfo->i4OISHallFactorX = motorOISInfo.i4OISHallFactorX;
    a_pMotorOISInfo->i4OISHallFactorY = motorOISInfo.i4OISHallFactorY;

    return 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 MCUDrv::getCurrLensName(MUINT8 *LensName)
{
#if 1
    MINT32 i;

    memcpy(LensName, m_u1MotorFolderName, sizeof(MUINT8) * 32);

    mcuMotorName motorName;
    memcpy(motorName.uMotorName, m_LensInitFunc[m_u4LensIdx].LensDrvName, 32);
    CAM_LOGW("[%s] Dev(%d) ,DrvName(%s)", __FUNCTION__, m_u4SensorDev, motorName.uMotorName);

    for (i = 0; i < 32; i++)
    {
        if (motorName.uMotorName[i] == '\0')
            break;

        motorName.uMotorName[i] = tolower(motorName.uMotorName[i]);
    }

    CAM_LOGW("[%s] Dev(%d) ,DrvFileName(%s)", __FUNCTION__, m_u4SensorDev, motorName.uMotorName);
    memcpy(LensName, motorName.uMotorName, sizeof(MUINT8) * 32);
    memcpy(m_u1MotorFolderName, motorName.uMotorName, sizeof(MUINT8) * 32);
#else
    MINT32 fdMCU = -1;

    fdMCU = open(MCU_DRV_NAME[m_McuDevIdx], O_RDWR);

    memcpy(LensName, m_u1MotorFolderName, sizeof(MUINT8) * 32);

    if (fdMCU < 0)
    {
        if (m_u4LensIdx == 0)
        {
            CAM_LOGW("[%s] Dev(%d) , dummy lens", __FUNCTION__, m_u4SensorDev);
            return 1;
        }
        else
        {
            CAM_LOGW("[%s] Dev(%d) , Lens error opening (%s)", __FUNCTION__, m_u4SensorDev, strerror(errno));
            return 0;
        }
    }

    mcuMotorName motorName;
    memcpy(motorName.uMotorName, m_LensInitFunc[m_u4LensIdx].LensDrvName, 32);
    CAM_LOGW("[%s] Dev(%d) ,DrvName(%s)", __FUNCTION__, m_u4SensorDev, motorName.uMotorName);

    int err = ioctl(fdMCU, mcuIOC_G_GETDRVNAME, &motorName);
    if (err <= 0)
    {
        CAM_LOGD("[%s] Dev(%d) , ioctl - mcuIOC_G_MOTOROISINFO, error %s\n", __FUNCTION__, m_u4SensorDev, strerror(errno));
        close(fdMCU);
        fdMCU = -1;
        return 0;
    }

    close(fdMCU);
    fdMCU = -1;
    CAM_LOGW("[%s] Dev(%d) ,DrvFileName(%s)", __FUNCTION__, m_u4SensorDev, motorName.uMotorName);
    memcpy(LensName, motorName.uMotorName, sizeof(MUINT8) * 32);
    memcpy(m_u1MotorFolderName, motorName.uMotorName, sizeof(MUINT8) * 32);
#endif

    return 1;
}
