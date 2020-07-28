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
package com.mediatek.contacts.editor;

import android.accounts.Account;
import android.app.Activity;
import android.content.Context;
import android.content.ContentValues;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts.Data;
import android.provider.ContactsContract.RawContacts;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.widget.Toast;

import com.android.contacts.GroupMetaDataLoader;
import com.android.contacts.R;
import com.android.contacts.activities.ContactEditorActivity;
import com.android.contacts.model.AccountTypeManager;
import com.android.contacts.model.RawContactDelta;
import com.android.contacts.model.RawContactDeltaList;
import com.android.contacts.model.RawContactModifier;
import com.android.contacts.model.ValuesDelta;
import com.android.contacts.model.account.AccountType;
import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.model.account.BaseAccountType;
import com.android.contacts.model.dataitem.DataItem;
import com.android.contacts.model.dataitem.DataKind;
import com.android.contacts.model.dataitem.StructuredNameDataItem;
import com.android.contacts.preference.ContactsPreferences;
import com.android.contacts.editor.LabeledEditorView;
import com.android.contacts.editor.TextFieldsEditorView;

import com.mediatek.contacts.GlobalEnv;
import com.mediatek.contacts.aassne.SimAasSneUtils;
import com.mediatek.contacts.model.account.AccountWithDataSetEx;
import com.mediatek.contacts.simcontact.PhbInfoUtils;
import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.Log;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ContactEditorUtilsEx {
    private static String TAG = "ContactEditorUtilsEx";

    /**
     * Bug fix for ALPS00477285.
     * Restore the StructuredNameDataItem in {@link #onRestoreInstanceState(Parcelable)}.
     * If the dataItem is not an instanceof StructuredNameDataItem.
     * It will initial the item in {@link #setValues(DataKind kind, ValuesDelta entry,
     * RawContactDelta state, boolean readOnly, ViewIdGenerator vig)}
     * @param ss The SavedState the passed from
     *            {@link #onRestoreInstanceState(Parcelable)}
     * @return snapshot StructuredNameDataItem
     */
    public static StructuredNameDataItem restoreStructuredNameDataItem(ContentValues values) {
        DataItem dataItem = DataItem.createFrom(values);

        if (dataItem instanceof StructuredNameDataItem) {
            return ((StructuredNameDataItem) dataItem);
        } else {
            Log.w(TAG, "[restoreStructuredNameDataItem] The dataItem is not an instance " +
                    "of StructuredNameDataItem!!!" + " mimeType: " +
                    values.getAsString(Data.MIMETYPE));
            return null;
        }
    }

    /**
     * M: Change feature: AccountSwitcher.
     * This method is the extension of google method getOnlyOrDefaultAccount() after added
     * the sim/usim account.
     * If the default account is sim account, covert it to AccountWithDataSetEx.
     * Additionally, if the default account is not existed,
     * take the local phone account as the default account.
     * @param prefs SharedPreferences
     * @param currentWritableAccounts List<AccountWithDataSet>
     * @param keyDefaultAccount String
     * @param keyKnownAccounts String
     * @return AccountWithDataSet the default account.
     */
    public static AccountWithDataSet getOnlyOrDefaultAccountEx(ContactsPreferences contactsPrefs,
            List<AccountWithDataSet> currentWritableAccounts) {
        if (currentWritableAccounts.size() == 1) {
            return currentWritableAccounts.get(0);
        }
        boolean defaultAccountNotExist = true;
        //defaultAccount's type is always AccountWithDataSet which is
        //from contactsPrefs.getDefaultAccount()
        AccountWithDataSet defaultAccount = contactsPrefs.getDefaultAccount();
        for (AccountWithDataSet account : currentWritableAccounts) {
            if (account.equals(defaultAccount)) {
                //if account is sim account, here will covert
                //defaultAccount's type to AccountWithDataSetEx
                defaultAccount = account;
                defaultAccountNotExist = false;
                break;
            }
        }
        if (defaultAccountNotExist) {
            Log.d(TAG, "[getDefaultAccountEx] Default account is not exist, " +
                    "reset it to local phone account");
            for (AccountWithDataSet account : currentWritableAccounts) {
                if (AccountTypeUtils.ACCOUNT_TYPE_LOCAL_PHONE.equals(account.type)) {
                    defaultAccount = account;
                    contactsPrefs.setDefaultAccount(defaultAccount);
                    break;
                }
            }
        }
        return defaultAccount;
    }

    /**
     * If the passed account type is icc card account type, return true, else false.
     * @param accountType String
     * @return If icc card account type, return true, else false.
     */
    public static boolean isIccCardAccountType(String accountType) {
        if (AccountTypeUtils.isAccountTypeIccCard(accountType)) {
            return true;
        }
        return false;
    }

    /**
     * If subId is invalid, finish the passed activity.
     * @param context Context
     * @param subId Int
     * @return true if subId invalid.
     */
    public static boolean finishActivityIfInvalidSubId(Context context, int subId) {
        if (subId < 1) {
            Log.w(TAG, "[finishActivityIfInvalidSubId] mSubId < 1, may be the sim card " +
                    "has been plugin out!");
            Toast.makeText(context.getApplicationContext(), R.string.icc_phone_book_invalid,
                    Toast.LENGTH_SHORT).show();
            Activity activity = (Activity) context;
            activity.finish();
            return true;
        }
        return false;
    }

    /**
     * Save group metadata to intent.
     * @param state:
     * @param intent:
     * @param mGroupMetaData:
     */
    public static void processGroupMetadataToSim(RawContactDeltaList state,
            Intent intent, Cursor groupMetaData) {
        int i = 0;
        if (groupMetaData != null) {
            int groupNum = groupMetaData.getCount();
            String accountType = state.get(0).getValues().getAsString(RawContacts.ACCOUNT_TYPE);
            if ((accountType.equals(AccountTypeUtils.ACCOUNT_TYPE_USIM)
                    || AccountTypeUtils.ACCOUNT_TYPE_CSIM.equals(accountType))
                    && groupNum > 0) {
                String groupName[] = new String[groupNum];
                long groupId[] = new long[groupNum];
                groupMetaData.moveToPosition(-1);
                while (groupMetaData.moveToNext()) {
                    Log.sensitive(TAG, "[processGroupMetadataToSim] ACCOUNT_NAME: "
                            + groupMetaData.getString(GroupMetaDataLoader.ACCOUNT_NAME)
                            + ",DATA_SET: "
                            + groupMetaData.getString(GroupMetaDataLoader.DATA_SET)
                            + ",GROUP_ID: "
                            + groupMetaData.getLong(GroupMetaDataLoader.GROUP_ID)
                            + ", TITLE: "
                            + groupMetaData.getString(GroupMetaDataLoader.TITLE));
                    groupName[i] = groupMetaData.getString(GroupMetaDataLoader.TITLE);
                    groupId[i] = groupMetaData.getLong(GroupMetaDataLoader.GROUP_ID);
                    i++;
                    Log.d(TAG, "[processGroupMetadataToSim] I : " + i);
                }
                intent.putExtra("groupName", groupName);
                intent.putExtra("groupNum", groupNum);
                intent.putExtra("groupId", groupId);
                Log.d(TAG, "[processGroupMetadataToSim] groupNum : "
                        + Log.anonymize(groupNum));

            }
        }
    }

    /**
     * Show or hide input method
     * @param visible:
     * @param context:
     */
    public static void setInputMethodVisible(boolean visible, Context context) {
        ContactEditorActivity activity = (ContactEditorActivity) context;
        int mode = visible ? WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE
                : WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN;
        mode = mode | WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE;
        activity.getWindow().setSoftInputMode(mode);
    }


    /**
     * Update the RawContactId of photo in mUpdatedPhotos.
     * @param oldState:
     * @param newState:
     * @param updatedPhotos:
     */
    public static void updatePhotoState(RawContactDelta oldState,
            RawContactDelta newState, Bundle updatedPhotos) {
        Log.d(TAG, "[updatePhotoState] mUpdatedPhotos: " +  updatedPhotos);
        if (updatedPhotos == null || oldState == null || newState == null) {
            return;
        }

        String key = String.valueOf(oldState.getRawContactId());
        if (updatedPhotos.containsKey(key)) {
            Uri photoUri = updatedPhotos.getParcelable(key);
            updatedPhotos.remove(key);
            updatedPhotos.putParcelable(String.valueOf(newState.getRawContactId()), photoUri);
            Log.d(TAG, "[updatePhotoState] Update the RawContactId of photoUri contained in " +
                    "mUpdatedPhotos, from old RawContactId " + oldState.getRawContactId()
                    + " to new RawContactId " + newState.getRawContactId());
        }
    }

    /**
     * Update aas view.
     * @param context
     * @param state:
     * @param content:
     */
    public static void updateAasView(Context context, RawContactDeltaList state,
            LinearLayout content) {
        int numRawContacts = state.size();
        for (int i = 0; i < numRawContacts; i++) {
            final RawContactDelta rawContactDelta = state.get(i);
            //update AAS label instead of OPAasExtension.VIEW_UPDATE_LABEL
            String accountType = rawContactDelta.getAccountType();
            if (AccountTypeUtils.isUsimOrCsim(accountType)) {
                final ValuesDelta values = rawContactDelta.getValues();
                if (!values.isVisible()) {
                    continue;
                }
                final String dataSet = values.getAsString(RawContacts.DATA_SET);
                final AccountType accType = AccountTypeManager.getInstance(context).getAccountType(
                            accountType, dataSet);
                DataKind dataKind = accType.getKindForMimetype(Phone.CONTENT_ITEM_TYPE);
                int subId = SimAasSneUtils.getCurSubId();
                GlobalEnv.getSimAasEditor().updatePhoneType(subId, dataKind);
                updateEditorViewsLabel((ViewGroup) content);
            }
        }
    }

    private static void updateEditorViewsLabel(ViewGroup viewGroup) {
        int count = viewGroup.getChildCount();
        for (int i = 0; i < count; i++) {
            View v = viewGroup.getChildAt(i);
            if (v instanceof TextFieldsEditorView) {
                ((LabeledEditorView) v).updateValues();
            } else if (v instanceof ViewGroup) {
                updateEditorViewsLabel((ViewGroup) v);
            }
        }
    }

    /**
     * Set sim card data kind max count first for CR ALPS01447420
     * @param newAccountType:
     * @param subId:
     */
    public static void setSimDataKindCountMax(final AccountType newAccountType, int subId) {
        for (DataKind kind : newAccountType.getSortedDataKinds()) {
            if ((AccountTypeUtils.ACCOUNT_TYPE_USIM.equals(newAccountType.accountType)
                    || AccountTypeUtils.ACCOUNT_TYPE_CSIM.equals(newAccountType.accountType))
                    && kind.mimeType.equals(Phone.CONTENT_ITEM_TYPE) && kind.typeOverallMax <= 0) {
                kind.typeOverallMax = PhbInfoUtils.getUsimAnrCount(subId) + 1;
                Log.d(TAG, "[setSimDataKindCountMax] Usim max number = ANR + 1 = " +
                        kind.typeOverallMax);
            }
        }
    }
    /**
     * Clear all children foucs.
     * @param content:
     */
    public static void clearChildFoucs(LinearLayout content) {
        Log.d(TAG, " save reload and mLookupUri is null");
        if (content != null) {
            int count = content.getChildCount();
            for (int i = 0; i < count; i++) {
                content.getChildAt(i).clearFocus();
            }
        }
    }

    /**
     * show Raw contact deltalist.
     * @param state:
     */
    public static void showLogContactState(RawContactDeltaList state) {
        if (state != null) {
            Log.i(TAG, "[showLogContactState] state size = " + state.size());
        }
    }

    /**
     * Ensure the DataKind of sim contact support or not.
     * If support, ensure the DataKind and its data ready.
     * If not support, remove the DataKind.
     */
    public static void ensureDataKindsForSim(RawContactDeltaList state,
            int subId, Context context) {
        // ensure [AAS][SNE] kind updated and exists
        GlobalEnv.getSimAasEditor().ensurePhoneKindForCompactEditor(state,
                subId, context);
        GlobalEnv.getSimSneEditor().onEditorBindForCompactEditor(state,
                subId, context);

        // ALPS00566570, some USIM card not support email
        ensureEmailKindForSim(state, subId, context);
    }

    public static void updateDataKindsForSim(AccountType type, int subId) {
        GlobalEnv.getSimAasEditor().updatePhoneKind(type, subId);
        GlobalEnv.getSimSneEditor().updateNickNameKind(type, subId);
        updateEmailKind(type, subId);
    }

    /**
     * Ensure email DataKind for some USIM card not support to
     * store Email address.
     */
    private static void ensureEmailKindForSim(RawContactDeltaList state,
            int subId, Context context) {
        int numRawContacts = state.size();
        final AccountTypeManager accountTypes = AccountTypeManager.getInstance(context);
        for (int i = 0; i < numRawContacts; i++) {
            final RawContactDelta rawContactDelta = state.get(i);
            final AccountType type = rawContactDelta.getAccountType(accountTypes);
            Log.d(TAG, "[ensureEmailKindForSim] loop " + i + " subid=" + subId);
            if (type != null && AccountTypeUtils.isUsimOrCsim(type.accountType)) {
                int emailCount = PhbInfoUtils.getUsimEmailCount(subId);
                if (emailCount > 0) {
                    AccountTypeUtils.addDataKindEmail(type);
                    Log.d(TAG, "[ensureEmailKindForSim] ensure email kind exists");
                    RawContactModifier.ensureKindExists(rawContactDelta, type,
                            Email.CONTENT_ITEM_TYPE);
                } else {
                    AccountTypeUtils.removeDataKind(type, Email.CONTENT_ITEM_TYPE);
                }
            }
        }
    }

    /**
     * update the email DataKind for sim AccountType.
     */
    private static void updateEmailKind(AccountType type, int subId) {
        if (type != null && AccountTypeUtils.isUsimOrCsim(type.accountType)) {
            int emailCount = PhbInfoUtils.getUsimEmailCount(subId);
            if (emailCount > 0) {
                AccountTypeUtils.addDataKindEmail(type);
            } else {
                AccountTypeUtils.removeDataKind(type, Email.CONTENT_ITEM_TYPE);
            }
        }
    }

    public static AccountWithDataSet getAccountWithDataSet(Context context,
            Account account, String dataSet, Bundle intentExtras) {
        if (account != null) {
            if (AccountTypeUtils.isAccountTypeIccCard(account.type)) {
                return new AccountWithDataSetEx(account.name, account.type, dataSet,
                        AccountTypeUtils.getSubIdBySimAccountName(context, account.name));
            } else {
                return new AccountWithDataSet(account.name, account.type, dataSet);
            }
        } else {
            return intentExtras.<AccountWithDataSet>getParcelable(
                    ContactEditorActivity.EXTRA_ACCOUNT_WITH_DATA_SET);
        }
    }
}
