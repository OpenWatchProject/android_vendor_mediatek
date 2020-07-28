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

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.location.Address;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.telephony.SmsManager;
import android.widget.Toast;

import com.mediatek.rcse.mvc.ChatImpl.ReceiveFileTransfer;
import com.mediatek.rcse.mvc.GroupChat1.GroupChatImplStatus;
import com.mediatek.rcse.plugin.message.IpMessageConsts;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.PluginProxyActivity;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.ChatScreenWindowContainer;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.activities.widgets.ReceivedFileTransferItemBinder;
import com.mediatek.rcse.activities.widgets.UnreadMessagesContainer;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.api.RegistrationApi.IRegistrationStatusListener;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatModel.IChat1;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.interfaces.ChatModel.IChatMessage;
import com.mediatek.rcse.interfaces.ChatView;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation.Information;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IMultiChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.plugin.message.PluginGroupChatWindow;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.provider.RichMessagingDataProvider;
import com.mediatek.rcse.provider.UnregMessageProvider;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ContactIdUtils;
import com.mediatek.rcse.service.NetworkChangedReceiver;
import com.mediatek.rcse.service.NetworkChangedReceiver.OnNetworkStatusChangedListerner;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.Utils;
import com.mediatek.rcse.provider.UnregGroupMessageProvider;


import com.mediatek.rcs.R;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingError;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.AppSettings;
import com.mediatek.rcse.settings.RcsSettings;


import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.TreeSet;
import java.util.UUID;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.atomic.AtomicReference;


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

import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.RcsServiceException;
import com.gsma.services.rcs.RcsService;
import com.gsma.services.rcs.JoynServiceNotAvailableException;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.capability.CapabilitiesListener;
import com.gsma.services.rcs.capability.CapabilityService;
import com.gsma.services.rcs.chat.OneToOneChat;
import com.gsma.services.rcs.chat.ChatIntent;
import com.gsma.services.rcs.chat.OneToOneChatIntent;
import com.gsma.services.rcs.chat.GroupChatIntent;
import com.gsma.services.rcs.chat.ChatLog;
//import com.gsma.services.rcs.chat.ChatMessage;
import com.gsma.services.rcs.chat.ChatService;
import com.gsma.services.rcs.chat.GroupChat;
import com.gsma.services.rcs.chat.GroupChatIntent;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.ft.FileTransfer;
import com.gsma.services.rcs.ft.FileTransferIntent;
import com.gsma.services.rcs.ft.OneToOneFileTransferListener;
import com.gsma.services.rcs.ft.GroupFileTransferListener;
import com.gsma.services.rcs.ft.FileTransferLog;
import com.gsma.services.rcs.ft.FileTransferService;
import com.gsma.services.rcs.ft.FileTransfer.Disposition;
import com.gsma.services.rcs.ft.FileTransfer.ReasonCode;
import com.gsma.services.rcs.ft.FileTransfer.State;
import com.gsma.services.rcs.groupdelivery.GroupDeliveryInfo;
import com.gsma.services.rcs.chat.OneToOneChatListener;
import com.gsma.services.rcs.chat.GroupChatListener;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;
import com.gsma.services.rcs.chat.ExtendChatListener;
import com.gsma.services.rcs.chat.ExtendChat;
//import com.gsma.services.rcs.chat.ExtendMessage;

import com.orangelabs.rcs.core.ims.service.im.chat.event.User;

/**
 * This is the virtual Module part in the MVC pattern
 */
public class ModelImpl implements IChatManager {

    public static final String TAG = "M0CF ModelImpl";
    private static final String CONTACT_NAME = "contactDisplayname";
    private static final String INTENT_MESSAGE = "messages";
    private static final String EMPTY_STRING = "";
    private static final String COMMA = ", ";
    private static final String SEMICOLON = ";";
    public static final Timer TIMER = new Timer();
    public static final int INDEXT_ONE = 1;
    public static int sIdleTimeout = 0;
    private ApiReceiver mReceiver = null;
    private IntentFilter mIntentFilter = null;
    private static final IChatManager INSTANCE = new ModelImpl();
    private static One2OneChatListener mOne2OneChatListener = null;
    private static One2OneFileTransferListener mOne2OneFileTransferListener = null;
    private static GroupChatModelListener mGroupChatModelListener = null;
    private static GroupModelFileTransferListener mGroupModelFileTransferListener = null;
    private static One2OneExtendChatListener mOne2OneExtendChatListener = null;
    public static final HandlerThread CHAT_WORKER_THREAD = new HandlerThread(
            "Chat Worker");
    /*protected Handler mWorkerHandler = new Handler(
            CHAT_WORKER_THREAD.getLooper());*/

    protected static final String ANONYMITY_NOT_ALLOWED = "Anonymity not allowed";
    protected static final String FUNCTION_NOT_ALLOWED  = "Function not allowed";
    protected static final String SIZE_EXCEEDED         = "Size Exceeded";
    protected static final String TEST_ERROR            = "Test Error";
    protected static final String NO_DESTINATIONS       = "No destinations";
    protected static final String VERSION_UNSUPPORTED   = "132 Version not supported";
    protected static final String SERVICE_UNAUTHORIZED  = "127 Service not authorized";

    static {
        CHAT_WORKER_THREAD.start();
        AsyncTask<Void, Void, Void> task = new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... arg0) {
                sIdleTimeout = RcsSettings.getInstance()
                        .getIsComposingTimeout() * 1000;
                return null;
            }
        };
        task.execute();
    }

    protected List<String> mReceivedAfterReloadFt = new ArrayList<String>();

    public SentFileTransferManager mOutGoingFileTransferManager = new SentFileTransferManager();

    // The map retains Object&IChat&List<Participant>
    private final Map<Object, IChat1> mChatMap = new HashMap<Object, IChat1>();

    private final Map<String, IChat1> mChatImplMap = new HashMap<String, IChat1>();

    public static final Map<String, Object> sFileTransferImplMap = new HashMap<String, Object>();

    public static IChatManager getInstance() {

        return INSTANCE;
    }

    /**
     * This is the Module constructor in the MVC pattern.
     * .
     */
    public ModelImpl() {
        super();
        if (mReceiver == null && mIntentFilter == null) {
            mReceiver = new ApiReceiver();
            mIntentFilter = new IntentFilter();
            mIntentFilter.addAction(GroupChatIntent.ACTION_NEW_INVITATION); // TODo
                                                                            // check
                                                                            // this
            mIntentFilter.addAction(ChatIntent.ACTION_DELIVERY_STATUS);
            mIntentFilter.addAction(FileTransferIntent.ACTION_DELIVERY_STATUS);
            mIntentFilter.addAction(GroupChatIntent.ACTION_REINVITATION);
            mIntentFilter.addAction(ChatIntent.ACTION_REINITIALIZE_LISTENER);
            MediatekFactory.getApplicationContext().registerReceiver(mReceiver,
                    mIntentFilter);
            /*if (ApiManager.getInstance() != null){
                ChatService chatService = ApiManager.getInstance().getChatApi();
                if(chatService != null){
                    Logger.v(TAG, "ModelImpl addEventListener ");
                    try {
                    chatService.addEventListener(getOne2OneChatListener());
                    chatService.addEventListener(getGroupChatListener());
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    //chatService.addExtendEventListener(getOne2OneExtendChatListener());
                } else {
                    Logger.v(TAG, "ModelImpl chatService is null ");
                }
            } else {
                Logger.v(TAG, "ModelImpl ApiManager is null ");
            }*/
        }
    }

    /**
     * Gets the single instance of One2OneChatListener.
     *
     * @return single instance of One2OneChatListener
     */
    public  synchronized One2OneChatListener getOne2OneChatListener() {
        if (null == mOne2OneChatListener) {
            mOne2OneChatListener = new One2OneChatListener();
        }
        return mOne2OneChatListener;
    }

    /**
     * Gets the single instance of One2OneFileListener.
     *
     * @return single instance of One2OneFileListener
     */
    public  synchronized One2OneFileTransferListener getOne2OneFileListener() {
        if (null == mOne2OneFileTransferListener) {
            mOne2OneFileTransferListener = new One2OneFileTransferListener();
        }
        return mOne2OneFileTransferListener;
    }

    /**
     * Gets the single instance of GroupChatModelListener.
     *
     * @return single instance of GroupChatModelListener
     */
    public  synchronized GroupChatModelListener getGroupChatListener() {
        if (null == mGroupChatModelListener) {
            mGroupChatModelListener = new GroupChatModelListener();
        }
        return mGroupChatModelListener;
    }

    /**
     * Gets the single instance of GroupModelFileTransferListener.
     *
     * @return single instance of GroupModelFileTransferListener
     */
    public  synchronized GroupModelFileTransferListener getGroupFileTransferListener() {
        if (null == mGroupModelFileTransferListener) {
            mGroupModelFileTransferListener = new GroupModelFileTransferListener();
        }
        return mGroupModelFileTransferListener;
    }

    /**
     * Gets the single instance of One2OneExtendChatListener.
     *
     * @return single instance of One2OneExtendChatListener
     */
    public  synchronized One2OneExtendChatListener getOne2OneExtendChatListener() {
        if (null == mOne2OneExtendChatListener) {
            mOne2OneExtendChatListener = new One2OneExtendChatListener();
        }
        return mOne2OneExtendChatListener;
    }

    @Override
    public IChat1 addChat(List<Participant> participants, Object chatTag,
            String chatId) {
        Logger.v(TAG, "addChat() entry, participants: " + participants
                + " chatTag: " + chatTag);
        int size = 0;
        IChat1 chat = null;
        ParcelUuid parcelUuid = null;
        if (chatTag == null) {
            UUID uuid = UUID.randomUUID();
            parcelUuid = new ParcelUuid(uuid);
        } else {
            parcelUuid = (ParcelUuid) chatTag;
        }
        Logger.v(TAG, "addChat() parcelUuid: " + parcelUuid
                + ",participants = " + participants);
        if (ContactsListManager.getInstance() == null) {
            Logger.v(TAG,
                    "addChat() ContactsListManager is null ");
            ContactsListManager.initialize(MediatekFactory
                    .getApplicationContext());
        }
        if (null != participants && participants.size() > 0) {
            size = participants.size();
            if (size > 1) {
                chat = new GroupChat1(this, null, participants, parcelUuid);
                mChatMap.put(parcelUuid, chat);
                mChatImplMap.put(chatId, chat);
                for (int i = 0; i < ((GroupChat1) chat).getParticipantInfos()
                        .size(); i++) {
                    ((GroupChat1) chat).getParticipantInfos().get(i)
                            .setmChatID(chatId);
                }

                IGroupChatWindow chatWindow = ViewImpl.getInstance()
                        .addGroupChatWindow(parcelUuid,
                                ((GroupChat1) chat).getParticipantInfos());
                ((GroupChat1) chat).setChatWindow(chatWindow);
            } else {
                chat = getOne2OneChat(participants.get(0));
                if (null == chat) {
                    Logger.i(TAG, "addChat() The one-2-one chat with "
                            + participants + "doesn't exist.");
                    Participant participant = participants.get(0);
                    chat = new One2OneChat(this, null, participant, parcelUuid);
                    mChatMap.put(parcelUuid, chat);
                    mChatImplMap.put(participant.getContact(), chat);
                    String number = participant.getContact();
                    if(ContactsListManager.getInstance() == null) {
                        ContactsListManager.initialize(MediatekFactory.getApplicationContext());
                    }
                    CapabilityService capabilityApi = null;
                    if (ContactsListManager.getInstance()
                            .isLocalContact(number)
                            || ContactsListManager.getInstance().isStranger(
                                    number)) {
                        Logger.v(TAG,
                                "addChat() the number is local or stranger"
                                        + number);
                        try {
                            capabilityApi = ApiManager.getInstance().getCapabilityApi();
                            capabilityApi.requestContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(number));
                        } catch (Exception e) {
                            e.printStackTrace();
                            Logger.v(TAG,
                                    "addChat() getContactCapabilities" +
                                    " JoynServiceException");
                        }
                    } else {
                        try {
                            capabilityApi = ApiManager
                                    .getInstance().getCapabilityApi();
                            Logger.v(TAG,
                                    "addChat() the number is not local or stranger"
                                            + number + ", capabilityApi= "
                                            + capabilityApi);
                            if (capabilityApi != null) {
                                Capabilities capability = null;
                                try {
                                    capability = capabilityApi
                                            .getContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(number));
                                    capabilityApi.requestContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(number));
                                } catch (RcsServiceException e) {
                                    e.printStackTrace();
                                    Logger.v(TAG,
                                            "addChat() getContactCapabilities" +
                                            " JoynServiceException");
                                }
                                Logger.v(TAG, "capability = " + capability);
                                if (capability != null) {
                                    if (capability.isImSessionSupported()) {
                                        ContactsListManager.getInstance()
                                                .setStrangerList(number, true);
                                    } else {
                                        ContactsListManager.getInstance()
                                                .setStrangerList(number, false);
                                    }
                                }
                            }
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                    IOne2OneChatWindow chatWindow = ViewImpl.getInstance()
                            .addOne2OneChatWindow(parcelUuid,
                                    participants.get(0));
                    ((One2OneChat) chat).setChatWindow(chatWindow);
                } else {
                    Logger.i(TAG, "addChat() The one-2-one chat with "
                            + participants + "has existed.");
                    ViewImpl.getInstance().switchChatWindowByTag(
                            (ParcelUuid) (((One2OneChat) chat).mTag));
                }
            }
        }
        Logger.v(TAG, "addChat(),the chat is " + chat);
        return chat;
    }

    public IChat1 addMultiChat(List<Participant> participants, Object chatTag,
            String chatId) {
        Logger.v(TAG, "addMultiChat() entry, participants: " + participants
                + " chatTag: " + chatTag);

        int size = 0;
        IChat1 chat = null;
        ParcelUuid parcelUuid = null;
        if (chatTag == null) {
            UUID uuid = UUID.randomUUID();
            parcelUuid = new ParcelUuid(uuid);
        } else {
            parcelUuid = (ParcelUuid) chatTag;
        }
        Logger.v(TAG, "addMultiChat() parcelUuid: " + parcelUuid
                + ",participants = " + participants);
        if (ContactsListManager.getInstance() == null) {
            Logger.v(TAG,
                    "addMultiChat() ContactsListManager is null ");
            ContactsListManager.initialize(MediatekFactory
                    .getApplicationContext());
        }
        if (null != participants && participants.size() > 0) {
            size = participants.size();
            String multiContact = generateContactsText(participants);
            Participant multiParticipant = new Participant(multiContact, multiContact);
            chat = null;//getOne2OneChat(multiParticipant);
            if (null == chat) {
                Logger.i(TAG, "addChat() The multi chat with "
                        + participants + "doesn't exist.");
                chat = new MultiChat(this, null, multiParticipant, parcelUuid);
                mChatMap.put(parcelUuid, chat);
                IMultiChatWindow chatWindow = ViewImpl.getInstance()
                        .addMultiChatWindow(parcelUuid,initiateMultiParticipantInfo(participants));
                ((MultiChat) chat).setChatWindow(chatWindow);
            } /*else {
                Logger.i(TAG, "addChat() The multi chat with "
                        + participants + "has existed.");
                ViewImpl.getInstance().switchChatWindowByTag(
                        (ParcelUuid) (((MultiChat) chat).mTag));
            }*/
         }

        Logger.v(TAG, "addChat(),the multi chat is " + chat);
        return chat;
    }

    private List<ParticipantInfo> initiateMultiParticipantInfo(List<Participant> participants) {
        CopyOnWriteArrayList<ParticipantInfo> mParticipants =
                new CopyOnWriteArrayList<ParticipantInfo>();
        if (null != participants && 0 != participants.size()) {
            for (Participant participant : participants) {
                mParticipants.add(new ParticipantInfo(participant,
                        User.STATE_PENDING));
            }
        }
        return mParticipants;
    }

    public String generateContactsText(List<Participant> participants) {
        String result = null;
        List<String> contacts = new ArrayList();

        Iterator it = participants.iterator();
        while(it.hasNext()){
            Participant participant = (Participant)it.next();
            contacts.add(participant.mContact);
        }

        if (contacts != null && contacts.size() >= 1) {
            result = contacts.get(0);
            for (int i = 1; i < contacts.size(); i++) {
                String element = contacts.get(i);
                result += "," + element;
            }
        }

        return result;
    }

    public List<String> generateContactsList(String contact) {
        String[] splited = null;
        ArrayList<String> result = new ArrayList<String>();

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

    private IChat1 addChat(List<Participant> participants) {
        return addChat(participants, null, null);
    }

    public void onChatServiceConnected() {

        Logger.v(TAG, "onChatServiceConnected oneToonechatlistener: " + mOne2OneChatListener +
                " groupchatlistner:" + mGroupChatModelListener  +" mOne2OneExtendChatListener:" + mOne2OneExtendChatListener);

        if (ApiManager.getInstance() != null) {
            ChatService chatService = ApiManager.getInstance().getChatApi();
            if (chatService != null) {
                try {
                    if (mOne2OneChatListener == null) {
                       chatService.addEventListener(getOne2OneChatListener());
                    }
                    if (mGroupChatModelListener == null) {
                       chatService.addEventListener(getGroupChatListener());
                    }
                    if (mOne2OneExtendChatListener == null) {
                        chatService.addExtendEventListener(getOne2OneExtendChatListener());
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                Logger.v(TAG, "ModelImpl chatService is null");
            }
        }
    }

    public void onChatServiceDisconnected() {
        Logger.d(TAG, "onChatServiceDisconnected : " + mOne2OneChatListener
                + " groupchatlistner:" + mGroupChatModelListener
                + " mOne2OneExtendChatListener:" + mOne2OneExtendChatListener);
        mOne2OneChatListener = null;
        mGroupChatModelListener = null;
        mOne2OneExtendChatListener = null;
    }

    public void onFileTransferServiceConnected() {

        Logger.v(TAG, "onFileTransferServiceConnected oneToonefilelistener: " + mOne2OneFileTransferListener +
                " groupfilelistner:" + mGroupModelFileTransferListener);

        if (ApiManager.getInstance() != null) {
            FileTransferService filetransferService = ApiManager.getInstance().getFileTransferApi();
            if (filetransferService != null) {
                try {
                    if (mOne2OneFileTransferListener == null) {
                       filetransferService.addEventListener(getOne2OneFileListener());
                    }
                    if (mGroupModelFileTransferListener == null) {
                        filetransferService.addEventListener(getGroupFileTransferListener());
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                Logger.v(TAG, "ModelImpl filetransferService is null ");
            }
        }
    }

    public void onFileTransferServiceDisconnected() {
        Logger.d(TAG, "onFileTransferServiceDisconnected:"
                + mOne2OneFileTransferListener
                + " groupfilelistner:" + mGroupModelFileTransferListener);
        mOne2OneFileTransferListener = null;
        mGroupModelFileTransferListener = null;
    }

    public void onRegistrationDisconnected() {
        Logger.d(TAG, "onRegistrationDisconnected oneToonechatlistener: " + mOne2OneChatListener +
                " groupchatlistner:" + mGroupChatModelListener);
        mOne2OneChatListener = null;
        mGroupChatModelListener = null;
        mOne2OneExtendChatListener = null;
        mGroupModelFileTransferListener = null;
        mOne2OneFileTransferListener = null;
    }
    /**
     * @param contact
     * .
     * @return .
     */
    public IChat1 getOne2oneChatByContact(String contact) {
        Logger.i(TAG, "ABC getOne2oneChatByContact() contact: " + contact);
        IChat1 chat = null;
        if (TextUtils.isEmpty(contact)) {
            Logger.i(TAG, "ABC getOne2oneChatByContact() return null");
            return chat;
        }
        ArrayList<Participant> participant = new ArrayList<Participant>();
        participant.add(new Participant(contact, contact));
        chat = addChat(participant, null, null);
        Logger.i(TAG, "ABC getOne2oneChatByContact() return NEW CHAT");
        return chat;
    }

    private IChat1 getOne2OneChat(Participant participant) {
        Logger.i(TAG, "getOne2OneChat() entry the participant is "
                + participant);
        Collection<IChat1> chats = mChatMap.values();
        for (IChat1 chat : chats) {
            if (chat instanceof One2OneChat
                    && ((One2OneChat) chat).isDuplicated(participant)) {
                Logger.v(TAG, "getOne2OneChat() find the 1-2-1 chat with "
                        + participant + " has existed");
                return chat;
            }
        }
        Logger.v(TAG, "getOne2OneChat() could not find the 1-2-1 chat with "
                + participant);
        return null;
    }

    private IChat1 getMultiChat(Participant participant) {
        Logger.i(TAG, "getMultiChat() entry the participant is "
                + participant);
        Collection<IChat1> chats = mChatMap.values();
        for (IChat1 chat : chats) {
            if (chat instanceof MultiChat
                    && ((MultiChat) chat).isDuplicated(participant)) {
                Logger.v(TAG, "getMultiChat() find the 1-2-1 chat with "
                        + participant + " has existed");
                return chat;
            }
        }
        Logger.v(TAG, "getMultiChat() could not find the multi chat with "
                + participant);
        return null;
    }

    @Override
    public IChat1 getChat(Object mTag) {
        Logger.v(TAG, "getChat(),tag = " + mTag);
        if (mTag == null) {
            Logger.v(TAG, "tag is null so return null");
            return null;
        }
        if (mChatMap.isEmpty()) {
            Logger.v(TAG, "mChatMap is empty so no chat exist");
            return null;
        }
        if (mTag instanceof ParcelUuid) {
            ParcelUuid tag = new ParcelUuid(UUID.fromString(mTag.toString()));
            Logger.v(TAG, "tgetChat(),tag instanceof ParcelUuid");
            IChat1 chat = mChatMap.get(tag);
            Logger.v(TAG, "return chat = " + chat);
            return chat;
        } else {
            Logger.v(TAG, "tgetChat(),tag not instanceof ParcelUuid");
            IChat1 chat = mChatMap.get(mTag);
            Logger.v(TAG, "return chat = " + chat);
            return chat;
        }
    }

    public IChat1 getChatImpl(String tag) {
        Logger.v(TAG, "getChatImpl(),tag = " + tag);
        if (tag == null) {
            Logger.v(TAG, "tag is null so return null");
            return null;
        }
        if (mChatImplMap.isEmpty()) {
            Logger.v(TAG, "mChatImplMap is empty so no chat exist");
            return null;
        }
        IChat1 chat = mChatImplMap.get(tag);
        Logger.v(TAG, "return chat = " + chat);
        return chat;
    }

    public Object getFileTransferImpl(String transferId) {
        Logger.v(TAG, "getFileTransferImpl(),transferId = " + transferId);
        if (transferId == null) {
            Logger.v(TAG, "transferId is null so return null");
            return null;
        }
        if (sFileTransferImplMap.isEmpty()) {
            Logger.v(TAG, "mFileTransferImplMap is empty so no filetransfer exist");
            return null;
        }
        Object filetransfer = sFileTransferImplMap.get(transferId);
        Logger.v(TAG, "return filetransfer = " + filetransfer);
        return filetransfer;
    }

    public IChat1 getNewGroupChat(Object mTag) {
        Logger.v(TAG, "getNewGroupChat(),tag = " + mTag);
        IChat1 chat = mChatMap.get(mTag);
        Logger.v(TAG, "return chat = " + chat);
        if (chat == null) {
            ParcelUuid tag = new ParcelUuid(UUID.fromString(mTag.toString()));
            chat = mChatMap.get(tag);
        }
        Logger.v(TAG, "getNewGroupChat(),chat = " + chat);
        return chat;
    }

    @Override
    public List<IChat1> listAllChat() {
        List<IChat1> list = new ArrayList<IChat1>();
        if (mChatMap.isEmpty()) {
            Logger.w(TAG, "mChatMap is empty1");
            return list;
        }
        Collection<IChat1> collection = mChatMap.values();
        Iterator<IChat1> iter = collection.iterator();
        while (iter.hasNext()) {
            IChat1 chat = iter.next();
            if (chat != null) {
                Logger.w(TAG, "listAllChat()-IChat1 is empty");
                list.add(chat);
            }
        }
        return list;
    }

    @Override
    public boolean removeChat(Object tag) {
        if (tag == null) {
            Logger.w(TAG, "removeChat()-The tag is null");
            return false;
        }

        ParcelUuid uuid = new ParcelUuid(UUID.fromString(tag.toString()));
        IChat1 chat = mChatMap.remove(uuid);
        if (chat != null) {
            ((ChatImpl) chat).onDestroy();

            // IChat1 chatToRemove = getChat(tag);
            if (chat instanceof One2OneChat) {
                // EventsLogApi eventsLogApi = null;
                if (ApiManager.getInstance() != null) {
                    // eventsLogApi =
                    // ApiManager.getInstance().getEventsLogApi(); //TODo check
                    // this
                    RichMessagingDataProvider.getInstance()
                            .deleteMessagingLogForContact(
                                    ((One2OneChat) chat).getParticipant()
                                            .getContact());
                }
            } else {
                try {
                    RichMessagingDataProvider.getInstance().deleteGroupChat(((GroupChat1) chat).getChatId());
                } catch(Exception e) {
                    e.printStackTrace();
                }
                // for group currently not supported
            }

            mOutGoingFileTransferManager.onChatDestroy(tag);
            ViewImpl.getInstance().removeChatWindow(
                    ((ChatImpl) chat).getChatWindow());
            return true;
        } else {
            Logger.v(TAG, "removeChat()-The chat is null");
            return false;
        }
    }

    /**
     * Remove group chat, but does not close the window associated with the
     * chat.
     * .
     * @param tag
     *            The tag of the group chat to be removed.
     * @return True if success, else false.
     */
    public boolean quitGroupChat(Object tag) {
        Logger.v(TAG, "quitGroupChat() entry, with tag is " + tag);
        if (tag == null) {
            Logger.w(TAG, "quitGroupChat() tag is null");
            return false;
        }
        IChat1 chat = getChat(tag);
        if (chat instanceof GroupChat1) {
            ((GroupChat1) chat).onQuit();
            mOutGoingFileTransferManager.onChatDestroy(tag);
            return true;
        } else {
            Logger.v(TAG, "quitGroupChat() chat is null");
            return false;
        }
    }

    /**
     * export group chat messages
     * .
     * @param tag
     *            The tag of the group chat to be exported.
     * @return True if success, else false.
     */
    public boolean exportGroupChat(Object tag) {
        Logger.v(TAG, "exportGroupChat() entry, with tag is " + tag);
        if (tag == null) {
            Logger.w(TAG, "exportGroupChat() tag is null");
            return false;
        }
        IChat1 chat = getChat(tag);
        if (chat instanceof GroupChat1) {
            ((GroupChat1) chat).initiateExportChat();
            return true;
        } else {
            Logger.v(TAG, "exportGroupChat() chat is null");
            return false;
        }
    }

    /**
     * export group chat messages
     * .
     * @param tag
     *            The tag of the group chat to be exported.
     * @return True if success, else false.
     */
    public boolean clearExtraMessageGroup(Object tag) {
        Logger.v(TAG, "clearExtraMessageGroup() entry, with tag is " + tag);
        if (tag == null) {
            Logger.w(TAG, "clearExtraMessageGroup() tag is null");
            return false;
        }
        IChat1 chat = getChat(tag);
        if (chat instanceof GroupChat1) {
            ((GroupChat1) chat).clearExtraMessageGroup();
            return true;
        } else {
            Logger.v(TAG, "clearExtraMessageGroup() chat is null");
            return false;
        }
    }

    /**
     * when there is not not file transfer capability, cancel all the file
     * transfer.
     * .
     * @param tag
     *            The tag of the chat.
     * @param reason
     *            the reason for file transfer not available.
     */
    public void handleFileTransferNotAvailable(Object tag, int reason) {
        Logger.v(TAG, "handleFileTransferNotAvailable() entry, with tag is "
                + tag + " reason is " + reason);
        if (tag == null) {
            Logger.w(TAG, "handleFileTransferNotAvailable() tag is null");
        } else {
            mOutGoingFileTransferManager.clearTransferWithTag(tag, reason);
        }
    }

    /**
     * @param participant
     * .
     */
    public void removeChatByContact(Participant participant) {
        IChat1 chat = getOne2OneChat(participant);
        Logger.v(TAG, "removeChatByContact(),participant = " + participant
                + ", chat = " + chat);
        if (chat != null) {
            removeChat((ChatImpl) chat);
        }

        // EventsLogApi eventsLogApi = null; //TODo check this
        if (ApiManager.getInstance() != null) {
            // eventsLogApi = ApiManager.getInstance().getEventsLogApi();
            RichMessagingDataProvider.getInstance()
                    .deleteMessagingLogForContact(participant.getContact());
        }
    }

    private boolean removeChat(final ChatImpl chat) {
        if (mChatMap.isEmpty()) {
            Logger.w(TAG, "removeChat()-mChatMap is empty");
            return false;
        }
        if (!mChatMap.containsValue(chat)) {
            Logger.w(TAG, "removeChat()-mChatMap didn't contains this IChat1");
            return false;
        } else {
            Logger.v(TAG, "mchatMap size is " + mChatMap.size());
            mChatMap.values().remove(chat);
            Logger.v(TAG, "After removded mchatMap size is " + mChatMap.size()
                    + ", chat = " + chat);
            if (chat != null) {
                chat.onDestroy();
                mOutGoingFileTransferManager.onChatDestroy(chat.mTag);
                ViewImpl.getInstance().removeChatWindow(chat.getChatWindow());
            }
            return true;
        }
    }

    /**
     * .
     * Clear all the chat messages. Include clear all the messages in date base
     * ,clear messages in all the chat window and clear the last message in each
     * chat list item.
     */
    public void clearAllChatHistory() {
        if (ApiManager.getInstance() != null
                && ApiManager.getInstance().getContext() != null) {
            ContentResolver contentResolver = ApiManager.getInstance()
                    .getContext().getContentResolver();
            if (contentResolver != null) {
                contentResolver.delete(ChatLog.Message.CONTENT_URI, null, null);
            }
            List<IChat1> list = INSTANCE.listAllChat();
            int size = list.size();
            Logger.v(TAG,
                    "clearAllChatHistory(), the size of the chat list is "
                            + size);
            for (int i = 0; i < size; i++) {
                IChat1 chat = list.get(i);
                if (chat != null) {
                    ((ChatImpl) chat).clearChatWindowAndList();
                }
            }
        }
    }

    /**
     * @param tag
     * .
     * @param msgId
     * .
     */
    public void handleDeleteMessage(Object tag, String msgId) {
        Logger.i(TAG, "handleDeleteMessage msgId: " + msgId);
        // delete from window Add window function to delete
        IChat1 chat = ModelImpl.getInstance().getChat(tag);
        if (chat instanceof ChatImpl) {
            try {
                ((ChatImpl) chat).getChatWindow().removeChatMessage(msgId);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        /*if (RichMessagingDataProvider.getInstance() == null) {
            RichMessagingDataProvider.createInstance(MediatekFactory
                    .getApplicationContext());
        }
        // delete message from database
        RichMessagingDataProvider.getInstance().deleteMessage(msgId);
        // delete file from database
        RichMessagingDataProvider.getInstance().deleteFileTranfser(msgId);*/

        if (ApiManager.getInstance() != null){
            ChatService chatService = ApiManager.getInstance().getChatApi();
            if(chatService != null){
                try {
                    chatService.deleteMessage(msgId);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                Logger.v(TAG, "handleDeleteMessage chatService is null ");
            }

            FileTransferService fileTransferService = ApiManager.getInstance()
                    .getFileTransferApi();
            if (fileTransferService != null) {
                try {
                    fileTransferService.deleteFileTransfer(msgId);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                Logger.v(TAG, "handleDeleteMessage filetransferService is null ");
            }
        } else {
            Logger.v(TAG, "handleDeleteMessage ApiManager is null ");
        }
        handleCancelFileTransfer(tag, msgId);
    }

    /**
     * @param tag
     * .
     * @param msgId
     * .
     */
    public void handleDeleteMessageByTag(Object tag, String msgId,int messageTag) {
        Logger.i(TAG, "handleDeleteMessage msgId: " + msgId + ", messageTag: " + messageTag);
        // delete from window Add window function to delete
        IChat1 chat = ModelImpl.getInstance().getChat(tag);
        if (chat instanceof ChatImpl) {
            try {
                ((ChatImpl) chat).getChatWindow().removeChatMessage(msgId);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        if (RichMessagingDataProvider.getInstance() == null) {
            RichMessagingDataProvider.createInstance(MediatekFactory
                    .getApplicationContext());
        }
        // delete message from database
        RichMessagingDataProvider.getInstance().deleteUnregGroupMessage(messageTag);
        // delete file from database
        //RichMessagingDataProvider.getInstance().deleteFileTranfser(msgId);
        //handleCancelFileTransfer(tag, msgId);
    }

    /**
     * @param tag
     * .
     * @param msgIdList
     * .
     */
    public void handleDeleteMessageList(Object tag, List<String> msgIdList) {
        // delete from window Add window function to delete
        Logger.i(TAG, "handleDeleteMessageList size: " + msgIdList.size());
        try {
            IChat1 chat = ModelImpl.getInstance().getChat(tag);
            for (int i = 0; i < msgIdList.size(); i++) {
                try {
                    handleDeleteMessage(tag, msgIdList.get(i));
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Class for Chat Message .
     *.
     */
    private static class ChatMessage1 implements IChatMessage {
        private ChatMessage mChatMessage;

        public ChatMessage1(ChatMessage chatMessage) {
            mChatMessage = chatMessage;
        }

        @Override
        public ChatMessage getChatMessage() {
            return mChatMessage;
        }
    }


    /**
     * Chat List Provider
     *.
     */
    public static class ChatListProvider {

        private Integer mMessageId;
        private String mChatId;

        /**
         * @return .
         */
        public Integer getMessageId() {
            return mMessageId;
        }

        /**
         * @param messageId
         * .
         */
        public void setMessageId(Integer messageId) {
            this.mMessageId = messageId;
        }

        /**
         * @param chatID
         * .
         */
        public void setChatId(String chatID) {
            this.mChatId = chatID;
        }

        /**
         * @return .
         */
        public String getChatID() {
            return mChatId;
        }

        private ArrayList<Participant> mParticipantlist;

        /**
         * @return .
         */
        public ArrayList<Participant> getParticipantlist() {
            return mParticipantlist;
        }

        /**
         * @param participantlist
         * .
         */
        public void setParticipantlist(ArrayList<Participant> participantlist) {
            this.mParticipantlist = participantlist;
        }

    }

    /**
     * @param context
     * .
     * @return .
     */
    public ArrayList<ChatListProvider> getChatListHistory(Context context) {
        ArrayList<ChatListProvider> recentChats = null;
        if (RichMessagingDataProvider.getInstance() == null) {
            RichMessagingDataProvider.createInstance(context);
        }

        RichMessagingDataProvider msgDataProvider = RichMessagingDataProvider
                .getInstance();

        // get the recent chats history
        recentChats = msgDataProvider.getRecentChats();

        return recentChats;
    }


    /**
     * This class used to receive broadcasts from RCS Core Service.
     * .
     */

    private class ApiReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(final Context context, final Intent intent) {
            new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                    Logger.i(TAG, "ABC ModelImpl onReceive ");
                    String action = intent.getAction();
                    Logger.i(TAG, "ABC ModelImpl onReceive acdtion: " + action);
                    if (action == null) {
                        Logger.i(TAG, "ABC ModelImpl onReceive return");
                        return null;
                    }

                    if (OneToOneChatIntent.ACTION_DELIVERY_STATUS
                            .equalsIgnoreCase(action)) {
                        String remoteContact = null;/*intent
                                .getStringExtra(ChatIntent.EXTRA_CONTACT);*/
                        String msgId = intent.getStringExtra(OneToOneChatIntent.EXTRA_MESSAGE_ID);
                        String status = intent.getStringExtra(OneToOneChatIntent.EXTRA_STATUS);
                        if (ApiManager.getInstance() != null){
                            ChatService chatService = ApiManager.getInstance().getChatApi();
                            if(chatService != null){
                                try {
                                    com.gsma.services.rcs.chat.ChatMessage msg = chatService.getChatMessage(msgId);
                                    remoteContact = msg.getRemoteContact().toString();
                                    Logger.v(TAG, " handleMessageDeliveryStatus remote " + remoteContact);
                                } catch (Exception e) {
                                    e.printStackTrace();
                                }
                            } else {
                                Logger.v(TAG, "chatService is null ");
                            }
                        } else {
                            Logger.v(TAG, "ApiManager is null ");
                        }
                        Logger.i(TAG,
                                "ABC handleMessageDeliveryStatus() entry, remoteContact:"
                                        + remoteContact + "Msgid:" + msgId);

                        Logger.i(TAG,
                                "ABC handleMessageDeliveryStatus() entry, remoteContact:"
                                        + remoteContact + "status" + status);
                        // remoteContact =
                        // PhoneUtils.extractNumberFromUri(remoteContact);
                        IChat1 chat = null;
                        chat = getOne2oneChatByContact(remoteContact);
                        if (null != chat) {
                            Logger.i(TAG,
                                    "ABC handleMessageDeliveryStatus() chat found");
                            if (chat instanceof One2OneChat) {
                                ((One2OneChat) chat).onMessageDelivered(msgId,
                                        status, 0);
                            }
                        }
                        /*chat = getGroupChat(null);
                        if (null != chat) {
                            if (chat instanceof GroupChat1) {
                                ((GroupChat1) chat).onMessageDelivered(msgId,status,0);
                            }
                        }*/
                        return null;
                    } else  if (FileTransferIntent.ACTION_DELIVERY_STATUS
                            .equalsIgnoreCase(action)) {
                        String remoteContact = intent.getStringExtra(OneToOneChatIntent.EXTRA_CONTACT);
                        String msgId = intent.getStringExtra(OneToOneChatIntent.EXTRA_MESSAGE_ID);
                        String status = intent.getStringExtra(OneToOneChatIntent.EXTRA_STATUS);
                       /* if (ApiManager.getInstance() != null){
                            ChatService chatService = ApiManager.getInstance().getChatApi();
                            if(chatService != null){
                                try {
                                    com.gsma.services.rcs.chat.ChatMessage msg = chatService.getChatMessage(msgId);
                                    remoteContact = msg.getRemoteContact().toString();
                                    Logger.v(TAG, " handleMessageDeliveryStatus remote "+ remoteContact);
                                } catch (Exception e) {
                                    e.printStackTrace();
                                }
                            } else {
                                Logger.v(TAG, "chatService is null ");
                            }
                        } else {
                            Logger.v(TAG, "ApiManager is null ");
                        }*/
                        Logger.i(TAG,
                                "ABC handleFileDeliveryStatus() entry, remoteContact:"
                                        + remoteContact + "Msgid:" + msgId + ", status:" + status);
                        IChat1 chat = null;
                        chat = getOne2oneChatByContact(remoteContact);
                        if (null != chat) {
                            Logger.i(TAG,
                                    "ABC handleFileDeliveryStatus() chat found");
                            if (chat instanceof One2OneChat) {
                                ((One2OneChat) chat).onFileDelivered(msgId,status);
                            }
                        }
                        /*chat = getGroupChat(null);
                        if (null != chat) {
                            if (chat instanceof GroupChat1) {
                                ((GroupChat1) chat).onFileDelivered(msgId,status);
                            }
                        }*/
                        return null;
                    } else if (GroupChatIntent.ACTION_NEW_INVITATION
                            .equalsIgnoreCase(action)) {
                        try {
                            Logger.v(TAG, "DEF New chat inviation");
                            String groupChatId = intent
                                    .getStringExtra(GroupChatIntent.EXTRA_CHAT_ID);
                            if (ApiManager.getInstance() == null
                                    || ApiManager.getInstance().getChatApi() == null) {
                                return null;
                            }
                            GroupChat groupChatImpl = ApiManager.getInstance()
                                    .getChatApi().getGroupChat(groupChatId);
                            if (groupChatImpl == null) {
                                return null;
                            }
                            ChatMessage msg = intent
                                    .getParcelableExtra("firstMessage");

                            if (msg != null) {
                                Logger.w(
                                        TAG,
                                        "first message group"
                                                + msg.getContent());
                            } else {
                                Logger.w(TAG,
                                        "first message group firstmessage:NULL");
                            }

                            /**
                             * managing extra local chat participants that are
                             * not present in the invitation for sending them
                             * invite request.@{
                             */
                            // getting the participant list for this invitaion
                            String participantList = intent
                                    .getStringExtra("participantList");

                            ArrayList<IChatMessage> chatMessages = new
                            ArrayList<IChatMessage>();
                            chatMessages.add(new ChatMessageReceived(msg));
                            String chatId = intent
                                    .getStringExtra(RcsNotification.CHAT_ID);

                            IChat1 chat = getGroupChat(chatId);
                            if (intent.getBooleanExtra("isGroupChatExist",
                                    false)) {
                                Logger.v(TAG, "DEF handleReInvitation");
                                if (chat != null) {
                                    ((GroupChat1) chat).handleReInvitation(
                                            groupChatImpl, participantList);
                                } else {
                                    Logger.v(TAG, "Chat null in onReceive");
                                }
                            }
                        } catch (Exception e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }
                        return null;
                    } else if (GroupChatIntent.ACTION_REINVITATION
                            .equalsIgnoreCase(action)) {
                        Logger.v(TAG, " GROUP ACTION_REINVITATION");
                        /*try {

                            String groupChatId = intent
                                    .getStringExtra(GroupChatIntent.EXTRA_CHAT_ID);
                            if (ApiManager.getInstance() == null
                                    || ApiManager.getInstance().getChatApi() == null) {
                                return null;
                            }
                            GroupChat groupChatImpl = ApiManager.getInstance()
                                    .getChatApi().getGroupChat(groupChatId);
                            if (groupChatImpl == null) {
                                return null;
                            }
                            ChatMessage msg = intent
                                    .getParcelableExtra(GroupChatIntent.EXTRA_CHAT_MESSAGE);

                            if (msg != null) {
                                Logger.w(
                                        TAG,
                                        "New message group"
                                                + msg.getContent());
                            } else {
                                Logger.w(TAG,
                                        "first message group firstmessage:NULL");
                            }
                            IChat1 chat = getGroupChat(groupChatId);
                            Logger.v(TAG, "DEF handleReInvitation1");
                            if (chat != null) {
                                //((GroupChat1) chat).setGroupListener(groupChatId,msg,groupChatImpl);
                            } else {
                                Logger.v(TAG, "Chat null in onReceive");
                            }
                        } catch (JoynServiceException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        } catch (Exception e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }*/
                        return null;
                    } else if (ChatIntent.ACTION_REINITIALIZE_LISTENER.equalsIgnoreCase(action)) {
                        try {
                            Logger.v(TAG, " ACTION_REINITIALIZE_LISTENER");
                            /*String contact = intent.getStringExtra(ChatIntent.EXTRA_CONTACT);
                            String status = intent.getStringExtra(ChatIntent.EXTRA_STATUS);
                            String msgid =  intent.getStringExtra(ChatIntent.EXTRA_MSG_ID);
                            contact = Utils.extractNumberFromUri(contact);
                            Logger.v(TAG, "Chat0 values: contact: " + contact + " ; status: " + status + " ;msgid: " + msgid);
                            if (ApiManager.getInstance() == null
                                    || ApiManager.getInstance().getChatApi() == null) {
                                return null;
                            }
                            Chat chatImpl = ApiManager.getInstance().getChatApi().getChat(contact);
                            if (chatImpl == null) {
                                Logger.v(TAG, "Chat0 null in onReceive");
                                return null;
                            }
                            handleMessageDeliveryStatus(contact,msgid,status,new Date().getTime());
                            IChat1 chat = null;

                            chat = getOne2OneChat(new Participant(contact, contact));
                            if (null != chat) {
                                Logger.i(TAG, "chat found add listener");
                                ((One2OneChat) chat).addNewListener(contact);
                            }*/
                            Logger.i(TAG, "reinitialize exit");
                        } catch (Exception e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }
                        return null;
                    }
                    return null;
                }
            } .execute();
        }
    }

    /**
     * This class is the implementation of sent Chat Message.
     *.
     */
    public static class ChatMessageSent extends ChatMessage1 {
        /**
         * @param sentMessage
         * .
         */
        public ChatMessageSent(ChatMessage sentMessage) {
            super(sentMessage);
        }
    }

    /**
     * This class is the implementation of a received chat message used in model
     * part.
     * .
     */
    public static class ChatMessageReceived extends ChatMessage1 {
        /**
         * @param receivedMessage
         * .
         */
        public ChatMessageReceived(ChatMessage receivedMessage) {
            super(receivedMessage);
        }
    }

    /**
     * The call-back method of the interface called when the unread message
     * number changed.
     * .
     */
    public interface UnreadMessageListener {
        /**
         * The call-back method to update the unread message when the number of
         * unread message changed.
         * .
         * @param chatWindowTag
         *            The chat window tag indicates which to update.
         * @param clear
         *            Whether cleared all unread message.
         */
        void onUnreadMessageNumberChanged(Object chatWindowTag, boolean clear);
    }



    private boolean isChatExisted(Participant participant) {
        Logger.v(TAG, "isHaveChat() The participant is " + participant);
        boolean bIsHaveChat = false;
        // Find the chat in the chat list
        Collection<IChat1> chatList = mChatMap.values();
        if (chatList != null) {
            for (IChat1 chat : chatList) {
                if (chat instanceof One2OneChat) {
                    if (null != participant
                            && (((One2OneChat) chat).isDuplicated(participant))) {
                        bIsHaveChat = true;
                    }
                }
            }
        }
        Logger.i(TAG, "isHaveChat() end, bIsHaveChat = " + bIsHaveChat);
        return bIsHaveChat;
    }

    @Override
    public boolean handleInvitation(Intent intent, boolean isCheckSessionExist) {
        Logger.v(TAG, "handleInvitation entry");
        String action = null;
        if (intent == null) {
            Logger.v(TAG, "handleInvitation intent is null");
            return false;
        } else {
            action = intent.getAction();
        }
        /*
         * if
         * (MessagingApiIntents.CHAT_SESSION_REPLACED.equalsIgnoreCase(action))
         * { Logger.v(TAG,
         * " handleInvitation() the action is CHAT_SESSION_REPLACED "); } else
         * if (MessagingApiIntents.CHAT_INVITATION.equalsIgnoreCase(action)) {
         * return handleChatInvitation(intent, isCheckSessionExist); }
         */
        if (OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_CHAT_MESSAGE.equalsIgnoreCase(action)) {
            // return handleChatInvitation(intent, isCheckSessionExist); //TODo
            // check this
        }
        Logger.v(TAG, "handleInvitation exit: action = " + action);
        return false;
    }

    /**
     * @param intent
     * .
     * @param isCheckSessionExist
     * .
     * @return .
     */
    public boolean handleO2OInvitation(Intent intent,
            boolean isCheckSessionExist) {
        Logger.v(TAG, "handleO2OInvitation entry");
        String action = null;
        if (intent == null) {
            Logger.v(TAG, "handleO2OInvitation intent is null");
            return false;
        } else {
            action = intent.getAction();
        }

        if (OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_CHAT_MESSAGE.equalsIgnoreCase(action) ||
                OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_STANDALONE_CHAT_MESSAGE.equalsIgnoreCase(action)) {
            return handleO2OChatInvitation(intent, isCheckSessionExist);
        }
        Logger.v(TAG, "handleO2OInvitation exit: action = " + action);
        return false;
    }

    public void handleNewMessage(String msgId, Intent intent) {
        Logger.v(TAG, "handleNewMessage entry:" + msgId);
        String number = null;
        String chatId = null;
        String text = null;
        if (ApiManager.getInstance() != null){
            ChatService chatService = ApiManager.getInstance().getChatApi();
            if(chatService != null){
                try {
                    com.gsma.services.rcs.chat.ChatMessage msg = chatService.getChatMessage(msgId);
                    number = msg.getRemoteContact().toString();
                    chatId = msg.getChatId();
                    text = msg.getContent();
                    Logger.v(TAG, " handleNewMessage number " + number + ",chatId:" + chatId  + ",txt:" + text);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                Logger.v(TAG, "chatService is null ");
            }
        } else {
            Logger.v(TAG, "ApiManager is null ");
        }

        String displayName = intent.getStringExtra(GroupChatIntent.EXTRA_DISPLAY_NAME);
        Logger.v(TAG, "handleNewMessage displayName:" + displayName);
        GroupChat1 chat = (GroupChat1) getChatImpl(chatId);
        if(chat == null){
            Logger.v(TAG, "handleNewMessage chat is null");
            return;
        }
        if(displayName == null){
            displayName = ContactsListManager.getInstance()
                    .getDisplayNameByPhoneNumber(number);
            Logger.v(TAG, "handleNewMessage displayName1:" + displayName);
        }
        ChatMessage message = new ChatMessage(msgId,number, text,new Date(),true,displayName);
        chat.handleReceiveMessage(message);
    }

    /**
     * @param intent
     * .
     * @param isCheckSessionExist
     * .
     * @return .
     */
    public boolean handleNewGroupInvitation(Intent intent,
            boolean isCheckSessionExist) {
        Logger.v(TAG, "handleNewGroupInvitation entry");
        String action = null;
        if (intent == null) {
            Logger.v(TAG, "handleNewGroupInvitation intent is null");
            return false;
        } else {
            action = intent.getAction();
        }
        if (GroupChatIntent.ACTION_NEW_INVITATION.equalsIgnoreCase(action)) {
            return handleNewGroupChatInvitation(intent, isCheckSessionExist);
        }
        Logger.v(TAG, "handleNewGroupInvitation exit: action = " + action);
        return false;
    }

    private List<String> convertSetToList(Set<String> participantInfos) {
        // create an iterator
        Iterator iterator = participantInfos.iterator();
        List<String> pList = new ArrayList<String>();

        // check values
        while (iterator.hasNext()) {
            pList.add(iterator.next().toString());
        }
        return pList;
    }

    private List<String> convertIdSetToList(Set<ContactId> contacts) {
        // create an iterator
        Iterator iterator = contacts.iterator();
        List<String> pList = new ArrayList<String>();

        // check values
        while (iterator.hasNext()) {
            pList.add(iterator.next().toString());
        }
        return pList;
    }

    /**
     * @param intent
     * .
     * @param isCheckSessionExist
     * .
     * @return .
     */
    public boolean handleNewGroupChatInvitation(Intent intent,
            boolean isCheckSessionExist) {
        ApiManager instance = ApiManager.getInstance();
        //String sessionId = intent.getStringExtra("sessionId");
        boolean isAutoAccept = intent.getBooleanExtra(
                RcsNotification.AUTO_ACCEPT, false);
        String contactNumber = intent
                .getStringExtra(GroupChatIntent.EXTRA_CONTACT);
        if (instance != null) {
            ChatService chatService = instance.getChatApi();
            if (chatService != null) {
                try {
                    String groupChatId = intent
                            .getStringExtra(GroupChatIntent.EXTRA_CHAT_ID);
                    GroupChat groupChatImpl = chatService
                            .getGroupChat(groupChatId);
                    if (groupChatImpl == null) {
                        Logger.e(TAG, "The groupChatImpl is null");
                        return false;
                    }
                    List<String> participants = convertIdSetToList(groupChatImpl
                            .getParticipants().keySet());
                    if (participants == null || participants.size() == 0) {
                        Logger.e(TAG,
                                "The getParticipants is null, or size is 0");
                        return false;
                    }
                    Logger.i(TAG,
                            "handleNewGroupChatInvitation message contact: "
                                    + contactNumber);
                    int participantCount = participants.size();

                    Logger.w(TAG, "handleChatInvitation group");
                    String groupSubject = intent
                            .getStringExtra(GroupChatIntent.EXTRA_SUBJECT);
                    List<Participant> participantsList = new ArrayList<Participant>();
                    for (int i = 0; i < participantCount; i++) {
                        String remoteParticipant = participants.get(i)
                                .toString();
                        String number = Utils
                                .extractNumberFromUri(remoteParticipant);
                        String name = number;
                        if (Utils.isANumber(number)) {
                            name = ContactsListManager.getInstance()
                                    .getDisplayNameByPhoneNumber(number);
                        } else {
                            Logger.e(TAG, "the participant " + number
                                    + " is not a real number");
                        }
                        Participant fromSessionParticipant = new Participant(
                                number, name);
                        participantsList.add(fromSessionParticipant);
                    }
                    String chatId = intent
                            .getStringExtra(GroupChatIntent.EXTRA_CHAT_ID);
                    ParcelUuid tag = (ParcelUuid) intent
                            .getParcelableExtra(ChatScreenActivity.KEY_CHAT_TAG);

                    IChat1 chat = getGroupChat(chatId);
                    Logger.w(TAG, "handleChatInvitation group ChatId : "
                            + chatId);
                    if (chat == null) {
                        Logger.w(TAG, "handleChatInvitation chat is null Tag: "
                                + tag);
                        chat = addChat(participantsList, tag, chatId);
                        ((GroupChat1) chat).setmChatId(chatId);
                        /* add group name , get from Intent */
                        ((GroupChat1) chat)
                                .addGroupSubjectFromInvite(groupSubject);
                        ((GroupChat1) chat).setmInvite(true);
                    } else {
                        // restart chat.
                        Logger.w(TAG, "handleChatInvitation restatrt chat ");
                        ParcelUuid chatTag = (ParcelUuid) (((GroupChat1) chat).mTag);
                        Logger.v(TAG,
                                "handleChatInvitation() restart chat tag: "
                                        + chatTag);
                        ChatScreenWindowContainer.getInstance().focus(chatTag);
                        ViewImpl.getInstance().switchChatWindowByTag(chatTag);
                    }
                    Logger.w(TAG, "ADD TO chat " + isAutoAccept);
                    boolean closedStatus = intent.getBooleanExtra("isClosedGroupChat",false);
                    ((GroupChat1) chat).setIsGroupChatStatus(closedStatus);
                    chat.setInviteContact(contactNumber);
                    chat.handleInvitation(groupChatImpl, null, isAutoAccept);
                    return true;
                } catch (Exception e) {
                    Logger.e(TAG, "getChatSession fail");
                    e.printStackTrace();
                }
            }
        }
        return false;
    }

    /**
     * @param intent
     * .
     * @param isCheckSessionExist
     * .
     * @return .
     */
    public boolean handleO2OChatInvitation(Intent intent,
            boolean isCheckSessionExist) {
        ApiManager instance = ApiManager.getInstance();
        String sessionId = intent.getStringExtra("sessionId");
        boolean isAutoAccept = intent.getBooleanExtra(
                RcsNotification.AUTO_ACCEPT, false);
        String contactNumber = null;//intent.getStringExtra(ChatIntent.EXTRA_CONTACT);
        String content = null;
        String displayName =  intent.getStringExtra(RcsNotification.DISPLAY_NAME);

        String msgId = intent.getStringExtra(OneToOneChatIntent.EXTRA_MESSAGE_ID);
        Logger.v(TAG, " handleO2OChatInvitation msgId " + msgId);
        if (ApiManager.getInstance() != null){
            ChatService chatService = ApiManager.getInstance().getChatApi();
            if(chatService != null){
                try {
                    com.gsma.services.rcs.chat.ChatMessage msg = chatService.getChatMessage(msgId);
                    contactNumber = msg.getRemoteContact().toString();
                    content = msg.getContent();
                    Logger.v(TAG, " handleO2OChatInvitation remote " + contactNumber + ",content:" + content);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                Logger.v(TAG, "chatService is null ");
            }
        } else {
            Logger.v(TAG, "ApiManager is null ");
        }
        if (instance != null) {
            ChatService chatService = instance.getChatApi();
            if (chatService != null) {
                try {
                    //Chat o2oChatImpl = chatService.getChat(contactNumber);
                    OneToOneChat o2oChatImpl = chatService.getOneToOneChat(ContactIdUtils.createContactIdFromTrustedData(contactNumber));
                    /*if (o2oChatImpl == null) {
                        try {
                            Set<Chat> totalChats = null;
                            if (o2oChatImpl == null) {
                                totalChats = chatService.getChats();
                                Logger.e(TAG, "aaa2 getChatSession size: "
                                        + totalChats.size());
                                Logger.v(TAG, "The2 chat session is null3 : "
                                        + totalChats.size());
                            }
                            for (Chat setElement : totalChats) {
                                if (setElement.getRemoteContact().equals(
                                        contactNumber)) {
                                    Logger.e(TAG, "Ge2t chat session finally");
                                    // might work or might throw exception, Java
                                    // calls it indefined behaviour:
                                    o2oChatImpl = setElement;
                                    break;
                                }
                            }
                        } catch (JoynServiceException e) {
                            Logger.e(TAG, "Ge2t chat session xyz");
                            e.printStackTrace();
                        }
                    }*/
                    if (o2oChatImpl == null) {
                        Logger.e(TAG, "The getChatSession is null");
                        return false;
                    }
                    Logger.v(TAG, "handleChatInvitation message");
                    ArrayList<ChatMessage> messages = new ArrayList();
                    ChatMessage message = new ChatMessage(msgId,contactNumber, content,new Date(),true,displayName);
                    messages.add(message);
                    ArrayList<IChatMessage> chatMessages = new ArrayList<IChatMessage>();
                    if (null != messages) {
                        int size = messages.size();
                        for (int i = 0; i < size; i++) {
                            ChatMessage msg = messages.get(i);
                            if (msg != null) {
                                Logger.w(TAG, "ChatMessage:" + "msg" + msg
                                        + "string" + msg.getContent());
                            } else {
                                Logger.w(TAG, "ChatMessage: is NULL");
                            }

                            if (msg != null) {
                                chatMessages.add(new ChatMessageReceived(msg));
                            }
                        }
                    }
                    Logger.v(TAG, "handleChatInvitation O2O");
                    List<Participant> participantsList = new ArrayList<Participant>();
                    String name = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(contactNumber);
                    if(name == null){
                        name = contactNumber;
                    }
                    Participant fromSessionParticipant = new Participant(
                            contactNumber, name);
                    participantsList.add(fromSessionParticipant);
                    if (!isCheckSessionExist) {
                        IChat1 currentChat = addChat(participantsList);
                        currentChat.handleInvitation(o2oChatImpl, chatMessages,
                                isAutoAccept);
                        return true;
                    } else {
                        if (isChatExisted(fromSessionParticipant)) {
                            IChat1 currentChat = addChat(participantsList);
                            currentChat.handleInvitation(o2oChatImpl,
                                    chatMessages, isAutoAccept);
                            Logger.v(TAG, "handleInvitation exit with true");
                            return true;
                        } else {
                            Logger.v(TAG, "handleInvitation exit with false");
                            return false;
                        }
                    }
                } catch (Exception e) {
                    Logger.e(TAG, "handleO2OChatInvitation fail1");
                    e.printStackTrace();
                }
            }
        }
        return false;
    }

    /**
     * @param fileTransferId
     * .
     * @param isAutoAccept
     * .
     * @return .
     */
    public boolean handleFileTransferInvitation(String fileTransferId,
            boolean isAutoAccept) {
        return true;
    }

    /**
     * @param fileTransferId
     * .
     * @param isAutoAccept
     * .
     * @param isGroup .
     * @param chatSessionId .
     * @param chatId .
     * @return .
     */
    public boolean handleFileTransferInvitation(String fileTransferId,
            boolean isAutoAccept, boolean isGroup, String chatSessionId,
            String chatId,String msgid) {
        Logger.w(TAG, "M0CFF handleFileTransferInvitation isGroup" + isGroup
                + "isAutoAccept" + isAutoAccept +", msgid: " + msgid);
        Logger.e(TAG, "M0CFF handleFileTransferInvitation chatSessionId"
                + chatSessionId + "sessionId" + fileTransferId + "chatid"
                + chatId);
        ApiManager instance = ApiManager.getInstance();
        if (instance != null) {
            FileTransferService fileTransferService = instance
                    .getFileTransferApi();
            if (fileTransferService != null) {
                try {
                    FileTransfer fileTransferObject = fileTransferService
                            .getFileTransfer(fileTransferId);
                    if (fileTransferObject == null) {
                        Logger.w(TAG,
                                "M0CFF handleFileTransferInvitation-The getFileTransferSession" +
                                " is null");
                        return false;
                    }

                    List<Participant> participantsList = new ArrayList<Participant>();
                    String number = Utils
                            .extractNumberFromUri(fileTransferObject
                                    .getRemoteContact().toString());

                    // Get the contact name from contact list
                    String name = EMPTY_STRING;
                    Logger.e(TAG,
                            "M0CFF handleFileTransferInvitation, number = "
                                    + name);
                    if (null != number) {
                        if (ContactsListManager.getInstance() == null) {
                            ContactsListManager.initialize(MediatekFactory.getApplicationContext());
                        }
                        String tmpContact = ContactsListManager.getInstance()
                                .getDisplayNameByPhoneNumber(number);
                        if (tmpContact != null) {
                            name = tmpContact;
                        } else {
                            name = number;
                        }
                    }

                    Participant fromSessionParticipant = new Participant(
                            number, name);
                    participantsList.add(fromSessionParticipant);
                    if (isGroup) {
                        IChat1 chat = getGroupChat(chatId);
                        if (null != chat) {
                            ((GroupChat1) chat).addReceiveFileTransfer(
                                    fileTransferObject, isAutoAccept, true);
                            if (!((GroupChat1) chat).mIsInBackground) {
                                Logger.v(
                                        TAG,
                                        "M0CFF handleFileTransferInvitation() group"
                                                + "-handleInvitation exit with true, is the" +
                                                    " current chat!");
                                return true;
                            } else {
                                Logger.v(
                                        TAG,
                                        "M0CFF handleFileTransferInvitation() group"
                                                + "-handleInvitation exit with false, is not" +
                                                   " the current chat!");
                                return false;
                            }
                        }
                    } else {
                        if (isChatExisted(fromSessionParticipant)) {
                            IChat1 chat = addChat(participantsList);
                            if (chat != null) {
                                ((One2OneChat) chat)
                                        .addReceiveFileTransfer(
                                                fileTransferObject,
                                                isAutoAccept, false);
                                if (!((One2OneChat) chat).mIsInBackground) {
                                    Logger.v(
                                            TAG,
                                            "M0CFF handleFileTransferInvitation()"
                                                    + "-handleInvitation exit with true," +
                                                       " is the current chat!");
                                    //Date date = new Date();
                                    ChatMessage msg = new ChatMessage(msgid,number,"",new Date(),true,"");
                                    /*ChatMessage msg = new ChatMessage(msgid, number,
                                            "", null, number, RcsService.Direction.INCOMING.toInt(), date.getTime(), date.getTime(), 0L, 0L,
                                            ChatLog.Message.Content.Status.RECEIVED.toInt() , ChatLog.Message.Content.ReasonCode.UNSPECIFIED.toInt(), null, true, false);*/
                                    ((One2OneChat) chat).markMessageAsDisplayed(msg);
                                    return true;
                                } else {
                                    Logger.v(
                                            TAG,
                                            "M0CFF handleFileTransferInvitation()"
                                                    + "-handleInvitation exit with true," +
                                                     " is not the current chat!");
                                    //Date date = new Date();
                                    ChatMessage msg = new ChatMessage(msgid,number,"",new Date(),true,"");
                                    /*ChatMessage msg = new ChatMessage(msgid, number,
                                            "", null, number, RcsService.Direction.INCOMING.toInt(), date.getTime(), date.getTime(), 0L, 0L,
                                            ChatLog.Message.Content.Status.RECEIVED.toInt() , ChatLog.Message.Content.ReasonCode.UNSPECIFIED.toInt(), null, true, false);*/
                                    ((One2OneChat) chat).addUnreadrFTMessage(msg);
                                    return false;
                                }
                            }
                        } else {
                            Logger.v(TAG,
                                    "M0CFF handleFileTransferInvitation-handleInvitation" +
                                    " exit with false");
                            IChat1 chat = addChat(participantsList);
                            ((One2OneChat) chat).addReceiveFileTransfer(
                                    fileTransferObject, isAutoAccept, false);
                            //Date date = new Date();
                            ChatMessage msg = new ChatMessage(msgid,number,"",new Date(),true,"");
                            /*ChatMessage msg = new ChatMessage(msgid, number,
                                    "", null, number, RcsService.Direction.INCOMING.toInt(), date.getTime(), date.getTime(), 0L, 0L,
                                    ChatLog.Message.Content.Status.RECEIVED.toInt() , ChatLog.Message.Content.ReasonCode.UNSPECIFIED.toInt(), null, true, false);*/
                            ((One2OneChat) chat).addUnreadrFTMessage(msg);
                            return false;
                        }
                    }
                } catch (Exception e) {
                    Logger.e(TAG,
                            "M0CFF handleFileTransferInvitation-getParticipants fail1");
                    e.printStackTrace();
                }
            }
        }
        return false;
    }

    @Override
    public void handleMessageDeliveryStatus(String contact, String msgId,
            String status, long timeStamp) {
        ContentResolver contentResolver = MediatekFactory
                .getApplicationContext().getContentResolver();
        String[] selectionArg = { msgId.toString() };
        Cursor cursor = contentResolver.query(ChatLog.Message.CONTENT_URI,
                null, ChatLog.Message.MESSAGE_ID + "=?", selectionArg, null);
        String remoteContact = null;
        if (cursor != null) {
            if (cursor.moveToFirst()) {
                remoteContact = cursor.getString(cursor
                        .getColumnIndex(ChatLog.Message.CONTACT));
            }
            cursor.close();
        }

        if (remoteContact == null) {
            Logger.i(TAG,
                    "handleMessageDeliveryStatus() exit by null remote contact");
            return;
        }

        Logger.i(TAG, "handleMessageDeliveryStatus() entry, remoteContact:"
                + remoteContact + "Msgid:" + msgId);
        remoteContact = Utils.extractNumberFromUri(remoteContact);
        IChat1 chat = null;

        chat = getOne2OneChat(new Participant(remoteContact, remoteContact));
        if (null != chat) {
            Logger.i(TAG, "handleMessageDeliveryStatus() chat found");
            if (chat instanceof One2OneChat) {
                ((One2OneChat) chat).onMessageDelivered(msgId, status,
                        timeStamp);
            }
        }

        // TODO fix me
        chat = getGroupChat(null);
        if (null != chat) {
            if (chat instanceof GroupChat1) {
                ((GroupChat1) chat)
                        .onMessageDelivered(msgId, status, timeStamp);
            }
        }
    }

    /**
     * Called by the controller when the user needs to cancel an on-going file
     * transfer.
     * .
     * @param tag
     *            The chat window tag where the file transfer is in.
     * @param fileTransferTag
     *            The tag indicating which file transfer will be canceled.
     */
    public void handleCancelFileTransfer(Object tag, Object fileTransferTag) {
        Logger.v(TAG, "M0CFF handleCancelFileTransfer(),tag = " + tag
                + ",fileTransferTag = " + fileTransferTag);
        IChat1 chat = getChat(tag);
        if (chat == null) {
            chat = getOne2oneChatByContact((String) tag);
        }
        if (chat instanceof One2OneChat) {
            One2OneChat oneOneChat = ((One2OneChat) chat);
            oneOneChat.handleCancelFileTransfer(fileTransferTag);
        } else if(chat instanceof GroupChat1) {
            GroupChat1 groupChat = ((GroupChat1) chat);
            groupChat.handleCancelFileTransfer(fileTransferTag);
        }
        Logger.v(TAG,
                "M0CFF handleCancelFileTransfer() it's a sent file transfer");
        mOutGoingFileTransferManager.cancelFileTransfer(fileTransferTag);
    }

    /**
     * Called by the controller when the user needs to resend a file.
     * .
     * @param fileTransferTag
     *            The tag of file which the user needs to resend.
     */
    public void handleResendFileTransfer(Object fileTransferTag) {
        Logger.v(TAG, "M0CFF handleResendFileTransfer() TAG: "
                + fileTransferTag);
        mOutGoingFileTransferManager.resendFileTransfer(fileTransferTag);
    }

    /**
     * Called by the controller when the user needs to pause a file.
     * .
     * @param chatWindowTag
     *            The tag of chat window.
      * @param fileTransferTag
     *            The tag of file which the user needs to resend.
      * @param option
     *            The tag of file which the user needs to resend.
     */
    public void handlePauseFileTransfer(Object chatWindowTag,
            Object fileTransferTag, int option) {
        Logger.v(TAG, "M0CFF handlePauseFileTransfer(),tag = " + chatWindowTag
                + ",fileTransferTag = " + fileTransferTag + "option:" + option);
        if (option == 0) {
            mOutGoingFileTransferManager.pauseFileTransfer(fileTransferTag);
        } else {
            IChat1 chat = getChat(chatWindowTag);
            if (chat instanceof One2OneChat) {
                Logger.v(TAG, "M0CFF handlePauseFileTransfer() One2OneChat");
                One2OneChat oneOneChat = ((One2OneChat) chat);
                oneOneChat.handlePauseReceiveFileTransfer(fileTransferTag);
            } else if (chat instanceof GroupChat1) {
                Logger.v(TAG, "M0CFF handlePauseFileTransfer() GroupChat1");
                GroupChat1 goupChat = ((GroupChat1) chat);
                goupChat.handlePauseReceiveFileTransfer(fileTransferTag);
            }
        }
    }

    /**
     * Called by the controller when the user needs to resume a file.
     * .
     * @param chatWindowTag
     *            The tag of chat window.
      * @param fileTransferTag
     *            The tag of file which the user needs to resume.
      * @param option
     *            The tag of file which the user needs to resume.
     */
    public void handleResumeFileTransfer(Object chatWindowTag,
            Object fileTransferTag, int option) {
        Logger.v(TAG, "M0CFF handleResumeFileTransfer() TAG: "
                + fileTransferTag + "OPTION: " + option);
        if (option == 0) {
            mOutGoingFileTransferManager.resumeFileTransfer(fileTransferTag);
        } else {
            IChat1 chat = null;
            if(chatWindowTag instanceof String) {
                chat = getOne2oneChatByContact((String) chatWindowTag);
            } else {
                chat = getChat(chatWindowTag);
            }
            if (chat instanceof One2OneChat) {
                One2OneChat oneOneChat = ((One2OneChat) chat);
                oneOneChat.handleResumeReceiveFileTransfer(fileTransferTag);
            } else if (chat instanceof GroupChat1) {
                GroupChat1 goupChat = ((GroupChat1) chat);
                goupChat.handleResumeReceiveFileTransfer(fileTransferTag);
            }
        }
    }

    /**
     * This class represents a file structure used to be shared.
     *.
     */
    public static class FileStruct {
        public static final String TAG = "M0CFF FileStruct";

        /**
         * Generate a file struct instance using a session and a path, this
         * method should only be called for Received File Transfer.
         * .
         * @param fileTransferObject
         *            The object of the file transfer.
         * @param filePath
         *            The path of the file.
         * @return The file struct instance.
         * @throws RemoteException .
         */
        public static FileStruct from(FileTransfer fileTransferObject,
                String filePath) throws RemoteException {
            FileStruct fileStruct = null;
            String fileName = null;
            try {
                fileName = fileTransferObject.getFileName();
                long fileSize = fileTransferObject.getFileSize();
                String fileTransferId = fileTransferObject.getTransferId();
                Date date = new Date();
                if (!RcsSettings.getInstance()
                        .isFileTransferThumbnailSupported()) {
                    Logger.v(TAG,
                            "Filestruct thumbnail not supported fileContact"
                                    + fileTransferObject.getRemoteContact());
                    fileStruct = new FileStruct(filePath, fileName, fileSize,
                            fileTransferId, date,
                            fileTransferObject.getRemoteContact().toString());
                } else {
                    Logger.v(TAG, "FileStruct thumbnail supported"
                            + fileTransferObject.getFileIcon().getPath());
                    fileStruct = new FileStruct(filePath, fileName, fileSize,
                            fileTransferId, date,
                            fileTransferObject.getRemoteContact().toString(),
                            fileTransferObject.getFileIcon().getPath());
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            return fileStruct;
        }

        /**
         * Generate a file struct instance using a path, this method should only
         * be called for Sent File Transfer.
         * .
         * @param filePath
         *            The path of the file.
         * @return The file struct instance.
         */
        public static FileStruct from(String filePath) {
            FileStruct fileStruct = null;
            File file = new File(filePath);
            if (file.exists()) {
                Date date = new Date();
                fileStruct = new FileStruct(filePath, file.getName(),
                        file.length(), new ParcelUuid(UUID.randomUUID()), date);
            }
            Logger.v(TAG, "from() fileStruct: " + fileStruct);
            return fileStruct;
        }

        /**
         * @param filePath
         * .
         * @param name
         * .
         * @param size
         * .
         * @param fileTransferTag
         * .
         * @param date
         * .
         */
        public FileStruct(String filePath, String name, long size,
                Object fileTransferTag, Date date) {
            mFilePath = filePath;
            mName = name;
            mSize = size;
            mFileTransferTag = fileTransferTag;
            mDate = (Date) date.clone();
        }

        /**
         * @param filePath
         * .
         * @param name .
         * @param size .
         * @param fileTransferTag .
         * @param date .
         * @param remote .
         */
        public FileStruct(String filePath, String name, long size,
                Object fileTransferTag, Date date, String remote) {
            mFilePath = filePath;
            mName = name;
            mSize = size;
            mFileTransferTag = fileTransferTag;
            mDate = (Date) date.clone();
            mRemote = remote;
        }

        /**
         * @param filePath .
         * @param name .
         * @param size .
         * @param fileTransferTag .
         * @param date .
         * @param remote .
         * @param thumbNail .
         */
        public FileStruct(String filePath, String name, long size,
                Object fileTransferTag, Date date, String remote,
                String thumbNail) {
            mFilePath = filePath;
            mName = name;
            mSize = size;
            mFileTransferTag = fileTransferTag;
            mDate = (Date) date.clone();
            mRemote = remote;
            mThumbnail = thumbNail;
        }

 public FileStruct(String filePath, String name, long size, Object fileTransferTag, Date date, String remote, String thumbNail,int reload) {
            mFilePath = filePath;
            mName = name;
            mSize = size;
            mFileTransferTag = fileTransferTag;
            mDate = (Date) date.clone();
            mRemote = remote;
            mThumbnail = thumbNail;
            mReload = reload;
        }


        public String mFilePath = null;

        public String mThumbnail = null;

         public int mReload = 0;

        public String mName = null;

        public long mSize = -1;

        public Object mFileTransferTag = null;

        public Date mDate = null;

        public String mRemote = null;

        /**
         * Override toString function.
         * .
         * @return  .
         */
        public String toString() {
            return TAG + "file path is " + mFilePath + " file name is " + mName
                    + " size is " + mSize + " FileTransferTag is "
                    + mFileTransferTag + " date is " + mDate;
        }
    }

    /**
     * This class represents a chat event structure used to be shared.
     */
    public static class ChatEventStruct {
        public static final String TAG = "M0CF ChatEventStruct";

        /**
         * @param info .
         * @param relatedInfo .
         * @param dt .
         */
        public ChatEventStruct(Information info, Object relatedInfo, Date dt) {
            information = info;
            relatedInformation = relatedInfo;
            date = (Date) dt.clone();
        }

        public Information information = null;

        public Object relatedInformation = null;

        public Date date = null;

        /**
         * Override toString function.
         * .
         * @return  .
         */
        public String toString() {
            return TAG + "information is " + information
                    + " relatedInformation is " + relatedInformation
                    + " date is " + date;
        }
    }

    /**
     * Clear all history of the chats, include both one2one chat and group chat.
     * .
     * @return True if successfully clear, false otherwise.
     */
    public boolean clearAllHistory() {
        Logger.v(TAG, "clearAllHistory() entry");
        ControllerImpl controller = ControllerImpl.getInstance();
        boolean result = false;
        if (null != controller) {
            Message controllerMessage = controller.obtainMessage(
                    ChatController.EVENT_CLEAR_CHAT_HISTORY, null, null);
            controllerMessage.sendToTarget();
            result = true;
        }
        Logger.w(TAG, "clearAllHistory() exit with result = " + result);
        return result;
    }

    /**
     * Called by the controller when the user needs to start a file transfer.
     * .
     * @param target
     *            The tag of the chat window in which the file transfer starts.
     *            or the contact to receive this file.
     * @param filePath
     *            The path of the file to be transfered.
     * @param fileTransferTag
     *            The tag of the file to be transfered.
     */
    public void handleSendFileTransferInvitation(Object target,
            String filePath, Object fileTransferTag) {
        Logger.w(TAG,
                "M0CFF handleSendFileTransferInvitation() user starts to send file "
                        + filePath + " to target " + target
                        + ", fileTransferTag: " + fileTransferTag);
        IChat1 chat = null;
        if (target instanceof String) {
            String contact = (String) target;
            List<Participant> participants = new ArrayList<Participant>();
            participants.add(new Participant(contact, contact));
            chat = addChat(participants);
        } else {
            chat = getChat(target);
        }

        Logger.w(TAG, "M0CFF handleSendFileTransferInvitation() chat: " + chat);
        if (chat instanceof One2OneChat) {
            Participant participant = ((One2OneChat) chat).getParticipant();
            Logger.w(TAG,
                    "M0CFF handleSendFileTransferInvitation() user starts to send O2O file "
                            + filePath + " to " + participant
                            + ", fileTransferTag: " + fileTransferTag);
            mOutGoingFileTransferManager
                    .onAddSentFileTransfer(((One2OneChat) chat)
                            .generateSentFileTransfer(filePath, fileTransferTag));
        } else if (chat instanceof GroupChat1) {
            List<ParticipantInfo> participantsInfo = ((GroupChat1) chat)
                    .getParticipantInfos();
            List<String> participants = new ArrayList<String>();
            for (ParticipantInfo info : participantsInfo) {
                participants.add(info.getContact());
            }
            GroupChatImplStatus status = ((GroupChat1) chat).getStatus();
            Logger.v(TAG, "Groupstatus() : " + status);
            /*if(status == GroupChatImplStatus.MANULLY_REJOIN){
                Logger.v(TAG, "send after rejoin ");
                handleGroupSendFileTransferInvitation(target,filePath,fileTransferTag);
                //((GroupChat1) chat).rejoinGroup();
            }else {*/
                mOutGoingFileTransferManager
                        .onAddSentFileTransfer(((GroupChat1) chat)
                                .generateSentFileTransfer(chat, filePath,
                                        fileTransferTag));
            //}

        } else if(chat instanceof MultiChat) {
            mOutGoingFileTransferManager
            .onAddSentFileTransfer(((MultiChat) chat)
                    .generateSentFileTransfer(filePath, fileTransferTag));
        }
    }

    List<Object> mFileGroupTransferTag = new ArrayList<Object>();
    List<String> mFileGroupTransferPath = new ArrayList<String>();
    Object mGroupFileTransferTarget = null;

    /**
     * .
     */
    public void startGroupFileStransfer() {
        Logger.v(TAG, "M0CFF startGroupFileStransfer()");
        if (mGroupFileTransferTarget != null) {
            Logger.v(TAG, "M0CFF startGroupFileStransfer() not empty");
            Object target = mGroupFileTransferTarget;
            int index = 0;
            IChat1 chat = null;
            Logger.v(TAG, "M0CFF startGroupFileStransfer() size is: "
                    + mFileGroupTransferTag.size());
            if (target instanceof String) {
                Logger.v(TAG, "M0CFF startGroupFileStransfer()1");
                String contact = (String) target;
                List<Participant> participants = new ArrayList<Participant>();
                participants.add(new Participant(contact, contact));
                chat = addChat(participants);
            } else {
                Logger.v(TAG, "M0CFF startGroupFileStransfer()2");
                chat = getChat(target);
            }
            for (String filePath : mFileGroupTransferPath) {
                Object fileTransferTag = mFileGroupTransferTag.get(index);
                Logger.v(TAG, "M0CFF startGroupFileStransfer() Tag is: "
                        + fileTransferTag + "Paths is: " + filePath);
                if (chat instanceof GroupChat1) {
                    List<ParticipantInfo> participantsInfo = ((GroupChat1) chat)
                            .getParticipantInfos();
                    List<String> participants = new ArrayList<String>();
                    for (ParticipantInfo info : participantsInfo) {
                        participants.add(info.getContact());
                    }
                    mOutGoingFileTransferManager
                            .onAddSentFileTransfer(((GroupChat1) chat)
                                    .generateSentFileTransfer(chat, filePath,
                                            fileTransferTag));

                }
            }
            mGroupFileTransferTarget = null;
            mFileGroupTransferTag.clear();
            mFileGroupTransferPath.clear();
        }

        Logger.v(TAG, "M0CFF startGroupFileStransfer() exit");
    }

    public void addChatImpl(String tag, IChat1 chat){
        Logger.v(TAG, "addChatImpl(),tag = " + tag + ",chat" + chat);
        if (tag == null || chat == null) {
            Logger.v(TAG, "addChatImpl tag is null so return");
            return;
        }
        mChatImplMap.put(tag, chat);
    }

    /**
     * Called by the controller when the user needs to start a file transfer.
     * .
     * @param target
     *            The tag of the chat window in which the file transfer starts.
     *            or the contact to receive this file.
     * @param filePath
     *            The path of the file to be transfered.
     * @param fileTransferTag
     *            The tag of the file to be transfered.
     */
    public void handleGroupSendFileTransferInvitation(Object target,
            String filePath, Object fileTransferTag) {
        Logger.v(TAG,
                "M0CFF handleGroupSendFileTransferInvitation() user starts to send file "
                        + filePath + " to target " + target
                        + ", fileTransferTag: " + fileTransferTag);

        mFileGroupTransferTag.add(fileTransferTag);
        mFileGroupTransferPath.add(filePath);
        mGroupFileTransferTarget = target;
    }

    /**
     * .
     */
    public void handleFileResumeAfterStatusChange() {
        Logger.v(TAG, "M0CFF resumeFileSend 02 entry");
        mOutGoingFileTransferManager.resumesFileSend();
    }

    public static class ResumeFileTransfer extends SentFileTransfer {

        protected boolean isReceiveFileTransfer = false;

        public ResumeFileTransfer(IFileTransfer fileTransfer,
                Participant participant, Object fileTransferTag,
                FileStruct fileStruct, boolean isReceiveTransfer) {
            super(fileTransfer, participant, fileTransferTag, fileStruct);
            Logger.v(TAG, "ResumeFileTransfer constructor "
                    + "fileTransferTag: " + fileTransferTag);
            this.isReceiveFileTransfer = isReceiveTransfer;

        }
    }

    /**
     * File transfer session event listener for 1-1 chat.
     *.
     */
    private class One2OneFileTransferListener extends OneToOneFileTransferListener {
        private static final String TAG = "M0CFF One2OneFileTransferListener";

        public void onStateChanged(ContactId contact, String transferId, State state,
                ReasonCode reasonCode){
            Logger.v(TAG, "onStateChanged() this file is "
                    + transferId + ",state" + state + ", contact=" + contact.toString() + ",reason:"+ reasonCode.toInt());
            Object fileTransfer = (Object) getFileTransferImpl(transferId);
            if(fileTransfer == null){
                Logger.v(TAG, "onStateChanged fileTransfer is null");
                return;
            }
            try {
                if(fileTransfer instanceof SentFileTransfer){
                    Logger.v(TAG, "onStateChanged fileTransfer is SentFileTransfer");
                    SentFileTransfer sentFileTransfer = (SentFileTransfer)fileTransfer;
                    String filename = sentFileTransfer.mFileTransferObject.getFileName();
                    if(state == FileTransfer.State.TRANSFERRED){
                        sentFileTransfer.onFileTransferred(filename);
                    } else if(state == FileTransfer.State.STARTED){
                        sentFileTransfer.onTransferStarted();
                    } else if(state == FileTransfer.State.ABORTED ){
                        sentFileTransfer.onTransferAborted(reasonCode.toInt());
                    } else if(state == FileTransfer.State.REJECTED ||
                            state == FileTransfer.State.FAILED){
                        sentFileTransfer.onTransferError(reasonCode.toInt());
                    } else if(state == FileTransfer.State.DELIVERED ){
                        //sentFileTransfer.onTransferAborted();
                    } else if(state == FileTransfer.State.DISPLAYED ){
                        //sentFileTransfer.onTransferAborted();
                    }
                } else {
                    Logger.v(TAG, "onStateChanged fileTransfer is ReceiveFileTransfer");
                    ReceiveFileTransfer rcvFileTransfer = (ReceiveFileTransfer)fileTransfer;
                    String filename = rcvFileTransfer.mFileTransferObject.getFileName();
                    if(state == FileTransfer.State.TRANSFERRED){
                        rcvFileTransfer.onFileTransferred(filename);
                    } else if(state == FileTransfer.State.STARTED){
                        rcvFileTransfer.onTransferStarted();
                    } else if(state == FileTransfer.State.ABORTED ){
                        rcvFileTransfer.onTransferAborted(reasonCode.toInt());
                    } else if(state == FileTransfer.State.REJECTED ||
                            state == FileTransfer.State.FAILED){
                        rcvFileTransfer.onTransferError(reasonCode.toInt());
                    } else if(state == FileTransfer.State.DELIVERED ){
                        //sentFileTransfer.onTransferAborted();
                    } else if(state == FileTransfer.State.DISPLAYED ){
                        //sentFileTransfer.onTransferAborted();
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        public void onProgressUpdate(ContactId contact, String transferId, long currentSize,
                long totalSize){
            Logger.v(TAG, "onProgressUpdate() this file is "
                    + transferId + ",currentsize" + currentSize + ", totalsize ="+ totalSize + ", contact=" + contact.toString());
            Object fileTransfer = (Object) getFileTransferImpl(transferId);
            if(fileTransfer == null){
                Logger.v(TAG, "onProgressUpdate fileTransfer is null");
                return;
            }

            if(fileTransfer instanceof SentFileTransfer){
                Logger.v(TAG, "onProgressUpdate fileTransfer is SentFileTransfer");
                SentFileTransfer sentFileTransfer = (SentFileTransfer)fileTransfer;
                sentFileTransfer.onTransferProgress(currentSize, totalSize);

            } else {
                Logger.v(TAG, "onProgressUpdate fileTransfer is ReceiveFileTransfer");
                ReceiveFileTransfer rcvFileTransfer = (ReceiveFileTransfer)fileTransfer;
                rcvFileTransfer.onTransferProgress(currentSize, totalSize);
            }
        }

        public void onDeleted(ContactId contact, Set<String> transferIds){
            Logger.v(TAG, "onDeleted() this file is "
                    + transferIds +",contact=" + contact.toString());
            /*Object fileTransfer = (Object) getFileTransferImpl(transferId);
            if(fileTransfer == null){
                Logger.v(TAG, "onDeleted fileTransfer is null");
                return;
            }*/
        }
    }

    /**
     * File transfer session event listener for group chat.
     *.
     */
    private class GroupModelFileTransferListener extends GroupFileTransferListener {
        private static final String TAG = "M0CFF GroupModelFileTransferListener";

        public void onStateChanged(String chatId, String transferId, FileTransfer.State state,
                FileTransfer.ReasonCode reasonCode){
            Logger.v(TAG, "onStateChanged() this file is "
                    + transferId + ",state" + state + ", chatid=" + chatId + ", reason:" + reasonCode.toInt());
            Object fileTransfer = (Object) getFileTransferImpl(transferId);
            if(fileTransfer == null){
                Logger.v(TAG, "onStateChanged fileTransfer is null");
                return;
            }

            try {
                if(fileTransfer instanceof SentFileTransfer){
                    Logger.v(TAG, "onStateChanged fileTransfer is SentFileTransfer");
                    SentFileTransfer sentFileTransfer = (SentFileTransfer)fileTransfer;
                    String filename = sentFileTransfer.mFileTransferObject.getFileName();
                    if(state == FileTransfer.State.TRANSFERRED){
                        sentFileTransfer.onFileTransferred(filename);
                    } else if(state == FileTransfer.State.STARTED){
                        sentFileTransfer.onTransferStarted();
                    } else if(state == FileTransfer.State.ABORTED ){
                        if(reasonCode == FileTransfer.ReasonCode.REJECTED_BY_SECONDARY_DEVICE){
                            sentFileTransfer.onTransferError(reasonCode.toInt());
                        } else {
                        sentFileTransfer.onTransferAborted(reasonCode.toInt());
                        }
                    } else if(state == FileTransfer.State.REJECTED ||
                            state == FileTransfer.State.FAILED){
                        sentFileTransfer.onTransferError(reasonCode.toInt());
                    }
                } else {
                    Logger.v(TAG, "onStateChanged fileTransfer is ReceiveFileTransfer");
                    ReceiveFileTransfer rcvFileTransfer = (ReceiveFileTransfer)fileTransfer;
                    String filename = rcvFileTransfer.mFileTransferObject.getFileName();
                    if(state == FileTransfer.State.TRANSFERRED){
                        rcvFileTransfer.onFileTransferred(filename);
                    } else if(state == FileTransfer.State.STARTED){
                        rcvFileTransfer.onTransferStarted();
                    } else if(state == FileTransfer.State.ABORTED ){
                        rcvFileTransfer.onTransferAborted(reasonCode.toInt());
                    } else if(state == FileTransfer.State.REJECTED ||
                            state == FileTransfer.State.FAILED){
                        rcvFileTransfer.onTransferError(reasonCode.toInt());
                    } else if(state == FileTransfer.State.DELIVERED ){
                        //sentFileTransfer.onTransferAborted();
                    } else if(state == FileTransfer.State.DISPLAYED ){
                        //sentFileTransfer.onTransferAborted();
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        public void onDeliveryInfoChanged(String chatId, ContactId contact, String transferId,
                GroupDeliveryInfo.Status status, GroupDeliveryInfo.ReasonCode reasonCode){
            Logger.v(TAG, "onDeliveryInfoChanged() this file is "
                    + transferId + ",status" + status.toInt() + ", chatid=" + chatId + ", contact:" + contact.toString() + ",reason:" + reasonCode.toInt());
        }

        public void onProgressUpdate(String chatId, String transferId, long currentSize,
                long totalSize){
            Logger.v(TAG, "onProgressUpdate() this file is "
                    + transferId + ",currentsize" + currentSize + ", totalsize ="+ totalSize + ", chatid=" + chatId);
            Object fileTransfer = (Object) getFileTransferImpl(transferId);
            if(fileTransfer == null){
                Logger.v(TAG, "onProgressUpdate fileTransfer is null");
                return;
            }
            if(fileTransfer instanceof SentFileTransfer){
                Logger.v(TAG, "onProgressUpdate fileTransfer is SentFileTransfer");
                SentFileTransfer sentFileTransfer = (SentFileTransfer)fileTransfer;
                sentFileTransfer.onTransferProgress(currentSize, totalSize);

            } else {
                Logger.v(TAG, "onProgressUpdate fileTransfer is ReceiveFileTransfer");
                ReceiveFileTransfer rcvFileTransfer = (ReceiveFileTransfer)fileTransfer;
                rcvFileTransfer.onTransferProgress(currentSize, totalSize);
            }
        }

        public void onDeleted(String chatId, Set<String> transferIds){
            Logger.v(TAG, "onDeleted() this file is "
                    + transferIds +",chatid=" + chatId);
        }
    }


    /**
     * This class describe one single out-going file transfer, and control the
     * status itself.
     *.
     */
    public static class SentFileTransfer implements
            OnNetworkStatusChangedListerner {
        private static final String TAG = "M0CFF SentFileTransfer";
        public static final String KEY_FILE_TRANSFER_TAG = "file transfer tag";

        protected Object mChatTag = null;

        protected Object mFileTransferTag = null;

        protected FileStruct mFileStruct = null;

        protected IFileTransfer mFileTransfer = null;

        protected FileTransfer mFileTransferObject = null;

        //protected FileTransferListener mFileTransferListener = null;

        protected Participant mParticipant = null;
        protected List<String> mParticipants = null;
        protected IChat1 mChat = null;

        /**
         * @param chatTag .
         * @param chat .
         * @param groupChat .
         * @param filePath .
         * @param participants .
         * @param fileTransferTag .
         */
        public SentFileTransfer(Object chatTag, IChat1 chat,
                IGroupChatWindow groupChat, String filePath,
                List<String> participants, Object fileTransferTag) {

            Logger.v(TAG, "SentFileTransfer() constructor chatTag is "
                    + chatTag + " one2OneChat is " + groupChat
                    + " filePath is " + filePath + "fileTransferTag: "
                    + fileTransferTag);
            if (null != chatTag && null != groupChat && null != filePath
                    && null != participants) {
                mChatTag = chatTag;
                mChat = chat;
                mFileTransferTag = (fileTransferTag != null ? fileTransferTag
                        : UUID.randomUUID());
                mFileStruct = new FileStruct(filePath,
                        extractFileNameFromPath(filePath), 0, mFileTransferTag,
                        new Date());
                mFileTransfer = groupChat.addSentFileTransfer(mFileStruct);
                mFileTransfer.setStatus(Status.PENDING);
                mParticipants = participants;
                NetworkChangedReceiver.addListener(this);
            }

        }

        /**
         * @param chatTag .
         * @param one2OneChat .
         * @param filePath .
         * @param participant .
         * @param fileTransferTag .
         */
        public SentFileTransfer(Object chatTag, IOne2OneChatWindow one2OneChat,
                String filePath, Participant participant, Object fileTransferTag) {
            Logger.v(TAG, "SentFileTransfer() constructor chatTag is "
                    + chatTag + " one2OneChat is " + one2OneChat
                    + " filePath is " + filePath + "fileTransferTag: "
                    + fileTransferTag);
            if (null != chatTag && null != one2OneChat && null != filePath
                    && null != participant) {
                mChatTag = chatTag;
                mFileTransferTag = (fileTransferTag != null ? fileTransferTag
                        : UUID.randomUUID());
                mFileStruct = new FileStruct(filePath,
                        extractFileNameFromPath(filePath), 0, mFileTransferTag,
                        new Date());
                mFileTransfer = one2OneChat.addSentFileTransfer(mFileStruct);
                mFileTransfer.setStatus(Status.PENDING);
                mParticipant = participant;
                NetworkChangedReceiver.addListener(this);
            }
        }

        /**
         * @param chatTag .
         * @param one2OneChat .
         * @param filePath .
         * @param participant .
         * @param fileTransferTag .
         */
        public SentFileTransfer(Object chatTag, IMultiChatWindow one2OneChat,
                String filePath, Participant participant, Object fileTransferTag) {
            Logger.v(TAG, "SentFileTransfer() constructor chatTag is "
                    + chatTag + " one2OneChat is " + one2OneChat
                    + " filePath is " + filePath + "fileTransferTag: "
                    + fileTransferTag);
            if (null != chatTag && null != one2OneChat && null != filePath
                    && null != participant) {
                mChatTag = chatTag;
                mFileTransferTag = (fileTransferTag != null ? fileTransferTag
                        : UUID.randomUUID());
                mFileStruct = new FileStruct(filePath,
                        extractFileNameFromPath(filePath), 0, mFileTransferTag,
                        new Date());
                mFileTransfer = one2OneChat.addSentFileTransfer(mFileStruct);
                mFileTransfer.setStatus(Status.PENDING);
                mParticipant = participant;
                NetworkChangedReceiver.addListener(this);
            }
        }

        /**
         * @param chatTag .
         * @param one2OneChat .
         * @param filePath .
         * @param participant .
         * @param fileTransferTag .
         */
        public SentFileTransfer(IFileTransfer fileTransfer,
                Participant participant, Object fileTransferTag,
                FileStruct fileStruct) {
            Logger.v(TAG, "SentFileTransferResume constructor "
                    + "fileTransferTag: " + fileTransferTag);
            if (null != participant) {
                this.mFileTransferTag = fileTransferTag;
                this.mFileStruct = fileStruct;
                this.mFileTransfer = fileTransfer;
                this.mFileTransfer.setStatus(Status.PENDING);
                this.mParticipant = participant;
                // this.mOnSendFinishListener = mOutGoingFileTransferManager;
                NetworkChangedReceiver.addListener(this);
            }
        }

        public void resumeReceiverFileTransfer() {
            Logger.i(TAG, "resumeMsrpFileTransfer() resume file transfer");
        }

        public boolean isResumeTransfer(){
            if(this instanceof ResumeFileTransfer){
                return ((ResumeFileTransfer) this).isReceiveFileTransfer;
            } else {
                return false;
            }
        }

        public void resumeMsrpFileTransfer() {
            Logger.i(TAG, "resumeMsrpFileTransfer() resume file transfer");
            final boolean isReceive = ((ResumeFileTransfer) this).isReceiveFileTransfer;
            // Send text message
            Thread t = new Thread() {
                public void run() {
                    try {
                        ApiManager instance = ApiManager.getInstance();
                        Logger.i(TAG, "resumeMsrpFileTransfer(), isReceive: "
                                + isReceive);
                        if (instance != null) {
                            FileTransferService fileTransferService = instance
                                    .getFileTransferApi();
                            if (fileTransferService != null) {
                                Logger.i(TAG,
                                        "resumeMsrpFileTransfer(), active0");
                                try {
                                    if (mFileTransfer != null) {
                                        mFileTransfer
                                                .setStatus(ChatView.IFileTransfer.Status.FAILED);
                                    }
                                    Logger.i(
                                            TAG,
                                            "resumeMsrpFileTransfer(), active1:"
                                                    + mFileTransferTag
                                                            .toString());
                                    // if(((ResumeFileTransfer)this).isReceiveFileTransfer
                                    // == false){
                                    //mFileTransferListener = new FileTransferSenderListener();
                                    // }
                                    /*mFileTransferObject = fileTransferService
                                            .resumeFileTransfer(
                                                    mFileTransferTag.toString()
                                                    );*///TODO
                                    if (null != mFileTransferObject) {
                                        long currentSize = mFileTransferObject
                                                .getFileSize();
                                        long totalSize = mFileStruct.mSize;
                                        String fileTransferId = mFileTransferObject
                                                .getTransferId();
                                        Logger.i(TAG,
                                                "resumeMsrpFileTransfer(), active1"
                                                        + fileTransferId + "currentSize" + currentSize + "totalsze" + totalSize);
                                        //mFileTransfer.updateTag(fileTransferId,
                                               // mFileStruct.mSize);
                                        mFileTransferTag = fileTransferId;
                                        mFileStruct.mFileTransferTag = fileTransferId;
                                        mFileTransfer
                                               .setStatus(Status.TRANSFERING);
                                        //mFileTransferListener.onTransferProgress(currentSize, totalSize);

                                        if (!isReceive) {
                                            setNotification();
                                        }
                                    } else {
                                        // mFileTransfer.setStatus(Status.WAITING);
                                        Logger.e(TAG,
                                                "send() failed, mFileTransferObject is null, filePath is active01"
                                                        + mFileStruct.mFilePath);
                                        onFailed();
                                        onFileTransferFinished(IOnSendFinishListener.Result.RESUMABLE);
                                    }
                                } catch (Exception e) {
                                    e.printStackTrace();
                                    Logger.i(TAG,
                                            "resumeMsrpFileTransfer(),  active3");
                                    onFailed();
                                    onFileTransferFinished(IOnSendFinishListener.Result.RESUMABLE);
                                }
                                ;
                            }
                        }
                    } catch (Exception e) {
                        Logger.i(TAG, "exception in resumeFileTransfer");
                        e.printStackTrace();
                    }
                }
            };
            t.start();
        }

        /**
         * Create a thumbnail from a filename.
         * .
         * @param filename
         *            Filename.
         * @return Thumbnail.
         */
        public static byte[] createFileThumbnail(String filename) {
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            InputStream in = null;
            try {
                File file = new File(filename);
                in = new FileInputStream(file);
                Bitmap bitmap = BitmapFactory.decodeStream(in);
                int width = bitmap.getWidth();
                int height = bitmap.getHeight();
                long size = file.length();

                // Resize the bitmap
                float scale = 0.4f;
                Matrix matrix = new Matrix();
                matrix.postScale(scale, scale);

                // Recreate the new bitmap
                Bitmap resizedBitmap = Bitmap.createBitmap(bitmap, 0, 0, width,
                        height, matrix, true);

                // Compress the file to be under the limit (10KBytes)
                int quality = 90;
                int maxSize = 1024 * 10;
                if (size > maxSize) {
                    while (size > maxSize) {
                        out = new ByteArrayOutputStream();
                        resizedBitmap.compress(CompressFormat.JPEG, quality,
                                out);
                        out.flush();
                        out.close();
                        size = out.size();
                        quality -= 10;
                    }
                } else {
                    out = new ByteArrayOutputStream();
                    resizedBitmap.compress(CompressFormat.JPEG, 90, out);
                    out.flush();
                    out.close();
                }
                if (in != null) {
                    in.close();
                }
            } catch (NullPointerException e) {
                return null;
            } catch (FileNotFoundException e) {
                return null;
            } catch (IOException e) {
                return null;
            }
            return out.toByteArray();
        }

        /**
         * Create a thumbnail from a filename.
         * .
         * @param filename
         *            Filename.
         * @return Thumbnail.
         */
        public static byte[] createImageThumbnail(String filename) {
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            InputStream in = null;
            try {
                File file = new File(filename);
                in = new FileInputStream(file);
                Bitmap bitmap = BitmapFactory.decodeStream(in);
                int width = bitmap.getWidth();
                int height = bitmap.getHeight();
                long size = file.length();

                // Resize the bitmap

                float scale = 0.1f;
                if (size > 1000 * 1024) {
                    scale = 0.02f;
                } else if (size > 500 * 1024) {
                    scale = 0.04f;
                } else if (size > 300 * 1024) {
                    scale = 0.1f;
                } else if (size < 300 * 1024 && size > 50 * 1024) {
                    scale = 0.15f;
                } else {
                    scale = 0.2f;
                }

                Matrix matrix = new Matrix();
                matrix.postScale(scale, scale);

                // Recreate the new bitmap
                Bitmap resizedBitmap = Bitmap.createBitmap(bitmap, 0, 0, width,
                        height, matrix, true);

                // Compress the file to be under the limit (10KBytes)
                int quality = 90;
                int maxSize = 1024 * 2;
                if (size > maxSize) {
                    while (size > maxSize) {
                        out = new ByteArrayOutputStream();
                        resizedBitmap.compress(CompressFormat.JPEG, quality,
                                out);
                        out.flush();
                        out.close();
                        size = out.size();
                        quality -= 5;
                    }
                } else {
                    out = new ByteArrayOutputStream();
                    resizedBitmap.compress(CompressFormat.JPEG, 90, out);
                    out.flush();
                    out.close();
                }
                if (in != null) {
                    in.close();
                }
            } catch (NullPointerException e) {
                if (in != null) {
                    try {
                        in.close();
                    } catch (IOException m) {
                        m.printStackTrace();
                    }
                }
                return null;
            } catch (FileNotFoundException n) {
                if (in != null) {
                    try {
                        in.close();
                    } catch (IOException e) {
                        n.printStackTrace();
                    }
                }
                return null;
            } catch (IOException e) {
                return null;
            }
            return out.toByteArray();
        }

        /**
         * @param fileName Name of the file.
         * @return return thumbnail file name.
         */
        public String getThumnailFile(String fileName) {
            return null;
        }

        private Set<String> convertParticipantsToContactsSet(
                List<String> participants) {
            Set<String> pList = new HashSet<String>();
            int size = participants.size();
            for (int i = 0; i < size; i++) {
                pList.add(participants.get(i));
            }
            return pList;
        }

        /**
         * .
         */
        protected void send() {
            Logger.w(TAG, "checkNext() send new file");
            ApiManager instance = ApiManager.getInstance();
            if (instance != null) {
                FileTransferService fileTransferService = instance
                        .getFileTransferApi();
                CapabilityService capabilityApi = ApiManager.getInstance().getCapabilityApi();
                boolean useLargeMode = false;
                boolean useFTMode = false;
                if (fileTransferService != null) {
                    Capabilities currentRemoteCapablities = null;
                    try {
                        try {
                            currentRemoteCapablities = capabilityApi.
                                    getContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(mParticipant.getContact()));
                            if(currentRemoteCapablities != null) {
                                 useFTMode= currentRemoteCapablities.isFileTransferSupported();
                            }

                        } catch (Exception e) {
                            e.printStackTrace();
                            Logger.e(TAG,"M0CFF checkNext() getContactCapabilities JoynServiceException ");
                        }
                            boolean isStandAloneSupported = RcsSettings.getInstance().isCPMStandAloneModeSupported();
                            if (RcsSettings.getInstance().isSupportOP08()) {
                                if(currentRemoteCapablities != null) {
                                    isStandAloneSupported = currentRemoteCapablities.isStandaloneMsgSupported();
                                }
                            }
                            useLargeMode = !useFTMode && isStandAloneSupported;
                            Logger.v(TAG, "send() file isStandAloneSupported: " + isStandAloneSupported + " useFTMode: " + useFTMode);
                        Logger.v(TAG, "send() file useLargeMode: " + useLargeMode);
                        //mFileTransferListener = new FileTransferSenderListener();
                        if (mParticipant != null) {
                            if(useLargeMode) {
                                Logger.w(TAG,
                                        "checkNext() send new file largemode supported");
                                if(mParticipant.getContact().contains(",")){
                                    Logger.v(TAG, "send() file send multiple ");
                                    mFileTransferObject = fileTransferService
                                            .transferFileToMultiple(
                                                    generateContactsList(mParticipant.getContact()),
                                                    Uri.fromFile(new File(mFileStruct.mFilePath)),
                                                    Disposition.ATTACH, true
                                                    );
                                } else {
                                mFileTransferObject = fileTransferService
                                        .transferFileLargeMode(
                                                ContactIdUtils.createContactIdFromTrustedData(mParticipant.getContact()),
                                                Uri.fromFile(new File(mFileStruct.mFilePath)),
                                                Disposition.ATTACH, true
                                                );
                                }
                            } else {
                                if (RcsSettings.getInstance()
                                        .isFileTransferThumbnailSupported()) {
                                    Logger.w(TAG,
                                            "checkNext() send new file thumbnail supported");
                                    mFileTransferObject = fileTransferService
                                            .transferFile(
                                                    ContactIdUtils.createContactIdFromTrustedData(mParticipant.getContact()),
                                                    Uri.fromFile(new File(mFileStruct.mFilePath)),
                                                    true);
                                } else {
                                    Logger.w(TAG,
                                            "checkNext() send new file thumbnail not supported");
                                    mFileTransferObject = fileTransferService
                                            .transferFile(
                                                    ContactIdUtils.createContactIdFromTrustedData(mParticipant.getContact()),
                                                    Uri.fromFile(new File(mFileStruct.mFilePath)),
                                                    false);
                                }
                            }
                        } else {
                          if(mChat != null){
                            GroupChat mGroupChat = ((GroupChat1) mChat)
                                    .getGroupChatImpl();
                            try {
                                GroupChat.State state = mGroupChat.getState();
                                GroupChat.ReasonCode reasonCode = mGroupChat.getReasonCode();
                                Logger.v(TAG, "checkNext() state:" + state.toInt() + ",reason:" + reasonCode.toInt());
                                if(state == GroupChat.State.ABORTED && reasonCode == GroupChat.ReasonCode.REJECTED_BY_SECONDARY_DEVICE) {
                                    sendFilebyMMS();
                                    onFailed();
                                    onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                                    return;
                                }
                            } catch (Exception e) {
                                e.printStackTrace();
                            }
                          if(mGroupChat != null) {
                            if (RcsSettings.getInstance()
                                    .isFileTransferThumbnailSupported()) {
                                Logger.w(TAG,
                                        "checkNext() send new group file thumbnail supported");
                                mFileTransferObject = fileTransferService
                                        .transferFileToGroupChat(
                                                mGroupChat.getChatId(),
                                                Uri.fromFile(new File(mFileStruct.mFilePath)),
                                                true);
                            } else {
                                Logger.w(TAG,
                                        "checkNext() send new group file thumbnail not" +
                                        " supported");
                                mFileTransferObject = fileTransferService
                                        .transferFileToGroupChat(
                                                mGroupChat.getChatId(),
                                                Uri.fromFile(new File(mFileStruct.mFilePath)),
                                                false);
                            }
                          }
                         }
                        }
                        if (null != mFileTransferObject) {
                            mFileStruct.mSize = mFileTransferObject
                                    .getFileSize();
                            String fileTransferId = mFileTransferObject
                                    .getTransferId();
                            Logger.v(TAG, "send() transferid add map" + fileTransferId);
                            sFileTransferImplMap.put(fileTransferId, this);
                            mFileTransfer.updateTag(fileTransferId,
                                    mFileStruct.mSize);
                            mFileTransferTag = fileTransferId;
                            mFileStruct.mFileTransferTag = fileTransferId;
                            mFileTransfer.setStatus(Status.WAITING);
                            setNotification();
                        } else {
                            // mFileTransfer.setStatus(Status.WAITING);
                            Logger.e(TAG,
                                    "send() failed, mFileTransferObject is null, filePath is "
                                            + mFileStruct.mFilePath);
                            onFailed();
                            onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                        }
                    }  catch (Exception e) {
                        e.printStackTrace();
                        onFailed();
                        onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                    }
                }
            }
        }

        public List<String> generateContactsList(String contact) {
            String[] splited = null;
            List<String> result = new ArrayList<String>();

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

        protected void sendbyLargeMode() {
            Logger.v(TAG, "sendbyLargeMode() send new file");
            try{
                FileTransferService fileTransferService = ApiManager.getInstance()
                        .getFileTransferApi();
                mFileTransferObject = fileTransferService
                        .transferFileLargeMode(
                                ContactIdUtils.createContactIdFromTrustedData(mParticipant.getContact()),
                                Uri.fromFile(new File(mFileStruct.mFilePath)),
                                Disposition.ATTACH, true
                                );

                if (null != mFileTransferObject) {
                    mFileStruct.mSize = mFileTransferObject
                            .getFileSize();
                    String fileTransferId = mFileTransferObject
                            .getTransferId();
                    Logger.v(TAG, "send() transferid add map" + fileTransferId);
                    sFileTransferImplMap.put(fileTransferId, this);
                    mFileTransfer.updateTag(fileTransferId,
                            mFileStruct.mSize);
                    mFileTransferTag = fileTransferId;
                    mFileStruct.mFileTransferTag = fileTransferId;
                    mFileTransfer.setStatus(Status.WAITING);
                    //setNotification();
                } else {
                    // mFileTransfer.setStatus(Status.WAITING);
                    Logger.e(TAG,
                            "sendbyLargeMode() failed, mFileTransferObject is null, filePath is "
                                    + mFileStruct.mFilePath);
                    onFailed();
                    onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                }
            } catch (Exception e) {
                e.printStackTrace();
                onFailed();
                onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
            }
        }



        /**
         * .
         */
        protected void resend() {
            Logger.w(TAG, "resend() send file:" + (String)mFileTransferTag);
            ApiManager instance = ApiManager.getInstance();
            if (instance != null) {
                FileTransferService fileTransferService = instance
                        .getFileTransferApi();
                CapabilityService capabilityApi = ApiManager.getInstance().getCapabilityApi();
                //boolean useLargeMode = false;
                if (fileTransferService != null) {
                    try {
                        if (mParticipant != null) {
                            Logger.v(TAG, "resend() send file mParticipant:" + mParticipant.getContact());
                            mFileTransferObject = fileTransferService.getFileTransfer((String)mFileTransferTag);
                            Logger.v(TAG, "resend() send file :" + mFileTransferObject);
                            if(mFileTransferObject != null){
                                mFileTransferObject.resendTransfer();
                            }
                        } else {
                          if(mChat != null){
                            Logger.v(TAG, "resend() send file mChat:" + mChat);
                            GroupChat mGroupChat = ((GroupChat1) mChat)
                                    .getGroupChatImpl();
                            Logger.v(TAG, "resend() send file mGroupChat:" + mGroupChat);
                            if(mGroupChat != null) {
                                mFileTransferObject = fileTransferService
                                        .resendFileTransfer((String)mFileTransferTag,
                                                mGroupChat.getChatId(),
                                                mParticipants,
                                                Uri.fromFile(new File(mFileStruct.mFilePath)),
                                                true);
                            }
                         }
                        }
                        if (null != mFileTransferObject) {
                            mFileStruct.mSize = mFileTransferObject
                                    .getFileSize();
                            String fileTransferId = mFileTransferObject
                                    .getTransferId();
                            Logger.v(TAG, "send() transferid add map" + fileTransferId);
                            sFileTransferImplMap.put(fileTransferId, this);
                            mFileTransfer.updateTag(fileTransferId,
                                    mFileStruct.mSize);
                            mFileTransferTag = fileTransferId;
                            mFileStruct.mFileTransferTag = fileTransferId;
                            mFileTransfer.setStatus(Status.WAITING);
                            setNotification();
                        } else {
                            // mFileTransfer.setStatus(Status.WAITING);
                            Logger.e(TAG,
                                    "send() failed, mFileTransferObject is null, filePath is "
                                            + mFileStruct.mFilePath);
                            onFailed();
                            onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                        }
                    }  catch (Exception e) {
                        e.printStackTrace();
                        onFailed();
                        onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                    }
                }
            }
        }

        private void onPrepareResend() {
            Logger.v(TAG, "onPrepareResend() file " + mFileStruct.mFilePath
                    + " to " + mParticipant);
            if (null != mFileTransfer) {
                mFileTransfer.setStatus(Status.PENDING);
            }
        }

        private void onPause() {
            // Logger.v(TAG, "onPause() file " + mFileStruct.mFilePath
            // + " to " + mParticipant);
            Logger.v(TAG, "onPause() file 123");
            if (null != mFileTransfer) {
                try {
                    Logger.v(TAG, "onPause 1");
                    mFileTransferObject.pauseTransfer();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }

        private void onResume() {
            // Logger.v(TAG, "onResume() file " + mFileStruct.mFilePath
            // + " to " + mParticipant);
            Logger.v(TAG, "onResume 123");
            if (null != mFileTransfer) {
                try {
                    Logger.v(TAG, "onResume 1");
                    if (null != mFileTransferObject) {
                        // if(mFileTransferObject.isSessionPaused()){ ////TODo
                        // check this
                        Logger.v(TAG, "onResume 2");
                        //if (mFileTransferObject.isHttpFileTransfer()) {
                        mFileTransferObject.resumeTransfer();
                        //}
                        // }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }

        private void onCancel() {
            Logger.v(TAG, "onCancel() entry: mFileTransfer = " + mFileTransfer);
            if (null != mFileTransfer) {
                mFileTransfer.setStatus(Status.CANCEL);
            }
        }

        private void onFailed() {
            Logger.v(TAG, "onFailed() entry: mFileTransfer = " + mFileTransfer);
            if (null != mFileTransfer) {
                mFileTransfer.setStatus(Status.FAILED);
            }
        }

        private void onNotAvailable(int reason) {
            Logger.v(TAG, "onNotAvailable() reason is " + reason);
            if (One2OneChat.FILETRANSFER_ENABLE_OK == reason) {
                return;
            } else {
                switch (reason) {
                case One2OneChat.FILETRANSFER_DISABLE_REASON_REMOTE:
                    onFailed();
                    break;
                case One2OneChat.FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED:
                case One2OneChat.FILETRANSFER_DISABLE_REASON_NOT_REGISTER:
                    onFailed();
                    break;
                default:
                    Logger.w(TAG, "onNotAvailable() unknown reason " + reason);
                    break;
                }
            }
        }

        private void onDestroy() {
            Logger.v(TAG, "onDestroy() sent file transfer mFilePath "
                    + ((null == mFileStruct) ? null : mFileStruct.mFilePath)
                    + " mFileTransferObject = " + mFileTransferObject
                    + ", mFileTransferListener = ");
            if (null != mFileTransferObject) {
                try {
                    /*if (null != mFileTransferListener) {
                        mFileTransferObject
                                .removeEventListener(mFileTransferListener);
                    }*/
                    cancelNotification();
                    mFileTransferObject.abortTransfer();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }

        /**
         * @param result .
         */
        protected void onFileTransferFinished(
                IOnSendFinishListener.Result result) {
            Logger.v(TAG, "onFileTransferFinished() mFileStruct = "
                    + mFileStruct + ", file = "
                    + ((null == mFileStruct) ? null : mFileStruct.mFilePath)
                    + ", mOnSendFinishListener = " + mOnSendFinishListener
                    + ", mFileTransferListener = "
                    + ", result = " + result);
            if (null != mOnSendFinishListener) {
                mOnSendFinishListener.onSendFinish(SentFileTransfer.this,
                        result);
                if (null != mFileTransferObject) {
                    try {
                        /*if (null != mFileTransferListener) {
                            mFileTransferObject
                                    .removeEventListener(mFileTransferListener);
                        }*/
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
            // Remove network listener when transfer is finished.
            NetworkChangedReceiver.removeListener(this);
        }

        /**
         * @param filePath Path of file to be transferred .
         * @return return file path .
         */
        public static String extractFileNameFromPath(String filePath) {
            if (null != filePath) {
                int lastDashIndex = filePath.lastIndexOf("/");
                if (-1 != lastDashIndex
                        && lastDashIndex < filePath.length() - 1) {
                    String fileName = filePath.substring(lastDashIndex + 1);
                    return fileName;
                } else {
                    Logger.e(TAG,
                            "extractFileNameFromPath() invalid file path:"
                                    + filePath);
                    return filePath;
                }
            } else {
                Logger.e(TAG, "extractFileNameFromPath() filePath is null");
                return null;
            }
        }


        /**
         * File transfer session event listener.
         *.
         */
        //private class FileTransferSenderListener extends OneToOneFileTransferListener {}

        /**
         * Callback called when the file transfer is started .
         */
        public void onTransferStarted() {
            Logger.v(TAG, "onTransferStarted() this file is "
                    + mFileStruct.mFilePath);
        }

        /**
         * Callback called when the file transfer has been aborted .
         */
        public void onTransferAborted(int error) {
            Logger.e(TAG,
                    "File transfer onTransferAborted(): mFileTransfer = "
                            + mFileTransfer);
            if (mParticipant == null) {
                Logger.v(TAG,
                        "FileTransferSenderListener onTransferAborted mParticipant is null");
                return;
            }
            if(error == FileTransfer.ReasonCode.ABORTED_BY_USER.toInt() ||
               error == FileTransfer.ReasonCode.REJECTED_BY_USER.toInt() ){
                return;
            }
            if (mFileTransfer != null) {
                mFileTransfer.setStatus(Status.FAILED);
                IChat1 chat = ModelImpl.getInstance().getChat(mChatTag); // TODo
                                                                         // Check
                                                                         // this
                Logger.v(TAG, "onTransferAborted(): chat = " + chat);
                if (chat instanceof ChatImpl) {
                    ((ChatImpl) chat).checkCapabilities();
                }
            }
            /*try {
                //mFileTransferObject.removeEventListener(this);
            } catch (Exception e) {
                e.printStackTrace();
            }*/
            mFileTransferObject = null;
            if (RcsSettings.getInstance().isSupportOP08()) {
                onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
            } else {
            onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
        }
        }

        /**
         * Callback called when the file transfer is paused.
         * .
         */
        public void onTransferPaused() {
            // notify that this chat have a file transfer
            Logger.e(TAG, "onTransferStarted() entry");
        }
        /**
         * Callback called when the file transfer is resumed.
         * .
         */
        public void onTransferResumed(String oldFTid, String newFTId) {
            // notify that this chat have a file transfer
            Logger.v(TAG, "onTransferStarted() entry");
        }

        /**
         * Callback called when the transfer has failed.
         * .
         * @param error
         *            Error.
         * @see FileTransfer.Error.
         */
        public void onTransferError(int error) {
            Logger.e(TAG, "onTransferError(),error = " + error
                    + ", mFileTransfer =" + mFileTransfer);
            //switch (error) {
            if(error == FileTransfer.ReasonCode.REJECTED_BY_TIMEOUT.toInt()){
                Logger.v(TAG,
                        "onTransferError(), the file transfer invitation is failed.");
                if (mFileTransfer != null) {
                    mFileTransfer
                            .setStatus(ChatView.IFileTransfer.Status.TIMEOUT);
                }
                onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);

            } else if(error == FileTransfer.ReasonCode.FAILED_DATA_TRANSFER.toInt()) {

                Logger.v(TAG,
                        "onTransferError(), file transfer failed , send by MMS");

                if (mFileTransfer != null) {
                    mFileTransfer
                            .setStatus(ChatView.IFileTransfer.Status.FAILED);
                }
                onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                sendFilebyMMS();
            } else if(error == FileTransfer.ReasonCode.AUTO_RESEND.toInt()) {

                Logger.v(TAG,
                        "onTransferError(), file transfer failed , autoresend");

                /*if (mFileTransfer != null) {
                    mFileTransfer
                            .setStatus(ChatView.IFileTransfer.Status.FAILED);
                }
                onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);*/
                AsyncTask.execute(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            mFileTransferObject.resendTransfer();
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                });
            } else if(error == FileTransfer.ReasonCode.FAILED_NOT_ALLOWED_TO_SEND.toInt()) {

                Logger.v(TAG,
                        "onTransferError(), file transfer failed , fallback to larger mode");

                /*if (mFileTransfer != null) {
                    mFileTransfer
                            .setStatus(ChatView.IFileTransfer.Status.FAILED);
                }
                onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);*/
                AsyncTask.execute(new Runnable() {
                    @Override
                    public void run() {
                        sendbyLargeMode();
                    }
                });

            } else if(error == FileTransfer.ReasonCode.ABORTED_BY_REMOTE.toInt() ||
                    error == FileTransfer.ReasonCode.REJECTED_BY_REMOTE.toInt()) {

                Logger.v(TAG,
                        "onTransferError(), your file transfer invitation has been rejected");
                if (mFileTransfer != null) {
                    mFileTransfer
                            .setStatus(ChatView.IFileTransfer.Status.REJECTED);
                }
                onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
            } else if (error == FileTransfer.ReasonCode.FAILED_FALLBACK_MMS.toInt()) {
                Logger.v(TAG,
                        "onTransferError(), fallback to MMS");
                if (mFileTransfer != null) {
                    mFileTransfer
                            .setStatus(ChatView.IFileTransfer.Status.FALLBACK_MMS);
                }
                onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
            } else if (error == FileTransfer.ReasonCode.FAILED_INITIATION.toInt()) {
                Logger.v(TAG,
                        "onTransferError(), your file transfer invitation has failed");
                if (mFileTransfer != null) {
                    mFileTransfer
                            .setStatus(ChatView.IFileTransfer.Status.FAILED);
                }
                onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
            } else if (error == FileTransfer.ReasonCode.FAILED_SAVING.toInt()) {
                Logger.v(TAG, "onTransferError(), saving of file failed");
                if (mFileTransfer != null) {
                    mFileTransfer
                            .setStatus(ChatView.IFileTransfer.Status.FAILED);
                }
                onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
            } else {
                Logger.e(TAG, "onTransferError() unknown error " + error);
                onFailed();
                onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
            }
            /*case FileTransfer.ReasonCode.TRANSFER_RESUME:
                Logger.i(TAG,
                        "onTransferError(), need to resume failed file");
                onFileTransferFinished(IOnSendFinishListener.Result.RESUMABLE);
                try {
                    if (null != mFileTransferListener) {
                        mFileTransferObject
                                .removeEventListener(mFileTransferListener);
                    }
                } catch(Exception e){
                    Logger.i(TAG,
                            "onTransferError(), MEDIA_RESUME_FAILED  failed to remove listener");
                    e.printStackTrace();
                }
                Logger.i(TAG,
                        "onTransferError(), MEDIA_RESUME_FAILED  add to active");

             // Send text message
                Thread t = new Thread() {
                    public void run() {
                         ApiManager instance = ApiManager.getInstance();
                        Logger.i(TAG,
                                "onTransferError(), MEDIA_RESUME_FAILED  add to active0");
                         if (instance != null) {
                            FileTransferService fileTransferService = instance
                                    .getFileTransferApi();
                                   if (fileTransferService != null) {
                                Logger.i(TAG,
                                        "onTransferError(), MEDIA_RESUME_FAILED  add to active3");
                                  try {
                                    if (mFileTransfer != null) {
                                        mFileTransfer
                                                .setStatus(ChatView.IFileTransfer.Status.FAILED);
                                    }
                                    Logger.i(
                                            TAG,
                                            "onTransferError(), MEDIA_RESUME_FAILED  add to active1:"
                                                    + mFileTransferTag
                                                            .toString());
                                    mFileTransferListener = new FileTransferSenderListener();
                                    mFileTransferObject = fileTransferService
                                            .resumeFileTransfer(
                                                    mFileTransferTag
                                                            .toString(),
                                                    mFileTransferListener);
                                    if (null != mFileTransferObject) {
                                        mFileStruct.mSize = mFileTransferObject
                                                .getFileSize();
                                        String fileTransferId = mFileTransferObject
                                                .getTransferId();
                                        Logger.i(TAG,
                                                "onTransferError(), MEDIA_RESUME_FAILED  add to active"
                                                        + fileTransferId);
                                        mFileTransfer.updateTag(
                                                fileTransferId,
                                                mFileStruct.mSize);
                                        mFileTransferTag = fileTransferId;
                                        mFileStruct.mFileTransferTag = fileTransferId;
                                        mFileTransfer
                                                .setStatus(Status.WAITING);
                                        setNotification();
                                    } else {
                                        // mFileTransfer.setStatus(Status.WAITING);
                                        Logger.e(
                                                TAG,
                                                "send() failed, mFileTransferObject is null, filePath is "
                                                        + mFileStruct.mFilePath);
                                        onFailed();
                                        onFileTransferFinished(IOnSendFinishListener.Result.RESUMABLE);
                                    }
                                } catch (JoynServiceException e) {
                                    e.printStackTrace();
                                    Logger.i(TAG,
                                            "onTransferError(), MEDIA_RESUME_FAILED  add to active2");
                                    onFailed();
                                    onFileTransferFinished(IOnSendFinishListener.Result.RESUMABLE);
                                } catch (NullPointerException e) {
                                    e.printStackTrace();
                                    Logger.i(TAG,
                                            "onTransferError(), MEDIA_RESUME_FAILED  add to active3");
                                    onFailed();
                                    onFileTransferFinished(IOnSendFinishListener.Result.RESUMABLE);
                                }
                                ;
                        }
                      }
                    }
                };
                t.start();
                break;*/

        }

        private void sendFilebyMMS() {
            if(mParticipant == null){
                Logger.v(TAG, "sendFilebyMMS() entry, it is group chat");
                final SmsManager smsManager = SmsManager.getDefault();
                smsManager.getAutoPersisting();
                // MMS config is loaded asynchronously. Wait a bit so it will be loaded.
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                 // Ignore
                }
                final String fileName = mFileStruct.mName;
                final File sendFile = new File( mFileStruct.mFilePath);
                AsyncTask.execute(new Runnable() {
                    @Override
                    public void run() {
                        for(String participant : mParticipants) {
                            final byte[] pdu = buildPdu(RcsSettings.getInstance().getUserProfileImsUserName(), participant);
                            Uri writerUri = (new Uri.Builder())
                                    .authority("")
                                    .path(mFileStruct.mFilePath)
                                    .scheme(ContentResolver.SCHEME_FILE)
                                    .build();
                            Logger.v(TAG, "sendFilebyMMS(): Uri: " + writerUri.toString());
                            FileOutputStream writer = null;
                            Uri contentUri = null;
                            try {
                                writer = new FileOutputStream(sendFile);
                                writer.write(pdu);
                                contentUri = writerUri;
                                Logger.v(TAG, "sendFilebyMMS(): just wrote file");
                            } catch (final IOException e) {
                                Logger.v(TAG, "sendFilebyMMS(): FAILED: couldn't write file");
                            } finally {
                                if (writer != null) {
                                    try {
                                        writer.close();
                                    } catch (IOException e) {
                                    }
                                }
                            }
                            if (contentUri != null) {
                                smsManager.sendMultimediaMessage(MediatekFactory.getApplicationContext(), contentUri, null, null, null);
                                Logger.v(TAG, "sendMMS(): just sent");
                            } else {
                                Logger.v(TAG, "sendMMS(): FAILED: couldn't write file so didn't send");
                            }
                        }
                    }
                });
            }
        }

        private byte[] buildPdu(String fromNumber, String toNumber) {
            Logger.v(TAG, "buildPdu(): fromNumber: " + fromNumber + ",toNumber:" + toNumber);
            final SendReq req = new SendReq();
            // From, per spec
            req.setFrom(new EncodedStringValue(fromNumber));
            // To
            final String[] recipients = new String[1];
            recipients[0] = toNumber;
            final EncodedStringValue[] encodedNumbers = EncodedStringValue.encodeStrings(recipients);
            if (encodedNumbers != null) {
                req.setTo(encodedNumbers);
            }
            // Subject

            req.setSubject(new EncodedStringValue("MMS"));
            // Date
            req.setDate(System.currentTimeMillis() / 1000);
            // Body
            final PduBody body = new PduBody();
            // Add text part. Always add a smil part for compatibility, without it there
            // may be issues on some carriers/client apps
            //final int size = addImagePart(body, attach_file) + addTextPart(body, text_message, true);

            final int size = addMessagePart(body, true);
            req.setBody(body);
            // message size
            req.setMessageSize(size);

            req.setBody(body);
            // Message size
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

        private int addMessagePart(PduBody pb, boolean addTextSmil) {
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
            String mMessage = "MMS";
            PduPart part = new PduPart();
            part.setCharset(CharacterSets.UTF_8);
            part.setContentType(ContentType.TEXT_PLAIN.getBytes());
            part.setContentLocation(TEXT_PART_FILENAME.getBytes());
            int index = TEXT_PART_FILENAME.lastIndexOf(".");
            String contentId = (index == -1) ? TEXT_PART_FILENAME : TEXT_PART_FILENAME.substring(0, index);
            part.setContentId(contentId.getBytes());
            part.setData(mMessage.getBytes());
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
         * Callback called during the transfer progress.
         * .
         * @param currentSize
         *            Current transferred size in bytes.
         * @param totalSize
         *            Total size to transfer in bytes.
         */
        public void onTransferProgress(long currentSize, long totalSize) {
            // Because of can't received file transfered callback, so add
            // current size equals total size change status to finished
            Logger.v(TAG, "onTransferProgress() the file "
                    + mFileStruct.mFilePath + " with " + mParticipant
                    + " is transferring, currentSize is " + currentSize
                    + " total size is " + totalSize + ", mFileTransfer = "
                    + mFileTransfer);
            if (mFileTransfer != null) {
                if (currentSize < totalSize) {
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.
                                    IFileTransfer.Status.TRANSFERING);
                    mFileTransfer.setProgress(currentSize);
                    updateNotification(currentSize);
                } else {
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.
                                    IFileTransfer.Status.FINISHED);
                    // onFileTransferFinished(IOnSendFinishListener.Result.REMOVABLE);
                }
            }
        }

        /**
         * Callback called when the file has been transferred.
         * .
         * @param filename
         *            Filename including the path of the transferred file.
         */
        public void onFileTransferred(String filename) {
            Logger.v(TAG, "onFileTransferred() entry, fileName is "
                    + filename + ", mFileTransfer = " + mFileTransfer);
            if (mFileTransfer != null) {
                mFileTransfer
                        .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
                if(isResumeTransfer()){
                    try {
                        Logger.v(TAG, "onFileTransferred set resumable filename");
                        mFileTransfer.setFilePath(filename);
                    } catch (NullPointerException e) {
                        Logger.v(TAG, "resume onFileTransferred exception");
                    }
                }
            }
            onFileTransferFinished(IOnSendFinishListener.Result.REMOVABLE);
        }

        private NotificationManager getNotificationManager() {
            ApiManager apiManager = ApiManager.getInstance();
            if (null != apiManager) {
                Context context = apiManager.getContext();
                if (null != context) {
                    Object systemService = context
                            .getSystemService(Context.NOTIFICATION_SERVICE);
                    return (NotificationManager) systemService;
                } else {
                    Logger.e(TAG, "getNotificationManager() context is null");
                    return null;
                }
            } else {
                Logger.e(TAG, "getNotificationManager() apiManager is null");
                return null;
            }
        }

        private int getNotificationId() {
            if (null != mFileTransferTag) {
                return mFileTransferTag.hashCode();
            } else {
                Logger.w(TAG, "getNotificationId() mFileTransferTag: "
                        + mFileTransferTag);
                return 0;
            }
        }

        private void setNotification() {
            updateNotification(0);
        }

        private void updateNotification(long currentSize) {
            Logger.v(TAG, "updateNotification() entry, currentSize is "
                    + currentSize + ", mFileTransferTag is " + mFileTransferTag
                    + ", mFileStruct = " + mFileStruct);
            NotificationManager notificationManager = getNotificationManager();
            if (null != notificationManager) {
                if (null != mFileStruct) {
                    Context context = ApiManager.getInstance().getContext();
                    Notification.Builder builder = new Notification.Builder(
                            context);
                    builder.setProgress((int) mFileStruct.mSize,
                            (int) currentSize, currentSize < 0);
                    String title = null;
                    if (mParticipant != null) {
                        title = context.getResources().getString(
                                R.string.ft_progress_bar_title,
                                ContactsListManager.getInstance()
                                        .getDisplayNameByPhoneNumber(
                                                mParticipant.getContact()));
                    } else {
                        IChat1 chat = ModelImpl.getInstance().getChat(mChatTag);

                        if (chat instanceof GroupChat1) {
                            title = context.getResources().getString(
                                    R.string.ft_progress_bar_title,
                                    ((GroupChat1) chat).getGroupSubject());
                        }
                    }
                    builder.setContentTitle(title);
                    builder.setAutoCancel(false);
                    builder.setContentText(extractFileNameFromPath(mFileStruct.mFilePath));
                    builder.setContentInfo(buildPercentageLabel(context,
                            mFileStruct.mSize, currentSize));
                    builder.setSmallIcon(R.drawable.rcs_notify_file_transfer);
                    PendingIntent pendingIntent;
                    if (Logger.getIsIntegrationMode()) {
                        Intent intent = new Intent();
                        Uri uri;
                        intent.setAction(Intent.ACTION_SENDTO);
                        if (RcsSettings.getInstance().getMessagingUx() == 0) {
                            if (mParticipant != null) {
                                uri = Uri.parse(PluginProxyActivity.MMSTO
                                        + IpMessageConsts.JOYN_START
                                        + mParticipant.getContact());
                            } else {
                                IChat1 chat = ModelImpl.getInstance().getChat(
                                        mChatTag);

                                uri = Uri
                                        .parse(PluginProxyActivity.MMSTO
                                                + ((GroupChat1) chat)
                                                        .getGroupSubject());

                            }
                            intent.putExtra("chatmode",
                                    IpMessageConsts.ChatMode.JOYN);

                        } else {
                            if (mParticipant != null) {
                                uri = Uri.parse(PluginProxyActivity.MMSTO
                                        + mParticipant.getContact());
                            } else {
                                IChat1 chat = ModelImpl.getInstance().getChat(
                                        mChatTag);

                                uri = Uri
                                        .parse(PluginProxyActivity.MMSTO
                                                + ((GroupChat1) chat)
                                                        .getGroupSubject());

                            }
                        }

                        intent.setData(uri);
                        pendingIntent = PendingIntent.getActivity(context, 0,
                                intent, PendingIntent.FLAG_UPDATE_CURRENT);
                    } else {
                        Intent intent = new Intent(context,
                                ChatScreenActivity.class);
                        intent.putExtra(ChatScreenActivity.KEY_CHAT_TAG,
                                (ParcelUuid) mChatTag);
                        pendingIntent = PendingIntent.getActivity(context, 0,
                                intent, PendingIntent.FLAG_UPDATE_CURRENT);
                    }
                    builder.setContentIntent(pendingIntent);
                    notificationManager.notify(getNotificationId(),
                            builder.getNotification());
                }
            }
        }

        private void cancelNotification() {
            NotificationManager notificationManager = getNotificationManager();
            Logger.v(TAG, "cancelNotification() entry, mFileTransferTag is "
                    + mFileTransferTag + ",notificationManager = "
                    + notificationManager);
            if (null != notificationManager) {
                notificationManager.cancel(getNotificationId());
            }
        }

        private static String buildPercentageLabel(Context context,
                long totalBytes, long currentBytes) {
            if (totalBytes <= 0) {
                return null;
            } else {
                final int percent = (int) (100 * currentBytes / totalBytes);
                return context.getString(R.string.ft_percent, percent);
            }
        }

        protected IOnSendFinishListener mOnSendFinishListener = null;

        /**
         * Listener to handle File Transfer finish .
         *
         */
        protected interface IOnSendFinishListener {
            /**
             * Result of file transfer whether resendable or removable.
             *.
             */
            static enum Result {
                REMOVABLE, // This kind of result indicates that this File
                // transfer should be removed from the manager
                RESENDABLE,
                // This kind of result indicates that this File transfer will
                // have a chance to be resent in the future
                RESUMABLE
                // This kind of result indicates that this File transfer will
                // have a chance to be resumed in the future
            };

            /**
             * @param sentFileTransfer Sent file transfer object .
             * @param result Result of File transfer .
             */
            void onSendFinish(SentFileTransfer sentFileTransfer, Result result);
        }

        @Override
        public void onNetworkStatusChanged(boolean isConnected) {
            boolean hhtpFT = false;
            try {
                if (mFileTransferObject != null) {
                    hhtpFT = true;//mFileTransferObject.isHttpFileTransfer();
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            Logger.v(TAG, "onNetworkStatusChanged Model" + isConnected
                    + "Http: " + hhtpFT);

            if (!isConnected && !hhtpFT) {
                onFailed();
                onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
            }
        }
    }

    /**
     * .
     */
    public void clearFileTransferQueue() {
        Logger.w(TAG, "clearFileTransferQueue");
        mOutGoingFileTransferManager.clearFtList();
    }

    //private SentFileTransferManager mOutGoingFileTransferManager = new SentFileTransferManager();

    /**
     * This class is used to manage the whole sent file transfers and make it
     * work in queue.
     */
    static class SentFileTransferManager implements
            SentFileTransfer.IOnSendFinishListener {
        private static final String TAG = "M0CFF SentFileTransferManager";

        private static final int MAX_ACTIVATED_SENT_FILE_TRANSFER_NUM = 1;

        private ConcurrentLinkedQueue<SentFileTransfer> mPendingList = new ConcurrentLinkedQueue<SentFileTransfer>();

        private CopyOnWriteArrayList<SentFileTransfer> mActiveList = new CopyOnWriteArrayList<SentFileTransfer>();

        private CopyOnWriteArrayList<SentFileTransfer> mResendableList = new CopyOnWriteArrayList<SentFileTransfer>();

        public CopyOnWriteArrayList<ResumeFileTransfer> mResumableList = new CopyOnWriteArrayList<ResumeFileTransfer>();

        private synchronized void checkNext() {
            int activatedNum = mActiveList.size();
            if (activatedNum < MAX_ACTIVATED_SENT_FILE_TRANSFER_NUM) {
                Logger.w(TAG, "checkNext() current activatedNum is "
                        + activatedNum
                        + " will find next file transfer to send");
                final SentFileTransfer nextFileTransfer = mPendingList.poll();
                if (null != nextFileTransfer) {
                    Logger.w(TAG,
                            "checkNext() next file transfer found, just send it!");

                    AsyncTask.execute(new Runnable() {
                        @Override
                        public void run() {
                            nextFileTransfer.send();
                        }
                    });
                    mActiveList.add(nextFileTransfer);
                } else {
                    try {
                        if (RcsSettings.getInstance().isSupportOP08()) {
                            final ResumeFileTransfer resumableFileTransfer = mResumableList
                                    .get(0);
                            mResumableList.remove(resumableFileTransfer);
                            if (null != resumableFileTransfer) {
                                Logger.w(TAG,
                                        "checkNext() resumable file transfer found, just send it!");

                                AsyncTask.execute(new Runnable() {
                                    @Override
                                    public void run() {
                                        resumableFileTransfer
                                                .resumeMsrpFileTransfer();
                                    }
                                });
                                SentFileTransfer mSentFileTransfer = (SentFileTransfer) resumableFileTransfer;
                                mActiveList.add(mSentFileTransfer);
                            }
                        }
                    } catch (Exception e) {
                    Logger.w(TAG,
                                "checkNext() exception in resumable file transfer found,");
                        e.printStackTrace();
                    }
                    Logger.w(TAG,
                            "checkNext() next file transfer not found, resumable list is null");
                }
            } else {
                try {
                SentFileTransfer mSentFt = mActiveList.get(0);
                Logger.w(TAG,"checkNext() Activated file is:" + mSentFt);
                } catch(Exception e){
                    e.printStackTrace();
                }
                Logger.w(TAG, "checkNext() current activatedNum is "
                        + activatedNum
                        + " MAX_ACTIVATED_SENT_FILE_TRANSFER_NUM is "
                        + MAX_ACTIVATED_SENT_FILE_TRANSFER_NUM
                        + " so no need to find next pending file transfer");
            }
        }

        /**
         * .
         */
        public void clearFtList() {
            Logger.w(TAG,
                    "clearFtList mPendingList size: " + mPendingList.size()
                            + "resend size: " + mResendableList.size());
            if (RichMessagingDataProvider.getInstance() == null) {
                RichMessagingDataProvider.createInstance(MediatekFactory
                        .getApplicationContext());
            }
            RichMessagingDataProvider.getInstance().deleteFileTranfser(null);
            mPendingList.clear();
            mResendableList.clear();
        }

        /**
         * @param tag .
         */
        public void clearContactFtList(String tag) {
            Logger.w(
                    TAG,
                    "clearContactFtList mPendingList size: "
                            + mPendingList.size() + "resend size: "
                            + mResendableList.size());
            if (RichMessagingDataProvider.getInstance() == null) {
                RichMessagingDataProvider.createInstance(MediatekFactory
                        .getApplicationContext());
            }
            Logger.w(TAG, "clearContactFtList tag: " + tag);
            Iterator pendingIterator = mPendingList.iterator();
            while (pendingIterator.hasNext()) {
                SentFileTransfer pendingFile = (SentFileTransfer) pendingIterator
                        .next();
                if (pendingFile.mChatTag.toString().equals(tag)) {
                    mPendingList.remove(pendingFile);
                    RichMessagingDataProvider.getInstance().deleteFileTranfser(
                            pendingFile.mFileTransferTag.toString());
                }
            }
            Iterator resendableIterator = mResendableList.iterator();
            while (resendableIterator.hasNext()) {
                SentFileTransfer resendableFile = (SentFileTransfer) resendableIterator
                        .next();
                if (resendableFile.mChatTag.toString().equals(tag)) {
                    mResendableList.remove(resendableFile);
                    RichMessagingDataProvider.getInstance().deleteFileTranfser(
                            resendableFile.mFileTransferTag.toString());
                }
            }
        }

        /**
         * .
         */
        public void resumesFileSend() {
            Logger.w(TAG, "resumeFileSend 01 entry");
            SentFileTransfer resumeFile = null;
            try {
                if (!mActiveList.isEmpty()) {
                resumeFile = mActiveList.get(0);
                }
            } catch (NullPointerException e) {
                e.printStackTrace();
            } catch (ArrayIndexOutOfBoundsException e) {
                e.printStackTrace();
            }
            if (resumeFile != null) {
                Logger.v(TAG, "resumeFileSend not null");
                resumeFile.onResume();
            }
            Logger.v(TAG, "resumeFileSend 01 exit");
            resumesMsrpFileSend();
        }

        /**
         * .
         */
        public void resumesMsrpFileSend() {
            Logger.w(TAG, "resumesMsrpFileSend 01 entry");
            ResumeFileTransfer resumeFile = null;
            try {
                if (!mResumableList.isEmpty()) {
                    resumeFile = mResumableList.get(0);
                    mActiveList.add(resumeFile);
                }
            } catch (NullPointerException e) {
                e.printStackTrace();
                Logger.v(TAG, "resumesMsrpFileSend exception");
            } catch (ArrayIndexOutOfBoundsException e) {
                e.printStackTrace();
                Logger.v(TAG,
                        "resumesMsrpFileSend ArrayIndexOutOfBoundsException");

            }
            try {
                if (resumeFile != null) {
                    Logger.v(TAG, "resumesMsrpFileSend not null");
                    onAddSentResumeFileTransfer(resumeFile);
                    resumeFile.resumeMsrpFileTransfer();
                }
            } catch (Exception e) {
                e.printStackTrace();
                Logger.v(TAG, "resumesMsrpFileSend exceoption");
            }
            Logger.v(TAG, "resumesMsrpFileSend 01 exit");
        }

        /**
         * @param sentFileTransfer .
         */
        public void onAddSentFileTransfer(SentFileTransfer sentFileTransfer) {
            Logger.w(TAG, "onAddSentFileTransfer() entry, sentFileTransfer =  "
                    + sentFileTransfer);
            if (null != sentFileTransfer) {
                Logger.w(TAG, "onAddSentFileTransfer() entry, file "
                        + sentFileTransfer.mFileStruct + " is going to be sent");
                sentFileTransfer.mOnSendFinishListener = this;
                mPendingList.add(sentFileTransfer);
                checkNext();
            }
        }

        /**
         * @param sentFileTransfer .
         */
        public void onAddSentResumeFileTransfer(
                ResumeFileTransfer resumeFileTransfer) {
            Logger.w(TAG,
                    "onAddSentResumeFileTransfer() entry, sentFileTransfer =  "
                            + resumeFileTransfer);
            if (null != resumeFileTransfer) {
                Logger.w(TAG, "onAddSentFileTransfer() entry, file "
                        + resumeFileTransfer.mFileStruct
                        + " is going to be sent");
                if (resumeFileTransfer.mOnSendFinishListener == null) {
                    resumeFileTransfer.mOnSendFinishListener = this;
                }
                // mPendingList.add(sentFileTransfer);
                // checkNext();
            }
        }

        /**
         * @param tag .
         */
        public void onChatDestroy(Object tag) {
            Logger.v(TAG, "onChatDestroy entry, tag is " + tag);
            clearTransferWithTag(tag, One2OneChat.FILETRANSFER_ENABLE_OK);
        }

        /**
         * @param tag .
         * @param reason .
         */
        public void clearTransferWithTag(Object tag, int reason) {
            Logger.v(TAG, "onFileTransferNotAvalible() entry tag is " + tag
                    + " reason is " + reason);
            if (null != tag) {
                Logger.v(TAG, "onFileTransferNotAvalible() tag is " + tag);
                ArrayList<SentFileTransfer> toBeDeleted = new ArrayList<SentFileTransfer>();
                for (SentFileTransfer fileTransfer : mActiveList) {
                    if (tag.equals(fileTransfer.mChatTag)) {
                        Logger.v(TAG,
                                "onFileTransferNotAvalible() sent file transfer with chatTag "
                                        + tag + " found in activated list");
                        fileTransfer.onNotAvailable(reason);
                        fileTransfer.onDestroy();
                        toBeDeleted.add(fileTransfer);
                    }
                    try {
                        if (fileTransfer != null) {
                            RichMessagingDataProvider.getInstance()
                                    .deleteFileTranfser(
                                            fileTransfer.mFileTransferTag
                                                    .toString());
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                if (toBeDeleted.size() > 0) {
                    Logger.v(TAG,
                            "onFileTransferNotAvalible() need to remove some file transfer from"
                                    + " activated list");
                    mActiveList.removeAll(toBeDeleted);
                    if(reason != One2OneChat.FILETRANSFER_ENABLE_OK){
                        mResendableList.addAll(toBeDeleted);
                    }
                    toBeDeleted.clear();
                }
                for (SentFileTransfer fileTransfer : mPendingList) {
                    if (tag.equals(fileTransfer.mChatTag)) {
                        Logger.v(TAG,
                                "onFileTransferNotAvalible() sent file transfer with chatTag "
                                        + tag + " found in pending list");
                        fileTransfer.onNotAvailable(reason);
                        toBeDeleted.add(fileTransfer);
                    }
                    try {
                        if (fileTransfer != null) {
                            RichMessagingDataProvider.getInstance()
                                    .deleteFileTranfser(
                                            fileTransfer.mFileTransferTag
                                                    .toString());
                        }

                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                if (toBeDeleted.size() > 0) {
                    Logger.v(TAG,
                            "onFileTransferNotAvalible() need to remove some file transfer from"
                                    + " pending list");
                    mPendingList.removeAll(toBeDeleted);
                    if(reason != One2OneChat.FILETRANSFER_ENABLE_OK){
                        mResendableList.addAll(toBeDeleted);
                    }
                    toBeDeleted.clear();
                }
                for (SentFileTransfer fileTransfer : mResendableList) {
                    if (tag.equals(fileTransfer.mChatTag)) {
                        Logger.v(TAG,
                                "onFileTransferNotAvalible() sent file transfer with chatTag "
                                        + tag
                                        + " found in mResendableList list");
                        fileTransfer.onNotAvailable(reason);
                        toBeDeleted.add(fileTransfer);
                    }
                    try {
                        if (fileTransfer != null) {
                            RichMessagingDataProvider.getInstance()
                                    .deleteFileTranfser(
                                            fileTransfer.mFileTransferTag
                                                    .toString());
                        }

                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                if (toBeDeleted.size() > 0) {
                    Logger.v(TAG, "onFileTransferNotAvalible() "
                            + "need to remove some file transfer from"
                            + " mResendableList list");
                    //mResendableList.removeAll(toBeDeleted);
                    toBeDeleted.clear();
                }
            }
        }

        /**
         * @param targetFileTransferTag .
         */
        public void resendFileTransfer(Object targetFileTransferTag) {
            SentFileTransfer fileTransfer = findResendableFileTransfer(targetFileTransferTag);
            Logger.w(TAG, "resendFileTransfer() the file transfer with tag "
                    + targetFileTransferTag + " is " + fileTransfer);
            if (null != fileTransfer) {
                fileTransfer.onPrepareResend();
                Logger.v(
                        TAG,
                        "resendFileTransfer() the file transfer with tag "
                                + targetFileTransferTag
                                + " found, remove it from resendable list and add it into" +
                                 " pending list");
                mResendableList.remove(fileTransfer);
                mActiveList.add(fileTransfer);
                //checkNext();
                fileTransfer.resend();
            }
        }

        /**
         * @param targetFileTransferTag .
         */
        public void pauseFileTransfer(Object targetFileTransferTag) {
            SentFileTransfer fileTransfer = findActiveFileTransfer(targetFileTransferTag);
            Logger.w(TAG, "pauseFileTransfer() the file transfer with tag "
                    + targetFileTransferTag + " is " + fileTransfer);
            if (null != fileTransfer) {
                Logger.w(TAG, "pauseFileTransfer() the file transfer with tag "
                        + targetFileTransferTag + " found, ");
                fileTransfer.onPause();
            }
        }

        /**
         * @param targetFileTransferTag .
         */
        public void resumeFileTransfer(Object targetFileTransferTag) {
            SentFileTransfer fileTransfer = findActiveFileTransfer(targetFileTransferTag);
            Logger.w(TAG, "resumeFileTransfer() the file transfer with tag "
                    + targetFileTransferTag + " is " + fileTransfer);
            if (null != fileTransfer) {
                Logger.w(TAG,
                        "resumeFileTransfer() the file transfer with tag "
                                + targetFileTransferTag + " found");
                fileTransfer.onResume();
            }
        }

        /**
         * @param targetFileTransferTag .
         */
        public void cancelFileTransfer(Object targetFileTransferTag) {
            Logger.v(TAG,
                    "cancelFileTransfer() begin to cancel file transfer with tag "
                            + targetFileTransferTag);
            SentFileTransfer fileTransfer = findPendingFileTransfer(targetFileTransferTag);
            if (null != fileTransfer) {
                Logger.v(TAG,
                        "cancelFileTransfer() the target file transfer with tag "
                                + targetFileTransferTag
                                + " found in pending list");
                fileTransfer.onCancel();
                mPendingList.remove(fileTransfer);
            } else {
                fileTransfer = findActiveFileTransfer(targetFileTransferTag);
                Logger.w(TAG,
                        "cancelFileTransfer() the target file transfer with tag "
                                + targetFileTransferTag
                                + " found in active list is " + fileTransfer);
                if (null != fileTransfer) {
                    Logger.w(TAG,
                            "cancelFileTransfer() the target file transfer with tag "
                                    + targetFileTransferTag
                                    + " found in active list");
                    fileTransfer.onCancel();
                    fileTransfer.onDestroy();
                    onSendFinish(fileTransfer, Result.REMOVABLE);
                }
            }
        }

        @Override
        public void onSendFinish(final SentFileTransfer sentFileTransfer,
                final Result result) {
            Logger.w(TAG, "onSendFinish(): sentFileTransfer = "
                    + sentFileTransfer + ", result = " + result);
            if (mActiveList.contains(sentFileTransfer)) {
                sentFileTransfer.cancelNotification();
                Logger.w(TAG, "onSendFinish() file transfer "
                        + sentFileTransfer.mFileStruct + " with "
                        + sentFileTransfer.mParticipant + " finished with "
                        + result + " remove it from activated list");
                switch (result) {
                case RESENDABLE:
                    mResendableList.add(sentFileTransfer);
                    mActiveList.remove(sentFileTransfer);
                    break;
                case RESUMABLE:
                    ResumeFileTransfer resumeFileTransfer = null;
                    if (sentFileTransfer.mParticipants == null) {
                        resumeFileTransfer = new ResumeFileTransfer(
                                sentFileTransfer.mFileTransfer,
                                sentFileTransfer.mParticipant,
                                sentFileTransfer.mFileTransferTag,
                                sentFileTransfer.mFileStruct, false);
                    } else {
                        resumeFileTransfer = new ResumeFileTransfer(
                                sentFileTransfer.mFileTransfer,
                                sentFileTransfer.mParticipant,
                                sentFileTransfer.mFileTransferTag,
                                sentFileTransfer.mFileStruct, false);
                    }
                    mResumableList.add(resumeFileTransfer);
                    mActiveList.remove(sentFileTransfer);
                    break;
                case REMOVABLE:
                    mActiveList.remove(sentFileTransfer);
                    break;
                default:
                    break;
                }
                AsyncTask.execute(new Runnable() {
                    @Override
                    public void run() {
                        checkNext();
                    }
                });
            }
        }

        private SentFileTransfer findActiveFileTransfer(Object targetTag) {
            Logger.w(TAG, "findActiveFileTransfer entry, targetTag is "
                    + targetTag);
            return findFileTransferByTag(mActiveList, targetTag);
        }

        private SentFileTransfer findPendingFileTransfer(Object targetTag) {
            Logger.w(TAG, "findPendingFileTransfer entry, targetTag is "
                    + targetTag);
            return findFileTransferByTag(mPendingList, targetTag);
        }

        private SentFileTransfer findResendableFileTransfer(Object targetTag) {
            Logger.v(TAG, "findResendableFileTransfer entry, targetTag is "
                    + targetTag);
            return findFileTransferByTag(mResendableList, targetTag);
        }

        private SentFileTransfer findFileTransferByTag(
                Collection<SentFileTransfer> whereToFind, Object targetTag) {
            if (null != whereToFind && null != targetTag) {
                for (SentFileTransfer sentFileTransfer : whereToFind) {
                    Object fileTransferTag = sentFileTransfer.mFileTransferTag;
                    if (targetTag.equals(fileTransferTag)) {
                        Logger.w(TAG,
                                "findFileTransferByTag() the file transfer with targetTag "
                                        + targetTag + " found");
                        return sentFileTransfer;
                    }
                }
                Logger.w(TAG, "findFileTransferByTag() not found targetTag "
                        + targetTag);
                return null;
            } else {
                Logger.e(TAG, "findFileTransferByTag() whereToFind is "
                        + whereToFind + " targetTag is " + targetTag);
                return null;
            }
        }
    }

    @Override
    public void reloadMessages(String tag, List<Integer> messageIds) {
        Logger.w(TAG, "reloadMessages() messageIds: " + messageIds + " tag is "
                + tag);
        ContentResolver contentResolver = MediatekFactory
                .getApplicationContext().getContentResolver();
        if (tag != null) {
            reloadGroupMessage(tag, messageIds, contentResolver);
        } else {
            reloadOne2OneMessages(messageIds, contentResolver);
        }

    }

    /**
     * .
     */
    public void reloadNewMessages() {
        Logger.w(TAG, "reloadNewMessages() ");
        reloadNewO2OMessages();
        reloadNewGroupMessages();
    }

    private void reloadNewO2OMessages() {
        Logger.w(TAG, "reloadNewO2OMessages() ");
        if (RichMessagingDataProvider.getInstance() == null) {
            RichMessagingDataProvider.createInstance(MediatekFactory
                    .getApplicationContext());
        }
        List<Integer> messageIds = RichMessagingDataProvider.getInstance()
                .getAllO2OMessageID();
        Logger.w(TAG, "reloadNewO2OMessages() before msgid: " + messageIds);
        Cursor cursor = RichMessagingDataProvider.getInstance()
                .getAllO2OFtMessageID();
        try {
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    Integer messageId = -1;
                    messageId = cursor.getInt(0);
                    if (!messageIds.contains(messageId)) {
                        messageIds.add(messageId);
                    }
                    Logger.w(TAG, "reloadNewO2OMessages() cursor: " + messageId);
                }
            }
            Logger.w(TAG, "reloadNewO2OMessages() msgid: " + messageIds);
            ContentResolver contentResolver = MediatekFactory
                    .getApplicationContext().getContentResolver();
            reloadOne2OneMessages(messageIds, contentResolver);
        } catch (NullPointerException e) {
            e.printStackTrace();
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    private void reloadNewGroupMessages() {
        Logger.w(TAG, "reloadNewGroupMessages() ");
        if (RichMessagingDataProvider.getInstance() == null) {
            RichMessagingDataProvider.createInstance(MediatekFactory
                    .getApplicationContext());
        }
        Cursor cursor = RichMessagingDataProvider.getInstance()
                .getAllGroupChatMessageID();
        Cursor ftCursor = RichMessagingDataProvider.getInstance()
                .getAllGroupFtMessageID();
        // The map retains Object&IChat&List<Participant>
        Map<String, ArrayList<Integer>> mChatMa = new HashMap<String, ArrayList<Integer>>();
        Logger.w(TAG, "reloadNewGroupMessages() before outside cursor: ");
        Set<String> chatSet = new HashSet<String>();
        if (cursor != null || ftCursor != null) {
            Logger.w(TAG, "reloadNewGroupMessages() close is not null ");
            try {
                while (cursor.moveToNext()) {
                    String chatId = "";
                    chatId += cursor.getString(1);
                    Logger.w(TAG, "reloadNewGroupMessages() cursor: " + chatId);
                    chatSet.add(chatId);
                }
                while (ftCursor.moveToNext()) {
                    String chatId = "";
                    chatId += ftCursor.getString(1);
                    Logger.w(TAG, "reloadNewGroupMessages() cursor: " + chatId
                            + "MessageId: " + ftCursor.getInt(0));
                    if (!chatSet.contains(chatId)) {
                        chatSet.add(chatId);
                    }
                }
                Logger.w(TAG, "reloadNewGroupMessages() cursor0: chatSet: "
                        + chatSet);
                cursor.moveToFirst();
                ftCursor.moveToFirst();
                for (String s : chatSet) {
                    Logger.w(TAG, "reloadNewGroupMessages() cursor1: " + s);
                    ArrayList<SortedIds> msgIDList = new ArrayList<SortedIds>();

                    try {
                        if (cursor.getCount() > 0) {
                            do {
                                String chatId = cursor.getString(1);
                                if (chatId.equals(s)) {
                                    Integer msgId = cursor.getInt(0);
                                    Long timestamp = cursor.getLong(2);
                                            SortedIds newId = new SortedIds(msgId,
                                                    timestamp);
                                    Logger.w(TAG,
                                            "reloadNewGroupMessages() cursor2: matches"
                                                            + s + ": messageId: "
                                                            + msgId);
                                    msgIDList.add(newId);
                                }
                            } while (cursor.moveToNext());
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }

                    try {
                        if (ftCursor.getCount() > 0) {
                            do {
                                String chatId = ftCursor.getString(1);
                                if (chatId.equals(s)) {
                                    Integer msgId = ftCursor.getInt(0);
                                    Long timestamp = ftCursor.getLong(2);
                                            SortedIds newId = new SortedIds(msgId,
                                                    timestamp);
                                    newId.file = 1;
                                    Logger.w(TAG,
                                            "reloadNewGroupMessages() cursor3: matches"
                                                            + s + ": messageId: "
                                                            + msgId);
                                    msgIDList.add(newId);
                                }
                            } while (ftCursor.moveToNext());
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    ArrayList<SortedIds> newMsgIdList = sortTimestamp(msgIDList);
                    Logger.w(TAG, "reloadNewGroupMessages() newMsgIdList: "
                            + newMsgIdList);
                    List<Integer> msgIDList1 = new ArrayList<Integer>();
                    List<Integer> booleanList1 = new ArrayList<Integer>();
                    for (int i = 0; i < newMsgIdList.size(); i++) {
                        msgIDList1.add(newMsgIdList.get(i).msgId);
                        booleanList1.add(newMsgIdList.get(i).file);
                    }
                    Logger.w(TAG, "reloadNewGroupMessages() msgIDList1: "
                            + msgIDList1 + " s1:" + s);
                    ContentResolver contentResolver = MediatekFactory
                            .getApplicationContext().getContentResolver();
                    reloadNewSingleGroupMessage(s, msgIDList1, booleanList1,
                            contentResolver, 0);
                    cursor.moveToFirst();
                    ftCursor.moveToFirst();
                }
                Logger.w(TAG, "reloadNewGroupMessages() close outside cursor: ");
            } catch (NullPointerException e) {
                e.printStackTrace();
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
                if (ftCursor != null) {
                    ftCursor.close();
                }
            }
        }
    }

    /**
     * @param msgIDList .
     * @return .
     */
    public ArrayList<SortedIds> sortTimestamp(ArrayList<SortedIds> msgIDList) {
        ArrayList<SortedIds> newSortedList = new ArrayList<SortedIds>();
        int loopCount = msgIDList.size();
        SortedIds temp = null;
        for (int i = 0; i < msgIDList.size(); ) {
            temp = msgIDList.get(i);
            for (int j = 0; j < msgIDList.size(); j++) {
                if (temp.timestamp > msgIDList.get(j).timestamp) {
                    temp = msgIDList.get(j);
                }
            }
            Logger.w(TAG, "sortTimestamp: " + temp.timestamp);
            newSortedList.add(temp);
            msgIDList.remove(temp);
        }
        return newSortedList;
    }

    /**
     * Class to sort Ids by timestamp
     *.
     */
    public class SortedIds {

        /**
         * @param msgId2 .
         * @param timestamp2 .
         */
        public SortedIds(Integer msgId2, Long timestamp2) {
            // TODO Auto-generated constructor stub
            this.msgId = msgId2;
            this.timestamp = timestamp2;
            this.file = 0;
        }

        /**
         * Content message.
         */
        public Integer msgId = 0;

        /**
         * System message.
         */
        public Long timestamp = 1L;

        public Integer file = 0;

    }

    private void reloadNewGroupfILEMessages() {
        Logger.w(TAG, "reloadNewGroupfILEMessages() ");
        if (RichMessagingDataProvider.getInstance() == null) {
            RichMessagingDataProvider.createInstance(MediatekFactory
                    .getApplicationContext());
        }
        Cursor cursor = RichMessagingDataProvider.getInstance()
                .getAllGroupFtMessageID();
        // The map retains Object&IChat&List<Participant>
        Map<String, ArrayList<Integer>> mChatMa = new HashMap<String, ArrayList<Integer>>();
        Logger.w(TAG, "reloadNewGroupfILEMessages() before outside cursor: ");
        Set<String> chatSet = new HashSet<String>();
        if (cursor != null) {
            Logger.w(TAG, "reloadNewGroupfILEMessages() close is not null ");
            while (cursor.moveToNext()) {
                String chatId = "";
                chatId += cursor.getString(1);
                Logger.w(TAG, "reloadNewGroupfILEMessages() cursor: " + chatId);
                chatSet.add(chatId);
            }
            Logger.w(TAG, "reloadNewGroupfILEMessages() cursor0: ");
            cursor.moveToFirst();
            for (String s : chatSet) {
                Logger.w(TAG, "reloadNewGroupfILEMessages() cursor1: " + s);
                ArrayList<Integer> msgIDList = new ArrayList<Integer>();
                do {
                    String chatId = cursor.getString(1);
                    if (chatId.equals(s)) {
                        Integer msgId = cursor.getInt(0);
                        Logger.w(TAG,
                                "reloadNewGroupfILEMessages() cursor2: matches"
                                        + s + ": messageId: " + msgId);
                        msgIDList.add(msgId);
                    }
                } while (cursor.moveToNext());

                ContentResolver contentResolver = MediatekFactory
                        .getApplicationContext().getContentResolver();
                // reloadNewSingleGroupMessage(s,msgIDList,contentResolver,1);
                cursor.moveToFirst();
            }
            Logger.w(TAG, "reloadNewGroupfILEMessages() close outside cursor: ");
            cursor.close();
        }
        Logger.w(TAG, "reloadNewGroupfILEMessages() exit outside cursor: ");
    }

    private void reloadNewSingleGroupMessage(String tag,
            List<Integer> messageIds, List<Integer> booleanIds,
            ContentResolver contentResolver, int flag1) {
        Logger.w(TAG, "reloadNewSingleGroupMessage() entry: " + tag);
        // Collections.reverse(messageIds);
        int length = PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER
                .length();
        ParcelUuid parcelUuid = null;
        UUID uuid = UUID.randomUUID();
        parcelUuid = new ParcelUuid(uuid);
        int i = 0;
        int j = 0;

        HashSet<Participant> participantList = new HashSet<Participant>();
        ArrayList<ReloadMessageInfo> messageList = new ArrayList<ReloadMessageInfo>();
        TreeSet<String> sessionIdList = new TreeSet<String>();
        for (Integer messageId : messageIds) {
            // for(int i = 0;i<=1;i++){
            // if(flag1 == 1){
            // i = 1;
            // }

            Logger.w(TAG, "reloadNewSingleGroupMessage() entry: " + messageId
                    + "file: " + booleanIds.get(i));

            ReloadMessageInfo messageInfo = null;
            try {
                messageInfo = loadMessageFromId(sessionIdList,
                    messageId, contentResolver, booleanIds.get(i));
            } catch (Exception e1) {
                e1.printStackTrace();
            }
            i++;
            // ReloadMessageInfo messageInfo = null;
            if (null != messageInfo) {
                Object obj = messageInfo.getMessage();
                messageList.add(messageInfo);
                if (obj instanceof ChatMessage) {
                    ChatMessage message = (ChatMessage) obj;
                    String contact = message.getRemoteContact().toString();
                    contact = Utils.extractNumberFromUri(contact);
                    boolean isRcsValidNumber = false;
                    try {
                        isRcsValidNumber = ApiManager.getInstance()
                                .getContactsApi().isRcsValidNumber(contact);
                    } catch (Exception e) {
                        e.printStackTrace();
                        isRcsValidNumber = false;
                    }
                    if (!isRcsValidNumber) {
                        Logger.w(TAG,
                                "reloadNewSingleGroupMessage() the contact is not valid user "
                                        + contact);
                        continue;
                    }
                    Logger.w(TAG,
                            "reloadNewSingleGroupMessage() the contact is "
                                    + contact);
                    if (!TextUtils.isEmpty(contact)) {
                        Participant participant = new Participant(contact,
                                contact);
                        participantList.add(participant);
                    }
                }
            }
            // }
        }
        if (RichMessagingDataProvider.getInstance() == null) {
            RichMessagingDataProvider.createInstance(MediatekFactory
                    .getApplicationContext());
        }
        List<String> connectedParticapnts = RichMessagingDataProvider.getInstance()
                .getGroupChatConnectedParticipants(tag);
        Logger.w(TAG, "reloadNewSingleGroupMessage() connectedParticapnts: "
                + connectedParticapnts);
        for (String participant : connectedParticapnts) {
            boolean isRcsValidNumber = false;
            try {
                isRcsValidNumber = ApiManager.getInstance()
                        .getContactsApi().isRcsValidNumber(participant);
            } catch (Exception e) {
                e.printStackTrace();
                isRcsValidNumber = false;
            }
            if (!isRcsValidNumber) {
                Logger.w(TAG,
                        "reloadNewSingleGroupMessage() the participant contact is not valid user "
                                + participant);
                continue;
            }
            if (!TextUtils.isEmpty(participant)) {
                Participant newParticipant = new Participant(participant,
                        participant);
                participantList.add(newParticipant);
            }
        }
        Logger.w(TAG, "reloadNewSingleGroupMessage() the sessionIdList is "
                + sessionIdList);
        //fillParticipantList(sessionIdList, participantList, contentResolver);
        Logger.w(TAG, "reloadNewSingleGroupMessage() participantList is "
                + participantList);
        if (participantList.size() < ChatFragment.GROUP_MIN_MEMBER_NUM) {
            Logger.w(TAG, "reloadNewSingleGroupMessage() not group");
            return;
        }
        IChat1 chat = mChatMap.get(parcelUuid);
        if (chat != null) {
            Logger.w(TAG,
                    "reloadNewSingleGroupMessage() the chat already exist chat is "
                            + chat);
        } else {
            Logger.w(TAG,
                    "reloadNewSingleGroupMessage() the chat does not exist ");
            chat = new GroupChat1(this, null, new ArrayList<Participant>(
                    participantList), parcelUuid);
            mChatMap.put(parcelUuid, chat);
            IGroupChatWindow chatWindow = ViewImpl.getInstance()
                    .addGroupChatWindow(parcelUuid,
                            ((GroupChat1) chat).getParticipantInfos());
            ((GroupChat1) chat).setChatWindow(chatWindow);
        }
        for (ReloadMessageInfo messageInfo : messageList) {
            if (null != messageInfo) {
                Object obj = messageInfo.getMessage();
                if (obj instanceof ChatMessage) {
                    ChatMessage message = (ChatMessage) obj;
                    int messageType = messageInfo.getMessageType();
                   ((ChatImpl) chat).reloadMessage(message, messageType, messageInfo.getMessageStatus(),-1,messageInfo.getChatId());
                } else if (obj instanceof FileStruct) {
                    FileStruct fileStruct = (FileStruct) obj;
                    int messageType = messageInfo.getMessageType();
                    String contact = fileStruct.mRemote;
                    Logger.w(TAG, "reloadGroupMessage() file : contact = "
                            + contact + "filesize" + fileStruct.mSize
                            + "Thumb:" + fileStruct.mThumbnail);
                    Logger.w(TAG, "reloadGroupMessage() file : name = "
                            + fileStruct.mName);
                    ((ChatImpl) chat).reloadFileTransfer(fileStruct,
                            messageType, messageInfo.getMessageStatus());
                    // ((ChatImpl) chat).reloadMessage(message, messageType,
                    // -1,-1,messageInfo.getChatId());
                }
            }
        }
    }

    private void reloadGroupMessage(String tag, List<Integer> messageIds,
            ContentResolver contentResolver) {
        Logger.w(TAG, "reloadGroupMessage() entry");
        Collections.reverse(messageIds);
        int flag = 0;
        int length = PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER
                .length();
        String mChatId = null;
        ParcelUuid parcelUuid = null;
        if (tag != null) {
            String realTag = tag;
            if (tag.contains(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
                realTag = tag.substring(length);
            }
            parcelUuid = ParcelUuid.fromString(realTag);
        } else {
            UUID uuid = UUID.randomUUID();
            parcelUuid = new ParcelUuid(uuid);
        }

        HashSet<Participant> participantList = new HashSet<Participant>();
        ArrayList<ReloadMessageInfo> messageList = new ArrayList<ReloadMessageInfo>();
        TreeSet<String> sessionIdList = new TreeSet<String>();
        for (Integer messageId : messageIds) {
            for (int i = 0; i <= 1; i++) {
                ReloadMessageInfo messageInfo = null;
                try {
                    messageInfo = loadMessageFromId(
                        sessionIdList, messageId, contentResolver, i);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                if(messageInfo == null){
                    if(i==0) {
                        messageInfo = loadMessageFromLocal(
                                sessionIdList, messageId, contentResolver);
                    }
                }
                // ReloadMessageInfo messageInfo = null;
                if (null != messageInfo) {
                    if (flag == 0) {
                        flag = 1;
                        mChatId = messageInfo.getChatId();
                        Logger.w(TAG,
                                "reloadGroupMessage() the participant chat id is: "
                                        + mChatId);
                        if (RichMessagingDataProvider.getInstance() == null) {
                            RichMessagingDataProvider
                                    .createInstance(MediatekFactory
                                            .getApplicationContext());
                        }
                        List<String> connectedParticapnts = RichMessagingDataProvider
                                .getInstance()
                                .getGroupChatConnectedParticipants(
                                        messageInfo.getChatId());
                        Logger.w(TAG,
                                "reloadGroupMessage() the participant contact size is: "
                                        + connectedParticapnts.size());

                        for (String participant : connectedParticapnts) {
                            Logger.w(TAG,
                                    "reloadGroupMessage() the participant contact "
                                            + participant);
                            if (!TextUtils.isEmpty(participant)) {
                                Logger.v(TAG,
                                        "reloadGroupMessage() add to list");
                                Participant newParticipant = new Participant(
                                        participant, participant);
                                participantList.add(newParticipant);
                            }
                        }
                    }
                }

                if (null != messageInfo) {
                    Object obj = messageInfo.getMessage();
                    messageList.add(messageInfo);
                    if (obj instanceof ChatMessage) {
                        ChatMessage message = (ChatMessage) obj;
                        String contact = message.getRemoteContact().toString();
                        contact = Utils.extractNumberFromUri(contact);
                        Logger.w(TAG, "reloadGroupMessage() the contact is "
                                + contact);
                        /*if (!TextUtils.isEmpty(contact)) {
                            Participant participant = new Participant(contact,
                                    contact);
                            participantList.add(participant);
                        }*/
                    }
                }
            }
        }

        Logger.w(TAG, "reloadGroupMessage() the sessionIdList is "
                + sessionIdList + " ; ChatId is: " + mChatId);
        //fillParticipantList(sessionIdList, participantList, contentResolver);
        Logger.w(TAG, "reloadGroupMessage() participantList is "
                + participantList);
        if (participantList.size() < ChatFragment.GROUP_MIN_MEMBER_NUM) {
            Logger.w(TAG, "reloadGroupMessage() not group");
            return;
        }
        IChat1 chat = mChatMap.get(parcelUuid);
        if (chat != null) {
            Logger.w(TAG,
                    "reloadGroupMessage() the chat already exist chat is "
                            + chat);
        } else {
            Logger.w(TAG, "reloadGroupMessage() the chat does not exist ");
            chat = new GroupChat1(this, null, new ArrayList<Participant>(
                    participantList), parcelUuid);
            mChatMap.put(parcelUuid, chat);
            IGroupChatWindow chatWindow = ViewImpl.getInstance()
                    .addGroupChatWindow(parcelUuid,
                            ((GroupChat1) chat).getParticipantInfos());
            ((GroupChat1) chat).setChatWindow(chatWindow);
        }

        IChat1 chatImpl = mChatImplMap.get(mChatId);
        if (chatImpl != null) {
            Logger.w(TAG,
                    "reloadGroupMessage() the chatImpl already exist chat is "
                            + chatImpl);
        } else {
            Logger.w(TAG, "reloadGroupMessage() the chatImpl does not exist ");
            mChatImplMap.put(mChatId, chat);
        }
        ((GroupChat1) chat).setmChatId(mChatId);
        ((GroupChat1) chat).reloadStackApi(mChatId);
        ((GroupChat1) chat).setAfterReboot(true);

        for (ReloadMessageInfo messageInfo : messageList) {
            if (null != messageInfo) {
                Object obj = messageInfo.getMessage();
                if (obj instanceof ChatMessage) {
                    ChatMessage message = (ChatMessage) obj;
                    int messageType = messageInfo.getMessageType();
                     ((ChatImpl) chat).reloadMessage(message, messageType, messageInfo.getMessageStatus(),-1,messageInfo.getChatId());
                } else if (obj instanceof FileStruct) {
                    FileStruct fileStruct = (FileStruct) obj;
                    int messageType = messageInfo.getMessageType();
                    String contact = fileStruct.mRemote;
                    Logger.w(TAG, "reloadGroupMessage() file : contact = "
                            + contact + "filesize" + fileStruct.mSize
                            + "Thumb:" + fileStruct.mThumbnail);
                    Logger.w(TAG, "reloadGroupMessage() file : name = "
                            + fileStruct.mName);
                    ((ChatImpl) chat).reloadFileTransfer(fileStruct,
                            messageType, messageInfo.getMessageStatus());
                    // ((ChatImpl) chat).reloadMessage(message, messageType,
                    // -1,-1,messageInfo.getChatId());
                }
            }
        }
    }

    private void fillParticipantList(TreeSet<String> sessionIdList,
            HashSet<Participant> participantList,
            ContentResolver contentResolver) {
        Logger.w(TAG, "fillParticipantList() entry the  sessionIdList is "
                + sessionIdList + " participantList is " + participantList);
        for (String sessionId : sessionIdList) {
            Cursor cursor = null;
            String[] selectionArg = { sessionId,
            // Integer.toString(EventsLogApi.TYPE_GROUP_CHAT_SYSTEM_MESSAGE)
            // //TODo check this
            };
            try {
                cursor = contentResolver.query(ChatLog.Message.CONTENT_URI,
                        null, ChatLog.Message.CHAT_ID + "=? AND "
                                + ChatLog.Message.MESSAGE_TYPE + "=?",
                        selectionArg, null);
                if (cursor != null && cursor.moveToFirst()) {
                    do {
                        String remote = cursor
                                .getString(cursor
                                        .getColumnIndex(ChatLog.Message.CONTACT));
                        Logger.v(TAG, "fillParticipantList() the remote is "
                                + remote);
                        if (remote.length() - INDEXT_ONE <= INDEXT_ONE) {
                            Logger.e(TAG,
                                    "fillParticipantList() the remote is no content");
                            continue;
                        }
                        if (remote.contains(COMMA)) {
                            Logger.v(TAG,
                                    "fillParticipantList() the remote has COMMA ");
                            String subString = remote.substring(INDEXT_ONE,
                                    remote.length() - INDEXT_ONE);
                            Logger.v(TAG,
                                    "fillParticipantList() the remote is "
                                            + subString);
                            String[] contacts = subString.split(COMMA);
                            for (String contact : contacts) {
                                Logger.v(TAG,
                                        "fillParticipantList() arraylist the contact is "
                                                + contact);
                                Participant participant = new Participant(
                                        contact, contact);
                                participantList.add(participant);
                            }
                        } else if (remote.contains(SEMICOLON)) {
                            Logger.v(TAG,
                                    "fillParticipantList() the remote has SEMICOLON ");
                            String[] contacts = remote.split(SEMICOLON);
                            for (String contact : contacts) {
                                Logger.v(TAG,
                                        "fillParticipantList() arraylist the contact is "
                                                + contact);
                                Participant participant = new Participant(
                                        contact, contact);
                                participantList.add(participant);
                            }
                        } else {
                            Logger.v(TAG,
                                    "fillParticipantList() remote is single ");
                            Participant participant = new Participant(remote,
                                    remote);
                            participantList.add(participant);
                        }
                    } while (cursor.moveToNext());
                } else {
                    Logger.e(TAG, "fillParticipantList() the cursor is null");
                }
            } finally {
                if (null != cursor) {
                    cursor.close();
                }
            }
        }
    }

    private void reloadOne2OneMessages(List<Integer> messageIds,
            ContentResolver contentResolver) {
        Logger.w(TAG, "reloadOne2OneMessages() entry");
        TreeSet<String> sessionIdList = new TreeSet<String>();
        for (Integer messageId : messageIds) {
            for (int i = 0; i <= 1; i++) {
                Logger.w(TAG, "reloadOne2OneMessages() kkkk messageId: "
                        + messageId);
                ReloadMessageInfo info = null;
                try {
                    info = loadMessageFromId(sessionIdList,
                        messageId, contentResolver, i);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                // ReloadMessageInfo info = null;
                if(info == null){
                    if(i==0) {
                        info = loadO2OMessageFromLocal(
                                sessionIdList, messageId, contentResolver);
                    }
                }
                if (null != info) {
                    Object obj = info.getMessage();
                    int messageType = info.getMessageType();
                    if (obj instanceof ChatMessage) {
                        ChatMessage message = (ChatMessage) obj;
                        int messageStatus = info.getMessageStatus();
                        String contact = message.getRemoteContact().toString();
                        Logger.w(TAG, "reloadOne2OneMessages() : contact = "
                                + contact);
                        if (null != contact) {
                            if (contact.contains("sip:conference")) {
                                Logger.v(TAG, "contact is conference , ie NULL");
                                continue;
                            }
                            contact = Utils.extractNumberFromUri(contact);
                        }
                        Logger.w(TAG, "reloadOne2OneMessages() : contact = "
                                + contact);
                        if (!TextUtils.isEmpty(contact)) {
                            ArrayList<Participant> participantList = new ArrayList<Participant>();
                            participantList.add(new Participant(contact,
                                    contact));
                            IChat1 chat = addChat(participantList);
                            ((ChatImpl) chat).reloadMessage(message,
                                    messageType, messageStatus,
                                    info.getMessageTag(), info.getChatId());
                        }
                    } else if (obj instanceof FileStruct) {
                        FileStruct fileStruct = (FileStruct) obj;
                        String contact = fileStruct.mRemote;
                        Logger.w(TAG,
                                "reloadOne2OneMessages() file : contact = "
                                        + contact + " message status:"
                                        + info.getMessageStatus() + " filesize"
                                        + fileStruct.mSize + " Thumb:"
                                        + fileStruct.mThumbnail);
                        Logger.w(TAG, "reloadOne2OneMessages() file : name = "
                                + fileStruct.mName);
                        if (!TextUtils.isEmpty(contact)) {
                            ArrayList<Participant> participantList = new ArrayList<Participant>();
                            participantList.add(new Participant(contact,
                                    contact));
                            IChat1 chat = addChat(participantList);
                            ((ChatImpl) chat).reloadFileTransfer(fileStruct,
                                    messageType, info.getMessageStatus());
                        }
                    }
                }
            }
        }
    }

    private void reloadGroupMessages(List<Integer> messageIds,
            ContentResolver contentResolver, Object tag, IChat1 chat) {
        Logger.w(TAG, "reloadGroupMessages() entry");
        TreeSet<String> sessionIdList = new TreeSet<String>();
        String chatId;
        for (Integer messageId : messageIds) {
            for (int i = 0; i <= 1; i++) {
                ReloadMessageInfo info = null;
                try {
                    info = loadMessageFromId(sessionIdList,
                        messageId, contentResolver, i);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                // ReloadMessageInfo info = null;
                if (null != info) {
                    Object obj = info.getMessage();
                    int messageType = info.getMessageType();
                    if (obj instanceof ChatMessage) {
                        ChatMessage message = (ChatMessage) obj;
                        int messageStatus = info.getMessageStatus();
                        chatId = sessionIdList.first().toString();
                        ((GroupChat1) chat).setmChatId(chatId);
                        ((ChatImpl) chat).reloadMessage(message, messageType,
                                messageStatus, -1, info.getChatId());
                        // }
                    } else if (obj instanceof FileStruct) {
                        FileStruct fileStruct = (FileStruct) obj;
                        String contact = fileStruct.mRemote;
                        Logger.w(TAG, "reloadOne2OneMessages() : contact = "
                                + contact);
                        if (!TextUtils.isEmpty(contact)) {
                            ArrayList<Participant> participantList = new ArrayList<Participant>();
                            participantList.add(new Participant(contact,
                                    contact));
                            ((ChatImpl) chat).reloadFileTransfer(fileStruct,
                                    messageType, info.getMessageStatus());
                        }
                    }
                }
            }
        }

        if (sessionIdList.size() == 1) {
            Logger.w(TAG, "reloadGroupMessages() : stack size is 1");
            ((GroupChat1) chat)
                    .reloadStackApi(sessionIdList.first().toString());

        }
    }

    private ReloadMessageInfo loadMessageFromLocal(TreeSet<String> sessionIdList,
            Integer msgId, ContentResolver contentResolver) {
        Logger.w(TAG, "loadMessageFromLocal() msgId: " + msgId );
        Cursor cursor = null;
        int emptyCursor = 0;
        String[] selectionArg = { msgId.toString()};
        Logger.w(TAG, "loadMessageFromLocal() MsgTag: " + msgId);
        try {
            cursor = contentResolver.query(UnregGroupMessageProvider.CONTENT_URI,
                    null, UnregGroupMessageProvider.KEY_MESSAGE_TAG + "=?",
                    selectionArg, null);
            if (cursor.moveToFirst()) {
                emptyCursor = 1;
                int messageDirection = 1;
                int messageType = 0;
                String messageId = cursor.getString(cursor
                        .getColumnIndex(UnregGroupMessageProvider.KEY_MESSAGE_ID));
                messageId = "-1";
                String remote = "DEFAULT_REMOTE";
                String text = cursor.getString(cursor
                        .getColumnIndex(UnregGroupMessageProvider.KEY_MESSAGE));
                int messagetag = cursor.getInt(cursor
                        .getColumnIndex(UnregGroupMessageProvider.KEY_MESSAGE_TAG));
                String chatId = cursor.getString(cursor
                        .getColumnIndex(UnregGroupMessageProvider.KEY_CHAT_ID));
                int messageStatus = 5;
                sessionIdList.add(chatId);
                Date date = new Date();
                long timeStamp = cursor.getLong(cursor
                        .getColumnIndex(UnregGroupMessageProvider.KEY_TIMESTAMP));
                date.setTime(timeStamp);

                ChatMessage message = new ChatMessage(messageId,
                        remote, text, date, false, null);
                /*ChatMessage message = new ChatMessage(messageId, remote,
                        text, null, remote, RcsService.Direction.OUTGOING.toInt(), timeStamp, timeStamp, 0L, 0L,
                        ChatLog.Message.Content.Status.SENDING.toInt() , ChatLog.Message.Content.ReasonCode.UNSPECIFIED.toInt(), chatId, true, false);*/
                Logger.w(TAG, "loadMessageFronId() messageId: "
                        + messageId + " , remote: " + remote
                        + " , text: " + text + " , timeStamp: "
                        + timeStamp + " , messageType: "
                        + messageDirection + " , messageStatus: "
                        + messageStatus);
                ReloadMessageInfo messageInfo = new ReloadMessageInfo(
                        message, messageDirection, messageStatus,
                        chatId);
                messageInfo.setMessageTag(messagetag);
                return messageInfo;
            } else {
                Logger.i(TAG, "loadMessageFromLocal() empty message cursor");
                return null;
            }
        } catch(Exception e){
            e.printStackTrace();
            Logger.i(TAG, "loadMessageFromLocal(), exception");
        }
        finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        Logger.i(TAG, "loadMessageFromLocal()return null");
        return null;
    }

    private ReloadMessageInfo loadO2OMessageFromLocal(TreeSet<String> sessionIdList,
            Integer msgId, ContentResolver contentResolver) {
        Logger.w(TAG, "loadO2OMessageFromLocal() msgId: " + msgId );
        Cursor cursor = null;
        int emptyCursor = 0;
        String[] selectionArg = { msgId.toString()};
        Logger.w(TAG, "loadO2OMessageFromLocal() MsgTag: " + msgId);
        try {
            cursor = contentResolver.query(UnregMessageProvider.CONTENT_URI,
                    null, UnregMessageProvider.KEY_MESSAGE_TAG + "=?",
                    selectionArg, null);
            if (cursor.moveToFirst()) {
                emptyCursor = 1;
                int messageDirection = 1;
                int messageType = 0;
                String messageId  = "-1";
                String text = cursor.getString(cursor
                        .getColumnIndex(UnregMessageProvider.KEY_MESSAGE));
                int messagetag = cursor.getInt(cursor
                        .getColumnIndex(UnregMessageProvider.KEY_MESSAGE_TAG));
                String chatId = cursor.getString(cursor
                        .getColumnIndex(UnregMessageProvider.KEY_CHAT_ID));
                String remote = chatId;
                int messageStatus = 5;
                sessionIdList.add(chatId);
                Date date = new Date();
                long timeStamp = cursor.getLong(cursor
                        .getColumnIndex(UnregMessageProvider.KEY_TIMESTAMP));
                date.setTime(timeStamp);
               // messageId = messagetag;

                ChatMessage message = new ChatMessage(messageId,
                        remote, text, date, false, null);
                /*ChatMessage message = new ChatMessage(messageId, remote,
                        text, null, remote, RcsService.Direction.OUTGOING.toInt(), timeStamp, timeStamp, 0L, 0L,
                        ChatLog.Message.Content.Status.SENDING.toInt() , ChatLog.Message.Content.ReasonCode.UNSPECIFIED.toInt(), chatId, true, false);*/
                Logger.w(TAG, "loadO2OMessageFromLocal() messageId: "
                        + messageId + " , remote: " + remote
                        + " , text: " + text + " , timeStamp: "
                        + timeStamp + " , messageType: "
                        + messageDirection + " , messageStatus: "
                        + messageStatus);
                ReloadMessageInfo messageInfo = new ReloadMessageInfo(
                        message, messageDirection, messageStatus,
                        chatId);
                messageInfo.setMessageTag(messagetag);
                return messageInfo;
            } else {
                Logger.i(TAG, "loadMessageFromLocal() empty message cursor");
                return null;
            }
        } catch(Exception e){
            e.printStackTrace();
            Logger.i(TAG, "loadMessageFromLocal(), exception");
        }
        finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        Logger.i(TAG, "loadMessageFromLocal()return null");
        return null;
    }

    private ReloadMessageInfo loadMessageFromId(TreeSet<String> sessionIdList,
            Integer msgId, ContentResolver contentResolver, int flag) {
        Logger.w(TAG, "loadMessageFronId() msgId: " + msgId + " flag: " + flag);
        Cursor cursor = null;
        int emptyCursor = 0;
        String[] selectionArg = { msgId.toString(), "2" };
        if (flag == 0) {
            Logger.w(TAG, "loadMessageFronId() wwww");
            try {
                cursor = contentResolver.query(ChatLog.Message.CONTENT_URI,
                        null, ChatLog.Message.ID + "=?" + " AND  "
                                + ChatLog.Message.MESSAGE_TYPE + "<> ?",
                        selectionArg, null);
                if (cursor.moveToFirst()) {
                    emptyCursor = 1;
                    int messageDirection = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.DIRECTION));
                    int messageType = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.MESSAGE_TYPE));
                    String messageId = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.MESSAGE_ID));
                    String remote = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.CONTACT));
                    byte[] bodyData = cursor.getBlob(cursor
                            .getColumnIndex(ChatLog.Message.CONTENT));
                    // byte[] bodyData = blobtext.getBytes(1, (int)
                    // blobtext.length());
                    String text = null;
                    if (bodyData != null) {
                        text = new String(bodyData);
                    }
                    int messagetag = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.ID));
                    String chatId = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.CHAT_ID));
                    int messageStatus = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.STATUS));
                    sessionIdList.add(chatId);
                    Date date = new Date();
                    long timeStamp = cursor.getLong(cursor
                            .getColumnIndex(ChatLog.Message.TIMESTAMP));
                    date.setTime(timeStamp);

                    /*
                     * if (ChatLog.Message.Direction.INCOMING == messageType ||
                     * EventsLogApi.TYPE_OUTGOING_CHAT_MESSAGE == messageType ||
                     * EventsLogApi.TYPE_INCOMING_GROUP_CHAT_MESSAGE ==
                     * messageType ||
                     * EventsLogApi.TYPE_OUTGOING_GROUP_CHAT_MESSAGE ==
                     * messageType ||
                     * EventsLogApi.TYPE_GROUP_CHAT_SYSTEM_MESSAGE ==
                     * messageType) {
                     */
                    if (!(ChatLog.Message.Type.SYSTEM == messageType && remote
                            .equals(chatId))) { // TODo check this
                        ChatMessage message = new ChatMessage(messageId,
                                remote, text, date, false, null);
                        /*ChatMessage message = new ChatMessage(messageId, remote,
                                text, null, remote, RcsService.Direction.OUTGOING.toInt(), timeStamp, timeStamp, 0L, 0L,
                                ChatLog.Message.Content.Status.SENDING.toInt() , ChatLog.Message.Content.ReasonCode.UNSPECIFIED.toInt(), chatId, true, false);*/
                        Logger.w(TAG, "loadMessageFronId() messageId: "
                                + messageId + " , remote: " + remote
                                + " , text: " + text + " , timeStamp: "
                                + timeStamp + " , messageType: "
                                + messageDirection + " , messageStatus: "
                                + messageStatus);
                        ReloadMessageInfo messageInfo = new ReloadMessageInfo(
                                message, messageDirection, messageStatus,
                                chatId);
                        messageInfo.setMessageTag(messagetag);
                        return messageInfo;
                    } else {
                        return null;
                    }
                } else {
                    Logger.w(TAG, "loadMessageFronId() empty message cursor");
                    return null;
                }
            } finally {
                if (null != cursor) {
                    cursor.close();
                }
            }
        } else {
            Logger.w(TAG, "loadMessageFronId() wwww1");
            try {
                String[] ftSelectionArg = { msgId.toString() };
                cursor = contentResolver.query(FileTransferLog.CONTENT_URI,
                        null, FileTransferLog.BASECOLUMN_ID + "=?", ftSelectionArg, null);
                if (cursor.moveToFirst()) {
                    emptyCursor = 1;
                    int messageDirection = cursor.getInt(cursor
                            .getColumnIndex(FileTransferLog.DIRECTION));

                    if (messageDirection == 0 || messageDirection == 1) { // TODo
                                                                          // check
                                                                          // this
                        String fileName = cursor.getString(cursor
                                .getColumnIndex(FileTransferLog.FILENAME));
                        // String fileThumb =
                        // cursor.getString(cursor.getColumnIndex(FileTransferLog.FILEICON));
                        String fileThumb = null;
                        long fileSize = cursor.getLong(cursor
                                .getColumnIndex(FileTransferLog.FILESIZE));
                        String remote = cursor
                                .getString(cursor
                                        .getColumnIndex(FileTransferLog.CONTACT));
                        int messageStatus = cursor.getInt(cursor
                                .getColumnIndex(FileTransferLog.STATE));
                        String messagetag = cursor.getString(cursor
                                .getColumnIndex(FileTransferLog.FT_ID));

                        Date date = new Date();
                        long timeStamp = cursor.getLong(cursor
                                .getColumnIndex(ChatLog.Message.TIMESTAMP));
                        date.setTime(timeStamp);
                        File filePath = new File(fileName);
                        String fName = filePath.getName();
                        int reload = 0;
                        if ((messageStatus == FileTransfer.State.TRANSFERRED.toInt())
                            || (messageStatus == FileTransfer.State.DISPLAYED.toInt())
                            || (messageStatus == FileTransfer.State.DELIVERED.toInt())) {
                            reload = 1;
                        } else {
                            Logger.v(TAG, "add mReceivedAfterReloadFt Id: " + messagetag);
                            mReceivedAfterReloadFt.add(messagetag);
                        }
                        Logger.w(TAG, "loadMessageFronId() file messageId: " + messagetag
                                + " , messageStatus= " + messageStatus
                                + " , remote: " + remote + "\n filsize:" + fileSize
                                + " Thumb:" + fileThumb + " fName=" + fName + " reload: " + reload);
                        FileStruct fileStruct = new FileStruct(
                                fileName, fName, fileSize, messagetag,
                                date, remote , fileThumb, reload);
                        ReloadMessageInfo fileInfo = new ReloadMessageInfo(
                                fileStruct, messageDirection, messageStatus);
                        fileInfo.setMessageTag(0);
                        return fileInfo;
                    } else {
                        return null;
                    }
                } else {
                    if (emptyCursor == 0) {
                        Intent it = new Intent();
                        it.setAction(PluginUtils.ACTION_REALOD_FAILED);
                        it.putExtra("ipMsgId", msgId);
                        // MediatekFactory.getApplicationContext().sendStickyBroadcast(it);
                        Logger.w(TAG,
                                "loadMessageFronId() empty FT cursor, removing from mms DB");
                    }
                    Logger.w(TAG, "loadMessageFronId() empty FT cursor");
                    return null;
                }
            } finally {
                if (null != cursor) {
                    cursor.close();
                }
            }
        }
    }

    @Override
    public void closeAllChat() {
        Logger.w(TAG, "closeAllChat()");
        Collection<IChat1> chatSet = mChatMap.values();
        List<Object> tagList = new ArrayList<Object>();
        for (IChat1 iChat : chatSet) {
            tagList.add(((ChatImpl) iChat).getChatTag());
        }
        for (Object object : tagList) {
            removeChat(object);
        }
        // clear all chat history
        ContentResolver contentResolver = ApiManager.getInstance().getContext()
                .getContentResolver();
        contentResolver.delete(ChatLog.Message.CONTENT_URI, null, null);
        contentResolver.delete(FileTransferLog.CONTENT_URI, null, null);
    }

    @Override
    public void closeAllChatFromMemory() {
        Logger.w(TAG, "closeAllChat()");
        Collection<IChat1> chatSet = mChatMap.values();
        List<Object> tagList = new ArrayList<Object>();
        for (IChat1 iChat : chatSet) {
            tagList.add(((ChatImpl) iChat).getChatTag());
        }
        for (Object object : tagList) {
            removeChat(object);
        }

    }

    /**
     * Get group chat with chat id.
     * .
     * @param chatId
     *            The chat id.
     * @return The group chat.
     */
    public IChat1 getGroupChat(String chatId) {
        Logger.w(TAG, "getGroupChat() entry, chatId: " + chatId);
        Collection<IChat1> chats = mChatMap.values();
        IChat1 result = null;
        for (IChat1 chat : chats) {
            if (chat instanceof GroupChat1) {
                String id = ((GroupChat1) chat).getChatId();
                if (id != null && id.equals(chatId)) {
                    result = chat;
                    break;
                }
            }
        }
        Logger.w(TAG, "getGroupChat() exit, result: " + result);
        return result;
    }

    /**
     * Used to reload message information.
     */
    private static class ReloadMessageInfo {
        private Object mMessage;
        private int mMessageType;
        private int mMessageStatus;
        private int mMessageTag;
        private String mChatId = null;

        public int getMessageTag() {
            return mMessageTag;
        }

        public void setMessageTag(int messageTag) {
            this.mMessageTag = messageTag;
        }

        /**
         * Constructor.
         * .
         * @param message
         *            The message.
         * @param type
         *            The message type.
         * @param status
         *            The message status.
         */
        public ReloadMessageInfo(Object message, int type, int status) {
            this.mMessage = message;
            this.mMessageType = type;
            this.mMessageStatus = status;
        }

        /**
         * Constructor.
         * .
         * @param message
         *            The message.
         * @param type
         *            The message type.
         * @param status
         *            The message status.
         */
        public ReloadMessageInfo(Object message, int type, int status,
                String chatId) {
            this.mMessage = message;
            this.mMessageType = type;
            this.mMessageStatus = status;
            this.mChatId = chatId;
        }

        /**
         * Constructor.
         * .
         * @param message
         *            The message.
         * @param type
         *            The message type.
         */
        public ReloadMessageInfo(Object message, int type) {
            this.mMessage = message;
            this.mMessageType = type;
        }

        /**
         * Get the message.
         * .
         * @return The message.
         */
        public Object getMessage() {
            return mMessage;
        }

        /**
         * Get the message type.
         * .
         * @return The message type.
         */
        public int getMessageType() {
            return mMessageType;
        }

        /**
         * Get the message status.
         * .
         * @return The message status.
         */
        public int getMessageStatus() {
            return mMessageStatus;
        }

        /**
         * Get the message type.
         * .
         * @return The message type.
         */
        public String getChatId() {
            return mChatId;
        }
    }


    private class GroupChatModelListener extends GroupChatListener {

        public static final String TAG = "M0CF GroupChatModelListener";
        /*private GroupChatImplStatus mStatus = GroupChatImplStatus.UNKNOWN;
        protected GroupChat mGroupChatImpl = null;
        private GroupChatImplInfo mGroupChatImplInfo = null;
        private GroupChatParticipants mGroupChatParticipants = new GroupChatParticipants();
        private ArrayList<String> mParticipantsToBeInvited = new ArrayList<String>();*/

        /*protected GroupChatModelListener(Set<String> pSet) {
            // mGroupChatImpl = info.getGroupchatImpl();
            // mGroupChatImplInfo = info;
            // mGroupChatImplStack.push(mGroupChatImplInfo);
            List<String> pList = new ArrayList<String>();
            if (pSet != null) {
                pList = convertSetToList(pSet);
            }
            if (pList == null) {
                return;
            }
            mGroupChatParticipants
                    .initicalGroupChatParticipant(convertContactsToParticipants(pList));
            List<String> currentList =
                convertParticipantInfosToContacts(mCurrentParticipants.mParticipants);
            for (String contact : currentList) {
                if (!pList.contains(contact)) {
                    mParticipantsToBeInvited.add(contact);
                    Logger.v(TAG,
                            "GroupChatModelListener mParticipantsToBeInvited contact: "
                                    + contact);
                }
            }
            mCurrentParticipants = mGroupChatParticipants;
            // TODO auto-invite OTHER participants.
        }*/

        protected GroupChatModelListener() {
        }

        /**
         * Callback called when the group chat state is changed
         *
         * @param chatId chat id
         * @param state group chat state
         * @param reasonCode reason code
         */
        public void onStateChanged(String chatId, GroupChat.State state, GroupChat.ReasonCode reasonCode){
            Logger.v(TAG, "onStateChanged chatid " + chatId + ",state" + state.toInt() +
                    "reasonCode" + reasonCode.toInt());
            GroupChat1 chat = (GroupChat1) getChatImpl(chatId);
            if(chat == null){
                Logger.v(TAG, "onStateChanged chat is null");
                return;
            }

            try {
            if(state == GroupChat.State.STARTED){
                chat.onSessionStarted();
            } else if(state == GroupChat.State.ABORTED){
                chat.onSessionAborted();
            } else if(state == GroupChat.State.FAILED ||
                    state == GroupChat.State.REJECTED ||
                    state == GroupChat.State.CLOSED_BY_USER){
                chat.onSessionError(reasonCode.toInt());
            }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        /**
         * Callback called when an Is-composing event has been received. If the remote is typing a
         * message the status is set to true, else it is false.
         *
         * @param chatId the chat ID
         * @param contact the contact ID
         * @param status Is-composing status
         */
        public void onComposingEvent(String chatId, ContactId contact, boolean status){
            Logger.v(TAG, "onComposingEvent chatid " + chatId + ",contact " + contact.toString() +
                    "status " + status);
            GroupChat1 chat = (GroupChat1) getChatImpl(chatId);
            if(chat == null){
                Logger.v(TAG, "onComposingEvent chat is null");
                return;
            }
            chat.onComposingEvent(contact.toString(), status);
        }

        /**
         * Callback called when a message status/reasonCode is changed.
         *
         * @param chatId chat id
         * @param mimeType MIME-type
         * @param msgId message id
         * @param status message status
         * @param reasonCode reason code
         */
        public void onMessageStatusChanged(String chatId, String mimeType, String msgId,
                ChatLog.Message.Content.Status status, ChatLog.Message.Content.ReasonCode reasonCode){
            Logger.v(TAG, "onMessageStatusChanged chatid " + chatId + ",mimetype " + mimeType +
                    "status " + status.toInt() + "msgId:" + msgId + ",reason:" + reasonCode.toInt());

            GroupChat1 chat = (GroupChat1) getChatImpl(chatId);
            if(chat == null){
                Logger.v(TAG, "onMessageStatusChanged chat is null");
                return;
            }

            if(status == ChatLog.Message.Content.Status.SENT){
                chat.onReportSentMessage(msgId);
            } else if(status == ChatLog.Message.Content.Status.FAILED){
                chat.onReportFailedMessage(msgId);
            } else if(status == ChatLog.Message.Content.Status.DELIVERED ||
                    status == ChatLog.Message.Content.Status.RECEIVED ||
                    status == ChatLog.Message.Content.Status.UNREAD_REPORT ||
                    status == ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED){
                chat.onReportMessageDelivered(msgId);
            } else if(status == ChatLog.Message.Content.Status.DISPLAYED){
                chat.onReportMessageDisplayed(msgId);
            }
        }

        /**
         * Callback called when a group delivery info status/reasonCode was changed for a single
         * recipient to a group message.
         *
         * @param chatId chat id
         * @param contact contact
         * @param mimeType MIME-type
         * @param msgId message id
         * @param status message status
         * @param reasonCode status reason code
         */
        public void onMessageGroupDeliveryInfoChanged(String chatId, ContactId contact,
                String mimeType, String msgId, GroupDeliveryInfo.Status status,
                GroupDeliveryInfo.ReasonCode reasonCode){
            Logger.v(TAG, "onMessageGroupDeliveryInfoChanged chatid " + chatId + ",mimetype " + mimeType +
                    "status " + status.toInt() + "msgId:" + msgId + ",reason:" + reasonCode.toInt() + ",contact"
                    + contact.toString());

            GroupChat1 chat = (GroupChat1) getChatImpl(chatId);
            if(chat == null){
                Logger.v(TAG, "onMessageGroupDeliveryInfoChanged chat is null");
                return;
            }

            if(status == GroupDeliveryInfo.Status.DELIVERED){
                chat.onReportMessageDeliveredContact(msgId, contact.toString());
            } else if(status == GroupDeliveryInfo.Status.DISPLAYED){
                chat.onReportMessageDisplayedContact(msgId, contact.toString());
            } else if(status == GroupDeliveryInfo.Status.FAILED){
                chat.onReportMessageFailedContact(msgId, contact.toString());
            }
        }

        /**
         * Callback called when a participant status has been changed in a group chat.
         *
         * @param chatId chat id
         * @param contact contact id
         * @param status participant status
         */
        public void onParticipantStatusChanged(String chatId, ContactId contact,
                ParticipantStatus status){
            Logger.v(TAG, "onParticipantStatusChanged chatid " + chatId + ",contact:" + contact.toString()
                    + ",status:" + status);

            GroupChat1 chat = (GroupChat1) getChatImpl(chatId);
            if(chat == null){
                Logger.v(TAG, "onParticipantStatusChanged chat is null");
                return;
            }

            if(status == ParticipantStatus.CONNECTED){
                chat.onParticipantJoined(contact.toString(), contact.toString());
            } else if(status == ParticipantStatus.DISCONNECTED ||
                    status == ParticipantStatus.FAILED ||
                    status == ParticipantStatus.DECLINED ||
                    status == ParticipantStatus.TIMEOUT){
                chat.onParticipantDisconnected(contact.toString());
            } else if(status == ParticipantStatus.DEPARTED){
                chat.onParticipantLeft(contact.toString());
            }
        }

        /**
         * Callback called when a delete operation completed that resulted in that one or several group
         * chats was deleted specified by the chatIds parameter.
         *
         * @param chatIds chat ids of those deleted chats
         */
        public void onDeleted(Set<String> chatIds){
            Logger.v(TAG, "onDeleted chatid " + chatIds);
        }

        /**
         * Callback called when a delete operation completed that resulted in that one or several group
         * chat messages was deleted specified by the msgIds parameter corresponding to a specific group
         * chat.
         *
         * @param chatId chat id of those deleted messages
         * @param msgIds message ids of those deleted messages
         */
        public void onMessagesDeleted(String chatId, Set<String> msgIds){
            Logger.v(TAG, "onDeleted chatid " + chatId + ",msgIds:" + msgIds);
        }

        /**
         * @param info .
         *//*
        public void setGroupChatImplInfo(GroupChatImplInfo info) {
            Logger.v(TAG, "setGroupChatImplInfo " + info.getGroupchatImpl());
            mGroupChatImpl = info.getGroupchatImpl();
            mGroupChatImplInfo = info;
            mGroupChatImplStack.push(mGroupChatImplInfo);

        }*/

        private void onDestroy() {
            Logger.v(TAG, "onDestroy entry, mGroupChatImpl: ");
            /*mCurrentParticipants.resetAllStatus();
            if (null != mGroupChatImpl) {
                try {
                    Logger.v(TAG, "onDestroy() mGroupChatImpl" + mGroupChatImpl);
                    mGroupChatImpl.removeEventListener(this);
                } catch (Exception e) {
                    Logger.e(TAG, "onDestroy() RcsServiceException" + e);
                    e.printStackTrace();
                }
            }*/
        }

        /*public  void onInviteParticipantsResult(int errType, String statusCode){
            Logger.v(TAG, "onInviteParticipantsResult(), statusCode: " + statusCode);

            if(statusCode.contains("Maximum number of participants reached")){
                mWorkerHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            Logger.i(TAG, "onSessionError()");
                            Toast.makeText(ApiManager.getInstance().getContext(), "Too Many Participants", Toast.LENGTH_LONG).show();

                        } catch(Exception e){
                            e.printStackTrace();
                        }
                }
                });
            }
        }*/

        /*public  void onGroupChatDissolved(){
        }*/

        /**
         * Callback called when SIP notify is received in group conference
         * (Callback received for add participant, remove participant, User left, nick name change)
         * @param confState conference state
         */
        /*public void onConferenceNotify(String confState, List<ConferenceUser> users){

        }*/

        /**
         * Callback called participant is kicked out(removed) by chairman
         * (Callback received by other than removed participants)
         * @param contact kicked out participant
         *//*
        public void onReportParticipantKickedOut(String contact){

        }

        *//**
         * Callback called when the session has been aborted by chairman
         *//*
        public void onSessionAbortedbyChairman(){

        }

        *//** Callback called when nickname is modified
        * (callback received only by user who modified the nickname)
        * @param errType errorType
        *//*
       public void onModifyNickNameResult(int errType, int statusCode){

        }

       *//**
        * Callback called when nickname is changed
        * (callback received by every group member)
        * @param contact contact who modified nick name
        *//*
       public  void onNickNameChanged(String contact, String newNickName){

       }*/



        /**
     * Callback called when a message has failed to be delivered to the remote
     *
     * @param msgId Message ID
     */
    /*public  void onReportFailedMessage(String msgId, int errtype, String statusCode) {

        Logger.e(TAG, "onReportFailedMessage() entry, msgId: " + msgId);
        ISentChatMessage msg = (ISentChatMessage) mChatWindow
                .getSentChatMessage(msgId);
        if (msg != null) {
            msg.updateStatus(Status.FAILED);
        }
    }

        *//**
     * Callback called when a message has failed to be delivered to the remote
     *
     * @param msgId Message ID
     *//*
    public  void onReportDeliveredMessage(String msgId){
    }

    *//**
     * Callback called when a message has been sent to remote
     *
     * @param msgId Message ID
     *//*
    public  void onReportSentMessage(final String msgId){
        Logger.i(TAG, "ABCG onReportSentMessage() entry, msgId: "+ msgId);
        Thread t = new Thread() {
            public void run() {
                Logger.i(TAG, "ABCG onReportSentMessage1() entry, msgId: "+ msgId);
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {

                    e.printStackTrace();
                }
                ISentChatMessage msg = (ISentChatMessage) mChatWindow
                        .getSentChatMessage(msgId);

                if (msg != null) {
                    msg.updateStatus(Status.SENT);
                }
            }
        };
        t.start();
    }

        *//**
         * Callback called when the session is well established and messages may
         * be exchanged with the group of participants.
         *//*
        public void onSessionStarted() {
            Logger.v(TAG, "onSessionStarted entry, mStatus: " + mStatus);
            mCurrentGroupChatImpl.set(mGroupChatImpl);
            mStatus = GroupChatImplStatus.ACTIVE;
            changeGroupChatImplStatus(mGroupChatImplInfo, mStatus);
            ((IGroupChatWindow) mChatWindow).setIsComposing(false, null);
            ((IGroupChatWindow) mChatWindow).setmChatID(mChatId);
            ((IGroupChatWindow) mChatWindow).onSessionStarted();
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    sendRestoredMessages(mGroupChatImpl);
                    clearRestoredMessages();
                }
            });
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
            new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                    Logger.v(TAG, "mReceivedMessagesDelayed = "
                            + mReceivedMessagesDelayed);
                    if (mReceivedMessagesDelayed != null) {
                        int size = mReceivedMessagesDelayed.size();
                        for (int i = 0; i < size; i++) {
                            markMessageAsDisplayed(mReceivedMessagesDelayed
                                    .get(i));
                        }
                        mReceivedMessagesDelayed.clear();
                    }
                    return null;
                }
            } .execute();

            new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                    Logger.v(TAG,
                            "handleSessionStarted entry startGroupFileStransfer");
                    ModelImpl instance = (ModelImpl) ModelImpl.getInstance();
                    if (instance != null) {
                        instance.startGroupFileStransfer();
                    }
                    Logger.v(TAG,
                            "handleSessionStarted exit startGroupFileStransfer");
                    return null;
                }
            } .execute();
        }

        *//**
         * Callback called when the session has been aborted or terminated .
         *//*
        public void onSessionAborted() {
            Logger.e(TAG, "onSessionAborted() entry, mStatus: " + mStatus);
            ((IGroupChatWindow) mChatWindow).setIsComposing(false, null);
            int location = mGroupChatImplStack.indexOf(mGroupChatImplInfo);
            if (location != -1) {
                mStatus = mGroupChatImplStack.get(location)
                        .getGroupChatImplStatus();
            }
            if (mStatus != GroupChatImplStatus.AUTO_REJOIN) {
                mStatus = GroupChatImplStatus.MANULLY_REJOIN;
            }
            changeGroupChatImplStatus(mGroupChatImplInfo, mStatus);
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
            Logger.v(TAG, "handleSessionAborted() exit, mStatus: " + mStatus);
            onDestroy();
        }

        *//**
         * Callback called when the session has failed.
         * .
         * @param error
         *            Error.
         * @see GroupChat.Error.
         *//*
        public void onSessionError(int error) {
            Logger.e(TAG, "onSessionError() entry, error: " + error
                    + " mStatus: " + mStatus);
            ((IGroupChatWindow) mChatWindow).setIsComposing(false, null);
            int location = mGroupChatImplStack.indexOf(mGroupChatImplInfo);
            if (location != -1) {
                mStatus = mGroupChatImplStack.get(location)
                        .getGroupChatImplStatus();
            }
            if(error == GroupChat.Error.INVITATION_DECLINED_TOO_MANY_PARTICIPANTS){
                if(mStatus == GroupChatImplStatus.REJOINING)
                    mStatus = GroupChatImplStatus.MANULLY_RESTART;
                else if(mStatus == GroupChatImplStatus.INVITING)
                    mStatus = GroupChatImplStatus.MANULLY_REJOIN;
                else
                    mStatus = GroupChatImplStatus.UNKNOWN;

                mWorkerHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            Logger.i(TAG, "onSessionError()");
                            Toast.makeText(ApiManager.getInstance().getContext(), "Too Many Participants", Toast.LENGTH_LONG).show();

                        } catch(Exception e){
                            e.printStackTrace();
                        }
                }
                });

            }
            else if ((mGroupChatImplStack.size() == 1 && mStatus == GroupChatImplStatus.INVITING)) {
                mStatus = GroupChatImplStatus.UNKNOWN;
            } else if (mStatus == GroupChatImplStatus.RESTARTING) {

                *//**
                 * M: added to restart a new grp chat in case of error or
                 * timeout @(
                 *//*
                // mStatus = SessionStatus.TERMINATED;
                Logger.v(TAG,
                        "set mStatus = GroupChatImplStatus.MANULLY_RESTART");
                mStatus = GroupChatImplStatus.MANULLY_RESTART;
                *//**
                 * @}
                 *//*

            } else if (mStatus == GroupChatImplStatus.REJOINING) {
                mStatus = GroupChatImplStatus.MANULLY_RESTART;
                restartGroup(null, mChatId);
                return;
            } else {
                mStatus = GroupChatImplStatus.MANULLY_REJOIN;
            }
            changeGroupChatImplStatus(mGroupChatImplInfo, mStatus);
            Message msg = Message.obtain();
            msg.what = GroupChatHandler.UPDATE_SEND_BUTTON;
            mUiHandler.sendMessage(msg);
            onDestroy();
            Logger.v(TAG, "onSessionError() exit, mStatus: " + mStatus);
        }

        *//**
         * Callback called when a new message has been received.
         * .
         * @param message
         *            New chat message.
         * @see ChatMessage.
         *//*
        public void onNewMessage(ChatMessage message) {
            Logger.v(
                    TAG,
                    "ABCG onNewMessage() entry DISPLAY: "
                            + message.getDisplayName());
            GroupChatModelListener.this.handleReceiveMessage(message,
                    mGroupChatParticipants);
            Logger.v(TAG, "onNewMessage() exit");
        }

        *//**
         * Callback called when a new geoloc has been received.
         * .
         * @param message
         *            Geoloc message.
         * @see GeolocMessage.
         *//*
        public void onNewGeoloc(GeolocMessage message) {
        }

        *//**
         * Callback called when a message has been delivered to the remote.
         * .
         * @param msgId
         *            Message ID.
         * @param contact .
         *//*
        public void onReportMessageDeliveredContact(String msgId, String contact) {
            Logger.e(TAG, "ABCG onReportMessageDelivered() entry, msgId: "
                    + msgId + "contact: " + contact);
            ISentChatMessage msg = (ISentChatMessage) mChatWindow
                    .getSentChatMessage(msgId);
            if (msg != null) {
                msg.updateStatus(Status.DELIVERED, contact, null); // TODo Contact
                                                             // info required
                                                             // from stack
            }
        }

        *//**
         * Callback called when a message has been displayed by the remote.
         * .
         * @param msgId
         *            Message ID.
         * @param contact .
         *//*
        public void onReportMessageDisplayedContact(String msgId, String contact) {
            Logger.v(TAG, "ABCG onReportMessageDisplayed() entry, msgId: "
                    + msgId + "contact: " + contact);
            ISentChatMessage msg = (ISentChatMessage) mChatWindow
                    .getSentChatMessage(msgId);
            if (msg != null) {
                msg.updateStatus(Status.DISPLAYED, contact, null); // TODo Contact
                                                             // info required
                                                             // from stack
            }
        }

        *//**
         * Callback called when a message has failed to be delivered to the
         * remote.
         * .
         * @param msgId
         *            Message ID.
         * @param contact .
         *//*
        public void onReportMessageFailedContact(String msgId, String contact) {
            Logger.e(TAG, "ABCG onReportMessageFailedContact() entry, msgId: " + msgId
                    + "contact: " + contact);
            ISentChatMessage msg = (ISentChatMessage) mChatWindow
                    .getSentChatMessage(msgId);
            if (msg != null) {
                msg.updateStatus(Status.FAILED, contact, null);
            }
        }

        *//**
         * Callback called when a message has been delivered to the remote.
         * .
         * @param msgId
         *            Message ID.
         *//*
        public void onReportMessageDelivered(String msgId) {
            Logger.v(TAG, "onReportMessageDelivered() entry, msgId: " + msgId);
        }

        *//**
         * Callback called when a message has been displayed by the remote.
         * .
         * @param msgId
         *            Message ID.
         *//*
        public void onReportMessageDisplayed(String msgId) {
            Logger.v(TAG, "onReportMessageDisplayed() entry, msgId: " + msgId);
        }

        *//**
         * Callback called when a message has failed to be delivered to the
         * remote.
         * .
         * @param msgId
         *            Message ID.
         *//*
        public void onReportMessageFailed(String msgId) {
            Logger.e(TAG, "onReportMessageFailed() entry, msgId: " + msgId);
            ISentChatMessage msg = (ISentChatMessage) mChatWindow
                    .getSentChatMessage(msgId);
            if (msg != null) {
                msg.updateStatus(Status.FAILED);
            }
        }

        *//**
         * Callback called when an Is-composing event has been received. If the
         * remote is typing a message the status is set to true, else it is
         * false.
         * .
         * @param contact
         *            Contact.
         * @param status
         *            Is-composing status.
         *//*
        public void onComposingEvent(String contact, boolean status) {
            Logger.v(TAG, "onComposingEvent() contact: " + contact
                    + " status: " + status);
            int size = mGroupChatParticipants.mParticipants.size();
            String contactNumber = Utils.extractNumberFromUri(contact);
            for (int index = 0; index < size; index++) {
                final Participant participant = mGroupChatParticipants.mParticipants
                        .get(index).mParticipant;
                Logger.v(TAG, "onComposingEvent() participant: " + participant);
                if (null != participant) {
                    String remoteContact = participant.getContact();
                    if (null != remoteContact
                            && remoteContact.equals(contactNumber)) {
                        ((IGroupChatWindow) mChatWindow).setIsComposing(status,
                                participant);
                    }
                }
            }
        }

        *//**
         * Callback called when a new participant has joined the group chat.
         * .
         * @param contact
         *            Contact.
         * @param contactDisplayname
         *            Contact displayname.
         *//*
        public void onParticipantJoined(String contact,
                String contactDisplayname) {
            boolean result = mGroupChatParticipants.updateParticipants(contact,
                    contactDisplayname, User.STATE_CONNECTED,
                    mParticipantsToBeInvited);
            if (result) {
                mParticipantsToBeInvited.remove(contact);
            }
            Logger.v(TAG, "onParticipantJoined() result: " + result);
        }

        *//**
         * Callback called when a participant has left voluntary the group chat.
         * .
         * @param contact
         *            Contact.
         *//*
        public void onParticipantLeft(String contact) {
            boolean result = mGroupChatParticipants.updateParticipants(contact,
                    null, User.STATE_DEPARTED, mParticipantsToBeInvited);
            if (result) {
                mParticipantsToBeInvited.remove(contact);
            }
            Logger.v(TAG, "onParticipantLeft() result: " + result);
        }

        *//**
         * Callback called when a participant is disconnected from the group
         * chat.
         * .
         * @param contact
         *            Contact.
         *//*
        public void onParticipantDisconnected(String contact) {
            boolean result = mGroupChatParticipants.updateParticipants(contact,
                    null, User.STATE_DISCONNECTED, mParticipantsToBeInvited);
            if (result) {
                mParticipantsToBeInvited.remove(contact);
            }
            Logger.v(TAG, "onParticipantDisconnected() result: " + result);
        }

        @Override
        public void onSetChairmanResult(int errType, int statusCode) {
            // TODO Auto-generated method stub

        }

        @Override
        public void onChairmanChanged(String newChairman) {
            // TODO Auto-generated method stub

        }

        @Override
        public void onModifySubjectResult(int errType, int statusCode) {
            // TODO Auto-generated method stub

        }

        @Override
        public void onSubjectChanged(String newSubject) {
            // TODO Auto-generated method stub

        }

        @Override
        public void onRemoveParticipantResult(int errType, int statusCode, String participant) {
            // TODO Auto-generated method stub

        }

        @Override
        public void onReportMeKickedOut(String from) {
            // TODO Auto-generated method stub

        }

        @Override
        public void onAbortConversationResult(int errType, int statusCode) {
            // TODO Auto-generated method stub

        }

        @Override
        public void onQuitConversationResult(int errType, int statusCode) {
            // TODO Auto-generated method stub

        }*/
     }

    private class One2OneChatListener extends OneToOneChatListener {

        private static final String TAG = "M0CF One2OneChatListener";

        //private OneToOneChat mCurrentChatImpl = null;

        private String mSessionId = null;
        private String mCallId = null;

        /**
         * .
         */
        public One2OneChatListener() {
                Logger.v(TAG, "Constructor() session:" + mSessionId
                        + " Call Id: " + mCallId);
                /*
                 * try { //mSessionId = currentChatImpl.getSessionID(); //
                 * mCallId = currentChatImpl.getChatID(); Logger.v(TAG,
                 * "Constructor() session:" + mSessionId + " Call Id: " +
                 * mCallId); } catch (RemoteException e) { e.printStackTrace();
                 * }
                 */

        }

        /**
         * Callback called when a message status/reasonCode is changed.
         *
         * @param contact Contact ID
         * @param mimeType MIME-type of message
         * @param msgId Message Id
         * @param status Status
         * @param reasonCode Reason code
         */
        public void onMessageStatusChanged(ContactId contact, String mimeType, String msgId,
                ChatLog.Message.Content.Status status, ChatLog.Message.Content.ReasonCode reasonCode){
            Logger.v(TAG, "onMessageStatusChanged contact " + contact.toString() + ",mimetype " + mimeType +
                    "status " + status.toInt() + "msgId:" + msgId + ",reason:" + reasonCode.toInt());
            String number = contact.toString();
            One2OneChat chat = (One2OneChat) getChatImpl(number);
            if(chat == null){
                Logger.v(TAG, "onMessageStatusChanged chat is null");
                return;
            }

            if(status == ChatLog.Message.Content.Status.SENT){
                chat.onReportMessageSent(msgId);
            } else if(status == ChatLog.Message.Content.Status.FAILED){
                chat.onReportMessageFailed(msgId, reasonCode.toInt());
            } else if(status == ChatLog.Message.Content.Status.DELIVERED ||
                    status == ChatLog.Message.Content.Status.RECEIVED ||
                    status == ChatLog.Message.Content.Status.UNREAD_REPORT ||
                    status == ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED){
                chat.onReportMessageDelivered(msgId);
            } else if(status == ChatLog.Message.Content.Status.DISPLAYED){
                chat.onReportMessageDisplayed(msgId);
            }
        }

        /**
         * Callback called when an Is-composing event has been received. If the remote is typing a
         * message the status is set to true, else it is false.
         *
         * @param contact Contact ID
         * @param status Is-composing status
         */
        public void onComposingEvent(ContactId contact, boolean status){
            Logger.v(TAG, "onComposingEvent contact " + contact.toString() +
                    "status " + status);
            String number = contact.toString();
            One2OneChat chat = (One2OneChat) getChatImpl(number);
            if(chat == null){
                Logger.v(TAG, "onComposingEvent chat is null");
                return;
            }

            chat.onComposingEvent(status);
        }

        /**
         * Callback called when a delete operation completed that resulted in that one or several one to
         * one chat messages was deleted specified by the msgIds parameter corresponding to a specific
         * contact.
         *
         * @param contact contact id of those deleted messages
         * @param msgIds message ids of those deleted messages
         */
        public void onMessagesDeleted(ContactId contact, Set<String> msgIds){
            Logger.v(TAG, "onDeleted contact " + contact.toString() + ",msgIds:" + msgIds);
        }

        /**
         * @param o2oChatImpl
         */
        /*public void setChatImpl(Chat o2oChatImpl) {
            mCurrentChatImpl = o2oChatImpl;
        }*/

        private void destroySelf() {
            Logger.v(TAG, "destroySelf entry  ");
            /*if (null != mCurrentChatImpl) {
                try {
                    Logger.v(TAG, "destroySelf() session id is " + mSessionId
                            + " Call Id: " + mCallId);
                    mCurrentChatImpl.removeEventListener(this);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                mCurrentChatImpl = null;
            }
            onChatListenerDestroy(this);*/
        }

        /**
         * Callback called when a new message has been received.
         * .
         * @param message
         *            Chat message.
         * @see ChatMessage
         */
        public void onNewMessage(final ChatMessage message) {
            Logger.v(TAG, "onNewMessage()   message id:" + message.getId()
                    + " message text:" + message.getContent());
            /*mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                   // onReceiveMessage(message);
                }
            });*/
        }

       public  void onNewBurnFileTransfer(String transferId,Boolean isGroup,String chatSessionId,String ChatId){
        }

        public  void onNewBurnMessageArrived(ChatMessage msg){
        }

        /**
         * Callback called when a new geoloc has been received.
         * .
         * @param message
         *            Geoloc message.
         * @see GeolocMessage
         */
        /*public void onNewGeoloc(GeolocMessage message) {
        }*/

        /**
     * Callback called when a message has failed to be delivered to the remote
     *
     * @param msgId Message ID
     */
    public  void onReportDeliveredMessage(String msgId){
    }

    /**
     * Callback called when a message is sent to the remote
     *
     * @param msgId Message ID
     */
    /*public  void onReportMessageSent(final String msgId){
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
        public void onReportMessageDelivered(final String msgId) {
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
        public void onReportMessageDisplayed(final String msgId) {
            Logger.v(TAG, "onReportMessageDisplayed()  session:" + msgId);
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
         * @param timestamp
         *            Time of delivery.
         *//*
        public void onReportMessageTimestampDelivered(final String msgId, final long timestamp) {
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
         *            Time of delivery.
         *//*
        public void onReportMessageTimestampDisplayed(final String msgId, final long timestamp) {
            Logger.v(TAG, "onReportMessageDisplayed()  session:" + msgId);
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mSentMessageManager.onMessageDelivered(msgId,
                            ISentChatMessage.Status.DISPLAYED, timestamp);
                }
            });
        }

        *//**
         * Callback called when a message has failed to be delivered to the
         * remote.
         * .
         * @param msgId
         *            Message ID.
         *//*
        public void onReportMessageFailed(final String msgId) {
            Logger.e(TAG, "onReportMessageFailed()  session:" + msgId);
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mSentMessageManager.onMessageDelivered(msgId,
                            ISentChatMessage.Status.FAILED, 0);
                }
            });
        }

  *//**
         * Callback called when a message has failed because of some error
         * .
         * @param msgId
         *  Message ID.
         *//*
        public void onReportFailedMessage(final String msgId , int error, final String statusCode) {
            Logger.e(TAG, "onReportFailedMessage()  session:" + msgId + " Status: " + statusCode);
            try {
                String tempResult = null;
                ContentResolver contentResolver = MediatekFactory.getApplicationContext().getContentResolver();
                Cursor cursor = contentResolver.query(ChatLog.Message.CONTENT_URI,
                        new String[] {
                        ChatLog.Message.MESSAGE_ID,
                        ChatLog.Message.CONTENT
                    },
                    "(" + ChatLog.Message.MESSAGE_ID + "='" + msgId + "')",
                    null,
                    ChatLog.Message.TIMESTAMP + " DESC");

                if (cursor.moveToFirst()) {
                    byte[] blobText = null;
                    blobText = cursor.getBlob(1);

                    if(blobText!=null){
                        String status = new String(blobText);
                        Logger.i(TAG, "onReportFailedMessage()  session:" + status);
                        tempResult = status;
                    }
                }
                cursor.close();

                final String result = tempResult;

                if(statusCode.contains("415") || statusCode.contains("timeout")){
                    //Display "Media Not Allowed"
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            try {
                                Logger.i(TAG, "onReportFailedMessage()  text:" + msgId);
                                if(statusCode.contains("415")) {
                                    Toast.makeText(ApiManager.getInstance().getContext(), "Media Not Allowed", Toast.LENGTH_LONG).show();
                                }
                                mSentMessageManager.onMessageDelivered(msgId,ISentChatMessage.Status.FAILED, 0);
                            } catch(Exception e){
                                e.printStackTrace();
                            }
                    }
                    });
                } else if(statusCode.contains("408") || statusCode.contains("403")){
                    // Send text message
                    Thread t = new Thread() {
                        public void run() {
                            try {
                                Logger.i(TAG, "onReportFailedMessage()  text:" + result);
                                mWorkerHandler.post(new Runnable() {
                                    @Override
                                    public void run() {
                                        mSentMessageManager.onMessageDelivered(msgId,
                                                ISentChatMessage.Status.DECLINED, 0);
                                    }
                                });
                            } catch(Exception e){
                                e.printStackTrace();
                            }
                        }
                    };
                    t.start();
                } else if(statusCode.contains("400")) {
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mSentMessageManager.onMessageDelivered(msgId,
                                    ISentChatMessage.Status.FAILED, 0);
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
        public void onReportMessageInviteFailed(final String msgId) {
            Logger.e(TAG, "onReportMessageInviteFailed()  session:" + msgId);
            try {
                String result = null;
                ContentResolver contentResolver = MediatekFactory.getApplicationContext().getContentResolver();
                Cursor cursor = contentResolver.query(ChatLog.Message.CONTENT_URI,
                new String[] {
                    ChatLog.Message.MESSAGE_ID,
                    ChatLog.Message.CONTENT
                },
                "(" + ChatLog.Message.MESSAGE_ID + "='" + msgId + "')",
                null,
                ChatLog.Message.TIMESTAMP + " DESC");

                if (cursor.moveToFirst()) {
                    byte[] blobText = null;
                    blobText = cursor.getBlob(1);

                    if(blobText!=null){
                        String status = new String(blobText);
                        Logger.i(TAG, "onReportMessageInviteFailed()  session:" + status);
                                 result = status;
                    }
                }
                cursor.close();

                if(result.length() < MAX_PAGER_LENGTH){
                    // Send text message
                    Thread t = new Thread() {
                        public void run() {
                            try {
                                mCurrentChatImpl.resendMessage(msgId);
                            } catch(Exception e){
                                e.printStackTrace();
                            }
                        }
                    };
                    t.start();
                } else {
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mSentMessageManager.onMessageDelivered(msgId,
                                    ISentChatMessage.Status.FAILED, 0);
                        }
                    });
                }
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
                String result = null;
                ContentResolver contentResolver = MediatekFactory.getApplicationContext().getContentResolver();
                Cursor cursor = contentResolver.query(ChatLog.Message.CONTENT_URI,
                new String[] {
                    ChatLog.Message.MESSAGE_ID,
                    ChatLog.Message.CONTENT
                },
                "(" + ChatLog.Message.MESSAGE_ID + "='" + msgId + "')",
                null,
                ChatLog.Message.TIMESTAMP + " DESC");

                if (cursor.moveToFirst()) {
                byte[] blobText = null;
                blobText = cursor.getBlob(1);

                if(blobText!=null){
                    String status = new String(blobText);
                        result = status;
                 }
                }
                cursor.close();
                boolean displayMessage = (text.contains(ModelImpl.ANONYMITY_NOT_ALLOWED) || text.contains(ModelImpl.FUNCTION_NOT_ALLOWED)
                                        || text.contains(ModelImpl.SIZE_EXCEEDED) || text.contains(ModelImpl.TEST_ERROR) || text.contains(ModelImpl.NO_DESTINATIONS));

                boolean sendMessage = text.contains(ModelImpl.VERSION_UNSUPPORTED) || text.contains(ModelImpl.SERVICE_UNAUTHORIZED);

                if(displayMessage){
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            try {
                            if(text.contains(ModelImpl.ANONYMITY_NOT_ALLOWED)){
                                    Toast.makeText(ApiManager.getInstance().getContext(), "Operator does not support anonymous requests. Please adjust your client settings", Toast.LENGTH_LONG).show();
                                } else if(text.contains(ModelImpl.SIZE_EXCEEDED)){
                                    Toast.makeText(ApiManager.getInstance().getContext(), "Message sent is too big, resize and try again", Toast.LENGTH_LONG).show();
                                } else if(text.contains(ModelImpl.FUNCTION_NOT_ALLOWED)){
                                    Toast.makeText(ApiManager.getInstance().getContext(), "You are forbidden from sending messages", Toast.LENGTH_LONG).show();
                                } else if(text.contains(ModelImpl.TEST_ERROR)){
                                    Toast.makeText(ApiManager.getInstance().getContext(), "999 Test Error", Toast.LENGTH_LONG).show();
                                } else if(text.contains(ModelImpl.NO_DESTINATIONS)){
                                    Toast.makeText(ApiManager.getInstance().getContext(), "Please select one or more recipients.", Toast.LENGTH_LONG).show();
                                }
                                mSentMessageManager.onMessageDelivered(msgId,ISentChatMessage.Status.FAILED, 0);
                        } catch(Exception e){
                            e.printStackTrace();
                        }
                    }
                    });
                } else if(result.length() < MAX_PAGER_LENGTH && sendMessage){
                    // Send text message
                    Thread t = new Thread() {
                        public void run() {
                            try {
                                mCurrentChatImpl.resendMessage(msgId);
                            } catch(Exception e){
                                e.printStackTrace();
                            }
                        }
                    };
                    t.start();
                } else if(text.contains("Decline")){
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mSentMessageManager.onMessageDelivered(msgId,
                                    ISentChatMessage.Status.DECLINED, 0);
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
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        *//**
         * Callback called when an Is-composing event has been received. If the
         * remote is typing a message the status is set to true, else it is
         * false.
         * .
         * @param status
         *            Is-composing status.
         *//*
        public void onComposingEvent(final boolean status) {
            Logger.v(TAG, "onComposingEvent()  session: the status is "
                    + status);
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    ((IOne2OneChatWindow) mChatWindow).setIsComposing(status);
                }
            });
        }*/
    }

    /**
     * One One Extend chat Listener.
     *
     */
    private class One2OneExtendChatListener extends ExtendChatListener {

        private static final String TAG = "M0CF ExtendOne2OneChatListener";

        private ExtendChat mCurrentChatImpl = null;

        private String mSessionId = null;
        private String mCallId = null;

        /**
         * .
         */
        public One2OneExtendChatListener() {
            if (null != mCurrentChatImpl) {
                Logger.v(TAG, "Constructor() session:" + mSessionId
                        + " Call Id: " + mCallId);
            }
        }

        /**
         * @param o2oChatImpl
         */
        public void setChatImpl(ExtendChat o2oChatImpl) {
            mCurrentChatImpl = o2oChatImpl;
        }

        private void destroySelf() {
            Logger.v(TAG, "destroySelf entry : mCurrentSession = "
                    + mCurrentChatImpl);
            if (null != mCurrentChatImpl) {
                try {
                    Logger.v(TAG, "destroySelf() session id is " + mSessionId
                            + " Call Id: " + mCallId);
                    //mCurrentChatImpl.removeEventListener(this);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                mCurrentChatImpl = null;
            }
            //onExtendChatListenerDestroy(this);
        }

        /**
         * Callback called when a message status/reasonCode is changed.
         *
         * @param contact Contact ID
         * @param mimeType MIME-type of message
         * @param msgId Message Id
         * @param status Status
         * @param reasonCode Reason code
         */
        public  void onMessageStatusChanged(String chatId,ContactId contact, String mimeType, String msgId,
                ChatLog.Message.Content.Status status, ChatLog.Message.Content.ReasonCode reasonCode){
            Logger.v(TAG, "onMessageStatusChanged contact " + contact.toString() + ",mimetype " + mimeType +
                    "status " + status.toInt() + "msgId:" + msgId + ",reason:" + reasonCode.toInt() + ",chatId: " + chatId);
            try {
                if(contact.toString().equals(chatId)){
            String number = contact.toString();
            One2OneChat chat = (One2OneChat) getChatImpl(number);
            if(chat == null){
                        Logger.v(TAG, "onMessageStatusChanged extendonetoone chat is null");
                return;
            }

            if(status == ChatLog.Message.Content.Status.SENT){
                chat.onReportMessageSent(msgId);
            } else if(status == ChatLog.Message.Content.Status.FAILED){
                chat.onReportMessageFailed(msgId, reasonCode.toInt());
            } else if(status == ChatLog.Message.Content.Status.DELIVERED ||
                    status == ChatLog.Message.Content.Status.RECEIVED ||
                    status == ChatLog.Message.Content.Status.UNREAD_REPORT ||
                    status == ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED){
                chat.onReportMessageDelivered(msgId);
            } else if(status == ChatLog.Message.Content.Status.DISPLAYED){
                chat.onReportMessageDisplayed(msgId);
            }
                } else {
                    MultiChat chat = (MultiChat) getChatImpl(chatId);
                    if(chat == null){
                        Logger.v(TAG, "onMessageStatusChanged multi chat is null");
                        return;
                    }

                    if(status == ChatLog.Message.Content.Status.SENT){
                        chat.onReportMessageSent(msgId);
                    } else if(status == ChatLog.Message.Content.Status.FAILED){
                        chat.onReportMessageFailed(msgId, reasonCode.toInt());
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        /**
         * Callback called when a delete operation completed that resulted in that one or several one to
         * one chat messages was deleted specified by the msgIds parameter corresponding to a specific
         * contact.
         *
         * @param contact contact id of those deleted messages
         * @param msgIds message ids of those deleted messages
         */
        public void onMessagesDeleted(ContactId contact, Set<String> msgIds){

        }
        /**
         * Callback called when a new message has been received
         *
         * @param message Chat message
         * @see ChatMessage
         */
        /*public void onNewMessage(ExtendMessage message){

        }*/

        /**
         * Callback called when an Is-composing event has been received. If the remote is typing a
         * message the status is set to true, else it is false.
         *
         * @param contact Contact ID
         * @param status Is-composing status
         */
        public void onComposingEvent(ContactId contact, boolean status){
            Logger.v(TAG, "onComposingEvent contact " + contact.toString() +
                    "status " + status);
            try {
            String number = contact.toString();
            One2OneChat chat = (One2OneChat) getChatImpl(number);
            if(chat == null){
                Logger.v(TAG, "onComposingEvent chat is null");
                return;
            }

            chat.onComposingEvent(status);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        /**
         * Callback called when a new message has been received.
         * .
         * @param message
         *            Chat message.
         * @see ChatMessage
         */
        /*public void onNewMessage(final ExtendMessage message) {
            Logger.v(TAG, "onNewMessage()   message id:" + message.getId()
                    + " message text:" + message.getMessage());
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
            Logger.v(TAG, "onReportMessageDisplayed()  session:" + msgId);
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
         *//*
        public void onReportMessageTimestampDisplayed(final String msgId, String contact,final long timestamp) {
            Logger.v(TAG, "onReportMessageDisplayed()  session:" + msgId);
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

                String tempResult = null;
                ContentResolver contentResolver = MediatekFactory.getApplicationContext().getContentResolver();
                Cursor cursor = contentResolver.query(ChatLog.Message.CONTENT_URI,
                        new String[] {
                        ChatLog.Message.MESSAGE_ID,
                        ChatLog.Message.BODY
                    },
                    "(" + ChatLog.Message.MESSAGE_ID + "='" + msgId + "')",
                    null,
                    ChatLog.Message.TIMESTAMP + " DESC");

                if (cursor.moveToFirst()) {
                    byte[] blobText = null;
                    blobText = cursor.getBlob(1);

                    if(blobText!=null){
                        String status = new String(blobText);
                        Logger.i(TAG, "onReportFailedMessage()  session:" + status);
                        tempResult = status;
                    }
                }
                cursor.close();

                final String result = tempResult;

                if(statusCode.contains("415") || statusCode.contains("timeout")){
                    //Display "Media Not Allowed"
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            try {
                                Logger.i(TAG, "onReportFailedMessage()  text:" + msgId);
                                if(statusCode.contains("415")) {
                                    Toast.makeText(ApiManager.getInstance().getContext(), "Media Not Allowed", Toast.LENGTH_LONG).show();
                                mSentMessageManager.onMessageDelivered(msgId,ISentChatMessage.Status.FAILED, 0);
                                }
                            mCurrentChatImpl.resendMessage(msgId);
                            } catch(Exception e){
                                e.printStackTrace();
                            }
                    }
                    });
                } else if(statusCode.contains("408") || statusCode.contains("403")){
                    // Send text message
                    Thread t = new Thread() {
                        public void run() {
                            try {
                                Logger.i(TAG, "onReportFailedMessage()  text:" + result);
                                if(result != null){
                                mCurrentChatImpl.sendMessage(result);
                                 }
                            } catch(Exception e){
                                e.printStackTrace();
                            }
                        }
                    };
                    t.start();
                } else if(statusCode.contains("400") && result != null && result.length() < MAX_PAGER_LENGTH) {
                    // Send text message
                    Thread t = new Thread() {
                        public void run() {
                            try {
                                Logger.i(TAG, "onReportFailedMessage()  text1:" + result);
                                    mCurrentChatImpl.resendMessage(msgId);
                            } catch(Exception e){
                                e.printStackTrace();
                            }
                        }
                    };
                    t.start();
                } else {
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mSentMessageManager.onMessageDelivered(msgId,
                                    ISentChatMessage.Status.FAILED, 0);
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
            try {
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
                String result = null;
                ContentResolver contentResolver = MediatekFactory.getApplicationContext().getContentResolver();
                Cursor cursor = contentResolver.query(ChatLog.Message.CONTENT_URI,
                new String[] {
                    ChatLog.Message.MESSAGE_ID,
                    ChatLog.Message.BODY
                },
                "(" + ChatLog.Message.MESSAGE_ID + "='" + msgId + "')",
                null,
                ChatLog.Message.TIMESTAMP + " DESC");

                if (cursor.moveToFirst()) {
                    byte[] blobText = null;
                    blobText = cursor.getBlob(1);

                    if(blobText!=null){
                        String status = new String(blobText);
                        Logger.i(TAG, "onReportMessageInviteFailed()  session:" + status);
                                 result = status;
                    }
                }
                cursor.close();

                if(result.length() < MAX_PAGER_LENGTH){
                    // Send text message
                    Thread t = new Thread() {
                        public void run() {
                            try {
                                mCurrentChatImpl.resendMessage(msgId);
                            } catch(Exception e){
                                e.printStackTrace();
                            }
                        }
                    };
                    t.start();
                } else {
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mSentMessageManager.onMessageDelivered(msgId,
                                    ISentChatMessage.Status.FAILED, 0);
                        }
                    });
                }
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
                String result = null;
                ContentResolver contentResolver = MediatekFactory.getApplicationContext().getContentResolver();
                Cursor cursor = contentResolver.query(ChatLog.Message.CONTENT_URI,
                new String[] {
                    ChatLog.Message.MESSAGE_ID,
                    ChatLog.Message.BODY
                },
                "(" + ChatLog.Message.MESSAGE_ID + "='" + msgId + "')",
                null,
                ChatLog.Message.TIMESTAMP + " DESC");

                if (cursor.moveToFirst()) {
                byte[] blobText = null;
                blobText = cursor.getBlob(1);

                if(blobText!=null){
                    String status = new String(blobText);
                        result = status;
                 }
                }
                cursor.close();
                boolean displayMessage = (text.contains(ModelImpl.ANONYMITY_NOT_ALLOWED) || text.contains(ModelImpl.FUNCTION_NOT_ALLOWED)
                                        || text.contains(ModelImpl.SIZE_EXCEEDED) || text.contains(ModelImpl.TEST_ERROR));

                boolean sendMessage = text.contains(ModelImpl.VERSION_UNSUPPORTED) || text.contains(ModelImpl.SERVICE_UNAUTHORIZED);

                if(displayMessage){
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            try {
                            if(text.contains(ModelImpl.ANONYMITY_NOT_ALLOWED)){
                                    Toast.makeText(ApiManager.getInstance().getContext(), "Operator does not support anonymous requests. Please adjust your client settings", Toast.LENGTH_LONG).show();
                                } else if(text.contains(ModelImpl.SIZE_EXCEEDED)){
                                    Toast.makeText(ApiManager.getInstance().getContext(), "Message sent is too big, resize and try again", Toast.LENGTH_LONG).show();
                                } else if(text.contains(ModelImpl.FUNCTION_NOT_ALLOWED)){
                                    Toast.makeText(ApiManager.getInstance().getContext(), "You are forbidden from sending messages", Toast.LENGTH_LONG).show();
                                } else if(text.contains(ModelImpl.TEST_ERROR)){
                                    Toast.makeText(ApiManager.getInstance().getContext(), "Media Not Allowed", Toast.LENGTH_LONG).show();
                                } else if(text.contains(ModelImpl.NO_DESTINATIONS)){
                                    Toast.makeText(ApiManager.getInstance().getContext(), "Please select one or more recipients.", Toast.LENGTH_LONG).show();
                                }
                                mSentMessageManager.onMessageDelivered(msgId,ISentChatMessage.Status.FAILED, 0);
                        } catch(Exception e){
                            e.printStackTrace();
                        }
                    }
                    });
                } else if(result.length() < MAX_PAGER_LENGTH && sendMessage){
                    // Send text message
                    Thread t = new Thread() {
                        public void run() {
                            try {
                                mCurrentChatImpl.resendMessage(msgId);
                            } catch(Exception e){
                                e.printStackTrace();
                            }
                        }
                    };
                    t.start();
                } else {
                    mWorkerHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mSentMessageManager.onMessageDelivered(msgId,
                                    ISentChatMessage.Status.FAILED, 0);
                        }
                    });
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }*/
    }

    /**
     * One One chat Listener.
     *
     */
    private class MultiChatListener extends ExtendChatListener {

        private static final String TAG = "M0CF MultiChatListener";

        private ExtendChat mCurrentChatImpl = null;

        private String mSessionId = null;
        private String mCallId = null;

        /**
         * .
         */
        public MultiChatListener() {
            if (null != mCurrentChatImpl) {
                Logger.v(TAG, "Constructor() session:" + mSessionId
                        + " Call Id: " + mCallId);
            }
        }

        /**
         * @param o2oChatImpl
         */
        public void setChatImpl(ExtendChat o2oChatImpl) {
            mCurrentChatImpl = o2oChatImpl;
        }

        private void destroySelf() {
            Logger.v(TAG, "destroySelf entry : mCurrentSession = "
                    + mCurrentChatImpl);
            if (null != mCurrentChatImpl) {
                try {
                    Logger.v(TAG, "destroySelf() session id is " + mSessionId
                            + " Call Id: " + mCallId);
                    //mCurrentChatImpl.removeEventListener(this);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                mCurrentChatImpl = null;
            }
            //onExtendChatListenerDestroy(this);
        }

        /**
         * Callback called when a message status/reasonCode is changed.
         *
         * @param contact Contact ID
         * @param mimeType MIME-type of message
         * @param msgId Message Id
         * @param status Status
         * @param reasonCode Reason code
         */
        public  void onMessageStatusChanged(String chatId,ContactId contact, String mimeType, String msgId,
                ChatLog.Message.Content.Status status, ChatLog.Message.Content.ReasonCode reasonCode){
            Logger.v(TAG, "onMessageStatusChanged chatid " + chatId + ",mimetype " + mimeType +
                    "status " + status.toInt() + "msgId:" + msgId + ",reason:" + reasonCode.toInt());

            MultiChat chat = (MultiChat) getChatImpl(chatId);
            if(chat == null){
                Logger.v(TAG, "onMessageStatusChanged chat is null");
                return;
            }

            if(status == ChatLog.Message.Content.Status.SENT){
                chat.onReportMessageSent(msgId);
            } else if(status == ChatLog.Message.Content.Status.FAILED){
                chat.onReportMessageFailed(msgId, reasonCode.toInt());
            }
        }

        /**
         * Callback called when a delete operation completed that resulted in that one or several one to
         * one chat messages was deleted specified by the msgIds parameter corresponding to a specific
         * contact.
         *
         * @param contact contact id of those deleted messages
         * @param msgIds message ids of those deleted messages
         */
        public void onMessagesDeleted(ContactId contact, Set<String> msgIds){

        }
        /**
         * Callback called when a new message has been received
         *
         * @param message Chat message
         * @see ChatMessage
         */
        /*public void onNewMessage(ExtendMessage message){

        }*/

        /**
         * Callback called when an Is-composing event has been received. If the remote is typing a
         * message the status is set to true, else it is false.
         *
         * @param contact Contact ID
         * @param status Is-composing status
         */
        public void onComposingEvent(ContactId contact, boolean status){

        }

        /**
         * Callback called when a new message has been received.
         * .
         * @param message
         *            Chat message.
         * @see ChatMessage
         */
        /*public void onNewMessage(final ExtendMessage message) {
            Logger.v(TAG, "onNewMessage()   message id:" + message.getId()
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
            Logger.v(TAG, "onReportMessageDisplayed()  session:" + msgId);
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
            Logger.v(TAG, "onReportMessageDisplayed()  session:" + msgId);
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
        }*/
    }
}
