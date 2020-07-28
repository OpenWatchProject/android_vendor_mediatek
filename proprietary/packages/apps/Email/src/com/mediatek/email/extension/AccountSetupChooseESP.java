/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
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
package com.mediatek.email.extension;

import java.util.regex.Matcher;
import java.util.regex.Pattern;


import com.android.email.R;

import android.app.Activity;
import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.android.email.activity.setup.AccountSetupFinal;
import com.android.emailcommon.Logging;
import com.android.emailcommon.mail.Address;
import com.android.mail.utils.LogUtils;
import com.mediatek.email.ext.IServerProviderExt;

/**
 * The AccountSetupChooseESP activity help user to choose the commonly used
 * email service provider(ESP) more quickly.
 */

public class AccountSetupChooseESP extends ListActivity {

    public static final String UNKNOWN_DOMAIN = "UNKNOWN";
    private static String sSelectedProviderDomain = UNKNOWN_DOMAIN;
    // use this flag to check whether set up is finished, if finish, should finish this activity.
    private boolean mSetUpFinsished = false;
    private static final String EXTRA_ACCOUNT_SETUP_FINISHED = "extra_account_setup_finished";
    // Cache the intent, and using for re-start original flow.
    private Intent mCachedIntent;

    // use these tag refer to the provider names, domains or logos position in
    // xml file, if want use other providers to display, just add these names,
    // domains and logos
    // to xml, and change these tags.
    private final static int SERVER_189_PROVIDER_POSITION_IN_XML = 0;
    private final static int EXCHAGNE_PROVIDER_POSITION_IN_XML = 1;
    private final static int DEFAULT_PROVIDER_POSITION_IN_XML = SERVER_189_PROVIDER_POSITION_IN_XML;
    private final static int FIRST_PROVIDER_POSITION_IN_XML = SERVER_189_PROVIDER_POSITION_IN_XML;
    private final static int SENCOND_PROVIDER_POSITION_IN_XML = EXCHAGNE_PROVIDER_POSITION_IN_XML;
    private final static int THIRD_PROVIDER_POSITION_IN_XML = 2;
    private final static int FOURTH_PROVIDER_POSITION_IN_XML = 3;
    private final static int FIFTH_PROVIDER_POSITION_IN_XML = 4;
    private final static int SIXTH_PROVIDER_POSITION_IN_XML = 5;
    private final static int SEVENTH_PROVIDER_POSITION_IN_XML = 6;
    private final static int[] PROVIDER_POSITION = { FIRST_PROVIDER_POSITION_IN_XML,
            SENCOND_PROVIDER_POSITION_IN_XML, THIRD_PROVIDER_POSITION_IN_XML,
            FOURTH_PROVIDER_POSITION_IN_XML, FIFTH_PROVIDER_POSITION_IN_XML,
            SIXTH_PROVIDER_POSITION_IN_XML, SEVENTH_PROVIDER_POSITION_IN_XML };

    private static final String TAG = "AccountSetupChooseESP";

    private IServerProviderExt mExtension;
    // use to save ESP names.
    private String[] mESPNames;
    // use to save ESP domain names.
    private String[] mESPDomains;
    // use to save ESP icons to show.
    private int[] mESPIcons;
    // decide how many ESP to show in list.
    private int mEspDisplayCountLimit;

    // if user cancel or finished the set up flow, sIsSetUpFinsished will be true, activity will
    // be finish.
    public static void onAccountSetupFinished(Activity fromActivity) {
        LogUtils.d(TAG, "account set up finished");
        Intent i = new Intent(fromActivity, AccountSetupChooseESP.class);
        // use FLAG_ACTIVITY_REORDER_TO_FRONT cause this activity to be brought to the front of its
        // task's history stack if it is already running.
        // this activity will be finished by AccountSetupBasic
        // actionAccountCreateFinishedAccountFlow(), actionAccountCreateFinished(), or itself
        // onCreate() and onResume() using sIsSetUpFinsished flag to check whether account set up
        // finished.
        i.setFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
        i.putExtra(EXTRA_ACCOUNT_SETUP_FINISHED, true);
        fromActivity.startActivity(i);
    }

    private void redirectToOriginalFlow(Context context) {
        mCachedIntent.setClass(context, AccountSetupFinal.class);
        context.startActivity(mCachedIntent);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        LogUtils.d(TAG, "AccountSetupChooseESP onCreate");
        resetESPSettings();
        // for add account not used AccountSetupChooseESP activity, that means add accounts from
        // external,this activity is create by onAccountSetupFinished, destroy activity early
        // on onCreate.
        mCachedIntent = getIntent();
        Intent i = getIntent();
        mSetUpFinsished = i.getBooleanExtra(EXTRA_ACCOUNT_SETUP_FINISHED, false);
        if (mSetUpFinsished) {
            finish();
            return;
        }
        /*// if this was start by FLOW_MODE_ACCOUNT_MANAGER_EAS, we just ignore it.
        int flowMode = i.getIntExtra(AccountSetupBasics.EXTRA_FLOW_MODE, SetupData.FLOW_MODE_UNSPECIFIED);
        if (flowMode == SetupData.FLOW_MODE_ACCOUNT_MANAGER_EAS) {
            redirectToOriginalFlow(this);
            finish();
            return;
        }*/
        setContentView(R.layout.mtk_account_setup_choose_provider);
        mExtension = OPExtensionFactory.getProviderExtension(this);
        loadResources();
        MyAdapter adapter = new MyAdapter(this);
        setListAdapter(adapter);
    }

    // any where destroy this activity means set up finished or canceled, reset
    // sIsSetUpFinsished flag.
    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    // if the set up flow is over, should finish this activity.
    // should restore the basic settings when every resume.
    @Override
    public void onResume() {
        super.onResume();
        // for add account for edit new message, should destroy this activity from resume, like
        // AccountsetupBasic activity.
        if (mSetUpFinsished) {
            finish();
        }
        resetESPSettings();
    }

    // use to clear the old setting data.
    public static void resetESPSettings() {
        sSelectedProviderDomain = UNKNOWN_DOMAIN;
    }

    private void setSelectedDomain(int position) {
        sSelectedProviderDomain = mESPDomains[PROVIDER_POSITION[position]];
    }

    public static String getSelectedDoamin() {
        return sSelectedProviderDomain;
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        LogUtils.d(TAG, "AccountSetupChooseESP onListItemClick position: " + position);
        // record which item user selected.
        setSelectedDomain(position);
        // restart the original account setup flow.
        redirectToOriginalFlow(this);
    }

    public class MyAdapter extends BaseAdapter {
        private LayoutInflater mInflater;
        private int mItemCount = 0;

        public MyAdapter(Context context) {
            this.mInflater = LayoutInflater.from(context);
            mItemCount = mEspDisplayCountLimit;
        }

        public int getCount() {
            return mItemCount;
        }

        public Object getItem(int arg0) {
            return null;
        }

        public long getItemId(int arg0) {
            return 0;
        }

        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                convertView = mInflater.inflate(R.layout.mtk_account_setup_provider_item, null);
            }
            ImageView logoView = (ImageView) convertView.findViewById(R.id.logo);
            TextView othersView = (TextView) convertView.findViewById(R.id.name);
            logoView.setImageDrawable((mExtension.getContext().getResources()
                    .getDrawable(mESPIcons[PROVIDER_POSITION[position]])));
            othersView.setText(mESPNames[PROVIDER_POSITION[position]]);
            return convertView;
        }
    }

    private void loadResources() {
        mESPNames = mExtension.getProviderNames();
        mESPIcons = mExtension.getProviderIcons();
        mESPDomains = mExtension.getProviderDomains();
        mEspDisplayCountLimit = mExtension.getDisplayESPNum();
    }

    /**
     * check whether this account is a special account, the signature
     * and username of special account will be set  as special needed.
     *
     * like: 189 account, will set it's username as "189 provider", signature as "send from 189".
     * @param accountName
     * @return
     */
    public static boolean isSpecialESPAccount(IServerProviderExt extension, String accountName) {
        if (extension == null || TextUtils.isEmpty(accountName) || !Address.isAllValid(accountName)) {
            return false;
        }
        String specialESPName = extension.getDefaultProviderDomain();
        String[] emailParts = accountName.split("@");
        if (specialESPName != null) {
            String domain = emailParts[1];
            return domain.equals(specialESPName) ? true : false;
        } else {
            Logging.d(TAG, "account name is wrong! accountName : " + accountName);
            return false;
        }
    }

    /** Check whether selected exchange from this activity.
     *
     * @param extension
     * @return
     */
    public static boolean isSelectedExchange(final IServerProviderExt extension) {
        if (extension == null || !extension.isSupportProviderList()) {
            return false;
        }
        final String[] providerDomains = extension.getProviderDomains();
        if (providerDomains != null) {
            String exchangeDefaultDomain = providerDomains[EXCHAGNE_PROVIDER_POSITION_IN_XML];
            return sSelectedProviderDomain.equals(exchangeDefaultDomain);
        }
        return false;
    }

    /** Check whether selected special ESP from this activity. like "189"
     *
     * @param extension
     * @return
     */
    public static boolean isSelectedSpecialESP(IServerProviderExt extension) {
        if (extension == null || !extension.isSupportProviderList()) {
            return false;
        }
        final String[] providerDomains = extension.getProviderDomains();
        if (providerDomains != null) {
            String specialESPDomain = providerDomains[DEFAULT_PROVIDER_POSITION_IN_XML];
            return sSelectedProviderDomain.equals(specialESPDomain);
        }
        return false;
    }

    /** Check whether selected "exchange" or "others" from this activity.
     *
     * @return
     */
    public static boolean isServerDomainUnknown() {
        String domain = sSelectedProviderDomain;
        if (domain.contains(".")) {
            return false;
        }
        return true;
    }

    /**
     * check whether one Email Prefix Name is valid,
     *
     * @param emailName
     * @return if name is valid,return true,
     *         like, "test" return true, "ts%e()t+-#" return false.
     */
    public static boolean isEmailPrefixNameValid(String emailName) {
        Pattern p = Pattern
                .compile("^((\\u0022.+?\\u0022@)|(([\\Q-!#$%&'*+/=?^`{}|~\\E\\w])+(\\.[\\Q-!#$%&'*+/=?^`{}|~\\E\\w]+)*@?))");
        Matcher m = p.matcher(emailName);
        return m.matches();
    }

}
