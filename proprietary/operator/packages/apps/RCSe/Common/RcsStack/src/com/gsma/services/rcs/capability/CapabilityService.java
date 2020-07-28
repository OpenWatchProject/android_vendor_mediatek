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

package com.gsma.services.rcs.capability;

import java.util.Iterator;
import java.util.List;
import java.util.Set;

import com.gsma.services.rcs.ICoreServiceWrapper;
import com.gsma.services.rcs.JoynContactFormatException;
import com.gsma.services.rcs.JoynService;
import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.JoynServiceListener;
import com.gsma.services.rcs.JoynServiceConfiguration;
import com.gsma.services.rcs.JoynServiceNotAvailableException;
import com.gsma.services.rcs.JoynServiceRegistrationListener;
import com.gsma.services.rcs.RcsService;
import com.gsma.services.rcs.RcsServiceException;
import com.gsma.services.rcs.RcsServiceListener;
import com.gsma.services.rcs.RcsServiceNotAvailableException;
import com.gsma.services.rcs.RcsServiceListener.ReasonCode;

import com.gsma.services.rcs.Logger;
import com.gsma.services.rcs.contact.ContactId;



import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.IInterface;
import android.os.RemoteException;
import android.telephony.SubscriptionManager;


/**
 * Capability service offers the main entry point to read capabilities
 * of remote contacts, to initiate capability discovery and to receive
 * capabilities updates. Several applications may connect/disconnect
 * to the API.
 *
 * The parameter contact in the API supports the following formats:
 * MSISDN in national or international format, SIP address, SIP-URI
 * or Tel-URI.
 *
 * @author Jean-Marc AUFFRET
 */
public class CapabilityService extends RcsService {
    /**
     * Intent broadcasted to discover extensions
     *
     * @see CapabilityService.EXTENSION_MIME_TYPE
     */
    public final static String INTENT_EXTENSIONS = "com.gsma.services.rcs.capability.EXTENSION";

    /**
     * Extension MIME type
     */
    public final static String EXTENSION_MIME_TYPE = "com.gsma.services.rcs";

    /**
     * API
     */
    private ICapabilityService api = null;

    public static final String TAG = "CapabilityService";

    private Context mContext = null;

    public static final ComponentName DEFAULT_IMPL_COMPONENT =
        new ComponentName("com.orangelabs.rcs", "com.orangelabs.rcs.service.RcsCoreService");
    public static final ComponentName STANDALONE_PRESENCE_IMPL_COMPONENT =
        new ComponentName("com.mediatek.presence", "com.mediatek.presence.service.RcsCoreService");

    /**
     * Constructor
     *
     * @param ctx Application context
     * @param listener Service listener
     */
    public CapabilityService(Context ctx, RcsServiceListener listener) {
        super(ctx, listener);
        mContext = ctx;
    }

    /**
     * Connects to the API
     */
    public void connect() {
        /*if (this.ctx.checkCallingOrSelfPermission(Permissions.RCS_READ_CAPABILITIES) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(" Required permission RCS_READ_CAPABILITIES");
        }*/
        /*if (this.ctx.checkCallingOrSelfPermission(Permissions.ANDROID_READ_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
            throw new SecurityException(" Required permission ANDROID_READ_CONTACTS");
        }*/
        Logger.i(TAG, "connect() entry");
        Intent intent = new Intent();
        ComponentName cmp;
        if (JoynServiceConfiguration.isPresenceDiscoverySupported(mContext))
            cmp = STANDALONE_PRESENCE_IMPL_COMPONENT;
        else
            cmp = DEFAULT_IMPL_COMPONENT;
        intent.setComponent(cmp);
        intent.setAction(ICapabilityService.class.getName());
        mContext.bindService(intent, apiConnection, 0);

    }

    /**
     * Disconnects from the API
     */
    public void disconnect() {
        try {
            Logger.i(TAG, "disconnect() entry");
            mContext.unbindService(apiConnection);
        } catch (IllegalArgumentException e) {
            // Nothing to do
        }
    }

    /**
     * Set API interface
     *
     * @param api API interface
     */
    protected void setApi(IInterface api) {
        super.setApi(api);
        Logger.i(TAG, "setApi entry" + api);
        this.api = (ICapabilityService) api;
    }

    /**
     * Service connection
     */
    private ServiceConnection apiConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            ICoreServiceWrapper mCoreServiceWrapperBinder = ICoreServiceWrapper.Stub.asInterface(service);
            IBinder binder = null;
            try {
                binder = mCoreServiceWrapperBinder.getCapabilitiesServiceBinder();
            setApi(ICapabilityService.Stub.asInterface(binder));
            if (mListener != null) {
                mListener.onServiceConnected();
            }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        public void onServiceDisconnected(ComponentName className) {
            Logger.i(TAG, "onServiceDisconnected entry");
            if (mListener == null) {
                setApi(null);
                return;
            }
            ReasonCode reasonCode = ReasonCode.CONNECTION_LOST;
            try {
                if (!mRcsServiceControl.isActivated()) {
                    reasonCode = ReasonCode.SERVICE_DISABLED;
                }
            } catch (RcsServiceException e) {
                // Do nothing
            }
            mListener.onServiceDisconnected(reasonCode);
            setApi(null);
        }
    };

    /**
     * Returns the capabilities supported by the local end user. The supported
     * capabilities are fixed by the MNO and read during the provisioning.
     *
     * @return Capabilities
     * @throws RcsServiceException
     */
    public Capabilities getMyCapabilities() throws RcsServiceException {
        Logger.i(TAG, "getMyCapabilities entry");
        if (api != null) {
            try {
                int subId = SubscriptionManager.getDefaultSubscriptionId();
                int slotId = SubscriptionManager.getSlotIndex(subId);
                if (slotId == SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
                    slotId = 0;
                }
                return api.getMyCapabilities(slotId);
            } catch (Exception e) {
                throw new RcsServiceException(e.getMessage());
            }
        } else {
            throw new RcsServiceNotAvailableException();
        }
    }

    /**
     * publish capability
     *
     * @param isModify boolean
     * @param caps Capabilities
     * @return boolean
     * @throws RcsServiceException
     */
    public boolean publishMyCap(boolean isModify, Capabilities caps) throws RcsServiceException {
        Logger.i(TAG, "publishMyCap entry");
        if (api != null) {
            try {
                int subId = SubscriptionManager.getDefaultSubscriptionId();
                int slotId = SubscriptionManager.getSlotIndex(subId);
                if (slotId == SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
                    slotId = 0;
                }
                return api.publishMyCap(slotId, isModify, caps);
            } catch (Exception e) {
                throw new RcsServiceException(e.getMessage());
            }
        } else {
            throw new RcsServiceNotAvailableException();
        }
    }


    /**
     * Returns the capabilities of a given contact from the local database. This
     * method doesnt request any network update to the remote contact. The parameter
     * contact supports the following formats: MSISDN in national or international
     * format, SIP address, SIP-URI or Tel-URI. If the format of the contact is not
     * supported an exception is thrown.
     *
     * @param contact Contact
     * @return Capabilities
     * @throws JoynServiceException
     * @throws JoynContactFormatException
     *//*
    public Capabilities getContactCapabilities(String contact) throws JoynServiceException, JoynContactFormatException {
        Logger.i(TAG, "getContactCapabilities entry" + contact);
        if (api != null) {
            try {
                return api.getContactCapabilities(contact);
            } catch (Exception e) {
                throw new JoynServiceException(e.getMessage());
            }
        } else {
            throw new JoynServiceNotAvailableException();
        }
    }*/
    /**
     * Returns the capabilities of a given contact from the local database. This
     * method doesnt request any network update to the remote contact. The parameter
     * contact supports the following formats: MSISDN in national or international
     * format, SIP address, SIP-URI or Tel-URI. If the format of the contact is not
     * supported an exception is thrown.
     *
     * @param ContactId contact
     * @return Capabilities
     * @throws RcsServiceException
     * @throws JoynContactFormatException
     */
    public Capabilities getContactCapabilities(ContactId contact) throws RcsServiceException, RcsServiceNotAvailableException {
        Logger.i(TAG, "getContactCapabilities entry" + contact);
        if (api != null) {
            try {
                return api.getContactCapabilities(contact);
            } catch (Exception e) {
                throw new RcsServiceException(e.getMessage());
            }
        } else {
            throw new RcsServiceNotAvailableException();
        }
    }

    /**
     * Requests capabilities to a remote contact. This method initiates in background
     * a new capability request to the remote contact by sending a SIP OPTIONS. The
     * result of the capability request is sent asynchronously via callback method of
     * the capabilities listener. A capability refresh is only sent if the timestamp
     * associated to the capability has expired (the expiration value is fixed via MNO
     * provisioning). The parameter contact supports the following formats: MSISDN in
     * national or international format, SIP address, SIP-URI or Tel-URI. If the format
     * of the contact is not supported an exception is thrown. The result of the
     * capability refresh request is provided to all the clients that have registered
     * the listener for this event.
     *
     * @param contact Contact
     * @throws JoynServiceException
     * @throws JoynContactFormatException
     *//*
    public void requestContactCapabilities(String contact) throws JoynServiceException, JoynContactFormatException {
        Logger.i(TAG, "requestContactCapabilities entry" + contact);
        if (api != null) {
            try {
                api.requestContactCapabilities(contact);
            } catch (Exception e) {
                throw new JoynServiceException(e.getMessage());
            }
        } else {
            throw new JoynServiceNotAvailableException();
        }
    }*/


    /**
     * Requests capabilities to a remote contact. This method initiates in background
     * a new capability request to the remote contact by sending a SIP OPTIONS. The
     * result of the capability request is sent asynchronously via callback method of
     * the capabilities listener. A capability refresh is only sent if the timestamp
     * associated to the capability has expired (the expiration value is fixed via MNO
     * provisioning). The parameter contact supports the following formats: MSISDN in
     * national or international format, SIP address, SIP-URI or Tel-URI. If the format
     * of the contact is not supported an exception is thrown. The result of the
     * capability refresh request is provided to all the clients that have registered
     * the listener for this event.
     *
     * @param ContactId Contact
     * @throws JoynServiceException
     * @throws JoynContactFormatException
     */
    public void requestContactCapabilities(ContactId contact) throws RcsServiceException, RcsServiceNotAvailableException {
        Logger.i(TAG, "requestContactCapabilities entry" + contact);
        if (api != null) {
            try {
                api.requestContactCapabilities(contact);
            } catch (Exception e) {
                throw new RcsServiceException(e.getMessage());
            }
        } else {
            throw new RcsServiceNotAvailableException();
        }
    }

   /**
     * MTK added function
     * request availability for a remote contact.
     *
     * @param contact Contact
     * @throws JoynServiceException
     * @throws JoynContactFormatException
     */
    public void requestContactAvailability(ContactId contact) throws RcsServiceException, RcsServiceNotAvailableException {
        Logger.i(TAG, "requestContactAvailability entry" + contact);
        if (api != null) {
            try {
                api.requestContactAvailability(contact);
            } catch (Exception e) {
                throw new RcsServiceException(e.getMessage());
            }
        } else {
            throw new RcsServiceNotAvailableException();
        }
    }

    /**
     * Requests capabilities for a group of remote contacts. This method initiates
     * in background new capability requests to the remote contact by sending a
     * SIP OPTIONS. The result of the capability request is sent asynchronously via
     * callback method of the capabilities listener. A capability refresh is only
     * sent if the timestamp associated to the capability has expired (the expiration
     * value is fixed via MNO provisioning). The parameter contact supports the
     * following formats: MSISDN in national or international format, SIP address,
     * SIP-URI or Tel-URI. If the format of the contact is not supported an exception
     * is thrown. The result of the capability refresh request is provided to all the
     * clients that have registered the listener for this event.
     *
     * @param contacts List of contacts
     * @throws JoynServiceException
     * @throws JoynContactFormatException
     *//*
    public void requestContactCapabilities(Set<String> contacts) throws JoynServiceException, JoynContactFormatException {
        Logger.i(TAG, "requestContactCapabilities entry" + contacts);
        Iterator<String> values = contacts.iterator();
        while (values.hasNext()) {
            requestContactCapabilities(values.next());
        }
    }*/

    /**
     * Requests capabilities for a group of remote contacts. This method initiates
     * in background new capability requests to the remote contact by sending a
     * SIP OPTIONS. The result of the capability request is sent asynchronously via
     * callback method of the capabilities listener. A capability refresh is only
     * sent if the timestamp associated to the capability has expired (the expiration
     * value is fixed via MNO provisioning). The parameter contact supports the
     * following formats: MSISDN in national or international format, SIP address,
     * SIP-URI or Tel-URI. If the format of the contact is not supported an exception
     * is thrown. The result of the capability refresh request is provided to all the
     * clients that have registered the listener for this event.
     *
     * @param contacts List of contacts
     * @throws JoynServiceException
     * @throws JoynContactFormatException
     */
    public void requestContactCapabilities(Set<ContactId> contacts) throws RcsServiceException, RcsServiceNotAvailableException {
        Logger.i(TAG, "requestContactCapabilities entry" + contacts);
        Iterator<ContactId> values = contacts.iterator();
        while (values.hasNext()) {
            requestContactCapabilities(values.next());
        }
    }

    /**
     * Requests capabilities for all contacts existing in the local address book. This
     * method initiates in background new capability requests for each contact of the
     * address book by sending SIP OPTIONS. The result of a capability request is sent
     * asynchronously via callback method of the capabilities listener. A capability
     * refresh is only sent if the timestamp associated to the capability has expired
     * (the expiration value is fixed via MNO provisioning). The result of the capability
     * refresh request is provided to all the clients that have registered the listener
     * for this event.
     *
     * @param contacts List of contacts
     * @throws JoynServiceException
     */
    public void requestAllContactsCapabilities() throws RcsServiceException {
        Logger.i(TAG, "requestAllContactsCapabilities entry");
        if (api != null) {
            try {
                api.requestAllContactsCapabilities();
            } catch (Exception e) {
                throw new RcsServiceException(e.getMessage());
            }
        } else {
            throw new RcsServiceNotAvailableException();
        }
    }

    /**
     * Returns service version
     *
     * @return Version
     * @see Build.VERSION_CODES
     * @throws JoynServiceException
     *//*
    public int getServiceVersion() throws JoynServiceException {
        Logger.i(TAG, "getServiceVersion entry");
        if (api != null) {
            if (version == null) {
                try {
                    version = api.getServiceVersion();
                } catch (Exception e) {
                    throw new JoynServiceException(e.getMessage());
                }
            }
            Logger.i(TAG, "getServiceVersion is" + version);
            return version;
        } else {
            throw new JoynServiceNotAvailableException();
        }
    }*/

    /**
     * Returns true if the service is registered to the platform, else returns
     * false
     *
     * @return Returns true if registered else returns false
     * @throws JoynServiceException
     *//*
    public boolean isServiceRegistered() throws RcsServiceException {
        Logger.i(TAG, "isServiceRegistered entry");
        if (api != null) {
            boolean serviceStatus = false;
            try {
                serviceStatus = api.isServiceRegistered();
            } catch (Exception e) {
                throw new RcsServiceException(e.getMessage());
            }
            Logger.i(TAG, "isServiceRegistered" + serviceStatus);
            return serviceStatus;
        } else {
            throw new RcsServiceNotAvailableException();
        }
    }*/

    /**
     * Registers a capabilities listener on any contact
     *
     * @param listener Capabilities listener
     * @throws JoynServiceException
     */
    public void addCapabilitiesListener(CapabilitiesListener listener) throws JoynServiceException {
        Logger.i(TAG, "addCapabilitiesListener entry" + listener);
        if (api != null) {
            try {
                api.addCapabilitiesListener(listener);
            } catch (Exception e) {
                throw new JoynServiceException(e.getMessage());
            }
        } else {
            throw new JoynServiceNotAvailableException();
        }
    }

    /**
     * Unregisters a capabilities listener
     *
     * @param listener Capabilities listener
     * @throws JoynServiceException
     */
    public void removeCapabilitiesListener(CapabilitiesListener listener) throws JoynServiceException {
        Logger.i(TAG, "removeCapabilitiesListener entry" + listener);
        if (api != null) {
            try {
                api.removeCapabilitiesListener(listener);
            } catch (Exception e) {
                throw new JoynServiceException(e.getMessage());
            }
        } else {
            throw new JoynServiceNotAvailableException();
        }
    }

    /**
     * Registers a capabilities listener on a list of contacts
     *
     * @param contacts Set of contacts
     * @param listener Capabilities listener
     * @throws JoynServiceException
     * @throws JoynContactFormatException
     *//*
    public void addCapabilitiesListener(Set<String> contacts, CapabilitiesListener listener) throws JoynServiceException, JoynContactFormatException {
        Logger.i(TAG, "addCapabilitiesListener entry" + contacts + listener);
        if (api != null) {
            try {
                Iterator<String> list = contacts.iterator();
                while (list.hasNext()) {
                    String contact = list.next();
                    api.addContactCapabilitiesListener(contact, listener);
                }
            } catch (Exception e) {
                throw new JoynServiceException(e.getMessage());
            }
        } else {
            throw new JoynServiceNotAvailableException();
        }
    }*/

    /**
     * Registers a capabilities listener on a list of contacts
     *
     * @param contacts Set of contacts
     * @param listener Capabilities listener
     * @throws JoynServiceException
     * @throws JoynContactFormatException
     */
    public void addCapabilitiesListener(Set<ContactId> contacts, CapabilitiesListener listener) throws RcsServiceException, RcsServiceNotAvailableException {
        Logger.i(TAG, "addCapabilitiesListener entry" + contacts + listener);
        if (api != null) {
            try {
                Iterator<ContactId> list = contacts.iterator();
                while (list.hasNext()) {
                    ContactId contact = list.next();
                    api.addContactCapabilitiesListener(contact, listener);
                }
            } catch (Exception e) {
                throw new RcsServiceException(e.getMessage());
            }
        } else {
            throw new RcsServiceNotAvailableException();
        }
    }

    /**
     * Unregisters a capabilities listener on a list of contacts
     *
     * @param contacts Set of contacts
     * @param listener Capabilities listener
     * @throws JoynServiceException
     * @throws JoynContactFormatException
     *//*
    public void removeCapabilitiesListener(Set<String> contacts, CapabilitiesListener listener) throws JoynServiceException, JoynContactFormatException {
        Logger.i(TAG, "removeCapabilitiesListener entry" + contacts + listener);
        if (api != null) {
            try {
                Iterator<String> list = contacts.iterator();
                while (list.hasNext()) {
                    String contact = list.next();
                    api.removeContactCapabilitiesListener(contact, listener);
                }
            } catch (Exception e) {
                throw new JoynServiceException(e.getMessage());
            }
        } else {
            throw new JoynServiceNotAvailableException();
        }
    }*/

    /**
     * Unregisters a capabilities listener on a list of contacts
     *
     * @param contacts Set of contacts
     * @param listener Capabilities listener
     * @throws JoynServiceException
     * @throws JoynContactFormatException
     */
    public void removeCapabilitiesListener(Set<ContactId> contacts, CapabilitiesListener listener) throws RcsServiceException, RcsServiceNotAvailableException {
        Logger.i(TAG, "removeCapabilitiesListener entry" + contacts + listener);
        if (api != null) {
            try {
                Iterator<ContactId> list = contacts.iterator();
                while (list.hasNext()) {
                    ContactId contact = list.next();
                    api.removeContactCapabilitiesListener(contact, listener);
                }
            } catch (Exception e) {
                throw new RcsServiceException(e.getMessage());
            }
        } else {
            throw new RcsServiceNotAvailableException();
        }
    }

    /**
     * MTK added function
     * Force request capabilities for a remote contact.
     *
     * @param contact Contact
     * @throws JoynServiceException
     * @throws JoynContactFormatException
     */
    public void forceRequestContactCapabilities(ContactId contact) throws JoynServiceException,
            JoynContactFormatException {
        Logger.i(TAG, "forceRequestContactCapabilities entry" + contact);
        if (api != null) {
            try {
                api.forceRequestContactCapabilities(contact);
            } catch (Exception e) {
                throw new JoynServiceException(e.getMessage());
            }
        } else {
            throw new JoynServiceNotAvailableException();
        }
    }

    /**
     * Requests capabilities for a group of remote contacts.
     *
     * @param contacts List of contacts
     * @throws JoynServiceException
     * @throws JoynContactFormatException
     */
    public void requestContactsCapabilities(List<ContactId> contacts) throws JoynServiceException,
            JoynContactFormatException {
        Logger.i(TAG, "requestContactCapabilities by list entry" + contacts);
        if (api != null) {
            try {
                api.requestContactsCapabilities(contacts);
            } catch (Exception e) {
                throw new JoynServiceException(e.getMessage());
            }
        } else {
            throw new JoynServiceNotAvailableException();
        }
    }
}
