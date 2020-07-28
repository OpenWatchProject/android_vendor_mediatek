package com.mediatek.apmonitor.kpi;

/*
 * This class reprensent a general status of Wi-Fi
 */
public class WifiInfoMessage extends KpiObjBaseRoot {

    private static final int VERSION = 1;
    public int mLinkSpeed = -1;
    public int mRssi = -1;
    public int mNetworkID = -1;
    public boolean mHiddenSSID = false;
    public String mBssid = null;
    public String mSsid = null;
    public String mDetailState = null;
    public String mSupplicantState = null;
    public String mIp = null;
    public String mMacAddress = null;
    public String mGateway = null;
    public String mNetMask = null;
    public String mDns1 = null;
    public String mDns2 = null;
    public String mServerIP = null;

    public WifiInfoMessage() {
        super(VERSION);
    }

    @Override
    public void serialize() {
        /* linkSpeed */
        writeInteger(mLinkSpeed);
        /* rssi */
        writeInteger(mRssi);
        /* networkID */
        writeInteger(mNetworkID);
        /* hiddenSSID */
        writeBoolean(mHiddenSSID);

        /* bssid */
        writeIntegerForStringLen(mBssid);
        /* ssid */
        writeIntegerForStringLen(mSsid);
        /* detailState */
        writeIntegerForStringLen(mDetailState);
        /* supplicantState */
        writeIntegerForStringLen(mSupplicantState);
        /* ip */
        writeIntegerForStringLen(mIp);
        /* macAddress */
        writeIntegerForStringLen(mMacAddress);
        /* gateway */
        writeIntegerForStringLen(mGateway);
        /* netMask */
        writeIntegerForStringLen(mNetMask);
        /* dns1 */
        writeIntegerForStringLen(mDns1);
        /* dns2 */
        writeIntegerForStringLen(mDns2);
        /* serverIP */
        writeIntegerForStringLen(mServerIP);

        /* bssid */
        writeString(mBssid);
        /* ssid */
        writeString(mSsid);
        /* detailState */
        writeString(mDetailState);
        /* supplicantState */
        writeString(mSupplicantState);
        /* ip */
        writeString(mIp);
        /* macAddress */
        writeString(mMacAddress);
        /* gateway */
        writeString(mGateway);
        /* netMask */
        writeString(mNetMask);
        /* dns1 */
        writeString(mDns1);
        /* dns2 */
        writeString(mDns2);
        /* serverIP */
        writeString(mServerIP);
    }

    @Override
    public void deserialize() {
        // Not implemented
    }
}