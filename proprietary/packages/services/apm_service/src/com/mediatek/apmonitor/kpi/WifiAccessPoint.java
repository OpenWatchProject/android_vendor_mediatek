package com.mediatek.apmonitor.kpi;

import java.util.ArrayList;

/*
 * This class reprensent a Wi-Fi access point
 */
public class WifiAccessPoint extends KpiObjBase {
    /*
     * WEP(1)
     * WPA(2)
     * WPA2(3)
     * WPA-CCMP(4)
     * WPA-TKIP(5)
     * WPA2-CCMP(6)
     * WPA2-TKIP(7)
     * WPA2-Enterprise(8)
     */
    public int mSecurity = -1;
    public boolean mQosCapability = false;
    /* US(0) Others(1) */
    public int mCountryInfo = -1;
    public String mBssid = null;
    public String mSsid = null;
    public ArrayList<Integer> mSupporedtRates;
    public WifiChannel mWifiChannel;
    public WifiHTCapability mWifiHTCapability;

    @Override
    public void serialize() {
        // security
        writeInteger(mSecurity);
        // qosCapability
        writeBoolean(mQosCapability);
        // countryInfo
        writeInteger(mCountryInfo);
        // bssid_len
        writeIntegerForStringLen(mBssid);
        // ssid_len
        writeIntegerForStringLen(mSsid);
        // supportedRates_num
        if (mSupporedtRates != null) {
            writeInteger(mSupporedtRates.size()); // for mSupporedtRates
        } else {
            writeInteger(0);
        }
        // wifiScannedChannel_num
        writeInteger(1); //Only 1 for mWifiChannel
        // wifiHTCapability_num
        writeInteger(1); //Only 1 for mWifiHTCapability
        // bssid
        writeString(mBssid);
        // ssid
        writeString(mSsid);
        // supportedRates
        if (mSupporedtRates != null) {
            for (int i = 0; i < mSupporedtRates.size(); i++) {
                writeInteger(mSupporedtRates.get(i).intValue());
            }
        }
        // WifiChannel_v1
        byte[] wifiChannelBytes = mWifiChannel.build();
        writeByteArray(wifiChannelBytes.length, wifiChannelBytes);
        // WifiHTCapability_v1
        byte[] wifiHTCapabilityBytes = mWifiHTCapability.build();
        writeByteArray(wifiHTCapabilityBytes.length, wifiHTCapabilityBytes);
    }

    @Override
    public void deserialize() {
        // Not implemented
    }
}

