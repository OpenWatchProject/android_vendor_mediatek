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
package com.mediatek.rcse.plugin.message;

import android.app.Activity;

import android.content.Context;
import android.content.Intent;

//import com.google.android.mms.ContentType;

import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.SelectContactsActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.plugin.message.IpMessageConsts.*;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.Utils;

import com.mediatek.rcse.service.MediatekFactory;

import java.util.ArrayList;

import com.gsma.services.rcs.chat.GroupChat;

/**
 * The Class IpMessageActivitiesManager.
 */
public class IpMessageActivitiesManager {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "IpMessageActivitiesManager";
    /**
     * The Constant ACTION_SETTINGS_ACIVITY.
     */
    private static final String ACTION_SETTINGS_ACIVITY =
            "com.mediatek.rcse.action.SETTINGS_ACTIVITY";
    /**
     * The Constant EXTRA_KEY_FROM_MMS.
     */
    private static final String EXTRA_KEY_FROM_MMS = "extraKeyFromMms";
    /**
     * The Constant EXTRA_VALUE_FROM_MMS.
     */
    private static final String EXTRA_VALUE_FROM_MMS = "extraValueFromMms";
    /**
     * The Constant PACKAGE_SOUND_RECORDER.
     */
    private static final String PACKAGE_SOUND_RECORDER = "com.android.soundrecorder";
    /**
     * The Constant CLASS_SOUND_RECORDER.
     */
    private static final String CLASS_SOUND_RECORDER = "com.android.soundrecorder.SoundRecorder";

    private static IpMessageActivitiesManager sIpMessageActivitiesManager;

    public static synchronized IpMessageActivitiesManager getInstance(Context context) {
        if (sIpMessageActivitiesManager == null) {
            sIpMessageActivitiesManager = new IpMessageActivitiesManager(context);
        }
        return sIpMessageActivitiesManager;
    }

    /**
     * Instantiates a new ip message activities manager.
     *
     * @param context the context
     */
    private IpMessageActivitiesManager(Context context) {
    }
    /**
     *  Start remote activity of RCSe.
     *
     * @param context the context
     * @param intent the intent
     */
    public void startRemoteActivity(Context context, Intent intent) {
        Logger.d(TAG, "startRemoteActivity() entry intent is " + intent);
        String actionStr = intent.getAction();
        if (RemoteActivities.CONTACT.equals(actionStr)) {
            int type = intent.getIntExtra(RemoteActivities.KEY_TYPE, 0);
            String[] contacts = intent
                    .getStringArrayExtra(RemoteActivities.KEY_ARRAY);
            ArrayList<Participant> originalContacts = null;
            if (null != contacts && contacts.length > 0) {
                originalContacts = new ArrayList<Participant>();
                for (String contact : contacts) {
                    if (PluginUtils.getMessagingMode() == 0) {
                        if (contact.startsWith(IpMessageConsts.JOYN_START)) {
                            contact = contact.substring(4);
                        }
                    }
                    originalContacts.add(new Participant(contact, contact));
                }
            }
            Logger.d(TAG,
                    "startRemoteActivity the action is CONTACT_SELECT type: "
                            + type + " , originalContacts: " + originalContacts);
            startContactSelectionActivity(type, context, originalContacts);
        } else if (RemoteActivities.CHAT_DETAILS_BY_THREAD_ID.equals(actionStr)) {
            Logger.d(TAG,
                    "startRemoteActivity the action is chat detail by thread id");
            long threadId = intent.getLongExtra(RemoteActivities.KEY_THREAD_ID,
                    -1);
            String contact = PluginGroupChatWindow.getContactByThreadId(
                    context, threadId);
            if (PluginGroupChatWindow.isGroupChatInvitation(contact)) {
                Logger.d(TAG, "startRemoteActivity it is a group chat invite"
                        + ", contact is " + contact);
                startRemoteInvitationDialog(context, intent, threadId, contact);
            } else {
                Logger.d(TAG, "startRemoteActivity it is a normal group chat"
                        + ", contact is " + contact);
                PluginChatWindowManager.startGroupChatDetailActivity(threadId,
                        context);
            }
        } else if (RemoteActivities.SYSTEM_SETTINGS.equals(actionStr)) {
            Logger.d(TAG, "startRemoteActivity() action is SYSTEM_SETTINGS");
            startSettingsActivity(context);
        } else if (RemoteActivities.MEDIA_DETAIL.equals(actionStr)) {
            long msgId = intent.getLongExtra(RemoteActivities.KEY_MESSAGE_ID, 0);
            IpMessage message = IpMessageManager.getMessage( msgId);
            if (message != null && message instanceof IpAttachMessage) {
                PluginUtils.onViewFileDetials(
                        ((IpAttachMessage) message).getPath(), context);
            } else {
                Logger.w(TAG,
                        "startRemoteActivity(), MEDIA_DETAIL action, not a attach message!");
            }
        } else if (RemoteActivities.AUDIO.equals(actionStr)) {
            int requestCode = intent.getIntExtra(
                    IpMessageConsts.RemoteActivities.KEY_REQUEST_CODE, 0);
            Intent recordintent = new Intent(Intent.ACTION_GET_CONTENT);
            // Recordintent.setType(ContentType.AUDIO_AMR);
            recordintent.setClassName(PACKAGE_SOUND_RECORDER,
                    CLASS_SOUND_RECORDER);
            ((Activity) context).startActivityForResult(recordintent,
                    requestCode);
        }
    }
    /**
     * Start remote invitation dialog.
     *
     * @param context the context
     * @param intent the intent
     * @param threadId the thread id
     * @param contact the contact
     */
    private void startRemoteInvitationDialog(Context context, Intent intent,
            long threadId, String contact) {
        Logger.d(TAG, "startRemoteInvitationDialog entry");
        String sessionId = PluginGroupChatWindow.getSessionIdByContact(contact);
        if (null != sessionId) {
            String notifyContent = PluginGroupChatWindow
                    .getGroupChatInvitationInfoInMms(threadId);
            Intent dialogIntent = new Intent();
            dialogIntent.putExtras(intent.getExtras());
            dialogIntent.setAction(InvitationDialog.ACTION);
            dialogIntent.putExtra(InvitationDialog.KEY_STRATEGY,
                    InvitationDialog.STRATEGY_IPMES_GROUP_INVITATION);
            dialogIntent.putExtra(InvitationDialog.SESSION_ID, sessionId);
            dialogIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK
                    | Intent.FLAG_ACTIVITY_NEW_TASK);
            dialogIntent.putExtra(Utils.IS_GROUP_CHAT, true);
            dialogIntent.putExtra(PluginGroupChatWindow.GROUP_CHAT_CONTACT,
                    contact);
            dialogIntent
                    .putExtra(RcsNotification.NOTIFY_CONTENT, notifyContent);
            GroupChat chatSession = PluginGroupChatWindow
                    .getChatSession(sessionId);
            if (null != chatSession) {
                Logger.d(TAG,
                        "startRemoteInvitationDialog chatSession is not null");
                // InstantMessage instantMessage =
                // chatSession.getFirstMessage();
                /*
                 * if (null != instantMessage) { Logger.d(TAG,
                 * "startRemoteInvitationDialog msg is " + instantMessage);
                 * ArrayList<InstantMessage> messages = new
                 * ArrayList<InstantMessage>();
                 * messages.add(instantMessage);
                 * dialogIntent.putParcelableArrayListExtra(
                 * PluginGroupChatWindow.MESSAGES, messages); } else {
                 * Logger.e(TAG, "startRemoteInvitationDialog msg is null");
                 * }
                 */
            } else {
                Logger.d(TAG,
                        "startRemoteInvitationDialog session is null, timeout");
            }
            context.startActivity(dialogIntent);
        } else {
            Logger.e(TAG, "startRemoteInvitationDialog sessionId is null");
        }
    }
    /**
     * Start contact selection activity.
     *
     * @param type the type
     * @param context the context
     * @param originalContact the original contact
     */
    private static void startContactSelectionActivity(int type,
            Context context, ArrayList<Participant> originalContact) {
        if (type == SelectContactType.IP_MESSAGE_USER) {
            Logger.d(TAG, "startContactSelectionActivity the type " + type);
            Intent intentSelect = new Intent(
                    PluginApiManager.RcseAction.SELECT_PLUGIN_CONTACT_ACTION);
            intentSelect.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK
                    | Intent.FLAG_ACTIVITY_NEW_TASK);
            intentSelect.putExtra(ChatScreenActivity.KEY_EXSITING_PARTICIPANTS,
                    originalContact);
            if (null != originalContact) {
                intentSelect.putExtra(
                        SelectContactsActivity.KEY_IS_NEED_ORIGINAL_CONTACTS,
                        true);
            }
            MediatekFactory.getApplicationContext().startActivity(intentSelect);
        } else {
            Logger.w(TAG, "startContactSelectionActivity() unknown type: "
                    + type);
        }
    }
    /**
     * Start settings activity.
     *
     * @param context the context
     */
    private void startSettingsActivity(Context context) {
        Intent intentSettings = new Intent(ACTION_SETTINGS_ACIVITY);
        intentSettings.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intentSettings.putExtra(EXTRA_KEY_FROM_MMS, EXTRA_VALUE_FROM_MMS);
        context.startActivity(intentSettings);
    }
}
