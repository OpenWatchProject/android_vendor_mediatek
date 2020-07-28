package com.mediatek.usbchecker;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;

import com.android.internal.app.AlertController;
import com.android.internal.app.AlertActivity;

public class UsbCheckerActivity extends AlertActivity
        implements DialogInterface.OnClickListener {
    private static final String TAG = "UsbChecker.Activity";

    private UsbChekcerFinishedReceiver mUsbCheckerFinishedReceiver;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mUsbCheckerFinishedReceiver = new UsbChekcerFinishedReceiver(this);

        final AlertController.AlertParams ap = mAlertParams;
        ap.mTitle = getString(R.string.alert_title);
        ap.mMessage = getString(R.string.alert_content);
        ap.mPositiveButtonText = getString(android.R.string.ok);
        ap.mPositiveButtonListener = this;

        setupAlert();
    }

    private class UsbChekcerFinishedReceiver extends BroadcastReceiver {
        private final Activity mActivity;
        public UsbChekcerFinishedReceiver(Activity activity) {
            mActivity = activity;
        }

        @Override
        public void onReceive(Context content, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "UsbChekcerFinishedReceiver onReceive:" + action);
            if (UsbCheckerConstants.INTENT_USB_CHECKER_FINISH.equals(action)) {
                mActivity.finish();
            }
        }
    }

    @Override
    public void onStart() {
        Log.d(TAG, "onStart()");
        super.onStart();
        IntentFilter filter = new IntentFilter(UsbCheckerConstants.INTENT_USB_CHECKER_FINISH);
        registerReceiver(mUsbCheckerFinishedReceiver, filter);
    }

    @Override
    protected void onStop() {
        Log.d(TAG, "onStop()");
        if (mUsbCheckerFinishedReceiver != null) {
            unregisterReceiver(mUsbCheckerFinishedReceiver);
        }
        super.onStop();
    }

    public void onClick(DialogInterface dialog, int which) {
        Log.d(TAG, "ok button onClick()");
        finish();
    }
}
