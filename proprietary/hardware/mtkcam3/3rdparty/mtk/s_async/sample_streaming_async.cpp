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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam3/3rdparty/core/singleton_cache.h>

#include <cutils/properties.h>

#include "s_async_data.h"

using NSCam::NSPipelinePlugin::Interceptor;
using NSCam::NSPipelinePlugin::PipelinePlugin;
using NSCam::NSPipelinePlugin::PluginRegister;
using NSCam::NSPipelinePlugin::Join;
using NSCam::NSPipelinePlugin::JoinPlugin;

using namespace NSCam::NSPipelinePlugin;
using NSCam::MSize;

using NSCam::MERROR;
using NSCam::IImageBuffer;
using NSCam::IMetadata;

using NSCam::NSTPIUtil::SingletonCache;

#ifdef LOG_TAG
#undef LOG_TAG
#endif // LOG_TAG
#define LOG_TAG "MtkCam/TPI_S_ASYNC"
#include <log/log.h>
#include <android/log.h>
#include <mtkcam/utils/std/ULog.h>

#define MY_LOGI(fmt, arg...)  CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)  CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)  CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)  CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define FUNCTION_IN   MY_LOGD("%s +", __FUNCTION__)
#define FUNCTION_OUT  MY_LOGD("%s -", __FUNCTION__)

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_TPI_PLUGIN);

class S_ASYNC_Plugin : public JoinPlugin::IProvider
{
public:
    typedef JoinPlugin::Property Property;
    typedef JoinPlugin::Selection Selection;
    typedef JoinPlugin::Request::Ptr RequestPtr;
    typedef JoinPlugin::RequestCallback::Ptr RequestCallbackPtr;

public:
    S_ASYNC_Plugin();
    ~S_ASYNC_Plugin();
    const Property& property();
    void set(MINT32 openID1, MINT32 openID2);
    void init();
    void uninit();
    MERROR negotiate(Selection& sel);
    MERROR process(RequestPtr pRequest, RequestCallbackPtr pCallback);
    void abort(std::vector<RequestPtr> &pRequests);

private:
    MERROR getConfigSetting(Selection &sel);
    MERROR getP1Setting(Selection &sel);
    MERROR getP2Setting(Selection &sel);
    void simulate(IImageBuffer *in);

private:
    int   mOpenID1  = 0;
    int   mOpenID2  = 0;

    bool  mUseLog   = false;
    bool  mUseNV21  = false;
    bool  mUseSize  = false;
    bool  mUseWait  = false;
    int   mDelay    = 0;
    int   mDelayMin = 0;
    int   mDelayMax = 0;
    int   mInitDelay = 0;

    SingletonCache<S_ASYNC_Data> mSingletonData;
};

S_ASYNC_Plugin::S_ASYNC_Plugin()
{
    MY_LOGI("create S_ASYNC plugin");
}

S_ASYNC_Plugin::~S_ASYNC_Plugin()
{
    MY_LOGI("destroy  S_ASYNC plugin");
}

void S_ASYNC_Plugin::set(MINT32 openID1, MINT32 openID2)
{
    MY_LOGD("set openID1:%d openID2:%d", openID1, openID2);
    mOpenID1 = openID1;
    mOpenID2 = openID2;
}

const S_ASYNC_Plugin::Property& S_ASYNC_Plugin::property()
{
    static Property prop;
    static bool inited;

    if (!inited) {
        prop.mName = "MTK ASYNC ASD";
        prop.mFeatures = MTK_FEATURE_ASYNC_ASD;
        inited = true;
    }
    return prop;
}

MERROR S_ASYNC_Plugin::negotiate(Selection &sel)
{
    MERROR ret = OK;

    if( sel.mSelStage == eSelStage_CFG )
    {
        ret = getConfigSetting(sel);
    }
    else if( sel.mSelStage == eSelStage_P1 )
    {
        ret = getP1Setting(sel);
    }
    else if( sel.mSelStage == eSelStage_P2 )
    {
        ret = getP2Setting(sel);
    }

    return ret;
}

MERROR S_ASYNC_Plugin::getConfigSetting(Selection &sel)
{
    mUseLog   = property_get_bool("vendor.debug.tpi.s.async.log", 0);
    mUseNV21  = property_get_bool("vendor.debug.tpi.s.async.nv21", 0);
    mUseSize  = property_get_bool("vendor.debug.tpi.s.async.size", 0);
    mUseWait  = property_get_bool("vendor.debug.tpi.s.async.wait", 0);
    mDelay    = property_get_int32("vendor.debug.tpi.s.async.delay", 80);
    mDelayMin = property_get_int32("vendor.debug.tpi.s.async.min", 15);
    mDelayMax = property_get_int32("vendor.debug.tpi.s.async.max", 90);
    mInitDelay = property_get_int32("vendor.debug.tpi.s.async.initDelay", 60);

    mDelay = std::min(std::max(mDelay, 0), 300);
    mDelayMin = std::min(std::max(mDelayMin, 0), 300);
    mDelayMax = std::min(std::max(mDelayMax, mDelayMin), 300);

    sel.mCfgOrder = 99;
    sel.mCfgJoinEntry = eJoinEntry_S_ASYNC;
    sel.mCfgRun = property_get_bool("vendor.debug.tpi.s.async", 0);
    sel.mCfgExpectMS = mDelay;
    sel.mCfgAsyncType = mUseWait ? eAsyncType_WAITING : eAsyncType_FIX_RATE;
    sel.mIBufferMain1.setRequired(MTRUE);
    if( mUseNV21 )
    {
        sel.mIBufferMain1.addAcceptedFormat(NSCam::eImgFmt_NV21);
    }
    if( mUseSize )
    {
        sel.mIBufferMain1.setSpecifiedSize(MSize(640, 480));
    }
    sel.mOBufferMain1.setRequired(MTRUE);

    IMetadata *meta = sel.mIMetadataApp.getControl().get();
    MY_LOGD("sessionMeta=%p, delay=%d(min/max:%d/%d)", meta, mDelay, mDelayMin, mDelayMax);

    return OK;
}

MERROR S_ASYNC_Plugin::getP1Setting(Selection &sel)
{
    (void)sel;
    return OK;
}

MERROR S_ASYNC_Plugin::getP2Setting(Selection &sel)
{
    MBOOL run = MTRUE;
    sel.mP2Run = run;
    return OK;
}

void S_ASYNC_Plugin::init()
{
    MY_LOGI("init S_ASYNC plugin %d:%s, initDelay:%d ms", mOpenID1, S_ASD_SINGLETON_KEY, mInitDelay);
    if( mInitDelay > 0 )
    {
        usleep(mInitDelay * 1000);
    }
    mSingletonData.attach(mOpenID1, S_ASD_SINGLETON_KEY, "s_async_plugin");
    MY_LOGI("init S_ASYNC plugin %d:%s, done", mOpenID1, S_ASD_SINGLETON_KEY);
}

void S_ASYNC_Plugin::uninit()
{
    MY_LOGI("uninit S_ASYNC plugin %d:%s", mOpenID1, S_ASD_SINGLETON_KEY);
    mSingletonData.detach();
}

void S_ASYNC_Plugin::abort(std::vector<RequestPtr> &pRequests)
{
    (void)pRequests;
    MY_LOGD("uninit S_ASYNC plugin");
};

MERROR S_ASYNC_Plugin::process(RequestPtr pRequest, RequestCallbackPtr pCallback)
{
    (void)pCallback;
    MERROR ret = -EINVAL;
    MBOOL needRun = MFALSE;

    needRun = MTRUE;
    MY_LOGD("enter ASYNC_PLUGIN needRun=%d", needRun);

    if( needRun &&
        pRequest->mIBufferMain1 != NULL )
    {
        IImageBuffer *in = pRequest->mIBufferMain1->acquire();
        simulate(in);
        pRequest->mIBufferMain1->release();
        ret = OK;
    }

    MY_LOGD("exit ASYNC_PLUGIN ret=%d", ret);
    return ret;
}

void S_ASYNC_Plugin::simulate(IImageBuffer *in)
{
    MSize inSize;
    unsigned inPlane = 0;
    unsigned delayMS = 0;
    if( in )
    {
        inSize = in->getImgSize();
        inPlane = in->getPlaneCount();
    }

    delayMS = mDelayMin + rand()%(mDelayMax-mDelayMin+1);

    S_ASYNC_Data data;
    mSingletonData.query(data);
    data.mCounter += 1;
    data.mValue = delayMS;
    mSingletonData.update(data);

    MY_LOGD("ASYNC_PLUGIN in(%dx%d:%d)=%p delay=%d data.mCounter=%d data.mValue=%d", inSize.w, inSize.h, inPlane, in, delayMS, data.mCounter, data.mValue);
    usleep(delayMS * 1000);
}

//REGISTER_PLUGIN_PROVIDER_DYNAMIC(Join, S_ASYNC_Plugin, MTK_FEATURE_ASYNC_ASD);
REGISTER_PLUGIN_PROVIDER(Join, S_ASYNC_Plugin);
