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
//#include <error_test.h>
#define LOG_TAG "Iop/p2HPEQNode"
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include "HalPipeWrapper.h"
#include <mtkcam/drv/IHalSensor.h>
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

//HungWen 2018/11/01
//mark it because of this file don't have any initial function to do DBG_LOG_CONFIG(iopipe, p2HP_EQNode)
//property_get must be used in initial functio to reduce cpu time
//DECLARE_DBG_LOG_VARIABLE(p2HP_EQNode);
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

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper_Thread::
doMdpStart(
    )
{
    LOG_VRB("+");
    MBOOL ret=MTRUE;
    MINT32  p2cqIdx;        //p2 cq user use
    MINT32  p2burstQIdx;
    MINT32  p2RingBufIdx;
    EBufferTag  bufTag;     //blocking or vss
    MUINT32 frameNum;

    //(1) get item
    list<FParam>::iterator it;
    this->mpHalPipeWrapper->getLock(ELockEnum_wDoMdpStartUnitList);
    if(mThreadProperty==EThreadProperty_DIP_1)
    {
        it = this->mpHalPipeWrapper->mLDIPwaitDoMDPStartUnitList.begin();
    }
    else
    {
        LOG_ERR("mThreadPorperty should equal to EThreadProperty_DIP_1 == 0, something wrong");
        this->mpHalPipeWrapper->releaseLock(ELockEnum_wDoMdpStartUnitList);
        return MFALSE;
    }
    this->mpHalPipeWrapper->releaseLock(ELockEnum_wDoMdpStartUnitList);

    //MINT32 shift=0;
    p2cqIdx = (*it).p2cqIdx;
    p2RingBufIdx = (*it).p2RingBufIdx;
    p2burstQIdx = (*it).p2burstQIdx;
    bufTag = (*it).bufTag;
    frameNum = (*it).frameNum;

    LOG_INF("bufTag:%d, p2cqIdx:%d, p2RingBufIdx:%d, p2burstQIdx:%d, frameNum:%d", bufTag, p2cqIdx, p2RingBufIdx, p2burstQIdx, frameNum);

    //(2) do mdp start flow
    ret=this->mpPostProcPipe->start(p2cqIdx, p2RingBufIdx, p2burstQIdx, frameNum);
    if(!ret)
    {
        LOG_ERR("P2 Start Fail!! bufTag:(%d), p2cqIdx:(%d), p2RingBufIdx:(%d), p2burstQIdx:%d, frameNum:%d", bufTag, p2cqIdx, p2RingBufIdx, p2burstQIdx, frameNum);
        return ret;
    }

    //(3) send signal to deque thread that a buffer package is enqued
    if (bufTag != EBufferTag_Vss)
    {
        //this->mpHalPipeWrapper->mHalPWrapperDQThread[static_cast<int>(EThreadProperty_DIP_1)]->addCmd(ECmd_ENQUE_VSSFRAME_RDY);
    //}
    //else
    //{
        this->mpHalPipeWrapper->mHalPWrapperDQThread[static_cast<int>(EThreadProperty_DIP_1)]->addCmd(ECmd_ENQUE_RDY);
    }

    //(5) pop out
    this->mpHalPipeWrapper->getLock(ELockEnum_wDoMdpStartUnitList);
    this->mpHalPipeWrapper->mLDIPwaitDoMDPStartUnitList.pop_front();
    this->mpHalPipeWrapper->releaseLock(ELockEnum_wDoMdpStartUnitList);

    LOG_VRB("-");
    return ret;
}

