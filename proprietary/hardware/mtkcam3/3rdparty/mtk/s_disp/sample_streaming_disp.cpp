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

#include <cutils/properties.h>

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

using NSCam::EImageFormat;
using NSCam::eImgFmt_NV21;
using NSCam::eImgFmt_NV12;
using NSCam::eImgFmt_YV12;
using NSCam::eImgFmt_UNKNOWN;

#ifdef LOG_TAG
#undef LOG_TAG
#endif // LOG_TAG
#define LOG_TAG "MtkCam/TPI_S_Disp"
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

class S_DISP_Plugin : public JoinPlugin::IProvider
{
public:
    typedef JoinPlugin::Property Property;
    typedef JoinPlugin::Selection Selection;
    typedef JoinPlugin::Request::Ptr RequestPtr;
    typedef JoinPlugin::RequestCallback::Ptr RequestCallbackPtr;
    typedef JoinPlugin::ConfigParam ConfigParam;

public:
    S_DISP_Plugin();
    ~S_DISP_Plugin();
    const Property& property();
    void set(MINT32 openID1, MINT32 openID2);
    void init();
    void uninit();
    void config(const ConfigParam &param);
    MERROR negotiate(Selection& sel);
    MERROR process(RequestPtr pRequest, RequestCallbackPtr pCallback);
    void abort(std::vector<RequestPtr> &pRequests);

private:
    MERROR getConfigSetting(Selection &sel);
    MERROR getP1Setting(Selection &sel);
    MERROR getP2Setting(Selection &sel);
    void drawMask(IImageBuffer *buffer, float fx, float fy, float fw, float fh);

private:
    int    mOpenID1 = 0;
    int    mOpenID2 = 0;
};

S_DISP_Plugin::S_DISP_Plugin()
{
    MY_LOGI("create S_DISP plugin");
}

S_DISP_Plugin::~S_DISP_Plugin()
{
    MY_LOGI("destroy  S_DISP plugin");
}

void S_DISP_Plugin::set(MINT32 openID1, MINT32 openID2)
{
    MY_LOGD("set openID1:%d openID2:%d", openID1, openID2);
    mOpenID1 = openID1;
    mOpenID2 = openID2;
}

const S_DISP_Plugin::Property& S_DISP_Plugin::property()
{
    static Property prop;
    static bool inited;

    if (!inited) {
        prop.mName = "MTK DISP";
        prop.mFeatures = MTK_FEATURE_FB;
        inited = true;
    }
    return prop;
}

MERROR S_DISP_Plugin::negotiate(Selection &sel)
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

MERROR S_DISP_Plugin::getConfigSetting(Selection &sel)
{
    sel.mCfgOrder = 5;
    sel.mCfgJoinEntry = eJoinEntry_S_DISP_ONLY;
    sel.mCfgRun = property_get_bool("vendor.debug.tpi.s.disp", 0);
    sel.mIBufferMain1.setRequired(MTRUE);

    IMetadata *meta = sel.mIMetadataApp.getControl().get();
    MY_LOGD("sessionMeta=%p", meta);
    return OK;
}

MERROR S_DISP_Plugin::getP1Setting(Selection &sel)
{
    (void)sel;
    return OK;
}

MERROR S_DISP_Plugin::getP2Setting(Selection &sel)
{
    MBOOL run = property_get_bool("vendor.debug.tpi.s.disp.run", 1);
    sel.mP2Run = run;
    MY_LOGD("mP2Run=%d", sel.mP2Run);
    return OK;
}

void S_DISP_Plugin::init()
{
    MY_LOGI("init S_DISP plugin");
}

void S_DISP_Plugin::uninit()
{
    MY_LOGI("uninit S_DISP plugin");
}

void S_DISP_Plugin::config(const ConfigParam &param)
{
    MY_LOGI("config S_DISP plugin: fmt=0x%x size=%dx%d stride=%d",
            param.mIBufferMain1.mFormat,
            param.mIBufferMain1.mSize.w, param.mIBufferMain1.mSize.h,
            param.mIBufferMain1.mStride);
}

void S_DISP_Plugin::abort(std::vector<RequestPtr> &pRequests)
{
    (void)pRequests;
    MY_LOGD("uninit S_DISP plugin");
};

MERROR S_DISP_Plugin::process(RequestPtr pRequest, RequestCallbackPtr pCallback)
{
    (void)pCallback;
    MERROR ret = -EINVAL;
    MBOOL needRun = MFALSE;
    IImageBuffer *disp = NULL;

    needRun = property_get_bool("vendor.debug.tpi.s.disp", 0);
    MY_LOGD("enter DISP_PLUGIN needRun=%d", needRun);

    if( needRun &&
        pRequest->mIBufferMain1 != NULL )
    {
        MSize inSize;
        unsigned inPlane = 0;
        EImageFormat inFmt = eImgFmt_UNKNOWN;

        disp = pRequest->mIBufferMain1->acquire();

        if( disp )
        {
            inSize = disp->getImgSize();
            inPlane = disp->getPlaneCount();
            inFmt = (EImageFormat)disp->getImgFormat();
        }

        MY_LOGD("DISP_PLUGIN disp=[%d](%dx%d)(0x%x)=%p",
                inPlane, inSize.w, inSize.h, inFmt, disp);

        if( disp )
        {
            drawMask(disp, 0.8, 0.8, 0.1, 0.1);
            // simulate delay 10 ms
            usleep(10000);
            ret = OK;
        }

        pRequest->mIBufferMain1->release();
    }

    MY_LOGD("exit DISP_PLUGIN ret=%d", ret);
    return ret;
}

void S_DISP_Plugin::drawMask(IImageBuffer *buffer, float fx, float fy, float fw, float fh)
{
    // sample: modify output buffer
    if( buffer )
    {
        char *ptr = (char*)buffer->getBufVA(0);
        if( ptr )
        {
            char mask = 128;
            MSize size = buffer->getImgSize();
            int stride = buffer->getBufStridesInBytes(0);
            int y_from = fy * size.h;
            int y_to = (fy + fh) * size.h;
            int x = fx * size.w;
            int width = fw * size.w;

            y_to = std::max(0, std::min(y_to, size.h));
            y_from = std::max(0, std::min(y_from, y_to));
            x = std::max(0, std::min(x, size.w));
            width = std::max(0, std::min(width, size.w));

            for( int y = y_from; y < y_to; ++y )
            {
                memset(ptr+y*stride + x, mask, width);
            }
        }
    }
}


REGISTER_PLUGIN_PROVIDER_DYNAMIC(Join, S_DISP_Plugin, MTK_FEATURE_FB);
