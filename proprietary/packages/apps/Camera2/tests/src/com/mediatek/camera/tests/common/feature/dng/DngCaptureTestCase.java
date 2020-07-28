package com.mediatek.camera.tests.common.feature.dng;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject;
import android.support.test.uiautomator.UiObject2;
import android.support.test.uiautomator.UiSelector;
import android.support.test.uiautomator.Until;

import com.mediatek.camera.portability.SystemProperties;
import com.mediatek.camera.tests.CameraCaptureTestCase;
import com.mediatek.camera.tests.Log;
import com.mediatek.camera.tests.Utils;

import junit.framework.Assert;

import java.io.File;
import java.util.ArrayList;
import java.util.List;


/**
 * This is for dng capture test.
 * one capture, two images.
 * adb shell setprop vendor.dngStressCapture.num 50.
 * test 50 times capture for testDngRepeatingCapture() and testDngRepeatingEnterCameraCapture().
 */

public class DngCaptureTestCase extends CameraCaptureTestCase {
    private static final String TAG = DngCaptureTestCase.class.getSimpleName();
    private static final int NEW_WINDOW_TIME_OUT_MS = 2000;
    private String mCurrentPackages;

    /**
     * check dng state and indicator after pause and resume.
     */
    @DngCaseAnnotation
    public void testDngStateAfterPauseAndResume() {
        //check dng state after pause and resume
        doDngStateAndIndicatorCheck();
        //switch to front, check the same capture flow
        if (!switchCamera()) {
            return;
        }
        Log.d(TAG, "[testDngStateAfterPauseAndResume], switch to front camera");
        doDngStateAndIndicatorCheck();
    }

    /**
     * check dng capture, two images saved, one is dng the other is jpeg.
     * check thumbnail.
     * check go to gallery.
     */
    @DngCaseAnnotation
    public void testDngCapture() {
        //clear folder before repeat capture
        doDngBasicCapture();
        //switch to front, check the same capture flow
        if (!switchCamera()) {
            return;
        }
        Log.d(TAG, "[testDngCapture], switch to front camera");
        doDngBasicCapture();
    }

    /**
     * test dng state in the current mode.
     * Video/pip/panarama/slow motion
     */
    @DngCaseAnnotation
    public void testDngRestrictionWithMode() {
        backToNormalPictureMode();
        if (!waitForPreviewReady()) {
            return;
        }
        //if normal not support dng, no need check
        boolean isbackDngSupport = setDngState(true);
        if (!isbackDngSupport) {
            Log.d(TAG, "[testDngRestrictionWithMode], camera not support dng");
            return;
        }
        //check dng in mode list
        List<String> modeNameList = getModeList();
        for (int i = 0; i < modeNameList.size(); i++) {
            if (!enterModeMenuFromMainPreview()) {
                return;
            }
            //UiObject2 modeListCollection = mUiDevice.findObject(By.res(Utils.MODE_LIST_ID));
            UiObject2 modeType = mUiDevice.findObject(By.desc(modeNameList.get(i)));
            if (modeType != null) {
                Log.d(TAG, "[testDngRestrictionWithMode], mode: " + modeNameList.get(i));
                modeType.click();
                waitForPreviewReady();
                //check shutter mode for dng in the mode type
                doRestrictionCheckWithShutterMode(modeNameList.get(i));
            } else {
                backToMainScreen();
            }
        }
        //back to normal picture mode
        backToNormalPictureMode();
    }

    /**
     * test dng restriction with features.
     */
    @DngCaseAnnotation
    public void testDngRestrictionWithHdr() {
        if (!waitForPreviewReady()) {
            return;
        }
        //turn on dng, turn on the restriction feature, check dng state
        List<String> hdrDiscriptionList = new ArrayList<>();
        hdrDiscriptionList.add("HDR auto");
        hdrDiscriptionList.add("HDR on");
        hdrDiscriptionList.add("HDR off");
        for (int i = 0; i < hdrDiscriptionList.size(); i++) {
            doHdrRestrictionCheck(hdrDiscriptionList.get(i));
        }
    }

    /**
     * Repeat take picture.
     * times with no error.
     */
    @DngCaseAnnotation
    public void testDngRepeatingCapture() {
        doRepeatingCapture();
        if (!switchCamera()) {
            return;
        }
        Log.d(TAG, "[testDngRepeatingCapture], switch to front camera");
        //switch to front camera
        doRepeatingCapture();
    }

    /**
     * Repeating test pause/resume/capture dng.
     */
    @DngCaseAnnotation
    public void testDngRepeatingEnterCameraCapture() {
        doRepeatingPauseAndResumeCapture();
        //switch to front camera
        if (!switchCamera()) {
            return;
        }
        Log.d(TAG, "[testDngRepeatingEnterCameraCapture], switch to front camera");
        doRepeatingPauseAndResumeCapture();
    }

    private static int getDngStressTestNumber() {
        int num = 1;
        try {
            num = SystemProperties.getInt("vendor.dngStressCapture.num", 1);
        } catch (IllegalArgumentException e) {
            Log.d(TAG, "getDngStressTestNumber, failed: " + e.getMessage());
        }
        return num;
    }

    private List<String> getModeList() {
        List<String> modeList = new ArrayList<>();
        modeList.add("Normal");
        modeList.add("PIP");
        modeList.add("Panorama");
        modeList.add("Slow motion");
        return modeList;
    }

    private boolean isModeSupportDng(String modeType,
                                     String shutterModeTitle) {
        if ("Normal".equals(modeType)
                && "Picture".equals(shutterModeTitle)) {
            return true;
        } else {
            return false;
        }
    }

    private boolean waitForPreviewReady() {
        boolean isReady = mUiDevice.wait(Until.hasObject(
                By.res(Utils.SHUTTER_LIST_ID).descEndsWith("is previewing")), Utils.TIME_OUT_MS);
        if (!isReady) {
            Log.e(TAG, "waitForPreviewReady, preview not ready");
        }
        return isReady;
    }

    private void doRestrictionCheckWithShutterMode(String modeType) {
        //check dng in shutter list
        UiObject2 shutterModeCollection = mUiDevice.findObject(By.res(Utils.SHUTTER_LIST_ID));
        List<UiObject2> shutterModeList = shutterModeCollection.getChildren();
        Log.d(TAG, "[doRestrictionCheckWithShutterMode], shutter mode num: "
                + shutterModeList.size());
        for (int j = 0; j < shutterModeList.size(); j++) {
            UiObject2 shutterModeTitle = shutterModeList.get(j).findObject(
                    By.res(Utils.SHUTTER_CHILD_TEXT));
            String shutterTitle = shutterModeTitle.getText();
            Log.d(TAG, "[doRestrictionCheckWithShutterMode], shutter title: "
                    + shutterTitle);
            shutterModeTitle.clickAndWait(Until.newWindow(), NEW_WINDOW_TIME_OUT_MS);
            if (!enterSettingMenuFromMainPreview()) {
                return;
            }
            UiObject2 dngPreference = mUiDevice.findObject(By.descStartsWith("DNG_DISC_"));
            if (isModeSupportDng(modeType, shutterTitle)) {
                Assert.assertNotNull(dngPreference);
            } else {
                Assert.assertNull(dngPreference);
            }
            backToMainScreen();
        }
    }

    private void doHdrRestrictionCheck(String discription) {
        if (setDngState(true)) {
            UiObject2 hdrIcon = mUiDevice.findObject(By.res(Utils.FEATURE_HDR_ICON));
            if (hdrIcon != null) {
                hdrIcon.clickAndWait(Until.newWindow(), NEW_WINDOW_TIME_OUT_MS);
                UiObject2 optionMenu = mUiDevice.findObject(By.res(Utils.FEATURE_HDR_OPTION_MENU));
                if (optionMenu != null) {
                    UiObject2 hdrOptionIcon = mUiDevice.findObject(By.desc(discription));
                    if (hdrOptionIcon != null) {
                        hdrOptionIcon.clickAndWait(Until.newWindow(), NEW_WINDOW_TIME_OUT_MS);
                    }
                }
                //hdr off
                hdrIcon = mUiDevice.findObject(By.res(Utils.FEATURE_HDR_ICON));
                if (hdrIcon != null) {
                    if ("HDR off".equals(hdrIcon.getContentDescription())) {
                        if (!enterSettingMenuFromMainPreview()) {
                            return;
                        }
                        Assert.assertEquals(true, getDngState(true));
                        backToMainScreen();
                    } else {
                        if (!enterSettingMenuFromMainPreview()) {
                            return;
                        }
                        Assert.assertEquals(true, getDngState(false));
                        backToMainScreen();
                    }
                }
            }
            setDngState(false);
        }
    }

    private void doDngStateAndIndicatorCheck() {
        if (!waitForPreviewReady()) {
            return;
        }
        boolean isbackDngSupport = setDngState(true);
        if (!isbackDngSupport) {
            Log.d(TAG, "[doDngStateAndIndicatorCheck], camera not support dng");
            return;
        }
        pauseActivity();
        mUiDevice.waitForWindowUpdate(mUiDevice.getCurrentPackageName(),
                NEW_WINDOW_TIME_OUT_MS);

        resumeActivity();
        mUiDevice.waitForWindowUpdate(mUiDevice.getCurrentPackageName(),
                NEW_WINDOW_TIME_OUT_MS);
        if (!enterSettingMenuFromMainPreview()) {
            return;
        }
        Assert.assertEquals(true, getDngState(true));
        backToMainScreen();
        //check indicator show
        UiObject dngOnIndicator = mUiDevice.findObject(
                new UiSelector().resourceId(Utils.DNG_INDICATOR_ID));
        if (dngOnIndicator != null) {
            Assert.assertEquals(true, dngOnIndicator.waitForExists(NEW_WINDOW_TIME_OUT_MS));
        }
        if (setDngState(false)) {
            UiObject dngOffIndicator = mUiDevice.findObject(
                    new UiSelector().resourceId(Utils.DNG_INDICATOR_ID));
            if (dngOffIndicator != null) {
                Assert.assertEquals(true, dngOffIndicator.waitUntilGone(NEW_WINDOW_TIME_OUT_MS));
            }
        }
    }

    private void doCaptureAndResultCheck() {
        if (waitForPreviewReady()) {
            try {
                captureAndCheckSucceed();
                Assert.assertEquals(false, Utils.waitForTrueWithTimeOut(
                        mImageChecher, Utils.TIME_OUT_MS));
            } catch (Exception e) {
                return;
            }
        }
    }

    private void doDngBasicCapture() {
        boolean isDngSupport = setDngState(true);
        if (!isDngSupport) {
            Log.d(TAG, "[doDngBasicCapture], not support dng");
            return;
        }
        try {
            boolean isHasContent = checkThumbnailIsEmpty();
            if (isHasContent) {
                Log.w(TAG, "[doDngBasicCapture], thubnail should be null");
                deleteCameraFolder();
            }
            doCaptureAndResultCheck();

            if (!waitForPreviewReady()) {
                return;
            }
            //check thumbnail
            isHasContent = checkThumbnailIsEmpty();
            Assert.assertEquals(true, isHasContent);

            //tap thumbnail to open gallery
            mCurrentPackages = mUiDevice.getCurrentPackageName();
            UiObject2 thumbnailUiObject = mUiDevice.findObject(By.res(Utils.THUMBNAIL_RES_ID));
            if (thumbnailUiObject != null && thumbnailUiObject.isClickable()) {
                Log.d(TAG, "[doDngBasicCapture], thumbnail click");
                thumbnailUiObject.click();
                //Image may not saved.
                Assert.assertEquals(false, Utils.waitForTrueWithTimeOut(mPackagesChecker,
                        Utils.TIME_OUT_MS));
                //back to camera
                mUiDevice.pressBack();
                mUiDevice.waitForWindowUpdate(
                        mUiDevice.getCurrentPackageName(), NEW_WINDOW_TIME_OUT_MS);
            }

            //switch dng to off, back to normal preview
            if (!setDngState(false)) {
                Log.d(TAG, "[doDngBasicCapture], unhandle failed");
                return;
            }
            //clear image after capture
            deleteCameraFolder();
        } catch (Exception e) {
            return;
        }
    }

    private void doRepeatingCapture() {
        if (!waitForPreviewReady()) {
            return;
        }
        boolean isDngSupport = setDngState(true);
        if (!isDngSupport) {
            Log.d(TAG, "[doRepeatingCapture], back camera not support dng");
            return;
        }
        int num = getDngStressTestNumber();
        for (int i = 0; i < num; i++) {
            doCaptureAndResultCheck();
            //clear image after capture
            deleteCameraFolder();
        }
        //switch dng to off
        setDngState(false);
    }

    private void doRepeatingPauseAndResumeCapture() {
        if (!waitForPreviewReady()) {
            return;
        }
        boolean isDngSupport = setDngState(true);
        if (!isDngSupport) {
            Log.d(TAG, "[doRepeatingPauseAndResumeCapture], camera not support dng");
            return;
        }
        int num = getDngStressTestNumber();
        for (int i = 0; i < num; i++) {
            pauseActivity();
            mUiDevice.waitForWindowUpdate(mUiDevice.getCurrentPackageName(),
                    NEW_WINDOW_TIME_OUT_MS);

            resumeActivity();
            mUiDevice.waitForWindowUpdate(mUiDevice.getCurrentPackageName(),
                    NEW_WINDOW_TIME_OUT_MS);

            doCaptureAndResultCheck();
            //clear image after capture
            deleteCameraFolder();
        }
        setDngState(false);
    }

    private boolean enterModeMenuFromMainPreview() {
        UiObject2 modeButton = mUiDevice.findObject(By.res(Utils.MODE_SELECT_BUTTON_ID));
        if (modeButton != null && modeButton.isEnabled()) {
            modeButton.click();
            if (!Utils.waitForTrueWithTimeOut(mModeMenuChecker, Utils.TIME_OUT_MS)) {
                return true;
            }
        }
        Log.e(TAG, "[enterModeMenuFromMainPreview], enter mode menu failed");
        return false;
    }

    private boolean enterSettingMenuFromModeList() {
        UiObject2 settingButton = mUiDevice.findObject(By.res(Utils.MODE_SETTING_BUTTON_ID));
        if (settingButton != null && settingButton.isEnabled()) {
            settingButton.click();
            if (!Utils.waitForTrueWithTimeOut(mSettingMenuChecker, Utils.TIME_OUT_MS)) {
                return true;
            }
        }
        Log.e(TAG, "[enterSettingMenuFromModeList], enter setting menu failed");
        return false;
    }

    private boolean enterSettingMenuFromMainPreview() {
        if (enterModeMenuFromMainPreview()) {
            return enterSettingMenuFromModeList();
        }
        return false;
    }

    private void backToNormalPictureMode() {
        enterModeMenuFromMainPreview();
        //UiObject2 modeListCollection = mUiDevice.findObject(By.res(Utils.MODE_LIST_ID));
        UiObject2 modeType = mUiDevice.findObject(By.desc("Normal"));
        if (modeType != null) {
            modeType.clickAndWait(Until.newWindow(), NEW_WINDOW_TIME_OUT_MS);
        }
        UiObject2 shutterModeCollection = mUiDevice.findObject(By.res(Utils.SHUTTER_LIST_ID));
        if (shutterModeCollection != null) {
            int childnum = shutterModeCollection.getChildCount();
            if (childnum > 1) {
                UiObject2 pictureMode = shutterModeCollection.findObject(By.text("Picture"));
                if (pictureMode != null) {
                    pictureMode.clickAndWait(Until.newWindow(), NEW_WINDOW_TIME_OUT_MS);
                }
            }
        }
    }

    private void backToMainScreen() {
        mUiDevice.pressBack();
        mUiDevice.waitForWindowUpdate(mUiDevice.getCurrentPackageName(), NEW_WINDOW_TIME_OUT_MS);
    }

    private boolean switchCamera() {
        UiObject2 switchButton = mUiDevice.findObject(By.res(Utils.SWITCH_BUTTON_ID));
        if (switchButton != null) {
            switchButton.clickAndWait(Until.newWindow(), NEW_WINDOW_TIME_OUT_MS);
            return true;
        }
        return false;
    }

    private boolean setDngState(boolean isOn) {
        if (!enterSettingMenuFromMainPreview()) {
            backToMainScreen();
            return false;
        }
        if (getDngState(isOn)) {
            //no need change the state.
            backToMainScreen();
            return true;
        }
        //need change state.
        boolean isSuccessed = false;
        UiObject2 dngPreference = mUiDevice.findObject(By.descStartsWith("DNG_DISC_"));
        if (dngPreference != null) {
            dngPreference.wait(Until.enabled(true), NEW_WINDOW_TIME_OUT_MS);
            //check icon state
            List<UiObject2> dngPreferenceChildList = dngPreference.getChildren();
            if (dngPreferenceChildList.size() > 1) {
                //switch icon, index 1
                UiObject2 switchIcon = dngPreferenceChildList.get(1).findObject(
                        By.clazz("android.widget.Switch"));
                //wait for click
                boolean isReady = switchIcon.wait(Until.enabled(true), NEW_WINDOW_TIME_OUT_MS);
                if (!isReady) {
                    Log.e(TAG, "[setDngState], switchIcon not enabled");
                }
                switchIcon.clickAndWait(Until.newWindow(), NEW_WINDOW_TIME_OUT_MS);
                if (getDngState(isOn)) {
                    isSuccessed = true;
                }
            }
        }
        backToMainScreen();
        if (!isSuccessed) {
            Log.e(TAG, "[setDngState], dng state is not expected");
        }
        return isSuccessed;
    }

    private boolean getDngState(boolean isOn) {
        //called in setting page
        String dngState = "OFF";
        UiObject2 dngPreference = mUiDevice.findObject(By.descStartsWith("DNG_DISC_"));
        if (dngPreference != null) {
            dngPreference.wait(Until.enabled(true), NEW_WINDOW_TIME_OUT_MS);
        } else {
            Log.d(TAG, "[getDngState], there is no dng setting item");
            return false;
        }
        if (dngPreference != null && dngPreference.isEnabled() &&
                dngPreference.getContentDescription().equals("DNG_DISC_ON")) {
            dngState = "ON";
        }
        if (isOn && dngState.equals("ON") || !isOn && dngState.equals("OFF")) {
            Log.d(TAG, "[getDngState], dng state is OK");
            return true;
        }
        return false;
    }

    private boolean checkThumbnailIsEmpty() {
        UiObject2 thumbnailUiObject = mUiDevice.findObject(By.res(Utils.THUMBNAIL_RES_ID));
        if (thumbnailUiObject != null) {
            thumbnailUiObject.wait(Until.enabled(true), NEW_WINDOW_TIME_OUT_MS);
            return Utils.isThumbnailHasContent(thumbnailUiObject);
        }
        Log.e(TAG, "[checkThumbnailIsEmpty] thumbnail null");
        return false;
    }

    private Utils.Checker mModeMenuChecker = new Utils.Checker() {
        @Override
        public boolean check() {
            UiObject2 modeTitle = mUiDevice.findObject(By.text("Modes"));
            if (modeTitle != null && modeTitle.isEnabled()) {
                return true;
            }
            return false;
        };
    };

    private Utils.Checker mSettingMenuChecker = new Utils.Checker() {
        @Override
        public boolean check() {
            UiObject2 settingTitle = mUiDevice.findObject(By.text("Settings"));
            if (settingTitle != null && settingTitle.isEnabled()) {
                return true;
            }
            return false;
        };
    };

    private Utils.Checker mPackagesChecker = new Utils.Checker() {
        @Override
        public boolean check() {
            String newPackage = mUiDevice.getCurrentPackageName();
            if (mCurrentPackages.equals(newPackage)) {
                UiObject2 thumbnailUiObject = mUiDevice.findObject(By.res(Utils.THUMBNAIL_RES_ID));
                if (thumbnailUiObject != null && thumbnailUiObject.isClickable()) {
                    Log.d(TAG, "[mPackagesChecker], thumbnail click");
                    thumbnailUiObject.click();
                }
            }
            return !(mCurrentPackages.equals(newPackage));
        };
    };

    private Utils.Checker mImageChecher = new Utils.Checker() {
        @Override
        public boolean check() {
            return checkCaptureResult();
        };
    };

    private boolean checkCaptureResult() {
        File imageDir = new File(DCIM_CAMERA_FOLDER_ABSOLUTE_PATH);
        if (imageDir == null) {
            return false;
        }
        if (imageDir.isDirectory()) {
            File[] dirChildren = imageDir.listFiles();
            if (dirChildren == null || dirChildren.length == 0) {
                return false;
            }
            for (int i = 0; i < dirChildren.length; i++) {
                if (dirChildren[i].getName().contains(".dng")) {
                    Log.d(TAG, "[checkCaptureResult] getName: " + dirChildren[i].getName());
                    return true;
                }
                continue;
            }
        }
        return false;
    }
}
