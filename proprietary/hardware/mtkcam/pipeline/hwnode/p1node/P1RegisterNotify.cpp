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

#define LOG_TAG "MtkCam/P1NodeImp"
//
#include "P1RegisterNotify.h"
#include "P1TaskCtrl.h"
#include "P1NodeImp.h"


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
// P1NotifyCrop
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
void
P1NotifyCrop::
p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    if (mspP1Notify != NULL) {
        mspP1Notify->doNotifyCrop(pIn, pOut);
    }
    return;
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// P1NotifyQuality
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
void
P1NotifyQuality::
p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    if (mspP1Notify != NULL) {
        mspP1Notify->doNotifyQuality(pIn, pOut);
    }
    return;
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// P1RegisterNotify
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
P1RegisterNotify::
P1RegisterNotify(sp<P1NodeImp> spP1NodeImp)
    : mspP1NodeImp(spP1NodeImp)
    , mOpenId(-1)
    , mLogLevel(0)
    , mLogLevelI(0)
    , mBurstNum(1)
    , mpNotifyCrop(NULL)
    , mpNotifyQuality(NULL)
{
    config();
    //MY_LOGI0("CTR");
};


/******************************************************************************
 *
 ******************************************************************************/
P1RegisterNotify::
~P1RegisterNotify()
{
    //MY_LOGI0("DTR");
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1RegisterNotify::
init()
{
    MY_LOGD3("+++");
    uninit();
    //
    if (mpNotifyCrop == NULL) {
        mpNotifyCrop = new P1NotifyCrop(this);
    }
    //
    if (mpNotifyQuality == NULL) {
        mpNotifyQuality = new P1NotifyQuality(this);
    }
    //
    MY_LOGD3("---");
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1RegisterNotify::
uninit()
{
    MY_LOGD3("+++");
    //
    if (mpNotifyCrop != NULL) {
        delete mpNotifyCrop;
        mpNotifyCrop = NULL;
    }
    //
    if (mpNotifyQuality != NULL) {
        delete mpNotifyQuality;
        mpNotifyQuality = NULL;
    }
    //
    MY_LOGD3("---");
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1RegisterNotify::
config()
{
    if (mspP1NodeImp != NULL) {
        mOpenId = mspP1NodeImp->getOpenId();
        mLogLevel = mspP1NodeImp->mLogLevel;
        mLogLevelI = mspP1NodeImp->mLogLevelI;
        if (mspP1NodeImp->mBurstNum > 1) {
            mBurstNum = mspP1NodeImp->mBurstNum;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1RegisterNotify::
doNotifyCrop(MVOID * pIn, MVOID * pOut)
{
    U_if (mspP1NodeImp == NULL) {
        MY_LOGE("P1NodeImp not exist");
        return;
    }
    U_if (pIn == NULL || pOut == NULL) {
        MY_LOGE("NotifyCrop In/Out NULL - In[%p] Out[%p]", pIn, pOut);
        return;
    }
    MY_LOGI2("NotifyCrop - In[%p] Out[%p]", pIn, pOut);
    BIN_INPUT_INFO * pInInfo = (BIN_INPUT_INFO *)pIn;
    RRZ_REG_CFG * pOutCfg = (RRZ_REG_CFG *)pOut;
    MSize curSize = MSize(pInInfo->TarBinOut_W, pInInfo->TarBinOut_H);
    //
    mspP1NodeImp->setCurrentBinSize(curSize);
    pOutCfg->bRRZ_Bypass = MTRUE;
    //
    { // if (pInInfo != NULL && pOutCfg != NULL) // no need to check since the pIn/pOut has checked
        MBOOL found = MFALSE;
        Mutex::Autolock _l(mspP1NodeImp->mProcessingQueueLock);
        if (mspP1NodeImp->mProcessingQueue.empty()) {
            MY_LOGI0("ProcessingQueue is empty num:%d", (MINT32)pInInfo->Magic);
            return;
        } else {
            P1QueAct qAct;
            std::vector< P1QueJob >::iterator it_job =
                mspP1NodeImp->mProcessingQueue.begin();
            for (; it_job != mspP1NodeImp->mProcessingQueue.end(); it_job++) {
                for (size_t i = 0; i < it_job->size(); i++) {
                    if (it_job->edit(i).getNum() == (MINT32)pInInfo->Magic) {
                        qAct = it_job->edit(i);
                        found = MTRUE;
                        //MY_LOGI3("Found Job(%d)[%zu/%zu]:(%d == %d)", it_job->getIdx(), i, it_job->size(), it_job->edit(i).getNum(), pInInfo->Magic);
                        break;
                    }
                    //MY_LOGI3("NotFound Job(%d)[%zu/%zu]:(%d == %d)", it_job->getIdx(), i, it_job->size(), it_job->edit(i).getNum(), pInInfo->Magic);
                }
            }
            if (found) {
                if (qAct.getNum() != P1ACT_NUM_NULL) {
                    P1Act act = GET_ACT_PTR(act, qAct, RET_VOID);
                    MBOOL isSetChange = MFALSE;
                    MBOOL isSizeChange = (act->refBinSize == curSize) ?
                        MFALSE : MTRUE;
                    //if (IS_BURST_OFF) // exclude burst mode
                    mspP1NodeImp->attemptCtrlResize(qAct, isSetChange);
                    //MY_LOGI3("Act(%d) isSetChange(%d) isSizeChange(%d)", qAct.getNum(), isSetChange, isSizeChange);
                    if (isSetChange || isSizeChange) {
                        MY_LOGI2("Resize Change set(%d) size(%d) cur"
                            P1_SIZE_STR "ref" P1_SIZE_STR "crop" P1_RECT_STR
                            "dst" P1_SIZE_STR "MaxRatio(%d)",
                            isSetChange, isSizeChange, P1_SIZE_VAR(curSize),
                            P1_SIZE_VAR(act->refBinSize),
                            P1_RECT_VAR(act->cropRect_resizer),
                            P1_SIZE_VAR(act->dstSize_resizer),
                            mspP1NodeImp->mResizeRatioMax);
                        if (act->refBinSize.w > curSize.w) {
                            BIN_RESIZE(act->cropRect_resizer.p.x);
                            BIN_RESIZE(act->cropRect_resizer.p.y);
                            BIN_RESIZE(act->cropRect_resizer.s.w);
                            BIN_RESIZE(act->cropRect_resizer.s.h);
                        } else if (act->refBinSize.w < curSize.w) {
                            BIN_REVERT(act->cropRect_resizer.p.x);
                            BIN_REVERT(act->cropRect_resizer.p.y);
                            BIN_REVERT(act->cropRect_resizer.s.w);
                            BIN_REVERT(act->cropRect_resizer.s.h);
                        } // for performance consideration, only check W
                        MY_LOGI3("BIN check crop" P1_RECT_STR "dst" P1_SIZE_STR,
                            P1_RECT_VAR(act->cropRect_resizer),
                            P1_SIZE_VAR(act->dstSize_resizer));
                        //
                        if (act->cropRect_resizer.s.w *
                            mspP1NodeImp->mResizeRatioMax >
                            act->dstSize_resizer.w * 100) {
                            act->dstSize_resizer.w =
                                ((act->cropRect_resizer.s.w *
                                mspP1NodeImp->mResizeRatioMax) + (100 - 1))
                                / 100;
                            act->dstSize_resizer.w =
                                ALIGN_UPPER(act->dstSize_resizer.w, 2);
                        } else if (act->cropRect_resizer.s.w <
                            act->dstSize_resizer.w) {
                            act->dstSize_resizer.w = act->cropRect_resizer.s.w;
                        }
                        //
                        if (act->cropRect_resizer.s.h *
                            mspP1NodeImp->mResizeRatioMax >
                            act->dstSize_resizer.h * 100) {
                            act->dstSize_resizer.h =
                                ((act->cropRect_resizer.s.h *
                                mspP1NodeImp->mResizeRatioMax) + (100 - 1))
                                / 100;
                            act->dstSize_resizer.h =
                                ALIGN_UPPER(act->dstSize_resizer.h, 2);
                        } else if (act->cropRect_resizer.s.h <
                            act->dstSize_resizer.h) {
                            act->dstSize_resizer.h = act->cropRect_resizer.s.h;
                        }
                        MY_LOGI3("LMT check crop" P1_RECT_STR "dst" P1_SIZE_STR,
                            P1_RECT_VAR(act->cropRect_resizer),
                            P1_SIZE_VAR(act->dstSize_resizer));
                        { //if (IS_BURST_OFF) // exclude burst mode
                            pOutCfg->bRRZ_Bypass = MFALSE;
                            pOutCfg->src_x = act->cropRect_resizer.p.x;
                            pOutCfg->src_y = act->cropRect_resizer.p.y;
                            pOutCfg->src_w = act->cropRect_resizer.s.w;
                            pOutCfg->src_h = act->cropRect_resizer.s.h;
                            pOutCfg->tar_w = act->dstSize_resizer.w;
                            pOutCfg->tar_h = act->dstSize_resizer.h;
                            MY_LOGI2("Resize Change set(%d) size(%d) End -"
                                "OutCfg[Bypass:%d src(%d,%d-%dx%d) tar(%dx%d)]",
                                isSetChange, isSizeChange, pOutCfg->bRRZ_Bypass,
                                pOutCfg->src_x, pOutCfg->src_y,
                                pOutCfg->src_w, pOutCfg->src_h,
                                pOutCfg->tar_w, pOutCfg->tar_h);
                        }
                    }
                }
            } else {
                MY_LOGW("Notify Frame Not Found - Drv(%d)", pInInfo->Magic);
                size_t amount = mspP1NodeImp->mProcessingQueue.size();
                MUINT32 i = 0;
                std::vector< P1QueJob >::iterator it =
                    mspP1NodeImp->mProcessingQueue.begin();
                for (; it != mspP1NodeImp->mProcessingQueue.end(); it++, i++) {
                    MY_LOGW("ProcessingQueue[%d/%zu] = job(%d-%d)",
                        i, amount, (*it).getIdx(), (*it).getLastNum());
                }
            }
        }
        P1_ATOMIC_RELEASE;
    }
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1RegisterNotify::
doNotifyQuality(MVOID * pIn, MVOID * pOut)
{
    U_if (mspP1NodeImp == NULL) {
        MY_LOGE("P1NodeImp not exist");
        return;
    }
    MY_LOGI0("NotifyQuality - In[%p] Out[%p]", pIn, pOut);
    //
    mspP1NodeImp->setQualitySwitching(MFALSE);
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID *
P1RegisterNotify::
getNotifyCrop()
{
    U_if (mpNotifyCrop == NULL) {
        MY_LOGE("NotifyCrop not exist");
    }
    return (MVOID *) mpNotifyCrop;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID *
P1RegisterNotify::
getNotifyQuality()
{
    U_if (mpNotifyQuality == NULL) {
        MY_LOGE("NotifyQuality not exist");
    }
    return (MVOID *) mpNotifyQuality;
}


};//namespace NSP1Node
};//namespace v3
};//namespace NSCam


