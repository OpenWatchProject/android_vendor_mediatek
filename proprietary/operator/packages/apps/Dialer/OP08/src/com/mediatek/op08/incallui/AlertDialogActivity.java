package com.mediatek.op08.incallui;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.CheckBox;

import com.mediatek.op08.dialer.R;

/**
 * show alert dialog.
 */
public class AlertDialogActivity extends Activity {
    private static final String TAG = "OP08AlertDialogActivity";
    private AlertDialog mAlertDialog = null;
    static final String KEY_IS_FIRST_WIFI_CALL = "key_first_wifi_call";

    @Override
    protected void onResume() {
        super.onResume();
        if (PreferenceManager.getDefaultSharedPreferences(this).getBoolean(
                KEY_IS_FIRST_WIFI_CALL, true)) {
            Log.d(TAG, "onResume showCongratsPopup");
            showCongratsPopup();
        } else {
            Log.d(TAG, "onResume dont show CongratsPopup, dont show again clicked");
            onDialogDismissed();
            finish();
        }
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

    private void showCongratsPopup() {
        Log.d(TAG, "showCongratsPopup");
        AlertDialog.Builder alert = new AlertDialog.Builder(this);
        LayoutInflater adbInflater = LayoutInflater.from(this);
        View checkBoxView = adbInflater.inflate(R.layout.mtk_first_wifi_call_ends, null);
        alert.setTitle(R.string.wifi_title);
        alert.setView(checkBoxView);
        CheckBox checkBox = (CheckBox) checkBoxView.findViewById(R.id.checkbox);
        alert.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Log.d(TAG, "[WFC] in onClick showCongratsPopup checkBox.isChecked()"
                        + checkBox.isChecked());
                if (checkBox.isChecked()) {
                    Log.d(TAG, "[WFC]showCongratsPopup checked True");
                    SharedPreferences  pref = PreferenceManager
                            .getDefaultSharedPreferences(getApplicationContext());
                    SharedPreferences.Editor editor = pref.edit();
                    editor.putBoolean(KEY_IS_FIRST_WIFI_CALL, false);
                    editor.commit();
                }
                onDialogDismissed();
                finish();
            }
        });
        alert.setOnCancelListener(new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                onDialogDismissed();
                finish();
            }
        });
        mAlertDialog = alert.create();
        mAlertDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        mAlertDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED);
        mAlertDialog.show();
    }

    private  void onDialogDismissed() {
        if (mAlertDialog != null) {
            mAlertDialog.dismiss();
            mAlertDialog = null;
        }
    }
}