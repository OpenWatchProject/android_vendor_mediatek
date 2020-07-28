package com.mediatek.email.ui;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.text.SpannableStringBuilder;

import com.android.email.R;
import com.android.emailcommon.utility.Utility;

/**
 * M: Alert dialog for interpreting "Smart push" to the user.
 */
public class SmartPushAlertDialog extends DialogFragment {
    public final static String TAG = "SmartPushAlertDialog";
    private SmartPushDialogCallback mCallback;

    public static SmartPushAlertDialog newInstance(SmartPushDialogCallback callback) {
        SmartPushAlertDialog dialog = new SmartPushAlertDialog();
        dialog.setCallback(callback);
        return dialog;
    }

    private void setCallback(SmartPushDialogCallback callback) {
        this.mCallback = callback;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Context context = getActivity();
        SpannableStringBuilder ssb = new SpannableStringBuilder();
        Utility.appendBold(ssb,
                getString(R.string.account_setup_options_mail_check_frequency_smartpush));
        ssb.append(getString(R.string.smart_push_alert_dialog_message));
        Dialog dialog = new AlertDialog.Builder(context).setTitle(
                R.string.smart_push_alert_dialog_title).setMessage(ssb).setPositiveButton(
                getString(android.R.string.ok), new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dismiss();
                        // Record the dialog shown state.
                        if (mCallback != null) {
                            mCallback.onConfirm();
                        }
                    }
                }).create();

        return dialog;
    }

    public interface SmartPushDialogCallback {
        public void onConfirm();
    }
}