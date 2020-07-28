/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include <mtkcam/pipeline/extension/ThirdPartyFeatures.h>

#define LOG_TAG "3rdFB"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

#include <mutex>

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
// ----------------------------------------------------------------------------
#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
// ----------------------------------------------------------------------------


using android::sp;
using android::wp;
using android::IParamsManager;

using namespace NSCam;


namespace MTK3rdParty {

static std::mutex s_lockParamMgr;
static wp< IParamsManager > s_wpParamsMgr;

sp< IParamsManager > getParamsManager()
{
    std::unique_lock< std::mutex > _locker( s_lockParamMgr );
    return s_wpParamsMgr.promote();
}


void setParamsManager(const sp<IParamsManager>& paramMgr)
{
    std::unique_lock< std::mutex > _locker( s_lockParamMgr );
    s_wpParamsMgr = paramMgr;
}


int FaceBeauty::run(
        NSCam::IImageBuffer*    pImg,
        NSCam::IMetadata*       pAppMeta,
        NSCam::IMetadata*       pHalMeta
)
{
    FUNCTION_SCOPE;

    // check custom hint tag
    MINT32 customHint = 0;
    if (IMetadata::getEntry<MINT32>(pHalMeta, MTK_PLUGIN_CUSTOM_HINT, customHint)) {
        MY_LOGD("got MTK_PLUGIN_CUSTOM_HINT:%d from HalMetaData", customHint);
    }
    else {
        MY_LOGD("cannot get MTK_PLUGIN_CUSTOM_HINT(default:%d)", customHint);
    }

    // how to retrieve orientation (value: 0, 90, 180, 270)
    MINT32 jpegOrientation = 0;
    if (IMetadata::getEntry<MINT32>(pAppMeta, MTK_JPEG_ORIENTATION, jpegOrientation)) {
        MY_LOGD("got MTK_JPEG_ORIENTATION:%d from AppMetaData", jpegOrientation);
    }
    else {
        MY_LOGD("cannot get MTK_JPEG_ORIENTATION(default:%d)", jpegOrientation);
    }

    auto imgFmt     = pImg->getImgFormat();
    if (__builtin_expect( imgFmt != eImgFmt_NV21, false )) {
        MY_LOGE("%s:%d: img format only supports NV21.", __FILE__, __LINE__);
        return EINVAL;
    }

    auto width      = pImg->getImgSize().w;
    auto height     = pImg->getImgSize().h;

    auto stride0    = pImg->getBufStridesInBytes(0); // Y
    auto stride1    = pImg->getBufStridesInBytes(1); // UV
    auto lines      = pImg->getImgSize().h;

    void* addr0     = (void*)(long long)pImg->getBufVA(0); // Y
    void* addr1     = (void*)(long long)pImg->getBufVA(1); // UV

    MY_LOGD("wxh=(%d,%d),stride(0,1)=(%d,%d),lines=%d",
            width, height, stride0, stride1, lines);
    MY_LOGD("addr Y=%p, addr UV=%p", addr0, addr1);

    // TODO: add your 3rd party algo here

    return 0;
}


int
FaceBeauty::isEnable(
        const NSCam::IMetadata*     pAppMeta        __attribute__((unused)),
        const NSCam::IMetadata*     pHalMeta        __attribute__((unused))
)
{
    MINT32 customHint = 0; // custom hint value is 0 means no need to enable FaceBeauty
    if (IMetadata::getEntry<MINT32>(pHalMeta, MTK_PLUGIN_CUSTOM_HINT, customHint)) {
        MY_LOGD("got MTK_PLUGIN_CUSTOM_HINT:%d from HalMetaData", customHint);
        return customHint;
    }
    else {
        MY_LOGD("cannot get MTK_PLUGIN_CUSTOM_HINT(default:%d)", customHint);
        return 0;
    }
}

};
