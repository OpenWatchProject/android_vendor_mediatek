/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.feature.setting.facedetection;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.View;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.CoordinatesTransform;

/**
 * This is for face view.
 */
@SuppressWarnings("deprecation")
public class FaceView extends View {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(FaceView.class.getSimpleName());
    private Drawable mFaceIndicator;
    private Face[] mFaces;
    private int mDisplayOrientation;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private boolean mMirror;
    // specify after reset mReallyShown as false, whether this face view has been drawn
    private boolean mReallyShown = false;

    /**
     * init the face view.
     *
     * @param context the activity context.
     * @param attrs   view AttributeSet.
     */
    public FaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mFaceIndicator = context.getResources().getDrawable(R.drawable.ic_face_detection_focusing);
    }

    /**
     * Return if this face view has been drawn after reset the really shown status as false
     *
     * @return
     */
    public boolean hasReallyShown() {
        return mReallyShown;
    }

    /**
     * Reset the really shown status as false
     */
    public void resetReallyShown() {
        mReallyShown = false;
    }

    /**
     * Set faces from native for drawing.
     *
     * @param faces detected faces.
     */
    public void setFaces(Face[] faces) {
        mFaces = faces;
        invalidate();
    }

    /**
     * Set display orientation.
     *
     * @param displayOrientation the display orientation.
     * @param cameraId           the camera id.
     */
    public void setDisplayOrientation(int displayOrientation, int cameraId) {
        mDisplayOrientation = displayOrientation;
        mMirror = CameraUtil.isCameraFacingFront(getContext(), cameraId);
    }

    /**
     * Set display orientation.
     *
     * @param width  the width of preview.
     * @param height the height of preview.
     */
    public void updatePreviewSize(int width, int height) {
        LogHelper.d(TAG, "[updatePreviewSize] width = " + width + ", height = " + height);
        mPreviewWidth = width;
        mPreviewHeight = height;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        mReallyShown = true;
        if (mFaces != null && mFaces.length > 0) {
            for (int i = 0; i < mFaces.length; i++) {
                Rect rect = CoordinatesTransform.normalizedPreviewToUi(mFaces[i].rect,
                        mPreviewWidth, mPreviewHeight,
                        mDisplayOrientation, mMirror);
                mFaceIndicator.setBounds(rect.left, rect.top,
                        rect.right, rect.bottom);
                mFaceIndicator.draw(canvas);
            }
        }
        super.onDraw(canvas);
    }
}
