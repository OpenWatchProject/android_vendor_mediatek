package com.mediatek.miravision.ui;

import com.mediatek.miravision.utils.Utils;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.CheckBox;

public class RemiderDialogFragment extends DialogFragment {

    private static final String TAG = "Miravision/RemiderDialogFragment";
    private AlertDialog mAlertDialog;
    private CheckBox mNotRemindAgainCheck;
    private Utils mUtils;

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Log.d(TAG, "RemiderDialogFragment onCreateDialog()");
        mUtils = new Utils(getActivity());
        mAlertDialog = new AlertDialog.Builder(getActivity()).setIcon(
                android.R.drawable.ic_dialog_alert).setTitle(R.string.picture_mode_user)
                .setView(createDialogView()).setPositiveButton(
                        Resources.getSystem().getIdentifier("yes", "string", "android"),
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                if (mNotRemindAgainCheck.isChecked()) {
                                    mUtils.setSharePrefBoolenValue(
                                    PictureModeFragment.SHARED_PREFERENCES_USER_MODE_KEY, true);
                                }
                            }
                        }).setCancelable(false).create();
        return mAlertDialog;
    }

    private View createDialogView() {
        final LayoutInflater layoutInflater = (LayoutInflater) getActivity().getSystemService(
                Context.LAYOUT_INFLATER_SERVICE);
        View view = layoutInflater.inflate(R.layout.notify_dialog_view, null);
        mNotRemindAgainCheck = (CheckBox) view.findViewById(R.id.checkbox);
        mNotRemindAgainCheck.setChecked(true);
        return view;
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "RemiderDialogFragment onDestroy()");
        mAlertDialog.dismiss();
        mAlertDialog = null;
        super.onDestroy();
    }
}
