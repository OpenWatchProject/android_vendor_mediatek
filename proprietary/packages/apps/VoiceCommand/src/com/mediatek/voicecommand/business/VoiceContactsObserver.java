/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2014. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.voicecommand.business;

import android.content.Context;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.provider.ContactsContract.Contacts;
import android.text.TextUtils;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.util.Log;

import java.util.ArrayList;

/**
 * Receives call backs for contacts changes to content.
 * 
 */
public class VoiceContactsObserver extends ContentObserver {
    private static final String TAG = "VoiceContactsObserver";

    private Context mContext;
    private Handler mMainHandler;
    private HandlerThread mHandlerThread;
    private Handler mVoiceHandler;

    private static final int MSG_GET_CONTACTS_NAME = 1000;

    public static final Uri CONTACTS_URI = Uri
            .parse("content://com.android.contacts/contacts?address_book_index_extras=true&directory=0");

    /**
     * VoiceContactsObserver constructor.
     * 
     * @param context
     *            context
     * @param handler
     *            the handler to run onChange(boolean) on
     */
    public VoiceContactsObserver(Context context, Handler handler) {
        super(handler);
        Log.i(TAG, "[VoiceContactsObserver]new...");
        mContext = context;
        mMainHandler = handler;
        mHandlerThread = new HandlerThread("VoiceHandlerThread");
        mHandlerThread.start();
        Looper looper = mHandlerThread.getLooper();
        if (looper == null){
            Log.e(TAG, "[VoiceContactsObserver]looper is null.");
            return;
        }
        mVoiceHandler = new VoiceHandler(looper);
    }

    @Override
    public void onChange(boolean selfChange, Uri uri) {
        super.onChange(selfChange, uri);
        Log.i(TAG, "[onChange]uri = " + uri);
        if (mVoiceHandler.hasMessages(MSG_GET_CONTACTS_NAME)) {
            mVoiceHandler.removeMessages(MSG_GET_CONTACTS_NAME);
        }
        mVoiceHandler.sendEmptyMessage(MSG_GET_CONTACTS_NAME);
    }

    /**
     * A Handler allows you to send and process voice wake up Message and
     * Runnable objects associated with VoiceHandlerThread's MessageQueue.
     * 
     */
    private class VoiceHandler extends Handler {
        public VoiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            Log.d(TAG, "[handleMessage]msg.what = " + msg.what);
            switch (msg.what) {
            case MSG_GET_CONTACTS_NAME:
                sendToMainHandler();
                break;

            default:
                break;
            }
        }
    }

    /**
     * Method to signal boot complete for Contacts.
     */
    public void bootCompletedReceive() {
        mVoiceHandler.sendEmptyMessage(MSG_GET_CONTACTS_NAME);
    }

    /**
     * Query all contact name from contact table.
     * 
     * @return all contact name
     */
    public String[] getContactsNames() {
        ArrayList<String> contactsList = new ArrayList<String>();
        String columnsInbox[] = new String[] { Contacts.DISPLAY_NAME };
        Cursor cursor = null;
        try {
            cursor = mContext.getContentResolver().query(CONTACTS_URI, columnsInbox, null, null,
                    Contacts.SORT_KEY_PRIMARY);
            if (cursor != null && cursor.moveToFirst()) {
                int count = cursor.getCount();
                Log.d(TAG, "[getContactsNames] cursor count = " + count);
                String contact;
                for (int i = 0; i < count; i++) {
                    int columnIndex = cursor.getColumnIndex(Contacts.DISPLAY_NAME);
                    if (columnIndex >= 0) {
                        contact = cursor.getString(columnIndex);
                        if (!TextUtils.isEmpty(contact)) {
                            contactsList.add(contact);
                        }
                    }
                    cursor.moveToNext();
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return contactsList.toArray(new String[contactsList.size()]);
    }

    private void sendToMainHandler() {
        Log.d(TAG, "[sendToMainHandler]...");
        if (mMainHandler.hasMessages(VoiceCommandListener.ACTION_VOICE_CONTACTS_NAME)) {
            mMainHandler.removeMessages(VoiceCommandListener.ACTION_VOICE_CONTACTS_NAME);
        }
        // Query database after remove the contacts msg of main handler
        String[] contactsNames = getContactsNames();
        VoiceMessage message = new VoiceMessage();
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_CONTACTS_NAME;
        Bundle bundle = DataPackage.packageSendInfo(contactsNames, null);
        message.mExtraData = bundle;

        Message msg = mMainHandler.obtainMessage();
        msg.what = VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS;
        msg.obj = message;
        mMainHandler.sendMessage(msg);
    }
}
