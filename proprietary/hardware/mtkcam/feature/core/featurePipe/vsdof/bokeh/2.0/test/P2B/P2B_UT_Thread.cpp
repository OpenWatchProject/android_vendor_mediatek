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
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__
//
#define LOG_VRB(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define LOG_INF(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[ERROR][%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[%s] " fmt, __func__, ##arg); } while(0)
//
#define LOG_TAG "P2B_UT_THREAD"
//
#include <utils/RefBase.h>
#include <mtkcam/feature/effectHalBase/IEffectHal.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
#include "P2B_UT_Thread.h"
#include <DebugUtil.h>
#include <string>
//
#include <sys/resource.h>
//
using namespace android;
using namespace VSDOF::Bokeh::UT;
//************************************************************************
// utility function
//************************************************************************
#define NS_PER_SEC  1000000000
#define NS_PER_MS   1000000
#define NS_PER_US   1000
static void Wait(int ms)
{
  long waitSec;
  long waitNSec;
  waitSec = (ms * NS_PER_MS) / NS_PER_SEC;
  waitNSec = (ms * NS_PER_MS) % NS_PER_SEC;
  struct timespec t;
  t.tv_sec = waitSec;
  t.tv_nsec = waitNSec;
  if( nanosleep(&t, NULL) != 0 )
  {
  }
}
/*******************************************************************************
 *
 ********************************************************************************/
P2B_UT_Thread::
P2B_UT_Thread(MINT32 scenario):
    mP2BNode("P2B_UT", nullptr, 0xFFFF),
    mDebugNode("Debug", nullptr, 0xFFFF, scenario)
{
    LOG_DBG("CKH: ctor(0x%x)", this);
    //
    mP2BNode.connectData(P2B_MDP_REQUEST, P2B_MDP_REQUEST, &mDebugNode);
    mP2BNode.connectData(P2B_MDP_WDMA, P2B_MDP_WDMA, &mDebugNode);
    mP2BNode.connectData(P2B_OUT, P2B_OUT, &mDebugNode);
}
/*******************************************************************************
 *
 ********************************************************************************/
P2B_UT_Thread::
~P2B_UT_Thread()
{
    LOG_DBG("CKH: dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2B_UT_Thread::
init(P2B_UT_Params params)
{
    LOG_DBG("+");
    //
    mP2BNode.init();
    LOG_DBG("1");
    mDebugNode.init();
    LOG_DBG("2");
    //
    mP2BNode.start();
    LOG_DBG("3");
    mDebugNode.start();
    LOG_DBG("4");
    //
    mP2B_UT = new P2B_UT();
    LOG_DBG("5");
    mP2B_UT->init(params);
    LOG_DBG("-");
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2B_UT_Thread::
uninit()
{
    LOG_DBG("CKH: + isRunning(%d)", isRunning());
    mbIsExit = MTRUE;
    if(isRunning())
    {
        requestExit();
        mRequestCond.signal();
        join();
    }
    mDebugNode.stop();
    mP2BNode.stop();
    //
    mDebugNode.uninit();
    mP2BNode.uninit();
    //
    mP2BNode.disconnect();
    //
    LOG_DBG("CKH: -");
    //
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
bool
P2B_UT_Thread::
threadLoop()
{
    LOG_DBG("+");
    Mutex::Autolock _l(mRequestLock);
    //
    //SmartImageBuffer smDMGImgBuf = nullptr;
    //SmartImageBuffer smDMBGImgBuf = nullptr;
    //
    sp<EffectRequest> request = nullptr;
    //
    if(mP2B_UT->generate(request/*, smDMGImgBuf, smDMBGImgBuf*/))
    {
        if(!mbIsExit)
        {
            LOG_DBG("CKH: onData");
            request->mpOnRequestProcessed = effectHalCb;
            request->mpTag = this;
            mP2BNode.onData(ID_ROOT_ENQUE, request);
            //mP2BNode.onData(GF_BOKEH_DMG, smDMGImgBuf);
            //mP2BNode.onData(GF_BOKEH_DMBG, smDMBGImgBuf);
            // dump image buffer
            /*makePath("/sdcard/vsdof/bokeh_ut/result/", 0660);
            string saveFileName = string("/sdcard/vsdof/bokeh_ut/result/dmg_")+
                               to_string(request->getRequestNo())+
                               string(".yuv");
            smDMGImgBuf->mImageBuffer->saveToFile(saveFileName.c_str());
            saveFileName = string("/sdcard/vsdof/bokeh_ut/result/dmbg_")+
                               to_string(request->getRequestNo())+
                               string(".yuv");
            smDMBGImgBuf->mImageBuffer->saveToFile(saveFileName.c_str());
            sp<EffectFrameInfo> frame = request->vInputFrameInfo.valueFor(BOKEH_ER_BUF_MAIN1);
            if(frame.get())
            {
                sp<IImageBuffer> buf = nullptr;
                frame->getFrameBuffer(buf);
                saveFileName = string("/sdcard/vsdof/bokeh_ut/result/main_")+
                               to_string(request->getRequestNo())+
                               string(".yuv");
                buf->saveToFile(saveFileName.c_str());
            }*/
        }
    }
    else
    {
        LOG_DBG("wait request");
        mRequestCond.wait(mRequestLock);
    }
    Wait(66);
    LOG_DBG("-");
    return true;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
P2B_UT_Thread::
effectHalCb(
    MVOID* tag,
    String8 status,
    sp<NSCam::EffectRequest>& request)
{
    const sp<EffectParameter> params = request->getRequestParameter();
    MINT32 scenario = params->getInt(VSDOF_FRAME_SCENARIO);
    MINT32 nr3dFlag = params->getInt(VSDOF_FRAME_3DNR_FLAG);
    MINT32 gmvx = params->getInt(VSDOF_FRAME_GMV_X);
    MINT32 gmvy = params->getInt(VSDOF_FRAME_GMV_Y);
    MINT32 afFlag = params->getInt(VSDOF_FRAME_AF_TRIGGER_FLAG);
    MINT32 afx = params->getInt(VSDOF_FRAME_AF_POINT_X);
    MINT32 afy = params->getInt(VSDOF_FRAME_AF_POINT_Y);
    MINT32 bokehLevel = params->getInt(VSDOF_FRAME_BOKEH_LEVEL);
    MINT32 gsensor = params->getInt(VSDOF_FRAME_G_SENSOR_ORIENTATION);
    //
    LOG_DBG("requestId(%d), scenario(%d), nr3dFlag(%d), GMV(%d:%d), afFlag(%d), af(%d:%d), bokehLevel(%d), gsensor(%d)",
                request->getRequestNo(),
                scenario,
                nr3dFlag,
                gmvx,
                gmvy,
                afFlag,
                afx,
                afy,
                bokehLevel,
                gsensor);
    // dump image buffer
    makePath("/sdcard/vsdof/bokeh_ut/result/p2b", 0660);
    sp<IImageBuffer> buffer = nullptr;
    string saveFileName = "";
    sp<EffectFrameInfo> frame = nullptr;
    for(MINT32 i=0;i<request->vOutputFrameInfo.size();++i)
    {
        frame = request->vOutputFrameInfo.valueAt(i);
        if(frame->getFrameNo() == BOKEH_ER_BUF_DISPLAY)
        {
            frame->getFrameBuffer(buffer);
            saveFileName = string("/sdcard/vsdof/bokeh_ut/result/p2b/WDMA_")+
                            to_string(request->getRequestNo())+
                            string(".yuv");
            buffer->saveToFile(saveFileName.c_str());
        }
        else if(frame->getFrameNo() == BOKEH_ER_BUF_RECORD)
        {
            frame->getFrameBuffer(buffer);
            saveFileName = string("/sdcard/vsdof/bokeh_ut/result/p2b/WROT_")+
                            to_string(request->getRequestNo())+
                            string(".yuv");
            buffer->saveToFile(saveFileName.c_str());
        }
    }
}