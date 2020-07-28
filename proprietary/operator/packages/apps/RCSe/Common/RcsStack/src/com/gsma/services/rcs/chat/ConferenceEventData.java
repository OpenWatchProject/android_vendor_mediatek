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
package com.gsma.services.rcs.chat;


import java.util.ArrayList;
import java.util.List;

import com.gsma.services.rcs.Logger;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * Chat message
 *
 */
public class ConferenceEventData implements Parcelable {
    public static final String TAG = "TAPI-ConferenceEventData";

    public static final class State {
        public static final String FULL = "full";
        public static final String PARTIAL = "partial";
    }

    private String state = "";

    private String subject = "";

    private String chairman = "";

    private List<ConferenceUser> users;

    public String getState() {
        return state;
    }

    public void setState(String state) {
        this.state = state;
    }

    public String getSubject() {
        return subject;
    }

    public void setSubject(String subject) {
        this.subject = subject;
    }

    public String getChairman() {
        return chairman;
    }

    public void setChairman(String chairman) {
        this.chairman = chairman;
    }

    public List<ConferenceUser> getUsers() {
        return users;
    }

    public void setUsers(List<ConferenceUser> users) {
        this.users = users;
    }

    public ConferenceEventData(
            String state,
            String subject,
            String chairman,
            List<ConferenceUser> users) {
        this.state = state;
        this.subject = subject;
        this.chairman = chairman;
        this.users = users;
    }

    public ConferenceEventData(Parcel source) {
        this.state = source.readString();
        this.subject = source.readString();
        this.chairman = source.readString();

        if (source.dataAvail() > 0) {
            int size = source.readInt();
            this.users = new ArrayList<ConferenceUser>(size);
            for (int i = 0; i < size; i++ ) {
                ConferenceUser user = (ConferenceUser)source.readParcelable(null);
                this.users.add(user);
            }
        }
    }

    public ConferenceEventData() {
    }

    public void writeToParcel(Parcel dest, int arg1) {
        dest.writeString(state);
        dest.writeString(subject);
        dest.writeString(chairman);
        if (users != null) {
            dest.writeInt(users.size());
            for (ConferenceUser user : users) {
                dest.writeParcelable(user, 0);
            }
        }
    }

    /**
     * Parcelable creator
     *
     * @hide
     */
    public static final Parcelable.Creator<ConferenceEventData> CREATOR
            = new Parcelable.Creator<ConferenceEventData>() {
        public ConferenceEventData createFromParcel(Parcel source) {
            return new ConferenceEventData(source);
        }

        public ConferenceEventData[] newArray(int size) {
            return new ConferenceEventData[size];
        }
    };

    public static class ConferenceUser implements Parcelable{
        /**
         * User State
         */
        private String state;

        /**
         * Status of the user
         */
        private String status;

        /**
         * disconnection method
         */
        private String method;

        /**
         * role of the user
         */
        private String role;

        /**
         * etype of the user
         */
        private String etype;

        /**
         * displayname of the user
         */
        private String displayName;

        /**
         * entity of the user
         */
        private String entity;


        /**
         * Constructor for outgoing message
         *
         * @param entity entity
         * @param state state
         * @param status status
         * @param method method
         * @param role role
         * @param etype etype
         * @param displayname displayname
         *
         */
        public ConferenceUser(
                String entity,
                String state,
                String status,
                String method,
                String role,
                String etype,
                String displayName) {
            Logger.i(TAG, "ConferenceUser entry" +
                    "entity=" + entity + " state=" + state + " status=" + status +
                    " method=" + method + " role=" + role + " etype=" + etype +
                    " displayName=" + displayName);
            this.entity = entity;
            this.state = state;
            this.status = status;
            this.method = method;
            this.role = role;
            this.etype = etype;
            this.displayName = displayName;
        }

        /**
         * Constructor
         *
         * @param source Parcelable source
         * @hide
         */
        public ConferenceUser(Parcel source) {
            this.entity = source.readString();
            this.state = source.readString();
            this.status = source.readString();
            this.method = source.readString();
            this.role = source.readString();
            this.etype = source.readString();
            this.displayName = source.readString();
        }


        /**
         * Write parcelable object
         *
         * @param dest The Parcel in which the object should be written
         * @param flags Additional flags about how the object should be written
         * @hide
         */
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(entity);
            dest.writeString(state);
            dest.writeString(status);
            dest.writeString(method);
            dest.writeString(role);
            dest.writeString(etype);
            dest.writeString(displayName);
        }

        /**
         * Parcelable creator
         *
         * @hide
         */
        public static final Parcelable.Creator<ConferenceUser> CREATOR
            = new Parcelable.Creator<ConferenceUser>() {
            public ConferenceUser createFromParcel(Parcel source) {
                return new ConferenceUser(source);
            }

            public ConferenceUser[] newArray(int size) {
                return new ConferenceUser[size];
            }
        };

        /**
         * Returns the role
         *
         * @return role
         */
        public String getRole() {
            return role;
        }

        /**
         * Returns the state
         *
         * @return state
         */
        public String getState() {
            return state;
        }

        /**
         * Returns the status
         *
         * @return status
         */
        public String getStatus() {
            return status;
        }

        /**
         * Returns the method
         *
         * @return method
         */
        public String getDisconnectMethod() {
            return method;
        }

        /**
         * Returns the etype
         *
         * @return etype
         */
        public String getEtype() {
            return etype;
        }

        /**
         * Returns the entity
         *
         * @return entity
         */
        public String getEntity() {
            return entity;
        }

        /**
         * Returns the display name
         *
         * @return displayName
         */
        public String getDisplayName() {
            return displayName;
        }

        @Override
        public int describeContents() {
            // TODO Auto-generated method stub
            return 0;
        }

        public static class State {
            public final static String FULL = "full";
            public final static String PARTIAL = "partial";
            public final static String DELETED = "deleted";
        }
        public static class Status {
            public final static String CONNECTED = "connected";
            public final static String DISCONNECTED = "disconnected";
            public final static String PENDING = "pending";
        }
        public static class Method {
            public final static String BOOTED = "booted";
            public final static String DEPARTED = "departed";
        }
        public static class Role {
            public final static String CHAIRMAN = "chairman";
            public final static String PARTICIPANT = "participant";
        }
    }

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }
}
