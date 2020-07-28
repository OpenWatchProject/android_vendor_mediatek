package com.mediatek.apmonitor.kpi;

import java.util.ArrayList;

/*
 * This class reprensent a connection related events
 */
public class WifiScanMessage extends KpiObjBaseRoot {
    private static final int VERSION = 1;
    public long mTimeScanStart = 0;
    public long mTimeScanEnd = 0;
    /* Passive(1) Active(2) */
    public int mScanType = 2;
    public ArrayList<WifiChannel> mWifiChannels;
    public ArrayList<WifiAccessPoint> mAccessPoints;

    public WifiScanMessage() {
        super(VERSION);
    }

    @Override
    public void serialize() {
        /* Timestamp of scan start */
        writeLong(mTimeScanStart);
        /* Timestamp of scan end */
        writeLong(mTimeScanEnd);
        /* Scan type */
        writeInteger(mScanType);
        /* channel numbers */
        writeInteger(mWifiChannels.size());
        /* access point numbers */
        writeInteger(mAccessPoints.size());
        /* Scanned Wi-Fi channels */
        for (int i = 0; i < mWifiChannels.size(); i++) {
            WifiChannel ch = mWifiChannels.get(i);
            byte[] wifiChannelBytes = ch.build();
            writeByteArray(wifiChannelBytes.length, wifiChannelBytes);
        }
        /* Scanned Wi-Fi access points */
        for (int i = 0; i < mAccessPoints.size(); i++) {
            WifiAccessPoint ap = mAccessPoints.get(i);
            byte[] accessPointBytes = ap.build();
            writeByteArray(accessPointBytes.length, accessPointBytes);
        }
    }

    @Override
    public void deserialize() {
        // Not implemented
    }
}
