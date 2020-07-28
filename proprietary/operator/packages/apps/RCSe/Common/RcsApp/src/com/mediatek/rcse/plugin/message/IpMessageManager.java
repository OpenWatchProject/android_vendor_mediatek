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

import android.app.ProgressDialog;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.SQLException;
import android.media.MediaFile;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.provider.Telephony;
import android.provider.Telephony.Sms;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.plugin.message.IpMessageConsts.IpMessageType;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.Utils;
import com.mediatek.rcse.service.VideoCompress;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.mediatek.rcse.settings.AppSettings;

import com.mediatek.rcs.R;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map.Entry;
import java.util.Random;
import java.util.Set;
import java.util.TreeSet;
import java.util.UUID;

import com.gsma.services.rcs.chat.ChatService;
import com.gsma.services.rcs.ft.FileTransferService;

/**
 * IpMessagemanager used for Managing Message Service from MMS app.
 * @author MTK33296
 *
 */
public class IpMessageManager {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "IpMessageManager";
    /**
     * The s cache rcse message.
     */
    private static HashMap<Long, IpMessage> sCacheRcseMessage =
            new HashMap<Long, IpMessage>();

    /**
     * getsCacheRcseMessage.
     * @return Hash map
     */
    public static HashMap<Long, IpMessage> getsCacheRcseMessage() {
        return sCacheRcseMessage;
    }
    /**
     * setsCacheRcseMessage.
     * @param sCacheRcseMessage cache message
     */
    public static void setsCacheRcseMessage(
            HashMap<Long, IpMessage> sCacheRcseMessage) {
        IpMessageManager.sCacheRcseMessage = sCacheRcseMessage;
    }

    /**
     * The number to message ids map.
     */
    private static HashMap<String, ArrayList<Long>> sNumberToMessageIdsMap =
            new HashMap<String, ArrayList<Long>>();
    /**
     * The s message map.
     */
    private static HashMap<String, Long> sMessageMap =
            new HashMap<String, Long>();
    /**
     * The m pre sent message list.
     */
    private HashMap<Long, PresentTextMessage> mPreSentMessageList =
            new HashMap<Long, PresentTextMessage>();
    /**
     * The Constant SAVE_SUCCESS.
     */
    private static final int SAVE_SUCCESS = 1;
    /**
     * The Constant SAVE_DRAFT.
     */
    private static final int SAVE_DRAFT = 0;
    /**
     * The Constant SAVE_FAIL.
     */
    private static final int SAVE_FAIL = -1;
    /**
     * The Constant RANDOM.
     */
    private static final Random RANDOM = new Random();
    /**
     * The Constant MESSAGE_TAG_RANGE.
     */
    private static final int MESSAGE_TAG_RANGE = 1000;
    /**
     * The Constant SPACE.
     */
    private static final String SPACE = " ";
    /**
     * The Constant FAILED.
     */
    private static final int FAILED = 5;
    /**
     * The m rejected.
     */
    private String mRejected = null;
    /**
     * The m canceled.
     */
    private String mCanceled = null;
    /**
     * The m you.
     */
    private String mYou = null;
    /**
     * The m failed.
     */
    private String mFailed = null;
    /**
     * The m warning no ft capability.
     */
    private String mWarningNoFtCapability = null;
    /**
     * The Constant COMMA.
     */
    static final String COMMA = ",";
    /**
     * The m context.
     */
    public Context mContext;
    /**
     * The m ui handler.
     */
    private Handler mUiHandler = null;
    /**
     * The resize dialog shown.
     */
    public static boolean sResizeDialogShown = false;

    public static final Uri SMS_CONTENT_URI = Uri.parse("content://sms");

    private ProgressDialog mProgressDialog;
    /**
     * The m pending files.
     */
    ArrayList<PendingFiles> mPendingFiles =
            new ArrayList<IpMessageManager.PendingFiles>();

    private static IpMessageManager sIpMessageManager;

    public static synchronized IpMessageManager getInstance(Context context) {
        if (sIpMessageManager == null) {
            sIpMessageManager = new IpMessageManager(context);
        }
        return sIpMessageManager;
    }

    /**
     * constructor.
     * @param context context
     */
    private IpMessageManager(Context context) {
        mContext = context;
        initializeStringInRcse();
        mUiHandler = new Handler(Looper.getMainLooper());
        Logger.d(TAG, "MessageManagerExt() entry ");
    }
    /**
     * Initialize string in rcse.
     */
    private void initializeStringInRcse() {
        mRejected = PluginUtils
                .getStringInRcse(R.string.file_transfer_rejected);
        mCanceled = PluginUtils
                .getStringInRcse(R.string.file_transfer_canceled);
        mYou = PluginUtils.getStringInRcse(R.string.file_transfer_you);
        mFailed = PluginUtils.getStringInRcse(R.string.file_transfer_failed);
        mWarningNoFtCapability = PluginUtils
                .getStringInRcse(R.string.warning_no_file_transfer_capability);
    }
    /**
     * Message present in cache or not.
     *
     * @param messageIdInMms the message id in mms
     * @return true, if is in cache
     */
    public static boolean isInCache(Long messageIdInMms) {
        Logger.d(TAG, "isInCache() entry! messageIdInMms = " + messageIdInMms);
        synchronized (sCacheRcseMessage) {
            return (sCacheRcseMessage.containsKey(messageIdInMms));
        }
    }
    /**
     * Remove present message after it has been really sent out.
     *
     * @param messageTag            The tag of the present message
     * @return TRUE if the message has been removed, otherwise FALSE
     */
    public boolean removePresentMessage(int messageTag) {
        synchronized (mPreSentMessageList) {
            Collection<PresentTextMessage> values = mPreSentMessageList
                    .values();
            for (PresentTextMessage message : values) {
                if (messageTag == message.messageTag) {
                    Logger.d(TAG, "removePresentMessage() messageTag: "
                            + messageTag + " found!");
                    values.remove(message);
                    return true;
                }
            }
            Logger.d(TAG, "removePresentMessage() messageTag: " + messageTag
                    + " not found!");
            return false;
        }
    }
    /**
     * is joyn messaging disabled.
     * @return true/false
     */
    public boolean isJoynMessagingDisabled() {
        if (AppSettings.getInstance() == null) {
            AppSettings.createInstance(mContext);
        }
        boolean joynDisabled = AppSettings.getInstance()
                .getJoynMessagingDisabledFullyIntegrated();
        Logger.d(TAG, "isJoynMessagingDisabled() entry! disableStatus = "
                + joynDisabled);
        return joynDisabled;
    }
    /**
     * clearAllHistory.
     */
    public static void clearAllHistory() {
        Logger.d(TAG, "clearAllHistory() entry!");
        synchronized (sCacheRcseMessage) {
            sCacheRcseMessage.clear();
        }
        synchronized (sMessageMap) {
            sMessageMap.clear();
        }
        ((ModelImpl) ModelImpl.getInstance()).clearAllChatHistory();
    }
    /**
     * Clear all chat history.
     *
     * @param messageIdsinMMS the message idsin mms
     * @param number the number
     */
    private void clearAllChatHistory(List<Long> messageIdsinMMS, String number) {
        try {
        Logger.d(TAG, "clearAllChatHistory() entry!");
        ArrayList<String>ids = new ArrayList<String>();
        if(number == null)
            return;
        for (Long messageId : messageIdsinMMS) {
        	String msgId =  getMessageIdinRcse(messageId);
			if (msgId != null) {
				ids.add(msgId);
			}
            synchronized (sCacheRcseMessage) {
                sCacheRcseMessage.remove(messageId);
            }
            synchronized (sMessageMap) {
                sMessageMap.remove(getMessageIdinRcse(messageId));
            }
        }
		if(PluginUtils.getMessagingMode() == 0)
        {
            if (number.startsWith(IpMessageConsts.JOYN_START)) {
                number = number.substring(4);
            }
        }
		String tag = PluginChatWindowManager.findWindowTagIndex(number).getWindowTag();
        Logger.d(TAG, "clearAllChatHistory() number is " + number + "& messageIds " + ids + " & tag is" + tag);
        Message controllerMessage = PluginController.obtainMessage(
        ChatController.EVENT_DELETE_MESSAGE_LIST, tag,ids);
        controllerMessage.sendToTarget();
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
    /**
     * Clear chat history of this number.
     * @param number number
     */
    public void clearChatHistory(String number) {
        ArrayList<Long> messageIdsForNumber = null;
        messageIdsForNumber = sNumberToMessageIdsMap.get(number);
        if (messageIdsForNumber != null && !messageIdsForNumber.isEmpty()) {
            clearAllChatHistory(messageIdsForNumber, number);
        }
        // PluginUtils.removeMessagesFromDatabase(messageIdsForNumber);
    }
    /**
     * This message present in cache or not.
     * @param messageIdInRcse id in RCS database
     * @return true/false
     */
    public static boolean isInCache(String messageIdInRcse) {
        Logger.d(TAG, "isInCache() entry! messageIdInRcse = " + messageIdInRcse);
        synchronized (sMessageMap) {
            return (sMessageMap.containsKey(messageIdInRcse));
        }
    }
    /**
     * Max length of message a user can send.
     *
     * @return the max text limit
     */
    public int getMaxTextLimit() {
        int limit = PluginUtils.getMaximumTextLimit();
        if(RcseComposeActivity.mCurrentChatMode == IpMessageConsts.ChatMode.XMS){
            Logger.d(TAG, "getMaxTextLimit XMS return 0");
            return 0;
        }
        return limit;
    }
    /**
     * Add this message in Plug-in cache, so MMS can access it without IPC.
     *
     * @param messageIdInMms id in mms db
     * @param messageIdInRcse id in rcs db
     * @param message the message
     */
    public static void addMessage(Long messageIdInMms, String messageIdInRcse,
            IpMessage message) {
        Logger.d(TAG, "addMessage() entry! messageIdInMms = " + messageIdInMms
                + " messageIdInRcse = " + messageIdInRcse + "\n message = "
                + message);

        ArrayList<Long> messageIds = new ArrayList<Long>();
        synchronized (sCacheRcseMessage) {
            // If the map contains the key(messageIdInMms), will update the
            // value(message)
            sCacheRcseMessage.put(messageIdInMms, message);
           /* if (sNumberToMessageIdsMap.containsKey(message.getTo())) {
                Logger.d(TAG,
                        "addMessage() already contains key! messageIdInMms = "
                                + messageIdInMms + " messageIdInRcse = "
                                + messageIdInRcse + " message contact = "
                                + message.getTo());
                messageIds = sNumberToMessageIdsMap.get(message.getTo());
                messageIds.add(messageIdInMms);
                sNumberToMessageIdsMap.put(message.getTo(), messageIds);
            } else {
                Logger.d(TAG,
                        "addMessage() doesnot contain key! messageIdInMms = "
                                + messageIdInMms + " messageIdInRcse = "
                                + messageIdInRcse + " message contact = "
                                + message.getTo());
                messageIds.add(messageIdInMms);
                sNumberToMessageIdsMap.put(message.getTo(), messageIds);
            }*/
        }
        synchronized (sMessageMap) {
            // First remove the entry which value contains messageIdInMms, then
            // add a new entry to the map for update the messageIdInRcse
            sMessageMap.values().remove(messageIdInMms);
            sMessageMap.put(messageIdInRcse, messageIdInMms);
        }
    }
    /**
     * Remove this message from cache.
     * @param messageIdInMms id in mms
     * @param messageIdInRcse id in rcs db
     * @param number number
     */
    public static void removeMessage(Long messageIdInMms,
            String messageIdInRcse, String number) {
        Logger.d(TAG, "removeMessage() entry! messageIdInMms = "
                + messageIdInMms + " messageIdInRcse = " + messageIdInRcse);
        synchronized (sCacheRcseMessage) {
            sCacheRcseMessage.remove(messageIdInMms);
        }
        synchronized (sMessageMap) {
            sMessageMap.remove(messageIdInRcse);
        }
    }
    /**
     * Delete single message from cache & from RCS database as well.
     *
     * @param messageIdInMms id in mms
     * @param messageIdInRcse id in rcse
     * @param number the number
     */
    public static void deleteMessage(Long messageIdInMms,
            String messageIdInRcse) {
        Logger.d(TAG, "deleteMessage() entry! messageIdInMms = "
                + messageIdInMms + " messageIdInRcse = " + messageIdInRcse);
        String number = null;
		IpMessage message = null;
		message = getMessage(messageIdInMms);
        if (message != null) {
            number = message.getTo();

        if (number == null || number.equals(""))
                number = message.getFrom();
        }
        synchronized (sCacheRcseMessage) {
            sCacheRcseMessage.remove(messageIdInMms);
        }
        synchronized (sMessageMap) {
            sMessageMap.remove(messageIdInRcse);
        }
        Logger.d(TAG, "deleteMessage() " + messageIdInMms + "number =" + number);
    	if(number == null || number.equals(""))
		    return;
        String tag = PluginChatWindowManager.findWindowTagIndex(number)
                .getWindowTag();
        Message controllerMessage = PluginController.obtainMessage(
                ChatController.EVENT_DELETE_MESSAGE, tag, messageIdInRcse);
        if(number.startsWith(IpMessageConsts.GROUP_START)) {
            ArrayList<String>ids = new ArrayList<String>();
            ids.add(messageIdInRcse);
            controllerMessage = PluginController.obtainMessage(
                    ChatController.EVENT_DELETE_MESSAGE_LIST, tag, ids);
            Logger.d(TAG, "deleteMessage() group chat! ids size = "
                    + ids.size() + " messageIdInRcse = " + messageIdInRcse);
        }
        controllerMessage.sendToTarget();
    }
    /**
     * API called by MMS with ids to delete from mms db, cache, and from rcs db.
     * @param ids ids
     * @param delImportant true/false
     * @param delLocked true/false
     */
    public void deleteIpMsg(long[] ids, boolean delLocked) {
        Logger.d(TAG, "deleteIpMsg() entry! " + ids);
        ArrayList<String> numbers = new ArrayList<String>();
        IpMessage message = null;
        if(ids.length <=0)
		{
		    Logger.d(TAG, "deleteIpMsg() legth zero, so nothing to delete");
		    return;
		}

		if(ids.length == 1)
		{
            PluginUtils.removeMessageFromMMSDatabase(ids[0]);
		    deleteMessage(ids[0], getMessageIdinRcse(ids[0]));
		    return;
		}
        PluginUtils.removeMultiMessagesFromMMSDatabase(ids);
        if (ids.length != 0) {
			for (int i = 0; i < ids.length; i++) {
				String number = null;
				message = getMessage(ids[i]);
        if (message != null) {
					number = message.getTo();

					if (number == null || number.equals(""))
                number = message.getFrom();
            }
				if (sNumberToMessageIdsMap.containsKey(number)) {
					Logger.d(TAG,
							"numberToMessageIdsMap() already contains key! messageIdInMms = "
									+ ids[i] + " & number is " + number);
					ArrayList<Long> messageIds = sNumberToMessageIdsMap
							.get(number);
					messageIds.add(ids[i]);
					sNumberToMessageIdsMap.put(number, messageIds);
				} else {
					Logger.d(TAG,
							"numberToMessageIdsMap() doesnot contains key! messageIdInMms = "
									+ ids[i] + " & number is " + number);
					ArrayList<Long> messageIds = new ArrayList<Long>();
					messageIds.add(ids[i]);
					sNumberToMessageIdsMap.put(number, messageIds);
				}
            }
            }
		for (String number : sNumberToMessageIdsMap.keySet()) {
			ArrayList<Long> messageIds = sNumberToMessageIdsMap.get(number);
			Logger.d(TAG, "deleteIpMsg() exit loop! number is " + number
					+ "& messageIds " + messageIds);
			clearAllChatHistory(messageIds, number);
        }
		sNumberToMessageIdsMap.clear();
    }
    /**
     * getMessage from plug-in cache.
     *
     * @param messageIdInMms id in mms db
     * @return the message
     */
    public static IpMessage getMessage(Long messageIdInMms) {
        synchronized (sCacheRcseMessage) {
            return sCacheRcseMessage.get(messageIdInMms);
        }
    }
    /**
     * Get messageId in MMS db.
     *
     * @param messageIdInRcse id in rcse db
     * @return the message id
     */
    public static Long getMessageId(String messageIdInRcse) {
        synchronized (sMessageMap) {
            return sMessageMap.get(messageIdInRcse);
        }
    }
    /**
     * getMessageId in Rcs db.
     *
     * @param messageIdMms id in mms db
     * @return the message idin rcse
     */
    public static String getMessageIdinRcse(Long messageIdMms) {
        synchronized (sMessageMap) {
            for (Entry<String, Long> e : sMessageMap.entrySet()) {
                String key = e.getKey();
                Long value = e.getValue();
                if (value.equals(messageIdMms)) {
                    return key;
                }
            }
        }
        return null;
    }
    /**
     * Update cache with new tag file.
     * @param oldTag old tag
     * @param fileStruct file structure
     * @param remote remote contact
     */
    public static void updateCache(String oldTag,
            FileStructForBinder fileStruct, String remote) {
        String newTag = fileStruct.mFileTransferTag;
        Logger.d(TAG, "updateCache(), oldTag = " + oldTag + " newTag = "
                + newTag);
        long messageIdInMms = getMessageId(oldTag);
        int type = getMessage(messageIdInMms).getType();
        removeMessage(messageIdInMms, oldTag, remote);
        IpMessage message = PluginUtils.exchangeIpMessage(type, fileStruct,
                remote);
        addMessage(messageIdInMms, newTag, message);
    }
    /**
     * Status of this ipmessage.
     *
     * @param msgId msgid in mms
     * @return the status
     */
    public int getStatus(long msgId) {
        Logger.d(TAG, "getStatus() entry with msgId " + msgId);
        Status status = null;
        IpMessage message = getMessage(msgId);
        if (message == null) {
            Logger.e(TAG, "getStatus(), message is null!");
            return 0;
        }
        if (message instanceof PluginIpTextMessage) {
            status = ((PluginIpTextMessage) message).getMessageStatus();
            return convertToMmsStatus(status);
        } else {
            Logger.w(TAG, "getStatus() ipMessage is " + message);
            int messageType = message.getType();
            switch (messageType) {
            case IpMessageConsts.IpMessageType.PICTURE:
                if (message instanceof PluginIpImageMessage) {
                    return ((PluginIpImageMessage) message).getStatus();
                } else {
                    return IpMessageConsts.IpMessageStatus.MO_INVITE;
                }
            case IpMessageConsts.IpMessageType.VIDEO:
                if (message instanceof PluginIpVideoMessage) {
                    return ((PluginIpVideoMessage) message).getStatus();
                } else {
                    return IpMessageConsts.IpMessageStatus.MO_INVITE;
                }
            case IpMessageConsts.IpMessageType.VOICE:
                if (message instanceof PluginIpVoiceMessage) {
                    return ((PluginIpVoiceMessage) message).getStatus();
                } else {
                    return IpMessageConsts.IpMessageStatus.MO_INVITE;
                }
            case IpMessageConsts.IpMessageType.VCARD:
                if (message instanceof PluginIpVcardMessage) {
                    return ((PluginIpVcardMessage) message).getStatus();
                } else {
                    return IpMessageConsts.IpMessageStatus.MO_INVITE;
                }
            case IpMessageConsts.IpMessageType.CALENDAR:
                if (message instanceof PluginIpVCalendarMessage) {
                    return ((PluginIpVCalendarMessage) message).getStatus();
                } else {
                    return IpMessageConsts.IpMessageStatus.MO_INVITE;
                }
            default:
                return ((PluginIpAttachMessage) message).getStatus();
            }
        }
    }
    /**
     * get ipmessage in plug-in cache on mms id(called by MMS to show on UI).
     *
     * @param msgId msgid in mms
     * @return the ip msg info
     */
    public IpMessage getIpMsgInfo(long msgId) {
        Logger.v(TAG, "getIpMsgInfo() msgId = " + msgId);
        IpMessage ipMessage = getMessage(msgId);
        if (ipMessage != null) {
            Logger.v(TAG, "getIpMsgInfo() found in message cache");
            // Logger.d(TAG, "getIpMsgInfo()Status is " +
            // ((PluginIpTextMessage)ipMessage).getMessageStatus());
            return ipMessage;
        } else {
            Logger.v(TAG, "Can not find nessage in the cache!");
            synchronized (mPreSentMessageList) {
                ipMessage = mPreSentMessageList.get(msgId);
            }
            if (ipMessage != null) {
                Logger.v(TAG, "getIpMsgInfo() found in present message list");
                if(ipMessage.getType() == IpMessageConsts.IpMessageType.TEXT)
                    ipMessage.setStatus(IpMessageConsts.IpMessageStatus.OUTBOX);
                return ipMessage;
            }
            Logger.v(TAG,
                    "getIpMsgInfo() cannot find this message, forget to add it into cache?");
            return new IpTextMessage();
        }
    }

    public void initSpamReport(String contact, String msgId, int msgType) {
        Logger.d(TAG, " [spam-report] initSpamReport contact = " + contact +
                " msgId = " + msgId+" msgType = " + msgType);
        ChatService service = ApiManager.getInstance().getChatApi();
        FileTransferService ftService = ApiManager.getInstance().getFileTransferApi();
        try {
            if (msgType == IpMessageType.TEXT) {
                //service.initiateSpamReport(contact, msgId);
            } else {
                //ftService.initiateFileSpamReport(contact, msgId);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    /**
     * Convert status in RCSe to the status corresponding in Mms.
     *
     * @param status
     *            The status in RCSe
     * @return status in Mms
     */
    static public int convertToMmsStatus(Status status) {
        Logger.d(TAG, "convertToMmsStatus() entry with status is " + status);
        int statusInMms = IpMessageConsts.IpMessageStatus.FAILED;
        if (status == null) {
            return IpMessageConsts.IpMessageStatus.INBOX;
        }
        switch (status) {
        case SENDING:
            statusInMms = IpMessageConsts.IpMessageStatus.OUTBOX;
            break;
        case DELIVERED:
            statusInMms = IpMessageConsts.IpMessageStatus.DELIVERED;
            break;
        case DISPLAYED:
            statusInMms = IpMessageConsts.IpMessageStatus.VIEWED;
            break;
        case FAILED:
            statusInMms = IpMessageConsts.IpMessageStatus.FAILED;
            break;
        case SENT:
            statusInMms = IpMessageConsts.IpMessageStatus.SENT;
            break;
        default:
            statusInMms = IpMessageConsts.IpMessageStatus.FAILED;
            break;
        }
        Logger.d(TAG, "convertToMmsStatus() entry exit with statusInMms is "
                + statusInMms);
        return statusInMms;
    }
    /**
     * Send and save this IpMessage.
     *
     * @param msg ipmessage
     * @param sendMsgMode mode
     * @return the int
     */
    public int saveIpMsg(IpMessage msg, int sendMsgMode) {
        String contact = msg.getTo();
        Logger.w(TAG, "saveIpMsg() entry");
        if (TextUtils.isEmpty(contact)) {
            Logger.w(TAG, "saveIpMsg() invalid contact: " + contact);
            return SAVE_FAIL;
        }
        if (msg.getStatus() == IpMessageConsts.IpMessageStatus.DRAFT) {
            Logger.w(TAG, "saveIpMsg() Draft message status " + msg.getStatus());
            return SAVE_DRAFT;
        }
        if (msg instanceof IpTextMessage) {
            String messageBody = ((IpTextMessage) msg).getBody();
            Logger.d(TAG, "saveIpMsg() send a text message: " + messageBody
                    + " to contact: " + contact);
            return saveChatMsg(messageBody, contact);
        } else if (msg instanceof IpAttachMessage) {
            Logger.d(TAG, "saveIpMsg() send a file to contact: " + contact);
            return saveFileTransferMsg(msg, contact);
        } else {
            Logger.w(TAG, "saveIpMsg() unsupported ip message type");
            return SAVE_FAIL;
        }
    }
    /**
     * get download progress of this IpMessage to be shown UI.
     *
     * @param msgId msg id in mms
     * @return the download process
     */
    public int getDownloadProcess(long msgId) {
        Logger.d(TAG, "getDownloadProcess(), msgId = " + msgId);
        IpMessage message = getMessage(msgId);
        if (message == null) {
            Logger.e(TAG, "getDownloadProcess(), message is null!");
            return 0;
        }
        int messageType = message.getType();
        switch (messageType) {
        case IpMessageConsts.IpMessageType.PICTURE:
            return ((PluginIpImageMessage) message).getProgress();
        case IpMessageConsts.IpMessageType.VIDEO:
            return ((PluginIpVideoMessage) message).getProgress();
        case IpMessageConsts.IpMessageType.VOICE:
            return ((PluginIpVoiceMessage) message).getProgress();
        case IpMessageConsts.IpMessageType.VCARD:
            return ((PluginIpVcardMessage) message).getProgress();
        case IpMessageConsts.IpMessageType.CALENDAR:
            return ((PluginIpVCalendarMessage) message).getProgress();
        case IpMessageConsts.IpMessageType.TEXT:
            return 0;
        default:
            return ((PluginIpAttachMessage) message).getProgress();
        }
    }
    /**
     * Set the status of ipmessage by user action e.g SENDINg,CANCEL etc on MMS UI.
     * @param msgId id in mms db
     * @param msgStatus actual status
     */
    public void setIpMessageStatus(long msgId, int msgStatus) {
        Logger.d(TAG, "setIpMessageStatus(), msgId = " + msgId
                + " msgStatus = " + msgStatus);
        IpMessage message = getMessage(msgId);
        if (message == null) {
            Logger.e(TAG, "setIpMessageStatus(), message is null!");
            return;
        }
        String contact = message.getFrom();
        if (contact == null) {
            contact = message.getTo();
        }
        int messageType = message.getType();
        String messageTag;
        // Get file transfer tag
        switch (messageType) {
        case IpMessageConsts.IpMessageType.PICTURE:
            messageTag = ((PluginIpImageMessage) message).getTag();
            break;
        case IpMessageConsts.IpMessageType.VIDEO:
            messageTag = ((PluginIpVideoMessage) message).getTag();
            break;
        case IpMessageConsts.IpMessageType.VOICE:
            messageTag = ((PluginIpVoiceMessage) message).getTag();
            break;
        case IpMessageConsts.IpMessageType.VCARD:
            messageTag = ((PluginIpVcardMessage) message).getTag();
            break;
        case IpMessageConsts.IpMessageType.CALENDAR:
            messageTag = ((PluginIpVCalendarMessage) message).getTag();
            break;
        default:
            messageTag = ((PluginIpAttachMessage) message).getTag();
            break;
        }
        Logger.d(TAG, "setIpMessageStatus(), messageTag is " + messageTag);
        // Sent message to rcse controller
        Message controllerMessage = null;
        switch (msgStatus) {
        case IpMessageConsts.IpMessageStatus.MO_INVITE:
            controllerMessage = PluginController.obtainMessage(
                    ChatController.EVENT_FILE_TRANSFER_RESENT,
                    message.getFrom(), messageTag);
            break;
        case IpMessageConsts.IpMessageStatus.MO_CANCEL:
            controllerMessage = PluginController.obtainMessage(
                    ChatController.EVENT_FILE_TRANSFER_CANCEL,
                    message.getFrom(), messageTag);
            break;
        case IpMessageConsts.IpMessageStatus.MT_RECEIVING:
            onUserAccept(mContext, message, messageTag);
            break;
        case IpMessageConsts.IpMessageStatus.MT_REJECT:
            controllerMessage = PluginController.obtainMessage(
                    ChatController.EVENT_FILE_TRANSFER_RECEIVER_REJECT,
                    message.getFrom(), messageTag);
            break;
        case IpMessageConsts.IpMessageStatus.MO_PAUSE:
            controllerMessage = PluginController.obtainMessage(
                    ChatController.EVENT_FILE_TRANSFER_PAUSE, contact,
                    messageTag);
            break;
        case IpMessageConsts.IpMessageStatus.MT_PAUSE:
            controllerMessage = PluginController.obtainMessage(
                    ChatController.EVENT_FILE_TRANSFER_PAUSE, contact,
                    messageTag);
            break;
        case IpMessageConsts.IpMessageStatus.MO_RESUME:
            controllerMessage = PluginController.obtainMessage(
                    ChatController.EVENT_FILE_TRANSFER_RESUME, contact,
                    messageTag);
            controllerMessage.arg1 = 0;
            break;
        case IpMessageConsts.IpMessageStatus.MT_RESUME:
            controllerMessage = PluginController.obtainMessage(
                    ChatController.EVENT_FILE_TRANSFER_RESUME, contact,
                    messageTag);
            controllerMessage.arg1 = 1;
            break;
        default:
            break;
        }
        if (controllerMessage == null) {
            Logger.e(TAG, "setIpMessageStatus(), controllerMessage is null!");
            return;
        }
        controllerMessage.sendToTarget();
    }
    /**
     * On user accept.
     *
     * @param context the context
     * @param message the message
     * @param tag the tag
     */
    private void onUserAccept(Context context, IpMessage message, String tag) {
        long maxFileSize = ApiManager.getInstance()
                .getMaxSizeforFileThransfer();
        long warningFileSize = ApiManager.getInstance()
                .getWarningSizeforFileThransfer();
        Logger.v(TAG, "onUserAccept entry: maxFileSize = " + maxFileSize
                + ", warningFileSize = " + warningFileSize);
        IpAttachMessage ipAttachMessage = ((IpAttachMessage) message);
        long fileSize = ipAttachMessage.getSize();
        if (fileSize >= warningFileSize && warningFileSize != 0) {
            SharedPreferences sPrefer = PreferenceManager
                    .getDefaultSharedPreferences(context);
            Boolean isRemind = sPrefer.getBoolean(SettingsFragment.RCS_REMIND,
                    false);
            Logger.w(TAG, "WarningDialog: isRemind = " + isRemind);
            if (isRemind) {
                Logger.v(TAG, "onUserAccept Is Remind true");
                Intent intent = new Intent(InvitationDialog.ACTION);
                intent.putExtra(RcsNotification.CONTACT, message.getFrom());
                intent.putExtra(RcsNotification.SESSION_ID, tag);
                intent.putExtra(InvitationDialog.KEY_STRATEGY,
                        InvitationDialog.STRATEGY_FILE_TRANSFER_SIZE_WARNING);
                intent.putExtra(InvitationDialog.KEY_IS_FROM_CHAT_SCREEN, true);
                String content = MediatekFactory.getApplicationContext()
                        .getString(R.string.file_size_warning_message);
                intent.putExtra(RcsNotification.NOTIFY_CONTENT, content);
                intent.putExtra(RcsNotification.NOTIFY_SIZE,
                        String.valueOf(fileSize));
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(intent);
            } else {
                onUserAcceptFile(mContext, message, tag);
            }
        } else {
            if (!RcsSettings.getInstance().isFileTransferAutoAccepted()) {
                Logger.v(TAG, "onUserAccept Is File transfer Accept false");
                Intent intent = new Intent(InvitationDialog.ACTION);
                intent.putExtra(RcsNotification.CONTACT, message.getFrom());
                intent.putExtra(RcsNotification.SESSION_ID, tag);
                intent.putExtra(InvitationDialog.KEY_STRATEGY,
                        InvitationDialog.STRATEGY_AUTO_ACCEPT_FILE);
                intent.putExtra(InvitationDialog.KEY_IS_FROM_CHAT_SCREEN, true);
                String content = "Do you want to auto accept file";
                intent.putExtra(RcsNotification.NOTIFY_CONTENT, content);
                intent.putExtra(RcsNotification.NOTIFY_SIZE,
                        String.valueOf(fileSize));
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK
                        | Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(intent);
            }
            onUserAcceptFile(mContext, message, tag);
        }
        Logger.v(TAG, "onUserAccept exit");
    }
    /**
     * On user accept file.
     *
     * @param context the context
     * @param message the message
     * @param tag the tag
     */
    private void onUserAcceptFile(Context context, IpMessage message, String tag) {
        Logger.v(TAG, "onUserAcceptFile");
        Message controllerMessage = null;
        controllerMessage = PluginController.obtainMessage(
                ChatController.EVENT_FILE_TRANSFER_RECEIVER_ACCEPT,
                message.getFrom(), tag);
        if (controllerMessage == null) {
            Logger.e(TAG, "setIpMessageStatus(), controllerMessage is null!");
            return;
        }
        controllerMessage.sendToTarget();
    }
    /**
     * Save chat msg.
     *
     * @param message the message
     * @param contact the contact
     * @return the int
     */
    private int saveChatMsg(String message, String contact) {
        if (TextUtils.isEmpty(message)) {
            Logger.e(TAG, "saveChatMsg() invalid message: " + message);
            return SAVE_FAIL;
        }
        Logger.d(TAG, "saveChatMsg() message: " + message + " , contact: "
                + contact);
        if (PluginUtils.getMessagingMode() == 0) {
            if (contact.startsWith(IpMessageConsts.JOYN_START)) {
                contact = contact.substring(4);
            }
        }
        if (!contact.startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER) &&
                !contact.startsWith(PluginMultiChatWindow.MULTI_CONTACT_STRING_BEGINNER))
        contact = Utils.formatNumberToInternational(contact);
        if (!contact.contains(COMMA)) {
            int messageTag = generateMessageTag();
            PresentTextMessage ipMessage = new PresentTextMessage(messageTag,
                    contact, message);
            mPreSentMessageList
                    .put(PluginUtils.storeMessageInMmsDb(messageTag, message,
                            contact, PluginUtils.OUTBOX_MESSAGE, 0), ipMessage);
            sentMessageViaRCSe(message, contact, messageTag);
        } else {
            Set<String> contactSet = collectMultiContact(contact);
            Logger.d(TAG, "saveChatMsg() send chat message to multi contact: "
                    + contactSet);
            long threadId = Telephony.Threads.getOrCreateThreadId(
                    MediatekFactory.getApplicationContext(), contactSet);
            for (String singleContact : contactSet) {
                int messageTag = generateMessageTag();
                sentMessageViaRCSe(message, singleContact, messageTag);
                PresentTextMessage ipMessage = new PresentTextMessage(
                        messageTag, singleContact, message);
                mPreSentMessageList.put(PluginUtils.storeMessageInMmsDb(
                        messageTag, message, singleContact,
                        PluginUtils.OUTBOX_MESSAGE, threadId), ipMessage);
            }
        }
        return SAVE_SUCCESS;
    }
    /**
     * Sent message via rc se.
     *
     * @param message the message
     * @param contact the contact
     * @param messageTag the message tag
     */
    public void sentMessageViaRCSe(String message, String contact,
            int messageTag) {
        Logger.d(TAG, "sentMessageViaRCSe() message: " + message
                + " , contact: " + contact + ", messageTag: " + messageTag);
        Message controllerMessage = PluginController.obtainMessage(
                ChatController.EVENT_SEND_MESSAGE,
                Utils.formatNumberToInternational(contact), message);
        controllerMessage.arg1 = messageTag;
        controllerMessage.sendToTarget();
    }
    /**
     * Warning size for FT transfer as in config by server.
     *
     * @return the warning size for file transfer
     */
    public long getWarningSizeForFileTransfer() {
        return ApiManager.getInstance().getWarningSizeforFileThransfer();
    }

    /**
     * Pending files to be sent.
     * @author MTK33296
     *
     */
    public class PendingFiles {
        /**
         * The muuid.
         */
        ParcelUuid mUuid;

        /** set id.
         * @param muuid uu id
         */
        public void setMuuid(ParcelUuid muuid) {
            this.mUuid = muuid;
        }

        /**
         * The mcontact.
         */
        String mContact;

        /**
         * set contact.
         * @param contact contact
         */
        public void setContact(String contact) {
            this.mContact = contact;
        }

        /**
         * The mfile path.
         */
        String mFilePath;

        /**
         * Sets the file path.
         *
         * @param filePath the new file path
         */
        public void setFilePath(String filePath) {
            this.mFilePath = filePath;
        }
        /**
         * Instantiates a new pending files.
         *
         * @param uuid the uuid
         * @param contact the contact
         * @param filepath the filepath
         */
        public PendingFiles(ParcelUuid uuid, String contact, String filepath) {
            Logger.d(TAG, "PendingFiles() uuid, contact & path is " + uuid
                    + contact + filepath);
            mUuid = uuid;
            mContact = contact;
            mFilePath = filepath;
        }
    }

    /**
     * Send resized files.
     *
     * @param compressImage the compress image
     */
    public void sendResizedFiles(boolean compressImage) {
        Logger.d(TAG, "sendResizedFiles() uuid, contact & path is " + compressImage);
        sResizeDialogShown = false;
        for (int i = 0; i < mPendingFiles.size(); i++) {
            ParcelUuid fileTransferTag = mPendingFiles.get(i).mUuid;
            String contact = mPendingFiles.get(i).mContact;
            String filePath = mPendingFiles.get(i).mFilePath;
            Logger.d(TAG, "PendingFiles() uuid, contact & path is "
                    + fileTransferTag + contact + filePath);
            if (compressImage) {
                filePath = Utils.compressImage(filePath);
            }
            sentFileViaRCSe(filePath, contact, fileTransferTag);
        }
        mPendingFiles.clear();
    }
    /**
     * Save file transfer msg.
     *
     * @param msg the msg
     * @param contact the contact
     * @return the int
     */
    private int saveFileTransferMsg(IpMessage msg, String contact) {
        IpAttachMessage ipAttachMessage = ((IpAttachMessage) msg);
        boolean mCompressDialogRequired = false;
        if (PluginUtils.getMessagingMode() == 0) {
            if (contact.startsWith(IpMessageConsts.JOYN_START)) {
                contact = contact.substring(4);
            }
        }
        contact = Utils.formatNumberToInternational(contact);
        String filePath = ipAttachMessage.getPath();
        int index = filePath.lastIndexOf("/");
        String fileName = filePath.substring(index + 1);
        String mimeType = MediaFile.getMimeTypeForFile(fileName);
        // If mms send true to compress image then we will compress
        if (mimeType == null) {
            Logger.d(TAG, "saveFileTransferMsg() mimeType null");
            mUiHandler.post(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(
                            MediatekFactory.getApplicationContext(),
                            PluginUtils
                                    .getStringInRcse(R.string.file_formats_not_support),
                            Toast.LENGTH_SHORT).show();
                }
            });
            return SAVE_FAIL;
        }
        if (mimeType.contains(Utils.FILE_TYPE_IMAGE)
                && AppSettings.getInstance().isEnabledCompressingImageFromDB()) {
            Logger.d(TAG, "Compress image enabled in Settings");
            filePath = Utils.compressImage(filePath);
            Logger.d(TAG, "Compress image filepath after compress is" + filePath);
        } else if (mimeType.contains(Utils.FILE_TYPE_IMAGE)
                && !AppSettings.getInstance().isEnabledCompressingImageFromDB()) {
            boolean remind = AppSettings.getInstance()
                    .restoreRemindCompressFlag();
            Logger.d(TAG,
                    "Do hit the user to select whether to compress. remind = "
                            + remind);
            if (remind && !RcsSettings.getInstance().isSupportOP07()) {
                mCompressDialogRequired = true;
            } else {
                Logger.d(TAG, "Do not compress image");
            }
        }
        // If mms send true to compress image then we will compress
        // filePath = Utils.compressImage(filePath);
        if (TextUtils.isEmpty(filePath)) {
            Logger.e(TAG, "saveFileTransferMsg() invalid filePath: " + filePath);
            return SAVE_FAIL;
        }

        if(checkIfDRMFile(filePath)) {
            Logger.d(TAG,
                    "saveFileTransferMsg drm locked file not support ");
            mUiHandler.post(new Runnable() {
                @Override
                public void run() {
            Toast.makeText(
                            MediatekFactory.getApplicationContext(),
                            PluginUtils
                                    .getStringInRcse(R.string.file_formats_not_support),
                            Toast.LENGTH_LONG).show();
                }
            });
            return SAVE_FAIL;

        }

        File file = new File(filePath);
        if (!file.exists()) {
            Logger.e(TAG, "saveFileTransferMsg() file does not exist: "
                    + filePath);
            return SAVE_FAIL;
        }
        long fileSize = file.length();
        Logger.d(TAG, "saveFileTransferMsg , fileSize=" + fileSize);

        if (fileSize == 0) {
            Logger.e(TAG, "saveFileTransferMsg() file size zero "
                    + filePath);
            mUiHandler.post(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(
                            MediatekFactory.getApplicationContext(),
                            PluginUtils
                                    .getStringInRcse(R.string.zero_file_repick_message),
                            Toast.LENGTH_SHORT).show();
                }
            });
            return SAVE_FAIL;
        }
        long maxFileSize = ApiManager.getInstance()
                .getMaxSizeforFileThransfer();
        if (fileSize >= maxFileSize && maxFileSize != 0) {
            Logger.d(TAG,
                    "saveFileTransferMsg() file is too large, file size is "
                            + fileSize);
            mUiHandler.post(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(
                            MediatekFactory.getApplicationContext(),
                            PluginUtils
                                    .getStringInRcse(R.string.large_file_repick_message),
                            Toast.LENGTH_SHORT).show();
                }
            });
            return SAVE_FAIL;
        }

        if (mimeType.contains(Utils.FILE_TYPE_VIDEO)
                && AppSettings.getInstance().isEnabledCompressingVideoFromDB() && fileSize >= 1024*1024
                && RcsSettings.getInstance().isSupportOP07()) {
            Logger.d(TAG, "Compress video enabled in Settings");
            Logger.v(TAG, "Show Progress For Compression");
            Intent intent = new Intent(InvitationDialog.ACTION);
            intent.putExtra(InvitationDialog.KEY_STRATEGY,
                    InvitationDialog.STRATEGY_SHOW_VIDEO_PROGRESS_DIALOG);
            intent.putExtra("showProgress", true);
            intent.putExtra("fileName", fileName);
            intent.putExtra(RcsNotification.CONTACT, contact);
            intent.putExtra("filePath", filePath);
            ParcelUuid uuid = new ParcelUuid(UUID.randomUUID());
            intent.putExtra("uuid", uuid);
            intent.putExtra("size", fileSize);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK
                    | Intent.FLAG_ACTIVITY_NEW_TASK);
            MediatekFactory.getApplicationContext().startActivity(intent);
           // Start compressing in back ground thread
            new VideoCompressor(filePath, fileName, contact, (IpAttachMessage) msg).execute();
            return SAVE_SUCCESS;
        }
        // int index = filePath.lastIndexOf("/");
        // String fileName = filePath.substring(index + 1);
        String fileTransferString = PluginUtils
                .getStringInRcse(R.string.file_transfer_title);
        if (!contact.contains(COMMA)) {
            ParcelUuid uuid = new ParcelUuid(UUID.randomUUID());
            long fileTransferIdInMms = PluginUtils.insertDatabase(
                    fileTransferString, contact, Integer.MAX_VALUE,
                    PluginUtils.OUTBOX_MESSAGE);
            FileStructForBinder fileStructForBinder = new FileStructForBinder(
                    filePath, fileName, ipAttachMessage.getSize(), uuid, null);
            IpMessage ipMessage = PluginUtils.analysisFileType(contact,
                    fileStructForBinder);
            addMessage(fileTransferIdInMms, uuid.toString(), ipMessage);
            if (!mCompressDialogRequired) {
                Logger.d(TAG,
                        "saveFileTransferMsg() Compress Dialog no need to show ");
                sentFileViaRCSe(filePath, contact, uuid);
            } else {
                if (!sResizeDialogShown) {
                    Logger.d(TAG,
                            "saveFileTransferMsg() Compress Dialog showing ");
                    Intent intent = new Intent(InvitationDialog.ACTION);
                    intent.putExtra(RcsNotification.CONTACT, contact);
                    intent.putExtra("filePath", filePath);
                    intent.putExtra(InvitationDialog.KEY_STRATEGY,
                            InvitationDialog.STRATEGY_IPMES_RESIZE_FILE);
                    intent.putExtra("uuid", uuid);
                    sResizeDialogShown = true;
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    MediatekFactory.getApplicationContext()
                            .startActivity(intent);
                } else {
                    Logger.d(TAG,
                            "saveFileTransferMsg() Compress Dialog not showing ");
                    mPendingFiles
                            .add(new PendingFiles(uuid, contact, filePath));
                }
            }
        } else {
            Set<String> contactSet = collectMultiContact(contact);
            Logger.d(TAG, "saveFileTransferMsg() send file to multi contact: "
                    + contactSet);
            long threadId = Telephony.Threads.getOrCreateThreadId(
                    MediatekFactory.getApplicationContext(), contactSet);
            for (String singleContact : contactSet) {
                ParcelUuid uuid = new ParcelUuid(UUID.randomUUID());
                long fileTransferIdInMms = PluginUtils.insertDatabase(
                        fileTransferString, singleContact, Integer.MAX_VALUE,
                        PluginUtils.OUTBOX_MESSAGE, threadId);
                FileStructForBinder fileStructForBinder = new FileStructForBinder(
                        filePath, fileName, ipAttachMessage.getSize(), uuid,
                        null);
                IpMessage ipMessage = PluginUtils.analysisFileType(contact,
                        fileStructForBinder);
                addMessage(fileTransferIdInMms, uuid.toString(), ipMessage);
                sentFileViaRCSe(filePath, singleContact, uuid);
            }
        }
        return SAVE_SUCCESS;
    }

    class VideoCompressor extends AsyncTask<Void, Void, String>{

        String filePath = "";
        String fileName = "";
        String remoteContact = "";
        IpAttachMessage message = null;
        boolean compressed = false;

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            Log.d("VideoCompressor", "onPreExecute entry");
        }

        public VideoCompressor(String originFilePath, String filename, String contact,IpAttachMessage msg){
            filePath = originFilePath;
            fileName = filename;
            remoteContact = contact;
            message = msg;
            Log.d("VideoCompressor", "Constructor Entry filePath=" + filePath + "contact=" + contact);
        }

        @Override
        protected String doInBackground(Void... voids) {
            try {
                return VideoCompress.compressVideo(filePath);
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                return filePath;
            }
        }

        @Override
        protected void onPostExecute(String compressedFileName) {
            super.onPostExecute(compressedFileName);

            if(compressedFileName.equalsIgnoreCase(filePath)) {
                Log.d("VideoCompressor", "onPostExecute FAIL, compressedFileName=" + compressedFileName);
                compressed = false;
            } else {
                Log.d("VideoCompressor", "onPostExecute SUCCESS, compressedFileName=" + compressedFileName);
                compressed = true;
            }
            Logger.v(TAG, "Dismiss Progress For Compression");
            Intent intent = new Intent(InvitationDialog.ACTION);
            intent.putExtra(InvitationDialog.KEY_STRATEGY,
                    InvitationDialog.STRATEGY_SHOW_VIDEO_PROGRESS_DIALOG);
            intent.putExtra("showProgress", false);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK
                    | Intent.FLAG_ACTIVITY_NEW_TASK);
            MediatekFactory.getApplicationContext().startActivity(intent);

            if(compressedFileName.equals("")) {
                compressedFileName = filePath;
            }
            if(VideoCompress.ismCancelCompression()) {
                mUiHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(
                                MediatekFactory.getApplicationContext(),
                                PluginUtils
                                        .getStringInRcse(R.string.compress_video_cancelled),
                                Toast.LENGTH_SHORT).show();
                    }
                });
            }
            ParcelUuid uuid = new ParcelUuid(UUID.randomUUID());
            long fileTransferIdInMms = PluginUtils.insertDatabase(
                    "File Transfer", remoteContact, Integer.MAX_VALUE,
                    PluginUtils.OUTBOX_MESSAGE);
            FileStructForBinder fileStructForBinder = new FileStructForBinder(
                    compressedFileName, fileName, message.getSize(), uuid, null);
            IpMessage ipMessage = PluginUtils.analysisFileType(remoteContact,
                    fileStructForBinder);
            addMessage(fileTransferIdInMms, uuid.toString(), ipMessage);
            sentFileViaRCSe(compressedFileName, remoteContact, uuid);
        }
    }

    public static boolean checkIfDRMFile(String fileName) {
        Logger.d(TAG, "checkIfDRMFile() entry with fileName = " + fileName);
        int pos = fileName.lastIndexOf(".");
        Logger.d(TAG, "getMimeType(): pos = " + pos);
        if (pos < 0 || pos > fileName.length()) {
            return false;
        }
        final String fileNameExtension = fileName.substring(pos);
        if(fileNameExtension.equals(".dcf"))
            return true;
        return false;
    }

    /**
     * Checks if is enabled compress image.
     *
     * @return true, if is enabled compress image
     */
    public boolean isEnabledCompressImage() {
        boolean isCompress = AppSettings.getInstance()
                .isEnabledCompressingImageFromDB();
        Logger.d(TAG, "isEnabledCompressImage = " + isCompress);
        return isCompress;
    }
    /**
     * Sets the enable compress image.
     *
     * @param compressImage the new enable compress image
     */
    public void setEnableCompressImage(boolean compressImage) {
        Logger.d(TAG, "isEnabledCompressImage = " + compressImage);
        AppSettings.getInstance().setCompressingImage(compressImage);
    }
    /**
     * Sent file via rc se.
     *
     * @param filePath the file path
     * @param contact the contact
     * @param fileTransferTag the file transfer tag
     */
    public void sentFileViaRCSe(String filePath, String contact,
            ParcelUuid fileTransferTag) {
        Logger.d(TAG, "sentFileViaRCSe() filePath: " + filePath
                + " , contact: " + contact + " , fileTransferTag: "
                + fileTransferTag);
        Message controllerMessage = PluginController.obtainMessage(
                ChatController.EVENT_FILE_TRANSFER_INVITATION, contact,
                filePath);
        Bundle data = controllerMessage.getData();
        data.putParcelable(ModelImpl.SentFileTransfer.KEY_FILE_TRANSFER_TAG,
                fileTransferTag);
        controllerMessage.setData(data);
        controllerMessage.sendToTarget();
    }
    /**
     * Collect multi contact.
     *
     * @param contact the contact
     * @return the sets the
     */
    static Set<String> collectMultiContact(String contact) {
        String[] contacts = contact.split(COMMA);
        Set<String> contactSet = new TreeSet<String>();
        for (String singleContact : contacts) {
            contactSet.add(singleContact);
        }
        return contactSet;
    }
    /**
     * Resend message.
     *
     * @param msgId the msg id
     * @param simId the sim id
     */
    public void resendMessage(long msgId, int simId) {
        resendMessage(msgId);
    }
    /**
     * Resend message.
     *
     * @param msgId the msg id
     */
    public void resendMessage(long msgId) {
        Logger.d(TAG, "resendMessage() msgId + " + msgId);
        IpMessage msg = getIpMsgInfo(msgId);
        String messageTagInRcse = PluginUtils.findMessageTagInRcseDb(Long
                .toString(msgId));
        String idInRcse = getMessageIdinRcse(msgId);
        Logger.d(TAG, "resendMessage() msgId + " + msgId + "tag is"
                + messageTagInRcse + "ID is" + idInRcse);
        if (idInRcse != null && !idInRcse.equals("")) {
            if (msg instanceof IpAttachMessage) {
                Logger.d(TAG, "setIpMessageStatus() calling entry ");
                setIpMessageStatus(msgId, 11);
            }
        }
        if (msg instanceof IpTextMessage && idInRcse != null) {
            String contact = ((IpTextMessage) msg).getTo();
            Logger.d(TAG, "resendMessage() IpTextMessage entry contact is "
                    + contact);
            int id = PluginUtils.findIdInRcseDb(idInRcse);
            if (contact != null && PluginUtils.getMessagingMode() == 0) {
                if (contact.startsWith(IpMessageConsts.JOYN_START)) {
                    contact = contact.substring(4);
                }
            }
            Logger.d(TAG, "resendMessage() IpTextMessage exit contact is "
                    + contact);
            sentMessageViaRCSe(((IpTextMessage) msg).getBody(), contact, id);
        }
    }
    /**
     * Gets the ip message status string.
     *
     * @param msgId the msg id
     * @return the ip message status string
     */
    public String getIpMessageStatusString(long msgId) {
        Logger.d(TAG, "getIpMessageStatusString(), msgId = " + msgId);
        IpMessage message = getMessage(msgId);
        long fileSize;
        if (message == null) {
            Logger.e(TAG, "getIpMessageStatusString(), message is null!");
            return null;
        }
        if (message.getType() == IpMessageConsts.IpMessageType.TEXT) {
            return "";
        }
        int messageType = message.getType();
        String remote = ContactsListManager.getInstance()
                .getDisplayNameByPhoneNumber(message.getFrom());
        int transferStatus;
        int rcsStatus;
        String fileName;
        // Get file transfer tag
        switch (messageType) {
        case IpMessageConsts.IpMessageType.PICTURE:
            fileName = ((PluginIpImageMessage) message).getName();
            fileSize = ((PluginIpImageMessage) message).getSize();
            transferStatus = ((PluginIpImageMessage) message).getStatus();
            rcsStatus = ((PluginIpImageMessage) message).getRcsStatus();
            break;
        case IpMessageConsts.IpMessageType.VIDEO:
            fileName = ((PluginIpVideoMessage) message).getName();
            fileSize = ((PluginIpVideoMessage) message).getSize();
            transferStatus = ((PluginIpVideoMessage) message).getStatus();
            rcsStatus = ((PluginIpVideoMessage) message).getRcsStatus();
            break;
        case IpMessageConsts.IpMessageType.VOICE:
            fileName = ((PluginIpVoiceMessage) message).getName();
            fileSize = ((PluginIpVoiceMessage) message).getSize();
            transferStatus = ((PluginIpVoiceMessage) message).getStatus();
            rcsStatus = ((PluginIpVoiceMessage) message).getRcsStatus();
            /*
             * if (transferStatus ==
             * IpMessageConsts.IpMessageStatus.MT_RECEIVED) { if
             * (((PluginIpVoiceMessage) message).getDuration() == 0) {
             * Logger.d(TAG,
             * "getIpMessageStatusString(), need to get duration!");
             * ((PluginIpVoiceMessage) message).analysisAttribute(); Intent it =
             * new Intent();
             * it.setAction(IpMessageConsts.IpMessageStatus.ACTION_MESSAGE_STATUS
             * ); it.putExtra(IpMessageConsts.STATUS, transferStatus);
             * it.putExtra(IpMessageConsts.IpMessageStatus.IP_MESSAGE_ID,
             * msgId); IpNotificationsManager.notify(it); } }
             */
            break;
        case IpMessageConsts.IpMessageType.VCARD:
            fileName = ((PluginIpVcardMessage) message).getName();
            fileSize = ((PluginIpVcardMessage) message).getSize();
            transferStatus = ((PluginIpVcardMessage) message).getStatus();
            rcsStatus = ((PluginIpVcardMessage) message).getRcsStatus();
            break;
        case IpMessageConsts.IpMessageType.CALENDAR:
            fileName = ((PluginIpVCalendarMessage) message).getName();
            fileSize = ((PluginIpVCalendarMessage) message).getSize();
            transferStatus = ((PluginIpVCalendarMessage) message).getStatus();
            rcsStatus = ((PluginIpVCalendarMessage) message).getRcsStatus();
            break;
        default:
            fileName = ((PluginIpAttachMessage) message).getName();
            fileSize = ((PluginIpAttachMessage) message).getSize();
            transferStatus = ((PluginIpAttachMessage) message).getStatus();
            rcsStatus = ((PluginIpAttachMessage) message).getRcsStatus();
            break;
        }
        switch (transferStatus) {
        case IpMessageConsts.IpMessageStatus.MT_RECEIVING:
        case IpMessageConsts.IpMessageStatus.MT_RECEIVED: {
            if ((messageType == IpMessageConsts.IpMessageType.PICTURE)
                    || (messageType == IpMessageConsts.IpMessageType.VIDEO)) {
            return (fileSize + "KB");
            } else {
                return fileSize + "KB";
            }
        }
        case IpMessageConsts.IpMessageStatus.MO_INVITE:
        case IpMessageConsts.IpMessageStatus.MO_SENDING:
        case IpMessageConsts.IpMessageStatus.MO_SENT:
        case IpMessageConsts.IpMessageStatus.MO_DELIVERED:
        case IpMessageConsts.IpMessageStatus.MO_VIEWED:
            return fileName;
        case IpMessageConsts.IpMessageStatus.MO_REJECTED:
            return remote + SPACE + mRejected + SPACE + fileName;
        case IpMessageConsts.IpMessageStatus.MO_CANCEL:
            if (false/*!PluginUtils.isFtSupportedInRcse(message.getFrom())*/) {
                Logger.d(TAG,
                        "getIpMessageStatusString() mWarningNoFtCapability "
                                + mWarningNoFtCapability);
                return mWarningNoFtCapability;
            } else {
                Logger.d(TAG, "getIpMessageStatusString() support ft");
                return mYou + SPACE + mCanceled + SPACE + "file sized "
                        + fileSize + "KB";
            }
        case IpMessageConsts.IpMessageStatus.MT_CANCEL:
            if (rcsStatus == FAILED) {
                return "File Transfer" + SPACE + mFailed;
            } else {
                return remote + SPACE + mCanceled + SPACE + "file sized "
                        + fileSize + "KB";
            }
        case IpMessageConsts.IpMessageStatus.MT_INVITED:
            return (fileSize + "KB");
        case IpMessageConsts.IpMessageStatus.MEDIA_NOT_SUPPORTED:
            return (fileSize + SPACE + " not supported by server");
        case IpMessageConsts.IpMessageStatus.MT_REJECT:
            return mYou + SPACE + mRejected + SPACE + " file sized " + fileSize
                    + "KB";
        default:
            return null;
        }
    }
    /**
     * Generate message tag.
     *
     * @return the int
     */
    private static int generateMessageTag() {
        int messageTag = RANDOM.nextInt(MESSAGE_TAG_RANGE) + 1;
        messageTag = Integer.MAX_VALUE - messageTag;
        return messageTag;
    }

    /**
     * Use this class to be the cache when a message is in present status.
     */
    private static class PresentTextMessage extends IpTextMessage {
        /**
         * The message tag.
         */
        public int messageTag;

        /**
         * Instantiates a new present text message.
         *
         * @param chatMessageTag the chat message tag
         * @param contact the contact
         * @param text the text
         */
        private PresentTextMessage(int chatMessageTag, String contact,
                String text) {
            this.messageTag = chatMessageTag;
            this.setBody(text);
            this.setType(IpMessageConsts.IpMessageType.TEXT);
            //super.setStatus(-1);
        }
    }
}
