package com.mediatek.ppl.ui;

import com.mediatek.ppl.R;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;

public class DialogDisablePplFragment extends DialogFragment {

    public static DialogDisablePplFragment newInstance() {
        DialogDisablePplFragment frag = new DialogDisablePplFragment();
        return frag;
    }

    @Override
    public Dialog onCreateDialog(Bundle saveInstanceState) {


        return new AlertDialog.Builder(getActivity())
                .setMessage(R.string.description_disable_confirm)
                .setPositiveButton(android.R.string.ok,
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            ((ControlPanelActivity) getActivity()).onDiableConfirmed();
                        }
                    }
                )
                .setNegativeButton(android.R.string.cancel, null)
                .create();
    }
}
