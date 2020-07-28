package com.mediatek.email.extension;

import com.android.emailcommon.VendorPolicyLoader;
import com.android.mail.utils.LogUtils;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/**
 * M: This receiver receives vendor email package change and locale change,
 * then reload vendor policy package's information to make it updated.
 *
 */
public class VendorPackageBroadcastReceiver extends BroadcastReceiver {

    private static final String TAG = LogUtils.TAG;
    @Override
    public void onReceive(Context context, Intent intent) {
        if (Intent.ACTION_LOCALE_CHANGED.equals(intent.getAction())) {
            LogUtils.d(TAG, "Locale changed, reload vendor policy");
            VendorPolicyLoader.reloadInstance(context);
        }
        if (Intent.ACTION_PACKAGE_ADDED.equals(intent.getAction())
                || Intent.ACTION_PACKAGE_CHANGED.equals(intent.getAction())
                || Intent.ACTION_PACKAGE_REPLACED.equals(intent.getAction())) {
            LogUtils.d(TAG, "Vendor policy apk has been changed, need to relaunch");
            System.exit(-1);
        }
    }

}
