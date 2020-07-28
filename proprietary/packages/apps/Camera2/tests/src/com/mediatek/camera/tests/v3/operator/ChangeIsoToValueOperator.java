package com.mediatek.camera.tests.v3.operator;

import android.hardware.Camera;
import android.text.TextUtils;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;

public class ChangeIsoToValueOperator extends SettingRadioButtonOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            ChangeIsoToValueOperator.class.getSimpleName());
    private String mTargetIsoValue;

    public ChangeIsoToValueOperator(String targetIsoValue) {
        mTargetIsoValue = targetIsoValue;
    }

    @Override
    protected void doOperate(int index) {
        super.doOperate(index);
        TestContext.mLatestIsoSettingValue = mTargetIsoValue;
    }

    @Override
    protected int getSettingOptionsCount() {
        return 1;
    }

    @Override
    protected String getSettingTitle() {
        return "ISO";
    }

    @Override
    protected String getSettingOptionTitle(int index) {
        return mTargetIsoValue;
    }

    @Override
    public boolean isSupported(int index) {
        if (!mTargetIsoValue.equals("1600") || CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.iso");
        } else {
            List<String> supportedISO;
            Camera.Parameters parameters =
                    TestContext.mLatestCameraFacing == Camera.CameraInfo.CAMERA_FACING_BACK
                            ? TestContext.mBackCameraParameters
                            : TestContext.mFrontCameraParameters;
            supportedISO = split(parameters.get("iso-speed-values"));
            LogHelper.d(TAG, "[isSupported] API1, mTargetIsoValue = " + mTargetIsoValue
                    + ", TestContext.mLatestCameraFacing = " + TestContext.mLatestCameraFacing);
            for (String iso : supportedISO) {
                if ("1600".equalsIgnoreCase(iso)) {
                    LogHelper.d(TAG, "[isSupported] return true for 1600");
                    return true;
                }
            }
            LogHelper.d(TAG, "[isSupported] return false for 1600");
            for (String iso : supportedISO) {
                LogHelper.d(TAG, "[isSupported] support iso contains " + iso);
            }
            return false;
        }
    }

    private ArrayList<String> split(String str) {
        if (str == null) {
            return null;
        }

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<String> subStrings = new ArrayList<>();
        for (String s : splitter) {
            subStrings.add(s);
        }
        return subStrings;
    }
}
