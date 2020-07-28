package com.mediatek.galleryportable;

import android.content.Context;
import android.os.Environment;
import android.os.storage.StorageManager;
import android.util.Log;

import com.mediatek.storage.StorageManagerEx;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.RuntimeException;

public class StorageManagerUtils {
    private static final String TAG = "Gallery2/StorageManagerUtils";
    private static boolean sHasChecked = false;
    private static boolean sIsStorageManagerExExist = false;
    private static boolean sHasIsExternalSDCardFunction = false;
    private static boolean sHasIsUSBOTGFunction = false;
    private static boolean sHasGetDefaultPathFunction = false;
    private static boolean sHasGetVolumnPathsFunction = false;
    private static Method sMethodIsExternalSDCard = null;
    private static Method sMethodIsUSBOTG = null;

    public static String getDefaultPath() {
        checkWhetherSupport();
        if (sHasGetDefaultPathFunction) {
            try {
                return StorageManagerEx.getDefaultPath();
            } catch (RuntimeException e) {
                Log.d(TAG, "<getDefaultPath> RuntimeException, reget from sdk api", e);
                return Environment.getExternalStorageDirectory().getAbsolutePath();
            }
        } else {
            return Environment.getExternalStorageDirectory().getAbsolutePath();
        }
    }

    public static String getStorageForCache(Context context) {
        StorageManager storageManager = (StorageManager) context
                .getSystemService(Context.STORAGE_SERVICE);
        String internalStoragePath = null;
        try {
            String[] volumes = storageManager.getVolumePaths();
            // get internal storage / phone storage
            // if volume is mounted && not external sd card && not usb otg,
            // we treat it as internal storage / phone storage
            for (String str : volumes) {
                if (StorageManagerUtils.isExternalSDCard(str)) {
                    continue;
                }
                if (StorageManagerUtils.isUSBOTG(str)) {
                    continue;
                }
                if (Environment.MEDIA_MOUNTED.equalsIgnoreCase(storageManager.getVolumeState
                        (str))) {
                    internalStoragePath = str;
                    break;
                }
            }
        } catch (UnsupportedOperationException e) {
            Log.d(TAG, "<getStorageForCache> UnsupportedOperationException", e);
        }

        if (internalStoragePath == null || internalStoragePath.equals("")) {
            return Environment.getExternalStorageDirectory().getAbsolutePath();
        } else {
            return internalStoragePath;
        }
    }

    private static boolean isExternalSDCard(String path) throws UnsupportedOperationException {
        checkWhetherSupport();
        if (sHasIsExternalSDCardFunction) {
            try {
                return (boolean) sMethodIsExternalSDCard.invoke(null, path);
            } catch (IllegalAccessException e) {
                Log.e(TAG, "isExternalSDCard", e);
                return false;
            } catch (InvocationTargetException e) {
                Log.e(TAG, "isExternalSDCard", e);
                return false;
            }
        } else {
            throw new UnsupportedOperationException(
                    "There is no StorageManagerEx in current platform"
                            + ", not support to check if input path is external sd card",
                    new Throwable());
        }
    }

    private static boolean isUSBOTG(String path) throws UnsupportedOperationException {
        checkWhetherSupport();
        if (sHasIsUSBOTGFunction) {
            try {
                return (boolean) sMethodIsUSBOTG.invoke(null, path);
            } catch (IllegalAccessException e) {
                Log.e(TAG, "isUSBOTG", e);
                return false;
            } catch (InvocationTargetException e) {
                Log.e(TAG, "isUSBOTG", e);
                return false;
            }
        } else {
            throw new UnsupportedOperationException(
                    "There is no StorageManagerEx in current platform"
                            + ", not support to check if input path is USB OTG",
                    new Throwable());
        }
    }

    public static String[] getVolumnPaths(StorageManager storageManager) {
        checkWhetherSupport();
        if (sHasGetVolumnPathsFunction) {
            return storageManager.getVolumePaths();
        } else {
            throw new UnsupportedOperationException(
                    "There is no StorageManagerEx in current platform"
                            + ", not support to check if input path is USB OTG",
                    new Throwable());
        }
    }

    private static void checkWhetherSupport() {
        if (!sHasChecked) {
            try {
                Class<?> clazz =
                        StorageManagerUtils.class.getClassLoader().loadClass(
                                "com.mediatek.storage.StorageManagerEx");
                sIsStorageManagerExExist = (clazz != null);
            } catch (ClassNotFoundException e) {
                sIsStorageManagerExExist = false;
            }
            if (!sIsStorageManagerExExist) {
                sHasChecked = true;
                return;
            }
            try {
                sMethodIsExternalSDCard =
                        StorageManagerEx.class.getDeclaredMethod("isExternalSDCard",
                                String.class);
                sHasIsExternalSDCardFunction = (sMethodIsExternalSDCard != null);
            } catch (NoSuchMethodException e) {
                sHasIsExternalSDCardFunction = false;
            }
            try {
                sMethodIsUSBOTG =
                        StorageManagerEx.class.getDeclaredMethod("isUSBOTG",
                                String.class);
                sHasIsUSBOTGFunction = (sMethodIsUSBOTG != null);
            } catch (NoSuchMethodException e) {
                sHasIsUSBOTGFunction = false;
            }
            try {
                Method method =
                        StorageManagerEx.class.getDeclaredMethod("getDefaultPath");
                sHasGetDefaultPathFunction = (method != null);
            } catch (NoSuchMethodException e) {
                sHasGetDefaultPathFunction = false;
            }
            try {
                Method method =
                        StorageManager.class.getDeclaredMethod("getVolumePaths");
                sHasGetVolumnPathsFunction = (method != null);
            } catch (NoSuchMethodException e) {
                sHasGetVolumnPathsFunction = false;
            }
            sHasChecked = true;
            return;
        }
    }
}
