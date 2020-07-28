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
import android.os.Handler;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.android.camera.R;
import com.android.camera.v2.ui.UiUtil;

/**
 * A on-screen hint is a view containing a little message for the user and will
 * be shown on the screen continuously. This class helps you create and show
 * those.
 *
 * <p>
 * When the view is shown to the user, appears as a floating view over the
 * application.
 * <p>
 * The easiest way to use this class is to call one of the static methods that
 * constructs everything you need and returns a new {@code OnScreenToast} object.
 */
public class OnScreenToast {

    private static final int                        TOAST_DURATION = 2000; // milliseconds

    private View                                    mToastView;
    private TextView                                mMessageView;
    private final Handler                           mHandler = new Handler();

    public OnScreenToast(Activity activity, ViewGroup group) {
        LayoutInflater inflate = activity.getLayoutInflater();
        mToastView = inflate.inflate(R.layout.onscreen_mode_toast_v2, group, false);
        mMessageView = (TextView) mToastView.findViewById(R.id.message);
        group.addView(mToastView);
    }

    public void showToast(CharSequence text) {
        mMessageView.setText(text);
        mHandler.removeCallbacks(mShow);
        mHandler.removeCallbacks(mHide);
        mHandler.post(mShow);
    }

    public void hideToast() {
        mHandler.removeCallbacks(mShow);
        mHandler.removeCallbacks(mHide);
        mHandler.post(mHide);
    }


    private final Runnable mShow = new Runnable() {
        @Override
        public void run() {
            UiUtil.fadeIn(mToastView);
            mHandler.postDelayed(mHide, TOAST_DURATION);

        }
    };

    private final Runnable mHide = new Runnable() {
        @Override
        public void run() {
            UiUtil.fadeOut(mToastView);
        }
    };
}
