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

package com.mediatek.ppl.ui;

import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.Toast;

import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;

public class SetupPasswordActivity extends PplBasicActivity implements PplRelativeLayout.IOnResizeListener {

    private static final String TAG = "PPL/SetupPasswordActivity";
    private static final String KEY_FIRST_INPUT = "first_input";
    private static final String KEY_SECOND_INPUT = "second_input";

    private Intent mLaunchIntent;

    protected ScrollView mScrollView;
    protected ProgressBar mProgressBar;
    protected PplRelativeLayout mLayoutOuter;
    protected LinearLayout mLayoutUp;
    protected LinearLayout mLayoutDown;
    protected Button mNextButton;
    protected EditText mFirstPassword;
    protected EditText mSecondPassword;

    @Override
    protected void onCreate(Bundle saveInstanceState) {
        super.onCreate(saveInstanceState);
        mLaunchIntent = getIntent();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putString(KEY_FIRST_INPUT, mFirstPassword.getText().toString());
        outState.putString(KEY_SECOND_INPUT, mSecondPassword.getText().toString());
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        if (null != savedInstanceState) {
            mFirstPassword.setText(savedInstanceState.getString(KEY_FIRST_INPUT));
            mSecondPassword.setText(savedInstanceState.getString(KEY_SECOND_INPUT));
        }
        super.onRestoreInstanceState(savedInstanceState);
    }

    @Override
    protected void onRegisterEvent() {
        mEventReceiver.addAction(PplService.Intents.UI_QUIT_SETUP_WIZARD);
        mEventReceiver.addAction(PplService.Intents.UI_NO_SIM);
    }

    @Override
    protected void onPrepareLayout() {

        setContentView(R.layout.setup_password);

        mScrollView = (ScrollView) findViewById(R.id.scrollview_setup_password);
        mLayoutOuter = (PplRelativeLayout) findViewById(R.id.layout_setup_password_outer);
        mLayoutUp = (LinearLayout) findViewById(R.id.layout_setup_password_up);
        mLayoutDown = (LinearLayout) findViewById(R.id.layout_setup_password_down);

        mProgressBar = (ProgressBar) findViewById(R.id.common_progress);
        mNextButton = (Button) findViewById(R.id.btn_bottom_next);
        mFirstPassword = (EditText) findViewById(R.id.et_setup_password_input1);
        mSecondPassword = (EditText) findViewById(R.id.et_setup_password_input2);

        mLayoutOuter.setOnResizeListener(this);
        mNextButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                onButtonClicked();
            }
        });

        mFirstPassword.setTypeface(Typeface.SANS_SERIF);
        mFirstPassword.addTextChangedListener(mTextWather);
        mSecondPassword.setTypeface(Typeface.SANS_SERIF);
        mSecondPassword.addTextChangedListener(mTextWather);
    }


    @Override
    protected void onInitLayout() {
        mNextButton.setEnabled(false);
        mProgressBar.setVisibility(View.VISIBLE);
        mLayoutUp.setVisibility(View.GONE);
        mLayoutDown.setVisibility(View.GONE);
        mFirstPassword.requestFocus();
    }

    @Override
    protected void onPplServiceConnected(Bundle saveInstanceState) {
        if (null != mLaunchIntent  &&
            (0 != (mLaunchIntent.getFlags() & Intent.FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY))) {

            if (mBinder.isEnabled()) {
                gotoActivity(this, LoginPplActivity.class);
                finish();
            } else if (mBinder.isProvisioned()) {
                gotoActivity(this, LaunchPplActivity.class);
                finish();
            } else {
                mProgressBar.setVisibility(View.GONE);
                mLayoutUp.setVisibility(View.VISIBLE);
                mLayoutDown.setVisibility(View.VISIBLE);
            }
        } else {
            mProgressBar.setVisibility(View.GONE);
            mLayoutUp.setVisibility(View.VISIBLE);
            mLayoutDown.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void onPanelChange(int h) {
        mScrollView.scrollTo(0, h);
    }

    private TextWatcher mTextWather = new TextWatcher() {

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
            if (mFirstPassword.getText().length() >= PplService.MIN_PASSWORD_LENGTH &&
                mSecondPassword.getText().length() >= PplService.MIN_PASSWORD_LENGTH) {
                    mNextButton.setEnabled(true);
                } else {
                    mNextButton.setEnabled(false);
                }
        }

        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {

        }

        @Override
        public void afterTextChanged(Editable s) {

        }
    };

    protected void onButtonClicked() {
        if (!mFirstPassword.getText().toString().equals(mSecondPassword.getText().toString())) {
            Toast.makeText(this, R.string.toast_passwords_do_not_match, Toast.LENGTH_SHORT).show();
        } else {
            mBinder.savePassword(mFirstPassword.getText().toString(), PplService.EDIT_TYPE_SETUP);
            gotoActivity(this, SetupTrustedContactsActivity.class);
            //gotoActivity(this, LaunchPplActivity.class);
        }
    }

}
