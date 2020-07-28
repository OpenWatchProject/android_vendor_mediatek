/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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


package com.mediatek.presence.core.ims.service.capability;

import java.util.ArrayList;
import java.util.List;
import java.util.HashSet;
import java.util.Iterator;
import java.util.concurrent.locks.ReentrantLock;

import android.database.Cursor;
import android.os.Build;
import android.provider.ContactsContract.CommonDataKinds.Phone;

import com.mediatek.presence.addressbook.AddressBookEventListener;
import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.service.ContactInfo;
import com.mediatek.presence.core.ims.service.ImsService;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.ContactNumberUtils;
import com.mediatek.presence.utils.PhoneUtils;
import com.mediatek.presence.utils.SimUtils;
import com.mediatek.presence.utils.logger.Logger;

/**
 * Capability discovery service
 */
public class NACapabilityService extends CapabilityService implements AddressBookEventListener {

    /**
     * Options manager
     */
    private OptionsManager optionsManager;

    /**
     * Anonymous fetch manager
     */
    private AnonymousFetchManager anonymousFetchManager;

    /**
     * Polling manager
     */
    private PollingManager pollingManager;

    /**
     * Flag: set during the address book changed procedure, if we are notified of a change
     */
    private boolean isRecheckNeeded = false;

    /**
     * Flag indicating if a check procedure is in progress
     */
    private boolean isCheckInProgress = false;

    //maybe no need now.
    private ReentrantLock lock = new ReentrantLock();

    /**
     * Constructor
     *
     * @param parent
     *            IMS module
     * @throws CoreException
     */
    public NACapabilityService(ImsModule parent) throws CoreException {
        super(parent);
        logger = Logger.getLogger(mSlotId, "NACapabilityService");

        // Instanciate the polling manager
        pollingManager = new PollingManager(this);

        // Instanciate the options manager
        optionsManager = new OptionsManager(parent);

        // Instanciate the anonymous fetch manager
        anonymousFetchManager = new AnonymousFetchManager(parent);

        // Get capability extensions
        CapabilityUtils.updateExternalSupportedFeatures(AndroidFactory.getApplicationContext());

    }

    /**
     * Start the IMS service
     */
    @Override
    public synchronized void start() {
        if (isServiceStarted()) {
            // Already started
            return;
        }
        setServiceStarted(true);

        logger.debug("CapabilitiesService.start()");

        /// Add for op07 @{
        if (SimUtils.isAttSimCard(mSlotId)) {
            // Start options manager
            optionsManager.start();
            return;
        }
        /// @}

        // Listen to address book changes
        getImsModule().getCore().getAddressBookManager().addAddressBookListener(this);

        // Start polling
        pollingManager.start();

        // Force a first capability check
        Thread t = new Thread() {
            @Override
            public void run() {
                handleAddressBookHasChanged();
            }
        };
        t.start();
    }

    /**
     * Stop the IMS service
     */
    @Override
    public synchronized void stop() {
        if (!isServiceStarted()) {
            // Already stopped
            return;
        }
        setServiceStarted(false);

        /// Add for op07 @{
        if (SimUtils.isAttSimCard(mSlotId)) {
            // Stop options manager
            optionsManager.stop();
            return;
        }
        /// @}

        // Stop polling
        pollingManager.stop();

        // Stop listening to address book changes
        getImsModule().getCore().getAddressBookManager().removeAddressBookListener(this);
    }

    /**
     * Check the IMS service
     */
    @Override
    public void check() {
    }

    /**
     *ImsService error handling
     */
    @Override
    public void onForbiddenReceived(ImsModule.EabActions action, String reason) {
    }

    @Override
    public void onNotProvisionedReceived() {
    }

    /**
     * Get the options manager
     *
     * @return Options manager
     */
    public OptionsManager getOptionsManager() {
        return optionsManager;
    }

    /**
     * Get the options manager
     *
     * @return Options manager
     */
    public AnonymousFetchManager getAnonymousFetchManager() {
        return anonymousFetchManager;
    }

    private boolean isContactCapabilityExpiry(String contact) {
        boolean result = false;
        int rcsExpiry  = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .getCapabilityExpiryTimeout();
        int nonRcsExpiry  = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .getNonRcsCapabilityExpiryTimeout();
        
        // Read capabilities from the database
        Capabilities capabilities = ContactsManager.getInstance().
                getContactCapabilities(contact);
        if (!SimUtils.isAttSimCard(mSlotId)
                && capabilities != null
                && !ContactsManager.getInstance().contactExists(contact)) {
            if (logger.isActivated()) {
                logger.debug("contact is not saved " + contact);
            }
            long delta = (System.currentTimeMillis() - capabilities.getTimestamp()) / 1000;
            if ((delta >= rcsExpiry) || (delta < 0) ||
                    (capabilities.getTimestamp() == 0)) {
                if (logger.isActivated()) {
                    logger.debug("Capabilities have expired for " + contact);
                }
                // contact is anonymous and capability is not valid, so set capability to null
                capabilities = null;
            }

        }

        if (capabilities == null) {
            if (logger.isActivated()) {
                logger.debug("No capability exist for " + contact);
            }
            result = true;
        } else {
            long delta = (System.currentTimeMillis() - capabilities.getTimestamp()) / 1000;
            if (logger.isActivated()) {
                logger.debug("Capabilities exist for " + contact + ", delta:" +
                        delta + " timestamp:" + capabilities.getTimestamp());
            }
            boolean needRefresh = delta >= rcsExpiry;
            /// Add for op07 @{
            if (SimUtils.isAttSimCard(mSlotId)) {
                needRefresh = ((capabilities.isRCSContact() && delta >= rcsExpiry) ||
                    (!capabilities.isRCSContact() && delta >= nonRcsExpiry));
            }
            /// @}

            if (needRefresh || (delta < 0) || (capabilities.getTimestamp() == 0)) {
                result = true;
            } else {
                if (logger.isActivated()) {
                    logger.debug("Capabilities have not been expired for " + contact +
                            " RCS expired:" + rcsExpiry +
                            " NON RCS expired:" + nonRcsExpiry);
                }
            }
        }

        return result;
    }

    /**
     * Request contact capabilities.
     *
     * @param contact
     *            Contact
     */
    public synchronized void requestContactCapabilities(String contact) {
        if (logger.isActivated()) {
            logger.debug("Request capabilities to " + contact);
        }

        /// Add for op07 @{
        if (SimUtils.isAttSimCard(mSlotId)) {
                if (!ContactNumberUtils.getDefault().isValidNumber(contact)) {
                    if (logger.isActivated()) {
                        logger.debug(contact + " is not a valid number");
                    }
                    return;
                }
        }
        /// @}

        // Extract contact phone number
        contact = PhoneUtils.extractNumberFromUri(contact);

        // Check if if it is a valid RCS number
        if (!ContactsManager.getInstance().isRcsValidNumber(contact)) {
            if (logger.isActivated()) {
                logger.debug(contact + " is not a valid joyn number");
            }
            return;
        }

        if (isContactCapabilityExpiry(contact)) {
            anonymousFetchManager.requestCapabilities(contact);
        }
    }

    /**
     * Request contact capability with a requestId (AOSP procedure)
     *
     * @param contact contact
     * @param requestId When device receives SIP response, presence statck shall send
     *                  a callback with requestId to client
     */
    public synchronized void requestContactCapabilities(String contact, int requestId) {
        if (logger.isActivated()) {
            logger.debug("Request capabilities to " + contact);
        }

        /// Add for op07 @{
        if (SimUtils.isAttSimCard(mSlotId)) {
                if (!ContactNumberUtils.getDefault().isValidNumber(contact)) {
                    if (logger.isActivated()) {
                        logger.debug(contact + " is not a valid number");
                    }
                    return;
                }
        }
        /// @}

        // Extract contact phone number
        contact = PhoneUtils.extractNumberFromUri(contact);

        // Check if if it is a valid RCS number
        if (!ContactsManager.getInstance().isRcsValidNumber(contact)) {
            if (logger.isActivated()) {
                logger.debug(contact + " is not a valid joyn number");
            }
            return;
        }

        if (isContactCapabilityExpiry(contact)) {
            anonymousFetchManager.requestCapabilities(contact, requestId);
        }
    }

    public synchronized void requestContactAvailability(String contact) {
        if (logger.isActivated()) {
            logger.debug("Request Availability to " + contact);
        }

        // VzW TODO: VZ_REQ_RCSVOLTE_4070
        if (SimUtils.isAttSimCard(mSlotId)) {
                if (!ContactNumberUtils.getDefault().isValidNumber(contact)) {
                    if (logger.isActivated()) {
                        logger.debug(contact + " is not a valid number");
                    }
                    return;
                }
        }

        // Extract contact phone number
        contact = PhoneUtils.extractNumberFromUri(contact);

        // Check if it is a valid RCS number
        if (!ContactsManager.getInstance().isRcsValidNumber(contact)) {
            if (logger.isActivated()) {
                logger.debug(contact + " is not a valid joyn number");
            }
            return;
        }
    }

    public synchronized void requestContactAvailability(String contact, int requestId) {
        //VzW only behavior. Hence, does not need to do anything in NACapabilityService
    }

    /**
     * Request capabilities for a list of contacts
     *
     * @param contactList
     *            List of contacts
     */
    public void requestContactsCapabilities(List<String> contactList) {
        if (contactList != null && contactList.size() > 0) {
            if (logger.isActivated()) {
                logger.debug("Request capabilities size: " + contactList.size());
            }

            //remove duplicated contact
            HashSet<String> set = new HashSet<String>(contactList);
            contactList.clear();
            //remove no expiry contact
            for (Iterator<String> it = set.iterator(); it.hasNext(); ) {
                String contact = it.next();
                ContactInfo contactInfo = ContactsManager.getInstance().
                        getContactInfo(contact);
                logger.debug("requestContactCapabilities contact: " + contact + " status:" + contactInfo.getRcsStatus());
                if (isContactCapabilityExpiry(contact) && contactInfo.getRcsStatus() != 9) {
                    contactList.add(contact);
                }
            }
            if (contactList != null && contactList.size() > 0) {
                logger.debug("Request capabilities actual size: " + contactList.size());
                anonymousFetchManager.requestCapabilities(contactList);
            }
        }
    }

    /**
     * Request capabilities for a list of contacts with a requestId (AOSP procedure)
     *
     * @param contactList contactList
     * @param requestId When device receives SIP response, presence statck shall send
     *                  a callback with requestId to client
     */
    public void requestContactsCapabilities(List<String> contactList, int requestId) {
        if (contactList != null && contactList.size() > 0) {
            if (logger.isActivated()) {
                logger.debug("Request capabilities size: " + contactList.size());
            }

            //remove duplicated contact
            HashSet<String> set = new HashSet<String>(contactList);
            contactList.clear();
            //remove no expiry contact
            for (Iterator<String> it = set.iterator(); it.hasNext(); ) {
                String contact = it.next();
                ContactInfo contactInfo = ContactsManager.getInstance().
                        getContactInfo(contact);
                logger.debug("requestContactCapabilities contact: " + contact
                        + " status:" + contactInfo.getRcsStatus());
                if (isContactCapabilityExpiry(contact) && contactInfo.getRcsStatus() != 9) {
                    contactList.add(contact);
                }
            }
            if (contactList != null && contactList.size() > 0) {
                logger.debug("Request capabilities actual size: " + contactList.size());
                anonymousFetchManager.requestCapabilities(contactList, requestId);
            }
        }
    }

    /**
     * Receive a capability request (options procedure)
     *
     * @param options
     *            Received options message
     */
    public void receiveCapabilityRequest(SipRequest options) {
        optionsManager.receiveCapabilityRequest(options);
    }

    /**
     * Receive a notification (anonymous fecth procedure)
     *
     * @param notify
     *            Received notify
     */
    public void receiveNotification(final SipRequest notify) {
        lock.lock();
        try {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    anonymousFetchManager.receiveNotification(notify);
                }
            }).start();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            lock.unlock();
        }
    }

    public void handleAddressBookHasChanged() {
        lock.lock();
        try {
            handleAddressBookHasChangedLocked();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            lock.unlock();
        }
    }

    /**
     * Address book content has changed
     */
    private void handleAddressBookHasChangedLocked() {
        // Update capabilities for the contacts that have never been queried
        if (logger.isActivated()) {
            logger.debug(" handleAddressBookHasChanged entry ");
        }
        if (isCheckInProgress) {
            isRecheckNeeded = true;
            return;
        }

        // We are beginning the check procedure
        isCheckInProgress = true;

        // Reset recheck flag
        isRecheckNeeded = false;

        // Check all phone numbers and query only the new ones
        String[] projection = {
                Phone._ID, Phone.NUMBER, Phone.RAW_CONTACT_ID
        };
        Cursor phonesCursor = AndroidFactory.getApplicationContext().getContentResolver().query(
                Phone.CONTENT_URI,
                projection,
                null,
                null,
                null);

        // List of unique number that will have to be queried for capabilities
        ArrayList<String> toBeTreatedNumbers = new ArrayList<String>();

        // List of unique number that have already been queried
        ArrayList<String> alreadyInEabOrInvalidNumbers = new ArrayList<String>();

        // We add "My number" to the numbers that are already RCS, so we don't query it if it is
        // present in the address book
        alreadyInEabOrInvalidNumbers.add(PhoneUtils.extractNumberFromUri(ImsModule.IMS_USER_PROFILE.getPublicUri()));

        while (phonesCursor.moveToNext()) {
            // Keep a trace of already treated row. Key is (phone number in international format)
            String phoneNumber = PhoneUtils.formatNumberToInternational(phonesCursor.getString(1));
            if (!alreadyInEabOrInvalidNumbers.contains(phoneNumber)) {
                // If this number is not considered RCS valid or has already an entry with RCS, skip
                // it
                if (ContactsManager.getInstance().isRcsValidNumber(phoneNumber)
                        && !ContactsManager.getInstance().isRcsAssociated(phoneNumber)
                        && (!ContactsManager.getInstance().isOnlySimAssociated(phoneNumber) || (Build.VERSION.SDK_INT > 10))) {
                    // This entry is valid and not already has a RCS raw contact, it can be treated
                    // We exclude the number that comes from SIM only contacts, as those cannot be
                    // aggregated to RCS raw contacts only if OS version if gingebread or fewer

                    if (!toBeTreatedNumbers.contains(phoneNumber))
                        toBeTreatedNumbers.add(phoneNumber);
                } else {
                    // This entry is either not valid or already RCS, this number is already done
                    alreadyInEabOrInvalidNumbers.add(phoneNumber);

                    // Remove the number from the treated list, if it is in it
                    toBeTreatedNumbers.remove(phoneNumber);
                }
            } else {
                // Remove the number from the treated list, it was already queried for another raw
                // contact on the same number
                toBeTreatedNumbers.remove(phoneNumber);

                // If it is a RCS contact and the raw contact is not associated with a RCS raw
                // contact,
                // then we have to create a new association for it
                long rawContactId = phonesCursor.getLong(2);
                if ((!ContactsManager.getInstance().isSimAccount(rawContactId) || (Build.VERSION.SDK_INT > 10))
                        && (ContactsManager.getInstance().getAssociatedRcsRawContact(rawContactId, phoneNumber) == -1)) {
                    ContactInfo currentInfo = ContactsManager.getInstance().getContactInfo(phoneNumber);
                    if (currentInfo != null && currentInfo.isRcsContact()) {
                        ContactsManager.getInstance().createRcsContact(currentInfo, rawContactId);
                    }
                }
            }
        }
        phonesCursor.close();

        if (logger.isActivated()) {
            logger.debug(" handleAddressBookHasChanged toBeTreatedNumbers " + toBeTreatedNumbers);
        }
        // Get the capabilities for the numbers that haven't got a RCS associated contact
        requestContactsCapabilities(toBeTreatedNumbers);

        // End of the check procedure
        isCheckInProgress = false;

        // Check if we have to make another check
        if (isRecheckNeeded) {
            handleAddressBookHasChanged();
        }
    }

    /**
     * Reset the content sharing capabities for a given contact
     *
     * @param contact
     *            Contact
     */
    public void resetContactCapabilitiesForContentSharing(String contact) {
        logger.debug("force contact capabilities for " + contact);
        Capabilities capabilities = ContactsManager.getInstance().getContactCapabilities(contact);
        if (capabilities != null) {
            // Force a reset of content sharing capabilities
            capabilities.setImageSharingSupport(false);
            capabilities.setVideoSharingSupport(false);

            // Update the database capabilities
            ContactsManager.getInstance().setContactCapabilities(contact, capabilities);

            // Notify listener
            getImsModule().getCore().getListener().handleCapabilitiesNotification(contact, capabilities);
        }
     }

    /**
     * Request contact capabilities, force send request to server.
     *
     * @param contact Contact number string
     * @return Capabilities
     */
    public synchronized void forceRequestContactCapabilities(String contact) {
        if (logger.isActivated()) {
            logger.debug("force request capabilities for " + contact);
        }

        if (!ContactNumberUtils.getDefault().isValidNumber(contact)) {
            if (logger.isActivated()) {
                logger.debug(contact + " is not a valid number");
            }
            return;
        }

        if (!ContactsManager.getInstance().isRcsValidNumber(contact)) {
            if (logger.isActivated()) {
                logger.debug(contact + " is not a valid joyn number");
            }
            return;
        }

        contact = PhoneUtils.extractNumberFromUri(contact);
        anonymousFetchManager.requestCapabilities(contact);
    }

    public int getSlotId() {
        return mSlotId;
    }

}

