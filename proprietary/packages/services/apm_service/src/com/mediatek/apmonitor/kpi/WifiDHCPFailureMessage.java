package com.mediatek.apmonitor.kpi;

/*
 * This class reprensent a message to update PS status
 */
public class WifiDHCPFailureMessage extends KpiObjBaseRoot {

    private static final int VERSION = 1;
    /*
     * Provision failure (0)
     */
    public int mDHCPFailReason = -1;

    public WifiDHCPFailureMessage() {
        super(VERSION);
    }

    @Override
    public void serialize() {
        /* DHCP fail reason */
        writeInteger(mDHCPFailReason);
    }

    @Override
    public void deserialize() {
        // Not implemented
    }
}