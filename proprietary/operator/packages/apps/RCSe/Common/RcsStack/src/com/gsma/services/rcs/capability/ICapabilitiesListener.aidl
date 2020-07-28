package com.gsma.services.rcs.capability;

import com.gsma.services.rcs.capability.Capabilities;

/**
 * Callback method for new capabilities
 */
interface ICapabilitiesListener {
	void onCapabilitiesReceived(in String contact, in Capabilities capabilities);
    void onErrorReceived(in String contact, in int type, in int status, in String reason);
}