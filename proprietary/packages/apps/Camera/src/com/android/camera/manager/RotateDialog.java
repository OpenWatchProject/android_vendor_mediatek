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

import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.TextView;

import com.android.camera.CameraActivity;
import com.android.camera.Log;
import com.android.camera.R;
import com.android.camera.SettingUtils;
import com.android.camera.ui.RotateLayout;

public class RotateDialog extends ViewManager {
    @SuppressWarnings("unused")
    private static final String TAG = "RotateDialog";

    private RotateLayout mRotateDialog;
    private View mRotateDialogTitleLayout;
    private View mRotateDialogButtonLayout;
    private TextView mRotateDialogTitle;
    private TextView mRotateDialogText;
    private TextView mRotateDialogButton1;
    private TextView mRotateDialogButton2;
    private View mRotateDialogTitleDivider;

    private String mTitle;
    private String mMessage;
    private String mButton1;
    private String mButton2;
    private Runnable mRunnable1;
    private Runnable mRunnable2;

    public RotateDialog(CameraActivity context) {
        super(context, VIEW_LAYER_OVERLAY);
    }

    @Override
    protected View getView() {
        View v = getContext().inflate(R.layout.rotate_dialog, getViewLayer());
        mRotateDialog = (RotateLayout) v.findViewById(R.id.rotate_dialog_layout);
        mRotateDialogTitleLayout = v.findViewById(R.id.rotate_dialog_title_layout);
        mRotateDialogButtonLayout = v.findViewById(R.id.rotate_dialog_button_layout);
        mRotateDialogTitle = (TextView) v.findViewById(R.id.rotate_dialog_title);
        mRotateDialogText = (TextView) v.findViewById(R.id.rotate_dialog_text);
        mRotateDialogButton1 = (Button) v.findViewById(R.id.rotate_dialog_button1);
        mRotateDialogButton2 = (Button) v.findViewById(R.id.rotate_dialog_button2);
        mRotateDialogTitleDivider = (View) v.findViewById(R.id.rotate_dialog_title_divider);
        return v;
    }

    private void resetRotateDialog() {
        // inflateDialogLayout();
        if (mRotateDialogTitleLayout != null) {
            mRotateDialogTitleLayout.setVisibility(View.GONE);
        }
        if (mRotateDialogButton1 != null) {
            mRotateDialogButton1.setVisibility(View.GONE);
        }
        if (mRotateDialogButton2 != null) {
            mRotateDialogButton2.setVisibility(View.GONE);
        }
        if (mRotateDialogButtonLayout != null) {
            mRotateDialogButtonLayout.setVisibility(View.GONE);
        }
    }

    private void resetValues() {
        mTitle = null;
        mMessage = null;
        mButton1 = null;
        mButton2 = null;
        mRunnable1 = null;
        mRunnable2 = null;
    }

    @Override
    protected void onRefresh() {
        resetRotateDialog();
        if (mTitle != null && mRotateDialogTitle != null) {
            mRotateDialogTitle.setTextColor(SettingUtils.getMainColor(getContext()));
            mRotateDialogTitle.setText(mTitle);
            if (mRotateDialogTitleLayout != null) {
                mRotateDialogTitleLayout.setVisibility(View.VISIBLE);
            }
            if (mRotateDialogTitleDivider != null) {
                mRotateDialogTitleDivider.setBackgroundColor(SettingUtils
                        .getMainColor(getContext()));
            }
        }
        if (mRotateDialogText != null) {
            mRotateDialogText.setText(mMessage);
        }
        if (mButton1 != null) {
            mRotateDialogButton1.setText(mButton1);
            mRotateDialogButton1.setContentDescription(mButton1);
            mRotateDialogButton1.setVisibility(View.VISIBLE);
            mRotateDialogButton1.setEnabled(true);
            mRotateDialogButton1.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    mRotateDialogButton1.setEnabled(false);
                    hide();
                    if (mRunnable1 != null) {
                        mRunnable1.run();
                    }
                }
            });
            mRotateDialogButtonLayout.setVisibility(View.VISIBLE);
        }
        if (mButton2 != null) {
            mRotateDialogButton2.setText(mButton2);
            mRotateDialogButton2.setContentDescription(mButton2);
            mRotateDialogButton2.setVisibility(View.VISIBLE);
            mRotateDialogButton2.setEnabled(true);
            mRotateDialogButton2.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    mRotateDialogButton2.setEnabled(false);
                    if (mRunnable2 != null) {
                        mRunnable2.run();
                    }
                    hide();
                }
            });
            mRotateDialogButtonLayout.setVisibility(View.VISIBLE);
        }
        Log.d(TAG, "onRefresh() mTitle=" + mTitle + ", mMessage=" + mMessage + ", mButton1="
                + mButton1 + ", mButton2=" + mButton2 + ", mRunnable1=" + mRunnable1
                + ", mRunnable2=" + mRunnable2);
    }

    public void showAlertDialog(String title, String msg, String button1Text, final Runnable r1,
            String button2Text, final Runnable r2) {
        resetValues();
        mTitle = title;
        mMessage = msg;
        mButton1 = button1Text;
        mButton2 = button2Text;
        mRunnable1 = r1;
        mRunnable2 = r2;
        show();
    }

    @Override
    public boolean collapse(boolean force) {
        if (isShowing()) {
            Log.d(TAG, "[collapse] mRunnable1:" + mRunnable1);
            hide();
            if (mRunnable1 != null) {
                mRunnable1.run();
            }
            return true;
        }
        return super.collapse(force);
    }

    @Override
    protected Animation getFadeInAnimation() {
        return AnimationUtils.loadAnimation(getContext(), R.anim.setting_popup_grow_fade_in);
    }

    @Override
    protected Animation getFadeOutAnimation() {
        return AnimationUtils.loadAnimation(getContext(), R.anim.setting_popup_shrink_fade_out);
    }

    private Animation mDialogFadeIn;
    private Animation mDialogFadeOut;

    protected void fadeIn() {
        if (getShowAnimationEnabled()) {
            if (mDialogFadeIn == null) {
                mDialogFadeIn = getFadeInAnimation();
            }
            if (mDialogFadeIn != null && mRotateDialog != null) {
                mRotateDialog.startAnimation(mDialogFadeIn);
            }
        }
    }

    protected void fadeOut() {
        if (getHideAnimationEnabled()) {
            if (mDialogFadeOut == null) {
                mDialogFadeOut = getFadeOutAnimation();
            }
            if (mDialogFadeOut != null && mRotateDialog != null) {
                mRotateDialog.startAnimation(mDialogFadeOut);
            }
        }
    }
}
