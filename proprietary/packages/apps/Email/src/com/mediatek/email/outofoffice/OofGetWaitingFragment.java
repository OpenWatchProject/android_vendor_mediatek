package com.mediatek.email.outofoffice;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnCancelListener;
import android.os.Bundle;
import android.os.RemoteException;
import android.util.Log;

import com.android.email.R;
import com.android.email.service.EmailServiceUtils;
import com.android.mail.utils.LogUtils;

import com.android.emailcommon.mail.ServerCommandInfo;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.service.EmailServiceProxy;
import com.android.emailcommon.service.OofParams;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.android.emailcommon.utility.Utility;
import com.mediatek.mail.ui.utils.UiUtilities;

public class OofGetWaitingFragment extends Fragment {
    public static final String TAG = "OofGetWaitingFragment";
    public static final String BUNDLE_KEY = "OofGetWaitingFragment.accountId";

    private final static long DEFAULT_ID = -1;
    // State
    private final static int STATE_START = Integer.MIN_VALUE;
    private int mState = STATE_START;

    // Support for UI
    public boolean mAttached;
    private boolean mPaused;
    private WaitingSaveOofDialog mCheckingDialog;
    private Account mAccount;
    private Context mContext;
    private long mAccountId = DEFAULT_ID;
    private AlertDialog mAlertDialog = null;

    // Support for AsyncTask and account checking
    OofTask mOofCheckTask;
    OofParams mOofParams;


    public OofGetWaitingFragment() {
        super();
    }

    public OofGetWaitingFragment(long accountId) {
        mAccountId = accountId;
    }

    /**
     * Create a retained, invisible fragment that checks accounts
     *
     * @param mode incoming or outgoing
     */
    public static OofGetWaitingFragment newInstance(long accountId, Fragment parentFragment) {
        OofGetWaitingFragment f = new OofGetWaitingFragment(accountId);
        f.setTargetFragment(parentFragment, 0);
        return f;
    }

    /**
     * Fragment initialization.  Because we never implement onCreateView, and call
     * setRetainInstance here, this creates an invisible, persistent, "worker" fragment.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRetainInstance(true);
        if (savedInstanceState != null) {
            mAccountId = savedInstanceState.getLong(BUNDLE_KEY);
        }
        mAccount = Account.restoreAccountWithId(mContext, mAccountId);
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        outState.putLong(BUNDLE_KEY, mAccountId);
        super.onSaveInstanceState(outState);
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        mContext = activity;
    }

    /**
     * This is called when the Fragment's Activity is ready to go, after
     * its content view has been installed; it is called both after
     * the initial fragment creation and after the fragment is re-attached
     * to a new activity.
     */
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mAttached = true;

        // If this is the first time, start the AsyncTask
        if (mOofCheckTask == null) {
            mOofCheckTask = (OofTask)
                    new OofTask().executeParallel();
            LogUtils.d(TAG, this + " ### onActivityCreated  #### "
                    + "  " + mOofCheckTask);
        }
    }

    /**
     * When resuming, restart the progress/error UI if necessary by re-reporting previous values
     */
    @Override
    public void onResume() {
        super.onResume();
        mPaused = false;
        if (mState != STATE_START) {
            reportProgress(mState);
        }
    }

    @Override
    public void onPause() {
        mPaused = true;
        if (mAlertDialog != null) {
            mAlertDialog.dismiss();
            mAlertDialog = null;
        }
        super.onPause();
    }

    /**
     * This is called when the fragment is going away.  It is NOT called
     * when the fragment is being propagated between activity instances.
     */
    @Override
    public void onDestroy() {
        Utility.cancelTaskInterrupt(mOofCheckTask);
        mOofCheckTask = null;
        super.onDestroy();
    }

    /**
     * This is called right before the fragment is detached from its current activity instance.
     * All reporting and callbacks are halted until we reattach.
     */
    @Override
    public void onDetach() {
        super.onDetach();
        mAttached = false;
    }

    private void setupWaitingDialog(FragmentManager fm) {
        // Display a normal progress message
        mCheckingDialog = (WaitingSaveOofDialog) fm.findFragmentByTag(WaitingSaveOofDialog.TAG);
        if (mCheckingDialog == null) {
            mCheckingDialog = WaitingSaveOofDialog.newInstance(this);
            fm.beginTransaction()
                    .add(mCheckingDialog, WaitingSaveOofDialog.TAG)
                    .commitAllowingStateLoss();
        }
    }
    /**
     * The worker (AsyncTask) will call this (in the UI thread) to report progress.  If we are
     * attached to an activity, update the progress immediately;  If not, simply hold the
     * progress for later.
     * @param newState The new progress state being reported
     */
    private void reportProgress(int newState) {
        mState = newState;
        // If we are attached, create, recover, and/or update the dialog
        if (mAttached && !mPaused) {
            final FragmentManager fm = getFragmentManager();

            switch (newState) {
                case STATE_START:
                    setupWaitingDialog(fm);
                    break;
                // we just catch network error at here, if server not support Oof or something else,
                // alert when save Oof.
                case ServerCommandInfo.OofInfo.NETWORK_SHUT_DOWN:
                    if (UiUtilities.isWifiOnly(mContext)) { //For wifi only case,ALPS00458924.
                            showAlertDialog(fm, R.string.unable_to_connect,
                                R.string.need_wifi_connection_prompt);
                    } else {
                        showAlertDialog(fm, R.string.unable_to_connect,
                            R.string.need_connection_prompt);
                    }
                    break;
                case ServerCommandInfo.OofInfo.SYNC_OOF_UNINITIALIZED:
                    // Account may not initialized
                    showAlertDialog(fm, R.string.account_settings_oof_get_status_fail,
                            R.string.account_settings_oof_get_status_fail_summary);
                    break;
                default:
                    // immediately terminate, clean up, and report back
                    // 1. get rid of progress dialog (if any)
                    recoverAndDismissCheckingDialog();
                    // 2. exit self
                    fm.popBackStack();
                    // 3. report OK back to target fragment or activity
                    // If the fragment in background just save check_status and send call on onResume.
                    // or send the call back immediately.
                    startOofActivity();
                    break;
            }
        }
    }

    /**
     * Show the alert dialog when get status failed
     */
    private void showAlertDialog(final FragmentManager fm, int titeId, int messageId) {
        recoverAndDismissCheckingDialog();
        AlertDialog.Builder bld = new AlertDialog.Builder(mContext);
        bld.setTitle(titeId);
        bld.setIconAttribute(android.R.attr.alertDialogIcon);
        bld.setMessage(messageId);
        bld.setPositiveButton(android.R.string.ok,
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog,
                    int which) {
                fm.popBackStack();
            }
        });
        bld.setOnCancelListener(new OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                fm.popBackStack();
            }
        });
        mAlertDialog = bld.create();
        mAlertDialog.setCanceledOnTouchOutside(false);
        mAlertDialog.show();
    }

    /**
     * Recover and dismiss the progress dialog fragment
     */
    private void recoverAndDismissCheckingDialog() {
        mCheckingDialog = (WaitingSaveOofDialog)
                    getFragmentManager().findFragmentByTag(WaitingSaveOofDialog.TAG);
        if (mCheckingDialog != null) {
            mCheckingDialog.dismissAllowingStateLoss();
            mCheckingDialog = null;
        }
    }

    /**
     * This is called when the user clicks "cancel" on the progress dialog.  Shuts everything
     * down and dismisses everything.
     * This should cause us to remain in the current screen (not accepting the settings)
     */
    private void onCheckingDialogCancel() {
        // 1. kill the checker
        Utility.cancelTaskInterrupt(mOofCheckTask);
        mOofCheckTask = null;
        // 2. kill self with no report - this is "cancel"
        Log.d(TAG, "onCheckingDialogCancel");

        finished();
    }

    /** Kill self if not already killed. */
    private void finished() {
        FragmentManager fm = getFragmentManager();
        if (fm == null && getTargetFragment() != null) {
            fm = getTargetFragment().getFragmentManager();
        }
        if (fm != null) {
            LogUtils.d(TAG, "Kill current fragment if not already killed (finish)");
            fm.popBackStack(this.getId(), FragmentManager.POP_BACK_STACK_INCLUSIVE);
        }
    }

    /**
     * This AsyncTask does the actual account checking
     *
     * TODO: It would be better to remove the UI complete from here (the exception->string
     * conversions).
     */
    private void startOofActivity() {
        if (mAccount == null) {
            return;
        }
        mState = STATE_START;
        Intent intent = OofSettingsActivity.createIntent(mContext,
                mAccount.mId, mOofParams);
        startActivity(intent);
    }
    /**
     * M: Task for update out of office settings.
     *
     */
    private class OofTask extends EmailAsyncTask<Void, Void, Integer> {

        public OofTask() {
            super(null);
        }

        @Override
        protected Integer doInBackground(Void... params) {
            reportProgress(STATE_START);
            final EmailServiceProxy proxy = EmailServiceUtils
                    .getServiceForAccount(mContext, mAccount.mId);
            try {
                mOofParams = proxy.syncOof(mAccount.mId, null, true);
            } catch (RemoteException e) {
                Log.d(TAG, "syncOof is Error.");
            }
            if (mOofParams == null || mOofParams.getmStatus() > ServerCommandInfo.OofInfo.SET_OR_SAVE_SUCCESS) {
                Log.d(TAG, "get oof status faild. mOofParams = " + mOofParams);
                return ServerCommandInfo.OofInfo.SYNC_OOF_UNINITIALIZED;
            }
            return mOofParams.getmStatus();
        }

        @Override
        protected void onSuccess(Integer result) {
            mState = result;
            if (mAttached) {
                reportProgress(result);
            }
        }
    }

    /**
     * M: DialogFragment for waiting update out of office settings.
     *
     */
    public static class WaitingSaveOofDialog extends DialogFragment {
        public static final String TAG = "WaitingSaveOofDialog";

        public static WaitingSaveOofDialog newInstance(OofGetWaitingFragment  parentFragment) {
            final WaitingSaveOofDialog dialog = new WaitingSaveOofDialog();
            dialog.setTargetFragment(parentFragment, 0);
            return dialog;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Context context = getActivity();
            final ProgressDialog progressDialog = new ProgressDialog(context);
            progressDialog.setIndeterminate(true);
            progressDialog.setCanceledOnTouchOutside(false);
            progressDialog.setMessage(context
                            .getString(R.string.account_settings_oof_get_status)
                            + "...");
            return progressDialog;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            OofGetWaitingFragment  parentFragment = (OofGetWaitingFragment) getTargetFragment();
            parentFragment.onCheckingDialogCancel();
            super.onCancel(dialog);
        }
    }
}
