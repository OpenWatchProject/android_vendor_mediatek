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
package com.android.camera.v2.app;

import android.view.MotionEvent;
import android.view.View;

import com.android.camera.CameraActivity;
import com.android.camera.v2.util.CameraUtil;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;


/**
 * An gesture manager received gesture events .
 */
public class GestureManagerImpl extends GestureManager {
    private static final Tag TAG = new Tag(GestureManagerImpl.class.getSimpleName());
    private final AppController      mAppController;
    private final CameraActivity     mCameraActivity;
    private MyListener               mLocalGestureListener = new MyListener();
    private GestureRecognizer        mGestureRecoginzer;

    public GestureManagerImpl(AppController app) {
        mAppController = app;
        mCameraActivity = (CameraActivity) app.getActivity();
        mGestureRecoginzer = new GestureRecognizer(mCameraActivity, mLocalGestureListener);
        mAppController.getPreviewManager().setGestureListener(mTouchListener);
    }
    @Override
    public void onOrientationChanged(int orientation) {
    }

    private View.OnTouchListener mTouchListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            LogHelper.i(TAG, "Gesture ontouch");
            mGestureRecoginzer.onTouchEvent(event);
            return false;
        }
    };

    private float[] convertPortraitDistanceByOrientation(float dx, float dy) {
        float[] distance = new float[]{dx, dy};
        // match preview frame rect
        switch ((mGsensorOrientation + CameraUtil.getDisplayRotation(mCameraActivity)) % 360) {
        case 0:
            break;
        case 90:
            float temp = dx;
            dx = dy;
            dy = -temp;
            break;
        case 180:
            dx = -dx;
            dy = -dy;
            break;
        case 270:
            float temp2 = dx;
            dx = -dy;
            dy = temp2;
            break;
        }
        LogHelper.i(TAG, "display rotation:" + CameraUtil.getDisplayRotation(mCameraActivity) +
                " orientation :" + mGsensorOrientation);
        distance[0] = dx;
        distance[1] = dy;
        return distance;
    }

    private class MyListener implements GestureRecognizer.Listener {
        @Override
        public void onDown(float x, float y) {
            LogHelper.i(TAG, "onDown x:" + x + ",y:" + y);
            boolean interceptEvent = mGestureNotifier.onDown(x, y);
        }

        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
            boolean interceptEvent = mGestureNotifier.onFling(e1, e2, velocityX, velocityY);
            return false;
        }

        @Override
        public boolean onScroll(float dx, float dy, float totalX, float totalY) {
            LogHelper.i(TAG, "onScroll (dx,dy)" + "(" + dx + "," + dy + ")" +
                    " totalX = " + totalX + " totalY = " + totalY);
            //TODO what about for totalX and totalY?
            float[] distance = convertPortraitDistanceByOrientation(dx, dy);
            boolean interceptEvent = mGestureNotifier.onScroll(distance[0], distance[1],
                    totalX, totalY);
            return false;
        }

        @Override
        public boolean onSingleTapUp(float x, float y) {
            boolean interceptEvent = mGestureNotifier.onSingleTapUp(x, y);
            return false;
        }

        @Override
        public boolean onSingleTapConfirmed(float x, float y) {
            boolean interceptEvent = mGestureNotifier.onSingleTapConfirmed(x, y);
            return false;
        }

        @Override
        public void onUp() {
            boolean interceptEvent = mGestureNotifier.onUp();
        }

        @Override
        public boolean onDoubleTap(float x, float y) {
            mGestureNotifier.onDoubleTap(x, y);
            return true;
        }

        @Override
        public boolean onScale(float focusX, float focusY, float scale) {
            mGestureNotifier.onScale(focusX, focusY, scale);
            return true;
        }

        @Override
        public boolean onScaleBegin(float focusX, float focusY) {
            mGestureNotifier.onScaleBegin(focusX, focusY);
            return true;
        }

        @Override
        public void onScaleEnd() {
        }

        @Override
        public void onLongPress(float x, float y) {
            boolean interceptEvent = mGestureNotifier.onLongPress(x, y);
        }
    }
}