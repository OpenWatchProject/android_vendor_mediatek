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
#define LOG_TAG "Vendor_UT_THREAD"
//
#include <utils/RefBase.h>
#include <mtkcam/feature/effectHalBase/IEffectHal.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
#include "Vendor_UT_Thread.h"
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
Vendor_UT_Thread::
Vendor_UT_Thread(MINT32 scenario):
    mVendorNode("Vendor_UT", nullptr, 0xFFFF),
    mDebugNode("Debug", nullptr, 0xFFFF, scenario)
{
    MY_LOGD("Vendor_UT_Thread: ctor(0x%x)", this);
    //
    mVendorNode.connectData(P2B_MDP_REQUEST, P2B_MDP_REQUEST, &mDebugNode, CONNECTION_DIRECT);
    mVendorNode.connectData(P2B_MDP_WDMA, P2B_MDP_WDMA, &mDebugNode, CONNECTION_DIRECT);
    mVendorNode.connectData(VENDOR_OUT, VENDOR_OUT, &mDebugNode, CONNECTION_DIRECT);
}
/*******************************************************************************
 *
 ********************************************************************************/
Vendor_UT_Thread::
~Vendor_UT_Thread()
{
    MY_LOGD("~Vendor_UT_Thread: dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Vendor_UT_Thread::
init(Vendor_UT_Params params)
{
    MY_LOGD("+");
    //asd
    mVendorNode.init();
    mDebugNode.init();
    //
    mVendorNode.start();
    mDebugNode.start();
    //
    MY_LOGD("Eric:mVendor_UT init start");
    mVendor_UT = new Vendor_UT();
    mVendor_UT->init(params);

    MY_LOGD("Eric:mVendor_UT init end");
    //
    MY_LOGD("-");
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Vendor_UT_Thread::
uninit()
{
    MY_LOGD("Vendor_UT_Thread isRunning(%d)", isRunning());
    mbIsExit = MTRUE;
    if(isRunning())
    {
        requestExit();
        mRequestCond.signal();
        join();
    }
    mDebugNode.stop();
    mVendorNode.stop();
    //
    mDebugNode.uninit();
    mVendorNode.uninit();
    //
    mVendorNode.disconnect();
    //
    MY_LOGD("-");
    //
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
bool
Vendor_UT_Thread::
threadLoop()
{
    MY_LOGD("Eric:Vendor_UT_Thread isRunning");
    MY_LOGD("+");
    Mutex::Autolock _l(mRequestLock);
    //
    //SmartImageBuffer smDMGImgBuf = nullptr;
    //SmartImageBuffer smDMBGImgBuf = nullptr;
    //
    sp<EffectRequest> request = nullptr;
    //
    if(mVendor_UT->generate(request/*, smDMGImgBuf, smDMBGImgBuf*/))
    {
        if(!mbIsExit)
        {
            MY_LOGD("Vendor_UT_Thread:onData");
            request->mpOnRequestProcessed = effectHalCb;
            request->mpTag = this;
            mVendorNode.onData(ID_ROOT_ENQUE, request);
            MY_LOGD("Vendor_UT_Thread:onData done");
            //mVendorNode.onData(GF_BOKEH_DMG, smDMGImgBuf);
            //mVendorNode.onData(GF_BOKEH_DMBG, smDMBGImgBuf);
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
        MY_LOGD("wait request");
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
Vendor_UT_Thread::
effectHalCb(
    MVOID* tag,
    String8 status,
    sp<NSCam::EffectRequest>& request)
{
    const sp<EffectParameter> params = request->getRequestParameter();
    MINT32 scenario = params->getInt(VSDOF_FRAME_SCENARIO);
    MINT32 bokehLevel = params->getInt(VSDOF_FRAME_BOKEH_LEVEL);
    MINT32 gsensor = params->getInt(VSDOF_FRAME_G_SENSOR_ORIENTATION);
    //
    MY_LOGD("requestId(%d), scenario(%d), bokehLevel(%d), gsensor(%d)",
                request->getRequestNo(),
                scenario,
                bokehLevel,
                gsensor);
    // dump image buffer
    makePath("/sdcard/vsdof/bokeh_ut/result/vendor", 0660);
    sp<IImageBuffer> buffer = nullptr;
    string saveFileName = "";
    sp<EffectFrameInfo> frame = nullptr;
    for(MINT32 i=0;i<request->vOutputFrameInfo.size();++i)
    {
        frame = request->vOutputFrameInfo.valueAt(i);
        if(frame->getFrameNo() == BOKEH_ER_BUF_DISPLAY)
        {
            frame->getFrameBuffer(buffer);
            saveFileName = string("/sdcard/vsdof/bokeh_ut/result/vendor/WDMA_")+
                            to_string(request->getRequestNo())+
                            string(".yuv");
            buffer->saveToFile(saveFileName.c_str());
        }
    }
}