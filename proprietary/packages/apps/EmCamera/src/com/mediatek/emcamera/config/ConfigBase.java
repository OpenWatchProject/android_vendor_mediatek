package com.mediatek.emcamera.config;

import android.app.Activity;
import android.content.SharedPreferences;
import android.hardware.Camera.Parameters;

public abstract class ConfigBase {
    protected Activity mActivity;

    ConfigBase(Activity activity) {
        mActivity = activity;
    }
    public abstract void initComponents();
    public abstract void initSupportedUi(Parameters parameters);
    public abstract void setStatusToDefault();
    public abstract boolean saveValues(SharedPreferences.Editor editor);
}
