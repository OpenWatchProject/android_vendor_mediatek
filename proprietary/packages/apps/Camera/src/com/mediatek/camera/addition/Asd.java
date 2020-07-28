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
package com.mediatek.camera.addition;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice.AsdListener;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.util.Log;

public class Asd extends CameraAddition {
    private static final String TAG = "Asd";

    private static final int SCENE_BACK_LIGHT = 2;
    private static final int SCENE_BACK_LIGHT_PORTRAIT = 8;
    private static final int SCENE_UNKNOWN = -1;
    private int mLastScene = SCENE_UNKNOWN;

    private enum AsdState {
        STATE_IDLE, STATE_OPENED,
    }

    private AsdState mCurrentState = AsdState.STATE_IDLE;

    public Asd(ICameraContext cameraContext) {
        super(cameraContext);
    }

    @Override
    public boolean isSupport() {
        boolean isSupport = false;
        if ("on".equals(mISettingCtrl.getSettingValue(SettingConstants.KEY_ASD))) {
            isSupport = true;
        }
        return isSupport;
    }

    @Override
    public void open() {
        startAsd();
    }

    @Override
    public boolean isOpen() {
        boolean isOpen = false;
        if (AsdState.STATE_IDLE != mCurrentState) {
            isOpen = true;
        }
        Log.d(TAG, "[isOpen] isOpen:" + isOpen);
        return isOpen;
    }

    @Override
    public void close() {
        stopAsd();
    }

    public void startAsd() {
        Log.d(TAG, "[startAsd]...");
        updateCameraDevice();
        if (mICameraDevice == null) {
            return;
        }
        mICameraDevice.setAsdCallback(mASDCaptureCallback);
        mCurrentState = AsdState.STATE_OPENED;
    }

    private void stopAsd() {
        Log.d(TAG, "[stopAsd]mCurrentState = " + mCurrentState);
        if (mCurrentState == AsdState.STATE_IDLE) {
            return;
        }

        if (mICameraDevice != null) {
            mICameraDevice.setAsdCallback(null);
            mICameraAppUi.restoreSceneMode();
        }
        mLastScene = SCENE_UNKNOWN;
        mCurrentState = AsdState.STATE_IDLE;
    }

    private final AsdListener mASDCaptureCallback = new AsdListener() {
        public void onDeviceCallback(int scene) {
            Log.d(TAG, "[onDeviceCallback] onDetected scene = " + scene + "," +
                    "mLastScene:" + mLastScene);

            if (mLastScene != scene) {
                boolean suggestedHdr = (scene == SCENE_BACK_LIGHT
                        || scene == SCENE_BACK_LIGHT_PORTRAIT);
                mICameraAppUi.onDetectedSceneMode(scene, suggestedHdr);
                mLastScene = scene;
            }
        }
    };
}