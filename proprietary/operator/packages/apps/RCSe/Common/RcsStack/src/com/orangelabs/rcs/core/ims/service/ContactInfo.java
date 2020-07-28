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

package com.orangelabs.rcs.core.ims.service;

import com.orangelabs.rcs.core.ims.service.capability.Capabilities;
import com.orangelabs.rcs.core.ims.service.presence.PresenceInfo;
import android.util.SparseArray;

/**
 * Contact info
 */
public class ContactInfo {
    /**
     * The contact is RCS capable but there is no special presence relationship with the user
     */
    public final static int RCS_CAPABLE = 0;
    
    /**
     * The contact is not RCS
     */
    public final static int NOT_RCS = 1;

    /** 
     * Presence relationship: contact 'rcs granted' with the user 
     */
    public final static int RCS_ACTIVE = 2;
    
    /**
     * Presence relationship: the user has revoked the contact 
     */
    public final static int RCS_REVOKED = 3;
    
    /**
     * Presence relationship: the user has blocked the contact 
     */
    public final static int RCS_BLOCKED = 4;
    
    /**
     * Presence relationship: the user has sent an invitation to the contact without response for now 
     */
    public final static int RCS_PENDING_OUT = 5;
    
    /**
     * Presence relationship: the contact has sent an invitation to the user without response for now 
     */
    public final static int RCS_PENDING = 6;
    
    /** 
     * Presence relationship: the contact has sent an invitation to the user and cancel it
     */
    public final static int RCS_CANCELLED = 7;
    
    /** 
     * We have never queried the contact capabilities for now
     */
    public final static int NO_INFO = 8;
    
    /** 
     * Registration state : unknown
     */
    public final static int REGISTRATION_STATUS_UNKNOWN = 0;
    
    /** 
     * Registration state : registered
     */
    public final static int REGISTRATION_STATUS_ONLINE = 1;
    
    /** 
     * Registration state : not registered
     */
    public final static int REGISTRATION_STATUS_OFFLINE = 2;
    
    /**
     * Capabilities
     */
    private Capabilities capabilities = null;
    
    /**
     * Presence info, relevant only if social info is activated for this contact
     */
    private PresenceInfo presenceInfo = null;
    
    /**
     * Contact
     */
    private String contact = null;
    
    /**
     * Registration state
     */
    private int registrationState = REGISTRATION_STATUS_UNKNOWN;
    
    /**
     * RCS status
     */
    private int rcsStatus = ContactInfo.NOT_RCS;
    
    /**
     * RCS status timestamp
     */
    private long rcsStatusTimestamp = 0L;
    
    /**
     * Constructor
     */
    public ContactInfo() {
    }

    /**
     * Constructor
     * 
     * @param contactInfo
     */
    public ContactInfo(ContactInfo info) {
        this.contact = info.getContact();
        this.registrationState = info.getRegistrationState();
        this.rcsStatus = info.getRcsStatus();
        this.rcsStatusTimestamp = info.getRcsStatusTimestamp();
        this.capabilities = info.capabilities;
        this.presenceInfo = info.getPresenceInfo();        
    }

    /**
     * Set the capabilities
     * 
     * @param capabilities Capabilities
     */
    public void setCapabilities(Capabilities capabilities) {
        this.capabilities = capabilities;
    }
    
    /**
     * Returns the capabilities
     * 
     * @return Capabilities
     */
    public Capabilities getCapabilities(){
        return capabilities;
    }
    
    /**
     * Set the presence info
     * 
     * @param info Presence info
     */
    public void setPresenceInfo(PresenceInfo info) {
        this.presenceInfo = info;
    }
    
    /**
     * Returns the presence info
     * 
     * @return PresenceInfo
     */
    public PresenceInfo getPresenceInfo(){
        return presenceInfo;
    }

    /**
     * Set the contact
     * 
     * @param contact Contact
     */
    public void setContact(String contact) {
        this.contact = contact;
    }
    
    /**
     * Returns the contact
     * 
     * @return contact
     */
    public String getContact(){
        return contact;
    }
    
    /**
     * Set the RCS status
     * 
     * @param rcsStatus RCS status
     */
    public void setRcsStatus(int rcsStatus) {
        this.rcsStatus = rcsStatus;
    }
    
    /**
     * Returns the RCS status
     * 
     * @return rcsStatus
     */
    public int getRcsStatus(){
        return rcsStatus;
    }
    
    /**
     * Set the registration state
     * 
     * @param int registrationState
     */
    public void setRegistrationState(int registrationState) {
        this.registrationState = registrationState;
    }
    
    /**
     * Returns the registration state
     * 
     * @return registrationState
     */
    public int getRegistrationState(){
        return registrationState;
    }
    
    /**
     * Set the RCS status timestamp
     * 
     * @param timestamp Last RCS status date of change
     */
    public void setRcsStatusTimestamp(long timestamp) {
        this.rcsStatusTimestamp = timestamp;
    }
    
    /**
     * Returns the RCS status timestamp
     * 
     * @return timestamp
     */
    public long getRcsStatusTimestamp(){
        return rcsStatusTimestamp;
    }

    /**
     * Is a RCS contact
     *
     * @return true if the contact is RCS
     */
    public boolean isRcsContact() {
        return (rcsStatus != ContactInfo.NO_INFO && rcsStatus != ContactInfo.NOT_RCS);

    }

    /**
     * Returns a string representation of the object
     * 
     * @return String
     */
    public String toString() {
        String result =  "Contact=" + contact +
            ", Status=" + rcsStatus +
            ", State=" + registrationState +
            ", Timestamp=" + rcsStatusTimestamp;
        if (capabilities != null) {
            result += ", Capabilities=" + capabilities.toString();
        }
        if (presenceInfo != null) {
            result += ", Presence=" + presenceInfo.toString();
        }
        return result;
    }


    /**
     * IMS registration state
     */
    public enum RegistrationState {
        /**
         * Unknown registration state
         */
        UNKNOWN(0),
        /**
         * Registered
         */
        ONLINE(1),
        /**
         * Not registered
         */
        OFFLINE(2);

        private int mValue;

        /**
         * A data array to keep mapping between value and RegistrationState
         */
        private static SparseArray<RegistrationState> mValueToEnum = new SparseArray<RegistrationState>();
        static {
            for (RegistrationState entry : RegistrationState.values()) {
                mValueToEnum.put(entry.toInt(), entry);
            }
        }

        private RegistrationState(int value) {
            mValue = value;
        }

        /**
         * Gets integer value associated to RegistrationState instance
         * 
         * @return value
         */
        public final int toInt() {
            return mValue;
        }

        /**
         * Returns a RegistrationState instance for the specified integer value.
         * 
         * @param value to convert
         * @return instance of RegistrationState
         */
        public static RegistrationState valueOf(int value) {
            RegistrationState state = mValueToEnum.get(value);
            if (state != null) {
                return state;
            }
            return UNKNOWN;
        }
    }


 /**
     * User blocking state
     */
    public enum BlockingState {
        /**
         * User is not blocked
         */
        NOT_BLOCKED(0),
        /**
         * user is blocked
         */
        BLOCKED(1);

        private int mValue;

        /**
         * A data array to keep mapping between value and BlockingState
         */
        private static SparseArray<BlockingState> mValueToEnum = new SparseArray<BlockingState>();
        static {
            for (BlockingState entry : BlockingState.values()) {
                mValueToEnum.put(entry.toInt(), entry);
            }
        }

        private BlockingState(int value) {
            mValue = value;
        }

        /**
         * Gets integer value associated to BlockingState instance
         * 
         * @return value
         */
        public final int toInt() {
            return mValue;
        }

        /**
         * Returns a BlockingState instance for the specified integer value.
         * 
         * @param value to convert
         * @return instance of BlockingState
         */
        public static BlockingState valueOf(int value) {
            BlockingState state = mValueToEnum.get(value);
            if (state != null) {
                return state;
            }
            return NOT_BLOCKED;
        }
    }

    /**
     * RCS status
     */
    public enum RcsStatus {
        /**
         * The contact is RCS capable but there is no special presence relationship with the user
         */
        RCS_CAPABLE(0),
        /**
         * The contact is not RCS
         */
        NOT_RCS(1),
        /**
         * Presence relationship: contact 'rcs granted' with the user
         */
        ACTIVE(2),
        /**
         * Presence relationship: the user has revoked the contact
         */
        REVOKED(3),
        /**
         * Presence relationship: the user has blocked the contact
         */
        BLOCKED(4),
        /**
         * Presence relationship: the user has sent an invitation to the contact without response
         * for now
         */
        PENDING_OUT(5),
        /**
         * Presence relationship: the contact has sent an invitation to the user without response
         * for now
         */
        PENDING(6),
        /**
         * Presence relationship: the contact has sent an invitation to the user and cancel it
         */
        CANCELLED(7),
        /**
         * We have never queried the contact capabilities for now
         */
        NO_INFO(8);

        private int mValue;

        /**
         * A data array to keep mapping between value and PresenceSharingStatus
         */
        private static SparseArray<RcsStatus> mValueToEnum = new SparseArray<RcsStatus>();
        static {
            for (RcsStatus entry : RcsStatus.values()) {
                mValueToEnum.put(entry.toInt(), entry);
            }
        }

        private RcsStatus(int value) {
            mValue = value;
        }

        /**
         * Gets integer value associated to RcsStatus instance
         * 
         * @return value
         */
        public final int toInt() {
            return mValue;
        }

        /**
         * Returns a RcsStatus instance for the specified integer value.
         * 
         * @param value to convert
         * @return instance of RcsStatus
         */
        public static RcsStatus valueOf(int value) {
            RcsStatus status = mValueToEnum.get(value);
            if (status != null) {
                return status;
            }
            return NO_INFO;
        }
    }



}
