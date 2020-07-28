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
// seanlin test \mediatek\custom\common\hal\camera\camera\camera_custom_msdk.cpp
//#include "MediaLog.h"
#include "camera_custom_msdk.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
//#include "camera_tuning_para.h"
//#include "msdk_sensor_exp.h"

#include "camera_custom_sensor.h"

#include <string.h>
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <aee.h>

#include <unordered_map>
#include <mutex>

#include <mtkcam/drv/mem/cam_cal_drv.h>

#define MTK_LOG_ENABLE 1

#undef LOG_TAG
#define LOG_TAG "CAM_CUS_MSDK"

#define CAM_MSDK_LOG(fmt, arg...)    ALOGD(LOG_TAG " " fmt, ##arg)
#define CAM_MSDK_ERR(fmt, arg...)    ALOGE(LOG_TAG "Err: %5d: " fmt, __LINE__, ##arg)

#define CAM_MSDK_ASSERT aee_system_warning(LOG_TAG, NULL, DB_OPT_DEFAULT, "Do not allow open custom library twice");


MSDK_SENSOR_INIT_FUNCTION_STRUCT *pstSensorInitFunc = NULL;
MSDK_LENS_INIT_FUNCTION_STRUCT LensInitFunc[MAX_NUM_OF_SUPPORT_LENS+1] =
{ {0,0,0,{0},NULL},
  {0,0,0,{0},NULL},
  {0,0,0,{0},NULL},
  {0,0,0,{0},NULL},
};

MUINT32 gMainLensIdx;
MUINT32 gSubLensIdx;

typedef fptrDefault (*dlGetFuncDefault)(MUINT32 sensorType);
typedef fptrFlicker (*dlGetFuncFlicker)(MUINT32 sensorType);

static std::mutex g_mapLock;
static std::unordered_map<MUINT64, MBOOL> g_hadOpen;
static std::unordered_map<MUINT64, MBOOL> g_isLocked;
static std::unordered_map<MUINT64, void *> g_handleMap;
static std::unordered_map<MUINT64, fptrDefault> g_defaultMap;
static std::unordered_map<MUINT64, fptrFlicker> g_flickerMap;

#if 0
void GetDSCSupportInfo(PDSC_INFO_STRUCT pDSCSupportInfo)
{
	memcpy(pDSCSupportInfo, &DSCSupportInfo, sizeof(DSC_INFO_STRUCT));
}	/* GetDSCSupportInfo() */

void GETCameraDeviceSupportInfo(PDEVICE_INFO_STRUCT pDeviceSupportInfo)
{
	memcpy(pDeviceSupportInfo, &DeviceSupportInfo, sizeof(DEVICE_INFO_STRUCT));
} /* GETCameraDeviceSupportInfo() */
#endif

/*******************************************************************************
*
********************************************************************************/
MUINT32 cameraCustomInit()
{
    GetSensorInitFuncList(&pstSensorInitFunc);
    return 0;
}

//This function may be modify according to different customer
MINT64 compareSensorIdAndModuleId(MUINT32 r_u4SensorId, MUINT32 r_u8ModuleId)
{
    MINT64 defaultIndex=-1;
    MINT64 matchIndex=-1;

    //compare find the index of this module id and sensor id
    UINT32 i;
    for (i=0;i<MAX_NUM_OF_SUPPORT_SENSOR;i++)
    {
        if (pstSensorInitFunc[i].SensorId == 0){
            CAM_MSDK_LOG("[%s] last item in sensorList found.  i(%u)", __func__, i);
            break;
        }
        if (r_u4SensorId == pstSensorInitFunc[i].SensorId && 0==pstSensorInitFunc[i].moduleId )
        {
            defaultIndex=i;
            CAM_MSDK_LOG("[%s] SID %X is found", __func__, pstSensorInitFunc[i].SensorId);
        }

        if (r_u4SensorId == pstSensorInitFunc[i].SensorId && r_u8ModuleId==pstSensorInitFunc[i].moduleId)
        {
            matchIndex=i;
            CAM_MSDK_LOG("[%s] target SID %x and MID %x are found", __func__, r_u4SensorId, r_u8ModuleId);
            break;
        }
    }

    if(matchIndex==-1 && defaultIndex!=-1)
    {
        CAM_MSDK_LOG("Use default SID data");
        matchIndex=defaultIndex;
    } else if(matchIndex!=-1)
        CAM_MSDK_LOG("Use MID data");

    return matchIndex;
}

/*******************************************************************************
*
********************************************************************************/

MBOOL openSoLibrary(MUINT32 SensorId, MUINT32 ModuleId)
{
    CAM_MSDK_LOG("[%s] Find sensor id %x and module id %x", __func__, SensorId, ModuleId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return MFALSE; //no match sensorId
    }

    //find path name
    int status = -EINVAL;
    char path[64]="";
    strncat(path, (char *)pstSensorInitFunc[i].drvname, 32);
    if(pstSensorInitFunc[i].moduleIndex!=0) {
        char tmpNumStr[10]="";
        sprintf(tmpNumStr,"_%d",pstSensorInitFunc[i].moduleIndex);
        strncat(path, tmpNumStr, 10);
    }
    strncat(path, "_tuning.so", 10);

    MUINT64 SMID=((UINT64)SensorId<<32)+ModuleId;
    void *handle = NULL;

    //dlopen
    g_mapLock.lock();
    if(g_handleMap.count(SMID)==0)
    {
        handle = dlopen(path, RTLD_NOW);
        if (handle == NULL) {
            char const *err_str = dlerror();
            CAM_MSDK_LOG("load: module=%s\n%s", path, err_str?err_str:"unknown");
            status = -EINVAL;
            g_mapLock.unlock();
            return MFALSE;
        }

        //push back handle to map
        g_handleMap.insert(std::make_pair(SMID, handle));
    } else {
        handle=g_handleMap[SMID];
    }

    //dlsym1
    if(g_defaultMap.count(SMID)==0)
    {
        const char *sym1 = "getDefaultDataFunc";
        dlGetFuncDefault imports1;
        fptrDefault getCameraDefault;

        imports1 = (dlGetFuncDefault)dlsym(handle, sym1);
        if(imports1 == NULL) {
            CAM_MSDK_LOG("load: couldn't find symbol %s", sym1);
            status = -EINVAL;
            g_mapLock.unlock();
            return MFALSE;
        }
        getCameraDefault=imports1(pstSensorInitFunc[i].sensorType);

        //push back g_defaultMap to map
        g_defaultMap.insert(std::make_pair(SMID, getCameraDefault));
    }

    //dlsym2
    if(g_flickerMap.count(SMID)==0)
    {
        const char *sym2 = "getFlickerParaFunc";
        dlGetFuncFlicker imports2;
        fptrFlicker getCameraFlicker;

        imports2 = (dlGetFuncFlicker)dlsym(handle, sym2);
        if(imports2 == NULL) {
            CAM_MSDK_LOG("load: couldn't find symbol %s", sym2);
            status = -EINVAL;
            g_mapLock.unlock();
            return MFALSE;
        }
        getCameraFlicker=imports2(pstSensorInitFunc[i].sensorType);

        //push back g_flickerMap to map
        g_flickerMap.insert(std::make_pair(SMID, getCameraFlicker));
    }
    g_mapLock.unlock();

    CAM_MSDK_LOG("[%s] end of open %s", __func__, path);

    return MTRUE;
}

MUINT32 openCustomTuningLibrary(MUINT32 SensorId, MUINT32 ModuleId)
{
    MUINT64 SMID=((UINT64)SensorId<<32)+ModuleId;
    CAM_MSDK_LOG("[%s]", __func__);
    g_hadOpen[SMID]=openSoLibrary(SensorId, ModuleId);
    return g_hadOpen[SMID];
}

MBOOL closeSoLibrary(MUINT32 SensorId, MUINT32 ModuleId)
{
    CAM_MSDK_LOG("[%s] +", __func__);
    MUINT64 SMID=((UINT64)SensorId<<32)+ModuleId;

    //Remove keys
    g_mapLock.lock();
    if(g_handleMap.count(SMID)>0) {
        dlclose(g_handleMap[SMID]);
        g_handleMap.erase(SMID);
    }
    if(g_defaultMap.count(SMID)>0)
        g_defaultMap.erase(SMID);
    if(g_flickerMap.count(SMID)>0)
        g_flickerMap.erase(SMID);

    g_mapLock.unlock();
    CAM_MSDK_LOG("[%s] -", __func__);

    return MTRUE;
}

MUINT32 closeCustomTuningLibrary(MUINT32 SensorId, MUINT32 ModuleId)
{
    MUINT64 SMID=((UINT64)SensorId<<32)+ModuleId;
    g_hadOpen[SMID]=MFALSE;

    MBOOL ret=closeSoLibrary(SensorId, ModuleId);

    CAM_MSDK_LOG("[%s]", __func__);

    return ret;
}


MUINT32 lockCustomTuningLibrary(MUINT32 SensorId, MUINT32 ModuleId)
{
    MUINT64 SMID=((UINT64)SensorId<<32)+ModuleId;
    g_isLocked[SMID]=MTRUE;

    return MTRUE;
}

MUINT32 unlockCustomTuningLibrary(MUINT32 SensorId, MUINT32 ModuleId)
{
    MUINT64 SMID=((UINT64)SensorId<<32)+ModuleId;

    g_isLocked[SMID]=MFALSE;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
void GetCameraDefaultPara(MUINT32 SensorId,
                          MUINT32 ModuleId,
						  PNVRAM_CAMERA_ISP_PARAM_STRUCT pCameraISPDefault,
						  PNVRAM_CAMERA_3A_STRUCT pCamera3ADefault,
						  PNVRAM_CAMERA_SHADING_STRUCT pCameraShadingDefault,
						  PAE_PLINETABLE_STRUCT pCameraAEPlineData)
{

    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return; //no match sensorId
    }

    MUINT64 SMID=((UINT64)SensorId<<32)+ModuleId;

    fptrDefault getCameraDefault;
    if(g_defaultMap.count(SMID)>0)
        getCameraDefault=g_defaultMap[SMID];
    else if(!g_hadOpen[SMID])
    {
        if(g_isLocked[SMID])
            CAM_MSDK_ASSERT
        openSoLibrary(SensorId, ModuleId);
        getCameraDefault=g_defaultMap[SMID];
    }
    else
        return;

	if (pCameraISPDefault != NULL)
        getCameraDefault(CAMERA_NVRAM_DATA_ISP,(VOID*)pCameraISPDefault,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));

	if (pCamera3ADefault != NULL)
	    getCameraDefault(CAMERA_NVRAM_DATA_3A,(VOID*)pCamera3ADefault,sizeof(NVRAM_CAMERA_3A_STRUCT));

	if (pCameraShadingDefault != NULL)
	    getCameraDefault(CAMERA_NVRAM_DATA_SHADING,(VOID*)pCameraShadingDefault,sizeof(NVRAM_CAMERA_SHADING_STRUCT));

	if (pCameraAEPlineData != NULL)
	    getCameraDefault(CAMERA_DATA_AE_PLINETABLE,(VOID*)pCameraAEPlineData,sizeof(AE_PLINETABLE_T));

    if (pCameraISPDefault == NULL && pCamera3ADefault == NULL && pCameraShadingDefault == NULL && pCameraAEPlineData == NULL)
        CAM_MSDK_LOG("pCameraISPDefault & pCamera3ADefault & pCameraShadingDefault & pCameraAEPlineData are NULL");

    if(!g_hadOpen[SMID])
        closeSoLibrary(SensorId, ModuleId);

} /* GetCameraNvramValue() */
/*******************************************************************************
*  EEPROM related function
********************************************************************************/
UINT32 CAM_CALGetCalData(UINT32* pGetSensorCalData);
UINT32 EEPROMGetCalData(UINT32* pGetSensorCalData);

MUINT32 GetCameraCalData(MUINT32 SensorId, MUINT32* pGetSensorCalData)
{
    int result;

    switch (SensorId) {
    case IMX386_SENSOR_ID:
    case S5K3M3_SENSOR_ID:
        result = EEPROMGetCalData(pGetSensorCalData);
        break;

    default:
        result = CAM_CALGetCalData(pGetSensorCalData);
    }
    return result;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 LensCustomInit(unsigned int a_u4CurrSensorDev)
{
    GetLensInitFuncList(&LensInitFunc[0], a_u4CurrSensorDev);
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
void GetLensDefaultPara(MUINT32 a_u4SensorID, MUINT32 ModuleId, PNVRAM_LENS_PARA_STRUCT pLensParaDefault)
{

    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, a_u4SensorID);
    MINT64 i=compareSensorIdAndModuleId(a_u4SensorID, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", a_u4SensorID);
        return; //no match sensorId
    }

    MUINT64 SMID=((MUINT64)a_u4SensorID<<32)+ModuleId;

    fptrDefault getCameraDefault;
    if(g_defaultMap.count(SMID)>0)
        getCameraDefault=g_defaultMap[SMID];
    else if(!g_hadOpen[SMID])
    {
        if(g_isLocked[SMID])
            CAM_MSDK_ASSERT
        openSoLibrary(a_u4SensorID, ModuleId);
        getCameraDefault=g_defaultMap[SMID];
    }
    else
        return;

    if (pLensParaDefault!=NULL)
        getCameraDefault(CAMERA_NVRAM_DATA_LENS,(VOID*)pLensParaDefault,sizeof(NVRAM_LENS_PARA_STRUCT));
    else
        CAM_MSDK_LOG("[%s] pLensParaDefault is NULL", __func__);

    if(!g_hadOpen[SMID])
        closeSoLibrary(a_u4SensorID, ModuleId);

}
/*******************************************************************************
*
********************************************************************************/
MUINT32 LensCustomSetIndex(MUINT32 a_u4CurrIdx)
{
    gMainLensIdx = a_u4CurrIdx;

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 LensCustomGetInitFunc(MSDK_LENS_INIT_FUNCTION_STRUCT *a_pLensInitFunc)
{
    if (a_pLensInitFunc != NULL) {
        memcpy(a_pLensInitFunc, &LensInitFunc[0], sizeof(MSDK_LENS_INIT_FUNCTION_STRUCT) * MAX_NUM_OF_SUPPORT_LENS);
        return 0;
    }
    return -1;
}

int msdkGetFlickerPara(MUINT32 SensorId, MUINT32 ModuleId, int SensorMode, void* buf, int binRatio)
{
    CAM_MSDK_LOG("msdkGetFlickerPara(): id(%d), mode(%d), binRatio(%d).",
            SensorId, SensorMode, binRatio);

    if (!pstSensorInitFunc) {
        CAM_MSDK_LOG("msdkGetFlickerPara(): init camera custom.");
        cameraCustomInit();
    }

    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return -1; //no match sensorId
    }

    MUINT64 SMID=((UINT64)SensorId<<32) + ModuleId;

    fptrFlicker getCameraFlickerPara;
    if(g_flickerMap.count(SMID)>0)
        getCameraFlickerPara=g_flickerMap[SMID];
    else if(!g_hadOpen[SMID])
    {
        if(g_isLocked[SMID])
            CAM_MSDK_ASSERT
        openSoLibrary(SensorId, ModuleId);
        getCameraFlickerPara=g_flickerMap[SMID];
    }
    else
        return -1;

    getCameraFlickerPara(SensorMode, binRatio, buf);

    if(!g_hadOpen[SMID])
        closeSoLibrary(SensorId, ModuleId);

    return 0;
}

int msdkGetFlickerParaAll(MUINT32 SensorId, MUINT32 ModuleId, int SensorMode, FLICKER_CUST_PARA*** buf, int binRatio)
{
    CAM_MSDK_LOG("msdkGetFlickerPara(): id(%d), mode(%d), binRatio(%d).",
            SensorId, SensorMode, binRatio);

    if (!pstSensorInitFunc) {
        CAM_MSDK_LOG("msdkGetFlickerPara(): init camera custom.");
        cameraCustomInit();
    }

    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return -1; //no match sensorId
    }

    MUINT64 SMID=((UINT64)SensorId<<32) + ModuleId;

    fptrFlicker getCameraFlickerPara;
    if(g_flickerMap.count(SMID)>0)
        getCameraFlickerPara=g_flickerMap[SMID];
    else if(!g_hadOpen[SMID])
    {
        if(g_isLocked[SMID])
            CAM_MSDK_ASSERT
        openSoLibrary(SensorId, ModuleId);
        getCameraFlickerPara=g_flickerMap[SMID];
    }
    else
        return -1;

    for(int j=0;j<SensorMode;j++)
    {
        for(int k=0;k<binRatio;k++)
        {
            getCameraFlickerPara(j, k+1, &((*buf)[j][k]));
        }
    }

    if(!g_hadOpen[SMID])
        closeSoLibrary(SensorId, ModuleId);

    return 0;
}

MINT32 GetCameraTsfDefaultTbl(MUINT32 SensorId, MUINT32 ModuleId, PCAMERA_TSF_TBL_STRUCT pCameraTsfDefault)
{
    MINT32 ret = -1;

    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return ret; //no match sensorId
    }

    //find path name
    //int status = -EINVAL;
    char path[64]="";
    strncat(path, (char *)pstSensorInitFunc[i].drvname, 32);
    if(pstSensorInitFunc[i].moduleIndex!=0) {
        char tmpNumStr[10]="";
        sprintf(tmpNumStr,"_%d",pstSensorInitFunc[i].moduleIndex);
        strncat(path, tmpNumStr, 10);
    }
    strncat(path, "_tuning.so", 10);

    MUINT64 SMID=((UINT64)SensorId<<32)+ModuleId;

    fptrDefault getCameraDefault;
    if(g_defaultMap.count(SMID)>0)
        getCameraDefault=g_defaultMap[SMID];
    else if(!g_hadOpen[SMID])
    {
        if(g_isLocked[SMID])
            CAM_MSDK_ASSERT
        openSoLibrary(SensorId, ModuleId);
        getCameraDefault=g_defaultMap[SMID];
    }
    else
        return ret;

    if (pCameraTsfDefault!=NULL)
        ret = getCameraDefault(CAMERA_DATA_TSF_TABLE,(VOID*)pCameraTsfDefault,sizeof(CAMERA_TSF_TBL_STRUCT));
    else
        CAM_MSDK_LOG("[%s] pCameraTsfDefault is NULL", __func__);

    if(!g_hadOpen[SMID])
        closeSoLibrary(SensorId, ModuleId);

    return ret;
}

MINT32 GetCameraBpciDefaultTbl(MUINT32 SensorId, MUINT32 ModuleId, PCAMERA_BPCI_STRUCT pCameraBPCIDefault)
{
    MINT32 ret = -1;

    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return ret; //no match sensorId
    }

    //find path name
    //int status = -EINVAL;
    char path[64]="";
    strncat(path, (char *)pstSensorInitFunc[i].drvname, 32);
    if(pstSensorInitFunc[i].moduleIndex!=0) {
        char tmpNumStr[10]="";
        sprintf(tmpNumStr,"_%d",pstSensorInitFunc[i].moduleIndex);
        strncat(path, tmpNumStr, 10);
    }
    strncat(path, "_tuning.so", 10);

    MUINT64 SMID=((UINT64)SensorId<<32)+ModuleId;

    fptrDefault getCameraDefault;
    if(g_defaultMap.count(SMID)>0)
        getCameraDefault=g_defaultMap[SMID];
    else if(!g_hadOpen[SMID])
    {
        if(g_isLocked[SMID])
            CAM_MSDK_ASSERT
        openSoLibrary(SensorId, ModuleId);
        getCameraDefault=g_defaultMap[SMID];
    }
    else
        return ret;

    if (pCameraBPCIDefault!=NULL)
        ret = getCameraDefault(CAMERA_DATA_PDC_TABLE,(VOID*)pCameraBPCIDefault,sizeof(CAMERA_BPCI_STRUCT));
    else
        CAM_MSDK_LOG("[%s] pCameraBPCIDefault is NULL", __func__);

    if(!g_hadOpen[SMID])
        closeSoLibrary(SensorId, ModuleId);

    return ret;
}

#if MTK_CAM_NEW_NVRAM_SUPPORT
MINT32 GetCameraIdxTblDefaultPara(MUINT32 SensorId, MUINT32 ModuleId, PNVRAM_CAMERA_IDX_STRUCT pCameraIdxTblDefault)
{
	MINT32 ret = -1;
	if (NULL == pstSensorInitFunc[0].getCameraIndexMgr)
    {
        CAM_MSDK_LOG("[%s]: uninit yet", __func__);
        return ret; //not initial pstSensorInitFunc yet
    }

    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return ret; //no match sensorId
    }

	if (pCameraIdxTblDefault!=NULL)
        ret = pstSensorInitFunc[i].getCameraIndexMgr(CAMERA_NVRAM_IDX_TBL,(VOID*)pCameraIdxTblDefault,sizeof(NVRAM_CAMERA_IDX_STRUCT));
    else
        CAM_MSDK_LOG("[%s] pCameraIdxTblDefault is NULL", __func__);

	return ret;
}
#endif

MINT32 GetCameraFeatureDefault(MUINT32 SensorId, MUINT32 ModuleId, NVRAM_CAMERA_FEATURE_STRUCT *pCamFeatureDefault)
{
    MINT32 ret = -1;

    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return ret; //no match sensorId
    }

    MUINT64 SMID=((UINT64)SensorId<<32)+ModuleId;

    fptrDefault getCameraDefault;
    if(g_defaultMap.count(SMID)>0)
        getCameraDefault=g_defaultMap[SMID];
    else if(!g_hadOpen[SMID])
    {
        if(g_isLocked[SMID])
            CAM_MSDK_ASSERT
        openSoLibrary(SensorId, ModuleId);
        getCameraDefault=g_defaultMap[SMID];
    }
    else
        return ret;

    if (pCamFeatureDefault!=NULL)
        ret = getCameraDefault(CAMERA_NVRAM_DATA_FEATURE,(VOID*)pCamFeatureDefault,sizeof(NVRAM_CAMERA_FEATURE_STRUCT));
    else
        CAM_MSDK_LOG("[%s] pCamFeatureDefault is NULL", __func__);

    if(!g_hadOpen[SMID])
        closeSoLibrary(SensorId, ModuleId);

    return ret;
}

MUINT32 GetCameraFlashDefaultPara(MUINT32 SensorId, MUINT32 ModuleId, PNVRAM_CAMERA_STROBE_STRUCT pCameraStrobeDefault)
{
    MINT32 ret = -1;

    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return ret; //no match sensorId
    }

    MUINT64 SMID=((UINT64)SensorId<<32)+ModuleId;

    fptrDefault getCameraDefault;
    if(g_defaultMap.count(SMID)>0)
        getCameraDefault=g_defaultMap[SMID];
    else if(!g_hadOpen[SMID])
    {
        if(g_isLocked[SMID])
            CAM_MSDK_ASSERT
        openSoLibrary(SensorId, ModuleId);
        getCameraDefault=g_defaultMap[SMID];
    }
    else
        return ret;

    if (pCameraStrobeDefault!=NULL)
        ret = getCameraDefault(CAMERA_NVRAM_DATA_STROBE,(VOID*)pCameraStrobeDefault,sizeof(NVRAM_CAMERA_STROBE_STRUCT));
    else
        CAM_MSDK_LOG("[%s] pCameraStrobeDefault is NULL", __func__);

    if(!g_hadOpen[SMID])
        closeSoLibrary(SensorId, ModuleId);

    return ret;
}

MUINT32 GetCameraFlashDefaultCalData(MUINT32 SensorId, MUINT32 ModuleId, PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT pCameraFlashCalData)
{
    MINT32 ret = -1;

    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return ret; //no match sensorId
    }

    MUINT64 SMID=((UINT64)SensorId<<32)+ModuleId;

    fptrDefault getCameraDefault;
    if(g_defaultMap.count(SMID)>0)
        getCameraDefault=g_defaultMap[SMID];
    else if(!g_hadOpen[SMID])
    {
        if(g_isLocked[SMID])
            CAM_MSDK_ASSERT
        openSoLibrary(SensorId, ModuleId);
        getCameraDefault=g_defaultMap[SMID];
    }
    else
        return ret;

    if (pCameraFlashCalData!=NULL)
        ret = getCameraDefault(CAMERA_NVRAM_DATA_FLASH_CALIBRATION,(VOID*)pCameraFlashCalData,sizeof(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT));
    else
        CAM_MSDK_LOG("[%s] pCameraFlashCalData is NULL", __func__);

    if(!g_hadOpen[SMID])
        closeSoLibrary(SensorId, ModuleId);

    return ret;
}
