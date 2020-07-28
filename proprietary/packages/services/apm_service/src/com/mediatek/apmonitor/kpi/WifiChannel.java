package com.mediatek.apmonitor.kpi;

/*
 * This class reprensent a Wi-Fi channel
 */
public class WifiChannel extends KpiObjBase {

    public int mWifiFreq = -1;
    public int mChannelNum = -1;

    @Override
    public void serialize() {
        writeInteger(mWifiFreq);
        writeInteger(mChannelNum);
    }

    @Override
    public void deserialize() {
        // Not implemented
    }
}

