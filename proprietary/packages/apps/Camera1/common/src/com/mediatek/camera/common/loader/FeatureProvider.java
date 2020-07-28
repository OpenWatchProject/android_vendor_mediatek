/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 *     the prior written permission of MediaTek inc. and/or its licensor, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.common.loader;

import android.app.Activity;
import android.hardware.Camera.Parameters;
import android.os.AsyncTask;
import android.os.ConditionVariable;

import com.mediatek.camera.common.IAppUi.ModeItem;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.mode.DeviceUsage;
import com.mediatek.camera.common.setting.ICameraSetting;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * A provider for features, control loading process and provider
 * the dynamic loaded features to other modules.
 */
public class FeatureProvider {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(FeatureProvider.class.getSimpleName());
    private ConcurrentHashMap<String, IFeatureEntry> mAllEntries = new ConcurrentHashMap<>();
    private ConcurrentHashMap<String, IFeatureEntry> mBuildInEntries = new ConcurrentHashMap<>();
    private ConcurrentHashMap<String, IFeatureEntry> mPluginEntries = new ConcurrentHashMap<>();
    private CopyOnWriteArrayList<FeatureLoadDoneListener> mFeatureLoadDoneListeners =
            new CopyOnWriteArrayList<>();
    private CopyOnWriteArrayList<String> mNotifiedApi1BuildInCameraIds
            = new CopyOnWriteArrayList();
    private CopyOnWriteArrayList<String> mNotifiedApi1PluginCameraIds
            = new CopyOnWriteArrayList();
    private final ConditionVariable mPluginLoadCondition = new ConditionVariable();
    private final ConditionVariable mBuildInLoadCondition = new ConditionVariable();
    private final Activity mActivity;
    private final Object mNotifyApi1Sync = new Object();

    /**
     * 3rd party need implement this interface if you want get the feature entry.
     * when all features are loaded will be fire this function.
     */
    public interface FeatureLoadDoneListener {
        /**
         * Notify when build in feature loaded and parameters ready.
         * @param cameraId which camera's setting and feature ready.
         * @param cameraApi which api.
         */
        void onBuildInLoadDone(String cameraId, CameraApi cameraApi);
        /**
         * Notify when plugin feature loaded and parameters ready.
         * @param cameraId which camera's setting and feature ready.
         * @param cameraApi which api.
         */
        void onPluginLoadDone(String cameraId, CameraApi cameraApi);
    }

    /**
     * A key used to do feature lookups.
     * <p>For example, to get a mode feature instance.
     * <code><pre>
     * ICameraMode mode = featureProvider.getInstance(
     *      new featureProvider.Key("<mode name>", ICameraMode.class));
     * </pre></code>
     * </p>
     */
    public static final class Key<T> {
        private final String mName;
        private final Class<T> mClassType;
        /**
         * Key constructor.
         * @param name Key's name, it should be same with the feature name.
         * @param classType The feature' class type, if the feature is a
         *                  mode, the type will be ICameraMode.class, if the
         *                  feature is a setting, the type will be ICameraSetting.class.
         */
        public Key(String name, Class<T> classType) {
            mName = name;
            mClassType = classType;
        }

        /**
         * Get the key description name.
         * @return The key name.
         */
        @Nonnull
        public String getName() {
            return mName;
        }

        @Override
        public final int hashCode() {
            return mName.hashCode();
        }

        @SuppressWarnings("unchecked")
        @Override
        public final boolean equals(@Nonnull Object object) {
            return object instanceof Key && ((Key<T>) object).mName.equals(mName);
        }
    }

    /**
     * A provider for features, control loading process and provider.
     * the dynamic loaded features to other modules.
     * @param app current IApp.
     */
    public FeatureProvider(IApp app) {
        mActivity = app.getActivity();
        mPluginLoadCondition.close();
        mBuildInLoadCondition.close();
        loadFeatureInBackground();
    }

    /**
     * Register {@link FeatureLoadDoneListener} to receive non-build in feature load done event.
     * @param pluginLoadDoneListener the listener to receive load done event.
     */
    public void registerFeatureLoadDoneListener(
            @Nonnull FeatureLoadDoneListener pluginLoadDoneListener) {
        if (pluginLoadDoneListener == null) {
            return;
        }
        if (!mFeatureLoadDoneListeners.contains(pluginLoadDoneListener)) {
            mFeatureLoadDoneListeners.add(pluginLoadDoneListener);
        }
        postNotifiedBuildInFeatureLoadDone(pluginLoadDoneListener);
        postNotifiedPluginFeatureLoadDone(pluginLoadDoneListener);
    }

    /**
     * Unregister {@link FeatureLoadDoneListener}.
     * @param pluginLoadDoneListener the listener to be unregistered.
     */
    public void unregisterPluginLoadDoneListener(
            @Nonnull FeatureLoadDoneListener pluginLoadDoneListener) {
        if (pluginLoadDoneListener == null) {
            return;
        }
        if (mFeatureLoadDoneListeners.contains(pluginLoadDoneListener)) {
            mFeatureLoadDoneListeners.remove(pluginLoadDoneListener);
        }
    }

    /**
     * Update current mode key to feature entry, dual camera zoom need to set properties
     * in photo and video mode before open camera, this notify only update to setting feature.
     * @param currentModeKey current mode key.
     */
    public void updateCurrentModeKey(String currentModeKey) {
        LogHelper.d(TAG, "[updateCurrentModeKey] current mode key:" + currentModeKey);
        if (mBuildInEntries.size() <= 0) {
            mBuildInLoadCondition.block();
        }
        Iterator iterator = mBuildInEntries.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry item = (Map.Entry) iterator.next();
            IFeatureEntry entry = (IFeatureEntry) item.getValue();
            if (ICameraSetting.class.equals(entry.getType())) {
                entry.updateCurrentModeKey(currentModeKey);
            }
        }
    }

    /**
     * Update device usage with mode key, dual camera zoom may update normal
     * photo and video mode's device usage.
     * @param modeKey this device's mode key.
     * @param originalDeviceUsage the original device usage.
     * @return the updated device usage.
     */
    public DeviceUsage updateDeviceUsage(String modeKey, DeviceUsage originalDeviceUsage) {
        LogHelper.d(TAG, "[updateDeviceUsage] mode key:" + modeKey +
                         ", device type:" + originalDeviceUsage.getDeviceType());
        DeviceUsage tempDeviceUsage = originalDeviceUsage;
        Iterator iterator = mBuildInEntries.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry item = (Map.Entry) iterator.next();
            IFeatureEntry entry = (IFeatureEntry) item.getValue();
            if (ICameraSetting.class.equals(entry.getType())) {
                tempDeviceUsage =
                        entry.updateDeviceUsage(modeKey, tempDeviceUsage);
            }
        }
        return tempDeviceUsage;
    }

    /**
     * Get a feature instance by the given key.
     * it maybe always return the same instance every time use same key.
     * also,it maybe return the new one every time use same key.
     * it dependency the feature entry implementer;
     * if the key don't include in the entry map will return a null.
     * @param key The feature key.
     * @param <T> Feature key type.
     * @param currentCameraApi current camera api, if need check support must not null.
     * @param checkSupport whether need check support.
     * @return The feature instance,
     */
    public <T> T getInstance(@Nonnull Key<T> key,
                             @Nullable CameraApi currentCameraApi,
                             boolean checkSupport) {
        if (!mAllEntries.containsKey(key.getName())) {
            mBuildInLoadCondition.block();
        }
        if (!mAllEntries.containsKey(key.getName())) {
            mPluginLoadCondition.block();
        }
        IFeatureEntry entry = mAllEntries.get(key.getName());
        LogHelper.d(TAG, "[getInstance],key = " + key.getName() + ",entry = " + entry);
        if (entry == null) {
            return null;
        }
        if (checkSupport) {
            return entry.isSupport(currentCameraApi, mActivity) ? (T) entry.createInstance() : null;
        }
        return (T) entry.createInstance();
    }

    /**
     * Get the feature instance list by the given key and its stage is equal with
     * the input stage. It maybe always return the same instance every time use
     * same classType. Also,it maybe return the new one every time use same classType.
     * it dependency the feature entry implementer; also will be return am empty
     * list when don't find a instance in the all instances.
     *
     * @param classType The feature class type, if the feature is a mode,
     *                  the type will be the ICameraMode.class, if the feature is
     *                  a setting, it will be the ICameraSetting.class.
     * @param currentCameraApi current camera api.
     * @param stage The setting stage.
     * @return The feature instance list.
     */
    public List<?> getInstancesByStage(Class<?> classType,
                                       @Nonnull CameraApi currentCameraApi,
                                       int stage) {
        ArrayList list = new ArrayList();
        // get the values from hashMap use iterator method.
        Iterator iterator = mBuildInEntries.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry item = (Map.Entry) iterator.next();
            IFeatureEntry entry = (IFeatureEntry) item.getValue();
            if (classType.equals(entry.getType())
                    && entry.isSupport(currentCameraApi, mActivity)
                    && entry.getStage() == stage) {
                list.add(entry.createInstance());
            }
        }
        return list;
    }

    /**
     * Get the feature instance list by the given key.
     * it maybe always return the same instance every time use same classType.
     * also,it maybe return the new one every time use same classType.
     * it dependency the feature entry implementer;
     * also will be return am empty list when don't find a instance in the all instances.
     * @param classType The feature class type, if the feature is a mode,
     *                  the type will be the ICameraMode.class, if the feature is
     *                  a setting, it will be the ICameraSetting.class.
     * @param currentCameraApi current camera api.
     * @return The feature instance list.
     */
    public List<?> getAllBuildInInstance(Class<?> classType, @Nonnull CameraApi currentCameraApi) {
        ArrayList list = new ArrayList();
        // get the values from hashMap use iterator method.
        Iterator iterator = mBuildInEntries.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry item = (Map.Entry) iterator.next();
            IFeatureEntry entry = (IFeatureEntry) item.getValue();
            if (classType.equals(entry.getType()) && entry.isSupport(currentCameraApi, mActivity)) {
                list.add(entry.createInstance());
            }
        }
        return list;
    }

    /**
     * Get the APK instance list by the given key.
     * it maybe always return the same instance every time use same classType.
     * also,it maybe return the new one every time use same classType.
     * it dependency the feature entry implementer;
     * also will be return am empty list when don't find a instance in the all instances.
     * @param classType The feature class type, if the feature is a mode,
     *                  the type will be the ICameraMode.class, if the feature is
     *                  a setting, it will be the ICameraSetting.class.
     * @param currentCameraApi current camera api.
     * @return The feature instance list.
     */
    public List<?> getAllPlugInInstance(Class<?> classType, @Nonnull CameraApi currentCameraApi) {
        ArrayList list = new ArrayList();

        // get the values from hashMap use iterator method.
        Iterator iterator = mPluginEntries.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry item = (Map.Entry) iterator.next();
            IFeatureEntry entry = (IFeatureEntry) item.getValue();
            if (entry.isSupport(currentCameraApi, mActivity) && classType.equals(entry.getType())) {
                list.add(entry.createInstance());
            }
        }
        return list;
    }

    /**
     * Get all the mode items.you must be after the onPluginLoadDone() notify done.
     * @param currentCameraApi current camera api.
     * @return all the mode items.
     */
    public List<ModeItem> getAllModeItems(@Nonnull CameraApi currentCameraApi) {
        ArrayList list = new ArrayList();
        // get the values from hashMap use iterator method.
        Iterator iterator = mAllEntries.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry item = (Map.Entry) iterator.next();
            IFeatureEntry entry = (IFeatureEntry) item.getValue();
            if (entry.isSupport(currentCameraApi, mActivity)) {
                ModeItem modeItem = entry.getModeItem();
                if (modeItem != null) {
                    list.add(modeItem);
                }
            }
        }
        return list;
    }

    /**
     * Update parameters to feature provider.
     * @param cameraId current opened camera id.
     * @param parameters current camera's parameters.
     */
    public void updateCameraParameters(@Nonnull String cameraId, @Nonnull Parameters parameters) {
        LogHelper.d(TAG, "[updateCameraParameters] camera id:" + cameraId);
        if (parameters == null || cameraId == null) {
            return;
        }
        synchronized (mNotifyApi1Sync) {
            if (!mNotifiedApi1BuildInCameraIds.contains(cameraId)) {
                if (mBuildInEntries.size() > 0) {
                    notifyApi1BuildInFeatureLoadDone(cameraId);
                }
                if (mPluginEntries.size() > 0) {
                    notifyApi1PluginFeatureLoadDone(cameraId);
                }
            }
        }
    }

    private void loadFeatureInBackground() {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                synchronized (mNotifyApi1Sync) {
                    mBuildInEntries = FeatureLoader.loadBuildInFeatures(mActivity);
                    mAllEntries.putAll(mBuildInEntries);
                    mBuildInLoadCondition.open();
                    notifyAllApi1BuildInFeatureLoadDone();
                    notifyAllApi2BuildInFeatureLoadDone();

                    mPluginEntries = FeatureLoader.loadPluginFeatures(mActivity);
                    mAllEntries.putAll(mPluginEntries);
                    mPluginLoadCondition.open();
                    notifyAllApi2PluginFeatureLoadDone();
                    notifyAllApi1PluginFeatureLoadDone();
                    return null;
                }
            }
        } .executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    private void notifyAllApi1BuildInFeatureLoadDone() {
        if (mBuildInEntries.size() <= 0) {
            return;
        }
        ConcurrentHashMap<String, DeviceDescription> deviceDescriptions =
                CameraApiHelper.getDeviceSpec(mActivity).getDeviceDescriptionMap();
        Collection<String> cameraIds = deviceDescriptions.keySet();
        for (String cameraId : cameraIds) {
            if (deviceDescriptions.get(cameraId).getParameters() != null) {
                notifyApi1BuildInFeatureLoadDone(cameraId);
            }
        }
    }

    private void notifyAllApi1PluginFeatureLoadDone() {
        if (mPluginEntries.size() <= 0) {
            return;
        }
        ConcurrentHashMap<String, DeviceDescription> deviceDescriptions =
                CameraApiHelper.getDeviceSpec(mActivity).getDeviceDescriptionMap();
        Collection<String> cameraIds = deviceDescriptions.keySet();
        for (String cameraId : cameraIds) {
            if (deviceDescriptions.get(cameraId).getParameters() != null) {
                notifyApi1PluginFeatureLoadDone(cameraId);
            }
        }
    }

    private void postNotifiedBuildInFeatureLoadDone(FeatureLoadDoneListener loadDoneListener) {
        //Sync main thread/load thread/device handler thread, or load done listener
        //could not work if there is no content in mNotifiedApi1BuildInCameraIds.
        synchronized(mNotifyApi1Sync) {
            // Post API1
            for (String cameraId : mNotifiedApi1BuildInCameraIds) {
                loadDoneListener.onBuildInLoadDone(cameraId, CameraApi.API1);
            }
            // Post API2
            DeviceSpec deviceSpec = CameraApiHelper.getDeviceSpec(mActivity);
            if (deviceSpec == null || mBuildInEntries.size() <= 0) {
                return;
            }
            Collection<String> cameraIds = deviceSpec.getDeviceDescriptionMap().keySet();
            for (String cameraId : cameraIds) {
                loadDoneListener.onBuildInLoadDone(cameraId, CameraApi.API2);
            }
        }
    }

    private void postNotifiedPluginFeatureLoadDone(FeatureLoadDoneListener loadDoneListener) {
        // Post API1
        for (String cameraId : mNotifiedApi1PluginCameraIds) {
            loadDoneListener.onPluginLoadDone(cameraId, CameraApi.API1);
        }
        // Post API2
        DeviceSpec deviceSpec = CameraApiHelper.getDeviceSpec(mActivity);
        if (deviceSpec == null || mPluginEntries.size() <= 0) {
            return;
        }
        Collection<String> cameraIds = deviceSpec.getDeviceDescriptionMap().keySet();
        for (String cameraId : cameraIds) {
            loadDoneListener.onPluginLoadDone(cameraId, CameraApi.API2);
        }
    }

    private void notifyApi1BuildInFeatureLoadDone(String cameraId) {
        for (FeatureLoadDoneListener featureLoadDoneListener : mFeatureLoadDoneListeners) {
            featureLoadDoneListener.onBuildInLoadDone(cameraId, CameraApi.API1);
        }
        if (!mNotifiedApi1BuildInCameraIds.contains(cameraId)) {
            mNotifiedApi1BuildInCameraIds.add(cameraId);
        }
    }

    private void notifyApi1PluginFeatureLoadDone(String cameraId) {
        for (FeatureLoadDoneListener featureLoadDoneListener : mFeatureLoadDoneListeners) {
            featureLoadDoneListener.onPluginLoadDone(cameraId, CameraApi.API1);
        }
        if (!mNotifiedApi1PluginCameraIds.contains(cameraId)) {
            mNotifiedApi1PluginCameraIds.add(cameraId);
        }
    }

    private void notifyAllApi2BuildInFeatureLoadDone() {
        if (mBuildInEntries.size() <= 0) {
            return;
        }
        DeviceSpec deviceSpec = CameraApiHelper.getDeviceSpec(mActivity);
        Collection<String> cameraIds = deviceSpec.getDeviceDescriptionMap().keySet();
        for (String cameraId : cameraIds) {
            for (FeatureLoadDoneListener featureLoadDoneListener : mFeatureLoadDoneListeners) {
                featureLoadDoneListener.onBuildInLoadDone(cameraId, CameraApi.API2);
            }
        }
    }

    private void notifyAllApi2PluginFeatureLoadDone() {
        if (mPluginEntries.size() <= 0) {
            return;
        }
        DeviceSpec deviceSpec = CameraApiHelper.getDeviceSpec(mActivity);
        Collection<String> cameraIds = deviceSpec.getDeviceDescriptionMap().keySet();
        for (String cameraId : cameraIds) {
            for (FeatureLoadDoneListener featureLoadDoneListener : mFeatureLoadDoneListeners) {
                featureLoadDoneListener.onPluginLoadDone(cameraId, CameraApi.API2);
            }
        }
    }
}
