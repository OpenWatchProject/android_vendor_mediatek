/*
 * Copyright (C) 2006 The Android Open Source Project
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

package com.android.phone.settings.fdn;

import static android.view.Window.PROGRESS_VISIBILITY_OFF;
import static android.view.Window.PROGRESS_VISIBILITY_ON;

import android.app.Activity;
import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.ContactsContract.CommonDataKinds;
import android.telephony.PhoneNumberUtils;
import android.text.Editable;
import android.text.Selection;
import android.text.Spannable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.method.DialerKeyListener;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.PhoneFactory;
import com.android.phone.PhoneGlobals;
import com.android.phone.R;
import com.android.phone.SubscriptionInfoHelper;
import com.mediatek.settings.CallSettingUtils;

/**
 * Activity to let the user add or edit an FDN contact.
 */
public class EditFdnContactScreen extends Activity
        implements PhoneGlobals.SubInfoUpdateListener {
    private static final String LOG_TAG = PhoneGlobals.LOG_TAG;
    private static final boolean DBG = true;

    // Menu item codes
    private static final int MENU_IMPORT = 1;
    private static final int MENU_DELETE = 2;

    private static final String INTENT_EXTRA_NAME = "name";
    private static final String INTENT_EXTRA_NUMBER = "number";

    private static final int PIN2_REQUEST_CODE = 100;

    private SubscriptionInfoHelper mSubscriptionInfoHelper;

    private String mName;
    private String mNumber;
    private String mPin2;
    private boolean mAddContact;
    private QueryHandler mQueryHandler;

    private EditText mNameField;
    private EditText mNumberField;
    private LinearLayout mPinFieldContainer;
    private Button mButton;

    private Handler mHandler = new Handler();

    /**
     * Constants used in importing from contacts
     */
    /** request code when invoking subactivity */
    private static final int CONTACTS_PICKER_CODE = 200;
    /** projection for phone number query */
    private static final String[] NUM_PROJECTION = new String[] {CommonDataKinds.Phone.DISPLAY_NAME,
            CommonDataKinds.Phone.NUMBER};
    /** static intent to invoke phone number picker */
    private static final Intent CONTACT_IMPORT_INTENT;
    static {
        CONTACT_IMPORT_INTENT = new Intent(Intent.ACTION_GET_CONTENT);
        CONTACT_IMPORT_INTENT.setType(CommonDataKinds.Phone.CONTENT_ITEM_TYPE);
    }
    /** flag to track saving state */
    private boolean mDataBusy;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        resolveIntent();

        getWindow().requestFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.edit_fdn_contact_screen);
        setupView();
        setTitle(mAddContact ? R.string.add_fdn_contact : R.string.edit_fdn_contact);

        displayProgress(false);

        //M: Add for host swap
        PhoneGlobals.getInstance().addSubInfoUpdateListener(this);
    }

    /**
     * We now want to bring up the pin request screen AFTER the
     * contact information is displayed, to help with user
     * experience.
     *
     * Also, process the results from the contact picker.
     */
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        if (DBG) log("onActivityResult request:" + requestCode + " result:" + resultCode);

        switch (requestCode) {
            case PIN2_REQUEST_CODE:
                Bundle extras = (intent != null) ? intent.getExtras() : null;
                if (extras != null) {
                    mPin2 = extras.getString("pin2");
                    if (mAddContact) {
                        addContact();
                    } else {
                        updateContact();
                    }
                } else if (resultCode != RESULT_OK) {
                    // if they cancelled, then we just cancel too.
                    if (DBG) log("onActivityResult: cancelled.");
                    finish();
                }
                break;

            // look for the data associated with this number, and update
            // the display with it.
            case CONTACTS_PICKER_CODE:
                if (resultCode != RESULT_OK) {
                    if (DBG) log("onActivityResult: cancelled.");
                    return;
                }
                Cursor cursor = null;
                try {
                    cursor = getContentResolver().query(intent.getData(),
                        NUM_PROJECTION, null, null, null);
                    if ((cursor == null) || (!cursor.moveToFirst())) {
                        Log.w(LOG_TAG, "onActivityResult: bad contact data, no results found.");
                        return;
                    }
                    mNameField.setText(cursor.getString(0));
                    mNumberField.setText(cursor.getString(1));
                } finally {
                    if (cursor != null) {
                        cursor.close();
                    }
                }
                break;
        }
    }

    /**
     * Overridden to display the import and delete commands.
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);

        Resources r = getResources();

        // Added the icons to the context menu
        menu.add(0, MENU_IMPORT, 0, r.getString(R.string.importToFDNfromContacts))
                .setIcon(R.drawable.ic_menu_contact);
        menu.add(0, MENU_DELETE, 0, r.getString(R.string.menu_delete))
                .setIcon(android.R.drawable.ic_menu_delete);
        return true;
    }

    /**
     * Allow the menu to be opened ONLY if we're not busy.
     */
    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        boolean result = super.onPrepareOptionsMenu(menu);
        return mDataBusy ? false : result;
    }

    /**
     * Overridden to allow for handling of delete and import.
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Log.d(LOG_TAG, "[onOptionsItemSelected]item text = " + item.getTitle());
        switch (item.getItemId()) {
            case MENU_IMPORT:
                startActivityForResult(CONTACT_IMPORT_INTENT, CONTACTS_PICKER_CODE);
                return true;

            case MENU_DELETE:
                /// [MTK_FDN] Add PIN2 locked tips @{
                if (!needTipsPIN2Blocked()) {
                deleteSelected();
                }
                /// @}
                return true;

            case android.R.id.home:
                onBackPressed();
                return true;
        }

        return super.onOptionsItemSelected(item);
    }

    private void resolveIntent() {
        Intent intent = getIntent();

        mSubscriptionInfoHelper = new SubscriptionInfoHelper(this, intent);

        mName =  intent.getStringExtra(INTENT_EXTRA_NAME);
        mNumber =  intent.getStringExtra(INTENT_EXTRA_NUMBER);

        ///M:ALPS03424700, ADN list can store name with no number
        //mAddContact = TextUtils.isEmpty(mNumber);
        mAddContact = intent.getBooleanExtra(INTENT_EXTRA_ADD, false);
    }

    /**
     * We have multiple layouts, one to indicate that the user needs to
     * open the keyboard to enter information (if the keybord is hidden).
     * So, we need to make sure that the layout here matches that in the
     * layout file.
     */
    private void setupView() {
        mNameField = (EditText) findViewById(R.id.fdn_name);
        if (mNameField != null) {
            mNameField.setOnFocusChangeListener(mOnFocusChangeHandler);
            mNameField.setOnClickListener(mClicked);
            mNameField.addTextChangedListener(mTextWatcher);
        }

        mNumberField = (EditText) findViewById(R.id.fdn_number);
        if (mNumberField != null) {
            mNumberField.setTextDirection(View.TEXT_DIRECTION_LTR);
            mNumberField.setKeyListener(DialerKeyListener.getInstance());
            mNumberField.setOnFocusChangeListener(mOnFocusChangeHandler);
            mNumberField.setOnClickListener(mClicked);
            mNumberField.addTextChangedListener(mTextWatcher);
        }

        if (!mAddContact) {
            if (mNameField != null) {
                mNameField.setText(mName);
            }
            if (mNumberField != null) {
                mNumberField.setText(mNumber);
            }
        }

        mButton = (Button) findViewById(R.id.button);
        if (mButton != null) {
            mButton.setOnClickListener(mClicked);
            setButtonEnabled();
        }

        mPinFieldContainer = (LinearLayout) findViewById(R.id.pinc);

    }

    private String getNameFromTextField() {
        return mNameField.getText().toString();
    }

    private String getNumberFromTextField() {
        return mNumberField.getText().toString();
    }

    /**
     * Enable Save button if text has been added to both name and number
     */
    private void setButtonEnabled() {
        if (mButton != null && mNameField != null && mNumberField != null) {
            mButton.setEnabled(mNameField.length() > 0 && mNumberField.length() > 0);
        }
    }

    /**
      * @param number is voice mail number
      * @return true if number length is less than 20-digit limit
      *
      * TODO: Fix this logic.
      */
     private boolean isValidNumber(String number) {
         ///M: ALPS04014979 Only check number is empty after stripSeparators
         return (number.length() > 0);
     }


    private void addContact() {
        if (DBG) log("addContact");

        ///M: ALPS03938923 Use stripSeparators instead of convertAndStrip
        final String number = PhoneNumberUtils.stripSeparators(getNumberFromTextField());

        if (!isValidNumber(number)) {
            handleResult(false, true);
            return;
        }

        Uri uri = FdnList.getContentUri(mSubscriptionInfoHelper);

        ContentValues bundle = new ContentValues(3);
        bundle.put("tag", getNameFromTextField());
        bundle.put("number", number);
        bundle.put("pin2", mPin2);

        mQueryHandler = new QueryHandler(getContentResolver());
        mQueryHandler.startInsert(0, null, uri, bundle);
        displayProgress(true);
        showStatus(getResources().getText(R.string.adding_fdn_contact));
    }

    private void updateContact() {
        if (DBG) log("updateContact");

        final String name = getNameFromTextField();
        ///M: ALPS03938923 Use stripSeparators instead of convertAndStrip
        final String number = PhoneNumberUtils.stripSeparators(getNumberFromTextField());

        if (!isValidNumber(number)) {
            handleResult(false, true);
            return;
        }
        Uri uri = FdnList.getContentUri(mSubscriptionInfoHelper);

        ContentValues bundle = new ContentValues();
        bundle.put("tag", mName);
        bundle.put("number", mNumber);
        bundle.put("newTag", name);
        bundle.put("newNumber", number);
        bundle.put("pin2", mPin2);

        mQueryHandler = new QueryHandler(getContentResolver());
        mQueryHandler.startUpdate(0, null, uri, bundle, null, null);
        displayProgress(true);
        showStatus(getResources().getText(R.string.updating_fdn_contact));
    }

    /**
     * Handle the delete command, based upon the state of the Activity.
     */
    private void deleteSelected() {
        // delete ONLY if this is NOT a new contact.
        if (!mAddContact) {
            Intent intent = mSubscriptionInfoHelper.getIntent(DeleteFdnContactScreen.class);
            intent.putExtra(INTENT_EXTRA_NAME, mName);
            intent.putExtra(INTENT_EXTRA_NUMBER, mNumber);
            startActivity(intent);
        }
        finish();
    }

    private void authenticatePin2() {
        Intent intent = new Intent();
        intent.setClass(this, GetPin2Screen.class);
        intent.setData(FdnList.getContentUri(mSubscriptionInfoHelper));
        /// [MTK_FDN] Add this for query left pin retry numbers
        intent.putExtra(SubscriptionInfoHelper.SUB_ID_EXTRA, mSubscriptionInfoHelper.getSubId());
        startActivityForResult(intent, PIN2_REQUEST_CODE);
    }

    private void displayProgress(boolean flag) {
        // indicate we are busy.
        mDataBusy = flag;
        getWindow().setFeatureInt(
                Window.FEATURE_INDETERMINATE_PROGRESS,
                mDataBusy ? PROGRESS_VISIBILITY_ON : PROGRESS_VISIBILITY_OFF);
        // make sure we don't allow calls to save when we're
        // not ready for them.
        mButton.setClickable(!mDataBusy);
    }

    /**
     * Removed the status field, with preference to displaying a toast
     * to match the rest of settings UI.
     */
    private void showStatus(CharSequence statusMsg) {
        if (statusMsg != null) {
            Toast.makeText(this, statusMsg, Toast.LENGTH_LONG)
                    .show();
        }
    }

    private void handleResult(boolean success, boolean invalidNumber) {
        if (success) {
            if (DBG) log("handleResult: success!");
            showStatus(getResources().getText(mAddContact ?
                    R.string.fdn_contact_added : R.string.fdn_contact_updated));
        } else {
            if (DBG) log("handleResult: failed!");
            if (invalidNumber) {
                ///M: ALPS04014979 Only check number is empty after stripSeparators
                showStatus(getResources().getText(R.string.fdn_contact_number_invalid));
            } else {
                /// Get phone by sub id. @{
                if (mSubscriptionInfoHelper.getPhone().getIccCard().getIccPin2Blocked()) {
                    showStatus(getResources().getText(R.string.fdn_enable_puk2_requested));
                } else if (mSubscriptionInfoHelper.getPhone().getIccCard().getIccPuk2Blocked()) {
                /// @}
                    showStatus(getResources().getText(R.string.puk2_blocked));
                } else {
                    // There's no way to know whether the failure is due to incorrect PIN2 or
                    // an inappropriate phone number.
                    showStatus(getResources().getText(R.string.pin2_or_fdn_invalid));
                }
            }
        }

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                finish();
            }
        }, 2000);

    }

    private final View.OnClickListener mClicked = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            if (mPinFieldContainer.getVisibility() != View.VISIBLE) {
                return;
            }

            if (v == mNameField) {
                mNumberField.requestFocus();
            } else if (v == mNumberField) {
                mButton.requestFocus();
            } else if (v == mButton) {
                ///M: ALPS03938923 Use stripSeparators instead of convertAndStrip
                final String number = PhoneNumberUtils.stripSeparators(getNumberFromTextField());

                if (!isValidNumber(number)) {
                    handleResult(false, true);
                    return;
                }
                // Authenticate the pin AFTER the contact information
                // is entered, and if we're not busy.
                if (!mDataBusy) {
                    /// [MTK_FDN] Add check for PIN2 locked tips. @{
                    if (!needTipsPIN2Blocked()) {
                        authenticatePin2();
                    } else {
                        log("[onClick] PIN2 Locked!!");
                    }
                    /// @}
                }
            }
        }
    };

    private final View.OnFocusChangeListener mOnFocusChangeHandler =
            new View.OnFocusChangeListener() {
        @Override
        public void onFocusChange(View v, boolean hasFocus) {
            if (hasFocus) {
                TextView textView = (TextView) v;
                Selection.selectAll((Spannable) textView.getText());
            }
        }
    };

    private final TextWatcher mTextWatcher = new TextWatcher() {
        @Override
        public void afterTextChanged(Editable arg0) {}

        @Override
        public void beforeTextChanged(CharSequence arg0, int arg1, int arg2, int arg3) {}

        @Override
        public void onTextChanged(CharSequence arg0, int arg1, int arg2, int arg3) {
            setButtonEnabled();
        }
    };

    private class QueryHandler extends AsyncQueryHandler {
        public QueryHandler(ContentResolver cr) {
            super(cr);
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor c) {
        }

        @Override
        protected void onInsertComplete(int token, Object cookie, Uri uri) {
            if (DBG) log("onInsertComplete");
            displayProgress(false);
            /** [MTK_FDN] Remove Google default action, follow MTK own framework result. @{
            handleResult(uri != null, false);
             @} **/
            handleResult(getInsertResult(uri) > 0, false);
        }

        @Override
        protected void onUpdateComplete(int token, Object cookie, int result) {
            if (DBG) log("onUpdateComplete");
            displayProgress(false);
            handleResult(result > 0, false);
        }

        @Override
        protected void onDeleteComplete(int token, Object cookie, int result) {
        }
    }

    private void log(String msg) {
        Log.d(LOG_TAG, "[EditFdnContact] " + msg);
    }

    //----------------------- MTK -------------------------
    // [MTK_FDN] @{
    private static final String INTENT_EXTRA_ADD = "addContact";

    /**
     * Get insert result.
     *  1= Ok
     *  0= unknown error code
     * -1= number length too long
     * -2= name length too long
     * -3= Storage is full
     * -4= Phone book is not ready
     * -5= Pin2 error
     * @param uri
     * @return
     */
    private int getInsertResult(Uri uri) {
        if (uri == null) {
            log("[getInsertResult]  uri == null.");
            return 0;
        }
        log("[getInsertResult]  uri.toString() = " + uri.toString());
        String str = uri.toString();
        int result = 0;
        if (str.indexOf("error") == -1) {
            result = 1;
        } else {
            str = str.replace("content://icc/error/", "");
            result = Integer.valueOf(str).intValue();
        }
        log("[getInsertResult] result=" + result);
        return result;
    }

    private boolean needTipsPIN2Blocked() {
        if (CallSettingUtils.getPin2RetryNumber(mSubscriptionInfoHelper.getSubId()) == 0) {
            log("[onClick] retry number is 0, tips...");
            Toast.makeText(
                    this, getString(R.string.fdn_puk_need_tips), Toast.LENGTH_SHORT).show();
            return true;
        }
        return false;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        PhoneGlobals.getInstance().removeSubInfoUpdateListener(this);
    }

    @Override
    public void handleSubInfoUpdate() {
        finish();
    }
    /// @}
}
