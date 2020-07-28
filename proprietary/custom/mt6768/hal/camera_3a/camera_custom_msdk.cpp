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
#include <utility>
//#include <aee.h>

#include <unordered_map>
#include <mutex>

#include <mtkcam/drv/mem/cam_cal_drv.h>

#define MTK_LOG_ENABLE 1

#undef LOG_TAG
#define LOG_TAG "CAM_CUS_MSDK"

#define CAM_MSDK_LOG(fmt, arg...)    ALOGD(LOG_TAG " " fmt, ##arg)
#define CAM_MSDK_ERR(fmt, arg...)    ALOGE(LOG_TAG "Err: %5d: " fmt, __LINE__, ##arg)

//#define CAM_MSDK_ASSERT aee_system_warning(LOG_TAG, NULL, DB_OPT_DEFAULT, "Do not allow open custom library twice");


MSDK_SENSOR_INIT_FUNCTION_STRUCT *pstSensorInitFunc = NULL;
MSDK_LENS_INIT_FUNCTION_STRUCT LensInitFunc[MAX_NUM_OF_SUPPORT_LENS+1] =
{ {0,0,{0},NULL},
  {0,0,{0},NULL},
  {0,0,{0},NULL},
  {0,0,{0},NULL},
};

MUINT32 gMainLensIdx;
MUINT32 gSubLensIdx;

typedef fptrDefault (*dlGetFuncDefault)(MUINT32 sensorType);
typedef fptrFlicker (*dlGetFuncFlicker)(MUINT32 sensorType);

typedef struct DLOPEN_STRUCT
{
    MBOOL isLocked;
    MBOOL opendExt;
    void *handle;
    fptrDefault defaultFunc;
    fptrFlicker flickerFunc;
    DLOPEN_STRUCT()
        : isLocked(MFALSE)
        , opendExt(MFALSE)
        , handle(NULL)
        , defaultFunc(NULL)
        , flickerFunc(NULL)
    {
    }
}DLOPEN_STRUCT_T;

static std::unordered_map<MINT64, DLOPEN_STRUCT_T> g_handleList;


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
            CAM_MSDK_LOG("last item in sensorList found.  i(%u)", i);
            break;
        }
        if (r_u4SensorId == pstSensorInitFunc[i].SensorId )
        {
            defaultIndex=i;
        }
    }

    if(matchIndex==-1 && defaultIndex!=-1)
    {
        CAM_MSDK_LOG("SID %X data is found", r_u4SensorId);
        matchIndex=defaultIndex;
    } else if(matchIndex!=-1)
        CAM_MSDK_LOG("Use MID data");

    return matchIndex;
}

/*******************************************************************************
*
********************************************************************************/

DLOPEN_STRUCT_T openDefaultSoLibrary(MINT64 i)
{
    //find path name
    char path[64]="";
    sprintf(path,"%s_tuning.so", (char *)pstSensorInitFunc[i].drvname);

    //dlopen
    void *handle=NULL;
    fptrDefault output=NULL;

    if(g_handleList[i].handle != NULL)
        handle=g_handleList[i].handle;
    else
    {
        if(g_handleList[i].isLocked)
            CAM_MSDK_ERR("Yang Library was locked");
        handle=dlopen(path, RTLD_NOW);
        if (handle == NULL) {
            char const *err_str = dlerror();
            CAM_MSDK_LOG("load: module=%s\n%s", path, err_str?err_str:"unknown");
            return g_handleList[i];
        }
        g_handleList[i].handle=handle;
    }

    if(g_handleList[i].defaultFunc != NULL)
    {
        output=g_handleList[i].defaultFunc;
        CAM_MSDK_LOG("[%s] using exist handle", __func__);
    }
    else
    {
        //dlsym1
        const char *sym1 = "getDefaultDataFunc";
        dlGetFuncDefault imports1;
        imports1 = (dlGetFuncDefault)dlsym(handle, sym1);
        if(imports1 == NULL) {
            CAM_MSDK_ERR("load: couldn't find symbol %s", sym1);
            return g_handleList[i];
        }

        output=imports1(pstSensorInitFunc[i].sensorType);
        if(output==NULL) {
            CAM_MSDK_ERR("function is not found");
            return g_handleList[i];
        }
        g_handleList[i].defaultFunc=output;

        CAM_MSDK_LOG("[%s] end of open %s", __func__, path);
    }

    return g_handleList[i];
}

DLOPEN_STRUCT_T openFlickerSoLibrary(MINT64 i)
{
    //find path name
    char path[64]="";
    sprintf(path,"%s_tuning.so", (char *)pstSensorInitFunc[i].drvname);

    //dlopen
    void *handle=NULL;
    fptrFlicker output=NULL;

    if(g_handleList[i].handle != NULL)
        handle=g_handleList[i].handle;
    else
    {
        handle=dlopen(path, RTLD_NOW);
        if (handle == NULL) {
            char const *err_str = dlerror();
            CAM_MSDK_LOG("load: module=%s\n%s", path, err_str?err_str:"unknown");
            return g_handleList[i];
        }
        g_handleList[i].handle=handle;
    }

    if(g_handleList[i].flickerFunc != NULL)
        output=g_handleList[i].flickerFunc;
    else
    {
        //dlsym2
        const char *sym2 = "getFlickerParaFunc";
        dlGetFuncFlicker imports2;
        imports2 = (dlGetFuncFlicker)dlsym(handle, sym2);
        if(imports2 == NULL) {
            CAM_MSDK_LOG("load: couldn't find symbol %s", sym2);
            return g_handleList[i];
        }

        output=imports2(pstSensorInitFunc[i].sensorType);
        if(output==NULL) {
            CAM_MSDK_ERR("function is not found");
            return g_handleList[i];
        }
        g_handleList[i].flickerFunc=output;

        CAM_MSDK_LOG("[%s] end of open %s", __func__, path);
    }

    return g_handleList[i];
}


MUINT32 openCustomTuningLibrary(MUINT32 SensorId, MUINT32 ModuleId)
{
    CAM_MSDK_LOG("[%s]", __func__);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return MFALSE; //no match sensorId
    }
    DLOPEN_STRUCT_T mydefaultHandle = openDefaultSoLibrary(i);
    DLOPEN_STRUCT_T myflickerHandle = openFlickerSoLibrary(i);
    g_handleList[i].opendExt = MTRUE;

    if(mydefaultHandle.defaultFunc && myflickerHandle.flickerFunc)
        return 0;
    else
        return -1;
}

MBOOL closeSoLibrary(MINT64 i)
{
    if(g_handleList[i].handle != NULL && !g_handleList[i].opendExt)
    {
        CAM_MSDK_LOG("[%s] ", __func__);
        int ret = dlclose(g_handleList[i].handle);
        g_handleList[i].defaultFunc = NULL;
        g_handleList[i].flickerFunc = NULL;
        g_handleList[i].handle = NULL;
        return ret;
    }
    return MFALSE;
}

MUINT32 closeCustomTuningLibrary(MUINT32 SensorId, MUINT32 ModuleId)
{
    CAM_MSDK_LOG("[%s]", __func__);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return MFALSE; //no match sensorId
    }

    g_handleList[i].opendExt=MFALSE;

    return closeSoLibrary(i);
}


MUINT32 lockCustomTuningLibrary(MUINT32 SensorId, MUINT32 ModuleId)
{
    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return MFALSE; //no match sensorId
    }

    g_handleList[i].isLocked=MTRUE;

    return MTRUE;
}

MUINT32 unlockCustomTuningLibrary(MUINT32 SensorId, MUINT32 ModuleId)
{
    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return MFALSE; //no match sensorId
    }

    g_handleList[i].isLocked=MFALSE;

    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
void GetCameraDefaultPara(MUINT32 SensorId,
						  PNVRAM_CAMERA_ISP_PARAM_STRUCT pCameraISPDefault,
						  PNVRAM_CAMERA_3A_STRUCT pCamera3ADefault,
						  PNVRAM_CAMERA_SHADING_STRUCT pCameraShadingDefault,
						  PAE_PLINETABLE_STRUCT pCameraAEPlineData)
{
    MUINT32 ModuleId=0;
    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return; //no match sensorId
    }

    DLOPEN_STRUCT_T soHandle=openDefaultSoLibrary(i);
    if(soHandle.defaultFunc==NULL)
        return;
    fptrDefault getCameraDefault=soHandle.defaultFunc;

    if (pCameraISPDefault!=NULL)
        getCameraDefault(CAMERA_NVRAM_DATA_ISP,(VOID*)pCameraISPDefault,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));

    if (pCamera3ADefault != NULL)
	    getCameraDefault(CAMERA_NVRAM_DATA_3A,(VOID*)pCamera3ADefault,sizeof(NVRAM_CAMERA_3A_STRUCT));

    if (pCameraShadingDefault!=NULL)
	    getCameraDefault(CAMERA_NVRAM_DATA_SHADING,(VOID*)pCameraShadingDefault,sizeof(NVRAM_CAMERA_SHADING_STRUCT));

    if (pCameraAEPlineData != NULL)
	    getCameraDefault(CAMERA_DATA_AE_PLINETABLE,(VOID*)pCameraAEPlineData,sizeof(AE_PLINETABLE_T));

    if (pCameraISPDefault == NULL && pCamera3ADefault == NULL && pCameraShadingDefault == NULL && pCameraAEPlineData == NULL)
        CAM_MSDK_LOG("pCameraISPDefault & pCamera3ADefault & pCameraShadingDefault & pCameraAEPlineData are NULL");

    closeSoLibrary(i);
} /* GetCameraNvramValue() */

MUINT32 GetCameraCalData(MUINT32 SensorId, MUINT32* pGetSensorCalData)
{
    (void) SensorId;
    (void) pGetSensorCalData;

    return 0;
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
void GetLensDefaultPara(MUINT32 a_u4SensorID, PNVRAM_LENS_PARA_STRUCT pLensParaDefault)
{
    MUINT32 LensId = LensInitFunc[gMainLensIdx].LensId;

    if (LensInitFunc[0].getLensDefault == NULL)
    {
        CAM_MSDK_LOG("[GetLensDefaultPara]: uninit yet\n\n");
        return;
    }

    /*
    for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
    {
        if (LensId == LensInitFunc[i].LensId)
        {
            break;
        }
    }
    */
    if (pLensParaDefault != NULL)
    {
        MUINT32 i = (0<=gMainLensIdx && gMainLensIdx<MAX_NUM_OF_SUPPORT_LENS) ? gMainLensIdx : 0;
        CAM_MSDK_LOG("[GetLensDefaultPara]: %d, %x", gMainLensIdx, LensInitFunc[gMainLensIdx].SensorId);
        LensInitFunc[i].getLensDefault((VOID*)pLensParaDefault, sizeof(NVRAM_LENS_PARA_STRUCT));
    }
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


int msdkGetFlickerPara(MUINT32 SensorId, int SensorMode, void* buf, int binRatio)
{
    MUINT32 ModuleId=0;
    CAM_MSDK_LOG("msdkGetFlickerPara(): id(%d), mode(%d).",
            SensorId, SensorMode);

    if (!pstSensorInitFunc) {
        CAM_MSDK_LOG("msdkGetFlickerPara(): init camera custom.");
        cameraCustomInit();
    }
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return -1; //no match sensorId
    }

    DLOPEN_STRUCT_T soHandle=openFlickerSoLibrary(i);
    if(soHandle.flickerFunc==NULL)
        return -1;
    fptrFlicker getCameraFlickerPara=soHandle.flickerFunc;

    getCameraFlickerPara(SensorMode, binRatio, buf);

    closeSoLibrary(i);

    return 0;
}

int msdkGetFlickerParaAll(MUINT32 SensorId, int SensorMode, FLICKER_CUST_PARA*** buf, int binRatio)
{
    MUINT32 ModuleId=0;
    CAM_MSDK_LOG("msdkGetFlickerPara(): id(%d), mode(%d), binRatio(%d).",
            SensorId, SensorMode, binRatio);

    if (!pstSensorInitFunc) {
        CAM_MSDK_LOG("msdkGetFlickerPara(): init camera custom.");
        cameraCustomInit();
    }

    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return -1; //no match sensorId
    }

    DLOPEN_STRUCT_T soHandle=openFlickerSoLibrary(i);
    if(soHandle.flickerFunc==NULL)
        return -1;
    fptrFlicker getCameraFlickerPara=soHandle.flickerFunc;

    for(int j=0;j<SensorMode;j++)
    {
        for(int k=0;k<binRatio;k++)
        {
            getCameraFlickerPara(j, k+1, &((*buf)[j][k]));
        }
    }

    closeSoLibrary(i);

    return 0;
}

MINT32 GetCameraTsfDefaultTbl(MUINT32 SensorId, PCAMERA_TSF_TBL_STRUCT pCameraTsfDefault)
{
    MUINT32 ModuleId=0;
    MINT32 ret = -1;

    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return ret; //no match sensorId
    }

    DLOPEN_STRUCT_T soHandle=openDefaultSoLibrary(i);
    if(soHandle.defaultFunc==NULL)
        return ret;
    fptrDefault getCameraDefault=soHandle.defaultFunc;

    if (pCameraTsfDefault!=NULL)
        ret = getCameraDefault(CAMERA_DATA_TSF_TABLE,(VOID*)pCameraTsfDefault,sizeof(CAMERA_TSF_TBL_STRUCT));
    else
        CAM_MSDK_LOG("[%s] pCameraTsfDefault is NULL", __func__);

    closeSoLibrary(i);

    return ret;
}

MINT32 GetCameraBpciDefaultTbl(MUINT32 SensorId, PCAMERA_BPCI_STRUCT pCameraBPCIDefault)
{
    MUINT32 ModuleId=0;
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

    if (pCameraBPCIDefault!=NULL)
        ret = pstSensorInitFunc[i].getCameraIndexMgr(CAMERA_DATA_PDC_TABLE,(VOID*)pCameraBPCIDefault,sizeof(CAMERA_BPCI_STRUCT));
    else
        CAM_MSDK_LOG("[%s] pCameraBPCIDefault is NULL", __func__);

    return ret;
}


MINT32 GetCameraFeatureDefault(MUINT32 SensorId, NVRAM_CAMERA_FEATURE_STRUCT *pCamFeatureDefault)
{
    MUINT32 ModuleId=0;
    MINT32 ret = -1;

    CAM_MSDK_LOG("[%s] Find sensor id %x", __func__, SensorId);
    MINT64 i=compareSensorIdAndModuleId(SensorId, ModuleId);
    if (i==-1)
    {
        CAM_MSDK_LOG("No sensor %x in this list", SensorId);
        return ret; //no match sensorId
    }

    DLOPEN_STRUCT_T soHandle=openDefaultSoLibrary(i);
    if(soHandle.defaultFunc==NULL)
        return ret;
    fptrDefault getCameraDefault=soHandle.defaultFunc;

    if (pCamFeatureDefault!=NULL)
        ret = getCameraDefault(CAMERA_NVRAM_DATA_FEATURE,(VOID*)pCamFeatureDefault,sizeof(NVRAM_CAMERA_FEATURE_STRUCT));
    else
        CAM_MSDK_LOG("[%s] pCamFeatureDefault is NULL", __func__);

    closeSoLibrary(i);

    return ret;
}
