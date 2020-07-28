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

import android.app.Activity;
import android.app.Dialog;
import android.content.AsyncQueryHandler;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.mms.callback.IComposeActivityCallback;
import com.mediatek.mms.callback.IWorkingMessageCallback;

import java.util.ArrayList;
import java.util.List;

/**
* Provide ComposeMessageActivity management related interface
*/
public class DefaultIpComposeActivityExt implements IIpComposeActivityExt {

    public boolean onIpComposeActivityCreate(
            Activity context, IComposeActivityCallback callback,
            Handler handler, Handler uiHandler,
            ImageButton sendButton, TextView typingTextView,
            TextView strangerTextView, View bottomPanel, Bundle bundle,
            ImageButton shareButton, LinearLayout panel,  EditText textEditor) {
        return false;
    }

    public boolean onIpComposeActivityResume(boolean isSmsEnabled,
            EditText textEditor, TextWatcher watcher, TextView textCounter,
            View recipientsEditor, View subjectTextEditor) {
        return false;
    }

    public boolean onIpComposeActivityPause() {
        return false;
    }

    public boolean onIpComposeActivityDestroy() {
        return false;
    }

    public boolean onIpTextChanged(CharSequence s, int start, int before, int count) {
        return false;
    }

    public boolean onIpAfterTextChanged(Editable s, String beforeTextChangeString) {
        return false;
    }

    public boolean onIpMessageListItemHandler(
            int msg, long currentMsgId, long threadId, long subId) {
        return false;
    }

    public boolean onIpUpdateCounter(CharSequence text, int start, int before, int count) {
        return false;
    }

    public boolean onIpDeleteMessageListenerClick(IIpMessageItemExt ipMsgItem) {
        return false;
    }

    public boolean onIpDiscardDraftListenerClick() {
        return false;
    }

    public boolean onIpCreateContextMenu(ContextMenu menu, boolean isSmsEnabled,
            boolean isForwardEnabled, IIpMessageItemExt ipMsgItem) {
        return false;
    }

    public boolean onIpMenuItemClick(MenuItem item, IIpMessageItemExt ipMsgItem) {
        return false;
    }

    public boolean onIpUpdateTitle(String number,
            String title, ImageView fullIntegratedView, ArrayList<String> titles) {
        return false;
    }

    public boolean onIpRecipientsEditorFocusChange(boolean hasFocus, List<String> numbers) {
        return false;
    }

    public boolean onIpInitialize(Intent intent, IWorkingMessageCallback workingMessageCallback) {
        return false;
    }

    public boolean onIpSaveInstanceState(Bundle outState, long threadId) {
        return false;
    }

    public boolean onIpShowSmsOrMmsSendButton(boolean isMms) {
        return false;
    }

    public boolean onIpPrepareOptionsMenu(IIpConversationExt ipConv, Menu menu) {
        return false;
    }

    public void onIpMsgActivityResult(
            Context context, int requestCode, int resultCode, Intent data) {}


    public boolean onIpHandleForwardedMessage(Intent intent) {
        return false;
    }

    public boolean onIpTextEditorKey(View v, int keyCode, KeyEvent event) {
        return false;
    }

    public boolean onIpInitMessageList(ListView list, IIpMessageListAdapterExt adapter) {
        return false;
    }

    public boolean loadIpMessagDraft(long threadId) {
        return false;
    }

    public boolean onIpSaveDraft(long threadId) {
        return false;
    }

    public boolean onIpResetMessage() {
        return false;
    }

    public boolean checkIpMessageBeforeSendMessage(long subId, boolean bCheckEcmMode) {
        return false;
    }

    public boolean onIpUpdateTextEditorHint() {
        return false;
    }

    public boolean onIpMsgOptionsItemSelected(
            IIpConversationExt ipConv, MenuItem item, long threadId) {
        return false;
    }

    public boolean handleIpMessage(Message msg) {
        return false;
    }

    public void onIpRecipientsChipChanged(int number) {
        return;
    }

    public boolean onIpHandleSharePanelMessage(Message msg) {
        return false;
    }

    public boolean onIpKeyDown(int keyCode, KeyEvent event) {
        return false;
    }

    public boolean onIPQueryMsgList(AsyncQueryHandler mQueryHandler,
            int token, Object cookie, Uri uri, String[]
            projection, String selection, String[] selectionArgs, String orderBy) {
        return false;
    }

    public void onIpConfig(Configuration newConfig) {
        return;
    }

    public Dialog onIpCreateDialog(int id) {
        return null;
    }

    public boolean onIpCheckRecipientsCount() {
        return false;
    }

    public boolean showIpMessageDetails(IIpMessageItemExt msgItem) {
        return false;
    }

    public boolean isIpRecipientCallable(String[] numbers) {
        return false;
    }

    public String dialIpRecipient(String number) {
        return number;
    }

    public boolean onIpProtocolChanged(boolean mms, boolean needToast) {
        return false;
    }

    public boolean toastIpConvertInfo(boolean toMms) {
        return false;
    }

    @Override
    public boolean onIpLayoutSizeChanged(boolean isSoftKeyBoardShow) {
        return false;
    }

    @Override
    public void onIpRecipientsEditorTouch() {
    }

    @Override
    public void onIpSubjectTextEditorFocusChange(boolean hasFocus) {
    }

    @Override
    public void onIpSubjectTextEditorTouch() {
    }

    @Override
    public void onIpTextEditorTouch() {
    }

    @Override
    public void resetIpConfiguration(boolean isLandscapeOld,
            boolean isLandscapeNew, boolean isSoftKeyBoardShow) {
    }

    @Override
    public boolean updateIpFullScreenTextEditorHeight() {
        return false;
    }

    @Override
    public boolean lockMessage(Context context, IIpMessageItemExt msgItem, boolean locked) {
        return false;
    }

    public boolean startMsgListQuery(AsyncQueryHandler mQueryHandler, int token, Object cookie,
            Uri uri, String[] projection, String selection, String[] selectionArgs,
            String orderBy) {
        return false;
    }

    @Override
    public boolean subSelection() {
        return false;
    }

    @Override
    public boolean onDeleteComplete(int token) {
        return false;
    }

    public void onIPRequestPermissionsResult(
            final int requestCode, final String permissions[], final int[] grantResults) {

    }

    public void initRecipientsEditor(View recipientsEditor) {
        //do noting
    }
}

