/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.android.email.activity.setup;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Rect;
import android.os.AsyncTask;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnFocusChangeListener;
import android.widget.EditText;
import android.widget.MultiAutoCompleteTextView.Tokenizer;

import com.android.email.R;
import com.android.email.activity.UiUtilities;
import com.android.emailcommon.mail.Address;
import com.mediatek.email.extension.AccountSetupChooseMailProvider;
import com.mediatek.email.provider.history.EmailAddress;
import com.mediatek.email.ui.DropdownAccountsArrayAdapter;
import com.mediatek.email.ui.DropdownAccountsFilter;
import com.mediatek.email.ui.DropdownAddressFilter;
import com.mediatek.email.ui.EmailAccountAutoCompleteTextView;
import com.mediatek.email.ui.EmailAccountAutoCompleteTextView.EmailAccountTokenizer;
import com.mediatek.email.ui.EmailAccountAutoCompleteTextView.EmailHistoryTokenizer;

import java.util.Arrays;

public class AccountSetupBasicsFragment extends AccountSetupFragment {
    ///M: Change mEmailView from EditeView to EmailAccountAutoCompleteTextView;
    private EmailAccountAutoCompleteTextView mEmailView;
    private View mManualSetupView;
    private boolean mManualSetup;

    /** M: add for email address history and account list.@{ */
    DropdownAccountsArrayAdapter<String> mDropdownAdapter;
    DropdownAccountsArrayAdapter<String> mHisAddressAdapter;
    Tokenizer mHisAddressTokenizer = new EmailHistoryTokenizer();
    Tokenizer mEmailAccountTokenizer = new EmailAccountTokenizer();
    Context mContext = null;
    /** @} */
    /** M: add for email plugin's specified email hints and domains @{ */
    private String mEmailHint;
    private String mDefaultDomain;
    /** @} */

    public interface Callback extends AccountSetupFragment.Callback {
    }

    public static AccountSetupBasicsFragment newInstance() {
        return new AccountSetupBasicsFragment();
    }

    public AccountSetupBasicsFragment() {}

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        final View view = inflateTemplatedView(inflater, container,
                R.layout.account_setup_basics_fragment, -1);

        mEmailView = (EmailAccountAutoCompleteTextView) UiUtilities.getView(view, R.id.account_email);
        mManualSetupView = UiUtilities.getView(view, R.id.manual_setup);
        mManualSetupView.setOnClickListener(this);
        /// M: Set email hint if possible @{
        Activity act = getActivity();
        if (act != null) {
            Intent intent = act.getIntent();
            mEmailHint = intent.getStringExtra(AccountSetupChooseMailProvider.EXTRA_ACCOUNT_SETUP_EMAIL_HINT);
            mDefaultDomain = intent.getStringExtra(AccountSetupChooseMailProvider.EXTRA_ACCOUNT_SETUP_DEFAULT_DOMAIN);
        }
        if (!TextUtils.isEmpty(mEmailHint)) {
            mEmailView.setHint(mEmailHint);
        }
        /// @}

        final TextWatcher textWatcher = new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                /** M: Add for auto match address when create account. @{ */
                if (!TextUtils.isEmpty(s) && !(EmailPluginUtils.isSupportProviderList(mContext)
                        && AccountSetupChooseMailProvider.isServerDomainValid(mDefaultDomain))) {
                    int indexOfAt = s.toString().indexOf("@");
                    if (indexOfAt > 0 && indexOfAt < mEmailView.getSelectionStart()) {
                        if (mEmailView.getAdapter() != mDropdownAdapter) {
                            mEmailView.setAdapter(mDropdownAdapter);
                            mEmailView.setTokenizer(mEmailAccountTokenizer);
                        }
                    } else {
                        if (mEmailView.getAdapter() != mHisAddressAdapter) {
                            mEmailView.setAdapter(mHisAddressAdapter);
                            mEmailView.setTokenizer(mHisAddressTokenizer);
                        }
                    }
                }
                if (mDropdownAdapter != null) {
                    String email = mEmailView.getText().toString();
                    String[] emailParts = email.split("@");
                    if (emailParts.length > 0) {
                        mDropdownAdapter.setUserName(emailParts[0]);
                    }
                    // Scroll the editTextView to the top of the screen.
                    int scrollX = mEmailView.getScrollX();
                    int scrollY = mEmailView.getScrollY();
                    int xoff = 0;
                    int yoff = 0;
                    Rect r = new Rect(scrollX, scrollY,  scrollX + mEmailView.getDropDownWidth() + xoff,
                            scrollY + 500 + mEmailView.getHeight() + yoff);
                    mEmailView.requestRectangleOnScreen(r, true);
                }
                /** @} */
            }

            @Override
            public void afterTextChanged(Editable s) {
                validateFields();
            }
        };

        mEmailView.addTextChangedListener(textWatcher);

        setPreviousButtonVisibility(View.GONE);

        setManualSetupButtonVisibility(View.VISIBLE);

        /**
         * M: Modify for email address history.When view loses focus, save the
         * email address, check this account whether being set a default domain
         * and complete the server name auto. @{
         */
        mEmailView.setOnFocusChangeListener(new OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                // when view loses focus, save the email address.
                if (!hasFocus) {
                    /// M: Auto complete the email address with specified default domain if possible @{
                    if (EmailPluginUtils.isSupportProviderList(mContext)) {
                        String address = mEmailView.getText().toString();
                        if (AccountSetupChooseMailProvider.isEmailPrefixNameValid(address)
                                && AccountSetupChooseMailProvider.isServerDomainValid(mDefaultDomain)) {
                            int end = address.endsWith("@") ? address.length() - 1 : address
                                    .length();
                            address = address.substring(0, end) + "@" + mDefaultDomain;
                            mEmailView.setText(address);
                        }
                    }
                    /// @}
                    new SaveAddressTask().execute(mEmailView.getText().toString());
                }
            }
        });
        mContext = getActivity();
        mDropdownAdapter = new DropdownAccountsArrayAdapter<String>(mContext,
                R.layout.account_setup_popup_list,
                AccountSettingsUtils.collectAutoSetupDomain(mContext));
        mDropdownAdapter.setFilter(new DropdownAccountsFilter<String>(mDropdownAdapter));
        mHisAddressAdapter = new DropdownAccountsArrayAdapter<String>(mContext, R.layout.account_setup_popup_list);
        mHisAddressAdapter.setFilter(new DropdownAddressFilter<String>(mHisAddressAdapter));
        mEmailView.setAdapter(mHisAddressAdapter);
        mEmailView.setTokenizer(mHisAddressTokenizer);

        /// M: Set history adapter if we don't have a valid domain set, otherwise
        // we use drop-down adapter with only the selected domain @{
        if (EmailPluginUtils.isSupportProviderList(mContext)
                && AccountSetupChooseMailProvider.isServerDomainValid(mDefaultDomain)) {
            // replace objects with only the selected valid domain
            mDropdownAdapter.setObjects(Arrays.asList("@" + mDefaultDomain));
            // replace with a new filter
            mDropdownAdapter.setFilter(new DropdownAccountsFilter<String>(mDropdownAdapter));
            mEmailView.setAdapter(mDropdownAdapter);
            mEmailView.setTokenizer(mEmailAccountTokenizer);
        }
        /// @}

        new PrepareAddressDataTask().execute();
        /** @} */
        return view;
    }

    @Override
    public void onViewStateRestored(Bundle savedInstanceState) {
        super.onViewStateRestored(savedInstanceState);
        validateFields();
    }

    @Override
    public void onClick(View v) {
        final int viewId = v.getId();
        final Callback callback = (Callback) getActivity();

        if (viewId == R.id.next) {
            // Handle "Next" button here so we can reset the manual setup diversion
            mManualSetup = false;
            callback.onNextButton();
            /// M: prevents click it quickly after previous one.
            blockNextButton();
        } else if (viewId == R.id.manual_setup) {
            mManualSetup = true;
            callback.onNextButton();
        } else {
            super.onClick(v);
        }
    }

    /** M: API to check  valid domain in email address @{ */
    public boolean isValidDomain(String email_address){
        final String domain = email_address != null ? email_address.split("@")[1].trim() : null;
        char[] domain_char_sequence = domain.toCharArray();
        if (TextUtils.isEmpty(domain)|| domain.contains("@") || domain.contains("\u200F")) {
             return false;
         }
        // check for each char if it is not an Ascii char
        for(char check : domain_char_sequence)
        {
            // these are the set of valid domain part of email as per RFC952 ,RFC822
            if( !((check >= 0x30 && check <= 0x39) // for 0-9
                  || (check >= 0x41 && check <= 0x5a) //for A-Z
                  || (check >= 0x61 && check <= 0x7a) // for a-z
                  || (check == 0x2d || check == 0x2e))) // for "-" and "."
            {
                return false;
            }
        }
        return true;
    }
   /** @} */

    private void validateFields() {
        final String emailField = getEmail();
        final Address[] addresses = Address.parse(emailField);

        final boolean emailValid = !TextUtils.isEmpty(emailField)
                && addresses.length == 1
                && !TextUtils.isEmpty(addresses[0].getAddress())
                /// M: Must verify if it is valid address, otherwise the Email will
                /// crash when setup account through autodiscover.
                && Address.isValidEmailAddress(addresses[0].getAddress())
                && isValidDomain(emailField);

        setNextButtonEnabled(emailValid);
    }


    /**
     * Set visibitlity of the "manual setup" button
     * @param visibility {@link View#INVISIBLE}, {@link View#VISIBLE}, {@link View#GONE}
     */
    public void setManualSetupButtonVisibility(int visibility) {
        mManualSetupView.setVisibility(visibility);
    }

    @Override
    public void setNextButtonEnabled(boolean enabled) {
        super.setNextButtonEnabled(enabled);
        mManualSetupView.setEnabled(enabled);
    }

    public void setEmail(final String email) {
        mEmailView.setText(email);
    }

    public String getEmail() {
        return mEmailView.getText().toString().trim();
    }

    public boolean isManualSetup() {
        return mManualSetup;
    }

    /**
     * M: PrepareAddressDataTask for querying address outside of the UI thread, and update
     * the data of mHisAddressAdapter.
     */
    private class PrepareAddressDataTask extends AsyncTask<String, Void, Cursor> {
        @Override
        protected Cursor doInBackground(String... params) {
            Cursor c = EmailAddress.queryAddress(mContext, EmailAddress.CONTENT_URI);
            return c;
        }

        @Override
        protected void onPostExecute(Cursor c) {
            super.onPostExecute(c);
            mHisAddressAdapter.clear();
            if (c != null) {
                try {
                    while (c.moveToNext()) {
                        String address = c.getString(1);
                        mHisAddressAdapter.add(address);
                    }
                } finally {
                    if (c != null) {
                        c.close();
                    }
                }
            }
        }

    }

    /**
     * M: SaveAddressTask for saving address outside of the UI thread
     */
    private class SaveAddressTask extends AsyncTask<String, Void, Void> {
        @Override
        protected Void doInBackground(String... params) {
            EmailAddress.saveAddress(mContext, params[0]);
            new PrepareAddressDataTask().execute();
            return null;
        }
    }
}
