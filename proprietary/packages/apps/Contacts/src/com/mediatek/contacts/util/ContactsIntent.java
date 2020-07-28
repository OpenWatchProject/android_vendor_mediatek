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
package com.mediatek.contacts.util;

import android.content.Intent;

/**
 * ContactsIntent is a helper class to manage all of Intent not defined in
 * Android.
 */

public final class ContactsIntent {
    private static final String TAG = "ContactsIntent";
    /** [VoLTE ConfCall] @{ */
    public static final String CONFERENCE_CALL_LIMIT_NUMBER = "CONFERENCE_CALL_LIMIT_NUMBER";
    public static final String CONFERENCE_CALL_RESULT_INTENT_EXTRANAME =
            "com.mediatek.contacts.list.pickdataresult";
    public static final int CONFERENCE_CALL_LIMITES = 5;

    public static final String CONFERENCE_SENDER = "CONFERENCE_SENDER";
    public static final String CONFERENCE_CONTACTS = "CONTACTS";

    /** @} */
    /**
     * Check whether intent is defined in ContactsIntent or not
     *
     * @param intent
     * @return true: intent is defined in ContactsIntent; otherwise return
     *         false.
     */
    public static boolean contain(Intent intent) {
        if (null == intent) {
            Log.w(TAG, "[contain]intent is null,donothing!");
            return false;
        }

        String action = intent.getAction();
        Log.d(TAG, "[contain]action is:" + action);
        if (LIST.ACTION_PICK_MULTI_CONTACTS.equals(action)
                || LIST.ACTION_PICK_MULTI_EMAILS.equals(action)
                || LIST.ACTION_PICK_MULTI_PHONES.equals(action)
                || LIST.ACTION_PICK_MULTI_PHONEANDEMAILS.equals(action)
                || LIST.ACTION_PICK_MULTI_DATAS.equals(action)
                /**
                 * TODO:CT NEW FEATURE: MMS add contacts. choose a dropdown list
                 * group item
                 */
                || LIST.ACTION_PICK_MULTI_PHONEANDIMSANDSIPCONTACTS.equals(action)) {
            return true;
        }

        return false;
    }

    /**
     * The action for com.mediatek.contacts.list.
     */
    public static final class LIST {
        public static final String ACTION_PICK_MULTI_CONTACTS =
                "mediatek.intent.action.contacts.list.PICKMULTICONTACTS";
        public static final String ACTION_PICK_MULTI_EMAILS =
                "mediatek.intent.action.contacts.list.PICKMULTIEMAILS";
        public static final String ACTION_PICK_MULTI_PHONES =
                "mediatek.intent.action.contacts.list.PICKMULTIPHONES";
        public static final String ACTION_PICK_MULTI_DATAS =
                "mediatek.intent.action.contacts.list.PICKMULTIDATAS";
        public static final String ACTION_PICK_MULTI_PHONEANDEMAILS =
                "mediatek.intent.action.contacts.list.PICKMULTIPHONEANDEMAILS";
        public static final String ACTION_PICK_MULTI_PHONEANDIMSANDSIPCONTACTS =
                "mediatek.intent.action.contacts.list.PICKMULTIPHONEANDIMSANDSIPCONTACTS";
    }

    /**
     * The action for multiple choice.
     */
    public static final class MULTICHOICE {
        public static final String ACTION_MULTICHOICE_PROCESS_FINISH =
                "com.mediatek.intent.action.contacts.multichoice.process.finish";
    }
}
