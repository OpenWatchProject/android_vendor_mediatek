package com.orangelabs.rcs.service.broadcaster;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.utils.IntentUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.extension.IMultimediaStreamingSessionListener;
import com.gsma.services.rcs.extension.MultimediaMessagingSessionIntent;
import com.gsma.services.rcs.extension.MultimediaSession.ReasonCode;
import com.gsma.services.rcs.extension.MultimediaSession.State;

import android.content.Intent;
import android.os.RemoteCallbackList;
import android.os.RemoteException;

/**
 * MultimediaStreamingSessionEventBroadcaster maintains the registering and unregistering of
 * IMultimediaStreamingSessionListener and also performs broadcast events on these listeners upon
 * the trigger of corresponding callbacks.
 */
public class MultimediaStreamingSessionEventBroadcaster implements
        IMultimediaStreamingSessionEventBroadcaster {

    private final RemoteCallbackList<IMultimediaStreamingSessionListener> mMultimediaStreamingListeners = new RemoteCallbackList<>();

    private final Logger logger = Logger.getLogger(getClass().getName());

    public MultimediaStreamingSessionEventBroadcaster() {
    }

    public void addMultimediaStreamingEventListener(IMultimediaStreamingSessionListener listener) {
        mMultimediaStreamingListeners.register(listener);
    }

    public void removeMultimediaStreamingEventListener(IMultimediaStreamingSessionListener listener) {
        mMultimediaStreamingListeners.unregister(listener);
    }

    @Override
    public void broadcastPayloadReceived(ContactId contact, String sessionId, byte[] content) {
        final int N = mMultimediaStreamingListeners.beginBroadcast();
        for (int i = 0; i < N; i++) {
            try {
                mMultimediaStreamingListeners.getBroadcastItem(i).onPayloadReceived(contact,
                        sessionId, content);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mMultimediaStreamingListeners.finishBroadcast();
    }

    @Override
    public void broadcastStateChanged(ContactId contact, String sessionId, State state,
            ReasonCode reasonCode) {
        int rcsState = state.toInt();
        int rcsReasonCode = reasonCode.toInt();
        final int N = mMultimediaStreamingListeners.beginBroadcast();
        for (int i = 0; i < N; i++) {
            try {
                mMultimediaStreamingListeners.getBroadcastItem(i).onStateChanged(contact,
                        sessionId, rcsState, rcsReasonCode);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mMultimediaStreamingListeners.finishBroadcast();
    }

    @Override
    public void broadcastInvitation(String sessionId, Intent rtpSessionInvite) {
        rtpSessionInvite.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        IntentUtils.tryToSetReceiverForegroundFlag(rtpSessionInvite);
        rtpSessionInvite.putExtra(MultimediaMessagingSessionIntent.EXTRA_SESSION_ID, sessionId);

        AndroidFactory.getApplicationContext().sendBroadcast(rtpSessionInvite,
                                                             "com.gsma.services.permission.RCS");
    }
}
