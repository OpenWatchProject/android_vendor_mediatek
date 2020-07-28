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
package com.mediatek.contacts;

import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import com.android.contacts.ContactSaveService;
import com.android.contacts.ContactsApplication;
import com.android.contacts.activities.RequestPermissionsActivity;
import com.android.contacts.vcard.VCardService;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.common.carrierexpress.CarrierExpressManager;
import com.mediatek.contacts.eventhandler.GeneralEventHandler.PhbState;
import com.mediatek.contacts.list.service.MultiChoiceService;
import com.mediatek.contacts.simcontact.PhbInfoUtils;
import com.mediatek.contacts.util.ContactsConstants;
import com.mediatek.contacts.util.Log;

/**
 * An extension of ContactsApplication, init some environment variable and there
 * is two static util method.
 */
public class ContactsApplicationEx {
    private static String TAG = "ContactsApplicationEx";

    private static ContactsApplication sContactsApplication = null;

    /**
     * Extension for ContactsApplication.onCreate().
     *
     * @param contactsApplication
     *            ContactsApplication.
     */
    public static void onCreateEx(ContactsApplication contactsApplication) {
        sContactsApplication = contactsApplication;
        ExtensionManager.registerApplicationContext(contactsApplication);
        // retrieve the application context for ContactsCommon
        GlobalEnv.setApplicationContext(contactsApplication);
        // [Sim Contact Flow][AAS][SNE]
        GlobalEnv.setSimAasEditor();
        GlobalEnv.setSimSneEditor();
        // fix ALPS00286964:Remove all contacts notifications
        NotificationManager notificationManager = (NotificationManager) contactsApplication
                .getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.cancelAll();

        PhbInfoUtils.clearActiveUsimPhbInfoMap();
        IntentFilter intentFilter = new IntentFilter(TelephonyIntents.ACTION_PHB_STATE_CHANGED);
        // Support OP switched in runtime. @{
        intentFilter.addAction(CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED);
        // @}
        contactsApplication.registerReceiver(sAppExReceiver, intentFilter);
    }

    /**
     * When Contacts app is busy, some operations should be forbidden.
     *
     * @return true if there are time consuming operation running, such as:
     *         Multi-delete, import/export, SIMService running, group batch
     *         operation
     */
    public static boolean isContactsApplicationBusy() {
        boolean isMultiDeleting = MultiChoiceService.isProcessing(MultiChoiceService.TYPE_DELETE);
        boolean isMultiCopying = MultiChoiceService.isProcessing(MultiChoiceService.TYPE_COPY);
        boolean isVcardProcessing = VCardService.isProcessing(VCardService.TYPE_IMPORT);
        boolean isGroupSavingInTransaction = ContactSaveService.isGroupTransactionProcessing();
        Log.i(TAG, "[isContactsApplicationBusy] multi-del: " + isMultiDeleting
                + ", multi-copy: " + isMultiCopying + ", vcard: " + isVcardProcessing
                + ",group-trans: " + isGroupSavingInTransaction);
        return (isMultiDeleting || isMultiCopying || isVcardProcessing
                || isGroupSavingInTransaction);
    }

    /**
     * Get ContactsApplication instance.
     *
     * @return ContactsApplication
     */
    public static ContactsApplication getContactsApplication() {
        return sContactsApplication;
    }

    private static BroadcastReceiver sAppExReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "[onReceive] Received Intent:" + intent);
            String action = intent.getAction();
            if (action.equals(TelephonyIntents.ACTION_PHB_STATE_CHANGED)) {
                boolean isPhbReady = intent.getBooleanExtra(PhbState.PHBREADY, false);
                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        ContactsConstants.ERROR_SUB_ID);
                if (RequestPermissionsActivity.hasBasicPermissions(context)) {
                    PhbInfoUtils.refreshActiveUsimPhbInfoMap(isPhbReady, subId);
                }
            } else if (action.equals(CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED)) {
                ExtensionManager.resetExtensions();
            }
        }
    };
}
