/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010-2016 Orange.
 * Copyright (C) 2014 Sony Mobile Communications Inc.
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
 *
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are licensed under the License.
 ******************************************************************************/

package com.orangelabs.rcs.utils;

import com.gsma.services.rcs.RcsPermissionDeniedException;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.contact.ContactUtil;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Contact utility functions
 */
public class ContactIdUtils {

    private static volatile ContactUtil mContactUtil;

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * A class to hold a valid phone number
     *//*
    public final static class PhoneNumber {
        private final String mNumber;

        *//**
         * Constructor
         * 
         * @param number a valid phone number
         *//*
         package private PhoneNumber(String number) {
            mNumber = number;
        }

        *//**
         * Gets the valid phone number
         * 
         * @return the valid phone number
         *//*
        public String getNumber() {
            return mNumber;
        }
    }

    *//**
     * Gets a valid phone number from a URI
     * 
     * @param uri phone number
     * @return the phone number without URI formating or null if not valid
     *//*
    public static PhoneNumber getValidPhoneNumberFromUri(String uri) {
        String number = PhoneUtils.extractNumberFromUriWithoutFormatting(uri);
        synchronized (ContactUtil.class) {
            if (mContactUtil == null) {
                mContactUtil = com.gsma.services.rcs.contact.ContactUtil.getInstance(AndroidFactory
                        .getApplicationContext());
            }
        }
        try {
            if (mContactUtil.isValidContact(number)) {
                return new PhoneNumber(number);
            }
        } catch (RcsPermissionDeniedException e) {
            if (sLogger.isActivated()) {
                sLogger.error("Failed to validate phone number from URI '" + uri + ("'!"), e);
            }
        }
        return null;
    }

    *//**
     * Gets a valid phone number from a contact got from Android system.<br>
     * (By Android system, we mean telephony manager, address book, etc)
     * 
     * @param contact from Android system
     * @return the phone number or null if not valid
     *//*
    public static PhoneNumber getValidPhoneNumberFromAndroid(String contact) {
        synchronized (ContactUtil.class) {
            if (mContactUtil == null) {
                mContactUtil = com.gsma.services.rcs.contact.ContactUtil.getInstance(AndroidFactory
                        .getApplicationContext());
            }
        }
        try {
            if (mContactUtil.isValidContact(contact)) {
                return new PhoneNumber(contact);
            }
        } catch (RcsPermissionDeniedException e) {
            sLogger.error("Failed to validate phone number from Android '" + contact + ("'!"), e);
        }
        return null;
    }

    *//**
     * Creates a ContactId from a validated phone number
     * 
     * @param phoneNumber the validated phone number
     * @return the Contact Identifier
     *//*
    public static ContactId createContactIdFromValidatedData(PhoneNumber phoneNumber) {
        synchronized (ContactUtil.class) {
            if (mContactUtil == null) {
                mContactUtil = com.gsma.services.rcs.contact.ContactUtil.getInstance(AndroidFactory
                        .getApplicationContext());
            }
        }
        try {
            return mContactUtil.formatContact(phoneNumber.getNumber());

        } catch (RcsPermissionDeniedException e) {
            
             * This exception cannot occur since PhoneNumber can only be instantiated for valid
             * numbers.
             
            String errorMessage = "Phone number '" + phoneNumber
                    + "' cannot be converted into contactId!";
            throw new IllegalStateException(errorMessage, e);
        }
    }*/

    /**
     * Creates a ContactId from a trusted data
     * (By trusted data, we mean RCS providers)
     * 
     * @param contact from a trusted data
     * @return the ContactId contact id
     */
    public static ContactId createContactIdFromTrustedData(String contact) {
        synchronized (ContactUtil.class) {
            if (mContactUtil == null) {
                mContactUtil = ContactUtil.getInstance(AndroidFactory.getApplicationContext());
            }
        }
        try {
            contact = PhoneUtils.extractNumberFromUri(contact);
            return mContactUtil.formatContact(contact);

        } catch (RcsPermissionDeniedException e) {
            /*
             * This exception should not occur since core stack cannot be started if country code
             * cannot be resolved.
             */
            String errorMessage = "Failed to convert phone number '" + contact
                    + "' into contactId!";
            throw new IllegalStateException(errorMessage, e);
        }
    }

}
