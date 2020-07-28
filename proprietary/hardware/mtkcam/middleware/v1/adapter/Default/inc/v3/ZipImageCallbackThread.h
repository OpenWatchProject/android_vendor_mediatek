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

#ifndef _MTK_HAL_CAMADAPTER_MTKENG_INC_ZIPIMAGECALLBACKTHREAD_H_
#define _MTK_HAL_CAMADAPTER_MTKENG_INC_ZIPIMAGECALLBACKTHREAD_H_
//
#include <queue>
using namespace std;
//
#include <utils/threads.h>
#include <utils/StrongPointer.h>

namespace android {
namespace NSDefaultAdapter {


/******************************************************************************
 *  ZIP (Compressed) Image Callback Thread
 ******************************************************************************/
class ZipImageCallbackThread : public Thread
{
    public:

        typedef enum
        {
            // notify callback
            callback_type_image,
            callback_type_shutter,
            callback_type_p2done_notify,
            // data callback
            callback_type_raw,
            callback_type_raw_notify,
            callback_type_raw16,
            callback_type_metadata_raw16,
            callback_type_postview,
            // exit type
            callback_type_exit,
        }callback_type;

        struct callback_data
        {
            callback_type           type;
            //image
            camera_memory*          pImage;
            uint32_t                u4CallbackIndex;
            bool                    isFinalImage;
            //shutter
            bool                    bMtkExt;
            sp<CamMsgCbInfo>        spCamMsgCbInfo;
            int32_t                 i4PlayShutterSound;
            //p2done
            int32_t                 p2done_notifyCaptureDone; // flag indicates to send MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE within P2DONE CB or not
            //
            camera_metadata_t*      pResult;
            camera_metadata_t*      pCharateristic;
            //
            uint32_t                u4ShotMode;

            callback_data(  callback_type       _type = callback_type_exit,
                            camera_memory*      _pImage = NULL,
                            uint32_t            _u4CallbackIndex = 0,
                            bool                _isFinalImage = true,
                            bool                _bMtkExt = false,
                            sp<CamMsgCbInfo>    _spCamMsgCbInfo = NULL,
                            int32_t             _i4PlayShutterSound = 0,
                            camera_metadata_t*  _pResult = NULL,
                            camera_metadata_t*  _pCharateristic = NULL,
                            uint32_t            _u4ShotMode = NSCam::eShotMode_Undefined)
                : type(_type)
                , pImage(_pImage)
                , u4CallbackIndex(_u4CallbackIndex)
                , isFinalImage(_isFinalImage)
                , bMtkExt(_bMtkExt)
                , spCamMsgCbInfo(_spCamMsgCbInfo)
                , i4PlayShutterSound(_i4PlayShutterSound)
                , p2done_notifyCaptureDone(0)
                , pResult(_pResult)
                , pCharateristic(_pCharateristic)
                , u4ShotMode(_u4ShotMode)
            {}
        };

    protected:  ////                Data Members.
        sp<Thread>                  mpSelf;
        char const*const            mpszThreadName;
        IStateManager*              mpStateMgr;
        sp<CamMsgCbInfo>            mpCamMsgCbInfo;
        uint32_t                    mu4ShotMode;
        IState::ENState             mNextState;
        bool                        mIsExtCallback;
        MBOOL                       mIsThirdPartyZsd;
        int                         mCameraMode;

        Mutex                       mLock;
        Condition                   mCondCallback;

        queue<callback_data>        mqTodoCallback;

        Mutex                       mExitLock;
        MBOOL                       mbForceExit;

    public:

        ZipImageCallbackThread(
                IStateManager* pStateMgr,
                sp<CamMsgCbInfo> pCamMsgCbInfo
                );
        ~ZipImageCallbackThread();

        // Good place to do one-time initializations
        status_t    readyToRun();
        void        setShotMode(uint32_t const u4ShotMode, const IState::ENState nextState, String8 const s8AppMode);
        void        setThirdPartyZsdFlag(bool isThirdPartyZsd) {mIsThirdPartyZsd = isThirdPartyZsd;}
        void        setCameraMode(int cameraMode) {mCameraMode = cameraMode;}
        uint32_t    getShotMode() { return mu4ShotMode; }
        bool        isExtCallback() { return mIsExtCallback; }
        void        addCallbackData(callback_data* pData);
        void        forceExit();

    protected:

        bool        getCallbackData(callback_data* pData);
        bool        handleShutter(callback_data const& data);
        bool        handleP2doneNotify(callback_data const& data);
        bool        handleRawCallback(callback_data const& data);
        bool        handleRawNotifyCallback(callback_data const& data);
        bool        handleRaw16Callback(callback_data const& data);
        bool        handleMetadataRaw16Callback(callback_data& data);
        bool        handlePostviewCallback(callback_data const& data);
        bool        handleImageCallback(callback_data const& data);
        bool        processCallback(callback_data& pData);

    private:

        bool        threadLoop();
};

}; // namespace NSDefaultAdapter
}; // namespace android
#endif  //_MTK_HAL_CAMADAPTER_MTKENG_INC_ZIPIMAGECALLBACKTHREAD_H_

