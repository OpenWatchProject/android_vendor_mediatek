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
package com.mediatek.camera.common.relation;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This class is for setting and getting feature shared preferences values.
 *
 * <p>This class introduces the concept of "scope" for a setting, which is the generality
 * of a feature. If the feature can be accessed crossing the entire application, it has
 * a global scope, it is stored in the default SharedPreferences file. If the feature only
 * can be accessed in current camera device, it has a camera scope, it is stored in the
 * SharedPreferences related to camera id.</p>
 *
 */
public class DataStore {
    /** This scope stores and retrieves settings from default preferences. */
    private static final String GLOBAL_SCOPE = "_global_scope";
    private static final String PREFIX_CAMERA_SCOPE = "_preferences_";
    private static final String PREFIX_SAVING_TIMESTAMP = "_saving_timestamp";
    private final Context mContext;
    private final String mPackageName;
    private final Object mLock = new Object();
    private final Map<String, SharedPreferencesWrapper> mPrefWrapperMap = new ConcurrentHashMap<>();
    private final CopyOnWriteArrayList<String> mGlobalKeys = new CopyOnWriteArrayList();


    /**
     * Constructor for data store.
     *
     * @param context The activity context.
     */
    public DataStore(Context context) {
        mContext = context;
        mPackageName = mContext.getPackageName();
    }

    /**
     * Get the global scope, the shared preference of this scope can be sharing crossing
     * the entire application.
     *
     * @return The global scope.
     */
    public String getGlobalScope() {
        return GLOBAL_SCOPE;
    }

    /**
     * Get the camera scope, the shared preference of this scope only can be sharing
     * in the specified camera indicated by input camera id.
     *
     * @param cameraId The id of specified camera.
     * @return The camera scope.
     */
    public String getCameraScope(int cameraId) {
        return PREFIX_CAMERA_SCOPE + cameraId;
    }

    /**
     * Set the value to shared preferences or save the value to a cache.
     *
     * @param key The name used to record.
     * @param value The new value going to set.
     * @param scope Used to index shared preferences.
     * @param cached True means that value is cached in hash map, it will be saved until activity
     *               is destroyed. False means that value will be saved in shared preferences, and
     *               it is saved even activity is destroyed.
     */
    public void setValue(String key, String value, String scope, boolean cached) {
        setValue(key, value, scope, cached, false);
    }

    /**
     * Set the value to shared preferences or save the value to a cache.
     *
     * @param key The name used to record.
     * @param value The new value going to set.
     * @param scope Used to index shared preferences.
     * @param cached True means that value is cached in hash map, it will be saved until activity
     *               is destroyed. False means that value will be saved in shared preferences, and
     *               it is saved even activity is destroyed.
     * @param keepSavingTime This parameter is used to keep the time that setting saving value.
     *                       True means to keep time, false means to don't keep time. The saving
     *                       time indicates when the user operates setting. The setting latest
     *                       operated by user has largest timestamp which saving in the shared
     *                       preferences. When setting initialized next time, this timestamp will
     *                       decide when the restriction of this setting will be posted out after
     *                       it is initialized. The setting will more earlier post its restriction
     *                       when its value saving timestamp more larger.
     *                       This parameter is set as true when setting is exclusive with another
     *                       one setting and both can be set as "on" in the shared preferences, like
     *                       "hdr" and "flash", they are exclusive and can be set as "on" in the
     *                       shared preferences. So the saving timestamp can be decide which
     *                       setting's restriction will be first executed, that cause another one
     *                       setting is limited.
     */
    public void setValue(String key, String value, String scope,
                         boolean cached, boolean keepSavingTime) {
        if (GLOBAL_SCOPE.equals(scope)) {
            mGlobalKeys.add(key);
        }
        SharedPreferencesWrapper prefWrapper = getSharedPreferencesWrapperSync(scope);
        prefWrapper.setValue(key, value, cached);

        if (keepSavingTime) {
            SharedPreferencesWrapper prefWrapperForSavingTimestamp
                    = getSharedPreferencesWrapperSync(scope + PREFIX_SAVING_TIMESTAMP);
            String timestamp = String.valueOf(System.currentTimeMillis());
            prefWrapperForSavingTimestamp.setValue(key, timestamp, false);
        }
    }

    /**
     * Get the value from shared preferences or cache.
     *
     * @param key The name used to query.
     * @param defaultValue The default value. If the key has no value in the shared preferences, it
     *                     will return default value.
     * @param scope Used to index shared preferences.
     * @return The value in shared preferences or cache.
     */
    public String getValue(String key, String defaultValue, String scope) {
        if (mGlobalKeys.contains(key)) {
            scope = GLOBAL_SCOPE;
        }
        SharedPreferencesWrapper prefWrapper = getSharedPreferencesWrapperSync(scope);
        return prefWrapper.getValue(key, defaultValue);
    }

    /**
     * Get the settings that keeps timestamp of saving values.
     *
     * @param cameraId The id of camera.
     * @return A list of settings that keeps timestamp of saving values. The order of settings in
     *         the list is that it is in more front position when its saving timestamp is lager.
     */
    public List<String> getSettingsKeepSavingTime(int cameraId) {
        SharedPreferencesWrapper prefWrapperInCameraScope = getSharedPreferencesWrapperSync(
                getCameraScope(cameraId) + PREFIX_SAVING_TIMESTAMP);
        SharedPreferencesWrapper prefWrapperInGlobalScope = getSharedPreferencesWrapperSync(
                GLOBAL_SCOPE + PREFIX_SAVING_TIMESTAMP);

        List<String> outputKeys = new LinkedList<>();
        List<Long> outputTimestamps = new LinkedList<>();
        sortSettingByTimestamp(prefWrapperInCameraScope.getAll(), outputKeys, outputTimestamps);
        sortSettingByTimestamp(prefWrapperInGlobalScope.getAll(), outputKeys, outputTimestamps);

        return outputKeys;
    }

    private SharedPreferencesWrapper getSharedPreferencesWrapperSync(String scope) {
        synchronized (mLock) {
            SharedPreferencesWrapper prefWrapper = mPrefWrapperMap.get(scope);
            if (prefWrapper == null) {
                prefWrapper = new SharedPreferencesWrapper(scope);
                mPrefWrapperMap.put(scope, prefWrapper);
            }
            return prefWrapper;
        }
    }

    private void sortSettingByTimestamp(Map<String, ?> inputMap,
                                        List<String> outputKeys,
                                        List<Long> outputTimestamps) {
        Set<String> inputKeys = inputMap.keySet();
        Iterator<String> iterator = inputKeys.iterator();
        while (iterator.hasNext()) {
            String key = iterator.next();
            Long timestamp = Long.parseLong((String) inputMap.get(key));
            int index = -1;
            for (int i = 0; i < outputTimestamps.size(); i++) {
                if (timestamp > outputTimestamps.get(i)) {
                    index = i;
                    break;
                }
            }
            if (index == -1) {
                index = outputKeys.size();
            }
            outputKeys.add(index, key);
            outputTimestamps.add(index, timestamp);
        }
    }

    /**
     * This wrapper class used to save value to shared preferences or cache value in hash map.
     */
    private class SharedPreferencesWrapper {
        private final String mScope;
        private final SharedPreferences mSharedPreferences;
        private final Map<String, String> mValueCache = new ConcurrentHashMap<>();

        SharedPreferencesWrapper(String scope) {
            mScope = scope;
            mSharedPreferences = getPreferencesFromScope(mScope);
        }

        /**
         * Set the value to shared preferences or save to cache.
         *
         * @param key The name used to record.
         * @param value The new value for the preference.
         * @param cached True means that value is cached in hash map, it will be saved until
         *               activity is destroyed. False means that value will be saved in shared
         *               preferences, and it is saved even activity is destroyed.
         */
        void setValue(String key, String value, boolean cached) {
            if (cached) {
                mValueCache.put(key, value);
            } else {
                mSharedPreferences.edit().putString(key, value).apply();
            }
        }

        /**
         * Get the value from shared preferences or cache.
         *
         * @param key The name of the preference to modify.
         * @param defaultValue The default value. If the key has no value in the shared
         *                     preferences, it will return default value.
         * @return The value in shared preferences or cache.
         */
        public String getValue(String key, String defaultValue) {
            if (mValueCache.containsKey(key)) {
                return mValueCache.get(key);
            } else {
                return mSharedPreferences.getString(key, defaultValue);
            }
        }

        /**
         * Get all the keys and values in the shared preferences.
         *
         * @return Return all the keys and values.
         */
        public Map<String, ?> getAll() {
            return mSharedPreferences.getAll();
        }

        private SharedPreferences getPreferencesFromScope(String scope) {
            if (scope.equals(GLOBAL_SCOPE)) {
                return PreferenceManager.getDefaultSharedPreferences(mContext);
            }
            return openPreferences(scope);
        }

        private SharedPreferences openPreferences(String scope) {
            return mContext.getSharedPreferences(
                    mPackageName + scope, Context.MODE_PRIVATE);
        }
    }
}
