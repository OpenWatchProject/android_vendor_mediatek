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
package com.mediatek.contacts.activities;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.telephony.PhoneNumberUtils;
import android.widget.Toast;

import com.android.contacts.R;
import com.android.contacts.activities.PeopleActivity;
import com.android.contacts.list.ContactEntryListFragment;
import com.android.contacts.model.AccountTypeManager;
import com.android.contacts.vcard.VCardCommonArguments;
import com.android.contacts.list.ContactPickerFragment;

import com.mediatek.contacts.list.service.MultiChoiceService;
import com.mediatek.contacts.simservice.SimEditProcessor;
import com.mediatek.contacts.util.ContactsSettingsUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.contacts.util.MtkToast;

public class ActivitiesUtils {

    private static final String TAG = "ActivitiesUtils";

    public static Handler initHandler(final Activity activity) {
        SimEditProcessor.Listener l = (SimEditProcessor.Listener) activity;
        Handler handler = new Handler() {
            public void handleMessage(Message msg) {
                String content = null;
                int contentId = msg.arg1;
                Bundle bundle = msg.getData();
                if (bundle != null) {
                    content = bundle.getString("content");
                }
                onShowToast(activity, content, contentId);
            }
        };
        SimEditProcessor.registerListener(l, handler);

        return handler;
    }

    /** Add for SIM Service refactory */
    public static void onShowToast(Activity activity, String msg, int resId) {
        Log.d(TAG, "[onShowToast]msg: " + msg + " ,resId: " + resId);
        if (msg != null) {
            Toast.makeText(activity, msg, Toast.LENGTH_SHORT).show();
        } else if (resId != -1) {
            Toast.makeText(activity, resId, Toast.LENGTH_SHORT).show();
        }
    }

    public static void setPickerFragmentAccountType(Activity activity,
            ContactEntryListFragment<?> listFragment) {
        if (listFragment instanceof ContactPickerFragment) {
            ContactPickerFragment fragment = (ContactPickerFragment) listFragment;
            int accountTypeShow = activity.getIntent().getIntExtra(
                    ContactsSettingsUtils.ACCOUNT_TYPE, ContactsSettingsUtils.ALL_TYPE_ACCOUNT);
            Log.d(TAG, "[setPickerFragmentAccountType]accountTypeShow:" + accountTypeShow);
            fragment.setAccountType(accountTypeShow);
        }
    }

    /** New Feature */
    public static boolean checkSimNumberValid(Activity activity, String ssp) {
        if (ssp != null && !PhoneNumberUtils.isGlobalPhoneNumber(ssp)) {
            Toast.makeText(activity.getApplicationContext(), R.string.sim_invalid_number,
                    Toast.LENGTH_SHORT).show();
            activity.finish();
            return true;
        }
        return false;
    }

    public static boolean doImport(Context context) {
        Log.i(TAG, "[doImport]...");

        if (MultiChoiceService.isProcessing(MultiChoiceService.TYPE_DELETE)) {
            Toast.makeText(context, R.string.contact_delete_all_tips, Toast.LENGTH_SHORT).show();
            return true;
        }

        final Intent intent = new Intent(context, ContactImportExportActivity.class);
        /* add callingActivity extra for enter ContactImportExportActivity from PeopleActivity,
         * cause by Dialer can hung up ContactImportExportActivity so we should distinguish which
         * Activity start ContactImportExportActivity by using callingActivity.@{
         */
        intent.putExtra(VCardCommonArguments.ARG_CALLING_ACTIVITY,
                    PeopleActivity.class.getName());
        intent.putExtra(VCardCommonArguments.ARG_CALLING_TYPE, "Import");
        //@}
        context.startActivity(intent);
        return true;
    }

    public static boolean doExport(Context context) {
        Log.i(TAG, "[doImportExport]...");

        if (MultiChoiceService.isProcessing(MultiChoiceService.TYPE_DELETE)) {
            Toast.makeText(context, R.string.contact_delete_all_tips, Toast.LENGTH_SHORT).show();
            return true;
        }

        final Intent intent = new Intent(context, ContactImportExportActivity.class);
        /* add callingActivity extra for enter ContactImportExportActivity from PeopleActivity,
         * cause by Dialer can hung up ContactImportExportActivity so we should distinguish which
         * Activity start ContactImportExportActivity by using callingActivity.@{
         */
        intent.putExtra(VCardCommonArguments.ARG_CALLING_ACTIVITY,
                    PeopleActivity.class.getName());
        intent.putExtra(VCardCommonArguments.ARG_CALLING_TYPE, "Export");
        //@}
        context.startActivity(intent);
        return true;
    }

    public static boolean conferenceCall(Activity activity) {
        Log.i(TAG, "[conferenceCall]...");
        final Intent intent = new Intent();
        intent.setClassName(activity, "com.mediatek.contacts.list.ContactListMultiChoiceActivity")
                .setAction(
                        com.mediatek.contacts.util.
                        ContactsIntent.LIST.ACTION_PICK_MULTI_PHONEANDIMSANDSIPCONTACTS);
        intent.putExtra(com.mediatek.contacts.util.ContactsIntent.CONFERENCE_SENDER,
                com.mediatek.contacts.util.ContactsIntent.CONFERENCE_CONTACTS);
        activity.startActivity(intent);

        return true;
    }

    private static int getAvailableStorageCount(Activity activity) {
        int storageCount = 0;
        final StorageManager storageManager = (StorageManager) activity.getApplicationContext()
                .getSystemService(activity.STORAGE_SERVICE);
        if (null == storageManager) {
            Log.w(TAG, "[getAvailableStorageCount]storageManager is null,return 0.");
            return 0;
        }
        StorageVolume[] volumes = storageManager.getVolumeList();
        for (StorageVolume volume : volumes) {
            String path = volume.getPath();
            if (!Environment.MEDIA_MOUNTED.equals(storageManager.getVolumeState(path))) {
                    continue;
            }
            storageCount++;
        }
        Log.d(TAG, "[getAvailableStorageCount]storageCount = " + storageCount);
        return storageCount;
    }

    public static boolean showImportExportMenu(Activity activity) {
        int availableStorageCount = getAvailableStorageCount(activity);
        int accountSize = AccountTypeManager
                .getInstance(activity).getAccounts(false).size();
        Log.d(TAG, "[showImportExportMenu]availableStorageCount = " + availableStorageCount
                + ",accountSize = " + accountSize);
        return !((availableStorageCount == 0) && (accountSize <= 1));
    }
}
