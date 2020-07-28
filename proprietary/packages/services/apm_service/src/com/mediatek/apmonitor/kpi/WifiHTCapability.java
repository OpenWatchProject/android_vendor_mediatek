package com.mediatek.apmonitor.kpi;

/*
 * This class reprensent HT capability of an access point
 */
public class WifiHTCapability extends KpiObjBase {

    public int mChannelWidth = -1;
    public int mSecondaryChannelOffset = -1;

    @Override
    public void serialize() {
        writeInteger(mChannelWidth);
        writeInteger(mSecondaryChannelOffset);
    }

    @Override
    public void deserialize() {
        // Not implemented
    }
}

