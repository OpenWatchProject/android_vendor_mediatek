package com.mediatek.gnssdebugreport;

import android.net.LocalSocketAddress.Namespace;
import android.util.Log;

import com.mediatek.gnssdebugreport.mnldinterface.Debug2MnldInterface;
import com.mediatek.gnssdebugreport.mnldinterface.Debug2MnldInterface.Debug2MnldInterfaceSender;
import com.mediatek.gnssdebugreport.mnldinterface.DebugReqStatusCategory;
import com.mediatek.gnssdebugreport.mnldinterface.Mnld2DebugInterface;
import com.mediatek.gnssdebugreport.mnldinterface.Mnld2DebugInterface.Mnld2DebugInterfaceReceiver;
import com.mediatek.gnssdebugreport.mnldinterface.MnldGpsStatusCategory;
import com.mediatek.socket.base.UdpClient;
import com.mediatek.socket.base.UdpServer;


/**
 * Class for connection and communication with mnld.
 */
public class MnldConn {
    private static final String TAG = "GnssDebugReport/MnldConn";

    private static final String CHANNEL_OUT = "mtk_debugService2mnld"; // send cmd to mnld
    private static final String CHANNEL_IN  = "mtk_mnld2debugService"; // get cmd from mnld
    private UdpClient mUdpClient;
    private UdpServer mUdpServer;
    private Debug2MnldInterfaceSender mToMnldSender;

    private OnResponseListener mCallBack = null;
    private Object mMnldAckLock = new Object();

    private static final int RETRY_TIMES = 20;
    private static final int RETRY_INTERVAL = 1000;
    private static final int WAITING_ACK_INTERVAL = 200;


    private boolean mMnldAckRec = false;
    private DebugStartThread mDebugStartThread = null;
    private DebugStopThread mDebugStopThread = null;


    /**
     * Interface to hand data from socket.
     *
     */
    public interface OnResponseListener {
        /**
         * @param response data from socket
         */
        void onDebugInfoResp(String response);
    }

    void startDebug() {
        mDebugStartThread = new DebugStartThread();
        mDebugStartThread.start();
    }

    void stopDebug() {
        mDebugStopThread = new DebugStopThread();
        mDebugStopThread.start();

    }

    /**
     * Constructor, initial parameters and start thread to send/receive.
     *
     * @param callBack
     *            Callback when message received
     */
    public MnldConn(OnResponseListener callBack) {
        Log.v(TAG, "MnldConn constructor");
  
        mToMnldSender = new Debug2MnldInterfaceSender();
        mUdpClient = new UdpClient(CHANNEL_OUT, Namespace.ABSTRACT,
                Debug2MnldInterface.MAX_BUFF_SIZE);

        mUdpServer = new UdpServer(CHANNEL_IN, Namespace.ABSTRACT,
                Mnld2DebugInterface.MAX_BUFF_SIZE);
        mUdpServer.setReceiver(mReceiver);
        this.mCallBack = callBack;

    }


    class DebugStartThread extends Thread {
        public void run() {
            if (!connect(true)) {
                Log.e(TAG, "Connect fail");
            }

            Log.d(TAG, "mDebugStartThread end");
        }
    }


    class DebugStopThread extends Thread {
        public void run() {
            if (!connect(false)) {
                Log.e(TAG, "Connect fail");
            }
            Log.d(TAG, "mDebugStopThread end");
        
        }
    }

    /**
     * Start client socket and connect with server.
     */

    boolean connect(boolean on) {
        mMnldAckRec = false;
        for (int k = 0; k < RETRY_TIMES; k++) {
            Log.d(TAG, "connect times:" + k + " state:" + on);
            boolean ret = mToMnldSender.debugReqMnldMsg(mUdpClient,
                    on ? DebugReqStatusCategory.startDebug : DebugReqStatusCategory.stopDebug);
            if (!ret) {
                return false;
            }
            int checkTimes = RETRY_INTERVAL / WAITING_ACK_INTERVAL;
            for (int i = 0; i < checkTimes; i++) {
                sleep(WAITING_ACK_INTERVAL);
                synchronized(mMnldAckLock) {
                    if (mMnldAckRec) {
                        return true;
                    }
                }
            }
        }
        return false;
    }


    private Mnld2DebugInterfaceReceiver mReceiver = new Mnld2DebugInterfaceReceiver() {
        @Override
        public void mnldAckDebugReq() {
            Log.d(TAG, "receive mnld Ack debug request");
            synchronized(mMnldAckLock) {
                mMnldAckRec = true;
            }

        }

        @Override
        public void mnldUpdateReboot() {
            Log.d(TAG, "receive mnld update reboot");
            startDebug();
        }

        @Override
        public void mnldUpdateMessageInfo(String msg) {
            Log.d(TAG, "receive mnld debug message: " + msg);
            mCallBack.onDebugInfoResp(msg);
        }

        @Override
        public void mnldUpdateGpsStatus(MnldGpsStatusCategory status) {
            // TODO Auto-generated method stub
            switch(status) {
            case gpsStarted:
                Log.d(TAG, "receive mnld updated gps started status");
                break;
            case gpsStopped:
                Log.d(TAG, "receive mnld updated gps stop status");
                break;
            default:
                break;
            }
        }
    };


    private void sleep(long time) {
        // TODO: not exactly
        try {
            Thread.sleep(time);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
