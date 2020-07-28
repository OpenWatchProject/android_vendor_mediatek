package com.mediatek.engineermode.rfdesense;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.graphics.BitmapFactory;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.support.v4.app.NotificationCompat;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmApplication;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;


public class RfDesenseService extends Service {
    private final String TAG = "RfDesense/TxTestService";

    private final int STATE_NONE = 0;
    private final int STATE_STARTED = 1;
    private final String DEFAULT_VALUES = "[GSM,850,192,5,5]" +
            "[TDSCDMA,BAND34,10087,24,10]" +
            "[WCDMA,BAND1,9612,23,10]" +
            "[LTE(FDD),BAND1,19500,24,10]" +
            "[LTE(TDD),BAND34,20175,24,10]" +
            "[CDMA(1X),BC0,384,83,10]" +
            "[10]";

    private List<RfDesenseServiceData> mRfDesenseServiceData = new
            ArrayList<RfDesenseServiceData>();
    private HashMap<String, RfDesenseServiceData>
            mRfdesenseDefaultData = new
            HashMap<String, RfDesenseServiceData>();
    private HashMap<Integer, String> mErrorCodeMapping = new HashMap<Integer, String>();
    private int mErrorCodeKey = 1000;
    private RfDesenseRatInfo mCurrectRatInfo = null;
    private int mState = STATE_NONE;

    private long mTestDuration = 0;
    private int mTestCount = 1;
    private long mTestDurationSended = 0;
    private int mTestCountSended = 0;
    private boolean mIsSending = false;
    private Toast mToast = null;
    private ArrayList<RfDesenseRatInfo> mRatList = new
            ArrayList<RfDesenseRatInfo>();
    private final Handler mServiceHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
                case RfDesenseTxTest.MSG_START_TX_TEST:
                    Elog.d(TAG, "RfService -> entry airplane succeed...");
                    if (mCurrectRatInfo != null) {
                        mCurrectRatInfo.setRatSendState(true);
                        sendAtCommand(mCurrectRatInfo.getRatCmdStart(), RfDesenseTxTest
                                .MSG_START_TX);
                        Elog.d(TAG, "RfService ->send: " + mCurrectRatInfo.getRatName() + " "
                                + mCurrectRatInfo.getRatCmdStart());
                    } else {
                        Elog.d(TAG, "mCurrectRatInfo == null");
                    }
                    break;
                case RfDesenseTxTest.MSG_TURN_OFF_RF:
                    mServiceHandler.sendMessageDelayed(Message.obtain(mServiceHandler,
                            RfDesenseTxTest
                            .MSG_START_TX_TEST), 2000);
                    break;
                case RfDesenseTxTest.MSG_EWMPOLICY_WCDMA:
                    Elog.d(TAG, "RfService ->AT+EWMPOLICY=0 send succeed");
                    Elog.d(TAG, "RfService ->send AT+ECSRA=2,1,0,1,1,0 ...");
                    sendAtCommand("AT+ECSRA=2,1,0,1,1,0", RfDesenseTxTest.MSG_ECSRA);
                    break;
                case RfDesenseTxTest.MSG_EWMPOLICY_TDSCDMA:
                    Elog.d(TAG, "RfService ->AT+EWMPOLICY=0 send succeed");
                    Elog.d(TAG, "RfService ->send AT+ECSRA=2,0,1,0,1,0 ...");
                    sendAtCommand("AT+ECSRA=2,0,1,0,1,0", RfDesenseTxTest.MSG_ECSRA);
                    break;
                case RfDesenseTxTest.MSG_ECSRA:
                    Elog.d(TAG, "RfService ->AT+ECSRA send succeed");
                    startAirplane();
                    break;
                case RfDesenseTxTest.MSG_SWITCH_RAT_DONE:
                    if (ar.exception == null) {
                        Elog.d(TAG, "RfService ->switch rat succeed");
                        if (mCurrectRatInfo.getRatName().equals(RfDesenseTxTest
                                .mRatName[1])) { // tdscdma
                            Elog.d(TAG, "RfService ->send AT+EWMPOLICY=0");
                            sendAtCommand("AT+EWMPOLICY=0", RfDesenseTxTest.MSG_EWMPOLICY_TDSCDMA);
                        } else if (mCurrectRatInfo.getRatName().equals
                                (RfDesenseTxTest.mRatName[2])
                                ) { // wcdma
                            Elog.d(TAG, "RfService ->send AT+EWMPOLICY=0");
                            sendAtCommand("AT+EWMPOLICY=0", RfDesenseTxTest.MSG_EWMPOLICY_WCDMA);
                        } else { // other rat
                            Elog.d(TAG, "RfService ->entry airplane...");
                            startAirplane();
                        }
                    } else {
                        Elog.d(TAG, "RfService ->switch rat failed");
                        mServiceHandler.sendMessageDelayed(Message.obtain(mServiceHandler,
                                RfDesenseTxTest
                                .MSG_SWITCH_RAT), 1000);
                    }
                    break;
                case RfDesenseTxTest.MSG_SWITCH_RAT:
                    if (mState == STATE_STARTED) {
                        Elog.d(TAG, "RfService ->reboot modem succeed");
                        mCurrectRatInfo = getCurrectRatInfo();
                        if (mCurrectRatInfo != null
                                && !mCurrectRatInfo.getRatCmdSwitch().equals("")) {
                            Elog.d(TAG, "RfService ->switch rat(" + mCurrectRatInfo
                                    .getRatCmdSwitch() + ")");
                            sendAtCommand(mCurrectRatInfo.getRatCmdSwitch(), RfDesenseTxTest
                                    .MSG_SWITCH_RAT_DONE);
                        } else {
                            Elog.d(TAG, "RfService ->no need switch rat ");
                            Elog.d(TAG, "RfService ->entry airplane...");
                            startAirplane();
                        }
                    }
                    break;
                case RfDesenseTxTest.MSG_TURN_ON_RF:
                    mServiceHandler.sendMessageDelayed(Message.obtain(mServiceHandler,
                            RfDesenseTxTest.MSG_SWITCH_RAT), 1000);
                    break;

                case RfDesenseTxTest.MSG_CONTINUE_TX:
                    if (mState == RfDesenseTxTest.STATE_STOPPED) {
                        stopTx();
                        return;
                    }
                    mTestDurationSended += 1;
                    Elog.d(TAG, "RfService ->mTestDurationSended:" + mTestDurationSended);
                    if (mTestDurationSended > mTestDuration) {
                        mTestDurationSended = 0;
                        txTestStop(RfDesenseTxTest.MSG_NEXT_RAT);
                    } else {
                        mServiceHandler.sendMessageDelayed(Message.obtain(mServiceHandler,
                                RfDesenseTxTest
                                .MSG_CONTINUE_TX), 1000);
                    }
                    break;
                case RfDesenseTxTest.MSG_NEXT_RAT:
                    if (ar.exception == null) {
                        Elog.d(TAG, "RfService ->stop cmd ok");
                        //sendResultToClient(2004, "done");
                        mCurrectRatInfo = getCurrectRatInfo();
                        if (mCurrectRatInfo != null) {
                            rebootModem();
                        } else {
                            Elog.d(TAG, "RfService ->send done,mTestCountSended = " +
                                    mTestCountSended);
                            mTestCountSended++;
                            if (mTestCountSended < mTestCount) {
                                for (int i = 0; i < RfDesenseTxTest.mRatName.length; i++) {
                                    mRatList.get(i).setRatSendState(false);
                                }
                                rebootModem();
                            } else {
                                Elog.d(TAG, "RfService ->send all rat done");
                                sendResultToClient(2000, "");
                                updateUIView();
                                stopSelf();
                            }
                        }
                    } else {
                        Elog.d(TAG, "RfService ->stop cmd failed");
                        //sendResultToClient(2002, "stop cmd failed");
                        updateUIView();
                    }
                    break;
                case RfDesenseTxTest.MSG_START_TX:
                    if (ar.exception == null) {
                        Elog.d(TAG, "RfService ->start cmd ok");
                        Elog.d(TAG, "RfService ->mTestDuration = " + mTestDuration);
                        sendResultToClient(2001, "start test");
                        mServiceHandler.sendMessageDelayed(Message.obtain(mServiceHandler,
                                RfDesenseTxTest
                                .MSG_CONTINUE_TX), 1000);
                    } else {
                        Elog.d(TAG, "RfService ->start cmd failed");
                        // sendResultToClient(2002, "start cmd failed");
                        mServiceHandler.sendMessageDelayed(Message.obtain(mServiceHandler,
                                RfDesenseTxTest.MSG_CONTINUE_TX), 1000);
                    }
                    break;
            }
        }
    };
    private TelephonyManager mTelephonyManager;
    private int mRadioStatesLast = -1;
    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {

        @Override
        public void onRadioPowerStateChanged(
                @TelephonyManager.RadioPowerState int state) {
            if (state == TelephonyManager.RADIO_POWER_ON) {
                Elog.v(TAG, "RADIO_POWER_ON");
                mServiceHandler.sendEmptyMessage(RfDesenseTxTest.MSG_TURN_ON_RF);
                mRadioStatesLast = state;
            } else if (state == TelephonyManager.RADIO_POWER_OFF) {
                Elog.v(TAG, "RADIO_POWER_OFF");
                if (mRadioStatesLast == TelephonyManager.RADIO_POWER_ON)
                    mServiceHandler.sendEmptyMessage(RfDesenseTxTest.MSG_TURN_OFF_RF);
                mRadioStatesLast = state;
            } else if (state == TelephonyManager.RADIO_POWER_UNAVAILABLE) {
                mRadioStatesLast = state;
                Elog.v(TAG, "RADIO_POWER_UNAVAILABLE");
            }
        }
    };

    //{192, 33, 128}  channel \ tx power \ BAND \ time
    void initDefaultSuppotData() {
        mRfdesenseDefaultData.put("GSM 850", new RfDesenseServiceData(190, 5, 128, 10));
        mRfdesenseDefaultData.put("GSM 900", new RfDesenseServiceData(62, 5, 2, 10));
        mRfdesenseDefaultData.put("GSM 1800", new RfDesenseServiceData(700, 0, 8, 10));
        mRfdesenseDefaultData.put("GSM 1900", new RfDesenseServiceData(661, 0, 16, 10));

        mRfdesenseDefaultData.put("WCDMA BAND1", new RfDesenseServiceData(9750, 23, 1, 10));
        mRfdesenseDefaultData.put("WCDMA BAND2", new RfDesenseServiceData(9400, 23, 2, 10));
        mRfdesenseDefaultData.put("WCDMA BAND3", new RfDesenseServiceData(1112, 23, 3, 10));
        mRfdesenseDefaultData.put("WCDMA BAND4", new RfDesenseServiceData(1412, 23, 4, 10));
        mRfdesenseDefaultData.put("WCDMA BAND5", new RfDesenseServiceData(4182, 23, 5, 10));
        mRfdesenseDefaultData.put("WCDMA BAND6", new RfDesenseServiceData(4175, 23, 6, 10));
        mRfdesenseDefaultData.put("WCDMA BAND7", new RfDesenseServiceData(2175, 23, 7, 10));
        mRfdesenseDefaultData.put("WCDMA BAND8", new RfDesenseServiceData(2787, 23, 8, 10));
        mRfdesenseDefaultData.put("WCDMA BAND9", new RfDesenseServiceData(8837, 23, 9, 10));
        mRfdesenseDefaultData.put("WCDMA BAND10", new RfDesenseServiceData(3025, 23, 10, 10));
        mRfdesenseDefaultData.put("WCDMA BAND11", new RfDesenseServiceData(3524, 23, 11, 10));
        mRfdesenseDefaultData.put("WCDMA BAND12", new RfDesenseServiceData(3647, 23, 12, 10));
        mRfdesenseDefaultData.put("WCDMA BAND13", new RfDesenseServiceData(3805, 23, 13, 10));
        mRfdesenseDefaultData.put("WCDMA BAND14", new RfDesenseServiceData(3905, 23, 14, 10));
        mRfdesenseDefaultData.put("WCDMA BAND19", new RfDesenseServiceData(337, 23, 19, 10));
        mRfdesenseDefaultData.put("WCDMA BAND20", new RfDesenseServiceData(4350, 23, 20, 10));
        mRfdesenseDefaultData.put("WCDMA BAND21", new RfDesenseServiceData(487, 23, 21, 10));
        mRfdesenseDefaultData.put("WCDMA BAND22", new RfDesenseServiceData(4625, 23, 22, 10));

        mRfdesenseDefaultData.put("TDSCDMA BAND34", new RfDesenseServiceData(10087, 24, 1, 10));
        //BAND A
        mRfdesenseDefaultData.put("TDSCDMA BAND39", new RfDesenseServiceData(9500, 24, 6, 10));
        //BAND F

        mRfdesenseDefaultData.put("LTE(FDD) BAND1", new RfDesenseServiceData(19500, 23, 1, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND2", new RfDesenseServiceData(18800, 23, 2, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND3", new RfDesenseServiceData(17475, 23, 3, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND4", new RfDesenseServiceData(17325, 23, 4, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND5", new RfDesenseServiceData(8365, 23, 5, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND6", new RfDesenseServiceData(8300, 23, 46, 10));

        mRfdesenseDefaultData.put("LTE(FDD) BAND7", new RfDesenseServiceData(25350, 23, 7, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND8", new RfDesenseServiceData(8975, 23, 8, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND9", new RfDesenseServiceData(17674, 23, 9, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND10", new RfDesenseServiceData(17400, 23, 10, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND11", new RfDesenseServiceData(14379, 23, 11, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND12", new RfDesenseServiceData(7075, 23, 12, 10));

        mRfdesenseDefaultData.put("LTE(FDD) BAND13", new RfDesenseServiceData(7820, 23, 13, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND14", new RfDesenseServiceData(7930, 23, 14, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND17", new RfDesenseServiceData(7100, 23, 17, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND18", new RfDesenseServiceData(8225, 23, 18, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND19", new RfDesenseServiceData(8375, 23, 19, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND20", new RfDesenseServiceData(8470, 23, 20, 10));

        mRfdesenseDefaultData.put("LTE(FDD) BAND21", new RfDesenseServiceData(14554, 23, 21, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND22", new RfDesenseServiceData(34500, 23, 22, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND23", new RfDesenseServiceData(20100, 23, 23, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND24", new RfDesenseServiceData(16435, 23, 24, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND25", new RfDesenseServiceData(18825, 23, 25, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND26", new RfDesenseServiceData(8315, 23, 26, 10));

        mRfdesenseDefaultData.put("LTE(FDD) BAND27", new RfDesenseServiceData(8155, 23, 27, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND28", new RfDesenseServiceData(7255, 23, 28, 10));

        mRfdesenseDefaultData.put("LTE(FDD) BAND30", new RfDesenseServiceData(23100, 23, 30, 10));
        mRfdesenseDefaultData.put("LTE(FDD) BAND31", new RfDesenseServiceData(4550, 23, 31, 10));

        mRfdesenseDefaultData.put("LTE(TDD) BAND33", new RfDesenseServiceData(19100, 23, 33, 10));
        mRfdesenseDefaultData.put("LTE(TDD) BAND34", new RfDesenseServiceData(20175, 23, 34, 10));
        mRfdesenseDefaultData.put("LTE(TDD) BAND35", new RfDesenseServiceData(18800, 23, 35, 10));
        mRfdesenseDefaultData.put("LTE(TDD) BAND36", new RfDesenseServiceData(19600, 23, 36, 10));
        mRfdesenseDefaultData.put("LTE(TDD) BAND37", new RfDesenseServiceData(19200, 23, 37, 10));
        mRfdesenseDefaultData.put("LTE(TDD) BAND38", new RfDesenseServiceData(25950, 23, 38, 10));
        mRfdesenseDefaultData.put("LTE(TDD) BAND39", new RfDesenseServiceData(19000, 23, 39, 10));
        mRfdesenseDefaultData.put("LTE(TDD) BAND40", new RfDesenseServiceData(23500, 23, 40, 10));
        mRfdesenseDefaultData.put("LTE(TDD) BAND41", new RfDesenseServiceData(25930, 23, 41, 10));
        mRfdesenseDefaultData.put("LTE(TDD) BAND42", new RfDesenseServiceData(35000, 23, 42, 10));
        mRfdesenseDefaultData.put("LTE(TDD) BAND43", new RfDesenseServiceData(37000, 23, 43, 10));
        mRfdesenseDefaultData.put("LTE(TDD) BAND44", new RfDesenseServiceData(7530, 23, 44, 10));
        mRfdesenseDefaultData.put("CDMA(1X) BC0", new RfDesenseServiceData(384, 23 + 60, 0, 10));

        mErrorCodeMapping.put(1000, "argument parse pass and start tx");
        mErrorCodeMapping.put(1001, "[rat,");
        mErrorCodeMapping.put(1002, "service is tx ing,please try again later");
        mErrorCodeMapping.put(1003, "service works well");

        mErrorCodeMapping.put(2000, "test finished");
        mErrorCodeMapping.put(2001, "test start ret");
        mErrorCodeMapping.put(2002, "test failed");
        mErrorCodeMapping.put(2003, "test stoped");
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }


    private void parseIntentData(String arguments) {
        String[] info;
        String rat[] = arguments.split("\\[|\\]");
        RfDesenseServiceData mDefaultRfData = null;
        if (rat.length == 0) {
            sendResultToClient(1001, "null]");
            return;
        }
        for (int i = 0; i < rat.length; i++) {
            if (!rat[i].isEmpty()) {
                Elog.d(TAG, "RfService ->rat = " + rat[i]);
                RfDesenseServiceData rfreceiveData = new com
                        .mediatek.engineermode.rfdesense.RfDesenseServiceData();
                info = rat[i].split(",");
                if (info.length == 1) {
                    try {
                        int count = Integer.valueOf(info[0]);
                        if (count == -1)
                            mTestCount = 1;
                        else
                            mTestCount = count;
                    } catch (Exception e) {
                        if (info[0].equals("TEST")) {
                            sendResultToClient(1003, "");
                        } else {
                            sendResultToClient(1001, info[0] + "]");
                        }
                        return;
                    }
                } else if (info.length < 5) {
                    sendResultToClient(1001, info[0] + "]");
                    return;
                } else {
                    rfreceiveData.setRat(info[0]);
                    mDefaultRfData = mRfdesenseDefaultData.get(info[0] + " " + info[1]);
                    if (mDefaultRfData != null) {
                        rfreceiveData.setBand(mDefaultRfData.getBand());
                    } else {
                        sendResultToClient(1001, info[0] + "]");
                        return;
                    }
                    try {
                        int channel = Integer.valueOf(info[2]);
                        if (channel == -1)
                            rfreceiveData.setChannel(mDefaultRfData.getChannel());
                        else
                            rfreceiveData.setChannel(channel);

                        int power = Integer.valueOf(info[3]);
                        if (power == -1)
                            rfreceiveData.setPower(mDefaultRfData.getPower());
                        else
                            rfreceiveData.setPower(power);

                        int time = Integer.valueOf(info[4]);
                        if (time == -1)
                            rfreceiveData.setTime(mDefaultRfData.getTime());
                        else
                            rfreceiveData.setTime(time);

                        if (info.length > 5) {
                            int bw = Integer.valueOf(info[5]);
                            rfreceiveData.setBw(bw);
                        }
                        if (info.length > 6) {
                            int rb = Integer.valueOf(info[6]);
                            rfreceiveData.setRb(rb);
                        }
                        if (info.length > 7 || (info.length > 5 &&
                                !(info[0].equals(RfDesenseTxTest.mRatName[3]) ||
                                        info[0].equals(RfDesenseTxTest.mRatName[4])))
                                ) {
                            sendResultToClient(1001, info[0] + "]");
                            return;
                        }
                    } catch (Exception e) {
                        sendResultToClient(1001, info[0] + "]");
                        return;
                    }
                    mRfDesenseServiceData.add(rfreceiveData);
                }
            }
        }
    }

    private void initRatList() {
        //init rf desense cmd
        for (int i = 0; i < RfDesenseTxTest.mRatName.length; i++) {
            RfDesenseRatInfo mRatInfo = new RfDesenseRatInfo();
            mRatInfo.setRatName(RfDesenseTxTest.mRatName[i]);
            mRatInfo.setRatCmdStart(RfDesenseTxTest.mRatCmdStart[i]);
            mRatInfo.setRatCmdStop(RfDesenseTxTest.mRatCmdStop[i]);
            mRatInfo.setRatCmdSwitch(RfDesenseTxTest.mRatCmdSwitch[i]);
            mRatInfo.setRatPowerRead(RfDesenseTxTest.mRatCmdPowerRead[i]);
            mRatInfo.setRatCheckState(false);
            mRatInfo.setRatSendState(false);
            mRatList.add(mRatInfo);
        }
    }

    private void updateRatList() {
        int k = 0;
        for (int i = 0; i < mRfDesenseServiceData.size(); i++) {
            //Elog.d(TAG, "RfService ->result = " + mRfDesenseServiceData.get(i).toString());
            for (k = 0; k < RfDesenseTxTest.mRatName.length; k++) {
                if ((RfDesenseTxTest.mRatName[k].equals
                        (mRfDesenseServiceData.get(i).getRat()))) {
                    //Elog.d(TAG, "RfService ->mRatInfo default = " + mRatList.get(k)
                    // .getRatCmdStart());
                    mRatList.get(k).setRatCmdLteBwRb(mRfDesenseServiceData.get(i).getBw(),
                            mRfDesenseServiceData.get(i).getRb());
                    mRatList.get(k).setRatCmdStart(RfDesenseTxTest.mRatName[k],
                            mRfDesenseServiceData.get(i).getChannel(),
                            mRfDesenseServiceData.get(i).getPower(),
                            mRfDesenseServiceData.get(i).getBand()
                    );
                    mRatList.get(k).setRatTxtimes(mRfDesenseServiceData.get(i).getTime());
                    Elog.d(TAG, "RfService ->mRatInfo send = " + RfDesenseTxTest.mRatName[k]
                            + "," + mRatList.get(k).getRatCmdStart() +
                            ",time = " + mRatList.get(k).getRatTxtimes());
                    mRatList.get(k).setRatCheckState(true);
                    break;
                }
            }
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Elog.d(TAG, "RfService -> onCreate()");
        mTelephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener
                .LISTEN_RADIO_POWER_STATE_CHANGED);
        startForeground(1, getNotification("Rfdesense service started...", 0));
    }

    @Override
    public void onDestroy() {
        Elog.d(TAG, "RfService -> onDestroy");

        stopForeground(true);
        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        mState = RfDesenseTxTest.STATE_STOPPED;

        if (mIsSending == true)
            sendResultToClient(2003, "");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Elog.d(TAG, "RfService -> onStartCommand()");
        if (mIsSending == true) {
            //Elog.d(TAG, "RfService ->it is tx ing ,please wait");
            sendResultToClient(1002, "");
            return START_NOT_STICKY;
        }
        String arguments = intent.getStringExtra(RfDesenseBroadcastReceiver.ARGUMENTS);
        if (arguments == null) {
            arguments = DEFAULT_VALUES;
            Elog.d(TAG, "RfService -> use default arguments");
        }
        arguments = arguments.replaceAll(" ", "");
        Elog.d(TAG, "RfService -> arguments: " + arguments);

        mRatList.clear();
        mRfDesenseServiceData.clear();
        mErrorCodeKey = 1000;

        initDefaultSuppotData();

        initRatList();

        parseIntentData(arguments);

        if (mErrorCodeKey == 1000) {
            updateRatList();
            sendResultToClient(1000, "");
            startTx();
        }
        return START_NOT_STICKY;
    }

    void sendResultToClient(int errorCodeKey, String msg) {
        String info = "";
        String rat = "";
        mErrorCodeKey = errorCodeKey;
        if (mCurrectRatInfo != null) {
            rat = mCurrectRatInfo.getRatName();
        }
        if (errorCodeKey == 2002 || errorCodeKey == 2001) {
            info = "[count," + mTestCountSended + "][rat," + rat + "]";
        } else {
            info = mErrorCodeMapping.get(errorCodeKey) + msg;
        }

        Intent intent = new Intent();
        intent.setAction("com.mediatek.engineermode.rfdesenseServiceResult");
        intent.putExtra("result_id", errorCodeKey);
        intent.putExtra("result_info", info);
        sendBroadcast(intent,"com.mediatek.engineermode.rfdesenseService.permission");
        Elog.d(TAG, "RfService ->send " + errorCodeKey + " : " + mErrorCodeMapping.get
                (errorCodeKey) + info);
    }


    void stopTx() {
        mIsSending = false;
        mServiceHandler.removeCallbacksAndMessages(null);
        rebootModem();
        super.onDestroy();
    }

    void startTx() {
        mCurrectRatInfo = getCurrectRatInfo();
        if (mCurrectRatInfo != null) {
            Elog.d(TAG, "RfService ->mCurrectRatInfo = " + mCurrectRatInfo.getRatCmdStart());
        } else {
            Elog.d(TAG, "RfService ->you must set at least one rat to tx ");
            return;
        }
        mIsSending = true;

        mTestCountSended = 0;
        mTestDurationSended = 0;
        mState = STATE_STARTED;
        Elog.d(TAG, "RfService ->mTestCount = " + mTestCount);
        rebootModem();
    }

    private RfDesenseRatInfo getCurrectRatInfo() {
        int index;
        mCurrectRatInfo = null;
        for (index = 0; index < mRatList.size(); index++) {
            if (mRatList.get(index).getRatCheckState()) {
                if (mRatList.get(index).getRatSendState()) {
                    continue;
                }
                mCurrectRatInfo = mRatList.get(index);
                mTestDuration = mCurrectRatInfo.getRatTxtimes() * 60;
                break;
            }
        }
        return mCurrectRatInfo;
    }

    private void updateUIView() {
        mIsSending = false;
        for (int i = 0; i < RfDesenseTxTest.mRatName.length;
             i++) {
            mRatList.get(i).setRatSendState(false);
        }
    }

    void startAirplane() {
        Elog.d(TAG, "start entry Airplane...");
        EmUtils.setAirplaneModeEnabled(true);
    }

    private void rebootModem() {
        EmUtils.setAirplaneModeEnabled(false);
        EmUtils.rebootModem();
    }

    private void txTestStop(int what) {
        if (mCurrectRatInfo != null) {
            sendAtCommand(mCurrectRatInfo.getRatCmdStop(), what);
            Elog.d(TAG, "RfService ->stop: " + mCurrectRatInfo.getRatName() + " " +
                    mCurrectRatInfo.getRatCmdStop());
        } else {
            Elog.d(TAG, "RfService ->mCurrectRatInfo is null");
            updateUIView();
        }
    }

    private void sendAtCommand(String str, int what) {
        String cmd[] = new String[]{str, ""};
        // Elog.d(TAG, "RfService ->send: " + cmd[0]);
        EmUtils.invokeOemRilRequestStringsEm(cmd, mServiceHandler.obtainMessage(what));
    }

    private NotificationManager getNotificationManager() {
        return (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
    }

    private Notification getNotification(String title, int progress) {
        Intent intent = new Intent(this, RfDesenseTxTest.class);
        PendingIntent pi = PendingIntent.getActivity(this, 0, intent, 0);
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this,
                EmApplication.getSilentNotificationChannelID());
        builder.setSmallIcon(R.drawable.cross);
        builder.setLargeIcon(BitmapFactory.decodeResource(getResources(), R.drawable.cross));
        builder.setContentIntent(pi);
        builder.setContentTitle(title);
        return builder.build();
    }
}
