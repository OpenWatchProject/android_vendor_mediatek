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

package com.mediatek.mms.callback;

import android.content.Intent;
import android.net.Uri;
import android.view.View;
import android.view.View.OnCreateContextMenuListener;

import java.util.List;

public interface IComposeActivityCallback {

    /**
     * getRecipientsEditorInfo
     * @return boolean
     */
    public String getRecipientsEditorInfo();


    public boolean isIpRecipientEditorVisible();

    public List<String> getRecipientsEditorInfoList();

    /**
     * getConversationInfo
     * @return String[]: conversation info
     */
    public String[] getConversationInfo();

    /**
     * convertIpMessageToMmsOrSms
     */
    public void convertIpMessageToMmsOrSms(int type,
            boolean isAppend, String path, String text, int subId);

    /**
     * getCurrentThreadId
     * @return threadId
     */
    public long getCurrentThreadId();

    /**
     * callback deleteDraftSmsMessage
     */
    public void deleteDraftSmsMessage();

    /**
     * callbackCheckConditionsAndSendMessage
     * @param bCheckEcmMode: bCheckEcmMode
     */
    public void callbackCheckConditionsAndSendMessage(boolean bCheckEcmMode);

    /**
     * callbackUpdateSendButtonState
     */
    public void callbackUpdateSendButtonState();

    /**
     * callback checkBeforeSendMessage
     * @param bCheckEcmMode: bCheckEcmMode
     */
     public void callbackCheckBeforeSendMessage(boolean bCheckEcmMode);

    /**
     * callback UpdateButtonState
     * @param canResponse: canResponse
     */
    public void callbackUpdateButtonState(boolean canResponse);

    /**
     * callback SendMessage
     * @param bCheckEcmMode: bCheckEcmMode
     */
    public void callbackSendMessage(boolean bCheckEcmMode);

    /**
     * callback asyncDeleteDraftSmsMessage
     */
    public void asyncDeleteDraftSmsMessage();

    /**
     * callback addContacts
     * @param pickCount: pickCount
     * @param requestCode: requestCode
     */
    public void addIpContacts(int pickCount, int requestCode);

    /**
     * callback genThreadIdFromContacts
     * @param data: data
     * @return threadId
     */
    public long genIpThreadIdFromContacts(Intent data);

    /**
     * callback setOnlineDividerString
     * @param onLine: onLine
     */
    public void setIpOnlineDividerString(String onLine);

    /**
     * callback updateOnlineDividerTime
     */
    public void updateIpOnlineDividerTime();

    /**
     * callback notifyDataSetChanged
     */
    public void notifyIpDataSetChanged();

    /**
     * callback invalidateOptionsMenu
     */
    public void invalidateIpOptionsMenu();

    /**
     * callback showOrMmsSendButton
     * @param isMms: isMms
     */
    public View showIpOrMmsSendButton(boolean isMms);

    /**
     * callback setClearCacheFlag
     * @param clearCacheFlag: clearCacheFlag
     */
    public void setClearIpCacheFlag(boolean clearCacheFlag);

    /**
     * callback setDraftState
     * @param hasDraft: hasDraft
     */
    public void setIpDraftState(boolean hasDraft);

    /**
     * callback runAsyncInThreadPool
     * @param backgroundTask: backgroundTask
     * @param postExecuteTask: postExecuteTask
     * @param dialogStringId: dialogStringId
     */
    public void runIpAsyncInThreadPool(final Runnable backgroundTask,
            final Runnable postExecuteTask, final int dialogStringId);

    /**
     * callback syncWorkingRecipients
     */
    public void syncIpWorkingRecipients();

    /**
     * callback guaranteeThreadId
     */
    public void guaranteeIpThreadId();

    /**
     * callback onPreMessageSent
     */
    public void onPreIpMessageSent();

    /**
     * callback resetMessage
     */
    public void resetIpMessage();

    /**
     * callback onMessageSent
     */
    public void onIpMessageSent();

    /**
     * callback onClick
     * @param view: view
     */
    public void onIpClick(View view);

    /**
     * callback isSubjectEditorVisible
     * @return boolean
     */
    public boolean isIpSubjectEditorVisible();

    /**
     * callback showSubjectEditor
     * @param isShow: isShow
     */
    public void showIpSubjectEditor(boolean isShow);

    /**
     * callback drawBottomPanel
     */
    public void drawIpBottomPanel();

    /**
     * call back hideSharePanel
     */
    public void hideIpSharePanel();

    /**
     * call back addAttachment
     * @param commonAttachmentType: commonAttachmentType
     */
    public void addIpAttachment(int commonAttachmentType, boolean append);

    public void enableShareButton(boolean enable);

    public void resetSharePanel();

    public IWorkingMessageCallback getWorkingMessage();

    public void hideIpRecipientEditor();

    public void onIpSubSelection();

    public void updateIpTitle();

    public void hideIpInputMethod();

    public void showAddAttachmentDialog(boolean append);

    /**
     * check size before append
     * @return true if size is ok
     */
    public boolean checkSizeBeforeAppend();

    public int setAttachment(int type, Uri uri, boolean append);

    public void handleAddAttachmentError(int result, int mediaTypeStringId);

    public void checkRecipientsCountCallback();

    public void confirmForChangeMmsToSmsCallback();

    public void goToConversationListCallback();

    public void setHadToSlideShowEditor(boolean hadToSlideShowEditor);

    public void setWorkingMessage(IWorkingMessageCallback workingMessage);

    public void updateThreadIdIfRunningCallback();

    public void drawTopPanelCallback(boolean showSubjectEditor);

    public void processNewIntentCallback(Intent intent);

    public void updateCounterCallback(CharSequence text, int start, int before, int count);

    public boolean isPreparedForSendingCallback();

    public void setIsPickContatct(boolean isPickContatct);

    public void updateSendButtonStateCallback(boolean enabled);

    public void updateTextEditorHintCallback();

    public void setSendButtonCanResponse(boolean sendButtonCanResponse);

    public boolean getSendButtonCanResponse();

    public boolean isHasRecipientCountCallback();

    public void requestRecipientsEditorFocus();

    public CharSequence getTextEditorText();

    public void hideOrShowTopPanelCallback();

    public boolean getIsSmsEnabled();

    public OnCreateContextMenuListener getOnCreateContextMenuListener();

    public String getNumbersFromIntentCallback(Intent intent);

    public void confirmSendMessageIfNeededCallback();

    public void setSubIdCallback(int subId);

    public void updateFullScreenTextEditorHeightCallback();

    public void showKeyBoardCallback(boolean isShow);

    public void setTextEditorMaxHeight(int linesHeight);

    public void unpreparedForSendingAlertCallback();

    public boolean showWfcSendButtonPopUpCallback();
}
