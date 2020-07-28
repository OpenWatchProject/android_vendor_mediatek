package com.mediatek.email.ui;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.os.Bundle;

import com.android.email.R;

/**
 * M: Confirm dialog for respond inline.
 */
public class EditQuotedConfirmDialog extends DialogFragment implements
        DialogInterface.OnClickListener {
    public static final String TAG = "EditQuotedConfirmDialog";

    public interface Callback {
        void onConfimRespondInline();
    }

    /**
     * Create a new dialog.
     */
    public static EditQuotedConfirmDialog newInstance() {
        final EditQuotedConfirmDialog dialog = new EditQuotedConfirmDialog();
        return dialog;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final Context context = getActivity();
        final Resources res = context.getResources();
        final AlertDialog.Builder b = new AlertDialog.Builder(context);
        b.setTitle(res.getString(R.string.edit_quoted_dialog_title))
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setMessage(res.getString(R.string.edit_quoted_not_append_all))
                .setPositiveButton(android.R.string.ok, this)
                .setNegativeButton(android.R.string.cancel, this);
        Dialog d = b.create();
        d.setCanceledOnTouchOutside(false);
        return d;
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
        case DialogInterface.BUTTON_POSITIVE:
            getCallback().onConfimRespondInline();
            break;
        default:
            // do nothing
            dismissAllowingStateLoss();
        }
    }

    private Callback getCallback() {
        return (Callback) getActivity();
    }
}
