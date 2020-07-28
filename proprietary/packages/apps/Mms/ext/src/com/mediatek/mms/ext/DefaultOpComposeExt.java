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

package com.mediatek.mms.ext;

import android.app.Activity;
import android.content.AsyncQueryHandler;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.SubscriptionInfo;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.google.android.mms.pdu.PduBody;

import com.mediatek.mms.callback.IComposeActivityCallback;
import com.mediatek.mms.callback.IMessageItemCallback;
import com.mediatek.mms.callback.IMessageListAdapterCallback;
import com.mediatek.mms.callback.IRecipientsEditorCallback;
import com.mediatek.mms.callback.ISlideshowModelCallback;
import com.mediatek.mms.callback.ITextSizeAdjustHost;
import com.mediatek.mms.callback.IWorkingMessageCallback;

import java.util.ArrayList;
import java.util.List;


public class DefaultOpComposeExt extends ContextWrapper implements IOpComposeExt {
    private static final String TAG = "Mms/MmsComposeImpl";

    public DefaultOpComposeExt(Context context) {
        super(context);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item, IOpWorkingMessageExt workingMessageExt,
            InputMethodManager inputMethodManager) {
        return false;
    }

    @Override
    public void showSubjectEditor(EditText subjectEditor) {
        return;
    }

    public void addCallAndContactMenuItems(Context context, ContextMenu menu, int menuId,
            ArrayList<String> urls, IMessageItemCallback msgItem) {
        Log.d(TAG, "addCallAndContactMenuItems addMmsUrlToBookMark");
        return;
    }

    @Override
    public boolean asyncAttachVCardByContactsId(boolean append) {
        return false;
    }

    @Override
    public boolean processNewIntent(Intent intent) {
        return false;
    }

    @Override
    public boolean subSelectionDialog(int subId) {
        return false;
    }

    @Override
    public void editSlideshow(boolean hadToSlideShowEditor) {

    }

    @Override
    public boolean onNewIntent(final Intent intent, boolean hadToSlideShowEditor, Uri tempMmsUri,
            final IWorkingMessageCallback workingMessage, Object activity,
            IRecipientsEditorCallback recipientsEditor, final int box) {
        return false;
    }

    @Override
    public boolean forwardMessage(Context context, IMessageItemCallback msgItem,
            String nameAndNumber, Intent intent) {
        return false;
    }

    @Override
    public void onStart(ITextSizeAdjustHost host, Activity activity) {

    }

    @Override
    public void onResume(int subCount, String text, boolean isSmsEnabled,
            View recipientsEditor, View subjectTextEditor) {
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu, boolean isSmsEnabled,
            boolean isRecipientsEditorVisible, int contactsSize, long threadId) {
        return false;
    }

    @Override
    public boolean onClick(View v, ImageButton recipientsPicker) {
        return false;
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        return false;
    }

    @Override
    public void initResourceRefs(LinearLayout buttonWithCounter, TextView textCounter,
                                 IOpAttachmentEditorExt attachmentEditor) {
    }

    @Override
    public boolean updateSendButtonState(boolean requiresMms, int recipientCount,
            int smsRecipientLimit, int subCount) {
        return false;
    }

    @Override
    public boolean attachVCalendar(
            boolean append,
            int type,
            Uri uri,
            int mediaTypeStringId) {
        return false;
    }

    @Override
    public String checkRecipientsCount(String message, boolean requiresMms,
            int recipientCount, int mmsLimitCount, boolean isRecipientsEditorEmpty,
            boolean isConversationRecipientEmpty, Intent intent,
            IMessageListAdapterCallback msgListAdapter,
            List<SubscriptionInfo> subInfoList,
            boolean isRecipientsEditorVisible) {
        return message;
    }

    @Override
    public boolean handleAttachmentEditorHandlerMessage(Message msg, boolean sendButtonCanResponse,
            IWorkingMessageCallback opWorkingMessageCallback, ISlideshowModelCallback slideshow,
            boolean compressingImage) {
        Log.d(TAG, "handleAttachmentEditorHandlerMessage");
        if (msg == null) {
            if (slideshow != null && opWorkingMessageCallback != null) {
                int size = slideshow.sizeCallback();
                Log.d(TAG, "size = " + size);
                if (size > 1) {
                    opWorkingMessageCallback.setIpText("");
                }
            }
        }
        return false;
    }

    @Override
    public boolean haveSomethingToCopyToSDCard(Context context, PduBody body) {
        return false;
    }

    @Override
    public void onCreate(IComposeActivityCallback ipComposeActivityCallback, Intent intent,
            IntentFilter intentFilter, Activity oldCompose, Activity compose,
            Bundle savedInstanceState, Handler uiHandler, ImageButton shareButton,
            LinearLayout panel,  EditText textEditor) {
    }

    @Override
    public boolean onMenuItemClick(MenuItem item,
            Intent intentMultiSave, Intent deliveryIntent) {
        return false;
    }

    @Override
    public int getSmsEncodingType(int encodingType, Context context) {
        return encodingType;
    }

    @Override
    public boolean checkConditionsAndSendMessage(boolean isMms, final boolean bcheckMode) {
        return false;
    }

    @Override
    public int subSelection() {
        return 0;
    }

    @Override
    public boolean updateSendButton(boolean enabled, int subCount, boolean requiresMms,
                                boolean hasSlideshow) {
        return false;
    }

    @Override
    public void onConfigurationChanged() {
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return false;
    }

    @Override
    public boolean onLayoutSizeChanged(boolean isSoftKeyBoardShow) {
        return false;
    }

    @Override
    public void onRecipientsEditorFocusChange(boolean hasFocus) {
    }

    @Override
    public void onRecipientsEditorTouch() {
    }

    @Override
    public void onSubjectTextEditorFocusChange(boolean hasFocus) {
    }

    @Override
    public void onSubjectTextEditorTouch() {
    }

    @Override
    public void onTextEditorTouch() {
    }

    @Override
    public void resetConfiguration(boolean isLandscapeOld,
            boolean isLandscapeNew, boolean isSoftKeyBoardShow) {
    }

    @Override
    public boolean updateFullScreenTextEditorHeight() {
        return false;
    }

    @Override
    public boolean sendMessageForDualSendButton() {
       return false;
    }

    @Override
    public void initResourceRefsOnResume(LinearLayout buttonWithCounter, TextView textCounter,
                                 IOpAttachmentEditorExt attachmentEditor) {
    }
}
