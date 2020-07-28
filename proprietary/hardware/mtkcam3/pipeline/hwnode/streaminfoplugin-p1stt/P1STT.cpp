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

#define LOG_TAG "streaminfoplugin-p1stt"
//
#include <mtkcam3/plugin/streaminfo/StreamInfoPlugin.h>
//
#include <memory>
//
#include <cutils/compiler.h>
//
#include <mtkcam/utils/std/Log.h>

#include <mtkcam/def/ImageFormat.h>
#include <mtkcam/aaa/IIspMgr.h>
#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/def/ImageBufferInfo.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <cutils/properties.h>


using namespace NSCam::plugin::streaminfo;
using namespace NSCam;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/

namespace {
/**
 *  P1 STT Plugin
 */
struct SizeInfo
{
    bool    support = false;
    MSize   size;
    MINT    format = eImgFmt_UNKNOWN;
    size_t  stride = 0;
};

static P1STT::InfoPtr generateBufferInfo(uint32_t batchNum, uint32_t maxBatchNum, P1STT::BufInfoMap &map)
{
    P1STT::InfoPtr p = NULL;
    auto it = map.find(batchNum);
    if( it != map.end() && it->second )
    {
        p = it->second;
    }
    else
    {
        P1STT::InfoPtr baseInfo = map[maxBatchNum];
        if( !baseInfo )
        {
            MY_LOGF("Can not find batch(%d) info, which should exist in the beginning!!", maxBatchNum);
        }
        else
        {
            p = std::make_shared<ImageBufferInfo>();
            (*p) = (*baseInfo);
            p->count = batchNum;
            map[batchNum] = p;
        }
    }
    return p;
}

static void updateP1SttData(uint32_t batchNum, P1STT* p1stt)
{
    if( !p1stt )
    {
        MY_LOGF("p1stt is null when updating!");
        return;
    }
    if( batchNum > p1stt->mData->maxBatch )
    {
        MY_LOGF("P1STT update data batchNum(%d) > max Batch(%d)", batchNum, p1stt->mData->maxBatch);
    }
    else
    {
        if( p1stt->mData->useLcso )
        {
            p1stt->mLcsoInfo = generateBufferInfo(batchNum, p1stt->mData->maxBatch, p1stt->mData->lcsoInfoMap);
        }
        if( p1stt->mData->useLcsho )
        {
            p1stt->mLcshoInfo = generateBufferInfo(batchNum, p1stt->mData->maxBatch, p1stt->mData->lcshoInfoMap);
        }
    }
}

static std::shared_ptr<ImageBufferInfo> createInfo(int32_t logLevel, size_t initOffset, std::shared_ptr<IHwInfoHelper>& helper, uint32_t batchNum, SizeInfo &sizeInfo, size_t &endOffset)
{
    std::shared_ptr<ImageBufferInfo> outInfo = std::make_shared<ImageBufferInfo>();

    if ( ! helper->getDefaultBufPlanes_Pass1(outInfo->bufPlanes, sizeInfo.format, sizeInfo.size, sizeInfo.stride) )
    {
        MY_LOGF("!!err: infohelper->getDefaultBufPlanes_Pass1: FAILED!");
    }
    outInfo->imgFormat  = sizeInfo.format;
    outInfo->imgWidth   = sizeInfo.size.w;
    outInfo->imgHeight  = sizeInfo.size.h;

    size_t allocSizeInBytes = [](BufPlanes const& bp, MINT32 log) {
            size_t size = 0;
            for (size_t i = 0 ; i < bp.count ; i++) {
                size += bp.planes[i].sizeInBytes;
                MY_LOGD_IF(log, "Create Plane[%d]: size=%zu", i++, size);
            }
            return size;
        } (outInfo->bufPlanes, logLevel);

    outInfo->bufStep = allocSizeInBytes;
    outInfo->count   = batchNum;
    endOffset        = initOffset + batchNum * allocSizeInBytes;

    if( logLevel )
    {
        for(size_t i = 0; i < outInfo->count ; i++)
        {
            MY_LOGD("offset[%zu] = %zu", i, initOffset + i * allocSizeInBytes);
        }
        MY_LOGD("endOffset = %zu", endOffset);
    }
    return outInfo;
}

static P1STT createP1Stt(uint32_t batchNum, int32_t sensorId)
{
    std::shared_ptr<P1STT::DataStore> data = std::make_shared<P1STT::DataStore>();
    data->logLevel = property_get_int32("vendor.debug.P1STT.log", 0);
    data->maxBatch = batchNum;
    (void) sensorId;
#if MTKCAM_LTM_SUPPORT
    NS3Av3::LCSO_Param lcsoParam;
    NS3Av3::Buffer_Info bufInfo;
    SizeInfo lcsoInfo, lcshoInfo;
    std::shared_ptr<IHwInfoHelper> infohelper = IHwInfoHelperManager::get()->getHwInfoHelper(sensorId);
    MY_LOGF_IF(infohelper==nullptr, "getHwInfoHelper");
    auto ispHal = MAKE_HalISP(sensorId, LOG_TAG);
    if(ispHal==NULL)
    {
        MY_LOGE("ispHal is NULL!");
    }
    else
    {
        if (ispHal->queryISPBufferInfo(bufInfo))
        {
            MY_LOGD("queryISPBufferInfo, LCSO : support : %d, format : %d, size : %dx%d, LCSHO : support : %d, format : %d, size : %dx%d",
                    bufInfo.LCESO_Param.bSupport, bufInfo.LCESO_Param.format,
                    bufInfo.LCESO_Param.size.w, bufInfo.LCESO_Param.size.h,
                    bufInfo.LCESHO_Param.bSupport, bufInfo.LCESHO_Param.format,
                    bufInfo.LCESHO_Param.size.w, bufInfo.LCESHO_Param.size.h);
            lcsoInfo.support    = bufInfo.LCESO_Param.bSupport;
            lcsoInfo.format     = bufInfo.LCESO_Param.format;
            lcsoInfo.size       = bufInfo.LCESO_Param.size;
            lcsoInfo.stride     = bufInfo.LCESO_Param.stride;

            lcshoInfo.support   = bufInfo.LCESHO_Param.bSupport;
            lcshoInfo.format    = bufInfo.LCESHO_Param.format;
            lcshoInfo.size      = bufInfo.LCESHO_Param.size;
            lcshoInfo.stride    = bufInfo.LCESHO_Param.stride;
        }
        else if ( auto ispMgr = MAKE_IspMgr() )
        {
            ispMgr->queryLCSOParams(lcsoParam);
            lcsoInfo.support    = lcsoParam.size.w > 0 && lcsoParam.size.h > 0;
            lcsoInfo.format     = lcsoParam.format;
            lcsoInfo.size       = lcsoParam.size;
            lcsoInfo.stride     = lcsoParam.stride;
        }
        else {
            MY_LOGE("Query IIspMgr FAILED!");
        }
    }
    if(ispHal != NULL)
    {
        ispHal->destroyInstance(LOG_TAG);
    }

    data->useLcso = lcsoInfo.support;
    data->useLcsho = lcshoInfo.support;
    size_t offset = 0;
    if( data->useLcso )
    {
        data->lcsoInfoMap[batchNum] = createInfo(data->logLevel, offset, infohelper, batchNum, lcsoInfo, offset);
    }
    if( data->useLcsho )
    {
        data->lcshoInfoMap[batchNum] = createInfo(data->logLevel, offset, infohelper, batchNum, lcshoInfo, offset);
    }
    data->totalSize = offset;
#endif
    return P1STT{
            .mData = data,
            .mLcsoInfo = data->useLcso ? data->lcsoInfoMap[batchNum] : nullptr,
            .mLcshoInfo = data->useLcsho ? data->lcshoInfoMap[batchNum] : nullptr,
            .updateFunc = updateP1SttData,
        };
}

static const PluginInfo p1SttPlugin
{
    .pluginId = PluginId::P1STT,
    .variantData = [](DeterminePluginDataArgument const& arg1, const void* arg2){
            uint32_t batchNum = 1;
            int32_t sensorId = 0;
            if(arg1.sensorId.empty())
            {
                MY_LOGE("No sensorId in Plugin query args !!! use id = 0");
            }
            else
            {
                sensorId = arg1.sensorId[0];
            }
            if ( arg2 != nullptr ) {
                MY_LOGD("An argument exsits");
                const P1STT_QueryParam *param = reinterpret_cast<const P1STT_QueryParam*>(arg2);
                batchNum = param->p1Batch;
                MY_LOGD("batch num change to (%d), sId(%d)", batchNum, sensorId);
            }
            else {
                MY_LOGD("No argument: use default setting");
            }

            P1STT stt = createP1Stt(batchNum, sensorId);

            return PluginData{
                .privDataId = PrivateDataId::P1STT,
                .privData = stt,
                .allocInfo = {
                        { .sizeInBytes = stt.getTotalSize(), },
                    },
            };
        },
    .options = 1, //1: It's a resident plugin, which won't be unloaded once it's registered.
};

}; // namespace


/******************************************************************************
 *
 ******************************************************************************/
extern "C"
void
FetchPlugins(FetchPluginsParams const arg __unused)
{
    if (CC_UNLIKELY( arg.plugins == nullptr )) {
        return;
    }

    auto& plugins = *arg.plugins;
    plugins = {p1SttPlugin};
}

