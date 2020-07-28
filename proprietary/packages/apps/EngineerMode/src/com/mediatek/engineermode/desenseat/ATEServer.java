package com.mediatek.engineermode.desenseat;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.BitmapFactory;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.support.v4.app.NotificationCompat;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmApplication;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.clkqualityat.ATEServerClock;

public class ATEServer extends Service implements ATEServerSockets.OnResponseListener {
    public static final int INDEX_GSM_BAND = 0;
    public static final int INDEX_UMTS_BAND = 1;
    public static final int INDEX_LTE_FDD_BAND = 2;
    public static final int INDEX_LTE_TDD_BAND = 3;
    public static final int INDEX_CDMA_BAND = 4;
    public static final int INDEX_BAND_MAX = 5;
    public static String mCurrectRunCMD = "0000";
    public static ATEServerSockets mServerSockets = null;
    public final String TAG = "ATEServer";
    private final int TRANSMIT_WARRING = 0;
    private final int TRANSMIT_ERROR = 1;
    private final int TRANSMIT_SUCCEED = 2;
    private ATEServerClock mATEServerClock = null;
    private ATEServerGNSS mATEServerGnss = null;
    private LocationManager mLocationManager;

    private String itemSupport = "";
    private long[] mbandSupported = new long[TestItem.INDEX_BAND_MAX];
    private final Handler responseHandler = new Handler() {
        public void handleMessage(final Message msg) {
            AsyncResult asyncResult = (AsyncResult) msg.obj;
            switch (msg.what) {
                case DesenseAtActivity.SUPPORT_QUERY:
                    if (asyncResult.exception == null) {
                        System.arraycopy(Util.getSupportedBand((String[]) asyncResult.result), 0,
                                mbandSupported, 0, TestItem.INDEX_BAND_MAX - 1);

                        // query cdma supported modes
                        String[] cmd = new String[3];
                        cmd[0] = DesenseAtActivity.QUERY_CURRENT_COMMAND_CDMA;
                        cmd[1] = DesenseAtActivity.SAME_COMMAND_CDMA;
                        cmd[2] = "DESTRILD:C2K";
                        String[] cmd_s = ModemCategory.getCdmaCmdArr(cmd);
                        Elog.d(TAG, "query at cmd: " + cmd_s[0] +
                                ",cmd_s.length = " + cmd_s.length);
                        EmUtils.invokeOemRilRequestStringsEm(true, cmd_s,
                                responseHandler.obtainMessage(DesenseAtActivity
                                        .SUPPORT_QUERY_CDMA));
                    } else {
                        sendCommandResult(TRANSMIT_ERROR, "Get modem rf band support error");
                        Elog.w(TAG, "ATC_SUPPORT_QUERY: responseHandler get have exception!");
                    }
                    break;
                case DesenseAtActivity.SUPPORT_QUERY_CDMA:
                    if (asyncResult.exception == null) {
                        mbandSupported[TestItem.INDEX_CDMA_BAND] =
                                Util.getSupportedBandCdma((String[]) asyncResult.result);
                    } else {
                        Elog.w(TAG, "ATC_SUPPORT_CDMA_QUERY: responseHandler get have exception!");
                        mbandSupported[TestItem.INDEX_CDMA_BAND] = 0;
                    }
                    mATEServerClock.init(ATEServer.this, mServerSockets, mbandSupported);
                    mATEServerGnss.init(ATEServer.this, mServerSockets, mbandSupported);
                    BandSupportReport();
                    Elog.v(TAG, "query support bands finished.");
                    break;
                default:
                    break;
            }
        }
    };
    private LocationListener mLocListener = new LocationListener() {

        // @Override
        public void onLocationChanged(Location location) {
            // Elog.v(TAG, "Enter onLocationChanged function");
        }

        // @Override
        public void onProviderDisabled(String provider) {
            Elog.v(TAG, "Enter onProviderDisabled function");
        }

        // @Override
        public void onProviderEnabled(String provider) {
            Elog.v(TAG, "Enter onProviderEnabled function");
        }

        // @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            //    Elog.v(TAG, "Enter onStatusChanged function");
        }
    };


    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void sendCommandResult(int type, String text) {
        //  final String cmd = "PMTK2011";
        // cmd = cmd + "," + type + "," + text;
        new Thread(new Runnable() {
            @Override
            public void run() {
                if (mServerSockets != null) {
                    mServerSockets.sendCommand("PMTK2011," + mCurrectRunCMD + "," + type + "," +
                            text);
                } else
                    Elog.d(TAG, "mServerSockets is null");
            }
        }).start();
    }

    private void BandSupportReport() {
        itemSupport = "";
        itemSupport += "1," + mbandSupported[INDEX_GSM_BAND];
        if (ModemCategory.getModemType() == ModemCategory.MODEM_TD) {
            itemSupport += ",1," + mbandSupported[INDEX_UMTS_BAND];
        } else if (ModemCategory.getModemType() == ModemCategory.MODEM_FDD) {
            itemSupport += ",2," + mbandSupported[INDEX_UMTS_BAND];
        } else {
            itemSupport += ",0,0";
        }
        if (ModemCategory.isLteSupport()) {
            itemSupport += ",1," + (mbandSupported[INDEX_LTE_FDD_BAND] & ~(0x1 << 31)) + ",";
            itemSupport += mbandSupported[INDEX_LTE_TDD_BAND];
        } else {
            itemSupport += ",0,0";
        }
        if (ModemCategory.isCdma()) {
            itemSupport += ",1," + (mbandSupported[INDEX_CDMA_BAND]);
        } else {
            itemSupport += ",0,0";
        }
        Elog.d(TAG, "itemSupport = " + itemSupport);

        new Thread(new Runnable() {
            @Override
            public void run() {
                if (mServerSockets != null) {
                    mServerSockets.sendCommand("PMTK2025," + itemSupport);
                } else {
                    Elog.d(TAG, "mServerSockets is null");
                }
                sendCommandResult(TRANSMIT_SUCCEED, "Get modem type succeed");
            }
        }).start();
    }

    public void onResponse(String response) throws InterruptedException {
        Elog.d(TAG, "ATEServer -> Receive CMD: " + response);
        if (response.startsWith("$PMTK")) {
            mCurrectRunCMD = response.substring(5, 9);
            Elog.d(TAG, "mCurrectRunCMD = " + mCurrectRunCMD);
        }
        if (response.startsWith("$PMTK2000")) {
            sendCommandResult(TRANSMIT_SUCCEED, "Server connect succeed");
            return;
        } else if (response.startsWith("$PMTK2015")) {      //querySupportMode
            querySupportMode();
            return;
        }
        if (mATEServerGnss.mTestConditionGNSS == null) {
            sendCommandResult(TRANSMIT_ERROR, "Please query modem type again");
            return;
        } else if (response.startsWith("$PMTK2001")) {      //test item set
            mATEServerGnss.testItemSet(response);
        } else if (response.startsWith("$PMTK2002")) {      //gnss band set
            mATEServerGnss.gnssBandeSet(response);
        } else if (response.startsWith("$PMTK2003")) {      //test method set
            mATEServerGnss.testModesSet(response);
        } else if (response.startsWith("$PMTK2004")) {      //test API support start
            mATEServerGnss.startAPITest(response);
        } else if (response.startsWith("$PMTK2013")) {      //test API support stop
            mATEServerGnss.stopAPITest(response);
        } else if (response.startsWith("$PMTK2006")) {       //rf band config
            mATEServerGnss.rfBandConfig(response);
        } else if (response.startsWith("$PMTK2007")) {      //test start
            mATEServerGnss.startGNSSTest(response);
        } else if (response.startsWith("$PMTK2014")) {      //test stop
            mATEServerGnss.stopGNSSTest(response);
        } else if (response.startsWith("$PMTK2012")) {      //rfBandQuery
            mATEServerGnss.rfBandQuery(response);
        } else if (response.startsWith("$PMTK2020")) {      //testItemSet
            mATEServerClock.testItemSet(response);
        } else if (response.startsWith("$PMTK2021")) {      //Clock test item set
            mATEServerClock.testConfigSet(response);
        } else if (response.startsWith("$PMTK2022")) {      //Clock test item query
            mATEServerClock.startClockQualityTest(response);
        } else if (response.startsWith("$PMTK2028")) {      //Clock test item query
            mATEServerClock.stopClockQualityTest(response);
        } else {
            sendCommandResult(TRANSMIT_ERROR, "Server not support the cmd: " + response);
        }
        return;
    }

    private void gps_provider() {
        mLocationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
        try {
            mLocationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
            if (mLocationManager != null) {
                mLocationManager.requestLocationUpdates(
                        LocationManager.GPS_PROVIDER, 0, 0, mLocListener);

                if (mLocationManager
                        .isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                    Elog.w(TAG, "provider enabled");
                }
            } else {
                Elog.w(TAG, "new mLocationManager failed");
            }
        } catch (SecurityException e) {
            Toast.makeText(this, "security exception", Toast.LENGTH_LONG)
                    .show();
            Elog.w(TAG, "Exception: " + e.getMessage());
        } catch (IllegalArgumentException e) {
            Elog.w(TAG, "Exception: " + e.getMessage());
        }
    }

    private void querySupportMode() {
        final String[] queryCommon = {"AT+EPBSE=?", "+EPBSE:"};
        Elog.v(TAG, "sendAtCommand: " + queryCommon[0] + ", " + queryCommon[1]);
        try {
            EmUtils.invokeOemRilRequestStringsEm(false, queryCommon, responseHandler.obtainMessage
                    (DesenseAtActivity.SUPPORT_QUERY));
        } catch (Exception e) {
            Elog.e(TAG, "phone is null");
            sendCommandResult(TRANSMIT_ERROR, "Get modem rf band support error");
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Elog.d(TAG, "ATEServer--->onCreate");
        mATEServerGnss = new ATEServerGNSS();
        mATEServerClock = new ATEServerClock();
    }


    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Elog.d(TAG, "ATEServer----->onStartCommand");

        startForeground(1, getNotification("the socket server started...", 0));
        gps_provider();
        if (mServerSockets == null) {
            Elog.d(TAG, "ATEServer----->mServerSockets init");
            mServerSockets = new ATEServerSockets(this);
        }

        if (mServerSockets.mAliveServer == true) {
            Elog.d(TAG, "ATEServer----->service is runing");
        } else {
            Elog.d(TAG, "ATEServer----->service is stop,start it");
            mServerSockets.openServer();
        }

        return START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        stopForeground(true);
        Elog.d(TAG, "ATEServer----->onDestroy()");
        mServerSockets.closeServer();
        mServerSockets = null;
        mLocationManager.removeUpdates(mLocListener);
        mATEServerClock.unit();
        //unregisterReceiver(mReceiver);
    }

    private NotificationManager getNotificationManager() {
        return (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
    }

    private Notification getNotification(String title, int progress) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this,
                EmApplication.getSilentNotificationChannelID());
        builder.setSmallIcon(R.drawable.cross);
        builder.setLargeIcon(BitmapFactory.decodeResource(getResources(), R.drawable.cross));
        builder.setContentTitle(title);
        return builder.build();
    }
}
