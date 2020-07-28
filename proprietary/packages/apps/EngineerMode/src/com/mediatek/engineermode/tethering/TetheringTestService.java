package com.mediatek.engineermode.tethering;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.LinkProperties;
import android.net.Network;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.text.TextUtils;

import com.mediatek.engineermode.Elog;

import java.util.ArrayList;

import vendor.mediatek.hardware.netdagent.V1_0.INetdagent;

/**
 * Class for firewall control in background service.
 *
 */
public class TetheringTestService extends Service {
    private static final String TAG = "TetheringTestService";
    private Context mContext;
    private Network mCurrentNetwork;

    private boolean mIsNetworkAvailable;
    private boolean mIsTetheringOn;
    private boolean mIsNsIotCmdSent;

    private String mDownIface = "";
    private String mUpstreamIface = "";

    private static ConnectivityManager sConnMgr;
    private static TetheringHandler sTetherHandler;

    private static final int EVENT_TEST_STATE_CHANGED = 0;

    /**
     * Activity onCreate function.
     *
     */
    @Override
    public void onCreate() {
        super.onCreate();

        mContext = getApplicationContext();
        sConnMgr = (ConnectivityManager) mContext.getSystemService(
                Context.CONNECTIVITY_SERVICE);
        mContext = this.getApplicationContext();

        HandlerThread tetherThread = new HandlerThread(TAG);
        tetherThread.start();
        sTetherHandler = new TetheringHandler(tetherThread.getLooper());

        Elog.d(TAG, "onCreate");
        init();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String action = intent.getAction();
        Elog.d(TAG, "onStartCommand:" + action);
        return Service.START_STICKY;
    }

    private void init() {
        final IntentFilter filter = new IntentFilter();
        filter.addAction(ConnectivityManager.ACTION_TETHER_STATE_CHANGED);
        mContext.registerReceiver(mBroadcastReceiver, filter);

        sConnMgr.registerDefaultNetworkCallback(mNetworkCallback);
        mCurrentNetwork = sConnMgr.getActiveNetwork();
        mIsNetworkAvailable = (mCurrentNetwork != null) ? true : false;
        String[] tethered = sConnMgr.getTetheredIfaces();
        mIsTetheringOn  = (tethered.length != 0) ? true : false;
        mIsNsIotCmdSent = false;

        sTetherHandler.sendEmptyMessage(EVENT_TEST_STATE_CHANGED);
    }

    private void uninit() {
        enableNsIotTest(false);
        mContext.unregisterReceiver(mBroadcastReceiver);
        sConnMgr.unregisterNetworkCallback(mNetworkCallback);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        uninit();
        super.onDestroy();
    }

    private void enableNsIotTest(boolean enabled) {
        // Todo

        String[] tethered = sConnMgr.getTetheredIfaces();
        if (tethered.length != 1) {
            Elog.e(TAG, "tethered iface:" + tethered.length);
        } else {
            mDownIface = tethered[0];
        }

        if (mCurrentNetwork != null) {
            LinkProperties link = sConnMgr.getLinkProperties(mCurrentNetwork);
            if (link != null) {
                mUpstreamIface = link.getInterfaceName();
            } else {
                Elog.e(TAG, "No interface for upstreaming");
            }
        } else {
            Elog.e(TAG, "Current network is null");
        }

        if (mDownIface.length() == 0 || mUpstreamIface.length() == 0) {
            Elog.e(TAG, "network interface is not ready");
            return;
        }

        Elog.i(TAG, "Upstream iface:" + mUpstreamIface
                + " downstream iface:" + mDownIface);

        try {
            // Get IP address here
            INetdagent agent = INetdagent.getService();
            if (agent == null) {
                Elog.e(TAG, "agnet is null");
                return;
            }
            String cmd = "";
            if (enabled) {
                // Get the IP address of USB client
                agent.dispatchNetdagentCmd("netdagent firewall get_usb_client " + mDownIface);
                Thread.sleep(500);
                String ipaddr = SystemProperties.get("vendor.net.rndis.client");
                if (ipaddr.length() == 0) {
                    Thread.sleep(500);
                    agent.dispatchNetdagentCmd("netdagent firewall get_usb_client " + mDownIface);
                    ipaddr = SystemProperties.get("vendor.net.rndis.client");
                }
                Elog.i(TAG, "ipaddr from vendor.net.rndis.client:" + ipaddr);
                //Send command
                cmd = "netdagent firewall set_udp_forwarding " + mDownIface + " " +
                        mUpstreamIface + " " + ipaddr;
            } else {
                cmd = "netdagent firewall clear_udp_forwarding " + mDownIface + " " +
                        mUpstreamIface;
            }
            Elog.i(TAG, "Send " + cmd);
            agent.dispatchNetdagentCmd(cmd);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private final NetworkCallback mNetworkCallback = new NetworkCallback() {
        @Override
        public void onAvailable(Network network) {
            String iface = "";
            mCurrentNetwork = network;
            mIsNetworkAvailable = true;

            LinkProperties link = sConnMgr.getLinkProperties(network);
            if (link != null) {
                iface = link.getInterfaceName();
            } else {
                Elog.e(TAG, "No interface for upstreaming");
            }

            Elog.d(TAG, "onAvailable:" + network + ":" + iface);
            if (!TextUtils.isEmpty(mUpstreamIface) &&
                    !TextUtils.isEmpty(iface)) {
                Elog.d(TAG, "Need to update upstraming iface:" + mUpstreamIface);
            }

            sTetherHandler.sendEmptyMessage(EVENT_TEST_STATE_CHANGED);
        }

        @Override
        public void onLost(Network network) {
            if (network.equals(mCurrentNetwork)) {
                mCurrentNetwork = null;
                mIsNetworkAvailable = false;
                sTetherHandler.sendEmptyMessage(EVENT_TEST_STATE_CHANGED);
            }
            Elog.d(TAG, "onLost:" + network);
        };
    };

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            switch (intent.getAction()) {
                case ConnectivityManager.ACTION_TETHER_STATE_CHANGED:
                    ArrayList<String> activeTetherIfaces = intent.getStringArrayListExtra(
                        ConnectivityManager.EXTRA_ACTIVE_TETHER);

                    mIsTetheringOn = activeTetherIfaces != null
                            && activeTetherIfaces.size() > 0;
                    Elog.d(TAG, "Tethering " + (mIsTetheringOn ? "on" : "off"));
                    sTetherHandler.sendEmptyMessage(EVENT_TEST_STATE_CHANGED);
                    break;
                default:
                    break;
            }
        }
    };

    /** Handler to do the tethering. */
    private class TetheringHandler extends Handler {

        public TetheringHandler(Looper l) {
            super(l);
        }

        @Override
        public void handleMessage(Message msg) {
            int i = msg.arg1;
            switch (msg.what) {
                case EVENT_TEST_STATE_CHANGED:
                    if (mIsNetworkAvailable && mIsTetheringOn) {
                        enableNsIotTest(true);
                        mIsNsIotCmdSent = true;
                    } else if (mIsNsIotCmdSent && !mIsNetworkAvailable) {
                        enableNsIotTest(false);
                        mIsNsIotCmdSent = false;
                    } else if (mIsNsIotCmdSent && !mIsTetheringOn) {
                        enableNsIotTest(false);
                        mIsNsIotCmdSent = false;
                    }
                    Elog.i(TAG, "NSIOT enabled flag:" + mIsNsIotCmdSent
                        + " network:" + mIsNetworkAvailable
                        + " tethering:" + mIsTetheringOn);
                    break;
                default:
                    break;
            }
        }
    }

}
