package com.mediatek.lbs.em2.ui;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ToggleButton;
import android.widget.TextView;
import android.location.LocationManager;
import android.support.v4.content.LocalBroadcastManager;
import android.telecom.TelecomManager;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;

import com.mediatek.lbs.em2.utils.AgpsInterface;

import java.io.IOException;
import java.io.File;
import java.lang.Math;
import java.util.ArrayList;


public class LbsC2kDbg extends Activity {

    public static final String C2K_DEBUG_REPORT_FOLDER = "DbgScreenReport";
    public static final String EM_C2K_DEBUG_MSG = "com.mediatek.lbs.em2.action.debug_message";
    public static final int CALL_STATE_UNKNOWN = 0;
    public static final int CALL_STATE_IDLE = 1;
    public static final int CALL_STATE_ACTIVE = 2;

    private final static int MESSAGE_UPDATE_CALL_STATE       = 0;
    private final static int MESSAGE_TIMER_TICK              = 1;
    private final static int MESSAGE_PARSE_MESSAGE           = 2;

    private LocationManager mLocationManager = null;
    private int mAllSvNum;
    public String mDbgScreenRportPath;
    ArrayList<PilotInfo> mPilotList = new ArrayList<PilotInfo>();
    ArrayList<SvInfo> mSvList = new ArrayList<SvInfo>();
    private int mCallState = CALL_STATE_UNKNOWN;
    private boolean mBroadcastReceiverRegistered = false;
    private int mWaitFixTimeout = 30;
    private boolean mLaunchDialerWhenTimeout = false;
    private int mTimeCount = 0;
    protected AgpsInterface mAgpsInterface;
    private LocalBroadcastManager mLocalBroadcastManager;

    private   ToggleButton mToggleButtonC2kDbg;
    private   Button   mButtonCall;
    private   Button   mButtonIncreaseTimeout;
    private   Button   mButtonDecreaseTimeout;
    private   Button   mButtonCearReports;
    private   TextView mTextViewC2kLocationValue;
    private   TextView mTextViewC2kTimeStampValue;
    private   TextView mTextViewC2kHVelocityValue;
    private   TextView mTextViewC2kVVelocityValue;
    private   TextView mTextViewC2kHeadingValue;
    private   TextView mTextViewC2kHeightValue;
    private   TextView mTextViewC2kUncAngleValue;
    private   TextView mTextViewC2kUncAlongPerpValue;
    private   TextView mTextViewC2kUncVerValue;
    private   TextView mTextViewC2kSidNidBaseIdValue;
    private   TextView mTextViewC2kRefPnValue;
    private   TextView mTextViewC2kBaseLocation;

    private   TextView mTextViewC2kPpmNum;
    private   TextView mTextViewC2kPrmNum;
    private   TextView mTextViewC2kPpmResult;
    private   TextView mTextViewC2kPrmResult;
    private   TextView mTextViewReportPath;
    private   TextView mTextViewWaitFixTimeout;

    private StringList mPpmStringList = new StringList(12);
    private StringList mPrmStringList = new StringList(12);

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
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        try {
            initAgpsInterface();
        } catch (RuntimeException e) {
            log("onCreate, Failed to initAgpsInterface");
            return;
        }

        setContentView(R.layout.c2k_dbg);
        mLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);
        mLocalBroadcastManager = LocalBroadcastManager.getInstance(this);

        initUI();

        log("onCreate, set debug Screen function disabled");
        HandleCallStateUpdated(CALL_STATE_UNKNOWN);
        mButtonCall.setEnabled(false);
        setC2kDebugScreen(false);
    }

    protected void initAgpsInterface() {
        try {
            mAgpsInterface = new AgpsInterface();
        } catch (IOException e) {
            log("agpsInterface connection failure");
            e.printStackTrace();
        }
    }

    private void registerBroadcasts() {
        if (!mBroadcastReceiverRegistered) {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(EM_C2K_DEBUG_MSG);
            mLocalBroadcastManager.registerReceiver(mBroadcastReceiver, intentFilter);
        }
        mBroadcastReceiverRegistered = true;
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
                if (mCallState != CALL_STATE_ACTIVE) {
                    mCallState = CALL_STATE_ACTIVE;
                    sendMessage(MESSAGE_UPDATE_CALL_STATE, mCallState);
                }
            }  else if(state==TelephonyManager.CALL_STATE_IDLE){
                log("call_state_changed callback state Idle");
                mCallState = CALL_STATE_IDLE;
                sendMessage(MESSAGE_UPDATE_CALL_STATE, mCallState);
            }
        }
    };

    private void unregisterBroadcasts() {
        if (mBroadcastReceiverRegistered) {
            mLocalBroadcastManager.unregisterReceiver(mBroadcastReceiver);
        }
        mBroadcastReceiverRegistered = false;
        TelephonyManager telephonyManager =
                      (TelephonyManager)getSystemService(Context.TELEPHONY_SERVICE);
        telephonyManager.listen(callStateListener,PhoneStateListener.LISTEN_NONE);
    }

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(EM_C2K_DEBUG_MSG)) {
                String dbgMsg = intent.getExtras().getString("vzw_dbg");
                log("vzw_dbg: " + dbgMsg);
                sendMessage(MESSAGE_PARSE_MESSAGE, 0, 0, dbgMsg);
            }
        }
    };

    @Override
    public void onDestroy() {
        log("onDestroy");
        super.onDestroy();
        setC2kDebugScreen(false);
        unregisterBroadcasts();
        mHandler.removeMessages(MESSAGE_TIMER_TICK);
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    protected void initUI() {
        mToggleButtonC2kDbg= (ToggleButton) findViewById(R.id.toggleButton_C2kDbg);
        mButtonCall = (Button) findViewById(R.id.button_Call);
        mButtonIncreaseTimeout= (Button) findViewById(R.id.button_IncreaseTimeout);
        mButtonDecreaseTimeout= (Button) findViewById(R.id.button_DecreaseTimeout);
        mButtonCearReports =  (Button) findViewById(R.id.button_ClearReports);

        mTextViewC2kLocationValue = (TextView) findViewById(R.id.TextC2k_LocationValue);
        mTextViewC2kTimeStampValue = (TextView) findViewById(R.id.TextC2k_TimeStampValue);
        mTextViewC2kHVelocityValue = (TextView) findViewById(R.id.TextC2k_HVelocityValue);
        mTextViewC2kVVelocityValue = (TextView) findViewById(R.id.TextC2k_VVelocityValue);
        mTextViewC2kHeadingValue = (TextView) findViewById(R.id.TextC2k_HeadingValue);
        mTextViewC2kHeightValue = (TextView) findViewById(R.id.TextC2k_HeightValue);
        mTextViewC2kUncAngleValue = (TextView) findViewById(R.id.TextC2k_UncAngleValue);
        mTextViewC2kUncAlongPerpValue = (TextView) findViewById(R.id.TextC2k_UncAlongPerpValue);
        mTextViewC2kUncVerValue = (TextView) findViewById(R.id.TextC2k_UncVerValue);
        mTextViewC2kSidNidBaseIdValue = (TextView) findViewById(R.id.TextC2k_SidNidBaseIdValue);
        mTextViewC2kRefPnValue = (TextView) findViewById(R.id.TextC2k_RefPnValue);
        mTextViewC2kBaseLocation = (TextView) findViewById(R.id.TextC2k_BaseLocationValue);

        mTextViewC2kPpmNum = (TextView) findViewById(R.id.TextC2k_PpmNumValue);
        mTextViewC2kPrmNum = (TextView) findViewById(R.id.TextC2k_PrmNumValue);
        mTextViewC2kPpmResult = (TextView) findViewById(R.id.TextView_PpmResult);
        mTextViewC2kPrmResult = (TextView) findViewById(R.id.TextView_PrmResult);
        mTextViewReportPath = (TextView) findViewById(R.id.TextView_ReportPath);
        mTextViewWaitFixTimeout    = (TextView) findViewById(R.id.TextView_WaitFixTimeout);

        mToggleButtonC2kDbg.setChecked(false);
        mToggleButtonC2kDbg.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mToggleButtonC2kDbg.isChecked()) {
                    log("C2kDebug button Clicked ON");
                    mButtonCall.setEnabled(true);
                    registerBroadcasts();
                    mButtonCearReports.setEnabled(false);
                } else {
                    log("C2kDebug button Clicked OFF");
                    mHandler.removeMessages(MESSAGE_TIMER_TICK);
                    HandleCallStateUpdated(CALL_STATE_UNKNOWN);
                    mButtonCall.setEnabled(false);
                    unregisterBroadcasts();
                    clearAllValues();
                    mButtonCearReports.setEnabled(true);
                }
                setC2kDebugScreen(mToggleButtonC2kDbg.isChecked());
            }
        });

        mButtonCall.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mCallState == CALL_STATE_ACTIVE) {
                    final TelecomManager tm = (TelecomManager) getSystemService(Context.TELECOM_SERVICE);
                    tm.showInCallScreen(false);
                } else {
                    Intent intent = new Intent(Intent.ACTION_DIAL);
                    startActivity(intent);
                }
            }
        });
        mButtonIncreaseTimeout.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mWaitFixTimeout++;
                mTextViewWaitFixTimeout.setText("Waiting Fix Timeout (s) : "+mWaitFixTimeout);
            }
        });
        mButtonDecreaseTimeout.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mWaitFixTimeout--;
                mWaitFixTimeout = mWaitFixTimeout < 0 ? 0 : mWaitFixTimeout;
                mTextViewWaitFixTimeout.setText("Waiting Fix Timeout (s) : "+mWaitFixTimeout);
            }
        });
        mButtonCearReports.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                showConfirmDialog();
            }
        });

        mDbgScreenRportPath = "/sdcard/" + C2K_DEBUG_REPORT_FOLDER + "/";
        log("Report file path: " + mDbgScreenRportPath);
        File dirFile = new File(mDbgScreenRportPath);
        if(!dirFile.exists()){
            dirFile.mkdirs();
        }
        mTextViewReportPath.setText(mDbgScreenRportPath);
    }

    void deleteReportFiles() {
        log("deleteReportFiles in path: " + mDbgScreenRportPath);
        File dirFile = new File(mDbgScreenRportPath);
        if (dirFile.isDirectory()) {
            for (File child : dirFile.listFiles()) {
                child.delete();
            }
        }
    }

    private void showConfirmDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Are you sure to detele all Reports?");
        builder.setCancelable(false);

        builder.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                deleteReportFiles();
            }
        });

        builder.setNegativeButton("No", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
            }
        });

        AlertDialog alert = builder.create();
        alert.show();
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
            case MESSAGE_TIMER_TICK:
                HandleTimerCallInfo(mTimeCount);
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

    private void HandleTimerCallInfo(int timeCount) {
        mTimeCount++;
        int iMin = timeCount / 60;
        int iSec = timeCount % 60;
        String sSec = iSec < 10 ? "0"+iSec : ""+iSec;
        String sMin = iMin < 10 ? "0"+iMin : ""+iMin;
        mButtonCall.setText("Call Active : " + sMin +":" + sSec);

        if (timeCount >= mWaitFixTimeout && mLaunchDialerWhenTimeout) { // Launch dialer app
            log("Wait fix timeout: " + mWaitFixTimeout);
            final TelecomManager tm = (TelecomManager) getSystemService(Context.TELECOM_SERVICE);
            tm.showInCallScreen(false);
            mLaunchDialerWhenTimeout = false;
        }
        sendDelayedMessage(MESSAGE_TIMER_TICK, 1 * 1000);
    }
    private void setC2kDebugScreen(boolean enabled) {
        log("setC2kDebugScreen enabled: " + enabled);
        if (mAgpsInterface == null) {
            log("setC2kDebugScreen return because of null mAgpsInterface");
            return;
        }
        mAgpsInterface.setVzwDebugScreenEnable(enabled);

        if (enabled) {
            // start c2k debug screen report service
            startService(new Intent().setClass(this, DbgScreenReporter.class));
        } else {
            // stop c2k debug screen report service
            stopService(new Intent().setClass(this, DbgScreenReporter.class));
        }
    }

    void parseMsg(String record) {
        if (record.contains("vzw_aa")) {
            parseAa(record);
        } else if (record.contains("vzw_ppm")) {
            parsePpm(record);
        } else if (record.contains("vzw_pos")) {
            parsePos(record);
        } else if (record.contains("vzw_prm")) {
            parsePrm(record);
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
        mTextViewC2kLocationValue.setText("");
        mTextViewC2kTimeStampValue.setText("");
        mTextViewC2kHVelocityValue.setText("");
        mTextViewC2kVVelocityValue.setText("");
        mTextViewC2kHeadingValue.setText("");
        mTextViewC2kHeightValue.setText("");   // current is 0
        mTextViewC2kUncAngleValue.setText("");
        mTextViewC2kUncAlongPerpValue.setText("");
        mTextViewC2kUncVerValue.setText("");
        mTextViewC2kSidNidBaseIdValue.setText("");
        mTextViewC2kRefPnValue.setText("");
        mTextViewC2kBaseLocation.setText("");
        mTextViewC2kPpmNum.setText("0");
        mTextViewC2kPrmNum.setText("0");
        mTextViewC2kPpmResult.setText("");
        mTextViewC2kPrmResult.setText("");
    }

    void parsePpm(String record) {

        // $GPGSV,vzw_ppm,6,0,0,917504,8224,0,1,128,1583872,24594,0,2,50,4189952,12288,0,
        // "$GPGSV,vzw_ppm,6,3,8,3276800,10284,0,4,160,11264,0,0,5,0,1759313921,5468,164,";
        // NUM_PILOTS_P,index,PN,PILOT_PN_PHASE,PILOT_STRENGTH,RMS_ERR_PHASE,...,index,PN,...
        // 6, (0,0,917504,8224,0),(1,128,1583872,24594,0),(2,50,4189952,12288,0),
        // 6, (3,8,3276800,10284,0),(4,160,11264,0,0),(5,0,1759313921,5468,164),

        String[] values = LbsUtility.split(record);
        //log("values 0: " + values[0]); // $GPGGA
        //log("values 1: " + values[1]); // vzw_pos
        //log("values 2: " + values[2]); // NUM_PILOTS_P
        if (values == null) {
            log("parsePpm Error, split record return null");
            return;
        }
        int i = 2;
        int numPilots = (values.length >= 3) ? LbsUtility.parseInt(values[i++]) : 0;
        while (i < values.length) {
            if (values.length - i < 5) {
                log("parsePpm Error, left item size is less than 5, msg: " + record);
                break;
            }
            String index = values[i++];
            String pn = values[i++];
            String pilot_pn_phase = values[i++];
            String pilot_strength = values[i++];
            String rms_err_phase = values[i++];

            PilotInfo pilotInfo = new PilotInfo();
            pilotInfo.mIndex = LbsUtility.parseInt(index);
            pilotInfo.mPn = LbsUtility.parseInt(pn);
            pilotInfo.mPilot_pn_phase = convertPilotPnPhase(pilot_pn_phase);
            pilotInfo.mPilot_strength = convertPilotStrength(pilot_strength);
            pilotInfo.mRms_err_phase = LbsUtility.parseInt(rms_err_phase);
            mPilotList.add(pilotInfo);
        }
        mTextViewC2kPpmNum.setText(""+numPilots);

        mPpmStringList.clear();
        mPpmStringList.add("        PN  PN_Phase  Ec/Io");
        for (PilotInfo p : mPilotList) {
            String sIndex = (p.mIndex > 9 ? ""+p.mIndex : "0"+ p.mIndex);
            String sPn = (p.mPn > 9 ? ""+p.mPn : "0"+ p.mPn);

            mPpmStringList.add("\n["+sIndex+"]  " + sPn + "  " +
                    String.format("%.04f", p.mPilot_pn_phase) + "  " +
                    String.format("%.01f", p.mPilot_strength));
        }
        mTextViewC2kPpmResult.setText(mPpmStringList.get());
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
            log("parsePrm Error, split record return null");
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
            svInfo.mPs_doppler = convertPsDoppler(ps_doppler);
            svInfo.mSv_code_ph = convertSvCodePh(sv_code_ph_wh, sv_code_ph_fr);
            mSvList.add(svInfo);
        }
        mTextViewC2kPrmNum.setText(""+numSatellites);

        mPrmStringList.clear();
        mPrmStringList.add("    PRN  C/NO  Dopp  Code_Ph");
        for (SvInfo s : mSvList) {
            String sIndex = (s.mIndex > 9 ? ""+s.mIndex : "0"+ s.mIndex);
            String sPrn = (s.mSv_prn_num > 9 ? ""+s.mSv_prn_num : "0"+ s.mSv_prn_num);

            mPrmStringList.add("\n["+sIndex+"]  " + sPrn + "  " + s.mSv_cno + ".0  " +
                    String.format("%.02f", s.mPs_doppler) + "  " +
                    String.format("%.04f", s.mSv_code_ph));
        }
        mTextViewC2kPrmResult.setText(mPrmStringList.get());
    }

    void parsePos(String record) {

        // $GPGGA,vzw_pos,7509295,-13803921,2,6,6,8,0,506,0,0,28,1,17,0,0,0,0,0,2001,10,1,1,4,42,2
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
        final String uncertaintyAngle = values[4];
        final String uncertaintyAlong = values[5];
        final String uncertaintyPerp = values[6];
        final String uncertaintyVer = values[7];
        final String altitude = values[8];
        final String heading = values[9];
        final String velocity_hor = values[10];
        final String velocity_ver = values[11];
        final String sid = values[12];
        final String nid = values[13];
        final String baseId = values[14];
        final String baseLatitude = values[15];
        final String baseLongitude = values[16];
        final String refPn = values[17];
        final String optMask = values[18];
        final String extOptMask = values[19];
        final String year = values[20];
        final String month = values[21];
        final String day = values[22];
        final String hour = values[23];
        final String minute = values[24];
        final String second = values[25];
        final String testCount = values[26];
        String sTestCount = getTestCount(testCount);
        String sTimestamp = getTimestamp(year, month, day, hour, minute, second);

        mTextViewC2kLocationValue.setText(convertLatitude(latitude)+" / "+convertLongitude(longitude)+" "+ sTestCount);
        mTextViewC2kTimeStampValue.setText(sTimestamp);
        mTextViewC2kHVelocityValue.setText(convertHVelocity(velocity_hor) + " m/s");
        mTextViewC2kVVelocityValue.setText(convertVVelocity(velocity_ver) + " m/s");
        mTextViewC2kHeadingValue.setText(convertHeading(heading) + " d");
        mTextViewC2kHeightValue.setText(altitude + " m");   // current is 0
        mTextViewC2kUncAngleValue.setText(convertUncAngle(uncertaintyAngle) + " d");
        mTextViewC2kUncAlongPerpValue.setText(convertFromTable(uncertaintyAlong)+" m /" + convertFromTable(uncertaintyPerp) + " m");
        mTextViewC2kUncVerValue.setText(convertFromTable(uncertaintyVer) + " m");
        mTextViewC2kSidNidBaseIdValue.setText(sid+" / "+nid+" / "+baseId);
        mTextViewC2kRefPnValue.setText(refPn);
        mTextViewC2kBaseLocation.setText(convertLatitude(baseLatitude)+" / "+convertLongitude(baseLongitude));
    }

    private float convertPilotStrength(String record) {
        int iPilot_strength = LbsUtility.parseInt(record);
        if (iPilot_strength == 0) {
            return 0;
        }
        float fPilot_strength = (float)iPilot_strength /-2;  // * (-0.5)
        return fPilot_strength;
    }

    private float convertPilotPnPhase(String record) {
        int iPilot_pn_phase = LbsUtility.parseInt(record);
        float fPilot_pn_phase = (float)iPilot_pn_phase/16;
        return fPilot_pn_phase;
    }

    private float convertSvCodePh(String str_sv_code_ph_wh, String str_sv_code_ph_fr) {
        int sv_code_ph_wh = LbsUtility.parseInt(str_sv_code_ph_wh);
        int sv_code_ph_fr = LbsUtility.parseInt(str_sv_code_ph_fr);
        float sv_code_ph = (float)sv_code_ph_wh + (float)sv_code_ph_fr/(1<<10);
        return sv_code_ph;
    }

    private float convertPsDoppler(String record) {
        int iPsDoppler = LbsUtility.parseInt(record);
        float fPsDoppler = (float)iPsDoppler*0.2f;
        return fPsDoppler;
    }
    private String convertUncAngle(String record) {
        int iUncAngle = LbsUtility.parseInt(record);
        float fUncAngle = (float)iUncAngle*5.625f;
        return String.format("%.06f", fUncAngle);
    }

    private String convertHeading(String record) {
        int iHeading = LbsUtility.parseInt(record);
        float fHeading = (float)iHeading*360/(1<<10);
        return String.format("%.06f", fHeading);
    }

    private String convertVVelocity(String record) {
        int iVV = LbsUtility.parseInt(record);
        float fVV = (float)iVV*0.5f;
        return String.format("%.06f", fVV);
    }

    private String convertHVelocity(String record) {
        int iHV = LbsUtility.parseInt(record);
        float fHV = (float)iHV*0.25f;
        return String.format("%.06f", fHV);
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
        int iMonth = LbsUtility.parseInt(month);
        String sMonth;
        switch(iMonth) {
            case 1:
                sMonth = "Jan"; break;
            case 2:
                sMonth = "Feb"; break;
            case 3:
                sMonth = "Mar"; break;
            case 4:
                sMonth = "Apr"; break;
            case 5:
                sMonth = "May"; break;
            case 6:
                sMonth = "Jun"; break;
            case 7:
                sMonth = "Jul"; break;
            case 8:
                sMonth = "Aug"; break;
            case 9:
                sMonth = "Sep"; break;
            case 10:
                sMonth = "Oct"; break;
            case 11:
                sMonth = "Nov"; break;
            case 12:
                sMonth = "Dec"; break;
            default:
                sMonth = ""; break;
        }
        return sMonth+" "+day+", "+year+" "+
            (hour.length() > 1 ? hour : "0"+hour) + ":" +
            (minute.length() > 1 ? minute : "0"+minute) + ":" +
            (second.length() > 1 ? second : "0"+second);
    }

    private String getTestCount(String record) {
        String result;
        int iTestCount = LbsUtility.parseInt(record);
        switch(iTestCount) {
            case 1:
                result = "(1st)"; break;
            case 2:
                result = "(2nd)"; break;
            case 3:
                result = "(3rd)"; break;
            default:
                result = "(" + record + "th)"; break;
        }
        return result;
    }


    protected static void log(Object msg) {
        Log.d("LocationEM_C2kDbg", "" + msg);
    }

    private String convertFromTable(String record) {
        String result;
        int value = LbsUtility.parseInt(record);

        switch(value) {
            case 0://00000:
                result = "0.5"; break;
            case 1: //00001:
                result = "0.75"; break;
            case 2: //00010:
                result = "1"; break;
            case 3: //00011:
                result = "1.5"; break;
            case 4: //00100:
                result = "2"; break;
            case 5: //00101:
                result = "3"; break;
            case 6: //00110:
                result = "4"; break;
            case 7: //00111:
                result = "6"; break;
            case 8: //01000:
                result = "8"; break;
            case 9: //01001:
                result = "12"; break;
            case 10: //01010:
                result = "16"; break;
            case 11: //01011:
                result = "24"; break;
            case 12: //01100:
                result = "32"; break;
            case 13: //01101:
                result = "48"; break;
            case 14: //01110:
                result = "64"; break;
            case 15: //01111:
                result = "96"; break;
            case 16: //10000:
                result = "128"; break;
            case 17: //10001:
                result = "192"; break;
            case 18: //10010:
                result = "256"; break;
            case 19: //10011:
                result = "384"; break;
            case 20: //10100:
                result = "512"; break;
            case 21: //10101:
                result = "768"; break;
            case 22: //10110:
                result = "1024"; break;
            case 23: //10111:
                result = "1536"; break;
            case 24: //11000:
                result = "2048"; break;
            case 25: //11001:
                result = "3072"; break;
            case 26: //11010:
                result = "4096"; break;
            case 27: //11011:
                result = "6144"; break;
            case 28: //11100:
                result = "8192"; break;
            case 29: //11101:
                result = "12288"; break;
            case 30: //11110:  // larger than 12288
            default:
                result = "12288"; break;
        }
        return result;
    }

    void HandleCallStateUpdated(int callState) {
        switch (callState) {
            case CALL_STATE_UNKNOWN:
                mButtonCall.setText("Call State");
                break;
            case CALL_STATE_IDLE:
                mButtonCall.setText("Call Idle");
                mHandler.removeMessages(MESSAGE_TIMER_TICK);
                break;
            case CALL_STATE_ACTIVE:
                mButtonCall.setText("Call Active : 00:00");
                mLaunchDialerWhenTimeout = true;
                mTimeCount = 0;
                sendDelayedMessage(MESSAGE_TIMER_TICK, 1 * 1000);
                clearAllValues();
                break;
        }
    }
}
