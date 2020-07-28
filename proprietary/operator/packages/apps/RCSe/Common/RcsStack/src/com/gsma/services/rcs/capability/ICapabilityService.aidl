package com.gsma.services.rcs.capability;

import java.util.List;

import com.gsma.services.rcs.IJoynServiceRegistrationListener;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.capability.ICapabilitiesListener;
import com.gsma.services.rcs.contact.ContactId;

/**
 * Capability service API
 */
interface ICapabilityService {
    boolean isServiceRegistered(int slotId);

    Capabilities getMyCapabilities(int slotId);

    Capabilities getContactCapabilities(in ContactId contact);

    void requestContactCapabilities(in ContactId contact);

    void requestAllContactsCapabilities();

    void addCapabilitiesListener(in ICapabilitiesListener listener);

    void removeCapabilitiesListener(in ICapabilitiesListener listener);

    void addContactCapabilitiesListener(in ContactId contact, in ICapabilitiesListener listener);

    void removeContactCapabilitiesListener(in ContactId contact, in ICapabilitiesListener listener);

    int getServiceVersion();

    // MTK added for OP07
    void forceRequestContactCapabilities(in ContactId contact);

    // MTK added for OP07
    void requestContactsCapabilities(in List<ContactId> contacts);

    // MTK added for OP12
    void requestContactAvailability(in ContactId contact);

    boolean publishMyCap(int slotId, boolean isModify, in Capabilities caps);
}
