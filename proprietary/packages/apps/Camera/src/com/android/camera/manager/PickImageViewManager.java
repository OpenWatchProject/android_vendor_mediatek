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

import android.content.Context;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.media.ExifInterface;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;

import com.android.camera.CameraActivity;
import com.android.camera.Log;
import com.android.camera.R;
import com.android.camera.SaveRequest;
import com.android.camera.Util;
import com.android.camera.ui.EVPickerItem;

import java.io.IOException;

public class PickImageViewManager extends ViewManager {
    private static final String TAG = "PickImageViewManager";

    private View mRootView;
    private int mFrameWidth;
    private int mFrameHeight;
    private boolean mS3DMode = false;
    /** package */
    static final boolean ANIMATION = true;

    private int mDisplayOrientaion;
    private int mDisplayRotation;

    private boolean mNeedInitialize = true;

    private static final int HEIGHT_PADDING = 40;
    private static final int WIDTH_PADDING = 200;

    private EVPickerItem mEv0;
    private EVPickerItem mEvp;
    private EVPickerItem mEvm;
    private EVPickerItem[] mEvPickers;

    private SaveRequest[] mSaveRequests;
    private int mPictures2Pick = 0;
    private SelectedChangedListener mSelectedChangedListener;
    private double mAspectRatio;

    public interface SelectedChangedListener {
        void onSelectedChanged(boolean selected);
    }

    private OnClickListener mOnClickListener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            int id = v.getId();
            Log.i(TAG, "onClick id=" + id);
            switch (id) {
            case R.id.checkBoxEv0:
            case R.id.checkBoxEvPlus:
            case R.id.checkBoxEvMinus:
                    if (mPictures2Pick == 1
                            && ((EVPickerItem) mRootView.findViewById(id)).isSelected()) {
                        /* disable the other selected item */
                        final int[] evIds = { R.id.checkBoxEv0, R.id.checkBoxEvPlus,
                                R.id.checkBoxEvMinus };
                        EVPickerItem p;
                        for (int i : evIds) {
                            p = (EVPickerItem) mRootView.findViewById(i);
                            if (i != id && p.isSelected()) {
                                p.performClick();
                            }
                        }
                    }
                // if any on the ev key is checked.enable done key.
                mSelectedChangedListener.onSelectedChanged(isAnyImgSelected());
                break;
            default:
                break;
            }
        }
    };

    public boolean isSelected(int id) {
        return mEvPickers[id].isSelected();
    }

    public PickImageViewManager(CameraActivity context, int pictures2Pick) {
        super(context, ViewManager.VIEW_LAYER_TOP);
        mPictures2Pick = pictures2Pick;
    }

    public void show() {
        super.show();
        if (mNeedInitialize) {
            initializeViewManager();
            configLayoutOrientation();
            displayImages();
            mNeedInitialize = false;
        }
    }

    /**
     * will be called if app want to show current view which hasn't been
     * created.
     *
     * @return
     */
    @Override
    protected View getView() {
        View view = inflate(R.layout.select_view);
        mRootView = view.findViewById(R.id.selectroot);
        return view;
    }

    private void initializeViewManager() {
        mEv0 = (EVPickerItem) mRootView.findViewById(R.id.checkBoxEv0);
        mEvp = (EVPickerItem) mRootView.findViewById(R.id.checkBoxEvPlus);
        mEvm = (EVPickerItem) mRootView.findViewById(R.id.checkBoxEvMinus);
        mEvPickers = new EVPickerItem[] { mEvm, mEv0, mEvp };

        mEv0.setOnClickListener(mOnClickListener);
        mEvp.setOnClickListener(mOnClickListener);
        mEvm.setOnClickListener(mOnClickListener);

        mDisplayOrientaion = getContext().getDisplayOrientation();
        mDisplayRotation = getContext().getDisplayRotation();

        WindowManager windowManager = (WindowManager) getContext().getSystemService(
                Context.WINDOW_SERVICE);

        int tmpHeight = windowManager.getDefaultDisplay().getHeight();
        int tmpWidth = windowManager.getDefaultDisplay().getWidth();
        mFrameWidth = Math.max(tmpHeight, tmpWidth);
        mFrameHeight = Math.min(tmpHeight, tmpWidth);
    }

    /**
     * will be called when app call release() to unload views from view
     * hierarchy.
     */
    protected void onRelease() {
        mNeedInitialize = true;
    }

    private void configLayoutOrientation() {
        Log.i(TAG, "configLayoutOrientation RequestedOrientation="
                + getContext().getRequestedOrientation());
        if (getContext().getRequestedOrientation() == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT) {
            super.onOrientationChanged(270);
        }
    }

    public void setSelectedChangedListener(SelectedChangedListener selectedChangedListener) {
        mSelectedChangedListener = selectedChangedListener;
    }

    public void setAspectRatio(double aspectRatio) {
        mAspectRatio = aspectRatio;
    }

    private boolean isAnyImgSelected() {
        return (mEv0.isSelected() || mEvp.isSelected() || mEvm.isSelected());
    }

    public boolean displayImages() {
        Log.d(TAG, "displayImages mSaveRequests=" + mSaveRequests);
        if (mSaveRequests == null) {
            return false;
        }

        int thumbHeight = mFrameHeight / 2 - HEIGHT_PADDING;
        int thumbWidth = (int) (thumbHeight * mAspectRatio);
        float bmpWidth;
        float bmpHeight;

        int maxWidth = mFrameWidth - WIDTH_PADDING; // width of control
        if (thumbWidth > maxWidth) {

            thumbWidth = maxWidth;
            thumbHeight = (int) (thumbWidth / mAspectRatio);
        }

        // create bitmaps
        int[] widgetId = { R.id.checkBoxEvMinus, R.id.checkBoxEv0, R.id.checkBoxEvPlus };

        EVPickerItem p;
        Bitmap bmp;
        Matrix matrix = null;
        String tmpPath = null;
        int orientation = 0;
        for (int i = 0; i < mSaveRequests.length; i++) {
            tmpPath = mSaveRequests[i].getTempFilePath();
            p = (EVPickerItem) mRootView.findViewById(widgetId[i]);
            // set thumbnail unchecked
            if (p.isSelected()) {
                p.performClick();
            }

            bmp = Util.makeBitmap(tmpPath, thumbWidth, thumbWidth * thumbHeight);

            if (bmp == null) {
                Log.d(TAG, "File is gone or damaged:" + mSaveRequests[i].getTempFilePath());
                return false;
            }
            try {
                ExifInterface exif = new ExifInterface(tmpPath);
                orientation = Util.getExifOrientation(exif);
            } catch (IOException ex) {
                Log.e(TAG, "cannot read exif", ex);
                return false;
            }
            orientation = mSaveRequests[i].getJpegRotation()
                    - orientation
                    + Util.getGapOrientation(getContext().getDisplayRotation(), getContext()
                            .getCameraId());
            bmpWidth = Math.max((float) bmp.getWidth(), (float) bmp.getHeight());
            bmpHeight = Math.min((float) bmp.getWidth(), (float) bmp.getHeight());

            if (matrix == null) {
                matrix = new Matrix();
                matrix.postRotate(-orientation);
                int requestdOrientation = getContext().getRequestedOrientation();
                if (requestdOrientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT) {
                    matrix.postScale(((float) thumbHeight) / bmpHeight, ((float) thumbWidth)
                            / bmpWidth);
                } else {
                    matrix.postScale(((float) thumbWidth) / bmpWidth, ((float) thumbHeight)
                            / bmpHeight);
                }
            }
            bmp = Bitmap.createBitmap(bmp, 0, 0, bmp.getWidth(), bmp.getHeight(), matrix, true);
            p.setImageBitmap(bmp);

            // set EV0 as chosen by default.
            if (widgetId[i] == R.id.checkBoxEv0) {
                p.performClick();
            }

            Log.d(TAG,
                    i + " orientation=" + orientation + " bmpWidth=" + bmpWidth + " bmpHeight="
                            + bmpHeight + " mSaveRequests[i].getJpegRotation()="
                            + mSaveRequests[i].getJpegRotation() + " thumb: " + thumbWidth + "x"
                            + thumbHeight + " bmp=" + bmp.getWidth() + "x" + bmp.getHeight()
                            + " mAspectRatio=" + mAspectRatio);
        }
        return true;
    }

    public void setSaveRequests(SaveRequest[] saveRequests) {
        mSaveRequests = saveRequests;
    }
}
