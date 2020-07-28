package com.mediatek.ppl.ui;

import com.mediatek.ppl.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;

public class DialogChooseNumFragment extends DialogFragment {

    private static final String ARG_KEY_ITEMS = "items";
    private static final String ARG_KEY_NAME = "name";
    private static final String ARG_KEY_LINE_INDEX = "index";

    public static DialogChooseNumFragment newInstance(String[] items, String name, int idx) {

        DialogChooseNumFragment frag = new DialogChooseNumFragment();

        Bundle args = new Bundle();
        args.putStringArray(ARG_KEY_ITEMS, items);
        args.putString(ARG_KEY_NAME, name);
        args.putInt(ARG_KEY_LINE_INDEX, idx);
        frag.setArguments(args);
        return frag;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final Activity activity = getActivity();
        Bundle args = getArguments();
        final String[] items = args.getStringArray(ARG_KEY_ITEMS);
        final String name = args.getString(ARG_KEY_NAME);
        final int index = args.getInt(ARG_KEY_LINE_INDEX);
        return new AlertDialog.Builder(activity)
            .setTitle(R.string.title_choose_number)
            .setNegativeButton(android.R.string.cancel, null)
            .setItems(items, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    ((SetupTrustedContactsActivity) activity).setContectBySelect(items[which], name, index);
                }
            })
            .create();

    }

}
