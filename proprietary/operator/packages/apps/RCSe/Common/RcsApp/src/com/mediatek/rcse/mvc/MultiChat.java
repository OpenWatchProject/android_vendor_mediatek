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

import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.preference.PreferenceManager;
import android.telephony.SmsManager;
import android.text.TextUtils;
import android.widget.Toast;


import com.mediatek.rcs.R;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.interfaces.ChatModel.IChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IMultiChatWindow;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.mvc.ChatImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatMessageReceived;
import com.mediatek.rcse.mvc.ModelImpl.ChatMessageSent;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ModelImpl.SentFileTransfer;
import com.mediatek.rcse.mvc.view.SentChatMessage;
import com.mediatek.rcse.provider.RichMessagingDataProvider;
import com.mediatek.rcse.provider.UnregGroupMessageProvider;
import com.mediatek.rcse.provider.UnregMessageProvider;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ContactIdUtils;
import com.mediatek.rcse.service.RcsNotification;

import com.orangelabs.rcs.core.ims.service.ContactInfo;
//import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.provider.messaging.ChatData;
import com.orangelabs.rcs.provider.messaging.MessageData;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.service.PluginApiManager.CapabilitiesChangeListener;
//import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.mediatek.rcse.settings.RcsSettings;
import com.mediatek.rcse.settings.RcsSettings.SelfCapabilitiesChangedListener;
//import com.orangelabs.rcs.utils.PhoneUtils;

import com.google.android.mms.ContentType;
import com.google.android.mms.InvalidHeaderValueException;
import com.google.android.mms.pdu.CharacterSets;
import com.google.android.mms.pdu.EncodedStringValue;
import com.google.android.mms.pdu.GenericPdu;
import com.google.android.mms.pdu.PduBody;
import com.google.android.mms.pdu.PduComposer;
import com.google.android.mms.pdu.PduHeaders;
import com.google.android.mms.pdu.PduParser;
import com.google.android.mms.pdu.PduPart;
import com.google.android.mms.pdu.PduPersister;
import com.google.android.mms.pdu.SendConf;
import com.google.android.mms.pdu.SendReq;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;
import java.util.TreeSet;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import com.gsma.services.rcs.RcsServiceException;
import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.capability.CapabilityService;
import com.gsma.services.rcs.chat.OneToOneChat;
import com.gsma.services.rcs.chat.OneToOneChatListener;
import com.gsma.services.rcs.chat.ChatLog;
//import com.gsma.services.rcs.chat.ChatMessage;
import com.gsma.services.rcs.chat.ChatService;
import com.gsma.services.rcs.chat.ExtendChat;
import com.gsma.services.rcs.chat.ExtendChatListener;
//import com.gsma.services.rcs.chat.ExtendMessage;
//import com.gsma.services.rcs.chat.GeolocMessage;
import com.gsma.services.rcs.chat.GroupChat;
import com.gsma.services.rcs.ft.FileTransfer;
import com.gsma.services.rcs.RcsService;

/**
 * This class is the implementation of a 1-2-1 chat model.
 */
public class MultiChat extends ChatImpl implements
SelfCapabilitiesChangedListener {
    public static final String TAG = "M0CF MultiChat";
    /*
     * public static final int FILETRANSFER_ENABLE_OK = 0; public static final
     * int FILETRANSFER_DISABLE_REASON_NOT_REGISTER = 1; public static final int
     * FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED = 2; public static final
     * int FILETRANSFER_DISABLE_REASON_REMOTE = 3;
     */

    public static final int MAX_PAGER_LENGTH = RcsSettings.getInstance().getMaxPagerContentSize();
    public static final int LOAD_DEFAULT = 20;
    public static final int LOAD_ZERO_SHOW_HEADER = 0;
    public static final int TYPE_CHAT_SYSTEM_MESSAGE = ChatLog.Message.Type.SYSTEM;
    public static final int TYPE_GROUP_CHAT_SYSTEM_MESSAGE = ChatLog.Message.Type.SYSTEM;
    public static final int STATUS_TERMINATED = ChatLog.Message.Content.Status.FAILED.toInt();
    // com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status
    // fileTransferStatus =
    // com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING;

  private CopyOnWriteArrayList<IChatMessage> mAllMessages =
      new CopyOnWriteArrayList<IChatMessage>();
  
  //private MultiChatListener multiChatListener = null;
  
  private ExtendChat mExtendChatImpl = null;

    /**
     * MultiChat Constructor.
     *
     * @param modelImpl
     * .
     * @param chatWindow
     * .
     * @param participant
     *.
     * @param tag
     * .
     */
    public MultiChat(ModelImpl modelImpl, IMultiChatWindow chatWindow,
            Participant participant, Object tag) {
        super(tag);
        Logger.d(TAG, "MultiChat() entry with modelImpl is " + modelImpl
                + " chatWindow is " + chatWindow + " participant is "
                + participant + " tag is " + tag);
        mChatWindow = chatWindow;
        mParticipant = participant;
        mFileTransferController = new FileTransferController();
        RcsSettings rcsSetting = RcsSettings.getInstance();
        if (rcsSetting == null) {
            Logger.d(TAG, "MultiChat() the rcsSetting is null ");
            Context context = ApiManager.getInstance().getContext();
            RcsSettings.createInstance(context);
            rcsSetting = RcsSettings.getInstance();
        }
        rcsSetting.registerSelfCapabilitiesListener(this);
    }

    int mCapabilityTimeoutValue = RcsSettings.getInstance()
    .getMessagingCapbailitiesValidiy();

    /**
     * Set chat window for this chat.
     * .
     * @param chatWindow
     *            The chat window to be set.
     */
    public void setChatWindow(IChatWindow chatWindow) {
        Logger.d(TAG, "setChatWindow entry: mChatWindow = " + mChatWindow
                + ", chatWindow = " + chatWindow);
        super.setChatWindow(chatWindow);
    }

    /**
     * .
     */
    protected void checkAllCapability() {
        Logger.d(TAG,
                "M0CFF checkAllCapability() entry: mFileTransferController = "
                + mFileTransferController + ", mParticipant = "
                + mParticipant);
        final RegistrationApi registrationApi = ApiManager.getInstance()
        .getRegistrationApi();
        try {
            if (mFileTransferController != null) {
                if (registrationApi != null && registrationApi.isRegistered()) {
                    Logger.d(TAG,
                            "M0CFF checkAllCapability() already registered");
                    mFileTransferController.setRegistrationStatus(true);
                    CapabilityService capabilityApi = ApiManager.getInstance()
                    .getCapabilityApi();
                    Logger.v(TAG, "M0CFF checkAllCapability() capabilityApi = "
                            + capabilityApi);
                    if (capabilityApi != null) {
                        Capabilities myCapablities = capabilityApi
                        .getMyCapabilities();
                        if (null != mParticipant) {
                            String contact = mParticipant.getContact();
                            Capabilities remoteCapablities = null;
                            try {
                                remoteCapablities = capabilityApi
                                .getContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(contact));
                                // capabilityApi.requestContactCapabilities(contact);
                            } catch (Exception e) {
                                Logger.d(TAG,
                                "M0CFF checkAllCapability() getContactCapabilities " +
                                "RcsServiceException");
                            }
                            Logger.v(TAG,
                                    "M0CFF checkAllCapability() myCapablities = "
                                    + myCapablities
                                    + ",remoteCapablities = "
                                    + remoteCapablities);
                            if (myCapablities != null) {
                                mFileTransferController
                                .setLocalFtCapability(false);
                                if (myCapablities.isFileTransferSupported()) {
                                    Logger.d(TAG,
                                            "M0CFF checkAllCapability() my capability support " +
                                            "filetransfer");
                                    mFileTransferController
                                    .setLocalFtCapability(true);
                                    if (remoteCapablities != null) {
                                        mFileTransferController
                                        .setRemoteFtCapability(false);
                                        if (remoteCapablities
                                                .isFileTransferSupported()) {
                                            Logger.d(TAG,
                                                    "M0CFF checkAllCapability() participant " +
                                                    "support filetransfer ");
                                            mFileTransferController
                                            .setRemoteFtCapability(true);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                mFileTransferController.setRegistrationStatus(false);
            }
        } catch (RcsServiceException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
        /*if (mFileTransferController != null) {
            mFileTransferController.controlFileTransferIconStatus();
        }*/
    }

    /**
     * Return the IChatWindow.
     * .
     * @return IChatWindow
     */
    public IChatWindow getChatWindow() {
        return mChatWindow;
    }

    /**
     * Return the whole messages.
     * .
     * @return The message list.
     */
    public CopyOnWriteArrayList<IChatMessage> getAllMessages() {
        return mAllMessages;
    }

    /**
     * Clear the 1-2-1 chat history of a particular participant.
     * .
     * @return True if success, else False.
     */
    public boolean clearHistoryForContact() {
        Logger.d(TAG, "clearHistoryForContact() entry: mParticipant = "
                + mParticipant);
        ApiManager instance = ApiManager.getInstance();
        if (instance == null) {
            Logger.d(TAG,
            "clearHistoryForContact(), the ApiManager instance is null");
            return false;
        }
        Context context = instance.getContext();
        if (context == null) {
            Logger.d(TAG,
            "clearHistoryForContact(), the Context instance is null");
            return false;
        }
        if (mParticipant != null) {
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    String contact = mParticipant.getContact();
                    Logger.d(TAG, "mParticipant.getContact() is " + contact);
                    if (RichMessagingDataProvider.getInstance() != null) {
                        RichMessagingDataProvider.createInstance(MediatekFactory
                                .getApplicationContext());
                    }
                    RichMessagingDataProvider.getInstance()
                    .deleteMessagingLogForContact(contact);
                    RichMessagingDataProvider.getInstance()
                    .deleteFtLogForContact(contact);
                }
            });
            clearChatWindowAndList();
            mChatWindow.addLoadHistoryHeader(false);
            Logger.d(TAG, "clearHistoryForContact() exit with true");
            return true;
        }
        Logger.d(TAG, "clearHistoryForContact() exit with false");
        return false;
    }

    @Override
    protected synchronized void onDestroy() {
        mSentMessageManager.onChatDestroy();
        mMessageDepot.removeUnregisteredMessage();
        mAllMessages.clear();
        try {
            ContactsListManager.getInstance().setStrangerList(
                    mParticipant.getContact(), false);
            RcsSettings rcsSetting = RcsSettings.getInstance();
            Logger.v(TAG, "onDestroy(): rcsSetting = " + rcsSetting);
            if (rcsSetting != null) {
                rcsSetting.unregisterSelfCapabilitiesListener(this);
            }
            ChatService chatService = ApiManager.getInstance().getChatApi();
            if (null != chatService) {
                OneToOneChat chatImpl = chatService.getOneToOneChat(ContactIdUtils.createContactIdFromTrustedData(mParticipant.getContact()));
                if (null != chatImpl) {
                    //multiChatListener.destroySelf();
                }
            } else {
                Logger.e(TAG, "getSessionByState() messagingApi is null");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        super.onDestroy();
    }

    /**
     * @return
     */
    protected boolean getRegistrationState() {
        Logger.d(TAG, "MultiChat-getRegistrationState()");
        ApiManager apiManager = ApiManager.getInstance();
        if (apiManager == null) {
            Logger.d(TAG, "getRegistrationState()-The apiManager is null");
            return false;
        } else {
            RegistrationApi registrationApi = apiManager.getRegistrationApi();
            if (registrationApi == null) {
                Logger.d(TAG,
                "getRegistrationState()-The registrationApi is null");
                return false;
            } else {
                return registrationApi.isRegistered();
            }
        }
    }     

    /**
     * @param o2oChatImpl
     * @param content
     * @return
     */
    private ChatMessage sendStandAloneMessage(OneToOneChat o2oChatImpl, String content) {
        try {
            Logger.d(TAG, "ABC sendStandAloneMessage() ");
        	String messageId = null;
             ChatService chatService = ApiManager.getInstance().getChatApi();
             if(mExtendChatImpl == null){
                 mExtendChatImpl = chatService.getExtendO2OChat(ContactIdUtils.createContactIdFromTrustedData(mParticipant.getContact()));
                 /*MultiChatListener newO2OChatListener = new MultiChatListener();
                 chatService.addExtendEventListener(newO2OChatListener);
                 newO2OChatListener.setChatImpl(mExtendChatImpl);               
                 multiChatListener = newO2OChatListener;*/
                 //Logger.d(TAG, "ABC mOne2OneExtendChatListener() " + multiChatListener);
                 
             }
             com.gsma.services.rcs.chat.ChatMessage msg = mExtendChatImpl.sendMessage(content);
        	 //return msg;
             messageId = msg.getId();
            if (!TextUtils.isEmpty(messageId) && mParticipant != null) {
                Date date = new Date();
                
                return new ChatMessage(messageId, mParticipant.getContact(),
                        content, new Date(), true, RcsSettings.getInstance()
                        .getJoynUserAlias());
            } else {
                Logger.e(TAG, "sendStandAloneMessage mParticipant is null or messageId is empty, "
                        + "mParticipant is " + mParticipant + " messageId is "
                        + messageId);
                return null;
            }
        } catch (NullPointerException e) {
            e.printStackTrace();
            return null;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public Set<String> generateContactsList(String contact) {
        String[] splited = null;
        Set<String> result = new TreeSet<String>();

        if (contact.indexOf(',') >= 0) {
            splited = contact.split(",");
        }

        if (splited == null)
            result.add(contact);
        else {
            for (int i = 0; i < splited.length; i++) {
                if (!splited[i].isEmpty())
                    result.add(splited[i]);
            }
        }

        return result;
    }

    /**
     * @param content
     * @return
     */
    private ChatMessage sendStandAloneMessage(String content) {
        try {
            Logger.v(TAG,"ABC sendStandAloneMessage() entry");
            ChatService chatService = ApiManager.getInstance().getChatApi();
            CapabilityService capabilityApi = ApiManager.getInstance()
            .getCapabilityApi();
            if (null != chatService && null != mParticipant) {
                try {
                    if (isCapbilityExpired(mParticipant.getContact())) {
                        Logger.v(TAG,"ABC sendStandAloneMessage() checkAllCapability");
                        checkAllCapability();
                    }
                } catch (Exception e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
                //ExtendChat extendChatImpl = chatService.getExtendO2OChat(ContactIdUtils.createContactIdFromTrustedData(mParticipant.getContact()));
                Set<String> contacts = generateContactsList(mParticipant.getContact());
                ExtendChat extendChatImpl = chatService.getExtendMultiChat(contacts);
                String chatId = extendChatImpl.getExtendChatId();
                Logger.v(TAG,"ABC sendStandAloneMessage() extendChatId" + chatId );
                MultiChat chat = (MultiChat) ModelImpl.getInstance().getChatImpl(chatId);
                if(chat == null){
                    ModelImpl.getInstance().addChatImpl(chatId, this);
                }
                /*if(multiChatListener == null){
                    multiChatListener = new MultiChatListener();
                }*/
                /*if(extendChatImpl == null){
                    List<String> contacts = generateContactsList(mParticipant.getContact());
                    chatService.openMultipleChat(contacts, multiChatListener);
                }*/
                //multiChatListener.setChatImpl(extendChatImpl);
                Logger.v(TAG,"ABC sendStandAloneMessage() send");
                String messageId = null;
                com.gsma.services.rcs.chat.ChatMessage msg =  extendChatImpl.sendMessage(content);
            	//return msg;
                // first message
                return new ChatMessage(msg.getId(), mParticipant.getContact(),
                        content, new Date(), true, RcsSettings.getInstance()
                        .getJoynUserAlias());
            } else {
                Logger.e(TAG, "sendStandAloneMessage() messagingApi is "
                        + chatService + " ,mParticipant is " + mParticipant);
                return null;
            }
        } catch (JoynServiceException e) {
            e.printStackTrace();
            return null;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    /**
     * @param content
     * @return
     */
    private ChatMessage sendRegisteredMessage(String content) {
        return sendStandAloneMessage(content);
    }

    // This is a helper method for auto testing
    /**
     * @param content
     * @param messageTag
     */
    protected void sendMessage(final String content, Integer messageTag) {
        sendMessage(content, messageTag.intValue());
    }

    @Override
    public void sendMessage(final String content, final int messageTag) {
        Logger.w(TAG, "ABC sendMessage() The content is " + content);
        if (messageTag == 0) {
            // messageTag is 0 means will send this meesage via sms
            /*Date date = new Date();
            ChatMessage messageViaSms = new ChatMessage(null, mParticipant.getContact(),
                    content, null, mParticipant.getDisplayName(), RcsService.Direction.OUTGOING.toInt(), date.getTime(), date.getTime(), 0L, 0L, 
                    ChatLog.Message.Content.Status.SENDING.toInt() , ChatLog.Message.Content.ReasonCode.UNSPECIFIED.toInt(), null, true, false);*/
            ChatMessage messageViaSms = new ChatMessage(null,
                    mParticipant.getContact(), content, new Date(), false,
                    RcsSettings.getInstance().getJoynUserAlias());
            if (mChatWindow != null) {
                mChatWindow.addSentMessage(messageViaSms, messageTag);
            }
            return;
        }
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                ChatMessage message = null;
                boolean isSuccess = false;
                String messageId = null;
                    Logger.w(TAG, "sendMessage() send registered message");
                    message = sendRegisteredMessage(content);
                    if (null != message) {
                        // mMessageDepot.storeMessage(content, messageTag);
                        Logger.w(TAG, "sendMessage() send registered pass");
                        isSuccess = true;
                        if (mMessageDeleteTag == messageTag) {
                            Logger.w(TAG, "sendMessage() mMessageSent TRUE"
                                    + messageTag);
                            mMessageSent = 1;
                        }
                    } else {
                        Logger.w(TAG, "sendMessage() message is null");
                        message = mMessageDepot.storeMessage(content,
                                messageTag);
                    }
                /*} else {
                    // Store the unregistered sending messages
                    Logger.w(TAG, "sendMessage() store unregistered message");
                    message = mMessageDepot.storeMessage(content, messageTag);
                }*/

                if (message != null) {
                    messageId = message.getId();
                    Logger.d(TAG, "sendMessage() message id is" + messageId);
                }

                mComposingManager.messageWasSent();
                if (mChatWindow != null) {
                    ISentChatMessage sentMessage = mMessageDepot
                    .checkIsResend(messageTag);
                    if (null == sentMessage) {
                        Logger.w(TAG, "sendMessage() new message");
                        // This is a new message, not a resent one
                        sentMessage = mChatWindow.addSentMessage(message,
                                messageTag);
                        mAllMessages.add(new ChatMessageSent(message));
                    }
                    /**
                     * Added to notify the user that the sip invite is failed.@{
                     */
                    // if (message.isInviteMessage()) {
                    // Logger.w(TAG, "sendMessage() isInviteMessage"); //TODo
                    // check this
                    // mLastISentChatMessageList.add(sentMessage);
                    // }
                    /**
                     * @}
                     */

                    Logger.w(TAG, "sendMessage() sentMessage: " + sentMessage
                            + " isSuccess: " + isSuccess);

                    if (null != sentMessage) {
                        if (isSuccess) {
                            if (sentMessage instanceof SentChatMessage) {
                                Logger.w(TAG,
                                "sendMessage(), is SentMessage, update message");
                               sentMessage.updateMessage(message);
                            }
                            mMessageDepot.updateStoredMessage(messageTag,
                                    sentMessage);
                            mSentMessageManager.onMessageSent(sentMessage);
                            sentMessage.updateStatus(Status.SENDING);
                        } else {
                            Logger.w(TAG, "sendMessage() status failed");
                            // mMessageDepot.addMessageTag(messageTag,messageId);
                            mMessageDepot.updateStoredMessage(messageTag,
                                    sentMessage);
                            sentMessage.updateStatus(Status.FAILED);
                        }
                    }
                }
            }
        };
        Thread currentThread = Thread.currentThread();
        if (currentThread.equals(mWorkerThread)) {
            Logger.w(TAG, "sendMessage() run on worker thread");
            worker.run();
        } else {
            Logger.w(TAG, "sendMessage() post to worker thread");
            mWorkerHandler.post(worker);
        }
    }

    @Override
    protected boolean setIsComposing(boolean isComposing) {
        try {
            ChatService chatService = ApiManager.getInstance().getChatApi();
            Logger.w(TAG, "setIsComposing() isComposing= " + isComposing);
            if (chatService != null) {
                OneToOneChat o2oChatImpl = chatService.getOneToOneChat(ContactIdUtils.createContactIdFromTrustedData(mParticipant
                        .getContact()));
                if (null != o2oChatImpl) {
                    //o2oChatImpl.sendIsComposingEvent(isComposing);
                    return true;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    /**
     * @param chatListener
     */
    private void onChatListenerDestroy(ExtendChatListener chatListener) {
        ChatService chatService = ApiManager.getInstance().getChatApi();
        try {
            OneToOneChat o2oChatImpl = chatService.getOneToOneChat(ContactIdUtils.createContactIdFromTrustedData(mParticipant
                    .getContact()));
            Logger.d(TAG, "onChatListenerDestroy() entry: o2oChatImpl = "
                    + o2oChatImpl);
            /*if (null == o2oChatImpl) {
                Logger.d(TAG, "onChatListenerDestroy() o2oChatImpl is null");
                ((IMultiChatWindow) mChatWindow).setIsComposing(false);
                ((IOne2OneChatWindow) mChatWindow)
                .setRemoteOfflineReminder(false);
            }*/
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * @param chatListener
     */
    private void onExtendChatListenerDestroy(ExtendChatListener chatListener) {
        ChatService chatService = ApiManager.getInstance().getChatApi();
        try {
            //ExtendChat o2oChatImpl = chatService.getExtendMultiChat(mParticipant.getContact());
            /*Logger.d(TAG, "onExtendChatListenerDestroy() entry: o2oChatImpl = "
                    + o2oChatImpl);*/
            /*if (null == o2oChatImpl) {
                Logger.d(TAG, "onExtendChatListenerDestroy() o2oChatImpl is null");
                ((IOne2OneChatWindow) mChatWindow).setIsComposing(false);
                ((IOne2OneChatWindow) mChatWindow)
                .setRemoteOfflineReminder(false);
            }*/
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * This method is normally to handle the received delivery notifications via
     * SIP.
     * .
     * @param messageId
     *            The message id of the delivery notification.
     * @param status
     *            The type of the delivery notification.
     * @param timeStamp
     *            The timeStamp of the delivery notification.
     */
    public void onMessageDelivered(final String messageId, final String status,
            final long timeStamp) {
        Logger.i(TAG, "O2O onMessageDelivered() msgId: " + messageId + " status: " + status);
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                mSentMessageManager.onMessageDelivered(messageId,
                        formatStatus(status), timeStamp);
            }
        };
        Thread currentThread = Thread.currentThread();
        if (currentThread.equals(mWorkerThread)) {
            Logger.i(TAG, "onMessageDelivered() run on worker thread");
            worker.run();
        } else {
            Logger.i(TAG, "onMessageDelivered() post to worker thread");
            mWorkerHandler.post(worker);
        }
    }
    
    /**
     * This method is normally to handle the received delivery notifications via
     * SIP.
     * .
     * @param messageId
     *            The message id of the delivery notification.
     * @param status
     *            The type of the delivery notification.
     * @param timeStamp
     *            The timeStamp of the delivery notification.
     */
    public void onFileDelivered(final String messageId, final String status) {
        Logger.i(TAG, "O2O onFileDelivered() msgId: " + messageId + " status: " + status + ", mChatWindow: " + mChatWindow);
        com.mediatek.rcse.interfaces.ChatView.IFileTransfer msg =
                (com.mediatek.rcse.interfaces.ChatView.IFileTransfer) (mChatWindow
                    .getSentChatMessage(messageId));
            if (msg != null) {
                Logger.i(TAG, "O2O onFileDelivered() msgId: " + messageId);
                if(status.contains("delivered"))
                msg.setStatus(
                        com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.DELIVERED); 
                else{
                    msg.setStatus(
                            com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.DISPLAYED); 
                }
            }
    }

    /**
     * @param message
     */
    private void onReceiveMessage(ChatMessage message) {
        String messageInfo = "messageId:" + message.getId() + " message Text:"
        + message.getContent();
        Logger.d(TAG, "onReceiveMessage() entry, " + messageInfo);
        Context context = MediatekFactory.getApplicationContext();
        //if (message.isDisplayedReportRequested()) {
        if(true) {
            Logger.d(TAG, "onReceiveMessage() DisplayedRequested is true, "
                    + messageInfo);
            if (!mIsInBackground) {
                /*
                 * Mark the received message as displayed when the chat window
                 * is not in background
                 */
                markMessageAsDisplayed(message);
            } else {
                /*
                 * Save the message and will mark it as displayed when the chat
                 * screen resumes
                 */
                MultiChat.this.addUnreadMessage(message);
                /*
                 * Showing notification of a new incoming message when the chat
                 * window is in background
                 */
                RcsNotification.getInstance().onReceiveMessageInBackground(
                        context, mTag, message, null, 0);
            }
        } else {
            if (!mIsInBackground) {
                /*
                 * Mark the received message as read if the chat window is not
                 * in background
                 */
                markMessageAsRead(message);
            } else {
                /*
                 * Save the message and will mark it as read when the activity
                 * resumes
                 */
                MultiChat.this.addUnreadMessage(message);
                /*
                 * Showing notification of a new incoming message when the chat
                 * window is in background
                 */
                RcsNotification.getInstance().onReceiveMessageInBackground(
                        context, mTag, message, null, 0);
            }
        }
        synchronized(this){
            mChatWindow.addReceivedMessage(message, !mIsInBackground);
        }
        mAllMessages.add(new ChatMessageReceived(message));
        //mSentMessageManager.markSendingMessagesDisplayed();
    }

    @Override
    protected void markMessageAsDisplayed(ChatMessage msg) {
            
    }

    /*
    * Callback called when a message is sent to the remote
    * 
    * @param msgId Message ID
    */
   public  void onReportMessageSent(final String msgId){
       Logger.d(TAG, "onReportMessageSent()  session:" + msgId);
       mWorkerHandler.post(new Runnable() {
           @Override
           public void run() {
               mSentMessageManager.onMessageSent(msgId,
                       ISentChatMessage.Status.SENT, 0);
           }
       });
   }
   
   /*
   * Callback called when a message has failed because of some error
   * .
   * @param msgId
   *  Message ID.
   */
   public void onReportMessageFailed(final String msgId , int error) {
          Logger.d(TAG, "onReportFailedMessage()  msgId:" + msgId + " error: " + error);
          try {
              if(error == ChatLog.Message.Content.ReasonCode.FALLBACK.toInt()){
                  Logger.d(TAG, "onReportFailedMessage() fallback");
                  String content = "";
                  if (ApiManager.getInstance() != null){
                      ChatService chatService = ApiManager.getInstance().getChatApi();
                      if(chatService != null){
                          try {
                              com.gsma.services.rcs.chat.ChatMessage msg = chatService.getChatMessage(msgId);
                              content = msg.getContent();
                              Logger.d(TAG, " onReportFailedMessage content "+ content);
                          } catch (Exception e) {
                              e.printStackTrace();
                          }
                      } else {
                          Logger.d(TAG, "chatService is null ");
                      }
                  } else {
                      Logger.d(TAG, "ApiManager is null ");
                  }
                  SendbyGroupMMS(content);
                  
              } else {
                  mWorkerHandler.post(new Runnable() {
                      @Override
                      public void run() {
                          mSentMessageManager.onMessageDelivered(msgId,
                                  ISentChatMessage.Status.FAILED, 0);//set status 
                      }
                  });
              }
          } catch (Exception e) {
              e.printStackTrace();
          }
    }
   
    public void SendbyGroupMMS(final String content){
        Logger.d(TAG, "SendbyGroupMMS() entry:" + content);
        final SmsManager smsManager = SmsManager.getDefault();
        smsManager.getAutoPersisting();
        // MMS config is loaded asynchronously. Wait a bit so it will be loaded.
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
         // Ignore
        }
        
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                Set<String> contacts = generateContactsList(mParticipant.getContact());
                /*for(String participant : contacts) {
                    smsManager.sendTextMessage(participant, null, content, null,
                            null);
                    Logger.d(TAG, "SendbySMS(): just sent" + participant);
                }*/
                final byte[] pdu = buildPdu(MediatekFactory.getApplicationContext(), contacts, content, content);
                // Create local provider file for sending PDU
                final String fileName = "send." + String.valueOf(Math.abs(new Random().nextLong())) + ".dat";
                final File sendFile = new File("/storage/emulated/0/"+fileName);
                sendFile.setReadable(true, false);
                Logger.d(TAG, "SendbyGroupMMS openFile: " + sendFile.getAbsolutePath().toString());
                writePdu(sendFile, pdu);
                Uri contentUri = (new Uri.Builder())
                        .authority("")
                        .path(sendFile.getAbsolutePath().toString())
                        .scheme(ContentResolver.SCHEME_FILE)
                        .build();
                Logger.d(TAG, "SendbyGroupMMS(): Uri: " + contentUri.toString());
                smsManager.sendMultimediaMessage(MediatekFactory.getApplicationContext(), contentUri, null, null, null);
                }
        });
            
    }
    
    private static boolean writePdu(File file, byte[] pdu) {
        FileOutputStream writer = null;
        try {
            writer = new FileOutputStream(file);
            writer.write(pdu);
            return true;
        } catch (final IOException e) {
            return false;
        } finally {
            if (writer != null) {
                try {
                    writer.close();
                } catch (IOException e) {
                }
            }
        }
    }
    
    private byte[] buildPdu(Context context, Set<String> contacts, String subject, String text) {
        Logger.d(TAG, "buildPdu() contacts: " + contacts);
        final SendReq req = new SendReq();
        // From, per spec
        req.setFrom(new EncodedStringValue(RcsSettings.getInstance().getUserProfileImsUserName()));
        // To
        final String[] recipients = new String[contacts.size()];
        int i = 0;
        for(String contact : contacts){
            recipients[i] = contact;
            i++;
        }
        final EncodedStringValue[] encodedNumbers = EncodedStringValue.encodeStrings(recipients);
        if (encodedNumbers != null) {
            req.setTo(encodedNumbers);
        }
        // Subject
        
        req.setSubject(new EncodedStringValue(subject));
        // Date
        req.setDate(System.currentTimeMillis() / 1000);
        // Body
        final PduBody body = new PduBody();
        // Add text part. Always add a smil part for compatibility, without it there
        // may be issues on some carriers/client apps
        //final int size = addImagePart(body, attach_file) + addTextPart(body, text_message, true);
        
        final int size = addMessagePart(body, text, true);
        req.setBody(body);
        // message size
        req.setMessageSize(size);
        // Message class
        req.setMessageClass(PduHeaders.MESSAGE_CLASS_PERSONAL_STR.getBytes());
        // Expiry
        req.setExpiry(7*24*60*60);
        // The following set methods throw InvalidHeaderValueException
        try {
            // Priority
            req.setPriority(PduHeaders.PRIORITY_NORMAL);
            // Delivery report
            req.setDeliveryReport(PduHeaders.VALUE_NO);
            // Read report
            req.setReadReport(PduHeaders.VALUE_NO);
        } catch (InvalidHeaderValueException e) {
            return null;
        }

        return new PduComposer(MediatekFactory.getApplicationContext(), req).make();
    }
    
    private int addMessagePart(PduBody pb, String message, boolean addTextSmil) {
        final String TEXT_PART_FILENAME = "text_0.txt";
        final String sSmilText =
                "<smil>" +
                        "<head>" +
                            "<layout>" +
                                "<root-layout/>" +
                                "<region height=\"100%%\" id=\"Text\" left=\"0%%\" top=\"0%%\" width=\"100%%\"/>" +
                            "</layout>" +
                        "</head>" +
                        "<body>" +
                            "<par dur=\"8000ms\">" +
                                "<text src=\"%s\" region=\"Text\"/>" +
                            "</par>" +
                        "</body>" +
                "</smil>";
        PduPart part = new PduPart();
        part.setCharset(CharacterSets.UTF_8);
        part.setContentType(ContentType.TEXT_PLAIN.getBytes());
        part.setContentLocation(TEXT_PART_FILENAME.getBytes());
        int index = TEXT_PART_FILENAME.lastIndexOf(".");
        String contentId = (index == -1) ? TEXT_PART_FILENAME : TEXT_PART_FILENAME.substring(0, index);
        part.setContentId(contentId.getBytes());
        part.setData(message.getBytes());
        pb.addPart(part);
        if (addTextSmil) {
            String smil = String.format(sSmilText, TEXT_PART_FILENAME);
            addSmilPart(pb, smil);
        }
        return part.getData().length;
            }
            
    private void addSmilPart(PduBody pb, String smil) {
        PduPart smilPart = new PduPart();
        smilPart.setContentId("smil".getBytes());
        smilPart.setContentType(ContentType.APP_SMIL.getBytes());
        smilPart.setContentLocation("smil.xml".getBytes());
        smilPart.setData(smil.getBytes());
        pb.addPart(0, smilPart);
    }

    /**
     * One One chat Listener.
     *
     *//*
    private class MultiChatListener extends ExtendChatListener {

        private static final String TAG = "M0CF MultiChatListener";

        private ExtendChat mCurrentChatImpl = null;

        private String mSessionId = null;
        private String mCallId = null;

        *//**
         * .
         *//*
        public MultiChatListener() {
            if (null != mCurrentChatImpl) {
                Logger.d(TAG, "Constructor() session:" + mSessionId
                        + " Call Id: " + mCallId);
            }
        }

        *//**
         * @param o2oChatImpl
         *//*
        public void setChatImpl(ExtendChat o2oChatImpl) {
            mCurrentChatImpl = o2oChatImpl;
        }

        private void destroySelf() {
            Logger.d(TAG, "destroySelf entry : mCurrentSession = "
                    + mCurrentChatImpl);
            if (null != mCurrentChatImpl) {
                try {
                    Logger.d(TAG, "destroySelf() session id is " + mSessionId
                            + " Call Id: " + mCallId);
                    mCurrentChatImpl.removeEventListener(this);
                } catch (JoynServiceException e) {
                    e.printStackTrace();
                }
                mCurrentChatImpl = null;
            }
            onExtendChatListenerDestroy(this);
        }

        *//**
         * Callback called when a new message has been received.
         * .
         * @param message
         *            Chat message.
         * @see ChatMessage
         *//*
        public void onNewMessage(final ExtendMessage message) {
            Logger.d(TAG, "onNewMessage()   message id:" + message.getId()
                    + " message text:" + message.getContent());
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    onReceiveMessage(message);
                }
            });
        }
 
        *//**
         * Callback called when a message is sent to the remote
         * 
         * @param msgId Message ID
         *//*
        public  void onReportMessageSent(final String msgId){
            Logger.e(TAG, "onReportMessageSent()  session:" + msgId);
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mSentMessageManager.onMessageSent(msgId,
                            ISentChatMessage.Status.SENT, 0);
                }
            });
        }

        *//**
         * Callback called when a message has been delivered to the remote.
         * .
         * @param msgId
         *            Message ID.
         *//*
        public void onReportMessageDelivered(final String msgId, String contact) {
            Logger.e(TAG, "onReportMessageDelivered()  session:" + msgId);
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mSentMessageManager.onMessageDelivered(msgId,
                            ISentChatMessage.Status.DELIVERED, 0);
                }
            });
        }

        *//**
         * Callback called when a message has been displayed by the remote
         * .
         * @param msgId
         *            Message ID.
         *//*
        public void onReportMessageDisplayed(final String msgId, String contact) {
            Logger.d(TAG, "onReportMessageDisplayed()  session:" + msgId);
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mSentMessageManager.onMessageDelivered(msgId,
                            ISentChatMessage.Status.DISPLAYED, 0);
                }
            });
        }
        
        *//**
         * Callback called when a message has been delivered to the remote.
         * .
         * @param msgId
         *            Message ID.
         *  @param timestamp
         *            Time when message was delivered.
         *//*
        public void onReportMessageTimestampDelivered(final String msgId, String contact,final long timestamp) {
            Logger.e(TAG, "onReportMessageDelivered()  session:" + msgId);
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mSentMessageManager.onMessageDelivered(msgId,
                            ISentChatMessage.Status.DELIVERED, timestamp);
                }
            });
        }
        
        *//**
         * Callback called when a message has been displayed by the remote
         * .
         * @param msgId
         *            Message ID.
         * @param timestamp
         *            Time when message was displayed.
         *//*
        public void onReportMessageTimestampDisplayed(final String msgId, String contact,final long timestamp) {
            Logger.d(TAG, "onReportMessageDisplayed()  session:" + msgId);
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mSentMessageManager.onMessageDelivered(msgId,
                            ISentChatMessage.Status.DISPLAYED, timestamp);
                }
            });
        }

        *//**
         * Callback called when a message has failed because of some error
         * .
         * @param msgId
         *  Message ID.
         *//*
        public void onReportMessageFailed(final String msgId , int error, final String statusCode) {
            Logger.e(TAG, "onReportFailedMessage()  session:" + msgId + " Status: " + statusCode);
            try {
                if(RcsSettings.getInstance().isSupportOP07() && statusCode.contains("fallback")){
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mSentMessageManager.onMessageDelivered(msgId,
                                    ISentChatMessage.Status.FALLBACK, 0);
                        }
                    });
                } else {
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mSentMessageManager.onMessageDelivered(msgId,
                                    ISentChatMessage.Status.FAILED, 0);//set status 
                        }
                    });
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        
        *//**
         * Callback called when Invite for a message is failed
         * 
         * @param msgId
         *  Message ID.
         *//*
        public void onReportMessageInviteError(final String msgId , String warningText, boolean isForbidden) {
            Logger.e(TAG, "onReportMessageInviteError()  msgId:" + msgId + " isForbidden: " + isForbidden);
            try{
                if(RcsSettings.getInstance().isSupportOP07() && warningText.contains("fallback")){
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mSentMessageManager.onMessageDelivered(msgId,
                                    ISentChatMessage.Status.FALLBACK, 0);
                        }
                    });
                } else {
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mSentMessageManager.onMessageDelivered(msgId,
                                    ISentChatMessage.Status.FAILED, 0);
                        }
                    });
                }
            } catch(Exception e){
                e.printStackTrace();
            }
            
        }
        
        *//**
         * Callback called when Invite for a message is failed
         * 
         * @param msgId
         *  Message ID.
         *//*
        public void onReportMessageInviteFailed(final String msgId) {
            Logger.e(TAG, "onReportMessageInviteFailed()  session:" + msgId);
            try {
                mWorkerHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mSentMessageManager.onMessageDelivered(msgId,
                                ISentChatMessage.Status.FAILED, 0);
                    }
                });
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        
        *//**
         * Callback called when Invite for a message is forbidden
         * 
         * @param msgId
         *   Message ID.
         *//*
        public void onReportMessageInviteForbidden(final String msgId, final String text) {
            Logger.e(TAG, "onReportMessageInviteForbidden()  session:" + msgId + ": text " + text);
            try {
                mWorkerHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mSentMessageManager.onMessageDelivered(msgId,
                                ISentChatMessage.Status.FAILED, 0);
                    }
                });
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }*/

    private static ISentChatMessage.Status formatStatus(String s) {
        Logger.d(TAG, "formatStatus entry with status: " + s);
        ISentChatMessage.Status status = ISentChatMessage.Status.SENDING;
        if (s == null) {
            return status;
        }
        if (s.equals(ImdnDocument.DELIVERY_STATUS_DELIVERED)) {
            status = ISentChatMessage.Status.DELIVERED;
        } else if (s.equals(ImdnDocument.DELIVERY_STATUS_DISPLAYED)) {
            status = ISentChatMessage.Status.DISPLAYED;
        } else if (s.equals(ImdnDocument.DELIVERY_STATUS_SENT)) {
            status = ISentChatMessage.Status.SENT;
        } else {
            status = ISentChatMessage.Status.FAILED;
        }
        Logger.d(TAG, "formatStatus entry exit");
        return status;
    }

    /**
     * Judge whether participants is duplicated.
     * .
     * @param participant
     *            The participants to be compared.
     * @return True, if participants is duplicated, else false.
     */
    public boolean isDuplicated(Participant participant) {
        return mParticipant.equals(participant);
    }

    @Override
    protected void checkCapabilities() {
        checkAllCapability();
    }

    @Override
    protected void queryCapabilities() {
        checkAllCapability();
    }

    /**
     * Set contact name of sender.
     * .
     * @param contact
     *            The participants to be compared.
     */
    public void setInviteContact(String contact) {
        
    }
    
    public void handleInvitation(GroupChat o2oChatImpl,
            ArrayList<IChatMessage> messages, boolean isAutoAccept) {
        
    }
    
    public void handleInvitation(OneToOneChat o2oChatImpl,
            ArrayList<IChatMessage> messages, boolean isAutoAccept) {
        
    }

    @Override
    protected synchronized void onResume() {
        super.onResume();
        Logger.v(TAG, "onResume() entry");
        RcsNotification.getInstance()
        .cancelFileTransferNotificationWithContact(
                mParticipant.getContact());
        if (isCapbilityExpired(mParticipant.getContact())) {
            Logger.v(TAG, "onResume() checkAllCapabilityA");
            checkAllCapability();
        }
        if(RcsSettings.getInstance().isSupportOP08()){
            try {
                CapabilityService capabilityApi = ApiManager.getInstance().getCapabilityApi();
                capabilityApi.requestContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(mParticipant.mContact));
            } catch (Exception e) {
                e.printStackTrace();
                Logger.d(TAG,
                        "on resume exception");
            }
        }
        Logger.v(TAG, "onResume() exit");
    }

    /**
     * @param contact
     * .
     * @return
     */
    boolean isCapbilityExpired(String contact) {
        CapabilityService capabilityApi = ApiManager.getInstance()
        .getCapabilityApi();
        Logger.v(TAG, "M0CFF isCapbilityExpired() entry" + contact);
        if (capabilityApi != null) {
            Capabilities currentRemoteCapablities = null;
            try {
                currentRemoteCapablities = capabilityApi
                .getContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(contact));
            } catch (Exception e) {
                Logger.d(TAG,
                        "M0CFF isCapbilityExpired() getContactCapabilities RcsServiceException");
            }
            long delta = 0;
            if (currentRemoteCapablities != null) {
                
            } else {
                Logger.v(TAG, "M0CFF isCapbilityExpired() true1");
                return true;
            }
            if (delta >= mCapabilityTimeoutValue || mCapabilityTimeoutValue == 0
                    || delta == 0) {
                Logger.v(TAG, "isCapbilityExpired() true2");
                return true;
            }
        } else {
            Logger.v(TAG, "M0CFF isCapbilityExpired() true3");
            return true;
        }
        Logger.v(TAG, "M0CFF isCapbilityExpired() false");
        return false;
    }

    @Override
    public void onStatusChanged(boolean status) {
        Logger.w(TAG, "onStatusChanged the status is " + status
                + ", mFileTransferController = " + mFileTransferController);
        Logger.d(TAG, "resumeFileSend onStatusChanged");
        if (status) {
            // Re-send the stored unregistered messages only if the status is
            // true
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mMessageDepot.resendFailedMessages();
                }
            });

            if (mFileTransferController != null) {
                mFileTransferController.setRegistrationStatus(true);
                //mFileTransferController.controlFileTransferIconStatus();
            }
            ModelImpl instance = (ModelImpl) ModelImpl.getInstance();
            Logger.d(TAG, "resumeFileSend 03");
            if (instance != null) {
                instance.handleFileResumeAfterStatusChange();
                handleResumePauseReceiveFileTransfer();
                Logger.d(TAG, "resumeFileSend 04");
            }
        } else {
            if (mFileTransferController != null) {
                mFileTransferController.setRegistrationStatus(false);
                //mFileTransferController.controlFileTransferIconStatus();
            }
            ModelImpl instance = (ModelImpl) ModelImpl.getInstance();
            if (!RcsSettings.getInstance().isFtAlwaysOn()) {
                if (instance != null) {
                    instance.handleFileTransferNotAvailable(mTag,
                            FILETRANSFER_DISABLE_REASON_NOT_REGISTER);
                }
                mReceiveFileTransferManager.cancelReceiveFileTransfer();
            }
        }
    }

    /**
     * Get participant of this chat.
     * .
     * @return participant of this chat.
     */
    public Participant getParticipant() {
        return mParticipant;
    }

    @Override
    public void loadChatMessages(int count) {
        Logger.v(TAG, "loadChatMessage entry!");
        Context context;
        if (ApiManager.getInstance() != null) {
            context = ApiManager.getInstance().getContext();
        } else {
            Logger.e(TAG, "ApiManager getInstance is null!");
            return;
        }
        Logger.e(TAG, "Then load history , context = " + context
                + "mChatWindow = " + mChatWindow + "mAllMessages = "
                + mAllMessages);
        if (context != null && mChatWindow != null && mAllMessages != null) {
            QueryHandler queryHandler = new QueryHandler(ApiManager
                    .getInstance().getContext(), mChatWindow, mAllMessages);

            // Do not take the chat terminated messages
            String chatTerminatedExcludedSelection = " AND NOT(("
                + ChatLog.Message.MESSAGE_TYPE + "=="
                + TYPE_CHAT_SYSTEM_MESSAGE + ") AND ("
                + ChatLog.Message.STATUS + "== "
                + STATUS_TERMINATED + "))";

            // Do not take the group chat entries concerning this contact
            chatTerminatedExcludedSelection += " AND NOT("
                + ChatLog.Message.MESSAGE_TYPE + "=="
                + TYPE_GROUP_CHAT_SYSTEM_MESSAGE + ")";

            // take all concerning this contact
            String firstMessageId = null;
            for (int i = 0; i < mAllMessages.size(); i++) {
                IChatMessage firstChatMessage = mAllMessages.get(i);
                if (firstChatMessage != null) {
                    ChatMessage firstMessage = firstChatMessage
                    .getChatMessage();
                    if (firstMessage != null) {
                        firstMessageId = firstMessage.getId();
                        break;
                    }
                }
            }
            queryHandler.startQuery(
                    count,
                    firstMessageId,
                    ChatLog.Message.CONTENT_URI,
                    null,
                    ChatLog.Message.CONTACT
                    + "='"
                    + com.mediatek.rcse.service.Utils
                    .formatNumberToInternational(mParticipant
                            .getContact()) + "'"
                            + chatTerminatedExcludedSelection, null,
                            ChatLog.Message.TIMESTAMP + " DESC");
        }
    }

    /**
     * Query Handler .
     *.
     */
    private static class QueryHandler extends AsyncQueryHandler {
        IChatWindow mWindow;
        CopyOnWriteArrayList<IChatMessage> mAllMessages;

        /**
         * @param context .
         * @param chatWindow .
         * @param allMessages .
         */
        public QueryHandler(Context context, IChatWindow chatWindow,
                CopyOnWriteArrayList<IChatMessage> allMessages) {
            super(context.getContentResolver());
            mWindow = chatWindow;
            mAllMessages = allMessages;
        }

        private boolean judgeUnLoadedHistory(Cursor cursor,
                Object firstMessageId) {
            String firstMessage;
            if (firstMessageId == null) {
                firstMessage = "";
                Logger.d(TAG, "judgeUnLoadedHistory, firstMessageId is null");
            } else {
                firstMessage = firstMessageId.toString();
                Logger.d(TAG, "judgeUnLoadedHistory, firstMessageId = "
                        + firstMessageId);
            }
            if (cursor != null && !cursor.isAfterLast()) {
                do {
                    int messageType = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.DIRECTION));
                    String messageId = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.MESSAGE_ID));
                    if (messageType == RcsService.Direction.INCOMING.toInt()
                            || messageType == RcsService.Direction.OUTGOING.toInt()) {
                        if (null != messageId
                                && !messageId.equals(firstMessage)) {
                            return true;
                        } else {
                            Logger.d(TAG,
                            "judgeUnLoadedHistory, the two message is the same!");
                        }
                    }
                } while (cursor.moveToNext());
            } else {
                Logger.w(TAG,
                "judgeUnLoadedHistory, The cursor is null or cursor is last data!");
            }
            return false;
        }

        @Override
        protected void onQueryComplete(int count, Object loadedId, Cursor cursor) {
            Logger.v(TAG, "onQueryComplete enter!");
            String messageId = null;
            try {
                if (cursor != null && cursor.moveToFirst()) {
                    if (loadedId != null) {
                        findLoadedMessage(cursor, loadedId);
                    }
                    // judge if have history, show the header
                    if (count == 0) {
                        if (judgeUnLoadedHistory(cursor, loadedId)) {
                            mWindow.addLoadHistoryHeader(true);
                        } else {
                            mWindow.addLoadHistoryHeader(false);
                        }
                        return;
                    }
                    loadMessage(cursor, messageId, loadedId, count);
                    // if have next message set header visible
                    if (judgeUnLoadedHistory(cursor, messageId)) {
                        mWindow.addLoadHistoryHeader(true);
                    } else {
                        mWindow.addLoadHistoryHeader(false);
                    }
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        }

        /**
         * Find the message from cursor with a id loadedId and move the cursor
         * to next.
         * .
         * @param cursor
         *            A cursor.
         * @param loadedId
         *            message id.
         * @return True if the message with a loadedId exist in cursor,
         *         otherwise return false.
         */
        private boolean findLoadedMessage(Cursor cursor, Object loadedId) {
            do {
                String id = cursor.getString(cursor
                        .getColumnIndex(ChatLog.Message.MESSAGE_ID));
                if (id != null && id.equals(loadedId)) {
                    if (!cursor.isAfterLast()) {
                        cursor.moveToNext();
                    }
                    return true;
                }
            } while (cursor.moveToNext());
            return false;
        }

        /**
         * Load message from cursor to memory.
         * .
         * @param cursor
         *            A cursor.
         * @param messageId
         *            A message Id, it is a output parameter.
         * @param loadedId
         *            The loaded message Id.
         * @param count
         *            The number of message to be loaded.
         * @return True, always return true.
         */
        private boolean loadMessage(Cursor cursor, String messageId,
                Object loadedId, int count) {
            Logger.v(TAG, "loadMessage(): messageId = " + messageId
                    + ", loadedId = " + loadedId + ", count = " + count);
            int i = 0;
            if (!cursor.isAfterLast()) {
                do {
                    ChatMessage message = null;
                    /*messageId = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.MESSAGE_ID));
                    int messageType = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.DIRECTION));
                    String messageData = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.BODY));
                    int messageStatus = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.MESSAGE_STATUS));
                    Date date = new Date(cursor.getLong(cursor
                            .getColumnIndex(ChatLog.Message.TIMESTAMP)));
                    String remount = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.CONTACT));
                    message = new ChatMessage(messageId, remount, messageData,
                            date, true, RcsSettings.getInstance()
                            .getJoynUserAlias());*/
                    
                    messageId = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.MESSAGE_ID));
                    int messageType = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.DIRECTION));
                    int reasonCode = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.REASON_CODE));
                    int readStatus = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.READ_STATUS));
                    int expiredDelivery = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.EXPIRED_DELIVERY));
                    String messageData = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.CONTENT));
                    int messageStatus = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.STATUS));
                    long timestamp = cursor.getLong(cursor
                            .getColumnIndex(ChatLog.Message.TIMESTAMP));
                    long timestampSent = cursor.getLong(cursor
                            .getColumnIndex(ChatLog.Message.TIMESTAMP_SENT));
                    long timestampDelivered = cursor.getLong(cursor
                            .getColumnIndex(ChatLog.Message.TIMESTAMP_DELIVERED));
                    long timestampDisplayed = cursor.getLong(cursor
                            .getColumnIndex(ChatLog.Message.TIMESTAMP_DISPLAYED));
                    String remote = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.CONTACT));
                    String mimetype = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.MIME_TYPE));
                    String displayName = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.DISPLAY_NAME));
                    String chatId = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.CHAT_ID));
                    message = new ChatMessage(messageId, remote, messageData,
                            new Date(timestamp), true, RcsSettings.getInstance()
                            .getJoynUserAlias());
                    /*message = new ChatMessage(messageId, remote,
                            messageData, mimetype, displayName, messageType, timestamp, timestampSent, timestampDelivered, timestampDisplayed, 
                            messageStatus , reasonCode, chatId, readStatus != 0, expiredDelivery != 0);*/
                    if (messageType == RcsService.Direction.INCOMING.toInt()) {
                        if (messageId.equals(loadedId)) {
                            continue;
                        }
                        // message.setIsHistory(true); //TODo check this
                        mWindow.addReceivedMessage(message, true);
                        mAllMessages.add(0, new ChatMessageReceived(message));
                        i++;
                    } else if (messageType == RcsService.Direction.OUTGOING.toInt()) {
                        if (messageId.equals(loadedId)) {
                            continue;
                        }
                        // message.setIsHistory(true); //TODo check this
                        ISentChatMessage sent = mWindow.addSentMessage(message,
                                -1);
                        mAllMessages.add(0, new ChatMessageSent(message));
                        Logger.d(TAG, "sent = " + sent);
                        sent.updateStatus(getStatusEnum(messageStatus));
                        i++;
                    }
                } while (cursor.moveToNext() && i < count);
            }
            return true;
        }

    }

    // TODo Replace al EventsLog APIs and enums with some new APIs andenums.
    /**
     * Get message status from database.
     * .
     * @return Status enum.
     */
    private static Status getStatusEnum(int status) {
        Logger.w(TAG, "getStatusEnum entry, status = " + status);
        ChatLog.Message.Content.Status statusEnum = ChatLog.Message.Content.Status.valueOf(status);
        /*switch (statusEnum) {
        case ChatLog.Message.Content.Status.DISPLAYED:
            return Status.DISPLAYED;
        case ChatLog.Message.Content.Status.SENT:
            return Status.SENT;
        case ChatLog.Message.Content.Status.RECEIVED:
        case ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED:
        case ChatLog.Message.Content.Status.DELIVERED:
        case ChatLog.Message.Content.Status.UNREAD_REPORT:
            return Status.DELIVERED;
        default:
            return Status.FAILED;
        }*/
        
        if(status == ChatLog.Message.Content.Status.DISPLAYED.toInt()){
            return Status.DISPLAYED;
        } else if (status == ChatLog.Message.Content.Status.SENT.toInt()){
            return Status.SENT;
        } else if (status == ChatLog.Message.Content.Status.RECEIVED.toInt() ||
                status == ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED.toInt() ||
                status == ChatLog.Message.Content.Status.DELIVERED.toInt() ||
                status == ChatLog.Message.Content.Status.UNREAD_REPORT.toInt()){
            return Status.DELIVERED;
        } else {
            return Status.FAILED;
        }
    }

    /**
     * Generate an sent file transfer instance in a specific chat window.
     * .
     * @param filePath
     *            The path of the file to be sent.
     * @param fileTransferTag
     *            The tag of the file to be sent.
     * @return A sent file transfer instance in a specific chat window.
     */
    public SentFileTransfer generateSentFileTransfer(String filePath,
            Object fileTransferTag) {
        return new SentFileTransfer(mTag, (IMultiChatWindow) mChatWindow,
                filePath, mParticipant, fileTransferTag);
    }
        
    public void addUnreadrFTMessage(ChatMessage msg) {
        String msgid = msg.getId();
        Logger.v(TAG, "M0CFF addUnreadrFTMessage msgid: " + msgid);
        if(msgid != null)
        MultiChat.this.addUnreadMessage(msg);
    }

    /**
     * @param fileTransferTag
     * .
     */
    public void handleCancelFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer = mReceiveFileTransferManager
        .findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "M0CFF handleCancelFileTransfer entry(): receiveFileTransfer = "
                + receiveFileTransfer + ", fileTransferTag = "
                + fileTransferTag);
        if (null != receiveFileTransfer) {
            receiveFileTransfer.cancelFileTransfer();
        }
    }

    /**
     * @param fileTransferTag
     * .
     */
    public void handlePauseReceiveFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer = mReceiveFileTransferManager
        .findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "M0CFF handlePauseReceiveFileTransfer entry(): receiveFileTransfer = "
                + receiveFileTransfer + ", fileTransferTag = "
                + fileTransferTag);
        if (null != receiveFileTransfer) {
            Logger.d(TAG, "handlePauseReceiveFileTransfer 1");
            receiveFileTransfer.onPauseReceiveTransfer();
        }
    }

    /**
     * .
     */
    public void handleResumePauseReceiveFileTransfer() {
        Logger.d(TAG, "M0CFF handleResumePauseReceiveFileTransfer 0");
        ReceiveFileTransfer resumeFile = null;
        // resumeFile = mReceiveFileTransferManager
        try {
            resumeFile = mReceiveFileTransferManager.getReceiveTransfer();
        } catch (NullPointerException e) {
            Logger.d(TAG, "M0CFF resumeFileSend exception");
        }
        if (null != resumeFile) {
            Logger.d(TAG, "M0CFF handleResumePauseReceiveFileTransfer 1");
            resumeFile.onResumeReceiveTransfer();
        }
    }

    @Override
    public void onCapabilitiesReceived(String contact, Capabilities capabilities) {
        Logger.w(TAG, "M0CFF onCapabilityChanged() entry the contact is "
                + contact + " capabilities is " + capabilities);
        if (mParticipant == null) {
            Logger.d(TAG, "M0CFF onCapabilityChanged() mParticipant is  null");
            return;
        }
        String participantNumber = mParticipant.getContact();
        Logger.w(TAG, "M0CFF onCapabilityChanged() participantNumbert is "
                + participantNumber);
        if (participantNumber.equals(contact)) {
            Logger.d(TAG,
                    "M0CFF onCapabilityChanged() the participant equals the contact " +
                    "and number is "
                    + contact);
            if (capabilities.isImSessionSupported()
                    || ContactsListManager.getInstance().isLocalContact(
                            participantNumber)
                            || ContactsListManager.getInstance().isStranger(
                                    participantNumber)) {
                Logger.w(TAG,
                        "M0CFF onCapabilityChanged() the participant is rcse contact "
                        + contact);
                onCapabilityChangedWhenRemoteIsRcse(contact, capabilities);
            } else {
                Logger.w(TAG,
                        "M0CFF onCapabilityChanged() the participant is not rcse contact "
                        + contact);
                if (!RcsSettings.getInstance().isFtAlwaysOn()) {
                    if (mFileTransferController != null) {
                        mFileTransferController.setRemoteFtCapability(false);
                        //mFileTransferController.controlFileTransferIconStatus();
                    }
                    IChatManager instance = ModelImpl.getInstance();
                    if (instance != null) {
                        ((ModelImpl) instance).handleFileTransferNotAvailable(
                                mTag,
                                FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
                    }
                    mReceiveFileTransferManager.cancelReceiveFileTransfer();
                }
                Logger.w(TAG, "M0CFF mChatWindow = " + mChatWindow);
                if (mChatWindow != null) {
                    /*((IOne2OneChatWindow) mChatWindow)
                    .setRemoteOfflineReminder(false);*/
                }
            }
        }
        Logger.d(TAG, "M0CFF onCapabilityChanged() exit");
    }

    /**
     * Callback called when error is received for a given contact
     *
     * @param contact Contact
     * @param type subscribe type(Capability Polling, Availabilty Fetch)
     * @param status error code
     * @param reason reason of error
     */
    @Override
    public void onErrorReceived(String contact, int type, int status, String reason){}

    private void onCapabilityChangedWhenRemoteIsRcse(String number,
            Capabilities capabilities) {
        Logger.d(TAG,
                "M0CFF onCapabilityChangedWhenRemoteIsRcse() the participant support rcse "
                + number + ", mFileTransferController = "
                + mFileTransferController);
        if (capabilities.isFileTransferSupported()) {
            Logger.d(TAG,
            "M0CFF onCapabilityChangedWhenRemoteIsRcse() is filetransfersupported");
            if (mFileTransferController != null) {
                mFileTransferController.setRemoteFtCapability(true);
                //mFileTransferController.controlFileTransferIconStatus();
            }
        } else if (!RcsSettings.getInstance().isFtAlwaysOn()) {
            Logger.d(TAG,
            "M0CFF onCapabilityChanged isn't filetransfersupported");
            if (mFileTransferController != null) {
                mFileTransferController.setRemoteFtCapability(false);
                //mFileTransferController.controlFileTransferIconStatus();
            }
            IChatManager instance = ModelImpl.getInstance();
            if (instance != null) {
                ((ModelImpl) instance).handleFileTransferNotAvailable(mTag,
                        FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
            }
            mReceiveFileTransferManager.cancelReceiveFileTransfer();
        }
        int registrationState = ContactInfo.REGISTRATION_STATUS_OFFLINE;
        try {
            registrationState = ApiManager.getInstance().getContactsApi()
                    .getRegistrationState(number);
        } catch (Exception e) {
            // TODO: handle exception
        }
        //Logger.d(TAG, "M0CFF onCapabilityChanged() contact info is " + info);
        //int registrationState = info.getRegistrationState();
        if (ContactInfo.REGISTRATION_STATUS_ONLINE == registrationState
                || RcsSettings.getInstance().isImAlwaysOn()) {
            Logger.w(TAG,
                    "M0CFF onCapabilityChangedWhenRemoteIsRcse() ,the participant "
                    + number + " is online");
            if (mChatWindow != null) {
              /*  ((IOne2OneChatWindow) mChatWindow)
                .setRemoteOfflineReminder(false);*/
            }
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mMessageDepot.resendStoredMessages();
                }
            });
        } else {
            boolean isLocalContact = ContactsListManager.getInstance()
            .isLocalContact(number);
            if (isLocalContact) {
                Logger.w(TAG,
                        "M0CFF onCapabilityChangedWhenRemoteIsRcse() ,the participant "
                        + number + " is offline");
               /* ((IOne2OneChatWindow) mChatWindow)
                .setRemoteOfflineReminder(true);*/
            }
        }
    }

    /**
     * @param stackCapabilities
     * .
     */
    public void onCapabilitiesChangedListener(
            com.orangelabs.rcs.core.ims.service.capability.Capabilities stackCapabilities) {
        Set<String> exts = new HashSet<String>(
                stackCapabilities.getSupportedExtensions());
        Capabilities capabilities = new Capabilities(
                stackCapabilities.isImageSharingSupported(),
                stackCapabilities.isVideoSharingSupported(),
                stackCapabilities.isImSessionSupported(),
                stackCapabilities.isFileTransferSupported(),
                stackCapabilities.isGeolocationPushSupported(),
                stackCapabilities.isIPVoiceCallSupported(),
                stackCapabilities.isIPVideoCallSupported(), exts,
                stackCapabilities.isSipAutomata(),
                stackCapabilities.isFileTransferHttpSupported(),
                stackCapabilities.isRCSContact(),
                stackCapabilities.isIntegratedMessagingMode(),
                stackCapabilities.isCsVideoSupported());

        Logger.d(TAG,
                "M0CFF onCapabilitiesChangedListener() entry : capabilites is "
                + capabilities + ", mFileTransferController = "
                + mFileTransferController);
        if (capabilities == null) {
            return;
        }
        if (capabilities.isFileTransferSupported()) {
            Logger.d(TAG,
            "M0CFF onCapabilitiesChangedListener() self filetransfer support");
            if (mFileTransferController != null) {
                mFileTransferController.setLocalFtCapability(true);
                //mFileTransferController.controlFileTransferIconStatus();
            }
        } else {
            Logger.d(TAG,
            "M0CFF onCapabilitiesChangedListener() self filetransfer not support");
            if (mFileTransferController != null) {
                mFileTransferController.setLocalFtCapability(false);
                //mFileTransferController.controlFileTransferIconStatus();
            }
            ModelImpl instance = (ModelImpl) ModelImpl.getInstance();
            if (!RcsSettings.getInstance().isFtAlwaysOn()) {
                if (instance != null) {
                    instance.handleFileTransferNotAvailable(mTag,
                            FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
                }
                if (mReceiveFileTransferManager != null) {
                    mReceiveFileTransferManager.cancelReceiveFileTransfer();
                }
            }
        }
        Logger.d(TAG, "M0CFF onCapabilitiesChangedListener() exit ");
    }

    private final MessageDepot mMessageDepot = new MessageDepot();

    /**
     * Added to notify the user that the sip invite is failed.@{
     * .
     */
    private final ArrayList<ISentChatMessage> mLastISentChatMessageList =
        new ArrayList<ISentChatMessage>();
    /**
     * @}
     */

    int mMessageSent = 0;
    int mMessageDeleteTag = 0;

    /**
     * The class is used to manage the stored message sent in unregistered
     * status.
     */
    private class MessageDepot {
        public final String tag = "M0CF MessageDepot@" + mTag;
        private static final String WHERE = UnregMessageProvider.KEY_CHAT_ID
        + "=?";
        private static final String WHEREDELETE = UnregMessageProvider.KEY_MESSAGE_TAG
        + "=?";
        private  String[] mSelectionArg = null;

        private final Map<Integer, WeakReference<ISentChatMessage>> mMessageMap =
            new ConcurrentHashMap<Integer, WeakReference<ISentChatMessage>>();

        private final Map<Integer, String> mMessageTagMap = new HashMap<Integer, String>();

        /**
         * Check whether need to add a new message to View.
         * .
         * @param messageTag
         * .
         * @return null if this message is not for resent, otherwise an instance
         *         of ISentChatMessage.
         */
        public ISentChatMessage checkIsResend(int messageTag) {
            Logger.d(tag, "checkIsResend() entry, messageTag: ");
            WeakReference<ISentChatMessage> reference = mMessageMap
            .get(messageTag);
            if (null != reference) {
                ISentChatMessage message = reference.get();
                mMessageMap.remove(messageTag);
                Logger.d(tag, "checkIsResend() message: " + message);
                return message;
            } else {
                return null;
            }
        }

        /**
         * @param messageTag
         * .
         * @return
         */
        public ISentChatMessage getMessageTag(int messageTag) {
            Logger.d(tag, "checkIsResend() entry, messageTag: ");
            WeakReference<ISentChatMessage> reference = mMessageMap
            .get(messageTag);
            if (null != reference) {
                ISentChatMessage message = reference.get();
                mMessageMap.remove(messageTag);
                Logger.d(tag, "checkIsResend() message: " + message);
                return message;
            } else {
                return null;
            }
        }

        /**
         * @param messageTag
         * .
         * @param message
         * .
         */
        public void updateStoredMessage(int messageTag, ISentChatMessage message) {
            Logger.d(tag, "updateStoredMessage() messageTag: " + messageTag);
            mMessageMap.put(messageTag, new WeakReference<ISentChatMessage>(
                    message));
        }

        public void addMessageTag(int messageTag, String message) {
            Logger.d(tag, "addMessageTag() messageTag: " + messageTag + " Id "
                    + message);
            mMessageTagMap.put(messageTag, message);
        }

        /**
         * @param msgId
         * .
         * @return.
         */
        public String getMsgTag(int msgId) {
            Logger.d(tag, "checkIsResend() entry, messageTag: ");
            String messageTag = mMessageTagMap.get(msgId);
            Logger.d(tag, "getMsgTag() message: " + messageTag);
            return messageTag;
        }

        /**
         * @param messageTag
         * .
         * @param msgId
         * .
         */
        public void removeMessageTag(int messageTag, String msgId) {
            Logger.d(tag, "removeMessageTag() messageTag: " + messageTag
                    + " Id " + msgId);
            mMessageTagMap.remove(msgId);
        }

       public void removeMessageId(int messageTag) {
            Logger.d(tag, "removeMessageTag() msgId: " + messageTag);
            try {
                Logger.d(tag, "removeMessageTag() messageTag: " + messageTag);
                ContentResolver resolver = ApiManager.getInstance().getContext().getContentResolver();
              
                 int counts = resolver.delete(UnregMessageProvider.CONTENT_URI,UnregMessageProvider.KEY_MESSAGE_TAG+ "=" + messageTag,null); 
                 Logger.i(tag, "removeMessageTag() Remove " + counts + " messages"); 
              } catch (Exception e) {
                 e.printStackTrace(); 
                 Logger.e(tag,"removeMessageTag() Query exception"); 
              }
            
        }

        /**
         * @param content
         * .
         * @param messageTag
         * .
         * @return.
         */
        public ChatMessage storeMessage(String content, int messageTag) {
            Logger.d(tag, "storeMessage() entry : content =" + content
                    + ",messageTag = " + messageTag);
            if (!TextUtils.isEmpty(content)) {
                ApiManager apiManager = ApiManager.getInstance();
                Logger.d(tag, "storeMessage() : apiManager =" + apiManager);
                Context context = null;
                mSelectionArg = new String[] { mParticipant.getContact() };
                ContentResolver resolver = null;
                if (apiManager != null) {
                    context = apiManager.getContext();
                    Logger.d(tag, "storeMessage() : context =" + context);
                    try {
                        if (context != null && mParticipant != null) {
                            resolver = context.getContentResolver();
                            Date date = new Date();
                            ChatMessage msg = new ChatMessage(
                                    com.mediatek.rcse.service.Utils.generateMessageId(),
                                    mParticipant.getContact(), content,
                                    new Date(), true, RcsSettings.getInstance()
                                    .getJoynUserAlias());
                            /*ChatMessage msg = new ChatMessage(com.mediatek.rcse.service.Utils.generateMessageId(), mParticipant.getContact(),
                                    content, null, mParticipant.getDisplayName(), RcsService.Direction.OUTGOING.toInt(), date.getTime(), date.getTime(), 0L, 0L, 
                                    ChatLog.Message.Content.Status.SENDING.toInt() , ChatLog.Message.Content.ReasonCode.UNSPECIFIED.toInt(), null, true, false);*/
                            ContentValues values = new ContentValues();
                            values.put(UnregMessageProvider.KEY_CHAT_TAG,
                                    mTag.toString());
                            values.put(UnregMessageProvider.KEY_MESSAGE_TAG,
                                    messageTag);
                            values.put(UnregMessageProvider.KEY_MESSAGE,
                                    content);
                            values.put(UnregMessageProvider.KEY_CHAT_ID,
                                    mParticipant.getContact());
                            values.put(UnregMessageProvider.KEY_TIMESTAMP,
                                    new Date().getTime());
                            resolver.insert(UnregMessageProvider.CONTENT_URI,
                                    values);
                            Logger.d(tag,
                            "storeMessage() Store messages while the sender is not registered");
                            return msg;
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                        Logger.d(tag, "storeMessage fail");
                    }
                }
            }
            return null;
        }

        /**
         * Resend stored messages in DB.
         */
        public void resendStoredMessages() {
            Cursor cursor = null;
            try {
                ContentResolver resolver = ApiManager.getInstance()
                .getContext().getContentResolver();
                Logger.w(tag, "resendStoredMessages() entry");
                // Query
                mSelectionArg = new String[] { mParticipant.getContact() };
                cursor = resolver.query(UnregMessageProvider.CONTENT_URI, null,
                        WHERE, mSelectionArg, null);
                if (cursor != null) {
                    int messageIndex = cursor
                    .getColumnIndex(UnregMessageProvider.KEY_MESSAGE);
                    int messageTagIndex = cursor
                    .getColumnIndex(UnregMessageProvider.KEY_MESSAGE_TAG);
                    for (cursor.moveToFirst(); !cursor.isAfterLast(); cursor
                    .moveToNext()) {
                        String message = cursor.getString(messageIndex);
                        final int messageTag = cursor.getInt(messageTagIndex);
                        mMessageSent = 0;
                        mMessageDeleteTag = messageTag;
                        removeMessageId(messageTag);
                        MultiChat.this.sendMessage(message, messageTag);
                        
                        Logger.w(
                                tag,
                                "resendStoredMessages() chat["
                                + cursor.getString(0)
                                + "] send message" + message
                                + " with message tag: " + messageTag + " ,ChatId: " + mParticipant.getContact());
                        if (mMessageSent == 1) {
                            // Delete current cursor
                            Logger.w(tag, "resendStoredMessages() delet"
                                    + messageTag);
                            resolver.delete(UnregMessageProvider.CONTENT_URI,
                                    UnregMessageProvider.KEY_MESSAGE_TAG
                                    + " = " + messageTag, null);
                            mMessageSent = 0;
                            mMessageDeleteTag = 0;
                        }
                    }
                }
            } catch (SQLiteException e) {
                e.printStackTrace();
                Logger.w(tag, "resendStoredMessages() Query exception");
            } catch (NullPointerException e) {
                e.printStackTrace();
                Logger.w(tag, "resendStoredMessages() null exception");
            } finally {
                mMessageSent = 0;
                mMessageDeleteTag = 0;
                if (cursor != null) {
                    cursor.close();
                }
            }
        }
            
            /**
             * Resend stored messages in DB.
             */
            public void resendFailedMessages() {
                Cursor cursor = null;
                try {
                    ContentResolver resolver = ApiManager.getInstance()
                    .getContext().getContentResolver();
                    Logger.w(tag, "resendFailedMessages() entry");
                    // Query
                    mSelectionArg = new String[] { mParticipant.getContact() };
                    cursor = resolver.query(UnregMessageProvider.CONTENT_URI, null,
                            WHERE, mSelectionArg, null);
                    if (cursor != null) {
                        int messageIndex = cursor
                        .getColumnIndex(UnregMessageProvider.KEY_MESSAGE);
                        int messageTagIndex = cursor
                        .getColumnIndex(UnregMessageProvider.KEY_MESSAGE_TAG);
                        for (cursor.moveToFirst(); !cursor.isAfterLast(); cursor
                        .moveToNext()) {
                            String message = cursor.getString(messageIndex);
                            final int messageTag = cursor.getInt(messageTagIndex);
                            mMessageSent = 0;
                            mMessageDeleteTag = messageTag;
                            
                            Logger.w(
                                    tag,
                                    "resendFailedMessages() chat["
                                    + cursor.getString(0)
                                    + "] send message" + message
                                    + " with message tag: " + messageTag + " ,ChatId: " + mParticipant.getContact());
                            
                        }
                    }
                } catch (SQLiteException e) {
                    e.printStackTrace();
                    Logger.w(tag, "resendFailedMessages() Query exception");
                } catch (NullPointerException e) {
                    e.printStackTrace();
                    Logger.w(tag, "resendStoredMessages() null exception");
                } finally {
                    mMessageSent = 0;
                    mMessageDeleteTag = 0;
                    if (cursor != null) {
                        cursor.close();
                    }
                }
            // Delete
            /*
             * try { int counts =
             * resolver.delete(UnregMessageProvider.CONTENT_URI, WHERE,
             * mSelectionArg); Logger.i(tag, "resendStoredMessages() Remove " +
             * counts + " messages"); } catch (SQLiteException e) {
             * e.printStackTrace(); Logger.e(tag,
             * "resendStoredMessages() Query exception"); }
             */
        }

        /**
         * Remove messages from application DB.
         */
        public void removeUnregisteredMessage() {
            try {
                ContentResolver resolver = ApiManager.getInstance()
                .getContext().getContentResolver();
                mSelectionArg = new String[] { mParticipant.getContact() };
                int count = resolver.delete(UnregMessageProvider.CONTENT_URI,
                        WHERE, mSelectionArg);
                Logger.i(tag, "removeUnregisteredMessage() Delete " + count
                        + " unregistered sending messages");
                mMessageMap.clear();
            } catch (NullPointerException e) {
                e.printStackTrace();
                Logger.w(tag, "resendStoredMessages() Query exception");
            }
        }
    }

    @Override
    protected void reloadFileTransfer(final FileStruct fileStruct,
            final int transferDirection, final int messageStatus) {
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG,
                        "M0CFF reloadFileTransfer()->run() entry, file transfer tag: "
                        + fileStruct.mFileTransferTag
                        + "file transfer path: " + fileStruct.mFilePath
                        + " , transferType: " + transferDirection
                        + ", messageStatus: " + messageStatus);
                IFileTransfer fileTransfer = null;
                if (RcsService.Direction.OUTGOING.toInt() == transferDirection) { // TODo
                    // check
                    // this
                    fileTransfer = ((IMultiChatWindow) mChatWindow)
                    .addSentFileTransfer(fileStruct);
                    Logger.d(TAG,
                            "M0CFF reloadFileTransfer(), messageStatus = "
                            + messageStatus);
  if (fileTransfer != null && ( messageStatus == 4 || messageStatus == 7 || messageStatus == 8)) { //TODo check this
                        // check
                        // this
                        fileTransfer
                        .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.
                                Status.FINISHED);
                    } else {
                        fileTransfer
                        .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.
                                Status.FAILED);
                    }
                }
            }
        };
        mWorkerHandler.post(worker);
    }

    @Override
    protected void reloadMessage(final ChatMessage message,
            final int messageType, final int messageStatus, int messagetag,
            final String chatId) {
        final int messageTag = messagetag;
        Logger.d(TAG, "reloadMessage() message " + message.getId() + " " + this
                + " messageStatus is " + messageStatus + "messageTag"
                + messagetag + "chatid " + chatId);
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                Logger.d(
                        TAG,
                        "reloadMessage()->run() entry, message id: "
                        + message.getId() + "message text: "
                        + message.getContent() + " , messageType: "
                        + messageType);
                if (RcsService.Direction.INCOMING.toInt() == messageType
                        && (chatId.equals(message.getRemoteContact()))) {
                    if (mChatWindow != null) {
                        Logger.d(TAG, "reloadMessage() the mchatwindow is "
                                + mChatWindow);
                        if(messageStatus == 2){
                    mChatWindow.addReceivedMessage(message, true);
					} else {
					    Logger.d(TAG, "mReceivedAfterReloadMessage Id: " + message.getId());
					    mReceivedAfterReloadMessage.add(message.getId());
					    mChatWindow.addReceivedMessage(message, false);
					}
                        mChatWindow.addReceivedMessage(message, true);
                        mAllMessages.add(new ChatMessageReceived(message));
                    }
                } else if (RcsService.Direction.OUTGOING.toInt() == messageType
                        && (chatId.equals(message.getRemoteContact()))) {
                    // Check if error condition
                    if (mChatWindow != null) {
                        ISentChatMessage sentMessage = mChatWindow
                        .addSentMessage(message, messageTag);
                        if (sentMessage != null) {
                            sentMessage
                            .updateStatus(getStatusEnum(messageStatus));
                            if (getStatusEnum(messageStatus) == Status.FAILED) {
                                Logger.w(TAG,
                                        "reloadMessage() Failed message add to depot text is"
                                        + message.getContent());
                                mMessageDepot.storeMessage(
                                        message.getContent(), messageTag);
                                mMessageDepot.updateStoredMessage(messageTag,
                                        sentMessage);
                                Logger.w(TAG,
                                        "reloadMessage() Failed message add to depot TAG is"
                                        + messageTag);
                            }
                        } else {
                            Logger.d(TAG,
                                    "reloadMessage() the ISentChatMessage is "
                                    + null);
                        }
                        mAllMessages.add(new ChatMessageSent(message));
                    }
                }
            }
        };
        mWorkerHandler.post(worker);
    }
}
