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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_DEBUG_DEBUG_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_DEBUG_DEBUG_H_
//
/******************************************************************************
 *
 ******************************************************************************/
#include <memory>
#include <set>
#include <string>
#include <vector>
//
#include <utils/Printer.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/**
 *
 */
struct  IDebuggee
{
    virtual         ~IDebuggee() {}

    /**
     * Get the debuggee name.
     * This name must match to one of the names defined in CommandTable.h
     */
    virtual auto    debuggeeName() const -> std::string                     = 0;

    /**
     * Dump debugging state.
     */
    virtual auto    debug(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                    ) -> void                                               = 0;

};


/**
 *
 */
struct  IDebuggeeCookie
{
    virtual     ~IDebuggeeCookie() {}
};


/**
 *
 */
struct  IDebuggeeManager
{
    static  auto    get() -> IDebuggeeManager*;

    virtual         ~IDebuggeeManager() {}

    /**
     * Attach/detach a debuggee for debugging.
     *
     * @param[in] priority: 0=middle, 1=high, -1=low
     */
    virtual auto    attach(
                        std::shared_ptr<IDebuggee> debuggee,
                        int priority = 0
                    ) -> std::shared_ptr<IDebuggeeCookie>                   = 0;

    virtual auto    detach(
                        std::shared_ptr<IDebuggeeCookie> cookie
                    ) -> void                                               = 0;

    /**
     * Get the interesting debug process names.
     */
    virtual auto    getDebugProcessNames() const -> std::set<std::string>   = 0;

    /**
     * Dump debugging state.
     */
    virtual auto    debug(
                        std::shared_ptr<android::Printer> printer,
                        const std::vector<std::string>& options
                    ) -> void                                               = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_DEBUG_DEBUG_H_

