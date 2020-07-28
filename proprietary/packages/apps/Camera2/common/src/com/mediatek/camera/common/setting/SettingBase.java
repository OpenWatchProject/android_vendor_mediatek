/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 * the prior written permission of MediaTek inc. and/or its licensor, any
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
 * NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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
package com.mediatek.camera.common.setting;

import android.app.Activity;
import android.os.Handler;
import android.os.Looper;

import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ISettingManager.SettingController;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Requester;
import com.mediatek.camera.common.setting.ISettingManager.SettingDeviceRequester;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import javax.annotation.Nonnull;

/**
 * This class is used to implement the base operations of setting.
 */
@SuppressWarnings("WeakerAccess")
public abstract class SettingBase implements ICameraSetting {
    protected IApp mApp;
    protected IAppUi mAppUi;
    protected DataStore mDataStore;
    protected Activity mActivity;
    protected ICameraContext mCameraContext;

    protected SettingDeviceRequester mSettingDeviceRequester;
    protected SettingDevice2Requester mSettingDevice2Requester;
    protected SettingController mSettingController;
    protected StatusMonitor mStatusMonitor;
    protected StatusMonitor.StatusResponder mStatusResponder;
    protected Handler mHandler;
    /**
     * The value this setting is set currently.
     */
    private String mValue;

    /**
     * The value stored in data store.
     */
    private String mValueInDataStore;

    /**
     * This list indicates the setting's values that can be selected by user in camera UI.
     * this list will be changed when setting is overridden by other settings or mode.
     */
    private List<String> mEntryValues = new ArrayList<>();

    /**
     * The original values that can be selected by user in camera UI. Mostly it is defined
     * in xml and it won't be changed.
     */
    private List<String> mSupportedEntryValues = new ArrayList<>();

    /**
     * The values supported by setting in current platform.
     */
    private List<String> mSupportedPlatformValues = new ArrayList<>();

    private OverridesList mOverridesList = new OverridesList();

    /**
     * The list used to record the overrides history of this setting.
     */
    private class OverridesList {
        private CopyOnWriteArrayList<Overrides> mOverriders = new CopyOnWriteArrayList<>();

        /**
         * Add a overrides to the list, If a overrides with its header key is equal to the
         * give overrides header key is already in the list, it will be replaced by the given
         * overrides, otherwise, add the given overrides as new one in the list.
         *
         * @param overrides Instance of {@link Overrides}.
         */
        public void add(Overrides overrides) {
            int index = indexOf(overrides.headerKey);
            if (index != -1) {
                Overrides oldOverrides = mOverriders.get(index);
                overrides.valueWhenOverride = oldOverrides.valueWhenOverride;
                mOverriders.set(index, overrides);
            } else {
                mOverriders.add(overrides);
            }
        }

        /**
         * Remove the overrides with its header key is equal to the given header key from
         * the list. If the list has no overrides with its header key is equal to the given
         * header key, nothing will happen.
         *
         * @param headerKey The header key of a overrides.
         * @return an index of current key or -1 if not has current key.
         */
        public int remove(String headerKey) {
            int index = indexOf(headerKey);
            if (index != -1) {
                mOverriders.remove(index);
            }
            return index;
        }

        /**
         * Get overrides at the first position in the list.
         *
         * @return A overrides at the first position in the list. It will be null if the list
         *         size is 0.
         */
        public Overrides getFirst() {
            if (mOverriders.size() == 0) {
                return null;
            }
            return mOverriders.get(0);
        }

        /**
         * Get the override list size.
         *
         * @return The override list size.
         */
        public int size() {
            return mOverriders.size();
        }

        /**
         * Get the override at the special position.
         *
         * @param index The position of the override list.
         * @return The override object at the special position.
         */
        public Overrides get(int index) {
            return mOverriders.get(index);
        }

        private int indexOf(String headerKey) {
            for (int i = 0; i < mOverriders.size(); i++) {
                Overrides overrides = mOverriders.get(i);
                if (overrides.headerKey.equals(headerKey)) {
                    return i;
                }
            }
            return -1;
        }
    }

    /**
     * A struct to describe the override info of setting which is overridden by
     * another one setting or a mode.
     */
    private class Overrides {
        /**
         * The string indicator who overrides this setting.
         */
        public String headerKey;
        /**
         * The setting current value when this override is arrival.
         */
        public String valueWhenOverride;
        /**
         * Value that setting going to change to in this override.
         */
        public String value;
        /**
         * Values that setting can be selected in this override.
         */
        public List<String> entryValues;
    }

    @Override
    public void init(IApp app,
                     ICameraContext cameraContext,
                     SettingController settingController) {
        mApp = app;
        mCameraContext = cameraContext;
        mDataStore = cameraContext.getDataStore();
        mAppUi = app.getAppUi();
        mSettingController = settingController;
        mActivity = app.getActivity();
        mHandler = new Handler(Looper.myLooper());
        mStatusMonitor = mCameraContext.getStatusMonitor(mSettingController.getCameraId());
        mStatusResponder = mStatusMonitor.getStatusResponder(getKey());
    }

    @Override
    public void setSettingDeviceRequester(
            @Nonnull SettingDeviceRequester settingDeviceRequester,
            @Nonnull SettingDevice2Requester settingDevice2Requester) {
        mSettingDeviceRequester = settingDeviceRequester;
        mSettingDevice2Requester = settingDevice2Requester;
    }

    @Override
    public void updateModeDeviceState(String newState) {
    }

    @Override
    public void addViewEntry() {

    }

    @Override
    public void removeViewEntry() {

    }

    @Override
    public void refreshViewEntry() {

    }

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {

    }

    @Override
    public synchronized void onModeClosed(String modeKey) {
        if (mOverridesList.indexOf(modeKey) >= 0) {
            this.overrideValues(modeKey, null, null);
        }
    }

    @Override
    public synchronized String getValue() {
        return mValue;
    }

    @Override
    public synchronized List<String> getEntryValues() {
        List<String> entryValues = new ArrayList<>();
        entryValues.addAll(mEntryValues);
        return entryValues;
    }

    @Override
    public synchronized List<String> getSupportedPlatformValues() {
        List<String> platformValues = new ArrayList<>();
        platformValues.addAll(mSupportedPlatformValues);
        return platformValues;
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        String valueBeforeOverride = "";
        synchronized (this) {
            valueBeforeOverride = mValue;
            Overrides overrides;
            if (currentValue == null && supportValues == null) {
                int index = mOverridesList.remove(headerKey);
                if (index == -1) {
                    return;
                }
                overrides = mOverridesList.getFirst();
                switchToOverridesValue(overrides);
            } else {
                overrides = new Overrides();
                overrides.headerKey = headerKey;
                overrides.valueWhenOverride = mValue;

                List<String> entryValues = new ArrayList<>(supportValues);
                // If the override entry values has no overlap with the platform supported
                // values, this overrides has no effect, so return.
                entryValues.retainAll(mSupportedPlatformValues);
                if (entryValues.size() == 0) {
                    return;
                }
                String value = currentValue;
                // If the override current value isn't contained in the platform supported
                // values but override entry values has overlap with the platform supported
                // values, then get the first value in the overlap as current value.
                if (!mSupportedPlatformValues.contains(currentValue)) {
                    value = entryValues.get(0);
                }

                overrides.value = value;
                overrides.entryValues = entryValues;
                mOverridesList.add(overrides);

                mValue = overrides.value;
                mEntryValues.clear();
                mEntryValues.addAll(overrides.entryValues);
                // Get the intersection of history override entry values as
                // setting current entry values
                for (int i = 0; i < mOverridesList.size() - 1; i++) {
                    Overrides overridesHistory = mOverridesList.get(i);
                    mEntryValues.retainAll(overridesHistory.entryValues);
                }
            }
        }
        if (mStatusResponder != null && mValue != null && !mValue.equals(valueBeforeOverride)) {
            mStatusResponder.statusChanged(getKey(), mValue);
        }
    }

    @Override
    public PreviewStateCallback getPreviewStateCallback() {
        return null;
    }

    /**
     * Get the store scope of setting to store its value in data store. Default, the scope
     * is bounds to camera id, if setting has different scope, it should override this method.
     *
     * @return The store scope of setting.
     */
    public String getStoreScope() {
        return mDataStore.getCameraScope(Integer.parseInt(mSettingController.getCameraId()));
    }

    /**
     * Update setting value.
     *
     * @param value The new value.
     */
    public void setValue(String value) {
        boolean changed = false;
        synchronized (this) {
            changed = (mValue != null && !mValue.equals(value));
            mValueInDataStore = value;
            mValue = value;
        }
        if (mStatusResponder != null && changed) {
            mStatusResponder.statusChanged(getKey(), value);
        }
    }

    /**
     * Update setting entry values.
     *
     * @param entryValues The new entry values.
     */
    public synchronized void setEntryValues(List<String> entryValues) {
        if (entryValues == null) {
            return;
        }
        mEntryValues.clear();
        mEntryValues.addAll(entryValues);
    }

    /**
     * Set setting supported entry values.
     *
     * @param supportedEntryValues The supported entry values.
     */
    public synchronized void setSupportedEntryValues(List<String> supportedEntryValues) {
        if (supportedEntryValues == null) {
            return;
        }
        mSupportedEntryValues.clear();
        mSupportedEntryValues.addAll(supportedEntryValues);
    }

    /**
     * Set setting supported values in current platform.
     *
     * @param supportedPlatformValues The supported values in current platform.
     */
    public synchronized void setSupportedPlatformValues(List<String> supportedPlatformValues) {
        if (supportedPlatformValues == null) {
            return;
        }
        mSupportedPlatformValues.clear();
        mSupportedPlatformValues.addAll(supportedPlatformValues);
    }

    /**
     * Remove the override produced by setting indicated by the input header key
     * from this setting.
     *
     * @param headerKey The key of setting which produces the override.
     */
    public synchronized void removeOverride(String headerKey) {
        mOverridesList.remove(headerKey);
    }

    private void switchToOverridesValue(Overrides overrides) {
        if (overrides == null) {
            restoreValue();
            return;
        }
        // switch to overrides value
        if (overrides.entryValues != null
                && overrides.entryValues.contains(mValueInDataStore)
                && !mValueInDataStore.equals(overrides.valueWhenOverride)) {
            mValue = mValueInDataStore;
        } else {
            mValue = overrides.value;
        }
        mEntryValues.clear();
        for (String value : overrides.entryValues) {
            mEntryValues.add(value);
        }
    }

    private void restoreValue() {
        mValue = mValueInDataStore;
        mEntryValues.clear();
        mEntryValues.addAll(mSupportedEntryValues);
        mEntryValues.retainAll(mSupportedPlatformValues);
    }
}
