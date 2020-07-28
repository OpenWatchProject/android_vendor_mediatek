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

package com.mediatek.wappush;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;

import com.android.mms.util.MmsLog;

import mediatek.telephony.MtkTelephony.WapPush;


public class SiExpiredCheck {

    private static String TAG = "Mms/WapPush";
    private static int SLEEP_INTERVAL = 10000;
    private boolean mIsStarted = false; // thread is started?
    private boolean mIsPaused = true;  // check is paused?

    private Context mContext;

    public SiExpiredCheck(Context context) {
        mContext = context;
    }

    /*
     * Check selected si message whether it expired
     */
    private static synchronized void siExpiredCheck(Context context) {
        if (context == null) {
            return;
        }

        int currentTime = (int) (System.currentTimeMillis() / 1000);
        ContentResolver resolver = context.getContentResolver();
        String[] projection = { WapPush._ID, WapPush.EXPIRATION, WapPush.ERROR };

        Cursor cursor = resolver.query(WapPush.CONTENT_URI_SI, projection, null, null, null);
        ContentValues expiredValues = new ContentValues();
        ContentValues noExpiredValues = new ContentValues();
        expiredValues.put(WapPush.ERROR, 1);
        noExpiredValues.put(WapPush.ERROR, 0);

        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    do {
                        long messageId = cursor.getLong(0);
                        int expiredTime = cursor.getInt(1);
                        int expried = cursor.getInt(cursor.getColumnIndexOrThrow(WapPush.ERROR));
                        //the message is expired!
                        if (expiredTime > 0 && expiredTime < currentTime && expried == 0) {
                            MmsLog.i(TAG, "SiExpiredCheck: message " + messageId + " is expired!");
                            resolver.update(WapPush.CONTENT_URI_SI,
                                    expiredValues, WapPush._ID + " = " + messageId, null);
                        }
                        //the message is not expired
                        if ((expiredTime > currentTime || expiredTime == 0) && expried == 1) {
                            MmsLog.i(TAG, "SiExpiredCheck: message "
                                    + messageId + " is set noexpired!");
                            resolver.update(WapPush.CONTENT_URI_SI,
                                    noExpiredValues, WapPush._ID + " = " + messageId, null);
                        }
                    } while(cursor.moveToNext());

                }
            } finally {
                cursor.close();
            }
        }
    }

    /*
     * When the time is changed , the expired message should be rechecked.
     */
    public static void onTimeChanged(final Context context) {
        MmsLog.i(TAG, "onTimeChanged");
        siExpiredCheck(context);
    }

    public void startExpiredCheck() {
        if (mIsPaused) {
            MmsLog.w(TAG, "startExpiredCheck!");
            mIsPaused = false;
        }
    }

    public void stopExpiredCheck() {
        if (!mIsPaused) {
            MmsLog.w(TAG, "stopExpiredCheck!");
            mIsPaused = true;
        }
    }

    private Thread mThread = new Thread() {
        public void run() {
            while (mIsStarted) {
                if (!mIsPaused) {
                    siExpiredCheck(mContext);
                }
                try {
                    sleep(SLEEP_INTERVAL);
                } catch (InterruptedException e) {
                    return;
                }
            }
        }
    };

    //start looper thread;
    public void startSiExpiredCheckThread() {
        try {
            mIsStarted = true;
            mThread.start();
        } catch (Exception e) {
            MmsLog.e(TAG, "SiExpiredCheck: thread start error!");
        }
    }

    //stop looper thread;
    public void stopSiExpiredCheckThread() {
        try {
            mThread.interrupt();
            mIsStarted = false;
        } catch (Exception e) {
            MmsLog.e(TAG, "SiExpiredCheck: thread stop error!");
        }
    }
}
