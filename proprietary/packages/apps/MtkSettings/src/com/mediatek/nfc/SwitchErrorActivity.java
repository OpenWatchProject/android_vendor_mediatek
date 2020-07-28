package com.mediatek.nfc;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.android.settings.R;

public class SwitchErrorActivity extends AlertActivity {
    private static final String TAG = "SwitchErrorActivity";

    private static final String FAIL_DIALOG_ACTION =
            "android.nfc.action.SWITCH_FAIL_DIALOG_REQUEST";
    private static final String NOT_SUPPORT_PROMPT_DIALOG_ACTION =
            "android.nfc.action.NOT_NFC_SIM_DIALOG_REQUEST";
    private static final String NOT_SUPPORT_PROMPT_TWO_SIM_DIALOG_ACTION =
            "android.nfc.action.NOT_NFC_TWO_SIM_DIALOG_REQUEST";
    private static final String EXTRA_WHAT_SIM = "android.nfc.extra.WHAT_SIM";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent i = getIntent();
        String action = i.getAction();
        if (FAIL_DIALOG_ACTION.equals(action)) {
            String mode = i.getStringExtra("mode");
            Log.d("@M_" + TAG, "switch fail mode is " + mode);
            showErrorDialog(mode);
        } else if (NOT_SUPPORT_PROMPT_DIALOG_ACTION.equals(action)) {
            String sim = i.getStringExtra(EXTRA_WHAT_SIM);
            Log.d("@M_" + TAG, "show not support dialog, sim is " + sim);
            showNotSupportDialog(sim);
        } else if (NOT_SUPPORT_PROMPT_TWO_SIM_DIALOG_ACTION.equals(action)) {
            Log.d("@M_" + TAG, "show not support dialog for SIM1 and SIM2");
            showTwoSimNotSupportDialog();
        } else {
            Log.e("@M_" + TAG, "Error: this activity may be started only with intent "
                  + "android.nfc.action.SWITCH_FAIL_DIALOG_REQUEST " + action);
            finish();
            return;
        }

    }

    private void showErrorDialog(String errorMode) {
        final AlertController.AlertParams p = mAlertParams;
        p.mIconId = android.R.drawable.ic_dialog_alert;
        p.mTitle = getString(R.string.card_emulation_switch_error_title);
        p.mMessage = getString(R.string.card_emulation_switch_error_message, errorMode);
        p.mPositiveButtonText = getString(android.R.string.ok);
        setupAlert();
    }

    private void showNotSupportDialog(String simDescription) {
        final AlertController.AlertParams p = mAlertParams;
        p.mIconId = android.R.drawable.ic_dialog_alert;
        p.mTitle = getString(R.string.card_emulation_switch_error_title);
        p.mMessage = getString(R.string.card_emulation_sim_not_supported_message, simDescription);
        p.mPositiveButtonText = getString(android.R.string.ok);
        setupAlert();
    }

    private void showTwoSimNotSupportDialog() {
        final AlertController.AlertParams p = mAlertParams;
        p.mIconId = android.R.drawable.ic_dialog_alert;
        p.mTitle = getString(R.string.card_emulation_switch_error_title);
        p.mMessage = getString(R.string.card_emulation_two_sim_not_supported_message);
        p.mPositiveButtonText = getString(android.R.string.ok);
        setupAlert();
    }

}
