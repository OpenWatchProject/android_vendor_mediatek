package com.orangelabs.rcs.core.ims.service;

public class SubscribeRequest {

    private SubscribeRequest(
            String requestUri,
            String event,
            String accept,
            String contentType,
            byte[] content,
            int expirePeriod) {
        this.target = requestUri;
        this.eventName = event;
        this.acceptContent = accept;
        this.contentType = contentType;
        this.content = content;
        this.expirePeriod = expirePeriod;
    }

    public String getRequestUri() {
        return target;
    }

    public String getSubscribeEvent() {
        return eventName;
    }

    public String getAcceptContent() {
        return acceptContent;
    }

    public int getExpirePeriod() {
        return expirePeriod;
    }

    public String getContentType() {
        return contentType;
    }

    public byte[] getContent() {
        return content;
    }

    /**
     */
    public static class Builder {

        /**
         * Default constructor for Builder.
         */
        public Builder() {}

        /**
         */
        public SubscribeRequest build() {
            return new SubscribeRequest(
                    mTarget, mEvent, mAcceptContent, mContentType, mContent, mExpirePeriod);
        }

        public Builder setRequestUri(String requestUri) {
            mTarget = requestUri;
            return this;
        }

        public Builder setSubscibeEvent(String event) {
            mEvent = event;
            return this;
        }

        public Builder setAcceptContent(String accept) {
            mAcceptContent = accept;
            return this;
        }

        public Builder setExpirePeriod(int expire) {
            mExpirePeriod = expire;
            return this;
        }

        public Builder setContent(String contentType, byte[] content) {
            mContentType = contentType;
            mContent = content;
            return this;
        }

        private String mTarget = null;
        private String mEvent = null;
        private String mAcceptContent = null;
        private String mContentType = null;
        private byte[] mContent = null;
        private int mExpirePeriod = -1;
    }

    private String target;
    private String eventName;
    private String acceptContent;
    private String contentType;
    private byte[] content;
    private int expirePeriod = -1;
}
