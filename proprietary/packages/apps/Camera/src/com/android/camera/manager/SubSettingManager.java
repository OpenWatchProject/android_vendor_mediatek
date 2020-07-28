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

import android.content.res.Configuration;
import android.view.View;
import android.view.ViewGroup;

import com.android.camera.CameraActivity;
import com.android.camera.Log;
import com.android.camera.R;
import com.android.camera.Util;
import com.android.camera.ui.RotateImageView;
import com.android.camera.ui.SubSettingLayout;

import com.mediatek.camera.platform.ICameraAppUi.ViewState;
import com.mediatek.camera.setting.SettingConstants;

public class SubSettingManager extends SettingManager {
    private static final String TAG = "SubSettingManager";

    public SubSettingManager(CameraActivity context) {
        super(context);
    }

    @Override
    protected View getView() {
        // TODO Auto-generated method stub
        View view = inflate(R.layout.sub_setting_indicator);
        mIndicator = (RotateImageView) view.findViewById(R.id.sub_setting_indicator);
        mIndicator.setOnClickListener(this);
        return view;
    }

    @Override
    public void onClick(View v) {
        // TODO Auto-generated method stub
        if (v == mIndicator) {
            if (!mShowingContainer) {
                showSetting();
            }
        }
    }

    @Override
    public void onRefresh() {

        if (mShowingContainer) { // just apply checker when showing settings
            //getContext().getSettingChecker().applyParametersToUI();
            notifyDataSetChanged();
        }
    }

    @Override
    public boolean collapse(boolean force) {
        boolean collapsechild = false;
        if (mShowingContainer && mPageView != null) {
            mPageView.collapseChild();
            hideSetting();
            collapsechild = true;
        }
        Log.v(TAG, "collapse(" + force + ") mShowingContainer=" + mShowingContainer + ", return "
                + collapsechild);
        return collapsechild;
    }

    @Override
    public void onPreferenceReady() {
        releaseSettingResource();
    }

    @Override
    public void onOrientationChanged(int orientation) {
        super.superOrientationChanged(orientation);
        Util.setOrientation(mPageView, getOrientation(), true);
        Util.setOrientation(mIndicator, getIndicatorOrientation(), true);
    }

    @Override
    public void show() {
        super.show();
        Util.setOrientation(mIndicator, getIndicatorOrientation(), false);
    }

    // Fix indicator direction
    private int getIndicatorOrientation() {
        int orientation = getContext().getResources().getConfiguration().orientation;

        return (orientation == Configuration.ORIENTATION_LANDSCAPE) ? 0 : 270;
    }

    public void showSetting() {
        // TODO Auto-generated method stub
        Log.d(TAG, "showSetting... start");

        if (getContext().isFullScreen()) {
            if (!mShowingContainer && getContext().getCameraAppUI().isNormalViewState()) {
                mMainHandler.removeMessages(MSG_REMOVE_SETTING);
                mShowingContainer = true;
                initializeSettings();
                refresh();
                mSettingLayout.setVisibility(View.VISIBLE);
                if (mSettingLayout.getParent() == null) {
                    Log.i("LeiLei", "showSetting getContext() = " + getContext());
                    getContext().addView(mSettingLayout, SETTING_PAGE_LAYER);
                }
                getContext().getCameraAppUI().setViewState(ViewState.VIEW_STATE_SUB_SETTING);
                startFadeInAnimation(mSettingLayout);
                mIndicator.setVisibility(View.GONE);
            }
            setChildrenClickable(true);
            Log.d(TAG, "showSetting... end");
        }
    }

    private SubSettingLayout mPageView;

    private void initializeSettings() {
        if (mSettingLayout == null && mSettingController.getPreferenceGroup() != null) {
            mSettingLayout = (ViewGroup) getContext().inflate(R.layout.sub_setting_container,
                    SETTING_PAGE_LAYER);
            // new page view
            mPageView = (SubSettingLayout) mSettingLayout.findViewById(R.id.sub_pager);
            mPageView.initialize(getSettinKeys(SettingConstants.SETTING_GROUP_SUB_COMMON), true);

        }
        Util.setOrientation(mPageView, getOrientation(), false);
    }

    private String[] getSettinKeys(int[] keys) {
        String[] keyStrings = new String[keys.length];
        for (int i = 0; i < keys.length; i++) {
            String key = SettingConstants.getSettingKey(keys[i]);
            keyStrings[i] = key;
        }
        return keyStrings;
    }

    public void hideSetting() {
        Log.v(TAG, "hideSetting() mShowingContainer=" + mShowingContainer + ", mSettingLayout="
                + mSettingLayout);

        if (mShowingContainer && mSettingLayout != null) {
            mMainHandler.removeMessages(MSG_REMOVE_SETTING);
            startFadeOutAnimation(mSettingLayout);
            mSettingLayout.setVisibility(View.GONE);
            mShowingContainer = false;
            if (getContext().getCameraAppUI().getViewState() == ViewState.VIEW_STATE_SUB_SETTING) {
                getContext().getCameraAppUI().restoreViewState();
            }
            mMainHandler.sendEmptyMessageDelayed(MSG_REMOVE_SETTING, DELAY_MSG_REMOVE_SETTING_MS);
        }
        setChildrenClickable(false);
        mIndicator.setVisibility(View.VISIBLE);
    }

    public void notifyDataSetChanged() {
        mPageView.setSettingChangedListener(SubSettingManager.this);
        mPageView.reloadPreference();
    }
}
