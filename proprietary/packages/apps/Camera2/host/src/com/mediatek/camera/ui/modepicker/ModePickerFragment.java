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
package com.mediatek.camera.ui.modepicker;


import android.app.Fragment;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import com.mediatek.camera.R;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.CameraUtil;

import java.util.List;


/**
 *  Mode picker fragment, it will show the current support mode list.
 */

public class ModePickerFragment extends Fragment implements IApp.OnOrientationChangeListener  {
    /**
     * Listener to listen mode fragment's state.
     */
    public interface StateListener {
        /**
         * Callback when setting fragment is created.
         */
        void onCreate();

        /**
         * Callback when setting fragment is resumed.
         */
        void onResume();

        /**
         * Callback when setting fragment is paused.
         */
        void onPause();

        /**
         * Callback when setting fragment is destroyed.
         */
        void onDestroy();
    }

    /**
     * Mode selected listener.
     */
    public interface OnModeSelectedListener {
        /**
         * When a mode is selected,notify the event.
         * @param modeInfo @param modeInfo Selected mode info.
         * @return Is the mode change success.
         */
        boolean onModeSelected(ModePickerManager.ModeInfo modeInfo);
    }
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(ModePickerFragment.class.getSimpleName());
    private RecyclerView mRecyclerView;
    private ModeItemAdapter mAdapter;
    private StateListener mStateListener;
    private View.OnClickListener mSettingClickedListener;
    private OnModeSelectedListener mModeSelectedListener;
    private List<ModePickerManager.ModeInfo> mModeList;
    private String mCurrentModeName;
    private int mOrientation;
    private int mSettingVisibility;
    private boolean mIsClickEnabled;


    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {

        View view = inflater.inflate(R.layout.mode_fragment, container, false);
        int compensateOri = CameraUtil.calculateRotateLayoutCompensate(
                ModePickerFragment.this.getActivity());
        CameraUtil.rotateViewOrientation(view, compensateOri, false);
        View settingView = view.findViewById(R.id.setting_view);
        settingView.setVisibility(mSettingVisibility);
        if (CameraUtil.isHasNavigationBar(getActivity())) {
            //get navigation bar height.
            int navigationBarHeight = CameraUtil.getNavigationBarHeight(getActivity());
            //set root view bottom margin to let the UI above the navigation bar.

            RelativeLayout.LayoutParams params =
                    (RelativeLayout.LayoutParams) settingView.getLayoutParams();
            params.bottomMargin += navigationBarHeight;
            settingView.setLayoutParams(params);
        }
        settingView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LogHelper.d(TAG, "onClick: mSettingClickedListener = " + mSettingClickedListener
                        + ", mIsClickEnabled = false");
                if (!mIsClickEnabled) {
                    return;
                }
                if (mSettingClickedListener != null) {
                    ModePickerFragment.this.getActivity()
                      .getFragmentManager().popBackStackImmediate();
                    mSettingClickedListener.onClick(view);
                }
            }
        });
        mRecyclerView = (RecyclerView) view.findViewById(R.id.mode_list);
        mAdapter = new ModeItemAdapter(getActivity(), new OnViewItemClickListenerImpl());
        mAdapter.updateCurrentModeName(mCurrentModeName);
        mAdapter.setModeList(mModeList);
        mRecyclerView.setLayoutManager(new GridLayoutManager(getActivity(), 2));
        mRecyclerView.setAdapter(mAdapter);
        mRecyclerView.addOnChildAttachStateChangeListener(
                new RecyclerView.OnChildAttachStateChangeListener() {
            @Override
            public void onChildViewAttachedToWindow(View view) {
                if (mRecyclerView != null) {
                    CameraUtil.rotateRotateLayoutChildView(getActivity(), mRecyclerView,
                            mOrientation, false);
                }
            }

            @Override
            public void onChildViewDetachedFromWindow(View view) {

            }
        });
        mRecyclerView.addOnItemTouchListener(new RecyclerView.OnItemTouchListener() {
            @Override
            public boolean onInterceptTouchEvent(RecyclerView rv, MotionEvent e) {
                return false;
            }

            @Override
            public void onTouchEvent(RecyclerView rv, MotionEvent e) {

            }

            @Override
            public void onRequestDisallowInterceptTouchEvent(boolean disallowIntercept) {

            }
        });
        return view;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mIsClickEnabled = true;
        if (mStateListener != null) {
            mStateListener.onCreate();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mStateListener != null) {
            mStateListener.onResume();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mStateListener != null) {
            mStateListener.onPause();
        }
    }


    @Override
    public void onDestroy() {
        super.onDestroy();
        mAdapter = null;
        if (mStateListener != null) {
            mStateListener.onDestroy();
        }
    }

    @Override
    public void onOrientationChanged(int orientation) {
        mOrientation = orientation;
        if (mRecyclerView != null && getActivity() != null) {
            CameraUtil.rotateRotateLayoutChildView(getActivity(), mRecyclerView,
                    orientation, true);
        }
    }

    /**
     * Refresh current mode list to be shown in the list.
     * @param modeList Current mode list.
     */
    public void refreshModeList(List<ModePickerManager.ModeInfo> modeList) {
        mModeList = modeList;
        if (mAdapter != null) {
            mAdapter.setModeList(modeList);
        }
    }

    /**
     *  Set fragment state change listener.
     * @param listener State listener.
     */
    public void setStateListener(StateListener listener) {
        mStateListener = listener;
    }

    /**
     * Update current running mode name.
     * @param modeName Current mode name string.
     */

    public void updateCurrentModeName(String modeName) {
        mCurrentModeName  = modeName;
    }

    public void setSettingClickedListener(View.OnClickListener listener) {
        mSettingClickedListener = listener;
    }

    public void setModeSelectedListener(OnModeSelectedListener listener) {
        mModeSelectedListener = listener;
    }

    /**
     * Set the setting icon visible or not.
     *
     * @param visible True means setting icon is visible, otherwise it is invisible.
     */
    public void setSettingIconVisible(boolean visible) {
        mSettingVisibility = visible ? View.VISIBLE : View.GONE;
    }

    /**
     * set the ui whether chickable.
     * @param enabled state.
     */
    public void setEnabled(boolean enabled) {
        mIsClickEnabled = enabled;
    }
    /**
     * OnViewItemClickListener implement.
     */
    private class OnViewItemClickListenerImpl implements ModeItemAdapter.OnViewItemClickListener {

        @Override
        public boolean onItemCLicked(ModePickerManager.ModeInfo modeInfo) {
            ModePickerFragment.this.getActivity().getFragmentManager().popBackStack();
            if (!mIsClickEnabled) {
                return false;
            }
            if (mModeSelectedListener.onModeSelected(modeInfo)) {
                return true;
            } else {
                return false;
            }
        }
    }

}
