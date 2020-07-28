/*
 * Copyright (C) 2014 MediaTek Inc.
 * Modification based on code covered by the mentioned copyright
 * and/or permission notice(s).
 */
/*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.mediatek.contacts.util;

import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.net.sip.SipManager;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.StructuredPostal;
import android.telephony.TelephonyManager;
import android.view.inputmethod.EditorInfo;

import com.android.contacts.R;
import com.android.contacts.model.AccountTypeManager;
import com.android.contacts.model.account.AccountType.EditField;
import com.android.contacts.model.account.AccountType;
import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.model.account.BaseAccountType.EmailActionInflater;
import com.android.contacts.model.account.BaseAccountType.SimpleInflater;
import com.android.contacts.model.dataitem.DataKind;

import com.google.common.collect.Lists;

import com.mediatek.contacts.aassne.Anr;
import com.mediatek.contacts.model.account.AccountWithDataSetEx;
import com.mediatek.contacts.simcontact.SimCardUtils;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.util.Log;

import java.util.List;
import java.util.Locale;

/**
 * some methods related to AccountType.
 */
public class AccountTypeUtils {
    private static final String TAG = "AccountTypeUtils";

    public static final String ACCOUNT_TYPE_SIM = "SIM Account";
    public static final String ACCOUNT_TYPE_USIM = "USIM Account";
    public static final String ACCOUNT_TYPE_LOCAL_PHONE = "Local Phone Account";
    public static final String ACCOUNT_TYPE_RUIM = "RUIM Account";
    public static final String ACCOUNT_TYPE_CSIM = "CSIM Account";
    public static final String ACCOUNT_NAME_LOCAL_PHONE = "Phone";
    public static final String ACCOUNT_NAME_LOCAL_TABLET = "Tablet";

    /*
     * Descriptions: cu feature change photo by slot id
     *
     * @param context the Context.
     *
     * @param subId the current sub id.
     *
     * @return the icon of the sub id.
     */
    public static Drawable getDisplayIconBySubId(Context context, int subId, int titleRes,
            int iconRes, String syncAdapterPackageName) {

        if (titleRes != -1 && syncAdapterPackageName != null) {
            Log.d(TAG, "[getDisplayIconBySubId] summaryrespackagename !=null");
            final PackageManager pm = context.getPackageManager();
            return pm.getDrawable(syncAdapterPackageName, iconRes, null);
        } else if (titleRes != -1) {
            /*
             * int photo = iconRes; LogUtils.d(TAG, "[Accounttype] subId = " +
             * subId); int i = SubInfoUtils.getColorUsingSubId(subId);
             * LogUtils.d(TAG, "[Accounttype] i = " + i); if (i == 0) { photo =
             * R.drawable.sim_indicator_yellow; } else if (i == 1) { photo =
             * R.drawable.sim_indicator_orange; } else if (i == 2) { photo =
             * R.drawable.sim_indicator_green; } else if (i == 3) { photo =
             * R.drawable.sim_indicator_purple; } else { photo =
             * R.drawable.ic_sim_card_white_36dp; } return
             * context.getResources().getDrawable(photo);
             */
            // Using google default sim icon.
            Drawable photoDrable = SubInfoUtils.getIconDrawable(subId);
            return photoDrable;

        } else {
            return null;
        }
    }

    /**
     * If the account name is one of Icc Card account names, like "USIM Account"
     * return true, otherwise, means the account is not a SIM/USIM/UIM account.
     *
     * FIXME: this implementation is not good, not OO. should try to remove it
     * in future refactor
     *
     * @param accountTypeString
     *            generally, it's a string like "USIM Account" or
     *            "Local Phone Account"
     * @return if it's a IccCard account, return true, otherwise false.
     */
    public static boolean isAccountTypeIccCard(String accountTypeString) {
        boolean isIccCardAccount = (ACCOUNT_TYPE_SIM.equals(accountTypeString)
                || ACCOUNT_TYPE_USIM.equals(accountTypeString)
                || ACCOUNT_TYPE_RUIM.equals(accountTypeString)
                || ACCOUNT_TYPE_CSIM.equals(accountTypeString));
        return isIccCardAccount;
    }

    /**
     * Considering SIM account will appear as it's display name, so we wrapper
     * this method to get display account name.
     *
     * @param context
     *            Context
     * @param accountName
     *            the original name
     * @return if the account is a SIM account, then return it's display name,
     *         otherwise, do nothing
     */
    public static String getDisplayAccountName(Context context, String accountName) {
        int subId = getSubIdBySimAccountName(context, accountName);
        return getDisplayAccountName(subId, accountName);
    }

    public static String getDisplayAccountName(int subId, String defaultAccountName) {
        String displayName = SubInfoUtils.getDisplaynameUsingSubId(subId);
        return displayName != null ? displayName : defaultAccountName;
    }

    /**
     * Get the subId based on account name, return -1 if the account is not SIM
     * account.
     *
     * @param context
     *            Context
     * @param accountName
     *            String
     */
    public static int getSubIdBySimAccountName(Context context, String accountName) {
        List<AccountWithDataSet> accounts = AccountTypeManager.getInstance(context).getAccounts(
                true);
        for (AccountWithDataSet account : accounts) {
            if (account instanceof AccountWithDataSetEx && account.name.equals(accountName)) {
                return ((AccountWithDataSetEx) account).getSubId();
            }
        }
        Log.d(TAG, "[getSubIdBySimAccountName]account " + Log.anonymize(accountName)
              + " is not sim account");
        return SubInfoUtils.getInvalidSubId();
    }

    /**
     * CMCC requests 7 edit fields in StructuredPostal.
     *
     * @param kind
     *            the data kind of StructuredPostal.
     * @param postalFlags
     *            BaseAccountType.FLAGS_POSTAL.
     */
    public static void setStructuredPostalFiledList(DataKind kind, int postalFlags) {
        final boolean useJapaneseOrder = Locale.JAPANESE.getLanguage().equals(
                Locale.getDefault().getLanguage());
        if (useJapaneseOrder) {
            kind.fieldList.add(new EditField(StructuredPostal.COUNTRY, R.string.postal_country,
                    postalFlags).setOptional(true));
            kind.fieldList.add(new EditField(StructuredPostal.POSTCODE, R.string.postal_postcode,
                    postalFlags));
            kind.fieldList.add(new EditField(StructuredPostal.REGION, R.string.postal_region,
                    postalFlags));
            kind.fieldList.add(new EditField(StructuredPostal.CITY, R.string.postal_city,
                    postalFlags));
            kind.fieldList.add(new EditField(StructuredPostal.NEIGHBORHOOD,
                    R.string.postal_neighborhood, postalFlags));
            kind.fieldList.add(new EditField(StructuredPostal.POBOX, R.string.postal_pobox,
                    postalFlags));
            kind.fieldList.add(new EditField(StructuredPostal.STREET, R.string.postal_street,
                    postalFlags));
        } else {
            kind.fieldList.add(new EditField(StructuredPostal.STREET, R.string.postal_street,
                    postalFlags));
            kind.fieldList.add(new EditField(StructuredPostal.POBOX, R.string.postal_pobox,
                    postalFlags));
            kind.fieldList.add(new EditField(StructuredPostal.NEIGHBORHOOD,
                    R.string.postal_neighborhood, postalFlags));
            kind.fieldList.add(new EditField(StructuredPostal.CITY, R.string.postal_city,
                    postalFlags));
            kind.fieldList.add(new EditField(StructuredPostal.REGION, R.string.postal_region,
                    postalFlags));
            kind.fieldList.add(new EditField(StructuredPostal.POSTCODE, R.string.postal_postcode,
                    postalFlags));
            kind.fieldList.add(new EditField(StructuredPostal.COUNTRY, R.string.postal_country,
                    postalFlags).setOptional(true));
        }
    }

    public static boolean isAccountTypeSipSupport(Context context) {
        TelephonyManager telephonyManager = new TelephonyManager(context);
        boolean ret = false;
        if (telephonyManager.isVoiceCapable() && SipManager.isVoipSupported(context)) {
            ret = true;
        }
        return ret;
    }

    public static String getAccountTypeBySub(int subId) {
        String simAccountType;
        if (SimCardUtils.isSimInsertedBySub(subId)) {
            simAccountType = getSimAccountTypeBySub(subId);
        } else {
            simAccountType = null;
        }
        return simAccountType;
    }

    private static String getSimAccountTypeBySub(int subId) {
        String simType = null;
        String simAccountType = null;
        simType = SimCardUtils.getSimTypeBySubId(subId);
        if (SimCardUtils.isUsimType(simType)) {
            simAccountType = AccountTypeUtils.ACCOUNT_TYPE_USIM;
        } else if (SimCardUtils.isSimType(simType)) {
            simAccountType = AccountTypeUtils.ACCOUNT_TYPE_SIM;
        } else if (SimCardUtils.isRuimType(simType)) {
            simAccountType = AccountTypeUtils.ACCOUNT_TYPE_RUIM;
        } else if (SimCardUtils.isCsimType(simType)) {
            simAccountType = AccountTypeUtils.ACCOUNT_TYPE_CSIM;
        }
        Log.d(TAG, "[getAccountTypeUsingSubId]subId:" + subId + ",AccountType:"
                + simAccountType);
        return simAccountType;
    }

    /**
     * @param subId
     *            sub id
     * @return the account name for this sub id
     */
    public static String getAccountNameUsingSubId(int subId) {
        String accountName = null;
        String iccCardType = SimCardUtils.getSimTypeBySubId(subId);
        if (iccCardType != null) {
            accountName = iccCardType + subId;
        }
        Log.d(TAG, "[getAccountNameUsingSubId]subId:" + subId + ",iccCardType =" + iccCardType
                + ",accountName:" + Log.anonymize(accountName));

        return accountName;
    }

    public static boolean isUsim(String accountType) {
        return AccountTypeUtils.ACCOUNT_TYPE_USIM.equals(accountType);
    }

    public static boolean isSim(String accountType) {
        return AccountTypeUtils.ACCOUNT_TYPE_SIM.equals(accountType);
    }

    public static boolean isCsim(String accountType) {
        return AccountTypeUtils.ACCOUNT_TYPE_CSIM.equals(accountType);
    }

    public static boolean isRuim(String accountType) {
        return AccountTypeUtils.ACCOUNT_TYPE_RUIM.equals(accountType);
    }

    public static boolean isUsimOrCsim(String accountType) {
        return isUsim(accountType) || isCsim(accountType);
    }

    public static boolean isSimOrRuim(String accountType) {
        return isSim(accountType) || isRuim(accountType);
    }

    public static boolean isPhoneNumType(String mimeType) {
        return Phone.CONTENT_ITEM_TYPE.equals(mimeType);
    }

    public static boolean isAasPhoneType(int type) {
        return (Anr.TYPE_AAS == type);
    }

    // refer: UsimAccountType.addDataKindNickname()
    public static void addDataKindEmail(AccountType accountType) {
        DataKind kind;
        try {
            kind = accountType.addKind(new DataKind(Email.CONTENT_ITEM_TYPE,
                    R.string.emailLabelsGroup, /*Weight.EMAIL*/15, true));
        } catch (Exception e) {
            Log.d(TAG, "[addDataKindEmail] addkind Exception = " + e.getMessage());
            return;
        }
        kind.actionHeader = new EmailActionInflater();
        kind.actionBody = new SimpleInflater(Email.DATA);
        kind.typeColumn = Email.TYPE;
        kind.typeOverallMax = 1;
        kind.typeList = Lists.newArrayList();
        kind.fieldList = Lists.newArrayList();
        kind.fieldList.add(new EditField(Email.DATA, R.string.emailLabelsGroup,
                /*BaseAccountType.FLAGS_EMAIL*/(EditorInfo.TYPE_CLASS_TEXT
                        | EditorInfo.TYPE_TEXT_VARIATION_EMAIL_ADDRESS)));
        Log.d(TAG, "[addDataKindEmail] add email data kind done");
    }

    public static void removeDataKind(AccountType accountType, String mimeType) {
        if (accountType.getKindForMimetype(mimeType) != null) {
            Log.d(TAG, "[removeDataKind] mimeType = " + mimeType + ", remove it");
            try {
                accountType.removeKind(mimeType);
            } catch (Exception de) {
                Log.d(TAG, "[removeDataKind]remove kind Exception");
            }
        }
    }
}
