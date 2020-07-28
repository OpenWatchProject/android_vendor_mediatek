package com.mediatek.mail.vip.activity;

import java.util.ArrayList;

import com.android.mail.R;
import com.android.emailcommon.mail.Address;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.android.emailcommon.utility.Utility;
import com.mediatek.mail.ui.utils.UiUtilities;
import com.mediatek.mail.vip.VipMember;

import android.app.ActionBar;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.Intent;
import android.database.ContentObserver;
import android.database.Cursor;
import android.os.Bundle;
import android.os.Handler;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.widget.TextView;

/**
 * M : Used as VipFragment container, and user can launch contact's activity to add
 * new vip member.
 */
public class VipListActivity extends Activity implements VipListFragment.Callback {
    public static final String TAG = "VIP_Settings/VipListActivity";
    public static final String ACCOUNT_ID = "accountId";
    public static final String VIP_LIST_ACITON = "mediatek.intent.action.EMAIL_VIP_ACTIVITY";
    public static final String PICK_MULTI_EMAILS_ACTION =
            "mediatek.intent.action.contacts.list.PICKMULTIEMAILS";
    public static final String EXTRA_PICK_DATA_RESULT =
            "com.mediatek.contacts.list.pickdataresult";
    public static final int REQUEST_CODE_PICK = 1;

    private ActionBar mActionBar;
    private ViewGroup mActionBarCustomView;
    private TextView mVipMembers;
    private long mAccountId;
    /** M: use for observer accounts deleted or not @{ */
    private ContentObserver mAccountObserver;
    /** @} */

    public static Intent createIntent(Context context, long accountId) {
        Intent i = new Intent();
        i.setAction(VIP_LIST_ACITON);
        i.putExtra(ACCOUNT_ID, accountId);
        return i;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.email_vip_activity);

        Intent i = getIntent();
        mAccountId = i.getLongExtra(ACCOUNT_ID, -1);
        if (savedInstanceState == null && mAccountId != -1) {
            // First-time init; create fragment to embed in activity.
            FragmentTransaction ft = getFragmentManager().beginTransaction();
            Fragment newFragment = VipListFragment.newInstance(mAccountId);
            ft.add(R.id.fragment_placeholder, newFragment);
            ft.commit();
        }
        mActionBar = getActionBar();
        // Configure action bar.
        mActionBar.setDisplayOptions(
                ActionBar.DISPLAY_HOME_AS_UP, ActionBar.DISPLAY_HOME_AS_UP);
        mActionBar.setDisplayShowCustomEnabled(true);
        mActionBar.setDisplayShowTitleEnabled(false);
        // Prepare the custom view
        mActionBar.setCustomView(R.layout.vip_actionbar_custom_view);
        mActionBarCustomView = (ViewGroup) mActionBar.getCustomView();
        mVipMembers = (TextView) mActionBarCustomView.findViewById(R.id.vip_member);
        /**
         * M: When all accounts has been deleted, the vip list activity will be
         * finished and login page will be displayed. @{
         */
        if (mAccountObserver == null) {
            mAccountObserver = new AccountContentObserver(null, this);
        }
        getContentResolver().registerContentObserver(Account.NOTIFIER_URI, true, mAccountObserver);
        /** @} */
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        getMenuInflater().inflate(R.menu.vip_add_contact_option, menu);
        return true;
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
            case R.id.choose_contact:
                onChooseVIPFromContact();
                break;
            default:
                return super.onOptionsItemSelected(item);
        }
        return true;
    }

    private void onChooseVIPFromContact() {
        Intent intent = new Intent(PICK_MULTI_EMAILS_ACTION);
        intent.setType(CommonDataKinds.Email.CONTENT_TYPE);
        UiUtilities.startRemoteActivityForResult(this, intent,
                REQUEST_CODE_PICK, true);
    }

    @Override
    protected void onDestroy() {
        if (mAccountObserver != null) {
            getContentResolver().unregisterContentObserver(mAccountObserver);
            mAccountObserver = null;
        }
        super.onDestroy();
    }

    @Override
    public void onVipMemberChanged(int vipNumber) {
        final String title = String.format(
                this.getString(R.string.vip_members_count), vipNumber);
        mVipMembers.setText(title);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == Activity.RESULT_OK && requestCode == REQUEST_CODE_PICK) {
            final long[] ids = data.getLongArrayExtra(EXTRA_PICK_DATA_RESULT);
            if (ids == null || ids.length <= 0) {
                return;
            }
            EmailAsyncTask.runAsyncParallel(new Runnable() {

                @Override
                public void run() {
                    VipListActivity.this.saveContactsAsVips(ids);
                }

            });
        }
    }

    private Address[] getEmailAddressesFromContacts(long[] contactIds) {
        ArrayList<Address> addresses = new ArrayList<Address>();
        if (contactIds == null || contactIds.length <= 0) {
            return addresses.toArray(new Address[] {});
        }
        StringBuilder selection = new StringBuilder();
        selection.append(CommonDataKinds.Email._ID);
        selection.append(" IN (");
        selection.append(contactIds[0]);
        for (int i = 1; i < contactIds.length; i++) {
            selection.append(",");
            selection.append(contactIds[i]);
        }
        selection.append(")");
        Cursor c = null;
        try {
            c = getContentResolver().query(CommonDataKinds.Email.CONTENT_URI,
                    new String[]{CommonDataKinds.Email.ADDRESS, ContactsContract.Data.DISPLAY_NAME},
                    selection.toString(), null, null);
            if (c == null) {
                return addresses.toArray(new Address[] {});
            }
            while (c.moveToNext()) {
                addresses.add(new Address(c.getString(0), c.getString(1)));
            }
            return addresses.toArray(new Address[] {});
        } finally {
            if (c != null) {
                c.close();
            }
        }
    }

    private void saveContactsAsVips(long[] contactIds) {
        if (contactIds == null || contactIds.length <= 0) {
            return;
        }
        /** M: Use for check address's validity before add vip @{ */
        Address[] addresses = getEmailAddressesFromContacts(contactIds);
        boolean hasInvalidAddress = false;
        ArrayList<Address> addressList = new ArrayList<Address>();
        for (Address address : addresses) {
            if (!Address.isAllValid(address.getAddress())) {
                hasInvalidAddress = true;
            } else {
                addressList.add(address);
            }
        }
        if (hasInvalidAddress) {
            Utility.showToast(VipListActivity.this,
                    R.string.message_compose_error_invalid_email);
        }
        /** @} */
        VipMember.addVIPs(this, mAccountId, addressList, new VipMember.AddVipsCallback() {
            @Override
            public void tryToAddDuplicateVip() {
                Utility.showToast(VipListActivity.this, R.string.not_add_duplicate_vip);
            }
            @Override
            public void addVipOverMax() {
                Utility.showToast(VipListActivity.this, R.string.can_not_add_vip_over_99);
            }
        });
    }

    /**
     * M: Observer invoked whenever account is deleted.
     */
    private class AccountContentObserver extends ContentObserver {
        private final Context mContext;

        public AccountContentObserver(Handler handler, Context context) {
            super(handler);
            mContext = context;
        }

        @Override
        public void onChange(boolean selfChange) {
            int count = Account.count(mContext, Account.CONTENT_URI);
            if (count == 0) {
                VipListActivity.this.finish();
            }
        }
    }
}
