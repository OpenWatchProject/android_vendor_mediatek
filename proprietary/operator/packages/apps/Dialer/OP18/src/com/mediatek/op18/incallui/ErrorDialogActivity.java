package com.mediatek.op18.incallui;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

import com.mediatek.op18.dialer.R;

/**
 * Used to display an error dialog from within the plugin when cerating outgoing call fails.
 */
public class ErrorDialogActivity extends Activity {
    private static final String TAG = ErrorDialogActivity.class.getSimpleName();
    private AlertDialog mGeneralErrorDialog = null;
    public static final String SHOW_VIDEO_CALL = "show_video";
    public static final String DOWNGRADE_TO_AUDIO = "com.mtk.plugin.DOWNGRADE_TO_AUDIO";
    public static final String DISMISS_VIDEO_CALL = "remove_video";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onPause() {
        super.onPause();
        //finish();
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


    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mGeneralErrorDialog != null) {
            mGeneralErrorDialog.dismiss();
            mGeneralErrorDialog = null;
        }
        getApplicationContext().unregisterReceiver(mReceiver);

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
            if (DISMISS_VIDEO_CALL.equals(intent.getAction())) {
                Log.d(TAG, "showGenericErrorDialog DISMISS_VIDEO_CALL");
                onDialogDismissed();
                finish();
            }
        }
    };

    private void showGenericErrorDialog() {
        if (mGeneralErrorDialog == null) {
            Log.d(TAG, "showGenericErrorDialog");
            IntentFilter filter = new IntentFilter();
            filter.addAction(DISMISS_VIDEO_CALL);
            getApplicationContext().registerReceiver(mReceiver, filter);
            Log.d(TAG, "showGenericErrorDialog this " + this);
            mGeneralErrorDialog = new AlertDialog.Builder(this)
            .setTitle(R.string.low_battery_warning)
            .setMessage(R.string.low_battery_detail)
            .setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    onDialogDismissed();
                    finish();
                }
            })
            .setNegativeButton(R.string.convert, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    Intent broadcastIntent = new Intent(DOWNGRADE_TO_AUDIO);
                    getApplicationContext().sendBroadcast(broadcastIntent);
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
            mGeneralErrorDialog.getWindow().addFlags(
                WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED);
            mGeneralErrorDialog.show();
        }
    }

    private  void onDialogDismissed() {
        if (mGeneralErrorDialog != null) {
            mGeneralErrorDialog.dismiss();
            mGeneralErrorDialog = null;
        }
    }
}
