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
package com.mediatek.camera.feature.setting.flash;

import android.app.Activity;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.widget.RotateImageView;


/**
 * This class manages the looks of the flash and flash mode choice view.
 */
public class FlashViewController {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(FlashViewController.class.getSimpleName());

    private static final int FLASH_ENTRY_LIST_SWITCH_SIZE = 2;
    private static final int FLASH_ENTRY_LIST_INDEX_0 = 0;
    private static final int FLASH_ENTRY_LIST_INDEX_1 = 1;
    private static final int FLASH_PRIORITY = 30;
    private static final int FLASH_SHUTTER_PRIORITY = 70;

    private static final String FLASH_AUTO_VALUE = "auto";
    private static final String FLASH_OFF_VALUE = "off";
    private static final String FLASH_ON_VALUE = "on";

    private static final int FLASH_VIEW_INIT = 0;
    private static final int FLASH_VIEW_ADD_QUICK_SWITCH = 1;
    private static final int FLASH_VIEW_REMOVE_QUICK_SWITCH = 2;
    private static final int FLASH_VIEW_HIDE_CHOICE_VIEW = 3;
    private static final int FLASH_VIEW_UPDATE_QUICK_SWITCH_ICON = 4;

    private ImageView mFlashEntryView;
    private ImageView mFlashIndicatorView;
    private ImageView mFlashOffIcon;
    private ImageView mFlashAutoIcon;
    private ImageView mFlashOnIcon;
    private View mFlashChoiceView;
    private View mOptionLayout;
    private final Flash mFlash;
    private final IApp mApp;
    private MainHandler mMainHandler;

    /**
     * Constructor of flash view.
     * @param flash Flash instance.
     * @param app   The application app level controller.
     */
    public FlashViewController(Flash flash, IApp app) {
        mFlash = flash;
        mApp = app;
        mMainHandler = new MainHandler(app.getActivity().getMainLooper());
        mMainHandler.sendEmptyMessage(FLASH_VIEW_INIT);
    }

    /**
     * add flash switch to quick switch.
     */
    public void addQuickSwitchIcon() {

        mMainHandler.sendEmptyMessage(FLASH_VIEW_ADD_QUICK_SWITCH);
    }

    /**
     * remove qiuck switch icon.
     */
    public void removeQuickSwitchIcon() {

        mMainHandler.sendEmptyMessage(FLASH_VIEW_REMOVE_QUICK_SWITCH);
    }

    /**
     * for overrides value, for set visibility.
     * @param isShow true means show.
     */
    public void showQuickSwitchIcon(boolean isShow) {
        mMainHandler.obtainMessage(FLASH_VIEW_UPDATE_QUICK_SWITCH_ICON, isShow).sendToTarget();
    }

    /**
     * close option menu.
     */
    public void hideFlashChoiceView() {
        mMainHandler.sendEmptyMessage(FLASH_VIEW_HIDE_CHOICE_VIEW);
    }

    // [Add for CCT tool] Receive keycode and enable/disable ZSD @{
    protected IApp.KeyEventListener getKeyEventListener() {
        return new IApp.KeyEventListener() {
            @Override
            public boolean onKeyDown(int keyCode, KeyEvent event) {
                if ((keyCode != CameraUtil.KEYCODE_SET_FLASH_ON
                        && keyCode != CameraUtil.KEYCODE_SET_FLASH_OFF)
                        || !CameraUtil.isSpecialKeyCodeEnabled()) {
                    return false;
                }
                return true;
            }

            @Override
            public boolean onKeyUp(int keyCode, KeyEvent event) {
                if (!CameraUtil.isSpecialKeyCodeEnabled()) {
                    return false;
                }
                if (keyCode != CameraUtil.KEYCODE_SET_FLASH_ON
                        && keyCode != CameraUtil.KEYCODE_SET_FLASH_OFF) {
                    return false;
                }
                if (mFlashEntryView == null) {
                    LogHelper.e(TAG, "[onKeyUp] mFlashEntryView is null");
                    return false;
                }

                if (keyCode == CameraUtil.KEYCODE_SET_FLASH_ON) {
                    LogHelper.i(TAG, "[onKeyUp] update flash on");
                    updateFlashEntryView(FLASH_ON_VALUE);
                    mFlash.onFlashValueChanged(FLASH_ON_VALUE);
                } else if (keyCode == CameraUtil.KEYCODE_SET_FLASH_OFF) {
                    LogHelper.i(TAG, "[onKeyUp] update flash off");
                    updateFlashEntryView(FLASH_OFF_VALUE);
                    mFlash.onFlashValueChanged(FLASH_OFF_VALUE);
                }
                return true;
            }
        };
    }
    // @}

    /**
     * Handler let some task execute in main thread.
     */
    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            LogHelper.d(TAG, "view handleMessage: " + msg.what);
            switch (msg.what) {
                case FLASH_VIEW_INIT:
                    mFlashEntryView = initFlashEntryView();
                    break;

                case FLASH_VIEW_ADD_QUICK_SWITCH:
                    mApp.getAppUi().addToQuickSwitcher(mFlashEntryView, FLASH_PRIORITY);
                    updateFlashEntryView(mFlash.getValue());
                    mApp.getAppUi().registerOnShutterButtonListener(mShutterListener,
                            FLASH_SHUTTER_PRIORITY);
                    break;

                case FLASH_VIEW_REMOVE_QUICK_SWITCH:
                    mApp.getAppUi().removeFromQuickSwitcher(mFlashEntryView);
                    //updateFlashIndicator(false);
                    mApp.getAppUi().unregisterOnShutterButtonListener(mShutterListener);
                    break;

                case FLASH_VIEW_UPDATE_QUICK_SWITCH_ICON:
                    if ((boolean) msg.obj) {
                        mFlashEntryView.setVisibility(View.VISIBLE);
                        updateFlashEntryView(mFlash.getValue());
                    } else {
                        mFlashEntryView.setVisibility(View.GONE);
                    }
                    break;

                case FLASH_VIEW_HIDE_CHOICE_VIEW:
                    if (mFlashChoiceView != null && mFlashChoiceView.isShown()) {
                        mApp.getAppUi().hideQuickSwitcherOption();
                        updateFlashEntryView(mFlash.getValue());
                        // Flash indicator no need to show now,would be enable later
                        // updateFlashIndicator(mFlash.getValue());
                    }
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * Update ui by the value.
     * @param value the value to change.
     *
     */
    private void updateFlashEntryView(final String value) {
        LogHelper.d(TAG, "[updateFlashView] currentValue = " + mFlash.getValue());
        if (FLASH_ON_VALUE.equals(value)) {
            mFlashEntryView.setImageResource(R.drawable.ic_flash_status_on);
            mFlashEntryView.setContentDescription(mApp.getActivity().getResources().getString(
                    R.string.accessibility_flash_on));
        } else if (FLASH_AUTO_VALUE.equals(value)) {
            mFlashEntryView.setImageResource(R.drawable.ic_flash_status_auto);
            mFlashEntryView.setContentDescription(mApp.getActivity().getResources().getString(
                    R.string.accessibility_flash_auto));
        } else {
            mFlashEntryView.setImageResource(R.drawable.ic_flash_status_off);
            mFlashEntryView.setContentDescription(mApp.getActivity().getResources().getString(
                    R.string.accessibility_flash_off));
        }
        // Flash indicator no need to show now,would be enable later
        // updateFlashIndicator(value);
    }

    /**
     * Initialize the flash view which will add to quick switcher.
     * @return the view add to quick switcher
     */
    private ImageView initFlashEntryView() {
        Activity activity = mApp.getActivity();
        RotateImageView view = (RotateImageView) activity.getLayoutInflater().inflate(
                R.layout.flash_icon, null);
        view.setOnClickListener(mFlashEntryListener);
        mFlashIndicatorView = (RotateImageView) activity.getLayoutInflater().inflate(
                R.layout.flash_indicator, null);
        return view;
    }

    /**
     * This listener used to monitor the flash quick switch icon click item.
     */
    private final View.OnClickListener mFlashEntryListener = new View.OnClickListener() {
        public void onClick(View view) {
            if (mFlash.getEntryValues().size() <= 1) {
                return;
            }
            if (mFlash.getEntryValues().size() > FLASH_ENTRY_LIST_SWITCH_SIZE) {
                initializeFlashChoiceView();
                updateChoiceView();
                mApp.getAppUi().showQuickSwitcherOption(mOptionLayout);
            } else {
                String value = mFlash.getEntryValues().get(FLASH_ENTRY_LIST_INDEX_0);
                if (value.equals(mFlash.getValue())) {
                    value = mFlash.getEntryValues().get(FLASH_ENTRY_LIST_INDEX_1);
                }
                updateFlashEntryView(value);
                // Flash indicator no need to show now,would be enable later
                // updateFlashIndicator(value);
                mFlash.onFlashValueChanged(value);
            }
        }
    };

    private View.OnClickListener mFlashChoiceViewListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            String value = "";
            if (mFlashAutoIcon == view) {
                value = FLASH_AUTO_VALUE;
            } else if (mFlashOnIcon == view) {
                value = FLASH_ON_VALUE;
            } else {
                value = FLASH_OFF_VALUE;
            }
            mApp.getAppUi().hideQuickSwitcherOption();
            updateFlashEntryView(value);
            // Flash indicator no need to show now,would be enable later
            // updateFlashIndicator(value);
            mFlash.onFlashValueChanged(value);
        }

    };

    private void updateFlashIndicator(final boolean value) {
        if (value) {
            mApp.getAppUi().addToIndicatorView(mFlashIndicatorView, FLASH_PRIORITY);
        } else {
            mApp.getAppUi().removeFromIndicatorView(mFlashIndicatorView);
        }
    }

    /**
     * This function used to high light the current choice for.
     * flash if flash choice view is show.
     */
    private void updateChoiceView() {
        if (FLASH_ON_VALUE.equals(mFlash.getValue())) {
            mFlashOnIcon.setImageResource(R.drawable.ic_flash_status_on_selected);
            mFlashOffIcon.setImageResource(R.drawable.ic_flash_status_off);
            mFlashAutoIcon.setImageResource(R.drawable.ic_flash_status_auto);
        } else if (FLASH_OFF_VALUE.equals(mFlash.getValue())) {
            mFlashOnIcon.setImageResource(R.drawable.ic_flash_status_on);
            mFlashOffIcon.setImageResource(R.drawable.ic_flash_status_off_selected);
            mFlashAutoIcon.setImageResource(R.drawable.ic_flash_status_auto);
        } else {
            mFlashOnIcon.setImageResource(R.drawable.ic_flash_status_on);
            mFlashOffIcon.setImageResource(R.drawable.ic_flash_status_off);
            mFlashAutoIcon.setImageResource(R.drawable.ic_flash_status_auto_selected);
        }
    }

    private void initializeFlashChoiceView() {
        if (mFlashChoiceView == null || mOptionLayout == null) {
            ViewGroup viewGroup =  mApp.getAppUi().getModeRootView();
            mOptionLayout = mApp.getActivity().getLayoutInflater().inflate(
                    R.layout.flash_option, viewGroup, false);
            mFlashChoiceView = mOptionLayout.findViewById(R.id.flash_choice);
            mFlashOnIcon = (ImageView) mOptionLayout.findViewById(R.id.flash_on);
            mFlashOffIcon = (ImageView) mOptionLayout.findViewById(R.id.flash_off);
            mFlashAutoIcon = (ImageView) mOptionLayout.findViewById(R.id.flash_auto);
            mFlashOffIcon.setOnClickListener(mFlashChoiceViewListener);
            mFlashOnIcon.setOnClickListener(mFlashChoiceViewListener);
            mFlashAutoIcon.setOnClickListener(mFlashChoiceViewListener);
        }
    }

    private final IAppUiListener.OnShutterButtonListener mShutterListener =
            new IAppUiListener.OnShutterButtonListener() {

                @Override
                public boolean onShutterButtonFocus(boolean pressed) {
                    if (pressed) {
                        hideFlashChoiceView();
                    }
                    return false;
                }

                @Override
                public boolean onShutterButtonClick() {
                    return false;
                }

                @Override
                public boolean onShutterButtonLongPressed() {
                    return false;
                }
            };
}
