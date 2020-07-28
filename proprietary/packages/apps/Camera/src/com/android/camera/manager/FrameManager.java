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
package com.android.camera.manager;

import android.graphics.drawable.Drawable;
import android.hardware.Camera.CameraInfo;
import android.view.View;

import com.android.camera.CameraActivity;
import com.android.camera.CameraHolder;
import com.android.camera.R;
import com.android.camera.ui.FaceView;
import com.android.camera.ui.FrameView;
import com.android.camera.ui.ObjectView;

/**
 * FrameManager manager Frame view, such as faceview, objectview and so on.
 *
 */
public class FrameManager extends ViewManager implements CameraActivity.OnOrientationListener {
    private static final String TAG = "FrameManager";

    public static final int OBJECT_TYPE = 0;
    public static final int FACE_TYPE = 1;

    // For Face Detection
    public static final int FACE_DETECTION_ICON_NUM = 4;
    public static final int FACE_FOCUSING = 0;
    public static final int FACE_FOCUSED = 1;
    public static final int FACE_FOCUSFAILD = 2;
    public static final int FACE_BEAUTY = 3;

    public static final int OBJECT_TRACKING_SUCCEED = 100;
    public static final int OBJECT_TRACKING_FAILED = 50;
    private Drawable[] mFaceStatusIndicator = new Drawable[FACE_DETECTION_ICON_NUM];
    private static final int[] FACE_DETECTION_ICON = new int[] {
            R.drawable.ic_face_detection_focusing, R.drawable.ic_face_detection_focused,
            R.drawable.ic_face_detection_failed, R.drawable.ic_facebeautify_frame };

    // For object tracking
    public static final int OBJECT_TRACKING_ICON_NUM = 3;
    public static final int OBJECT_FOCUSING = 0;
    public static final int OBJECT_FOCUSED = 1;
    public static final int OBJECT_FOCUSFAILED = 2;
    private Drawable[] mTrackStatusIndicator = new Drawable[OBJECT_TRACKING_ICON_NUM];
    private static final int[] OBJECT_TRACKING_ICON = new int[] { R.drawable.ic_object_tracking,
            R.drawable.ic_object_tracking_succeed, R.drawable.ic_object_tracking_failed };

    private boolean mEnableFaceBeauty;
    private CameraActivity mContext;
    private FrameView mFrameView = null;

    public FrameManager(CameraActivity context) {
        super(context);
        context.addOnOrientationListener(this);
        mContext = context;
    }

    @Override
    protected View getView() {
        return null;
    }

    public Drawable[] getViewDrawable(int type) {
        if (type == OBJECT_TYPE) {
            for (int i = 0; i < OBJECT_TRACKING_ICON_NUM; i++) {
                mTrackStatusIndicator[i] = mContext.getResources().getDrawable(
                        OBJECT_TRACKING_ICON[i]);
            }
            return mTrackStatusIndicator;
        } else {
            for (int i = 0; i < FACE_DETECTION_ICON_NUM; i++) {
                mFaceStatusIndicator[i] = mContext.getResources().getDrawable(
                        FACE_DETECTION_ICON[i]);
            }
            return mFaceStatusIndicator;
        }
    }

    // initialize view
    public void initializeFrameView(boolean isOtStated) {
        CameraInfo info = CameraHolder.instance().getCameraInfo()[mContext.getCameraId()];
        if (isOtStated) {
            mFrameView = (ObjectView) mContext.findViewById(R.id.object_view);
        } else {
            mFrameView = (FaceView) mContext.findViewById(R.id.face_view);
            mFrameView.setMirror(info.facing == CameraInfo.CAMERA_FACING_FRONT);
        }
        mFrameView.clear();
        mFrameView.setVisibility(View.VISIBLE);
        mFrameView.setDisplayOrientation(mContext.getDisplayOrientation());
        mFrameView.resume();
        setView(mFrameView);
    }

    public void setView(FrameView view) {
        mFrameView = view;
        enableFaceBeauty(mEnableFaceBeauty);
    }

    public FrameView getFrameView() {
        return mFrameView;
    }

    public void setvFBFacePoints() {
        if (mFrameView != null) {
            mFrameView.setvFBFacePoints();
        }
    }

    public void enableFaceBeauty(boolean enable) {
        mEnableFaceBeauty = enable;
        if (mFrameView != null) {
            mFrameView.enableFaceBeauty(enable);
        }
    }

    @Override
    public void onOrientationChanged(int orientation) {
        if (mFrameView != null) {
            mFrameView.setOrientation(orientation);
        }
    }

}
