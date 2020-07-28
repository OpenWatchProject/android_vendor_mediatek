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
package com.android.camera.v2.uimanager;

import android.app.Activity;
import android.content.Intent;
import android.provider.MediaStore;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;

import com.android.camera.R;
import com.android.camera.v2.ui.ShutterButton;
import com.android.camera.v2.ui.ShutterButton.Shutteristener;
import com.android.camera.v2.ui.UiUtil;

import java.util.ArrayList;
import java.util.List;

/**
 *  Shutter manager is used to manage shutter button ui.
 */
public class ShutterManager extends AbstractUiManager {
    private int                           mShutterLayout = R.layout.camera_shutter_photo_video_v2;

    private int                           mPhotoShutterButtonResId;
    private ShutterButton                 mPhotoShutterButton;
    private boolean                       mPhotoShutterEnabled = true;

    private int                           mVideoShutterButtonResId;
    private ShutterButton                 mVideoShutterButton;
    private boolean                       mVideoShutterEnabled = true;

    private View                          mOkButton;
    private View                          mCancelButton;
    private ViewGroup                     mViewGroup;
    private Activity mActivity;
    private OnOkCancelButtonClickListener mOnOkCancelButtonClickListener;

    private List<OnShutterButtonListener> mPhotoListeners
                                    = new ArrayList<OnShutterButtonListener>();
    private Shutteristener                mPhotoShutterButtonListener;

    private List<OnShutterButtonListener> mVideoListeners
                                    = new ArrayList<OnShutterButtonListener>();
    private Shutteristener                mVideoShutterButtonListener;

    private OnClickListener               mOkCancelClickListener = new OnOkCancelClickListener();
    // A callback to be invoked when a ShutterButton's pressed state changes.
    public interface OnShutterButtonListener {
        /**
         * Called when a ShutterButton has been pressed.
         * <p>
         * @param pressed The ShutterButton that was pressed.
         */
        void onFocused(boolean pressed);
        void onPressed();
        void onLongPressed();
    }
    public interface OnOkCancelButtonClickListener {
        public void onOkClick();
        public void onCancelClick();
    }

    public ShutterManager(Activity activity, ViewGroup parent) {
        super(activity, parent);
        setFilterEnable(false);
        mActivity = activity;
        mViewGroup = parent;
        mPhotoShutterButtonListener = new Shutteristener() {
            @Override
            public void onShutterButtonLongPressed() {
                for (OnShutterButtonListener listener : mPhotoListeners) {
                    listener.onLongPressed();
                }
            }
            @Override
            public void onShutterButtonFocus(boolean pressed) {
                for (OnShutterButtonListener listener : mPhotoListeners) {
                    listener.onFocused(pressed);
                }
            }
            @Override
            public void onShutterButtonClick() {
                for (OnShutterButtonListener listener : mPhotoListeners) {
                    listener.onPressed();
                }
            }
        };
        mVideoShutterButtonListener = new Shutteristener() {
            @Override
            public void onShutterButtonLongPressed() {
                for (OnShutterButtonListener listener : mVideoListeners) {
                    listener.onLongPressed();
                }
            }
            @Override
            public void onShutterButtonFocus(boolean pressed) {
                for (OnShutterButtonListener listener : mVideoListeners) {
                    listener.onFocused(pressed);
                }
            }
            @Override
            public void onShutterButtonClick() {
                for (OnShutterButtonListener listener : mVideoListeners) {
                    listener.onPressed();
                }
            }
        };

        Intent intent = activity.getIntent();
        String action = null;
        if (intent != null) {
            action = intent.getAction();
        }
        if (MediaStore.ACTION_IMAGE_CAPTURE.equals(action)) {
            mShutterLayout = R.layout.camera_shutter_photo_v2;
        }

        if (MediaStore.ACTION_VIDEO_CAPTURE.equals(action)) {
            mShutterLayout = R.layout.camera_shutter_video_v2;
        }
    }

    @Override
    protected View getView() {
        View view = null;
        view = inflate(mShutterLayout);
        UiUtil.setOrientation(view, (int) mViewGroup.getTag(), false);
        mPhotoShutterButton = (ShutterButton) view.findViewById(R.id.shutter_button);
        if (mPhotoShutterButton != null) {
            mPhotoShutterButton.setShutterListener(mPhotoShutterButtonListener);
        }

        mVideoShutterButton = (ShutterButton) view.findViewById(R.id.shutter_button_video);
        if (mVideoShutterButton != null) {
            mVideoShutterButton.setShutterListener(mVideoShutterButtonListener);
        }

        mOkButton = view.findViewById(R.id.done_button);
        if (mOkButton != null) {
            mOkButton.setOnClickListener(mOkCancelClickListener);
        }
        mCancelButton = view.findViewById(R.id.btn_cancel);
        if (mCancelButton != null) {
            mCancelButton.setOnClickListener(mOkCancelClickListener);
        }
        return view;
    }

    @Override
    protected void onRefresh() {
        super.onRefresh();
        if (mPhotoShutterButton != null) {
            boolean enable = mPhotoShutterEnabled && isEnable();
            mPhotoShutterButton.setEnabled(enable);
            mPhotoShutterButton.setClickable(enable);
            if (mPhotoShutterButtonResId > 0) {
                mPhotoShutterButton.setImageResource(mPhotoShutterButtonResId);
            }
        }
        if (mVideoShutterButton != null) {
            boolean enable = mVideoShutterEnabled && isEnable();
            mVideoShutterButton.setEnabled(enable);
            mVideoShutterButton.setClickable(enable);
            if (mVideoShutterButtonResId > 0) {
                mVideoShutterButton.setImageResource(mVideoShutterButtonResId);
            }
        }
    }

    @Override
    public void setEnable(boolean enable) {
        super.setEnable(enable);
        refresh(); // in order to onRefresh be called
    }

    public void performShutterButtonClick(boolean videoShutter) {
        if (videoShutter) {
            if (mVideoShutterButton != null) {
                mVideoShutterButton.performClick();
            }
        } else {
            if (mPhotoShutterButton != null) {
                mPhotoShutterButton.performClick();
            }
        }
    }

    public void switchShutterButtonImageResource(int imageResourceId, boolean isVideoButton) {
        if (isVideoButton) {
            mVideoShutterButtonResId = imageResourceId;
        } else {
            mPhotoShutterButtonResId = imageResourceId;
        }
        refresh();
    }

    public void switchShutterButtonLayout(int layoutId) {
        if (mShutterLayout != layoutId) {
            mShutterLayout = layoutId;
            reInflate();
        }
    }

    public boolean isShutterButtonEnabled(boolean videoShutter) {
        if (videoShutter) {
            return mVideoShutterEnabled;
        } else {
            return mPhotoShutterEnabled;
        }
    }

    public void setShutterButtonEnabled(boolean enabled, boolean videoShutter) {
        if (videoShutter) {
            mVideoShutterEnabled = enabled;
        } else {
            mPhotoShutterEnabled = enabled;
        }
        refresh();
    }

    public void setOnOkCancelButtonClickListener(OnOkCancelButtonClickListener listener) {
        mOnOkCancelButtonClickListener = listener;
    }
    /**
     * Add this listener to photo and video shutter button.
     * @param listener
     */
    public void addShutterButtonListener(OnShutterButtonListener listener) {
        if (listener != null && !mVideoListeners.contains(listener)) {
            mVideoListeners.add(listener);
        }
        if (listener != null && !mPhotoListeners.contains(listener)) {
            mPhotoListeners.add(listener);
        }
    }

    /**
     * Remove this listener to from photo and video shutter button.
     * @param listener
     */
    public void removeShutterButtonListener(OnShutterButtonListener listener) {
        if (listener != null && mVideoListeners.contains(listener)) {
            mVideoListeners.remove(listener);
        }
        if (listener != null && mPhotoListeners.contains(listener)) {
            mPhotoListeners.remove(listener);
        }
    }

    public void addShutterButtonListener(OnShutterButtonListener listener, boolean videoShutter) {
        if (videoShutter) {
            if (listener != null && !mVideoListeners.contains(listener)) {
                mVideoListeners.add(listener);
            }
        } else {
            if (listener != null && !mPhotoListeners.contains(listener)) {
                mPhotoListeners.add(listener);
            }
        }
    }

    public void removeShutterButtonListener(OnShutterButtonListener listener,
            boolean videoShutter) {
        if (videoShutter) {
            if (listener != null && mVideoListeners.contains(listener)) {
                mVideoListeners.remove(listener);
            }
        } else {
            if (listener != null && mPhotoListeners.contains(listener)) {
                mPhotoListeners.remove(listener);
            }
        }
    }

    private class OnOkCancelClickListener implements OnClickListener {
        @Override
        public void onClick(View v) {
            if (v == mOkButton) {
                if (mOnOkCancelButtonClickListener != null) {
                    mOnOkCancelButtonClickListener.onOkClick();
                }
            } else if (v == mCancelButton) {
                if (mOnOkCancelButtonClickListener != null) {
                    mOnOkCancelButtonClickListener.onCancelClick();
                }
            }
        }

    }
}
