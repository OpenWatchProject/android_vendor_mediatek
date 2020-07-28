package com.mediatek.fullscreenmode;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.drawable.Drawable;
import android.util.Log;

public final class ViewUtils {
    private static final String TAG = "ViewUtils";

    /*
     * @param context Context
     * @param pkg String, the apk's package name
     * @return the apk's label
     */
    public static String getAppLabel(Context context, String pkg) {
        PackageManager pm = context.getPackageManager();
        int retrieveFlags = PackageManager.GET_DISABLED_COMPONENTS;
        ApplicationInfo appInfo;
        try {
            appInfo = pm.getApplicationInfo(pkg, retrieveFlags);
        } catch (NameNotFoundException ex) {
            Log.w(TAG, "ApplicationInfo cannot be found for pkg:" + pkg, ex);
            return "";
        }
        return pm.getApplicationLabel(appInfo).toString();
    }

    /*
     * @param context Context
     * @param pkg String, the apk's package name
     * @return the apk's icon
     */
    public static Drawable getAppDrawable(Context context, String pkg) {
        PackageManager pm = context.getPackageManager();
        int retrieveFlags = PackageManager.GET_DISABLED_COMPONENTS;
        ApplicationInfo appInfo;
        try {
            appInfo = pm.getApplicationInfo(pkg, retrieveFlags);
        } catch (NameNotFoundException ex) {
            Log.w(TAG, "ApplicationInfo cannot be found for pkg:" + pkg, ex);
            return null;
        }
        return pm.getApplicationIcon(appInfo);
    }
}