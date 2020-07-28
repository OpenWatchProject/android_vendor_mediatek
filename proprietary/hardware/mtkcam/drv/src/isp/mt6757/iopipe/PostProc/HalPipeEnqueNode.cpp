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

#define LOG_TAG "Iop/p2HPEQNode"
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include "HalPipeWrapper.h"
#include <mtkcam/drv/IHalSensor.h>
#include <IPipe.h>
#include <IPostProcPipe.h>
#include <ispio_pipe_buffer.h>
#include <cutils/properties.h>  // For property_get().
//
/*************************************************************************************
* Log Utility
*************************************************************************************/
// Clear previous define, use our own define.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(p2HP_EQNode);
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (p2HP_EQNode_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (p2HP_EQNode_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (p2HP_EQNode_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (p2HP_EQNode_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (p2HP_EQNode_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (p2HP_EQNode_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;

extern nsecs_t nsTimeoutToWait; // = 3LL*1000LL*1000LL;//wait 3 msecs.

PortTypeMapping mPortTypeMapping[EPortType_Memory+1] =
{
    {EPortType_Sensor,NSImageio::NSIspio::EPortType_Sensor},
    {EPortType_Memory,NSImageio::NSIspio::EPortType_Memory}
};

NSImageio::NSIspio::EDrvScenario const gSwScenDrvScenMapping[ENormalStreamTag_total]=
{
    NSImageio::NSIspio::eDrvScenario_P2A,
    NSImageio::NSIspio::eDrvScenario_VSS,
    NSImageio::NSIspio::eDrvScenario_VFB_FB,
    NSImageio::NSIspio::eDrvScenario_MFB_Blending,
    NSImageio::NSIspio::eDrvScenario_MFB_Mixing,
    NSImageio::NSIspio::eDrvScenario_P2B_Bokeh,
    NSImageio::NSIspio::eDrvScenario_FE,
    NSImageio::NSIspio::eDrvScenario_FM,
    NSImageio::NSIspio::eDrvScenario_Color_Effect
};

PortIdxMappingAlltoP2 mPortIdxMappingAlltoP2[NSImageio::NSIspio::EPortIndex_UNKNOW]=
{
    {NSImageio::NSIspio::EPortIndex_TG1I,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_TG2I,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV_TG1I,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV_TG2I,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_LSCI,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CQI,    EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_IMGO,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_UFEO,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_RRZO,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV_IMGO,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV2_IMGO, EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_LCSO,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_AAO,    EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_AFO,    EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_PDO,    EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_EISO,    EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_FLKO,    EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_RSSO,  EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_PSO,  EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_IMGI,   EPostProcPortIdx_IMGI}, //18
    {NSImageio::NSIspio::EPortIndex_IMGBI,   EPostProcPortIdx_IMGBI},
    {NSImageio::NSIspio::EPortIndex_IMGCI,   EPostProcPortIdx_IMGCI},
    {NSImageio::NSIspio::EPortIndex_VIPI,   EPostProcPortIdx_VIPI},
    {NSImageio::NSIspio::EPortIndex_VIP2I,  EPostProcPortIdx_VIP2I},
    {NSImageio::NSIspio::EPortIndex_VIP3I,  EPostProcPortIdx_VIP3I},
    {NSImageio::NSIspio::EPortIndex_UFDI,   EPostProcPortIdx_UFDI},
    {NSImageio::NSIspio::EPortIndex_LCEI,   EPostProcPortIdx_LCEI},
    {NSImageio::NSIspio::EPortIndex_DMGI,   EPostProcPortIdx_DMGI},
    {NSImageio::NSIspio::EPortIndex_DEPI,   EPostProcPortIdx_DEPI},
    {NSImageio::NSIspio::EPortIndex_TDRI,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_IMG2O,  EPostProcPortIdx_IMG2O},
    {NSImageio::NSIspio::EPortIndex_IMG2BO,  EPostProcPortIdx_IMG2BO},
    {NSImageio::NSIspio::EPortIndex_IMG3O,  EPostProcPortIdx_IMG3O},
    {NSImageio::NSIspio::EPortIndex_IMG3BO, EPostProcPortIdx_IMG3BO},
    {NSImageio::NSIspio::EPortIndex_IMG3CO, EPostProcPortIdx_IMG3CO},
    {NSImageio::NSIspio::EPortIndex_MFBO,   EPostProcPortIdx_MFBO},
    {NSImageio::NSIspio::EPortIndex_FEO,    EPostProcPortIdx_FEO},
    {NSImageio::NSIspio::EPortIndex_WROTO,  EPostProcPortIdx_WROTO},
    {NSImageio::NSIspio::EPortIndex_WDMAO,  EPostProcPortIdx_WDMAO},
    {NSImageio::NSIspio::EPortIndex_JPEGO,  EPostProcPortIdx_JPEGO},
    {NSImageio::NSIspio::EPortIndex_VENC_STREAMO,  EPostProcPortIdx_VENC_STREAMO},
    {NSImageio::NSIspio::EPortIndex_RAWI,  EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV_0_TGI,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV_1_TGI,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV_2_TGI,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV_3_TGI,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV_4_TGI,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV_5_TGI,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_REGI,   EPostProcPortIdx_REGI}
};

#define TRANSFORM_MAX   8  //eTransform in ImageFormat.h
TransformMapping mTransformMapping[TRANSFORM_MAX]=
{
    {0,                 NSImageio::NSIspio::eImgRot_0,  NSImageio::NSIspio::eImgFlip_OFF},
    {eTransform_FLIP_H, NSImageio::NSIspio::eImgRot_0,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_FLIP_V, NSImageio::NSIspio::eImgRot_180,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_ROT_180,NSImageio::NSIspio::eImgRot_180,NSImageio::NSIspio::eImgFlip_OFF},
    {eTransform_ROT_90, NSImageio::NSIspio::eImgRot_90, NSImageio::NSIspio::eImgFlip_OFF},
    {eTransform_FLIP_H|eTransform_ROT_90, NSImageio::NSIspio::eImgRot_270,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_FLIP_V|eTransform_ROT_90, NSImageio::NSIspio::eImgRot_90,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_ROT_270,NSImageio::NSIspio::eImgRot_270,NSImageio::NSIspio::eImgFlip_OFF}
};

extern MINT32 getUs();

/******************************************************************************
 *
 ******************************************************************************/
NSImageio::NSIspio::ERawPxlID
HalPipeWrapper_Thread::
PixelIDMapping(MUINT32 pixIdP2)
{
    NSImageio::NSIspio::ERawPxlID p2hwpixelId = NSImageio::NSIspio::ERawPxlID_B;

    switch (pixIdP2)
    {
        case SENSOR_FORMAT_ORDER_RAW_B:
        {
            p2hwpixelId = NSImageio::NSIspio::ERawPxlID_B;
            break;
        }
        case SENSOR_FORMAT_ORDER_RAW_Gb:
        {
            p2hwpixelId = NSImageio::NSIspio::ERawPxlID_Gb;
            break;
        }
        case SENSOR_FORMAT_ORDER_RAW_Gr:
        {
            p2hwpixelId = NSImageio::NSIspio::ERawPxlID_Gr;
            break;
        }
        case SENSOR_FORMAT_ORDER_RAW_R:
        {
            p2hwpixelId = NSImageio::NSIspio::ERawPxlID_R;
            break;
        }
        default:
        {
            break;
        }
    }
    return p2hwpixelId;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper_Thread::
waitEnqueVSSAgain()
{
	Mutex::Autolock autoLock(mVssEnQCondMtx);
	//the condition timeout is used to avoid busy enque when there is only vss buffer in list
	//vss buffer is queried to do enque again if
	//(1) 3 ms later
	//(2) another enque request 
	mVssEnQCond.waitRelative(mVssEnQCondMtx, nsTimeoutToWait);

	return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper_Thread::
enqueJudgement(
    )
{
    DBG_LOG_CONFIG(iopipe, p2HP_EQNode);
    LOG_INF("+");
    MBOOL ret=MTRUE;
    MBOOL lastoneVss=MFALSE;
    //[1]query buffer
    list<FramePackage>::iterator it;
    this->mpHalPipeWrapper->getLock(ELockEnum_EQFramePackList);
    if(mThreadProperty==EThreadProperty_DIP_1)
    {
        #if 1
        it = this->mpHalPipeWrapper->mLDIPEQFramePackList.begin();
        #else
        for (it = this->mpHalPipeWrapper->mLDIPEQFramePackList.begin(); it != this->mpHalPipeWrapper->mLDIPEQFramePackList.end();it++)
        {
            if((*it).rParams.mvStreamTag[0] == ENormalStreamTag_Vss)
            {   //vss
                this->mpHalPipeWrapper->getLock(ELockEnum_VssOccupied);
                if(this->mpHalPipeWrapper->mbVssOccupied)
                {   //skip to next one
                    this->mpHalPipeWrapper->releaseLock(ELockEnum_VssOccupied);
                    LOG_INF("VSS occupied, skip to next one(%d)", (int)(this->mpHalPipeWrapper->mLDIPEQFramePackList.size()));
                    //last one is vss but previous vss is still occupied
                    if(it == this->mpHalPipeWrapper->mLDIPEQFramePackList.end())
                    {
                        lastoneVss=MTRUE;
                    }
                    else
                    {
                        it++;   //do next buffer, next is vss again?? need check
                        break;
                    }
                }
                else
                {   //get this one to do enque
                    this->mpHalPipeWrapper->mbVssOccupied = MTRUE;
                    this->mpHalPipeWrapper->releaseLock(ELockEnum_VssOccupied);
                    break;
                }
            }
            else
            {
                //wait and add another cmd to next enque
                
                break;
            }
        }
        #endif
    }
    else if(mThreadProperty==EThreadProperty_WarpEG)
    {}
    this->mpHalPipeWrapper->releaseLock(ELockEnum_EQFramePackList);

    if(!lastoneVss)
    {
        //[2]do enqueue
        ret=doEnque(*it);
        if(!ret)
        {
            LOG_ERR("doEnque fail");
        }
        else
        {
            this->mpHalPipeWrapper->getLock(ELockEnum_EQFramePackList);
            this->mpHalPipeWrapper->mLDIPEQFramePackList.erase(it);
            this->mpHalPipeWrapper->releaseLock(ELockEnum_EQFramePackList);
        }
    }
    else
    {
        LOG_INF("add vss again");
        waitEnqueVSSAgain();      
        this->mpHalPipeWrapper->mHalPWrapperEQThread[static_cast<int>(this->mThreadProperty)]->addCmd(ECmd_ENQUE_REQ);
    }
    LOG_DBG("-");
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
HalPipeWrapper_Thread::
doEnque(FramePackage &framePack
    )
{
    LOG_DBG("+");
    MBOOL ret=MTRUE;
    MUINT32 a=0,b=0;
    char filename[1024];
    QParams qParam=framePack.rParams;  

    SensorStaticInfo SensorStaticInfo;          
    MINT32 sensorDEVIdx=0;
    MUINT32 mOpenedSensor=0x0;
    //[1] parse frame number
    MINT32 frameNum=framePack.frameNum;
    //judge frame number exceeds supported burst queue number or not


    MINT32 inStartIdx=0, outStartIdx=0, hwStartIdx=0;;
    MINT32 inNum[frameNum];        //input dma num for each frame unit
    MINT32 outNum[frameNum];   //output dma num for each frame unit
    MINT32 hwNum[frameNum]; //number of specific hw mdoule setting for each frame unit
    int tmpC=0, inCnt=0,outCnt=0;
    int curFramegroup=0;
    int inPorts=0,outPorts=0;
    NSImageio::NSIspio::EDrvScenario drvScen;
    MUINT32 p2CQ=0x0, p2dupCQ=0x0;

    //TODO Temp solution
    for(int q=0;q<framePack.frameNum;q++)
    {
        inPorts += qParam.mvFrameParams[q].mvIn.size();
        outPorts += qParam.mvFrameParams[q].mvOut.size();
    }
    LOG_INF("framePack(%d),total in/out(%d/%d)",frameNum,inPorts,outPorts);

    //[2] do buffer related configuration for each frame unit
    vector<NSImageio::NSIspio::PortInfo const*> vPostProcInPorts;
    vector<NSImageio::NSIspio::PortInfo const*> vPostProcOutPorts;
    std::vector<DipModuleCfg> mvModule;
    NSImageio::NSIspio::PipePackageInfo pipePackageInfo;
    NSImageio::NSIspio::PortInfo ports[300];
    inStartIdx=outStartIdx=hwStartIdx=0;
    MINT32 inEndIdx=0,outEndIdx=0,hwEndIdx=0;
    MUINT32 dmaEnPort=0x0;
    int cnt=0;
    MUINT32 p2PortIdx=0x0;
    int planeNum=0, pixelInByte=0;
    int cropStartIdx=0;
    MUINT32 featureEntag=0x0;
    MINT32 curElementIdx=0;
    EBufferTag bufTag=EBufferTag_Blocking;
    MUINT32 queriedCQ;
    for(int q=0;q<framePack.frameNum;q++)
    {
        LOG_DBG("start to parse information, (%d_%d_%d)",qParam.mvFrameParams[q].mvIn.size(),qParam.mvFrameParams[q].mvOut.size(),qParam.mvFrameParams[q].mvModuleData.size());

        bufTag=EBufferTag_Blocking;

        //(1) reset and update
        vPostProcInPorts.resize(qParam.mvFrameParams[q].mvIn.size());     //in/out dma for each frame unit
        vPostProcOutPorts.resize(qParam.mvFrameParams[q].mvOut.size());
        inCnt=0;
        outCnt=0;
        dmaEnPort=0x0;
        featureEntag=0x0;
        LOG_DBG("frame(%d), in/out/hw(%d_%d/%d_%d/%d_%d)",q,inStartIdx,inEndIdx,outStartIdx,outEndIdx,hwStartIdx,hwEndIdx);
        //(2) input dma and crop setting
        for(int i=0;i< qParam.mvFrameParams[q].mvIn.size() ;i++)
        {
            ports[cnt].u4Stride[0]=ports[cnt].u4Stride[1]=ports[cnt].u4Stride[2]=0;
            planeNum = qParam.mvFrameParams[q].mvIn[i].mBuffer->getPlaneCount();
            p2PortIdx=(mPortIdxMappingAlltoP2[qParam.mvFrameParams[q].mvIn[i].mPortID.index].eP2PortIdx);
            if(p2PortIdx == EPostProcPortIdx_NOSUP)
            {
                LOG_ERR("not support this port: %d",qParam.mvFrameParams[q].mvIn[i].mPortID.index); //using maping string for debug?
                return -1;
            }
            else
            {
                //if user pass-in vipi~vip3i, we would record vipi only
                dmaEnPort |= (1 << p2PortIdx);
            }

            //get crop information
            if(p2PortIdx == EPostProcPortIdx_IMGI)
            {
                ports[cnt].crop1.x      = 0;
                ports[cnt].crop1.floatX = 0;
                ports[cnt].crop1.y      = 0;
                ports[cnt].crop1.floatY = 0;
                ports[cnt].crop1.w      = 0;
                ports[cnt].crop1.h      = 0;
                ports[cnt].resize1.tar_w = 0;
                ports[cnt].resize1.tar_h = 0;
                ports[cnt].crop2.x = 0;
                ports[cnt].crop2.floatX = 0;
                ports[cnt].crop2.y = 0;
                ports[cnt].crop2.floatY = 0;
                ports[cnt].crop2.w      = 0;
                ports[cnt].crop2.h      = 0;
                ports[cnt].resize2.tar_w = 0;
                ports[cnt].resize2.tar_h = 0;
                ports[cnt].crop3.x = 0;
                ports[cnt].crop3.floatX = 0;
                ports[cnt].crop3.y = 0;
                ports[cnt].crop3.floatY = 0;
                ports[cnt].crop3.w      = 0;
                ports[cnt].crop3.h      = 0;
                ports[cnt].resize3.tar_w = 0;
                ports[cnt].resize3.tar_h = 0;
                for(int k=0;k<qParam.mvFrameParams[q].mvCropRsInfo.size();k++)
                {
                    //crop1/crop2 may be different in each buffer node
                        switch(qParam.mvFrameParams[q].mvCropRsInfo[k].mGroupID)
                        {
                            case 1:
                                ports[cnt].crop1.x      = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.x;
                                ports[cnt].crop1.floatX = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.x;
                                ports[cnt].crop1.y      = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.y;
                                ports[cnt].crop1.floatY = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.y;
                                ports[cnt].crop1.w      = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w;
                                ports[cnt].crop1.h      = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h;
                                ports[cnt].resize1.tar_w = (qParam.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w == 0)?(qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w):(qParam.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w);
                                ports[cnt].resize1.tar_h = (qParam.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h == 0)?(qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h):(qParam.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h);
                                break;
                            case 2:
                                ports[cnt].crop2.x = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.x;
                                ports[cnt].crop2.floatX = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.x;
                                ports[cnt].crop2.y = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.y;
                                ports[cnt].crop2.floatY = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.y;
                                ports[cnt].crop2.w      = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w;
                                ports[cnt].crop2.h      = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h;
                                ports[cnt].resize2.tar_w = (qParam.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w == 0)?(qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w):(qParam.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w);
                                ports[cnt].resize2.tar_h = (qParam.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h == 0)?(qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h):(qParam.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h);
                                break;
                            case 3:
                                ports[cnt].crop3.x = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.x;
                                ports[cnt].crop3.floatX = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.x;
                                ports[cnt].crop3.y = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.y;
                                ports[cnt].crop3.floatY = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.y;
                                ports[cnt].crop3.w      = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w;
                                ports[cnt].crop3.h      = qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h;
                                ports[cnt].resize3.tar_w = (qParam.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w == 0)?(qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w):(qParam.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w);
                                ports[cnt].resize3.tar_h = (qParam.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h == 0)?(qParam.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h):(qParam.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h);
                                break;
                            default:
                                LOG_ERR("DO not support crop group (%d)",qParam.mvFrameParams[q].mvCropRsInfo[k].mGroupID);
                            break;
                        }
                }
            }

            //input dma inforamtion
            ports[cnt].u4ImgWidth  = qParam.mvFrameParams[q].mvIn[i].mBuffer->getImgSize().w;
            ports[cnt].u4ImgHeight = qParam.mvFrameParams[q].mvIn[i].mBuffer->getImgSize().h;
            ports[cnt].eImgFmt     = (EImageFormat)(qParam.mvFrameParams[q].mvIn[i].mBuffer->getImgFormat());
            ports[cnt].type        = mPortTypeMapping[qParam.mvFrameParams[q].mvIn[i].mPortID.type].eImgIOPortType;
            ports[cnt].index       = qParam.mvFrameParams[q].mvIn[i].mPortID.index; //need map to index defined in imageio
            ports[cnt].inout       = qParam.mvFrameParams[q].mvIn[i].mPortID.inout;
            ports[cnt].capbility   = qParam.mvFrameParams[q].mvIn[i].mPortID.capbility;
            ports[cnt].eImgRot     = mTransformMapping[qParam.mvFrameParams[q].mvIn[i].mTransform].eImgRot;
            ports[cnt].eImgFlip    = mTransformMapping[qParam.mvFrameParams[q].mvIn[i].mTransform].eImgFlip;
            for(int k=0;k<planeNum;k++)
            {
                ports[cnt].u4Stride[k] = qParam.mvFrameParams[q].mvIn[i].mBuffer->getBufStridesInBytes(k);
                ports[cnt].u4BufSize[k]= qParam.mvFrameParams[q].mvIn[i].mBuffer->getBufSizeInBytes(k);
                ports[cnt].u4BufVA[k]  = qParam.mvFrameParams[q].mvIn[i].mBuffer->getBufVA(k);
                ports[cnt].u4BufPA[k]  = qParam.mvFrameParams[q].mvIn[i].mBuffer->getBufPA(k);
                //ports[cnt].memID[k]    = qParam.mvIn[i].mBuffer->getFD(k);
            }
            //Note, Only support offset of vipi/img3o for nr3d (tpipe calculation, and nr3d + eis needed)
            //getExtOffsetInBytes is in byte, xoffset/yoffset are in pixel for tpipe
            switch(ports[cnt].eImgFmt)
            {
                case eImgFmt_YUY2:
                    pixelInByte=2;
                    break;
                case eImgFmt_YV12:
                default:
                    pixelInByte=1;
                    break;
            }
            ports[cnt].xoffset = (qParam.mvFrameParams[q].mvIn[i].mBuffer->getExtOffsetInBytes(0) % ports[cnt].u4Stride[0])/pixelInByte;//dma x-offset for tile calculation
            ports[cnt].yoffset = qParam.mvFrameParams[q].mvIn[i].mBuffer->getExtOffsetInBytes(0) / ports[cnt].u4Stride[0];//dma y-offset for tile calculation
            LOG_DBG("[mvIn (%d)] fmt(0x%x),w/h(%d/%d),oft(%d/%d/%d),stride(%d/%d/%d),cap(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )",\
                    ports[cnt].index,ports[cnt].eImgFmt,ports[cnt].u4ImgWidth,ports[cnt].u4ImgHeight,qParam.mvFrameParams[q].mvIn[i].mBuffer->getExtOffsetInBytes(0),\
                    ports[cnt].xoffset,ports[cnt].yoffset,ports[cnt].u4Stride[0],ports[cnt].u4Stride[1],ports[cnt].u4Stride[2],\
                    ports[cnt].capbility,ports[cnt].u4BufVA[0],ports[cnt].u4BufVA[1],ports[cnt].u4BufVA[2],ports[cnt].u4BufPA[0],ports[cnt].u4BufPA[1],ports[cnt].u4BufPA[2],\
                    ports[cnt].u4BufSize[0],ports[cnt].u4BufSize[1],ports[cnt].u4BufSize[2]);
            vPostProcInPorts.at(inCnt)=&(ports[cnt]);
            cnt++;
            inCnt++;
        }

        //(3) output dma
        for(int i=0;i< qParam.mvFrameParams[q].mvOut.size() ;i++)
        {
            ports[cnt].u4Stride[0]=ports[cnt].u4Stride[1]=ports[cnt].u4Stride[2]=0;
            planeNum = qParam.mvFrameParams[q].mvOut[i].mBuffer->getPlaneCount();
            //
            p2PortIdx=(mPortIdxMappingAlltoP2[qParam.mvFrameParams[q].mvOut[i].mPortID.index].eP2PortIdx);
            if(p2PortIdx == EPostProcPortIdx_NOSUP)
            {
                LOG_ERR("not support this port: %d",qParam.mvFrameParams[q].mvOut[i].mPortID.index);
                return -1;
            }
            else
            {
                //if user pass-in img3o~img3co, we would record img3o only
                dmaEnPort |= (1 << p2PortIdx);
            }
            if(p2PortIdx==EPostProcPortIdx_VENC_STREAMO)
            {
                this->mpHalPipeWrapper->getLock(ELockEnum_VencPortCnt);
                this->mpHalPipeWrapper->mVencPortCnt++;
                LOG_DBG("e mVencPortCnt(%d)",this->mpHalPipeWrapper->mVencPortCnt);
                this->mpHalPipeWrapper->releaseLock(ELockEnum_VencPortCnt);
            }
            //
            ports[cnt].eImgFmt     = (EImageFormat)(qParam.mvFrameParams[q].mvOut[i].mBuffer->getImgFormat());
            ports[cnt].type        = mPortTypeMapping[qParam.mvFrameParams[q].mvOut[i].mPortID.type].eImgIOPortType;//NSImageio::NSIspio::EPortType_Memory;
            ports[cnt].index       = qParam.mvFrameParams[q].mvOut[i].mPortID.index; //need map to index defined in imageio
            ports[cnt].inout       = qParam.mvFrameParams[q].mvOut[i].mPortID.inout;
            ports[cnt].capbility   = qParam.mvFrameParams[q].mvOut[i].mPortID.capbility;
            ports[cnt].eImgRot     = mTransformMapping[qParam.mvFrameParams[q].mvOut[i].mTransform].eImgRot;
            ports[cnt].eImgFlip    = mTransformMapping[qParam.mvFrameParams[q].mvOut[i].mTransform].eImgFlip;
            ports[cnt].u4ImgWidth  = qParam.mvFrameParams[q].mvOut[i].mBuffer->getImgSize().w;
            ports[cnt].u4ImgHeight = qParam.mvFrameParams[q].mvOut[i].mBuffer->getImgSize().h;
            for(int k=0;k<planeNum;k++)
            {
                ports[cnt].u4Stride[k] = qParam.mvFrameParams[q].mvOut[i].mBuffer->getBufStridesInBytes(k);
                ports[cnt].u4BufSize[k]= qParam.mvFrameParams[q].mvOut[i].mBuffer->getBufSizeInBytes(k);
                ports[cnt].u4BufVA[k]  = qParam.mvFrameParams[q].mvOut[i].mBuffer->getBufVA(k);
                ports[cnt].u4BufPA[k]  = qParam.mvFrameParams[q].mvOut[i].mBuffer->getBufPA(k);
                //ports[cnt].memID[k]    = qParam.mvOut[i].mBuffer->getFD(k);
            }
            //Note, Only support offset of vipi/img3o for nr3d (tpipe calculation, and nr3d + eis needed)
            //getExtOffsetInBytes is in byte, xoffset/yoffset are in pixel for tpipe
            switch(ports[cnt].eImgFmt)
            {
                case eImgFmt_YUY2:
                    pixelInByte=2;
                    break;
                case eImgFmt_YV12:
                default:
                    pixelInByte=1;
                    break;
            }
            ports[cnt].xoffset = (qParam.mvFrameParams[q].mvOut[i].mBuffer->getExtOffsetInBytes(0) % ports[cnt].u4Stride[0])/pixelInByte;//dma x-offset for tile calculation
            ports[cnt].yoffset = qParam.mvFrameParams[q].mvOut[i].mBuffer->getExtOffsetInBytes(0) / ports[cnt].u4Stride[0];//dma y-offset for tile calculation
            LOG_DBG("[mvOut (%d)] fmt(0x%x),w/h(%d/%d),oft(%d/%d/%d),stride(%d/%d/%d),cap(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )",\
                ports[cnt].index,ports[cnt].eImgFmt,ports[cnt].u4ImgWidth,ports[cnt].u4ImgHeight,qParam.mvFrameParams[q].mvOut[i].mBuffer->getExtOffsetInBytes(0),\
                ports[cnt].xoffset,ports[cnt].yoffset,ports[cnt].u4Stride[0],ports[cnt].u4Stride[1],ports[cnt].u4Stride[2],\
                ports[cnt].capbility,ports[cnt].u4BufVA[0],ports[cnt].u4BufVA[1],ports[cnt].u4BufVA[2],ports[cnt].u4BufPA[0],ports[cnt].u4BufPA[1],ports[cnt].u4BufPA[2],\
                ports[cnt].u4BufSize[0],ports[cnt].u4BufSize[1],ports[cnt].u4BufSize[2]);

            vPostProcOutPorts.at(outCnt)=&(ports[cnt]);
            cnt++;
            outCnt++;
        }

        //(4) specific module setting
        for(int i=0;i< qParam.mvFrameParams[q].mvModuleData.size() ;i++)
        {
            featureEntag  |=qParam.mvFrameParams[q].mvModuleData[i].moduleTag;
            LOG_DBG("[mvModule] moduleEnum(%d)",qParam.mvFrameParams[q].mvModuleData[i].moduleTag);
            DipModuleCfg module;
            module.eDipModule=static_cast<EDipModule>(qParam.mvFrameParams[q].mvModuleData[i].moduleTag);
            module.moduleStruct=qParam.mvFrameParams[q].mvModuleData[i].moduleStruct;
            mvModule.push_back(module);
        }

        //(5) get cq information
        drvScen=gSwScenDrvScenMapping[(qParam.mvFrameParams[q].mStreamTag)];
        if(drvScen == NSImageio::NSIspio::eDrvScenario_VSS)
        {
            bufTag=EBufferTag_Vss;
        }
        queriedCQ=this->mpPostProcPipe->queryCQ(drvScen, framePack.callerSensorDev, q, framePack.frameNum);
        p2CQ = (queriedCQ&0xffff0000) >> 16;
        p2dupCQ = (queriedCQ&0x0000ffff);

        

        //(6) do driver configuration and start
        pipePackageInfo.burstQIdx=q;
        pipePackageInfo.dupCqIdx=p2dupCQ;
        pipePackageInfo.p2cqIdx=p2CQ;
        pipePackageInfo.vModuleParams=mvModule;
        pipePackageInfo.pExtraParam=&(framePack.rParams.mvFrameParams[q].mvExtraParam);
        if(framePack.rParams.mvFrameParams[q].mTuningData == NULL)
        {
            LOG_WRN("Warning tuning Data size is NULL!!");
            pipePackageInfo.pTuningQue = NULL;
        }
        else
        {
            pipePackageInfo.pTuningQue = framePack.rParams.mvFrameParams[q].mTuningData;
        }
        
        if(qParam.mvFrameParams[q].mSensorIdx == -1)
        {
            pipePackageInfo.pixIdP2 = this->PixelIDMapping(framePack.pixID);
            LOG_INF("HMyo(0x%x)_fInfo (%d_%d_%d, %d_%d_%d_%d, %d), dEn(0x%08x),ftag (0x%x)", \
                framePack.callerID, p2CQ, p2dupCQ, q, drvScen, framePack.callerSensorDev,  pipePackageInfo.pixIdP2, bufTag, framePack.idx4semEQDone, dmaEnPort,featureEntag);
        }
        else
        {
            {
                sensorDEVIdx = this->mpHalPipeWrapper->mHalSensorList->querySensorDevIdx(qParam.mvFrameParams[q].mSensorIdx);
                mOpenedSensor=static_cast<MUINT32>(sensorDEVIdx);
                this->mpHalPipeWrapper->mHalSensorList->querySensorStaticInfo(mOpenedSensor, &SensorStaticInfo);
                pipePackageInfo.pixIdP2  = this->PixelIDMapping(SensorStaticInfo.sensorFormatOrder);
                LOG_DBG("HMyo qqq (%d), userSensorIdx(0x%x), sensorDEVIdx(0x%x),mOpenedSensor(0x%x),order(0x%x)", q, qParam.mvFrameParams[q].mSensorIdx, sensorDEVIdx,mOpenedSensor,pipePackageInfo.pixIdP2);
                LOG_INF("HMyo(0x%x)_fInfo (%d_%d_%d, %d_%d_%d_%d_%d, %d), dEn(0x%08x),ftag (0x%x)", \
                    framePack.callerID, p2CQ, p2dupCQ, q, drvScen, framePack.callerSensorDev, qParam.mvFrameParams[q].mSensorIdx,  pipePackageInfo.pixIdP2, bufTag, framePack.idx4semEQDone, dmaEnPort,featureEntag);
            }
        }
        pipePackageInfo.drvScen=drvScen;

        if(q == (frameNum-1))
        {
            pipePackageInfo.lastframe=MTRUE;
        }
        else
        {
            pipePackageInfo.lastframe=MFALSE;
        }
        if (this->mpHalPipeWrapper->m_iSaveReqToFile)
        {
            for (MUINT32 i = 0 ; i < qParam.mvFrameParams[q].mvIn.size() ; i++ )
            {
                if (qParam.mvFrameParams[q].mvIn[i].mBuffer->getPlaneCount()>=1)
                {
                    sprintf(filename, "%s/frm(%d)_vInPorts_[%d]_(0x%x)_w(%d)_h(%d)_stride(%d,0x0,0x0)_idx(%d)_dir(%d),Size(0x%08x),VA(0x%lx),PA(0x%lx).%s",DUMP_DIPPREFIX, 
                                        this->mpHalPipeWrapper->m_iEnqueFrmNum,
                                        i,
                                        (NSCam::EImageFormat)(qParam.mvFrameParams[q].mvIn[i].mBuffer->getImgFormat()),
                                        qParam.mvFrameParams[q].mvIn[i].mBuffer->getImgSize().w,
                                        qParam.mvFrameParams[q].mvIn[i].mBuffer->getImgSize().h,
                                        qParam.mvFrameParams[q].mvIn[i].mBuffer->getBufStridesInBytes(0),
                                        qParam.mvFrameParams[q].mvIn[i].mPortID.index,
                                        qParam.mvFrameParams[q].mvIn[i].mPortID.inout,
                                        qParam.mvFrameParams[q].mvIn[i].mBuffer->getBufSizeInBytes(0),
                                        (unsigned long)qParam.mvFrameParams[q].mvIn[i].mBuffer->getBufVA(0),
                                        (unsigned long)qParam.mvFrameParams[q].mvIn[i].mBuffer->getBufPA(0),
                                        "dat"); 
                }
                switch(qParam.mvFrameParams[q].mvIn[i].mPortID.index)
                {
                    case NSImageio::NSIspio::EPortIndex_IMGI:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_IMGI_BUFFER | DIP_DUMP_P2IN_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_IMGBI:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_IMGBI_BUFFER | DIP_DUMP_P2IN_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_IMGCI:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_IMGCI_BUFFER | DIP_DUMP_P2IN_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_VIPI:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_VIPI_BUFFER | DIP_DUMP_P2IN_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_LCEI:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_LCEI_BUFFER | DIP_DUMP_P2IN_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_DEPI:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_DEPI_BUFFER | DIP_DUMP_P2IN_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_DMGI:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_DMGI_BUFFER | DIP_DUMP_P2IN_BUFFER))
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_UFDI:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & DIP_DUMP_P2IN_BUFFER)
                        {
                            qParam.mvFrameParams[q].mvIn[i].mBuffer->saveToFile(filename);
                        }
                        break;
                }
            }
        
            if(framePack.rParams.mvFrameParams[q].mTuningData != NULL)
            {
                if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_TUNING_BUFFER | DIP_DUMP_P2IN_BUFFER))
                {
                    sprintf(filename, "%s/pTuningQue_frm(%d)_size(%d).%s",DUMP_DIPPREFIX, 
                                        this->mpHalPipeWrapper->m_iEnqueFrmNum,
                                        sizeof(dip_x_reg_t),
                                        "dat"); 
                    saveToFile(filename, (unsigned char*)framePack.rParams.mvFrameParams[q].mTuningData, sizeof(dip_x_reg_t));
                }

            }
            this->mpHalPipeWrapper->m_iEnqueFrmNum++;

        }
        ret=this->mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts,&pipePackageInfo);
        if(!ret)
        {
            LOG_ERR("postprocPipe config fail");
            //ASSERT, and callback...
            a=getUs();
            framePack.rParams.mpfnEnQFailCallback(framePack.rParams);
            b=getUs();
            LOG_ERR("=go enque fail callback(%d us)",b-a);
            return ret;
        }

        //(7) collect information for frame package queue
        MdpStartInfo mdpStartInfo;
        this->mpHalPipeWrapper->getLock(ELockEnum_wDQFramePackList);
        if(q==0)
        { //mark this to avoid add_pack0_frame0 -> remove pack -> add_pack0_frame1 -> add_pack1
            framePack.idxofwaitDQFrmPackList=this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size();   //push frame package for waiting deque and get current element index in waitDQList
            curElementIdx=framePack.idxofwaitDQFrmPackList;
            framePack.p2cqIdx=p2CQ;
            framePack.p2cqDupIdx=p2dupCQ;
            framePack.dequedNum = 0;
            this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.push_back(framePack);
        }
        else
        {
            MINT32 tempshift=0;
            if(curElementIdx >= this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size())
            {
                tempshift=curElementIdx-this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size()+1;
            }
            curElementIdx = curElementIdx - tempshift;
        }
        LOG_DBG("q(%d)_curElementIdx (%d)",q, curElementIdx);
        mdpStartInfo.frameNum=framePack.frameNum;
        mdpStartInfo.frameUnit.drvScen=drvScen;
        mdpStartInfo.frameUnit.portEn=dmaEnPort;
        mdpStartInfo.frameUnit.p2cqIdx=p2CQ;
        mdpStartInfo.frameUnit.p2cqDupIdx=p2dupCQ;
        mdpStartInfo.frameUnit.p2BurstQIdx=q;
        mdpStartInfo.frameUnit.callerID=framePack.callerID;
        mdpStartInfo.frameUnit.bufTag=bufTag;
        mdpStartInfo.frameUnit.idxofwaitDQFrmPackList=curElementIdx;

        //(8) notify to do mdp start procedure
        this->mpHalPipeWrapper->getLock(ELockEnum_wDoMdpStartUnitList);
        this->mpHalPipeWrapper->mLDIPwaitDoMDPStartUnitList.push_back(mdpStartInfo);
        this->mpHalPipeWrapper->releaseLock(ELockEnum_wDoMdpStartUnitList);
        this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFramePackList);

        //(8) notify to do mdp start procedure
        //this->mpHalPipeWrapper->getLock(ELockEnum_wDoMdpStartUnitList);
        //this->mpHalPipeWrapper->mLDIPwaitDoMDPStartUnitList.push_back(mdpStartInfo);
        //this->mpHalPipeWrapper->releaseLock(ELockEnum_wDoMdpStartUnitList);
        this->mpHalPipeWrapper->mHalPMdpConfigThread[static_cast<int>(this->mThreadProperty)]->addCmd(ECmd_MDPSTART_REQ);

        //[9] free local variables
        vPostProcInPorts.clear();
        vPostProcOutPorts.clear();
        mvModule.clear();

    }

    //signal for enque done
    ::sem_post(&(this->mpHalPipeWrapper->mSemEQDone[framePack.idx4semEQDone]));

    LOG_INF("-");
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper_Thread::
doMdpStart(
    )
{
    LOG_INF("+");
    MBOOL ret=MTRUE;

    //(1) get item
    list<MdpStartInfo>::iterator it;
    this->mpHalPipeWrapper->getLock(ELockEnum_wDoMdpStartUnitList);
    if(mThreadProperty==EThreadProperty_DIP_1)
    {
        it = this->mpHalPipeWrapper->mLDIPwaitDoMDPStartUnitList.begin();
    }
    else if(mThreadProperty==EThreadProperty_WarpEG)
    {}
    this->mpHalPipeWrapper->releaseLock(ELockEnum_wDoMdpStartUnitList);

    MINT32 shift=0;
    //(2) do mdp start flow
    ret=this->mpPostProcPipe->start();
    if(!ret)
    {
        LOG_ERR("P2 Start Fail!");
        MUINT32 a=0,b=0;
        FramePackage framePack;
        this->mpHalPipeWrapper->getLock(ELockEnum_wDQFramePackList);
        if((*it).frameUnit.idxofwaitDQFrmPackList >= this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size())
        {
            shift=(*it).frameUnit.idxofwaitDQFrmPackList-this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size()+1;
        }
        framePack=this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[((*it).frameUnit.idxofwaitDQFrmPackList - shift)];
        LOG_ERR("fail element index(0x%x_%d_%d_%d, %d_%d_%d_%d)",(*it).frameUnit.callerID, (*it).frameUnit.p2BurstQIdx, (*it).frameUnit.p2cqIdx, (*it).frameUnit.p2cqDupIdx, \
            ((*it).frameUnit.idxofwaitDQFrmPackList - shift), (*it).frameUnit.idxofwaitDQFrmPackList, this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size(), shift);
        this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFramePackList);
        //ASSERT, and callback...
        a=getUs();
        framePack.rParams.mpfnEnQFailCallback(framePack.rParams);
        b=getUs();
        LOG_ERR("=go start fail callback(%d us)",b-a);
        return ret;
    }

    //(3) sw buffer control
    FrameUnit frameUnit;
    this->mpHalPipeWrapper->getLock(ELockEnum_wDQFramePackList);
    frameUnit.drvScen=(*it).frameUnit.drvScen;
    frameUnit.portEn=(*it).frameUnit.portEn;
    frameUnit.p2cqIdx=(*it).frameUnit.p2cqIdx;
    frameUnit.p2cqDupIdx=(*it).frameUnit.p2cqDupIdx;
    frameUnit.p2BurstQIdx=(*it).frameUnit.p2BurstQIdx;
    frameUnit.callerID=(*it).frameUnit.callerID;
    frameUnit.bufTag=(*it).frameUnit.bufTag;
    //update element index if needed
    int tempIdx=(*it).frameUnit.idxofwaitDQFrmPackList;
    //case1, judge package size
    if(tempIdx >= this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size())
    {
        shift=tempIdx-this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size()+1; //avoid timing issue, addBatch -> addNode -> removeBatch
    }
    //case2, judge p2cq/dupcq to avoud racing between config_thread/start_thread
    vector<FramePackage>::iterator it_pack = this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.begin();
    for(int i = 0;i<tempIdx;i++)
    {
        it_pack++;
    }
    for(int minus_count = tempIdx ; minus_count>=0; minus_count--)
    {
        if(((*it).frameUnit.p2cqIdx == (*it_pack).p2cqIdx) && ((*it).frameUnit.p2cqDupIdx == (*it_pack).p2cqDupIdx))
        {
            LOG_DBG("HMyo the same(%d_%d_%d)",minus_count, tempIdx, shift);
            break;
        }
        else
        {
  
            shift+=1;
            it_pack--;
        }
    }
    //error handle
    if((tempIdx-shift) < 0)
    {
        LOG_ERR("ERR shift(%d/%d/%d)",shift, tempIdx, this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size());
        this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFramePackList);
        return MFALSE;
    }
    frameUnit.idxofwaitDQFrmPackList=tempIdx-shift;
    LOG_INF("frameUnit.idxofwaitDQFrmPackList (0x%x_%d_%d_%d, %d_%d_%d_%d)",frameUnit.callerID, frameUnit.p2BurstQIdx, frameUnit.p2cqIdx, frameUnit.p2cqDupIdx,\
        frameUnit.idxofwaitDQFrmPackList, tempIdx, this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size(),shift);
    this->mpHalPipeWrapper->getLock(ELockEnum_wDQFrameUnitList);
    this->mpHalPipeWrapper->mLDIPwaitDQFrameUnitList.push_back(frameUnit);
    //kenrel multi-process......
    ret=this->mpPostProcPipe->bufferQueCtrl(NSImageio::NSIspio::EPIPE_P2BUFQUECmd_ENQUE_FRAME, NSImageio::NSIspio::EPIPE_P2engine_DIP, frameUnit.callerID,\
        frameUnit.p2cqIdx, frameUnit.p2cqDupIdx ,frameUnit.p2BurstQIdx, (*it).frameNum, _no_matter_);
    this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFrameUnitList);
    this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFramePackList);
    if(!ret)
    {
        LOG_ERR("enque ioctrl fail, return");
        return ret;
    }

    //(4) send signal to deque thread that a buffer package is enqued
    if(this->mpHalPipeWrapper->mFps >= HIGH_FPS_THRESHOLD)
    {   //only DIP could be adopted in high high frame rate
        this->mpHalPipeWrapper->mHalPWrapperDQThread[static_cast<int>(EThreadProperty_DIP_1)]->addCmd(ECmd_ENQUE_RDY);
    }
    else
    {   //two thread are used for DIP and Warp engine respectively
        /*
        switch(this->mThreadProperty)
        {
            case EThreadProperty_DIP_1:
                this->mpHalPipeWrapper->mHalPWrapperDQThread[static_cast<int>(EThreadProperty_DIP_1)]->addCmd(ECmd_DEQUE_REQ);
                break;
            case EThreadProperty_WarpEG:
                this->mpHalPipeWrapper->mHalPWrapperDQThread[static_cast<int>(EThreadProperty_WarpEG)]->addCmd(ECmd_DEQUE_REQ);
                break;
            default:
                break;
        }*/
        this->mpHalPipeWrapper->mHalPWrapperDQThread[static_cast<int>(this->mThreadProperty)]->addCmd(ECmd_ENQUE_RDY);
    }

    //(5) pop out
    this->mpHalPipeWrapper->getLock(ELockEnum_wDoMdpStartUnitList);
    this->mpHalPipeWrapper->mLDIPwaitDoMDPStartUnitList.pop_front();
    this->mpHalPipeWrapper->releaseLock(ELockEnum_wDoMdpStartUnitList);

    LOG_DBG("-");
    return ret;
}

