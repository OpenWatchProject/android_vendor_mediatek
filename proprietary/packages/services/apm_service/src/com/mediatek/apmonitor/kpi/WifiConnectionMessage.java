package com.mediatek.apmonitor.kpi;

/*
 * This class reprensent a connection related events
 */
public class WifiConnectionMessage extends KpiObjBaseRoot {

    private static final int VERSION = 1;
    /*
     * Association      (0)
     * Reassociation    (1)
     * Disconnection    (2)
     * Authentication   (3)
     * DeAuthentication (4)
     */
    public int mType = -1;

    /*
     * When type is 0, 1, or 3:
     *     Success (0)
     *     Fail    (1)
     * When type is 2 or 4:
     *     802.11 association/authentication standard reason code
     */
    public int mCode = -1;
    public String mBssid = null;
    public String mSsid = null;

    public WifiConnectionMessage() {
        super(VERSION);
    }

    @Override
    public void serialize() {
        /* type */
        writeInteger(mType);
        /* result or reason code */
        writeInteger(mCode);
        /* bssid */
        writeIntegerForStringLen(mBssid);
        /* ssid */
        writeIntegerForStringLen(mSsid);
        /* bssid */
        writeString(mBssid);
        /* ssid */
        writeString(mSsid);
    }

    @Override
    public void deserialize() {
        // Not implemented
    }
}
