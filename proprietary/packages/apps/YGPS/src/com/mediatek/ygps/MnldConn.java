package com.mediatek.ygps;

import android.net.LocalSocketAddress.Namespace;
import android.util.Log;

import com.mediatek.socket.base.UdpClient;
import com.mediatek.ygps.mnldinterface.Debug2MnldInterface;
import com.mediatek.ygps.mnldinterface.Debug2MnldInterface.Debug2MnldInterfaceSender;

/**
 * Class for connection and communication with mnld.
 */
public class MnldConn {
    private static final String TAG = "YGPS/MnldConn";

    private static final String CHANNEL_OUT = "mtk_debugService2mnld"; // send cmd to mnld
    private UdpClient mUdpClient;
    private Debug2MnldInterfaceSender mToMnldSender;


    /**
     * Constructor, initial parameters and start thread to send/receive.
     *
     */
    public MnldConn() {
        Log.v(TAG, "MnldConn constructor");

        mToMnldSender = new Debug2MnldInterfaceSender();
        mUdpClient = new UdpClient(CHANNEL_OUT, Namespace.ABSTRACT,
                Debug2MnldInterface.MAX_BUFF_SIZE);

    }

    void setTriggerNe(boolean on) {
        mToMnldSender.debugMnldNeMsg(mUdpClient, on);
    }

    void setMnldProp(String value) {
        mToMnldSender.debugMnldRadioMsg(mUdpClient, value);
    }

}
