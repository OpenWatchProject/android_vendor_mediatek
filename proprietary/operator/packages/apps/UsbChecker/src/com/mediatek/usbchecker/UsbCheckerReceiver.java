package com.mediatek.usbchecker;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.UserHandle;
import android.util.Log;

public class UsbCheckerReceiver extends BroadcastReceiver{

    private static final String TAG = "UsbChecker.UsbCheckerReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (!UsbCheckerService.isFeatureSupported()) {
            Log.i(TAG, "Feature not enable, do nothing");
            return;
        }

        if (UserHandle.myUserId() != UserHandle.USER_SYSTEM) {
            Log.i(TAG, "Current user not system, do nothing");
            return;
        }

        if (UsbCheckerService.getActivateState()) {
            Log.i(TAG, "Device already activated, do nothing");
            return;
        }

        if (intent != null) {
            String action = intent.getAction();

            if (UsbCheckerService.USB_INTENT.equals(action) ||
                    UsbCheckerService.SIM_INTENT.equals(action) ||
                    Intent.ACTION_BOOT_COMPLETED.equals(action) ||
                    Intent.ACTION_BATTERY_CHANGED.equals(action)) {
                intent.setClass(context, UsbCheckerService.class);
                context.startService(intent);
            } else {
                Log.e(TAG, "Not expected action " + action + ", do nothing");
            }
        } else {
            Log.e(TAG, "intent is null");
        }
    }
}
