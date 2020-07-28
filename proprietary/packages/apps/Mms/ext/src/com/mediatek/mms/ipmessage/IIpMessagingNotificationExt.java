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

package com.mediatek.mms.ipmessage;

import android.app.Notification;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;

import java.util.Set;
import java.util.SortedSet;

public interface IIpMessagingNotificationExt {

    /**
     * Called on init
     * @param context
     * @return boolean
     * @internal
     */
    public boolean ipMessagingNotificationInit(Context context);

    /**
     * Called on formatBigMessage
     * @param number: number
     * @param sender: sender
     * @return sender: sender string
     * @internal
     */
    public String onIpFormatBigMessage(String number, String sender);

    /**
     * Called on addSmsNotificationInfos
     * @param msgId: message id
     * @param cursor: cursor
     * @return boolean
     * @internal
     */
    public boolean isIpAttachMessage(long msgId, Cursor cursor);

    /**
     * Called on addSmsNotificationInfos
     * @param msgId: message id
     * @param cursor: cursor
     * @return boolean
     * @internal
     */
    public Bitmap getIpBitmap(long msgId, Cursor cursor);

    /**
     * Called on getNewMessageNotificationInfo
     * @param number: number
     * @param threadId: threadId
     * @return boolean
     * @internal
     */
    public boolean onIpgetNewMessageNotificationInfo(String number, long threadId);

    /**
     * Called on updateNotification
     * @param number: number
     * @param threadId: threadId
     * @param title: title
     * @return String: notification title
     * @internal
     */
    public String getIpNotificationTitle(String number, long threadId, String title);

    /**
     * Called on updateNotification.
     * @param context: notification context
     * @param number: number
     * @param threadId: threadId
     * @param BitmapDrawable: default drawable
     * @return BitmapDrawable: notification drawble
     * @internal
     */
    public BitmapDrawable getIpNotificationDrawable(Context context, String number, long threadId,
                                BitmapDrawable drawable);

    /**
     * Called on updateNotification
     * @param noti: noti
     * @param number: number
     * @return boolean
     * @internal
     */
    public boolean setIpSmallIcon(Notification.Builder noti, String number);

    /**
     * Called on buildTickerMessage
     * @param address: address
     * @param displayAddress: displayAddress
     * @return displayAddress
     * @internal
     */
    public String ipBuildTickerMessage(String address, String displayAddress);

    /**
     * blockingUpdateNewMessageIndicator.
     * @param context Context
     * @param threads threads Set
     * @param notificationSet notifications set
     * @param lockObject lock object
     * @return unread count
     * @internal
     */
    public int blockingUpdateNewMessageIndicator(Context context, Set<Long> threads,
                                            SortedSet notificationSet, Object lockObject);

    /**
     * Get undelivered Message Count.
     * @param context Context
     * @param threadIdResult A container to put the result in, according to the following rules:
     *  threadIdResult[0] contains the thread id of the first message.
     *  threadIdResult[1] is nonzero if the thread ids of all the messages are the same.
     *  You can pass in null for threadIdResult.
     *  You can pass in a threadIdResult of size 1 to avoid the comparison of each thread id.
     * @return
     * @internal
     */
    public int getUndeliveredMessageCount(Context context, long[] threadIdResult);
}

