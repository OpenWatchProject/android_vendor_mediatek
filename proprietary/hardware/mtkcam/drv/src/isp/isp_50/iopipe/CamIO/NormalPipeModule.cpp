/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "NormalPipe"
//
#include <vector>
#include <algorithm>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/drv/iopipe/CamIO/Cam_QueryDef.h>

//
#include <Cam/ICam_capibility.h>
#include "FakeSensor.h"
#include <ResMgr.h>
#include <sec_mgr.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_NORMAL_PIPE);
//
//
using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSImageio;
using namespace NSIspio;

#define THIS_NAME  "NormalPipe_Module"
#define IQSenSum   (2)
#define IQSenCombi (4)

/******************************************************************************
 *  Global Function Declaration.
 ******************************************************************************/
extern "C" INormalPipe* createInstance_INormalPipe(MUINT32 SensorIndex, char const* szCallerName);


/******************************************************************************
 *  Module Version
 ******************************************************************************/
/**
 * The supported module API version.
 */
#define MY_MODULE_API_VERSION       MTKCAM_MAKE_API_VERSION(1, 0)

/**
 * The supported sub-module API versions in ascending order.
 * Assume: all cameras support the same sub-module api versions.
 */
#define MY_SUB_MODULE_API_VERSION   {MTKCAM_MAKE_API_VERSION(1, 0), }
static const std::vector<MUINT32>   gSubModuleApiVersion = []() {
    static std::vector<MUINT32> v(MY_SUB_MODULE_API_VERSION);
    std::sort(v.begin(), v.end());
    return v;
} ();


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
/******************************************************************************
 *
 ******************************************************************************/
#define IQ_MEM_NEW(dstPtr, type, size, memInfo)\
do {\
    memInfo.AllocMemSum += size; \
    dstPtr = new type;\
} while(0)
#define IQ_MEM_DEL(dstPtr, size, memInfo)\
do{\
    memInfo.FreedMemSum += size; \
    delete dstPtr;\
    dstPtr = NULL;\
}while(0)

namespace {

class platSensorsInfo {
public:
    typedef struct
    {
        MUINT32                   mIdx;
        MUINT32                   mTypeforMw;
        MUINT32                   mDevId;//main/sub/main0/...
        IHalSensor::ConfigParam   mConfig;
        SensorStaticInfo          mSInfo; //static
        SensorDynamicInfo         mDInfo; //Dynamic
        MUINTPTR                  mOccupiedOwner;
    }_sensorInfo_t;

    platSensorsInfo (void) {
        mExistedSCnt = 0;
        mSList = NULL;
        memset(mSenInfo, 0, sizeof(mSenInfo));
    }

    MUINT32         mExistedSCnt;
    IHalSensorList* mSList;
    _sensorInfo_t   mSenInfo[IOPIPE_MAX_SENSOR_CNT];
};

class IQ_CAPABILITY
{
public:
    typedef struct
    {
        MUINT32 AllocMemSum;
        MUINT32 FreedMemSum;
    }_MemInfo_t;

    IQ_CAPABILITY()
    {
        memset(&mMemInfo, 0, sizeof(mMemInfo));
        memset((void*)m_Name, 0, sizeof(m_Name));
    };
    virtual ~IQ_CAPABILITY(){};

    static IQ_CAPABILITY*  CreateInsatnce(char const* szCallerName);
           void            DestroyInstance(char const* szCallerName);

    typedef MVOID          (*IQTable_CB)(
                                 ResMgr*,
                                 Res_Meter_IN&,
                                 Res_Meter_OUT&,
                                 QueryOutData_t&
                            );

    static IQTable_CB       IQ_TBL[IQSenCombi][IQSenSum];

    static MBOOL            queryIQTable(
                                vector<QueryInData_t> const vInData,
                                vector<vector<QueryOutData_t>> &vOutData
                            );

    static MVOID            IQtable_H(
                                ResMgr *pResMgr,
                                Res_Meter_IN &Meter_In,
                                Res_Meter_OUT &Meter_Out,
                                QueryOutData_t &IQunit
                            );

    static MVOID            IQtable_L(
                                ResMgr *pResMgr,
                                Res_Meter_IN &Meter_In,
                                Res_Meter_OUT &Meter_Out,
                                QueryOutData_t &IQunit
                            );

public:
    static platSensorsInfo* m_pAllSensorInfo;
    static IHalSensor*      m_pHalSensor[IQSenSum];
    static _MemInfo_t       mMemInfo;
private:
    char                    m_Name[IOPIPE_MAX_USER_NAME_LEN];
};

class NormalPipeModule : public INormalPipeModule
{
public:     ////                    Operations.
                                    NormalPipeModule();

public:     ////                    Operations.

    virtual MERROR                  createSubModule(
                                        MUINT32 sensorIndex,
                                        char const* szCallerName,
                                        MUINT32 apiVersion,
                                        MVOID** ppInstance,
                                        MUINT32 pipeSelect = EPipeSelect_Normal
                                    );

    virtual MBOOL                   query(
                                        MUINT32 portIdx,
                                        MUINT32 eCmd,
                                        MINT imgFmt,
                                        NormalPipe_QueryIn const& input,
                                        NormalPipe_QueryInfo &queryInfo
                                    )   const;
    /*New query API for after ISP5.0*/
    virtual MBOOL                   query(
                                        MUINT32 eCmd,
                                        MUINTPTR pIO_struct
                                    )   const override;

};
};


/******************************************************************************
 *
 ******************************************************************************/
NormalPipeModule*
getNormalPipeModule()
{
    static NormalPipeModule singleton;
    return &singleton;
}


/******************************************************************************
 *
 ******************************************************************************/
extern "C"
mtkcam_module* get_mtkcam_module_iopipe_CamIO_NormalPipe()
{
    return static_cast<mtkcam_module*>(getNormalPipeModule());
}


/******************************************************************************
 *
 ******************************************************************************/
NormalPipeModule::
NormalPipeModule()
    : INormalPipeModule()
{
    //// + mtkcam_module initialization

    get_module_api_version      = []()->uint32_t{ return MY_MODULE_API_VERSION; };

    get_module_extension        = []()->void*   { return getNormalPipeModule(); };

    get_sub_module_api_version  = [](uint32_t const** versions, size_t* count, int index)->int
    {
        if  ( ! versions || ! count ) {
            MY_LOGE("[%#x] invalid arguments - versions:%p count:%p", index, versions, count);
            return -EINVAL;
        }

        //Assume: all cameras support the same sub-module api versions.
        *versions = gSubModuleApiVersion.data();
        *count = gSubModuleApiVersion.size();
        return 0;
    };

    //// - mtkcam_module initialization

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
NormalPipeModule::
createSubModule(
    MUINT32 sensorIndex,
    char const* szCallerName,
    MUINT32 apiVersion,
    MVOID** ppInstance,
    MUINT32 pipeSelect
)
{
    MERROR status = OK;

    if  ( ! ppInstance ) {
        MY_LOGE("[%s:%d] ppInstance==0", szCallerName, sensorIndex);
        return BAD_VALUE;
    }

    if  ( ! std::binary_search(gSubModuleApiVersion.begin(), gSubModuleApiVersion.end(), apiVersion) ) {
        MY_LOGE("[%s:%d] Unsupported sub-module api version:%#x", szCallerName, sensorIndex, apiVersion);
        return BAD_VALUE;
    }

    switch  (MTKCAM_GET_MAJOR_API_VERSION(apiVersion))
    {
        //Version = (1, 0)-(1, 0xFF)
        case 1:{
            *ppInstance = static_cast<INormalPipe*>(createInstance_INormalPipe(sensorIndex, szCallerName));
            break;
        }
        //
        default:{
            MY_LOGE(
                "[%s:%d] Not implement for sub-module api version:%#x",
                szCallerName, sensorIndex, apiVersion
            );
            status = INVALID_OPERATION;
            break;
        }
    }

    return status;
}

/******************************************************************************
 *
 ******************************************************************************/
platSensorsInfo*          IQ_CAPABILITY::m_pAllSensorInfo = NULL;
IHalSensor*               IQ_CAPABILITY::m_pHalSensor[IQSenSum] = {NULL, NULL};
IQ_CAPABILITY::_MemInfo_t IQ_CAPABILITY::mMemInfo;

/*for 1 sensor case, only call IQ_TBL[0][0] and IQ_TBL[1][0]*/
IQ_CAPABILITY::IQTable_CB IQ_CAPABILITY::IQ_TBL[IQSenCombi][IQSenSum] = {
    {IQ_CAPABILITY::IQtable_H, IQ_CAPABILITY::IQtable_H},
    {IQ_CAPABILITY::IQtable_L, IQ_CAPABILITY::IQtable_H},
    {IQ_CAPABILITY::IQtable_H, IQ_CAPABILITY::IQtable_L},
    {IQ_CAPABILITY::IQtable_L, IQ_CAPABILITY::IQtable_L}
};

/******************************************************************************
 *
 ******************************************************************************/
inline E_CamPixelMode CAP_MAP_PXLMODE(MUINT32 const sensorIdx, MUINT32 const Sen_PixMode)
{
    E_CamPixelMode tgPxlMode = ePixMode_NONE;

    if(NSCam::SENSOR_TYPE_YUV == IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType) {
        tgPxlMode = ePixMode_2;
    }
    else{
        switch (Sen_PixMode) {
            case ONE_PIXEL_MODE:
                tgPxlMode = ePixMode_1;
                break;
            case TWO_PIXEL_MODE:
                tgPxlMode = ePixMode_2;
                break;
            case FOUR_PIXEL_MODE:
                tgPxlMode = ePixMode_4;
                break;
            default:
                MY_LOGE("Unknown tg pixel mode mapping (%d)", (MUINT32)Sen_PixMode);
                break;
        }
    }

    return tgPxlMode;
}

inline E_CamPixelMode
CAP_MAP_PXLMODE(
    MUINT32 const sensorIdx
)
{
    E_CamPixelMode tgPxlMode = ePixMode_NONE;
    MUINT32 senType = 0;
    MUINT32 senPixMode = 0;

    if (IQ_CAPABILITY::m_pAllSensorInfo == NULL) {
        MY_LOGE("m_pAllSensorInfo is NULL.");
        return tgPxlMode;
    }

    senType = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType;
    senPixMode = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.pixelMode;

    if(NSCam::SENSOR_TYPE_YUV == senType) {
        tgPxlMode = ePixMode_2;
    }
    else{
        switch (senPixMode) {
            case ONE_PIXEL_MODE:
                tgPxlMode = ePixMode_1;
                break;
            case TWO_PIXEL_MODE:
                tgPxlMode = ePixMode_2;
                break;
            case FOUR_PIXEL_MODE:
                tgPxlMode = ePixMode_4;
                break;
            default:
                MY_LOGE("Unknown tg pixel mode mapping (%d)", senPixMode);
                break;
        }
    }

    return tgPxlMode;
}

inline MVOID
CAP_GET_TG_GRAB(
    MUINT32 const sensorIdx,
    MUINT const scenarioId,
    MUINT32 &x,
    MUINT32 &y
)
{
    if (IQ_CAPABILITY::m_pAllSensorInfo == NULL) {
        MY_LOGE("m_pAllSensorInfo is NULL.");
        return;
    }

    switch(scenarioId) {
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            x = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.previewWidth;
            y = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.previewHeight;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            x = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.captureWidth;
            y = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.captureHeight;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            x = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.videoWidth;
            y = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.videoHeight;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            x = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.video1Width;
            y = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.video1Height;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            x = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.video2Width;
            y = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.video2Height;
            break;
#if 0 // unsupport now
        case SENSOR_SCENARIO_ID_CUSTOM1:
            x = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST1;
            y = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST1;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM2:
            x = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST2;
            y = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST2;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM3:
            x = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST3;
            y = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST3;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM4:
            x = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST4;
            y = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST4;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM5:
            x = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST5;
            y = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST5;
            break;
#endif
        default:
            x = 0;
            y = 0;
            MY_LOGE("Error: scnario:%d sensorIdx:%d", scenarioId, sensorIdx);
            break;
    }
}

inline MUINT32
CAP_GET_SENSOR_FPS(
    MUINT32 const sensorIdx,
    MUINT const scenarioId
)
{
    MUINT32 fps = 0;

    if (IQ_CAPABILITY::m_pAllSensorInfo == NULL) {
        MY_LOGE("m_pAllSensorInfo is NULL.");
        return fps;
    }

    switch(scenarioId) {
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            fps = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.previewFrameRate;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            fps = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.captureFrameRate;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            fps = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.videoFrameRate;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            fps = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.video1FrameRate;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            fps = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.video2FrameRate;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM1:
            fps = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.custom1FrameRate;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM2:
            fps = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.custom2FrameRate;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM3:
            fps = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.custom3FrameRate;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM4:
            fps = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.custom4FrameRate;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM5:
            fps = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.custom5FrameRate;
            break;
        default:
            MY_LOGE("Error: scnario:%d sensorIdx:%d", scenarioId, sensorIdx);
            break;
    }
    return fps;
}

inline E_CAM_CTL_TG_FMT
CAP_GET_TG_HWFMT(
    MUINT32 sensorIdx
)
{
    E_CAM_CTL_TG_FMT HwVal = TG_FMT_RAW8;
    MUINT32 senType = 0;
    MUINT32 senBit = 0;

    if (IQ_CAPABILITY::m_pAllSensorInfo == NULL) {
        MY_LOGE("m_pAllSensorInfo is NULL.");
        return HwVal;
    }

    senType = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType;
    senBit  = IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.rawSensorBit;

    switch (senType) {
        case SENSOR_TYPE_RAW:
            {
                switch (senBit) {
                    case RAW_SENSOR_8BIT:
                        HwVal = TG_FMT_RAW8;
                        break;
                    case RAW_SENSOR_10BIT:
                        HwVal = TG_FMT_RAW10;
                        break;
                    case RAW_SENSOR_12BIT:
                        HwVal = TG_FMT_RAW12;
                        break;
                    case RAW_SENSOR_14BIT:
                        HwVal = TG_FMT_RAW14;
                        break;
                    default:
                        MY_LOGE("Fmt:[%d] NOT Support", senBit);
                        break;
                    }
            }
            break;
        case SENSOR_TYPE_YUV:
            {
                switch (senBit) {
                case SENSOR_FORMAT_ORDER_UYVY:
                case SENSOR_FORMAT_ORDER_VYUY:
                case SENSOR_FORMAT_ORDER_YVYU:
                case SENSOR_FORMAT_ORDER_YUYV:
                    HwVal = TG_FMT_YUV422;
                    break;
                default:
                    MY_LOGE("Fmt:[%d] NOT Support", senBit);
                    break;
                }
            }
            break;
        default:
            MY_LOGE("FmtType:[%d] NOT Support", senType);
            break;
    }
    return HwVal;
}

IQ_CAPABILITY*
IQ_CAPABILITY::
CreateInsatnce(
    char const* szCallerName
)
{
    static IQ_CAPABILITY Singleton;

    MINT32 len = strlen(szCallerName);
    if ((len == 0) || (len >= IOPIPE_MAX_USER_NAME_LEN)) {
        MY_LOGE("Ivalide user name length: %d", len);
        return MFALSE;
    }

    std::strncpy((char*)Singleton.m_Name, (char const*)szCallerName, len);

    if(szCallerName == NULL){
        MY_LOGE("no user name\n");
        return NULL;
    }

    if (IQ_CAPABILITY::m_pAllSensorInfo == NULL) {
        IHalSensorList*  pSList = NULL;
        MUINT32 SCnt = 0, i = 0;

        IQ_MEM_NEW(IQ_CAPABILITY::m_pAllSensorInfo, platSensorsInfo, sizeof(platSensorsInfo), IQ_CAPABILITY::mMemInfo);

        MY_LOGI("Caller:%s, pAllSensorInfo=0x%p", szCallerName, IQ_CAPABILITY::m_pAllSensorInfo);

        if (strcmp(szCallerName, "iopipeUseTM") == 0) {
            pSList = TS_FakeSensorList::getTestModel();
        }
        else {
            pSList = IHalSensorList::get();
        }
        if (IQ_CAPABILITY::m_pAllSensorInfo == NULL) {
            MY_LOGE("m_pAllSensorInfo is NULL");
            return NULL;
        }
        if (pSList == NULL) {
            MY_LOGE("pSList is NULL");
            return NULL;
        }

        SCnt =  pSList->queryNumberOfSensors();

        IQ_CAPABILITY::m_pAllSensorInfo->mSList       = pSList;
        IQ_CAPABILITY::m_pAllSensorInfo->mExistedSCnt = SCnt;
        if ((SCnt > IOPIPE_MAX_SENSOR_CNT) || (SCnt == 0)) {
            MY_LOGE("Not support %d sensors", SCnt);
            return MFALSE;
        }

        for (i = 0; i < SCnt; i++) {
            IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[i].mIdx       = i;
            IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[i].mTypeforMw = pSList->queryType(i);
            IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[i].mDevId     = pSList->querySensorDevIdx(i);
            pSList->querySensorStaticInfo(IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[i].mDevId, &IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[i].mSInfo);
            MY_LOGI("SensorId:%d, SensorName=%s, Type=%d, DevId=%d", i, pSList->queryDriverName(i),\
                    IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[i].mTypeforMw, IQ_CAPABILITY::m_pAllSensorInfo->mSenInfo[i].mDevId);
        }
    }

    return (IQ_CAPABILITY*)&Singleton;
}

void
IQ_CAPABILITY::
DestroyInstance(
    char const* szCallerName
)
{
    if(szCallerName == NULL){
        MY_LOGE("no user name\n");
        return;
    }

    IQ_MEM_DEL(IQ_CAPABILITY::m_pAllSensorInfo, sizeof(platSensorsInfo), IQ_CAPABILITY::mMemInfo);
}

MVOID
IQ_CAPABILITY::
IQtable_H(
    ResMgr *pResMgr,
    Res_Meter_IN &Meter_In,
    Res_Meter_OUT &Meter_Out,
    QueryOutData_t &IQunit
)
{
    Meter_In.offBin = MTRUE;
    Meter_In.offTwin = MFALSE;
    IQunit.IQlv = eCamIQ_H;

    if(pResMgr->Res_Meter(Meter_In, Meter_Out)){
        IQunit.clklv = Meter_Out.clk_level;
        IQunit.isTwin = Meter_Out.isTwin;
        IQunit.result = MTRUE;
    }
    else{
        MY_LOGE("Res_Meter is fail");
        IQunit.result = MFALSE;
    }
}

MVOID
IQ_CAPABILITY::
IQtable_L(
    ResMgr *pResMgr,
    Res_Meter_IN &Meter_In,
    Res_Meter_OUT &Meter_Out,
    QueryOutData_t &IQunit
)
{
    Meter_In.offBin = MFALSE;
    Meter_In.offTwin = MFALSE;
    IQunit.IQlv = eCamIQ_L;

    if(pResMgr->Res_Meter(Meter_In, Meter_Out)){
        IQunit.clklv = Meter_Out.clk_level;
        IQunit.isTwin = Meter_Out.isTwin;
        IQunit.result = MTRUE;
    }
    else{
        MY_LOGE("Res_Meter is fail");
        IQunit.result = MFALSE;
    }
}

MBOOL
IQ_CAPABILITY::
queryIQTable(
    vector<QueryInData_t> const vInData,
    vector<vector<QueryOutData_t>> &vOutData
)
{
    MBOOL ret = MTRUE;
    MINT32 SensorSum = 0, Sen_PixMode, i = 0, j = 0;
    char CallerName[32] = "NpipeIQContol";

    SensorSum = vInData.size();

    /*This API only for 1 and 2 sensor*/
    if((SensorSum == 0) || (SensorSum > 2)){
        MY_LOGE("sensor sum is not support!");
        return MFALSE;
    }
    if(vOutData.size() > 0){
        MY_LOGE("output vector must be empty!");
        return MFALSE;
    }
    if(IQ_CAPABILITY::m_pAllSensorInfo == NULL) {
        MY_LOGE("Error: m_pAllSensorInfo is NULL");
        return MFALSE;
    }
    else {
        m_pAllSensorInfo->mSList->searchSensors();
        for (i = 0 ; i < SensorSum ; i++){
             m_pAllSensorInfo->mSList->querySensorStaticInfo(m_pAllSensorInfo->mSList->querySensorDevIdx(vInData.at(i).sensorIdx), &(m_pAllSensorInfo->mSenInfo[vInData.at(i).sensorIdx].mSInfo));
        }
    }

    Res_Meter_IN src[SensorSum];
    Res_Meter_OUT dst[SensorSum];
    ResMgr *pResMgr[SensorSum];
    QueryOutData_t IQunit[SensorSum];

    for (j = 0; j < SensorSum; j++) {
        if (m_pHalSensor[j] == NULL) {
             m_pHalSensor[j] = m_pAllSensorInfo->mSList->createSensor(THIS_NAME, 1, &vInData.at(j).sensorIdx);
        }
        else {
            MY_LOGE("already createSensor[%d]", j);
            return MFALSE;
        }
        if (m_pHalSensor[j]  == NULL) {
            MY_LOGE("m_pHalSensor[%d] Fail", j);
            return MFALSE;
        }

        CAP_GET_TG_GRAB(vInData.at(j).sensorIdx, vInData.at(j).scenarioId, src[j].tg_crop_w, src[j].tg_crop_h);

        src[j].bypass_tg   = MFALSE;
        src[j].bypass_rrzo = MFALSE;
        src[j].pix_mode_tg = CAP_MAP_PXLMODE(vInData.at(j).sensorIdx, Sen_PixMode);
        src[j].tg_fps      = CAP_GET_SENSOR_FPS(vInData.at(j).sensorIdx, vInData.at(j).scenarioId);
        src[j].SrcImgFmt   = CAP_GET_TG_HWFMT(vInData.at(j).sensorIdx);

        src[j].rrz_out_w = vInData.at(j).rrz_out_w;
        src[j].pattern = vInData.at(j).pattern;
        //clk
        src[j].vClk.push_back(320 * 1000 * 1000);
        src[j].vClk.push_back(364 * 1000 * 1000);
        src[j].vClk.push_back(546 * 1000 * 1000);
        //src[j].MIPI_pixrate = 0;
        m_pHalSensor[j]->sendCommand(m_pAllSensorInfo->mSenInfo[vInData.at(j).sensorIdx].mDevId, SENSOR_CMD_GET_SENSOR_PIXELMODE,
                                (MUINTPTR)&vInData.at(j).sensorIdx, (MUINTPTR)&src[j].tg_fps , (MUINTPTR)&Sen_PixMode);

        MY_LOGI("vInData: sensorIdx(%d) scenarioId(%d) pix_mode_tg(%d) tg_fps(%d) SrcImgFmt(%d) rrz_out_w(%d) pattern(%d)",
            vInData.at(j).sensorIdx, vInData.at(j).scenarioId, (MUINT32)src[j].pix_mode_tg, src[j].tg_fps,
            (MUINT32)src[j].SrcImgFmt, src[j].rrz_out_w, (MUINT32)src[j].pattern);

        if (m_pHalSensor[j] != NULL) {
            m_pHalSensor[j]->destroyInstance(THIS_NAME);
            m_pHalSensor[j] = NULL;
        }
    }

    for(i = 0; i < SensorSum * 2; i++){
        vector<QueryOutData_t> vOut;

        /*Res_Attach*/
        for(j = 0; j < SensorSum; j++){
            pResMgr[j] = NULL;
            IQunit[j].sensorIdx = vInData.at(j).sensorIdx;
            pResMgr[j] = ResMgr::Res_Attach(user_camio, CallerName, src[j].tg_crop_w);
        }
        /*Res_Meter*/
        for(j = 0; j < SensorSum; j++){
            if(pResMgr[j] == NULL){
                MY_LOGE("pResMgr is Null");
                return MFALSE;
            }

            IQ_TBL[i][j](pResMgr[j], src[j], dst[j], IQunit[j]);

            if(IQunit[j].result == MTRUE){
                vOut.push_back(IQunit[j]);
            }
            else{
                MY_LOGE("Query Fail: i:%d j:%d fps:%d fmt:%d rrz_out_w:%d pattern:%d", i, j, src[j].tg_fps, src[j].SrcImgFmt, src[j].rrz_out_w, src[j].pattern);
                vOut.clear();
            }
        }

        /*Res_Detach*/
        for(j = 0; j < SensorSum; j++){
            if((pResMgr[j]->Res_Detach()) != MTRUE){
                MY_LOGW("Res_Detach() fail");
            }
        }

        //push back to vector
        vOutData.push_back(vOut);
    }

    //log
    for(i = 0 ; i < vOutData.size() ; i++){
        for(j = 0 ; j < vOutData[i].size() ; j++){
            MY_LOGI("[%d][%d] sensorIdx:%d isTwin:%d lvIQ:%d lvClk:%d result:%d", i, j, vOutData[i][j].sensorIdx, vOutData[i][j].isTwin, vOutData[i][j].IQlv, vOutData[i][j].clklv, vOutData[i][j].result);
        }
    }

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipeModule::
query(
    MUINT32 portIdx,
    MUINT32 eCmd,
    MINT imgFmt,
    NormalPipe_QueryIn const& input,
    NormalPipe_QueryInfo &queryInfo
)   const
{
    MBOOL ret = MTRUE;

    NormalPipe_InputInfo  qryInput;

    if(eCmd){
        CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

        qryInput.format     = (EImageFormat)imgFmt;
        qryInput.width      = input.width;
        qryInput.pixelMode  = input.pixMode;

        if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
            MY_LOGE("some query op fail\n");
            ret = MFALSE;
        }
        pinfo->DestroyInstance(LOG_TAG);
    }

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipeModule::
query(
    MUINT32 eCmd,
    MUINTPTR pIO_struct
)   const
{
    MBOOL ret = MTRUE;
    MUINT32 portIdx = 0;
    NormalPipe_InputInfo  qryInput;
    NormalPipe_QueryInfo  queryInfo;

    switch(eCmd){
        case ENPipeQueryCmd_X_PIX:
            {
                sCAM_QUERY_X_PIX *pQueryStruct = (sCAM_QUERY_X_PIX*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.x_pix;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_CROP_X_PIX:
            {
                sCAM_QUERY_CROP_X_PIX *pQueryStruct = (sCAM_QUERY_CROP_X_PIX*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.x_pix;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_X_BYTE:
            {
                sCAM_QUERY_X_BYTE *pQueryStruct = (sCAM_QUERY_X_BYTE*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.xsize_byte;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_CROP_X_BYTE:
            {
                sCAM_QUERY_CROP_X_BYTE *pQueryStruct = (sCAM_QUERY_CROP_X_BYTE*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.xsize_byte;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_CROP_START_X:
            {
                sCAM_QUERY_CROP_START_X *pQueryStruct = (sCAM_QUERY_CROP_START_X*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.crop_x;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_STRIDE_PIX:
            {
                sCAM_QUERY_STRIDE_PIX *pQueryStruct = (sCAM_QUERY_STRIDE_PIX*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }
                pQueryStruct->QueryOutput = queryInfo.stride_pix;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_STRIDE_BYTE:
            {
                sCAM_QUERY_STRIDE_BYTE *pQueryStruct = (sCAM_QUERY_STRIDE_BYTE*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.stride_byte;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_MAX_SEN_NUM:
            {
                sCAM_QUERY_MAX_SEN_NUM *pQueryStruct = (sCAM_QUERY_MAX_SEN_NUM*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.sen_num;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_PIPELINE_BITDEPTH:
            {
                sCAM_QUERY_PIPELINE_BITDEPTH *pQueryStruct = (sCAM_QUERY_PIPELINE_BITDEPTH*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.pipelinebitdepth;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_QUERY_FMT:
            {
                sCAM_QUERY_QUERY_FMT *pQueryStruct = (sCAM_QUERY_QUERY_FMT*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx = pQueryStruct->QueryInput.portId;
                qryInput.SecOn= pQueryStruct->QueryInput.SecOn;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.query_fmt;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_BURST_NUM:
            {
                sCAM_QUERY_BURST_NUM *pQueryStruct = (sCAM_QUERY_BURST_NUM*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.burstNum;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_SUPPORT_PATTERN:
            {
                sCAM_QUERY_SUPPORT_PATTERN *pQueryStruct = (sCAM_QUERY_SUPPORT_PATTERN*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.pattern;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_BS_RATIO:
            {
                sCAM_QUERY_BS_RATIO *pQueryStruct = (sCAM_QUERY_BS_RATIO*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx = pQueryStruct->QueryInput.portId;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.bs_ratio;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_D_Twin:
            {
                sCAM_QUERY_D_Twin *pQueryStruct = (sCAM_QUERY_D_Twin*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.D_TWIN;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_D_BayerEnc:
            {
                sCAM_QUERY_D_BAYERENC *pQueryStruct = (sCAM_QUERY_D_BAYERENC*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.function.Bits.D_BayerENC;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_IQ_LEVEL:
            {
                sCAM_QUERY_IQ_LEVEL *pQueryStruct = (sCAM_QUERY_IQ_LEVEL*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.IQlv;
                pinfo->DestroyInstance(LOG_TAG);

                if (queryInfo.IQlv) {
                    IQ_CAPABILITY* pIQinfo = IQ_CAPABILITY::CreateInsatnce(LOG_TAG);
                    if(pIQinfo->queryIQTable(pQueryStruct->QueryInput.vInData, pQueryStruct->QueryInput.vOutData) == MFALSE){
                        MY_LOGE("queryIQTable fail\n");
                        return MFALSE;
                    }
                    pIQinfo->DestroyInstance(LOG_TAG);
                }
            }
            break;
        case ENPipeQueryCmd_PDO_AVAILABLE:
            {
                sCAM_QUERY_PDO_AVAILABLE *pQueryStruct = (sCAM_QUERY_PDO_AVAILABLE*)pIO_struct;
                pQueryStruct->QueryOutput = MTRUE; //~isp3.0:pdo not supported, isp4.0~:pdo supported
            }
            break;
        case ENPipeQueryCmd_DYNAMIC_PAK:
            {
                sCAM_QUERY_DYNAMIC_PAK *pQueryStruct = (sCAM_QUERY_DYNAMIC_PAK*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.D_Pak;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_MAX_PREVIEW_SIZE:
            {
                sCAM_QUERY_MAX_PREVIEW_SIZE *pQueryStruct = (sCAM_QUERY_MAX_PREVIEW_SIZE*)pIO_struct;
                pQueryStruct->QueryOutput = 8192; //isp5.0:max rrzo width 8192
            }
            break;

        case ENPipeQueryCmd_HW_RES_MGR:
            {
                sCAM_QUERY_HW_RES_MGR *pQueryStruct = (sCAM_QUERY_HW_RES_MGR*)pIO_struct;

                // clear output vector
                pQueryStruct->QueryOutput.clear();

                for(MUINT32 i = 0 ; i < pQueryStruct->QueryInput.size() ; i++){
                    PIPE_SEL pipeSel;
                    pipeSel.sensorIdx = pQueryStruct->QueryInput.at(i).sensorIdx;
                    pipeSel.pipeSel = (pQueryStruct->QueryInput.at(i).stt_off)?EPipeSelect_NormalSv:EPipeSelect_Normal;
                    pQueryStruct->QueryOutput.push_back(pipeSel);
                }
            }
            break;
        case ENPipeQueryCmd_SEC_CAP:
            {
                sCAM_QUERY_SEC_CAP *pQueryStruct = (sCAM_QUERY_SEC_CAP*)pIO_struct;
                MBOOL CapMode = MFALSE;
                SecMgr *mpSecMgr = SecMgr::SecMgr_GetMgrObj();
                if(mpSecMgr){
                    mpSecMgr->SecMgr_sendCommand(SECMGRCmd_QUERY_CAP_CONSTRAINT,(MINTPTR)&CapMode,0,0);
                    pQueryStruct->QueryOutput = CapMode;
                }else{
                    MY_LOGE("Get SecMgr object failed\n");
                    return MFALSE;
                }
            }
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unsupport cmd(0x%x)\n",eCmd);
            break;
    }

    return ret;
}

