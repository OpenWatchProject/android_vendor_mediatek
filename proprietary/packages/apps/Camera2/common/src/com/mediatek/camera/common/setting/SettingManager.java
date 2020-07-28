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

import android.annotation.TargetApi;
import android.app.Activity;
import android.hardware.Camera.Parameters;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureFailure;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.OutputConfiguration;
import android.os.Build;
import android.view.Surface;

import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.CameraSysTrace;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.debug.profiler.IPerformanceProfile;
import com.mediatek.camera.common.debug.profiler.PerformanceTracker;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.loader.FeatureProvider;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.RestrictionDispatcher;
import com.mediatek.camera.common.relation.StatusMonitor;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * <p>This class plays a service role, through it, feature can get other features' info
 * and post restriction to change the entry values of other features.</p>
 *
 * <p>This class is bound to camera device, which means every camera device has one
 * instance of this class.</p>
 */
public class SettingManager implements ISettingManager,
                            ISettingManager.SettingController,
                            ISettingManager.SettingDeviceConfigurator,
                            ISettingManager.SettingDevice2Configurator {
    private static final String MAIN_THREAD = "main";
    private Tag mTag;
    private final SettingTable mSettingTable = new SettingTable();
    private final StatusMonitor mStatusMonitor = new StatusMonitor();
    private final RestrictionDispatcher mRestrictionDispatcher
            = new RestrictionDispatcher(mSettingTable);

    private SettingDeviceRequesterProxy mSettingDeviceRequesterProxy;
    private SettingDevice2RequesterProxy mSettingDevice2RequesterProxy;

    private ICameraMode.ModeType mModeType;
    private String mCameraId;
    private ICameraContext mCameraContext;
    private IApp mApp;
    private IAppUi mAppUi;
    private Activity mActivity;
    private CameraApi mCameraApi;
    private CameraCaptureSession.CaptureCallback mCaptureCallback;
    private HashMap<String, ICameraMode.ModeType> mPendingBindModeEvents = new HashMap<>();
    private Object mBindModeEventLock = new Object();
    private SettingAccessManager mSettingAccessManager = new SettingAccessManager();
    private boolean mInitialized = false;

    /**
     * Initialize SettingManager, this will be called before open camera.
     *
     * @param cameraId The id of camera that setting manager binds to.
     * @param app the instance of IApp.
     * @param cameraContext the CameraContext instance.
     * @param currentCameraApi current camera api.
     */
    public void init(String cameraId, IApp app,
                     ICameraContext cameraContext, CameraApi currentCameraApi) {
        mTag = new Tag(SettingManager.class.getSimpleName() + "-" + cameraId);
        LogHelper.i(mTag, "[init]+");
        mCameraId = cameraId;
        mApp = app;
        mCameraContext = cameraContext;
        mCameraApi = currentCameraApi;
        mActivity = app.getActivity();
        mAppUi = app.getAppUi();

        if (CameraApi.API1 == currentCameraApi) {
            mSettingDeviceRequesterProxy = new SettingDeviceRequesterProxy();
        } else if (CameraApi.API2 == currentCameraApi) {
            mSettingDevice2RequesterProxy = new SettingDevice2RequesterProxy();
        }
        mInitialized = true;
        LogHelper.i(mTag, "[init]-");
    }

    @Override
    public void createSettingsByStage(int stage) {
        LogHelper.d(mTag, "[createSettingsByStage]+, stage:" + stage
                + ", mInitialized:" + mInitialized);
        if (!mInitialized) {
            return;
        }
        FeatureProvider provider = mCameraContext.getFeatureProvider();
        List<ICameraSetting> settings =
                (List<ICameraSetting>)
                        provider.getInstancesByStage(ICameraSetting.class, mCameraApi, stage);

        String modeKey = null;
        ICameraMode.ModeType modeType = null;
        synchronized (mBindModeEventLock) {
            if (mPendingBindModeEvents.size() > 0) {
                modeKey = mPendingBindModeEvents.keySet().iterator().next();
                modeType = mPendingBindModeEvents.get(modeKey);
            }
        }
        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("createSettingsByStage" + stage + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            LogHelper.d(mTag, "[createSettingsByStage], access active failed, return");
            return;
        }
        if (!mInitialized) {
            LogHelper.d(mTag, "[createSettingsByStage], setting is uninitialized, return");
            mSettingAccessManager.recycleAccess(access);
            return;
        }
        for (ICameraSetting setting : settings) {
            CameraSysTrace.onEventSystrace("createSettingsByStage:" + setting, true);
            if (!access.isValid()) {
                break;
            }
            setting.init(mApp, mCameraContext, this);
            mSettingTable.add(setting);
            setting.setSettingDeviceRequester(mSettingDeviceRequesterProxy,
                    mSettingDevice2RequesterProxy);
            if (modeKey != null && modeType != null) {
                setting.onModeOpened(modeKey, modeType);
            }
            CameraSysTrace.onEventSystrace("createSettingsByStage:" + setting, false);
        }
        if (access.isValid()) {
            mSettingTable.classify(mCameraApi);
        }
        mSettingAccessManager.recycleAccess(access);
        LogHelper.d(mTag, "[createSettingsByStage]-");
    }

    @Override
    public void createAllSettings() {
        LogHelper.d(mTag, "[createAllSettings]+, mInitialized:" + mInitialized);
        if (!mInitialized) {
            return;
        }
        FeatureProvider provider = mCameraContext.getFeatureProvider();
        List<ICameraSetting> settings =
                (List<ICameraSetting>)
                        provider.getAllBuildInInstance(ICameraSetting.class, mCameraApi);
        if (settings.size() == 0) {
            LogHelper.d(mTag, "[createAllSettings], there is no setting created, so return");
            return;
        }

        List<ICameraSetting> hasCreatedSettings = mSettingTable.getAllSettings();
        if (hasCreatedSettings.size() > 0) {
            for (int i = 0; i < hasCreatedSettings.size(); i++) {
                for (int j = 0; j < settings.size(); j++) {
                    if (hasCreatedSettings.get(i).getKey()
                            .equals(settings.get(j).getKey())) {
                        settings.remove(j);
                        j--;
                        break;
                    }
                }
            }
        }
        if (settings.size() == 0) {
            LogHelper.d(mTag, "[createAllSettings], setting has created, so return");
            return;
        }

        String modeKey = null;
        ICameraMode.ModeType modeType = null;
        synchronized (mBindModeEventLock) {
            if (mPendingBindModeEvents.size() > 0) {
                modeKey = mPendingBindModeEvents.keySet().iterator().next();
                modeType = mPendingBindModeEvents.get(modeKey);
            }
        }

        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("createAllSettings" + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            LogHelper.d(mTag, "[createAllSettings], access active failed, return");
            return;
        }
        if (!mInitialized) {
            LogHelper.d(mTag, "[createAllSettings], setting is uninitialized, return");
            mSettingAccessManager.recycleAccess(access);
            return;
        }
        for (ICameraSetting setting : settings) {
            CameraSysTrace.onEventSystrace("createAllSettings:" + setting, true);
            if (!access.isValid()) {
                break;
            }
            setting.init(mApp, mCameraContext, this);
            mSettingTable.add(setting);
            setting.setSettingDeviceRequester(mSettingDeviceRequesterProxy,
                    mSettingDevice2RequesterProxy);
            if (modeKey != null && modeType != null) {
                setting.onModeOpened(modeKey, modeType);
            }
            CameraSysTrace.onEventSystrace("createAllSettings:" + setting, false);
        }
        if (access.isValid()) {
            mSettingTable.classify(mCameraApi);
        }
        mSettingAccessManager.recycleAccess(access);
        LogHelper.d(mTag, "[createAllSettings]-");
    }

    /**
     * Un-initialize SettingManager, this will be called before close camera.
     */
    public void unInit() {
        LogHelper.i(mTag, "[unInit]+, mInitialized:" + mInitialized);
        mInitialized = false;
        if (mSettingDeviceRequesterProxy != null) {
            mSettingDeviceRequesterProxy.unInit();
        }
        mSettingAccessManager.startControl();
        List<ICameraSetting> settings = mSettingTable.getAllSettings();
        for (ICameraSetting setting : settings) {
            setting.removeViewEntry();
            setting.unInit();
        }
        mSettingTable.removeAll();
        mSettingAccessManager.stopControl();
        LogHelper.i(mTag, "[unInit]-");
    }

    /**
     * This setting manager bind to one mode. This method should be called
     * after mode get setting manager, like in mode initialization stage or
     * after switching camera in mode.
     *
     * @param modeKey The key to indicator mode.
     * @param modeType The type of mode.
     */
    public void bindMode(String modeKey, ICameraMode.ModeType modeType) {
        LogHelper.d(mTag, "[bindMode] modeKey:" + modeKey + ", modeType:" + modeType);
        mModeType = modeType;
        List<ICameraSetting> settings = mSettingTable.getAllSettings();
        if (settings == null || settings.size() == 0) {
            synchronized (mBindModeEventLock) {
                mPendingBindModeEvents.put(modeKey, modeType);
            }
            return;
        }
        for (ICameraSetting setting : settings) {
            setting.onModeOpened(modeKey, modeType);
        }
    }

    /**
     * This setting manager unbind to one mode, and in this method setting manager
     * will clear the restriction of this mode. This method should be called in
     * mode un-initialization stage.
     *
     * @param modeKey The key to indicator mode.
     */
    public void unbindMode(String modeKey) {
        LogHelper.d(mTag, "[unbindMode] modeKey:" + modeKey);
        List<ICameraSetting> settings = mSettingTable.getAllSettings();
        for (ICameraSetting setting : settings) {
            setting.onModeClosed(modeKey);
        }
        synchronized (mBindModeEventLock) {
            mPendingBindModeEvents.clear();
        }
    }

    @Override
    public String getCameraId() {
        return mCameraId;
    }

    @Override
    public String queryValue(String key) {
        ICameraSetting cameraSetting = mSettingTable.get(key);
        if (cameraSetting != null) {
            return cameraSetting.getValue();
        }
        return null;
    }

    @Override
    public List<String> querySupportedPlatformValues(String key) {
        ICameraSetting cameraSetting = mSettingTable.get(key);
        if (cameraSetting != null) {
            return cameraSetting.getSupportedPlatformValues();
        }
        return null;
    }

    @Override
    public void postRestriction(Relation relation) {
        LogHelper.d(mTag, "[postRestriction], " + relation.getHeaderKey()
                + ":" + relation.getHeaderValue() + " post relation.");
        if (!mInitialized) {
            return;
        }
        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("postRestriction" + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            return;
        }
        mRestrictionDispatcher.dispatch(relation);
        mSettingAccessManager.recycleAccess(access);
    }

    @Override
    public void addViewEntry() {
        LogHelper.d(mTag, "[addViewEntry], mInitialized:" + mInitialized);
        if (!mInitialized) {
            return;
        }
        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("addViewEntry" + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            return;
        }

        List<ICameraSetting> settingsRelatedToMode = getSettingByModeType(mModeType);
        for (ICameraSetting setting : settingsRelatedToMode) {
            if (!access.isValid()) {
                break;
            }
            setting.addViewEntry();
        }

        List<ICameraSetting> settingsUnRelatedToMode = getSettingByModeType(
                mModeType == ICameraMode.ModeType.PHOTO ? ICameraMode.ModeType.VIDEO
                        : ICameraMode.ModeType.PHOTO);

        List<ICameraSetting> settingsForPhotoAndVideo
                = mSettingTable.getSettingListByType(ICameraSetting.SettingType.PHOTO_AND_VIDEO);
        settingsUnRelatedToMode.removeAll(settingsForPhotoAndVideo);
        for (ICameraSetting setting : settingsUnRelatedToMode) {
            if (!access.isValid()) {
                break;
            }
            setting.removeViewEntry();
        }
        mSettingAccessManager.recycleAccess(access);
        mAppUi.registerQuickIconDone();
        mAppUi.attachEffectViewEntry();
    }

    @Override
    public void removeViewEntry() {
        LogHelper.d(mTag, "[removeViewEntry], mInitialized:" + mInitialized);
        if (!mInitialized) {
            return;
        }
        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("removeViewEntry" + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            return;
        }

        List<ICameraSetting> settings = getSettingByModeType(mModeType);
        for (ICameraSetting setting : settings) {
            if (!access.isValid()) {
                break;
            }
            setting.removeViewEntry();
        }
        mSettingAccessManager.recycleAccess(access);
    }

    @Override
    public void refreshViewEntry() {
        LogHelper.d(mTag, "[refreshViewEntry], mInitialized:" + mInitialized);
        if (!mInitialized) {
            return;
        }

        final List<ICameraSetting> settings = getSettingByModeType(mModeType);
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                SettingAccessManager.Access access = mSettingAccessManager
                        .getAccess("refreshViewEntry" + this.hashCode());
                boolean successful = mSettingAccessManager.activeAccess(access);
                if(mInitialized && successful) {
                    for (ICameraSetting setting : settings) {
                        if (!access.isValid()) {
                            break;
                        }
                        setting.refreshViewEntry();
                    }
                    mAppUi.updateSettingIconVisibility();
                }
                mSettingAccessManager.recycleAccess(access);
            }
        });
    }

    @Override
    public StatusMonitor getStatusMonitor() {
        return mStatusMonitor;
    }

    @Override
    public void registerSettingItem(ICameraSetting setting) {
        mSettingTable.add(setting);
    }

    @Override
    public void unRegisterSettingItem(ICameraSetting setting) {
        mSettingTable.remove(setting);
    }

    @Override
    public void onPreviewStopped() {
        LogHelper.d(mTag, "[onPreviewStopped], mInitialized:" + mInitialized);
        if (!mInitialized) {
            return;
        }

        IPerformanceProfile pProfile = PerformanceTracker.create(mTag, "onPreviewStopped").start();

        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("onPreviewStopped" + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            return;
        }
        List<ICameraSetting> settings = getSettingByModeType(mModeType);
        for (ICameraSetting setting : settings) {
            if (!access.isValid()) {
                break;
            }
            if (setting.getPreviewStateCallback() != null) {
                setting.getPreviewStateCallback().onPreviewStopped();
            }
        }
        mSettingAccessManager.recycleAccess(access);

        pProfile.stop();
    }

    @Override
    public void onPreviewStarted() {
        if (!mInitialized) {
            return;
        }
        IPerformanceProfile pProfile = PerformanceTracker.create(mTag, "onPreviewStarted").start();

        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("onPreviewStarted" + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            return;
        }

        List<ICameraSetting> settings = getSettingByModeType(mModeType);
        for (ICameraSetting setting : settings) {
            if (!access.isValid()) {
                break;
            }
            if (setting.getPreviewStateCallback() != null) {
                setting.getPreviewStateCallback().onPreviewStarted();
            }
        }
        mSettingAccessManager.recycleAccess(access);

        pProfile.stop();
    }

    @Override
    public void updateModeDeviceRequester(@Nonnull SettingDeviceRequester settingDeviceRequester) {
        mSettingDeviceRequesterProxy.updateModeDeviceRequester(settingDeviceRequester);
    }

    @Override
    public void updateModeDevice2Requester(
            @Nonnull SettingDevice2Requester settingDevice2Requester) {
        mSettingDevice2RequesterProxy.updateModeDevice2Requester(settingDevice2Requester);
    }

    @Override
    public void updateModeDeviceStateToSetting(String modeName, String newState) {
        LogHelper.d(mTag,
                "[updateModeDeviceStateToSetting] mode:" + modeName + ",state:" + newState);
        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("updateModeDeviceState" + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            return;
        }

        List<ICameraSetting> settings = getSettingByModeType(mModeType);
        for (ICameraSetting setting : settings) {
            if (!access.isValid()) {
                break;
            }
            setting.updateModeDeviceState(newState);
        }
        mSettingAccessManager.recycleAccess(access);
    }

    @Override
    public SettingDeviceConfigurator getSettingDeviceConfigurator() {
        return this;
    }

    @Override
    public SettingDevice2Configurator getSettingDevice2Configurator() {
        return this;
    }

    @Override
    public OutputConfiguration getRawOutputConfiguration() {
        OutputConfiguration rawConfig = null;
        ICameraSetting setting = mSettingTable.get("key_dng");
        if (setting == null) {
        	return null;
        }
        Surface rawSurface = setting.getCaptureRequestConfigure().configRawSurface();
        if (rawSurface != null) {
        	rawConfig = new OutputConfiguration(rawSurface);
        }
        return rawConfig;
    }

    @SuppressWarnings("deprecation")
    @Override
    public void setOriginalParameters(Parameters originalParameters) {
        LogHelper.d(mTag, "[setOriginalParameters]+, mInitialized:" + mInitialized);
        if (!mInitialized) {
            return;
        }
        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("setOriginalParameters" + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            return;
        }

        List<ICameraSetting> settings = getSettingByModeType(mModeType);
        settings.retainAll(mSettingTable.getAllConfigParametersSettings());
        for (ICameraSetting setting : settings) {
            if (!access.isValid()) {
                break;
            }
            setting.getParametersConfigure().setOriginalParameters(originalParameters);
        }
        DataStore dataStore = mCameraContext.getDataStore();
        List<String> keys = dataStore.getSettingsKeepSavingTime(Integer.parseInt(mCameraId));
        for (int i = 0; i < keys.size(); i++) {
            ICameraSetting setting = mSettingTable.get(keys.get(i));
            boolean removed = settings.remove(setting);
            if (removed) {
                settings.add(setting);
            }
        }
        for (ICameraSetting setting : settings) {
            if (!access.isValid()) {
                break;
            }
            setting.postRestrictionAfterInitialized();
        }
        mSettingAccessManager.recycleAccess(access);
        LogHelper.d(mTag, "[setOriginalParameters]-");
    }

    @SuppressWarnings("deprecation")
    @Override
    public boolean configParameters(Parameters parameters) {
        LogHelper.d(mTag, "[configParameters]+, mInitialized:" + mInitialized);
        if (!mInitialized) {
            return false;
        }

        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("configParameters" + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            return false;
        }

        List<ICameraSetting> settings = getSettingByModeType(mModeType);
        settings.retainAll(mSettingTable.getAllConfigParametersSettings());
        boolean needRestartPreview = false;
        for (ICameraSetting setting : settings) {
            if (!access.isValid()) {
                break;
            }
            boolean restartPreview = setting.getParametersConfigure()
                    .configParameters(parameters);
            if (restartPreview) {
                LogHelper.d(mTag, "[configParameters], need restart preview:" + setting.getKey());
            }
            needRestartPreview = restartPreview || needRestartPreview;
        }
        mSettingAccessManager.recycleAccess(access);
        LogHelper.d(mTag, "[configParameters]-");
        return needRestartPreview;
    }


    @Override
    public boolean configParametersByKey(Parameters parameters, String key) {
        LogHelper.d(mTag, "[configParametersByKey]+, mInitialized:" + mInitialized);
        if (!mInitialized) {
            return false;
        }

        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("configParameters" + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            return false;
        }

        ICameraSetting setting = mSettingTable.get(key);
        boolean needRestartPreview = setting.getParametersConfigure()
                .configParameters(parameters);
        mSettingAccessManager.recycleAccess(access);
        LogHelper.d(mTag, "[configParameters]-");
        return needRestartPreview;
    }

    @Override
    public void configCommand(String key, CameraProxy cameraProxy) {
        LogHelper.d(mTag, "[configCommand] key:" + key);
        ICameraSetting setting = mSettingTable.get(key);
        if (setting != null) {
            setting.getParametersConfigure().configCommand(cameraProxy);
        }
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        LogHelper.d(mTag, "[setCameraCharacteristics]+, mInitialized:" + mInitialized);
        if (!mInitialized) {
            return;
        }
        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("setCameraCharacteristics" + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            return;
        }

        List<ICameraSetting> settings = getSettingByModeType(mModeType);
        settings.retainAll(mSettingTable.getAllCaptureRequestSettings());
        for (ICameraSetting setting : settings) {
            CameraSysTrace.onEventSystrace("setCameraCharacteristics:" + setting, true);
            if (!access.isValid()) {
                break;
            }
            ICameraSetting.ICaptureRequestConfigure requestConfigure =
                    setting.getCaptureRequestConfigure();
            requestConfigure.setCameraCharacteristics(characteristics);
            CameraSysTrace.onEventSystrace("setCameraCharacteristics:" + setting, false);
        }

        DataStore dataStore = mCameraContext.getDataStore();
        List<String> keys = dataStore.getSettingsKeepSavingTime(Integer.parseInt(mCameraId));
        for (int i = 0; i < keys.size(); i++) {
            ICameraSetting setting = mSettingTable.get(keys.get(i));
            boolean removed = settings.remove(setting);
            if (removed) {
                settings.add(setting);
            }
        }
        for (ICameraSetting setting : settings) {
            CameraSysTrace.onEventSystrace("setCameraCharacteristics.postRestriction:" + setting, false);
            if (!access.isValid()) {
                break;
            }
            setting.postRestrictionAfterInitialized();
            CameraSysTrace.onEventSystrace("setCameraCharacteristics.postRestriction:" + setting, false);
        }
        mSettingAccessManager.recycleAccess(access);
        LogHelper.d(mTag, "[setCameraCharacteristics]-");
    }

    @Override
    public void configCaptureRequest(@Nonnull Builder captureBuilder) {
        LogHelper.d(mTag, "[configCaptureRequest], mInitialized:" + mInitialized);
        if (!mInitialized) {
            return;
        }
        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("configCaptureRequest" + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            return;
        }
        List<ICameraSetting> settings = getSettingByModeType(mModeType);
        settings.retainAll(mSettingTable.getAllCaptureRequestSettings());
        for (ICameraSetting setting : settings) {
            if (!access.isValid()) {
                break;
            }
            ICameraSetting.ICaptureRequestConfigure requestConfigure =
                    setting.getCaptureRequestConfigure();
            requestConfigure.configCaptureRequest(captureBuilder);
        }
        mSettingAccessManager.recycleAccess(access);
    }

    @Override
    public void configSessionSurface(@Nonnull List<Surface> surfaceList) {
        LogHelper.d(mTag, "[configSessionSurface], mInitialized:" + mInitialized);
        if (!mInitialized) {
            return;
        }
        SettingAccessManager.Access access = mSettingAccessManager
                .getAccess("configSessionSurface" + this.hashCode());
        boolean successful = mSettingAccessManager.activeAccess(access);
        if (!successful) {
            return;
        }
        List<ICameraSetting> settings = getSettingByModeType(mModeType);
        settings.retainAll(mSettingTable.getAllCaptureRequestSettings());
        for (ICameraSetting setting : settings) {
            if (!access.isValid()) {
                break;
            }
            ICameraSetting.ICaptureRequestConfigure requestConfigure =
                    setting.getCaptureRequestConfigure();
            requestConfigure.configSessionSurface(surfaceList);
        }
        mSettingAccessManager.recycleAccess(access);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        if (mCaptureCallback == null) {
            mCaptureCallback = new CameraCaptureSession.CaptureCallback() {
                @Override
                public void onCaptureCompleted(CameraCaptureSession session,
                                               CaptureRequest request,
                                               TotalCaptureResult result) {
                    SettingAccessManager.Access access = mSettingAccessManager
                            .getAccess("onCaptureCompleted" + this.hashCode());
                    boolean successful = mSettingAccessManager.activeAccess(access, false);
                    if (!successful) {
                        return;
                    }
                    final List<ICameraSetting> settings = getSettingByModeType(mModeType);
                    settings.retainAll(mSettingTable.getAllCaptureRequestSettings());
                    for (ICameraSetting setting : settings) {
                        if (!access.isValid()) {
                            break;
                        }
                        ICameraSetting.ICaptureRequestConfigure requestConfigure =
                                setting.getCaptureRequestConfigure();
                        if (requestConfigure.getRepeatingCaptureCallback() != null) {
                            requestConfigure.getRepeatingCaptureCallback()
                                    .onCaptureCompleted(session, request, result);
                        }
                    }
                    mSettingAccessManager.recycleAccess(access, false);
                }

                @Override
                public void onCaptureFailed(CameraCaptureSession session,
                                            CaptureRequest request,
                                            CaptureFailure failure) {
                    SettingAccessManager.Access access = mSettingAccessManager
                            .getAccess("onCaptureFailed" + this.hashCode());
                    boolean successful = mSettingAccessManager.activeAccess(access, false);
                    if (!successful) {
                        return;
                    }
                    final List<ICameraSetting> settings = getSettingByModeType(mModeType);
                    settings.retainAll(mSettingTable.getAllCaptureRequestSettings());
                    for (ICameraSetting setting : settings) {
                        if (!access.isValid()) {
                            break;
                        }
                        ICameraSetting.ICaptureRequestConfigure requestConfigure =
                                setting.getCaptureRequestConfigure();
                        if (requestConfigure.getRepeatingCaptureCallback() != null) {
                            requestConfigure.getRepeatingCaptureCallback()
                                    .onCaptureFailed(session, request, failure);
                        }
                    }
                    mSettingAccessManager.recycleAccess(access, false);
                }
            };
        }
        return mCaptureCallback;
    }

    @Override
    public SettingController getSettingController() {
        return this;
    }

    private List<ICameraSetting> getSettingByModeType(ICameraMode.ModeType modeType) {
        List<ICameraSetting> settings = new ArrayList<>();
        switch (modeType) {
            case PHOTO:
                settings = mSettingTable.getSettingListByType(ICameraSetting.SettingType.PHOTO);
                break;

            case VIDEO:
                settings = mSettingTable.getSettingListByType(ICameraSetting.SettingType.VIDEO);
                break;

            default:
                break;
        }
        return settings;
    }
}