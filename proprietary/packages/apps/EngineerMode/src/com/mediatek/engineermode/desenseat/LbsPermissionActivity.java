package com.mediatek.engineermode.desenseat;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;

import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

/**
 * Activity to show Non-Framework LBS LOCATION permission dialog.
 * @author mtk80357
 *
 */
public class LbsPermissionActivity extends Activity {

    private static final int DIALOG_REENTER = 0;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        if (EmUtils.checkLocationProxyAppPermission(this, true)) {
            showDialog(DIALOG_REENTER);
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        // TODO Auto-generated method stub
        switch (id) {
        case DIALOG_REENTER:
            return new AlertDialog.Builder(this).setTitle(R.string.desense_at_warning_title)
                    .setMessage(R.string.desense_at_ate_reconnect_msg)
                    .setCancelable(false)
                    .setPositiveButton(R.string.dialog_ok,
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    finish();
                                }
                    }).create();
        default:
            return null;
        }

    }
}
