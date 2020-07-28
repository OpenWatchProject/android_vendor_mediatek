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
import android.content.Intent;
import android.widget.Toast;

import com.android.contacts.R;
import com.android.contacts.list.ContactEntryListAdapter;
import com.android.contacts.list.ContactListFilter;

import com.mediatek.contacts.util.Log;

public class PhoneAndEmailsPickerFragment extends DataKindBasePickerFragment {
    private static final String TAG = "PhoneAndEmailsPickerFragment";
    // Message only support no more than 100
    private int mNumberBalance = 100;
    private static final String RESULT_INTENT_EXTRA_NAME =
            "com.mediatek.contacts.list.pickdataresult";

    public void setNumberBalance(int numberBalance) {
        this.mNumberBalance = numberBalance;
    }

    @Override
    protected PhoneAndEmailsPickerAdapter createListAdapter() {
        PhoneAndEmailsPickerAdapter adapter = new PhoneAndEmailsPickerAdapter(
                getActivity(), getListView());
        adapter.setFilter(ContactListFilter
                .createFilterWithType(ContactListFilter.FILTER_TYPE_ALL_ACCOUNTS));
        return adapter;
    }

    @Override
    public void onOptionAction() {
        final long[] idArray = getCheckedItemIds();
        if (idArray == null) {
            Log.w(TAG, "[onOptionAction]idArray = null, return! ");
            return;
        }

        Log.i(TAG, "[onOptionAction]mNumberBalance = " + mNumberBalance + ",idArray.length = "
                + idArray.length);
        if (idArray.length > mNumberBalance) {
            String limitString = getResources().getString(R.string.contact_recent_number_limit,
                    String.valueOf(mNumberBalance));
            Toast.makeText(getActivity().getApplicationContext(), limitString, Toast.LENGTH_SHORT)
                    .show();
            return;
        }

        final Activity activity = getActivity();
        final Intent retIntent = new Intent();
        retIntent.putExtra(RESULT_INTENT_EXTRA_NAME, idArray);
        activity.setResult(Activity.RESULT_OK, retIntent);
        activity.finish();
    }
}
