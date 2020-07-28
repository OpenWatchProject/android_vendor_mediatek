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
 * Frame 3A Control is to do frame hw sync and 3A sync under dual camera mode
 * To create an instance of Frame3AControl, 2 device open id must be provided.
 ******************************************************************************/
#ifndef _FRAME_3A_CONTROL_H_
#define _FRAME_3A_CONTROL_H_

#include <mtkcam/feature/DualCam/IFrame3AControl.h>
//#include <BuiltinTypes.h>
#include <mtkcam/def/common.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/drv/IHalSensor.h>
//#include <mtkcam/aaa/ISync3A.h>
#include <utils/Mutex.h>
//#include <Hal3/IHal3A.h>

namespace NSCam
{

typedef struct Frame3AInfo
{
    MINT32                  openId;
    MINT32                  devId;
    NS3Av3::IHal3A*         hal3A;
    IHalSensor* halSensor;
    MUINT32                 shutter;
    MINT32                  minFps;
    MINT32                  maxFps;
    MINT8                   frameSyncState;
} Frame3AInfo_t;

class Frame3AControl
    : public IFrame3AControl
{

    private:
        Frame3AControl(
            MINT32 const openId_1,
            MINT32 const openId_2
        );
    public:

        ~Frame3AControl(void);
        // Interface of IFrame3AControl
        static sp<IFrame3AControl>  createInstance(
            MINT32 const openId_1,
            MINT32 const openId_2
        );
        // interface
        MBOOL                       init(void);
        MBOOL                       uninit(void);
        //
        MBOOL                       get(Frame3ASetting_t* setting, MUINT32 const type);
        MBOOL                       set(Frame3ASetting_t* setting, MUINT32 const type,
                                        IMetadata* appMetadata, IMetadata* halMetadata);
protected:
        MBOOL                       enable3ASync(MINT32 const openId, IMetadata* halMetadata);
        MBOOL                       disable3ASync(MINT32 const openId, IMetadata* halMetadata);
        //
        MBOOL                       enable2ASync(MINT32 const openId, IMetadata* halMetadata);
        MBOOL                       disable2ASync(MINT32 const openId, IMetadata* halMetadata);
        //
        MBOOL                       enableAFSync(MINT32 const openId, IMetadata* halMetadata);
        MBOOL                       disableAFSync(MINT32 const openId, IMetadata* halMetadata);
        //
        MBOOL                       enableFrameSync(
                                        MINT32 const openId,
                                        IMetadata* appMetadata,
                                        IMetadata* halMetadata);
        MBOOL                       disableFrameSync(
                                        MINT32 const openId,
                                        IMetadata* appMetadata,
                                        IMetadata* halMetadata,
                                        MBOOL const pauseOnly = MTRUE);
        //
        MVOID                       disableReaptingTag(IMetadata* halMetadata);
        MBOOL                       checkProperty(void);
        MBOOL                       init3ASync(MINT32 const openId_1, MINT32 const openId_2);
        MBOOL                       uninit3ASync(void);
        MBOOL                       initFrameSync(void);
        MBOOL                       uninitFrameSync(void);
        MINT32                      getIndexFromOpenId(MINT32 const openId);
        struct Frame3AInfo*         getFrame3AInfoFromOpenId(MINT32 const openId);
        MBOOL                       IsAELockState(Frame3AInfo_t* const f3aInfo, MBOOL state);
        status_t                    setHwsyncDrv(MBOOL enable);
        MINT32                      getHwSyncWaitTime(void);


        // Frame sync
        MBOOL                       mbDvfsLevel;
        MBOOL                       mFrameSyncDrvInit;
        MBOOL                       mFrameSyncOnUninit;

        // 3A sync
        //NS3Av3::ISync3AMgr*         mpSync3AMgr;

        Frame3AInfo_t               mFrame3AInfo[2];
        typedef enum Frame3AHWSyncState
        {
            F3A_HWSYNC_DISABLED   = 0,
            F3A_HWSYNC_LOCKING_AE,
            F3A_HWSYNC_AE_LOCKED,
            F3A_HWSYNC_ENABLING,
            F3A_HWSYNC_UNLOCKING_AE,
            F3A_HWSYNC_READY,
            F3A_HWSYNC_CANCELING,
            F3A_HWSYNC_PAUSED,
            F3A_HWSYNC_ERROR,
        } Frame3AHWSyncState_e;

        MBOOL                       mIs3ASyncEnabled;
        Mutex                       m3ASyncLock;

        MINT32                      mFrameSyncStartCount;
        MINT32                      mFrameSyncWaitCount;
        Mutex                       mFrameSyncLock;
        MINT32                      mFrameSyncCtrlID;

        MINT32                      mForce3ASyncDisabled;
        MINT32                      mForceFrameSyncDisabled;

        MINT32                      mLogCond;
        MBOOL                       mbCCUSupport;
};
} // namespace NSCam
#endif  // _FRAME_3A_CONTROL_H_
