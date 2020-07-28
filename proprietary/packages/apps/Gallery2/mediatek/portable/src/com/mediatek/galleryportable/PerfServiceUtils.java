package com.mediatek.galleryportable;

import android.os.RemoteException;
import android.util.Log;

import java.lang.reflect.Method;

public class PerfServiceUtils {
    private static final String TAG = "Gallery2/PerfServiceUtils";

    private static final String MTK_POWER_HAL_CLASS
            = "com.mediatek.powerhalwrapper.PowerHalWrapper";
    private static final String MTK_POWER_HAL_METHOD_GALLERY_BOOST = "galleryBoostEnable";
    private static Class<?> clazz = null;
    private static Method methodGalleryBoostEnable = null;
    private static boolean sHasChecked = false;
    private static boolean sSupportMtkPowerHal = false;

    public static void boostEnableTimeoutMs(int timeoutMs) {
        if (checkWetherSupport()) {
            if (clazz != null && methodGalleryBoostEnable != null) {
                Log.d(TAG, "<boostEnableTimeoutMs> do boost with " + timeoutMs + "ms !");
                try {
                    Method method = clazz.getDeclaredMethod("getInstance");
                    if (method != null) {
                        Object powerHal = method.invoke(clazz);
                        Log.d(TAG, "<boostEnableTimeoutMs> powerHal " + powerHal);
                        methodGalleryBoostEnable.invoke(powerHal, timeoutMs);
                    }
                } catch (Exception e) {
                    Log.e(TAG, "<boostEnableTimeoutMs> Exception", e);
                }
            }
        }
    }

    public static void boostEnable() {
        Log.d(TAG, "<boostEnable> Gallery boost enable");
        boostEnableTimeoutMs(Integer.MAX_VALUE);
    }

    public static void boostDisable() {
        Log.d(TAG, "<boostDisable> Gallery boost disable");
        boostEnableTimeoutMs(0);
    }

    private static boolean checkWetherSupport() {
        if (!sHasChecked) {
            try {
                clazz = PerfServiceUtils.class.getClassLoader()
                        .loadClass(MTK_POWER_HAL_CLASS);
                Log.d(TAG, "<checkWetherSupport> clazz: " + clazz);
                methodGalleryBoostEnable = clazz.getDeclaredMethod(
                        MTK_POWER_HAL_METHOD_GALLERY_BOOST, int.class);
                methodGalleryBoostEnable.setAccessible(true);
                Log.d(TAG, "<checkWetherSupport> methodPowerHint: " + methodGalleryBoostEnable);
                sSupportMtkPowerHal = true;
            } catch (ClassNotFoundException e) {
                sSupportMtkPowerHal = false;
                Log.e(TAG, "<checkWetherSupport> ClassNotFoundException", e);
            } catch (NoSuchMethodException e) {
                sSupportMtkPowerHal = false;
                Log.e(TAG, "<checkWetherSupport> NoSuchMethodException", e);
            }
            sHasChecked = true;
            Log.d(TAG, "<checkWetherSupport> sSupportMtkPowerHal: " + sSupportMtkPowerHal);
        }
        return sSupportMtkPowerHal;
    }
}