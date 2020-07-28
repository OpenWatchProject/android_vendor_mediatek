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

#ifndef _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_UTILS_STREAMINFO_ISTREAMINFOPLUGINMANAGER_H_
#define _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_UTILS_STREAMINFO_ISTREAMINFOPLUGINMANAGER_H_

#include <memory>
#include <optional>
//
#include <utils/Printer.h>
//
#include <mtkcam3/plugin/streaminfo/StreamInfoPlugin.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3 {


/**
 * An interface of stream info plugin manager.
 */
class IStreamInfoPluginManager
{
public:     ////    Definitions.
    using PluginId = NSCam::plugin::streaminfo::PluginId;
    using PluginInfo = NSCam::plugin::streaminfo::PluginInfo;
    using PluginData = NSCam::plugin::streaminfo::PluginData;
    using DeterminePluginDataArgument = NSCam::plugin::streaminfo::DeterminePluginDataArgument;

public:     ////    Interfaces.
    virtual         ~IStreamInfoPluginManager() = default;

    /**
     * Get the singleton.
     */
    static  auto    get() -> std::shared_ptr<IStreamInfoPluginManager>;

    /**
     * Dump the state.
     */
    virtual auto    dumpState(android::Printer* printer) -> void                = 0;

    /**
     * Query the plugin info.
     *
     * @param pluginId: The plugin Id.
     *
     * @return The plugin information.
     */
    virtual auto    queryPluginInfo(PluginId id) -> std::optional<PluginInfo>   = 0;

    /**
     * Determine the plugin data.
     *
     * @param plugin: The plugin info.
     *
     * @param arg: It will be forwarded to the function to determine the plugin data if necessary.
     *
     * @return The plugin data.
     */
    virtual auto    determinePluginData(
                        std::optional<PluginInfo> const& plugin,
                        DeterminePluginDataArgument const& arg1,
                        const void* arg2 = nullptr
                    ) -> std::optional<PluginData>                              = 0;

};


/******************************************************************************
 *
 ******************************************************************************/

};  //namespace
#endif  //_MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_UTILS_STREAMINFO_ISTREAMINFOPLUGINMANAGER_H_

