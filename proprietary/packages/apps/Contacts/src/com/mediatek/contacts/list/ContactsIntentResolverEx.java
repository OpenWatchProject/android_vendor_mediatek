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
import android.provider.Contacts.Intents.UI;

import com.android.contacts.list.ContactsIntentResolver;
import com.android.contacts.list.ContactsRequest;

import com.mediatek.contacts.util.ContactsIntent;
import com.mediatek.contacts.util.Log;

/**
 * A sub class to extend the ContactsIntentResolver parses
 * {@link com.mediatek.contacts.util.ContactsIntent} not defined in Android
 * source code.
 * <p>
 * ContactsIntentResolver parses a Contacts intent, extracting all relevant
 * parts and packaging them as a
 * {@link com.android.contacts.list.ContactsRequest} object.
 * </p>
 */

public class ContactsIntentResolverEx extends ContactsIntentResolver {
    private static final String TAG = "ContactsIntentResolverEx";

    public static final int REQ_TYPE_IMPORT_EXPORT_PICKER = 1;

    private static final int REQ_TYPE_VCARD_PICKER = 3;

    /** Mask for picking multiple contacts of packing vCard */
    public static final int MODE_MASK_VCARD_PICKER = 0x01000000;

    /** Mask for picking multiple contacts of import/export */
    public static final int MODE_MASK_IMPORT_EXPORT_PICKER = 0x02000000;

    public ContactsIntentResolverEx(Activity context) {
        super(context);
    }

    @Override
    public ContactsRequest resolveIntent(Intent intent) {
        Log.i(TAG, "[resolveIntent]intent: " + intent);
        if (ContactsIntent.contain(intent)) {
            String action = intent.getAction();
            Log.i(TAG, "[resolveIntent]Called with action: " + action);
            ContactsRequest request = new ContactsRequest();
            if (ContactsIntent.LIST.ACTION_PICK_MULTI_CONTACTS.equals(action)) {
                request.setActionCode(ContactsRequestAction.ACTION_PICK_MULTIPLE_CONTACTS);
                int requestType = intent.getIntExtra("request_type", 0);
                Log.i(TAG, "[resolveIntent]requestType: " + requestType);
                switch (requestType) {
                case REQ_TYPE_VCARD_PICKER:
                    request.setActionCode(ContactsRequestAction.ACTION_PICK_MULTIPLE_CONTACTS
                            | MODE_MASK_VCARD_PICKER);
                    break;

                case REQ_TYPE_IMPORT_EXPORT_PICKER:
                    request.setActionCode(ContactsRequestAction.ACTION_PICK_MULTIPLE_CONTACTS
                            | MODE_MASK_IMPORT_EXPORT_PICKER);
                    break;

                default:
                    break;
                }
            } else if (ContactsIntent.LIST.ACTION_PICK_MULTI_EMAILS.equals(action)) {
                request.setActionCode(ContactsRequestAction.ACTION_PICK_MULTIPLE_EMAILS);
            } else if (ContactsIntent.LIST.ACTION_PICK_MULTI_PHONES.equals(action)) {
                request.setActionCode(ContactsRequestAction.ACTION_PICK_MULTIPLE_PHONES);
            } else if (ContactsIntent.LIST.ACTION_PICK_MULTI_PHONEANDEMAILS.equals(action)) {
                request.setActionCode(ContactsRequestAction.ACTION_PICK_MULTIPLE_PHONEANDEMAILS);
            } else if (ContactsIntent.LIST.ACTION_PICK_MULTI_DATAS.equals(action)) {
                request.setActionCode(ContactsRequestAction.ACTION_PICK_MULTIPLE_DATAS);
            } else if (ContactsIntent.LIST.ACTION_PICK_MULTI_PHONEANDIMSANDSIPCONTACTS
                    .equals(action)) {
                request.setActionCode(
                        ContactsRequestAction.ACTION_PICK_MULTIPLE_PHONE_IMS_SIP_CALLS);
            }

            // Allow the title to be set to a custom String using an extra on
            // the intent
            String title = intent.getStringExtra(UI.TITLE_EXTRA_KEY);
            if (title != null) {
                request.setActivityTitle(title);
            }
            return request;
        }

        return super.resolveIntent(intent);
    }
}
