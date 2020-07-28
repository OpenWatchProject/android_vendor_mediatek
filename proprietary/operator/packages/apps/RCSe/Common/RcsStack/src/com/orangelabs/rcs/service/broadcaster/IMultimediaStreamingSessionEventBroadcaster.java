package com.orangelabs.rcs.service.broadcaster;

import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.extension.MultimediaSession.ReasonCode;
import com.gsma.services.rcs.extension.MultimediaSession.State;

import android.content.Intent;

/**
 * Interface to perform broadcast events on MultimediaStreamingSessionListeners
 */
public interface IMultimediaStreamingSessionEventBroadcaster {

    void broadcastPayloadReceived(ContactId contact, String sessionId, byte[] content);

    void broadcastStateChanged(ContactId contact, String sessionId, State state,
            ReasonCode reasonCode);

    void broadcastInvitation(String sessionId, Intent rtpSessionInvite);
}
