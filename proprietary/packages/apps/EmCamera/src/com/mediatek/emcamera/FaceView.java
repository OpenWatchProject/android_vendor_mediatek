/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.emcamera;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.hardware.Camera.Face;
import android.util.AttributeSet;
import android.view.View;

import com.mediatek.emcamera.R;

public class FaceView extends View implements FocusIndicator {
    private static final String TAG = "FaceView";
    private int mLastFaceNum;

    // For Face Detection
    public static final int FACE_DETECTION_ICON_NUM = 3;
    public static final int FACE_FOCUSING = 0;
    public static final int FACE_FOCUSED = 1;
    public static final int FACE_FOCUSFAILD = 2;

    private Drawable[] mFaceStatusIndicator = new Drawable[FACE_DETECTION_ICON_NUM];
    private static final int[] FACE_DETECTION_ICON = new int[] {
            R.drawable.ic_face_detection_focusing, R.drawable.ic_face_detection_focused,
            R.drawable.ic_face_detection_failed };

    private Face[] mFaces;
    private Drawable mFaceIndicator;
    // The value for android.hardware.Camera.setDisplayOrientation.
    private int mDisplayOrientation;
    // The orientation compensation for the object indicator to make it look
    // correctly in all device orientations. Ex: if the value is 90, the
    // indicator should be rotated 90 degrees counter-clockwise.
    private int mOrientation;
    private int mOrientationCompensation;
    private boolean mMirror;
    private boolean mPause;
    private Matrix mMatrix = new Matrix();
    private RectF mRect = new RectF();
    private int mPreviewWidth;
    private int mPreviewHeight;

    public FaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        for (int i = 0; i < FACE_DETECTION_ICON_NUM; i++) {
            mFaceStatusIndicator[i] = context.getResources().getDrawable(
                    FACE_DETECTION_ICON[i]);
        }
        mFaceIndicator = mFaceStatusIndicator[FACE_FOCUSING];
    }

    public void setFaces(Face[] faces) {
        int num = faces.length;
        if (mPause || (num == 0 && mLastFaceNum == 0)) {
            Elog.d(TAG, "Is Pausing or face num is 0");
            return;
        }
        mFaces = faces;
        mLastFaceNum = num;
        /*if (num > 0 && mFocusIndicatorRotateLayout != null
                && mFocusIndicatorRotateLayout.isFocusing()) {
            mFocusIndicatorRotateLayout.clear();
        }*/
        invalidate();
    }

    public void setDisplayOrientation(int orientation) {
        mDisplayOrientation = orientation;
        Elog.d(TAG, "mDisplayOrientation=" + orientation);
    }

    public void setOrientation(int orientation) {
        mOrientation = orientation;
        invalidate();
    }

    public void setOrientationCompensation(int orientation) {
        mOrientationCompensation = orientation;
        Elog.d(TAG, "mOrientationCompensation=" + orientation);
    }

    public void pause() {
        mPause = true;
    }

    public void resume() {
        mPause = false;
    }

    public void setMirror(boolean mirror) {
        mMirror = mirror;
        Elog.d(TAG, "mMirror=" + mirror);
    }

    public boolean faceExists() {
        return (mFaces != null && mFaces.length > 0);
    }

    @Override
    public void showStart() {
        mFaceIndicator = mFaceStatusIndicator[FACE_FOCUSING];
        invalidate();
    }

    // Ignore the parameter. No autofocus animation for face detection.
    @Override
    public void showSuccess() {
        mFaceIndicator = mFaceStatusIndicator[FACE_FOCUSED];
        invalidate();
    }

    // Ignore the parameter. No autofocus animation for face detection.
    @Override
    public void showFail() {
        mFaceIndicator = mFaceStatusIndicator[FACE_FOCUSFAILD];
        invalidate();
    }

    @Override
    public void clear() {
        // Face indicator is displayed during preview. Do not clear the
        // drawable.
        mFaceIndicator = mFaceStatusIndicator[FACE_FOCUSING];
        mFaces = null;
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        Elog.d(TAG, "onDraw,mFaces = " + mFaces);
        if (mFaces != null && mFaces.length > 0) {
            int rw, rh;
            rw = mPreviewWidth;
            rh = mPreviewHeight;
            if (((rh > rw) && ((mDisplayOrientation == 0) || (mDisplayOrientation == 180)))
                || ((rh < rw) && ((mDisplayOrientation == 90) || (mDisplayOrientation == 270)))) {
                int temp = rw;
                rw = rh;
                rh = temp;
            }
            // Prepare the matrix.
            Util.prepareMatrix(mMatrix, mMirror, mDisplayOrientation, rw, rh);

            float dx = (getWidth() - rw) / 2;
            float dy = (getHeight() - rh) / 2;
            Matrix pointMatrix = new Matrix();
            float[] pointes = new float[] { dx, dy };
            // Focus indicator is directional. Rotate the matrix and the canvas
            // so it looks correctly in all orientations.
            canvas.save();
            mMatrix.postRotate(mOrientation); // postRotate is clockwise
            canvas.rotate(-mOrientation); // rotate is counter-clockwise (for
                                          // canvas)
            pointMatrix.postRotate(mOrientationCompensation);
            pointMatrix.mapPoints(pointes);
            for (int i = 0; i < mFaces.length; i++) {
                // Transform the coordinates.
                mRect.set(mFaces[i].rect);
                Util.dumpRect(mRect, "Original rect");
                mMatrix.mapRect(mRect);
                Util.dumpRect(mRect, "Transformed rect");

                mRect.offset(pointes[0], pointes[1]);

                mFaceIndicator.setBounds(Math.round(mRect.left), Math.round(mRect.top),
                        Math.round(mRect.right), Math.round(mRect.bottom));
                mFaceIndicator.draw(canvas);
            }
            canvas.restore();
        }
        super.onDraw(canvas);
    }

    public void setPreviewSize(int width, int height) {
        mPreviewWidth = width;
        mPreviewHeight = height;
    }
}
