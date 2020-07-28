/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.feature.setting.scenemode;

import android.app.Activity;
import android.content.res.TypedArray;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.widget.ImageView;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

/**
 * Scene mode indicator view.
 */

public class SceneModeIndicatorView {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(SceneModeIndicatorView.class.getSimpleName());
    private static final int VIEW_PRIORITY = 7;
    private String[] mOriginalEntryValues;
    private int[] mOriginalIndicator;
    private Activity mActivity;
    private ImageView mIndicatorView;

    private static final int MSG_UPDATE_INDICATOR = 0;
    private Handler mMainHandler;

    /**
     * Scene mode indicator view constructor.
     *
     * @param activity The camera activity.
     */
    public SceneModeIndicatorView(Activity activity) {
        mActivity = activity;
        mMainHandler = new MainHandler(activity.getMainLooper());
        mIndicatorView = (ImageView) activity.getLayoutInflater().inflate(
                R.layout.scene_mode_indicator, null);

        mOriginalEntryValues = activity.getResources()
                .getStringArray(R.array.scene_mode_entryvalues);

        TypedArray array = activity.getResources()
                .obtainTypedArray(R.array.scene_mode_indicators);
        int n = array.length();
        mOriginalIndicator = new int[n];
        for (int i = 0; i < n; ++i) {
            mOriginalIndicator[i] = array.getResourceId(i, 0);
        }
        array.recycle();
    }

    /**
     * Get indicator view.
     *
     * @return The indicator view.
     */
    public ImageView getView() {
        return mIndicatorView;
    }

    /**
     * Get the indicator view priority in showing.
     *
     * @return The indicator view priority.
     */
    public int getViewPriority() {
        return VIEW_PRIORITY;
    }

    /**
     * Update indicator icon.
     *
     * @param scene Current scene mode value.
     */
    public void updateIndicator(String scene) {
        mMainHandler.removeMessages(MSG_UPDATE_INDICATOR);
        mMainHandler.obtainMessage(MSG_UPDATE_INDICATOR, scene).sendToTarget();
    }

    /**
     * Main handler to update scene mode indicator.
     */
    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_UPDATE_INDICATOR:
                    updateIndicator((String) msg.obj);
                    break;

                default:
                    break;
            }
        }

        private void updateIndicator(String scene) {
            LogHelper.d(TAG, "[updateIndicator], scene:" + scene);
            int index = -1;
            for (int i = 0; i < mOriginalEntryValues.length; i++) {
                if (mOriginalEntryValues[i].equals(scene)) {
                    index = i;
                    break;
                }
            }
            if (index <= 1) {
                // index <=1 means scene is off or auto, don't show indicator in
                // these scenes.
                mIndicatorView.setVisibility(View.GONE);
            } else {
                mIndicatorView.setImageDrawable(mActivity
                        .getResources().getDrawable(mOriginalIndicator[index]));
                mIndicatorView.setVisibility(View.VISIBLE);
            }
        }
    }
}
