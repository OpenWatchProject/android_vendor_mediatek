
package com.mediatek.ims.internal;

import java.util.LinkedList;
import java.util.List;

/**
 * Represents dialog-info element in Dialog Event Package.
 */
public class DialogInfo {
    private List<Dialog> mDialogs = new LinkedList<Dialog>();

    boolean addDialog(Dialog dialog) {
        return mDialogs.add(dialog);
    }

    List<Dialog> getDialogs() {
        return mDialogs;
    }

    /**
     * Represents dialog element in Dialog Event Package.
     */
    public static class Dialog {
        private int mDialogId;
        private boolean mExclusive = true;
        private String mState = "";
        private Local mLocal = new Local();

        /**
         * Initializes the dialog object.
         *
         * @param dialogId the value of id attribute
         * @param exclusive the value of sa:exclusive element
         * @param state the value of state element
         * @param local the object represents the local element
         */
        public Dialog(int dialogId, boolean exclusive, String state, Local local) {
            this.mDialogId = dialogId;
            this.mExclusive = exclusive;
            this.mState = state;
            this.mLocal = local;
        }

        int getDialogId() {
            return mDialogId;
        }

        boolean getExclusive() {
            return mExclusive;
        }

        String getState() {
            return mState;
        }

        Local getLocal() {
            return mLocal;
        }

        String getIdentity() {
            return mLocal.getIdentity();
        }

        String getTargetUri() {
            return mLocal.getTargetUri();
        }

        List<MediaAttribute> getMediaAttributes() {
            return mLocal.getMediaAttributes();
        }

        String getPname() {
            return mLocal.getPname();
        }

        String getPval() {
            return mLocal.getPval();
        }
    }

    /**
     * Represents local element in Dialog Event Package.
     */
    public static class Local {
        private String mIdentity = "";
        private String mTargetUri = "";
        private List<MediaAttribute> mMediaAttributes = new LinkedList<MediaAttribute>();
        private Param mParam = new Param();

        public void setIdentity(String identity) {
            this.mIdentity = identity;
        }

        String getIdentity() {
            return mIdentity;
        }

        public void setTargetUri(String targetUri) {
            this.mTargetUri = targetUri;
        }

        String getTargetUri() {
            return mTargetUri;
        }

        boolean addMediaAttribute(MediaAttribute mediaAttribute) {
            return mMediaAttributes.add(mediaAttribute);
        }

        List<MediaAttribute> getMediaAttributes() {
            return mMediaAttributes;
        }

        void setParam(Param param) {
            this.mParam = param;
        }

        String getPname() {
            return mParam.getPname();
        }

        String getPval() {
            return mParam.getPval();
        }
    }

    /**
     * Represents mediaAttributes element in Dialog Event Package.
     */
    public static class MediaAttribute {
        private String mMediaType = "";
        private String mMediaDirection = "";
        private boolean mPort0 = false;

        /**
         * Initializes the MediaAttribute object.
         *
         * @param mediaType the value of mediaType element
         * @param mediaDirection the value of mediaDirection element
         * @param port0 if mediaAttribute contains &lt;port0/&gt; element, which represents a
         *            downgraded video call
         */
        public MediaAttribute(String mediaType, String mediaDirection, boolean port0) {
            this.mMediaType = mediaType;
            this.mMediaDirection = mediaDirection;
            this.mPort0 = port0;
        }

        String getMediaType() {
            return mMediaType;
        }

        String getMediaDirection() {
            return mMediaDirection;
        }

        boolean isDowngradedVideo() {
            return mPort0;
        }
    }

    /**
     * Represents param element in Dialog Event Package.
     */
    public static class Param {
        private String mPnam = "";
        private String mPval = "";

        /**
         * Initializes the Param object.
         */
        public Param() {
        }

        /**
         * Initializes the Param object.
         *
         * @param pname value of pname attribute
         * @param pval value of pval attribute
         */
        public Param(String pname, String pval) {
            this.mPnam = pname;
            this.mPval = pval;
        }

        String getPname() {
            return mPnam;
        }

        String getPval() {
            return mPval;
        }
    }
}
