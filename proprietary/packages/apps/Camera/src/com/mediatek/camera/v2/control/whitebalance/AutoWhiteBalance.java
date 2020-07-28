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

package com.mediatek.camera.v2.control.whitebalance;

import android.app.Activity;
import android.graphics.RectF;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.TotalCaptureResult;
import android.view.ViewGroup;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.control.IControl.IAaaListener;
import com.mediatek.camera.v2.module.ModuleListener.CaptureType;
import com.mediatek.camera.v2.module.ModuleListener.RequestType;
import com.mediatek.camera.v2.setting.ISettingServant;
import com.mediatek.camera.v2.setting.ISettingServant.ISettingChangedListener;
import com.mediatek.camera.v2.setting.SettingConvertor;
import com.mediatek.camera.v2.util.SettingKeys;

import java.util.ArrayList;
import java.util.Map;

public class AutoWhiteBalance implements IWhiteBalance, ISettingChangedListener {
    private static final Tag TAG = new Tag(AutoWhiteBalance.class.getSimpleName());
    private final IAaaListener     mIAaaListener;
    private final ISettingServant  mSettingServant;
    private ArrayList<String>      mCaredSettingChangedKeys = new ArrayList<String>();
    private String                 mAWBMode = null;
    private RectF                  mPreviewArea;
    public AutoWhiteBalance(ISettingServant settingServant, IAaaListener aaaListener) {
        mIAaaListener = aaaListener;
        mSettingServant = settingServant;
    }

    @Override
    public void open(Activity activity, ViewGroup parentView,
            boolean isCaptureIntent) {
        updateCaredSettingChangedKeys();
        mSettingServant.registerSettingChangedListener(this, mCaredSettingChangedKeys,
                ISettingChangedListener.MIDDLE_PRIORITY);
    }

    @Override
    public void resume() {
        updateAwbCompensation();
    }

    @Override
    public void pause() {

    }

    @Override
    public void close() {
        mSettingServant.unRegisterSettingChangedListener(this);
    }

    @Override
    public void onPreviewAreaChanged(RectF previewArea) {
        mPreviewArea = previewArea;
    }

    @Override
    public void onSingleTapUp(float x, float y) {
        if (mPreviewArea == null) {
            return;
        }

    }

    @Override
    public void configuringSessionRequest(RequestType requestType,
            Builder requestBuilder, boolean bottomCamera) {
        LogHelper.i(TAG, "[configuringSessionRequests] + camera id:"
                + mSettingServant.getCameraId());
        if (mAWBMode != null) {
            int awb = SettingConvertor.convertStringToEnum(SettingKeys.KEY_WHITE_BALANCE, mAWBMode);
            requestBuilder.set(CaptureRequest.CONTROL_AWB_MODE, awb);
        }
        LogHelper.i(TAG, "[configuringSessionRequests]- requestType = " + requestType +
                " AWBMode = " + mAWBMode);
    }

    @Override
    public void onPreviewCaptureStarted(CaptureRequest request, long timestamp,
            long frameNumber) {

    }

    @Override
    public void onPreviewCaptureCompleted(CaptureRequest request,
            TotalCaptureResult result) {

    }

    @Override
    public void onSettingChanged(Map<String, String> result) {
        String[] keys = {SettingKeys.KEY_WHITE_BALANCE, SettingKeys.KEY_CAMERA_ID};
        for (String key : keys) {
            String value = result.get(key);
            if (value != null) {
                updateAwbCompensation();
            }
        }
    }

    private void updateCaredSettingChangedKeys() {
        String[] keys = {SettingKeys.KEY_WHITE_BALANCE, SettingKeys.KEY_CAMERA_ID};
        for (String key : keys) {
            if (key != null && !mCaredSettingChangedKeys.contains(key)) {
                mCaredSettingChangedKeys.add(key);
            }
        }

    }

    private void updateAwbCompensation() {
        LogHelper.i(TAG, "[updateSceneMode]+");
        String awb = mSettingServant.getSettingValue(SettingKeys.KEY_WHITE_BALANCE);
        if (awb != null && !awb.equals(mAWBMode)) {
            mAWBMode = awb;
            mIAaaListener.requestChangeCaptureRequets(false,
                    mIAaaListener.getRepeatingRequestType(), CaptureType.REPEATING_REQUEST);
        }
        LogHelper.i(TAG, "[updateSceneMode]- ");
    }
}