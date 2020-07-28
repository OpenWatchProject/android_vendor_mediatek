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

package com.android.camera.bridge;

import junit.framework.Assert;

import com.android.camera.FocusManager;
import com.android.camera.FocusManager.Listener;

import com.mediatek.camera.platform.IFocusManager;

public class FocusManagerImpl implements IFocusManager {
    private static final String TAG = "FocusManagerImpl";

    private FocusListener mFocusListener;
    private FocusManager mFocusManager;

    public FocusManagerImpl(FocusManager focusManager) {
        Assert.assertNotNull(focusManager);
        mFocusManager = focusManager;
    }

    @Override
    public void setAfData(byte[] data) {
        mFocusManager.setAfData(data);
    }

    @Override
    public boolean setListener(FocusListener listener) {
        mFocusListener = listener;
        mFocusManager.setListener(mListener);
        return true;
    }

    @Override
    public boolean setAeLock(boolean lock) {
        mFocusManager.setAeLock(lock);
        return true;
    }

    @Override
    public boolean setAwbLock(boolean lock) {
        mFocusManager.setAwbLock(lock);
        return true;
    }

    @Override
    public boolean resetTouchFocus() {
        mFocusManager.resetTouchFocus();
        return true;
    }

    @Override
    public void removeMessages() {
        mFocusManager.removeMessages();
    }

    @Override
    public boolean updateFocusUI() {
        mFocusManager.updateFocusUI();
        return true;
    }

    @Override
    public void focusAndCapture() {
        mFocusManager.doSnap();
    }

    @Override
    public void overrideFocusMode(String focusMode) {
        mFocusManager.overrideFocusMode(focusMode);
    }

    @Override
    public void clearFocusAndFaceUi() {
        mFocusManager.clearFocusAndFaceUi();
    }

    public void onAutoFocusMoving(boolean moving) {
        mFocusManager.onAutoFocusMoving(moving);
    }

    public void onPreviewStarted() {
        mFocusManager.onPreviewStarted();
    }

    public void onPreviewStopped() {
        mFocusManager.onPreviewStopped();
    }

    public String getFocusMode() {
        return mFocusManager.getFocusMode();
    }

    public boolean isFocusingSnapOnFinish() {
        return mFocusManager.isFocusingSnapOnFinish();
    }

    public void onShutterDown() {
        mFocusManager.onShutterDown();
    }

    public void onShutterUp() {
        mFocusManager.onShutterUp();
    }

    public void onAutoFocus(boolean focused) {
        mFocusManager.onAutoFocus(focused);
    }


    public void onSingleTapUp(int x, int y) {
        mFocusManager.onSingleTapUp(x, y);
    }

    public boolean getFocusAreaSupported() {
        return mFocusManager.getFocusAreaSupported();
    }

    public void setDistanceInfo(String info) {
        mFocusManager.setDistanceInfo(info);
    }

    public String getDistanceInfo() {
        return mFocusManager.getDistanceInfo();
    }

    public void cancelAutoFocus() {
        mFocusManager.cancelAutoFocus();
    }

    private Listener mListener = new Listener() {
        @Override
        public void autoFocus() {
            mFocusListener.autoFocus();
        }

        @Override
        public void cancelAutoFocus() {
            mFocusListener.cancelAutoFocus();
        }

        @Override
        public boolean capture() {
            return mFocusListener.capture();

        }

        @Override
        public void startFaceDetection() {
            mFocusListener.startFaceDetection();
        }

        @Override
        public void stopFaceDetection() {
            mFocusListener.stopFaceDetection();
        }

        @Override
        public void setFocusParameters() {
            mFocusListener.setFocusParameters();
        }

        @Override
        public void playSound(int soundId) {
            mFocusListener.playSound(soundId);
        }

        @Override
        public boolean readyToCapture() {
            return true;
        }
    };
}
