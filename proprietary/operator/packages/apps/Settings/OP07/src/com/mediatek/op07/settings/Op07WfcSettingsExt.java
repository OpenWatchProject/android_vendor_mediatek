package com.mediatek.op07.settings;

import android.content.Context;
import android.os.SystemProperties;
import android.util.Log;

import com.mediatek.settings.ext.DefaultWfcSettingsExt;

public class Op07WfcSettingsExt extends DefaultWfcSettingsExt {

    private static final String TAG = "Op07WfcSettingsExt";
    private Context mContext;
    public Op07WfcSettingsExt(Context context) {
        mContext = context;
    }
    @Override
    public boolean customizedATTWfcVisable() {
        // TODO Auto-generated method stub
        return isEntitlementEnabled();
    }

    public static boolean isEntitlementEnabled() {
        boolean isEntitlementEnabled = (1 == SystemProperties.getInt
                ("persist.vendor.entitlement_enabled", 1) ? true : false);
        Log.d(TAG, "isEntitlementEnabled:" + isEntitlementEnabled);
        return isEntitlementEnabled;
    }
}
