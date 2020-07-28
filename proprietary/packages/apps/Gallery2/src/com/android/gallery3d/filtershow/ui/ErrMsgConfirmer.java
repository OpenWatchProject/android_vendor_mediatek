package com.android.gallery3d.filtershow.ui;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.content.DialogInterface.OnKeyListener;
import android.os.Bundle;
import android.view.KeyEvent;

/**
 * M: use DialogFragment to show non-cancelable Dialog of confirmation use
 */
public class ErrMsgConfirmer extends DialogFragment {
    private static final String TAG = "Gallery2/ErrMsgConfirmer";
    private static final String KEY_MESSAGE = "message";

    private Runnable mConfirmCallBack;

    private static ErrMsgConfirmer newInstance(int messageID, final Runnable confirmCallBack) {
        ErrMsgConfirmer frag = new ErrMsgConfirmer();
        frag.mConfirmCallBack = confirmCallBack;
        Bundle args = new Bundle(1);
        args.putInt(KEY_MESSAGE, messageID);
        frag.setArguments(args);
        return frag;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Bundle args = getArguments();

        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setPositiveButton(android.R.string.ok,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        mConfirmCallBack.run();
                    }
        }).setCancelable(false);
        if (args.getInt(KEY_MESSAGE) > 0) {
            builder.setMessage(getString(args.getInt(KEY_MESSAGE)));
        }
        Dialog res = builder.create();
        res.setCanceledOnTouchOutside(false);
        res.setOnKeyListener(new OnKeyListener() {
            @Override
            public boolean onKey(DialogInterface arg0, int keyCode,
                    KeyEvent keyEvent) {
                if (keyCode == KeyEvent.KEYCODE_BACK
                        || keyCode == KeyEvent.KEYCODE_SEARCH
                        || keyCode == KeyEvent.KEYCODE_MENU) {
                    return true;
                }
                return false;
            }
        });
        return res;
    }

    private static final String errorDialogTag = "ERROR_DIALOG_TAG";
    private static android.app.FragmentManager sFragmentManager ;
    private static void dismissAllowingStateLoss(Activity activity) {
        sFragmentManager = activity.getFragmentManager();
        DialogFragment oldFragment = (DialogFragment) sFragmentManager
                .findFragmentByTag(errorDialogTag);
        if (null != oldFragment) {
            oldFragment.dismissAllowingStateLoss();
        }
    }

    public static void comfirmMessage(final Activity activity, int messageID,
            final Runnable confirmCallBack) {
        ErrMsgConfirmer.dismissAllowingStateLoss(activity);
        DialogFragment newFragment = ErrMsgConfirmer.newInstance(messageID, confirmCallBack);
        newFragment.show(sFragmentManager, errorDialogTag);
        sFragmentManager.executePendingTransactions();
    }
}
