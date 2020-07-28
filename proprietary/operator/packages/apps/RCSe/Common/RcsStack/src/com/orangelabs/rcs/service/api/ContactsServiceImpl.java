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

package com.orangelabs.rcs.service.api;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.gsma.services.rcs.Build;
import com.gsma.services.rcs.JoynService;
import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.contact.IContactService;
import com.gsma.services.rcs.contact.JoynContact;
import com.gsma.services.rcs.contact.RcsContact;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.contact.ContactUtil;

import android.os.RemoteException;

import com.orangelabs.rcs.core.ims.service.ContactInfo;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.utils.ContactIdUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Contacts service API implementation
 *
 * @author Jean-Marc AUFFRET
 */
public class ContactsServiceImpl extends IContactService.Stub {
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     */
    public ContactsServiceImpl() {
    }

    /**
     * Close API
     */
    public void close() {
        if (logger.isActivated()) {
            logger.info("Contacts service API is closed");
        }
    }

    /**
     * Returns the joyn contact infos from its contact ID (i.e. MSISDN)
     *
     * @param contactId Contact ID
     * @return Contact or null if not a joyn contact
     * @throws ServerApiException
     */
    public JoynContact getJoynContact(String contactId) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Get joyn contact " + contactId);
        }

        // Read capabilities in the local database
        ContactInfo contactInfo = ContactsManager.getInstance().getContactInfo(contactId);
        if ((contactInfo !=  null) && contactInfo.isRcsContact()){
            com.orangelabs.rcs.core.ims.service.capability.Capabilities capabilities = contactInfo.getCapabilities();
            Set<String> exts = new HashSet<String>();
            List<String> listExts = capabilities.getSupportedExtensions();
            for(int j=0; j < listExts.size(); j++) {
                exts.add(listExts.get(j));
            }
            Capabilities capaApi = new Capabilities(
                    capabilities.isImageSharingSupported(),
                    capabilities.isVideoSharingSupported(),
                    capabilities.isImSessionSupported(),
                    capabilities.isFileTransferSupported(),
                    capabilities.isGeolocationPushSupported(),
                    capabilities.isIPVoiceCallSupported(),
                    capabilities.isIPVideoCallSupported(),
                    exts,
                    capabilities.isSipAutomata(),
                    capabilities.isFileTransferHttpSupported(),
                    capabilities.isRCSContact(),
                    capabilities.isIntegratedMessagingMode(),
                    capabilities.isCsVideoSupported(),
                    capabilities.isBurnAfterRead()
                    );
            boolean registered = (contactInfo.getRegistrationState() == ContactInfo.REGISTRATION_STATUS_ONLINE);
            return new JoynContact(contactId, registered, capaApi);
        } else {
            return null;
        }
    }

    /**
     * Returns the list of joyn contacts
     *
     * @return List of contacts
     * @throws ServerApiException
     */
    public List<JoynContact> getJoynContacts() throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Get joyn contacts");
        }
        ArrayList<JoynContact> result = new ArrayList<JoynContact>();

        // Read capabilities in the local database
        List<String> contacts = ContactsManager.getInstance().getRcsContacts();
        for(int i =0; i < contacts.size(); i++) {
            String contact = contacts.get(i);
            ContactInfo contactInfo = ContactsManager.getInstance().getContactInfo(contact);
            if ((contactInfo !=  null) && contactInfo.isRcsContact()){
                com.orangelabs.rcs.core.ims.service.capability.Capabilities capabilities = contactInfo.getCapabilities();
                Capabilities capaApi = null;
                if (capabilities != null) {
                    Set<String> exts = new HashSet<String>();
                    List<String> listExts = capabilities.getSupportedExtensions();
                    for(int j=0; j < listExts.size(); j++) {
                        exts.add(listExts.get(j));
                    }
                    capaApi = new Capabilities(
                            capabilities.isImageSharingSupported(),
                            capabilities.isVideoSharingSupported(),
                            capabilities.isImSessionSupported(),
                            capabilities.isFileTransferSupported(),
                            capabilities.isGeolocationPushSupported(),
                            capabilities.isIPVoiceCallSupported(),
                            capabilities.isIPVideoCallSupported(),
                            exts,
                            capabilities.isSipAutomata(),
                            capabilities.isFileTransferHttpSupported(),
                            capabilities.isRCSContact(),
                            capabilities.isIntegratedMessagingMode(),
                            capabilities.isCsVideoSupported(),
                            capabilities.isBurnAfterRead()
                            );
                }
                boolean registered = (contactInfo.getRegistrationState() == ContactInfo.REGISTRATION_STATUS_ONLINE);
                result.add(new JoynContact(contact, registered, capaApi));
            }
        }

        return result;
    }

    /**
     * Returns the list of online contacts (i.e. registered)
     *
     * @return List of joyn contacts
     * @throws ServerApiException
     */
    public List<JoynContact> getJoynContactsOnline() throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Get registered joyn contacts");
        }
        ArrayList<JoynContact> result = new ArrayList<JoynContact>();

        // Read capabilities in the local database
        List<String> contacts = ContactsManager.getInstance().getRcsContacts();
        for(int i =0; i < contacts.size(); i++) {
            String contact = contacts.get(i);
            ContactInfo contactInfo = ContactsManager.getInstance().getContactInfo(contact);
            if ((contactInfo !=  null) && contactInfo.isRcsContact()){
                com.orangelabs.rcs.core.ims.service.capability.Capabilities capabilities = contactInfo.getCapabilities();
                if (contactInfo.getRegistrationState() == ContactInfo.REGISTRATION_STATUS_ONLINE) {
                    Capabilities capaApi = null;
                    if (capabilities != null) {
                        Set<String> exts = new HashSet<String>(capabilities.getSupportedExtensions());
                        capaApi = new Capabilities(
                                capabilities.isImageSharingSupported(),
                                capabilities.isVideoSharingSupported(),
                                capabilities.isImSessionSupported(),
                                capabilities.isFileTransferSupported(),
                                capabilities.isGeolocationPushSupported(),
                                capabilities.isIPVoiceCallSupported(),
                                capabilities.isIPVideoCallSupported(),
                                exts,
                                capabilities.isSipAutomata(),
                                capabilities.isFileTransferHttpSupported(),
                                capabilities.isRCSContact(),
                                capabilities.isIntegratedMessagingMode(),
                                capabilities.isCsVideoSupported(),
                                capabilities.isBurnAfterRead()
                                );
                    }
                    boolean registered = (contactInfo.getRegistrationState() == ContactInfo.REGISTRATION_STATUS_ONLINE);
                    result.add(new JoynContact(contact, registered, capaApi));
                }
            }
        }

        return result;
    }

    /**
     * Returns the list of contacts supporting a given extension (i.e. feature tag)
     *
     * @param serviceId Service ID
     * @return List of joyn contacts
     * @throws ServerApiException
     */
    public List<JoynContact> getJoynContactsSupporting(String serviceId) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Get joyn contacts supporting " + serviceId);
        }

        ArrayList<JoynContact> result = new ArrayList<JoynContact>();

        // Read capabilities in the local database
        List<String> contacts = ContactsManager.getInstance().getRcsContacts();
        for(int i =0; i < contacts.size(); i++) {
            String contact = contacts.get(i);
            ContactInfo contactInfo = ContactsManager.getInstance().getContactInfo(contact);
            if ((contactInfo !=  null) && contactInfo.isRcsContact()){
                com.orangelabs.rcs.core.ims.service.capability.Capabilities capabilities = contactInfo.getCapabilities();
                Capabilities capaApi = null;
                if (capabilities != null) {
                    ArrayList<String> exts = capabilities.getSupportedExtensions();
                    for (int j=0; j < exts.size(); j++) {
                        String ext = exts.get(j);
                        if (ext.equals(serviceId)) {
                            capaApi = new Capabilities(
                                    capabilities.isImageSharingSupported(),
                                    capabilities.isVideoSharingSupported(),
                                    capabilities.isImSessionSupported(),
                                    capabilities.isFileTransferSupported(),
                                    capabilities.isGeolocationPushSupported(),
                                    capabilities.isIPVoiceCallSupported(),
                                    capabilities.isIPVideoCallSupported(),
                                    new HashSet<String>(capabilities.getSupportedExtensions()),
                                    capabilities.isSipAutomata(),
                                    capabilities.isFileTransferHttpSupported(),
                                    capabilities.isRCSContact(),
                                    capabilities.isIntegratedMessagingMode(),
                                    capabilities.isCsVideoSupported(),
                                    capabilities.isBurnAfterRead()
                                    );
                            boolean registered = (contactInfo.getRegistrationState() == ContactInfo.REGISTRATION_STATUS_ONLINE);
                            result.add(new JoynContact(contact, registered, capaApi));
                        }
                    }
                }
            }
        }

        return result;
    }

    /**
     * Returns service version
     *
     * @return Version
     * @see Build.VERSION_CODES
     * @throws ServerApiException
     */
    public int getServiceVersion() throws ServerApiException {
        return Build.API_VERSION;
    }

    /**
     * Get all IM blocked contacts that stored in the local copy
     * @return
     */
    public List<String> getImBlockedContactsFromLocal(){
        return ContactsManager.getInstance()
        .getImBlockedContactsFromLocal();
    }

    public boolean isImBlockedForContact(String contact){
        return ContactsManager.getInstance()
               .isImBlockedForContact(contact);
    }

    public String getTimeStampForBlockedContact(String contact){
        return ContactsManager.getInstance()
        .getTimeStampForBlockedContact(contact);
    }

    /*public static Set<String> getAllBlockedList(){
        return ContactsManager.StrangerBlocker.getAllBlockedList();
    }*/

    public void setImBlockedForContact(String contact, boolean flag){
        ContactsManager.getInstance().setImBlockedForContact(contact,
                flag);
    }


    /**
     * Mark the contact as "blocked for FT"
     *
     * @param contact
     * @param flag indicating if we enable or disable the FT sessions with the contact
     */
    public void setFtBlockedForContact(String contact, boolean flag){
        ContactsManager.getInstance().setFtBlockedForContact(
                contact, flag);
    }

    /**
     * Check if number provided is a valid number for RCS
     * <br>It is not valid if :
     * <li>well formatted (not digits only or '+')
     * <li>minimum length
     *
     * @param number Phone number
     * @return Returns true if it is a RCS valid number
     */
    public boolean isRcsValidNumber(String number){
        return ContactsManager.getInstance().isRcsValidNumber(number);
    }

    /**
     * Get the registration state of a contact in the EAB
     *
     * @param contact Contact
     * @return Contact info
     */
    public int getRegistrationState(String contact) {
        ContactInfo info = ContactsManager.getInstance().getContactInfo(contact);
        return info.getRegistrationState();
    }


    /**
     * Load the IM blocked contacts to the local copy
     */
    public void loadImBlockedContactsToLocal(){
        ContactsManager.getInstance().loadImBlockedContactsToLocal();
    }

    /**
     * Get the contacts that are "IM blocked"
     *
     * @return list containing all contacts that are "IM blocked"
     */
    public List<String> getImBlockedContacts(){
        return ContactsManager.getInstance().getImBlockedContacts();
    }

    /**
     * Returns the RCS contact infos from its contact ID (i.e. MSISDN)
     *
     * @param ContactId Contact ID
     * @return Contact or null if not a rcs contact
     * @throws ServerApiException
     */
    public RcsContact getRcsContact(ContactId contact) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Get rcs contact " + contact);
        }

        // Read capabilities in the local database
        ContactInfo contactInfo = ContactsManager.getInstance().getContactInfo(contact.toString());
        if ((contactInfo !=  null) && contactInfo.isRcsContact()){
            com.orangelabs.rcs.core.ims.service.capability.Capabilities capabilities = contactInfo.getCapabilities();
            Set<String> exts = new HashSet<String>();
            List<String> listExts = capabilities.getSupportedExtensions();
            for(int j=0; j < listExts.size(); j++) {
                exts.add(listExts.get(j));
            }
            Capabilities capaApi = new Capabilities(
                    capabilities.isImageSharingSupported(),
                    capabilities.isVideoSharingSupported(),
                    capabilities.isImSessionSupported(),
                    capabilities.isFileTransferSupported(),
                    capabilities.isGeolocationPushSupported(),
                    capabilities.isIPVoiceCallSupported(),
                    capabilities.isIPVideoCallSupported(),
                    exts,
                    capabilities.isSipAutomata(),
                    capabilities.isFileTransferHttpSupported(),
                    capabilities.isRCSContact(),
                    capabilities.isIntegratedMessagingMode(),
                    capabilities.isCsVideoSupported(),
                    capabilities.isBurnAfterRead(),
                    capabilities.getTimestamp()
                    );
            boolean registered = (contactInfo.getRegistrationState() == ContactInfo.REGISTRATION_STATUS_ONLINE);
            return new RcsContact(contact, registered, capaApi, contact.toString(), contactInfo.getRcsStatus() == ContactInfo.RCS_BLOCKED ,
                    contactInfo.getRcsStatusTimestamp());
        } else {
            return null;
        }
    }

    /**
     * Returns the list of rcs contacts
     *
     * @return List of contacts
     * @throws ServerApiException
     */
    public List<RcsContact> getRcsContacts() throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Get rcs contacts");
        }
        ArrayList<RcsContact> result = new ArrayList<RcsContact>();

        // Read capabilities in the local database
        List<String> contacts = ContactsManager.getInstance().getRcsContacts();
        for(int i =0; i < contacts.size(); i++) {
            String contact = contacts.get(i);
            ContactInfo contactInfo = ContactsManager.getInstance().getContactInfo(contact);
            if ((contactInfo !=  null) && contactInfo.isRcsContact()){
                com.orangelabs.rcs.core.ims.service.capability.Capabilities capabilities = contactInfo.getCapabilities();
                Capabilities capaApi = null;
                if (capabilities != null) {
                    Set<String> exts = new HashSet<String>();
                    List<String> listExts = capabilities.getSupportedExtensions();
                    for(int j=0; j < listExts.size(); j++) {
                        exts.add(listExts.get(j));
                    }
                    capaApi = new Capabilities(
                            capabilities.isImageSharingSupported(),
                            capabilities.isVideoSharingSupported(),
                            capabilities.isImSessionSupported(),
                            capabilities.isFileTransferSupported(),
                            capabilities.isGeolocationPushSupported(),
                            capabilities.isIPVoiceCallSupported(),
                            capabilities.isIPVideoCallSupported(),
                            exts,
                            capabilities.isSipAutomata(),
                            capabilities.isFileTransferHttpSupported(),
                            capabilities.isRCSContact(),
                            capabilities.isIntegratedMessagingMode(),
                            capabilities.isCsVideoSupported(),
                            capabilities.isBurnAfterRead(),
                            capabilities.getTimestamp()
                            );
                }
                boolean registered = (contactInfo.getRegistrationState() == ContactInfo.REGISTRATION_STATUS_ONLINE);
                ContactId contactId = ContactIdUtils.createContactIdFromTrustedData(contact);
                result.add(new RcsContact(contactId, registered, capaApi, contact.toString(), contactInfo.getRcsStatus() == ContactInfo.RCS_BLOCKED,
                        contactInfo.getRcsStatusTimestamp()));
            }
        }

        return result;
    }

    /**
     * Returns the list of online contacts (i.e. registered)
     *
     * @return List of rcs contacts
     * @throws ServerApiException
     */
    public List<RcsContact> getRcsContactsOnline() throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Get registered rcs contacts");
        }
        ArrayList<RcsContact> result = new ArrayList<RcsContact>();

        // Read capabilities in the local database
        List<String> contacts = ContactsManager.getInstance().getRcsContacts();
        for(int i =0; i < contacts.size(); i++) {
            String contact = contacts.get(i);
            ContactInfo contactInfo = ContactsManager.getInstance().getContactInfo(contact);
            if ((contactInfo !=  null) && contactInfo.isRcsContact()){
                com.orangelabs.rcs.core.ims.service.capability.Capabilities capabilities = contactInfo.getCapabilities();
                if (contactInfo.getRegistrationState() == ContactInfo.REGISTRATION_STATUS_ONLINE) {
                    Capabilities capaApi = null;
                    if (capabilities != null) {
                        Set<String> exts = new HashSet<String>(capabilities.getSupportedExtensions());
                        capaApi = new Capabilities(
                                capabilities.isImageSharingSupported(),
                                capabilities.isVideoSharingSupported(),
                                capabilities.isImSessionSupported(),
                                capabilities.isFileTransferSupported(),
                                capabilities.isGeolocationPushSupported(),
                                capabilities.isIPVoiceCallSupported(),
                                capabilities.isIPVideoCallSupported(),
                                exts,
                                capabilities.isSipAutomata(),
                                capabilities.isFileTransferHttpSupported(),
                                capabilities.isRCSContact(),
                                capabilities.isIntegratedMessagingMode(),
                                capabilities.isCsVideoSupported(),
                                capabilities.isBurnAfterRead(),
                                capabilities.getTimestamp()
                                );
                    }
                    boolean registered = (contactInfo.getRegistrationState() == ContactInfo.REGISTRATION_STATUS_ONLINE);
                    ContactId contactId = ContactIdUtils.createContactIdFromTrustedData(contact);
                    result.add(new RcsContact(contactId, registered, capaApi, contact.toString(), contactInfo.getRcsStatus() == ContactInfo.RCS_BLOCKED,
                            contactInfo.getRcsStatusTimestamp()));
                }
            }
        }

        return result;
    }

    /**
     * Returns the list of contacts supporting a given extension (i.e. feature tag)
     *
     * @param serviceId Service ID
     * @return List of joyn contacts
     * @throws ServerApiException
     */
    public List<RcsContact> getRcsContactsSupporting(String tag) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Get rcs contacts supporting " + tag);
        }

        ArrayList<RcsContact> result = new ArrayList<RcsContact>();

        // Read capabilities in the local database
        List<String> contacts = ContactsManager.getInstance().getRcsContacts();
        for(int i =0; i < contacts.size(); i++) {
            String contact = contacts.get(i);
            ContactInfo contactInfo = ContactsManager.getInstance().getContactInfo(contact);
            if ((contactInfo !=  null) && contactInfo.isRcsContact()){
                com.orangelabs.rcs.core.ims.service.capability.Capabilities capabilities = contactInfo.getCapabilities();
                Capabilities capaApi = null;
                if (capabilities != null) {
                    ArrayList<String> exts = capabilities.getSupportedExtensions();
                    for (int j=0; j < exts.size(); j++) {
                        String ext = exts.get(j);
                        if (ext.equals(tag)) {
                            capaApi = new Capabilities(
                                    capabilities.isImageSharingSupported(),
                                    capabilities.isVideoSharingSupported(),
                                    capabilities.isImSessionSupported(),
                                    capabilities.isFileTransferSupported(),
                                    capabilities.isGeolocationPushSupported(),
                                    capabilities.isIPVoiceCallSupported(),
                                    capabilities.isIPVideoCallSupported(),
                                    new HashSet<String>(capabilities.getSupportedExtensions()),
                                    capabilities.isSipAutomata(),
                                    capabilities.isFileTransferHttpSupported(),
                                    capabilities.isRCSContact(),
                                    capabilities.isIntegratedMessagingMode(),
                                    capabilities.isCsVideoSupported(),
                                    capabilities.isBurnAfterRead(),
                                    capabilities.getTimestamp()
                                    );
                            boolean registered = (contactInfo.getRegistrationState() == ContactInfo.REGISTRATION_STATUS_ONLINE);
                            ContactId contactId = ContactIdUtils.createContactIdFromTrustedData(contact);
                            result.add(new RcsContact(contactId, registered, capaApi, contact.toString(), contactInfo.getRcsStatus() == ContactInfo.RCS_BLOCKED,
                                    contactInfo.getRcsStatusTimestamp()));
                        }
                    }
                }
            }
        }

        return result;
    }

    /**
     * Block a contact. Any communication from the given contact will be blocked and redirected to
     * the corresponding spambox.
     *
     * @param contact Contact ID
     * @throws RemoteException
     */
    public void blockContact(ContactId contact) throws ServerApiException {
        if (contact == null) {
            throw new ServerApiException("contact must not be null!");
        }
        if (logger.isActivated()) {
            logger.info("Block contact " + contact);
        }
        try {
            ContactsManager.getInstance().blockContact(contact.toString());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Unblock a contact
     *
     * @param contact Contact ID
     * @throws RemoteException
     */
    public void unblockContact(ContactId contact) throws ServerApiException {
        if (contact == null) {
            throw new ServerApiException("contact must not be null!");
        }
        if (logger.isActivated()) {
            logger.info("Unblock contact " + contact);
        }
        try {
            ContactsManager.getInstance().unblockContact(contact.toString());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}