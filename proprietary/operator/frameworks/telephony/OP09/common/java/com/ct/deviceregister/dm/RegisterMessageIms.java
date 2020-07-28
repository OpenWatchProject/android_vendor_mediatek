package com.ct.deviceregister.dm;

import com.ct.deviceregister.dm.utils.PlatformManager;

public class RegisterMessageIms extends RegisterMessage {

    private static final int LENGTH_MAX_MESSAGE_IMS = 136;
    private static final byte PROTOCOL_VERSION_IMS = 0x03;

    private static final String STUFF_IMEI = "000000000000000";

    private PlatformManager mPlatformManager;
    private int mSlot;

    public RegisterMessageIms(PlatformManager platformManager, int slot) {
        mPlatformManager = platformManager;
        mSlot = slot;
    }

    public String encryptImeiImsi() {
        String message = String.format("<a1><b1>%s<b2>%s<b3>%s<b4>%s<b5>%s</a1>",
                getModel(), PlatformManager.encryptMessage(getImei1()),
                PlatformManager.encryptMessage(getImei2()),
                PlatformManager.encryptMessage(getLteImsi()), getSoftwareVersion());
        return trimIfNeed(new StringBuilder(message));
    }

    @Override
    protected String[] getStartTags() {
        return new String[]{"<b1>", "<b2>", "<b3>", "<b4>", "<b5>"};
    }

    @Override
    protected String[] getEndTags() {
        return new String[]{"", "", "", "", ""};
    }

    @Override
    protected int getLengthOfEndTag() {
        // length of </a1>
        return 5;
    }

    @Override
    protected String[] getContent() {
        return new String[]{getModel(), getImei1(), getImei2(), getLteImsi(), getSoftwareVersion()};
    }

    @Override
    protected int getMaxLength() {
        // In SMS module, return null if (data.length + 1) > 140.
        // As header has 4 bytes, the max length here is 135
        return LENGTH_MAX_MESSAGE_IMS - 1;
    }

    @Override
    protected byte getProtocolVersion() {
        return PROTOCOL_VERSION_IMS;
    }

    @Override
    protected String generateChecksum(byte[] data) {
        return "";
    }

    /**
     *
     * @return IMEI for slot to send message, not slot 1
     */
    private String getImei1() {
        return mPlatformManager.getImei(mSlot);
    }

    /**
     *
     * @return IMEI for the other slot (not send message), not slot 2
     */
    private String getImei2() {
        if (mPlatformManager.isSingleLoad()) {
            return STUFF_IMEI;
        } else {
            if (mSlot == Const.SLOT_ID_0) {
                return mPlatformManager.getImei(Const.SLOT_ID_1);
            } else {
                return mPlatformManager.getImei(Const.SLOT_ID_0);
            }
        }
    }

    private String getLteImsi() {
        return mPlatformManager.getImsiInfo(mSlot);
    }
}
