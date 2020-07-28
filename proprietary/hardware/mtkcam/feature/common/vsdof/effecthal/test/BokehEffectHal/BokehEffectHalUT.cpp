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
#define LOG_TAG "BokehEffectHalUT"
//
#include <utils/RefBase.h>
#include <mtkcam/feature/effectHalBase/IEffectHal.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
#include "BokehEffectHalUT.h"
#include "FrameGeneratorFactory.h"
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
//
#include <sys/resource.h>
#include <DebugUtil.h>
#include <string>
//
#include <random>
//
#define BOKEHTHREAD_POLICY     (SCHED_OTHER)
#define BOKEHTHREAD_PRIORITY   (0)
//
using namespace android;
using namespace std;
using namespace VSDOF::Bokeh::UT;
using namespace StereoHAL;
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
BokehEffectHalUT::
BokehEffectHalUT():
gen(rd()),
dis(0,66)
{
    LOG_DBG("CKH: ctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
BokehEffectHalUT::
~BokehEffectHalUT()
{
    LOG_DBG("CKH: dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BokehEffectHalUT::
init(GeneratorParam param)
{
        std::mt19937 gen(rd());
    mpBokehEffectHal = new BokehEffectHal();
    if(mpBokehEffectHal == nullptr)
    {
        LOG_ERR("Create BokehEffectHal fail.");
        return MFALSE;
    }
    //
    if(param.scenario == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_PREVIEW)
    {
        LOG_DBG("Preview UT");
    }
    else if(param.scenario == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_CAPTURE)
    {
        LOG_DBG("Capture UT");
    }
    else if(param.scenario == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_RECORD)
    {
        LOG_DBG("Record UT");
    }
    //
    LOG_DBG("init BokehEffectHal");
    mpBokehEffectHal->init();
    sp<EffectParameter> effectParams = new EffectParameter();
    effectParams->set(BOKEH_EFFECT_REQUEST_OPEN_ID, 0xFFFF);
    effectParams->set(BOKEH_EFFECT_REQUEST_RUN_PATH, param.runPath);
    effectParams->set(VSDOF_FEAME_STOP_MODE, param.stopMode);//VSDOF_STOP_MODE::FLUSH);
    LOG_DBG("set params to BokehEffectHal");
    mpBokehEffectHal->setParameters(effectParams);
    mpBokehEffectHal->configure();
    mpBokehEffectHal->prepare();
    // init frame generator
    LOG_DBG("create FrameGenerator");
    mpFrameGenerator = FrameGeneratorFactory::createInstance(param.scenario);
    if(mpFrameGenerator == nullptr)
    {
        LOG_ERR("Create frame generator fail");
        return MFALSE;
    }
    LOG_DBG("set FrameGenerator params");
    if(!mpFrameGenerator->init(param))
    {
        LOG_ERR("FrameGeneratorFactory init fail");
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BokehEffectHalUT::
config()
{
    if(mpBokehEffectHal == nullptr)
    {
        LOG_ERR("mpBokehEffectHal is null");
        return MFALSE;
    }
    LOG_DBG("start BokehEffectHal");
    mpBokehEffectHal->start();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BokehEffectHalUT::
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
    if(mpBokehEffectHal != nullptr)
    {
        mpBokehEffectHal->abort();
        mpBokehEffectHal->release();
        mpBokehEffectHal->unconfigure();
        mpBokehEffectHal->uninit();
        delete mpBokehEffectHal;
        mpBokehEffectHal = NULL;
    }
    //
    if(mpFrameGenerator!=nullptr)
    {
        mpFrameGenerator->uninit();
        mpFrameGenerator = nullptr;
    }
    LOG_DBG("CKH: -");
    //
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
bool
BokehEffectHalUT::
threadLoop()
{
    Mutex::Autolock _l(mRequestLock);
    sp<EffectRequest> request =
                        mpFrameGenerator->generate();
    if(request != nullptr)
    {
        if(!mbIsExit)
        {
            LOG_DBG("CKH: updateEffectRequest");
            request->mpOnRequestProcessed = effectHalCb;
            request->mpTag = this;
            mpBokehEffectHal->updateEffectRequest(request);
        }
    }
    else
    {
        LOG_DBG("wait request");
        mRequestCond.wait(mRequestLock);
    }
    MINT32 waitTime = dis(gen);
    LOG_DBG("Wait time: %d", waitTime);
    Wait(waitTime);
    return true;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BokehEffectHalUT::
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
    sp<IImageBuffer> buffer = nullptr;
    string saveFileName = "";
    sp<EffectFrameInfo> frame = nullptr;
    if(scenario == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_PREVIEW)
    {
        makePath("/sdcard/vsdof/bokeh_ut/result/effectHal/prv", 0660);
        for(MINT32 i=0;i<request->vOutputFrameInfo.size();++i)
        {
            frame = request->vOutputFrameInfo.valueAt(i);
            frame->getFrameBuffer(buffer);
            saveFileName = string("/sdcard/vsdof/bokeh_ut/result/effectHal/prv/WDMA_")+
                            to_string(request->getRequestNo())+
                            string(".yuv");
            buffer->saveToFile(saveFileName.c_str());
        }
    }
    else if(scenario == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_RECORD)
    {
        makePath("/sdcard/vsdof/bokeh_ut/result/effectHal/rec", 0660);
        for(MINT32 i=0;i<request->vOutputFrameInfo.size();++i)
        {
            frame = request->vOutputFrameInfo.valueAt(i);
            if(frame->getFrameNo() == BOKEH_ER_BUF_DISPLAY)
            {
                frame->getFrameBuffer(buffer);
                saveFileName = string("/sdcard/vsdof/bokeh_ut/result/effectHal/rec/WDMA_")+
                                to_string(request->getRequestNo())+
                                string(".yuv");
                buffer->saveToFile(saveFileName.c_str());
            }
            else if(frame->getFrameNo() == BOKEH_ER_BUF_RECORD)
            {
                frame->getFrameBuffer(buffer);
                saveFileName = string("/sdcard/vsdof/bokeh_ut/result/effectHal/rec/WROT_")+
                                to_string(request->getRequestNo())+
                                string(".yuv");
                buffer->saveToFile(saveFileName.c_str());
            }
        }
    }
    else if(scenario == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_CAPTURE)
    {
        makePath("/sdcard/vsdof/bokeh_ut/result/effectHal/cap", 0660);
        for(MINT32 i=0;i<request->vOutputFrameInfo.size();++i)
        {
            frame = request->vOutputFrameInfo.valueAt(i);
            if(frame->getFrameNo() == BOKEH_ER_BUF_VSDOF_IMG)
            {
                frame->getFrameBuffer(buffer);
                saveFileName = string("/sdcard/vsdof/bokeh_ut/result/effectHal/cap/VSDOF_")+
                                to_string(request->getRequestNo())+
                                string(".yuv");
                buffer->saveToFile(saveFileName.c_str());
            }
            else if(frame->getFrameNo() == BOKEH_ER_BUF_CLEAN_IMG)
            {
                frame->getFrameBuffer(buffer);
                saveFileName = string("/sdcard/vsdof/bokeh_ut/result/effectHal/cap/Clean_")+
                                to_string(request->getRequestNo())+
                                string(".yuv");
                buffer->saveToFile(saveFileName.c_str());
            }
            else if(frame->getFrameNo() == BOKEH_ER_BUF_THUMBNAIL)
            {
                frame->getFrameBuffer(buffer);
                saveFileName = string("/sdcard/vsdof/bokeh_ut/result/effectHal/cap/Thumbnail_")+
                                to_string(request->getRequestNo())+
                                string(".yuv");
                buffer->saveToFile(saveFileName.c_str());
            }
        }
    }
}