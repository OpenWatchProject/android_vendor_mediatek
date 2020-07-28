package com.mediatek.op18.telecom;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.Bundle;
import android.util.Log;
import android.support.v4.content.LocalBroadcastManager;

/**
 * Used to display an error dialog from within the plugin when cerating outgoing call fails.
 */
public class ErrorDialogActivity extends Activity {
    private static final String TAG = ErrorDialogActivity.class.getSimpleName();
    public static final String SHOW_VIDEO_CALL = "show_video";
    public static final String DOWNGRADE_TO_AUDIO = "com.mtk.plugin.DOWNGRADE_TO_AUDIO";
    public static final String CONTINUE_AS_VIDEO = "com.mtk.plugin.CONTINUE_AS_VIDEO";
    private static final float BATTERY_PERCENTAGE = 0.15f;
    private AlertDialog mGeneralErrorDialog = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onPause() {
        super.onPause();
        finish();
    }

    @Override
    public void finish() {
        super.finish();
        // Don't show the return to previous task animation to avoid showing a black screen.
        // Just dismiss the dialog and undim the previous activity immediately.
        overridePendingTransition(0, 0);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        setIntent(intent);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mGeneralErrorDialog != null) {
            mGeneralErrorDialog.dismiss();
        }
        showErrorDialog();
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (mGeneralErrorDialog != null) {
            mGeneralErrorDialog.dismiss();
            mGeneralErrorDialog = null;
        }
    }

    private void showErrorDialog() {
        boolean showBatteryPopup = getIntent().getBooleanExtra(SHOW_VIDEO_CALL, false);
        if (showBatteryPopup) {
            showGenericErrorDialog();
            return;
        }
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.v(TAG, action);
            if (Intent.ACTION_BATTERY_CHANGED.equals(intent.getAction())) {
                int level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0);
                int scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, 0);
                float batteryPct = level / (float) scale;
                int plugged = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
                boolean isPlugged = plugged == BatteryManager.BATTERY_PLUGGED_AC
                          || plugged == BatteryManager.BATTERY_PLUGGED_USB;
                Log.d(TAG, "Telecom ACTION_BATTERY_CHANGED batteryPct " + batteryPct
                        + "isPlugged" + isPlugged + "mGeneralErrorDialog" + mGeneralErrorDialog);
                if (mGeneralErrorDialog != null && isPlugged) {
                    onDialogDismissed();
                    finish();
                    Intent broadcastIntent = new Intent(CONTINUE_AS_VIDEO);
                    LocalBroadcastManager.getInstance(getApplicationContext())
                      .sendBroadcast(broadcastIntent);
                }
            }
        }
    };

    private void showGenericErrorDialog() {
        Log.d(TAG, "Telecom showGenericErrorDialog");
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_BATTERY_CHANGED);
        getApplicationContext().registerReceiver(mReceiver, filter);
        mGeneralErrorDialog = new AlertDialog.Builder(this)
            .setTitle(R.string.low_battery_warning)
            .setMessage(R.string.low_battery_detail)
            .setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    Intent broadcastIntent = new Intent(CONTINUE_AS_VIDEO);
                    LocalBroadcastManager.getInstance(getApplicationContext())
                      .sendBroadcast(broadcastIntent);
                    onDialogDismissed();
                    finish();
                }
            })
            .setNegativeButton(R.string.convert, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    Intent broadcastIntent = new Intent(DOWNGRADE_TO_AUDIO);
                    LocalBroadcastManager.getInstance(getApplicationContext())
                      .sendBroadcast(broadcastIntent);
                    onDialogDismissed();
                    finish();
                }
            })
            .setOnCancelListener(new DialogInterface.OnCancelListener() {
                 @Override
                 public void onCancel(DialogInterface dialog) {
                     onDialogDismissed();
                     finish();
                }
            }).create();
        mGeneralErrorDialog.show();

    }

    private  void onDialogDismissed() {
        if (mGeneralErrorDialog != null) {
            mGeneralErrorDialog.dismiss();
            mGeneralErrorDialog = null;
        }
    }
}
