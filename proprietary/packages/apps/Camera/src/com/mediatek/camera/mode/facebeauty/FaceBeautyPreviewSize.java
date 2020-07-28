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
package com.mediatek.camera.mode.facebeauty;

import android.app.Activity;
import android.content.Context;
import android.hardware.Camera.Size;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.ISettingRule;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingUtils;
import com.mediatek.camera.util.Log;

import java.util.ArrayList;
import java.util.List;

public class FaceBeautyPreviewSize implements ISettingRule {
    private static final String TAG = "FaceBeautyPreviewSize";

    private static final int UNKNOWN_INDEX = -1;

    private ICameraContext mICameraContext;
    private ICameraDeviceManager mICameraDeviceManager;
    private ICameraDevice mICameraDevice;
    private ISettingCtrl mISettingCtrl;

    private Activity mActivity;
    private int mCameraId;
    private List<Size> mSupportedPreviewSizes = null;
    private Size mCurrentPreviewSize = null;
    private List<String> mConditions = new ArrayList<String>();

    public FaceBeautyPreviewSize(ICameraContext ct) {
        mICameraContext = ct;
        mActivity = mICameraContext.getActivity();
        mISettingCtrl = mICameraContext.getSettingController();
        Log.i(TAG, "[FaceBeautyPreviewSize]");
    }

    @Override
    public void execute() {
        String value = mISettingCtrl.getSettingValue(SettingConstants.KEY_FACE_BEAUTY);
        int index = mConditions.indexOf(value);
        Log.i(TAG, "[execute],index = " + index);

        initizeParameters();

        if (UNKNOWN_INDEX == index) {
            String ratio = mISettingCtrl.getSettingValue(SettingConstants.KEY_PICTURE_RATIO);
            SettingUtils.setPreviewSize(mActivity, mICameraDevice.getParameters(), ratio);
        } else {
            setVFBPreviewSize();
        }
    }

    @Override
    public void addLimitation(String condition, List<String> result, MappingFinder mappingFinder) {
        mConditions.add(condition);
    }

    private void initizeParameters() {
        if (mICameraDeviceManager == null) {
            mICameraDeviceManager = mICameraContext.getCameraDeviceManager();
        }
        mCameraId = mICameraDeviceManager.getCurrentCameraId();
        mICameraDevice = mICameraDeviceManager.getCameraDevice(mCameraId);
        if (mICameraDevice == null) {
            Log.e(TAG, "[initizeParameters] current mICameraDevice is null");
        } else {
            mSupportedPreviewSizes = mICameraDevice.getSupportedPreviewSizes();
            mCurrentPreviewSize = mICameraDevice.getPreviewSize();
            Log.i(TAG, "[initizeParameters] mCurrentPreviewSize : " + mCurrentPreviewSize.width
                    + " X " + ",width = " + mCurrentPreviewSize.height);
        }
    }

    private void setVFBPreviewSize() {
        for (int i = 0; i < mSupportedPreviewSizes.size(); i++) {
            if (mSupportedPreviewSizes.get(i).width > FaceBeautyParametersHelper.
                    VIDEO_FACE_BEAUTY_MAX_SOLUTION_WIDTH
                    || mSupportedPreviewSizes.get(i).height > FaceBeautyParametersHelper.
                    VIDEO_FACE_BEAUTY_MAX_SOLUTION_HEIGHT) {
                Log.v(TAG, "will remove VFB not supported preview size[" + i + "],Width = "
                        + mSupportedPreviewSizes.get(i).width + ",Height = "
                        + mSupportedPreviewSizes.get(i).height);
                mSupportedPreviewSizes.remove(i);
                // why need i--???
                // because if removed one,the list size will change,so the next
                // one will be ignore if not change the index
                i--;
            }
        }
        leftSupportedPreviewSize(mSupportedPreviewSizes);

        String previewRatio = mISettingCtrl.getSettingValue(SettingConstants.KEY_PICTURE_RATIO);
        Size optiomSize = SettingUtils.getOptimalPreviewSize((Context) mActivity,
                mICameraDevice.getParameters(), previewRatio);

        Log.d(TAG, "[setVFBPreviewSize] will set preview width = " + optiomSize.width
                + ",height = " + optiomSize.height);
        mICameraDevice.setPreviewSize(optiomSize.width, optiomSize.height);
    }

    private void leftSupportedPreviewSize(List<Size> size) {
        String mStringBuffer = "[leftSupportedPreviewSize] ";
        for (int i = 0; i < mSupportedPreviewSizes.size(); i++) {
            mStringBuffer += mSupportedPreviewSizes.get(i).width + "X"
                    + mSupportedPreviewSizes.get(i).height + "; ";
        }
        Log.d(TAG, "[leftSupportedPreviewSize] is : " + mStringBuffer);

    }
}
