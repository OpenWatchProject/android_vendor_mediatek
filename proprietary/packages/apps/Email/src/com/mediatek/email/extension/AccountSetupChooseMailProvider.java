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

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
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
import com.android.email.activity.setup.EmailPluginUtils;
import com.android.email.setup.AuthenticatorSetupIntentHelper;
import com.android.emailcommon.Logging;
import com.android.emailcommon.mail.Address;
import com.android.mail.ui.AbstractActivityController;
import com.android.mail.utils.LogUtils;
import com.mediatek.email.ext.IServerProviderExt;

/**
 * The AccountSetupChooseMailProvider activity help user to choose the commonly used
 * email service provider(ESP) more quickly.
 */

public class AccountSetupChooseMailProvider extends ListActivity {

    private static final String TAG = "AccountSetupChooseMailProvider";
    /// Exchange domain
    private static final String EXCHANGE_DOMAIN = "exchange";

    /// Extra information for provider list @{
    public static final String EXTRA_ACCOUNT_SETUP_EMAIL_HINT = "extra_account_setup_email_hint";
    public static final String EXTRA_ACCOUNT_SETUP_DEFAULT_DOMAIN = "extra_account_setup_domain";
    /// @}

    // use to save ESP names.
    private String[] mMailProviderNames;
    // use to save ESP domain names.
    private String[] mMailProviderDomains;
    // use to save ESP icons to show.
    private int[] mMailProviderIcons;
    // decide how many ESP to show in list.
    private int mMailProviderCount;
    // use to save ESP email hint, only supported by Vendor Policy
    private String[] mMailProviderHints;
    // ESP resources, maybe provided by CT plugin app or Vendor policy app
    private Resources mResources;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        LogUtils.d(TAG, "AccountSetupChooseMailProvider onCreate");
        setContentView(R.layout.mtk_account_setup_choose_provider);
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

    /**
     * M: Prepare some extra information in an intent and start it to redirect to original flow.
     * @param position
     */
    private void redirectToOriginalFlow(int position) {
        Intent intent = new Intent(getIntent());
        intent.setClass(this, AccountSetupFinal.class);
        // Hints only supported by Vendor Policy
        if (mMailProviderHints != null) {
            intent.putExtra(EXTRA_ACCOUNT_SETUP_EMAIL_HINT, mMailProviderHints[position]);
        }
        intent.putExtra(EXTRA_ACCOUNT_SETUP_DEFAULT_DOMAIN, mMailProviderDomains[position]);
        // If we select eas, add eas account type data.
        if (isSelectedEas(mMailProviderDomains[position])) {
            intent.putExtra(AuthenticatorSetupIntentHelper.EXTRA_FLOW_ACCOUNT_TYPE, getString(R.string.account_manager_type_exchange));
        } else {
            intent.removeExtra(AuthenticatorSetupIntentHelper.EXTRA_FLOW_ACCOUNT_TYPE);
        }
        startActivityForResult(intent, AbstractActivityController.ADD_ACCOUNT_REQUEST_CODE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        switch (requestCode) {
        case AbstractActivityController.ADD_ACCOUNT_REQUEST_CODE:
            // Forward to {@link MailActivityEmail} if success and finish itself
            if (RESULT_OK == resultCode) {
                LogUtils.d(TAG, "Account Setup Success, forward to MailActivityEmail");
                setResult(RESULT_OK);
                finish();
            } else {
                LogUtils.d(TAG, "Account Setup Canceled?");
            }
            break;
        default:
            LogUtils.e(TAG, "Unknown request code: %d", requestCode);
            break;
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        LogUtils.d(TAG, "AccountSetupChooseMailProvider onListItemClick position: " + position);
        // add selected provider information.
        // restart the original account setup flow.
        redirectToOriginalFlow(position);
    }

    public class MyAdapter extends BaseAdapter {
        private LayoutInflater mInflater;
        private int mItemCount = 0;

        public MyAdapter(Context context) {
            this.mInflater = LayoutInflater.from(context);
            mItemCount = mMailProviderCount;
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
            logoView.setImageDrawable((mResources.getDrawable(mMailProviderIcons[position])));
            othersView.setText(mMailProviderNames[position]);
            return convertView;
        }
    }

    private void loadResources() {
        if (EmailPluginUtils.isSupportProviderList(this)) {
            mMailProviderNames = EmailPluginUtils.getProviderNames(this);
            mMailProviderIcons = EmailPluginUtils.getProviderIcons(this);
            mMailProviderDomains = EmailPluginUtils.getProviderDomains(this);
            mMailProviderCount = EmailPluginUtils.getProviderCount(this);
            mResources = EmailPluginUtils.getProviderResources(this);
            mMailProviderHints = EmailPluginUtils.getProviderHints(this);
        } else {
            LogUtils.wtf(TAG, "Neither any of VendorPolicy nor CT plugin supported!");
        }
    }

    /**
     * check whether this account is a special account, the signature
     * and username of special account will be set  as special needed.
     *
     * like: 189 account, will set it's username as "189 provider", signature as "send from 189".
     * @param accountName
     * @return
     */
    public static boolean isSpecialMailAccount(IServerProviderExt extension, String accountName) {
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

    /**
     * Check whether "exchange" option in provider list is selected.
     * @param defaultDomain
     * @return
     */
    public static boolean isSelectedEas(final String defaultDomain) {
        return EXCHANGE_DOMAIN.equals(defaultDomain);
    }

    /**
     * Check whether the given domain string is valid
     * Domain is configured by Vendor, and one '.' is necessary and no '@' should exists
     * @param domain
     * @return
     */
    public static boolean isServerDomainValid(String domain) {
        if (TextUtils.isEmpty(domain) || !domain.contains(".") || domain.contains("@")) {
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
