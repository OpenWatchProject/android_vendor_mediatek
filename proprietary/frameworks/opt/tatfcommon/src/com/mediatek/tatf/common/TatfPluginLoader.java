package com.mediatek.tatf.common;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.IBinder;
import android.util.Log;

import dalvik.system.PathClassLoader;

public class TatfPluginLoader {

    private static final String TAG = "TatfServer";

    private Context mContext;

    private static TatfPluginLoader sInstance = null;

    private TatfPluginLoader(Context context) {
        mContext = context;
    }

    public static TatfPluginLoader getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new TatfPluginLoader(context);
        }
        return sInstance;
    }

    /**
     * PluginIn Loader class, key: Action, values: Class
     */
   private static HashMap<String, Object> sPluginObject = new HashMap<String, Object>();

    /**
     * package name defined in Configuration.
     * TODO: action, class and package information in a configuration file.
     *
     * @param action
     *            for different plugIn
     */
    private void reloadClass(String action) {
        if (sPluginObject.get(action) != null) {
            // Exit a class, no need reload again
            return;
        }
        try {
            if (Configuration.INCALLUI_SERVICE_ACTION.equals(action)) {
                Context pluginContext = mContext.createPackageContext(
                        Configuration.PLUGIN_INCALLUI_PACKAGE, Context.CONTEXT_IGNORE_SECURITY
                                | Context.CONTEXT_INCLUDE_CODE);
                ClassLoader classLoader = new PathClassLoader(pluginContext.getPackageCodePath(),
                        mContext.getClassLoader());

                Class<?> inCallUIClass = classLoader.loadClass(Configuration.PLUGIN_INCALLUI_CLASS);
                sPluginObject.put(action, inCallUIClass.newInstance());
                Log.w(TAG, "TatfPluginLoader: reload class successed, for: " + action);
            } else {
                Log.w(TAG, "TatfPluginLoader: not support this kind of action: " + action);
            }
        } catch (NameNotFoundException | InstantiationException | ClassNotFoundException
                | IllegalAccessException e) {
            Log.d(TAG, "TatfPluginLoader: loader plugin failed. action: " + action +
                    " package: " + Configuration.PLUGIN_INCALLUI_PACKAGE, e);
        }
    }

    /**
     * Return Binder align with action.
     * @param action
     * @return Binder from PlugIn.
     */
    public IBinder getBinder(String action) {
        Object pluginObject = sPluginObject.get(action);
        if (pluginObject == null) {
            reloadClass(action);
        }
        pluginObject = sPluginObject.get(action);
        if (pluginObject == null) {
            // still null, failed or not support.
            Log.w(TAG, "getBinder failed, can't load class");
            return null;
        }
        IBinder ret = null;
        try {
            Class<?>[] args = new Class<?>[] { Context.class };
            Method method = pluginObject.getClass().getMethod("getBinder", args);
            if (method != null) {
                ret = (IBinder) method.invoke(pluginObject, mContext);
            }
        } catch (NoSuchMethodException | IllegalAccessException | IllegalArgumentException
                | InvocationTargetException e) {
            Log.e(TAG, "getBinder Exception", e);
        }
        return ret;
    }

}
