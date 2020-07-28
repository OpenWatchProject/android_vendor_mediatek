package com.mediatek.apmonitor.kpi;

public abstract class KpiObjBaseRoot extends KpiObjBase {
    public static final int INVALID_VERSION = -1;

    private int mVersion = INVALID_VERSION;

    public KpiObjBaseRoot(int version) {
        mVersion = version;
    }
    private KpiObjBaseRoot() {};

    public int version() {
        return mVersion;
    }

    @Override
    public byte[] build() {
        // This is root KPI, prepend version bytes.
        writeInteger(mVersion);
        return super.build();
    }
}