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

package com.mediatek.rcse.mvc;

import android.content.ContentResolver;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Parcelable;
import android.widget.Toast;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatController.IChatController;
import com.mediatek.rcse.interfaces.ChatModel.IChat1;
import com.mediatek.rcse.mvc.ChatImpl;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.ApiManager;

import com.mediatek.rcs.R;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;

import java.util.ArrayList;
import java.util.List;

/**
 * This class implements IChatController.
 */

public class ControllerImpl implements IChatController {
    private static final String CONTROLLER_THREAD_NAME = "ControllerImpl";

    private static final String TAG = "ControllerImpl";

    private static ControllerImpl sControllerImpl = null;
    private ModelImpl mModel = (ModelImpl) ModelImpl.getInstance();

    /**
     * This class is used to manage the object passed from chat view.
     */
    public static class ChatObjectContainer {
        private Object mChatWindowTag = null;

        private Object mChatMessage = null;

        private ChatObjectContainer(Object tag, Object message) {
            mChatWindowTag = tag;
            mChatMessage = message;
        }
    }

    /**
     * Hanlder for controller.
     *
     */
    private class ControllerHandler extends Handler {
        private ControllerHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Logger.d(TAG, "handleMessage() entry  the Message is " + msg.what);
            try {
                ChatObjectContainer chatObjectContainer = (ChatObjectContainer) msg.obj;
                Object chatWindowTag = chatObjectContainer.mChatWindowTag;
                Object chatMessage = chatObjectContainer.mChatMessage;
                switch (msg.what) {
                    case ChatController.EVENT_SEND_MESSAGE:
                        if (chatMessage instanceof String) {
                            if (chatWindowTag instanceof String) {
                                String contact = (String) chatWindowTag;
                                if(RcsSettings.getInstance().isCPMStandAloneModeSupported()){
                                        handleSendStandAloneMessageByContact(contact,
                                        (String) chatMessage, msg.arg1);
                                } else {
                                handleSendMessageByContact(contact,
                                        (String) chatMessage, msg.arg1);
                                }
                            } else {
                                handleSendMessageByTag(chatWindowTag,
                                        (String) chatMessage, msg.arg1);
                            }
                        }
                    break;
                    case ChatController.EVENT_OPEN_WINDOW:
                        if (chatMessage instanceof Participant
                                || chatMessage instanceof List<?>) {
                            handleOpenChatWindow(chatMessage, chatWindowTag,
                                    (String) (msg.getData().get("chatId")));
                        }
                        break;
                    case ChatController.EVENT_OPEN_MULTI_CHAT_WINDOW:
                        if (chatMessage instanceof Participant
                                || chatMessage instanceof List<?>) {
                            handleOpenMultiChatWindow(chatMessage, chatWindowTag,
                                    (String) (msg.getData().get("chatId")));
                        }
                        break;
                    case ChatController.EVENT_CLOSE_WINDOW:
                        if (chatMessage instanceof Participant) {
                            handleCloseChatWindow(chatMessage);
                        } else {
                            handleCloseChatWindow(chatWindowTag);
                        }
                        break;
                    case ChatController.EVENT_SHOW_WINDOW:
                        if (chatWindowTag instanceof String) {
                            Logger.d(TAG, "handleMessage() message from mms");
                            String contact = (String) chatWindowTag;
                            handleShowChatWindowByContact(contact);
                        } else {
                            handleShowChatWindow(chatWindowTag);
                        }
                        break;
                    case ChatController.EVENT_CHAT_SERVICE_CONNECTED:
                        handleChatServiceConnected();
                        break;
                    case ChatController.EVENT_FILETRANSFER_SERVICE_CONNECTED:
                        handleFileTransferServiceConnected();
                        break;
                    case ChatController.EVENT_CHAT_SERVICE_DISCONNECTED:
                        handleChatServiceDisconnected();
                        break;
                    case ChatController.EVENT_FILETRANSFER_SERVICE_DISCONNECTED:
                        handleFileTransferServiceDisconnected();
                        break;
                    case ChatController.EVENT_REGISTRATION_DISCONNECTED:
                        handleRegistrationDisconnected();
                        break;
                    case ChatController.EVENT_QUERY_CAPABILITY:
                        handleQueryCapability(chatWindowTag);
                        break;
                    case ChatController.EVENT_HIDE_WINDOW:
                        if (chatWindowTag instanceof String) {
                            Logger.d(TAG, "handleMessage() message from mms");
                            String contact = (String) chatWindowTag;
                            handleHideChatWindowByContact(contact);
                        } else {
                            handleHideChatWindow(chatWindowTag);
                        }
                        break;
                    case ChatController.EVENT_GET_CHAT_HISTORY:
                        handleGetChatHistory(chatWindowTag,
                                Integer.parseInt((String) chatMessage));
                        break;
                    case ChatController.EVENT_CLEAR_CHAT_HISTORY:
                        handleClearChatHistory(chatWindowTag);
                        break;
                    case ChatController.EVENT_CLEAR_CHAT_HISTORY_MEMORY_ONLY:
                        handleClearChatHistoryMemory(chatWindowTag);
                        break;
                    case ChatController.EVENT_CLEAR_ALL_CHAT_HISTORY_MEMORY_ONLY:
                        handleClearAllChatHistoryMemory();
                        break;
                    case ChatController.EVENT_TEXT_CHANGED:
                        Logger.w(TAG, "ChatController.EVENT_TEXT_CHANGED");
                        IChat1 tempChat = null;
                        if (chatWindowTag instanceof String) {
                            tempChat = mModel
                                    .getOne2oneChatByContact((String) chatWindowTag);
                        } else {
                            tempChat = mModel.getChat(chatWindowTag);
                        }
                        if (chatMessage instanceof Boolean) {
                            Boolean isEmpty = (Boolean) chatMessage;
                            if (tempChat != null) {
                                Logger.w(TAG,
                                        "ChatController.EVENT_TEXT_CHANGED: isempty :"
                                                + isEmpty);
                                tempChat.hasTextChanged(isEmpty);
                            } else {
                                Logger.w(TAG,
                                        "ChatController.EVENT_TEXT_CHANGED: iChat = null");
                            }
                        } else {
                            Logger.e(TAG, "handleMessage the chatMessage : "
                                    + chatMessage + " is not Boolean type");
                        }
                        break;
                    case ChatController.EVENT_GROUP_ADD_PARTICIPANT:
                        handleGroupAddParticipants(chatWindowTag, chatMessage);
                        break;
                    case ChatController.EVENT_GROUP_REMOVE_PARTICIPANT:
                        handleGroupRemoveParticipants(chatWindowTag, chatMessage);
                        break;
                    case ChatController.EVENT_FILE_TRANSFER_INVITATION:
                        // Use chatMessage to get file path from view
                        handleSendFileTransferInvitation(chatWindowTag, chatMessage,
                                msg.getData());
                        break;
                    case ChatController.EVENT_GROUP_FILE_TRANSFER_INVITATION:
                        // Use chatMessage to get file path from view
                        handleGroupSendFileTransferInvitation(chatWindowTag,
                                chatMessage, msg.getData());
                        break;
                    case ChatController.EVENT_FILE_TRANSFER_RECEIVER_REJECT:
                        handleRejectFileTransfer(chatWindowTag, chatMessage);
                        break;
                    case ChatController.EVENT_FILE_TRANSFER_RECEIVER_ACCEPT:
                        handleAcceptFileTransfer(chatWindowTag, chatMessage);
                        break;
                    case ChatController.EVENT_FILE_TRANSFER_CANCEL:
                        handleCancelFileTransfer(chatWindowTag, chatMessage);
                        break;
                    case ChatController.EVENT_FILE_TRANSFER_RESENT:
                        handleResendFileTransfer(chatMessage);
                        break;
                    case ChatController.EVENT_FILE_TRANSFER_PAUSE:
                        handlePausFileTransfer(chatWindowTag, chatMessage, msg.arg1);
                        break;
                    case ChatController.EVENT_FILE_TRANSFER_RESUME:
                        handleResumFileTransfer(chatWindowTag, chatMessage, msg.arg1);
                        break;
                    case ChatController.EVENT_QUIT_GROUP_CHAT:
                        handleQuitGroupChat(chatWindowTag);
                        break;
                    case ChatController.EVENT_EXPORT_GROUP_CHAT:
                        handleExportGroupChat(chatWindowTag);
                        break;
                    case ChatController.EVENT_CLEAR_EXTRA_MESSAGE_GROUP_CHAT:
                        handleClearExtraMessageGroup(chatWindowTag);
                        break;
                    case ChatController.EVENT_RELOAD_MESSAGE:
                        Logger.d(TAG,
                                "handleMessage() EVENT_RELOAD_MESSAGE chatMessage: "
                                        + chatMessage);
                        if (chatMessage instanceof List) {
                            handleReloadMessages((String) chatWindowTag,
                                    (List<Integer>) chatMessage);
                        }
                        break;
                    case ChatController.EVENT_RELOAD_NEW_MESSAGE:
                        Logger.d(TAG,
                                "handleMessage() EVENT_RELOAD_NEW MESSAGE chatMessage: "
                                        + chatMessage);
                        handleReloadNewMessages();
                        break;
                    case ChatController.ADD_GROUP_SUBJECT:
                        handleAddGroupSubject(chatWindowTag, (String) chatMessage,
                                msg.arg1);
                        break;
                    case ChatController.ADD_CLOSED_GROUP_SUBJECT:
                        handleAddClosedGroupSubject(chatWindowTag, (String) chatMessage,
                                msg.arg1);
                        break;
                    case ChatController.EVENT_CLOSE_ALL_WINDOW:
                        handleCloseAllChat();
                        break;
                    case ChatController.EVENT_CLOSE_ALL_WINDOW_FROM_MEMORY:
                        handleCloseAllChatFromMemory();
                        break;
                    case ChatController.EVENT_DELETE_MESSAGE:
                        handleDeleteMessage(chatWindowTag, (String) chatMessage);
                        break;
                    case ChatController.EVENT_DELETE_MESSAGE_UNREG_MESSAGES:
                        handleDeleteUnregMessage(chatWindowTag, (String) chatMessage,msg.arg1);
                        break;
                    case ChatController.EVENT_DELETE_MESSAGE_LIST:
                        Logger.d(TAG, "handleDelteMessage() entry  the Message is "
                                + chatMessage);
                        handleQuitGroupChat(chatWindowTag);
                        handleDeleteMessageList(chatWindowTag,
                                (List<String>) chatMessage);
                        break;

                    default:
                        break;
                } // end switch
           } catch (Exception e) {
               e.printStackTrace();
        }
        }// end handleMessage
    }

    private void handleQuitGroupChat(Object tag) {
        Logger.d(TAG, "handleQuitGroupChat entry with tag: " + tag);
        mModel.quitGroupChat(tag);
    }

    private void handleExportGroupChat(Object tag) {
        Logger.d(TAG, "handleExportGroupChat entry with tag: " + tag);
        mModel.exportGroupChat(tag);
    }

    private void handleClearExtraMessageGroup(Object tag) {
        Logger.d(TAG, "handleClearExtraMessage entry with tag: " + tag);
        mModel.clearExtraMessageGroup(tag);
    }

    private void handleGroupAddParticipants(Object tag, Object participants) {
        IChat1 chat = mModel.getChat(tag);
        Logger.d(TAG, "handleGroupAddParticipants() tag: " + tag + " chat: "
                + chat);
        if (chat instanceof GroupChat1) {
            GroupChat1 groupChat = ((GroupChat1) chat);
            groupChat.addParticipants((List<Participant>) participants);
        }
    }

    private void handleGroupRemoveParticipants(Object tag, Object participants) {
        IChat1 chat = mModel.getChat(tag);
        Logger.d(TAG, "handleGroupRemoveParticipants() tag: " + tag + " chat: "
                + chat);
        if (chat instanceof GroupChat1) {
            GroupChat1 groupChat = ((GroupChat1) chat);
            groupChat.removeParticipants((List<Participant>) participants);
        }
    }

    private void handleAddGroupSubject(Object tag, String title, int sendInvite) {
        GroupChat1 chat = (GroupChat1) mModel.getChat(tag);
        if (chat instanceof GroupChat1) {
            GroupChat1 groupChat = ((GroupChat1) chat);
            groupChat.addGroupSubject(title, sendInvite);
        }
    }

    private void handleAddClosedGroupSubject(Object tag, String title, int sendInvite) {
        GroupChat1 chat = (GroupChat1) mModel.getChat(tag);
        if (chat instanceof GroupChat1) {
            GroupChat1 groupChat = ((GroupChat1) chat);
            groupChat.addClosedGroupSubject(title, sendInvite);
        }
    }

    private void handleClearChatHistory(Object tag) {
        Logger.d(TAG, "handleClearChatHistory() entry, the tag is: " + tag);
        if (tag == null) {
            ContentResolver contentResolver = ApiManager.getInstance()
                    .getContext().getContentResolver();
            // Clear all the messages in the database
            // contentResolver.delete(RichMessagingData.CONTENT_URI, null,
            // null); // TODo check this
            List<IChat1> list = mModel.listAllChat();
            mModel.clearFileTransferQueue();
            int size = list.size();
            for (int i = 0; i < size; i++) {
                IChat1 chat = list.get(i);
                Logger.d(TAG, "handleClearChatHistory() chat: " + chat
                        + " size: " + size);
                if (chat instanceof One2OneChat) {
                    One2OneChat oneOneChat = ((One2OneChat) chat);
                    oneOneChat.clearHistoryForContact();
                } else if (chat instanceof GroupChat1) {
                    GroupChat1 groupChat = ((GroupChat1) chat);
                    groupChat.clearGroupHistory();
                }

            }
        } else {
            IChat1 chat = mModel.getChat(tag);
            Logger.d(TAG, "handleClearChatHistory() chat: " + chat);
            if (chat instanceof One2OneChat) {
                One2OneChat oneOneChat = ((One2OneChat) chat);
                oneOneChat.clearHistoryForContact();
            } else if (chat instanceof GroupChat1) {
                GroupChat1 groupChat = ((GroupChat1) chat);
                groupChat.clearGroupHistory();
            }
        }
        Logger.d(TAG, "handleClearChatHistory() exit");
    }

    private void handleClearChatHistoryMemory(Object tag) {
        Logger.d(TAG, "handleClearChatHistoryMemory() entry, the tag is: "
                + tag);

        IChat1 chat = mModel.getChat(tag);
        Logger.d(TAG, "handleClearChatHistoryMemory() chat: " + chat);
        if (chat != null) {
            if (chat instanceof GroupChat1) {
                GroupChat1 groupChat = ((GroupChat1) chat);
                groupChat.clearGroupHistoryMemory();
            } else if (chat instanceof One2OneChat) {
                One2OneChat o2OChat = ((One2OneChat) chat);
                o2OChat.clearChatWindowAndList();
            }
        }

        Logger.d(TAG, "handleClearChatHistoryMemory() exit");
    }

    private void handleClearAllChatHistoryMemory() {
        Logger.d(TAG, "handleClearChatHistoryMemory() entry");
        List<IChat1> list = mModel.listAllChat();
        int size = list.size();
        for (int i = 0; i < size; i++) {
            IChat1 chat = list.get(i);
            Logger.d(TAG, "handleClearChatHistory() chat: " + chat + " size: "
                    + size);
            if (chat instanceof One2OneChat) {
                One2OneChat oneOneChat = ((One2OneChat) chat);
                oneOneChat.clearChatWindowAndList();
            } else if (chat instanceof GroupChat1) {
                GroupChat1 groupChat = ((GroupChat1) chat);
                groupChat.clearChatWindowAndList();
            }

        }

        Intent intentReload = new Intent(PluginUtils.ACTION_DB_CHANGE_RELOAD);
        MediatekFactory.getApplicationContext().sendBroadcast(intentReload);

        Logger.d(TAG, "handleClearChatHistoryMemory() exit");
    }

    private void handleSendFileTransferInvitation(Object tag, Object filePath,
            Bundle data) {
        RegistrationApi api = ApiManager.getInstance().getRegistrationApi();
        if (null != api && api.isRegistered()) {
            Parcelable fileTransferTag = (data != null ? data
                    .getParcelable(ModelImpl.SentFileTransfer.KEY_FILE_TRANSFER_TAG)
                    : null);
            mModel.handleSendFileTransferInvitation(tag, (String) filePath,
                    fileTransferTag);
        } else {
            Logger.w(
                    TAG,
                    "handleSendFileTransferInvitation, "
                            + "api is null or not registered, registertion status is null");
            Toast.makeText(ApiManager.getInstance().getContext(),
                    R.string.file_transfer_off_line, Toast.LENGTH_SHORT).show();
        }
    }

    private void handleGroupSendFileTransferInvitation(Object tag,
            Object filePath, Bundle data) {
        RegistrationApi api = ApiManager.getInstance().getRegistrationApi();
        if (null != api && api.isRegistered()) {
            Parcelable fileTransferTag = (data != null ? data
                    .getParcelable(ModelImpl.SentFileTransfer.KEY_FILE_TRANSFER_TAG)
                    : null);
            mModel.handleGroupSendFileTransferInvitation(tag,
                    (String) filePath, fileTransferTag);
        } else {
            Logger.w(
                    TAG,
                    "handleSendFileTransferInvitation, "
                            + "api is null or not registered, registertion status is null");
            Toast.makeText(ApiManager.getInstance().getContext(),
                    R.string.file_transfer_off_line, Toast.LENGTH_SHORT).show();
        }
    }

    private void handleRejectFileTransfer(Object tag, Object fileTransferTag) {
        IChat1 chat;
        if (tag instanceof String) {
            chat = mModel.getOne2oneChatByContact((String) tag);
        } else {
            chat = mModel.getChat(tag);
        }
        Logger.d(TAG, "handleRejectFileTransferAccept() tag: " + tag
                + " chat: " + chat);
        if (chat instanceof One2OneChat) {
            One2OneChat oneOneChat = ((One2OneChat) chat);
            oneOneChat.handleRejectFileTransfer(fileTransferTag);
        } else if(chat instanceof GroupChat1) {
            GroupChat1 groupChat = ((GroupChat1) chat);
            Logger.d(TAG, "handleRejectFileTransferAccept() group");
            groupChat.handleRejectFileTransfer(fileTransferTag);
        }
        Logger.d(TAG, "handleRejectFileTransferAccept() exit");

    }

    private void handleAcceptFileTransfer(Object tag, Object fileTransferTag) {
        IChat1 chat;
        if (tag instanceof String) {
            chat = mModel.getOne2oneChatByContact((String) tag);
        } else {
            chat = mModel.getChat(tag);
        }
        Logger.d(TAG, "handleAcceptFileTransfer() tag: " + tag + " chat: "
                + chat);
        if (chat instanceof One2OneChat) {
            One2OneChat oneOneChat = ((One2OneChat) chat);
            oneOneChat.handleAcceptFileTransfer(fileTransferTag);
        } else if (chat instanceof GroupChat1) {
            GroupChat1 groupChat = ((GroupChat1) chat);
            groupChat.handleAcceptFileTransfer(fileTransferTag);
        }

    }

    private void handleCancelFileTransfer(Object tag, Object fileTransferTag) {
        mModel.handleCancelFileTransfer(tag, fileTransferTag);
    }

    private void handlePausFileTransfer(Object chatWindowTag,
            Object fileTransferTag, int option) {
        Logger.d(TAG, "handlePausFileTransfer() tag: " + chatWindowTag
                + " fileTransferTag: " + fileTransferTag + "Option:" + option);
        mModel.handlePauseFileTransfer(chatWindowTag, fileTransferTag, option);
    }

    private void handleResumFileTransfer(Object chatWindowTag,
            Object fileTransferTag, int option) {
        Logger.d(TAG, "handlePausFileTransfer() tag: " + chatWindowTag
                + " fileTransferTag: " + fileTransferTag + "Option:" + option);
        mModel.handleResumeFileTransfer(chatWindowTag, fileTransferTag, option);
    }

    private void handleResendFileTransfer(Object fileTransferTag) {
        mModel.handleResendFileTransfer(fileTransferTag);
    }

    private void handleGetChatHistory(Object tag, int count) {
        IChat1 chat = mModel.getChat(tag);
        Logger.d(TAG, "handleGetChatHistory() tag: " + tag + " chat: " + chat);
        if (chat instanceof One2OneChat) {
            One2OneChat oneOneChat = ((One2OneChat) chat);
            oneOneChat.loadChatMessages(count);
        }
    }

    private void handleShowChatWindow(Object tag) {
        ChatImpl chat = (ChatImpl) mModel.getChat(tag);
        Logger.d(TAG, "handleShowChatWindow() tag: " + tag + " chat: " + chat);
        if (null != chat) {
            chat.onResume();
        }
    }

    private void handleShowChatWindowByContact(String contact) {
        ChatImpl chat = (ChatImpl) mModel.getOne2oneChatByContact(contact);
        Logger.d(TAG, "handleShowChatWindowByContact() contact: " + contact
                + " chat: " + chat);
        if (null != chat) {
            chat.onResume();
        }
    }

    private void handleChatServiceConnected() {
        Logger.v(TAG, "handleChatServiceConnected() ");
        mModel.onChatServiceConnected();
    }

    private void handleFileTransferServiceConnected() {
        Logger.v(TAG, "handleFileTransferServiceConnected() ");
        mModel.onFileTransferServiceConnected();
    }

    private void handleChatServiceDisconnected() {
        Logger.v(TAG, "handleChatServiceDisconnected() ");
        mModel.onChatServiceDisconnected();
    }

    private void handleFileTransferServiceDisconnected() {
        Logger.v(TAG, "handleFileTransferServiceDisconnected() ");
        mModel.onFileTransferServiceConnected();
    }

    private void handleRegistrationDisconnected() {
        Logger.d(TAG, "handleRegistrationDisconnected() ");
        mModel.onRegistrationDisconnected();
    }

    private void handleQueryCapability(Object tag) {
        ChatImpl chat = (ChatImpl) mModel.getChat(tag);
        Logger.d(TAG, "handleQueryCapability() tag: " + tag + " chat: " + chat);
        if (null != chat && chat instanceof One2OneChat) {
            ((One2OneChat) chat).checkAllCapability();
        }
    }

    private void handleHideChatWindow(Object tag) {
        ChatImpl chat = (ChatImpl) mModel.getChat(tag);
        Logger.d(TAG, "handleHideChatWindow() tag: " + tag + " chat: " + chat);
        if (null != chat) {
            chat.onPause();
        }
    }

    private void handleHideChatWindowByContact(String contact) {
        ChatImpl chat = (ChatImpl) mModel.getOne2oneChatByContact(contact);
        Logger.d(TAG, "handleHideChatWindowByContact() contact: " + contact
                + " chat: " + chat);
        if (null != chat) {
            chat.onPause();
        }
    }

    private boolean handleSendMessageByTag(Object tag, String message,
            int messageTag) {
        IChat1 chat = mModel.getChat(tag);
        Logger.d(TAG, "handleSendMessageByTag() tag: " + tag + " message: "
                + message + "messageTag: " + messageTag + " chat: " + chat);
        if (null != chat) {
            chat.sendMessage(message, messageTag);
            return true;
        }
        return false;

    }

    private boolean handleSendMessageByContact(String contact, String message,
            int messageTag) {
        Logger.d(TAG, "handleSendMessageByContact() contact: " + contact
                + ", message: " + message);
        IChat1 chat = null;
        if(contact.startsWith("7---"))
        {
            contact = contact.substring(4);
        	chat = mModel.getNewGroupChat(contact);
        }
        if (null != chat) {
            chat.sendMessage(message, messageTag);
            return true;
        }
        chat = mModel.getOne2oneChatByContact(contact);
        if (null != chat) {
            chat.sendMessage(message, messageTag);
            return true;
        }
        return false;
    }

    private boolean handleSendStandAloneMessageByContact(String contact, String message,
            int messageTag) {
        Logger.d(TAG, "handleSendStandAloneMessageByContact() contact: " + contact
                + ", message: " + message);
        IChat1 chat = null;
        if(contact.startsWith("7---"))
        {
        	Logger.d(TAG, "handleSendStandAloneMessageByContact() It is group");
            return handleSendMessageByContact(contact,message, messageTag);
        }
        if (null != chat) {
            chat.sendMessage(message, messageTag);
            return true;
        }
        chat = mModel.getOne2oneChatByContact(contact);
        if (null != chat) {
            chat.sendMessage(message, messageTag);
            return true;
        }
        return false;
    }

    private void handleOpenChatWindow(Object participant, Object tag,
            String chatId) {
        Logger.d(TAG, "handleOpenChatWindow() participant: " + participant);
        List<Participant> participantList = new ArrayList<Participant>();
        if (participant instanceof Participant) {
            participantList.add((Participant) participant);
        } else {
            participantList.addAll((List<Participant>) participant);
        }
        mModel.addChat(participantList, tag, chatId);
    }

    private void handleOpenMultiChatWindow(Object participant, Object tag,
            String chatId) {
        Logger.d(TAG, "handleOpenMultiChatWindow() participant: " + participant);
        List<Participant> participantList = new ArrayList<Participant>();
        participantList.addAll((List<Participant>) participant);
        mModel.addMultiChat(participantList, tag, chatId);
    }

    private void handleCloseChatWindow(Object reference) {
        if (reference instanceof Participant) {
            mModel.removeChatByContact((Participant) reference);
        } else {
            mModel.removeChat(reference);
        }
    }

    private void handleReloadMessages(String tag, List<Integer> messageIds) {
        Logger.d(TAG, "handleReloadMessages() the tag is " + tag
                + " messageIds is " + messageIds);
        mModel.reloadMessages(tag, messageIds);
    }

    private void handleReloadNewMessages() {
        Logger.d(TAG, "handleReloadNewMessages() ");
        mModel.reloadNewMessages();
    }

    private void handleCloseAllChat() {
        Logger.d(TAG, "handleCloseAllChat()");
        mModel.closeAllChat();
    }

    private void handleCloseAllChatFromMemory() {
        Logger.d(TAG, "handleCloseAllChatFromMemory()");
        mModel.closeAllChatFromMemory();
    }

    private void handleDeleteMessage(Object tag, String messageId) {
        try {
            mModel.handleDeleteMessage(tag, messageId);
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }

    private void handleDeleteUnregMessage(Object tag, String messageId, int messageTag) {
        try {
            mModel.handleDeleteMessageByTag(tag, messageId,messageTag);
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }

    private void handleDeleteMessageList(Object tag, List<String> messageId) {
        try {
            mModel.handleDeleteMessageList(tag, messageId);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private Handler mHandler = null;

    private HandlerThread mHandlerThread = null;

    protected ControllerImpl() {
        mHandlerThread = new HandlerThread(CONTROLLER_THREAD_NAME);
        mHandlerThread.start();
        mHandler = new ControllerHandler(mHandlerThread.getLooper());
    }

    /**
     * @return .
     */
    public static ControllerImpl getInstance() {
        Logger.v(TAG, "getInstance() entry");
        if (sControllerImpl == null) {
            sControllerImpl = new ControllerImpl();
        }
        return sControllerImpl;
    }

    @Override
    public void sendMessage(Message message) {
        mHandler.sendMessage(message);
    }

    @Override
    public Message obtainMessage(int eventType, Object tag, Object message) {
        ChatObjectContainer chatObjectManager = new ChatObjectContainer(tag,
                message);
        Message m = mHandler.obtainMessage(eventType,
                (Object) chatObjectManager);
        return m;
    }
}
