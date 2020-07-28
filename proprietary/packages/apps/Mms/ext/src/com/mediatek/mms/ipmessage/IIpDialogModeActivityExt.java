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

import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.mms.callback.IDialogModeActivityCallback;

public interface IIpDialogModeActivityExt {

    /**
     * M: called updateAvatarView
     * @param context DialogModeActivity
     * @param uris Urils
     * @param mmsView MMS view
     * @param cursor Cursor
     * @param callback {@link IDialogModeActivityCallback}
     * @param replyEditor Text Editor
     * @param smsContentText SMS content text
     * @param ipView ipView, plugin can add ipMessageview in this item
     * @param groupSender TextView
     * @param contactImage ContactImage
     * @return
     * @internal
     */
    public boolean onIpInitDialogView(Activity context, List<Uri> uris, View mmsView, Cursor cursor,
            IDialogModeActivityCallback callback, EditText replyEditor, TextView smsContentText,
            View ipView, TextView groupSender, ImageView contactImage);

    /**
     * M: called addNewUri
     * @param intent: intent
     * @param newUri: newUri
     * @return boolean
     * @internal
     */
    public boolean onIpAddNewUri(Intent intent, Uri newUri);

    /**
     * M: called onDestroy
     * @return boolean
     * @internal
     */
    public boolean onIpDestroy();

    /**
     * M: called setDialogView
     * @param simCharSequence: simCharSequence
     * @return CharSequence: simStatus
     * @internal
     */
    public String onIpSetDialogView();

    /**
     * M: called getSenderString
     * @return String: get sender
     * @internal
     */
    public String onIpGetSenderString();

    /**
     * M: called getSenderNumber
     * @return String: sender number
     * @internal
     */
    public String onIpGetSenderNumber();

    /**
     * M: called onClick
     * @param threadId: threadId
     * @return boolean
     * @internal
     */
    public boolean onIpClick(long threadId);

    /**
     * M: called sendReplySms
     * @param body: body
     * @param to: to
     * @return boolean
     * @internal
     */
    public boolean onIpSendReplySms(String body, String to);

    /**
     * M: called sendMessage
     * @param body: body
     * @param to: to
     * @return boolean
     * @internal
     */
    public boolean onIpSendMessage(String body, String to);

    /**
     * M: called updateSendButtonState
     * @param sendButton: sendButton
     * @return boolean
     * @internal
     */
    public boolean onIpUpdateSendButtonState(ImageButton sendButton);

    /**
     * Load current message content.
     * @param uri Uri
     * @param projection String[]
     * @param selection  String
     * @return Cursor
     * @internal
     */
    public Cursor loadCurMsg(Uri uri, String[] projection, String selection);

    /**
     * Mark the message as read.
     * @param context Context
     * @param uri Uri
     * @return return true if processed, else return false
     * @internal
     */
    public boolean markAsRead(Context context, Uri uri);

    /** Get received time.
     *
     * @param context application context
     * @param c Cursor
     * @return the formatted string.
     * @internal
     */
    public String getReceivedTime(Context context, Cursor c);
}

