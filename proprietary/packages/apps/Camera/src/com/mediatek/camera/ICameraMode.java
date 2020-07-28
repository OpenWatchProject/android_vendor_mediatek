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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.camera;

import android.graphics.SurfaceTexture;

public interface ICameraMode {

    public enum CameraModeType {
        EXT_MODE_PHOTO,
        EXT_MODE_VIDEO,
        EXT_MODE_FACE_BEAUTY,
        EXT_MODE_PANORAMA,
        EXT_MODE_PHOTO_PIP,
        EXT_MODE_VIDEO_PIP,
        EXT_MODE_SLOW_MOTION,
        EXT_MODE_STEREO_CAMERA,
        EXT_MODE_VIDEO_STEREO,
        EXT_MODE_PHOTO_STEREO
    }

    public enum ActionType {
        ACTION_ON_CAMERA_OPEN, //maybe not in main thread
        ACTION_ON_CAMERA_PARAMETERS_READY, //maybe not in main thread
        ACTION_ON_CAMERA_CLOSE,
        ACTION_ORITATION_CHANGED,
        ACTION_FACE_DETECTED,
        ACTION_PHOTO_SHUTTER_BUTTON_CLICK,
        ACTION_VIDEO_SHUTTER_BUTTON_CLICK,
        ACTION_SHUTTER_BUTTON_FOCUS,
        ACTION_SHUTTER_BUTTON_LONG_PRESS,
        ACTION_OK_BUTTON_CLICK,
        ACTION_CANCEL_BUTTON_CLICK,
        ACTION_ON_LONG_PRESS,
        ACTION_ON_SINGLE_TAP_UP,
        ACTION_PREVIEW_VISIBLE_CHANGED,
        ACTION_ON_FULL_SCREEN_CHANGED,
        ACTION_ON_BACK_KEY_PRESS,
        ACTION_ON_KEY_EVENT_PRESS,
        ACTION_ON_SURFACE_TEXTURE_READY,
        ACTION_ON_MEDIA_EJECT,
        ACTION_ON_RESTORE_SETTINGS,
        ACTION_ON_USER_INTERACTION,
        ACTION_ON_START_PREVIEW,
        ACTION_ON_STOP_PREVIEW,
        ACTION_SET_DISPLAYROTATION,
        ACTION_ON_PREVIEW_DISPLAY_SIZE_CHANGED,
        ACTION_ON_PREVIEW_BUFFER_SIZE_CHANGED,
        ACTION_ON_SETTING_BUTTON_CLICK,
        ACTION_SWITCH_DEVICE, //TODO: delete it
        ACTION_NOTIFY_SURFCEVIEW_DISPLAY_IS_READY,
        ACTION_ON_COMPENSATION_CHANGED,
        ACTION_DISABLE_VIDEO_RECORD,
        ACTION_ON_CONFIGURATION_CHANGED,
        ACTION_ON_SELFTIMER_STATE,
        ACTION_NOTIFY_SURFCEVIEW_DESTROYED,
        ACTION_CAN_DO_AUTO_FOCUS,
    }

    public enum ModeState {
        // current camera's initialize state
        STATE_UNKNOWN,
        // camera is idle,so can do focusing/capture/recording
        // Preview is started
        STATE_IDLE,
        // camera is in focusing ,so you can not do capture/recording
        STATE_FOCUSING,
        // camera is take picture
        STATE_CAPTURING,
        // camera current is used for Recording
        STATE_RECORDING,
        // current is saving picture
        STATE_SAVING,
        // camera is closed, you can not do the devices relative actions,
        // such as take picture or do focus
        STATE_CLOSED
    }

    public ModeState getModeState();

    public void setModeState(ModeState state);

    public void resume();

    public void pause();

    //TODO:delete it
    public void destory();

    /**
     * open camera mode
     *
     * @param activity
     * @param cameraContext
     *            camera mode can get FileSaver/FocusManager/CameraDevice...
     *            from this parameter.
     * @return success -> true, fail -> false
     */
    public boolean open();

    /**
     * close camera mode.
     *
     * @return success -> true, fail -> false
     */
    public boolean close();

    //TODO: delete the following 4 API:
    public boolean isDisplayUseSurfaceView();

    public boolean isDeviceUseSurfaceView();

    public SurfaceTexture getBottomSurfaceTexture();

    public SurfaceTexture getTopSurfaceTexture();
    //
    public boolean isRestartCamera();

    public boolean isNeedDualCamera();


    /**
     * notify events to camera mode, such as button click, button focus,
     * orientation change...
     *
     * @param type
     *            action type
     * @param arg
     * @return
     */
    public boolean execute(ActionType type, Object... arg);
}
