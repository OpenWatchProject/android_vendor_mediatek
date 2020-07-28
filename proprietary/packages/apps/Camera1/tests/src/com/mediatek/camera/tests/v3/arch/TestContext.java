package com.mediatek.camera.tests.v3.arch;

import android.hardware.Camera;
import android.hardware.camera2.CameraCharacteristics;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.List;

public class TestContext {
    private static final LogUtil.Tag TAG = Utils.getTestTag(TestContext.class.getSimpleName());

    public static String mLatestPhotoPath;
    public static String mLatestVideoPath;
    public static List<String> mLatestCsPhotoPath;
    public static String mLatestDngPath;
    public static int mTotalCaptureAndRecordCount;

    public static long mLatestStartRecordTime;
    public static long mLatestStopRecordTime;
    public static int mLatestRecordVideoDurationInSeconds;

    public static int mLatestPictureSizeSettingWidth;
    public static int mLatestPictureSizeSettingHeight;
    public static int mLatestVideoSizeSettingWidth;
    public static int mLatestVideoSizeSettingHeight;

    public static String mLatestIsoSettingValue;
    public static String mLatestWhiteBalanceSettingValue;
    public static String mLatestAntiFlickerSettingValue;

    public static Camera.Parameters mBackCameraParameters;
    public static Camera.Parameters mFrontCameraParameters;

    public static CameraCharacteristics mBackCameraCharacteristics;
    public static CameraCharacteristics mFrontCameraCharacteristics;

    // Camera.CameraInfo.CAMERA_FACING_BACK or Camera.CameraInfo.CAMERA_FACING_FRONT
    public static int mLatestCameraFacing = Camera.CameraInfo.CAMERA_FACING_BACK;

    public static int mLatestWifiIndex = 0;

    public static void reset() {
        mLatestPhotoPath = null;
        mLatestVideoPath = null;
        mLatestCsPhotoPath = null;
        mLatestDngPath = null;
        mTotalCaptureAndRecordCount = 0;

        mLatestStartRecordTime = 0;
        mLatestStopRecordTime = 0;
        mLatestRecordVideoDurationInSeconds = 0;

        mLatestPictureSizeSettingWidth = 0;
        mLatestPictureSizeSettingHeight = 0;
        mLatestVideoSizeSettingWidth = 0;
        mLatestVideoSizeSettingHeight = 0;

        mLatestIsoSettingValue = null;
        mLatestWhiteBalanceSettingValue = null;
        mLatestAntiFlickerSettingValue = null;

        mLatestCameraFacing = Camera.CameraInfo.CAMERA_FACING_BACK;
    }
}
