package com.orangelabs.rcs.service.broadcaster;

import com.gsma.services.rcs.RcsServiceRegistration.ReasonCode;

/**
 * Interface to perform broadcast events on RcsServiceRegistrationListeners
 */
public interface IRcsServiceRegistrationEventBroadcaster {

    void broadcastServiceRegistered();

    void broadcastServiceUnRegistered(ReasonCode reasonCode);
}