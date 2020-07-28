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
#define LOG_TAG "nvbuf_util_dep"

#include <mtkcam/utils/std/Log.h>
#include <mutex>

#define logW(fmt, arg...)    CAM_LOGW("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
#define logI(fmt, arg...)    CAM_LOGD("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
#define logE(fmt, arg...)    CAM_LOGE("[%s:%d] MError: " fmt, __FUNCTION__, __LINE__, ##arg)

#include <nvram_drv.h>
#include "nvram_drv_imp.h"
#include <aaa_types.h>
#include "flash_param.h"
#include "flash_tuning_custom.h"

#include <nvbuf_util.h>

#include "nvbuf_util_dep.h"

#include <mtkcam/drv/IHalSensor.h>

#ifdef NVRAM_SUPPORT
#include "camera_custom_msdk.h"
#endif

#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSCam;

int g_nvramArrInd[e_NvramTypeNum]=
{
    (int)CAMERA_NVRAM_DATA_ISP,
    (int)CAMERA_NVRAM_DATA_3A,
    (int)CAMERA_NVRAM_DATA_SHADING,
    (int)CAMERA_NVRAM_DATA_LENS,
    (int)CAMERA_DATA_AE_PLINETABLE,
    (int)CAMERA_NVRAM_DATA_STROBE,
    (int)CAMERA_DATA_TSF_TABLE,
    (int)CAMERA_DATA_PDC_TABLE,
    (int)CAMERA_NVRAM_DATA_GEOMETRY,
    (int)CAMERA_NVRAM_DATA_FOV,
    (int)CAMERA_NVRAM_DATA_FEATURE,
    (int)CAMERA_NVRAM_DATA_AF,
    (int)CAMERA_NVRAM_DATA_FLASH_CALIBRATION,
    (int)CAMERA_NVRAM_IDX_TBL,
};

int g_nvramSize[e_NvramTypeNum]=
{
    (int)sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
    (int)sizeof(NVRAM_CAMERA_3A_STRUCT),
    (int)sizeof(NVRAM_CAMERA_SHADING_STRUCT),
    (int)sizeof(NVRAM_LENS_PARA_STRUCT),
    (int)sizeof(AE_PLINETABLE_T),
    (int)sizeof(NVRAM_CAMERA_STROBE_STRUCT),
    (int)sizeof(CAMERA_TSF_TBL_STRUCT),
    (int)sizeof(CAMERA_BPCI_STRUCT),
    (int)sizeof(NVRAM_CAMERA_GEOMETRY_STRUCT),
    (int)sizeof(NVRAM_CAMERA_FOV_STRUCT),
    (int)sizeof(NVRAM_CAMERA_FEATURE_STRUCT),
    (int)sizeof(NVRAM_AF_PARA_STRUCT),
    (int)sizeof(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT),
    (int)sizeof(NVRAM_CAMERA_IDX_STRUCT),
};

// [ Debug ] for dump buf using
#define MAX_FILE_PATH_LEN 100
#define MAX_FILE_PRE_PATH_LEN 64
char gCreateDirD[MAX_FILE_PATH_LEN] = "mnt/vendor/nvcfg/camera/debug";
char gFilePathD[CAMERA_DATA_TYPE_NUM][MAX_FILE_PRE_PATH_LEN]=
{
    "mnt/vendor/nvcfg/camera/debug/nv_isp",
    "mnt/vendor/nvcfg/camera/debug/nv_3a",
    "mnt/vendor/nvcfg/camera/debug/nv_shading",
    "mnt/vendor/nvcfg/camera/debug/nv_lens",
    "mnt/vendor/nvcfg/camera/debug/nv_pline",
    "mnt/vendor/nvcfg/camera/debug/nv_strobe",
    "mnt/vendor/nvcfg/camera/debug/nv_tsf",
    "mnt/vendor/nvcfg/camera/debug/nv_pdc",
    "mnt/vendor/nvcfg/camera/debug/nv_geometry",
    "mnt/vendor/nvcfg/camera/debug/nv_fov",
    "mnt/vendor/nvcfg/camera/debug/nv_idx_tbl",
    "mnt/vendor/nvcfg/camera/debug/nv_feature",
    "mnt/vendor/nvcfg/camera/debug/nv_af_calibration",
    "mnt/vendor/nvcfg/camera/debug/nv_flash_calibration",
    "mnt/vendor/nvcfg/camera/debug/nv_version",
};
//

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

void getNvFileNameU(int dev, int id, char* outName)
{
    auto facing = 0; auto num = 0;
    NS3Av3::mapSensorIdxToFace(dev, facing, num);

    strncpy(outName, gFilePath[id], MAX_FILE_PRE_PATH_LEN);
    strncat(outName, gFacingName[facing], sizeof(gFacingName[facing]));

    char devSuffix[20] = {'\0'};
    if(num < 3) sprintf(devSuffix, "%s", gFacingNum[num]);
    else sprintf(devSuffix, "_%d", num);

    strncat(outName, devSuffix, sizeof(devSuffix));

    logI("outName(%s)", outName);
}

int getSensorID(CAMERA_DUAL_CAMERA_SENSOR_ENUM i4SensorDev, int& sensorId)
{
    sensorId = NS3Av3::mapSensorDevToSensorId(static_cast<MINT32>(i4SensorDev));

#ifdef WIN32
    sensorId=100;
#endif
    if(sensorId==0)
    {
        logE("sensorId(%d)",0);
        return NvBufUtil::e_NV_SensorIdNull;
    }
    return 0;
}


extern int getVerMem(void*& buf);

// for camera version use
int readVerNvramNoLock(void*& p)
{
    int err=0;
    err = getVerMem(p);
    if(err!=0)
        return err;
    if(g_isVerNvBufRead==1)
    {
        return 0;
    }
    int nvSize;
    nvSize = (int)sizeof(NVRAM_CAMERA_VERSION_STRUCT);
    //----------------------------
    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    if(nvDrv->isNvramExist(DUAL_CAMERA_MAIN_SENSOR, CAMERA_NVRAM_VERSION))
    {
    err  = nvDrv->readNoDefault(
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)DUAL_CAMERA_MAIN_SENSOR, CAMERA_NVRAM_VERSION, p, nvSize);
    }
    nvDrv->destroyInstance();
    g_isVerNvBufRead=1;
    return err;
}
int writeVerNvramNoLock()
{
    int err;
    void* buf;
    err = getVerMem(buf);
    if(err!=0)
        return err;
    int nvSize;
    nvSize = (int)sizeof(NVRAM_CAMERA_VERSION_STRUCT);
    //----------------------------
    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->writeNvram(
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)DUAL_CAMERA_MAIN_SENSOR, 0, CAMERA_NVRAM_VERSION,
            buf, nvSize );
    nvDrv->destroyInstance();
    return err;
}
//////

//In order to prevent from twice allocation in singleton
static std::mutex g_singletonLock;

////////////////////////////////////////////////////
template <class T>
static T* getMemT(CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev)
{
    static std::array<NS3Av3::INST_T<T>, SENSOR_IDX_MAX> gMultiton;

    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(sensorDev));

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return NULL;
    }

    NS3Av3::INST_T<T>& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<T>();
    } );

    return rSingleton.instance.get();
}

template <class T>
static int getMemDataType(CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void*& m)
{
    m = getMemT<T>(sensorDev);
    if(!m)
    {
        return NvBufUtil::e_NV_SensorDevWrong;
    }
    return 0;
}

int getMem(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void*& buf)
{
    if(nvRamId==CAMERA_NVRAM_DATA_ISP)      return getMemDataType<NVRAM_CAMERA_ISP_PARAM_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_3A)      return getMemDataType<NVRAM_CAMERA_3A_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_SHADING)     return getMemDataType<NVRAM_CAMERA_SHADING_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_LENS)        return getMemDataType<NVRAM_LENS_PARA_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_DATA_AE_PLINETABLE)     return getMemDataType<AE_PLINETABLE_T>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)      return getMemDataType<NVRAM_CAMERA_STROBE_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_DATA_TSF_TABLE)     return getMemDataType<CAMERA_TSF_TBL_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_DATA_PDC_TABLE)     return getMemDataType<CAMERA_BPCI_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_FOV)     return getMemDataType<NVRAM_CAMERA_FOV_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_FEATURE)     return getMemDataType<NVRAM_CAMERA_FEATURE_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_GEOMETRY)        return getMemDataType<NVRAM_CAMERA_GEOMETRY_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_AF)      return getMemDataType<NVRAM_AF_PARA_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_FLASH_CALIBRATION)       return getMemDataType<NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_IDX_TBL)		return getMemDataType<NVRAM_CAMERA_IDX_STRUCT>(sensorDev, buf);

    return NvBufUtil::e_NvramIdWrong;
}


extern int readVerNvramNoLock(void*& p);


int readRamVersion(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, int& version)
{
    int err;
    void* buf;

    err = readVerNvramNoLock(buf);
    if(err!=0)
    {
        logI("readVerNvramNoLock error!, err(%d)", err);
        return err;
    }
    NVRAM_CAMERA_VERSION_STRUCT* verStruct = (NVRAM_CAMERA_VERSION_STRUCT*)buf;

    auto devArrInd = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(sensorDev));

    if(devArrInd<0)
        return NvBufUtil::e_SensorDevWrong;

    logI("nvRamId(%d), ver: isp(%d),3a(%d),sh(%d),lens(%d),pl(%d),stb(%d),tsf(%d),pdc(%d),geo(%d),fov(%d),feature(%d)",nvRamId,
        verStruct->ispVer[devArrInd], verStruct->aaaVer[devArrInd], verStruct->shadingVer[devArrInd],
        verStruct->lensVer[devArrInd], verStruct->aePlineVer[devArrInd], verStruct->strobeVer[devArrInd],
        verStruct->tsfVer[devArrInd], verStruct->pdcVer[devArrInd], verStruct->geometryVer[devArrInd],
        verStruct->fovVer[devArrInd], verStruct->featureVer[devArrInd]);

    if(nvRamId==CAMERA_NVRAM_DATA_ISP)          {version = verStruct->ispVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_3A)      {version = verStruct->aaaVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_SHADING) {version = verStruct->shadingVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_LENS)    {version = verStruct->lensVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_DATA_AE_PLINETABLE) {version = verStruct->aePlineVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)  {version = verStruct->strobeVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_DATA_TSF_TABLE) {version = verStruct->tsfVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_DATA_PDC_TABLE) {version = verStruct->pdcVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_GEOMETRY) {version = verStruct->geometryVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FOV) {version = verStruct->fovVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FEATURE)   {version = verStruct->featureVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_AF) {version = verStruct->afVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FLASH_CALIBRATION)   {version = verStruct->flashCalibrationVer[devArrInd]; return 0;}
    else if(nvRamId==CAMERA_NVRAM_IDX_TBL) {version = verStruct->idxTblVer[devArrInd]; return 0;}

    return NvBufUtil::e_NvramIdWrong;
}


extern int writeVerNvramNoLock();

int writeRamVersion(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, int version)
{
    logI("nvId(%d), ver(%d)", (int)nvRamId, version);
    int err;
    void* buf;

    err = getVerMem(buf);
    if(err!=0)
        return err;

    auto devArrInd = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(sensorDev));

    if(devArrInd<0)
        return NvBufUtil::e_SensorDevWrong;

    NVRAM_CAMERA_VERSION_STRUCT* verStruct = (NVRAM_CAMERA_VERSION_STRUCT*)buf;
    if(nvRamId==CAMERA_NVRAM_DATA_ISP)          {verStruct->ispVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_3A)      {verStruct->aaaVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_SHADING) {verStruct->shadingVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_LENS)    {verStruct->lensVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_DATA_AE_PLINETABLE) {verStruct->aePlineVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)  {verStruct->strobeVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_DATA_TSF_TABLE) {verStruct->tsfVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_DATA_PDC_TABLE) {verStruct->pdcVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_GEOMETRY)   {verStruct->geometryVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FOV) {verStruct->fovVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FEATURE)   {verStruct->featureVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_AF)   {verStruct->afVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FLASH_CALIBRATION)   {verStruct->flashCalibrationVer[devArrInd]=version;}

    else return NvBufUtil::e_NvramIdWrong;
    err = writeVerNvramNoLock();
    return err;
}


int writeRamUpdatedVersion(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev)
{
    int err;
    void* buf;
    err = getVerMem(buf);
    if(err!=0)
        return err;

    auto devArrInd = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(sensorDev));

    if(devArrInd<0)
        return NvBufUtil::e_SensorDevWrong;

    NVRAM_CAMERA_VERSION_STRUCT* verStruct = (NVRAM_CAMERA_VERSION_STRUCT*)buf;
    unsigned char* buf2;
    buf2  = (unsigned char*)buf;
    logI("qq %d %d %d %d %d %d %d %d %d %d buf(%p)",
        buf2[0], buf2[1], buf2[2], buf2[3], buf2[4],
        buf2[5], buf2[6], buf2[7], buf2[8], buf2[9], buf
        );

    if(nvRamId==CAMERA_NVRAM_DATA_ISP)          {verStruct->ispVer[devArrInd]=NVRAM_CAMERA_PARA_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_3A)      {verStruct->aaaVer[devArrInd]=NVRAM_CAMERA_3A_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_SHADING) {verStruct->shadingVer[devArrInd]=NVRAM_CAMERA_SHADING_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_LENS)    {verStruct->lensVer[devArrInd]=NVRAM_CAMERA_LENS_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)  {verStruct->strobeVer[devArrInd]=NVRAM_CAMERA_STROBE_FILE_VERSION;}
    else if(nvRamId==CAMERA_DATA_TSF_TABLE)     {verStruct->tsfVer[devArrInd]=NVRAM_CAMERA_TSF_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_GEOMETRY)   {verStruct->geometryVer[devArrInd]=NVRAM_CAMERA_GEOMETRY_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FEATURE)   {verStruct->featureVer[devArrInd]=NVRAM_CAMERA_FEATURE_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FOV)   {verStruct->fovVer[devArrInd]=NVRAM_CAMERA_FOV_FILE_VERSION;}
    else if(nvRamId==CAMERA_DATA_AE_PLINETABLE)  {verStruct->aePlineVer[devArrInd]=NVRAM_CAMERA_PLINE_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_AF)   {verStruct->afVer[devArrInd]=NVRAM_CAMERA_AF_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FLASH_CALIBRATION)   {verStruct->flashCalibrationVer[devArrInd]=NVRAM_CAMERA_FLASH_CALIBRATION_FILE_VERSION;}

    else return NvBufUtil::e_NvramIdWrong;

    logI("qq %d %d %d %d %d %d %d %d %d %d buf2(%p)",
        buf2[0], buf2[1], buf2[2], buf2[3], buf2[4],
        buf2[5], buf2[6], buf2[7], buf2[8], buf2[9], buf);

    err = writeVerNvramNoLock();

    logI("- nvRamId(%d)", nvRamId);
    return err;
}
