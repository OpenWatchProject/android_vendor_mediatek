package com.orangelabs.rcs.service.broadcaster;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.utils.IntentUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.extension.IMultimediaMessagingSessionListener;
import com.gsma.services.rcs.extension.MultimediaMessagingSessionIntent;
import com.gsma.services.rcs.extension.MultimediaSession.ReasonCode;
import com.gsma.services.rcs.extension.MultimediaSession.State;

import android.content.Intent;
import android.os.RemoteCallbackList;
import android.os.RemoteException;

/**
 * MultimediaMessagingSessionEventBroadcaster maintains the registering and unregistering of
 * IMultimediaMessagingSessionListener and also performs broadcast events on these listeners upon
 * the trigger of corresponding callbacks.
 */
public class MultimediaMessagingSessionEventBroadcaster implements
        IMultimediaMessagingSessionEventBroadcaster {

    private final RemoteCallbackList<IMultimediaMessagingSessionListener> mMultimediaMessagingListeners = new RemoteCallbackList<>();

    private final Logger logger = Logger.getLogger(getClass().getName());

    public MultimediaMessagingSessionEventBroadcaster() {
    }

    public void addMultimediaMessagingEventListener(IMultimediaMessagingSessionListener listener) {
        mMultimediaMessagingListeners.register(listener);
    }

    public void removeMultimediaMessagingEventListener(IMultimediaMessagingSessionListener listener) {
        mMultimediaMessagingListeners.unregister(listener);
    }

    @Override
    public void broadcastMessageReceived(ContactId contact, String sessionId, byte[] message, String contentType) {
        final int N = mMultimediaMessagingListeners.beginBroadcast();
        for (int i = 0; i < N; i++) {
            try {
                mMultimediaMessagingListeners.getBroadcastItem(i).onMessageReceived(contact,
                        sessionId, message);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
            try {
                mMultimediaMessagingListeners.getBroadcastItem(i).onMessageReceived2(contact,
                        sessionId, message, contentType);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mMultimediaMessagingListeners.finishBroadcast();
    }

    @Override
    public void broadcastStateChanged(ContactId contact, String sessionId, State state,
            ReasonCode reasonCode) {
        int rcsState = state.toInt();
        int rcsReasonCode = reasonCode.toInt();
        final int N = mMultimediaMessagingListeners.beginBroadcast();
        for (int i = 0; i < N; i++) {
            try {
                mMultimediaMessagingListeners.getBroadcastItem(i).onStateChanged(contact,
                        sessionId, rcsState, rcsReasonCode);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mMultimediaMessagingListeners.finishBroadcast();
    }

    @Override
    public void broadcastInvitation(String sessionId, Intent msrpSessionInvite) {
        msrpSessionInvite.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        IntentUtils.tryToSetReceiverForegroundFlag(msrpSessionInvite);
        msrpSessionInvite.putExtra(MultimediaMessagingSessionIntent.EXTRA_SESSION_ID, sessionId);

        AndroidFactory.getApplicationContext().sendBroadcast(msrpSessionInvite,
                                                             "com.gsma.services.permission.RCS");
    }

    @Override
    public void broadcastMessagesFlushed(ContactId contact, String sessionId) {
        final int N = mMultimediaMessagingListeners.beginBroadcast();
        for (int i = 0; i < N; i++) {
            try {
                mMultimediaMessagingListeners.getBroadcastItem(i).onMessagesFlushed(contact, sessionId);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mMultimediaMessagingListeners.finishBroadcast();
    }
}
