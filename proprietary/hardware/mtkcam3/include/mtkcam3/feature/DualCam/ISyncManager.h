/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_I_SYNCMANAGER_H_
#define _MTK_HARDWARE_MTKCAM_I_SYNCMANAGER_H_

#include <utils/RefBase.h>
#include <mtkcam3/feature/hdrDetection/Defs.h>

using namespace android;


/******************************************************************************
 * ISyncManagerCallback
 ******************************************************************************/
namespace NSCam
{
enum
{
    //  Notify Preview Camera ID
    MTK_SYNCMGR_MSG_NOTIFY_MASTER_ID      = 0x00000001,
};

struct SyncManagerParams
{
    MUINT32         miZoomRatio = 0;
    //
    MBOOL           mb4KVideo = MFALSE;
    //
    NSCam::HDRMode  mHDRMode = NSCam::HDRMode::OFF;
    //
    void*           mUserData = nullptr;

    MINT32          mEisFactor;
    MSize           mRRZOSize;

    MBOOL           mPartialUpdate = MFALSE;

    MINT32          mIsVideoRec = MFALSE;
    // bokeh
    MINT32          mDofLevel = 0;
};

class ISyncManagerCallback : public virtual android::RefBase
{
    public:
        virtual ~ISyncManagerCallback() {}
        virtual MINT32 onEvent(
            MINT32 const i4OpenId,
            MINT32 arg1, MINT32 arg2, void* arg3) = 0;
};


class ISyncManager : public virtual android::RefBase
{
    public:
        static android::sp<ISyncManager> createInstance(
            MINT32 const i4OpenId
        );
        static android::sp<ISyncManager> getInstance(
            MINT32 const i4OpenId
        );

        virtual ~ISyncManager() {}

    public:
        virtual MINT32 updateSetting(
            MINT32 const i4OpenId,
            IMetadata* appMetadata,
            IMetadata* halMetadata,
            SyncManagerParams &syncParams
        ) = 0;


        // lock/unlock the state changing
        virtual MVOID lock(void* arg1) = 0;
        virtual MVOID unlock(void* arg1) = 0;

        //
        virtual MVOID registerMgrCb(sp<ISyncManagerCallback> cb) = 0;

        //
        virtual MVOID setEvent(MUINT32 key, void * arg) = 0;
};

} // namespace NSCam

#endif // _MTK_HARDWARE_MTKCAM_I_SYNCMANAGER_H_
