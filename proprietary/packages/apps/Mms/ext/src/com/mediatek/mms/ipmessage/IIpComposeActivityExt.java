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

import java.util.ArrayList;
import java.util.List;

import com.mediatek.mms.callback.IComposeActivityCallback;
import com.mediatek.mms.callback.IWorkingMessageCallback;

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

public interface IIpComposeActivityExt {

    /**
     * M: Called on ComposeMessageActivity onCreate
     * @param context: this ComposeMessageActivity
     * @param callback: ComposeMessageActivity callback
     * @param workingMessageCallback: WorkingMessage callback
     * @param handler: UI handler
     * @param sendButton: send button
     * @param typingTextView: typingTextView
     * @param strangerTextView: strangerTextView
     * @param bottomPanel: bottomPanel view
     * @param bundle: bundle object
     * @return boolean
     * @internal
     */
    public boolean onIpComposeActivityCreate(Activity context,
            IComposeActivityCallback callback, Handler handler, Handler uiHandler,
            ImageButton sendButton, TextView typingTextView,
            TextView strangerTextView, View bottomPanel, Bundle bundle,
            ImageButton shareButton, LinearLayout panel, EditText textEditor);

    /**
     * M: Called on ComposeMessageActivity onResume
     * @param isSmsEnabled: default Mms app is enable
     * @param textEditor: Text editor to type your message into
     * @param watcher: The text watcher of the textEditor
     * @param textCounter: Shows the number of characters used in text editor
     * @return boolean
     * @internal
     */
    public boolean onIpComposeActivityResume(boolean isSmsEnabled,
            EditText textEditor, TextWatcher watcher, TextView textCounter,
            View recipientsEditor, View subjectTextEditor);

    /**
     * M: Called on ComposeMessageActivity onPause
     * @return boolean
     * @internal
     */
    public boolean onIpComposeActivityPause();

    /**
     * M: Called on ComposeMessageActivity onDestroy
     * @return
     * @internal
     */
    public boolean onIpComposeActivityDestroy();

    /**
     * M: Called on mTextEditorWatcher onTextChanged
     * @param s: CharSequence
     * @param start: int
     * @param before: int
     * @param count: int
     * @return boolean
     * @internal
     */
    public boolean onIpTextChanged(CharSequence s, int start, int before, int count);

    /**
     * M: Called on mTextEditorWatcher afterTextChanged
     * @param s: Editable
     * @param beforeTextChangeString: beforeTextChangeString
     * @return boolean
     * @internal
     */
    public boolean onIpAfterTextChanged(Editable s, String beforeTextChangeString);

    /**
     * M: Called on mMessageListItemHandler handleMessage
     * @param msg: handle message
     * @param currentMsgId:currentMsgId
     * @param threadId: threadId
     * @param subId: subId
     * @return boolean
     * @internal
     */
    public boolean onIpMessageListItemHandler(int msg,
            long currentMsgId, long threadId, long subId);

    /**
     * M: Called on updateCounter
     * @param text: input editbox text
     * @param start: start count
     * @param before: before count
     * @param count: text count
     * @return boolean
     * @internal
     */
    public boolean onIpUpdateCounter(CharSequence text, int start, int before, int count);

    /**
     * M: DeleteMessageListener called
     * @param ipMsgItem: IIpMessageItemExt
     * @return boolean
     * @internal
     */
    public boolean onIpDeleteMessageListenerClick(IIpMessageItemExt ipMsgItem);

    /**
     * M: DiscardDraftListener called
     * @return boolean
     * @internal
     */
    public boolean onIpDiscardDraftListenerClick();

    /**
     * M: onCreateContextMenu called
     * @param menu: ContextMenu
     * @param isSmsEnabled: isSmsEnabled
     * @param isForwardEnabled: isForwardEnabled
     * @param ipMsgItem: this IpMessageItem
     * @return boolean
     * @internal
     */
    public boolean onIpCreateContextMenu(ContextMenu menu, boolean isSmsEnabled,
            boolean isForwardEnabled, IIpMessageItemExt ipMsgItem);

    /**
     * M: onMenuItemClick called
     * @param item: MenuItem
     * @param ipMsgItem: this IpMessageItem
     * @return boolean
     * @internal
     */
    public boolean onIpMenuItemClick(MenuItem item, IIpMessageItemExt ipMsgItem);

    /**
     * M: onIpUpdateTitle called
     * @param number: number
     * @param title: title
     * @param fullIntegratedView: fullIntegratedView
     * @param titles: titles
     * @return boolean
     * @internal
     */
    public boolean onIpUpdateTitle(String number,
            String title, ImageView fullIntegratedView, ArrayList<String> titles);

    /**
     * M: mRecipientsEditor onFocusChange called
     * @param hasFocus: hasFocus
     * @param numbers: numbers
     * @return boolean
     * @internal
     */
    public boolean onIpRecipientsEditorFocusChange(boolean hasFocus, List<String> numbers);

    /**
     * M: initialize called
     * @param intent: this activity intent
     * @return boolean
     * @internal
     */
    public boolean onIpInitialize(Intent intent, IWorkingMessageCallback workingMessageCallback);

    /**
     * M: onSaveInstanceState called
     * @param outState: outState
     * @param numbers: numbers
     * @param threadId: threadId
     * @return boolean
     * @internal
     */
    public boolean onIpSaveInstanceState(Bundle outState, long threadId);

    /**
     * M: showSmsOrMmsSendButton called
     * @param isMms: isMms
     * @return boolean
     * @internal
     */
    public boolean onIpShowSmsOrMmsSendButton(boolean isMms);

    /**
     * M: onPrepareOptionsMenu called
     * @param ipConv: IpConversation
     * @param menu: OptionsMenu
     * @return boolean
     * @internal
     */
    public boolean onIpPrepareOptionsMenu(IIpConversationExt ipConv, Menu menu);

    /**
     * M: onActivityResult called
     * @param context: this
     * @param requestCode: requestCode
     * @param resultCode: resultCode
     * @param data: data
     * @internal
     */
    public void onIpMsgActivityResult(Context context,
            int requestCode, int resultCode, Intent data);

    /**
     * M: handleForwardedMessage called
     * @param intent: intent
     * @return boolean
     * @internal
     */
    public boolean onIpHandleForwardedMessage(Intent intent);

    /**
     * M: mTextEditor onKey called
     * @param v: mTextEditor
     * @param keyCode: keyCode
     * @param event: event
     * @return boolean
     * @internal
     */
    public boolean onIpTextEditorKey(View v, int keyCode, KeyEvent event);

    /**
     * M: initMessageList called
     * @return boolean
     * @internal
     */
    public boolean onIpInitMessageList(ListView list, IIpMessageListAdapterExt adapter);

    /**
     * M: loadDraft called
     * @param threadId: threadId
     * @return boolean
     * @internal
     */
    public boolean loadIpMessagDraft(long threadId);

    /**
     * M: saveDraft called
     * @param number: number
     * @param threadId: threadId
     * @return boolean
     * @internal
     */
    public boolean onIpSaveDraft(long threadId);

    /**
     * M: resetMessage called
     * @return boolean
     * @internal
     */
    public boolean onIpResetMessage();

    /**
     * M: checkBeforeSendMessage called
     * @param bCheckEcmMode: bCheckEcmMode
     * @return boolean
     * @internal
     */
    public boolean checkIpMessageBeforeSendMessage(long subId, boolean bCheckEcmMode);

    /**
     * M: updateTextEditorHint called
     * @return boolean
     * @internal
     */
    public boolean onIpUpdateTextEditorHint();

    /**
     * M: onOptionsItemSelected called
     * @param ipConv: IpConversation
     * @param item: OptionsItem
     * @param threadId: threadId
     * @return boolean
     * @internal
     */
    public boolean onIpMsgOptionsItemSelected(
            IIpConversationExt ipConv, MenuItem item, long threadId);

    /**
     * M: handle share message
     * @param msg: message
     * @return boolean
     * @internal
     */
    public boolean handleIpMessage(Message msg);

    /**
     * @internal
     */
    public void onIpRecipientsChipChanged(int number);

    /**
     * @internal
     */
    public boolean onIpKeyDown(int keyCode, KeyEvent event);

    /**
     * @internal
     */
    public boolean onIPQueryMsgList(AsyncQueryHandler mQueryHandler,
            int token, Object cookie, Uri uri, String[]
            projection, String selection, String[] selectionArgs, String orderBy);

    /**
     * @internal
     */
    public void onIpConfig(Configuration newConfig);

    /**
     *
     * @param id  dialog id
     * @return Dialog
     * @internal
     */
    public Dialog onIpCreateDialog(int id);

    /**
     *
     * @return
     * @internal
     */
    public boolean onIpCheckRecipientsCount();

    /**
     * @internal
     */
    public boolean showIpMessageDetails(IIpMessageItemExt msgItem);

    /**
     * @internal
     */
    public boolean isIpRecipientCallable(String[] numbers);

    /**
     * M: handle dial recipient
     * @param number: dial number
     * @return Srtring
     * @internal
     */
    public String dialIpRecipient(String number);

    /**
     * M: Protocol Changed
     * @param mms: is mms
     * @param needToast: need Toast
     * @return boolean
     * @internal
     */
    public boolean onIpProtocolChanged(boolean mms, boolean needToast);

    /**
     * M: toast IpMsg Convert Info
     * @param toMms: Convert to mms
     * @return boolean
     * @internal
     */
    public boolean toastIpConvertInfo(boolean toMms);

    /**
     * M: Called on ComposeMessageActivity onTouch of RecipientsEditor
     * @internal
     */
    public void onIpRecipientsEditorTouch();

    /**
     * M: Called on ComposeMessageActivity onTouch of SubjectTextEditor
     * @internal
     */
    public void onIpSubjectTextEditorTouch();

    /**
     * M: Called on ComposeMessageActivity onFocusChange of SubjectTextEditor
     * @param hasFocus Whether its focus changed or not
     * @internal
     */
    public void onIpSubjectTextEditorFocusChange(boolean hasFocus);

    /**
     * M: Called on ComposeMessageActivity resetConfiguration
     * @param isLandscapeOld device was landscape or not before
     * @param isLandscapeNew device is landscape or not now
     * @param isSoftKeyBoardShow is soft key board show or not
     * @internal
     */
    public void resetIpConfiguration(boolean isLandscapeOld,
            boolean isLandscapeNew, boolean isSoftKeyBoardShow);

    /**
     * M: Called on ComposeMessageActivity onTouch of TextEditor
     * @internal
     */
    public void onIpTextEditorTouch();

    /**
     * M: Called on ComposeMessageActivity onLayoutSizeChanged
     * @param isSoftKeyBoardShow  is soft key board show or not
     * @internal
     */
    public boolean onIpLayoutSizeChanged(boolean isSoftKeyBoardShow);

    /**
     * M: Called on ComposeMessageActivity updateFullScreenTextEditorHeight
     * @return operator return true, default return false
     * @internal
     */
    public boolean updateIpFullScreenTextEditorHeight();

    /**
     * Lock Message.
     * @param context Context
     * @param msgItem IIpMessageItemExt
     * @param locked set locked value to provider
     * @return true if processed.
     * @internal
     */
    public boolean lockMessage(Context context, IIpMessageItemExt msgItem, boolean locked);

    /**
     * @internal
     */
    public boolean startMsgListQuery(AsyncQueryHandler mQueryHandler, int token, Object cookie,
            Uri uri, String[] projection, String selection, String[] selectionArgs, String orderBy);

    /**
     * @internal
     */
    public boolean subSelection();

    /**
     * This is called in onDeleteComplete of ComposeMessageActivity
     * @param token int
     * @return If plugin processed return true, host don't process any more, else return false.
     * @internal
     */
    public boolean onDeleteComplete(int token);

    /**
     * This is invoked to plugin when get result from request permissions
     * @param requestCode Request Code of the permission requested
     * @param permissions List of permissions
     * @param grantResults results of all permissions requested
     * @internal
     */
    public void onIPRequestPermissionsResult(
            final int requestCode, final String permissions[], final int[] grantResults);

    /**
     * This is called in initRecipientsEditor of composer.
     */
    public void initRecipientsEditor(View recipientsEditor);
}

