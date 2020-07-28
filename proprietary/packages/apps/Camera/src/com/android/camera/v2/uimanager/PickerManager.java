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
import android.view.View;
import android.view.ViewGroup;

import com.android.camera.R;
import com.android.camera.v2.ui.PickerButton;
import com.android.camera.v2.uimanager.preference.IconListPreference;
import com.android.camera.v2.uimanager.preference.ListPreference;
import com.android.camera.v2.uimanager.preference.PreferenceManager;
import com.android.camera.v2.util.SettingKeys;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

public class PickerManager extends AbstractUiManager implements PickerButton.Listener {
    private static final Tag TAG = new Tag(PickerManager.class.getSimpleName());
    private static final int                     PICKER_BUTTON_NUM = 5;
    private static final int                     BUTTON_HDR = 0;
    private static final int                     BUTTON_FLASH = 1;
    private static final int                     BUTTON_CAMERA = 2;
    private static final int                     BUTTON_STEREO = 3;
    private static final int                     BUTTON_SLOW_MOTION = 4;
    private static final int                     MAX_NUM_OF_SHOWEN = 4;

    private PreferenceManager                    mPreferenceManager;
    private PickerButton                         mSlowMotion;
    private PickerButton                         mHdr;
    private PickerButton                         mFlashPicker;
    private PickerButton                         mCameraPicker;
    private PickerButton                         mStereoPicker;
    private PickerButton[]                       mPickerButtons
                                = new PickerButton[PICKER_BUTTON_NUM];
    private OnPickedListener                     mOnPickedListener;

    // Picker button show order is defined or not.
    private boolean                              mOrderDefined = false;
    // The max number of switcher button showing on camera preview UI is 4. This define the priority
    // of switcher button, If the camera preview UI has 4 button, the rest will be shown
    // in setting box.
    private int[]                                mButtonPriority = {
            BUTTON_SLOW_MOTION,
            BUTTON_HDR,
            BUTTON_FLASH,
            BUTTON_CAMERA,
            BUTTON_STEREO,
    };

    private static final Map<Integer, String>  mButtonKeys =
            new HashMap<Integer, String>(PICKER_BUTTON_NUM);

    static {
        mButtonKeys.put(BUTTON_HDR,          SettingKeys.KEY_HDR);
        mButtonKeys.put(BUTTON_FLASH,        SettingKeys.KEY_FLASH);
        mButtonKeys.put(BUTTON_CAMERA,       SettingKeys.KEY_CAMERA_ID);
        mButtonKeys.put(BUTTON_STEREO,       SettingKeys.KEY_STEREO3D_MODE);
        mButtonKeys.put(BUTTON_SLOW_MOTION,  SettingKeys.KEY_SLOW_MOTION);
    }

    // Recode the setting showing position, false means showing on camera UI, true
    // means show in setting box.
    private static boolean[] sShowPosRecoder = new boolean[PICKER_BUTTON_NUM];
    // The switcher default show on camera preview UI.
    static {
        sShowPosRecoder[BUTTON_SLOW_MOTION]   = false;
        sShowPosRecoder[BUTTON_HDR]           = false;
        sShowPosRecoder[BUTTON_FLASH]         = false;
        sShowPosRecoder[BUTTON_CAMERA]        = false;
        sShowPosRecoder[BUTTON_STEREO]        = false;
    }

    public interface OnPickedListener {
        public void onPicked(String key, String value);
    }

    public PickerManager(Activity activity, ViewGroup parent,
            PreferenceManager preferenceManager) {
        super(activity, parent);
        mPreferenceManager = preferenceManager;
    }

    @Override
    protected View getView() {
        View view = inflate(R.layout.onscreen_pickers_v2);

        mSlowMotion   = (PickerButton) view.findViewById(R.id.onscreen_slow_motion_picker);
        mHdr          = (PickerButton) view.findViewById(R.id.onscreen_hdr_picker);
        mFlashPicker  = (PickerButton) view.findViewById(R.id.onscreen_flash_picker);
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

    @Override
    protected void onRefresh() {
        LogHelper.d(TAG, "[onRefresh], mOrderDefined:" + mOrderDefined);
        // Preference should update when switch camera.
        mSlowMotion.initialize((IconListPreference) mPreferenceManager.getListPreference(
                SettingKeys.KEY_SLOW_MOTION));
        mHdr.initialize((IconListPreference) mPreferenceManager.getListPreference(
                SettingKeys.KEY_HDR));
        mFlashPicker.initialize((IconListPreference) mPreferenceManager.getListPreference(
                SettingKeys.KEY_FLASH));
        mCameraPicker.initialize((IconListPreference) mPreferenceManager.getListPreference(
                SettingKeys.KEY_CAMERA_ID));
        mStereoPicker.initialize((IconListPreference) mPreferenceManager.getListPreference(
                SettingKeys.KEY_STEREO3D_MODE));

        for (PickerButton button : mPickerButtons) {
            if (button != null) {
                button.refresh();
            }
        }
    }

    @Override
    public boolean onPicked(PickerButton button, ListPreference preference,
            String newValue) {
        if (mOnPickedListener != null) {
            mOnPickedListener.onPicked(preference.getKey(), newValue);
        }
        return true;
    }

    @Override
    public void setEnable(boolean enabled) {
        super.setEnable(enabled);
        for (PickerButton button : mPickerButtons) {
            if (button != null) {
                button.setEnabled(enabled);
                button.setClickable(enabled);
            }
        }
    }

    public void notifyPreferenceReady() {
        LogHelper.i(TAG, "[notifyPreferenceReady]...");
        defineButtonOrder();
    }

    public void setOnPickedListener(OnPickedListener listener) {
        mOnPickedListener = listener;
    }

    public void performCameraPickerBtnClick() {
        ListPreference pref = mPreferenceManager.getListPreference(SettingKeys.KEY_CAMERA_ID);
        if (pref != null) {
            String value = pref.getValue();
            int index = pref.findIndexOfValue(value);
            if (index == ListPreference.INVALID_INDEX) {
                return;
            }
            CharSequence[] values = pref.getEntryValues();
            index = (index + 1) % values.length;
            String next = values[index].toString();
            if (mOnPickedListener != null) {
                mOnPickedListener.onPicked(SettingKeys.KEY_CAMERA_ID, next);
            }
            pref.setValueIndex(index);
        }
    }

    /**
     * Force enable picker button.
     * @param key The key which indicate the picker button
     */
    public void forceEnablePickerButton(String key) {
        LogHelper.d(TAG, "[forceEnablePickerButton], key:" + key);
        ListPreference pref = mPreferenceManager.getListPreference(key);
        if (pref.isEnabled()) {
            return;
        }
        pref.setEnabled(true);

        Set<Integer> keyset = mButtonKeys.keySet();
        Iterator<Integer> iterator = keyset.iterator();
        int buttonIndex = -1;
        while (iterator.hasNext()) {
            buttonIndex = iterator.next();
            if (mButtonKeys.get(buttonIndex).equals(key)) {
                break;
            }
        }
        if (buttonIndex > 0) {
            PickerButton button = mPickerButtons[buttonIndex];
            button.refresh();
        }

    }

    /**
     * Force disable picker button.
     * @param key The key which indicate the picker button
     */
    public void forceDisablePickerButton(String key) {
        LogHelper.d(TAG, "[forceDisablePickerButton], key:" + key);
        ListPreference pref = mPreferenceManager.getListPreference(key);
        if (!pref.isEnabled()) {
            return;
        }
        pref.setEnabled(false);
        Set<Integer> keyset = mButtonKeys.keySet();
        Iterator<Integer> iterator = keyset.iterator();
        int buttonIndex = -1;
        while (iterator.hasNext()) {
            buttonIndex = iterator.next();
            if (mButtonKeys.get(buttonIndex).equals(key)) {
                break;
            }
        }
        if (buttonIndex > 0) {
            PickerButton button = mPickerButtons[buttonIndex];
            button.refresh();
        }
    }

    private void applyListeners() {
        for (PickerButton button : mPickerButtons) {
            if (button != null) {
                button.setListener(this);
            }
        }
    }

    private void clearListeners() {
        for (PickerButton button : mPickerButtons) {
            if (button != null) {
                button.setListener(null);
            }
        }
    }

    private void defineButtonOrder() {
        int count = 0;
        if (mOrderDefined) {
            for (int i = 0; i < mButtonPriority.length; i++) {
                int buttonIndex = mButtonPriority[i];
                String key = mButtonKeys.get(buttonIndex);
                ListPreference pref = mPreferenceManager.getListPreference(key);
                if (pref != null && pref.isVisibled()) {
                    pref.showInSetting(sShowPosRecoder[buttonIndex]);
                }
            }
            return;
        }

        for (int i = 0; i < mButtonPriority.length; i++) {
            int buttonIndex = mButtonPriority[i];
            String key = mButtonKeys.get(buttonIndex);
            ListPreference pref = mPreferenceManager.getListPreference(key);
            if (pref != null && pref.isVisibled()) {
                pref.showInSetting(false);
                count ++;
                sShowPosRecoder[buttonIndex] = false;
            }

            if (count >= MAX_NUM_OF_SHOWEN) {
                break;
            }
        }
        mOrderDefined = true;
    }
}
