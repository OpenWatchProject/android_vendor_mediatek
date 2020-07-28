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
package com.mediatek.contacts.list;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.telecom.TelecomManager;

import com.android.contacts.CallUtil;
import com.android.contacts.compat.telecom.TelecomManagerCompat;
import com.android.contacts.list.ContactEntryListAdapter;
import com.android.contacts.list.ContactListFilter;

import com.mediatek.contacts.util.ContactsIntent;
import com.mediatek.contacts.util.Log;

import mediatek.telecom.MtkTelecomManager;

import java.util.ArrayList;

public class ConferenceCallsPickerFragment extends DataKindBasePickerFragment {
    private static final String TAG = "ConferenceCallsPickerFragment";
    public static final String FRAGMENT_ARGS = "intent";

    // for Reference call max number limited
    private int mReferenceCallMaxNumber = ContactsIntent.CONFERENCE_CALL_LIMITES;
    private Intent mIntent;
    private String mCallingActivity;

    @Override
    protected ConferenceCallsPickerAdapter createListAdapter() {
        ConferenceCallsPickerAdapter adapter = new ConferenceCallsPickerAdapter(
                getActivity(), getListView());
        adapter.setFilter(ContactListFilter
                .createFilterWithType(ContactListFilter.FILTER_TYPE_ALL_ACCOUNTS));

        mIntent = getArguments().getParcelable(FRAGMENT_ARGS);
        mReferenceCallMaxNumber = (int) mIntent.getIntExtra(
                ContactsIntent.CONFERENCE_CALL_LIMIT_NUMBER, mReferenceCallMaxNumber);
        Log.i(TAG, "mReferenceCallMaxNumber = " + mReferenceCallMaxNumber);
        adapter.setRefenceCallMaxNumber(mReferenceCallMaxNumber);
        mCallingActivity = (String) mIntent.getStringExtra(ContactsIntent.CONFERENCE_SENDER);
        return adapter;
    }

    /**
     *
     * @return The max count of current multi choice
     */
    protected int getMultiChoiceLimitCount() {
        return mReferenceCallMaxNumber;
    }

    @Override
    public void onOptionAction() {
        final long[] idArray = getCheckedItemIds();
        if (idArray == null || idArray.length <= 0) {
            Log.w(TAG, "[onOptionAction]return,idArray = " + idArray);
            return;
        }

        final Activity activity = getActivity();
        activity.getCallingActivity();
        // For contacts sender
        if (ContactsIntent.CONFERENCE_CONTACTS.equals(mCallingActivity)) {
            ArrayList<String> numbers = ((ConferenceCallsPickerAdapter) getAdapter()).
                    getPhoneNumberByDataIds(idArray);
            //Intent confCallIntent = CallUtil.getCallIntent(numbers.get(0));
            //confCallIntent.putExtra(MtkTelecomManager.EXTRA_START_VOLTE_CONFERENCE, true);
            Intent confCallIntent =
                    MtkTelecomManager.createConferenceInvitationIntent(getActivity());
            confCallIntent.setData(CallUtil.getCallUri(numbers.get(0)));
            confCallIntent.putStringArrayListExtra(
                    MtkTelecomManager.EXTRA_VOLTE_CONFERENCE_NUMBERS, numbers);
            TelecomManagerCompat.placeCall(
                    getActivity(),
                    (TelecomManager)getActivity().getSystemService(Context.TELECOM_SERVICE),
                    confCallIntent);
            Log.sensitive(TAG, "[onOptionAction]placeCall():" + numbers);
        } else {
            // For Dialer sender.
            final Intent retIntent = new Intent();
            retIntent.putExtra(ContactsIntent.CONFERENCE_CALL_RESULT_INTENT_EXTRANAME, idArray);
            activity.setResult(Activity.RESULT_OK, retIntent);
        }
        activity.finish();
    }

    @Override
    public void onSelectedContactsChangedViaCheckBox() {
        super.onSelectedContactsChangedViaCheckBox();
    }
}
