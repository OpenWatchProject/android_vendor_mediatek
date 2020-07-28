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

import android.content.ActivityNotFoundException;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.widget.Toast;

import com.android.browser.provider.Browser;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.ui.MessageUtils;
import com.android.mms.util.MmsLog;

import com.mediatek.pushparser.ParsedMessage;
import com.mediatek.pushparser.sl.SlMessage;

import mediatek.telephony.MtkTelephony.WapPush;

public class SlManager extends WapPushManager {

    protected SlManager(Context context) {
        super(context);
    }

    @Override
    public void handleIncoming(ParsedMessage message) {

        if (message == null) {
            MmsLog.w(TAG, "SlManager handleIncoming: null message");
            return;
        }

        SlMessage slMsg = null;
        try {
             slMsg = (SlMessage) message;
        } catch (Exception e) {
            MmsLog.e(TAG, "SlManager SiMessage error");
        }

        //store in db
        ContentValues values = new ContentValues();
        values.put(WapPush.ADDR, slMsg.getSenderAddr());
        values.put(WapPush.SERVICE_ADDR, slMsg.getServiceCenterAddr());
        values.put(WapPush.SUBSCRIPTION_ID, slMsg.getSubId());
        values.put(WapPush.URL, slMsg.url);
        values.put(WapPush.ACTION, slMsg.action);

        boolean isAutoLanuching = false;
        if (MmsConfig.getSlAutoLanuchEnabled() && autoLanuching(slMsg.url)) {
            values.put(WapPush.SEEN, WapPush.STATUS_SEEN);
            values.put(WapPush.READ, WapPush.STATUS_READ);
            isAutoLanuching = true;
        }

        Uri uri = mContext.getContentResolver().insert(WapPush.CONTENT_URI_SL, values);

        //notification
        if (uri != null) {
            MmsLog.ipi(TAG, "SlManager:Store msg! " + slMsg.url);
            if (!isAutoLanuching) {
                long threadId = WapPushMessagingNotification.getWapPushThreadId(mContext, uri);
                MmsLog.d(TAG, "insert new SL message, threaId:" + threadId);
                WapPushMessagingNotification.blockingUpdateNewMessageIndicator(mContext, threadId);
            }
        }
    }

    //if sl autoLanuching is set, we will open the url.
    private boolean autoLanuching(String url) {

        if (null == url) {
            return false;
        }

        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        boolean isAutoLoading = prefs.getBoolean(
                "pref_key_wappush_sl_autoloading", false);

        if (isAutoLoading) {

            WapPushMessagingNotification.notifySlAutoLanuchMessage(mContext, url);

            Uri uri = Uri.parse(MessageUtils.checkAndModifyUrl(url));
            Intent intent = new Intent(Intent.ACTION_VIEW, uri);
            intent.putExtra(Browser.EXTRA_APPLICATION_ID, mContext
                    .getPackageName());
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            try {
                mContext.startActivity(intent);
            } catch (ActivityNotFoundException ex) {
                Toast.makeText(mContext, R.string.error_unsupported_scheme,
                        Toast.LENGTH_LONG).show();
                MmsLog.e(TAG, "Scheme " + uri.getScheme() + "is not supported!");
            }
            return true;
        }

        return false;
    }

}
