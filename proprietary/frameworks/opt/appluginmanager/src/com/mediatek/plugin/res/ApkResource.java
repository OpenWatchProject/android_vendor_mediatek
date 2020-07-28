package com.mediatek.plugin.res;

import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.util.DisplayMetrics;

import com.mediatek.plugin.utils.Log;
import com.mediatek.plugin.utils.ReflectUtils;
import com.mediatek.plugin.utils.TraceHelper;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;

/**
 * Parser Apk file resource.
 */
public class ApkResource implements IResource {
    private static final String TAG = "PluginManager/ApkResource";
    private static final String FORMAT_STRING = "@string/";
    private static final String FORMAT_DRAWABLE = "@drawable/";
    protected String mFilePath = null;
    protected String mPackageProcessName = null;
    private Resources mResources;
    private AssetManager mAssetManager;
    private DisplayMetrics mMetrics;
    private Configuration mConfiguration;

    /**
     * Construct.
     * @param context
     *            The package context.
     * @param filePath
     *            Apk file path.
     * @param packageName
     *            The Apk package name.
     */
    public ApkResource(Context context, String filePath, String packageName) {
        mMetrics = context.getResources().getDisplayMetrics();
        mConfiguration = context.getResources().getConfiguration();
        mFilePath = filePath;
        mPackageProcessName = packageName;
        Log.d(TAG, "<ApkResParser> mPackageProcessName = " + mPackageProcessName);
    }

    /**
     * Get current Resource.
     * @return the Resource.
     */
    public Resources getResources() {
        if (mResources == null) {
            getResource();
        }
        return mResources;
    }

    @Override
    public String getString(String value) {
        Log.d(TAG, "<getString> value = " + value);
        if (value == null || !value.startsWith(FORMAT_STRING)) {
            return value;
        }
        String resName = null;
        String defType = value.substring(value.indexOf("@") + 1, value.indexOf("/"));
        Log.d(TAG, "<getString> defType = " + defType);
        String name = value.substring(value.indexOf("/") + 1);
        Log.d(TAG, "<getString> name = " + name);
        int id = getResource().getIdentifier(name, defType, mPackageProcessName);
        Log.d(TAG, "<getString> id = " + id);
        resName = getResource().getString(id);
        return resName;
    }

    @Override
    public Drawable getDrawable(String value) {
        Log.d(TAG, "<getDrawable> value = " + value);
        if (value == null || !value.startsWith(FORMAT_DRAWABLE)) {
            return null;
        }
        Drawable drawable = null;
        String defType = value.substring(value.indexOf("@") + 1, value.indexOf("/"));
        Log.d(TAG, "<getDrawable> defType = " + defType);
        String name = value.substring(value.indexOf("/") + 1);
        Log.d(TAG, "<getDrawable> name = " + name);
        int id = getResource().getIdentifier(name, defType, mPackageProcessName);
        Log.d(TAG, "<getDrawable> id = " + id);
        if (id > 0) {
            drawable = getResource().getDrawable(id);
        }
        return drawable;
    }

    private Resources getResource() {
        if (mResources == null) {
            TraceHelper.beginSection(">>>>ApkResource-getResource");
            Constructor<?> con = ReflectUtils.getConstructor(AssetManager.class);
            mAssetManager = (AssetManager) ReflectUtils.createInstance(con);
            Method addAssertPath =
                    ReflectUtils.getMethod(mAssetManager.getClass(), "addAssetPath",
                            String.class);
            Log.d(TAG, "<getResource> addAssertPath " + mFilePath);
            ReflectUtils.callMethodOnObject(mAssetManager, addAssertPath, mFilePath);
            Resources resources = new Resources(mAssetManager, mMetrics, mConfiguration);
            Log.d(TAG, "<getResource> resources " + resources);
            mResources = resources;
            TraceHelper.endSection();
        }
        return mResources;
    }

    /**
     * Get Current AssetManager for pluginManager.
     * @return the AssetManager.
     */
    public AssetManager getAssetManager() {
        if (mAssetManager == null) {
            getResources();
        }
        return mAssetManager;
    }

    @Override
    public String[] getString(String value, String separater) {
        if (value != null) {
            return value.split(separater);
        }
        return null;
    }

}
