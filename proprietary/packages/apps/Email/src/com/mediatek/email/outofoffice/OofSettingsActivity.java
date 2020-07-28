package com.mediatek.email.outofoffice;

import com.android.emailcommon.service.EmailServiceProxy;
import com.android.emailcommon.service.OofParams;
import com.android.email.R;
import com.android.email.service.EmailServiceUtils;
import com.mediatek.email.outofoffice.AccountSettingsOutOfOfficeFragment.Callback;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Parcelable;
import android.os.RemoteException;
import android.util.Log;
import android.view.MenuItem;

public class OofSettingsActivity extends Activity implements Callback {
    public static final String FRAGMENT_TAG = "AccountSettingsOutOfOfficeFragment";
    public static final String OOF_SETTING_ACTION = "mediatek.intent.action.OOF_SETTING_ACTION";
    private static final String ACCOUNT_ID = "account_id";
    private static final String OOF_PARAMS = "oof_params";

    private long mAccountId;
    private ActionBar mActionBar;
    public boolean mOkPressed;

    @Override
    public void onSettingFinished() {
        onBackPressed();
    }

    public static Intent createIntent(Context context, long accountId, Parcelable oofParams) {
        Intent i = new Intent();
        i.setAction(OOF_SETTING_ACTION);
        i.putExtra(ACCOUNT_ID, accountId);
        i.putExtra(OOF_PARAMS, oofParams);
        return i;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.oof_settings_activity);

        Intent i = getIntent();
        mAccountId = i.getLongExtra(ACCOUNT_ID, -1);
        OofParams oofParams = i.getParcelableExtra(OOF_PARAMS);
        if (savedInstanceState == null && mAccountId != -1 && oofParams != null) {
            // First-time init; create fragment to embed in activity.
            FragmentTransaction ft = getFragmentManager().beginTransaction();
            Fragment newFragment = AccountSettingsOutOfOfficeFragment.newInstance(mAccountId, oofParams);
            ft.add(R.id.fragment_holder, newFragment, FRAGMENT_TAG);
            ft.commit();
        }
        mActionBar = getActionBar();
        // Configure action bar.
        mActionBar.setDisplayOptions(
                ActionBar.DISPLAY_HOME_AS_UP, ActionBar.DISPLAY_HOME_AS_UP);
        mActionBar.setDisplayShowTitleEnabled(true);
        mActionBar.setTitle(R.string.account_settings_oof_label);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case android.R.id.home:
            // The app icon on the action bar is pressed.  Just emulate a back press.
            // TODO: this should navigate to the main screen, even if a sub-setting is open.
            // But we shouldn't just finish(), as we want to show "discard changes?" dialog
            // when necessary.
            onBackPressed();
            break;
        default:
            return super.onOptionsItemSelected(item);
        }
        return true;
    }

    @Override
    public void onBackPressed() {
        final EmailServiceProxy proxy = EmailServiceUtils
                    .getServiceForAccount(this, mAccountId);
        try {
            proxy.stopOof(mAccountId);
        } catch (RemoteException e) {
            Log.d("OofSettingsActivity", "stopOof Error");
        }
        //Button btn = (Button)AccountSettingsOutOfOfficeFragment.sSavebutton;
        AccountSettingsOutOfOfficeFragment fragment = (AccountSettingsOutOfOfficeFragment)
                getFragmentManager().findFragmentByTag(FRAGMENT_TAG);
        boolean isEnabled = fragment.getSaveButtonStatus();

        if (isEnabled && !mOkPressed) {
            FragmentManager fm = getFragmentManager();
            AlertDialogFragment adf = (AlertDialogFragment) fm
                    .findFragmentByTag(AccountSettingsOutOfOfficeFragment.CANCEL_ALERT_TAG);
            if (adf == null) {
                adf = AlertDialogFragment.newInstance(
                        R.string.account_settings_oof_cancel,
                        R.string.account_settings_oof_cancel_summary);
                adf.show(getFragmentManager(),
                        AccountSettingsOutOfOfficeFragment.CANCEL_ALERT_TAG);
            }
            return;
        }
        super.onBackPressed();
    }

    public static class AlertDialogFragment extends DialogFragment {
        private static final String TITLE_ID = "titleId";
        private static final String MSG_ID = "messageId";

        private int mTitleId;
        private int mMessageId;

        public AlertDialogFragment() {
        }

        public AlertDialogFragment(int titleId, int messageId) {
            super();
            this.mTitleId = titleId;
            this.mMessageId = messageId;
        }

        public static AlertDialogFragment newInstance(int titleId, int messageId) {
            final AlertDialogFragment dialog = new AlertDialogFragment(titleId, messageId);
            return dialog;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Context context = getActivity();
            final AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(context);
            if (savedInstanceState != null) {
                mTitleId = savedInstanceState.getInt(TITLE_ID);
                mMessageId = savedInstanceState.getInt(MSG_ID);
            }
            dialogBuilder.setTitle(mTitleId)
            .setIconAttribute(android.R.attr.alertDialogIcon)
            .setMessage(mMessageId)
            .setPositiveButton(android.R.string.ok,
                    new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    OofSettingsActivity activity = (OofSettingsActivity) getActivity();
                    activity.mOkPressed = true;
                    activity.onBackPressed();
                }
            })
            .setNegativeButton(android.R.string.cancel,
                    new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                }
            });
            AlertDialog alertDialog = dialogBuilder.create();
            alertDialog.setCanceledOnTouchOutside(false);
            return alertDialog;
        }

        @Override
        public void onSaveInstanceState(Bundle outState) {
            outState.putInt(TITLE_ID, mTitleId);
            outState.putInt(MSG_ID, mMessageId);
            super.onSaveInstanceState(outState);
        }
    }
}
