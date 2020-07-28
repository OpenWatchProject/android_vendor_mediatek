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
 ******************************************************************************/
#ifndef _IFRAME_3A_CONTROL_H_
#define _IFRAME_3A_CONTROL_H_

//#include <BuiltinTypes.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <mtkcam/aaa/IHal3A.h>
//#include <Hal3/IHal3A.h>
#include <mtkcam/utils/metadata/IMetadata.h>

using namespace android;

namespace NSCam
{

typedef struct Frame3ASetting
{
    MINT32 const            openId;          // for lowpower mode setting
    MBOOL                   is3ASyncEnabled;
    MUINT32                 hwSyncMode;
    //MUINT32               zoom_ratio;
    MINT32                  minFps;
    MINT32                  maxFps;
    // for 3a sync
    MINT32                  AELv_x10;
    MINT32                  AFDAC;
    MINT32                  isoValue;
    MBOOL                   isAFDone;
    MBOOL                   isSyncAFDone;
    MBOOL                   isSync2ADone;
    // for fov online
    MINT32                   maxDac;
    MINT32                   minDac;
    MINT32                   expTime;
    MINT32                   temperature;
    // set master/slave
    MINT32                  masterId;
    MINT32                  slaveId;
    // drop req mode
    MINT32                  dropMode;
} Frame3ASetting_t;

typedef enum Frame3ASettingType
{
    F3A_TYPE_3A_SYNC                = 1,
    F3A_TYPE_FRAME_SYNC             = 1 << 1,
    F3A_TYPE_FRAMERATE_CHANGE       = 1 << 2,
    F3A_TYPE_MASTER_SLAVE_SET       = 1 << 3,
    F3A_TYPE_DROP_MODE              = 1 << 4,
    F3A_TYPE_3A_INFO                = 1 << 5,
    F3A_TYPE_FOV_ONLINE_INFO = 1 << 6,
    F3A_TYPE_ALL                    = 0xFFFFFFFF
} Frame3ASettingType_e;

typedef enum Frame3AFpsType
{
    F3A_FPS_DONTCARE   = 0,
    F3A_FPS_HIGH       = 1,
    F3A_FPS_LOW        = 2,
    F3A_FPS_CUSTOM     = 3,
} Frame3AFpsType_e;

class IFrame3AControl
    : public virtual RefBase
{
    public:
        static sp<IFrame3AControl>   createInstance(MINT32 const openId_1, MINT32 const openId_2);
        virtual                      ~IFrame3AControl(void) {}

        // init 3A and frame sync
        virtual MBOOL                init(void) = 0;
        virtual MBOOL                uninit(void) = 0;
        //
        virtual MBOOL                get(Frame3ASetting_t* setting, MUINT32 const type) = 0;
        virtual MBOOL                set(Frame3ASetting_t* setting, MUINT32 const type,
                                         IMetadata* appMetadata, IMetadata* halMetadata) = 0;
};

} // namespace NSCam
#endif  // _IFRAME_3A_CONTROL_H_
