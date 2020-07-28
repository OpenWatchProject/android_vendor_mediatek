package com.mediatek.camera.tests.v3.operator;

import android.content.Context;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.ReflectUtils;
import com.mediatek.camera.tests.v3.util.Utils;

import java.lang.reflect.Constructor;

public class CaptureOrRecordUntilFullOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(CaptureOrRecordUntilFullOperator
            .class.getSimpleName());
    private static int MAX_OPERATE_TIMES_PHOTO = 100;
    private static int MAX_OPERATE_TIMES_VIDEO = 50;
    private static int RESERVE_BYTES_PASS_CRITERION = 1024 * 1024 / 2;
    private static long CAPTURE_RESERVE_BYTE;
    private static long RECORD_RESERVE_BYTE;

    @Override
    protected void doOperate() {
        long reserveByte;
        UiObject2 shutter = Utils.findObject(Utils.getShutterSelector());
        Utils.assertRightNow(shutter != null, "Can not find shutter");
        boolean isPictureMode;
        if (shutter.getContentDescription().equals("Picture")) {
            reserveByte = CAPTURE_RESERVE_BYTE;
            isPictureMode = true;
            LogHelper.d(TAG, "[doOperate] picture, target reserve " + reserveByte
                    + " byte, about " + (reserveByte / 1024 / 1024) + " MB");
        } else {
            reserveByte = RECORD_RESERVE_BYTE;
            isPictureMode = false;
            LogHelper.d(TAG, "[doOperate] video, target reserve " + reserveByte
                    + " byte, about " + (reserveByte / 1024 / 1024) + " MB");
        }


        int max_operate_times = isPictureMode ? MAX_OPERATE_TIMES_PHOTO : MAX_OPERATE_TIMES_VIDEO;
        int current_operate_times = 0;
        while (Utils.getReserveSpaceInByte() >= reserveByte
                && current_operate_times++ < max_operate_times) {
            new CaptureOrRecordOperator().operate(0);
            long currentReserveByte = Utils.getReserveSpaceInByte();
            LogHelper.d(TAG, "[doOperate] reserve space of " + Utils.getDefaultStoragePath()
                    + " is " + currentReserveByte + " byte, about "
                    + (currentReserveByte / 1024 / 1024) + " MB");
        }

        if (current_operate_times >= max_operate_times) {
            LogHelper.d(TAG, "[doOperate] operate times = " + current_operate_times
                    + ", reach max times");
        } else {
            LogHelper.d(TAG, "[doOperate] operate times = " + current_operate_times
                    + ", not reach max times");
        }

        Utils.assertRightNow(Utils.getReserveSpaceInByte() <= reserveByte
                        || (Utils.getReserveSpaceInByte() - reserveByte)
                        < RESERVE_BYTES_PASS_CRITERION,
                "Expected reserve byte is " + reserveByte
                        + ", but find " + Utils.getReserveSpaceInByte());
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
        return "Capture photos, or record videos until storage full";
    }

    static {
        Constructor cons = ReflectUtils.getConstructor(
                "com.mediatek.camera.common.storage.Storage", Context.class);
        Object storage = ReflectUtils.createInstance(cons, Utils.getContext());
        CAPTURE_RESERVE_BYTE = (long) ReflectUtils.callMethodOnObject(
                storage, "getCaptureThreshold");
        RECORD_RESERVE_BYTE = (long) ReflectUtils.callMethodOnObject(
                storage, "getRecordThreshold");
    }
}
