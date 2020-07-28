package com.mediatek.ppl.ui;

import com.mediatek.ppl.PlatformManager;
import com.mediatek.ppl.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;

public class DialogTurnoffAirplaneFragment extends DialogFragment {

    public static DialogTurnoffAirplaneFragment newInstance() {
        DialogTurnoffAirplaneFragment frag = new DialogTurnoffAirplaneFragment();
        return frag;
    }

    public Dialog onCreateDialog(Bundle saveInstanceState) {

        final Activity activity = getActivity();

        return new AlertDialog.Builder(getActivity())
                .setMessage(R.string.description_turnoff_airplane_mode_confirm)
                .setPositiveButton(android.R.string.ok,
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            PlatformManager.turnOffAirplaneMode(activity);
                        }
                    }
                )
                .setNegativeButton(android.R.string.cancel, null)
                .create();
    }
}
