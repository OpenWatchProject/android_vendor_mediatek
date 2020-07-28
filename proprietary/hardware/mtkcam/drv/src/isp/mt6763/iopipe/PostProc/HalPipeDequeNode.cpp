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

#define LOG_TAG "Iop/p2HPDQNode"
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include "HalPipeWrapper.h"
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
DECLARE_DBG_LOG_VARIABLE(p2HP_DQNode);
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (p2HP_DQNode_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (p2HP_DQNode_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (p2HP_DQNode_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (p2HP_DQNode_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (p2HP_DQNode_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (p2HP_DQNode_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;

nsecs_t nsTimeoutToWait = 3LL*1000LL*1000LL;//wait 3 msecs.

extern MINT32 getUs();
#if 0
PortIdxMappingP2toAll mPortIdxMappingP2toAll[EPostProcPortIdx_NUM]=
{
    {EPostProcPortIdx_IMGI,     NSImageio::NSIspio::EPortIndex_IMGI},
    {EPostProcPortIdx_IMGBI,     NSImageio::NSIspio::EPortIndex_IMGBI},
    {EPostProcPortIdx_IMGCI,     NSImageio::NSIspio::EPortIndex_IMGCI},
    {EPostProcPortIdx_VIPI,     NSImageio::NSIspio::EPortIndex_VIPI},
    {EPostProcPortIdx_VIP2I,    NSImageio::NSIspio::EPortIndex_VIP2I},
    {EPostProcPortIdx_VIP3I,    NSImageio::NSIspio::EPortIndex_VIP3I},
    {EPostProcPortIdx_UFDI,     NSImageio::NSIspio::EPortIndex_UFDI},
    {EPostProcPortIdx_LCEI,    NSImageio::NSIspio::EPortIndex_LCEI},
    {EPostProcPortIdx_DMGI,    NSImageio::NSIspio::EPortIndex_DMGI},
    {EPostProcPortIdx_DEPI,    NSImageio::NSIspio::EPortIndex_DEPI},
    {EPostProcPortIdx_MFBO,     NSImageio::NSIspio::EPortIndex_MFBO},
    {EPostProcPortIdx_FEO,      NSImageio::NSIspio::EPortIndex_FEO},
    {EPostProcPortIdx_IMG3CO,   NSImageio::NSIspio::EPortIndex_IMG3CO},
    {EPostProcPortIdx_IMG3BO,   NSImageio::NSIspio::EPortIndex_IMG3BO},
    {EPostProcPortIdx_IMG3O,    NSImageio::NSIspio::EPortIndex_IMG3O},
    {EPostProcPortIdx_IMG2BO,    NSImageio::NSIspio::EPortIndex_IMG2BO},
    {EPostProcPortIdx_IMG2O,    NSImageio::NSIspio::EPortIndex_IMG2O},
    {EPostProcPortIdx_JPEGO,    NSImageio::NSIspio::EPortIndex_JPEGO},
    {EPostProcPortIdx_WROTO,    NSImageio::NSIspio::EPortIndex_WROTO},
    {EPostProcPortIdx_WDMAO,    NSImageio::NSIspio::EPortIndex_WDMAO},
    {EPostProcPortIdx_VENC_STREAMO,    NSImageio::NSIspio::EPortIndex_VENC_STREAMO}
};
#endif

#if 1   //for compile err, need module owner reviewed
enum EIspRetStatus
{
    eIspRetStatus_VSS_NotReady  = 1,
    eIspRetStatus_Success       = 0,
    eIspRetStatus_Failed        = -1,
};
#endif


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper_Thread::
waitDequeVSSAgain()
{
	Mutex::Autolock autoLock(mVssDeQCondMtx);
	//the condition timeout is used to aboid busy deque when there is only vss buffer in list
	//vss buffer is enqued to list if
	//(1) 3 ms later
	//(2) there exist an incoming prv buffer(blocking deque), cuz we could deque the buffer first if vss buffer deque is still not done
	mVssDeQCond.waitRelative(mVssDeQCondMtx, nsTimeoutToWait);

	return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper_Thread::
eraseNotify(
	EBufferListTag bufferListTag,
	MINT32 index,
	MINT32 p2CQ,
	MINT32 p2dupCQidx,
	NSImageio::NSIspio::EPIPE_P2BUFQUECmd cmd,
	MUINT32 callerID)
{
    LOG_DBG("+,tag(%d),idx(%d),p2CQ(%d/%d)",bufferListTag,index,p2CQ,p2dupCQidx);
    MBOOL ret=MTRUE;

    if(bufferListTag==EBufferListTag_Package)
    {  //erase frame package from list
        this->mpHalPipeWrapper->getLock(ELockEnum_wDQFramePackList);
        LOG_DBG("Lsize(%lu), index(%d)",(unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size(),index);
        vector<FramePackage>::iterator it = this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.begin();
        switch(index)
        {
            case 0:
                this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.erase(it);
                break;
            case 1:
                it++;
                this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.erase(it);
                break;
            default:
                LOG_ERR("Wrong element Idx!");
                break;
        }

        //update element index in frame package list for each remaining frame unit and frame package
        this->mpHalPipeWrapper->getLock(ELockEnum_wDoMdpStartUnitList);
        LOG_DBG("updateMCList(%lu)",(unsigned long)this->mpHalPipeWrapper->mLDIPwaitDoMDPStartUnitList.size());
        for (list<MdpStartInfo>::iterator _t = this->mpHalPipeWrapper->mLDIPwaitDoMDPStartUnitList.begin(); _t != this->mpHalPipeWrapper->mLDIPwaitDoMDPStartUnitList.end(); _t++)
        {
            if((*_t).frameUnit.idxofwaitDQFrmPackList>0)
            {(*_t).frameUnit.idxofwaitDQFrmPackList -= 1;}
        }
        this->mpHalPipeWrapper->releaseLock(ELockEnum_wDoMdpStartUnitList);
        LOG_DBG("updateFUList(%lu)",(unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFrameUnitList.size());
        for (list<FrameUnit>::iterator t = this->mpHalPipeWrapper->mLDIPwaitDQFrameUnitList.begin(); t != this->mpHalPipeWrapper->mLDIPwaitDQFrameUnitList.end(); t++)
        {
            if((*t).idxofwaitDQFrmPackList>0)
            {(*t).idxofwaitDQFrmPackList -= 1;}
        }
        LOG_DBG("updateFPList(%lu)",(unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size());
        for (vector<FramePackage>::iterator t2 = this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.begin(); t2 != this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.end(); t2++)
        {
            if((*t2).idxofwaitDQFrmPackList>0)
            {(*t2).idxofwaitDQFrmPackList -= 1;}
        }
        this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFramePackList);
        //release user
        ret=this->mpPostProcPipe->updateCQUser(p2CQ);

    }
    else
    {   //erase frame unit from list
        this->mpHalPipeWrapper->getLock(ELockEnum_wDQFrameUnitList);
        list<FrameUnit>::iterator it = this->mpHalPipeWrapper->mLDIPwaitDQFrameUnitList.begin();
        switch(index)
        {
            case 0:
                this->mpHalPipeWrapper->mLDIPwaitDQFrameUnitList.erase(it);
                break;
            case 1:
                it++;
                this->mpHalPipeWrapper->mLDIPwaitDQFrameUnitList.erase(it);
                break;
            default:
                LOG_ERR("Wrong element Idx!");
                break;
        }
        //p2burstQIdx/frameNum/timeout no matter
        ret=this->mpPostProcPipe->bufferQueCtrl(cmd, NSImageio::NSIspio::EPIPE_P2engine_DIP, callerID, p2CQ,p2dupCQidx, _no_matter_ ,_no_matter_ ,_no_matter_);
        this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFrameUnitList);
        //release user
        //ret=this->mpPostProcPipe->updateCQUser(p2CQ);
    }

    LOG_DBG("-");
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper_Thread::
dequeJudgement()
{
    DBG_LOG_CONFIG(iopipe, p2HP_DQNode);
    LOG_DBG("+");
    //(pre) send signal to waiter to check dequeued buffer list
    //      * avoid the situation: dequeue finish signal of bufferA just coming right between user do condition check and go into wait when dequeue
    //      * always send a signal before dequeuing any buffer (if the problem mentioned above is met, do this before dequeuing bufferB could solve the problem)
    //ret=this->mpPostProcPipe->bufferQueCtrl(NSImageio::NSIspio::EPIPE_P2BUFQUECmd_WAKE_WAITFRAME,0,0,0);
    //change place?????
    
    MBOOL ret=MTRUE;
    MINT32 deqRet=eIspRetStatus_Failed;
    FrameUnit frameunit;
    MINT32 sizeofwaitDequeUnit=1;

    //(1) get frame unit
    this->mpHalPipeWrapper->getLock(ELockEnum_wDQFrameUnitList);
    sizeofwaitDequeUnit=(MINT32)(this->mpHalPipeWrapper->mLDIPwaitDQFrameUnitList.size());
    list<FrameUnit>::iterator it = this->mpHalPipeWrapper->mLDIPwaitDQFrameUnitList.begin();
    frameunit = *it;
    this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFrameUnitList);
    int dequeP2Cq=frameunit.p2cqIdx;
    LOG_DBG("dequeCq(%d)/(%d)",dequeP2Cq,frameunit.p2cqIdx);

    //(2) judge the buffer type to adopt deque procedure
    if(frameunit.bufTag == EBufferTag_Vss)
    {
        deqRet=doDeque(dequeP2Cq,frameunit,0);
        switch(deqRet)
        {
            case eIspRetStatus_VSS_NotReady:
                {
                    if(sizeofwaitDequeUnit==1)
                    {
                        LOG_INF("dCq_1 (%d)/(%d)",dequeP2Cq,frameunit.p2cqIdx);
                        waitDequeVSSAgain();
                        this->mpHalPipeWrapper->mHalPWrapperDQThread[static_cast<int>(this->mThreadProperty)]->addCmd(ECmd_ENQUE_RDY);
                    }
                    else
                    {   //do other existed blocking buffer if vss is not ready
                        it++;
                        frameunit=*it;
                        dequeP2Cq=frameunit.p2cqIdx;
                        LOG_DBG("dCq_2 (%d)/(%d)",dequeP2Cq,frameunit.p2cqIdx);
                        if(frameunit.bufTag==EBufferTag_Vss)
                        {
                            LOG_ERR("More than two users use vss simultaneously");
                            return MFALSE;
                        }
                        deqRet=doDeque(dequeP2Cq,frameunit,1);
                        if(deqRet==eIspRetStatus_Failed)
                        {
                            LOG_ERR("Dequeue Buffer Fail,cID(0x%x)!",frameunit.callerID);
                            ret=MFALSE;
                        }
                    }
                }
                break;
            case eIspRetStatus_Failed:
                LOG_ERR("Dequeue Buffer Fail,cID(0x%x)!",frameunit.callerID);
                ret=MFALSE;
                break;
            case eIspRetStatus_Success:
                //this->mpHalPipeWrapper->getLock(ELockEnum_VssOccupied); 
                //this->mpHalPipeWrapper->mbVssOccupied=MFALSE;
                //this->mpHalPipeWrapper->releaseLock(ELockEnum_VssOccupied); 
                break;
            default:
                break;
        }
    }
    else
    {
        //wait for doing deque procedure considering multi-process case
        ret=this->mpPostProcPipe->bufferQueCtrl(NSImageio::NSIspio::EPIPE_P2BUFQUECmd_WAIT_DEQUE, NSImageio::NSIspio::EPIPE_P2engine_DIP, 
            _no_matter_,_no_matter_,_no_matter_,_no_matter_,_no_matter_,_no_matter_);
        //
        if(!ret)
        {
            LOG_ERR("Wait Deque Fail,cID(0x%x)!",frameunit.callerID);
            ret=MFALSE;
        }
        else
        {
            deqRet=doDeque(dequeP2Cq,frameunit,0);
            if(deqRet==eIspRetStatus_Failed)
            {
                LOG_ERR("Dequeue Buffer Fail,cID(0x%x)!",frameunit.callerID);
                ret=MFALSE;
            }
        }
    }
    
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
HalPipeWrapper_Thread::
doDeque(
    MINT32 dequeP2Cq,
    FrameUnit &frameunit,
    MINT32 elementIdx
    )
{
    LOG_DBG("+");
    MBOOL ret=MFALSE;
    MINT32 deQret=eIspRetStatus_Failed;
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
    MBOOL isVencContained=MFALSE;
    MUINT32 a=0,b=0;

    LOG_DBG("HMyo(0x%x)_frameunit.portEn(0x%x)",frameunit.callerID, frameunit.portEn);
    //(1) deque dma
    LOG_DBG("yo(0x%x)_Dequeue path(%d) out buf portID(%d),p2cq(%d),p2cqDupIdx(%d)",frameunit.callerID, frameunit.drvScen, portID.index, frameunit.p2cqIdx, frameunit.p2cqDupIdx);
    deQret=this->mpPostProcPipe->dequeBuf(isVencContained,rQTSBufInfo,frameunit.drvScen,(MINT32)(frameunit.p2cqIdx),(MINT32)(frameunit.p2BurstQIdx),(MINT32)(frameunit.p2cqDupIdx));
    LOG_DBG("deQret(%d)",deQret);
    switch(deQret)
    {
        case eIspRetStatus_Failed:
            LOG_ERR("Dequeue path(%d) out buf portID(%d),p2cq(%d),p2cqDupIdx(%d)",frameunit.drvScen, portID.index, frameunit.p2cqIdx, frameunit.p2cqDupIdx);
            //TODO, AEE???
            return deQret;
            break;
        case eIspRetStatus_VSS_NotReady:
            LOG_DBG("Not ready...path(%d) out buf portID(%d),p2cq(%d),p2cqDupIdx(%d)",frameunit.drvScen, portID.index, frameunit.p2cqIdx, frameunit.p2cqDupIdx);
            return deQret;
        case eIspRetStatus_Success:
        default:                 
            break;
    }

    
#if 0

    //(1) deque dma
    for(int i = EPostProcPortIdx_VENC_STREAMO ; i > EPostProcPortIdx_DEPI ; i--)
    {
        if(frameunit.portEn & (1 << i))
        {
            portID.index=mPortIdxMappingP2toAll[i].ePortIndex;
            LOG_DBG("yo(0x%x)_Dequeue path(%d) out buf portID(%d),p2cq(%d),p2cqDupIdx(%d)",frameunit.callerID, frameunit.drvScen, portID.index, frameunit.p2cqIdx, frameunit.p2cqDupIdx);
            deQret=this->mpPostProcPipe->dequeOutBuf(portID,rQTSBufInfo,frameunit.drvScen,(MINT32)(frameunit.p2cqIdx),(MINT32)(frameunit.p2BurstQIdx),(MINT32)(frameunit.p2cqDupIdx));
            LOG_DBG("(%d), deQret(%d)",i, deQret);
            //slow motion
            switch(i)
            {
                case EPostProcPortIdx_VENC_STREAMO:
                    isVencContained=MTRUE;
                    break;
                default:
                    break;
            }
            switch(deQret)
            {
                case eIspRetStatus_Failed:
                    LOG_ERR("Dequeue path(%d) out buf portID(%d),p2cq(%d),p2cqDupIdx(%d)",frameunit.drvScen, portID.index, frameunit.p2cqIdx, frameunit.p2cqDupIdx);
                    //TODO, AEE???
                    return deQret;
                    break;
                case eIspRetStatus_VSS_NotReady:
                    LOG_DBG("Not ready...path(%d) out buf portID(%d),p2cq(%d),p2cqDupIdx(%d)",frameunit.drvScen, portID.index, frameunit.p2cqIdx, frameunit.p2cqDupIdx);
                    return deQret;
                case eIspRetStatus_Success:
                default:                 
                    break;
            }
        }
    }
    for(int i = EPostProcPortIdx_IMGI ; i > EPostProcPortIdx_NOSUP ; i--)//only deque imgi because mdp design.
    {
        if( frameunit.portEn & (1 << i))
        {
            portID.index=mPortIdxMappingP2toAll[i].ePortIndex;
            LOG_DBG("Dequeue path(%d) in buf portID(%d),p2cq(%d),p2cqDupIdx(%d)",frameunit.drvScen, portID.index, frameunit.p2cqIdx, frameunit.p2cqDupIdx);
            deQret=this->mpPostProcPipe->dequeInBuf(portID,rQTSBufInfo,frameunit.drvScen,(MINT32)(frameunit.p2cqIdx),(MINT32)(frameunit.p2BurstQIdx),(MINT32)(frameunit.p2cqDupIdx));
            switch(deQret)
            {
                case eIspRetStatus_Failed:
                    LOG_ERR("Dequeue path(%d) in buf portID(%d),p2cq(%d),p2cqDupIdx(%d)",frameunit.drvScen, portID.index, frameunit.p2cqIdx, frameunit.p2cqDupIdx);
                    //TODO, AEE???
                    return deQret;
                    break;
                case eIspRetStatus_VSS_NotReady:
                    LOG_DBG("Not ready...path(%d) out buf portID(%d),p2cq(%d),p2cqDupIdx(%d)",frameunit.drvScen, portID.index, frameunit.p2cqIdx, frameunit.p2cqDupIdx);
                    return deQret;
                case eIspRetStatus_Success:
                default:     
                    break;
            }
        }
    }
#endif
    ret=this->mpPostProcPipe->dequeMdpFrameEnd(frameunit.drvScen,(MINT32)(frameunit.p2cqIdx),(MINT32)(frameunit.p2BurstQIdx),(MINT32)(frameunit.p2cqDupIdx));

    //(2) judge deque result
    MBOOL dequeFramePackDone=MFALSE;
    if(ret)
    {
        int index=frameunit.idxofwaitDQFrmPackList;
        int frm=this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].dequedNum;
        //1. judge dequeued buffer number
        this->mpHalPipeWrapper->getLock(ELockEnum_wDQFramePackList);
        //...for debug returned match
        if(this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut.size()>0)
        {
        	LOG_INF("HMyo(0x%x)_FmvOut(0x%lx),dCq(%d/%d), idx(%d)",frameunit.callerID, (unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[0].mBuffer->getBufPA(0),frameunit.p2cqIdx,frameunit.p2cqDupIdx,index);
        }
        else
        {
        	LOG_ERR("no output dma, index(%d)",index);
        }

        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].dequedNum++;
        if(this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].dequedNum == this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].frameNum)
        {
            dequeFramePackDone=MTRUE;
            this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mDequeSuccess = MTRUE;
        }

        //2. buffer control
        if(!dequeFramePackDone)
        {
            this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFramePackList);
            eraseNotify(EBufferListTag_Unit,elementIdx,dequeP2Cq,(MINT32)(frameunit.p2cqDupIdx),NSImageio::NSIspio::EPIPE_P2BUFQUECmd_DEQUE_SUCCESS,frameunit.callerID);
            LOG_INF("+,tag(%d),idx(%d),p2CQ(%d/%d)",\
                         EBufferListTag_Unit,elementIdx,dequeP2Cq,(MINT32)(frameunit.p2cqDupIdx));
        }
        else
        {
            if(this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mpfnCallback == NULL)
            {
                //save to deuquedList
                FramePackage framepack=this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index];
                this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFramePackList);
                this->mpHalPipeWrapper->getLock(ELockEnum_DQFramePackList);
                this->mpHalPipeWrapper->mLDIPDQFramePackList.push_back(framepack);
                LOG_INF("deque!dBLSize(%d), add(0x%x)",(int)(this->mpHalPipeWrapper->mLDIPDQFramePackList.size()), framepack.callerID);
                this->mpHalPipeWrapper->releaseLock(ELockEnum_DQFramePackList);

                //
                eraseNotify(EBufferListTag_Unit,elementIdx,dequeP2Cq,(MINT32)(frameunit.p2cqDupIdx),NSImageio::NSIspio::EPIPE_P2BUFQUECmd_DEQUE_SUCCESS,frameunit.callerID);
                eraseNotify(EBufferListTag_Package,elementIdx,dequeP2Cq,(MINT32)(frameunit.p2cqDupIdx),NSImageio::NSIspio::EPIPE_P2BUFQUECmd_DEQUE_SUCCESS,frameunit.callerID);

                LOG_INF("+,tag(%d),idx(%d),p2CQ(%d/%d),\n\
                             +,tag(%d),idx(%d),p2CQ(%d/%d)",\
                            EBufferListTag_Unit,elementIdx,dequeP2Cq,(MINT32)(frameunit.p2cqDupIdx),\
                            EBufferListTag_Package,elementIdx,dequeP2Cq,(MINT32)(frameunit.p2cqDupIdx));

            }
            else
            {
                //callback
                LOG_DBG("idx(%d)",index);
                FramePackage framepack=this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index];
                this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFramePackList);
                LOG_DBG("=go deque callback (0x%lx)=",(unsigned long)framepack.rParams.mpfnCallback);
                a=getUs();
                framepack.rParams.mpfnCallback(framepack.rParams);
                b=getUs();

                //
                eraseNotify(EBufferListTag_Unit,elementIdx,dequeP2Cq,(MINT32)(frameunit.p2cqDupIdx),NSImageio::NSIspio::EPIPE_P2BUFQUECmd_DEQUE_SUCCESS,frameunit.callerID);
                eraseNotify(EBufferListTag_Package,elementIdx,dequeP2Cq,(MINT32)(frameunit.p2cqDupIdx),NSImageio::NSIspio::EPIPE_P2BUFQUECmd_DEQUE_SUCCESS,frameunit.callerID);

                LOG_INF("+,tag(%d),idx(%d),p2CQ(%d/%d),\n\
                         +,tag(%d),idx(%d),p2CQ(%d/%d),\n\
                         -,cb(%d us)",\
                        EBufferListTag_Unit,elementIdx,dequeP2Cq,(MINT32)(frameunit.p2cqDupIdx),\
                        EBufferListTag_Package,elementIdx,dequeP2Cq,(MINT32)(frameunit.p2cqDupIdx),b-a);
                //clear signal for kernel to notify that frame package is returned
                ret=this->mpPostProcPipe->bufferQueCtrl(NSImageio::NSIspio::EPIPE_P2BUFQUECmd_WAIT_FRAME, NSImageio::NSIspio::EPIPE_P2engine_DIP, framepack.callerID, \
                    _no_matter_, _no_matter_ , _no_matter_, _no_matter_, 10);
                if(!ret)
                {
                    LOG_ERR("clear kernel fail");
                    deQret =  eIspRetStatus_Failed;
                }
                else
                {
                    //if deque success(function return true), the return value should be changed to 0 which means eIspRetStatus_Success(0)
                    deQret =  eIspRetStatus_Success;
                }
            }
        }
    }
    else
    {
        LOG_ERR("path(%d) dequeMdpFrameEnd fail",frameunit.drvScen);
        //AEE???
        return eIspRetStatus_Failed;
    }

    if(isVencContained)
    {
        this->mpHalPipeWrapper->getLock(ELockEnum_VencPortCnt);
        this->mpHalPipeWrapper->mVencPortCnt--;
        LOG_DBG("d mVencPortCnt(%d)",this->mpHalPipeWrapper->mVencPortCnt);
        this->mpHalPipeWrapper->releaseLock(ELockEnum_VencPortCnt);
        //notify
        ::sem_post(&(this->mpHalPipeWrapper->mSemVencPortCnt));
    }

    // For Vss Concurrency Check +++++
    switch(frameunit.drvScen)
    {
    	case NSImageio::NSIspio::eDrvScenario_VSS:
    		this->mpHalPipeWrapper->getLock(ELockEnum_VssOccupied);
    		this->mpHalPipeWrapper->mbVssOccupied = false;
    		this->mpHalPipeWrapper->releaseLock(ELockEnum_VssOccupied);
    		//notify
    		::sem_post(&(this->mpHalPipeWrapper->mSemVssOccupied));
    		break;
    	default:
    		break;
    }
    // For Vss Concurrency Check -----
    
    //LOG_INF("-,cb(%d us)",b-a);
    return deQret;
}

