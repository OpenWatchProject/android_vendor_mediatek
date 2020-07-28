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

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract.CommonDataKinds.GroupMembership;
import android.provider.ContactsContract.CommonDataKinds.Photo;
import android.provider.ContactsContract.RawContacts;
//import android.util.Log;
import android.widget.ListPopupWindow;

import com.android.contacts.activities.ContactEditorActivity;
import com.android.contacts.model.Contact;
import com.android.contacts.model.RawContact;
import com.android.contacts.model.RawContactDelta;
import com.android.contacts.model.RawContactDeltaList;
import com.android.contacts.model.account.AccountType;
import com.android.contacts.model.account.AccountWithDataSet;

import com.google.common.collect.ImmutableList;

import com.mediatek.contacts.GlobalEnv;
import com.mediatek.contacts.model.account.AccountWithDataSetEx;
import com.mediatek.contacts.simcontact.SimCardUtils;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.contacts.util.ProgressHandler;
import com.mediatek.provider.MtkContactsContract;

public class SubscriberAccount {
    private static String TAG = "SubscriberAccount";

    private static final String KEY_SUBID = "key_subid";
    private static final String KEY_SAVEMODE_FOR_SIM = "key_savemode_for_sim";
    private static final String KEY_OLDSTATE = "key_oldstate";
    private static final String KEY_INDICATE_PHONE_OR_SIM = "key_indicate_phone_or_sim";
    private static final String KEY_SIM_INDEX = "key_sim_index";

    public static final int MODE_SIM_DEFAULT = 0;
    public static final int MODE_SIM_INSERT = 1;
    public static final int MODE_SIM_EDIT = 2;

    private int mIndicatePhoneOrSimContact = MtkContactsContract.RawContacts.INDICATE_PHONE; //-1
    private int mSaveModeForSim = MODE_SIM_DEFAULT;
    private int mSubId = SubInfoUtils.getInvalidSubId(); //-1
    private RawContactDeltaList mOldState;
    private int mSimIndex = -1;

    ProgressHandler mProgressHandler = new ProgressHandler();

    public SubscriberAccount() {

    }

    /**
     * Change feature: AccountSwitcher. Set the sim info variables of this
     * contact editor.
     *
     * @param account
     *            the corresponding sim account of this contact.
     */
    public void setSimInfo(AccountWithDataSet account) {
        if (!(account instanceof AccountWithDataSetEx)) {
            Log.sensitive(TAG, "[setSimInfo] not sim account, account=" + account);
            return;
        }

        mSubId = ((AccountWithDataSetEx)account).getSubId();
        GlobalEnv.getSimAasEditor().setCurrentSubId(mSubId);
        Log.d(TAG, "[setSimInfo] subId=" + mSubId);
    }

    /**
     * Change feature: AccountSwitcher. Clear the sim info variables.
     */
    public void clearSimInfo() {
        mSubId = -1;
        GlobalEnv.getSimAasEditor().setCurrentSubId(mSubId);
    }

    /**
     * Insert Raw data to sim card.
     * @param rawContacts
     */
    public void insertRawDataToSim(ImmutableList<RawContact> rawContacts) {
        RawContactDeltaList state = RawContactDeltaList.fromIterator(
                rawContacts.iterator());
        if (!isAccountTypeIccCard(state)) {
            // no need to new mOldState for reducing memory cost,
            // especially in onSaveInstanceStateSim(Bundle).
            return;
        }
        Log.d(TAG, "[insertRawDataToSim] keep mOldState for sim contact");
        /*
         * New Feature by Mediatek Begin. Original Android's code: CR
         * ID:ALPS00101852 Descriptions: insert data to SIM/USIM.
         */
        setSimSaveMode(MODE_SIM_EDIT);
        mOldState = state;
        /// M: [Sim Contact Flow][AAS][ALPS03920697] 1/3. @{
        GlobalEnv.getSimAasEditor().setOldAasIndicatorAndNames(mOldState);
        /// @}
        ContactEditorUtilsEx.showLogContactState(mOldState);
    }

    /**
     * Get Sub Id
     * @param contact
     */
    public void initIccCard(Contact contact) {
        mIndicatePhoneOrSimContact = contact.getIndicate();
        mSubId = contact.getIndicate();
        Log.i(TAG, "[initIccCard]mSubId = " + mSubId
                + ", mIndicatePhoneOrSimContact = " + mIndicatePhoneOrSimContact);
    }

    /**
     * Restore Icc info.
     * @param savedState
     */
    public void restoreSimAndSubId(Bundle savedState) {
        Log.d(TAG, "[restoreSimAndSubId]");
        mSubId = savedState.getInt(KEY_SUBID);
        mSaveModeForSim = savedState.getInt(KEY_SAVEMODE_FOR_SIM);
        mIndicatePhoneOrSimContact = savedState.getInt(KEY_INDICATE_PHONE_OR_SIM);
        mOldState = savedState.<RawContactDeltaList> getParcelable(KEY_OLDSTATE);
        mSimIndex = savedState.getInt(KEY_SIM_INDEX);
        Log.i(TAG, "[restoreSimAndSubId] mSubId : " + mSubId
                + " | mSaveModeForSim : " + mSaveModeForSim);
        ContactEditorUtilsEx.showLogContactState(mOldState);
    }

    /**
     * Set save mode and get Icc account type.
     * @param newAccountType
     */
    public void setSimSaveMode(int mode) {
        mSaveModeForSim = mode;
    }

    /**
     * Save sim state to bundle.
     * @param outState
     */
    public void onSaveInstanceStateSim(Bundle outState) {
        outState.putInt(KEY_SUBID, mSubId);
        outState.putInt(KEY_SAVEMODE_FOR_SIM, mSaveModeForSim);
        outState.putInt(KEY_INDICATE_PHONE_OR_SIM, mIndicatePhoneOrSimContact);
        outState.putInt(KEY_SIM_INDEX, mSimIndex);
        if (mOldState != null && mOldState.size() > 0) {
            outState.putParcelable(KEY_OLDSTATE, mOldState);
        }
        Log.sensitive(TAG, "[onSaveInstanceStateSim] mSubId : " + mSubId
                + ", mSaveModeForSim : " + mSaveModeForSim
                + ", mIndicatePhoneOrSimContact : " + mIndicatePhoneOrSimContact
                + ", mSimIndex : " + mSimIndex
                + ", mOldState : " + mOldState);
    }

    /**
     * Get subId and Sim type etc.
     * @param data
     */
    public void setAccountChangedSim(Intent data, Context context) {
        ContactEditorActivity activity = (ContactEditorActivity) context;
        /**
         * M:ALPS00403629 to show the soft inputmethod after
         * ContactEditorAccountsChangedActivity @{
         */
        ContactEditorUtilsEx.setInputMethodVisible(true, activity);
        /** @}*/
        /*
         * New Feature by Mediatek Begin. Original Android's code: CR ID:
         * ALPS00101852 Descriptions: create sim/usim contact
         */
        mSubId = data.getIntExtra("mSubId", -1);
        Log.i(TAG, "[setAccountChangedSim]msubId: " + mSubId);
        /*
         * @}.
         */
    }

    /**
     * Set sim info to intent.
     * @param intent
     * @param lookupUri
     */
    public void processSaveToSim(Intent intent, Uri lookupUri) {
        Log.i(TAG, "[processSaveToSim]mSaveModeForSim = " + mSaveModeForSim
                + ", mIndicatePhoneOrSimContact = " + mIndicatePhoneOrSimContact
                + ", mSimIndex = " + mSimIndex);
        if (mSaveModeForSim == MODE_SIM_INSERT) {
            // use sub id as INDICATE_PHONE_SIM for inserting a sim contact
            intent.putExtra(MtkContactsContract.RawContactsColumns.INDICATE_PHONE_SIM, mSubId);
        } else if (mSaveModeForSim == MODE_SIM_EDIT) {
            intent.putExtra(MtkContactsContract.RawContactsColumns.INDICATE_PHONE_SIM,
                    mIndicatePhoneOrSimContact);
            intent.putExtra("simIndex", mSimIndex);
        }

        intent.putExtra("simSaveMode", mSaveModeForSim);
        intent.setData(lookupUri);
    }

    /**
     * AccountSwitcher
     * @param currentState
     * @param newAccount
     */
    public boolean setAccountSimInfo(final RawContactDelta currentState,
            AccountWithDataSet newAccount, Context context) {
        ContactEditorActivity activity = (ContactEditorActivity) context;
        if (newAccount instanceof AccountWithDataSetEx) {
            if (!SimCardUtils.checkPHBStateAndSimStorage(activity,
                    ((AccountWithDataSetEx) newAccount).getSubId())) {
                return true;
            }
            // Remove photo saved in currentState when switch account to SIM
            // type.
            if (currentState != null && currentState.hasMimeEntries(Photo.CONTENT_ITEM_TYPE)) {
                currentState.removeEntry(Photo.CONTENT_ITEM_TYPE);
                Log.d(TAG,
                        "[setAccountSimInfo]remove photo in currentState as switch to sim account");
            }
            setSimInfo((AccountWithDataSetEx) newAccount);
        } else {
            clearSimInfo();
        }
        // Need to clear group member ship when switch account.
        if (currentState != null && currentState.
                hasMimeEntries(GroupMembership.CONTENT_ITEM_TYPE)) {
            currentState.removeEntry(GroupMembership.CONTENT_ITEM_TYPE);
        }

        return false;
    }

    public void setAndCheckSimInfo(Context context, AccountWithDataSet account) {
        if (!(account instanceof AccountWithDataSetEx)) {
            return;
        }
        setSimInfo(account);
        /* Fix [ALPS03435495], UX suggestion:
         * For better UE, should toast "Storage is full" to user
         * firstly if needed before enter Editor. @{ */
        SimCardUtils.ShowSimCardStorageInfoTask.showSimCardStorageInfo(context, true,
                ((AccountWithDataSetEx) account).getSubId());
        /* @} */
    }

    /**
     * Check Icc account card type.
     * @param state
     * @return true or false.
     */
    public boolean isAccountTypeIccCard(RawContactDeltaList state) {
        if (!state.isEmpty()) {
            String accountType = state.get(0).getValues().getAsString(RawContacts.ACCOUNT_TYPE);
            ContactEditorUtilsEx.showLogContactState(state);
            if (AccountTypeUtils.isAccountTypeIccCard(accountType)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Get member values.
     * @return values.
     */
    public int getIndicatePhoneOrSimContact() {
        return mIndicatePhoneOrSimContact;
    }

    /**
     * set member values.
     * @param indicatePhoneOrSimContact
     */
    public void setIndicatePhoneOrSimContact(int indicatePhoneOrSimContact) {
        mIndicatePhoneOrSimContact = indicatePhoneOrSimContact;
    }

    /**
     * Get member values.
     * @return values.
     */
    public int getSubId() {
        return mSubId;
    }

    /**
     * Get member values.
     * @return values
     */
    public RawContactDeltaList getOldState() {
        return mOldState;
    }

    /**
     * Get member values.
     * @return values
     */
    public int getSimIndex() {
        return mSimIndex;
    }

    /**
     * Set member values.
     * @param simIndex
     *            :
     */
    public void setSimIndex(int simIndex) {
        mSimIndex = simIndex;
    }

    /**
     * Get progress handler.
     * @return values
     */
    public ProgressHandler getProgressHandler() {
        return mProgressHandler;
    }

}