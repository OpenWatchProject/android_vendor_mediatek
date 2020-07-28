
package com.mediatek.gallery3d.video;

import android.content.Context;
import android.os.Build;
import android.os.storage.StorageManager;

import com.mediatek.gallery3d.util.Log;
import com.mediatek.galleryportable.StorageManagerUtils;
import com.mediatek.galleryportable.SystemPropertyUtils;

import java.io.File;

public class MtkVideoFeature {
    private static final String TAG = "VP_MtkVideoFeature";

    private static final String MTK_GMO_RAM_OPTIMIZE = "ro.vendor.mtk_gmo_ram_optimize";
    private static final String MTK_HOTKNOT = "ro.vendor.mtk_hotknot_support";
    private static final String SUPPER_DIMMING = "ro.vendor.mtk_ultra_dimming_support";
    private static final String MTK_CLEARMOTION = "ro.vendor.mtk_clearmotion_support";
    private static final String CTA_PROPERTY = "ro.vendor.mtk_cta_set";
    private static final String MTK_OMA_DRM = "ro.vendor.mtk_oma_drm_support";
    private static final String MTK_SUPPORT = "1";

    private static final boolean mIsGmoRamOptimize = MTK_SUPPORT.equals(SystemPropertyUtils
            .get(MTK_GMO_RAM_OPTIMIZE));

    private static final boolean mIsSupperDimmingSupport = MTK_SUPPORT.equals(SystemPropertyUtils
            .get(SUPPER_DIMMING));

    private static final boolean mIsClearMotionSupportd = MTK_SUPPORT.equals(SystemPropertyUtils
            .get(MTK_CLEARMOTION));

    /*private static final boolean mIsOmaDrmSupported = SystemPropertyUtils
            .getBoolean(MTK_OMA_DRM, false);*/
    // AOSP OMA DRM is always supported
    private static final boolean mIsOmaDrmSupported = true;

    // CTA Data Protection
    private static final boolean mIsSupportCTA = MTK_SUPPORT
            .equals(SystemPropertyUtils.get(CTA_PROPERTY));

    public static boolean isSimulateWfd() {
        int support = SystemPropertyUtils.getInt("wfd_debug", 0);
        Log.d(TAG, "isSimulateWfd() support " + support);
        return support == 1;
    }

    // M: is ram optimize Enable
    public static boolean isGmoRAM() {
        boolean enabled = mIsGmoRamOptimize;
        Log.d(TAG, "isGmoRAM() return " + enabled);
        return enabled;
    }

    public static boolean isGmoRamOptimize() {
        Log.v(TAG, "isGmoRamOptimize() " + mIsGmoRamOptimize);
        return mIsGmoRamOptimize;
    }

    public static boolean isSupperDimmingSupport() {
        Log.v(TAG, "isSupperDimmingSupport() " + mIsSupperDimmingSupport);
        return mIsSupperDimmingSupport;
    }

    /**
     * Is clear motion supported
     *
     * @return whether is support clear motion
     */
    public static boolean isClearMotionSupport() {
        Log.d(TAG, "isClearMotionSupported() return " + mIsClearMotionSupportd);
        return mIsClearMotionSupportd;
    }

    /**
     * Check whether OMA DRM v1.0 is supported or not
     * @return true if OMA DRM feature is enabled, otherwise return false
     */
    public static boolean isOmaDrmSupported() {
        Log.d(TAG, "isOmaDrmSupported() return " + mIsOmaDrmSupported);
        return mIsOmaDrmSupported;
    }

    /**
     * Check if support clear motion or not.
     * @param context
     * @return true if support, otherwise false
     */
    public static boolean isClearMotionMenuEnabled(Context context) {
        return isClearMotionSupport() && isFileExist(context, "SUPPORT_CLEARMOTION");
    }

    public static boolean isSupportCTA() {
        Log.d(TAG, "mIsSupportCTA() return " + mIsSupportCTA);
        return mIsSupportCTA;
    }

    public static boolean isPowerTest() {
        return MTK_SUPPORT.equals(SystemPropertyUtils.get("persist.power.auto.test"));
    }

    /**
     * Whether the file with the special name exist.If the file exist the menu
     * related to this file will be shown.Currently only Clear motion and
     * Picture quality dynamic contrast feature menu use this API.
     *
     * @param context
     *            The current context.
     * @param fileName
     *            The name of a file.
     * @return True if the file is exist in storage,false otherwise.
     */
    public static boolean isFileExist(Context context, String fileName) {
        boolean isFileExist = false;
        String[] path = StorageManagerUtils.getVolumnPaths((StorageManager) context
                .getSystemService(Context.STORAGE_SERVICE));
        if (path == null) {
            Log.w(TAG,
                    "isFileExist() storage volume path is null, return false");
            return false;
        }
        int length = path.length;
        for (int i = 0; i < length; i++) {
            if (path != null) {
                File file = new File(path[i], fileName);
                if (file.exists()) {
                    Log.v(TAG, "isFileExist() file exists with the name is "
                            + file);
                    isFileExist = true;
                }
            }
        }
        Log.v(TAG, "isFileExist() exit with isFileExist is " + isFileExist);
        return isFileExist;
    }

    /**
     * Whether support multi window mode
     * @return
     */
    public static boolean isMultiWindowSupport() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.N;
    }
}

