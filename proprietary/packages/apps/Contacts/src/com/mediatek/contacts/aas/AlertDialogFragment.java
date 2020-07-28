package com.mediatek.contacts.aas;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnDismissListener;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputFilter;
import android.text.TextWatcher;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;

import com.android.contacts.R;

import com.mediatek.contacts.util.Log;

public class AlertDialogFragment extends DialogFragment implements DialogInterface.OnClickListener {
    public static final String TAG = "AlertDialogFragment";

    public static final String TITLE = "title";
    public static final String CANCELABLE = "cancelable";
    public static final String ICON = "icon";
    public static final String MESSAGE = "message";
    public static final String LAYOUT = "layout";
    public static final String NEGATIVE_TITLE = "negativeTitle";
    public static final String POSITIVE_TITLE = "positiveTitle";
    public static final int INVIND_RES_ID = -1;

    protected OnClickListener mDoneListener = null;
    protected OnDismissListener mDismissListener = null;

    @Override
    public void onSaveInstanceState(Bundle outState) {
        outState.putAll(getArguments());
        super.onSaveInstanceState(outState);
    }

    public static AlertDialogFragment newInstance(int title, int layout, int icon, int message,
            boolean cancelable, int negativeTitle, int positiveTitle) {
        AlertDialogFragment f = new AlertDialogFragment();
        f.setArguments(f.createBundle(title, layout, icon, message, cancelable, negativeTitle,
                positiveTitle));
        return f;
    }

    public Bundle createBundle(int title, int layout, int icon, int message, boolean cancelable,
            int cancelTitle, int doneTitle) {
        Bundle bundle = new Bundle();
        bundle.putInt(TITLE, title);
        bundle.putBoolean(CANCELABLE, cancelable);
        bundle.putInt(ICON, icon);
        bundle.putInt(MESSAGE, message);
        bundle.putInt(LAYOUT, layout);
        bundle.putInt(NEGATIVE_TITLE, cancelTitle);
        bundle.putInt(POSITIVE_TITLE, doneTitle);
        return bundle;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Log.d(TAG, "[onCreateDialog]");
        AlertDialog.Builder builder = alertDialogBuild(savedInstanceState);
        return builder.create();
    }

    protected Builder alertDialogBuild(Bundle savedInstanceState) {
        Bundle args = null;
        if (savedInstanceState != null) {
            args = savedInstanceState;
        } else {
            args = getArguments();
        }
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        if (args != null) {
            int title = args.getInt(TITLE, INVIND_RES_ID);
            if (title != INVIND_RES_ID) {
                builder.setTitle(title);
            }

            int icon = args.getInt(ICON, INVIND_RES_ID);
            if (icon != INVIND_RES_ID) {
                builder.setIcon(icon);
            }

            int message = args.getInt(MESSAGE, INVIND_RES_ID);
            int layout = args.getInt(LAYOUT, INVIND_RES_ID);
            if (layout != INVIND_RES_ID) {
                View view = getActivity().getLayoutInflater().inflate(layout, null);
                builder.setView(view);
            } else if (message != INVIND_RES_ID) {
                builder.setMessage(message);
            }

            int cancel = args.getInt(NEGATIVE_TITLE, INVIND_RES_ID);
            if (cancel != INVIND_RES_ID) {
                builder.setNegativeButton(cancel, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });
            }

            int done = args.getInt(POSITIVE_TITLE, INVIND_RES_ID);
            if (done != INVIND_RES_ID) {
                builder.setPositiveButton(done, this);
            }

            boolean cancelable = args.getBoolean(CANCELABLE, true);
            builder.setCancelable(cancelable);
        }
        return builder;
    }

    public void setOnDoneListener(OnClickListener listener) {
        mDoneListener = listener;
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        if (mDoneListener != null) {
            mDoneListener.onClick(dialog, which);
        }
    }

    public void setDismissListener(OnDismissListener listener) {
        mDismissListener = listener;
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        if (mDismissListener != null) {
            mDismissListener.onDismiss(dialog);
        }
        super.onDismiss(dialog);
    }

    public static class EditTextDialogFragment extends AlertDialogFragment {
        public static final String TAG = "EditTextDialogFragment";
        public static final String DEFAULT_STRING = "defaultString";
        public static final int ERROR = -1;
        private EditText mEditText;

        public interface EditTextDoneListener {
            void onEditTextDone(String text);
        }

        public static EditTextDialogFragment newInstance(int title, int negativeTitle,
                int positiveTitle, String defaultString) {
            EditTextDialogFragment f = new EditTextDialogFragment();
            Bundle bundle = f.createBundle(title, INVIND_RES_ID, INVIND_RES_ID, INVIND_RES_ID,
                    true, negativeTitle, positiveTitle);
            bundle.putString(DEFAULT_STRING, defaultString);
            f.setArguments(bundle);
            return f;
        }

        @Override
        public void onSaveInstanceState(Bundle outState) {
            Log.d(TAG, "[onSaveInstanceState] mEditText="
                    + Log.anonymize(mEditText.getText().toString()));
            getArguments().putString(DEFAULT_STRING, mEditText.getText().toString());
            super.onSaveInstanceState(outState);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            Log.d(TAG, "[onCreateDialog] dialog tag = " + getTag());
            AlertDialog.Builder builder = alertDialogBuild(savedInstanceState);
            Bundle args = null;
            if (savedInstanceState == null) {
                args = getArguments();
            } else {
                Log.sensitive(TAG, "[onCreateDialog] savedInstanceState="
                      + savedInstanceState.getString(DEFAULT_STRING, ""));
                args = savedInstanceState;
            }
            if (args != null) {
                String defaultString = args.getString(DEFAULT_STRING, "");
                View view = getActivity().getLayoutInflater().inflate(R.layout.dialog_edit_text,
                        null);
                builder.setView(view);
                mEditText = (EditText) view.findViewById(R.id.edit_text);
                mEditText.setFilters(new InputFilter[] { new InputFilter.AllCaps() });
                if (mEditText != null) {
                    mEditText.setText(defaultString);
                    mEditText.setSelection(defaultString.length());
                }
            }
            return builder.create();
        }

        @Override
        public void onResume() {
            super.onResume();
            if (mEditText != null && mEditText.getText().length() == 0) {
                final Button button = ((AlertDialog) getDialog())
                        .getButton(DialogInterface.BUTTON_POSITIVE);
                if (button != null) {
                    button.setEnabled(false);
                }
            }
            getDialog().getWindow().setSoftInputMode(
                    WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
            setTextChangedCallback(mEditText, (AlertDialog) getDialog());
        }

        /**
         * This method register callback and set filter to Edit, in order to make sure that user
         * input is legal. The input can't be illegal filename and can't be too long.
         */
        protected void setTextChangedCallback(EditText editText, final AlertDialog dialog) {
            editText.addTextChangedListener(new TextWatcher() {

                @Override
                public void afterTextChanged(Editable arg0) {
                }

                @Override
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                }

                @Override
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                    if (s.toString().length() <= 0) {
                        Button botton = dialog.getButton(DialogInterface.BUTTON_POSITIVE);
                        if (botton != null) {
                            botton.setEnabled(false);
                        }
                    } else {
                        Button botton = dialog.getButton(DialogInterface.BUTTON_POSITIVE);
                        if (botton != null) {
                            botton.setEnabled(true);
                        }
                    }
                }
            });
        }

        public String getText() {
            if (mEditText != null) {
                return mEditText.getText().toString().trim();
            }
            return null;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (!(getActivity() instanceof AasTagActivity)) {
                Log.w(TAG, "[onClick] not AasTagActivity, do nothing");
                return;
            }
            EditTextDoneListener listener = ((AasTagActivity) getActivity()).
                    getEditTextDoneListener(getTag());
            if (listener != null) {
                listener.onEditTextDone(getText().trim());
            }
        }
    }
}
