package com.mediatek.camera.common.setting;

import android.os.Handler;
import android.os.HandlerThread;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import javax.annotation.Nonnull;

/**
 * An implementation of SettingDeviceRequester used by setting notify mode.
 */

public class SettingDeviceRequesterProxy implements ISettingManager.SettingDeviceRequester {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(SettingDeviceRequesterProxy.class.getSimpleName());

    private ISettingManager.SettingDeviceRequester mModeDeviceRequesterImpl;
    private Handler mHandler;

    public SettingDeviceRequesterProxy() {
        HandlerThread ht = new HandlerThread("API1-Setting-Change-Request-Handler");
        ht.start();
        mHandler = new Handler(ht.getLooper());
    }

    @Override
    public void requestChangeSettingValue(final String key) {
        LogHelper.d(TAG, "[requestChangeSettingValue] key:" + key);
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                synchronized (this) {
                    if (mModeDeviceRequesterImpl != null) {
                        mModeDeviceRequesterImpl.requestChangeSettingValue(key);
                    }
                }
            }
        });

    }

    @Override
    public void requestChangeSettingValueJustSelf(String key) {
        LogHelper.d(TAG, "[requestChangeSettingValueJustSelf] key:" + key);
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                synchronized (this) {
                    if (mModeDeviceRequesterImpl != null) {
                        mModeDeviceRequesterImpl.requestChangeSettingValueJustSelf(key);
                    }
                }
            }
        });
    }

    @Override
    public void requestChangeCommand(final String key) {
        LogHelper.d(TAG, "[requestChangeCommand] key:" + key);
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                synchronized (this) {
                    if (mModeDeviceRequesterImpl != null) {
                        mModeDeviceRequesterImpl.requestChangeCommand(key);
                    }
                }
            }
        });
    }

    @Override
    public void requestChangeCommandImmediately(String key) {
        requestChangeCommand(key);
    }


    public void updateModeDeviceRequester(
            @Nonnull ISettingManager.SettingDeviceRequester settingDeviceRequester) {
        synchronized (this) {
            mModeDeviceRequesterImpl = settingDeviceRequester;
        }
    }

    /**
     * unInit handler.
     */
    public void unInit() {
        mHandler.getLooper().quit();
    }
}
