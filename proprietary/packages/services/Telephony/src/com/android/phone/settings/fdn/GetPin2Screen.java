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

import android.app.ActionBar;
import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.text.InputType;
import android.text.TextUtils;
import android.text.method.DigitsKeyListener;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.android.phone.PhoneGlobals;
import com.android.phone.R;
import com.android.phone.SubscriptionInfoHelper;
import com.mediatek.settings.CallSettingUtils;

/**
 * Pin2 entry screen.
 */
public class GetPin2Screen extends Activity implements TextView.OnEditorActionListener,
        PhoneGlobals.SubInfoUpdateListener {
    private static final String LOG_TAG = PhoneGlobals.LOG_TAG;

    private EditText mPin2Field;
    private Button mOkButton;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        setContentView(R.layout.get_pin2_screen);

        mPin2Field = (EditText) findViewById(R.id.pin);
        mPin2Field.setKeyListener(DigitsKeyListener.getInstance());
        /// [MTK_FDN] Remove this for cursor don't show. @{
        //mPin2Field.setMovementMethod(null);
        /// @}
        mPin2Field.setOnEditorActionListener(this);
        mPin2Field.setInputType(
                InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_VARIATION_PASSWORD);

        mOkButton = (Button) findViewById(R.id.ok);
        mOkButton.setOnClickListener(mClicked);

        /// M: [MTK_FDN]
        onCreateMtk();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private String getPin2() {
        return mPin2Field.getText().toString();
    }

    private void returnResult() {
        Bundle map = new Bundle();
        map.putString("pin2", getPin2());

        Intent intent = getIntent();
        Uri uri = intent.getData();

        Intent action = new Intent();
        if (uri != null) action.setAction(uri.toString());
        setResult(RESULT_OK, action.putExtras(map));
        finish();
    }

    @Override
    public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
        if (actionId == EditorInfo.IME_ACTION_DONE) {
            mOkButton.performClick();
            return true;
        }
        return false;
    }

    private final View.OnClickListener mClicked = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            /// [MTK_FDN] Add check for
            /// 1. If retry number is 0, tips user to Change Pin2
            /// 2. If the Pin2 input is invalid, tips user also @{
            if (CallSettingUtils.getPin2RetryNumber(mSubId) == 0) {
                showStatus(getString(R.string.fdn_puk_need_tips));
                return;
            }
            if (!validatePin(mPin2Field.getText())) {
                showStatus(getString(R.string.invalidPin2));
                return;
            }
            /// @}

            if (TextUtils.isEmpty(mPin2Field.getText())) {
                return;
            }

            returnResult();
        }
    };

    private void log(String msg) {
        Log.d(LOG_TAG, "[GetPin2] " + msg);
    }

    //----------------------- MTK -------------------------
    // [MTK_FDN] Add retry left time behind the title
    private TextView mEnterPin2TitleTips;
    private int mSubId;
    // size limits for the pin.
    private static final int MIN_PIN_LENGTH = 4;
    private static final int MAX_PIN_LENGTH = 8;

    private void onCreateMtk() {
        mSubId = getIntent().getIntExtra(
                SubscriptionInfoHelper.SUB_ID_EXTRA, SubscriptionInfoHelper.NO_SUB_ID);

        /// Add left retry tips
        log("onCreateMtk mSubId: " + mSubId);
        mEnterPin2TitleTips = (TextView) findViewById(R.id.get_pin2_title);
        if (CallSettingUtils.getPin2RetryNumber(mSubId) == 0) {
            mEnterPin2TitleTips.setText(R.string.fdn_puk_need_tips);
        } else {
            mEnterPin2TitleTips.append(
                   "\n" + CallSettingUtils.getPinPuk2RetryLeftNumTips(this, mSubId, true));
        }
        disableActionBar();

        PhoneGlobals.getInstance().addSubInfoUpdateListener(this);
    }

    /**
     * Make the action bar's back key disabled.
     */
    private void disableActionBar() {
        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            // android.R.id.home will be triggered in onOptionsItemSelected()
            actionBar.setDisplayHomeAsUpEnabled(false);
        }
    }

    /**
     * Validate the pin entry.
     * @param pin This is the pin to validate
     * return true if the pin is valid, else return false.
     */
    private boolean validatePin(CharSequence pin) {
        return (pin.length() >= MIN_PIN_LENGTH && pin.length() <= MAX_PIN_LENGTH);
    }

    private void showStatus(CharSequence statusMsg) {
        if (statusMsg != null) {
            Toast.makeText(this, statusMsg, Toast.LENGTH_SHORT)
                    .show();
        }
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
}
