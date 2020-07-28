package com.mediatek.mdmlsample;

/**
 * Created by MTK03685 on 2015/9/16.
 */
public interface CommandProxyListener {
    void onCommandPreExecute();
    void onCommandProgressUpdate(int progressPercentage);
    void onCommandFinishUpdate(int commandResult);     // 0 : success   , others : error
}

/**
 *  To initialize MonitorTrapReceiver.
 */
interface TrapReceiverInitListener {
    void onTrapReceiverInit(long sessionId, String serverName);
}
