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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

package com.mediatek.camera.v2.detection.facedetection;

import android.app.Activity;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.RectF;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest.Key;
import android.view.View;
import android.view.ViewGroup;

import com.android.camera.R;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.setting.ISettingServant;
import com.mediatek.camera.v2.setting.ISettingServant.ISettingChangedListener;
import com.mediatek.camera.v2.setting.SettingCtrl;
import com.mediatek.camera.v2.util.SettingKeys;
import com.mediatek.camera.v2.util.Utils;
import com.mediatek.camera.v2.vendortag.TagRequest;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * Face detection view manager which used to manage face data,the manager cares whether it is in
 * face beauty mode(face beauty mode will show a colorful face indicator) and whether it is back or
 * front camera.
 *
 */
public class FdViewManager implements ISettingChangedListener, IFdViewManager {
    private static final Tag TAG = new Tag(FdViewManager.class.getSimpleName());
    private final ISettingServant mISettingServant;
    private ArrayList<String> mCaredSettingChangedKeys = new ArrayList<String>();

    private Activity mActivity;
    private FdView mFdView;
    private boolean mMirror = false;
    private boolean mIsForceFace3aSupported = false;
    private final static String DETECTION_ON = "on";
    /**
     * The constructor of face detection view manager.
     * @param settingServant The current setting instance.
     */
    public FdViewManager(ISettingServant settingServant) {
        mISettingServant = settingServant;
    }

    /**
     * The life cycle corresponding to camera activity onCreate.Some initialization will be take
     * here {@link com.mediatek.camera.v2.detection IDetectionManager #open(Activity, ViewGroup))}.
     *
     * @param activity The camera activity.
     * @param parentViewGroup The parent view group.
     */
    public void open(Activity activity, ViewGroup parentViewGroup) {
        LogHelper.i(TAG, "open");
        mActivity = activity;
        activity.getLayoutInflater().inflate(R.layout.facedetection_view, parentViewGroup, true);
        mFdView = (FdView) parentViewGroup.findViewById(R.id.face_detection_view);
        mFdView.setVisibility(View.VISIBLE);
        addCaredSettingChangedKeys(SettingKeys.KEY_CAMERA_ID);
        mISettingServant.registerSettingChangedListener(this, mCaredSettingChangedKeys,
                ISettingChangedListener.MIDDLE_PRIORITY);
    }

    /**
     * {@link com.mediatek.camera.v2.detection IDetectionManager #close()}.
     */
    public void close() {
        mISettingServant.unRegisterSettingChangedListener(this);
        mIsForceFace3aSupported = false;
    }

    /**
     * {@link com.mediatek.camera.v2.detection IDetectionManager #onOrientationChanged(int)}.
     * @param orientation The current G-sensor orientation.
     */
    public void onOrientationChanged(int orientation) {
        mFdView.onOrientationChanged(orientation);
    }

    /**
     * {@link com.mediatek.camera.v2.detection IDetectionManager #onPreviewAreaChanged(RectF)}.
     * @param previewRect The current G-sensor orientation.
     */
    public void onPreviewAreaChanged(RectF previewRect) {
        mFdView.onPreviewAreaChanged(previewRect);
    }

    /**
     * Whether force face 3a is supported or not.Note that while force face 3a enable, 3A will do
     * face AE/AF and discard scene mode.
     *
     * @return True:Force face 3a is supported.Otherwise,false;
     */
    public boolean isForceFace3aSupported() {
        return mIsForceFace3aSupported;
    }

    @Override
    public void initFaceView() {
        mFdView.setBlockDraw(false);
        updateFaceViewStatus();
    }

    @Override
    public void showFaceView(final int[] ids, final Rect[] rectangles, final byte[] scores,
            final Point[][] pointsInfo, final Rect cropRegion) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mFdView.setFaces(ids, rectangles, scores, pointsInfo, cropRegion);
            }
        });
    }

    @Override
    public void hideFaceView() {
        mFdView.setBlockDraw(true);
        mFdView.clear();
    }

    @Override
    public void onSettingChanged(Map<String, String> result) {
        boolean isFdOpened = DETECTION_ON.equals(mISettingServant
                .getSettingValue(SettingKeys.KEY_CAMERA_FACE_DETECT));
        if (!isFdOpened) {
            return;
        }
        String cameraId = result.get(SettingKeys.KEY_CAMERA_ID);
        if (cameraId != null) {
            updateFaceViewStatus();
        }
    }

    /**
     * The face view indicator drawable and the position depend very closely on the camera
     * id(Whether it is front or back camera) and the camera mode(Face beauty mode will show a
     * colorful indicator).Face view status should be updated either of them has changed.
     */
    private void updateFaceViewStatus() {
        mMirror = SettingCtrl.FRONT_CAMERA.equals(mISettingServant.getCameraId());
        mIsForceFace3aSupported = false;
        CameraCharacteristics characteristics = Utils.getCameraCharacteristics(mActivity,
                mISettingServant.getCameraId());
        List<Key<?>> availableRequestKeys = characteristics.getAvailableCaptureRequestKeys();
        for (Key<?> key : availableRequestKeys) {
            if (key.getName().equals(TagRequest.STATISTICS_FORCE_FACE_3A.getName())) {
                mIsForceFace3aSupported = true;
                break;
            }
        }
        mFdView.clear();
        mFdView.setMirror(mMirror);
    }

    private void addCaredSettingChangedKeys(String key) {
        if (key != null && !mCaredSettingChangedKeys.contains(key)) {
            mCaredSettingChangedKeys.add(key);
        }
    }
}
