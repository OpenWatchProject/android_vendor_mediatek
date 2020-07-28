package com.mediatek.apmonitor.kpi;

/*
 * This class reprensent a message to update PS status
 */
public class WifiPSStatusMessage extends KpiObjBaseRoot {

    private static final int VERSION = 1;
    public boolean mPSModeOn = false;

    public WifiPSStatusMessage() {
        super(VERSION);
    }

    @Override
    public void serialize() {
        /* Power save mode on */
        writeBoolean(mPSModeOn);
    }

    @Override
    public void deserialize() {
        // Not implemented
    }
}