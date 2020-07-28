package com.mediatek.datashaping;

/** {@hide} */
interface IDataShapingManager
{
    void enableDataShaping();
    void disableDataShaping();
    boolean openLteDataUpLinkGate(boolean isForce);
    /// M: integrate Doze and App Standby
    void setDeviceIdleMode(boolean enabled);
    boolean isDataShapingWhitelistApp(String packageName);
}
