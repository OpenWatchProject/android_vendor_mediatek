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
#define LOG_TAG "lsc_nvram"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

#include "ILscNvram.h"
#include <LscUtil.h>
#include <mtkcam/drv/IHalSensor.h>
#include <nvbuf_util.h>
#include "tsf_tuning_custom.h"
#include <mtkcam/drv/mem/cam_cal_drv.h>

#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

using namespace NSIspTuning;

#define MY_INST NS3Av3::INST_T<LscNvramImp>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

ILscNvram*
LscNvramImp::
getInstance(MUINT32 sensor)
{
   auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(sensor));

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<LscNvramImp>(sensor);
    } );

    return rSingleton.instance.get();
}

ILscNvram::E_LSC_OTP_T
LscNvramImp::
importEEPromData()
{
    E_LSC_OTP_T eRet = E_LSC_NO_OTP;
    MUINT32 i;
    LSC_LOG_BEGIN();

    CAMERA_CAM_CAL_TYPE_ENUM eCamCalDataType = CAMERA_CAM_CAL_DATA_SHADING_TABLE;
    CAM_CAL_DATA_STRUCT* pCalData = new CAM_CAL_DATA_STRUCT;

    if (pCalData == NULL)
    {
        LSC_ERR("Fail to allocate buffer!");
        return E_LSC_OTP_ERROR;
    }

#ifndef LSC_DBG
    AAA_TRACE_D("LSC nvram import eeprom get ins");

    CamCalDrvBase* pCamCalDrvObj = CamCalDrvBase::createInstance();
    if (!pCamCalDrvObj)
    {
        LSC_LOG("pCamCalDrvObj is NULL");
        delete pCalData;
        return E_LSC_NO_OTP;
    }

    MINT32 ret = pCamCalDrvObj->GetCamCalCalData(m_eSensorDev, eCamCalDataType, pCalData);

    AAA_TRACE_END_D;
#else
    MINT32 ret = 0;
    ::memcpy(pCalData, &_rDbgCamCalData, sizeof(CAM_CAL_DATA_STRUCT));
#endif

    LSC_LOG("ret(0x%08x)", ret);
    if (ret & CamCalReturnErr[eCamCalDataType])
    {
        LSC_LOG("Error(%s)", CamCalErrString[eCamCalDataType]);
        m_bIsEEPROMImported = MTRUE;
        delete pCalData;
        return E_LSC_NO_OTP;
    }
    else
    {
        LSC_LOG("Get OK");
    }

    MUINT32 u4Rot = 0;
    CAM_CAL_DATA_VER_ENUM eDataType  = pCalData->DataVer;
    CAM_CAL_LSC_DATA*     pLscData   = NULL;    // union struct

    LSC_LOG("eDataType(%d)", eDataType);
    switch (eDataType)
    {
    case CAM_CAL_SINGLE_EEPROM_DATA:
        LSC_LOG("CAM_CAL_SINGLE_EEPROM_DATA");
    case CAM_CAL_SINGLE_OTP_DATA:
        LSC_LOG("CAM_CAL_SINGLE_OTP_DATA");
        pLscData = &pCalData->SingleLsc.LscTable;
        u4Rot = pCalData->SingleLsc.TableRotation;
        break;
    case CAM_CAL_N3D_DATA:
        LSC_LOG("CAM_CAL_N3D_DATA");
        if (NSCam::SENSOR_DEV_MAIN == m_eSensorDev)
        {
            pLscData = &pCalData->N3DLsc.Data[0].LscTable;
            u4Rot = pCalData->N3DLsc.Data[0].TableRotation;
            LSC_LOG("CAM_CAL_N3D_DATA MAIN");
        }
        else if(NSCam::SENSOR_DEV_MAIN_2 == m_eSensorDev)
        {
            pLscData = &pCalData->N3DLsc.Data[1].LscTable;
            u4Rot = pCalData->N3DLsc.Data[1].TableRotation;
            LSC_LOG("CAM_CAL_N3D_DATA MAIN2");
        }
        else
        {
            LSC_LOG("CAM_CAL_N3D_DATA unsupport sensor %d", m_eSensorDev);
            delete pCalData;
            return E_LSC_NO_OTP;
        }
        break;
    default:
        LSC_ERR("Unknown eDataType(%d)", eDataType);
        m_bIsEEPROMImported = MTRUE;
        delete pCalData;
        return E_LSC_NO_OTP;
    }

    m_rOtp.TableRotation = u4Rot;
    ::memcpy(&m_rOtp.LscTable, pLscData, sizeof(CAM_CAL_LSC_DATA));
    LSC_LOG("u4Rot(%d), pLscData(%p)", u4Rot, pLscData);

    MUINT8 u1TblType = pLscData->MtkLcsData.MtkLscType;

    if (u1TblType & (1<<0))
    {
        // send table via sensor hal
        eRet = E_LSC_NO_OTP;
        //setSensorShading((MVOID*) &pLscData->SensorLcsData);
    }
    else if (u1TblType & (1<<1))
    {
        // do 1-to-3
        eRet = E_LSC_WITH_MTK_OTP;
        //m_fg1to3 = do123LutToSysram((MVOID*) &pLscData->MtkLcsData);
    }

    m_bIsEEPROMImported = MTRUE;

    delete pCalData;

    LSC_LOG_END();

    return eRet;
}
