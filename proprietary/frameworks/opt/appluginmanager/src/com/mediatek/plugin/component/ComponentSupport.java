package com.mediatek.plugin.component;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.mediatek.plugin.utils.Log;

/**
 * ComponentSupport for start Activity.
 */
public class ComponentSupport {
    private static final String TAG = "PluginManager/ComponentSupport";
    public final static String KEY_TARGET_PLUGIN_ID = "targetTargetId";
    public final static String KEY_TARGET_ACTVITY = "targetActivityName";
    public final static String VALUE_PROXY_ACTIVITY_NAME =
            "com.mediatek.plugin.component.PluginProxyActivity";

    /**
     * Start activity that defined in plugin apk.
     * @param context
     *            The application context of host.
     * @param intent
     *            The intent to start.
     * @param pluginId
     *            The plugin ID.
     */
    public static void startActivity(Context context, Intent intent, String pluginId) {
        startActivity(context, intent, null, pluginId);
    }

    /**
     * Start activity that defined in plugin apk.
     * @param context
     *            The application context of host.
     * @param intent
     *            The intent to start.
     * @param options
     *            Additional options for how the Activity should be started.
     * @param pluginId
     *            The plugin ID.
     */
    public static void startActivity(Context context, Intent intent, Bundle options,
                                     String pluginId) {
        String className = intent.getComponent().getClassName();
        Log.d(TAG, "<startActivity> className = " + className + " pluginId = " + pluginId);
        intent.putExtra(KEY_TARGET_ACTVITY, className);
        intent.putExtra(KEY_TARGET_PLUGIN_ID, pluginId);
        intent.setClassName(context, VALUE_PROXY_ACTIVITY_NAME);
        context.startActivity(intent, options);
    }

    /**
     * Launch an activity for which you would like a result when it finished. <br>
     * The target activity is defined in plugin apk.
     * @param activity
     *            The activity of host.
     * @param intent
     *            The intent to start.
     * @param requestCode
     *            If >= 0, this code will be returned in onActivityResult().
     * @param pluginId
     *            The plugin ID.
     */
    public static void startActivityForResult(Activity activity, Intent intent,
                                              int requestCode, String pluginId) {
        startActivityForResult(activity, intent, requestCode, null, pluginId);
    }

    /**
     * Launch an activity for which you would like a result when it finished. <br>
     * The target activity is defined in plugin apk.
     * @param activity
     *            The activity of host.
     * @param intent
     *            The intent to start.
     * @param requestCode
     *            If >= 0, this code will be returned in onActivityResult().
     * @param options
     *            Additional options for how the Activity should be started.
     * @param pluginId
     *            The plugin ID.
     */
    public static void
            startActivityForResult(Activity activity, Intent intent, int requestCode,
                                   Bundle options, String pluginId) {
        String className = intent.getComponent().getClassName();
        Log.d(TAG, "<startActivity> className = " + className + " pluginId = " + pluginId);
        intent.putExtra(KEY_TARGET_ACTVITY, className);
        intent.putExtra(KEY_TARGET_PLUGIN_ID, pluginId);
        intent.setClassName(activity, VALUE_PROXY_ACTIVITY_NAME);
        activity.startActivityForResult(intent, requestCode, options);
    }
}
