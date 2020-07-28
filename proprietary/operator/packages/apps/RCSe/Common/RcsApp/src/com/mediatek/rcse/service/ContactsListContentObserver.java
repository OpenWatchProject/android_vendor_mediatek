/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.service;

import android.content.Context;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;

/**
 * An asynchronous update interface for receiving notifications
 * about ContactsListContent information as the ContactsListContent is constructed.
 */
public class ContactsListContentObserver extends ContentObserver {
    // public Context Contexto=null;

    private static final String TAG = "ContactsListContentObserver";

    private Context mContext = null;
    private Looper mServiceLooper = null;
    private ServiceHandler mServiceHandler = null;

    private static final int CONTACT_TABLE_MODIFIED = 0;
    private static final int SYNC_COMPLETE_DELAY_TIMER = 3 * 1000; // 3 seconds.


    /**
     * This method is called when information about an ContactsListContent
     * which was previously requested using an asynchronous
     * interface becomes available.
     *
     * @param handler the handler
     * @param context the context
     */
    public ContactsListContentObserver(Handler handler, Context context) {
        super(handler);
        mContext = context;

        HandlerThread thread = new HandlerThread("ContactsListContentObserverHandler");
        thread.start();

        mServiceLooper = thread.getLooper();
        if (mServiceLooper != null) {
            mServiceHandler = new ServiceHandler(mServiceLooper);
        }

        Logger.d(TAG, "ContactsListContentObserver");
    }

    /**
     * This method is called when information about an ContactsListContent
     * which was previously requested using an asynchronous
     * interface becomes available.
     *
     * @param selfChange the self change
     */
    @Override
    public void onChange(boolean selfChange) {
        onChange(selfChange, null);
    }

    /**
     * This method is called when information about an ContactsListContent
     * which was previously requested using an asynchronous
     * interface becomes available.
     *
     * @param selfChange the self change
     * @param uri the uri
     */
    @Override
    public void onChange(boolean selfChange, Uri uri) {
        sendDelayedContactChangeMsg();
    }

    private void sendDelayedContactChangeMsg() {
        if (null != mServiceHandler) {
            // Remove any previous message for CONTACT_TABLE_MODIFIED.
            if (mServiceHandler.hasMessages(CONTACT_TABLE_MODIFIED)) {
                mServiceHandler.removeMessages(CONTACT_TABLE_MODIFIED);
            }

            // Send a new delayed message for CONTACT_TABLE_MODIFIED.
            Message msg = mServiceHandler.obtainMessage(CONTACT_TABLE_MODIFIED);
            mServiceHandler.sendMessageDelayed(msg, SYNC_COMPLETE_DELAY_TIMER);
        }
    }

    private final class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case CONTACT_TABLE_MODIFIED:
                Logger.d(TAG, "case CONTACT_TABLE_MODIFIED");
                updateDbChange();
                break;
            default:
                Logger.d(TAG, "default usecase hit! Do nothing");
                break;
            }
        }

        void updateDbChange() {
            new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                    ContactsListManager.getInstance().onContactsDbChange();
                    return null;
                }
            }.execute();
        }
    }
}
