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
import android.app.Service;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.hardware.Camera.Face;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.View;
import android.view.WindowManager;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.util.Utils;

/**
 * Face view used to show face indicator.
 */
public class FdView extends View {
    private static final Tag TAG = new Tag(FdView.class.getSimpleName());
    private static final boolean DEBUG = false;
    private final Context mContext;
    private boolean mMirror = false;

    private int mDisplaycompensation = Utils.ROTATION_0;
    private int mLastFaceNum;

    private FdUtil mFaceDetectionUtil;
    private Face[] mFaces;
    private Drawable mFaceIndicator;
    private Drawable[] mFaceStatusIndicator;

    private Point mPreviewBeginingPoint = new Point();
    private int mCropRegionLeft;
    private int mCropRegionTop;
    private int mCropRegionWidth;
    private int mCropRegionHeight;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private int mBufferWidth;
    private int mBufferHeight;
    private int mBufferCenterX;
    private int mBufferCenterY;

    private int mDisplayRotation = Utils.ROTATION_0;
    private final static int BEAUTY_FACE_SCORE = 100;
    private boolean mIsFbEnabled = false;
    private volatile boolean mBlocked;

    private RectF mRect = new RectF();
    private Matrix mMatrix = new Matrix();

    /**
     * Constructor that is called when inflating a view from XML. This is called when a view is
     * being constructed from an XML file, supplying attributes that were specified in the XML file.
     * This version uses a default style of 0, so the only attribute values applied are those in the
     * Context's Theme and the given AttributeSet.
     *
     * @param context The Context the view is running in, through which it can access the current
     * theme, resources, etc.
     * @param set The attributes of the XML tag that is inflating the view.
     */
    public FdView(Context context, AttributeSet set) {
        super(context, set);
        mContext = context;
        mFaceDetectionUtil = new FdUtil((Activity) context);
        mFaceStatusIndicator = mFaceDetectionUtil.getViewDrawable();
        mFaceIndicator = mFaceStatusIndicator[0];
    }

    /**
     * Called when preview area changed.
     * @param previewRect Preview area.
     */
    protected void onPreviewAreaChanged(RectF previewRect) {
        mPreviewBeginingPoint.x = Math.round(previewRect.left);
        mPreviewBeginingPoint.y = Math.round(previewRect.top);
        mBufferWidth = Math.round(previewRect.width());
        mBufferHeight = Math.round(previewRect.height());
        mPreviewWidth = Math.round(mBufferWidth + mPreviewBeginingPoint.x * 2);
        mPreviewHeight = Math.round(mBufferHeight + mPreviewBeginingPoint.y * 2);
        mBufferCenterX = mPreviewBeginingPoint.x + mBufferWidth / 2;
        mBufferCenterY = mPreviewBeginingPoint.y + mBufferHeight / 2;
        if (DEBUG) {
            LogHelper.d(TAG, "onPreviewAreaChanged,previewRect = " + previewRect.toShortString()
                    + "(mPreviewWidth ,mPreviewHeight) = (" + mPreviewWidth + " ," + mPreviewHeight
                    + "), buffer center is (" + mBufferCenterX + " ," + mBufferCenterY + " )");
        }
    }

    protected void onOrientationChanged(int orientation) {
        updateDisplayRotation(mContext);
    }

    protected void setMirror(boolean mirror) {
        mMirror = mirror;
    }

    protected void setFbEnabled(boolean fbEnabled) {
        mIsFbEnabled = fbEnabled;
    }

    protected void setFaces(int[] ids, Rect[] rectangles, byte[] scores, Point[][] pointsInfo,
            Rect cropRegion) {
        int length = 0;
        if (scores != null) {
            length = scores.length;
        }
        Face[] faces = new Face[length];
        mCropRegionLeft = cropRegion.left;
        mCropRegionTop = cropRegion.top;
        mCropRegionWidth = cropRegion.width();
        mCropRegionHeight = cropRegion.height();
        // convert the API2 Face to API 1 Face
        // landmark current not use,but the value may be null when FD mode
        // is Simple
        if (scores != null && pointsInfo != null) {
            for (int i = 0; i < length; i++) {
                Face tempFace = new Face();
                if (pointsInfo[i][0] != null) {
                    tempFace.leftEye = pointsInfo[i][0];
                }
                if (pointsInfo[i][1] != null) {
                    tempFace.rightEye = pointsInfo[i][1];
                }
                if (pointsInfo[i][2] != null) {
                    tempFace.mouth = pointsInfo[i][2];
                }

                if (rectangles[i] != null) {
                    tempFace.rect = rectangles[i];
                }
                tempFace.score = scores[i];
                faces[i] = tempFace;
            }
        }
        faceDetected(faces);
    }

    protected void setBlockDraw(boolean block) {
        mBlocked = block;
    }

    protected void clear() {
        mFaces = null;
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (!mBlocked && (mFaces != null) && (mFaces.length > 0)) {
            int width = mBufferWidth > mBufferHeight ? mBufferWidth : mBufferHeight;
            int height = mBufferWidth > mBufferHeight ? mBufferHeight : mBufferWidth;
            int translateWidthValue = 0;
            int translateHeightValue = 0;
            float previewRatio = (float) width / (float) height;
            float cropRegionRatio = (float) mCropRegionWidth / (float) mCropRegionHeight;
            float faceRatio = 0f;
            if (DEBUG) {
                LogHelper.d(TAG, "[onDraw] width = " + width
                        + ", height = " + height + ",mMirror = "
                        + mMirror + ",mDisplayOrientation = " + mDisplaycompensation
                        + ",previewRatio = " + previewRatio + ",cropRegionRatio = "
                        + cropRegionRatio);
            }
            if (previewRatio > cropRegionRatio) {
                faceRatio = (float) mCropRegionWidth / (float) width;
                translateHeightValue = Math.round((mCropRegionHeight - height * faceRatio) / 2);
            } else {
                faceRatio = (float) mCropRegionHeight / (float) height;
                translateWidthValue = Math.round((mCropRegionWidth - width * faceRatio) / 2);
            }
            for (int i = 0; i < mFaces.length; i++) {
                updateIndicator(mFaces[i].score == BEAUTY_FACE_SCORE);
                mRect.set(mFaces[i].rect);
                mMatrix.reset();
                // translate the face to the buffer coordinate
                mMatrix.postTranslate(-mCropRegionLeft, -mCropRegionTop);
                mMatrix.postTranslate(-translateWidthValue, -translateHeightValue);
                // scale the face indicator
                mMatrix.postScale(1 / faceRatio, 1 / faceRatio);
                mMatrix.mapRect(mRect);
                rotateFacePosition();
                mirrorFacePosition();
                mFaceIndicator.setBounds(mFaceDetectionUtil.rectFToRect(mRect));
                mFaceIndicator.draw(canvas);
            }
            canvas.save();
            canvas.restore();
        }
        super.onDraw(canvas);
    }

    private void faceDetected(Face[] faces) {
        mFaces = faces;
        if (faces != null) {
            int num = mFaces.length;
            if (DEBUG) {
                LogHelper.d(TAG, "faceDetected num of face = " + num + ",mLastFaceNum = "
                        + mLastFaceNum);
            }
            if (0 == num && 0 == mLastFaceNum) {
                return;
            }
            mLastFaceNum = num;
        }
        invalidate();
    }

    private void updateDisplayRotation(Context context) {
        WindowManager windowManager = (WindowManager) context
                .getSystemService(Service.WINDOW_SERVICE);
        int rotation = windowManager.getDefaultDisplay().getRotation();
        switch (rotation) {
            case Surface.ROTATION_0:
                mDisplayRotation = Utils.ROTATION_0;
                break;
            case Surface.ROTATION_90:
                mDisplayRotation = Utils.ROTATION_90;
                break;
            case Surface.ROTATION_180:
                mDisplayRotation = Utils.ROTATION_180;
                break;
            case Surface.ROTATION_270:
                mDisplayRotation = Utils.ROTATION_270;
                break;

            default:
                break;
        }
        mDisplaycompensation = mDisplayRotation;
    }

    /**
     * Rotate the position so it looks correctly in all orientations.
     */
    private void rotateFacePosition() {
        float rectWidth = mRect.right - mRect.left;
        float rectHeight = mRect.bottom - mRect.top;
        if (mDisplaycompensation == Utils.ROTATION_0) {
            float temp = mRect.left;
            mRect.left = mPreviewWidth - mRect.bottom;
            mRect.top = temp;
            mRect.right = mRect.left + rectHeight;
            mRect.bottom = mRect.top + rectWidth;
            mRect.offset(mPreviewBeginingPoint.x, mPreviewBeginingPoint.y);
        } else if (mDisplaycompensation == Utils.ROTATION_180) {
            float temp = mRect.top;
            mRect.left = temp;
            mRect.top = mPreviewHeight - mRect.right;
            mRect.right = mRect.left + rectHeight;
            mRect.bottom = mRect.top + rectWidth;
            mRect.offset(mPreviewBeginingPoint.x, mPreviewBeginingPoint.y);
        } else if (mDisplaycompensation == Utils.ROTATION_270) {
            mRect.left = mPreviewWidth - mRect.right;
            mRect.top = mPreviewHeight - mRect.bottom;
            mRect.right = mRect.left + rectWidth;
            mRect.bottom = mRect.top + rectHeight;
            mRect.offset(-mPreviewBeginingPoint.x, -mPreviewBeginingPoint.y);
        } else if (mDisplaycompensation == Utils.ROTATION_90) {
            mRect.offset(mPreviewBeginingPoint.x, mPreviewBeginingPoint.y);
        }
    }

    /**
     * Rotate the face position by the buffer center.
     */
    private void mirrorFacePosition() {
        if (mMirror) {
            float rectWidth = mRect.right - mRect.left;
            float rectHeight = mRect.bottom - mRect.top;
            if (mDisplaycompensation == Utils.ROTATION_90
                    || mDisplaycompensation == Utils.ROTATION_270) {
                mRect.left = mRect.right + 2 * (mBufferCenterX - mRect.right);
                mRect.right = mRect.left + rectWidth;
            } else if (mDisplaycompensation == Utils.ROTATION_0
                    || mDisplaycompensation == Utils.ROTATION_180) {
                mRect.top = mRect.bottom + 2 * (mBufferCenterY - mRect.bottom);
                mRect.bottom = mRect.top + rectHeight;
            }
        }
    }

    private void updateIndicator(boolean isBeautyFace) {
        if (mIsFbEnabled && isBeautyFace) {
            mFaceIndicator = mFaceStatusIndicator[mFaceStatusIndicator.length - 1];
            if (DEBUG) {
                LogHelper.d(TAG, "updateIndicator," +
                        " face beauty mode will show a colorful indicator");
            }
        } else {
            mFaceIndicator = mFaceStatusIndicator[0];
        }
    }
}
