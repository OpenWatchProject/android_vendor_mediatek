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

#ifndef _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PLUGIN_STREAMINFO_STREAMINFOPLUGIN_H_
#define _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PLUGIN_STREAMINFO_STREAMINFOPLUGIN_H_

#include <any>
#include <functional>
#include <ios>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "types.h"

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::plugin::streaminfo {


/**
 * @param sizeInBytes: The size, in bytes.
 */
struct AllocInfo
{
    size_t                      sizeInBytes{0};
};


static inline std::string toString(AllocInfo const& o)
{
    std::ostringstream oss;
    oss << "{";
    oss << ".sizeInBytes=" << o.sizeInBytes;
    oss << "}";
    return oss.str();
}


/**
 * @param privDataId: The private data Id.
 *
 * @param privData: The private data.
 *
 * @param allocInfo: The allocation information.
 *  There is only 1 heap (i.e. allocInfo.size()==1) supported at the moment.
 *
 */
struct PluginData
{
    PrivateDataId               privDataId{PrivateDataId::BAD};
    PrivateDataT                privData;
    std::vector<AllocInfo>      allocInfo;
};


static inline std::string toString(PluginData const& o)
{
    std::ostringstream oss;
    oss << "{";

    oss << " .privDataId=" << toString(o.privDataId);
    if ( ! has_data(o.privData) ) {
        oss << " .privData=N/A";
    }

    oss << " .allocInfo[" << o.allocInfo.size() << "]=";
    for (auto const& v : o.allocInfo) {
        oss << toString(v);
    }

    oss << " }";
    return oss.str();
}


/**
 * DeterminePluginDataFunctionT defines the function prototype to determine the plugin data.
 * DeterminePluginDataArgument is the type of its general argument.
 *
 * @param openId: Logical device open id.
 *
 * @param sensorId: Physical sensor id (0, 1, 2)
 */
struct DeterminePluginDataArgument
{
    int32_t                     openId = -1;
    std::vector<int32_t>        sensorId;
};
using DeterminePluginDataFunctionT = std::function<
    PluginData(
        /*general input*/DeterminePluginDataArgument const&,
        /*private input*/const void*
    )>;


static inline std::string toString(DeterminePluginDataArgument const& o)
{
    std::ostringstream oss;
    oss << "{";
    oss << " .openId=" << o.openId;
    oss << " .sensorId[" << o.sensorId.size() << "]=[";
    for (auto v : o.sensorId) {
        oss << ' ' << v;
    }
    oss << " ]";

    oss << " }";
    return oss.str();
}


/**
 * @param pluginId: The plugin Id.
 *
 * @param options: the options of this plugin.
 *  0: This plugin can be unloaded once all its information are retrived at startup.
 *  1: It's a resident plugin, which won't be unloaded once it's registered.
 *
 * @param variantData: the plugin data of one of its alternative types.
 */
struct PluginInfo
{
    PluginId                    pluginId{PluginId::BAD};

    uint32_t                    options{1};

    std::variant<
        /*0*/PluginData,
        /*1*/DeterminePluginDataFunctionT
                >               variantData;

};


static inline std::string toString(PluginInfo const& o)
{
    std::ostringstream oss;
    oss << "{";
    oss << " .pluginId=" << toString(o.pluginId);

    oss << " .options=" << std::hex << o.options;

    oss << " .variantData<" << o.variantData.index() << ">";
    if ( auto p = std::get_if<0>(&o.variantData) ) {
        oss << "=" << toString(*p);
    }

    oss << " }";
    return oss.str();
}


/**
 * The argument passed to the call FetchPlugins().
 *
 * @param[out] plugins: This field reports all the supported plugins (in this library).
 *  Callers must ensure this field is valid.
 */
struct FetchPluginsParams
{
    std::vector<PluginInfo>*    plugins{nullptr};
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace
#endif//_MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PLUGIN_STREAMINFO_STREAMINFOPLUGIN_H_
