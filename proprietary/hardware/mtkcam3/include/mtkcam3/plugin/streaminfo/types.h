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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PLUGIN_STREAMINFO_TYPES_H_
#define _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PLUGIN_STREAMINFO_TYPES_H_

#include <ios>
#include <map>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <mtkcam/def/common.h>
#include <mtkcam/def/ImageBufferInfo.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::plugin::streaminfo {


/**
 * Plugin Id
 */
enum class PluginId : uint32_t
{
    BAD = 0,    // Bad id
    P1STT,

};


static inline std::string toString(PluginId const o)
{
    switch (o)
    {
    case PluginId::BAD:             return "BAD";
    case PluginId::P1STT:           return "P1STT";
    default: break;
    }
    return "TBD?" + std::to_string(static_cast<uint32_t>(o));
}


/**
 * Private Data Id
 */
enum class PrivateDataId : uint32_t
{
    BAD = 0,    // Bad id
    P1STT,
};


static inline std::string toString(PrivateDataId const o)
{
    switch (o)
    {
    case PrivateDataId::BAD:        return "BAD";
    case PrivateDataId::P1STT:      return "P1STT";
    default: break;
    }
    return "TBD?" + std::to_string(static_cast<uint32_t>(o));
}


/**
 * This structure is corresponding to PrivateDataId::P1STT.
 *
 * @param useLcso: use Lcso if true.
 * @param useLcsho: use Lcesho if true.
 * @param lcsoInfo: store LCSO buffer layout type for user calling heap->createImageBuffers_FromBlobHeap() API
 * @param lcshoInfo: store LCSHO buffer layout type for user calling heap->createImageBuffers_FromBlobHeap() API
 * @param totalSize: heap total size in byte
 */
struct P1STT
{
public:
    typedef std::shared_ptr<ImageBufferInfo> InfoPtr;
    typedef std::map<uint32_t, InfoPtr> BufInfoMap;
    typedef void (*UPDATE_FUNC)(uint32_t batchNum, P1STT* p1stt);
    struct DataStore
    {
        bool    useLcso = false;
        bool    useLcsho = false;
        BufInfoMap lcsoInfoMap;
        BufInfoMap lcshoInfoMap;
        size_t  totalSize = 0;
        uint32_t maxBatch = 1;
        int32_t logLevel = 0;
    };

    bool useLcso() const { return mData ? mData->useLcso : false; }
    bool useLcsho() const { return mData ? mData->useLcsho : false; }
    size_t getTotalSize() const { return mData ? mData->totalSize : 0; }

    std::shared_ptr<DataStore>  mData;
    InfoPtr mLcsoInfo;
    InfoPtr mLcshoInfo;
    UPDATE_FUNC updateFunc = nullptr;

};

struct P1STT_QueryParam
{
    uint32_t p1Batch = 1;
};

static inline std::string toString(P1STT const& o)
{
    std::ostringstream oss;
    oss << "{";
    oss << " .useLcso=" << o.useLcso();
    oss << " .useLcsho=" << o.useLcsho();
    oss << " .LcsoOffset Cnt =" << (o.mLcsoInfo ? o.mLcsoInfo->count : 0);
    oss << " .LcshoOffset Cnt =" << (o.mLcshoInfo ? o.mLcshoInfo->count : 0);
    oss << " }";
    return oss.str();
}
/**
 * The variant definition of Private data types.
 */
using PrivateDataVariant = std::variant<
    /*PrivateDataId::BAD*/ std::monostate,
    /*PrivateDataId::P1STT*/ P1STT
>;


////////////////////////////////////////////////////////////////////////////////
//  The definition of Private data type and its access.
////////////////////////////////////////////////////////////////////////////////

/**
 * The definition of Private data type.
 */
using PrivateDataT = PrivateDataVariant;

/**
 * Obtains a pointer to the value of a given type, returns null on error.
 */
template<class T>
inline
const T* get_data_if(const PrivateDataT* operand) noexcept
{
    return std::get_if<T>(operand);
}

/**
 * Checks if it holds a data.
 */
inline
bool has_data(const PrivateDataT& operand) noexcept
{
    return ! std::holds_alternative<std::monostate>(operand);
}


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam::plugin::streaminfo
#endif//_MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PLUGIN_STREAMINFO_TYPES_H_
