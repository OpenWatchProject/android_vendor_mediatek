package com.mediatek.contacts.aas;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;

import com.mediatek.contacts.util.Log;

public class MessageAlertDialogFragment extends DialogFragment implements
        DialogInterface.OnClickListener {
    private static final String TAG = "MessageAlertDialogFragment";

    public static final String TITLE = "title";
    public static final String MESSAGE = "message";
    public static final String IS_OWN_CANCEL = "is_own_cancle";
    public static final String UPDATED_AAS_NAME = "updated_aas_name";
    private String mText = null;

    public static MessageAlertDialogFragment newInstance(int title,
            int message, boolean isOwnCancel, String text) {
        MessageAlertDialogFragment frag = new MessageAlertDialogFragment();
        Bundle args = new Bundle();
        args.putInt(TITLE, title);
        args.putInt(MESSAGE, message);
        args.putBoolean(IS_OWN_CANCEL, isOwnCancel);
        args.putString(UPDATED_AAS_NAME, text);
        frag.setArguments(args);
        return frag;
    }

    public interface AlertConfirmedListener {
        void onMessageAlertConfirmed(String text);
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        Log.d(TAG, "[onSaveInstanceState] updated_aas_name = "
                + Log.anonymize(getArguments().getString(UPDATED_AAS_NAME)));
        outState.putAll(getArguments());
        super.onSaveInstanceState(outState);
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Log.d(TAG, "[onCreateDialog] dialog tag = " + getTag());
        Bundle args = null;
        if (savedInstanceState != null) {
            args = savedInstanceState;
        } else {
            args = getArguments();
        }
        int title = args.getInt(TITLE);
        int message = args.getInt(MESSAGE);
        boolean isOwnCancel = args.getBoolean(IS_OWN_CANCEL);
        mText = args.getString(UPDATED_AAS_NAME);
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setIconAttribute(android.R.attr.alertDialogIcon)
                .setTitle(title).setMessage(message)
                .setPositiveButton(android.R.string.ok, this);
        if (isOwnCancel) {
            builder.setNegativeButton(android.R.string.cancel,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int whichButton) {
                        }
                    });
        }
        return builder.create();
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        if (!(getActivity() instanceof AasTagActivity)) {
            Log.w(TAG, "[onClick] not AasTagActivity, do nothing");
            return;
        }
        AlertConfirmedListener listener = ((AasTagActivity) getActivity()).
                getAlertConfirmedListener(getTag());
        if (listener != null) {
            listener.onMessageAlertConfirmed(mText);
        }
    }

}
