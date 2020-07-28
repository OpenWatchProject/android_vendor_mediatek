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

package com.mediatek.camera.common.loader;

import android.content.Context;
import android.os.ConditionVariable;

import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.debug.profiler.IPerformanceProfile;
import com.mediatek.camera.common.debug.profiler.PerformanceTracker;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.plugin.PluginManager;
import com.mediatek.plugin.PluginManager.PreloaderListener;
import com.mediatek.plugin.PluginUtility;
import com.mediatek.plugin.element.Extension;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.annotation.Nonnull;

/**
 * Used for load the features.
 */
public class FeatureLoader {
    private static final Tag TAG = new Tag(FeatureLoader.class.getSimpleName());
    private static final String BUILD_IN_PATH = "buildinplugin.xml";
    private static final String FEATURE_PLUGIN_PREFIX = "String";
    private static final String XML_PARSER_FORMAT = "UTF-8";
    private static final String PLUGIN_PATH = "/system/vendor/etc/camera";
    private static PluginManager sPluginManager;
    private static ConcurrentHashMap<String, IFeatureEntry>
            sBuildInEntries = new ConcurrentHashMap<>();
    private static ConcurrentHashMap<String, IFeatureEntry>
            sPluginEntries = new ConcurrentHashMap<>();
    private static Lock sPlugInLock = new ReentrantLock();

    /**
     * Update current mode key to feature entry, dual camera zoom need to set properties
     * in photo and video mode before open camera, this notify only update to setting feature.
     * @param context current application context.
     * @param currentModeKey current mode key.
     */
    public static void updateSettingCurrentModeKey(@Nonnull Context context,
                                                   @Nonnull String currentModeKey) {
        LogHelper.d(TAG, "[updateCurrentModeKey] current mode key:" + currentModeKey);
        if (sBuildInEntries.size() <= 0) {
            loadBuildInFeatures(context);
        }
        Iterator iterator = sBuildInEntries.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry item = (Map.Entry) iterator.next();
            IFeatureEntry entry = (IFeatureEntry) item.getValue();
            if (ICameraSetting.class.equals(entry.getType())) {
                entry.updateCurrentModeKey(currentModeKey);
            }
        }
    }

    /**
     * Notify setting feature before open camera, this event only need to notify setting feature.
     * @param context the context.
     * @param cameraId want to open which camera.
     * @param cameraApi use which api.
     */
    public static void notifySettingBeforeOpenCamera(@Nonnull Context context,
                                                     @Nonnull String cameraId,
                                                     @Nonnull CameraApi cameraApi) {
        LogHelper.d(TAG, "[notifySettingBeforeOpenCamera] id:" + cameraId + ", api:" + cameraApi);
        //don't consider plugin feature? because plugin feature need more time to load
        if (sBuildInEntries.size() <= 0) {
            loadBuildInFeatures(context);
        }
        Iterator iterator = sBuildInEntries.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry item = (Map.Entry) iterator.next();
            IFeatureEntry entry = (IFeatureEntry) item.getValue();
            if (ICameraSetting.class.equals(entry.getType())) {
                entry.notifyBeforeOpenCamera(cameraId, cameraApi);
            }
        }
    }

    /**
     * Load plugin feature entries, should be called in non-ui thread.
     * @param context the application context.
     * @return the plugin features.
     */
    public static ConcurrentHashMap<String, IFeatureEntry> loadPluginFeatures(
            final Context context) {
        if (sPluginEntries.size() <= 0) {
            sPlugInLock.lock();
            try {
                if (sPluginEntries.size() <= 0) {
                    final IPerformanceProfile profile
                            = PerformanceTracker.create(TAG, "Plug-in Loading");
                    profile.start();
                    createPluginManager(context);
                    ConditionVariable loadConditionVariable = new ConditionVariable();
                    loadConditionVariable.close();
                    sPluginManager.addPluginDir(PLUGIN_PATH);
                    sPluginManager.preloadAllPlugins(true, false, true, new PreloaderListener() {
                        @Override
                        public void onPreloadFinished() {
                            profile.mark("Plugin manager preload finished.");
                            sPluginEntries = new
                                 ConcurrentHashMap<>(doLoadPluginFeatureEntries(profile, context));
                            loadConditionVariable.open();
                        }
                    });
                    loadConditionVariable.block();
                    profile.stop();
                }
            } finally {
                sPlugInLock.unlock();
            }
        }
        return sPluginEntries;
    }

    /**
     * Load build in feature entries, should be called in non-ui thread.
     * @param context the application context.
     * @return the build-in features.
     */
    public static ConcurrentHashMap<String, IFeatureEntry> loadBuildInFeatures(Context context) {
        if (sBuildInEntries.size() > 0) {
            return sBuildInEntries;
        }
        IPerformanceProfile profile = PerformanceTracker.create(TAG,
                "Build-in Loading");
        profile.start();
        createPluginManager(context);
        XmlPullParser parser = getFeaturesParse(BUILD_IN_PATH, context);
        profile.mark("getFeaturesParse");
        String[] classNames = getFeatureNames(parser);
        profile.mark("getFeaturesString");
        if (classNames == null) {
            LogHelper.e(TAG, "[loadBuildInFeatureEntries] three is no feature plug in");
            return sBuildInEntries;
        }
        sBuildInEntries = new ConcurrentHashMap<>(loadClasses(classNames, profile, context));
        profile.stop();
        return sBuildInEntries;
    }

    private synchronized static void createPluginManager(Context context) {
        if (sPluginManager == null) {
            sPluginManager = PluginManager.getInstance(context);
        }
    }

    private static LinkedHashMap<String, IFeatureEntry> doLoadPluginFeatureEntries(
            IPerformanceProfile profile, Context context) {
        LinkedHashMap<String, IFeatureEntry> entries = new LinkedHashMap<>();
        Map<String, Extension> featureEntryExtensions =
                PluginUtility.getExt(sPluginManager, FeatureEntryBase.class);
        if (featureEntryExtensions == null || featureEntryExtensions.size() == 0) {
            LogHelper.d(TAG, "[loadPluginFeatureEntries] don't have only extensions");
            return entries;
        }

        Set<String> keySet = featureEntryExtensions.keySet();
        for (String key : keySet) {
            IFeatureEntry entry = (IFeatureEntry) PluginUtility.createInstance(
                    sPluginManager,
                    featureEntryExtensions.get(key), context, context.getResources());
            if (entry != null) {
                entry.setDeviceSpec(CameraApiHelper.getDeviceSpec(context));
                profile.mark(entry.getFeatureEntryName() + " created.");
                entries.put(entry.getFeatureEntryName(), entry);
                printLog(entry);
            }
        }
        profile.stop();
        return entries;
    }

    private static XmlPullParser getFeaturesParse(String name, Context context) {
        XmlPullParser xmlPullParse = null;
        try {
            try {
                InputStream inputStream = context.getAssets().open(name);
                xmlPullParse = XmlPullParserFactory.newInstance().newPullParser();
                xmlPullParse.setInput(inputStream, XML_PARSER_FORMAT);
            } catch (XmlPullParserException e) {
                e.printStackTrace();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return xmlPullParse;
    }

    private static String[] getFeatureNames(XmlPullParser parser) {
        List<String> entries = null;
        try {
            int eventType = parser.getEventType();
            while (XmlPullParser.END_DOCUMENT != eventType) {
                switch (eventType) {
                    case XmlPullParser.START_DOCUMENT:
                        entries = new ArrayList<>();
                        break;
                    case XmlPullParser.START_TAG:
                        if (parser.getName().equalsIgnoreCase(FEATURE_PLUGIN_PREFIX)) {
                            eventType = parser.next();
                            entries.add(parser.getText());
                        }
                        break;
                    case XmlPullParser.END_TAG:
                        //Do noting.
                        break;
                    default:
                        break;
                }
                eventType = parser.next();
            }
        } catch (XmlPullParserException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        if (entries.isEmpty()) {
            return null;
        } else {
            return entries.toArray(new String[entries.size()]);
        }
    }

    private static LinkedHashMap<String, IFeatureEntry> loadClasses(String[] classNames,
                                                             IPerformanceProfile profile,
                                                                    Context context) {
        LinkedHashMap<String, IFeatureEntry> entries = new LinkedHashMap<>();
        ClassLoader loader = FeatureLoader.class.getClassLoader();
        for (String name : classNames) {
            if (name != null) {
                try {
                    Class className = loader.loadClass(name);
                    Constructor constructors = className.getConstructors()[0];
                    IFeatureEntry entry = null;
                    try {
                        entry = (IFeatureEntry)
                                constructors.newInstance(context, context.getResources());
                    } catch (InvocationTargetException e) {
                        e.printStackTrace();
                    }
                    entry.setDeviceSpec(CameraApiHelper.getDeviceSpec(context));
                    String entryName = entry.getFeatureEntryName();
                    profile.mark(entryName + " created!");
                    entries.put(entryName, entry);
                } catch (ClassNotFoundException e) {
                    e.printStackTrace();
                } catch (InstantiationException e) {
                    e.printStackTrace();
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                }
            }
        }
        return entries;
    }

    private static void printLog(IFeatureEntry featureEntry) {
        LogHelper.i(TAG, "[printLog]: featureEntry :" + featureEntry);
        IAppUi.ModeItem item = featureEntry.getModeItem();
        if (item != null) {
            StringBuilder builder = new StringBuilder();
            builder.append("Feature:")
                    .append(item.mModeName)
                    .append(",class name:")
                    .append(item.mClassName)
                    .append(",mode unselected icon:")
                    .append(item.mModeUnselectedIcon)
                    .append(",mode selected icon:")
                    .append(item.mModeSelectedIcon)
                    .append(",priority:")
                    .append(item.mPriority)
                    .append(",shutter icon: ")
                    .append(item.mShutterIcon)
                    .append(",type:")
                    .append(item.mType)
                    .append(",supported camera id:")
                    .append(item.mSupportedCameraIds.toString())
                    .toString();
            LogHelper.i(TAG, "[printLog]: " + builder);
        }
    }
}