package com.mediatek.mdmlsample;

/**
 * Created by MTK03685 on 2015/9/15.
 */
public class ConnectionState {  // Current use public for each member variable as structure object
    // basic connection
    public long m_sid = 0;  // session id
    public String m_serverName = "demo_receiver";  // hard code demo server name
    public boolean m_bConnected = false;

    // operation state
    public boolean m_bSetupDefaultSubscribe = false;
    public boolean m_bTrapEnabled = false;  // is enable MDM
    public boolean m_bCommandProcessing = false;  // command proxy is executing now (used to disable UI widget)

    // setter


    // getter
}
