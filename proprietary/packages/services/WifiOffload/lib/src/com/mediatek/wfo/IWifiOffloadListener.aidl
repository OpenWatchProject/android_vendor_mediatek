/**
 * {@hide}
 */
package com.mediatek.wfo;

interface IWifiOffloadListener {
    oneway void onHandover(in int simIdx, in int stage, in int ratType);
    oneway void onRoveOut(in int simIdx, in boolean roveOut, in int rssi);
    oneway void onRequestImsSwitch(in int simIdx, in boolean isImsOn);
    oneway void onWifiPdnOOSStateChanged(in int simIdx, in int oosState);
    oneway void onAllowWifiOff();
    oneway void onWfcStateChanged(int simIdx, int state);
}
