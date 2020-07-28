/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
package com.gsma.services.rcs.chat;

import com.gsma.services.rcs.ICoreServiceWrapper;
import com.gsma.services.rcs.JoynServiceConfiguration;
import com.gsma.services.rcs.Logger;
import com.gsma.services.rcs.RcsGenericException;
import com.gsma.services.rcs.RcsIllegalArgumentException;
import com.gsma.services.rcs.RcsMaxAllowedSessionLimitReachedException;
import com.gsma.services.rcs.RcsPermissionDeniedException;
import com.gsma.services.rcs.RcsPersistentStorageException;
import com.gsma.services.rcs.RcsService;
import com.gsma.services.rcs.RcsServiceControl;
import com.gsma.services.rcs.RcsServiceException;
import com.gsma.services.rcs.RcsServiceListener;
import com.gsma.services.rcs.RcsServiceListener.ReasonCode;
import com.gsma.services.rcs.RcsServiceNotAvailableException;
import com.gsma.services.rcs.RcsServiceNotRegisteredException;
import com.gsma.services.rcs.capability.ICapabilityService;
import com.gsma.services.rcs.contact.ContactId;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.IInterface;
import android.os.RemoteException;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Map;
import java.util.Set;
import java.util.WeakHashMap;

/**
 * Chat service offers the main entry point to initiate chat 1-1 ang group
 * conversations with contacts. Several applications may connect/disconnect
 * to the API.
 *
 * The parameter contact in the API supports the following formats:
 * MSISDN in national or international format, SIP address, SIP-URI
 * or Tel-URI.
 *
 * @author Jean-Marc AUFFRET
 */
public class ChatService extends RcsService {
    /**
     * API
     */
    private IChatService mApi;

    private final Map<OneToOneChatListener, WeakReference<IOneToOneChatListener>> mOneToOneChatListeners = new WeakHashMap<>();
    private final Map<ExtendChatListener, WeakReference<IExtendChatListener>> mExtendChatListeners = new WeakHashMap<>();
    private final Map<GroupChatListener, WeakReference<IGroupChatListener>> mGroupChatListeners = new WeakHashMap<>();

    private static boolean sApiCompatible = false;

    private Context mContext = null;

    public static final String TAG = "ChatService";

    public static final ComponentName DEFAULT_IMPL_COMPONENT =
            new ComponentName("com.orangelabs.rcs", "com.orangelabs.rcs.service.RcsCoreService");

    /**
     * Constructor
     *
     * @param ctx Application context
     * @param listener Service listener
     */
    public ChatService(Context ctx, RcsServiceListener listener) {
        super(ctx, listener);
        mContext = ctx;
    }

    /**
     * Connects to the API
     *
     * @throws RcsPermissionDeniedException
     */
    public final void connect() {
        /*if (!sApiCompatible) {
            try {
                sApiCompatible = mRcsServiceControl.isCompatible(this);
                if (!sApiCompatible) {
                    throw new RcsPermissionDeniedException(
                            "The TAPI client version of the chat service is not compatible with the TAPI service implementation version on this device!");
                }
            } catch (RcsServiceException e) {
                throw new RcsPermissionDeniedException(
                        "The compatibility of TAPI client version with the TAPI service implementation version of this device cannot be checked for the chat service!",
                        e);
            }
        }
        Intent serviceIntent = new Intent(IChatService.class.getName());
        serviceIntent.setPackage(RcsServiceControl.RCS_STACK_PACKAGENAME);
        mCtx.bindService(serviceIntent, apiConnection, 0);*/
        Logger.i(TAG, "connect entry");
        Intent intent = new Intent();
        ComponentName cmp = DEFAULT_IMPL_COMPONENT;
        intent.setComponent(cmp);
        intent.setAction(IChatService.class.getName());
        mContext.bindService(intent, apiConnection, 0);
    }

    /**
     * Disconnects from the API
     */
    public void disconnect() {
        try {
            Logger.i(TAG, "disconnect entry");
            mCtx.unbindService(apiConnection);
        } catch (IllegalArgumentException e) {
            Logger.i(TAG, "disconnect exception");
            e.printStackTrace();
        }
    }

    /**
     * Set API interface
     *
     * @param api API interface
     * @hide
     */
    protected void setApi(IInterface api) {
        super.setApi(api);
        mApi = (IChatService) api;
    }

    /**
     * Service connection
     */
    private ServiceConnection apiConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            ICoreServiceWrapper mCoreServiceWrapperBinder = ICoreServiceWrapper.Stub.asInterface(service);
            IBinder binder = null;
            try {
                binder = mCoreServiceWrapperBinder.getChatServiceBinder();
            } catch (RemoteException e1) {
                Logger.i(TAG, "onServiceConnected exception");
                e1.printStackTrace();
            }
            setApi(IChatService.Stub.asInterface(binder));
            if (mListener != null) {
                mListener.onServiceConnected();
            }
        }

        public void onServiceDisconnected(ComponentName className) {
            Logger.i(TAG, "onServiceDisconnected entry");
            setApi(null);
            if (mListener == null) {
                return;
            }
            ReasonCode reasonCode = ReasonCode.CONNECTION_LOST;
            try {
                if (!mRcsServiceControl.isActivated()) {
                    reasonCode = ReasonCode.SERVICE_DISABLED;
                }
            } catch (RcsServiceException e) {
                Logger.i(TAG, "onServiceDisconnected exception");
            }
            mListener.onServiceDisconnected(reasonCode);
        }
    };

    /**
     * Returns the configuration of the chat service
     *
     * @return ChatServiceConfiguration
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public ChatServiceConfiguration getConfiguration() throws RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "getConfiguration entry");
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            return new ChatServiceConfiguration(mApi.getConfiguration());

        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }

    /**
     * Initiates a group chat with a group of contact and returns a GroupChat instance. The subject
     * is optional and may be null.
     *
     * @param contacts Set of contact identifiers
     * @param subject The subject is optional and may be null
     * @return GroupChat instance
     * @throws RcsPermissionDeniedException
     * @throws RcsServiceNotRegisteredException
     * @throws RcsMaxAllowedSessionLimitReachedException
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public GroupChat initiateGroupChat(Set<ContactId> contacts, String subject)
            throws RcsPermissionDeniedException, RcsServiceNotRegisteredException,
            RcsMaxAllowedSessionLimitReachedException, RcsPersistentStorageException,
            RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "initiateGroupChat entry: subject " + subject);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            IGroupChat chatIntf = mApi.initiateGroupChat(new ArrayList<>(contacts), subject);
            return new GroupChat(chatIntf);

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPermissionDeniedException.assertException(e);
            RcsServiceNotRegisteredException.assertException(e);
            RcsMaxAllowedSessionLimitReachedException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Initiates a closed group chat with a group of contact and returns a GroupChat instance. The subject
     * is optional and may be null.
     *
     * @param contacts Set of contact identifiers
     * @param subject The subject is optional and may be null
     * @return GroupChat instance
     * @throws RcsPermissionDeniedException
     * @throws RcsServiceNotRegisteredException
     * @throws RcsMaxAllowedSessionLimitReachedException
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public GroupChat initiateClosedGroupChat(Set<ContactId> contacts, String subject)
            throws RcsPermissionDeniedException, RcsServiceNotRegisteredException,
            RcsMaxAllowedSessionLimitReachedException, RcsPersistentStorageException,
            RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "initiateClosedGroupChat entry subject: " + subject);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            IGroupChat chatIntf = mApi.initiateClosedGroupChat(new ArrayList<ContactId>(contacts), subject);
            return new GroupChat(chatIntf);

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPermissionDeniedException.assertException(e);
            RcsServiceNotRegisteredException.assertException(e);
            RcsMaxAllowedSessionLimitReachedException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns a chat with a given contact
     *
     * @param contact ContactId
     * @return OneToOneChat
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public OneToOneChat getOneToOneChat(ContactId contact) throws RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "getOneToOneChat contact: " + contact.toString());
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            return new OneToOneChat(mApi.getOneToOneChat(contact));

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns a chat with a given contact
     *
     * @param contact ContactId
     * @return OneToOneChat
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public ExtendChat getExtendO2OChat(ContactId contact) throws RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "getExtendO2OChat contact: " + contact.toString());
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            return new ExtendChat(mApi.getExtendChat(contact));

        } catch (Exception e) {
            e.printStackTrace();
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns a chat with a set of contacts
     *
     * @param contacts Set
     * @return ExtendChat
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public ExtendChat getExtendMultiChat(Set<String> contacts) throws
      RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "getExtendMultiChat contacts: " + contacts.size());
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            return new ExtendChat(mApi.getExtendMultiChat(new ArrayList<String>(contacts)));

        } catch (Exception e) {
            e.printStackTrace();
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns a group chat from its unique ID. An exception is thrown if the chat ID does not exist
     *
     * @param chatId Chat ID
     * @return GroupChat
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public GroupChat getGroupChat(String chatId) throws RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "getGroupChat chatId: " + chatId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            return new GroupChat(mApi.getGroupChat(chatId));

        } catch (Exception e) {
            e.printStackTrace();
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns true if it is possible to initiate a new group chat now else returns false.
     *
     * @return boolean
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public boolean isAllowedToInitiateGroupChat() throws RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "isAllowedToInitiateGroupChat entry");
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            return mApi.isAllowedToInitiateGroupChat();

        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns true if it's possible to initiate a new group chat with the specified contactId right
     * now, else returns false.
     *
     * @param contact the remote contact
     * @return boolean
     * @throws RcsPersistentStorageException
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public boolean isAllowedToInitiateGroupChat(ContactId contact)
            throws RcsPersistentStorageException, RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "isAllowedToInitiateGroupChat entry");
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            return mApi.isAllowedToInitiateGroupChat2(contact);

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Deletes all one to one chat from history and abort/reject any associated ongoing session if
     * such exists.
     *
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void deleteOneToOneChats() throws RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "deleteOneToOneChats entry");
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.deleteOneToOneChats();
        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }

    /**
     * Deletes all group chat from history and abort/reject any associated ongoing session if such
     * exists.
     *
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void deleteGroupChats() throws RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "deleteGroupChats entry");
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.deleteGroupChats();
        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }

    /**
     * Deletes a one to one chat with a given contact from history and abort/reject any associated
     * ongoing session if such exists.
     *
     * @param contact the remote contact
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void deleteOneToOneChat(ContactId contact) throws RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "deleteOneToOneChat entry contact: " + contact.toString());
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.deleteOneToOneChat(contact);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Delete a group chat by its chat id from history and abort/reject any associated ongoing
     * session if such exists.
     *
     * @param chatId the chat ID
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void deleteGroupChat(String chatId) throws RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "deleteGroupChat entry chatId: " + chatId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.deleteGroupChat(chatId);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Delete a message from its message id from history.
     *
     * @param msgId the message ID
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void deleteMessage(String msgId) throws RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "deleteMessage entry msgId: " + msgId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.deleteMessage(msgId);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Disables and clears any delivery expiration for a set of chat messages regardless if the
     * delivery of them has expired already or not.
     *
     * @param msgIds the message IDs
     * @throws RcsServiceNotAvailableException
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public void clearMessageDeliveryExpiration(Set<String> msgIds)
            throws RcsServiceNotAvailableException, RcsPersistentStorageException,
            RcsGenericException {
        Logger.i(TAG, "clearMessageDeliveryExpiration entry msgId: " + msgIds.size());
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.clearMessageDeliveryExpiration(new ArrayList<String>(msgIds));
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Mark a received message as read (ie. displayed in the UI)
     *
     * @param msgId Message id
     * @throws RcsServiceNotAvailableException
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public void markMessageAsRead(String msgId) throws RcsServiceNotAvailableException,
            RcsPersistentStorageException, RcsGenericException {
        Logger.i(TAG, "markMessageAsRead entry msgId: " + msgId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            mApi.markMessageAsRead(msgId);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Adds a listener on group chat events
     *
     * @param listener Group chat listener
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void addEventListener(GroupChatListener listener)
            throws RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "addEventListener GroupChatListener " + listener);
        if (listener == null) {
            throw new RcsIllegalArgumentException("listener must not be null!");
        }
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            IGroupChatListener rcsListener = new GroupChatListenerImpl(listener);
            mGroupChatListeners.put(listener, new WeakReference<>(rcsListener));
            mApi.addEventListener3(rcsListener);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Removes a listener on group chat events
     *
     * @param listener Group chat event listener
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void removeEventListener(GroupChatListener listener)
            throws RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "removeEventListener GroupChatListener " + listener);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            WeakReference<IGroupChatListener> weakRef = mGroupChatListeners.remove(listener);
            if (weakRef == null) {
                return;
            }
            IGroupChatListener rcsListener = weakRef.get();
            if (rcsListener != null) {
                mApi.removeEventListener3(rcsListener);
            }
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Adds a listener for one-to-one chat events
     *
     * @param listener One-to-one chat listener
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void addEventListener(OneToOneChatListener listener)
            throws RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "addEventListener OneToOneChatListener " + listener);
        if (listener == null) {
            throw new RcsIllegalArgumentException("listener must not be null!");
        }
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            IOneToOneChatListener rcsListener = new OneToOneChatListenerImpl(listener);
            mOneToOneChatListeners.put(listener, new WeakReference<>(rcsListener));
            mApi.addEventListener2(rcsListener);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Removes a listener for one-to-one chat events
     *
     * @param listener One-to-one chat listener
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void removeEventListener(OneToOneChatListener listener)
            throws RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "removeEventListener OneToOneChatListener " + listener);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            WeakReference<IOneToOneChatListener> weakRef = mOneToOneChatListeners.remove(listener);
            if (weakRef == null) {
                return;
            }
            IOneToOneChatListener rcsListener = weakRef.get();
            if (rcsListener != null) {
                mApi.removeEventListener2(rcsListener);
            }
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Adds a listener for one-to-one chat events
     *
     * @param listener One-to-one chat listener
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void addExtendEventListener(ExtendChatListener listener)
            throws RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "addExtendEventListener ExtendChatListener " + listener);
        if (listener == null) {
            throw new RcsIllegalArgumentException("listener must not be null!");
        }
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            IExtendChatListener rcsListener = new ExtendChatListenerImpl(listener);
            mExtendChatListeners.put(listener, new WeakReference<>(rcsListener));
            mApi.addEventListenerExtend2(rcsListener);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Removes a listener for one-to-one chat events
     *
     * @param listener One-to-one chat listener
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public void removeExtendEventListener(ExtendChatListener listener)
            throws RcsServiceNotAvailableException, RcsGenericException {
        Logger.i(TAG, "removeExtendEventListener ExtendChatListener " + listener);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            WeakReference<IExtendChatListener> weakRef = mExtendChatListeners.remove(listener);
            if (weakRef == null) {
                return;
            }
            IExtendChatListener rcsListener = weakRef.get();
            if (rcsListener != null) {
                mApi.removeEventListenerExtend2(rcsListener);
            }
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns a chat message from its unique ID
     *
     * @param msgId Message id
     * @return ChatMessage
     * @throws RcsServiceNotAvailableException
     * @throws RcsGenericException
     */
    public ChatMessage getChatMessage(String msgId) throws RcsServiceNotAvailableException,
            RcsGenericException {
        Logger.i(TAG, "getChatMessage msgId " + msgId);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        try {
            return new ChatMessage(mApi.getChatMessage(msgId));

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Block messages in group, stack will not notify application about
     * any received message in this group
     *
     * @param chatId chatId of the group
     * @param flag true means block the message, false means unblock it
     * @throws JoynServiceException
     */
    public void blockGroupMessages(String chatId, boolean flag) throws RcsServiceNotAvailableException,
               RcsGenericException {
        Logger.i(TAG, "blockGroupMessages() entry with chatId: " + chatId + ",flag:" + flag);
        if (mApi == null) {
            throw new RcsServiceNotAvailableException();
        }
        if (mApi != null) {
            try {
                mApi.blockGroupMessages(chatId, flag);
            } catch (Exception e) {
                RcsIllegalArgumentException.assertException(e);
                throw new RcsGenericException(e);
            }
        }
    }
}
