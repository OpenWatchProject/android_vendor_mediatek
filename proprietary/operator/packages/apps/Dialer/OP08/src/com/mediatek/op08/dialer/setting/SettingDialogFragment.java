package com.mediatek.op08.dialer.setting;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.content.Context;
import android.os.Bundle;
import android.widget.EditText;
import android.view.View;

import android.util.Log;
import com.mediatek.op08.dialer.R;

public class SettingDialogFragment extends DialogFragment {

    private static final String TAG = "SettingDialogFragment";
    private static final String ARG_TEXT = "enteredText";
    private static final String DIALOG_SETTING_FRAGMENT = "SettingDialogFragment";
    private static final String ARG_TITLE = "argTitle";
    private static final String ARG_CONTENT = "argContent";
    private String mTitle;
    private String mContent;

    private EditText mEditText;
    private Callback mCallback;

    public static SettingDialogFragment show(String tilte, String content,
            FragmentManager fragmentManager, Callback callback) {
        Log.d(TAG, "SettingDialogFragment SHOW tilte:" + tilte);
        final SettingDialogFragment newFragment = SettingDialogFragment
                .newInstance(tilte, content);
        newFragment.setCallback(callback);
        newFragment.show(fragmentManager, DIALOG_SETTING_FRAGMENT);
        return newFragment;
    }

    private static SettingDialogFragment newInstance(String tilte,
            String content) {
        final SettingDialogFragment fragment = new SettingDialogFragment();
        final Bundle args = new Bundle();
        args.putString(ARG_TITLE, tilte);
        args.putString(ARG_CONTENT, content);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        super.onCreateDialog(savedInstanceState);

        mTitle = getArguments().getString(ARG_TITLE);
        mContent = getArguments().getString(ARG_CONTENT);

        View dialogView = View.inflate(getActivity(), R.layout.setting_edit_dialog, null);
        mEditText = (EditText) dialogView.findViewById(R.id.content_deschription);
        if (savedInstanceState != null) {
            Log.d(TAG, "onCreateDialog savedInstanceState != null");
            mEditText.setText(savedInstanceState.getCharSequence(ARG_TEXT));
        } else {
            mEditText.setText(mContent);
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity())
                .setTitle(mTitle)
                .setView(dialogView)
                .setPositiveButton(android.R.string.ok,
                        (dialog, id) -> {
                            Log.d(TAG, "click ok button.");
                                dismiss();
                                mCallback.onEnterContentDone(mEditText.getText().toString());
                            })
                 .setNegativeButton(android.R.string.cancel, null);
        Dialog settingDialog = builder.create();
        settingDialog.setCanceledOnTouchOutside(true);
        return settingDialog;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
    }

    @Override
    public void onPause() {
        // Dismiss on rotation.
        dismiss();
        mCallback = null;
        super.onPause();
    }

    public void setCallback(Callback callback) {
        mCallback = callback;
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        Log.d(TAG, "onSaveInstanceState");
        super.onSaveInstanceState(outState);
        outState.putCharSequence(ARG_TEXT, mEditText.getText());
    }

    public interface Callback {
        void onEnterContentDone(String result);
    }
}
