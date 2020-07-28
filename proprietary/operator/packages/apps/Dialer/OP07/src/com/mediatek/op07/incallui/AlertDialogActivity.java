package com.mediatek.op07.incallui;

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

import com.mediatek.op07.dialer.R;

/**
 * Used to display an error dialog from within the plugin during handover in video calls.
 */
public class AlertDialogActivity extends Activity {
    private static final String TAG = "OP07AlertDialogActivity";
    private static final String ACTION_CLOSE_DIALOG =
            "com.mediatek.incallui.plugin.ACTION_CLOSE_DIALOG";
    private AlertDialog mAlertDialog = null;
    private BroadcastReceiver mErrorDialogReceiver = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mErrorDialogReceiver = new ErrorDialogReceiver();
        IntentFilter filter = new IntentFilter(ACTION_CLOSE_DIALOG);
        this.registerReceiver(mErrorDialogReceiver, filter);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mAlertDialog == null) {
            showErrorDialog();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        dismissDialog();
        this.unregisterReceiver(mErrorDialogReceiver);
        mErrorDialogReceiver = null;
    }

    private void showErrorDialog() {
        Log.v(TAG, "show error dialog");
        AlertDialog.Builder dialogBuild = new AlertDialog.Builder(this);
        dialogBuild.setMessage(R.string.handover_to_lte)
        .setTitle(android.R.string.dialog_alert_title)
        .setIcon(android.R.drawable.ic_dialog_alert)
        .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    finish();
                }
        })
        .create();
        mAlertDialog = dialogBuild.show();
        mAlertDialog.setCanceledOnTouchOutside(false);
        mAlertDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        mAlertDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED);
    }

    private  void dismissDialog() {
        if (mAlertDialog != null) {
            mAlertDialog.dismiss();
            mAlertDialog = null;
        }
    }

    /**
     * Used to display an error dialog from within the plugin during handover in video calls.
     */
    private class ErrorDialogReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            finish();
        }
    }
}