package com.mediatek.lbs.em2.utils;

import java.util.ArrayList;

public class AgpsProfilingInfo {
    protected ArrayList<ProfileElement> elements;

    public AgpsProfilingInfo() {
        elements = new ArrayList<ProfileElement>();
    }

    public int getElementSize() {
        return elements.size();
    }

    public ArrayList<ProfileElement> getElements() {
        return elements;
    }

    public ProfileElement getElement(int index) {
        return elements.get(index);
    }

    public void addElement(int type, long timestamp, String message) {
        ProfileElement element = new ProfileElement(type, timestamp, message);
        elements.add(element);
    }

    public void clear() {
        elements.clear();
    }

    public String toString() {
        String ret = "";
        ret += "size=[" + elements.size() + "]\n";
        for (int i = 0; i < elements.size(); i++) {
            ret += "i=" + i + " " + elements.get(i) + "\n";
        }
        return ret;
    }

    public static class ProfileElement {
        public final static int PROFILING_TYPE_NORMAL = 0;
        public final static int PROFILING_TYPE_WARNING = 1;
        public final static int PROFILING_TYPE_ERROR = 2;
        protected int type;
        protected long timestamp;
        protected String message;

        public ProfileElement(int type, long timestamp, String message) {
            this.type = type;
            this.timestamp = timestamp;
            this.message = message;
        }

        public int getType() {
            return type;
        }

        public long getTimestamp() {
            return timestamp;
        }

        public String getMessage() {
            return message;
        }

        public void setType(int type) {
            this.type = type;
        }

        public void setTimestamp(long timestamp) {
            this.timestamp = timestamp;
        }

        public void setMessage(String message) {
            this.message = message;
        }

        public String toString() {
            String ret = "";
            ret += "type=[" + type + "] ";
            ret += "timestamp=[" + timestamp + "] ";
            ret += "message=[" + message + "] ";
            return ret;
        }
    }
}
