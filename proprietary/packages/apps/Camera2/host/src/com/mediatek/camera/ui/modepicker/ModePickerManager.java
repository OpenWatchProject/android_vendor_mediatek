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

import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.provider.MediaStore;
import android.view.View;
import android.view.ViewGroup;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.widget.RotateImageView;
import com.mediatek.camera.ui.AbstractViewManager;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentSkipListMap;

/**
 * A manager for mode list entry view.
 */
public class ModePickerManager extends AbstractViewManager {

    /**
     * Mode change listener, when a mode is selected, notify the new mode.
     */
    public interface OnModeChangedListener {
        /**
         * Notify the new mode info.
         * @param modeName The selected mode item.
         */
        void onModeChanged(String modeName);
    }

    /**
     * Mode info for mode picker fragment.
     */
    public class ModeInfo {
        public String mName;
        public Drawable mUnselectedIcon;
        public Drawable mSelectedIcon;
        public String[] mSupportedCameraIds;
    }

    private static final LogUtil.Tag TAG = new LogUtil.Tag(ModePickerManager.class.getSimpleName());
    private final IApp mApp;
    private static final String mModeFragmentText = "ModePickerFragment";
    private RotateImageView mModePickerView;
    private ModePickerFragment mFragment;
    private OnModeChangedListener mModeChangeListener;

    private ModeProvider mModeProvider;
    private String mCurrentCameraId = "0";
    private String mCurrentModeName;
    private View.OnClickListener mSettingClickedListener;

    private boolean mIsSettingIconVisible;
    /**
     * constructor of ModePickerManager.
     * @param app The {@link IApp} implementer.
     * @param parentView the root view of ui.
     */
    public ModePickerManager(IApp app, ViewGroup parentView) {
        super(app, parentView);
        mApp = app;
    }


    @Override
    protected View getView() {
        mModePickerView = (RotateImageView) mParentView
                .findViewById(R.id.mode);

        return mModePickerView;
    }

    @Override
    public void onPause() {
        super.onPause();
        hideModePickerFragment();
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        if (mFragment != null) {
            mFragment.setEnabled(enabled);
        }
    }

    /**
     * Register a mode item.
     * @param provider Mode provider, provider the mode information.
     */
    public void registerModeProvider(ModeProvider provider) {
        LogHelper.d(TAG, "registerModeProvider ");
        mModeProvider = provider;
    }

    /**
     * The mode has changed, notify the event to change mode.
     * @param newMode The new mode.
     */
    public void modeChanged(String newMode) {

    }

    /**
     * When preview started, notify the event to camera app UI.
     * @param previewCameraId current preview camera id.
     */
    public void onPreviewStarted(final String previewCameraId) {
        LogHelper.d(TAG, "onPreviewStarted  previewCameraId " + previewCameraId);
        mCurrentCameraId = previewCameraId;

        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mFragment != null) {
                    mFragment.refreshModeList(createModePickerList(previewCameraId));
                }
            }
        });


    }

    /**
     * Update current running mode info.
     * @param item Mode info.
     */

    public void updateCurrentModeItem(final IAppUi.ModeItem item) {
        mCurrentModeName = item.mModeName;

        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                updateModePickerView(item);
            }
        });
    }

    /**
     * Set mode change listener that used to notify mode change event.
     * @param listener The listener instance.
     */
    public void setModeChangeListener(OnModeChangedListener listener) {
        mModeChangeListener = listener;
    }

    public void setSettingClickedListener(View.OnClickListener listener) {
        mSettingClickedListener = listener;
    }

    /**
     * Set the setting icon visible or not.
     *
     * @param visible True means setting icon is visible, otherwise it is invisible.
     */
    public void setSettingIconVisible(boolean visible) {
        mIsSettingIconVisible = visible;
    }

    /**
     * Fragment state listener implementer.
     */
    private class FragmentStateListener implements ModePickerFragment.StateListener {

        @Override
        public void onCreate() {
            ViewGroup root = (ViewGroup) mApp.getActivity()
                    .findViewById(R.id.app_ui_root);
            root.setVisibility(View.GONE);
            mApp.getActivity()
                    .findViewById(R.id.preview_layout_container).setVisibility(View.GONE);
            mApp.registerOnOrientationChangeListener(mFragment);
            mApp.getAppUi().setUIEnabled(mApp.getAppUi().SHUTTER_BUTTON, false);
        }

        @Override
        public void onResume() {

        }

        @Override
        public void onPause() {

        }

        @Override
        public void onDestroy() {
            ViewGroup root = (ViewGroup) mApp.getActivity()
                    .findViewById(R.id.app_ui_root);
            root.setVisibility(View.VISIBLE);
            mApp.getActivity()
                    .findViewById(R.id.preview_layout_container).setVisibility(View.VISIBLE);
            mApp.unregisterOnOrientationChangeListener(mFragment);
            mApp.getAppUi().setUIEnabled(mApp.getAppUi().SHUTTER_BUTTON, true);
            mFragment = null;
        }
    }

    private List<ModeInfo> createModePickerList(String deviceId) {
        Map<String, IAppUi.ModeItem> modes = mModeProvider.getModes2();
        ConcurrentSkipListMap<Integer, ModeInfo> modeInfos
                = new ConcurrentSkipListMap<>();
        int facing = getCameraFacingById(deviceId);
        ModeInfo info = null;
        for (IAppUi.ModeItem item : modes.values()) {
            for (int j = 0; j < item.mSupportedCameraIds.length; j++) {
                if (getCameraFacingById(item.mSupportedCameraIds[j]) == facing) {
                    LogHelper.d(TAG, "find one mode = " + item.mModeName);
                    info = new ModeInfo();
                    info.mName = item.mModeName;
                    info.mSelectedIcon = item.mModeSelectedIcon;
                    info.mUnselectedIcon = item.mModeUnselectedIcon;
                    info.mSupportedCameraIds = item.mSupportedCameraIds;
                    modeInfos.put(item.mPriority, info);
                    break;
                }
            }
        }
        return new ArrayList(modeInfos.values());
    }

    private int getCameraFacingById(String deviceId) {
        CameraManager cameraManager = (CameraManager)
                mApp.getActivity().getSystemService(Context.CAMERA_SERVICE);
        try {
            CameraCharacteristics characteristics =
                    cameraManager.getCameraCharacteristics(deviceId);
            return characteristics.get(CameraCharacteristics.LENS_FACING);
        } catch (CameraAccessException e) {
            LogHelper.e(TAG, "[getCameraFacingById] CameraAccessException", e);
            return -1;
        } catch (IllegalArgumentException e) {
            LogHelper.e(TAG, "[getCameraFacingById] IllegalArgumentException", e);
            return -1;
        }
    }

    /**
     * The OnModeSelectedListener implement.
     */
    private class OnModeSelectedListenerImpl implements ModePickerFragment.OnModeSelectedListener {

        @Override
        public boolean onModeSelected(ModeInfo modeInfo) {
            if (modeInfo == null) {
                return false;
            }
            if (!isCameraIDSupported(modeInfo.mSupportedCameraIds, mCurrentCameraId)) {
                return false;
            }

            if (modeInfo.mSelectedIcon != null) {
                mModePickerView.setImageDrawable(modeInfo.mSelectedIcon);
            } else {
                mModePickerView.setImageResource(R.drawable.ic_normal_mode_selected);
            }

            mCurrentModeName = modeInfo.mName;

            mModeChangeListener.onModeChanged(mCurrentModeName);
            return true;
        }
    }

    private void updateModePickerView(IAppUi.ModeItem item) {

        String action = mApp.getActivity().getIntent().getAction();
        if (!(MediaStore.ACTION_IMAGE_CAPTURE.equals(action)
                || MediaStore.ACTION_VIDEO_CAPTURE.equals(action))) {

            if (item != null) {
                if (item.mModeSelectedIcon != null) {
                    mModePickerView.setImageDrawable(item.mModeSelectedIcon);
                } else {
                    mModePickerView.setImageResource(R.drawable.ic_normal_mode_selected);
                }
            }
            mModePickerView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    if (mFragment != null) {
                        return;
                    }
                    FragmentManager manager = mApp.getActivity().getFragmentManager();
                    FragmentTransaction transaction = manager.beginTransaction();

                    mFragment = new ModePickerFragment();
                    mFragment.setStateListener(new FragmentStateListener());
                    mFragment.setModeSelectedListener(new OnModeSelectedListenerImpl());
                    mFragment.setSettingIconVisible(mIsSettingIconVisible);
                    if (mIsSettingIconVisible) {
                        mFragment.setSettingClickedListener(mSettingClickedListener);
                    }
                    mFragment.updateCurrentModeName(mCurrentModeName);
                    mFragment.refreshModeList(createModePickerList(mCurrentCameraId));
                    transaction.replace(R.id.activity_root, mFragment, mModeFragmentText);
                    transaction.addToBackStack(mModeFragmentText);
                    transaction.commitAllowingStateLoss();
                }
            });
        } else {
            mModePickerView.setImageResource(R.drawable.ic_setting);
            mModePickerView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    if (mSettingClickedListener != null) {
                        mSettingClickedListener.onClick(view);
                    }
                }
            });
        }
    }

    private void hideModePickerFragment() {
        mApp.getActivity().getFragmentManager().popBackStackImmediate(mModeFragmentText,
                FragmentManager.POP_BACK_STACK_INCLUSIVE);
    }

    private boolean isCameraIDSupported(String[] modeSupportedIDs, String currentID) {
        LogHelper.d(TAG, "isCameraIDSupported [] = " + modeSupportedIDs.toString());
        for (String id : modeSupportedIDs) {
            if (getCameraFacingById(id) == getCameraFacingById(currentID)) {
                return true;
            }
        }
        return false;
    }
}