package com.orangelabs.rcs.service.broadcaster;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.extension.MultimediaSession.ReasonCode;
import com.gsma.services.rcs.extension.MultimediaSession.State;

import android.content.Intent;

/**
 * Interface to perform broadcast events on MultimediaMessagingSessionListeners
 */
public interface IMultimediaMessagingSessionEventBroadcaster {

    void broadcastMessageReceived(ContactId contact, String sessionId, byte[] message,
            String contentType);

    void broadcastStateChanged(ContactId contact, String sessionId, State state,
            ReasonCode reasonCode);

    void broadcastInvitation(String sessionId, Intent msrpSessionInvite);

    void broadcastMessagesFlushed(ContactId contact, String sessionId);
}
