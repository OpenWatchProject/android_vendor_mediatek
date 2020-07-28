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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_DELIVER_MGR_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_DELIVER_MGR_H_

#define LOG_TAG "MtkCam/P1NodeImp"
//
#include "P1Common.h"
#include "P1Utility.h"
#include "P1TaskCtrl.h"
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::NSP1Node;
using namespace NSCam::Utils::Sync;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NS3Av3;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace NSCam {
namespace v3 {
namespace NSP1Node {


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
class P1NodeImp;


/******************************************************************************
 *
 ******************************************************************************/
class P1DeliverMgr
    : public Thread
{

    //typedef android::List< MINT32 > NumList_T;
    //typedef android::Vector< P1FrameAct > ActQueue_T;
    //typedef std::list< MINT32 > NumList_T;
    typedef std::vector< MINT32 > NumList_T;
    typedef std::vector< P1FrameAct > ActQueue_T;

public:

    P1DeliverMgr();

    virtual ~P1DeliverMgr();

    void init(sp<P1NodeImp> pP1NodeImp);

    void uninit(void);

    void config(void);

    void runningSet(MBOOL bRunning);

    MBOOL runningGet(void);

    void exit(void);

public:

    MBOOL isActListEmpty(void);

    MBOOL registerActList(MINT32 num);

    MBOOL sendActQueue(P1QueAct & rAct, MBOOL needTrigger);

    MBOOL waitFlush(MBOOL needTrigger);

    MBOOL trigger(void);

    MVOID dumpInfo(void);

private:

    void dumpNumList(MBOOL isLock = MFALSE);

    void dumpActQueue(MBOOL isLock = MFALSE);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual status_t    readyToRun();

private:
    virtual bool        threadLoop();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ENUM.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    enum LOOP_STATE
    {
        LOOP_STATE_INIT         = 0,
        LOOP_STATE_WAITING,
        LOOP_STATE_PROCESSING,
        LOOP_STATE_DONE
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    sp<P1NodeImp>       mspP1NodeImp;
    MINT32              mOpenId;
    MINT32              mLogLevel;
    MINT32              mLogLevelI;
    MUINT8              mBurstNum;
    MBOOL               mLoopRunning;
    LOOP_STATE          mLoopState;
    Condition           mDoneCond;
    Condition           mDeliverCond;
    mutable Mutex       mDeliverLock;
    MINT32              mSentNum;
    NumList_T           mNumList;
    ActQueue_T          mActQueue;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Function Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL deliverLoop();


};

};//namespace NSP1Node
};//namespace v3
};//namespace NSCam

#endif //_MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_DELIVER_MGR_H_

