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

#define LOG_TAG "NvramDrv"

#include <sys/types.h>
#include <sys/stat.h>
#include <utils/Errors.h>
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <fcntl.h>
#include <aaa_types.h>
#include <nvram_drv.h>
#include "nvram_drv_imp.h"
#include "flash_param.h"
#include "flash_tuning_custom.h"

#ifdef NVRAM_SUPPORT
#include "camera_custom_msdk.h"
#endif

/*******************************************************************************
*
********************************************************************************/

/*******************************************************************************
*
********************************************************************************/
#define logW(fmt, arg...)    CAM_LOGW("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
#define logI(fmt, arg...)    CAM_LOGD("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
#define logE(fmt, arg...)    CAM_LOGE("[%s:%d] MError: " fmt, __FUNCTION__, __LINE__, ##arg)

using namespace android;
/*******************************************************************************
*
********************************************************************************/
static unsigned long const g_u4NvramDataSize[CAMERA_DATA_TYPE_NUM] =
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
    sizeof(NVRAM_CAMERA_FEATURE_STRUCT),
    sizeof(NVRAM_AF_PARA_STRUCT),
    sizeof(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT),
    sizeof(NVRAM_CAMERA_VERSION_STRUCT),
};


#define MAX_FILE_PATH_LEN 100
#define MAX_FILE_PRE_PATH_LEN 64
static char gCreateDir[MAX_FILE_PATH_LEN] = "mnt/vendor/nvcfg/camera";
static char gFilePath[CAMERA_DATA_TYPE_NUM][MAX_FILE_PRE_PATH_LEN]=
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
    "mnt/vendor/nvcfg/camera/nv_feature",
    "mnt/vendor/nvcfg/camera/nv_af_calibration",
    "mnt/vendor/nvcfg/camera/nv_flash_calibration",
    "mnt/vendor/nvcfg/camera/nv_version",
};

static bool gIsNvInit=0;
static Mutex gNvLock;

static void getNvFileName(int dev, int id, char* outName)
{
    char devSuffix[20] = {'\0'};
    strncpy(outName, gFilePath[id], MAX_FILE_PRE_PATH_LEN);

    if(dev==DUAL_CAMERA_MAIN_SENSOR)
        sprintf(devSuffix, "_main");
    else if(dev==DUAL_CAMERA_SUB_SENSOR)
        sprintf(devSuffix, "_sub");
    else if(dev==DUAL_CAMERA_MAIN_SECOND_SENSOR)
        sprintf(devSuffix, "_main_sec");
     else if(dev==DUAL_CAMERA_SUB_2_SENSOR)
        sprintf(devSuffix, "_sub_sec");
    strncat(outName, devSuffix, sizeof(devSuffix));
    logI("outName(%s)", outName);
}

static int writeNv(int dev, int id, void* buf)
{
    logI("dev(%d) id(%d) buf(%p)",dev, id, buf);
    Mutex::Autolock lock(gNvLock);
    int mask = umask(0);
    logI("umask(%d)",mask);
    if(gIsNvInit==0)
    {
        int ret;
        ret = mkdir("/mnt/vendor/nvcfg/camera", S_IRWXU | S_IRWXG | S_IXOTH);
        logI("mkdir ret(%d)",ret);
        gIsNvInit = 1;
    }
    char fname[MAX_FILE_PATH_LEN];
    getNvFileName(dev, id, fname);
    FILE* fp;
    fp = fopen(fname,"wb");
    if(fp==0)
    {
        logE("fopen fail");
        umask(mask);
        return -1;
    }

    fwrite(buf,1,g_u4NvramDataSize[id],fp);
    fclose(fp);
    umask(mask);
    return 0;
}

static int readNv(int dev, int id, void* buf)
{
    logI("dev(%d) id(%d) buf(%p)",dev, id, buf);
    Mutex::Autolock lock(gNvLock);
    int mask = umask(0);
    logI("umask(%d)",mask);
    if(gIsNvInit==0)
    {
        int ret;
        ret = mkdir("/mnt/vendor/nvcfg/camera", S_IRWXU | S_IRWXG | S_IXOTH);
        logI("mkdir ret(%d)",ret);
        gIsNvInit = 1;
    }
    char fname[MAX_FILE_PATH_LEN];
    getNvFileName(dev, id, fname);
    FILE* fp;
    fp = fopen(fname,"rb");
    if(fp==0)
    {
        logE("fopen fail");
        umask(mask);
        return -1;
    }
    int ret = 0;
    ret = fread(buf,1,g_u4NvramDataSize[id],fp);
    if ((unsigned long)ret != g_u4NvramDataSize[id])
    {
        fclose(fp);
        logE("read fail");
        umask(mask);
        return -1;
    }
    fclose(fp);
    umask(mask);
    return 0;
}

static int getMs()
{
    int t;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t = (ts.tv_sec*1000+ts.tv_nsec/1000000);
    return t;
}

static int isNvExist(int dev, int id)
{
    logI("dev(%d) id(%d)",dev, id);
    Mutex::Autolock lock(gNvLock);
    FILE* fp;
    char fname[MAX_FILE_PATH_LEN];
    getNvFileName(dev, id, fname);
    fp = fopen(fname,"rb");
    if(fp==0)
    {
        logI("return 0");
        return 0;
    }
    fclose(fp);
    logI("return 1");
    return 1;
}



static bool bCustomInit = 0; //[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
/*******************************************************************************
*
********************************************************************************/
NvramDrvBase*
NvramDrvBase::createInstance()
{
    return NvramDrv::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
NvramDrvBase*
NvramDrv::getInstance()
{
    static NvramDrv singleton;
    return &singleton;
}

int NvramDrv::isNvramExist(int dev, CAMERA_DATA_TYPE_ENUM id)
{
    return isNvExist(dev, id);
}

/*******************************************************************************
*
********************************************************************************/
void
NvramDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
NvramDrv::NvramDrv()
    : NvramDrvBase()
{
}

/*******************************************************************************
*
********************************************************************************/
NvramDrv::~NvramDrv()
{
}

/*******************************************************************************
*
********************************************************************************/

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

unsigned int
NvramDrv::
readModuleIdFromEEPROM(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType, MINT32 r_i4SensorIdx, int forceWait)
{
    logW("[%s] this function not support yet", __func__);

    return 0;
}

int NvramDrv::readNoDefault(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
                          CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
                          void *a_pNvramData,
                          unsigned long a_u4NvramDataSize)
{
    int err = NVRAM_NO_ERROR;
    logI("id(%d) dev(%d) a_u4NvramDataSize(%lu)",a_eSensorType,a_eNvramDataType, a_u4NvramDataSize);
    err = readNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);

    if (err != NVRAM_NO_ERROR)
        logE("err(%d)", err);
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
    if(a_eSensorType == DUAL_CAMERA_MAIN_SENSOR || a_eSensorType == DUAL_CAMERA_SUB_SENSOR || a_eSensorType == DUAL_CAMERA_MAIN_SECOND_SENSOR || a_eSensorType == DUAL_CAMERA_SUB_2_SENSOR)
       ;
    else
    {
        logI("sensorId error");
        return NVRAM_READ_PARAMETER_ERROR;
    }
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


    if( a_eNvramDataType==CAMERA_DATA_TSF_TABLE)
    {
        err = readDefaultData(a_eSensorType, u4SensorID, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR)
                logE("read default TSF Table error!");
          return err;
    }

    if( a_eNvramDataType==CAMERA_DATA_PDC_TABLE)
    {
        err = readDefaultData(a_eSensorType, u4SensorID, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR)
                logE("read default PDC Table error!");
          return err;
    }

    Mutex::Autolock lock(mLock);
    if(checkDataVersionNew(a_eNvramDataType, version)==NVRAM_NO_ERROR)
    {
        switch(a_eNvramDataType)
        {
        case CAMERA_NVRAM_DATA_ISP:
        case CAMERA_NVRAM_DATA_3A:
        case CAMERA_NVRAM_DATA_SHADING:
        case CAMERA_NVRAM_DATA_LENS:
        case CAMERA_NVRAM_DATA_STROBE:
        case CAMERA_NVRAM_DATA_GEOMETRY:
        case CAMERA_NVRAM_DATA_FOV:
        case CAMERA_NVRAM_DATA_FEATURE:
        case CAMERA_DATA_AE_PLINETABLE:
        case CAMERA_NVRAM_DATA_AF:
        case CAMERA_NVRAM_DATA_FLASH_CALIBRATION:
            err = readNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);
            if (err != NVRAM_NO_ERROR)
            {
                logE("read data error(%d)! ==> readDefaultData()", err);
                err = readDefaultData(a_eSensorType, u4SensorID, a_eNvramDataType, a_pNvramData);
                if (err != NVRAM_NO_ERROR)
                    logE("read defaultData error(%d)!", err);
            }
        default:
        break;
        }

    }
    else
    {
        err = readDefaultData(a_eSensorType, u4SensorID, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR)
            logE("read defaultData error(%d)! 2", err);
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



    Mutex::Autolock lock(mLock);

    switch(a_eNvramDataType) {
    case CAMERA_NVRAM_DATA_ISP:
    case CAMERA_NVRAM_DATA_3A:
    case CAMERA_NVRAM_DATA_SHADING:
    case CAMERA_NVRAM_DATA_LENS:
    case CAMERA_NVRAM_DATA_STROBE:
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
    case CAMERA_DATA_TSF_TABLE:
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
    if(a_eSensorType == DUAL_CAMERA_MAIN_SENSOR || a_eSensorType == DUAL_CAMERA_SUB_SENSOR || a_eSensorType == DUAL_CAMERA_MAIN_SECOND_SENSOR || a_eSensorType == DUAL_CAMERA_SUB_2_SENSOR)
       ;
    else
    {
        logI("sensorId error!");
        return NVRAM_READ_PARAMETER_ERROR;
    }
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

    Mutex::Autolock lock(mLock);
    err = writeNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);
    return err;
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


/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::readNvramData(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData
)
{
    logI("a_eSensorType(%d), a_eNvramDataType(%d)", a_eSensorType, a_eNvramDataType);
//seanlin 121221 avoid camera has not inited>
//[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
    if (!bCustomInit) {
        #ifndef USING_MTK_LDVT
        cameraCustomInit();
        if((a_eSensorType==DUAL_CAMERA_MAIN_2_SENSOR) || (a_eSensorType==DUAL_CAMERA_MAIN_SECOND_SENSOR))
            LensCustomInit(8);
        else
            LensCustomInit((unsigned int)a_eSensorType);
        #endif
        bCustomInit = 1;
    }
//[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
//seanlin 121221 avoid camera has not inited<

    int ret = 0;
#ifdef NVRAM_SUPPORT

    ret = readNv(a_eSensorType, a_eNvramDataType, a_pNvramData);
    if(ret != 0)
    {
        return ret;
    }

#endif
    unsigned char* dd;
    dd = (unsigned char*)a_pNvramData;
    logI("%d %d %d %d %d %d %d %d %d %d",
        (int)dd[0], (int)dd[1], (int)dd[2], (int)dd[3], (int)dd[4],
        (int)dd[5], (int)dd[6], (int)dd[7], (int)dd[8], (int)dd[9]);

    return NVRAM_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::writeNvramData(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData
)
{
    logI("+ a_eSensorType(%d), a_eNvramDataType(%d)", a_eSensorType, a_eNvramDataType);
    unsigned char* dd;
    dd = (unsigned char*)a_pNvramData;
    logI("%d %d %d %d %d %d %d %d %d %d",
        (int)dd[0], (int)dd[1], (int)dd[2], (int)dd[3], (int)dd[4],
        (int)dd[5], (int)dd[6], (int)dd[7], (int)dd[8], (int)dd[9]);

    int ret = 0;
#ifdef NVRAM_SUPPORT

    ret = writeNv(a_eSensorType, a_eNvramDataType, a_pNvramData);
    if(ret != 0)
    {
        return ret;
    }

#endif
    logI("-");

    return NVRAM_NO_ERROR;
}

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

//    static bool bCustomInit = 0; //[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
    logI("sensor ID(%ld); NVRAM data type(%d)", a_u4SensorID, a_eNvramDataType);

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
        logW("CAMERA_NVRAM_DATA_ISP");
        GetCameraDefaultPara(a_u4SensorID, (PNVRAM_CAMERA_ISP_PARAM_STRUCT)a_pNvramData,NULL,NULL,NULL);
        break;
    case CAMERA_NVRAM_DATA_3A:
        logW("CAMERA_NVRAM_DATA_3A");
        GetCameraDefaultPara(a_u4SensorID, NULL,(PNVRAM_CAMERA_3A_STRUCT)a_pNvramData,NULL,NULL);
        break;
    case CAMERA_NVRAM_DATA_SHADING:
        logW("CAMERA_NVRAM_DATA_SHADING");
        GetCameraDefaultPara(a_u4SensorID, NULL,NULL,(PNVRAM_CAMERA_SHADING_STRUCT)a_pNvramData,NULL);
        break;
    case CAMERA_NVRAM_DATA_LENS:
        logW("CAMERA_NVRAM_DATA_LENS");
        GetLensDefaultPara((PNVRAM_LENS_PARA_STRUCT)a_pNvramData);
        {
            PNVRAM_LENS_PARA_STRUCT pLensNvramData = (PNVRAM_LENS_PARA_STRUCT)a_pNvramData;
            pLensNvramData->Version = NVRAM_CAMERA_LENS_FILE_VERSION;
        }
        break;
    case CAMERA_DATA_AE_PLINETABLE:
        logW("CAMERA_DATA_AE_PLINETABLE");
        GetCameraDefaultPara(a_u4SensorID, NULL,NULL,NULL,(PAE_PLINETABLE_STRUCT)a_pNvramData);
        break;

    case CAMERA_NVRAM_DATA_STROBE:
        logW("CAMERA_NVRAM_DATA_STROBE cust_fillDefaultStrobeNVRam");
        int ret;
        ret = cust_fillDefaultStrobeNVRam(a_eSensorType, a_pNvramData);
        break;

    case CAMERA_NVRAM_DATA_FLASH_CALIBRATION:
        logW("CAMERA_NVRAM_DATA_FLASH_CALIBRATION cust_fillDefaultFlashCalibrationNVRam");
        int ret_flashCalibration;
        ret_flashCalibration = cust_fillDefaultFlashCalibrationNVRam(a_eSensorType, a_pNvramData);
        break;

    case CAMERA_DATA_TSF_TABLE:
        logW("CAMERA_DATA_TSF_TABLE");
        if (0 != GetCameraTsfDefaultTbl(a_u4SensorID, (PCAMERA_TSF_TBL_STRUCT)a_pNvramData))
        {
            return NVRAM_DEFAULT_DATA_READ_ERROR;
        }
        break;
    case CAMERA_DATA_PDC_TABLE:
        logW("CAMERA_DATA_PDC_TABLE");
        if (0 != GetCameraBpciDefaultTbl(a_u4SensorID, (PCAMERA_BPCI_STRUCT)a_pNvramData))
        {
            return NVRAM_DEFAULT_DATA_READ_ERROR;
        }
        break;
    case CAMERA_NVRAM_DATA_FEATURE:
        logW("CAMERA_NVRAM_DATA_FEATURE");
        GetCameraFeatureDefault(a_u4SensorID, (NVRAM_CAMERA_FEATURE_STRUCT*)a_pNvramData);
        break;

    default:
        logW("Not support type");
        break;
    }

#endif

    return NVRAM_NO_ERROR;
}

int nvGetFlickerPara(MUINT32 SensorId, int SensorMode, void* buf)
{
    logI("id(%d), mode(%d)", SensorId, SensorMode);
    int err;
    #ifndef USING_MTK_LDVT
    err = msdkGetFlickerPara(SensorId, SensorMode, buf);
    #endif
    if(err!=0)
        logE("err(%d)", err);
    return err;
}

int nvGetFlickerParaAll(MUINT32 SensorId, MINT32 /*a_eSensorType*/, int SensorMode, int BinRatio, FLICKER_CUST_PARA*** buf)
{
    logI("nvGetFlickerParaBin(): id(%d), mode(%d), binRatio(%d).",
            SensorId, SensorMode, BinRatio);
    int err = 0;
#ifndef USING_MTK_LDVT
    err = msdkGetFlickerParaAll(SensorId, SensorMode, buf, BinRatio);
    if (err)
        logE("nvGetFlickerParaBin(): error(%d).", err);
#endif
    return err;
}

int NvramDrv::writeNvramReal(void* buf, int id, int dev)
{
    logI("not support, buf(%p), id(%d), dev(%d)", buf, id, dev);
    return 0;
}

int NvramDrv::readNvramReal(void* buf, int id, int dev)
{
    logI("not support, buf(%p), id(%d), dev(%d)", buf, id, dev);
    return 0;
}

int NvramDrv::readMultiNvram(void* buf, int bufSz, int* idList, int num, int dev)
{
    logI("not support, buf(%p), bufSz(%d), idList(%p), num(%d), dev(%d)", buf, bufSz, (void*)idList, num, dev);
    return 0;
}


int NvramDrv::writeMultiNvram(void* buf, int bufSz, int* idList, int num, int dev)
{
    logI("not support, buf(%p), bufSz(%d), idList(%p), num(%d), dev(%d)", buf, bufSz, (void*)idList, num, dev);
    return 0;
}
