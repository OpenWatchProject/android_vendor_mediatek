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
#define LOG_TAG "NvramDrvDep"

#include <mtkcam/utils/std/Log.h>


#define logW(fmt, arg...)    CAM_LOGW("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
#define logI(fmt, arg...)    CAM_LOGD("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
#define logE(fmt, arg...)    CAM_LOGE("[%s:%d] MError: " fmt, __FUNCTION__, __LINE__, ##arg)

#include <nvram_drv.h>
#include "nvram_drv_imp.h"
#include "nvbuf_util_dep.h"

#include <aaa_types.h>
#include "flash_param.h"
#include "flash_tuning_custom.h"
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>
#include <mutex>
#include <math.h>

#ifdef NVRAM_SUPPORT
#include "camera_custom_msdk.h"
#endif

#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>

unsigned long const g_u4NvramDataSize[CAMERA_DATA_TYPE_NUM] =
{
    sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
    sizeof(NVRAM_CAMERA_3A_STRUCT),
    sizeof(NVRAM_CAMERA_SHADING_STRUCT),
    sizeof(NVRAM_LENS_PARA_STRUCT),
    sizeof(AE_PLINETABLE_T),
    sizeof(NVRAM_CAMERA_STROBE_STRUCT),
    sizeof(CAMERA_TSF_TBL_STRUCT),
    sizeof(CAMERA_BPCI_STRUCT),
    sizeof(NVRAM_CAMERA_GEOMETRY_STRUCT),
    sizeof(NVRAM_CAMERA_FOV_STRUCT),
    sizeof(NVRAM_CAMERA_IDX_STRUCT),
    sizeof(NVRAM_CAMERA_FEATURE_STRUCT),
    sizeof(NVRAM_AF_PARA_STRUCT),
    sizeof(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT),
    sizeof(NVRAM_CAMERA_VERSION_STRUCT),
};


char gCreateDir[MAX_FILE_PATH_LEN] = "/mnt/vendor/nvcfg/camera";
char gFilePath[CAMERA_DATA_TYPE_NUM][MAX_FILE_PRE_PATH_LEN]=
{
    "mnt/vendor/nvcfg/camera/nv_isp",
    "mnt/vendor/nvcfg/camera/nv_3a",
    "mnt/vendor/nvcfg/camera/nv_shading",
    "mnt/vendor/nvcfg/camera/nv_lens",
    "mnt/vendor/nvcfg/camera/nv_pline",
    "mnt/vendor/nvcfg/camera/nv_strobe",
    "mnt/vendor/nvcfg/camera/nv_tsf",
    "mnt/vendor/nvcfg/camera/nv_pdc",
    "mnt/vendor/nvcfg/camera/nv_geometry",
    "mnt/vendor/nvcfg/camera/nv_fov",
    "mnt/vendor/nvcfg/camera/nv_idx_tbl",
    "mnt/vendor/nvcfg/camera/nv_feature",
    "mnt/vendor/nvcfg/camera/nv_af_calibration",
    "mnt/vendor/nvcfg/camera/nv_flash_calibration",
    "mnt/vendor/nvcfg/camera/nv_version",
};

static char gFacingName[2][MAX_FILE_PRE_PATH_LEN]=
{
    "_main",
    "_sub",
};
static char gFacingNum[3][MAX_FILE_PRE_PATH_LEN]=
{
    "",
    "_sec",
    "_third",
};


void getNvFileName(int dev, int id, char* outName)
{
    auto facing = 0; auto num = 0;
    NS3Av3::mapSensorIdxToFace(dev, facing, num);

    strncpy(outName, gFilePath[id], MAX_FILE_PRE_PATH_LEN);
    strncat(outName, gFacingName[facing], sizeof(gFacingName[facing]));

    char devSuffix[20] = {'\0'};
    if(num < 3) sprintf(devSuffix, "%s", gFacingNum[num]);
    else sprintf(devSuffix, "_%d", num);
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::checkDataVersion(
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData
)
{
    int err = NVRAM_NO_ERROR;

    logI("[checkDataVersion]\n");

    if (a_eNvramDataType == CAMERA_NVRAM_DATA_ISP) { // ISP
        PNVRAM_CAMERA_ISP_PARAM_STRUCT pCameraNvramData = (PNVRAM_CAMERA_ISP_PARAM_STRUCT)a_pNvramData;

        logI("[ISP] NVRAM data version(%d); F/W data version(%d)", pCameraNvramData->Version, NVRAM_CAMERA_PARA_FILE_VERSION);

        if (pCameraNvramData->Version != NVRAM_CAMERA_PARA_FILE_VERSION) {
            err = NVRAM_DATA_VERSION_ERROR;
        }
    }
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_3A) { // 3A
        PNVRAM_CAMERA_3A_STRUCT p3ANvramData = (PNVRAM_CAMERA_3A_STRUCT)a_pNvramData;

        logI("[3A] NVRAM data version(%d); F/W data version(%d)", p3ANvramData->u4Version, NVRAM_CAMERA_3A_FILE_VERSION);

        if (p3ANvramData->u4Version != NVRAM_CAMERA_3A_FILE_VERSION) {
            err = NVRAM_DATA_VERSION_ERROR;
        }
    }
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_SHADING) { // Shading
        PNVRAM_CAMERA_SHADING_STRUCT pShadingNvramData = (PNVRAM_CAMERA_SHADING_STRUCT)a_pNvramData;

        logI("[Shading] NVRAM data version(%d); F/W data version(%d)", pShadingNvramData->Shading.Version, NVRAM_CAMERA_SHADING_FILE_VERSION);

        if (pShadingNvramData->Shading.Version != NVRAM_CAMERA_SHADING_FILE_VERSION) {
            err = NVRAM_DATA_VERSION_ERROR;
        }
    }
    else if (a_eNvramDataType == CAMERA_DATA_TSF_TABLE) { // TSF
        PCAMERA_TSF_TBL_STRUCT pTsfNvramData = (PCAMERA_TSF_TBL_STRUCT)a_pNvramData;

        logI("[TSF] NVRAM data version(%d); F/W data version(%d)", pTsfNvramData->Version, NVRAM_CAMERA_TSF_FILE_VERSION);

        if (pTsfNvramData->Version != NVRAM_CAMERA_TSF_FILE_VERSION) {
            err = NVRAM_DATA_VERSION_ERROR;
        }
    }
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_LENS) { // Lens
        PNVRAM_LENS_PARA_STRUCT pLensNvramData = (PNVRAM_LENS_PARA_STRUCT)a_pNvramData;

        logI("[Lens] NVRAM data version(%d); F/W data version(%d)", pLensNvramData->Version, NVRAM_CAMERA_LENS_FILE_VERSION);

        if (pLensNvramData->Version != NVRAM_CAMERA_LENS_FILE_VERSION) {
            err = NVRAM_DATA_VERSION_ERROR;
        }
    }
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_STROBE) { // strobe
        PNVRAM_CAMERA_STROBE_STRUCT pStrobeNvramData = (PNVRAM_CAMERA_STROBE_STRUCT)a_pNvramData;
        logI("[Strobe] NVRAM data version(%d); F/W data version(%d)", pStrobeNvramData->u4Version, NVRAM_CAMERA_STROBE_FILE_VERSION);
        if (pStrobeNvramData->u4Version != NVRAM_CAMERA_STROBE_FILE_VERSION) {
            err = NVRAM_DATA_VERSION_ERROR;
        }
    }
    else {
        logE("checkDataVersion(): incorrect data type");
}

    return err;
}

int checkDataVersionNew(
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    int version
)
{
    int err = NVRAM_NO_ERROR;
    logI("");
    int targetVersion=0;
    if (a_eNvramDataType == CAMERA_NVRAM_DATA_ISP)   // ISP
        targetVersion = NVRAM_CAMERA_PARA_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_3A)   // 3A
        targetVersion = NVRAM_CAMERA_3A_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_SHADING)   // Shading
        targetVersion = NVRAM_CAMERA_SHADING_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_DATA_TSF_TABLE)   // TSF
        targetVersion = NVRAM_CAMERA_TSF_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_LENS)   // Lens
        targetVersion = NVRAM_CAMERA_LENS_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_STROBE)   // strobe
        targetVersion = NVRAM_CAMERA_STROBE_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_GEOMETRY)   // geometry
        targetVersion = NVRAM_CAMERA_GEOMETRY_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_FOV)   // fov
        targetVersion = NVRAM_CAMERA_FOV_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_FEATURE)   // feature
        targetVersion = NVRAM_CAMERA_FEATURE_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_DATA_AE_PLINETABLE)   // ae pline
        targetVersion = NVRAM_CAMERA_PLINE_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_AF)   // af
        targetVersion = NVRAM_CAMERA_AF_FILE_VERSION;
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_FLASH_CALIBRATION)   // flash
        targetVersion = NVRAM_CAMERA_FLASH_CALIBRATION_FILE_VERSION;
    else
        logE("incorrect data type");

    logI("v(%d) vTar(%d)", version, targetVersion);

    if(version!=targetVersion)
        return NVRAM_DATA_VERSION_ERROR;
    else
        return NVRAM_NO_ERROR;
}

extern bool bCustomInit;//[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed

static MBOOL isPowerOn[e_SensorTypeNum] = {0, 0, 0, 0};
static MUINT32 senModuleID[e_SensorTypeNum] = {0, 0, 0, 0};
static std::mutex powerStateLock;

MUINT32 checkSensorPowerStatus(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType, MUINT32 m_i4SensorIdx, int forceWait)
{
#if 0
    // Polling sensor power on
    MUINT32 u4PowerOn = 0;
    NSCam::IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    NSCam::IHalSensor* pIHalSensor = pIHalSensorList->createSensor(LOG_TAG, m_i4SensorIdx);

    //LSC_LOG("waiting for sensor(%d) power on", m_eSensorDev);
    MINT32 i=100;
    while ((0 == (a_eSensorType & u4PowerOn) && i != 0 ) && forceWait)
    {
        usleep(1000 * 10);
        i --;
        pIHalSensor->sendCommand(a_eSensorType, NSCam::SENSOR_CMD_GET_SENSOR_POWER_ON_STETE, (MUINTPTR)&u4PowerOn, 0, 0);
    }

    logI("Index(%d) Sensor(%d) power on done %d", m_i4SensorIdx, a_eSensorType, u4PowerOn);
    pIHalSensor->destroyInstance(LOG_TAG);

    return ((a_eSensorType & u4PowerOn)!=0);
#else
    return MTRUE;
#endif
}

unsigned int
NvramDrv::
readModuleIdFromEEPROM(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType, MINT32 r_i4SensorIdx, int forceWait)
{
    MUINT32 u4ModuleId=0;

    MUINT32 m_i4FakeSensorIdx=log2((MUINT32)a_eSensorType);
    powerStateLock.lock();
    if(!isPowerOn[m_i4FakeSensorIdx]) {
        logI("[%s] r_i4SensorIdx is %d", __func__, r_i4SensorIdx);
        if(r_i4SensorIdx==-1)
        {
            powerStateLock.unlock();
            logI("[%s] This timing sensor shouldn't power on, return directly", __func__);
            return u4ModuleId;
        }
        isPowerOn[m_i4FakeSensorIdx]=checkSensorPowerStatus(a_eSensorType, r_i4SensorIdx, forceWait);
        if(!isPowerOn[m_i4FakeSensorIdx]) {
            powerStateLock.unlock();
            logI("[%s] sensor is not power on, return directly", __func__);
            return u4ModuleId;
        }
        powerStateLock.unlock();
    }
    else {
        powerStateLock.unlock();
        logI("[%s] sensor is power on, return module id %d directly", __func__, senModuleID[m_i4FakeSensorIdx]);
        return senModuleID[m_i4FakeSensorIdx];
    }

    //if(!checkSensorPowerStatus(a_eSensorType, m_i4FakeSensorIdx))
    //    return u4ModuleId;

#ifdef NVRAM_MODULE_EN
    //Read EEPROM for Module id
    CamCalDrvBase* pCamCalDrvObj = CamCalDrvBase::createInstance();

    CAMERA_CAM_CAL_TYPE_ENUM eCamCalDataType=CAMERA_CAM_CAL_DATA_PART_NUMBER;
    CAM_CAL_DATA_STRUCT pCalData;

    MINT32 ret = pCamCalDrvObj->GetCamCalCalData(a_eSensorType, eCamCalDataType, (void*) &pCalData);

    logI("[%s] PartNumber: VID:%x %x, SID: %x %x, LID %x %x, VCM_ID %x %x",
        __func__,
        pCalData.PartNumber[0], pCalData.PartNumber[1],
        pCalData.PartNumber[6], pCalData.PartNumber[7],
        pCalData.PartNumber[8], pCalData.PartNumber[9],
        pCalData.PartNumber[10], pCalData.PartNumber[11]);

    u4ModuleId=(pCalData.PartNumber[8]<<8)+pCalData.PartNumber[9]; //use lens id only

    logI("[%s] Read module id (string: %s, int %x) was got",__func__, pCalData.PartNumber, u4ModuleId);
#else //default module id
    u4ModuleId=0;

    logI("[%s] used debug module id (int %x) was got",__func__, u4ModuleId);
#endif

    senModuleID[m_i4FakeSensorIdx]=u4ModuleId;

    return u4ModuleId;
}

#ifdef NVRAM_MODULE_IMP

int
NvramDrv::readDefaultData(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    unsigned long a_u4SensorID,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData
)
{
    logI("[readDefaultData +] Sensor ID(%x); NVRAM data type(%d)", a_u4SensorID, a_eNvramDataType);

    MUINT32 r_u4ModuleID=0;

    //read module id from EEPROM
    r_u4ModuleID=readModuleIdFromEEPROM(a_eSensorType, -1, MFALSE);

//    static bool bCustomInit = 0; //[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed

#ifdef NVRAM_SUPPORT

    if (!bCustomInit) {
        cameraCustomInit();
        if((a_eSensorType==DUAL_CAMERA_MAIN_2_SENSOR) || (a_eSensorType==DUAL_CAMERA_MAIN_SECOND_SENSOR))
            LensCustomInit(8);
        else
            LensCustomInit((unsigned int)a_eSensorType);
        bCustomInit = 1;
    }

    switch (a_eNvramDataType) {
    case CAMERA_NVRAM_DATA_ISP:
        GetCameraDefaultPara(a_u4SensorID, r_u4ModuleID, (PNVRAM_CAMERA_ISP_PARAM_STRUCT)a_pNvramData,NULL,NULL,NULL);
        break;
    case CAMERA_NVRAM_DATA_3A:
        GetCameraDefaultPara(a_u4SensorID, r_u4ModuleID, NULL,(PNVRAM_CAMERA_3A_STRUCT)a_pNvramData,NULL,NULL);
        break;
    case CAMERA_NVRAM_DATA_SHADING:
        GetCameraDefaultPara(a_u4SensorID, r_u4ModuleID, NULL,NULL,(PNVRAM_CAMERA_SHADING_STRUCT)a_pNvramData,NULL);
        break;
    case CAMERA_NVRAM_DATA_LENS:
        {
            GetLensDefaultPara(a_u4SensorID, r_u4ModuleID, (PNVRAM_LENS_PARA_STRUCT)a_pNvramData);
            PNVRAM_LENS_PARA_STRUCT pLensNvramData = (PNVRAM_LENS_PARA_STRUCT)a_pNvramData;
            pLensNvramData->Version = NVRAM_CAMERA_LENS_FILE_VERSION;
        }
        break;
    case CAMERA_DATA_AE_PLINETABLE:
        GetCameraDefaultPara(a_u4SensorID, r_u4ModuleID, NULL,NULL,NULL,(PAE_PLINETABLE_STRUCT)a_pNvramData);
        break;

    case CAMERA_NVRAM_DATA_STROBE:
        int ret;
        GetCameraFlashDefaultPara(a_u4SensorID, r_u4ModuleID,(PNVRAM_CAMERA_STROBE_STRUCT)a_pNvramData);
        break;

    case CAMERA_NVRAM_DATA_FLASH_CALIBRATION:
        int ret_flashCalibration;
        GetCameraFlashDefaultCalData(a_u4SensorID, r_u4ModuleID,(PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT)a_pNvramData);
        break;

    case CAMERA_DATA_TSF_TABLE:
        if (0 != GetCameraTsfDefaultTbl(a_u4SensorID, r_u4ModuleID, (PCAMERA_TSF_TBL_STRUCT)a_pNvramData))
        {
            return NVRAM_DEFAULT_DATA_READ_ERROR;
        }
        break;
    case CAMERA_DATA_PDC_TABLE:
        if (0 != GetCameraBpciDefaultTbl(a_u4SensorID, r_u4ModuleID, (PCAMERA_BPCI_STRUCT)a_pNvramData))
        {
            return NVRAM_DEFAULT_DATA_READ_ERROR;
        }
        break;
    case CAMERA_NVRAM_DATA_FEATURE:
        GetCameraFeatureDefault(a_u4SensorID, r_u4ModuleID, (NVRAM_CAMERA_FEATURE_STRUCT*)a_pNvramData);
        break;

    case CAMERA_NVRAM_IDX_TBL:
        if (0 != GetCameraIdxTblDefaultPara(a_u4SensorID, r_u4ModuleID, (PNVRAM_CAMERA_IDX_STRUCT)a_pNvramData))
        {
            return NVRAM_DEFAULT_DATA_READ_ERROR;
        }
        break;

    default:
        break;
    }

#endif
    logI("[readDefaultData -]");
    return NVRAM_NO_ERROR;
}

#else

/*******************************************************************************
*
********************************************************************************/

int
NvramDrv::readDefaultData(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    unsigned long a_u4SensorID,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData
)
{
    logI("[readDefaultData +] Sensor ID(%ld); NVRAM data type(%d)", a_u4SensorID, a_eNvramDataType);

//    static bool bCustomInit = 0; //[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed

#ifdef NVRAM_SUPPORT

    if (!bCustomInit) {
        cameraCustomInit();
        if((a_eSensorType==DUAL_CAMERA_MAIN_2_SENSOR) || (a_eSensorType==DUAL_CAMERA_MAIN_SECOND_SENSOR))
            LensCustomInit(8);
        else
            LensCustomInit((unsigned int)a_eSensorType);
        bCustomInit = 1;
    }

    switch (a_eNvramDataType) {
    case CAMERA_NVRAM_DATA_ISP:
        GetCameraDefaultPara(a_u4SensorID, (PNVRAM_CAMERA_ISP_PARAM_STRUCT)a_pNvramData,NULL,NULL,NULL);
        break;
    case CAMERA_NVRAM_DATA_3A:
        GetCameraDefaultPara(a_u4SensorID, NULL,(PNVRAM_CAMERA_3A_STRUCT)a_pNvramData,NULL,NULL);
        break;
    case CAMERA_NVRAM_DATA_SHADING:
        GetCameraDefaultPara(a_u4SensorID, NULL,NULL,(PNVRAM_CAMERA_SHADING_STRUCT)a_pNvramData,NULL);
        break;
    case CAMERA_NVRAM_DATA_LENS:
        {
            GetLensDefaultPara(a_u4SensorID,(PNVRAM_LENS_PARA_STRUCT)a_pNvramData);
            PNVRAM_LENS_PARA_STRUCT pLensNvramData = (PNVRAM_LENS_PARA_STRUCT)a_pNvramData;
            pLensNvramData->Version = NVRAM_CAMERA_LENS_FILE_VERSION;
        }
        break;
    case CAMERA_DATA_AE_PLINETABLE:
        GetCameraDefaultPara(a_u4SensorID, NULL,NULL,NULL,(PAE_PLINETABLE_STRUCT)a_pNvramData);
        break;

    case CAMERA_NVRAM_DATA_STROBE:
        int ret;
        GetCameraFlashDefaultPara(a_u4SensorID, (PNVRAM_CAMERA_STROBE_STRUCT)a_pNvramData);
        break;

    case CAMERA_NVRAM_DATA_FLASH_CALIBRATION:
        int ret_flashCalibration;
        GetCameraFlashDefaultCalData(a_u4SensorID, (PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT)a_pNvramData);
        break;

    case CAMERA_DATA_TSF_TABLE:
        if (0 != GetCameraTsfDefaultTbl(a_u4SensorID, (PCAMERA_TSF_TBL_STRUCT)a_pNvramData))
        {
            return NVRAM_DEFAULT_DATA_READ_ERROR;
        }
        break;
    case CAMERA_DATA_PDC_TABLE:
        if (0 != GetCameraBpciDefaultTbl(a_u4SensorID, (PCAMERA_BPCI_STRUCT)a_pNvramData))
        {
            return NVRAM_DEFAULT_DATA_READ_ERROR;
        }
        break;
    case CAMERA_NVRAM_DATA_FEATURE:
        GetCameraFeatureDefault(a_u4SensorID, (NVRAM_CAMERA_FEATURE_STRUCT*)a_pNvramData);
        break;

    case CAMERA_NVRAM_IDX_TBL:
        if (0 != GetCameraIdxTblDefaultPara(a_u4SensorID, (PNVRAM_CAMERA_IDX_STRUCT)a_pNvramData))
        {
            return NVRAM_DEFAULT_DATA_READ_ERROR;
        }
        break;

    default:
        break;
    }

#endif
    logI("[readDefaultData -]");
    return NVRAM_NO_ERROR;
}

#endif

int nvGetFlickerPara(MUINT32 SensorId, CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType, int SensorMode, void* buf)
{
    logI("nvGetFlickerPara(): id(%d), mode(%d).", SensorId, SensorMode);

    MUINT32 r_u4ModuleID=0;
    NvramDrvBase* nvDrv;
	nvDrv = NvramDrvBase::createInstance();

    //read module id from EEPROM
	r_u4ModuleID=nvDrv->readModuleIdFromEEPROM(a_eSensorType, -1, MTRUE);
	nvDrv->destroyInstance();

    int err = 0;
#ifndef USING_MTK_LDVT
    err = msdkGetFlickerPara(SensorId, r_u4ModuleID, SensorMode, buf);
    if (err)
        logE("nvGetFlickerPara(): error(%d).", err);
#endif
    return err;
}

int nvGetFlickerParaBin(MUINT32 SensorId, MINT32 a_eSensorType, int SensorMode, int BinRatio, void* buf)
{
    logI("nvGetFlickerParaBin(): id(%d), mode(%d), binRatio(%d).",
            SensorId, SensorMode, BinRatio);

    MUINT32 r_u4ModuleID=0;
    NvramDrvBase* nvDrv;
	nvDrv = NvramDrvBase::createInstance();

    //read module id from EEPROM
	r_u4ModuleID=nvDrv->readModuleIdFromEEPROM((CAMERA_DUAL_CAMERA_SENSOR_ENUM)a_eSensorType, -1, MTRUE);
	nvDrv->destroyInstance();

    int err = 0;
#ifndef USING_MTK_LDVT
    err = msdkGetFlickerPara(SensorId, r_u4ModuleID, SensorMode, buf, BinRatio);
    if (err)
        logE("nvGetFlickerParaBin(): error(%d).", err);
#endif
    return err;
}

int nvGetFlickerParaAll(MUINT32 SensorId, MINT32 a_eSensorType, int SensorMode, int BinRatio, FLICKER_CUST_PARA*** buf)
{
    logI("nvGetFlickerParaBin(): id(%d), mode(%d), binRatio(%d).",
            SensorId, SensorMode, BinRatio);

    MUINT32 r_u4ModuleID=0;
    NvramDrvBase* nvDrv;
	nvDrv = NvramDrvBase::createInstance();

    //read module id from EEPROM
	r_u4ModuleID=nvDrv->readModuleIdFromEEPROM((CAMERA_DUAL_CAMERA_SENSOR_ENUM)a_eSensorType, -1, MTRUE);
	nvDrv->destroyInstance();

    int err = 0;
#ifndef USING_MTK_LDVT
    err = msdkGetFlickerParaAll(SensorId, r_u4ModuleID, SensorMode, buf, BinRatio);
    if (err)
        logE("nvGetFlickerParaBin(): error(%d).", err);
#endif
    return err;
}


int NvramDrv::readNvrameEx(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
                          unsigned long u4SensorID,
                          CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
                          void *a_pNvramData,
                          unsigned long a_u4NvramDataSize,
                          int version)
{
    int err = NVRAM_NO_ERROR;
    logI("sensor type(%d); NVRAM data type(%d)", a_eSensorType, a_eNvramDataType);
    if(a_pNvramData==0)
    {
        logI("buf adr error (=0)");
        return NVRAM_READ_PARAMETER_ERROR;
    }
    if(a_eNvramDataType < CAMERA_DATA_TYPE_START || a_eNvramDataType>=CAMERA_DATA_TYPE_NUM)
    {
        logI("date type id error");
        return NVRAM_READ_PARAMETER_ERROR;
    }
    if(a_u4NvramDataSize != g_u4NvramDataSize[a_eNvramDataType])
    {
        logI("buf size is error");
        return NVRAM_READ_PARAMETER_ERROR;
    }

    if( a_eNvramDataType==CAMERA_DATA_PDC_TABLE)
    {
        err = readDefaultData(a_eSensorType, u4SensorID, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR)
                logE("read default PDC Table error!");
          return err;
    }

    if( a_eNvramDataType==CAMERA_NVRAM_IDX_TBL)
    {
        err = readDefaultData(a_eSensorType, u4SensorID, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR)
                logE("read default Idx Table error!");
          return err;
    }

    std::lock_guard<std::mutex> lock(m_Lock);
    if(checkDataVersionNew(a_eNvramDataType, version)==NVRAM_NO_ERROR)
    {
        switch(a_eNvramDataType)
        {
        case CAMERA_NVRAM_DATA_ISP:
        case CAMERA_NVRAM_DATA_3A:
        case CAMERA_NVRAM_DATA_SHADING:
        case CAMERA_NVRAM_DATA_LENS:
        case CAMERA_NVRAM_DATA_STROBE:
        case CAMERA_DATA_TSF_TABLE:
        case CAMERA_NVRAM_DATA_GEOMETRY:
        case CAMERA_NVRAM_DATA_FOV:
        case CAMERA_NVRAM_DATA_FEATURE:
        case CAMERA_DATA_AE_PLINETABLE:
        case CAMERA_NVRAM_DATA_AF:
        case CAMERA_NVRAM_DATA_FLASH_CALIBRATION:
            err = readNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);
            if (err != NVRAM_NO_ERROR)
            {
                logE("read data error! ==> readDefaultData()");
                err = readDefaultData(a_eSensorType, u4SensorID, a_eNvramDataType, a_pNvramData);
                if (err != NVRAM_NO_ERROR)
                    logE("read defaultData error!");
            }
        default:
        break;
        }

    }
    else
    {
        err = readDefaultData(a_eSensorType, u4SensorID, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR)
            logE("read defaultData error! 2");
        return err;
    }
    return err;
}

int
NvramDrv::readNvram(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    unsigned long a_u4SensorID,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData,
    unsigned long a_u4NvramDataSize
)
{
    int err = NVRAM_NO_ERROR;

    logI("sensor type = %d; NVRAM data type = %d\n", a_eSensorType, a_eNvramDataType);

    if ((a_eSensorType > DUAL_CAMERA_MAIN_SECOND_SENSOR) ||
        (a_eSensorType < DUAL_CAMERA_MAIN_SENSOR) ||
        //(a_eNvramDataType > CAMERA_DATA_AE_PLINETABLE) ||
        (a_eNvramDataType >= CAMERA_DATA_TYPE_NUM) ||
        (a_eNvramDataType < CAMERA_NVRAM_DATA_ISP) ||
        (a_pNvramData == NULL) ||
        (a_u4NvramDataSize != g_u4NvramDataSize[a_eNvramDataType]))
    {
        logE("read parameter error!");
        return NVRAM_READ_PARAMETER_ERROR;
    }



    std::lock_guard<std::mutex> lock(m_Lock);

    switch(a_eNvramDataType) {
    case CAMERA_NVRAM_DATA_ISP:
    case CAMERA_NVRAM_DATA_3A:
    case CAMERA_NVRAM_DATA_SHADING:
    case CAMERA_NVRAM_DATA_LENS:
    case CAMERA_NVRAM_DATA_STROBE:
    case CAMERA_DATA_TSF_TABLE:
    case CAMERA_DATA_AE_PLINETABLE:
    case CAMERA_NVRAM_DATA_FOV:
    case CAMERA_NVRAM_DATA_AF:
    case CAMERA_NVRAM_DATA_FLASH_CALIBRATION:
        err = readNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR) {
            logE("read data error! ==> readDefaultData()");
            err = readDefaultData(a_eSensorType, a_u4SensorID, a_eNvramDataType, a_pNvramData);
            if (err != NVRAM_NO_ERROR) {
                logE("read defaultData error!");
            }
            break;
        }

        if (checkDataVersion(a_eNvramDataType, a_pNvramData) != NVRAM_NO_ERROR) {
            err = readDefaultData(a_eSensorType,a_u4SensorID, a_eNvramDataType, a_pNvramData);
            if (err != NVRAM_NO_ERROR) {
                logE("read defaultData error! 2");
            }
        }
        break;
    case CAMERA_DATA_PDC_TABLE:
        err = readDefaultData(a_eSensorType,a_u4SensorID, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR) {
            logE("read default TSF Table error!");
        }
        break;
    default:
        break;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::writeNvram(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    unsigned long a_u4SensorID,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData,
    unsigned long a_u4NvramDataSize
)
{
    int err = NVRAM_NO_ERROR;
    logI("sensor type(%d); a_u4SensorID(%lu), NVRAM data type(%d)", a_eSensorType, a_u4SensorID, a_eNvramDataType);
    if(a_pNvramData==0)
    {
        logI("buf adr error (=0)");
        return NVRAM_READ_PARAMETER_ERROR;
    }
    if(a_eNvramDataType < CAMERA_DATA_TYPE_START || a_eNvramDataType>=CAMERA_DATA_TYPE_NUM)
    {
        logI("date type id error!");
        return NVRAM_READ_PARAMETER_ERROR;
    }
    if(a_u4NvramDataSize != g_u4NvramDataSize[a_eNvramDataType])
    {
        logI("buf size is error!");
        return NVRAM_READ_PARAMETER_ERROR;
    }

    std::lock_guard<std::mutex> lock(m_Lock);
    err = writeNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);
    return err;
}
