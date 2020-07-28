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
import android.widget.Toast;

import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ModelImpl.ResumeFileTransfer;
import com.mediatek.rcse.mvc.ModelImpl.SentFileTransfer;
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
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.plugin.message.PluginGroupChatWindow;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.provider.RichMessagingDataProvider;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ContactIdUtils;
import com.mediatek.rcse.service.NetworkChangedReceiver;
import com.mediatek.rcse.service.NetworkChangedReceiver.OnNetworkStatusChangedListerner;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.Utils;


import com.mediatek.rcs.R;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingError;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;


import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
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

import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.RcsServiceException;
import com.gsma.services.rcs.JoynServiceNotAvailableException;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.capability.CapabilitiesListener;
import com.gsma.services.rcs.capability.CapabilityService;
//import com.gsma.services.rcs.chat.Chat;
import com.gsma.services.rcs.chat.ChatIntent;
import com.gsma.services.rcs.chat.ChatLog;
//import com.gsma.services.rcs.chat.ChatMessage;
import com.gsma.services.rcs.chat.ChatService;
import com.gsma.services.rcs.chat.GroupChat;
import com.gsma.services.rcs.chat.GroupChatIntent;
import com.gsma.services.rcs.ft.FileTransfer;
import com.gsma.services.rcs.ft.OneToOneFileTransferListener;
import com.gsma.services.rcs.ft.FileTransferLog;
import com.gsma.services.rcs.ft.FileTransferService;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.ft.FileTransfer.ReasonCode;
import com.gsma.services.rcs.ft.FileTransfer.State;

/**
 * It's a implementation of IChat, it indicates a specify chat model.
 * .
 */
public abstract class ChatImpl extends CapabilitiesListener implements
        IChat1, IRegistrationStatusListener {
    protected Participant mParticipant = null;

    private static final String TAG = "M0CF ChatImpl";
    // Chat message list
    private final List<IChatMessage> mMessageList = new LinkedList<IChatMessage>();

    protected final AtomicReference<GroupChat> mCurrentGroupChatImpl = new
    AtomicReference<GroupChat>();
    protected ComposingManager mComposingManager = new ComposingManager();
    protected Object mTag = null;
    protected boolean mIsInBackground = true;

    protected IChatWindow mChatWindow = null;

    protected List<ChatMessage> mReceivedInBackgroundToBeDisplayed = new
    ArrayList<ChatMessage>();
    protected List<ChatMessage> mReceivedInBackgroundToBeRead = new
    ArrayList<ChatMessage>();

 protected List<FileTransfer> mReceivedInBackgroundFtToBeDisplayed =
            new ArrayList<FileTransfer>();
    protected List<FileTransfer> mReceivedInBackgroundFtToBeRead =
            new ArrayList<FileTransfer>();
    
    protected List<String> mReceivedAfterReloadMessage =
            new ArrayList<String>();

    protected RegistrationApi mRegistrationApi = null;
    protected Thread mWorkerThread = ModelImpl.CHAT_WORKER_THREAD;
    protected Handler mWorkerHandler = new Handler(
            ModelImpl.CHAT_WORKER_THREAD.getLooper());

    public static final int FILETRANSFER_ENABLE_OK = 0;
    public static final int FILETRANSFER_DISABLE_REASON_NOT_REGISTER = 1;
    public static final int FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED = 2;
    public static final int FILETRANSFER_DISABLE_REASON_REMOTE = 3;
    public static final int GROUPFILETRANSFER_DISABLE = 4;
    com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status mFileTransferStatus =
        com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING;

    protected SentMessageManager mSentMessageManager = new SentMessageManager();

    /**
     * This class is used manage the sent messages, especially for the
     * time-out ones.
     * .
     */
    protected class SentMessageManager {
        public final String tag = "M0CFC SentMessageManager@" + mTag;

        protected static final long TIME_OUT_MILLI = 30000;
        // private static final long TIME_OUT_MILLI =
        // RcsSettings.getInstance().getDeliveryTimeout();
        protected final Map<String, SendMessageWatcher> mSendingMessage = new
        LinkedHashMap<String, SendMessageWatcher>();

        /**
         * @param sentMessage
         * .
         */
        public void onMessageSent(ISentChatMessage sentMessage) {
            if (null != sentMessage) {
                synchronized (mSendingMessage) {
                    SendMessageWatcher newWatcher = new SendMessageWatcher(
                            sentMessage);
                    mSendingMessage.put(sentMessage.getId(), newWatcher);
                    ModelImpl.TIMER.schedule(newWatcher, TIME_OUT_MILLI);
                    Logger.d(tag, "onMessageSent() add watcher: "
                            + newWatcher + ", current size: "
                            + mSendingMessage.size());
                }
            } else {
                Logger.w(tag, "onMessageSent() sentMessage is null");
            }
        }

        /**
         * @param messageId
         * .
         * @param status
         * .
         * @param timeStamp
         * .
         */
        public void onMessageDelivered(
                final String messageId,
                final com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status status,
                final long timeStamp) {
            Logger.d(tag, "onMessageDelivered() messageId is " + messageId
                    + ", status is " + status + ", time stamp: "
                    + timeStamp);

            if(mChatWindow == null) {
                Logger.e(tag, "Chat window is null for O2O Chat");
                return;
            }
            ISentChatMessage message = (ISentChatMessage) mChatWindow
                    .getSentChatMessage(messageId);

            if (null != message) {
                message.updateStatus(status);
                if (timeStamp > 0) {
                     message.updateDate(new Date(timeStamp));
                }
            } else {
                Logger.e(tag, "onMessageDelivered() message is null");
            }

            if (mSendingMessage.containsKey(messageId)) {
                synchronized (mSendingMessage) {
                    SendMessageWatcher watcher = mSendingMessage
                            .get(messageId);
                    Logger.d(tag, "onMessageDelivered() watcher: "
                            + watcher + ", messageId = " + messageId);
                    if (null != watcher) {
                        watcher.cancel();
                        mSendingMessage.remove(messageId);
                    }
                }
            }

        }

        /**
         * @param messageId
         * .
         * @param status
         * .
         * @param timeStamp
         * .
         */
        public void onMessageSent(
                final String messageId,
                final com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status status,
                final long timeStamp) {
            Logger.d(tag, "onMessageSent() messageId is " + messageId
                    + ", status is " + status + ", time stamp: "
                    + timeStamp);

            if(mChatWindow == null) {
                Logger.e(tag, "Chat window is null for O2O Chat");
                return;
            }
            ISentChatMessage message = (ISentChatMessage) mChatWindow
                    .getSentChatMessage(messageId);

            if (null != message) {
                message.updateStatus(status);
                if (timeStamp > 0) {
                    // message.updateDate(new Date(timeStamp));
                }
            } else {
                Logger.e(tag, "onMessageSent() message is null");
            }

            if (mSendingMessage.containsKey(messageId)) {
                synchronized (mSendingMessage) {
                    SendMessageWatcher watcher = mSendingMessage
                            .get(messageId);
                    Logger.d(tag, "onMessageDelivered() watcher: "
                            + watcher + ", messageId = " + messageId);
                    if (null != watcher) {
                        watcher.cancel();
                        mSendingMessage.remove(messageId);
                    }
                }
            }

        }

        /**
         * .
         */
        public void markSendingMessagesDisplayed() {
            Logger.v(tag, "markSendingMessagesDisplayed() entry");
            synchronized (mSendingMessage) {
                Collection<SendMessageWatcher> values = mSendingMessage
                        .values();
                if (values.size() > 0) {
                    for (SendMessageWatcher watcher : values) {
                        watcher.onMessageDelivered();
                        watcher.cancel();
                    }
                    mSendingMessage.clear();
                }
            }
        }

        /**
         * .
         */
        public void onChatDestroy() {
            Logger.v(tag, "onChatDestroy() entry");
            synchronized (mSendingMessage) {
                Collection<SendMessageWatcher> values = mSendingMessage
                        .values();
                if (values.size() > 0) {
                    for (SendMessageWatcher watcher : values) {
                        watcher.cancel();
                    }
                    mSendingMessage.clear();
                }
            }
        }

        /**
         * @param timerTask
         * .
         */
        protected void onTimeout(SendMessageWatcher timerTask) {
            synchronized (mSendingMessage) {
                Logger.d(TAG, "onTimeout() timerTask: " + timerTask);
                try {
                    CapabilityService capabilityApi = ApiManager
                            .getInstance().getCapabilityApi();
                    if (capabilityApi != null) {
                        capabilityApi
                                .requestContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(mParticipant.getContact()));
                    }
                }  catch (Exception e) {
                    Logger.d(TAG,
                            "onTimeout() getContactCapabilities JoynServiceException1");
                    e.printStackTrace();
                }

                // Core.getInstance().getCapabilityService().
                //requestContactCapabilities(mParticipant.getContact());
                mSendingMessage.remove(timerTask.getId());
                Logger.d(TAG, "onTimeout() current size: "
                        + mSendingMessage.size());
            }
        }

        /**
         * A time-out watcher for one single sent message.
         * .
         */
        protected class SendMessageWatcher extends TimerTask {
            public String tag = "M0CFC SendMessageWatcher:";
            private ISentChatMessage mMessage = null;
            private ISentChatMessage.Status mStatus = com.mediatek.rcse.
            interfaces.ChatView.ISentChatMessage.Status.SENDING;

            public SendMessageWatcher(ISentChatMessage sentChatMessage) {
                mMessage = sentChatMessage;
                tag += getId();
                Logger.v(tag, "Constructor");
            }

            @Override
            public String toString() {
                return tag;
            }

            public String getId() {
                return mMessage.getId();
            }

            /**
             * .
             */
            public void onMessageDelivered() {
                Logger.v(tag, "onMessageDelivered() entry");
                switch (mStatus) {
                case SENDING:
                case DELIVERED:
                    mStatus = com.mediatek.rcse.interfaces.ChatView.
                    ISentChatMessage.Status.DISPLAYED;
                    mMessage.updateStatus(mStatus);
                    Logger.d(tag, "onMessageDelivered() update status: "
                            + mStatus);
                    break;
                case FAILED:
                    Logger.w(tag, "onMessageDelivered() invalid status: "
                            + mStatus);
                    break;
                case DISPLAYED:
                    Logger.w(tag,
                            "onMessageDelivered() do nothing, status: "
                                    + mStatus);
                    break;
                default:
                    Logger.w(tag, "onMessageDelivered() unknown status: "
                            + mStatus);
                    break;
                }
            }

            @Override
            public void run() {
                onTimeout(SendMessageWatcher.this);
                switch (mStatus) {
                case SENDING:
                    mStatus = com.mediatek.rcse.interfaces.ChatView.
                    ISentChatMessage.Status.FAILED;
                    if(!RcsSettings.getInstance().isSupportOP08() && !RcsSettings.getInstance().isSupportOP07()){
                        mMessage.updateStatus(mStatus);
                    }
                    mMessage.updateDate(new Date());
                    Logger.d(tag, "run() update status: " + mStatus);
                    break;
                case FAILED:
                    Logger.w(tag, "run() invalid status: " + mStatus);
                    break;
                case DISPLAYED:
                case DELIVERED:
                    Logger.w(tag, "run() do nothing, status: " + mStatus);
                    break;
                default:
                    Logger.w(tag, "run() unknown status: " + mStatus);
                    break;
                }
            }
        }
    }

    protected FileTransferController mFileTransferController = null;

    /**
     * File Transfer Controller
     *.
     */
    protected class FileTransferController {
        private boolean mRegistrationStatus = true;
        private boolean mRemoteFtCapability = true;
        private boolean mLocalFtCapability = true;
        private boolean mRemoteGroupFtCapability = true;
        private boolean mLocalFtGroupCapability = true;

        private static final String TAG = "M0CFF ChatImpl";

        /**
         * @param status
         * .
         */
        public void setRegistrationStatus(boolean status) {
            Logger.d(TAG, "setRegistrationStatus entry status is " + status);
            mRegistrationStatus = status;
        }

        /**
         * @param status
         * .
         */
        public void setRemoteFtCapability(boolean status) {
            Logger.d(TAG, "setRemoteFtCapability entry status is " + status);
            mRemoteFtCapability = status;
        }

        /**
         * @param status
         * .
         */
        public void setLocalFtCapability(boolean status) {
            Logger.d(TAG, "setLocalFtCapability entry status is " + status);
            mLocalFtCapability = status;
        }

        /**
         * @param status
         * .
         */
        public void setLocalGroupFtCapability(boolean status) {
            Logger.i(TAG, "setGroupLocalFtCapability entry status is "
                    + status);
            mLocalFtGroupCapability = status;
        }

        /**
         * @param status
         * .
         */
        public void setRemoteGroupFtCapability(boolean status) {
            Logger.i(TAG, "setGroupRemoteFtCapability entry status is "
                    + status);
            mRemoteGroupFtCapability = status;
        }

        /**
         * .
         */
        public void controlGroupFileTransferIconStatus() {
            Logger.i(TAG,
                    "controlGroupFileTransferIconStatus entry: mChatWindow = "
                            + mChatWindow + ", mRegistrationStatus "
                            + mRegistrationStatus);
            Logger.d(TAG,
                    "controlGroupFileTransferIconStatus mLocalFtGroupCapability:"
                            + mLocalFtGroupCapability
                            + "mRemoteGroupFtCapability"
                            + mRemoteGroupFtCapability);
            if (mRegistrationStatus && mRemoteGroupFtCapability
                    && mLocalFtGroupCapability) {
                if (mChatWindow != null) {
                    ((IGroupChatWindow) mChatWindow)
                            .setFileTransferEnable(FILETRANSFER_ENABLE_OK);
                    Logger.d(TAG,
                            "controlGroupFileTransferIconStatus reason is" +
                            " FILETRANSFER_ENABLE_OK");
                }
            } else {
                if (mChatWindow != null) {
                    ((IGroupChatWindow) mChatWindow)
                            .setFileTransferEnable(GROUPFILETRANSFER_DISABLE);
                    Logger.i(TAG,
                            "controlGrouFileTransferIconStatus Disable");
                }
            }
        }

        /**
         * control the file transfer icon status caused by registration,
         * self capability, remote capability. All conditions are satisfied,
         * the icon works well.
         * .
         */
        public void controlFileTransferIconStatus() {
            Logger.i(TAG,
                    "controlFileTransferIconStatus entry: mChatWindow = "
                            + mChatWindow + ", mRegistrationStatus "
                            + mRegistrationStatus);
            if (mRegistrationStatus && mRemoteFtCapability
                    && mLocalFtCapability) {
                if (mChatWindow != null) {
                    ((IOne2OneChatWindow) mChatWindow)
                            .setFileTransferEnable(FILETRANSFER_ENABLE_OK);
                    Logger.i(TAG,
                            "controlFileTransferIconStatus reason is" +
                            " FILETRANSFER_ENABLE_OK");
                }
            } else {
                if (!mRegistrationStatus) {
                    if (mChatWindow != null) {
                        ((IOne2OneChatWindow) mChatWindow)
                        .setFileTransferEnable(FILETRANSFER_DISABLE_REASON_NOT_REGISTER);
                        Logger.i(
                                TAG,
                      "controlFileTransferIconStatus reason is FILETRANSFER_" +
                      "DISABLE_REASON_NOT_REGISTER");
                    }

                } else if (!mLocalFtCapability) {
                    if (mChatWindow != null) {
                        ((IOne2OneChatWindow) mChatWindow)
                    .setFileTransferEnable(FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
                        Logger.i(
                                TAG,
                                "controlFileTransferIconStatus()"
                               + " reason is FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED");
                    }

                } else if (!mRemoteFtCapability) {
                    if (mChatWindow != null) {
                        ((IOne2OneChatWindow) mChatWindow)
                                .setFileTransferEnable(FILETRANSFER_DISABLE_REASON_REMOTE);
                        Logger.i(
                                TAG,
                                "controlFileTransferIconStatus()"
                               + " reason is FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED");
                    }
                }
            }
        }
    }

    /**
     * This class is used to manage the whole receive file transfers.
     *.
     */
    protected class ReceiveFileTransferManager {
        private static final String TAG = "M0CFF ReceiveFileTransferManager";

        private CopyOnWriteArrayList<ReceiveFileTransfer> mActiveList = new
        CopyOnWriteArrayList<ReceiveFileTransfer>();

        /**
         * Handle a new file transfer invitation.
         * .
         * @param fileTransferObject
         * .
         * @param isAutoAccept
         * .
         * @param isGroup
         * .
         */
        public synchronized void addReceiveFileTransfer(
                FileTransfer fileTransferObject, boolean isAutoAccept,
                boolean isGroup) {
            Logger.d(TAG,
                    "addReceiveFileTransfer() entry, fileTransferObject = "
                            + fileTransferObject);
            if (null != fileTransferObject) {
                ReceiveFileTransfer receiveFileTransfer = new ReceiveFileTransfer(
                        fileTransferObject, isAutoAccept, isGroup);
                mActiveList.add(receiveFileTransfer);
            }
        }

        /**
         * @return.
         */
        public ReceiveFileTransfer getReceiveTransfer() {
            ReceiveFileTransfer resumeFile = null;
            Logger.d(TAG, "getReceiveTransfer 1");
            try {
                resumeFile = mActiveList.get(0);
            } catch (NullPointerException e) {
                Logger.d(TAG, "getReceiveTransfer exception");
            }
            catch (ArrayIndexOutOfBoundsException e) {
                Logger.d(TAG, "getReceiveTransfer exception");
            }
            return resumeFile;
        }

        /**
         * @param receiveFileTransfer
         * .
         */
        public synchronized void removeReceiveFileTransfer(
                ReceiveFileTransfer receiveFileTransfer) {
            Logger.d(TAG,
                    "removeReceiveFileTransfer() entry, receiveFileTransfer = "
                            + receiveFileTransfer);
            if (null != receiveFileTransfer) {
                mActiveList.remove(receiveFileTransfer);
                Logger.d(TAG,
                        "removeReceiveFileTransfer() the file transfer with" +
                        " receiveFileTransfer: "
                                + receiveFileTransfer);
            }

        }

        /**
         * cancels Received FileTransfer.
         */
        public synchronized void cancelReceiveFileTransfer() {
            Logger.d(TAG, "cancelReceiveFileTransfer entry");
            ArrayList<ReceiveFileTransfer> tempList = new ArrayList<ReceiveFileTransfer>(
                    mActiveList);
            int size = tempList.size();
            for (int i = 0; i < size; i++) {
                ReceiveFileTransfer receiveFileTransfer = tempList.get(i);
                if (null != receiveFileTransfer) {
                    receiveFileTransfer.cancelFileTransfer();
                }
            }
        }

        /**
         * Search an existing file transfer in the receive list.
         * .
         * @param targetTag
         * .
         * @return.
         */
        public synchronized ReceiveFileTransfer findFileTransferByTag(
                Object targetTag) {
            if (null != mActiveList && null != targetTag) {
                for (ReceiveFileTransfer receiveFileTransfer : mActiveList) {
                    Object fileTransferTag = receiveFileTransfer.mFileTransferTag;
                    if (targetTag.equals(fileTransferTag)) {
                        Logger.d(TAG,
                                "findFileTransferByTag() the file transfer with targetTag "
                                        + targetTag + " found");
                        return receiveFileTransfer;
                    }
                }
                Logger.d(TAG,
                        "findFileTransferByTag() not found targetTag "
                                + targetTag);
                return null;
            } else {
                Logger.e(TAG, "findFileTransferByTag(), targetTag is "
                        + targetTag);
                return null;
            }
        }
    }

    protected ReceiveFileTransferManager mReceiveFileTransferManager = new
    ReceiveFileTransferManager();

    /**
     * This class describe one single in-coming file transfer, and control
     * the status itself.
     * .
     */
    /**
     * @author mtk33241
     *
     */
    protected class ReceiveFileTransfer {
        private static final String TAG = "M0CFF ReceiveFileTransfer";
        protected Object mFileTransferTag = UUID.randomUUID();

        protected FileStruct mFileStruct = null;

        protected IFileTransfer mFileTransfer = null;

        protected FileTransfer mFileTransferObject = null;

        private long mTotalSize = 0;
        private long mCurrentSize = 0;
        boolean mNeedtoAutoresume = true;

        //protected FileTransferListener mFileTransferListener = null;

        /**
         * @param fileTransferObject.
         * .
         * @param isAutoAccept
         * .
         * @param isGroup
         * .
         */
        public ReceiveFileTransfer(FileTransfer fileTransferObject,
                boolean isAutoAccept, boolean isGroup) {
            if (null != fileTransferObject) {
                Logger.d(TAG,
                        "ReceiveFileTransfer() constructor FileTransfer is "
                                + fileTransferObject);
                handleFileTransferInvitation(fileTransferObject,
                        isAutoAccept, isGroup);
            }
        }

        /**
         * @param fileTransferObject
         * .
         * @param isAutoAccept
         * .
         * @param isGroupChat
         * .
         */
        protected void handleFileTransferInvitation(
                FileTransfer fileTransferObject, boolean isAutoAccept,
                boolean isGroupChat) {
            Logger.d(TAG, "handleFileTransferInvitation() entry "
                    + isAutoAccept);
            if (fileTransferObject == null) {
                Logger.e(TAG,
                        "handleFileTransferInvitation, fileTransferSession is null!");
                return;
            }
            try {
                mFileTransferObject = fileTransferObject;
                mFileTransferTag = mFileTransferObject.getTransferId();
                Logger.d(TAG, "handleFileTransferInvitation() transferid add map" + mFileTransferObject.getTransferId());
                ModelImpl.sFileTransferImplMap.put((String)mFileTransferTag, this);
                //mFileTransferListener = new FileTransferReceiverListener();
                //mFileTransferObject.addEventListener(mFileTransferListener);
                mFileStruct = FileStruct.from(mFileTransferObject, null);
            } catch (Exception e) {
                e.printStackTrace();
            }

            if (mFileStruct != null) {
                if (isGroupChat) {
                    mFileTransfer = ((IGroupChatWindow) mChatWindow)
                            .addReceivedFileTransfer(mFileStruct,
                                    isAutoAccept, !mIsInBackground);
                    if (isAutoAccept) {
                        mFileTransferStatus = com.mediatek.rcse.interfaces.ChatView.
                        IFileTransfer.Status.TRANSFERING;
                    } else {
                        mFileTransferStatus = com.mediatek.rcse.interfaces.ChatView.
                        IFileTransfer.Status.WAITING;
                    }
                } else {
                    mFileTransfer = ((IOne2OneChatWindow) mChatWindow)
                            .addReceivedFileTransfer(mFileStruct,
                                    isAutoAccept);
                    if (isAutoAccept) {
                        mFileTransferStatus = com.mediatek.rcse.interfaces.ChatView.
                        IFileTransfer.Status.TRANSFERING;
                    } else {
                        mFileTransferStatus = com.mediatek.rcse.interfaces.ChatView.
                        IFileTransfer.Status.WAITING;
                    }
                }

            }
        }

        /**
         * Accepts File Transfer Invitation from remote.
         */
        protected void acceptFileTransferInvitation() {
            if (mFileTransferObject != null) {
                try {
                    // Received file size in byte
                    long receivedFileSize = mFileTransferObject
                            .getFileSize();
                    long currentStorageSize = Utils.getFreeStorageSize();
                    Logger.d(TAG, "receivedFileSize = " + receivedFileSize
                            + "/currentStorageSize = " + currentStorageSize);
                    if (currentStorageSize > 0) {
                        if (receivedFileSize <= currentStorageSize) {
                            mFileTransferObject.acceptInvitation();
                            mFileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.
                                            IFileTransfer.Status.TRANSFERING);
                            mFileTransferStatus = com.mediatek.rcse.interfaces.ChatView.
                            IFileTransfer.Status.TRANSFERING;
                        } else {
                            mFileTransferObject.rejectInvitation();
                            mFileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.
                                            IFileTransfer.Status.REJECTED);
                            mFileTransferStatus = com.mediatek.rcse.interfaces.ChatView.
                            IFileTransfer.Status.REJECTED;
                            new Handler(Looper.getMainLooper())
                                .post(new Runnable() {
                                    @Override
                                    public void run() {
                                    Context context = ApiManager
                                            .getInstance()
                                            .getContext();
                                    String strToast = context
                              .getString(R.string.rcse_no_enough_storage_for_file_transfer);
                                    Toast.makeText(context,
                                            strToast,
                                            Toast.LENGTH_LONG)
                                            .show();
                                }
                                });
                        }
                    } else {
                        mFileTransferObject.rejectInvitation();
                        mFileTransfer
                                .setStatus(com.mediatek.rcse.interfaces.ChatView.
                                        IFileTransfer.Status.REJECTED);
                        new Handler(Looper.getMainLooper())
                            .post(new Runnable() {
                                @Override
                                public void run() {
                                Context context = ApiManager
                                        .getInstance().getContext();
                                String strToast = context.
                            getString(R.string.rcse_no_external_storage_for_file_transfer);
                                Toast.makeText(context, strToast,
                                        Toast.LENGTH_LONG).show();
                                }
                            });
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                mFileTransfer
                        .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.
                                Status.FAILED);
                Logger.d(TAG,
                        "acceptFileTransferInvitation(), mFileTransferObject is null");
            }
        }

        /**
         * Reject FileTransfer Invitation from remote.
         * .
         */
        protected void rejectFileTransferInvitation() {
            try {
                if (mFileTransferObject != null) {
                    /*if (mFileTransferListener != null) {
                        mFileTransferObject
                                .removeEventListener(mFileTransferListener);
                        mFileTransferListener = null;
                    } else {
                        Logger.w(TAG,
                                "rejectFileTransferInvitation(), " +
                                "mFileTransferReceiverListener is null!");
                    }*/
                    mFileTransferObject.rejectInvitation();
                    if (mFileTransfer != null) {
                        mFileTransfer
                                .setStatus(com.mediatek.rcse.interfaces.ChatView.
                                        IFileTransfer.Status.REJECTED);
                        mFileTransferStatus = com.mediatek.rcse.interfaces.ChatView.
                        IFileTransfer.Status.REJECTED;
                    }
                } else {
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.
                                    Status.FAILED);
                    mFileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.
                    Status.FAILED;
                    Logger.e(TAG,
                            "rejectFileTransferInvitation(), mFileTransferObject is null!");
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            mReceiveFileTransferManager.removeReceiveFileTransfer(this);
        }

        /**
         * Cancels File Transfer .
         * .
         */
        protected void cancelFileTransfer() {
            if (null != mFileTransferObject) {
                try {
                    mFileTransferObject.abortTransfer();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            onFileTransferCancel();
        }

        /**
         * Handle when Pause Transfer.
         * .
         */
        protected void onPauseReceiveTransfer() {
            Logger.v(TAG, "onPauseReceiveTransfer 1");
            if (null != mFileTransferObject) {
                try {
                    Logger.v(TAG, "onPauseReceiveTransfer 1");
                    mFileTransferObject.pauseTransfer();
                } catch (Exception e) {
                    Logger.v(TAG, "onPauseReceiveTransfer exception" + e);
                    e.printStackTrace();
                }
            }
            // onFileTransferCancel();
        }

        /**
         * Handle when resume File transfer.
         * .
         */
        protected void onResumeReceiveTransfer() {
            Logger.v(TAG, "onResumeReceiveTransfer");
            if (null != mFileTransferObject) {
                try {
                    Logger.v(TAG, "onResumeReceiveTransfer 1");
                    // if(mFileTransferObject.isSessionPaused()){ //TODo
                    // check this
                    // Logger.v(TAG,"onResumeReceiveTransfer 2");
                    //mFileTransferObject.resumeTransfer();
                    FileTransferService fileTransferService = ApiManager.getInstance().getFileTransferApi();
                    if(fileTransferService != null){
                        fileTransferService.resumeFileTransfer(mFileTransferObject.getTransferId());
                    }
                    mFileTransfer
                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.
                            Status.TRANSFERING);
                    
                    // }
                } catch (Exception e) {
                    Logger.v(TAG, "onResumeReceiveTransfer exception" + e);
                    e.printStackTrace();
                }
            }
            // onFileTransferCancel();
        }

        /**
         * @param dataSentCompleted
         * .
         */
        protected void onFileTransferFailed(boolean dataSentCompleted) {
            Logger.v(TAG,
                    "onFileTransferFailed() entry : mFileTransferListener = "
                            + "mFileTransfer = "
                            + mFileTransfer + ", mFileTransferSession = "
                            + mFileTransferObject);
            // notify file transfer canceled
            if (mFileTransfer != null) {
                if (dataSentCompleted) {
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.
                                    Status.FINISHED);
                } else {
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.
                                    Status.CANCELED);
                }
            }
            /*if (mFileTransferListener != null) {
                if (mFileTransferObject != null) {
                    try {
                        mFileTransferObject
                                .removeEventListener(mFileTransferListener);
                    } catch (JoynServiceException e) {
                        e.printStackTrace();
                    }
                }
            }*/

            mReceiveFileTransferManager.removeReceiveFileTransfer(this);
        }

        /**
         *  file transfer in progress was canceled by remote..
         *  .
         */
        protected void onFileTransferCanceled() {
            Logger.v(TAG,
                    "canceledFileTransfer() entry : mFileTransferListener = "
                            + "mFileTransfer = "
                            + mFileTransfer + ", mFileTransferObject = "
                            + mFileTransferObject);
            // notify file transfer canceled
            if (mFileTransfer != null) {
                mFileTransfer
                        .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.
                                Status.CANCELED);
                mFileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.
                Status.CANCELED;
            }
            /*if (mFileTransferListener != null) {
                if (mFileTransferObject != null) {
                    try {
                        mFileTransferObject
                                .removeEventListener(mFileTransferListener);
                    } catch (JoynServiceException e) {
                        e.printStackTrace();
                    }
                }
            }*/

            mReceiveFileTransferManager.removeReceiveFileTransfer(this);
        }

        /**
         *  file transfer in progress was canceled by remote..
         *  .
         */
        protected void onFileInterruptedResume() {
            //(ModelImpl(mOutGoingFileTransferManager)).mResumableList.add(this);
            Logger.v(TAG,"On Interruptedresume");
            ResumeFileTransfer resumeFileTransfer = new ResumeFileTransfer(mFileTransfer,
                    mParticipant, mFileTransferTag, mFileStruct,true);
            
            Logger.v(TAG,
                    "onFileInterruptedResume() entry : mFileTransferListener = "
                            + "mFileTransfer = "
                            + mFileTransfer + ", mFileTransferObject = "
                            + mFileTransferObject);           
            mReceiveFileTransferManager.removeReceiveFileTransfer(this);
        }

        /**
         *  Handle when file transfer failed.
         *  .
         */
        protected void onFileTransferFailed() {
            Logger.v(TAG,
                    "TIMEOUT failedFileTransfer() entry : mFileTransferListener = "
                            + "mFileTransfer = "
                            + mFileTransfer + ", mFileTransferObject = "
                            + mFileTransferObject);
            // notify file transfer failed on timeout
            if (mFileTransfer != null) {
                if (!(mFileTransferStatus == com.mediatek.rcse.interfaces.ChatView.
                        IFileTransfer.Status.CANCELED)) {
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.
                                    Status.FAILED);
                }
            }
            /*if (mFileTransferListener != null) {
                if (mFileTransferObject != null) {
                    try {
                        mFileTransferObject
                                .removeEventListener(mFileTransferListener);
                    } catch (JoynServiceException e) {
                        e.printStackTrace();
                    }
                }
            }*/

            mReceiveFileTransferManager.removeReceiveFileTransfer(this);
        }

        /**
         * You cancel a file transfer..
         * .
         */
        protected void onFileTransferCancel() {
            Logger.v(TAG,
                    "canceledFileTransfer() entry : mFileTransferListener = "
                            + ", mFileTransfer = "
                            + mFileTransfer + ", mFileTransferObject= "
                            + mFileTransferObject);
            // notify file transfer cancel
            if (mFileTransfer != null) {
                mFileTransfer
                        .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.
                                CANCEL);
            }
            /*if (mFileTransferListener != null) {
                if (mFileTransferObject != null) {
                    try {
                        mFileTransferObject
                                .removeEventListener(mFileTransferListener);
                    } catch (JoynServiceException e) {
                        e.printStackTrace();
                    }
                }
            }*/

            mReceiveFileTransferManager.removeReceiveFileTransfer(this);
        }

        /*private class FileTransferReceiverListener extends
        OneToOneFileTransferListener {}*/
            /**
             * Callback called when the file transfer is started.
             * .
             */
            public void onTransferStarted() {
                // notify that this chat have a file transfer
                Logger.v(TAG, "onTransferStarted() entry");
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
             * Callback called when the file transfer has been aborted.
             * .
             */
            public void onTransferAborted(int error) {
                    Logger.e(TAG,
                        "onTransferAborted, File transfer onTransferAborted error:" + error);
                if(error == FileTransfer.ReasonCode.ABORTED_BY_USER.toInt() ||
                   error == FileTransfer.ReasonCode.REJECTED_BY_USER.toInt() ){
                             return;
                }
                if (mFileTransfer != null) {
                    checkCapabilities();
                }
                mFileTransferObject = null;
                onFileInterruptedResume();
                onFileTransferFailed();
                
                // Send text message
                Thread t = new Thread() {
                    public void run() {
                        Logger.d(TAG, "handleFileResumeAfterStatusChange,before resume");
                        try {
                            Thread.sleep(5000);
                        } catch (InterruptedException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }
                        Logger.d(TAG, "handleFileResumeAfterStatusChange, after resume");
                        //((ModelImpl)ModelImpl.getInstance()).handleFileResumeAfterStatusChange();
                        //((ModelImpl)ModelImpl.getInstance()).mOutGoingFileTransferManager.mResumableList.remove(0);
                    }
                };
                t.start();                            
            }

            /**
             * Callback called when the transfer has failed.
             * .
             * @param error
             *            Error.
             */
            public void onTransferError(int error) {
                Logger.d(TAG, "onTransferError, error is :" + error);
            /*if (error == FileSharingError.SESSION_INITIATION_DECLINED) {
                    onFileTransferCanceled();
                    mFileTransferObject = null;
                } else if (error == FileSharingError.MEDIA_SAVING_FAILED) {
                    mFileStruct.mFilePath = ReceivedFileTransferItemBinder.FILEPATH_NO_SPACE;
                    onFileTransferCancel();
                    mFileTransferObject = null;
                } else if (error == FileSharingError.UNEXPECTED_EXCEPTION) {
                    onFileTransferCanceled();
                    onFileInterruptedResume();
                    mFileTransferObject = null;
                } else if (error == FileSharingError.SESSION_INITIATION_FAILED) {
                    boolean dataSentCompleted = true;
                    if (mTotalSize == 0) {
                        dataSentCompleted = false;
                    } else {
                        dataSentCompleted = (mTotalSize == mCurrentSize ? true
                                : false);
                    }
                    onFileTransferFailed(dataSentCompleted);
                    onFileInterruptedResume();
                    mFileTransferObject = null;
                } else {
                    onFileTransferCanceled();
                    onFileInterruptedResume();
                    mFileTransferObject = null;
            }*/
                if(error == FileTransfer.ReasonCode.REJECTED_MEDIA_FAILED.toInt()){
                    
                    
                    Logger.d(TAG, "onTransferError, mNeedtoresume:" + mNeedtoAutoresume);
                    if(mNeedtoAutoresume == true) {
                        onResumeReceiveTransfer();
                        mNeedtoAutoresume = false;
                    }else {
                        Logger.d(TAG, "onTransferError, start the timer");
                        final Timer timer = new Timer();
                        timer.schedule(new TimerTask() {
                            @Override
                            public void run() {
                                Logger.d(TAG, "onTransferError, timeout");
                                   onResumeReceiveTransfer();
                                   timer.cancel();
                                }
    
                         }, 10*60*1000);
                    if (mFileTransfer != null) {
                        mFileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.
                                            Status.FAILED);
                        }
                    }
                    
                } else {
            onFileTransferFailed();
            mFileTransferObject = null;
            }
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
                Logger.d(TAG,
                        "handleTransferProgress() entry: currentSize = "
                                + currentSize + ", totalSize = "
                                + totalSize + ", mFileTransfer = "
                                + mFileTransfer);
                if (mFileTransfer != null) {
                    mFileTransfer.setProgress(currentSize);
                }
                mTotalSize = totalSize;
                mCurrentSize = currentSize;
            }

            /**
             * Callback called when the file has been transferred.
             * .
             * @param filename
             *            Filename including the path of the transferred file.
             */
            public void onFileTransferred(String filename) {
                Logger.d(TAG, "onFileTransferred, filename is :" + filename
                        + ", mFileTransfer = " + mFileTransfer + "tag: "
                        + mFileStruct.mFileTransferTag);
                if (RichMessagingDataProvider.getInstance() == null) {
                    RichMessagingDataProvider.createInstance(MediatekFactory
                            .getApplicationContext());
                }
                RichMessagingDataProvider.getInstance()
                        .updateFileTransferUrl(
                                mFileStruct.mFileTransferTag.toString(),
                                filename);
                RichMessagingDataProvider.getInstance()
                        .updateFileTransferStatus(
                                mFileStruct.mFileTransferTag.toString(), FileTransfer.State.TRANSFERRED.toInt());
                if (mFileTransfer != null) {
                    try {
                        mFileTransfer.setFilePath(filename);
                    } catch (NullPointerException e) {
                        Logger.d(TAG, "onFileTransferred exception");
                    }
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.
                                    Status.FINISHED);
                    mFileTransfer = null;
                }
                if (mIsInBackground) {
                    addUnreadFt(mFileTransferObject);
                }else{
                    markNewFtAsRead(mFileTransferObject);
                }
                ReceiveFileTransfer receiveFileTransfer = mReceiveFileTransferManager
                        .findFileTransferByTag(mFileTransferTag);
                if (null != receiveFileTransfer) {
                    mReceiveFileTransferManager
                            .removeReceiveFileTransfer(receiveFileTransfer);
                }
            }

    

    }

    /**
     * Clear all the messages in chat Window and the latest message in chat
     * list.
     * .
     */
    public void clearChatWindowAndList() {
        Logger.d(TAG, "clearChatWindowAndList() entry");
        mChatWindow.removeAllMessages();
        Logger.d(TAG, "clearChatWindowAndList() exit");
    }

    /**
     * Set chat window for this chat.
     * .
     * @param chatWindow
     *            The chat window to be set.
     */
    public void setChatWindow(IChatWindow chatWindow) {
        Logger.d(TAG, "setChatWindow entry");
        mChatWindow = chatWindow;
    }

    /**
     * Add the unread message of this chat.
     * .
     * @param message
     *            The unread message to add.
     */
    protected void addUnreadMessage(ChatMessage message) {
        //if (message.isDisplayedReportRequested()) {
        if (true) {
            Logger.d(TAG, "mReceivedInBackgroundToBeDisplayed = "
                    + mReceivedInBackgroundToBeDisplayed);
            if (mReceivedInBackgroundToBeDisplayed != null) {
                mReceivedInBackgroundToBeDisplayed.add(message);
                if (mTag instanceof UUID) {
                    ParcelUuid parcelUuid = new ParcelUuid((UUID) mTag);
                    UnreadMessagesContainer.getInstance().add(parcelUuid);
                } else {
                    UnreadMessagesContainer.getInstance().add(
                            (ParcelUuid) mTag);
                }
                UnreadMessagesContainer.getInstance()
                        .loadLatestUnreadMessage();
            }
           Logger.d(TAG, "mReceivedInBackgroundToBeRead = "
                    + mReceivedInBackgroundToBeRead);
            if (mReceivedInBackgroundToBeRead != null) {
                mReceivedInBackgroundToBeRead.add(message);
                if (mTag instanceof UUID) {
                    ParcelUuid parcelUuid = new ParcelUuid((UUID) mTag);
                    UnreadMessagesContainer.getInstance().add(parcelUuid);
                } else {
                    UnreadMessagesContainer.getInstance().add(
                            (ParcelUuid) mTag);
                }
                UnreadMessagesContainer.getInstance()
                        .loadLatestUnreadMessage();
            }
        } else {
            Logger.d(TAG, "mReceivedInBackgroundToBeRead = "
                    + mReceivedInBackgroundToBeRead);
            if (mReceivedInBackgroundToBeRead != null) {
                mReceivedInBackgroundToBeRead.add(message);
                if (mTag instanceof UUID) {
                    ParcelUuid parcelUuid = new ParcelUuid((UUID) mTag);
                    UnreadMessagesContainer.getInstance().add(parcelUuid);
                } else {
                    UnreadMessagesContainer.getInstance().add(
                            (ParcelUuid) mTag);
                }
                UnreadMessagesContainer.getInstance()
                        .loadLatestUnreadMessage();
            }
        }
    }


	 /**
     * Add the unread message of this chat
     * 
     * @param message The unread message to add
     */
    protected void addUnreadFt(FileTransfer fileObject) {
        /*if (fileObject.isDisplayedReportRequested()) {
            Logger.d(TAG, "mReceivedInBackgroundFtToBeDisplayed = "
                    + mReceivedInBackgroundFtToBeDisplayed);
            if (mReceivedInBackgroundFtToBeDisplayed != null) {
                mReceivedInBackgroundFtToBeDisplayed.add(message);
                if (mTag instanceof UUID) {
                    ParcelUuid parcelUuid = new ParcelUuid((UUID) mTag);
                    UnreadMessagesContainer.getInstance().add(parcelUuid);
                } else {
                    UnreadMessagesContainer.getInstance().add((ParcelUuid) mTag);
                }
                UnreadMessagesContainer.getInstance().loadLatestUnreadMessage();
            }
        } else {*/
            Logger.d(TAG, "mReceivedInBackgroundFtToBeRead = "
                    + mReceivedInBackgroundFtToBeRead);
            if (mReceivedInBackgroundFtToBeRead != null) {
                mReceivedInBackgroundFtToBeRead.add(fileObject);   
            }
        //}
    }

	 /**
     * Get unread messages of this chat.
     * 
     * @return The unread messages.
     */
    public List<FileTransfer> getUnreadFt() {
        if (mReceivedInBackgroundFtToBeDisplayed != null
                && mReceivedInBackgroundFtToBeDisplayed.size() > 0) {
            return mReceivedInBackgroundFtToBeDisplayed;
        } else {
            return mReceivedInBackgroundFtToBeRead;
        }
    }

    /**
     * Clear all the unread message of this chat
     */
    protected void clearUnreadFt() {
        Logger.v(TAG,
                "clearUnreadFt(): mReceivedInBackgroundFtToBeDisplayed = "
                        + mReceivedInBackgroundFtToBeDisplayed
                        + ", mReceivedInBackgroundFtToBeRead = "
                        + mReceivedInBackgroundFtToBeRead);
        if (null != mReceivedInBackgroundFtToBeDisplayed) {
            mReceivedInBackgroundFtToBeDisplayed.clear();
        } 
        if (null != mReceivedInBackgroundFtToBeRead) {
            mReceivedInBackgroundFtToBeRead.clear();
        }
    }

    /**
     * Get unread messages of this chat.
     * .
     * @return The unread messages.
     */
    public List<ChatMessage> getUnreadMessages() {
        if (mReceivedInBackgroundToBeDisplayed != null
                && mReceivedInBackgroundToBeDisplayed.size() > 0) {
            return mReceivedInBackgroundToBeDisplayed;
        } else {
            return mReceivedInBackgroundToBeRead;
        }
    }

    /**
     * Clear all the unread message of this chat.
     * .
     */
    protected void clearUnreadMessage() {
        Logger.v(TAG,
                "clearUnreadMessage(): mReceivedInBackgroundToBeDisplayed = "
                        + mReceivedInBackgroundToBeDisplayed
                        + ", mReceivedInBackgroundToBeRead = "
                        + mReceivedInBackgroundToBeRead);
        if (null != mReceivedInBackgroundToBeDisplayed) {
            mReceivedInBackgroundToBeDisplayed.clear();
            UnreadMessagesContainer.getInstance().remove((ParcelUuid) mTag);
            UnreadMessagesContainer.getInstance().loadLatestUnreadMessage();
        }
        if (null != mReceivedInBackgroundToBeRead) {
            mReceivedInBackgroundToBeRead.clear();
            UnreadMessagesContainer.getInstance().remove((ParcelUuid) mTag);
            UnreadMessagesContainer.getInstance().loadLatestUnreadMessage();
        }
    }      

    /**
     * @param tag
     * .
     */
    protected ChatImpl(Object tag) {
        mTag = tag;
        // register IRegistrationStatusListener
        ApiManager apiManager = ApiManager.getInstance();
        Logger.v(TAG, "ChatImpl() entry: apiManager = " + apiManager);
        if (null != apiManager) {
            mRegistrationApi = ApiManager.getInstance()
                    .getRegistrationApi();
            Logger.v(TAG, "mRegistrationApi = " + mRegistrationApi);
            if (mRegistrationApi != null) {
                mRegistrationApi.addRegistrationStatusListener(this);
            }
            CapabilityService capabilityApi = ApiManager.getInstance()
                    .getCapabilityApi();
            Logger.v(TAG, "capabilityApi = " + capabilityApi);
            if (capabilityApi != null) {
                try {
                    capabilityApi.addCapabilitiesListener(this);
                } catch (JoynServiceException e) {
                    Logger.d(TAG,
                            "ChatImpl() getContactCapabilities JoynServiceException");
                }
            }
        }
    }

    /**
     * Get the chat tag of current chat.
     * .
     * @return The chat tag of current chat.
     */
    public Object getChatTag() {
        return mTag;
    }

    /**
     * .
     */
    protected void onPause() {
        Logger.v(TAG, "onPause() entry, tag: " + mTag);
        mIsInBackground = true;
    }

    /**
     * .
     */
    protected void onResume() {
        try{
            Logger.v(TAG, "onResume() entry, tag: " + mTag);
            mIsInBackground = false;
            if (mChatWindow != null) {
                mChatWindow.updateAllMsgAsRead();
                if (mParticipant != null) {
                    mChatWindow.updateAllMsgAsReadForContact(mParticipant);
                }
            }
            markUnreadMessageDisplayed();
            markUnreadFtDisplayed();
            markReloadDisplayed();
            // loadChatMessages(One2OneChat.LOAD_ZERO_SHOW_HEADER);
        } catch(Exception e){
            e.printStackTrace();
        }
    }

    /**
     * .
     */
    protected synchronized void onDestroy() {
        this.queryCapabilities();
        ApiManager apiManager = ApiManager.getInstance();
        Logger.v(TAG, "onDestroy() apiManager = " + apiManager);
        if (null != apiManager) {
            CapabilityService capabilityApi = ApiManager.getInstance()
                    .getCapabilityApi();
            Logger.v(TAG, "onDestroy() capabilityApi = " + capabilityApi);
            if (capabilityApi != null) {
                try {
                    capabilityApi.removeCapabilitiesListener(this);
                } catch (JoynServiceException e) {
                    Logger.d(TAG,
                            "addChat() onDestroy JoynServiceException");
                }
            }
        }
        Logger.v(TAG, "onDestroy() mRegistrationApi = " + mRegistrationApi
                + ",mCurrentGroupChatImpl = " + mCurrentGroupChatImpl.get());
        if (mRegistrationApi != null) {
            mRegistrationApi.removeRegistrationStatusListener(this);
        }
        if (mCurrentGroupChatImpl.get() != null) {
            try {
                terminateGroupChatImpl();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * Return the IChatWindow.
     * .
     * @return IChatWindow.
     */
    IChatWindow getChatWindow() {
        return mChatWindow;
    }

    /**
     * @param msg
     * .
     */
    protected void markMessageAsDisplayed(ChatMessage msg) {
        Logger.d(TAG, "markMessageAsDisplayed() entry");
        if (msg == null) {
            Logger.d(TAG, "markMessageAsDisplayed(),msg is null");
            return;
        }
        try {
            GroupChat tmpChatImpl = mCurrentGroupChatImpl.get();
            // ChatService messagingApi =
            // ApiManager.getInstance().getChatApi();
            SharedPreferences settings = PreferenceManager
                    .getDefaultSharedPreferences(ApiManager.getInstance()
                            .getContext());
            boolean isSendReadReceiptChecked = settings.getBoolean(
                    SettingsFragment.RCS_SEND_READ_RECEIPT, true);
            Logger.d(TAG,
                    "markMessageAsDisplayed() ,the value of isSendReadReceiptChecked is "
                            + isSendReadReceiptChecked);

	
			 /*if (ApiManager.getInstance() != null) {
            //events = ApiManager.getInstance().getEventsLogApi(); //TODo check this
                if(RichMessagingDataProvider.getInstance() == null){
        			RichMessagingDataProvider.createInstance(MediatekFactory.getApplicationContext());
        		}	
                RichMessagingDataProvider.getInstance().markChatMessageAsRead(msg.getId(), true);
            }*/
            /*if (tmpChatImpl == null) {
                Logger.d(TAG,
                        "markMessageAsDisplayed() ,tmpChatSession is null");
                return;
            }*/
            Logger.d(TAG,
                    "markMessageAsDisplayed() ,the value of isSendReadReceiptChecked is "
                            + isSendReadReceiptChecked);
            if (isSendReadReceiptChecked) {
                Logger.v(TAG,
                        "markMessageAsDisplayed(),send displayed message by msrp message");
                //tmpChatImpl.sendDisplayedDeliveryReport(msg.getId());
                ChatService chatService = ApiManager.getInstance().getChatApi();
                if(chatService != null){
                    chatService.markMessageAsRead(msg.getId());
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        Logger.d(TAG, "markMessageAsDisplayed() exit");
    }

    /**
     * @param msg
     * .
     */
    protected void markMessageAsRead(ChatMessage msg) {
        Logger.i(TAG, "markMessageAsRead() entry");
       // EventsLogApi events = null;
        /*if (ApiManager.getInstance() != null) {
            //events = ApiManager.getInstance().getEventsLogApi(); //TODo check this
            if(RichMessagingDataProvider.getInstance() == null){
    			RichMessagingDataProvider.createInstance(MediatekFactory.getApplicationContext());
    		}	
			try {
             RichMessagingDataProvider.getInstance().markChatMessageAsRead(msg.getId(), true);
			} catch(UnsupportedOperationException e) {
			    e.printStackTrace();
            }
        }*/
        Logger.i(TAG, "markMessageAsRead() exit");
    }

	 protected void markFtAsRead(FileTransfer msg) {
        try {
            Logger.d(TAG, "markMessageAsRead() entry:" + msg.getTransferId());
            // EventsLogApi events = null;
            if (ApiManager.getInstance() != null) {
                //events = ApiManager.getInstance().getEventsLogApi(); //TODo check this
                /*if (RichMessagingDataProvider.getInstance() == null) {
            		RichMessagingDataProvider.createInstance(MediatekFactory.getApplicationContext());
            	}	
            	try {
                    RichMessagingDataProvider.getInstance().markFTMessageAsRead(msg.getTransferId(), true);
            	} catch(Exception e) {
            		e.printStackTrace();
                }*/
                FileTransferService fileTransferService = ApiManager.getInstance().getFileTransferApi();
                if(fileTransferService != null){
                    fileTransferService.markFileTransferAsRead(msg.getTransferId());
                }
            }
            Logger.d(TAG, "markMessageAsRead() exit");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

     protected void markNewFtAsRead(FileTransfer msg) {
        try {
            Logger.d(TAG, "markNewFtAsRead() entry:" + msg.getTransferId());
      // EventsLogApi events = null;
            if (ApiManager.getInstance() != null) {
                //events = ApiManager.getInstance().getEventsLogApi(); //TODo check this
                /*if(RichMessagingDataProvider.getInstance() == null){
            		RichMessagingDataProvider.createInstance(MediatekFactory.getApplicationContext());
            	}	
            	try {
                    RichMessagingDataProvider.getInstance().markFTMessageAsRead(msg.getTransferId(), true);
            	} catch(Exception e) {
            		e.printStackTrace();
            	}*/
                FileTransferService fileTransferService = ApiManager.getInstance().getFileTransferApi();
                if(fileTransferService != null){
                    fileTransferService.markFileTransferAsRead(msg.getTransferId());
                }
                
            }
            Logger.d(TAG, "markMessageAsRead() exit");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * .
     */
    protected void markUnreadMessageDisplayed() {
        Logger.v(TAG, "markUnreadMessageDisplayed() entry");
        int size = mReceivedInBackgroundToBeDisplayed.size();
        for (int i = 0; i < size; i++) {
            ChatMessage msg = mReceivedInBackgroundToBeDisplayed.get(i);
            markMessageAsDisplayed(msg);
            Logger.v(TAG, "The message " + msg.getContent()
                    + " is displayed");
        }
        size = mReceivedInBackgroundToBeRead.size();
        for (int i = 0; i < size; i++) {
            ChatMessage msg = mReceivedInBackgroundToBeRead.get(i);
            markMessageAsRead(msg);
            Logger.v(TAG, "The message " + msg.getContent() + " is read");
        }
        clearUnreadMessage();
        Logger.v(TAG, "markUnreadMessageDisplayed() exit");
    }

protected void markUnreadFtDisplayed() {
        Logger.v(TAG, "markUnreadFtDisplayed() entry");
        int size = mReceivedInBackgroundFtToBeDisplayed.size();
        for (int i = 0; i < size; i++) {
            //ChatMessage msg = mReceivedInBackgroundFtToBeDisplayed.get(i);
            //markMessageAsDisplayed(msg);
            //Logger.v(TAG, "The message " + msg.getMessage() + " is displayed");
        }
        size = mReceivedInBackgroundFtToBeRead.size();
        for (int i = 0; i < size; i++) {
            FileTransfer msg = mReceivedInBackgroundFtToBeRead.get(i);
            markFtAsRead(msg);
			try{
                Logger.v(TAG, "The message " + msg.getTransferId() + " is read");
			} catch(Exception e) {
				e.printStackTrace();
			}
        }
        clearUnreadFt();
        Logger.v(TAG, "markUnreadFtDisplayed() exit");
    }
	
	protected void markReloadDisplayed() {
        Logger.v(TAG, "markReloadDisplayed() entry");
        Logger.v(TAG, "markReloadDisplayed() mReceivedAfterReloadMessage: " + mReceivedAfterReloadMessage);
        Logger.v(TAG, "markReloadDisplayed() mReceivedAfterReloadFt: " + ((ModelImpl)mTag).mReceivedAfterReloadFt);
        int size = mReceivedAfterReloadMessage.size();
        size = mReceivedAfterReloadMessage.size();
        boolean isChatClear = false;
        boolean isFtClear = false;
        try {
        for (int i = 0; i < size; i++) {
                    /*if(RichMessagingDataProvider.getInstance() == null){
                    RichMessagingDataProvider.createInstance(MediatekFactory.getApplicationContext());
                }   
                try {
                    RichMessagingDataProvider.getInstance().markChatMessageAsRead(mReceivedAfterReloadMessage.get(i), true);
                } catch(Exception e) {
                    e.printStackTrace();
                    }*/
                    if (ApiManager.getInstance() != null) {
                        ChatService chatService = ApiManager.getInstance().getChatApi();
                        if(chatService != null){
                            chatService.markMessageAsRead(mReceivedAfterReloadMessage.get(i));
                            isChatClear = true;
                        }
                }
            }
        
        size = ((ModelImpl)mTag).mReceivedAfterReloadFt.size();
        for (int i = 0; i < size; i++) {
            if (ApiManager.getInstance() != null) {
                //events = ApiManager.getInstance().getEventsLogApi(); //TODo check this
                    /*if(RichMessagingDataProvider.getInstance() == null){
                    RichMessagingDataProvider.createInstance(MediatekFactory.getApplicationContext());
                }   
                try {
                    RichMessagingDataProvider.getInstance().markFTMessageAsRead(((ModelImpl)mTag).mReceivedAfterReloadFt.get(i), true);
                } catch(Exception e) {
                    e.printStackTrace();
                    }*/
                    FileTransferService fileTransferService = ApiManager.getInstance().getFileTransferApi();
                    if(fileTransferService != null){
                        fileTransferService.markFileTransferAsRead(((ModelImpl)mTag).mReceivedAfterReloadFt.get(i));
                        isFtClear = true;
                }
            }
        }
            Logger.v(TAG, "markUnreadFtDisplayed() isChatClear:" + isChatClear);
            if(isFtClear){
        ((ModelImpl)mTag).mReceivedAfterReloadFt.clear();
            }
            if(isChatClear){
        mReceivedAfterReloadMessage.clear();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        Logger.v(TAG, "markUnreadFtDisplayed() exit");
    }


    private void terminateGroupChatImpl() throws RemoteException {
        if (mCurrentGroupChatImpl.get() != null) {
            try {
                mCurrentGroupChatImpl.get().leave();
            } catch (Exception e) {
                e.printStackTrace();
            }
            mCurrentGroupChatImpl.set(null);
            Logger.i(TAG,
                    "terminateGroupChatImpl()---mCurrentGroupChatImpl cancel and is null");
        }
    }

    /**
     * @param message
     * .
     * @param messageType
     * .
     * @param status
     * .
     * @param messageTag
     * .
     * @param chatId
     * .
     */
    protected void reloadMessage(ChatMessage message, int messageType,
            int status, int messageTag, String chatId) {
        Logger.w(TAG,
                "reloadMessage() sub-class needs to override this method");
    }

    /**
     * @param fileStruct
     * .
     * @param transferType
     * .
     * @param status
     * .
     */
    protected void reloadFileTransfer(FileStruct fileStruct,
            int transferType, int status) {
        Logger.w(TAG,
                "M0CFF reloadFileTransfer() sub-class needs to override this method");
    }

    /**
     * Composing Manager.
     *.
     */
    protected class ComposingManager {
        private static final int ACT_STATE_TIME_OUT = 60 * 1000;
        private boolean mIsComposing = false;
        private static final int STARTING_COMPOSING = 1;
        private static final int STILL_COMPOSING = 2;
        private static final int MESSAGE_WAS_SENT = 3;
        private static final int ACTIVE_MESSAGE_REFRESH = 4;
        private static final int IS_IDLE = 5;
        private ComposingHandler mWorkerHandler = new ComposingHandler(
                ModelImpl.CHAT_WORKER_THREAD.getLooper());

        /**
         * .
         */
        public ComposingManager() {
        }

        /**
         * Composing handler
         *.
         */
        protected class ComposingHandler extends Handler {
            public static final String TAG = "M0CF ComposingHandler";
            private boolean canSendIsComposing = false;

            /**
             * @param looper .
             */
            public ComposingHandler(Looper looper) {
                super(looper);
            }

            @Override
            public void handleMessage(Message msg) {
                Logger.w(TAG, "handleComposingMessage() the msg is:" + msg.what);
                switch (msg.what) {
                case STARTING_COMPOSING:
                    Logger.i(TAG, "handleComposingMessage() StartingComposing " + canSendIsComposing);
                    if(canSendIsComposing == true) {
                    if (setIsComposing(true)) {
                        mIsComposing = true;
                        mWorkerHandler.sendEmptyMessageDelayed(IS_IDLE,
                                ModelImpl.sIdleTimeout);
                        mWorkerHandler.sendEmptyMessageDelayed(
                                ACTIVE_MESSAGE_REFRESH, ACT_STATE_TIME_OUT);
                    } else {
                        Logger.d(TAG,
                                "handleComposingMessage STARTING_COMPOSING-> failed to set isComposing to true");
                    }
                    }
                    break;

                case STILL_COMPOSING:
                    mWorkerHandler.removeMessages(IS_IDLE);
                    mWorkerHandler.sendEmptyMessageDelayed(IS_IDLE,
                            ModelImpl.sIdleTimeout);
                    break;

                case MESSAGE_WAS_SENT:
                    Logger.i(TAG, "handleComposingMessage() MessageWasSent " + canSendIsComposing);
                    /*if(canSendIsComposing == true){
                         if (setIsComposing(false)) {
                        //if (true) {
                            mComposingManager.hasNoText();
                            mWorkerHandler.removeMessages(IS_IDLE);
                            mWorkerHandler
                                    .removeMessages(ACTIVE_MESSAGE_REFRESH);
                        } else {
                            Logger.d(TAG,
                                    "handleComposingMessage MESSAGE_WAS_SENT-> failed to set isComposing to false");
                        }
                    } else {*/
                        canSendIsComposing = true;
                        mComposingManager.hasNoText();
                        mWorkerHandler.removeMessages(IS_IDLE);
                        mWorkerHandler.removeMessages(ACTIVE_MESSAGE_REFRESH);
                    //}
                    break;

                case ACTIVE_MESSAGE_REFRESH:
                    Logger.i(TAG, "handleComposingMessage() ActiveMessageRefresh ");
                    if (setIsComposing(true)) {
                        mWorkerHandler.sendEmptyMessageDelayed(
                                ACTIVE_MESSAGE_REFRESH, ACT_STATE_TIME_OUT);
                    } else {
                        Logger.d(TAG,
                                "ACTIVE_MESSAGE_REFRESH-> failed to set isComposing to true");
                    }
                    break;

                case IS_IDLE:
                    Logger.i(TAG, "handleComposingMessage() IsIdle ");
                    if (setIsComposing(false)) {
                        mComposingManager.hasNoText();
                        mWorkerHandler
                                .removeMessages(ACTIVE_MESSAGE_REFRESH);
                    } else {
                        Logger.d(TAG,
                                "handleComposingMessage IS_IDLE-> failed to set isComposing to false");
                    }
                    break;

                default:
                    Logger.w(TAG, "handleComposingMessage()--message" + msg.what);
                    break;

                }
            }
        }

        /**
         * @param isEmpty
         * .
         */
        public void hasText(Boolean isEmpty) {
            Logger.w(TAG, "hasText() entry the edit is " + isEmpty);
            if (isEmpty) {
               // mWorkerHandler.sendEmptyMessage(MESSAGE_WAS_SENT);
            } else {
                if (!mIsComposing) {
                    mWorkerHandler.sendEmptyMessage(STARTING_COMPOSING);
                } else {
                    mWorkerHandler.sendEmptyMessage(STILL_COMPOSING);
                }
            }
        }

        public void hasNoText() {
            mIsComposing = false;
        }

        public void messageWasSent() {
            mWorkerHandler.sendEmptyMessage(MESSAGE_WAS_SENT);
        }
    }

    protected boolean setIsComposing(boolean isComposing) {
        if (mCurrentGroupChatImpl == null) {
            Logger.e(TAG, "setIsComposing() -- The chat with the tag "
                    + " doesn't exist!");
            return false;
        } else {
            try {
                if (mCurrentGroupChatImpl.get() != null) {
                    mCurrentGroupChatImpl.get().setComposingStatus(isComposing);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            return true;
        }
    }

    @Override
    public IChatMessage getSentChatMessage(int index) {
        if (index < 0 || index > mMessageList.size()) {
            return null;
        }
        return mMessageList.get(index);
    }

    @Override
    public int getChatMessageCount() {
        return mMessageList.size();
    }

    @Override
    public List<IChatMessage> listAllChatMessages() {
        return mMessageList;
    }

    @Override
    public abstract void loadChatMessages(int count);

    @Override
    public boolean removeMessage(int index) {
        if (index < 0 || index > mMessageList.size()) {
            return false;
        }
        mMessageList.remove(index);
        return true;
    }

    @Override
    public boolean removeMessages(int start, int end) {
        if (start < 0 || start > mMessageList.size()) {
            return false;
        }
        return true;
    }

    /**
     * Check capabilities before inviting a chat.
     */
    protected abstract void checkCapabilities();

    /**
     * Query capabilities after terminating a chat.
     */
    protected abstract void queryCapabilities();

    @Override
    public void hasTextChanged(boolean isEmpty) {
        mComposingManager.hasText(isEmpty);
    }

    // public abstract void onCapabilityChanged(String contact, Capabilities
    // capabilities);
    /**
     * @param contact
     *                 Contact name .
     * @param capabilities
     *                 Capabilities of contact.
     */
    public abstract void onCapabilitiesReceived(String contact,
            Capabilities capabilities);

    @Override
    public abstract void onStatusChanged(boolean status);
}
