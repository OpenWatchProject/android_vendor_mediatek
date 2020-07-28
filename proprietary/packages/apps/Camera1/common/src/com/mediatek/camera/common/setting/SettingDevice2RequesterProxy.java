package com.mediatek.camera.common.setting;

import android.hardware.camera2.CaptureRequest;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v2.Camera2CaptureSessionProxy;

import javax.annotation.Nonnull;

/**
 * An implementation of SettingDevice2Requester used by setting notify mode.
 */

public class SettingDevice2RequesterProxy implements ISettingManager.SettingDevice2Requester {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(SettingDevice2RequesterProxy.class.getSimpleName());

    private ISettingManager.SettingDevice2Requester mModeDevice2RequesterImpl;

    @Override
    public void createAndChangeRepeatingRequest() {
        synchronized (this) {
            if (mModeDevice2RequesterImpl != null) {
                mModeDevice2RequesterImpl.createAndChangeRepeatingRequest();
            }
        }
    }

    @Override
    public CaptureRequest.Builder createAndConfigRequest(int templateType) {
        synchronized (this) {
            if (mModeDevice2RequesterImpl != null) {
                return mModeDevice2RequesterImpl.createAndConfigRequest(templateType);
            }
        }
        return null;
    }

    @Override
    public Camera2CaptureSessionProxy getCurrentCaptureSession() {
        synchronized (this) {
            if (mModeDevice2RequesterImpl != null) {
                return mModeDevice2RequesterImpl.getCurrentCaptureSession();
            }
        }
        return null;
    }

    @Override
    public void requestRestartSession() {
        synchronized (this) {
            if (mModeDevice2RequesterImpl != null) {
                mModeDevice2RequesterImpl.requestRestartSession();
            }
        }
    }

    @Override
    public int getRepeatingTemplateType() {
        synchronized (this) {
            if (mModeDevice2RequesterImpl != null) {
                return mModeDevice2RequesterImpl.getRepeatingTemplateType();
            }
        }
        return -1;
    }

    public void updateModeDevice2Requester(
            @Nonnull ISettingManager.SettingDevice2Requester settingDevice2Requester) {
        synchronized (this) {
            mModeDevice2RequesterImpl = settingDevice2Requester;
        }
    }
}
