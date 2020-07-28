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
#define LOG_TAG "Iop/P2NStm"

#include "NormalStreamWrapper.h"

#include <IPipe.h>
#include <IPostProcPipe.h>
#include <mtkcam/drv/def/ispio_sw_scenario.h>
#include <vector>
#include <cutils/atomic.h>
#include <cutils/properties.h>  // For property_get().
#include "NormalStreamUpper.h"
#include <mtkcam/drv/def/IPostProcDef.h>

/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(P2NormalStream);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (P2NormalStream_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (P2NormalStream_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (P2NormalStream_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (P2NormalStream_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (P2NormalStream_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (P2NormalStream_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)



#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
using namespace std;

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;
using namespace NSCam::NSIoPipe::NSPostProc::Wrapper;

ESoftwareScenario const gSenDevSwScenMap[ENormalStreamTag_total][SENSOR_DEV_SUB_2+1]=
{
    {eSoftwareScenario_Main_Normal_Stream,eSoftwareScenario_Main_Normal_Stream,eSoftwareScenario_Sub_Normal_Stream,\
        eSoftwareScenario_total_num,eSoftwareScenario_Main2_Normal_Stream,eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_Sub2_Normal_Stream},//ENormalStreamTag_Prv
    //{eSoftwareScenario_Main_Normal_Capture,eSoftwareScenario_Main_Normal_Capture,eSoftwareScenario_Sub_Normal_Capture,\
        //eSoftwareScenario_total_num,eSoftwareScenario_Main2_Normal_Capture,eSoftwareScenario_total_num},//ENormalStreamTag_Cap
    {eSoftwareScenario_Main_VSS_Capture,eSoftwareScenario_Main_VSS_Capture,eSoftwareScenario_Sub_VSS_Capture,\
        eSoftwareScenario_total_num,eSoftwareScenario_Main2_VSS_Capture,eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_Sub2_VSS_Capture},//ENormalStreamTag_Vss
    {},
    {eSoftwareScenario_Main_Mfb_Blending,eSoftwareScenario_Main_Mfb_Blending,eSoftwareScenario_Sub_Mfb_Blending,\
        eSoftwareScenario_total_num,eSoftwareScenario_Main2_Mfb_Blending,eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_Sub2_Mfb_Blending},//ENormalStreamTag_MFB_Bld
    {eSoftwareScenario_Main_Mfb_Mixing,eSoftwareScenario_Main_Mfb_Mixing,eSoftwareScenario_Sub_Mfb_Mixing,\
        eSoftwareScenario_total_num,eSoftwareScenario_Main2_Mfb_Mixing,eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_Sub2_Mfb_Mixing},//ENormalStreamTag_MFB_Mix
    {},{},{},{},{},{},{},
    {eSoftwareScenario_Main_IP_Raw_TPipe_Stream,eSoftwareScenario_Main_IP_Raw_TPipe_Stream,eSoftwareScenario_Sub_IP_Raw_TPipe_Stream,\
        eSoftwareScenario_total_num,eSoftwareScenario_Main2_IP_Raw_TPipe_Stream,eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_total_num,eSoftwareScenario_Sub2_IP_Raw_TPipe_Stream}//ENormalStreamTag_IP_Tpipe
};

extern NSCam::NSIoPipeIsp3::NSPostProc::SwHwScenarioPathMapping mSwHwPathMapping[eSoftwareScenario_total_num];
/******************************************************************************
 *
 ******************************************************************************/
//for IP base
INormalStream*
INormalStream::
createInstance(
    MUINT32 openedSensorIndex, DipUserParam UserParam)
{
    return NormalStreamUpper::createInstance(
        openedSensorIndex, UserParam,
        [] (MUINT32 openedSensorIndex, DipUserParam UserParam) { return new NormalStream(openedSensorIndex, UserParam); }
    );
}

/*
//fadeout, remove later
INormalStream*
INormalStream::
createInstance(
    char const* szCallerName,
    ENormalStreamTag streamTag,
    MUINT32 openedSensorIndex,
    MBOOL isV3)
{
    szCallerName;streamTag;openedSensorIndex;isV3;
    LOG_ERR("plz use createInstance(MUINT32 openedSensorIndex)");
    return NULL;
    //FUNCTION_LOG_START;
    //[1] create HalPipeWrapper
    //return new NormalStream(openedSensorIndex,isV3);
    //FUNCTION_LOG_END;
}
*/

/******************************************************************************
*
******************************************************************************/
MUINT32
INormalStream::
getRegTableSize(void)
{
    MUINT32 size= sizeof(isp_reg_t);

    return size;
}


/******************************************************************************
*
******************************************************************************/
MBOOL
INormalStream::
queryDIPInfo(std::map<EDIPInfoEnum, MUINT32> &mapDipInfo)
{
    mapDipInfo[EDIPINFO_DIPVERSION] = EDIPHWVersion_30;
    mapDipInfo[EDIPINFO_BATCH_FRAME_BUFFER_COUNT] = 0x0;
    mapDipInfo[EDIPINFO_PER_FRAME_CB_SUPPORT] = MFALSE;
    mapDipInfo[EDIPINFO_FM_WIDTH_LIMITATION] = 2304;
    return MTRUE;
}


/******************************************************************************
*
******************************************************************************/
enum EDIPSecDMAType
INormalStream::
needSecureBuffer(MUINT32 portIdx)
{
    enum EDIPSecDMAType isSecPort = EDIPSecDMAType_TUNE;

    switch (portIdx) {
        case EPortIndex_IMGBI:
        case EPortIndex_IMGCI:
        case EPortIndex_UFDI:
        case EPortIndex_LCEI:
        case EPortIndex_DMGI:
        case EPortIndex_DEPI:
            isSecPort = EDIPSecDMAType_TUNE_SHARED;
            break;
        case EPortIndex_MFBO:
        case EPortIndex_FEO:
            isSecPort = EDIPSecDMAType_TUNE;
            break;
        case EPortIndex_IMGI:
        case EPortIndex_WROTO:
        case EPortIndex_WDMAO:
            isSecPort = EDIPSecDMAType_IMAGE;
            break;
        case EPortIndex_VIPI:
        case EPortIndex_VIP2I:
        case EPortIndex_VIP3I:
        case EPortIndex_IMG2O:
        case EPortIndex_IMG2BO:
        case EPortIndex_IMG3O:
        case EPortIndex_IMG3BO:
        case EPortIndex_IMG3CO:
        default:
            LOG_ERR("[DIP Secure] Not support port index (%d)", portIdx);
            break;
    }

    return isSecPort;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
NormalStream::
destroyInstance()
{
    FUNCTION_LOG_START;

    if(mpHalPipeWrapper==NULL)
    {
        LOG_INF("pipeID(%d),NULL mpHalPipeWrapper.",mPipeID);
    }
    else
    {
        mpHalPipeWrapper->destroyInstance();
        mpHalPipeWrapper=NULL;
    }

    if(mpPostProcPipe==NULL)
    {
        LOG_INF("pipeID(%d),NULL mpHalPipeWrapper.",mPipeID);
    }
    else
    {
        mpPostProcPipe->destroyInstance();
        mpPostProcPipe=NULL;
    }

    FUNCTION_LOG_END;
    delete this;
}
/*******************************************************************************
*
********************************************************************************/
NormalStream::
NormalStream(
    MUINT32 openedSensorIndex, DipUserParam UserParam)    //change to use sensor index
            : m_UserParam(UserParam)
            , mPipeID(EStreamPipeID_Normal)
            , mpHalPipeWrapper(NULL)
            , mpPostProcPipe(NULL)
            , mOpenedSensor(SENSOR_DEV_NONE)
            , mHalSensorList(NULL)
            , mStreamTag(ENormalStreamTag_Prv)
            , mJpegWorkBufSize(0)
            , mSWScen(eSoftwareScenario_total_num)
            , misV3(1)
            , pixIdP2(0)
{
   //
   DBG_LOG_CONFIG(iopipe, P2NormalStream);

   mpHalPipeWrapper = NSIoPipeIsp3::NSPostProc::HalPipeWrapper::createInstance();
   if(openedSensorIndex==0xFFFF)
   {    //pure pass2
        mOpenedSensor=SENSOR_DEV_NONE;
   }
   else
   {
       //p1+p2, get sensor dev firstly
       #ifndef USING_MTK_LDVT
       mHalSensorList=IHalSensorList::get();

       if(mHalSensorList)
       {
            //[1] get sensor dev index from sensorID
            //get sensor output pix id
            SensorStaticInfo SensorStaticInfo;
            MINT32 sensorDEVIdx=mHalSensorList->querySensorDevIdx(openedSensorIndex);
            mOpenedSensor=static_cast<MUINT32>(sensorDEVIdx);
            mHalSensorList->querySensorStaticInfo(mOpenedSensor, &SensorStaticInfo);
            pixIdP2 = SensorStaticInfo.sensorFormatOrder;
            LOG_INF("query from sensor openedSensorIndex (%d), eRawPxlID(%d)",mOpenedSensor,pixIdP2);
       }
       else
       {
            LOG_ERR("NULL mHalSensorList,sidx/sdev(0x%x/0x%x)",openedSensorIndex,mOpenedSensor);
       }
       #else
       //default use main sensor in ldvt load is user pass in sensor index
       mOpenedSensor=SENSOR_DEV_MAIN;
       LOG_INF("in LDVT load, sdev(0x%x)",mOpenedSensor);
       #endif
   }

   //default value for jpeg
   mJpegCfg.soi_en=1;   //fadeout, remove later
   mJpegCfg.fQuality=90;      //fadeout, remove later
   LOG_INF("sidx/sdev(0x%x/0x%x),swPipe cID(0x%lx)",openedSensorIndex,mOpenedSensor,(unsigned long)(&mDequeuedBufList));

}


/*******************************************************************************
*
********************************************************************************/
NormalStream::~NormalStream()
{

}

/******************************************************************************
 *
 ******************************************************************************/
// for IP base
MBOOL
NormalStream::
init(char const* szCallerName, MUINT32 secTag)
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    MBOOL ret = MTRUE;
    //MUINT32 cropPathNum=0; //not used

    //[1] init HalPipeWrapper, and caller postproc pipe object would be created at this stage according to different sw scenario

    //[IP BASE] Since we don't know mStreamTag in NormalStream init in ISP 4.0,
    // we use ENormalStreamTag_Prv first and replace it in enque
    if(mOpenedSensor == SENSOR_DEV_MAIN_3)
        mOpenedSensor = SENSOR_DEV_SUB_2;
    mSWScen=gSenDevSwScenMap[ENormalStreamTag_Prv][(MINT32)mOpenedSensor];
    if(mSWScen==eSoftwareScenario_total_num)
    {
        LOG_ERR("No support, sTag/sensorDev (%d)/(0x%x)!",mStreamTag,mOpenedSensor);
        ret=false;
        return ret;
    }
    else
    {
        if(mpHalPipeWrapper)
        {
            LOG_INF("sTag/sensorDev/swScen (%d)/(0x%x)/(%d)",mStreamTag,mOpenedSensor,mSWScen);
                     mpHalPipeWrapper->init(mSWScen,(unsigned long &)(this->mpPostProcPipe),misV3,pixIdP2);
            if(!ret)
            {
                LOG_ERR("mpHalPipeWrapper init fail, sz/pipeID (%s/%d)",szCallerName,mPipeID);
                return ret;
            }
        }
        else
        {
            LOG_ERR("NULL mpHalPipeWrapper, sz (%s)",szCallerName);
            return MFALSE;
        }
    }


    //[2] get supported crop paths
    /*
    if(this->mpPostProcPipe)
    {
        //query crop information
        mpPostProcPipe->queryCropPathNum(mSwHwPathMapping[mSWScen].hwPath, cropPathNum);
        for(int i=0;i<cropPathNum;i++)
        {
            MCropPathInfo crop;
            mCropPaths.push_back(crop);
        }
        //mpPostProcPipe->queryScenarioInfo(mSwHwPathMapping[mSWScen].hwPath, mCropPaths);
    }
    else
    {
        LOG_ERR("sTag(%d),Init fail!",mStreamTag);
        ret=false;
    }
    */
    //[3] set default value for jpg related params
    mpPostProcPipe->sendCommand(NSImageioIsp3::NSIspio::EPIPECmd_SET_JPEG_CFG, (MINTPTR)&mJpegCfg,0,0);

    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
//for legacy chip
MBOOL
NormalStream::
init(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID StreamPipeID, MUINT32 secTag)
{
    szCallerName;StreamPipeID;
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    MBOOL ret = MTRUE;
    //MUINT32 cropPathNum=0; //not used

    //[1] init HalPipeWrapper, and caller postproc pipe object would be created at this stage according to different sw scenario
    //[IP BASE] Since we don't know mStreamTag in NormalStream init in ISP 4.0,
    // we use ENormalStreamTag_Prv first and replace it in enque

    if(!szCallerName)
    {
        LOG_ERR("plz add userName");
        return MFALSE;
    }

    if(!StreamPipeID)
    {
        LOG_ERR("plz add Stream PipeID");
        return MFALSE;
    }

    mPipeID = StreamPipeID;

    if(mOpenedSensor == SENSOR_DEV_MAIN_3)
        mOpenedSensor = SENSOR_DEV_SUB_2;

    mSWScen=gSenDevSwScenMap[ENormalStreamTag_Prv][(MINT32)mOpenedSensor];
    if(mSWScen==eSoftwareScenario_total_num)
    {
         LOG_ERR("No support, sTag/sensorDev (%d)/(0x%x)!",mStreamTag,mOpenedSensor);
         ret=false;
         return ret;
     }
     else
     {
         if(mpHalPipeWrapper)
         {
             LOG_INF("sTag/sensorDev/swScen (%d)/(0x%x)/(%d), pipeID(%d)",mStreamTag,mOpenedSensor,mSWScen, StreamPipeID);
             mpHalPipeWrapper->init(mSWScen,(unsigned long &)(this->mpPostProcPipe),misV3,pixIdP2);
             if(!ret)
             {
                 LOG_ERR("mpHalPipeWrapper init fail, sz/pipeID (%s/%d)",szCallerName,mPipeID);
                 return ret;
             }
         }
         else
         {
             LOG_ERR("NULL mpHalPipeWrapper, sz (%s)",szCallerName);
             return MFALSE;
         }
     }

    //[3] set default value for jpg related params
    mpPostProcPipe->sendCommand(NSImageioIsp3::NSIspio::EPIPECmd_SET_JPEG_CFG, (MINTPTR)&mJpegCfg,0,0);

    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalStream::
uninit(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID SteamPipeID)
{
    szCallerName;SteamPipeID;
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    MBOOL ret = MTRUE;

    //[1] uninit postproc
    if (NULL != mpPostProcPipe)
    {
        if (true != mpPostProcPipe->uninit())
        {
            LOG_ERR("sTag(%d),caller PostProc uninit fail",mStreamTag);
            ret = false;
            return ret;
        }
    }

    //[2] uninit HalPipeWrapper
    if(mSWScen==eSoftwareScenario_total_num)
    {
        LOG_ERR("No support, sTag/sensorDev (%d)/(0x%x), pipeID(%d)!",mStreamTag,mOpenedSensor,  SteamPipeID);
        ret=false;
        return ret;
    }
    else
    {
        LOG_INF("sTag/sensorDev/swScen (%d)/(0x%x)/(%d), pipeID(%d)!",mStreamTag,mOpenedSensor,mSWScen, SteamPipeID);
        mpHalPipeWrapper->uninit(mSWScen,this->mpPostProcPipe);
    }

    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalStream::
uninit(char const* szCallerName)
{
    szCallerName;
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    MBOOL ret = MTRUE;

    //[1] uninit postproc
    if (NULL != mpPostProcPipe)
    {
        if (true != mpPostProcPipe->uninit())
        {
            LOG_ERR("sTag(%d),caller PostProc uninit fail",mStreamTag);
            ret = false;
            return ret;
        }
    }

    //[2] uninit HalPipeWrapper
    if(mSWScen==eSoftwareScenario_total_num)
    {
        LOG_ERR("No support, sTag/sensorDev (%d)/(0x%x)!",mStreamTag,mOpenedSensor);
        ret=false;
        return ret;
    }
    else
    {
        LOG_INF("sTag/sensorDev/swScen (%d)/(0x%x)/(%d)",mStreamTag,mOpenedSensor,mSWScen);
        mpHalPipeWrapper->uninit(mSWScen,this->mpPostProcPipe);
    }

    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalStream::
enque(NSCam::NSIoPipe::QParams const& rParams)
{
    //FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;

    //error handle
    if(rParams.mvFrameParams.size()<1)
    {
        LOG_ERR("unexpected frameParas size(%lu)", (unsigned long)rParams.mvFrameParams.size());
        return MFALSE;
    }

    //do not support multi-frame containing vss
    if(rParams.mvFrameParams.size() > 1)
    {
        for(MUINT32 i=0;i<rParams.mvFrameParams.size();i++)
        {
            if(rParams.mvFrameParams[i].mStreamTag == ENormalStreamTag_Vss)
            {
                LOG_ERR("do not support multi-frame containing vss (%d of %d)", i, rParams.mvFrameParams.size());
                return MFALSE;
            }
        }
    }

    // IP BASE : mapping QParams from 4.0 to 3.0
    mStreamTag = (ENormalStreamTag)rParams.mvFrameParams[0].mStreamTag;
    mSWScen=gSenDevSwScenMap[mStreamTag][(MINT32)mOpenedSensor];
    LOG_INF("Enque sTag/mSWScen (%d)/(%d)!",mStreamTag,mSWScen);

    if(mSWScen==eSoftwareScenario_total_num)
    {
        LOG_ERR("No support, sTag/sensorDev (%d)/(0x%x)!",mStreamTag,mOpenedSensor);
        ret=false;
        return ret;
    }
    else
    {
        NSIoPipeIsp3::NSPostProc::EBufferTag tag=NSIoPipeIsp3::NSPostProc::EBufferTag_Blocking;
        switch(mSWScen)
        {
            case eSoftwareScenario_Main_VSS_Capture:
            case eSoftwareScenario_Sub_VSS_Capture:
            case eSoftwareScenario_Main2_VSS_Capture:
            case eSoftwareScenario_Sub2_VSS_Capture:
                mpHalPipeWrapper->checkVssStatus(); // For Vss Concurrency Check
                tag=NSIoPipeIsp3::NSPostProc::EBufferTag_Vss;
                break;
            default:
                break;
        }
        NSIoPipeIsp3::NSPostProc::QFeatureData dummyData;
        ret=mpHalPipeWrapper->enque(mSWScen,pixIdP2,(rParams),this->mpPostProcPipe,this->mDequeuedBufList,tag,dummyData);
        if(!ret)
        {
            LOG_ERR("enque Fail");
        }
    }

    return ret;

}

/* for ISP3.0 build pass
MBOOL
NormalStream::
enque(
    NSCam::NSIoPipe::NSPostProc::QParams const& rParams)
{
    //FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;
    //FUNCTION_LOG_END;
    return ret;

}
*/
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalStream::
deque(
    NSCam::NSIoPipe::QParams& rParams,
    MINT64 i8TimeoutNs)
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = false;

    if(i8TimeoutNs == -1)   //temp solution for infinite wait
    {
        LOG_INF("no timeout set, infinite wait");
        i8TimeoutNs=30000000000;
    }

    LOG_DBG("sTag(%d),i8TimeoutNs(%lld)",mStreamTag,i8TimeoutNs);
    NSIoPipeIsp3::NSPostProc::QFeatureData dummyData;
    ret=mpHalPipeWrapper->deque(mSWScen,(rParams),this->mpPostProcPipe,(MUINTPTR)(&mDequeuedBufList),i8TimeoutNs,dummyData);

    FUNCTION_LOG_END;
    return ret;
}

/* for ISP3.0 build pass
MBOOL
NormalStream::
deque(
    NSCam::NSIoPipe::NSPostProc::QParams& rParams,
    MINT64 i8TimeoutNs)
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;
    FUNCTION_LOG_END;
    return ret;
}
*/
/******************************************************************************
*
******************************************************************************/

MBOOL
NormalStream::
setJpegParam(
    EJpgCmd jpgCmd,
    int arg1,
    int arg2)
{
    jpgCmd;arg1;arg2;
    FUNCTION_LOG_START;
    bool ret = true;
    if(mpPostProcPipe==NULL)
    {
        LOG_ERR("sTag(%d),NULL mpPostProcPipe",mStreamTag);
        return false;
    }
    switch(jpgCmd)
    {
        case EJpgCmd_SetWorkBufSize:    //setJpegParam(EJpgCmd_SetWorkBufSize,0,0)
            mpPostProcPipe->sendCommand(NSImageioIsp3::NSIspio::EPIPECmd_SET_JPEG_WORKBUF_SIZE, mJpegWorkBufSize,0,0);
            break;
        case EJpgCmd_SetQualityParam:
            mJpegCfg.soi_en=arg1;
            mJpegCfg.fQuality=arg2;     //setJpegParam(EJpgCmd_SetQualityParam,soi_en,fQuality)
            mpPostProcPipe->sendCommand(NSImageioIsp3::NSIspio::EPIPECmd_SET_JPEG_CFG, (MINTPTR)&mJpegCfg,0,0);
            break;
        default:
            break;
    }
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
*
******************************************************************************/
// ISP 3.0 do not have this func.

MBOOL
NormalStream::
setFps(
    MINT32 fps)
{
    fps;
    Mutex::Autolock lock(mModuleMtx);
/*
    if(mpHalPipeWrapper)
    {
        mpHalPipeWrapper->setFps(fps);
    }
    else
    {
        LOG_ERR("NULL mpHalPipeWrapper");
        return MFALSE;
    }
*/
    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
// ISP 3.0 do not have this func.

MBOOL
NormalStream::
sendCommand(
    ESDCmd cmd,
    MINTPTR arg1,
    MINTPTR arg2,
    MINTPTR arg3)
{
    cmd;arg1;arg2;arg3;
    bool ret=true;
    Mutex::Autolock lock(mModuleMtx);
/*
    NSImageio::NSIspio::EPIPECmd ccmd = NSImageio::NSIspio::EPIPECmd_NONE;
    switch(cmd)
    {
        case ESDCmd_CONFIG_VENC_DIRLK:
            ccmd = NSImageio::NSIspio::EPIPECmd_SET_VENC_DRLINK;
            break;
        case ESDCmd_RELEASE_VENC_DIRLK:
            ccmd = NSImageio::NSIspio::EPIPECmd_RELEASE_VENC_DRLINK;
            break;
        default:
            break;
    }
    //
    if(mpHalPipeWrapper)
    {
        ret=mpHalPipeWrapper->sendCommand(ccmd, arg1, arg2, arg3);
        if(!ret)
        {
            LOG_ERR("[Error]sendCommand(0x%x, 0x%x,0x%lx,0x%lx,0x%lx) Fail",cmd,ccmd,(long)arg1,(long)arg2,(long)arg3);
        }
    }
    else
    {
        LOG_ERR("NULL mpHalPipeWrapper");
        return MFALSE;
    }
*/
    return ret;
}



///remove later
/******************************************************************************
*
******************************************************************************/
MBOOL
NormalStream::
startVideoRecord(
	MINT32 wd,
	MINT32 ht,
	MINT32 fps)
{
    FUNCTION_LOG_START;
    LOG_INF("mSWScen(%d),mpHalPipeWrapper()0x%lx",mSWScen,(unsigned long)mpHalPipeWrapper);
    bool ret=true;
    //
    if(mSWScen==eSoftwareScenario_total_num)
    {
        LOG_ERR("plz do init first");
        ret=false;
    }
    else
    {
        if(mpHalPipeWrapper)
        {
            ret=mpHalPipeWrapper->startVideoRecord(wd,ht,fps,mSWScen);
        }
        else
        {
            LOG_ERR("Null pointer");
            ret=false;
        }
    }
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
NormalStream::
stopVideoRecord()
{
    FUNCTION_LOG_START;
    bool ret=true;
    //
    if(mSWScen==eSoftwareScenario_total_num)
    {
        LOG_ERR("plz do init first");
        ret=false;
    }
    else
    {
        if(mpHalPipeWrapper)
        {
            ret=mpHalPipeWrapper->stopVideoRecord(mSWScen);
        }
        else
        {
            LOG_ERR("Null pointer");
            ret=false;
        }
    }
    FUNCTION_LOG_END;
    return ret;
}
