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

package com.android.camera.ui;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.hardware.Camera.Face;
import android.util.AttributeSet;
import android.widget.RelativeLayout;

import com.android.camera.CameraActivity;
import com.android.camera.Log;
import com.android.camera.Util;
import com.android.camera.manager.FrameManager;

public class ObjectView extends FrameView {
    private static final String TAG = "ObjectView";

    private Runnable mStartAction = new StartAction();
    private Runnable mEndAction = new EndAction();
    private static final int ANIMATION_IDLE = 0;
    private static final int ANIMATION_DOING = 1;
    private static final int ANIMATION_DONE = 2;
    private static final int SCALING_UP_TIME = 300;
    private static final int SCALING_DOWN_TIME = 200;

    private int mZoomInAnimaState = ANIMATION_IDLE;
    private int mZoomOutAnimaState = ANIMATION_IDLE;

    private float mOldX = 2000;
    private float mOldY = 2000;

    public ObjectView(Context context, AttributeSet attr) {
        super(context, attr);
        mTrackIndicator = mTrackStatusIndicator[FrameManager.OBJECT_FOCUSING];
        mContext = (CameraActivity) context;
    }

    public void setObject(Face face) {
        Log.i(TAG, "setObject(), mZoomInAnimaState:" + mZoomInAnimaState + ""
                + ", mZoomOutAnimaState:" + mZoomOutAnimaState);
        mFace = face;
        if (mZoomInAnimaState == ANIMATION_DOING) {
            return;
        }
        if (mZoomOutAnimaState == ANIMATION_IDLE) {
            if (face.score == 100) {
                showSuccess(true);
            } else if (face.score == 50) {
                showFail(true);
            }
        } else if (mZoomOutAnimaState == ANIMATION_DONE && !mPause) {
            resetView();
            invalidate();
        }
    }

    public boolean faceExists() {
        return mFace != null;
    }

    @Override
    public void showStart() {
        Log.i(TAG, "showStart()");
        mZoomInAnimaState = ANIMATION_DOING;
        mZoomOutAnimaState = ANIMATION_IDLE;
        mTrackIndicator = mTrackStatusIndicator[FrameManager.OBJECT_FOCUSING];
        setBackground(mTrackIndicator);
        animate().withLayer().setDuration(SCALING_UP_TIME).scaleX(1.5f).scaleY(1.5f)
                .withEndAction(mStartAction);
    }

    @Override
    public void showSuccess(boolean timeout) {
        Log.i(TAG, "showSuccess()");
        mZoomOutAnimaState = ANIMATION_DOING;
        mTrackIndicator = mTrackStatusIndicator[FrameManager.OBJECT_FOCUSED];
        setBackground(mTrackIndicator);
        animate().withLayer().setDuration(SCALING_DOWN_TIME).scaleX(0.8f).scaleY(0.8f)
                .withEndAction(mEndAction);
    }

    @Override
    public void showFail(boolean timeout) {
        mZoomOutAnimaState = ANIMATION_DOING;
        mTrackIndicator = mTrackStatusIndicator[FrameManager.OBJECT_FOCUSFAILED];
        setBackground(mTrackIndicator);
        animate().withLayer().setDuration(SCALING_DOWN_TIME).scaleX(0.8f).scaleY(0.8f)
                .withEndAction(mEndAction);
    }

    @Override
    public void clear() {
        Log.i(TAG, "clear()");
        mFace = null;
        resetView();
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        Log.d(TAG, "onDraw(), mZoomInAnimaState:" + mZoomInAnimaState + ", mZoomOutAnimaState:"
                + mZoomOutAnimaState + ", mOrientation:" + mOrientation);
        if (mZoomOutAnimaState != ANIMATION_DONE || mZoomInAnimaState != ANIMATION_DONE) {
            return;
        }
        if (mFace != null) {
            Log.i(TAG, "mFace:" + mFace);
            if (mFace.score == 100) {
                mTrackIndicator = mTrackStatusIndicator[FrameManager.OBJECT_FOCUSED];
            } else {
                return;
            }
            int rw, rh;
            rw = mContext.getUnCropWidth();
            rh = mContext.getUnCropHeight();
            if (((rh > rw) && ((mDisplayOrientation == 0) || (mDisplayOrientation == 180)))
                    || ((rh < rw) && ((mDisplayOrientation == 90)
                            || (mDisplayOrientation == 270)))) {
                int temp = rw;
                rw = rh;
                rh = temp;
            }
            // Prepare the matrix.
            Util.prepareMatrix(mMatrix, false, mDisplayOrientation, rw, rh);
            // OT indicator is directional. Rotate the matrix and the canvas
            // so it looks correctly in all orientations.
            float dx = (float) (getWidth() - rw) / 2;
            float dy = (float) (getHeight() - rh) / 2;
            Matrix pointMatrix = new Matrix();
            float[] pointes = new float[] { dx, dy };
            canvas.save();
            mMatrix.postRotate(mOrientation);
            canvas.rotate(-mOrientation);
            pointMatrix.postRotate(mContext.getOrientationCompensation());
            pointMatrix.mapPoints(pointes);
            // Transform the coordinates.
            mRect.set(mFace.rect);
            // remember the last coordinator
            mOldX = calculateMiddlePoint(mRect.left, mRect.right);
            mOldY = calculateMiddlePoint(mRect.top, mRect.bottom);
            Util.dumpRect(mRect, "Original rect");
            mMatrix.mapRect(mRect);
            Util.dumpRect(mRect, "Transformed rect");
            mRect.offset(pointes[0], pointes[1]);

            mTrackIndicator.setBounds(Math.round(mRect.left), Math.round(mRect.top),
                    Math.round(mRect.right), Math.round(mRect.bottom));

            mTrackIndicator.draw(canvas);
            canvas.restore();
        }
        super.onDraw(canvas);
    }

    private class StartAction implements Runnable {
        @Override
        public void run() {
            mZoomInAnimaState = ANIMATION_DONE;
        }
    }

    private class EndAction implements Runnable {
        @Override
        public void run() {
            mZoomOutAnimaState = ANIMATION_DONE;
            resetView();
        }
    }

    private void resetView() {
        RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) getLayoutParams();
        setBackground(null);
        animate().cancel();
        setScaleX(1f);
        setScaleY(1f);
        params.width = mContext.getPreviewFrameWidth();
        params.height = mContext.getPreviewFrameHeight();
        params.setMargins(0, 0, 0, 0);
    }

    public void enableFaceBeauty(boolean enable) {
        mEnableBeauty = false;
    }

    public void setvFBFacePoints() {

    }

    private float calculateMiddlePoint(float x, float y) {
        return x + (y - x) / 2;
    }

    public float getPointX() {
        return mOldX;
    }

    public float getPointY() {
        return mOldY;
    }
}
