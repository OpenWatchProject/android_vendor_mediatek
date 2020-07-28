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
package com.android.camera.ui;

import com.android.camera.manager.FrameManager;
import com.android.camera.CameraActivity;
import com.android.camera.Log;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.hardware.Camera.Face;
import android.util.AttributeSet;
import android.view.View;

/**
 * The boxView base class, because the face view and object view are
 * incompatible, so we can define common.
 *
 */
public class FrameView extends View implements FocusIndicator {
    private static final String TAG = "BoxView";

    private static final int INIT_OT_POINT = 2000;
    // For FD
    protected Face[] mFaces;
    protected Drawable mFaceIndicator;
    protected Drawable[] mFaceStatusIndicator;
    // For OT
    protected Face mFace;
    protected Drawable mTrackIndicator;
    protected Drawable[] mTrackStatusIndicator;
    // Common
    protected CameraActivity mContext;
    protected boolean mEnableBeauty;
    // The value for android.hardware.Camera.setDisplayOrientation.
    protected int mDisplayOrientation;
    // The orientation compensation for the object indicator to make it look
    // correctly in all device orientations. Ex: if the value is 90, the
    // indicator should be rotated 90 degrees counter-clockwise.
    protected int mOrientation;
    protected boolean mMirror;
    protected boolean mPause;
    protected Matrix mMatrix = new Matrix();
    protected RectF mRect = new RectF();

    public FrameView(Context context, AttributeSet attr) {
        super(context, attr);
        mContext = (CameraActivity) context;
        mFaceStatusIndicator = mContext.getFrameManager().getViewDrawable(FrameManager.FACE_TYPE);
        mTrackStatusIndicator = mContext.getFrameManager()
                .getViewDrawable(FrameManager.OBJECT_TYPE);
    }

    public FrameView(Context context) {
        super(context);
    }

    public void setDisplayOrientation(int orientation) {
        mDisplayOrientation = orientation;
        Log.d(TAG, "mDisplayOrientation=" + orientation);
    }

    public void pause() {
        mPause = true;
    }

    public void resume() {
        mPause = false;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
    }

    public boolean faceExists() {
        return false;
    }

    @Override
    public void showStart() {
    }

    @Override
    public void showSuccess(boolean timeout) {
    }

    @Override
    public void showFail(boolean timeout) {
    }

    @Override
    public void needDistanceInfoShow(boolean needShow) {
    }

    @Override
    public void clear() {
    }

    public void setMirror(boolean mirror) {
        mMirror = mirror;
    }

    public void setFaces(Face[] faces) {
        mFaces = faces;
    }

    public void setObject(Face face) {
        mFace = face;
    }

    public void enableFaceBeauty(boolean enable) {
    }

    public void setvFBFacePoints() {
    };

    // public void release() {}

    public void setOrientation(int orientation) {
        mOrientation = orientation;
        invalidate();
    }

    public float getPointX() {
        return INIT_OT_POINT;
    }

    public float getPointY() {
        return INIT_OT_POINT;
    }
}
