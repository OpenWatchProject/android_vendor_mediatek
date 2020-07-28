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

#include "NormalStream.h"

#include <IPipe.h>
#include <IPostProcPipe.h>
#include <mtkcam/drv/def/ispio_sw_scenario.h>
#include <vector>
#include <cutils/atomic.h>
#include <cutils/properties.h>  // For property_get().


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

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;


/******************************************************************************
 *
 ******************************************************************************/
 INormalStream*
INormalStream::
createInstance(
    MUINT32 openedSensorIndex, DipUserParam UserParam)
{
    //[1] create HalPipeWrapper
    return new NormalStream(openedSensorIndex, UserParam);
}


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

/******************************************************************************
*
******************************************************************************/
MUINT32
INormalStream::
getRegTableSize(void)
{
	MUINT32 size=sizeof(dip_x_reg_t);

    return size;
}


/******************************************************************************
*
******************************************************************************/
MBOOL
INormalStream::
queryDIPInfo(std::map<EDIPInfoEnum, MUINT32> &mapDipInfo)
{
    mapDipInfo[EDIPINFO_DIPVERSION] = 0x40;
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
    FUNCTION_LOG_END;
    delete this;
}
/*******************************************************************************
*
********************************************************************************/
NormalStream::
NormalStream(
    MUINT32 openedSensorIndex, DipUserParam UserParam)    //change to use sensor index
            : mPipeID(EStreamPipeID_Normal)
            , mpHalPipeWrapper(NULL)
            , mOpenedSensor(SENSOR_DEV_NONE)
            , mHalSensorList(NULL)
            , m_UserParam(UserParam)
            , mJpegWorkBufSize(0)
{
   //
   DBG_LOG_CONFIG(iopipe, P2NormalStream);
   memset((char*)this->m_UserName, '\0', MAX_USER_NAME_LEN);
   m_UserName[strlen(this->m_UserName)]='\0';
   mpHalPipeWrapper=HalPipeWrapper::createInstance();
   if(openedSensorIndex==0xFFFF)
   {    //pure pass2
        mOpenedSensor=SENSOR_DEV_NONE;
        mP2PixId = 0x0;
       LOG_INF("0xffff, sdev(0x%x), pxlID(0x%x)",mOpenedSensor,mP2PixId);
   }
   else
   {
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
            mP2PixId = SensorStaticInfo.sensorFormatOrder;
            LOG_INF("query from sensor openedSensorIndex (%d), eRawPxlID(%d)",mOpenedSensor,mP2PixId);
       }
       else
       {
            LOG_ERR("NULL mHalSensorList,pipeID/sidx/sdev(%d/0x%x/0x%x)",mPipeID,openedSensorIndex,mOpenedSensor);
       }
       #else
       //default use main sensor in ldvt load is user pass in sensor index
       mOpenedSensor=SENSOR_DEV_MAIN;
       mP2PixId = 0x0;
       LOG_INF("in LDVT load, sdev(0x%x), pxlID(0x%x)",mOpenedSensor,mP2PixId);
       #endif
   }

   //default value for jpeg
   mJpegCfg.soi_en=1;   //fadeout, remove later
   mJpegCfg.fQuality=90;      //fadeout, remove later
   LOG_INF("pipeID/sidx/sdev/pxlID(%d/0x%x/0x%x/0x%x),swPipe cID(0x%lx)",mPipeID,openedSensorIndex,mOpenedSensor,mP2PixId,(unsigned long)(&mDequeuedBufList));
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
MBOOL
NormalStream::
init(char const* szCallerName, MUINT32 secTag)
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    MBOOL ret = MTRUE;
    MUINT32 cropPathNum=0;

    //[1] init HalPipeWrapper
    if((szCallerName !=NULL) && (strlen(szCallerName) < MAX_USER_NAME_LEN))
    {
        strncpy((char*)m_UserName,(char const*)szCallerName, strlen(szCallerName));
        m_UserName[strlen(szCallerName)]='\0';
    }
    else
    {
        LOG_ERR("plz add userName");
        return MFALSE;
    }
    
    LOG_INF("sz/pipeID/openedSensor/pixId (%s/%d/0x%x/0x%x)",szCallerName,mPipeID,mOpenedSensor,mP2PixId);
    if(mpHalPipeWrapper)
    {
        ret=mpHalPipeWrapper->init(szCallerName, mPipeID, secTag);
        if(!ret)
        {
            LOG_ERR("mpHalPipeWrapper init fail, sz/pipeID (%s/%d)",szCallerName,mPipeID);
            return ret;
        }
    }
    else
    {
        LOG_ERR("NULL mpHalPipeWrapper, sz/pipeID (%s/%d)",szCallerName,mPipeID);
        return MFALSE;
    }
    
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalStream::
init(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID StreamPipeID, MUINT32 secTag)
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    MBOOL ret = MTRUE;
    MUINT32 cropPathNum=0;

    //[1] init HalPipeWrapper
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
	
    LOG_INF("sz/pipeID/openedSensor/pixId (%s/%d/0x%x/0x%x)",szCallerName,StreamPipeID,mOpenedSensor,mP2PixId);
    if(mpHalPipeWrapper)
    {
        ret=mpHalPipeWrapper->init(szCallerName, StreamPipeID, secTag);
        if(!ret)
        {
            LOG_ERR("mpHalPipeWrapper init fail, sz/pipeID (%s/%d)",szCallerName,StreamPipeID);
            return ret;
        }
    }
    else
    {
        LOG_ERR("NULL mpHalPipeWrapper, sz/pipeID (%s/%d)",szCallerName,mPipeID);
        return MFALSE;
    }
    
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
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    MBOOL ret = MTRUE;

    //[1] uninit HalPipeWrapper
    LOG_INF("sz/pipeID/openedSensor/pixId (%s/%d/0x%x/0x%x)",szCallerName,SteamPipeID,mOpenedSensor,mP2PixId);
    if(mpHalPipeWrapper)
    {
        ret=mpHalPipeWrapper->uninit(szCallerName, SteamPipeID);
        if(!ret)
        {
            LOG_ERR("mpHalPipeWrapper init fail, sz/pipeID (%s/%d)",szCallerName,SteamPipeID);
            return ret;
        }
    }
    else
    {
        LOG_ERR("NULL mpHalPipeWrapper, sz/pipeID (%s/%d)",szCallerName,SteamPipeID);
        return MFALSE;
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
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    MBOOL ret = MTRUE;

    //[1] uninit HalPipeWrapper
    LOG_INF("sz/pipeID/openedSensor/pixId (%s/%d/0x%x/0x%x)",szCallerName,mPipeID,mOpenedSensor,mP2PixId);
    if(mpHalPipeWrapper)
    {
        ret=mpHalPipeWrapper->uninit(szCallerName, mPipeID);
        if(!ret)
        {
            LOG_ERR("mpHalPipeWrapper init fail, sz/pipeID (%s/%d)",szCallerName,mPipeID);
            return ret;
        }
    }
    else
    {
        LOG_ERR("NULL mpHalPipeWrapper, sz/pipeID (%s/%d)",szCallerName,mPipeID);
        return MFALSE;
    }
    
    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalStream::
enque(
    QParams const& rParams)
{
    //FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    MBOOL ret = MTRUE;

    if(mpHalPipeWrapper)
    {
        ret=mpHalPipeWrapper->enque(mPipeID, mOpenedSensor, mP2PixId, rParams, this->mDequeuedBufList, this->m_UserName);
        if(!ret)
        {
            LOG_ERR("enque Fail");
        }
    }
    else
    {
        LOG_ERR("NULL mpHalPipeWrapper");
        ret = MFALSE;
    }
    
    //FUNCTION_LOG_END;
    return ret;

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalStream::
deque(
    QParams& rParams,
    MINT64 i8TimeoutNs)
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    MBOOL ret = MTRUE;

    if(i8TimeoutNs == -1)   //temp solution for infinite wait
    {
        LOG_INF("no timeout set, infinite wait");
        i8TimeoutNs=30000000000;
    }

    LOG_DBG("pipeID(%d),i8TimeoutNs(%lld)",mPipeID,(long long)i8TimeoutNs);
    if(mpHalPipeWrapper)
    {
        ret=mpHalPipeWrapper->deque(mPipeID, rParams, (MUINTPTR)(&mDequeuedBufList), i8TimeoutNs);
        if(!ret)
        {
            LOG_ERR("pipeID(%d/0x%lx),i8TimeoutNs(%lld) deque Fail",mPipeID, (unsigned long)(&mDequeuedBufList), (long long)i8TimeoutNs);
        }
    }
    else
    {
        LOG_ERR("NULL mpHalPipeWrapper");
        ret = MFALSE;
    }

    FUNCTION_LOG_END;
    return ret;
}

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
    /*
    if(mpPostProcPipe==NULL)
    {
        LOG_ERR("pipeID(%d),NULL mpPostProcPipe",mPipeID);
        return false;
    }
    switch(jpgCmd)
    {
        case EJpgCmd_SetWorkBufSize:    //setJpegParam(EJpgCmd_SetWorkBufSize,0,0)
            mpPostProcPipe->sendCommand(EPIPECmd_SET_JPEG_WORKBUF_SIZE, mJpegWorkBufSize,0,0);
            break;
        case EJpgCmd_SetQualityParam:
            mJpegCfg.soi_en=arg1;
            mJpegCfg.fQuality=arg2;     //setJpegParam(EJpgCmd_SetQualityParam,soi_en,fQuality)
            mpPostProcPipe->sendCommand(EPIPECmd_SET_JPEG_CFG, (MINTPTR)&mJpegCfg,0,0);
            break;
        default:
            break;
    }
    */
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
NormalStream::
setFps(
    MINT32 fps)
{
    Mutex::Autolock lock(mModuleMtx);

    if(mpHalPipeWrapper)
    {
        mpHalPipeWrapper->setFps(fps);
    }
    else
    {
        LOG_ERR("NULL mpHalPipeWrapper");
        return MFALSE;
    }

    return MTRUE;
}


/******************************************************************************
*
******************************************************************************/
MBOOL
NormalStream::
sendCommand(
    ESDCmd cmd,
    MINTPTR arg1,
    MINTPTR arg2,
    MINTPTR arg3)
{
    bool ret=true;
    Mutex::Autolock lock(mModuleMtx);

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
	wd;ht;fps;
	return false;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
NormalStream::
stopVideoRecord()
{
	return false;
}

