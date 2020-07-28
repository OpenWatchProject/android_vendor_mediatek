package com.mediatek.lbs.em2.ui;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Handler;
import android.os.Message;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import java.util.ArrayList;
import java.util.Date;

import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;

public class DbgScreenReporter extends Service{
    private static final int SESSION_STATE_GET_AA = 1;
    private static final int SESSION_STATE_GET_PRM = 2;
    private static final int SESSION_STATE_GET_POS = 3;

    private final static int MESSAGE_UPDATE_CALL_STATE       = 0;
    private final static int MESSAGE_CREATE_REPORT           = 1;
    private final static int MESSAGE_PARSE_MESSAGE           = 2;

    private int mCallState = LbsC2kDbg.CALL_STATE_IDLE;
    private LocationManager mLocationManager = null;
    private int mAllSvNum;
    private ArrayList<PilotInfo> mPilotList = new ArrayList<PilotInfo>();
    private ArrayList<SvInfo> mSvList = new ArrayList<SvInfo>();
    private String mReportFileName;
    private String mCallDialTime;
    private String mCallEndTime;
    private String mFixInfo;
    private boolean mRegistered = false;
    private LocalBroadcastManager mLocalBroadcastManager;

    public class PilotInfo {
        int mIndex;
        int mPn;
        float mPilot_pn_phase;
        float mPilot_strength;
        float mRms_err_phase;
    }

    public class SvInfo {
        int mIndex;
        int mSv_prn_num;
        int mSv_cno;
        float mPs_doppler;
        float mSv_code_ph;
    }


    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void onCreate() {
        log("onCreate");
        // TODO Auto-generated method stub
        super.onCreate();
        mLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);
        mLocalBroadcastManager = LocalBroadcastManager.getInstance(this);

        setServiceForeground();
        sendMessage(MESSAGE_CREATE_REPORT, 0);
        mRegistered = true;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
     // TODO Auto-generated method stub
     log("onStartCommand");
     return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onStart(Intent intent, int startId) {
        log("onStart");
    }
    public void onDestroy(){
        log("onDestroy");
        super.onDestroy();
        if (mRegistered) {
            stopForeground(true);
            mLocalBroadcastManager.unregisterReceiver(mBroadcastReceiver);
            TelephonyManager telephonyManager =
                    (TelephonyManager)getSystemService(Context.TELEPHONY_SERVICE);
            telephonyManager.listen(callStateListener,PhoneStateListener.LISTEN_NONE);
        }
    }

    private void createReport() {
        mReportFileName = "E911TEST_" + LbsUtility.getTimeString2(new Date().getTime()) + ".csv";
        log("Create report file name: " + mReportFileName);
        String str = "Call Dial Time, Call End Time, Fix Time, Fix Type, Latitude, Longitude, #SV, #MS Sat, SV info, Avg CNo, call_count\n";
        LbsUtility.write2File(getApplicationContext(),
                LbsC2kDbg.C2K_DEBUG_REPORT_FOLDER, null, mReportFileName, str, true);

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(LbsC2kDbg.EM_C2K_DEBUG_MSG);
        mLocalBroadcastManager.registerReceiver(mBroadcastReceiver, intentFilter);

        TelephonyManager telephonyManager =
                      (TelephonyManager)getSystemService(Context.TELEPHONY_SERVICE);
        telephonyManager.listen(callStateListener,PhoneStateListener.LISTEN_CALL_STATE);
    }

    PhoneStateListener callStateListener = new PhoneStateListener() {
        public void onCallStateChanged(int state, String incomingNumber)
        {
            if(state==TelephonyManager.CALL_STATE_RINGING){
                log("call_state_changed callback state Ringing");
            } else if(state==TelephonyManager.CALL_STATE_OFFHOOK){
                log("call_state_changed callback state Offhook");
                if (mCallState != LbsC2kDbg.CALL_STATE_ACTIVE) {
                    mCallState = LbsC2kDbg.CALL_STATE_ACTIVE;
                    sendMessage(MESSAGE_UPDATE_CALL_STATE, mCallState);
                }
            }  else if(state==TelephonyManager.CALL_STATE_IDLE){
                log("call_state_changed callback state Idle");
                if (mCallState != LbsC2kDbg.CALL_STATE_IDLE) {
                    mCallState = LbsC2kDbg.CALL_STATE_IDLE;
                    sendMessage(MESSAGE_UPDATE_CALL_STATE, mCallState);
                }
            }
        }
    };

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(LbsC2kDbg.EM_C2K_DEBUG_MSG)) {
                String dbgMsg = intent.getExtras().getString("vzw_dbg");
                log("vzw_dbg: " + dbgMsg);
                sendMessage(MESSAGE_PARSE_MESSAGE, 0, 0, dbgMsg);
            }
        }
    };

    private void launchLocationEM(Context context) {
        context.startActivity(getLaunchIntent(context));
    }

    private Intent getLaunchIntent(Context context) {
        Intent intent = new Intent(context, MyTabActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        return intent;
    }

    void parseMsg(String record) {
        if (record.contains("vzw_aa")) {
            parseAa(record);
        } else if (record.contains("vzw_pos")) {
            parsePos(record);
        } else if (record.contains("vzw_prm")) {
            parsePrm(record);
        } else if (record.contains("vzw_ppm")) {
            //parsePpm(record);
        }
    }

    void parseAa(String record) {
        //$GPGSV,vzw_aa,9,19,18,5,12,16,21,11,22,9
        // Numbers of GNSS SVs,SV_PRN_NUM,
        String[] values = LbsUtility.split(record);
        if (values != null && values.length >= 3) {
            mAllSvNum = LbsUtility.parseInt(values[2]);
        }
    }

    private void clearAllValues() {
        mPilotList.clear();
        mSvList.clear();
        mFixInfo = null;
        mCallDialTime = null;
        mCallEndTime = null;
    }

    void parsePrm(String record) {
        // $GPGSV,vzw_prm,9,0,16,43,-11431,400,400,1,21,43,4501,1023,1023,2,11,42,16054,685,685,
        // $GPGSV,vzw_prm,9,3,18,44,10766,822,822,4,5,44,-8257,934,934,5,12,43,11389,510,510,
        // $GPGSV,vzw_prm,9,6,22,42,-6409,828,828,7,9,43,-17031,696,696,8,19,44,-2249,566,566,
        // NUM_PS_RANGES_P,index,SV_PRN_NUM,SV_CNO,PS_DOPPLER,SV_CODE_PH_WH,SV_CODE_PH_FR,...,index, SV_PRN_NUM,...
        // 9,(0,16,43,-11431,400,400),(1,21,43,4501,1023,1023),(2,11,42,16054,685,685),
        // 9,(3,18,44,10766,822,822),(4,5,44,-8257,934,934),(5,12,43,11389,510,510),
        // 9,(6,22,42,-6409,828,828),(7,9,43,-17031,696,696),(8,19,44,-2249,566,566),

        String[] values = LbsUtility.split(record);
        //log("values 0: " + values[0]); // $GPGGA
        //log("values 1: " + values[1]); // vzw_pos
        //log("values 2: " + values[2]); // NUM_PS_RANGES_P
        if (values == null) {
            log("parsePrm Error, pares record return null");
            return;
        }
        int i = 2;
        int numSatellites = (values.length >= 3) ? LbsUtility.parseInt(values[i++]) : 0;
        while (i < values.length) {
            if (values.length - i < 6) {
                log("parsePrm Error, left item size is less than 6, msg: " + record);
                break;
            }
            String index = values[i++];
            String sv_prn_num = values[i++];
            String sv_cno = values[i++];
            String ps_doppler = values[i++];
            String sv_code_ph_wh = values[i++];
            String sv_code_ph_fr = values[i++];

            SvInfo svInfo = new SvInfo();
            svInfo.mIndex = LbsUtility.parseInt(index);
            svInfo.mSv_prn_num = LbsUtility.parseInt(sv_prn_num);
            svInfo.mSv_cno = LbsUtility.parseInt(sv_cno);
            mSvList.add(svInfo);
        }
    }

    void parsePos(String record) {

        //$GPGGA,vzw_pos,7509296,-13803920,2,7,5,8,0,792,0,0,28,1,17,0,0,0,0,0,2001,10,1,1,6,40,4
        //$GPGGA,vzw_pos,7509295,-13803921,2,  // item 0-4
        //6,6,8,0,506,  //item 5-9
        //0,0,28,1,17,  //item 10-14
        //0,0,0,0,0,    //item 15-19
        //2001,10,1,1,4, //item 20-24
        //42,2           // item 25-26

        String[] values = LbsUtility.split(record);
        if (values == null || values.length < 27) {
            log("parsePos Error, item is null or size less than 27"); // $GPGGA
            return;
        }
        //log("values 0: " + values[0]); // $GPGGA
        //log("values 1: " + values[1]); // vzw_pos
        final String latitude = values[2]; // Latitude (eg. 7509295,)
        final String longitude = values[3]; // Longitude (eg. -13803921)
        final String year = values[20];
        final String month = values[21];
        final String day = values[22];
        final String hour = values[23];
        final String minute = values[24];
        final String second = values[25];
        final String testCount = values[26];
        String sTimestamp = getTimestamp(year, month, day, hour, minute, second);
        mFixInfo = generateFixReport(convertLatitude(latitude), convertLongitude(longitude),
                sTimestamp, testCount);
    }

    private String convertLatitude(String record) {
        int iLat = LbsUtility.parseInt(record);
        float fLat = (float)iLat*180/(1<<25);
        return String.format("%.06f", fLat);
    }

    private String convertLongitude(String record) {
        int iLong = LbsUtility.parseInt(record);
        float fLong = (float)iLong*360/(1<<26);
        return String.format("%.06f", fLong);
    }


    private String getTimestamp(String year, String month, String day,
            String hour, String minute, String second) {
        String result;
        return month+"/"+day+"/"+year+" "+
            (hour.length() > 1 ? hour : "0"+hour) + ":" +
            (minute.length() > 1 ? minute : "0"+minute) + ":" +
            (second.length() > 1 ? second : "0"+second);
    }

    private String generateFixReport(String latitude, String longitude,
        String timestamp, String testCount) {
        String str = "";
        int totalCno = 0;
        //str += ","; //min
        str += timestamp + ", ";
        str += ", "; // fix type
        str += latitude + ", ";
        str += longitude + ", ";
        str += mAllSvNum+ ", ";
        str += mSvList.size() + ", ";
        str += "( ";

        for (SvInfo s : mSvList) {
            String sPrn = (s.mSv_prn_num > 9 ? ""+s.mSv_prn_num : "0"+ s.mSv_prn_num);
            str += "SV" + sPrn + ":" + s.mSv_cno + " ";
            totalCno += s.mSv_cno;
        }
        str += "), ";
        str += (mSvList.size() > 0 ? totalCno/mSvList.size(): "")+ ", ";
        str += testCount+ "\n";
        return str;
        //LbsUtility.write2File(getApplicationContext(),
          //      LbsC2kDbg.C2K_DEBUG_REPORT_FOLDER, null, mReportFileName, str, true);
    }

    private void setServiceForeground() {
        log("setServiceForeground");
        String channelId = "mtk_app_channel_id";
        CharSequence channelName = "MTK APP";

        NotificationManager notificationManager =
                (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        int importance = NotificationManager.IMPORTANCE_LOW;
        NotificationChannel notificationChannel =
                new NotificationChannel(channelId, channelName, importance);
        notificationChannel.enableLights(false);
        notificationChannel.setLightColor(Color.RED);
        notificationChannel.enableVibration(false);
        notificationManager.createNotificationChannel(notificationChannel);

        PendingIntent p_intent = PendingIntent.getActivity(this, 0,
                getLaunchIntent(this), 0);
        Notification notification = new Notification.Builder(this)
         .setAutoCancel(true)
         .setContentTitle("Debug Screen Reporter")
         .setContentText("Service is Running! Tap to launch Activity.")
         .setContentIntent(p_intent)
         .setSmallIcon(R.drawable.ic_launcher2)
         .setWhen(System.currentTimeMillis())
         .setChannel(channelId)
         .build();
        log(String.format("notification = %s", notification));
        startForeground(0x4567, notification);   // notification ID
    }

    private void sendMessage(int what, int arg1) {
        sendMessage(what, arg1, 0, null);
    }

    private void sendDelayedMessage(int what, long delayMillis) {
        mHandler.sendEmptyMessageDelayed(what, delayMillis);
    }

    private void sendMessage(int what, int arg1, int arg2, Object obj) {
        mHandler.obtainMessage(what, arg1, arg2, obj).sendToTarget();
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch(msg.what) {
            case MESSAGE_UPDATE_CALL_STATE:
                HandleCallStateUpdated(msg.arg1);
                break;
            case MESSAGE_CREATE_REPORT:
                createReport();
                break;
            case MESSAGE_PARSE_MESSAGE:
                parseMsg((String)msg.obj);
                break;
            default:
                log("WARNING: unknown handle event recv!!");
                break;
            }
        }
    };

    void HandleCallStateUpdated(int callState) {
        //log("HandleCallStateUpdated callState: " + callState);
        switch (callState) {
            case LbsC2kDbg.CALL_STATE_UNKNOWN:
                break;
            case LbsC2kDbg.CALL_STATE_IDLE:
                mCallEndTime = LbsUtility.getTimeString3(new Date().getTime());
                //log("HandleCallStateUpdated Idle, mCallDialTime= " + mCallDialTime +" mCallEndTime = " + mCallEndTime);
                if (mCallDialTime != null ) {
                    LbsUtility.write2File(getApplicationContext(),
                            LbsC2kDbg.C2K_DEBUG_REPORT_FOLDER, null, mReportFileName,
                        mCallDialTime + ", " + mCallEndTime + ", " +
                        (mFixInfo == null? "\n" : mFixInfo) , true);
                }
                launchLocationEM(getApplicationContext());
                break;
            case LbsC2kDbg.CALL_STATE_ACTIVE:
                clearAllValues();
                mCallDialTime = LbsUtility.getTimeString3(new Date().getTime());
                launchLocationEM(getApplicationContext());
                break;
        }
    }


    protected static void log(Object msg) {
        Log.d("LocationEM_DbgScreenReporter", "" + msg);
    }
}
