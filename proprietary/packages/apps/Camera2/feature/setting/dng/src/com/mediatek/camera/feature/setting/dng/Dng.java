/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
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
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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

package com.mediatek.camera.feature.setting.dng;

import android.annotation.TargetApi;
import android.content.ContentValues;
import android.location.Location;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Size;

import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;
import com.mediatek.camera.common.storage.MediaSaver;
import com.mediatek.camera.common.utils.CameraUtil;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;


/**
 * This class is for Dng feature interacted with others.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class Dng extends SettingBase implements IAppUiListener.OnShutterButtonListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(Dng.class.getSimpleName());

    private static final String KEY_CSHOT = "key_continuous_shot";
    private static final String KEY_HDR = "key_hdr";
    private static final int KEY_DNG_SHUTTER_PRIORITY = 40;
    private static final int MSG_MODE_ON_SHUTTER_BUTTON_CLICK = 0;
    private static final int MSG_MODE_ON_ITEM_VIEW_CLICK = 1;

    private long mShutterDateTaken;
    private int mImageOrientation = -1;

    // View control
    private DngViewCtrl mDngViewCtrl = new DngViewCtrl();
    private List<String> mSupportValues = new ArrayList<String>();
    // Device control
    private DngDeviceCtrl mDngDeviceCtrl = new DngDeviceCtrl();
    private ISettingChangeRequester mSettingChangeRequester;
    private ICameraContext mCameraContext;
    private IApp mApp;
    private volatile boolean mIsDngCreatorBusy;
    private Handler mModeHandler;


    /**
     * Initialize setting. This will be called when do open camera.
     *
     * @param app               the instance of IApp.
     * @param cameraContext     the CameraContext.
     * @param settingController the SettingController.
     */
    public void init(IApp app,
                     ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        mApp = app;
        mCameraContext = cameraContext;
        mModeHandler = new ModeHandler(Looper.myLooper());
        mDngViewCtrl.init(app);
        mDngViewCtrl.setDngSettingViewListener(mDngSettingViewListener);
        mDngDeviceCtrl.setDngValueUpdateListener(mDngValueListener);
        mAppUi.registerOnShutterButtonListener(this, KEY_DNG_SHUTTER_PRIORITY);
        initSettingValue();
    }

    @Override
    public void unInit() {
        mDngViewCtrl.showDngIndicatorView(false);
        mAppUi.unregisterOnShutterButtonListener(this);
    }
    @Override
    public void setSettingDeviceRequester(
            @Nonnull ISettingManager.SettingDeviceRequester settingDeviceRequester,
            @Nonnull ISettingManager.SettingDevice2Requester settingDevice2Requester) {
        super.setSettingDeviceRequester(settingDeviceRequester, settingDevice2Requester);
        mDngViewCtrl.setSettingDeviceRequest(settingDevice2Requester);

    }
    @Override
    public void addViewEntry() {
        LogHelper.d(TAG, "[addViewEntry]");
        mAppUi.addSettingView(mDngViewCtrl.getDngSettingView());
    }

    @Override
    public void removeViewEntry() {
        LogHelper.d(TAG, "[removeViewEntry]");
        mAppUi.removeSettingView(mDngViewCtrl.getDngSettingView());
    }

    @Override
    public void refreshViewEntry() {
        if (getEntryValues().size() > 1) {
            LogHelper.d(TAG, "[refreshViewEntry], enable");
            initDngResBySwitch(true);
            mDngViewCtrl.setEntryValue(getEntryValues());
            mDngViewCtrl.setEnabled(true);
            mDngDeviceCtrl.setDngStatus(getValue(), false);
        } else {
            LogHelper.d(TAG, "[refreshViewEntry], disable");
            initDngResBySwitch(false);
            mDngViewCtrl.setEnabled(false);
            mDngDeviceCtrl.setDngStatus(IDngConfig.DNG_OFF, false);
        }
    }

    @Override
    public boolean onShutterButtonFocus(boolean pressed) {
        return false;
    }

    @Override
    public boolean onShutterButtonClick() {
        if (mIsDngCreatorBusy) {
            LogHelper.d(TAG, "[onShutterButtonClick] dng creator busy return");
            return true;
        }
        mModeHandler.sendEmptyMessage(MSG_MODE_ON_SHUTTER_BUTTON_CLICK);
        return false;
    }

    @Override
    public boolean onShutterButtonLongPressed() {
        return false;
    }

    @Override
    public void postRestrictionAfterInitialized() {
        Relation relation = DngRestriction.getRestriction().getRelation(getValue(), false);
        LogHelper.d(TAG, "[postRestrictionAfterInitialized] value = " + getValue());
        if (relation != null) {
            mSettingController.postRestriction(relation);
        }
    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO;
    }

    @Override
    public String getKey() {
        return IDngConfig.KEY_DNG;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        if (mSettingChangeRequester == null) {
            mSettingChangeRequester
                    = mDngDeviceCtrl.getParametersConfigure(mSettingDeviceRequester);
        }
        return (DngParameterConfig) mSettingChangeRequester;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mSettingChangeRequester == null) {
            mSettingChangeRequester
                    = mDngDeviceCtrl.getCaptureRequestConfigure(mSettingDevice2Requester);
        }
        mDngDeviceCtrl.setDngStatus(getValue(), false);
        return (DngCaptureRequestConfig) mSettingChangeRequester;
    }

    @Override
    public PreviewStateCallback getPreviewStateCallback() {
        return mPreviewStateCallback;
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        String lastValue = getValue();
        super.overrideValues(headerKey, currentValue, supportValues);
        String curValue = getValue();
        boolean isOn = IDngConfig.DNG_ON.equals(curValue);
        LogHelper.d(TAG, "[overrideValues], headerKey:" + headerKey
                            + ", currentValue:" + currentValue
                            + ", supportValues:" + supportValues
                            + ", isOn:" + isOn);
        if (!lastValue.equals(curValue)) {
            // Continuous shot override hdr and dng off, while hdr and dng are
            // mutually exclusive, so when starting continuous shot, the dng will
            // be off and the override that it is acted on hdr will be clean. After
            // continuous shot stopped, the dng will be overridden as off by hdr.
            // So keep the override that dng acts on hdr during continuous shot. It
            // is same to hdr.
            if (!KEY_CSHOT.equals(headerKey)) {
                mSettingController.postRestriction(
                        DngRestriction.getRestriction().getRelation(curValue, true));
            }
        }
        mDngDeviceCtrl.setDngStatus(curValue, false);
        mDngDeviceCtrl.notifyOverrideValue(curValue);
        mDngViewCtrl.setEntryValue(getEntryValues());
        mDngViewCtrl.showDngIndicatorView(isOn);
        mDngViewCtrl.updateDngView();
    }

    private void initSettingValue() {
        mSupportValues.add(IDngConfig.DNG_OFF);
        mSupportValues.add(IDngConfig.DNG_ON);
        setSupportedPlatformValues(mSupportValues);
        setSupportedEntryValues(mSupportValues);
        setEntryValues(mSupportValues);
        setValue(getCachedValue());
        mDngDeviceCtrl.setDngStatus(getValue(), false);
    }

    private MediaSaver.MediaSaverListener mMediaSaverListener
            = new MediaSaver.MediaSaverListener() {

        @Override
        public void onFileSaved(Uri uri) {
            LogHelper.d(TAG, "[onFileSaved] uri = " + uri);
        }
    };

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        LogHelper.d(TAG, "onModeOpened modeKey " + modeKey);
        if (ICameraMode.ModeType.VIDEO == modeType) {
            List<String> supportValues = new ArrayList<>();
            supportValues.add(IDngConfig.DNG_OFF);
            overrideValues(modeKey, IDngConfig.DNG_OFF, supportValues);
        }
    }

    @Override
    public void onModeClosed(String modeKey) {
        LogHelper.d(TAG, "onModeClosed modeKey :" + modeKey);
        super.onModeClosed(modeKey);
        //it restore the value in overrideValues.
        mDngDeviceCtrl.onModeClosed();
    }

    private ICameraSetting.PreviewStateCallback mPreviewStateCallback =
            new ICameraSetting.PreviewStateCallback() {

                @Override
                public void onPreviewStopped() {
                }

                @Override
                public void onPreviewStarted() {
                    boolean needOn = needDngStart(getCachedValue());
                    LogHelper.d(TAG, "[onPreviewStarted] needOn: " + needOn);
                    mDngDeviceCtrl.setDngStatus(needOn ? IDngConfig.DNG_ON : IDngConfig.DNG_OFF,
                            false);
                    requestDng();
                }
            };

    private IDngConfig.OnDngValueUpdateListener mDngValueListener
            = new IDngConfig.OnDngValueUpdateListener() {

        @Override
        public void onDngValueUpdate(List<String> dngList, boolean isSupport) {
            LogHelper.d(TAG, "[onDngValueUpdate] isSupport: " + isSupport);
            setSupportedPlatformValues(dngList);
            setSupportedEntryValues(dngList);
            setEntryValues(dngList);
            if (isSupport) {
                setValue(getCachedValue());
            } else {
                setValue(IDngConfig.DNG_OFF);
            }
        }

        @Override
        public void onSaveDngImage(byte[] data, Size rawSize) {
            ContentValues rawContentValues = updateRawCaptureContentValues(rawSize.getWidth(),
                    rawSize.getHeight(), mImageOrientation);
            mCameraContext.getMediaSaver().addSaveRequest(data, rawContentValues, null,
                    mMediaSaverListener);
        }

        @Override
        public int onDisplayOrientationUpdate() {
            //it is called after capture
            return mImageOrientation;
        }

        @Override
        public void onDngCreatorStateUpdate(boolean isBusy) {
            mIsDngCreatorBusy = isBusy;
        }
    };

    private ContentValues updateRawCaptureContentValues(int width, int height, int orientation) {
        String fileDirectory = mCameraContext.getStorageService().getFileDirectory();
        return DngUtils.getContentValue(mShutterDateTaken, fileDirectory,
                width, height, orientation);
    }

    private DngViewCtrl.OnDngSettingViewListener mDngSettingViewListener
            = new DngViewCtrl.OnDngSettingViewListener() {
        @Override
        public void onItemViewClick(boolean isOn) {
            mModeHandler.removeMessages(MSG_MODE_ON_ITEM_VIEW_CLICK);
            mModeHandler.obtainMessage(MSG_MODE_ON_ITEM_VIEW_CLICK, isOn).sendToTarget();
        }

        @Override
        public boolean onCachedValue() {
            return getCachedValue().equals(IDngConfig.DNG_ON);
        }

        @Override
        public boolean onUpdatedValue() {
            return getValue().equals(IDngConfig.DNG_ON);
        }
    };

    private String getCachedValue() {
        String value = mDataStore.getValue(getKey(), IDngConfig.DNG_OFF,
                getStoreScope());
        LogHelper.d(TAG, "[getCachedValue] value = " + value);
        return value;
    }

    private int updateDisplayOrientation(int orientation) {
        int cameraId = Integer.valueOf(mSettingController.getCameraId());
        LogHelper.i(TAG, "[updateDisplayOrientation] cameraId = " + cameraId);
        return CameraUtil.getJpegRotation(cameraId, orientation, mApp.getActivity());
    }

    private void requestDng() {
        if (mSettingChangeRequester != null) {
            mSettingChangeRequester.sendSettingChangeRequest();
        }
    }

    private void requestChangeOverrideValues() {
        if (mSettingChangeRequester != null) {
            mDngDeviceCtrl.requestChangeOverrideValues();
        }
    }


    private boolean needDngStart(String referenceValue) {
        if (!IDngConfig.DNG_ON.equals(getValue())) {
            return false;
        } else if (IDngConfig.DNG_OFF.equals(referenceValue)) {
            return false;
        } else {
            return true;
        }
    }

    private void initDngResBySwitch(boolean isOn) {
        if (!isOn) {
            mDngViewCtrl.showDngIndicatorView(isOn);
            LogHelper.d(TAG, "[initDngResBySwitch] is off");
            return;
        }
        boolean needOn = needDngStart(getCachedValue());
        mDngViewCtrl.showDngIndicatorView(needOn);
    }

    /**
     * Mode handler run in mode thread.
     */
    private class ModeHandler extends Handler {
        public ModeHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_MODE_ON_SHUTTER_BUTTON_CLICK:
                    doShutterButtonClick();
                    break;

                case MSG_MODE_ON_ITEM_VIEW_CLICK:
                    Boolean isOn = (Boolean) msg.obj;
                    doItemViewClick(isOn.booleanValue());
                    break;

                default:
                    break;
            }
        }
    }

    private void doShutterButtonClick() {
        String curValue = getValue();
        LogHelper.d(TAG, "[onShutterButtonClick] value = " + curValue);
        if (IDngConfig.DNG_ON.equals(curValue)) {
            mImageOrientation = updateDisplayOrientation(mApp.getGSensorOrientation());
            mShutterDateTaken = System.currentTimeMillis();
            mDngDeviceCtrl.setDngStatus(curValue, true);
        }
    }

    private void doItemViewClick(boolean isOn) {
        LogHelper.i(TAG, "[onItemViewClick], isOn:" + isOn);
        String value = isOn ? IDngConfig.DNG_ON : IDngConfig.DNG_OFF;
        removeOverride(KEY_HDR);
        setValue(value);
        mDngDeviceCtrl.setDngStatus(getValue(), false);
        mDataStore.setValue(getKey(), value, getStoreScope(), false, true);
        mSettingController.postRestriction(
                DngRestriction.getRestriction().getRelation(value, true));
        mSettingController.refreshViewEntry();
        mAppUi.refreshSettingView();
        requestChangeOverrideValues();
        mDngViewCtrl.showDngIndicatorView(isOn);
        requestDng();
    }
}
