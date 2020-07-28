package com.debug.loggerui;

import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.view.WindowManager;

import com.debug.loggerui.utils.Utils;

/**
 * Global space for storing layoutparams.
 */
public class MyApplication extends Application {

    private static final String TAG = Utils.TAG + "/MyApplication";
    private WindowManager.LayoutParams mLayoutParams = new WindowManager.LayoutParams();

    private static MyApplication sInstance;

    private SharedPreferences mSharedPreferences;
    private SharedPreferences mDefaultSharedPreferences;

    /**
     * @return LayoutParams
     */
    public WindowManager.LayoutParams getMyLayoutParams() {
        return mLayoutParams;
    }

    /**
     * @return SharedPreferences
     */
    public SharedPreferences getSharedPreferences() {
        return mSharedPreferences;
    }

    /**
     * @return SharedPreferences
     */
    public SharedPreferences getDefaultSharedPreferences() {
        return mDefaultSharedPreferences;
    }

    /**
     * @return MyApplication
     */
    public static MyApplication getInstance() {
        return sInstance;
    }

    @Override
    public void onCreate() {
        Utils.logi(TAG, "-->onCreate()");
        sInstance = this;
        mSharedPreferences = getSharedPreferences(Utils.CONFIG_FILE_NAME,
                Context.MODE_PRIVATE);
        mDefaultSharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);
        super.onCreate();
    }

}
