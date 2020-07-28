/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.dataprotection;

import com.android.internal.widget.LockPatternUtils;
import com.android.internal.widget.LockPatternView;
import com.android.internal.widget.LockPatternView.Cell;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnDismissListener;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.mediatek.dataprotection.utils.Log;

import java.util.List;

public class AlertDialogFragment extends DialogFragment implements
        OnClickListener {
    public static final String TAG = "AlertDialogFragment";

    private static final String TITLE = "title";
    private static final String CANCELABLE = "cancelable";
    private static final String ICON = "icon";
    private static final String MESSAGE = "message";
    private static final String LAYOUT = "layout";
    private static final String NEGATIVE_TITLE = "negativeTitle";
    private static final String POSITIVE_TITLE = "positiveTitle";

    public static final int INVIND_RES_ID = -1;

    protected OnClickListener mDoneListener;
    protected OnDismissListener mDismissListener = null;
    //protected ToastHelper mToastHelper = null;
    private OnDialogDismissListener mDialogDismissListener;

    private OnClickListener mCancelListener = null;

    @Override
    public void onSaveInstanceState(Bundle outState) {
        outState.putAll(getArguments());
        super.onSaveInstanceState(outState);
    }

    public static void showCancelTaskDialog(Activity activity,
            final DataProtectionService service, int id, String tag,
            final long taskId) {
        // Log.d(TAG, "showCancelDialog " + mNeedShowCancel + " " +
        // mCancelTitle);
        AlertDialogFragment listDialogFragment = (AlertDialogFragment) activity
                .getFragmentManager().findFragmentByTag(tag);
        final DialogInterface.OnClickListener cListener = new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                if (service != null) {
                    service.cancelTask(taskId);
                }
                dialog.dismiss();
            }
        };

        //always update fragment's title/message @{
        if (listDialogFragment != null) {
            dismissFragment(activity, listDialogFragment);
            listDialogFragment = null;
        }
        //@}

        if (activity.isResumed()) {
            Log.d(TAG, "showCancelTaskDialog - listDialogFragment == null");
            AlertDialogFragmentBuilder builder = new AlertDialogFragmentBuilder();
            listDialogFragment = builder.setDoneTitle(R.string.btn_ok)
                    .setCancelTitle(R.string.btn_cancel).setMessage(id)
                    .create();
            listDialogFragment.setOnDoneListener(cListener);
            // listDialogFragment.setOnDialogDismissListener(clickListener);

            listDialogFragment.show(activity.getFragmentManager(), tag);
            boolean ret = activity.getFragmentManager()
                    .executePendingTransactions();
        }
    }

    private static void dismissFragment(Activity activity, Fragment fragment) {
        if (fragment == null || activity == null) {
            return;
        }
        FragmentManager fragmentMgr = activity.getFragmentManager();
        FragmentTransaction transaction = fragmentMgr.beginTransaction();
        transaction.remove(fragment);
        transaction.commitAllowingStateLoss();
    }

    public static boolean dismissCancelTaskDialog(Activity activity,
            final DataProtectionService service, int id, String tag,
            final long taskId) {
        Log.d(TAG, "dismissCancelTaskDialog");
        AlertDialogFragment listDialogFragment = (AlertDialogFragment) activity
                .getFragmentManager().findFragmentByTag(tag);
        if (activity.isResumed()) {
            Log.d(TAG, "dismissCancelTaskDialog - activity isResumend()");
            if (listDialogFragment != null) {
                Log.d(TAG, "dismissCancelTaskDialog - listDialogFragment != null");
                listDialogFragment.dismiss();
                return true;
            } else {
                Log.d(TAG, "dismissCancelTaskDialog is null");
            }
        }
        return false;
    }

    public static class AlertDialogFragmentBuilder {
        protected final Bundle mBundle = new Bundle();

        /**
         * This method creates AlertDialogFragment with parameter of mBundle.
         *
         * @return AlertDialogFragment
         */
        public AlertDialogFragment create() {
            AlertDialogFragment f = new AlertDialogFragment();
            f.setArguments(mBundle);
            return f;
        }

        /**
         * This method sets TITLE for AlertDialogFragmentBuilder, which responds to title of dialog.
         *
         * @param resId resource id of title
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setTitle(int resId) {
            mBundle.putInt(TITLE, resId);
            return this;
        }

        /**
         * This method sets TITLE for AlertDialogFragmentBuilder, which responds to title of dialog.
         *
         * @param title title to show
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setTitle(String title) {
            mBundle.putString(TITLE, title);
            return this;
        }

        /**
         * This method sets LAYOUT for AlertDialogFragmentBuilder, which responds to layout of
         * dialog.
         *
         * @param resId resource id of layout
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setLayout(int resId) {
            mBundle.putInt(LAYOUT, resId);
            return this;
        }

        /**
         * This method sets CANCELABLE for AlertDialogFragmentBuilder (default value is true), which
         * responds to weather dialog can be canceled.
         *
         * @param cancelable true for can be canceled, and false for can not be canceled
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setCancelable(boolean cancelable) {
            mBundle.putBoolean(CANCELABLE, cancelable);
            return this;
        }

        /**
         * This method sets ICON for AlertDialogFragmentBuilder.
         *
         * @param resId resource id of icon
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setIcon(int resId) {
            mBundle.putInt(ICON, resId);
            return this;
        }

        /**
         * This method sets MESSAGE for AlertDialogFragmentBuilder, which is a string.
         *
         * @param resId resource id of message
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setMessage(int resId) {
            mBundle.putInt(MESSAGE, resId);
            return this;
        }

        /**
         * This method sets MESSAGE for AlertDialogFragmentBuilder, which is a string.
         *
         * @param message message to show
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setMessage(String message) {
            mBundle.putString(MESSAGE, message);
            return this;
        }

        /**
         * This method sets NEGATIVE_TITLE for AlertDialogFragmentBuilder, which responds to title
         * of negative button.
         *
         * @param resId resource id of title
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setCancelTitle(int resId) {
            mBundle.putInt(NEGATIVE_TITLE, resId);
            return this;
        }

        /**
         * This method sets POSITIVE_TITLE for AlertDialogFragmentBuilder, which responds to title
         * of positive button.
         *
         * @param resId resource id of title
         * @return AlertDialogFragmentBuilder
         */
        public AlertDialogFragmentBuilder setDoneTitle(int resId) {
            mBundle.putInt(POSITIVE_TITLE, resId);
            return this;
        }
    }

    /**
     * This method sets doneListenser for AlertDialogFragment
     *
     * @param listener doneListenser for AlertDialogFragment, which will response to press done
     *            button
     */
    public void setOnDoneListener(OnClickListener listener) {
        mDoneListener = listener;
    }

    public void setOnCancelListener(OnClickListener listener) {
        mCancelListener = listener;
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        if (mDoneListener != null) {
            mDoneListener.onClick(dialog, which);
        }
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        AlertDialog.Builder builder = createAlertDialogBuilder(savedInstanceState);
        return builder.create();
    }

    /**
     * This method gets a instance of AlertDialog.Builder
     *
     * @param savedInstanceState information for AlertDialog.Builder
     * @return
     */
    protected Builder createAlertDialogBuilder(Bundle savedInstanceState) {
        Bundle args = null;
        if (savedInstanceState == null) {
            args = getArguments();
        } else {
            args = savedInstanceState;
        }
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        if (args != null) {
            int titleRes = args.getInt(TITLE, INVIND_RES_ID);
            if (titleRes != INVIND_RES_ID) {
                builder.setTitle(titleRes);
            }

            String titleString = args.getString(TITLE);
            if (titleString != null) {
                builder.setTitle(titleString);
            }

            int icon = args.getInt(ICON, INVIND_RES_ID);
            if (icon != INVIND_RES_ID) {
                builder.setIcon(icon);
            }

            int messageRes = args.getInt(MESSAGE, INVIND_RES_ID);
            int layout = args.getInt(LAYOUT, INVIND_RES_ID);
            if (layout != INVIND_RES_ID) {
                View view = getActivity().getLayoutInflater().inflate(layout,
                        null);
                builder.setView(view);
            } else if (messageRes != INVIND_RES_ID) {
                builder.setMessage(messageRes);
            }

            String messageString = args.getString(MESSAGE);
            if (messageString != null) {
                builder.setMessage(messageString);
            }

            int cancel = args.getInt(NEGATIVE_TITLE, INVIND_RES_ID);

            if (cancel != INVIND_RES_ID) {
                builder.setNegativeButton(cancel, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                        if (mCancelListener != null) {
                            mCancelListener.onClick(dialog, id);
                        }
                    }
                });
            }

            int done = args.getInt(POSITIVE_TITLE, INVIND_RES_ID);
            if (done != INVIND_RES_ID) {
                builder.setPositiveButton(done, this);
            }

            //mToastHelper = new ToastHelper(getActivity());
            boolean cancelable = args.getBoolean(CANCELABLE, true);
            builder.setCancelable(cancelable);
        }
        return builder;
    }

    /**
     * This method sets dismissListener for AlertDialogFragment, which will response to
     * dismissDialog
     *
     * @param listener OnDismissListener for AlertDialogFragment
     */
    public void setDismissListener(OnDismissListener listener) {
        mDismissListener = listener;
    }

    /**
     * This method sets dismissListener for AlertDialogFragment, which will
     * response to dismissDialog
     *
     * @param listener
     *            OnDismissListener for AlertDialogFragment
     */
    public void setOnDialogDismissListener(OnDialogDismissListener listener) {
        mDialogDismissListener = listener;
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        if (mDismissListener != null) {
            mDismissListener.onDismiss(dialog);
        }
        if (mDialogDismissListener != null) {
            mDialogDismissListener.onDialogDismiss();
        }
        super.onDismiss(dialog);
    }

    public static class DynamicAlertDialogFragmentBuilder extends AlertDialogFragmentBuilder {

        @Override
        public DynamicAlertDialogFragment create() {
            DynamicAlertDialogFragment f = new DynamicAlertDialogFragment();
            f.setArguments(mBundle);
            return f;
        }

        public void setDefaultMessage(String message) {
            mBundle.putString(DynamicAlertDialogFragment.KEY_DEFAULT_MESSAGE,
                    message);
        }

/*        public void setLayout(int layoutId) {
            mBundle.putString(DynamicAlertDialogFragment.KEY_DEFAULT_MESSAGE,
                    message);
        }*/
    }

    public static class DynamicAlertDialogFragment extends AlertDialogFragment {
        public static final String KEY_DEFAULT_MESSAGE = "default_message";
        //public static final String KEY_DEFAULT_LAYOUT = "default_message";
        private TextView mContentView = null;
        private String mContent = null;
        private String mOkBtn = null;
        private int mCustomViewId = 0;

        public DynamicAlertDialogFragment() {
        }

        public DynamicAlertDialogFragment(String message, String okBtn, int id) {
            mContent = message;
            mOkBtn = okBtn;
            mCustomViewId = id;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            this.setOnDoneListener(this);
            AlertDialog.Builder builder = createAlertDialogBuilder(savedInstanceState);
            Bundle args = null;
            if (savedInstanceState == null) {
                args = getArguments();
            } else {
                args = savedInstanceState;
            }
            mCustomViewId = args.getInt(LAYOUT, 0);
            if (args != null && mCustomViewId > 0) {
                View view = getActivity().getLayoutInflater().inflate(
                        mCustomViewId, null);
                mContentView = (TextView) view.findViewById(R.id.message);
                mContentView.setText(args.getString(KEY_DEFAULT_MESSAGE, ""));
                builder.setView(view);
            }
            View view = getView();
            return builder.create();
        }

        public void setContentText(String text) {
            if (mContentView != null) {
                mContentView.setText(text);
            }
        }
    }

    public static class PatternInputDialogFragmentBuilder extends
            AlertDialogFragmentBuilder {
        @Override
        public PatternInputDialogFragment create() {
            PatternInputDialogFragment f = new PatternInputDialogFragment();
            f.setArguments(mBundle);
            return f;
        }

        public PatternInputDialogFragmentBuilder setUnlockFileName(
                String fileName) {
            mBundle.putString(PatternInputDialogFragment.KEY_UNLOCK_FILE,
                    fileName);
            // mBundle.putString(PatternInputDialogFragment.KEY_HEADER_TEXT,
            // headerText);
            return this;
        }

        public PatternInputDialogFragmentBuilder setDefault(String fileName) {
            mBundle.putString(PatternInputDialogFragment.KEY_UNLOCK_FILE,
                    fileName);
            // mBundle.putString(PatternInputDialogFragment.KEY_HEADER_TEXT,
            // headerText);
            return this;
        }
    }

    public static class PatternInputDialogFragment extends AlertDialogFragment {
        public static final String DEFAULT_STRING = "defaultString";
        public static final String DEFAULT_SELCTION = "defaultSelection";
        public static final String KEY_UNLOCK_FILE = "need_unlock_file";
        public static final String KEY_HEADER_TEXT = "header_text";

        public interface PasswordInputListener {
            void onPatternDetect(String password);
        }

        private TextView mHeaderText = null;
        private LockPatternView mPatternView = null;
        private PasswordInputListener mListener = null;
        private Button mBtnCancel = null;

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            this.setOnDoneListener(this);
            AlertDialog.Builder builder = createAlertDialogBuilder(savedInstanceState);
            Bundle args = null;
            if (savedInstanceState == null) {
                args = getArguments();
            } else {
                args = savedInstanceState;
            }
            if (args != null) {
                String fileName = args.getString(KEY_UNLOCK_FILE, "");
                String titleText = getString(R.string.pattern_input_title);
                //builder.setTitle(titleText);
                /* builder.setTitle(titleText + " <" + fileName + ">"); */
                View view = getActivity().getLayoutInflater().inflate(
                        R.layout.pattern_verify_dialog, null);
                mHeaderText = (TextView) view.findViewById(R.id.headerText);
                mHeaderText
                .setText(getString(R.string.unlock_pattern_header));
                mPatternView = (LockPatternView) view
                        .findViewById(R.id.lockPattern);
                mPatternView.setOnPatternListener(mPatternListener);
                mBtnCancel = (Button) view.findViewById(R.id.footerLeftButton);
                builder.setView(view);
            }
            View view = getView();
            /*
             * if (null != view) { mHeaderText = (TextView)
             * view.findViewById(R.id.headerText); mPatternView =
             * (LockPatternView) view.findViewById(R.id.lockPattern);
             * mPatternView.setOnPatternListener(mPatternListener); }
             */
            return builder.create();
        }

        private LockPatternView.OnPatternListener mPatternListener = new LockPatternView.OnPatternListener() {

            public void onPatternStart() {
            }

            public void onPatternCleared() {
            }

            public void onPatternCellAdded(List<Cell> pattern) {

            }

            public void onPatternDetected(List<LockPatternView.Cell> pattern) {
                Log.d(TAG, "onPatternDected....");
                if (pattern.size() < LockPatternUtils.MIN_LOCK_PATTERN_SIZE) {
                    //updateStage(Stage.ChoiceTooShort);
                    mHeaderText.setText(getResources().getString(R.string.lockpattern_recording_incorrect_too_short,
                            LockPatternUtils.MIN_LOCK_PATTERN_SIZE));
                } else {
                    Dialog dialog = getDialog();
                    if (dialog != null) {
                        if (mListener != null) {
                            mListener.onPatternDetect(LockPatternUtils
                                    .patternToString(pattern));
                        }
                        dialog.dismiss();
                    } else {
                        Log.d(TAG, "exception: cannot fetch dialog instance.");
                    }
                }
            }
        };

        public void setOnPatternInput(PasswordInputListener listener) {
            mListener = listener;
        }

        public void startCounter() {
            mCountdownTime.start();
        }

        private CountDownTimer mCountdownTime = new CountDownTimer(20 * 1000,
                1000) {

            @Override
            public void onFinish() {
                FragmentTransaction transaction = getFragmentManager()
                        .beginTransaction();
                transaction.remove(PatternInputDialogFragment.this);
                transaction.commitAllowingStateLoss();
            }

            @Override
            public void onTick(long arg0) {
                if (mBtnCancel != null) {
                    Log.d(TAG, "onTick " + "arg0 " + arg0);
                    mBtnCancel.setText("Cancel(" + arg0 / 1000 + ")");
                } else {
                    Log.d(TAG, "onTick button is null " + "arg0 " + arg0);
                }
            }

        };
    }

    public static class ChoiceDialogFragmentBuilder extends
            AlertDialogFragmentBuilder {
        @Override
        public ChoiceDialogFragment create() {
            ChoiceDialogFragment f = new ChoiceDialogFragment();
            f.setArguments(mBundle);
            return f;
        }

        /**
         * This method sets default choice and array for ChoiceDialogFragment.
         *
         * @param arrayId resource id for array
         * @param defaultChoice resource id for default choice
         * @return ChoiceDialogFragmentBuilder
         */
        public ChoiceDialogFragmentBuilder setDefault(int arrayId,
                int defaultChoice) {
            mBundle.putInt(ChoiceDialogFragment.DEFAULT_CHOICE, defaultChoice);
            mBundle.putInt(ChoiceDialogFragment.ARRAY_ID, arrayId);
            return this;
        }
    }

    public static class ChoiceDialogFragment extends AlertDialogFragment {
        public static final String CHOICE_DIALOG_TAG = "ChoiceDialogFragment";
        public static final String DEFAULT_CHOICE = "defaultChoice";
        public static final String ARRAY_ID = "arrayId";
        public static final String ITEM_LISTENER = "itemlistener";
        private int mArrayId;
        private int mDefaultChoice;
        private OnClickListener mItemLinster = null;

        /**
         * This method sets clickListener for ChoiceDialogFragment
         *
         * @param listener onClickListener, which will response press cancel button
         */
        public void setItemClickListener(OnClickListener listener) {
            mItemLinster = listener;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            //LogUtils.d(CHOICE_DIALOG_TAG, "Show alertSortDialog");
            AlertDialog.Builder builder = createAlertDialogBuilder(savedInstanceState);

            Bundle args = null;
            if (savedInstanceState == null) {
                args = getArguments();
            } else {
                args = savedInstanceState;
            }
            if (args != null) {
                mDefaultChoice = args.getInt(DEFAULT_CHOICE);
                mArrayId = args.getInt(ARRAY_ID);
            }
            builder.setSingleChoiceItems(mArrayId, mDefaultChoice, this);
            return builder.create();
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (mItemLinster != null) {
                mItemLinster.onClick(dialog, which);
            }
        }
    }

    public interface OnDialogDismissListener {
        void onDialogDismiss();
    }
}
