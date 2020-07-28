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

#ifndef DRV_ISP_CAM_IO_WRAPPER_UTILS_H
#define DRV_ISP_CAM_IO_WRAPPER_UTILS_H

// This header only contains implementation utilities
// Do not put any declaration related to the interface here

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>


namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {
namespace Wrapper {

enum DebugFlagEnum {
    DEBUG_DETAILS    = 0x1,
    DEBUG_FUNC_CALL  = 0x2,
    DEBUG_QUEUE      = 0x4,
    DEBUG_USER       = 0x8,
};

typedef unsigned int DebugFlag;


extern DebugFlag gDebugFlag; // Defined in NormalPipeWrapper.cpp

enum CamIoModule {
    NORMAL_PIPE_WRAPPER,
    LMV_IRQ_ADAPTER
};


template <CamIoModule module>
class LogLife
{
public:
    inline LogLife(const char *name, unsigned int debugFlag) : mName(name), mDebugFlag(debugFlag) {
        if (mDebugFlag & DEBUG_FUNC_CALL)
            CAM_ULOGD(NSCam::Utils::ULog::MOD_DRV_NORMAL_PIPE, "[%s] +", mName);
    }
    inline ~LogLife() {
        if (mDebugFlag & DEBUG_FUNC_CALL)
            CAM_ULOGD(NSCam::Utils::ULog::MOD_DRV_NORMAL_PIPE, "[%s] -", mName);
    }
private:
    const char *mName;
    unsigned int mDebugFlag;
};

};
};
};
};


// We don't use PIPE_xxx() here because there will be name conflicts while include other headers

#define WRP_DBG(fmt, arg...) \
    do { \
        if (NSCam::NSIoPipe::NSCamIOPipe::Wrapper::gDebugFlag & NSCam::NSIoPipe::NSCamIOPipe::Wrapper::DEBUG_DETAILS) { \
            CAM_ULOGD(NSCam::Utils::ULog::MOD_DRV_NORMAL_PIPE, "[%s]" fmt, __func__, ##arg); } \
    } while(0)

#define WRP_WRN(fmt, arg...)        CAM_ULOGW(NSCam::Utils::ULog::MOD_DRV_NORMAL_PIPE, "[%s]" fmt, __func__, ##arg)
#define WRP_ERR(fmt, arg...)        CAM_ULOGE(NSCam::Utils::ULog::MOD_DRV_NORMAL_PIPE, "[%s]" fmt, __func__, ##arg)
#define WRP_AST_IF(cond, fmt, arg...) CAM_LOGA_IF(cond, "[%s]" fmt, __func__, ##arg)

#define UNUSED(var) (void)(var)


#endif

