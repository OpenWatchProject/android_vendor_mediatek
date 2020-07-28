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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.mediatek.camera.ui;

import android.os.Bundle;
import android.preference.PreferenceFragment;
import android.support.v7.widget.Toolbar;
import android.view.View;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSettingView;

import java.util.ArrayList;
import java.util.List;

/**
 * Provide setting UI for camera.
 */
public class SettingFragment extends PreferenceFragment {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(SettingFragment.class.getSimpleName());

    private List<ICameraSettingView> mSettingViewList = new ArrayList<>();
    private StateListener mStateListener;
    private Toolbar mToolbar;

    /**
     * Listener to listen setting fragment's state.
     */
    public interface StateListener {
        /**
         * Callback when setting fragment is created.
         */
        public void onCreate();

        /**
         * Callback when setting fragment is resumed.
         */
        public void onResume();

        /**
         * Callback when setting fragment is paused.
         */
        public void onPause();

        /**
         * Callback when setting fragment is destroyed.
         */
        public void onDestroy();
    }

    public void setStateListener(StateListener listener) {
        mStateListener = listener;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        LogHelper.d(TAG, "[onCreate]");
        if (mStateListener != null) {
            mStateListener.onCreate();
        }
        super.onCreate(savedInstanceState);

        mToolbar = (Toolbar) getActivity().findViewById(R.id.toolbar);
        if (mToolbar != null) {
            mToolbar.setTitle(getActivity().getResources().getString(R.string.setting_title));
            mToolbar.setTitleTextColor(
                    getActivity().getResources().getColor(android.R.color.white));
            mToolbar.setNavigationIcon(
                    getActivity().getResources().getDrawable(R.drawable.ic_setting_up));
            mToolbar.setNavigationOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    LogHelper.i(TAG, "[onClick], activity:" + getActivity());
                    if (getActivity() != null) {
                        getActivity().getFragmentManager().popBackStack();
                    }
                }
            });
        }


        addPreferencesFromResource(R.xml.camera_preferences);

        synchronized (this) {
            for (ICameraSettingView view : mSettingViewList) {
                view.loadView(this);
            }
        }
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        LogHelper.d(TAG, "[onActivityCreated]");
        super.onActivityCreated(savedInstanceState);
    }

    @Override
    public void onResume() {
        LogHelper.d(TAG, "[onResume]");
        super.onResume();
        if (mToolbar != null) {
            mToolbar.setTitle(getActivity().getResources().getString(R.string.setting_title));
        }
        synchronized (this) {
            for (ICameraSettingView view : mSettingViewList) {
                view.refreshView();
            }
        }
        if (mStateListener != null) {
            mStateListener.onResume();
        }
    }

    @Override
    public void onPause() {
        LogHelper.d(TAG, "[onPause]");
        super.onPause();
        if (mStateListener != null) {
            mStateListener.onPause();
        }
    }

    @Override
    public void onDestroy() {
        LogHelper.d(TAG, "[onDestroy]");
        super.onDestroy();
        synchronized (this) {
            for (ICameraSettingView view : mSettingViewList) {
                view.unloadView();
            }
        }
        if (mStateListener != null) {
            mStateListener.onDestroy();
        }
    }

    /**
     * Add setting view instance to setting view list.
     *
     * @param view The instance of {@link ICameraSettingView}.
     */
    public synchronized void addSettingView(ICameraSettingView view) {
        if (view == null) {
            LogHelper.w(TAG, "[addSettingView], view:" + view, new Throwable());
            return;
        }
        if (!mSettingViewList.contains(view)) {
            mSettingViewList.add(view);
        }
    }

    /**
     * Remove setting view instance from setting view list.
     *
     * @param view The instance of {@link ICameraSettingView}.
     */
    public synchronized void removeSettingView(ICameraSettingView view) {
        mSettingViewList.remove(view);
    }

    /**
     * Refresh setting view.
     */
    public synchronized void refreshSettingView() {
        for (ICameraSettingView view : mSettingViewList) {
            view.refreshView();
        }
    }

    /**
     * Whether setting view tree has any visible child or not. True means it has at least
     * one visible child, false means it don't has any visible child.
     *
     * @return False if setting view tree don't has any visible child.
     */
    public synchronized boolean hasVisibleChild() {
        if (ICameraSettingView.JUST_DISABLE_UI_WHEN_NOT_SELECTABLE) {
            return mSettingViewList.size() > 0;
        }

        boolean visible = false;
        for (ICameraSettingView view : mSettingViewList) {
            if (view.isEnabled()) {
                visible = true;
            }
        }
        return visible;
    }
}
