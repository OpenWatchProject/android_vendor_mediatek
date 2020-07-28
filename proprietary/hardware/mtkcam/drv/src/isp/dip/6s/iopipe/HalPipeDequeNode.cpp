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
#include <mtkcam/utils/std/common.h>
#include <chrono>
#include "dip_aee.h"

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
//HungWen 2018/11/01
//mark it because of this file don't have any initial function to do DBG_LOG_CONFIG(iopipe, p2HP_DQNode)
//property_get must be used in initial functio to reduce cpu time
//DECLARE_DBG_LOG_VARIABLE(p2HP_DQNode);
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;
using namespace TuningUtils;

nsecs_t nsTimeoutToWait = 3LL*1000LL*1000LL;//wait 3 msecs.


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
dequeJudgement(list<FramePackage>& FramePackListRef)
{
    LOG_DBG("+");
    //DBG_LOG_CONFIG(iopipe, p2HP_DQNode);
    CAM_TRACE_BEGIN("dequeJudgement");

    MBOOL ret=MTRUE;
    MINT32 deqRet=eIspRetStatus_Failed;
    MBOOL isVencContained=MFALSE;
    MUINTPTR tpipeVa = 0;
    MUINTPTR ispVirRegVa = 0;
    std::chrono::steady_clock::time_point t1;
    std::chrono::steady_clock::time_point t2;
    char filename[1024];
    char dumppath[256];
    char sztmp[256];
    MINT32 frameNum = 0;
    MBOOL lastframe = MTRUE; //last frame in enque package or not
    //MUINT32 a=0,b=0;
    MBOOL dequeFramePackDone=MFALSE;
    MUINT32 dumptdriaddr = 0, dumpcmdqaddr = 0, dumpimgiaddr = 0;
    MBOOL bDequeBufIsTheSameAaGCEDump = MFALSE;
    int frm = 0;
    list<FramePackage>::iterator it;
    list<FramePackage>::iterator itend;
    list<FParam>::iterator iterFParam;
    int dequeP2Cq;
    int p2RingBufIdx;
    //int idxofwaitDQFrmPackList = 0;

    //(1) get frame unit according to ECmd_ENQUE_RDYf and ECmd_ENQUE_VSSFRAME_RDY
    this->mpHalPipeWrapper->getLock(ELockEnum_EQFramePackList);

    it =  FramePackListRef.begin();
    itend =  FramePackListRef.end();

    this->mpHalPipeWrapper->releaseLock(ELockEnum_EQFramePackList);

    if ((*it).m_FParamVec.size()<=0)
    {
        LOG_ERR("m_FParamVec.size() is zero!! It's impossible!! State Machine is wrong!!");
        return MFALSE;
    }
    frm=(*it).dequedNum;

    //(2) get frame cq number and ring buffer index.
    iterFParam = (*it).m_FParamVec.begin();
    dequeP2Cq=(*iterFParam).p2cqIdx;
    p2RingBufIdx = (*iterFParam).p2RingBufIdx;

    CAM_TRACE_BEGIN("dequeBuf");
    deqRet=this->mpPostProcPipe->dequeBuf(isVencContained,tpipeVa,ispVirRegVa,(MINT32)dequeP2Cq,(MINT32)p2RingBufIdx);
    CAM_TRACE_END();

    switch(deqRet)
    {
        case eIspRetStatus_VSS_NotReady:
            {
                //find next unit that is not vss
                LOG_INF("MDP's callback is out of order and Dip driver is FIFO, so we need to polling mdp again!!");
                waitDequeVSSAgain();
                this->mpHalPipeWrapper->mHalPWrapperDQThread[static_cast<int>(this->mThreadProperty)]->addCmd(ECmd_ENQUE_VSSFRAME_RDY);
            }
            return MTRUE;
        case eIspRetStatus_Failed:
            {
                this->mpPostProcPipe->getDipDumpInfo(dumptdriaddr, dumpcmdqaddr, dumpimgiaddr, bDequeBufIsTheSameAaGCEDump);
                LOG_ERR("Dequeue Buffer Fail, p2cq(%d),p2RingBufIdx(%d),dumptdriaddr(0x%x),dumpcmdqaddr(0x%x),dumpimgiaddr(0x%x),bDequeBufIsTheSameAaGCEDump(%d)", dequeP2Cq, (MINT32)p2RingBufIdx, dumptdriaddr,dumpcmdqaddr,dumpimgiaddr,bDequeBufIsTheSameAaGCEDump);
                //TODO, AEE???
                NSCam::EImageFormat ImgFmt_imgi = eImgFmt_UNKNOWN;
                //Save Buffer
                for (MUINT32 i = 0 ; i < (*it).rParams.mvFrameParams[frm].mvIn.size() ; i++ )
                {
                    if ((*it).rParams.mvFrameParams[frm].mvIn[i].mBuffer->getPlaneCount()>=1)
                    {
                        if ((*it).rParams.mvFrameParams[frm].mvIn[i].mPortID.index == NSImageio::NSIspio::EPortIndex_IMGI)
                        {
                            ImgFmt_imgi = (NSCam::EImageFormat)((*it).rParams.mvFrameParams[frm].mvIn[i].mBuffer->getImgFormat());
                            if ((ImgFmt_imgi == eImgFmt_UFO_FG_BAYER10) || (ImgFmt_imgi == eImgFmt_UFO_FG_BAYER12) || (ImgFmt_imgi == eImgFmt_UFO_BAYER10) || (ImgFmt_imgi == eImgFmt_UFO_BAYER12))
                            {

                                snprintf (dumppath, 256, "%s/", DUMP_DIPPREFIX);
                                if(!DrvMakePath(dumppath,0660))
                                {
                                    LOG_ERR("DIP makePath [%s] fail",dumppath);
                                }
                                else
                                {
                                    LOG_DBG("DIP makePath [%s] success",dumppath);
                                }
                                snprintf (filename, 1024, "%s/FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_frm(%d)_vInPorts_[%d]_(0x%x)_w(%d)_h(%d)_stride(%zu,0x0,0x0)_idx(%d)_dir(%d)_Size(0x%08zx)_VA(0x%lx)_PA(0x%lx).%s",DUMP_DIPPREFIX,
                                        (*it).rParams.mvFrameParams[frm].FrameNo,
                                        (*it).rParams.mvFrameParams[frm].RequestNo,
                                        (*it).rParams.mvFrameParams[frm].Timestamp,
                                        (*it).rParams.mvFrameParams[frm].UniqueKey,
                                        this->mpHalPipeWrapper->m_iDequeFrmNum,
                                        i,
                                        (NSCam::EImageFormat)((*it).rParams.mvFrameParams[frm].mvIn[i].mBuffer->getImgFormat()),
                                        (*it).rParams.mvFrameParams[frm].mvIn[i].mBuffer->getImgSize().w,
                                        (*it).rParams.mvFrameParams[frm].mvIn[i].mBuffer->getImgSize().h,
                                        (*it).rParams.mvFrameParams[frm].mvIn[i].mBuffer->getBufStridesInBytes(0),
                                        (*it).rParams.mvFrameParams[frm].mvIn[i].mPortID.index,
                                        (*it).rParams.mvFrameParams[frm].mvIn[i].mPortID.inout,
                                        (*it).rParams.mvFrameParams[frm].mvIn[i].mBuffer->getBufSizeInBytes(0),
                                        (unsigned long)(*it).rParams.mvFrameParams[frm].mvIn[i].mBuffer->getBufVA(0),
                                        (unsigned long)(*it).rParams.mvFrameParams[frm].mvIn[i].mBuffer->getBufPA(0),
                                        "dat");

                                (*it).rParams.mvFrameParams[frm].mvIn[i].mBuffer->saveToFile(filename);
                            }
                        }
                    }
                }
            }
            snprintf (dumppath, 256, "%s/", DEBUG_DIPPREFIX);
            if(!DrvMakePath(dumppath,0660))
            {
                LOG_ERR("DIP Debug makePath [%s] fail",dumppath);
            }
            else
            {
                LOG_INF("DIP Debug makePath [%s] success",dumppath);
                frameNum = (*it).rParams.mvFrameParams.size();
                if(frm == (frameNum-1))
                {
                    lastframe =  MTRUE;
                }
                else
                {
                    lastframe =  MFALSE;
                }
                this->mpPostProcPipe->dumpFailFrameSetting((*it).rParams.mvFrameParams[frm], dequeP2Cq, p2RingBufIdx, lastframe, (*it).m_UserName);

                sprintf(filename, "%s/FailTuningBuffer_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_Tag(%d)_pTuningQue(%lx)_p2Cq(%d)_p2RingBufIdx(%d)_lastframe(%d)_size(%lx).%s",DEBUG_DIPPREFIX,
                                    (*it).rParams.mvFrameParams[frm].FrameNo,
                                    (*it).rParams.mvFrameParams[frm].RequestNo,
                                    (*it).rParams.mvFrameParams[frm].Timestamp,
                                    (*it).rParams.mvFrameParams[frm].UniqueKey,
                                    (*it).rParams.mvFrameParams[frm].mStreamTag,
                                    (unsigned long)(*it).rParams.mvFrameParams[frm].mTuningData,
                                    dequeP2Cq,
                                    p2RingBufIdx,
                                    lastframe,
                                    sizeof(dip_a_reg_t),
                                    "dat");
                saveToFile(filename, (unsigned char*)(*it).rParams.mvFrameParams[frm].mTuningData, sizeof(dip_a_reg_t));
                sprintf(filename, "%s/tpipebuf_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_size(%d).%s",DEBUG_DIPPREFIX,
                                    (*it).rParams.mvFrameParams[frm].FrameNo,
                                    (*it).rParams.mvFrameParams[frm].RequestNo,
                                    (*it).rParams.mvFrameParams[frm].Timestamp,
                                    (*it).rParams.mvFrameParams[frm].UniqueKey,
                                    TPIPE_BUFFER_SIZE,
                                    "dat");
                saveToTextFile(filename, (unsigned char*)tpipeVa, TPIPE_BUFFER_SIZE);

                sprintf(filename, "%s/IspWKBuf_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_size(%d).%s",DEBUG_DIPPREFIX,
                                    (*it).rParams.mvFrameParams[frm].FrameNo,
                                    (*it).rParams.mvFrameParams[frm].RequestNo,
                                    (*it).rParams.mvFrameParams[frm].Timestamp,
                                    (*it).rParams.mvFrameParams[frm].UniqueKey,
                                    sizeof(dip_a_reg_t),
                                    "dat");
                saveToFile(filename, (unsigned char*)ispVirRegVa, sizeof(dip_a_reg_t));
            }
            if (bDequeBufIsTheSameAaGCEDump == MFALSE)
            {
                if (this->mpHalPipeWrapper->mDIPVSSFramePackList.size() > 0)
                {

                    list<FramePackage>::iterator vssit;
                    list<FramePackage>::iterator vssitend;
                    list<FParam>::iterator vssiterFParam;
                    int vssfrm = 0;
                    int vssdequeP2Cq;
                    int vssp2RingBufIdx;
                    MUINTPTR Vsstdriaddr = 0, Vssispviraddr = 0;

                     //(1) get frame unit according to ECmd_ENQUE_RDYf and ECmd_ENQUE_VSSFRAME_RDY
                    this->mpHalPipeWrapper->getLock(ELockEnum_EQFramePackList);
                    vssit =  this->mpHalPipeWrapper->mDIPVSSFramePackList.begin();
                    vssitend =  this->mpHalPipeWrapper->mDIPVSSFramePackList.end();
                    this->mpHalPipeWrapper->releaseLock(ELockEnum_EQFramePackList);

                    if ((*vssit).m_FParamVec.size()<=0)
                    {
                        LOG_ERR("m_FParamVec.size() of VSS is zero!! It's impossible!! State Machine is wrong!!");
                        return MFALSE;
                    }
                    vssfrm=(*vssit).dequedNum;

                    //(2) get frame cq number and ring buffer index.
                    vssiterFParam = (*vssit).m_FParamVec.begin();
                    vssdequeP2Cq=(*vssiterFParam).p2cqIdx;
                    vssp2RingBufIdx = (*vssiterFParam).p2RingBufIdx;

                    this->mpPostProcPipe->getVssDipWBInfo(Vsstdriaddr, Vssispviraddr, vssdequeP2Cq, vssdequeP2Cq);
                    LOG_ERR("VSS Fail Dump Frame Setting Start!!");
                    this->mpPostProcPipe->dumpFailFrameSetting((*vssit).rParams.mvFrameParams[vssfrm], vssdequeP2Cq, vssp2RingBufIdx, MTRUE, (*vssit).m_UserName);
                    LOG_ERR("VSS Fail Dump Frame Setting End!!");

                    sprintf(filename, "%s/Vss_FailTuningBuffer_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_Tag(%d)_pTuningQue(%lx)_p2Cq(%d)_p2RingBufIdx(%d)_lastframe(%d)_size(%d).%s",DEBUG_DIPPREFIX,
                                    (*vssit).rParams.mvFrameParams[vssfrm].FrameNo,
                                    (*vssit).rParams.mvFrameParams[vssfrm].RequestNo,
                                    (*vssit).rParams.mvFrameParams[vssfrm].Timestamp,
                                    (*vssit).rParams.mvFrameParams[vssfrm].UniqueKey,
                                    (*vssit).rParams.mvFrameParams[vssfrm].mStreamTag,
                                    (unsigned long)((*vssit).rParams.mvFrameParams[vssfrm].mTuningData),
                                    vssdequeP2Cq,
                                    vssp2RingBufIdx,
                                    lastframe,
                                    sizeof(dip_a_reg_t),
                                    "dat");
                    saveToFile(filename, (unsigned char*)(*vssit).rParams.mvFrameParams[vssfrm].mTuningData, sizeof(dip_a_reg_t));

                    if (Vsstdriaddr != 0)
                    {
                        sprintf(filename, "%s/Vss_tpipebuf_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_size(%d).%s",DEBUG_DIPPREFIX,
                                    (*vssit).rParams.mvFrameParams[vssfrm].FrameNo,
                                    (*vssit).rParams.mvFrameParams[vssfrm].RequestNo,
                                    (*vssit).rParams.mvFrameParams[vssfrm].Timestamp,
                                    (*vssit).rParams.mvFrameParams[vssfrm].UniqueKey,
                                    TPIPE_BUFFER_SIZE,
                                    "dat");
                        saveToTextFile(filename, (unsigned char*)Vsstdriaddr, TPIPE_BUFFER_SIZE);
                    }
                    if (Vssispviraddr != 0)
                    {
                        sprintf(filename, "%s/Vss_IspWKBuf_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_size(%lx).%s",DEBUG_DIPPREFIX,
                                    (*vssit).rParams.mvFrameParams[vssfrm].FrameNo,
                                    (*vssit).rParams.mvFrameParams[vssfrm].RequestNo,
                                    (*vssit).rParams.mvFrameParams[vssfrm].Timestamp,
                                    (*vssit).rParams.mvFrameParams[vssfrm].UniqueKey,
                                    sizeof(dip_a_reg_t),
                                    "dat");
                        saveToFile(filename, (unsigned char*)Vssispviraddr, sizeof(dip_a_reg_t));
                    }

                }
            }

            AEE_ASSERT("\nCRDISPATCH_KEY:Dip Deque Fail!!\n");
            //abort();
            return MFALSE;
        case eIspRetStatus_Success:
        default:
            break;
    }


    CAM_TRACE_BEGIN("dequeMdpFrameEnd");
    ret=this->mpPostProcPipe->dequeMdpFrameEnd();
    CAM_TRACE_END();

    //(2) judge deque result
    if(ret)
    {
        //Save Buffer
        if (this->mpHalPipeWrapper->m_iSaveReqToFile)
        {
            for (MUINT32 i = 0 ; i < (*it).rParams.mvFrameParams[frm].mvOut.size() ; i++ )
            {
                if ((*it).rParams.mvFrameParams[frm].mvOut[i].mBuffer->getPlaneCount()>=1)
                {
                    snprintf (filename, 1024, "%s/FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_frm(%d)_vOuputPorts_[%d]_(0x%x)_w(%d)_h(%d)_stride(%zu,0x0,0x0)_idx(%d)_dir(%d)_Size(0x%08zx)_VA(0x%lx)_PA(0x%lx).%s",DUMP_DIPPREFIX,
                                        (*it).rParams.mvFrameParams[frm].FrameNo,
                                        (*it).rParams.mvFrameParams[frm].RequestNo,
                                        (*it).rParams.mvFrameParams[frm].Timestamp,
                                        (*it).rParams.mvFrameParams[frm].UniqueKey,
                                        this->mpHalPipeWrapper->m_iDequeFrmNum,
                                        i,
                                        (NSCam::EImageFormat)((*it).rParams.mvFrameParams[frm].mvOut[i].mBuffer->getImgFormat()),
                                        (*it).rParams.mvFrameParams[frm].mvOut[i].mBuffer->getImgSize().w,
                                        (*it).rParams.mvFrameParams[frm].mvOut[i].mBuffer->getImgSize().h,
                                        (*it).rParams.mvFrameParams[frm].mvOut[i].mBuffer->getBufStridesInBytes(0),
                                        (*it).rParams.mvFrameParams[frm].mvOut[i].mPortID.index,
                                        (*it).rParams.mvFrameParams[frm].mvOut[i].mPortID.inout,
                                        (*it).rParams.mvFrameParams[frm].mvOut[i].mBuffer->getBufSizeInBytes(0),
                                        (unsigned long)(*it).rParams.mvFrameParams[frm].mvOut[i].mBuffer->getBufVA(0),
                                        (unsigned long)(*it).rParams.mvFrameParams[frm].mvOut[i].mBuffer->getBufPA(0),
                                        "dat");
                }
                switch((*it).rParams.mvFrameParams[frm].mvOut[i].mPortID.index)
                {
                    case NSImageio::NSIspio::EPortIndex_WDMAO:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_WDMA_BUFFER))
                        {
                            (*it).rParams.mvFrameParams[frm].mvOut[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_WROTO:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_WROT_BUFFER))
                        {
                            (*it).rParams.mvFrameParams[frm].mvOut[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_IMG2O:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_IMG2O_BUFFER))
                        {
                            (*it).rParams.mvFrameParams[frm].mvOut[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_IMG3O:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_IMG3O_BUFFER))
                        {
                            (*it).rParams.mvFrameParams[frm].mvOut[i].mBuffer->saveToFile(filename);
                        }
                        break;
                    case NSImageio::NSIspio::EPortIndex_PAK2O:
                        if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_PAK2O_BUFFER))
                        {
                            (*it).rParams.mvFrameParams[frm].mvOut[i].mBuffer->saveToFile(filename);
                        }
                        break;
                }
            }

            if(ispVirRegVa != 0)
            {
                if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_TUNING_BUFFER))
                {
                    sprintf(sztmp, "Dip_IspVirBuf_frm_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_frm(%d)_size(%d)",
                                        (*it).rParams.mvFrameParams[frm].FrameNo,
                                        (*it).rParams.mvFrameParams[frm].RequestNo,
                                        (*it).rParams.mvFrameParams[frm].Timestamp,
                                        (*it).rParams.mvFrameParams[frm].UniqueKey,
                                        this->mpHalPipeWrapper->m_iDequeFrmNum,
                                        DIP_REG_RANGE);
#if 0                //temp mark by build pass in ep
                    FILE_DUMP_NAMING_HINT dumphint;
                    dumphint.UniqueKey = (*it).rParams.mvFrameParams[frm].UniqueKey;
                    dumphint.FrameNo = (*it).rParams.mvFrameParams[frm].FrameNo;
                    dumphint.RequestNo = (*it).rParams.mvFrameParams[frm].RequestNo;
                    genFileName_Reg(filename, 1024, &dumphint, sztmp);

                    NSCam::TuningUtils::FileReadRule tungingfile;
                    tungingfile.DumpP2ForDP(filename, (char*)ispVirRegVa, int DIP_REG_RANGE);
#endif
                }
            }

            if(tpipeVa != 0)
            {
                if (this->mpHalPipeWrapper->m_iSaveReqToFile & (DIP_DUMP_TPIPE_BUFFER))
                {
                    sprintf(filename, "%s/tpipebuf_FN(%d)_RN(%d)_TS(0x%x)_UK(0x%x)_frm(%d)_size(%d).%s",DUMP_DIPPREFIX,
                                        (*it).rParams.mvFrameParams[frm].FrameNo,
                                        (*it).rParams.mvFrameParams[frm].RequestNo,
                                        (*it).rParams.mvFrameParams[frm].Timestamp,
                                        (*it).rParams.mvFrameParams[frm].UniqueKey,
                                        this->mpHalPipeWrapper->m_iDequeFrmNum,
                                        TPIPE_BUFFER_SIZE,
                                        "dat");
                    saveToTextFile(filename, (unsigned char*)tpipeVa, TPIPE_BUFFER_SIZE);
                }

            }

            this->mpHalPipeWrapper->m_iDequeFrmNum++;

        }
        LOG_DBG("Before check per frame callback");
        if((*it).rParams.mvFrameParams[frm].mpfnCallback != NULL){
                LOG_DBG("add per frame callback");
                t1 = std::chrono::steady_clock::now();
                (*it).rParams.mvFrameParams[frm].mpfnCallback((*it).rParams.mvFrameParams[frm],EFRAMECALLBACK_NOERR);
                t2 = std::chrono::steady_clock::now();
        }
        LOG_DBG("Done check per frame callback, t2 - t1 = %lld", std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
        (*it).dequedNum++;
        if((*it).dequedNum == (*it).frameNum)
        {
            dequeFramePackDone=MTRUE;
            (*it).rParams.mDequeSuccess = MTRUE;
        }
        //2. buffer control
        if(!dequeFramePackDone)
        {
            this->mpHalPipeWrapper->getLock(ELockEnum_EQFramePackList);
            if ((*it).m_FParamVec.size()>0)
            {
                (*it).m_FParamVec.pop_front();
            }
            else
            {
                LOG_ERR("Deque Frame State-Not Package Done machine Error!!! p2CQ(%d/%d)", dequeP2Cq,(MINT32)p2RingBufIdx);
            }
            this->mpHalPipeWrapper->releaseLock(ELockEnum_EQFramePackList);

            LOG_INF("+ p2CQ(%d/%d/%d/%d)",dequeP2Cq,(MINT32)p2RingBufIdx,(*it).dequedNum,(*it).frameNum);
        }
        else
        {
            if((*it).rParams.mpfnCallback == NULL)
            {
                LOG_INF("cb is null!!, p2CQ(%d/%d/%d/%d)", dequeP2Cq,(MINT32)p2RingBufIdx,(*it).dequedNum,(*it).frameNum);
                this->mpHalPipeWrapper->getLock(ELockEnum_EQFramePackList);
                if ((*it).m_FParamVec.size()>0)
                {
                    (*it).m_FParamVec.pop_front();
                    if ((*it).m_FParamVec.size()==0)
                    {
                        //this->mpHalPipeWrapper->mLDIPEQFramePackList.pop_front();
                        FramePackListRef.pop_front();
                        //FramePackListRef.erase(it);
                    }
                    else
                    {
                        LOG_ERR("Deque Package Machine Error!!! p2CQ(%d/%d)", dequeP2Cq,(MINT32)p2RingBufIdx);
                    }
                }
                else
                {
                    LOG_ERR("Deque Package In Deque Frame State Machine Error!!! p2CQ(%d/%d)", dequeP2Cq,(MINT32)p2RingBufIdx);
                }
                this->mpHalPipeWrapper->releaseLock(ELockEnum_EQFramePackList);
            }
            else
            {
                CAM_TRACE_BEGIN("p2callbackfn");
                //a=getUs();
                t1 = std::chrono::steady_clock::now();
                (*it).rParams.mpfnCallback((*it).rParams);
                t2 = std::chrono::steady_clock::now();
                //b=getUs();
                CAM_TRACE_END();
                LOG_INF("p2 callback!! p2CQ(%d/%d/%d/%d),cb(%lld us)",\
                         dequeP2Cq,(MINT32)p2RingBufIdx,(*it).dequedNum,(*it).frameNum,std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
                this->mpHalPipeWrapper->getLock(ELockEnum_EQFramePackList);
                if ((*it).m_FParamVec.size()>0)
                {
                    (*it).m_FParamVec.pop_front();
                    if ((*it).m_FParamVec.size()==0)
                    {
                        //this->mpHalPipeWrapper->mLDIPEQFramePackList.pop_front();
                        FramePackListRef.pop_front();
                        //FramePackListRef.erase(it);
                    }
                    else
                    {
                        LOG_ERR("Deque Package machine Error!!! p2CQ(%d/%d)", dequeP2Cq,(MINT32)p2RingBufIdx);
                    }

                }
                else
                {
                    LOG_ERR("Deque Frame State-Package Done machine Error!!! p2CQ(%d/%d)", dequeP2Cq,(MINT32)p2RingBufIdx);
                }
                this->mpHalPipeWrapper->releaseLock(ELockEnum_EQFramePackList);

            }
        }
    }
    else
    {
        LOG_ERR("dequeMdpFrameEnd fail, p2cq(%d),p2cqDupIdx(%d),dequedNum(%d),frameNum(%d)",dequeP2Cq, (MINT32)p2RingBufIdx,(*it).dequedNum,(*it).frameNum);
        //AEE???
        return ret;
    }
    CAM_TRACE_END();

    return ret;

}
