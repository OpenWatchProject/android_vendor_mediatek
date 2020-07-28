package com.mediatek.miravision.ui;

import com.mediatek.miravision.setting.MiraVisionJni;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;

public class ResetDialogFragment extends DialogFragment {

    private ResetListener mResetListener;

    interface ResetListener {
        void reset();
    }

    public void setResetListener(ResetListener listener) {
        if (listener != null) {
            mResetListener = listener;
        }
    }

    public void clearResetListener() {
        mResetListener = null;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        return new AlertDialog.Builder(getActivity()).setMessage(R.string.reset_dialog_message)
        .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                MiraVisionJni.resetPQ(getActivity());
                if (mResetListener != null) {
                    mResetListener.reset();
                }
                clearResetListener();
            }
        }).setNegativeButton(android.R.string.cancel, null).create();
    }
}
