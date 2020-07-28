package com.mediatek.nlpservice;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.net.ConnectivityManager;
import android.net.LocalSocketAddress.Namespace;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.os.HandlerThread;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.util.Log;

import com.mediatek.nlpservice.Mnld2NlpUtilsInterface.Mnld2NlpUtilsInterfaceReceiver;
import com.mediatek.socket.base.UdpClient;
import com.mediatek.socket.base.UdpServer;


public class LbsNlpUtils {
    private static final boolean DEBUG = true;

    // Messages for internal handler
    private final static int NLPS_MSG_GPS_STARTED = 0;
    private final static int NLPS_MSG_GPS_STOPPED = 1;
    private final static int NLPS_MSG_NLP_NIJ_REQ = 2;
    private final static int NLPS_MSG_NLP_NIJ_CANCEL = 3;
    private final static int NLPS_MSG_NLP_UPDATED = 4;

    private final static int NLP_CMD_SRC_UNKNOWN = 0;  // for legacy mnld, without request src
    private final static int NLP_CMD_SRC_MNL = (1 << 0);
    private final static int NLP_CMD_SRC_APM = (1 << 1);

    protected final static String SOCKET_ADDRESS = "mtk_mnld2nlputils";
    private static final int LAST_LOCATION_EXPIRED_TIMEOUT = (10*60*1000); //ms
    private static final String INJECT_NLP_LOC = "com.mediatek.location.INJECT_NLP_LOC";
    private static final String INJECT_NLP_PERMISSION = "com.mediatek.permission.INJECT_NLP_LOC";
    private LocationManager mLocationManager;
    private NlpsMsgHandler mHandler;
    private Context mContext;
    private int mNlpRequestedSrc = 0;
    private BroadcastReceiver mBroadcastReceiver;
    private IntentFilter mIntentFilter;
    private boolean mNetworkConnected = true;
    private Location mLastLocation = null;

    private UdpServer nlpUtilsServer;

    public LbsNlpUtils(Context context) {
        if (DEBUG) log("onCreate");
        mLocationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        mContext = context;

        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        mBroadcastReceiver = new BroadcastReceiver() {
            @Override
                public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (action.equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
                    connectivityAction(intent);
                }
            }
        };
        mContext.registerReceiver(mBroadcastReceiver, mIntentFilter);

        HandlerThread handlerThread = new HandlerThread("[LbsNlpUtils]");
        handlerThread.start();
        mHandler = new NlpsMsgHandler(handlerThread.getLooper());

        int buffSizeIn  = Mnld2NlpUtilsInterface.MAX_BUFF_SIZE;
        nlpUtilsServer = new UdpServer(SOCKET_ADDRESS, Namespace.ABSTRACT, buffSizeIn);
        nlpUtilsServer.setReceiver(mReceiver);
    }

    private void connectivityAction(Intent intent) {
        NetworkInfo info =
                intent.getParcelableExtra(ConnectivityManager.EXTRA_NETWORK_INFO);
        ConnectivityManager connManager = (ConnectivityManager)
                mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        info = info != null ? connManager.getNetworkInfo(info.getType()) : null;

        int networkState;
        if (intent.getBooleanExtra(ConnectivityManager.EXTRA_NO_CONNECTIVITY, false) ||
                (info != null && !info.isConnected()) ) {
            log("Connectivity set unConnected");
            mNetworkConnected = false;
        } else {
            log("Connectivity set connected");
            mNetworkConnected = true;
        }
    }

    public void releaseListeners() {
        nlpUtilsServer.close();
        mContext.unregisterReceiver(mBroadcastReceiver);
    }

    private boolean isNlpEnabled() {
        return mLocationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
    }

    private void startNlpQueryLocked(int src) {
        boolean isNlpEnabled = isNlpEnabled();
        log("startNlpQueryLocked isNlpEnabled=" + isNlpEnabled + " src:" + src +
                " mNlpRequestedSrc:" + mNlpRequestedSrc);
        if ((mNlpRequestedSrc & src) != 0) {
            stopNlpQueryLocked(src);
        }

        if (mNlpRequestedSrc == 0) {
            if (mLocationManager.getProvider(LocationManager.NETWORK_PROVIDER) != null) {
                LocationRequest request = LocationRequest.createFromDeprecatedProvider(
                        LocationManager.NETWORK_PROVIDER,
                        1000 /*minTime*/, 0 /*minDistance*/, false /*oneShot*/);
                request.setHideFromAppOps(true);
                mLocationManager.requestLocationUpdates(
                        request, mNetworkLocationListener, mHandler.getLooper());
            }
        }
        mNlpRequestedSrc |= src;
    }

    private void stopNlpQueryLocked(int src) {
        boolean isNlpEnabled = isNlpEnabled();
        if (DEBUG) log("stopNlpQueryLocked isNlpEnabled=" + isNlpEnabled + " src:" + src +
                " mNlpRequestedSrc:" + mNlpRequestedSrc);
        if ((mNlpRequestedSrc & src) != 0) {
            mNlpRequestedSrc &= ~(src);
            if ( mNlpRequestedSrc == 0) {
                mLocationManager.removeUpdates(mNetworkLocationListener);
            }
        }
    }

    private LocationListener mNetworkLocationListener = new LocationListener() {

        @Override
        public void onLocationChanged(Location location) {
            if (mNlpRequestedSrc == 0) {
                return;
            }
            if (mLastLocation == null || mLastLocation.getTime() < location.getTime()) {
                sendCommand(NLPS_MSG_NLP_UPDATED, 0);
            }
        }

        @Override
        public void onProviderDisabled(String provider) {
        }

        @Override
        public void onProviderEnabled(String provider) {
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
        }

    };

    public static void log(String msg) {
        Log.d("LbsNlpUtils", msg);
    }

    private synchronized void requestNlp(int src) {
        try {
            if (src == NLP_CMD_SRC_MNL) {
                mLastLocation = mLocationManager.
                        getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
                if (mLastLocation != null) {
                    long currentUtcTime = System.currentTimeMillis();
                    long nlpTime = mLastLocation.getTime();
                    long deltaMs = currentUtcTime - nlpTime;
                    log("requestNlp crTime: " + currentUtcTime + " nlpTime: "
                            + nlpTime + " delta: " + deltaMs);
                    if (deltaMs < LAST_LOCATION_EXPIRED_TIMEOUT && mNetworkConnected) {
                        Intent intent = new Intent(INJECT_NLP_LOC);
                        mContext.sendBroadcastAsUser(intent, UserHandle.ALL, INJECT_NLP_PERMISSION);
                    }
                }
            }
            startNlpQueryLocked(src);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private synchronized void releaseNlp(int src) {
        try {
            stopNlpQueryLocked(src);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private Mnld2NlpUtilsInterfaceReceiver mReceiver
            = new Mnld2NlpUtilsInterfaceReceiver() {
        @Override
        public void reqNlpLocation(int source) {
            if (DEBUG) log("reqNlpLocation() source = " + source);
            sendCommand(NLPS_MSG_NLP_NIJ_REQ, source);
        }

        @Override
        public void cancelNlpLocation(int source) {
            if (DEBUG) log("cancelNlpLocation() source = " + source);
            sendCommand(NLPS_MSG_NLP_NIJ_CANCEL, source);
        }
    };

    private void sendCommand(int cmd, int arg1) {
        Message msg = Message.obtain();
        msg.what = cmd;
        msg.arg1 = arg1;
        mHandler.sendMessage(msg);
    }

    private void sendCommandDelayed(int cmd, long delayMs) {
        Message msg = Message.obtain();
        msg.what = cmd;
        mHandler.sendMessageDelayed(msg, delayMs);
    }

    private class NlpsMsgHandler extends Handler {
        public NlpsMsgHandler(Looper looper) {
            super(looper, null, true);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case NLPS_MSG_NLP_NIJ_REQ:
                    if (DEBUG) log("handle NLPS_MSG_NLP_NIJ_REQ arg1: " + msg.arg1);
                    int reqSrc = msg.arg1;
                    if (reqSrc == NLP_CMD_SRC_UNKNOWN) {
                        reqSrc = NLP_CMD_SRC_MNL;
                    }
                    requestNlp(reqSrc);
                    break;
                case NLPS_MSG_NLP_NIJ_CANCEL:
                    if (DEBUG) log("handle NLPS_MSG_NLP_NIJ_CANCEL arg1: " + msg.arg1);
                    releaseNlp(msg.arg1);
                    break;
                case NLPS_MSG_NLP_UPDATED:
                    if (DEBUG) log("handle NLPS_MSG_NLP_UPDATED");
                    releaseNlp(NLP_CMD_SRC_MNL);
                    break;
                default:
                    log("Undefined message: " + msg.what);
            }
        }
    }
}
