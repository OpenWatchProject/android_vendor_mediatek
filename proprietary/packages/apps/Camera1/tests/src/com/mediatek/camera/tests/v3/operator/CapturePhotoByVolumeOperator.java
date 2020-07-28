package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;
import android.view.KeyEvent;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.regex.Pattern;

public class CapturePhotoByVolumeOperator extends OperatorOne {
    private boolean mByVolumeDownKey = true;

    public CapturePhotoByVolumeOperator() {
        mByVolumeDownKey = true;
    }

    public CapturePhotoByVolumeOperator(boolean byVolumeDownKey) {
        mByVolumeDownKey = byVolumeDownKey;
    }

    @Override
    protected void doOperate() {
        if (mByVolumeDownKey) {
            Utils.getUiDevice().pressKeyCode(KeyEvent.KEYCODE_VOLUME_DOWN);
        } else {
            Utils.getUiDevice().pressKeyCode(KeyEvent.KEYCODE_VOLUME_UP);
        }


        Pattern btnSavePattern = Pattern.compile(".*btn.*_save");
        UiObject2 panoramaSave = Utils.findObject(By.res(btnSavePattern),
                Utils.TIME_OUT_SHORT_SHORT);
        if (panoramaSave != null) {
            if (mByVolumeDownKey) {
                Utils.getUiDevice().pressKeyCode(KeyEvent.KEYCODE_VOLUME_DOWN);
            } else {
                Utils.getUiDevice().pressKeyCode(KeyEvent.KEYCODE_VOLUME_UP);
            }
        }
    }

    @Override
    public Page getPageBeforeOperate() {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Capture one photo by volume key, if panorama mode, capture photo without moving";
    }
}
