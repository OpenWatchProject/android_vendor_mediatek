/**
 * {@hide}
 */
package com.mediatek.wfo;

import com.mediatek.wfo.IWifiOffloadListener;
import com.mediatek.wfo.DisconnectCause;

interface IWifiOffloadService {
    void registerForHandoverEvent(in IWifiOffloadListener listener);
    void unregisterForHandoverEvent(in IWifiOffloadListener listener);
    int getRatType(in int simIdx);
    DisconnectCause getDisconnectCause(in int simIdx);
    void setEpdgFqdn(in int simIdx, in String fqdn, in boolean wfcEnabled);
    void updateCallState(in int simIdx, in int callId, in int callType, in int callState);
    boolean isWifiConnected();
    void updateRadioState(in int simIdx, in int radioState);
    boolean setMccMncAllowList(in String[] allowList);
    String[] getMccMncAllowList(in int mode);
    void factoryReset();
    boolean setWifiOff();
}
