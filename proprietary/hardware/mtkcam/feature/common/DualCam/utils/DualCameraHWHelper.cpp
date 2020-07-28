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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
/******************************************************************************
*
*******************************************************************************/
#define LOG_TAG "MtkCam/DualCameraHWHelper"

// Standard C header file
#include <string>
#include <map>
#include <mutex>
#include <iostream>
#include <sstream>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/drv/iopipe/CamIO/Cam_QueryDef.h>
// Module header file

// Local header file
#include <mtkcam/feature/DualCam/utils/DualCameraHWHelper.h>

/******************************************************************************
*
*******************************************************************************/
#define COMMON_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGA(fmt, arg...)    CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGF(fmt, arg...)    CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define COMMON_FUN_IN()             COMMON_LOGD("+")
#define COMMON_FUN_OUT()            COMMON_LOGD("-")

class scoped_tracer
{
public:
    scoped_tracer(const char* functionName)
    : mFunctionName(functionName)
    {
        CAM_LOGD("[%s] +", mFunctionName);
    }
    ~scoped_tracer()
    {
        CAM_LOGD("[%s] -", mFunctionName);
    }
private:
    const char* const mFunctionName;
};
#define COMMON_SCOPED_TRACER() scoped_tracer ___scoped_tracer(__FUNCTION__)

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam
{
using namespace NSCam::NSIoPipe::NSCamIOPipe;

/******************************************************************************
 *
 ******************************************************************************/
namespace { // begin anonymous namespace

template<typename T>
struct DefaultDeletePolicy final
{
    DefaultDeletePolicy() = delete;

    static inline MVOID doDelete(const T&)
    {
        // do not things
    }
};
//
template<typename T>
struct PointertDeletePolicy final
{
    PointertDeletePolicy() = delete;

    static inline MVOID doDelete(const T ptr)
    {
        delete ptr;
    }
};
//
template<typename T>
struct ArrayDeletePolicy final
{
    ArrayDeletePolicy() = delete;

    static inline MVOID doDelete(T arrayPtr)
    {
        delete [] arrayPtr;
    }
};
//
template<typename TKey, typename TValue, template <typename> class DeletePolicy = DefaultDeletePolicy>
class Cache final
{
public:
    Cache(const std::string name)
    : mName(name)
    {

    }
    //
    inline void add(const TKey& key, const TValue& value)
    {
        std::lock_guard<std::mutex> guard(mlocker);
        {
            mTable[key] = value;
        }
    }
    //
    inline void remove(const TKey& key)
    {
        std::lock_guard<std::mutex> guard(mlocker);
        {
            auto it = mTable.find(key);
            if(it != mTable.end())
            {
                mTable.erase(it);
            }
        }
    }
    //
    inline MBOOL tryGetValue(const TKey& key, TValue& value)
    {
        MBOOL ret = false;
        std::lock_guard<std::mutex> guard(mlocker);
        {
            auto it = mTable.find(key);
            if(it != mTable.end())
            {
                value = mTable[key];
            }
        }
        return ret;
    }
    inline MBOOL isExisting(const TKey& key)
    {
        MBOOL ret = false;
        std::lock_guard<std::mutex> guard(mlocker);
        {
            auto it = mTable.find(key);
            if(it != mTable.end())
            {
                ret = true;
            }
        }
        return ret;
    }
    inline ~Cache()
    {
        for(auto item : mTable)
        {
            DeletePolicy<TValue>::doDelete(item.second);
        }
    }
    //
private:
    const std::string mName;

private:
    std::mutex mlocker;
    std::map<TKey, TValue> mTable;
};
//
Cache<DualCameraHWHelper::OpenId, MINT32> g_RrzoWorstWidthCache("rrzo_worst_width");
Cache<std::string, sCAM_QUERY_IQ_LEVEL*, PointertDeletePolicy> g_ImgQualityQueryCache("imgquality");
Cache<DualCameraHWHelper::OpenId, MSize> g_smallRrzoSizeCache("small_rrzo_size");
Cache<DualCameraHWHelper::OpenId, MINT> g_rawImgoFmtCache("raw_imgo_fmt");
Cache<DualCameraHWHelper::OpenId, MINT> g_rawRrzoFmtCache("raw_rrzo_fmt");


} // anonymous namespace
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCameraHWHelper::
getDynamicTwinSupported()
{
    auto pModule = INormalPipeModule::get();
    if  ( ! pModule ) {
        COMMON_LOGE("INormalPipeModule::get() fail");
        return MFALSE;
    }

    MBOOL ret = MFALSE;
    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
    pModule->query(0, NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_D_Twin, 0, 0, info);
    ret = info.D_TWIN;

    COMMON_LOGD("is dynamic twin supported: %d", ret);
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCameraHWHelper::
getImgQualitySupportedByPlatform()
{
    COMMON_SCOPED_TRACER();

    MBOOL ret = MFALSE;
    #if MTKCAM_ISP_SUPPORT_IQ
    {
        COMMON_LOGD("[workaround] target platform is ISP_SUPPORT_IQ, force to support image quality control");
        ret = MTRUE;
    }
    #endif
    COMMON_LOGD("ret: %d", ret);
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCameraHWHelper::
getImgQualitySupported(OpenId firstOpenId, OpenId secondOpenId, MINT32 firstRrzoWidth, MINT32 secondRrzoWidth)
{
    // TODO: to caculate the more efficient key value
    auto getKey = [](OpenId firstOpenId, OpenId secondOpenId, MINT32 firstRrzoWidth, MINT32 secondRrzoWidth) -> std::string
    {
        std::ostringstream stringStream;
        stringStream << firstOpenId << "-" << secondOpenId << "-" << firstRrzoWidth << "-" << secondRrzoWidth;
        return stringStream.str();
    };

    std::string key = getKey(firstOpenId, secondOpenId, firstRrzoWidth, secondRrzoWidth);

    MBOOL ret = false;

    sCAM_QUERY_IQ_LEVEL* sIQ = new sCAM_QUERY_IQ_LEVEL();
    if(!g_ImgQualityQueryCache.tryGetValue(key, sIQ))
    {
        auto pModule = INormalPipeModule::get();
        if  ( ! pModule )
        {
            COMMON_LOGE("INormalPipeModule::get() fail");
            return ret;
        }

        struct QueryInData_t firstQueryInfo;
        struct QueryInData_t secondQueryInfo;

        // TG1
        firstQueryInfo.sensorIdx = firstOpenId;
        firstQueryInfo.scenarioId = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        firstQueryInfo.rrz_out_w = firstRrzoWidth;
        firstQueryInfo.pattern = eCAM_NORMAL;

        // TG2
        secondQueryInfo.sensorIdx = secondOpenId;
        secondQueryInfo.scenarioId = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        secondQueryInfo.rrz_out_w = secondRrzoWidth;
        secondQueryInfo.pattern = eCAM_NORMAL;

        // the push order will cause different result
        sIQ->QueryInput.vInData.clear();
        sIQ->QueryInput.vInData.push_back(firstQueryInfo);
        sIQ->QueryInput.vInData.push_back(secondQueryInfo);

        // query image quality table
        pModule->query((MUINT32)ENPipeQueryCmd_IQ_LEVEL, (MUINTPTR)sIQ);

        // support IQ or not
        if(sIQ->QueryOutput == MTRUE)
        {
            COMMON_LOGD("support 3-Raw imgQuality control");

            // just show info
            std::ostringstream stringStream;
            for(MUINT32 i = 0 ; i < sIQ->QueryInput.vOutData.size() ; i++)
            {
                stringStream << "************************************" << std::endl;
                stringStream << "ImgQuality Table Set: #" << i << std::endl;
                stringStream << "************************************" << std::endl;
                for(MUINT32 j = 0 ; j < sIQ->QueryInput.vOutData[i].size() ; j++)
                {
                    const auto& item = sIQ->QueryInput.vOutData[i][j];
                    stringStream << "sensorIdx: " << item.sensorIdx
                                 << "isTwin: " << item.isTwin
                                 << "lvIQ: " << item.IQlv
                                 << "lvClk: " << item.clklv
                                 << "result: " << item.result << std::endl;
                    COMMON_LOGD("%s", stringStream.str().c_str());
                }
            }
        }
        g_ImgQualityQueryCache.add(key, sIQ);
    }
    return sIQ->QueryOutput;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCameraHWHelper::
setRrzoWorstWidth(OpenId openId, MINT32 width)
{
    MINT32 value = -1;
    if(g_RrzoWorstWidthCache.tryGetValue(openId, value))
    {
        COMMON_LOGD("worst rrzo width is existing, replace, openId: %d, width: %d -> %d", openId, value, width);
        g_RrzoWorstWidthCache.add(openId, width);

    }
    else
    {
        COMMON_LOGD("worst rrzo width is not existing, add the new one, openId: %d, width: %d", openId, width);
        g_RrzoWorstWidthCache.add(openId, width);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCameraHWHelper::
tryGetRrzoWorstWidth(OpenId openId, MINT32& width)
{
    return g_RrzoWorstWidthCache.tryGetValue(openId, width);
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCameraHWHelper::
setSmallRrzoSize(const MSize& size)
{
    static const OpenId openId = -1;

    MSize value = -1;
    if(g_smallRrzoSizeCache.tryGetValue(openId, value))
    {
        COMMON_LOGD("small rrzo size is existing, replace, openId: %d, size: (%d, %d) -> (%d, %d)", openId, value.w, value.h, size.w, size.h);
        g_smallRrzoSizeCache.add(openId, size);
    }
    else
    {
        COMMON_LOGD("small rrzo size is not existing, add the new one, openId: %d, size: (%d, %d)", openId, size.w, size.h);
        g_smallRrzoSizeCache.add(openId, size);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCameraHWHelper::
tryGetSmallRrzoSize(MSize& size)
{
    static const OpenId openId = -1;
    return g_smallRrzoSizeCache.tryGetValue(openId, size);
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCameraHWHelper::
setRawFormat(OpenId openId, const RawFmt& imgo, const RawFmt& rrzo)
{
    MINT iImgofmt = -1, iRrzofmt = -1;
    MBOOL isExisting = g_rawRrzoFmtCache.tryGetValue(openId, iImgofmt);
    isExisting |= g_rawRrzoFmtCache.tryGetValue(openId, iRrzofmt);
    if(isExisting)
    {
        COMMON_LOGD("[%d] raw fmt is existing, replace, imgo:(%d->%d) rrzo:(%d->%d)", openId, iImgofmt, imgo, iRrzofmt, rrzo);
        g_rawRrzoFmtCache.add(openId, imgo);
        g_rawRrzoFmtCache.add(openId, rrzo);
    }
    else
    {
        COMMON_LOGD("[%d] raw fmt is not existing, replace, imgo:(%d) rrzo:(%d)", openId, imgo, rrzo);
        g_rawRrzoFmtCache.add(openId, imgo);
        g_rawRrzoFmtCache.add(openId, rrzo);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCameraHWHelper::
tryGetRawFormat(OpenId openId, RawFmt& imgo, RawFmt& rrzo)
{
    MBOOL isExisting = g_rawRrzoFmtCache.tryGetValue(openId, imgo);
    isExisting |= g_rawRrzoFmtCache.tryGetValue(openId, rrzo);
    return isExisting;
}
/******************************************************************************
 *
 ******************************************************************************/
} // NSCam