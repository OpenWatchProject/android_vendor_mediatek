/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.contacts.activities;

//import android.accounts.Account;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.LoaderManager.LoaderCallbacks;
import android.content.AsyncTaskLoader;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;

import android.content.Loader;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Environment;
import android.os.StatFs;
import android.os.UserHandle;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.contacts.R;
import com.android.contacts.activities.RequestImportVCardPermissionsActivity;
import com.android.contacts.list.UiIntentActions;
import com.android.contacts.model.AccountTypeManager;
import com.android.contacts.model.account.AccountInfo;
import com.android.contacts.model.account.AccountsLoader;
import com.android.contacts.model.account.AccountType;
import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.model.account.FallbackAccountType;
import com.android.contacts.util.AccountFilterUtil;
import com.android.contacts.util.AccountSelectionUtil;
import com.android.contacts.vcard.VCardCommonArguments;

import com.google.common.base.Function;
import com.google.common.collect.Lists;

import com.mediatek.contacts.ContactsSystemProperties;
import com.mediatek.contacts.eventhandler.BaseEventHandlerActivity;
import com.mediatek.contacts.list.ContactsIntentResolverEx;
import com.mediatek.contacts.list.service.MultiChoiceService;
import com.mediatek.contacts.model.account.AccountWithDataSetEx;
import com.mediatek.contacts.simcontact.SimCardUtils;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.ContactsPortableUtils;
import com.mediatek.contacts.util.ContactsIntent;
import com.mediatek.contacts.util.Log;
import com.mediatek.contacts.widget.ImportExportItem;
import com.mediatek.storage.StorageManagerEx;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class ContactImportExportActivity extends BaseEventHandlerActivity
        implements View.OnClickListener, AdapterView.OnItemClickListener,
        AccountsLoader.AccountsListener{
    private static final String TAG = "ContactImportExportActivity";

    public static final int REQUEST_CODE = 11111;
    public static final int RESULT_CODE = 11112;

    /*
     * To unify the storages(includes internal storage and external storage)
     * handling, we looks all of storages as one kind of account type.
     */
    public static final String STORAGE_ACCOUNT_TYPE = "_STORAGE_ACCOUNT";

    private static final int ACCOUNT_LOADER_ID = 0;

    private static final int SELECTION_VIEW_STEP_NONE = 0;
    private static final int SELECTION_VIEW_STEP_ONE = 1;
    private static final int SELECTION_VIEW_STEP_TWO = 2;

    private ListView mListView = null;
    private List<AccountWithDataSetEx> mAccounts = null;

    private int mShowingStep = SELECTION_VIEW_STEP_NONE;
    private int mCheckedPosition = 0;
    private boolean mIsFirstEntry = true;
    private AccountWithDataSetEx mCheckedAccount1 = null;
    private AccountWithDataSetEx mCheckedAccount2 = null;
    private List<ListViewItemObject> mListItemObjectList = new ArrayList<ListViewItemObject>();
    private AccountListAdapter mAdapter = null;
    // add mCallingActivity for who start ContactImportExportActivity
    private String mCallingActivityName = null;

    private String mType = null;

    private boolean isImport() {
        return ("Import".equals(mType)) ? true : false;
    }

    private boolean isExport() {
        return ("Export".equals(mType)) ? true : false;
    }

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        Log.i(TAG, "[onCreate]");

        /// M: [ALPS04156027] check basic permission like ImportVCardActivity
        if (RequestImportVCardPermissionsActivity.startPermissionActivity(this,
                isCallerSelf(this))) {
            Log.i(TAG,"[onCreate]startPermissionActivity,return.");
            return;
        }
        /// @}

        /* add for Dailer using ContactImportExport function.
         * need to check MultiChoiceService is or not Processing delete contacts
         * before Activity create when Dailer start ContactImportExportActivity.@{
         */
        if (MultiChoiceService.isProcessing(MultiChoiceService.TYPE_DELETE)) {
            Log.i(TAG, "[onCreate] MultiChoiceService isProcessing delete contacts" +
                    ",stop Create and return");
            setResult(RESULT_CANCELED);
            finish();
            return;
        }
        // reserve the activity who start this activity
        Bundle extras = getIntent().getExtras();
        if (extras == null) {
            Log.e(TAG, "[onCreate] callingActivity has no putExtra");
            finish();
            return;
        } else {
            mCallingActivityName = extras.getString(
                    VCardCommonArguments.ARG_CALLING_ACTIVITY, null);
            if (mCallingActivityName == null) {
                Log.e(TAG, "[onCreate] callingActivity = null and return");
                finish();
                return;
            }
            mType = extras.getString(VCardCommonArguments.ARG_CALLING_TYPE);
        }
        Log.d(TAG, "[onCreate]mCallingActivityName=" + mCallingActivityName + ", mType=" + mType);

        setContentView(R.layout.mtk_import_export_bridge_layout);

        ((Button) findViewById(R.id.btn_action)).setOnClickListener(this);
        ((Button) findViewById(R.id.btn_back)).setOnClickListener(this);

        ((LinearLayout) findViewById(R.id.buttonbar_layout)).setVisibility(View.GONE);

        mListView = (ListView) findViewById(R.id.list_view);
        mListView.setOnItemClickListener(this);

        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            actionBar.setDisplayOptions(
                    ActionBar.DISPLAY_HOME_AS_UP | ActionBar.DISPLAY_SHOW_TITLE,
                    ActionBar.DISPLAY_HOME_AS_UP | ActionBar.DISPLAY_SHOW_TITLE
                            | ActionBar.DISPLAY_SHOW_HOME);
            if (isImport()) {
                actionBar.setTitle(R.string.import_title);
            } else if (isExport()) {
                actionBar.setTitle(R.string.export_title);
            } else {
                actionBar.setTitle(R.string.imexport_title);
            }
        }

        mAdapter = new AccountListAdapter(ContactImportExportActivity.this);
        AccountsLoader.loadAccounts(this, 0, AccountTypeManager.writableFilter());

    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        setCheckedPosition(position);
        setCheckedAccount(position);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case android.R.id.home:
            finish();
            return true;
        default:
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onBackPressed() {
        if (mShowingStep > SELECTION_VIEW_STEP_ONE) {
            onBackAction();
        } else {
            super.onBackPressed();
        }
    }

    public void onClick(View view) {
        switch (view.getId()) {
        case R.id.btn_action:
        case R.id.btn_back:
            if (view.getId() == R.id.btn_action) {
                onNextAction();
            } else {
                onBackAction();
            }
            break;
        default:
            break;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.i(TAG, "[onActivityResult]requestCode:" + requestCode + ",resultCode:"
                + resultCode);
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == ContactImportExportActivity.REQUEST_CODE) {
            if (resultCode == ContactImportExportActivity.RESULT_CODE) {
                this.finish();
            }
        }
    }

    @Override
    protected void onDestroy() {
        mIsFinished = true;
        super.onDestroy();
        Log.i(TAG, "[onDestroy]");
    }

    @Override
    public void onAccountsLoaded(List<AccountInfo> data) {

        final List<AccountWithDataSetEx> accounts = extractAccountsEx(data);
        // /check whether the Activity's status still ok
        if (isActivityFinished()) {
            Log.w(TAG, "[onLoadFinished]isActivityFinished is true,return.");
            return;
        }
        if (accounts == null) { // Just in case...
            Log.e(TAG, "[onLoadFinished]data is null,return.");
            return;
        }
        Log.d(TAG, "[onLoadFinished]data = " + accounts);
        if (mAccounts == null) {
            mAccounts = accounts;
            // Add all of storages accounts
            mAccounts.addAll(getStorageAccounts());
            // If the accounts size is less than one item, we should not
            // show this view for user to import or export operations.
            if (mAccounts.size() <= 1) {
                Log.i(TAG, "[onLoadFinished]mAccounts.size = " + mAccounts.size());
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(getApplicationContext(),
                                R.string.xport_error_one_account, Toast.LENGTH_SHORT).show();
                    }
                });
                finish();
            }
            Log.i(TAG, "[onLoadFinished]mAccounts.size() = " + mAccounts.size() +",mAccounts:"
                    + mAccounts + ",mShowingStep =" + mShowingStep);
            if (mShowingStep == SELECTION_VIEW_STEP_NONE) {
                setShowingStep(SELECTION_VIEW_STEP_ONE);
            } else {
                setShowingStep(mShowingStep);
            }
            setCheckedAccount(mCheckedPosition);
            updateUi();
        }
    }

    private List<AccountWithDataSetEx> extractAccountsEx(List<AccountInfo> data) {

        List<AccountWithDataSet> accounts = AccountInfo.extractAccounts(data);
        List<AccountWithDataSetEx> accountsEx = new ArrayList<AccountWithDataSetEx>();

        for (AccountWithDataSet account : accounts) {
            AccountType accountType = getAccountType(account.type, account.dataSet, false);
            Log.d(TAG, "[loadAccountFilters]account.type = " + account.type
                    + ",account.name =" + Log.anonymize(account.name));
            if (accountType.isExtension() && !account.hasData(this)) {
                Log.d(TAG, "[loadAccountFilters]continue.");
                // Hide extensions with no raw_contacts.
                continue;
            }
            int subId = SubInfoUtils.getInvalidSubId();
            if (account instanceof AccountWithDataSetEx) {
                subId = ((AccountWithDataSetEx) account).getSubId();
            }
            Log.d(TAG, "[loadAccountFilters]subId = " + subId);
            accountsEx.add(new AccountWithDataSetEx(account.name, account.type, subId));
        }

        return accountsEx;

    }


    public void doImportExport() {
        Log.i(TAG, "[doImportExport]...");

        if (AccountTypeUtils.isAccountTypeIccCard(mCheckedAccount1.type)) {
            // UIM
            int subId = ((AccountWithDataSetEx) mCheckedAccount1).getSubId();
            /** change for PHB Status Refactoring @{ */
            if (!SimCardUtils.isPhoneBookReady(subId)) {
                Toast.makeText(this, R.string.icc_phone_book_invalid, Toast.LENGTH_LONG).show();
                finish();
                Log.i(TAG, "[doImportExport] phb is not ready.");
            } else {
                handleImportExportAction();
            }
            /** @} */
        } else {
            handleImportExportAction();
        }
    }

    public File getDirectory(String path, String defaultPath) {
        Log.d(TAG, "[getDirectory]path : " + Log.anonymize(path)
                +  ",defaultPath:" + Log.anonymize(defaultPath));
        return path == null ? new File(defaultPath) : new File(path);
    }

    public List<AccountWithDataSetEx> getStorageAccounts() {
        List<AccountWithDataSetEx> storageAccounts = new ArrayList<AccountWithDataSetEx>();
        StorageManager storageManager = (StorageManager) getApplicationContext().getSystemService(
                STORAGE_SERVICE);
        if (null == storageManager) {
            Log.w(TAG, "[getStorageAccounts]storageManager is null!");
            return storageAccounts;
        }

        if (ContactsPortableUtils.MTK_STORAGE_SUPPORT) {
            try {
                String defaultStoragePath = StorageManagerEx.getDefaultPath();
                if (!storageManager.getVolumeState(defaultStoragePath).equals(
                        Environment.MEDIA_MOUNTED)) {
                    Log.w(TAG, "[getStorageAccounts]State is  not MEDIA_MOUNTED!");
                    return storageAccounts;
                }
            } catch (Exception e) {
                Log.e(TAG, "StorageManagerEx.getDefaultPath native exception!");
                e.printStackTrace();
            }
        }

        // change for ALPS02390380, different user can use different storage, so change the API
        // to user related API.
        StorageVolume volumes[] = StorageManager.getVolumeList(UserHandle.myUserId(),
                StorageManager.FLAG_FOR_WRITE);
        if (volumes != null) {
            Log.d(TAG, "[getStorageAccounts]volumes are: " + volumes);
            for (StorageVolume volume : volumes) {
                String path = volume.getPath();
                ///[ALPS03465894]Add check for Sdcard inject.
                String state = Environment.getExternalStorageState(volume.getPathFile());
                Log.d(TAG, "[getStorageAccounts]path:" + Log.anonymize(path) + ", state=" + state);
                if (!state.equals(Environment.MEDIA_MOUNTED)) {
                    continue;
                }
                storageAccounts.add(new AccountWithDataSetEx(volume.getDescription(this),
                        STORAGE_ACCOUNT_TYPE, path));
            }
        }
        return storageAccounts;
    }

    // //////////////////////////private
    // funcation///////////////////////////////////////
    private class ListViewItemObject {
        public AccountWithDataSetEx mAccount;
        public ImportExportItem mView;

        public ListViewItemObject(AccountWithDataSetEx account) {
            mAccount = account;
        }

        public String getName() {
            if (mAccount == null) {
                Log.w(TAG, "[getName]mAccount is null!");
                return "null";
            } else {
                String displayName = null;
                displayName = AccountFilterUtil.getAccountDisplayNameByAccount(mAccount.type,
                        mAccount.name);
                Log.d(TAG, "[getName]type : " + mAccount.type
                        + ",name:" + Log.anonymize(mAccount.name)
                        + ",displayName:" + Log.anonymize(displayName));
                if (TextUtils.isEmpty(displayName)) {
                    if (AccountWithDataSetEx.isLocalPhone(mAccount.type)) {
                        return getString(R.string.account_phone_only);
                    }
                    return mAccount.name;
                } else {
                    return displayName;
                }
            }
        }
    }

    private class AccountListAdapter extends BaseAdapter {
        private final LayoutInflater mLayoutInflater;
        private Context mContext;

        public AccountListAdapter(Context context) {
            mContext = context;
            mLayoutInflater = (LayoutInflater) context
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return mListItemObjectList.size();
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public AccountWithDataSetEx getItem(int position) {
            return null;
        }

        public View getView(int position, View convertView, ViewGroup parent) {
            final ImportExportItem view;

            if (convertView != null) {
                view = (ImportExportItem) convertView;
            } else {
                view = (ImportExportItem) mLayoutInflater.inflate(
                        R.layout.mtk_contact_import_export_item, parent, false);
            }

            ListViewItemObject itemObj = mListItemObjectList.get(position);
            itemObj.mView = view;
            final AccountWithDataSet account = (AccountWithDataSet) itemObj.mAccount;
            final AccountType accountType = getAccountType(account.type,
                    account.dataSet, true);
            Drawable icon = null;
            String type = itemObj.getName();
            final int subId = itemObj.mAccount.getSubId();
            Log.d(TAG, "[getView] accounttype: " + accountType);
            if (accountType != null && accountType.isIccCardAccount()) {
                icon = accountType.getDisplayIconBySubId(mContext, subId);
                type = (String) accountType.getDisplayLabel(mContext);
            } else if (accountType != null) {
                icon = accountType.getDisplayIcon(mContext);
            }
            view.bindView(icon, type, account.dataSet);
            view.setActivated(mCheckedPosition == position);
            return view;
        }
    }

    private void onBackAction() {
        int pos = 0;
        setShowingStep(SELECTION_VIEW_STEP_ONE);
        pos = getCheckedAccountPosition(mCheckedAccount1);
        Log.d(TAG, "[onBackAction] mCheckedAccount1 =" + mCheckedAccount1 +",pos = " + pos);
        mCheckedPosition = pos;
        setCheckedAccount(mCheckedPosition);
        updateUi();
    }

    private void onNextAction() {
        Log.d(TAG, "[onNextAction] mShowingStep = " + mShowingStep);
        int pos = 0;
        if (mShowingStep >= SELECTION_VIEW_STEP_TWO) {
            doImportExport();
            return;
        }
        setShowingStep(SELECTION_VIEW_STEP_TWO);
        if (mIsFirstEntry || (mCheckedAccount1 == null && mCheckedAccount2 == null)) {
            pos = 0;
        } else {
            pos = getCheckedAccountPosition(mCheckedAccount2);
        }
        mIsFirstEntry = false;
        mCheckedPosition = pos;
        setCheckedAccount(mCheckedPosition);
        updateUi();
    }

    private void updateUi() {
        setButtonState(true);
        mListView.setAdapter(mAdapter);
    }

    private int getCheckedAccountPosition(AccountWithDataSetEx checkedAccount) {
        for (int i = 0; i < mListItemObjectList.size(); i++) {
            ListViewItemObject obj = mListItemObjectList.get(i);
            if (obj.mAccount.equals(checkedAccount)) {
                return i;
            }
        }
        return 0;
    }

    private void handleImportExportAction() {
        Log.d(TAG, "[handleImportExportAction]...");
        if (isStorageAccount(mCheckedAccount1) && !checkSDCardAvaliable(mCheckedAccount1.dataSet)
                || isStorageAccount(mCheckedAccount2)
                && !checkSDCardAvaliable(mCheckedAccount2.dataSet)) {
            new AlertDialog.Builder(this).setMessage(R.string.no_sdcard_message)
                    .setTitle(R.string.no_sdcard_title)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    }).show();
            return;
        }

        if (isStorageAccount(mCheckedAccount1)) { // import from SDCard
            if (mCheckedAccount2 != null) {
                AccountSelectionUtil.doImportFromSdCard(this, mCheckedAccount1.dataSet,
                        mCheckedAccount2);
            }
        } else {
            if (isStorageAccount(mCheckedAccount2)) { // export to SDCard
                if (isSDCardFull(mCheckedAccount2.dataSet)) { // SD card is full
                    Log.i(TAG, "[handleImportExportAction] isSDCardFull");
                    new AlertDialog.Builder(this)
                            .setMessage(R.string.storage_full)
                            .setTitle(R.string.storage_full)
                            .setPositiveButton(android.R.string.ok,
                                    new DialogInterface.OnClickListener() {
                                        public void onClick(DialogInterface dialog, int which) {
                                            finish();
                                        }
                                    }).show();
                    return;
                }

                Intent intent = new Intent(this,
                        com.mediatek.contacts.list.ContactListMultiChoiceActivity.class)
                        .setAction(ContactsIntent.LIST.ACTION_PICK_MULTI_CONTACTS)
                        .putExtra("request_type",
                                ContactsIntentResolverEx.REQ_TYPE_IMPORT_EXPORT_PICKER)
                        .putExtra("fromaccount", mCheckedAccount1)
                        .putExtra("toaccount", mCheckedAccount2)
                        .putExtra(VCardCommonArguments.ARG_CALLING_ACTIVITY, mCallingActivityName);
                startActivityForResult(intent, ContactImportExportActivity.REQUEST_CODE);
            } else { // account to account
                Intent intent = new Intent(this,
                        com.mediatek.contacts.list.ContactListMultiChoiceActivity.class)
                        .setAction(ContactsIntent.LIST.ACTION_PICK_MULTI_CONTACTS)
                        .putExtra("request_type",
                                ContactsIntentResolverEx.REQ_TYPE_IMPORT_EXPORT_PICKER)
                        .putExtra("fromaccount", mCheckedAccount1)
                        .putExtra("toaccount", mCheckedAccount2)
                        .putExtra(VCardCommonArguments.ARG_CALLING_ACTIVITY, mCallingActivityName);
                startActivityForResult(intent, ContactImportExportActivity.REQUEST_CODE);
            }
        }
    }

    private boolean checkSDCardAvaliable(final String path) {
        if (TextUtils.isEmpty(path)) {
            Log.w(TAG, "[checkSDCardAvaliable]path is null!");
            return false;
        }
        StorageManager storageManager = (StorageManager)getSystemService(Context.STORAGE_SERVICE);
        if (null == storageManager) {
            Log.i(TAG, "[checkSDCardAvaliable] story manager is null");
             return false;
        }
        String storageState = storageManager.getVolumeState(path);
        Log.d(TAG, "[checkSDCardAvaliable]path = " + Log.anonymize(path)
                + ",storageState = " + storageState);
        return storageState.equals(Environment.MEDIA_MOUNTED);
    }

    private boolean isSDCardFull(final String path) {
        if (TextUtils.isEmpty(path)) {
            Log.w(TAG, "[isSDCardFull]path is null!");
            return false;
        }
        Log.d(TAG, "[isSDCardFull] storage path is " + Log.anonymize(path));
        if (checkSDCardAvaliable(path)) {
            StatFs sf = null;
            try {
                sf = new StatFs(path);
            } catch (IllegalArgumentException e) {
                Log.e(TAG, "[isSDCardFull]catch exception:");
                e.printStackTrace();
                return false;
            }
            long availCount = sf.getAvailableBlocks();
            return !(availCount > 0);
        }

        return true;
    }

    private void setButtonState(boolean isTrue) {
        findViewById(R.id.btn_back).setVisibility(
                (isTrue && (mShowingStep > SELECTION_VIEW_STEP_ONE)) ? View.VISIBLE : View.GONE);

        findViewById(R.id.btn_action).setEnabled(
                isTrue && (mShowingStep > SELECTION_VIEW_STEP_NONE));
    }

    /**
     * showing the step of import/export step according to the parame
     * shaoingStep
     *
     * @param showingStep
     */
    private void setShowingStep(int showingStep) {
        mShowingStep = showingStep;
        mListItemObjectList.clear();

        ((LinearLayout) findViewById(R.id.buttonbar_layout)).setVisibility(View.VISIBLE);
        Log.d(TAG, "[setShowingStep]mShowingStep = " + mShowingStep);
        if (mShowingStep == SELECTION_VIEW_STEP_ONE) {
            int nonStorageCount = 0;
            ((TextView) findViewById(R.id.tips)).setText(R.string.tips_source);
            for (AccountWithDataSetEx account : mAccounts) {
                if (isImport()) {
                    if (!isStorageAccount(account)) nonStorageCount++;
                } else if (isExport()) {
                    AccountType accountType =
                        getAccountType(account.type, account.dataSet, true);
                    if (isStorageAccount(account) || accountType.isIccCardAccount()) {
                        continue;
                    }
                }
                mListItemObjectList.add(new ListViewItemObject(account));
            }
            /// If there is only one non-storage account, it should be hidden.
            /// because there is no target account for import if non-storage
            /// account is selected in step one.
            if (isImport() && (1 == nonStorageCount)) {
                for (ListViewItemObject obj : mListItemObjectList) {
                    if (!isStorageAccount(obj.mAccount)) {
                        mListItemObjectList.remove(obj);
                        break;
                    }
                }
            }
        } else if (mShowingStep == SELECTION_VIEW_STEP_TWO) {
            ((TextView) findViewById(R.id.tips)).setText(R.string.tips_target);
            for (AccountWithDataSetEx account : mAccounts) {
                if (!mCheckedAccount1.equals(account)) {
                    /*
                     * It is not allowed for the importing from Storage -> SIM
                     * or USIM and from SIM or USIM -> Storage and also is not
                     * for importing from Storage -> Storage
                     */
                    AccountType accountType = getAccountType(account.type, account.dataSet, true);
                    AccountType checkedAccountType = getAccountType(
                            mCheckedAccount1.type, mCheckedAccount1.dataSet, true);
                    Log.d(TAG, "[setShowingStep]accountType: " + accountType +
                            ", checkedAccountType: " + checkedAccountType);
                    if ((isStorageAccount(mCheckedAccount1) && accountType.isIccCardAccount()) ||
                        (checkedAccountType.isIccCardAccount() && isStorageAccount(account)) ||
                        (isStorageAccount(mCheckedAccount1) && isStorageAccount(account))) {
                        continue;
                    }
                    if (isImport()) {
                        if (isStorageAccount(account)) {
                            continue;
                        }
                    } else if (isExport()) {
                        if (!isStorageAccount(account)) {
                            continue;
                        }
                    }
                    mListItemObjectList.add(new ListViewItemObject(account));
                }
            }
        }
    }

    private AccountType getAccountType(String type, String dataSet, boolean supportStorage) {
        AccountType accountType = AccountTypeManager.
                getInstance(this).getAccountType(type, dataSet);
        if (null == accountType && supportStorage) {
            if (STORAGE_ACCOUNT_TYPE.equalsIgnoreCase(type)) {
                accountType = new FallbackAccountType(this);
            }
        }
        return accountType;
    }

    private static boolean isStorageAccount(final AccountWithDataSetEx account) {
        if (account != null) {
            return STORAGE_ACCOUNT_TYPE.equalsIgnoreCase(account.type);
        }
        return false;
    }

    private void setCheckedPosition(int checkedPosition) {
        if (mCheckedPosition != checkedPosition) {
            setListViewItemChecked(mCheckedPosition, false);
            mCheckedPosition = checkedPosition;
            setListViewItemChecked(mCheckedPosition, true);
        }
    }

    private void setCheckedAccount(int position) {
        if (mListItemObjectList.size() == 0) {
            Log.e(TAG, "[setCheckedAccount]mListItemObjectList.size() == 0" );
            finish();
            return;
        }
        if (mShowingStep == SELECTION_VIEW_STEP_ONE) {
            mCheckedAccount1 = mListItemObjectList.get(position).mAccount;
        } else if (mShowingStep == SELECTION_VIEW_STEP_TWO) {
            mCheckedAccount2 = mListItemObjectList.get(position).mAccount;
        }
        Log.d(TAG, "[setCheckedAccount]mCheckedAccount1 = " + mCheckedAccount1
                + ",mCheckedAccount2 =" + mCheckedAccount2 + ",pos = " + position);
    }

    private void setListViewItemChecked(int checkedPosition, boolean checked) {
        if (checkedPosition > -1) {
            ListViewItemObject itemObj = mListItemObjectList.get(checkedPosition);
            if (itemObj.mView != null) {
                itemObj.mView.setActivated(checked);
            }
        }
    }

    private boolean isActivityFinished() {
        return mIsFinished;
    }

    private boolean mIsFinished = false;

    /**
     * M: [ALPS04156027] copy from ImportVCardActivity used in check permission @{
     */
    private static boolean isCallerSelf(Activity activity) {
        // {@link Activity#getCallingActivity()} is a safer alternative to
        // {@link Activity#getCallingPackage()} that works around a
        // framework bug where getCallingPackage() can sometimes return null even when the
        // current activity *was* in fact launched via a startActivityForResult() call.
        //
        // (The bug happens if the task stack needs to be re-created by the framework after
        // having been killed due to memory pressure or by the "Don't keep activities"
        // developer option; see bug 7494866 for the full details.)
        //
        // Turns out that {@link Activity#getCallingActivity()} *does* return correct info
        // even in the case where getCallingPackage() is broken, so the workaround is simply
        // to get the package name from getCallingActivity().getPackageName() instead.
        final ComponentName callingActivity = activity.getCallingActivity();
        if (callingActivity == null) return false;
        final String packageName = callingActivity.getPackageName();
        if (packageName == null) return false;
        return packageName.equals(activity.getApplicationContext().getPackageName());
    }
    /// @}
}
