package com.mediatek.systemui.op09clip;

import android.content.Context;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.systemui.ext.DefaultQuickSettingsPlugin;

/**
 * Customize carrier text.
 */
public class Op09QuickSettingsPlugin extends DefaultQuickSettingsPlugin {
    public static final String TAG = "Op09QuickSettingsPlugin";

    private Context mContext;
    private TelephonyManager mTelephonyManager;

    /**
     * Constructs a new OP09QuickSettingsPlugin instance with Context.
     * @param context A Context object
     */
    public Op09QuickSettingsPlugin(Context context) {
        super(context);
        mContext = context;
        mTelephonyManager = TelephonyManager.from(context);
    }

    @Override
    public void disableDataForOtherSubscriptions() {
        int[] subList = SubscriptionManager.from(mContext).getActiveSubscriptionIdList();
        int defaultDataSubId = SubscriptionManager.getDefaultDataSubscriptionId();
        for (int subId : subList) {
            if (subId != defaultDataSubId && mTelephonyManager.getDataEnabled(subId)) {
                Log.d(TAG, "Disable other sub's data : " + subId);
                mTelephonyManager.setDataEnabled(subId, false);
            }
        }
    }
}
