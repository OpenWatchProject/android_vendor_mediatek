package com.mediatek.mwi;

public class MwiHeader {
    // Useful information of Mwi header
    private String mMsgAccount;

    public enum Label {
        MsgWait("Messages-Waiting"), MsgAccount("Message-Account"), VoiceMsg("Voice-Message"),
        VideoMsg("Video-Message"), FaxMsg("Fax-Message"), TextMsg("Text-Message");

        private String mLabel;
        private Label(String label) {
            mLabel = label;
        }
        public String getLabel() {
            return mLabel;
        }
    }

    public String getMsgAccount() {
        return mMsgAccount;
    }

    public void setMsgAccount(String msgAccount) {
        this.mMsgAccount = msgAccount;
    };
}
