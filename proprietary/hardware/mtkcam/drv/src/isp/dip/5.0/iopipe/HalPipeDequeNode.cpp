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
#include <mtkcam/utils/TuningUtils/FileReadRule.h>
#include "HalPipeWrapper.h"
#include <IPostProcPipe.h>
#include <ispio_pipe_buffer.h>
#include <cutils/properties.h>  // For property_get().
//
#include <dip_aee.h>

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
using namespace TuningUtils;

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
    int i=0;

    if(bufferListTag==EBufferListTag_Package)
    {  //erase frame package from list
        this->mpHalPipeWrapper->getLock(ELockEnum_wDQFramePackList);
        LOG_DBG("Lsize(%lu), index(%d)",(unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size(),index);
        vector<FramePackage>::iterator it = this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.begin();
#if 1
        if(index >= this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.size())
        {
            LOG_ERR("Wrong element Idx(%d)!",index);
            this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFramePackList);
            return MFALSE;
        }
        i=0;
        while(i<index)
        {
            it++;
            i++;
        }
        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList.erase(it);
#else
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
#endif
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
#if 1
        if(index >= this->mpHalPipeWrapper->mLDIPwaitDQFrameUnitList.size())
        {
            LOG_ERR("Wrong element Idx(%d)!",index);
            this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFrameUnitList);
            return MFALSE;
        }

        i=0;
        while(i<index)
        {
            it++;
            i++;
        }
        this->mpHalPipeWrapper->mLDIPwaitDQFrameUnitList.erase(it);
#else
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
#endif
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
    LOG_DBG("+");
    //(pre) send signal to waiter to check dequeued buffer list
    //      * avoid the situation: dequeue finish signal of bufferA just coming right between user do condition check and go into wait when dequeue
    //      * always send a signal before dequeuing any buffer (if the problem mentioned above is met, do this before dequeuing bufferB could solve the problem)
    //ret=this->mpPostProcPipe->bufferQueCtrl(NSImageio::NSIspio::EPIPE_P2BUFQUECmd_WAKE_WAITFRAME,0,0,0);
    //change place?????
    DBG_LOG_CONFIG(iopipe, p2HP_DQNode);

    MBOOL ret=MTRUE;
    MINT32 deqRet=eIspRetStatus_Failed;
    FrameUnit frameunit;
    MINT32 sizeofwaitDequeUnit=1;
    MINT32 i = 1;
    MBOOL getNotVss=MFALSE;
    MINT32 getNotVssIdx=-1;

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
#if 1
                    //find next unit that is not vss
                    for(i = 1; i < sizeofwaitDequeUnit ;i++)
                    {
                        it++;
                        if((*it).bufTag!=EBufferTag_Vss)
                        {
                            LOG_INF("get buf not VSS, i(%d), sizeofwaitDequeUnit(%d)", i, sizeofwaitDequeUnit);
                            getNotVss=MTRUE;
                            getNotVssIdx = i;
                            break;
                        }
                    }

                    if(!getNotVss)
                    {
                        LOG_DBG("dCq_1 (%d)/(%d)",dequeP2Cq,frameunit.p2cqIdx);
                        waitDequeVSSAgain();
                        this->mpHalPipeWrapper->mHalPWrapperDQThread[static_cast<int>(this->mThreadProperty)]->addCmd(ECmd_ENQUE_RDY);
                    }
                    else
                    {
                        frameunit=*it;
                        dequeP2Cq=frameunit.p2cqIdx;
                        LOG_DBG("dCq_2 (%d)/(%d)",dequeP2Cq,frameunit.p2cqIdx);
                        if(getNotVssIdx > -1)
                        {
                            deqRet=doDeque(dequeP2Cq,frameunit,getNotVssIdx);
                            if(deqRet==eIspRetStatus_Failed)
                            {
                                LOG_ERR("Dequeue Buffer Fail,cID(0x%x)!",frameunit.callerID);
                                ret=MFALSE;
                            }
                        }
                        else
                        {
                            LOG_ERR("getNotVssIdx err... (%d)",getNotVssIdx);
                            ret=MFALSE;
                        }
                    }
#else
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
#endif
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
    //NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
    MBOOL isVencContained=MFALSE;
    MUINTPTR tpipeVa = 0;
    MUINTPTR ispVirRegVa = 0;
    MUINT32 a=0,b=0;
    char filename[1024];
    char dumppath[256];
    char sztmp[256];
    int frm = 0;
    MINT32 frameNum = 0;
    MBOOL lastframe = MTRUE; //last frame in enque package or not

    MUINT32 dumptdriaddr = 0, dumpcmdqaddr = 0, dumpimgiaddr = 0;
    MBOOL bDequeBufIsTheSameAaGCEDump = MFALSE;

    LOG_DBG("HMyo(0x%x)_frameunit.portEn(0x%x)",frameunit.callerID, frameunit.portEn);
    //(1) deque dma
    LOG_DBG("yo(0x%x)_Dequeue path(%d) out buf portID(%d),p2cq(%d),p2cqDupIdx(%d)",frameunit.callerID, frameunit.drvScen, portID.index, frameunit.p2cqIdx, frameunit.p2cqDupIdx);
    LOG_DBG("drvSc(%d), dequeCq(%d), dequeDupCqIdx(%d), RingBufIdx(%d)!!\n", frameunit.drvScen, frameunit.p2cqIdx, frameunit.p2cqDupIdx, frameunit.p2RingBufIdx);
    deQret=this->mpPostProcPipe->dequeBuf(isVencContained,tpipeVa,ispVirRegVa,frameunit.drvScen,(MINT32)(frameunit.p2cqIdx),(MINT32)(frameunit.p2BurstQIdx),(MINT32)(frameunit.p2cqDupIdx),(MINT32)(frameunit.p2RingBufIdx));
    LOG_DBG("deQret(%d)",deQret);
    switch(deQret)
    {
        case eIspRetStatus_Failed:
            {
                this->mpPostProcPipe->getDipDumpInfo(dumptdriaddr, dumpcmdqaddr, dumpimgiaddr, bDequeBufIsTheSameAaGCEDump);
                LOG_ERR("Dequeue path(%d) out buf portID(%d),p2cq(%d),p2cqDupIdx(%d),dumptdriaddr(0x%x),dumpcmdqaddr(0x%x),dumpimgiaddr(0x%x),bDequeBufIsTheSameAaGCEDump(%d)",frameunit.drvScen, portID.index, frameunit.p2cqIdx, frameunit.p2cqDupIdx,dumptdriaddr,dumpcmdqaddr,dumpimgiaddr,bDequeBufIsTheSameAaGCEDump);
                //TODO, AEE???
            #if 1  // Dump bitstream & length table if imgi is ufo format
                this->mpHalPipeWrapper->getLock(ELockEnum_wDQFramePackList);
                int indexF=frameunit.idxofwaitDQFrmPackList;
                int frmF=this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].dequedNum;
                NSCam::EImageFormat ImgFmt_imgi = eImgFmt_UNKNOWN;

                //Save Buffer
                for (MUINT32 i = 0 ; i < this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn.size() ; i++ )
                {
                    if (this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn[i].mBuffer->getPlaneCount()>=1)
                    {
                        if (this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn[i].mPortID.index == NSImageio::NSIspio::EPortIndex_IMGI)
                        {
                            ImgFmt_imgi = (NSCam::EImageFormat)(this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn[i].mBuffer->getImgFormat());
                            if ((ImgFmt_imgi == eImgFmt_UFO_FG_BAYER10) || (ImgFmt_imgi == eImgFmt_UFO_FG_BAYER12) || (ImgFmt_imgi == eImgFmt_UFO_BAYER10) || (ImgFmt_imgi == eImgFmt_UFO_BAYER12))
                            {

                                snprintf (dumppath, 256, "%s/", DEBUG_DIPPREFIX);
                                if(!DrvMakePath(dumppath,0660))
                                {
                                    LOG_ERR("DIP makePath [%s] fail",dumppath);
                                }
                                else
                                {
                                    LOG_DBG("DIP makePath [%s] success",dumppath);
                                }
                                snprintf (filename, 1024, "%s/FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_frm(%d)_vInPorts_[%d]_(0x%x)_w(%d)_h(%d)_stride(%d,0x0,0x0)_idx(%d)_dir(%d)_Size(0x%08x)_VA(0x%lx)_PA(0x%lx).%s",DEBUG_DIPPREFIX,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].FrameNo,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].RequestNo,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].Timestamp,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].UniqueKey,
                                        this->mpHalPipeWrapper->m_iDequeFrmNum,
                                        i,
                                        (NSCam::EImageFormat)(this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn[i].mBuffer->getImgFormat()),
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn[i].mBuffer->getImgSize().w,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn[i].mBuffer->getImgSize().h,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn[i].mBuffer->getBufStridesInBytes(0),
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn[i].mPortID.index,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn[i].mPortID.inout,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn[i].mBuffer->getBufSizeInBytes(0),
                                        (unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn[i].mBuffer->getBufVA(0),
                                        (unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn[i].mBuffer->getBufPA(0),
                                        "dat");

                                this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mvIn[i].mBuffer->saveToFile(filename);
                            }
                        }
                    }
                }
                this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFramePackList);
            #endif


                 snprintf (dumppath, 256, "%s/", DEBUG_DIPPREFIX);
                 if(!DrvMakePath(dumppath,0660))
                 {
                     LOG_ERR("DIP Debug makePath [%s] fail",dumppath);
                 }
                 else
                 {
                     frameNum = this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams.size();
                     if(frmF == (frameNum-1))
                     {
                         lastframe =  MTRUE;
                     }
                     else
                     {
                         lastframe =  MFALSE;
                     }

                     LOG_INF("DIP Debug makePath [%s] success",dumppath);
                     sprintf(filename, "%s/FailTuningBuffer_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_Tag(%d)_pTuningQue(0x%lx)_p2Cq(%d)_p2RingBufIdx(%d)_lastframe(%d)_size(0x%lx).%s",DEBUG_DIPPREFIX,
                                         this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].FrameNo,
                                         this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].RequestNo,
                                         this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].Timestamp,
                                         this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].UniqueKey,
                                         this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mStreamTag,
                                         (unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mTuningData,
                                         dequeP2Cq,
                                         frameunit.p2RingBufIdx,
                                         lastframe,
                                         sizeof(dip_x_reg_t),
                                         "dat");
                     saveToFile(filename, (unsigned char*)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].mTuningData, sizeof(dip_x_reg_t));
                     sprintf(filename, "%s/tpipebuf_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_size(%d).%s",DEBUG_DIPPREFIX,
                                         this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].FrameNo,
                                         this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].RequestNo,
                                         this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].Timestamp,
                                         this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[indexF].rParams.mvFrameParams[frmF].UniqueKey,
                                         TPIPE_BUFFER_SIZE,
                                         "dat");
                     saveToTextFile(filename, (unsigned char*)tpipeVa, TPIPE_BUFFER_SIZE);


                 }


                if (bDequeBufIsTheSameAaGCEDump == MFALSE)
                {
                    //Dump 1st VSS Package Tdr buffer and Tuning Buffer.
                    FrameUnit Vssframeunit;
                    MBOOL bFoundVss = MFALSE;
                    int VssindexF;
                    int VssfrmF;
                    MUINTPTR Vsstdriaddr = 0;

                    //(1) get frame unit
                    this->mpHalPipeWrapper->getLock(ELockEnum_wDQFrameUnitList);
                    list<FrameUnit>::iterator it = this->mpHalPipeWrapper->mLDIPwaitDQFrameUnitList.begin();
                    for (;it !=this->mpHalPipeWrapper->mLDIPwaitDQFrameUnitList.end();it++ )
                    {
                        if((*it).bufTag==EBufferTag_Vss)
                        {
                            Vssframeunit = *it;
                            bFoundVss = MTRUE;
                            break;
                        }
                    }
                    this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFrameUnitList);


                    this->mpHalPipeWrapper->getLock(ELockEnum_wDQFramePackList);

                    if (bFoundVss == MTRUE)
                    {
                        VssindexF = Vssframeunit.idxofwaitDQFrmPackList;
                        VssfrmF = this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].dequedNum;

                        for (MUINT32 i = 0 ; i < this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn.size() ; i++ )
                        {
                            if (this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn[i].mBuffer->getPlaneCount()>=1)
                            {
                                if (this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn[i].mPortID.index == NSImageio::NSIspio::EPortIndex_IMGI)
                                {
                                    ImgFmt_imgi = (NSCam::EImageFormat)(this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn[i].mBuffer->getImgFormat());
                                    if ((ImgFmt_imgi == eImgFmt_UFO_FG_BAYER10) || (ImgFmt_imgi == eImgFmt_UFO_FG_BAYER12) || (ImgFmt_imgi == eImgFmt_UFO_BAYER10) || (ImgFmt_imgi == eImgFmt_UFO_BAYER12))
                                    {

                                        snprintf (dumppath, 256, "%s/", DEBUG_DIPPREFIX);
                                        if(!DrvMakePath(dumppath,0660))
                                        {
                                            LOG_ERR("VSS DIP makePath [%s] fail",dumppath);
                                        }
                                        else
                                        {
                                            LOG_DBG("VSS DIP makePath [%s] success",dumppath);
                                        }
                                        snprintf (filename, 1024, "%s/FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_frm(%d)_vInPorts_[%d]_(0x%x)_w(%d)_h(%d)_stride(%d,0x0,0x0)_idx(%d)_dir(%d)_Size(0x%08x)_VA(0x%lx)_PA(0x%lx).%s",DEBUG_DIPPREFIX,
                                                this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].FrameNo,
                                                this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].RequestNo,
                                                this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].Timestamp,
                                                this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].UniqueKey,
                                                this->mpHalPipeWrapper->m_iDequeFrmNum,
                                                i,
                                                (NSCam::EImageFormat)(this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn[i].mBuffer->getImgFormat()),
                                                this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn[i].mBuffer->getImgSize().w,
                                                this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn[i].mBuffer->getImgSize().h,
                                                this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn[i].mBuffer->getBufStridesInBytes(0),
                                                this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn[i].mPortID.index,
                                                this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn[i].mPortID.inout,
                                                this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn[i].mBuffer->getBufSizeInBytes(0),
                                                (unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn[i].mBuffer->getBufVA(0),
                                                (unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn[i].mBuffer->getBufPA(0),
                                                "dat");

                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mvIn[i].mBuffer->saveToFile(filename);
                                    }
                                }
                            }
                        }

                        this->mpPostProcPipe->getVssDipWBInfo(Vsstdriaddr, Vssframeunit.p2cqIdx, Vssframeunit.p2RingBufIdx);

                        frameNum = this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams.size();
                        if(VssfrmF == (frameNum-1))
                        {
                            lastframe =  MTRUE;
                        }
                        else
                        {
                            lastframe =  MFALSE;
                        }

                        sprintf(filename, "%s/FailTuningBuffer_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_Tag(%d)_pTuningQue(0x%lx)_p2Cq(%d)_p2CqDupIdx(%d)_p2RingBufIdx(%d)_lastframe(%d)_size(0x%lx).%s",DEBUG_DIPPREFIX,
                                    this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].FrameNo,
                                    this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].RequestNo,
                                    this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].Timestamp,
                                    this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].UniqueKey,
                                    this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mStreamTag,
                                    (unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mTuningData,
                                    Vssframeunit.p2cqIdx,
                                    Vssframeunit.p2cqDupIdx,
                                    Vssframeunit.p2RingBufIdx,
                                    lastframe,
                                    sizeof(dip_x_reg_t),
                                    "dat");
                        saveToFile(filename, (unsigned char*)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].mTuningData, sizeof(dip_x_reg_t));
                        if (Vsstdriaddr != 0)
                        {
                            sprintf(filename, "%s/tpipebuf_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_size(%d).%s",DEBUG_DIPPREFIX,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].FrameNo,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].RequestNo,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].Timestamp,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[VssindexF].rParams.mvFrameParams[VssfrmF].UniqueKey,
                                        TPIPE_BUFFER_SIZE,
                                        "dat");
                            saveToTextFile(filename, (unsigned char*)Vsstdriaddr, TPIPE_BUFFER_SIZE);

                        }
                    }

                    this->mpHalPipeWrapper->releaseLock(ELockEnum_wDQFramePackList);
                }
                //AEE_ASSERT("\nCRDISPATCH_KEY:Dip Deque Fail!!\n");
                ABORT();
                return deQret;
            }
            break;
        case eIspRetStatus_VSS_NotReady:
            LOG_DBG("Not ready...path(%d) out buf portID(%d),p2cq(%d),p2cqDupIdx(%d)",frameunit.drvScen, portID.index, frameunit.p2cqIdx, frameunit.p2cqDupIdx);
            return deQret;
        case eIspRetStatus_Success:
        default:
            break;
    }

    ret=this->mpPostProcPipe->dequeMdpFrameEnd(frameunit.drvScen,(MINT32)(frameunit.p2cqIdx),(MINT32)(frameunit.p2BurstQIdx),(MINT32)(frameunit.p2cqDupIdx));

    //(2) judge deque result
    MBOOL dequeFramePackDone=MFALSE;
    if(ret)
    {
        this->mpHalPipeWrapper->getLock(ELockEnum_wDQFramePackList);

        int index=frameunit.idxofwaitDQFrmPackList;
        int frm=this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].dequedNum;
        //1. judge dequeued buffer number
        //...for debug returned match
        if(this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut.size()>0)
        {
        	LOG_DBG("HMyo(0x%x)_FmvOut(0x%lx),dCq(%d/%d), idx(%d)",frameunit.callerID, (unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[0].mBuffer->getBufPA(0),frameunit.p2cqIdx,frameunit.p2cqDupIdx,index);
        }
        else
        {
        	LOG_ERR("no output dma, index(%d)",index);
        }

        //Save Buffer
        if (this->mpHalPipeWrapper->m_iSaveReqToFile)
        {
            for (MUINT32 i = 0 ; i < this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut.size() ; i++ )
            {
                if (this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mBuffer->getPlaneCount()>=1)
                {
                    snprintf (filename, 1024, "%s/FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_frm(%d)_vOuputPorts_[%d]_(0x%x)_w(%d)_h(%d)_stride(%d,0x0,0x0)_idx(%d)_dir(%d)_Size(0x%08x)_VA(0x%lx)_PA(0x%lx).%s",DUMP_DIPPREFIX,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].FrameNo,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].RequestNo,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].Timestamp,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].UniqueKey,
                                        this->mpHalPipeWrapper->m_iDequeFrmNum,
                                        i,
                                        (NSCam::EImageFormat)(this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mBuffer->getImgFormat()),
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mBuffer->getImgSize().w,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mBuffer->getImgSize().h,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mBuffer->getBufStridesInBytes(0),
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mPortID.index,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mPortID.inout,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mBuffer->getBufSizeInBytes(0),
                                        (unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mBuffer->getBufVA(0),
                                        (unsigned long)this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mBuffer->getBufPA(0),
                                        "dat");
                }
                switch(this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mPortID.index)
                {
                    case NSImageio::NSIspio::EPortIndex_WDMAO:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_WDMA_BUFFER))
                        {
                            this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_WROTO:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_WROT_BUFFER))
                        {
                            this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_IMG2O:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_IMG2O_BUFFER))
                        {
                            this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_IMG3O:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_IMG3O_BUFFER))
                        {
                            this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_PAK2O:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_PAK2O_BUFFER))
                        {
                            this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].mvOut[i].mBuffer->saveToFile(filename);
                        }
                        break;
                }
            }

            if(ispVirRegVa != 0)
            {
                if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_TUNING_BUFFER))
                {
                    sprintf(sztmp, "Dip_IspVirBuf_frm_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_frm(%d)_size(%d)",
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].FrameNo,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].RequestNo,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].Timestamp,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].UniqueKey,
                                        this->mpHalPipeWrapper->m_iDequeFrmNum,
                                        DIP_REG_RANGE);

                    FILE_DUMP_NAMING_HINT dumphint;
                    dumphint.UniqueKey = this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].UniqueKey;
                    dumphint.FrameNo = this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].FrameNo;
                    dumphint.RequestNo = this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].RequestNo;
                    genFileName_Reg(filename, 1024, &dumphint, sztmp);

                    NSCam::TuningUtils::FileReadRule tungingfile;
                    tungingfile.DumpP2ForDP(filename, (char*)ispVirRegVa, int DIP_REG_RANGE);
                }
            }

            if(tpipeVa != 0)
            {
                if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_TPIPE_BUFFER))
                {
                    sprintf(filename, "%s/tpipebuf_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_frm(%d)_size(%d).%s",DUMP_DIPPREFIX,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].FrameNo,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].RequestNo,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].Timestamp,
                                        this->mpHalPipeWrapper->mLDIPwaitDQFramePackList[index].rParams.mvFrameParams[frm].UniqueKey,
                                        this->mpHalPipeWrapper->m_iDequeFrmNum,
                                        TPIPE_BUFFER_SIZE,
                                        "dat");
                    saveToTextFile(filename, (unsigned char*)tpipeVa, TPIPE_BUFFER_SIZE);
                }

            }

            this->mpHalPipeWrapper->m_iDequeFrmNum++;

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
#if 0 //remove only support 1 vss at a time
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
#endif

    //LOG_INF("-,cb(%d us)",b-a);
    return deQret;
}

