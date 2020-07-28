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
package com.mediatek.contacts.simservice;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;

import com.android.contacts.ContactSaveService;
import com.android.contacts.interactions.ContactDeletionInteraction;

import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.simservice.SimProcessorManager.ProcessorCompleteListener;
import com.mediatek.contacts.util.Log;

public class SimDeleteProcessor extends SimProcessorBase {
    private static final String TAG = "SIMDeleteProcessor";

    private static Listener mListener = null;

    private Uri mSimUri = null;
    private Uri mLocalContactUri = null;
    private int mSimIndex = -1;
    private Context mContext;
    private Intent mIntent;
    private int mSubId = SubInfoUtils.getInvalidSubId();

    public final static String SIM_INDEX = "sim_index";
    public final static String LOCAL_CONTACT_URI = "local_contact_uri";

    public interface Listener {
        public void onSIMDeleteFailed();
        public void onSIMDeleteCompleted();
    }

    public static void registerListener(Listener listener) {
        if (listener instanceof ContactDeletionInteraction) {
            Log.i(TAG, "[registerListener]listener added to SIMDeleteProcessor:" + listener);
            mListener = listener;
        }
    }

    public static void unregisterListener(Listener listener) {
        Log.i(TAG, "[unregisterListener]removed from SIMDeleteProcessor: " + listener);
        mListener = null;
    }

    public SimDeleteProcessor(Context context, int subId, Intent intent,
            ProcessorCompleteListener listener) {
        super(intent, listener);
        Log.i(TAG, "[SIMDeleteProcessor]new...");
        mContext = context;
        mSubId = subId;
        mIntent = intent;
    }

    @Override
    public int getType() {
        return SimServiceUtils.SERVICE_WORK_DELETE;
    }

    @Override
    public void doWork() {
        if (isCancelled()) {
            Log.w(TAG, "[dowork]cancel remove work. Thread id = "
                    + Thread.currentThread().getId());
            return;
        }
        mSimUri = mIntent.getData();
        mSimIndex = mIntent.getIntExtra(SIM_INDEX, -1);
        mLocalContactUri = mIntent.getParcelableExtra(LOCAL_CONTACT_URI);
        if (mContext.getContentResolver().delete(mSimUri, "index = " + mSimIndex, null) <= 0) {
            Log.i(TAG, "[doWork] Delete SIM contact failed");
            if (mListener != null) {
                mListener.onSIMDeleteFailed();
            }
        } else {
            Log.i(TAG, "[doWork] Delete SIM contact successfully");
            mContext.startService(ContactSaveService.createDeleteContactIntent(mContext,
                    mLocalContactUri));
            if (mListener != null) {
                mListener.onSIMDeleteCompleted();
            }
        }
    }
}
