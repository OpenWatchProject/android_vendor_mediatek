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
using NSCam::NSTPIUtil::SingletonCache;

using namespace NSCam::NSPipelinePlugin;
using NSCam::MSize;

using NSCam::MERROR;
using NSCam::IImageBuffer;
using NSCam::IMetadata;

using NSCam::NSTPIUtil::SingletonCache;

#ifdef LOG_TAG
#undef LOG_TAG
#endif // LOG_TAG
#define LOG_TAG "MtkCam/TPI_S_META"
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

class S_META_Plugin : public JoinPlugin::IProvider
{
public:
    typedef JoinPlugin::Property Property;
    typedef JoinPlugin::Selection Selection;
    typedef JoinPlugin::Request::Ptr RequestPtr;
    typedef JoinPlugin::RequestCallback::Ptr RequestCallbackPtr;

public:
    S_META_Plugin();
    ~S_META_Plugin();
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
    IMetadata* acquire(const MetadataHandle::Ptr &meta);
    void release(const MetadataHandle::Ptr &meta);

private:
    int   mOpenID1  = 0;
    int   mOpenID2  = 0;

    SingletonCache<S_ASYNC_Data> mSingletonData;
};

S_META_Plugin::S_META_Plugin()
{
    MY_LOGI("create S_META plugin");
}

S_META_Plugin::~S_META_Plugin()
{
    MY_LOGI("destroy  S_META plugin");
}

void S_META_Plugin::set(MINT32 openID1, MINT32 openID2)
{
    MY_LOGD("set openID1:%d openID2:%d", openID1, openID2);
    mOpenID1 = openID1;
    mOpenID2 = openID2;
}

const S_META_Plugin::Property& S_META_Plugin::property()
{
    static Property prop;
    static bool inited;

    if (!inited) {
        prop.mName = "MTK META only";
        prop.mFeatures = MTK_FEATURE_ASYNC_ASD;
        inited = true;
    }
    return prop;
}

MERROR S_META_Plugin::negotiate(Selection &sel)
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

MERROR S_META_Plugin::getConfigSetting(Selection &sel)
{
    sel.mCfgOrder = 101;
    sel.mCfgJoinEntry = eJoinEntry_S_META_ONLY;
    sel.mCfgRun = property_get_bool("vendor.debug.tpi.s.meta", 0);
    IMetadata *meta = sel.mIMetadataApp.getControl().get();
    MY_LOGD("run=%d order=%d sessionMeta=%p", sel.mCfgRun, sel.mCfgOrder, meta);
    return OK;
}

MERROR S_META_Plugin::getP1Setting(Selection &sel)
{
    (void)sel;
    return OK;
}

MERROR S_META_Plugin::getP2Setting(Selection &sel)
{
    MBOOL run = MTRUE;
    sel.mP2Run = run;
    return OK;
}

void S_META_Plugin::init()
{
    MY_LOGI("init S_META plugin %d:%s", mOpenID1, S_ASD_SINGLETON_KEY);
    mSingletonData.attach(mOpenID1, S_ASD_SINGLETON_KEY, "s_meta_plugin");
}

void S_META_Plugin::uninit()
{
    MY_LOGI("uninit S_META plugin %d:%s", mOpenID1, S_ASD_SINGLETON_KEY);
    mSingletonData.detach();
}

void S_META_Plugin::abort(std::vector<RequestPtr> &pRequests)
{
    (void)pRequests;
    MY_LOGD("abort S_META plugin");
};

MERROR S_META_Plugin::process(RequestPtr pRequest, RequestCallbackPtr pCallback)
{
    (void)pCallback;
    MERROR ret = OK;
    MY_LOGD("enter S_META_PLUGIN");

    if( pRequest != NULL )
    {
        IMetadata *inApp      = acquire(pRequest->mIMetadataApp);
        IMetadata *inHal      = acquire(pRequest->mIMetadataHal1);
        IMetadata *inDynamic  = acquire(pRequest->mIMetadataDynamic1);
        IMetadata *outApp     = acquire(pRequest->mOMetadataApp);
        IMetadata *outHal     = acquire(pRequest->mOMetadataHal);

        S_ASYNC_Data data;
        mSingletonData.query(data);

        MY_LOGD("S_META_PLUGIN inApp=%p inHal=%p inDynamic=%p outApp=%p outHal=%p data.mCounter=%d data.mValue=%d", inApp, inHal, inDynamic, outApp, outHal, data.mCounter, data.mValue);

        release(pRequest->mIMetadataApp);
        release(pRequest->mIMetadataHal1);
        release(pRequest->mIMetadataDynamic1);
        release(pRequest->mOMetadataApp);
        release(pRequest->mOMetadataHal);
    }

    MY_LOGD("exit S_META_PLUGIN ret=%d", ret);
    return ret;
}

IMetadata* S_META_Plugin::acquire(const MetadataHandle::Ptr &meta)
{
    return meta != NULL ? meta->acquire() : NULL;
}

void S_META_Plugin::release(const MetadataHandle::Ptr &meta)
{
    if( meta != NULL )
    {
        meta->release();
    }
}

//REGISTER_PLUGIN_PROVIDER_DYNAMIC(Join, S_META_Plugin, MTK_FEATURE_ASYNC_ASD);
REGISTER_PLUGIN_PROVIDER(Join, S_META_Plugin);
