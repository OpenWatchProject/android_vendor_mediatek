package com.mediatek.op07.telecom;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

/**
 * Used to display an alert dialog from the plugin.
 */
public class AlertDialogActivity extends Activity {
    private static final String TAG = "OP07AlertDialogActivity";
    static final String ERROR_MESSAGE_ID_EXTRA = "error_message_id";
    private AlertDialog mGeneralAlertDialog;

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
        if (mGeneralAlertDialog != null) {
            mGeneralAlertDialog.dismiss();
        }
        showAlertDialog();
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (mGeneralAlertDialog != null) {
            mGeneralAlertDialog.dismiss();
            mGeneralAlertDialog = null;
        }
    }

    private void showAlertDialog() {
        final int error = getIntent().getIntExtra(ERROR_MESSAGE_ID_EXTRA, -1);
        if (error == -1) {
            finish();
        }
        showErrorDialog(error);
    }

    private void showErrorDialog(int resid) {
        Log.d(TAG, "showErrorDialog");
        final CharSequence msg = getResources().getText(resid);
        final DialogInterface.OnClickListener clickListener;
        final DialogInterface.OnCancelListener cancelListener;

        clickListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if (mGeneralAlertDialog != null) {
                    mGeneralAlertDialog.dismiss();
                    mGeneralAlertDialog = null;
                }
                finish();
            }
        };
        cancelListener = new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                if (mGeneralAlertDialog != null) {
                    mGeneralAlertDialog.dismiss();
                    mGeneralAlertDialog = null;
                }
                finish();
            }
        };

        mGeneralAlertDialog = new AlertDialog.Builder(this)
                .setMessage(msg).setPositiveButton(android.R.string.ok, clickListener)
                        .setOnCancelListener(cancelListener).create();

        mGeneralAlertDialog.show();
    }
}
