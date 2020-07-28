package com.mediatek.camera.portability;

import android.app.ActivityManager;

import java.lang.reflect.Method;

/**
 * Activity manager extension
 */
public class ActivityManagerExt {
    private static final String TAG = "ActivityManagerExt";
    private static Method sIsLowRamDeviceStaticMethod = ReflectUtil.getMethod(ActivityManager.class,
                    "isLowRamDeviceStatic");

    /**
     * Judge if the running device is low ram or not .
     * @return Ture yes, otherwise false.
     */
    public static boolean isLowRamDeviceStatic() {
        if (sIsLowRamDeviceStaticMethod == null) {
            return false;
        }
        return (boolean) ReflectUtil.callMethodOnObject(null, sIsLowRamDeviceStaticMethod);
    }
}