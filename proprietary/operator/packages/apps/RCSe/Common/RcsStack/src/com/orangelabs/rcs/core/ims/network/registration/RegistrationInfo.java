package com.orangelabs.rcs.core.ims.network.registration;

import java.util.Collection;
import java.util.HashMap;

public class RegistrationInfo {

    public static class State {
        public static final String FULL = "full";
        public static final String PARTIAL = "partial";
    }

    public RegistrationInfo(int version, String state) {
        this.version = version;
        this.state = state;
    }

    public int getVersion() {
        return version;
    }

    public String getState() {
        return state;
    }

    public Registration getRegistration(String id) {
        return registrations.get(id);
    }

    public Collection<Registration> getAllRegistrations() {
        return registrations.values();
    }

    public void addRegistration(Registration registration) {
        registrations.put(registration.getId(), registration);
    }

    public void removeRegistration(Registration registration) {
        registrations.remove(registration.getId());
    }

    public static class Registration {
        public static class State {
            public static final String INIT = "init";
            public static final String ACTIVE = "active";
            public static final String TERMINATED = "terminated";
        }

        public Registration(String id, String state, String aor) {
            this.id = id;
            this.state = state;
            this.aor = aor;
        }

        public String getId() {
            return id;
        }

        public String getState() {
            return state;
        }

        public String getAor() {
            return aor;
        }

        public Contact getContact(String id) {
            return contacts.get(id);
        }

        public Collection<Contact> getAllContacts() {
            return contacts.values();
        }

        public void addContact(Contact contact) {
            contacts.put(contact.getId(), contact);
        }

        public void removeContact(Contact contact) {
            contacts.remove(contact.getId());
        }

        private String aor;
        private String id;
        private String state;
        private HashMap<String, Contact> contacts = new HashMap<String, Contact>();
    }

    public static class Contact {
        public static class State {
            public static final String ACTIVE = "active";
            public static final String TERMINATED = "terminated";
        }

        public static class Event {
            public static final String REGISTERED = "registered";
            public static final String CREATED = "created";
            public static final String REFRESHED = "refreshed";
            public static final String SHORTENED = "shortened";
            public static final String EXPIRED = "expired";
            public static final String DEACTIVATED = "deactivated";
            public static final String PROBATION = "probation";
            public static final String UNREGISTERED = "unregistered";
            public static final String REJECTED = "rejected";
        }

        public Contact(String id, String state, String event) {
            this.id = id;
            this.state = state;
            this.event = event;
        }

        public String getId() {
            return id;
        }

        public String getState() {
            return state;
        }

        public String getEvent() {
            return event;
        }

        public String getUri() {
            return uri;
        }

        public String getDisplayName() {
            return displayName;
        }

        public int getDuration() {
            return duration;
        }

        public int getExpires() {
            return expires;
        }

        public int getRetryAfter() {
            return retryAfter;
        }

        public String getCallId() {
            return callId;
        }

        public String getCSeq() {
            return cSeq;
        }

        public int getPriority() {
            return priority;
        }

        public void setDisplayName(String displayName) {
            this.displayName = displayName;
        }

        public void setUri(String uri) {
            this.uri = uri;
        }

        public void setDuration(int duration) {
            this.duration = duration;
        }

        public void setExpires(int expires) {
            this.expires = expires;
        }

        public void setRetryAfter(int retryAfter) {
            this.retryAfter = retryAfter;
        }

        public void setCallId(String callId) {
            this.callId = callId;
        }

        public void setCSeq(String cSeq) {
            this.cSeq = cSeq;
        }

        public void setPriority(int priority) {
            this.priority = priority;
        }

        private String uri;
        private String id;
        private String state;
        private String event;
        private String displayName;
        private int duration;
        private int expires;
        private int retryAfter;
        private String callId;
        private String cSeq;
        private int priority;
    }

    private int version;
    private String state;

    private HashMap<String, Registration> registrations = new HashMap<String, Registration>();
}
