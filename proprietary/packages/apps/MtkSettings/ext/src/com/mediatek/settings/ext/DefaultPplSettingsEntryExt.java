package com.mediatek.settings.ext;

import android.os.SystemProperties;
import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.util.Log;
import androidx.preference.PreferenceGroup;

import dalvik.system.PathClassLoader;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;

public class DefaultPplSettingsEntryExt implements IPplSettingsEntryExt {
    private static final String TAG = "PPL/PplSettingsEntryExt";
    private static final boolean mIsPplSupport =
        SystemProperties.get("ro.vendor.mtk_privacy_protection_lock").equals("1");

    private static final String APK_PATH =
            "/product/priv-app/PrivacyProtectionLock/PrivacyProtectionLock.apk";
    private static final String PKG_NAME = "com.mediatek.ppl";
    private static final String TARGET_NAME =
            "com.mediatek.ppl.ext.PplSettingsEntryPlugin";

    private static Object mPplExt;

    public static IPplSettingsEntryExt getInstance(Context context) {
        if (context == null || !mIsPplSupport) {
            Log.e(TAG, "[getInstance] return Default!!! mIsPplSupport="
                    + mIsPplSupport + " ,context=" + context);
            return new DefaultPplSettingsEntryExt();
        }

        Log.d(TAG, "[getInstance] context=" + context);
        try {
            ClassLoader classLoader;
            classLoader = new PathClassLoader(APK_PATH, context.getClassLoader());

            Class<?> clazz = classLoader.loadClass(TARGET_NAME);
            Log.d(TAG, "Load class : " +  TARGET_NAME
                        + " successfully with classLoader:" + classLoader);

            Constructor<?> constructor = clazz.getConstructor(Context.class);
            Context opContext = context.createPackageContext(PKG_NAME,
                    Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
            mPplExt = constructor.newInstance(opContext);
            Log.d(TAG, "[getInstance] return plugin:" + mPplExt);
            return (IPplSettingsEntryExt)mPplExt;
        } catch (NoSuchMethodException  | InvocationTargetException |
                 ClassNotFoundException | NameNotFoundException |
                 InstantiationException | IllegalAccessException e) {
            // Use default constructor
            Log.d(TAG, "Exception occurs when initial instance", e);
        }

        Log.d(TAG, "[getInstance] return default()");
        return new DefaultPplSettingsEntryExt();
    }

    private DefaultPplSettingsEntryExt() {
    }

    public void addPplPrf(PreferenceGroup prefGroup) {
        Log.d(TAG,"addPplPrf() default");
    }

    public void enablerResume() {
        Log.d(TAG,"enablerResume() default");
    }

    public void enablerPause() {
        Log.d(TAG,"enablerPause() default");
    }
}
