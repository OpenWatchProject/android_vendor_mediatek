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

package com.mediatek.camera.common.thermal;

import android.app.Activity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.widget.RotateLayout;

/**
 * Warning dialog for thermal.
 */
public class WarningDialog {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(WarningDialog.class.getSimpleName());

    private IApp mApp;
    private Activity mActivity;
    private RotateLayout mRoot;
    private TextView mWarningDialogTime;
    private Button mDialogButton;

    /**
     * Constructor of warning dialog.
     * @param app the app instance.
     */
    public WarningDialog(IApp app) {
        mApp = app;
        mActivity = app.getActivity();
    }

    /**
     * Show warning dialog.
     */
    public void show() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                initView();
                if (mRoot.getVisibility() != View.VISIBLE) {
                    mRoot.setVisibility(View.VISIBLE);
                }
            }
        });
    }

    /**
     * Check the warning dialog is showing or not.
     *
     * @return true if showing, or false.
     */
    public boolean isShowing() {
        if (mRoot != null) {
            return mRoot.getVisibility() == View.VISIBLE;
        }
        return false;
    }

    /**
     * Hide warning dialog.
     */
    public void hide() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mRoot.getVisibility() == View.VISIBLE) {
                    mRoot.setVisibility(View.GONE);
                }
            }
        });
    }

    /**
     * release warning dialog view.
     */
    public void uninitView() {
        if (mRoot != null) {
            mApp.unregisterOnOrientationChangeListener(mOnOrientationChangeListener);
            mApp.getAppUi().getModeRootView().removeView(mRoot);
        }
    }

    /**
     * Update count Down time.
     * @param time the count time.
     */
    public void setCountDownTime(String time) {
        if (mWarningDialogTime != null) {
            mWarningDialogTime.setText(time);
        }
    }

    private void initView() {
        if (mRoot == null) {
            int layout = mActivity.getResources().getIdentifier("warning_dialog", "layout",
                    mActivity.getPackageName());

            int time = mActivity.getResources().getIdentifier("alert_dialog_time", "id",
                    mActivity.getPackageName());
            int button = mActivity.getResources().getIdentifier("alert_dialog_button", "id",
                    mActivity.getPackageName());
            mRoot = (RotateLayout) mActivity.getLayoutInflater().inflate(layout, null);
            mApp.getAppUi().getModeRootView().addView(mRoot);
            mWarningDialogTime = (TextView) mRoot.findViewById(time);
            mWarningDialogTime.setText("30");
            mDialogButton = (Button) mRoot.findViewById(button);
            mDialogButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    hide();
                }
            });
            mApp.registerOnOrientationChangeListener(mOnOrientationChangeListener);
        }
    }

    private IApp.OnOrientationChangeListener mOnOrientationChangeListener = new IApp
            .OnOrientationChangeListener() {
        @Override
        public void onOrientationChanged(int orientation) {
            CameraUtil.rotateRotateLayoutChildView(mActivity, mRoot,
                    orientation, true);
        }
    };
}
