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

import com.android.camera.CameraActivity;
import com.android.camera.Log;
import com.android.camera.ModeChecker;
import com.android.camera.R;
import com.android.camera.Util;
import com.android.camera.ui.PickerButton;
import com.android.camera.ui.PickerButton.Listener;

import com.mediatek.camera.setting.preference.IconListPreference;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.setting.SettingConstants;

public class PickerManager extends ViewManager implements Listener,
        CameraActivity.OnPreferenceReadyListener, CameraActivity.OnParametersReadyListener {
    private static final String TAG = "PickerManager";

    public interface PickerListener {
        boolean onSlowMotionPicked(String turnon);

        boolean onHdrPicked(String value);

        boolean onCameraPicked(int camerId);

        boolean onFlashPicked(String flashMode);

        boolean onStereoPicked(boolean stereoType);

        boolean onModePicked(int mode, String value, ListPreference preference);
    }

    private PickerButton mSlowMotion;
    private PickerButton mHdr;
    private PickerButton mFlashPicker;
    private PickerButton mCameraPicker;
    private PickerButton mStereoPicker;
    private PickerListener mListener;
    private boolean mPreferenceReady;
    private CameraActivity mContext;

    private static final int PICKER_BUTTON_NUM = 5;
    private static final int BUTTON_HDR = 0;
    private static final int BUTTON_FLASH = 1;
    private static final int BUTTON_CAMERA = 2;
    private static final int BUTTON_STEREO = 3;
    private static final int BUTTON_SLOW_MOTION = 4;
    private PickerButton[] mPickerButtons = new PickerButton[PICKER_BUTTON_NUM];

    private static final int MAX_NUM_OF_SHOWEN = 4;
    private int[] mButtonPriority = { BUTTON_SLOW_MOTION, BUTTON_HDR, BUTTON_FLASH, BUTTON_CAMERA,
            BUTTON_STEREO };
    private boolean mDefineOrder = false;
    private static boolean[] sShownStatusRecorder = new boolean[PICKER_BUTTON_NUM];
    static {
        sShownStatusRecorder[BUTTON_SLOW_MOTION] = false;
        sShownStatusRecorder[BUTTON_HDR] = false;
        sShownStatusRecorder[BUTTON_FLASH] = false;
        sShownStatusRecorder[BUTTON_CAMERA] = false;
        sShownStatusRecorder[BUTTON_STEREO] = false;
    }

    public PickerManager(CameraActivity context) {
        super(context);
        mContext = context;
        context.addOnPreferenceReadyListener(this);
        context.addOnParametersReadyListener(this);
    }

    @Override
    protected View getView() {
        View view = inflate(R.layout.onscreen_pickers);

        mSlowMotion = (PickerButton) view.findViewById(R.id.onscreen_slow_motion_picker);
        mHdr = (PickerButton) view.findViewById(R.id.onscreen_hdr_picker);
        mFlashPicker = (PickerButton) view.findViewById(R.id.onscreen_flash_picker);
        mCameraPicker = (PickerButton) view.findViewById(R.id.onscreen_camera_picker);
        mStereoPicker = (PickerButton) view.findViewById(R.id.onscreen_stereo3d_picker);

        mPickerButtons[BUTTON_SLOW_MOTION] = mSlowMotion;
        mPickerButtons[BUTTON_HDR] = mHdr;
        mPickerButtons[BUTTON_FLASH] = mFlashPicker;
        mPickerButtons[BUTTON_CAMERA] = mCameraPicker;
        mPickerButtons[BUTTON_STEREO] = mStereoPicker;
        applyListeners();
        return view;
    }

    private void applyListeners() {
        for (PickerButton button : mPickerButtons) {
            button.setListener(this);
        }
    }

    public void setListener(PickerListener listener) {
        mListener = listener;
    }

    @Override
    public void onPreferenceReady() {
        Log.i(TAG, "onPreferenceReady()");
        mPreferenceReady = true;
    }

    @Override
    public void onCameraParameterReady() {
        Log.i(TAG, "onCameraParameterReady(), mDefineOrder:" + mDefineOrder + "" +
                ", mPreferenceReady:" + mPreferenceReady);
        if (!mPreferenceReady) {
            return;
        }

        // the max number of button shown on PickerManager UI is 4, Slow motion,
        // hdr, flash, dual camera,
        // stereo camera have high priority,if the order of button is definite, do not
        // redefine again.
        if (!mDefineOrder) {
            int count = 0;
            for (int i = 0; i < mButtonPriority.length; i++) {
                ListPreference pref = null;
                boolean visible = false;
                int buttonIndex = mButtonPriority[i];
                switch (buttonIndex) {
                case BUTTON_SLOW_MOTION:
                    pref = (IconListPreference) getContext().getListPreference(
                            SettingConstants.ROW_SETTING_SLOW_MOTION);
                    break;
                case BUTTON_HDR:
                    pref = (IconListPreference) getContext().getListPreference(
                            SettingConstants.ROW_SETTING_HDR);
                    break;
                case BUTTON_FLASH:
                    pref = (IconListPreference) getContext().getListPreference(
                            SettingConstants.ROW_SETTING_FLASH);
                    break;
                case BUTTON_CAMERA:
                    pref = (IconListPreference) getContext().getListPreference(
                            SettingConstants.ROW_SETTING_DUAL_CAMERA);
                    visible = ModeChecker.getCameraPickerVisible(getContext());
                    if (visible) {
                        count++;
                        if (pref != null) {
                            pref.showInSetting(false);
                        }
                    }
                    pref = null;
                    break;
                case BUTTON_STEREO:
                    pref = (IconListPreference) getContext().getListPreference(
                            SettingConstants.ROW_SETTING_STEREO_MODE);
                    visible = ModeChecker.getStereoPickerVisibile(getContext());
                    if (visible) {
                        count++;
                        if (pref != null) {
                            pref.showInSetting(false);
                        }

                    }
                    pref = null;
                    break;
                default:
                    break;
                }

                if (pref != null && pref.getEntries() != null
                        && pref.getEntries().length > 1) {
                    pref.showInSetting(false);
                    count++;
                }
                Log.i(TAG, "count:" + count + ", buttonIndex:" + buttonIndex);
                if (count >= MAX_NUM_OF_SHOWEN) {
                    break;
                }
            }
            mDefineOrder = true;
        } else {
            for (int i = 0; i < mButtonPriority.length; i++) {
                ListPreference pref = null;
                int buttonIndex = mButtonPriority[i];
                switch (buttonIndex) {
                case BUTTON_SLOW_MOTION:
                    pref = (IconListPreference) getContext().getListPreference(
                            SettingConstants.ROW_SETTING_SLOW_MOTION);
                    break;
                case BUTTON_HDR:
                    pref = (IconListPreference) getContext().getListPreference(
                            SettingConstants.ROW_SETTING_HDR);
                    break;
                case BUTTON_FLASH:
                    pref = (IconListPreference) getContext().getListPreference(
                            SettingConstants.ROW_SETTING_FLASH);
                    break;
                case BUTTON_CAMERA:
                    pref = (IconListPreference) getContext().getListPreference(
                            SettingConstants.ROW_SETTING_DUAL_CAMERA);
                    break;
                case BUTTON_STEREO:
                    pref = (IconListPreference) getContext().getListPreference(
                            SettingConstants.ROW_SETTING_STEREO_MODE);
                    break;
                default:
                    break;
                }
                if (pref != null) {
                    pref.showInSetting(sShownStatusRecorder[buttonIndex]);
                }
            }
        }

        refresh();
    }

    @Override
    public void hide() {
        if (mContext.getCurrentMode() == ModePicker.MODE_VIDEO
                && "on".equals(mContext.getISettingCtrl().getSettingValue(
                        SettingConstants.KEY_HDR))) {
            for (int i = PICKER_BUTTON_NUM - 1; i >= 0; i--) {
                if (mPickerButtons[i] == mHdr) {
                    mPickerButtons[i].setEnabled(true);
                    mPickerButtons[i].setClickable(false);
                    mPickerButtons[i].setVisibility(View.VISIBLE);
                    super.fadeIn();
                } else {
                    Util.fadeOut(mPickerButtons[i]);
                }
            }
        } else {
            super.hide();
        }
    }

    @Override
    public boolean onPicked(PickerButton button, ListPreference pref, String newValue) {
        boolean picked = false;
        String key = pref.getKey();
        if (mListener != null) {
            int index = -1;
            for (int i = 0; i < PICKER_BUTTON_NUM; i++) {
                if (button.equals(mPickerButtons[i])) {
                    index = i;
                    break;
                }
            }

            switch (index) {
            case BUTTON_SLOW_MOTION:
                picked = mListener.onSlowMotionPicked(newValue);
                break;
            case BUTTON_HDR:
                button.setValue(newValue);
                picked = mListener.onHdrPicked(newValue);
                break;
            case BUTTON_FLASH:
                picked = mListener.onFlashPicked(newValue);
                break;
            case BUTTON_CAMERA:
                picked = mListener.onCameraPicked(Integer.parseInt(newValue));
                break;
            case BUTTON_STEREO:
                picked = mListener.onStereoPicked("1".endsWith(newValue) ? true : false);
                break;
            default:
                break;
            }

        }
        Log.i(TAG, "onPicked(" + key + ", " + newValue + ") mListener=" + mListener + " return "
                + picked);
        return picked;
    }

    public void setCameraId(int cameraId) {
        if (mCameraPicker != null) {
            mCameraPicker.setValue("" + cameraId);
        }
    }

    @Override
    public void onRefresh() {
        Log.d(TAG, "onRefresh(), mPreferenceReady:" + mPreferenceReady);
        if (!mPreferenceReady) {
            return;
        }

        mSlowMotion.initialize((IconListPreference) getContext().getListPreference(
                SettingConstants.ROW_SETTING_SLOW_MOTION));
        mHdr.initialize((IconListPreference) getContext().getListPreference(
                SettingConstants.ROW_SETTING_HDR));
        mFlashPicker.initialize((IconListPreference) getContext().getListPreference(
                SettingConstants.ROW_SETTING_FLASH));
        mCameraPicker.initialize((IconListPreference) getContext().getListPreference(
                SettingConstants.ROW_SETTING_DUAL_CAMERA));
        mStereoPicker.initialize((IconListPreference) getContext().getListPreference(
                SettingConstants.ROW_SETTING_STEREO_MODE));

        mSlowMotion.refresh();
        mHdr.refresh();
        mFlashPicker.refresh();

        boolean isCameraPickerVisible = ModeChecker.getCameraPickerVisible(getContext());
        if (isCameraPickerVisible) {
            mCameraPicker.refresh();
        } else {
            mCameraPicker.setVisibility(View.GONE);
        }
        boolean isStereoPickerVisibile = ModeChecker.getStereoPickerVisibile(getContext());
        if (isStereoPickerVisibile) {
            mStereoPicker.refresh();
        } else {
            mStereoPicker.setVisibility(View.GONE);
        }
    }

    @Override
    protected void onRelease() {
        super.onRelease();
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        for (PickerButton button : mPickerButtons) {
            if (button != null) {
                button.setEnabled(enabled);
                button.setClickable(enabled);
            }
        }
    }

    /**
     * Force to enable the picker button indicated by the input key.
     * @param key The key used to indicate the picker button.
     */
    public void forceEnable(String key) {
        if (SettingConstants.KEY_HDR.equals(key) && mHdr != null) {
            mHdr.forceEnable();
        }
    }

    /**
     * Do not to force enable the picker button indicated by the input key.
     * @param key The key used to indicate the picker button.
     */
    public void cancelForcedEnable(String key) {
        if (SettingConstants.KEY_HDR.equals(key) && mHdr != null) {
            mHdr.cancelForcedEnable();
        }
    }
}
