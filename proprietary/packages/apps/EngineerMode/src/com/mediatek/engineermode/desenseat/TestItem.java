/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.desenseat;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.KeyguardManager;
import android.app.WallpaperManager;
import android.bluetooth.BluetoothAdapter;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.DisplayMetrics;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.engineermode.bluetooth.BtTest;
import com.mediatek.engineermode.wifi.EMWifi;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;

/**
 * Test items.
 */
public abstract class TestItem {
    public static final int INDEX_GSM_BAND = 0;
    public static final int INDEX_UMTS_BAND = 1;
    public static final int INDEX_LTE_FDD_BAND = 2;
    public static final int INDEX_LTE_TDD_BAND = 3;
    public static final int INDEX_CDMA_BAND = 4;
    public static final int INDEX_BAND_MAX = 5;
    public static final String TEST_MP3_FILE = "DesenseAT.mp3";
    public static final String TEST_3GP_FILE = "DesenseAT.3gp";
    public static final String TEST_GIF_FILE = "DesenseAT.gif";
    protected static final String TAG = "DesenseAT/testItem";
    protected static final int TEST_STATE_ON = 0;
    protected static final int TEST_STATE_OFF = 1;
    private static final int CNR_FAIL_VALUE = 30;
    public static List<String> sItemsToCheck;
    private static ArrayList<TestItem> sTestItems;
    protected Context mContext;
    protected TestMode mTestMode;
    private TestResult mTestModeResult;
    private int mItemIndex;
    private long mTimeToWait;
    private boolean mTimeOut = false;
    private Timer mTimer;
    private TimerTask mTimerTask;


    static ArrayList<TestItem> getTestItems() {
        if (sTestItems == null) {
            sTestItems = new ArrayList<TestItem>();
            sTestItems.add(new Display3DTest());
            sTestItems.add(new SleepToNormalTest());
            sTestItems.add(new LcmOnTest());
            sTestItems.add(new BacklightTest());
            sTestItems.add(new WifiTxTest());
            sTestItems.add(new WifiRxTest());
            sTestItems.add(new BtRxInquiryScanTest());
            sTestItems.add(new BtTxTest());
            sTestItems.add(new LiveWallpaperTest());
            sTestItems.add(new Mp3PlayTest());
            sTestItems.add(new VideoPlayTest());
            sTestItems.add(new PhoneStorageReadTest());
            sTestItems.add(new PhoneStorageWriteTest());
            sTestItems.add(new ExternalSdCardReadTest());
            sTestItems.add(new ExternalSdCardWriteTest());
            sTestItems.add(new MemoryAccessTest());
            sTestItems.add(new FrontCameraPreviewTest());
            sTestItems.add(new BackCameraPreviewTest());
            sTestItems.add(new GsmBandVirtualTest());
            if (ModemCategory.getModemType() == ModemCategory.MODEM_TD) {
                sTestItems.add(new TddBandVirtualTest());
            } else if (ModemCategory.getModemType() == ModemCategory.MODEM_FDD) {
                sTestItems.add(new FddBandVirtualTest());
            }
            if (ModemCategory.isLteSupport()) {
                sTestItems.add(new LteBandVirtualTest());
            }
            if (ModemCategory.isCdma()) {
                sTestItems.add(new CdmaBandVirtualTest());
            }

        }

        return sTestItems;
    }

    static void updateContext(Context context) {
        if (sTestItems != null) {
            for (TestItem i : sTestItems) {
                i.setContext(context);
            }
        }
    }

    protected int getDbSpec() {
        return 30;
    }

    protected boolean isTestDone() {
        return mTestModeResult.state == TestResult.ResultState.STATE_DONE;
    }

    protected boolean isTestRefDone() {
        return mTestModeResult.state == TestResult.ResultState.STATE_REF_DONE;
    }

    protected boolean isTestNa() {
        return mTestModeResult.state == TestResult.ResultState.STATE_NA;
    }

    public void setItemIndex(int itemIndex) {
        mItemIndex = itemIndex;
    }

    protected abstract void init();

    protected abstract void reset();

    protected abstract void doTest();

    protected abstract boolean doApiTest();

    protected abstract String getTag();

    protected void returnToTestView() {
        do {
            Util.pressBack();
            sleep(Util.TIME_THREE_SEC);
        } while (!Util.isTopActivity(mContext, mContext.getClass().getName()));

    }

    protected void returnToTestView(String packetname) {
        Elog.d(TAG, "killing app packet name = " + packetname);
        try {
            ActivityManager mActivityManager = (ActivityManager) mContext.getSystemService
                    (Context.ACTIVITY_SERVICE);
            Method method = Class.forName("android.app.ActivityManager").getMethod
                    ("forceStopPackage", String.class);
            method.invoke(mActivityManager, packetname);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public boolean isCancelled() {
        return DesenseAtActivity.getCancelled();
    }

    void sleep(long time) {
        // TODO: not exactly
        if (!isCancelled()) {
            Util.sleep(time);
        }
    }

    TestResult run() {
        // Pass 1
        Elog.d(TAG, "init()");
        init();
        if (isCancelled()) {
            Elog.d(TAG, "after init return null");
            reset();
            return null;
        }
        Elog.d(TAG, "startTestMode()");
        startTestMode(TEST_STATE_ON);
        Elog.d(TAG, "stopTestMode()");
        stopTestMode(TEST_STATE_ON);
        Elog.d(TAG, "after stopTestMode()");
        if (mTimeOut && !isTestRefDone()) {
            mTestModeResult.result = TestResult.CONNECTION_FAIL;
        }

        if (mTestModeResult.result == TestResult.CONNECTION_FAIL) {
            reset();
            return mTestModeResult;
        }

        // Pass 2
        if (isCancelled()) {
            Elog.d(TAG, "after base return null");
            reset();
            return null;
        }
        Elog.d(TAG, "doTest()");
        doTest();
        if (!BandTest.sAtcDone) {
            reset();
            mTestModeResult.result = TestResult.MODEM_FAIL;
            return mTestModeResult;
        }
        if (mTimeOut && !isTestDone()) {
            mTestModeResult.result = TestResult.CONNECTION_FAIL;
        }

        if (mTestModeResult.result == TestResult.CONNECTION_FAIL) {
            reset();
            return mTestModeResult;
        }
        Elog.d(TAG, "calculateResult()");

        if (!checkForCNRValid()) {
            mTestModeResult.result = TestResult.CNR_FAIL;
        }
        reset();
        return mTestModeResult;

    }

    private boolean checkForCNRValid() {

        for (TestResult.Entry entry : mTestModeResult.subResults.values()) {

            if ((entry.averageBase <= CNR_FAIL_VALUE) || (entry.averageTest <= CNR_FAIL_VALUE)) {
                return false;
            }
            ArrayList<Float> data = entry.data;
            int size = data.size();
            for (int i = 0; i < size - 2; i++) {
                if ((data.get(i) <= CNR_FAIL_VALUE) && (data.get(i + 1) <= CNR_FAIL_VALUE)
                        && (data.get(i + 2) <= CNR_FAIL_VALUE)) {
                    return false;
                }
            }
        }

        return true;
    }

    void setContext(Context context) {
        mContext = context;
    }

    void setTestMode(TestMode mode) {
        mTestMode = mode;
    }

    protected void startTestMode(int testState) {
        if (testState == TEST_STATE_ON) {
            mTestModeResult = null;
        }
        mTestMode.setParameters(mItemIndex, testState, 30);
        new Thread() {
            public void run() {
                mTestModeResult = mTestMode.run();
            }
        }.start();

        mTimeOut = false;
        if (mTestMode.toString().equals("Signal")) {
            mTimeToWait = 40000;
        } else {
            mTimeToWait = 30000;
        }
        mTimer = new Timer();
        mTimerTask = new TimerTask() {
            public void run() {
                mTimeOut = true;
            }
        };
        mTimer.schedule(mTimerTask, mTimeToWait);

    }


    protected void stopTestMode(int testState) {

        if (testState == 0) {
            while ((mTestModeResult == null)
                    || (!isTestRefDone() && !isTestNa())) {
                if (!mTimeOut) {
                    sleep(Util.TIME_HUNDRED_MILI_SEC);
                } else {
                    break;
                }

            }
        } else {
            while ((mTestModeResult == null)
                    || (!isTestDone() && !isTestNa())) {
                if (!mTimeOut) {
                    sleep(Util.TIME_HUNDRED_MILI_SEC);
                } else {
                    break;
                }
            }
        }
        mTimer.cancel();
        mTimerTask.cancel();
        mTimer.purge();
        mTimer = null;
        mTimerTask = null;

    }
}

/**
 * TestMode.
 */
class SleepToNormalTest extends TestItem {
    @Override
    public void init() {
        switchSleepMode(true);
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        switchSleepMode(false);
        startTestMode(TEST_STATE_OFF);
        stopTestMode(TEST_STATE_OFF);
    }

    @Override
    public String toString() {
        return "Suspend";
    }

    @Override
    public String getTag() {
        return "1";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub

        if (!switchSleepMode(true)) {
            return false;
        }
        sleep(Util.TIME_FIVE_SEC);
        if (!switchSleepMode(false)) {
            return false;
        }
        return true;

    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        switchSleepMode(false);
        sleep(Util.TIME_FIVE_SEC);
    }
    private boolean mInSleepMode = false;
    private boolean switchSleepMode(boolean on) {
        Elog.d(TAG, "switchSleepMode : " + on);
        if (mInSleepMode == on) {
            return true;
        }
        try {
            if (ShellExe.execCommand("input keyevent 26", true) == Util.RETURN_SUCCESS) {
                mInSleepMode = !mInSleepMode;
                unlockKeyguard();
                return true;
            } else {
                return false;
            }
        } catch (IOException e) {
            return false;
        }
    }

    private void unlockKeyguard() {
        KeyguardManager keyguardMgr = (KeyguardManager) mContext.
            getSystemService(Context.KEYGUARD_SERVICE);
        if (mContext instanceof Activity) {
            Elog.i(TAG, "is activity");
            if (keyguardMgr.isKeyguardLocked()) {
                Elog.i(TAG, "is locked");
                keyguardMgr.requestDismissKeyguard((Activity)mContext,
                        new KeyguardManager.KeyguardDismissCallback() {

                            @Override
                            public void onDismissError() {
                                // TODO Auto-generated method stub
                                super.onDismissError();
                                Elog.i(TAG, "unlock error");
                            }

                            @Override
                            public void onDismissSucceeded() {
                                // TODO Auto-generated method stub
                                super.onDismissSucceeded();
                                Elog.i(TAG, "unlock success");
                            }
                });
            }
        }
    }
}

/**
 * TestMode.
 */
class LcmOnTest extends TestItem {
    protected int getDbSpec() {
        return 10;
    }

    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, true);
        Util.lcmOff();
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        Util.lcmOn();
        sleep(Util.TIME_FIVE_SEC);
        startTestMode(TEST_STATE_OFF);
//        sleep(30 * 1000);
        stopTestMode(TEST_STATE_OFF);
    }

    @Override
    public String toString() {
        return "LCM on";
    }

    @Override
    public String getTag() {
        return "2";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub

        if (!Util.lcmOff()) {
            return false;
        }
        sleep(Util.TIME_FIVE_SEC);
        if (!Util.lcmOn()) {
            return false;
        }
        return true;
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.lcmOn();

    }
}

/**
 * TestMode.
 */
class BacklightTest extends TestItem {
    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, true);
        Util.backlightOff();
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        Util.backlightOn();
        startTestMode(TEST_STATE_OFF);
        stopTestMode(TEST_STATE_OFF);
    }

    @Override
    public String toString() {
        return "Backlight";
    }

    @Override
    public String getTag() {
        return "3";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub

        if (!Util.backlightOff()) {
            return false;
        }
        sleep(Util.TIME_FIVE_SEC);
        if (!Util.backlightOn()) {
            return false;
        }
        return true;

    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.backlightOn();

    }
}

/**
 * TestMode.
 */
class WifiTxTest extends TestItem {
    // From WiFiTx6620.java
    private static final int ATPARAM_INDEX_COMMAND = 1;
    private static final int ATPARAM_INDEX_POWER = 2;
    private static final int ATPARAM_INDEX_RATE = 3;
    private static final int ATPARAM_INDEX_PREAMBLE = 4;
    private static final int ATPARAM_INDEX_ANTENNA = 5;
    private static final int ATPARAM_INDEX_PACKLENGTH = 6;
    private static final int ATPARAM_INDEX_PACKCOUNT = 7;
    private static final int ATPARAM_INDEX_PACKINTERVAL = 8;
    private static final int ATPARAM_INDEX_TEMP_COMPENSATION = 9;
    private static final int ATPARAM_INDEX_TXOP_LIMIT = 10;
    private static final int ATPARAM_INDEX_PACKCONTENT = 12;
    private static final int ATPARAM_INDEX_RETRY_LIMIT = 13;
    private static final int ATPARAM_INDEX_QOS_QUEUE = 14;
    private static final int ATPARAM_INDEX_BANDWIDTH = 15;
    private static final int ATPARAM_INDEX_GI = 16;
    private static final long ATPARAM_INDEX_CHANNEL0 = 18;
    private static final int ATPARAM_INDEX_POWER_UNIT = 31;
    private static final int ATPARAM_INDEX_CHANNEL_BANDWIDTH = 71;
    private static final int ATPARAM_INDEX_DATA_BANDWIDTH = 72;
    private static final int ATPARAM_INDEX_PRIMARY_SETTING = 73;
    private static final int ATPARAM_INDEX_SET_DBDC_ENABLE = 110;
    private static final int ATPARAM_INDEX_WF0 = 113;
    private static final int ATPARAM_INDEX_NSS = 114;
    private static final int TXOP_LIMIT_VALUE = 0x00020000;
    private static final int ANTENNA = 0;
    private static final long[] PACKCONTENT_BUFFER =
            {0xff220004, 0x33440006, 0x55660008, 0x55550019, 0xaaaa001b, 0xbbbb001d};
    private static final int COMMAND_INDEX_STARTTX = 1;
    private static final int COMMAND_INDEX_STOPTEST = 0;
    private static final String CHIP_6632 = "6632";
    private static final String CHIP_ADV = "0000";

    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, false);
        Util.lcmOff();
        Util.backlightOff();
        Util.switchWifi(mContext, false);
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        Util.enableWifiEm(mContext);
        startTestMode(TEST_STATE_OFF);
        // Rate index:
        //    Refer to com.mediatek.engineermode.wifi.WiFiTx6620.RateInfo.mPszRate
        // Channel frequency:
        //    Refer to com.mediatek.engineermode.wifi.ChannelInfo.sWifiChannelDatas
        if (CHIP_6632.equals(Util.getWifiChipId())
                || CHIP_ADV.equals(Util.getWifiChipId())) {
            Elog.d(TAG, "doTx6632");
            doTx6632(19, 0, 2412000); // 19dBm, 1M, Channel 1
            doTx6632(16, 0, 2437000); // 19dBm, 1M, Channel 6
            doTx6632(16, 0, 2462000); // 19dBm, 1M, Channel 11
            doTx6632(16, 11, 5500000); // 16dBm, 54M, Channel 100
        } else {
            Elog.d(TAG, "doTx6620");
            doTx6620(19, 0, 2412000); // 19dBm, 1M, Channel 1
            doTx6620(16, 0, 2437000); // 19dBm, 1M, Channel 6
            doTx6620(16, 0, 2462000); // 19dBm, 1M, Channel 11
            doTx6620(16, 11, 5500000); // 16dBm, 54M, Channel 100
        }

        stopTestMode(TEST_STATE_OFF);
        Util.disableWifiEm(mContext);
    }

    private void doTx6620(int power, int rateIndex, int channel) {
        if (isCancelled()) {
            return;
        }
        // From WiFiTx6620.java
        power *= 2;
        EMWifi.setATParam(ATPARAM_INDEX_POWER, power);
        EMWifi.setATParam(ATPARAM_INDEX_PREAMBLE, 0);
        EMWifi.setATParam(ATPARAM_INDEX_ANTENNA, ANTENNA);
        EMWifi.setATParam(ATPARAM_INDEX_PACKLENGTH, 1024);
        EMWifi.setATParam(ATPARAM_INDEX_PACKCOUNT, 3000);
        EMWifi.setATParam(ATPARAM_INDEX_PACKINTERVAL, 20);
        EMWifi.setATParam(ATPARAM_INDEX_TEMP_COMPENSATION, 0);
        EMWifi.setATParam(ATPARAM_INDEX_TXOP_LIMIT, TXOP_LIMIT_VALUE);
        for (int i = 0; i < PACKCONTENT_BUFFER.length; i++) {
            EMWifi.setATParam(ATPARAM_INDEX_PACKCONTENT, PACKCONTENT_BUFFER[i]);
        }
        EMWifi.setATParam(ATPARAM_INDEX_RETRY_LIMIT, 1);
        EMWifi.setATParam(ATPARAM_INDEX_QOS_QUEUE, 2);
        EMWifi.setATParam(ATPARAM_INDEX_GI, 0);
        EMWifi.setATParam(ATPARAM_INDEX_BANDWIDTH, 0); // TODO: check it
        EMWifi.setATParam(ATPARAM_INDEX_RATE, rateIndex);
        EMWifi.setChannel(channel);
        int ret = EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STARTTX);
        Elog.d(TAG, "start tx ret " + ret);
        sleep(Util.TIME_FIFTEEN_SEC); // TODO: is it OK?
        ret = EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STOPTEST);
        Elog.d(TAG, "stop tx ret " + ret);
    }

    private void doTx6632(int power, int rateIndex, int channel) {
        if (isCancelled()) {
            return;
        }
        // From WiFiTx6632.java
        EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_ENABLE, 0);
        EMWifi.setATParam(ATPARAM_INDEX_NSS, 1);
        EMWifi.setATParam(ATPARAM_INDEX_WF0, 1);
        EMWifi.setATParam(ATPARAM_INDEX_CHANNEL_BANDWIDTH, 0);
        EMWifi.setATParam(ATPARAM_INDEX_DATA_BANDWIDTH, 0);
        EMWifi.setATParam(ATPARAM_INDEX_PRIMARY_SETTING, 0);
        EMWifi.setATParam(ATPARAM_INDEX_CHANNEL0, channel);
        EMWifi.setATParam(ATPARAM_INDEX_RATE, rateIndex);
        EMWifi.setATParam(ATPARAM_INDEX_PACKLENGTH, 1024);
        EMWifi.setATParam(ATPARAM_INDEX_PACKCOUNT, 3000);
        EMWifi.setATParam(ATPARAM_INDEX_POWER_UNIT, 0);
        power *= 2;
        EMWifi.setATParam(ATPARAM_INDEX_POWER, power);
        EMWifi.setATParam(ATPARAM_INDEX_PREAMBLE, 0);
        EMWifi.setATParam(ATPARAM_INDEX_GI, 0);
        int ret = EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STARTTX);
        Elog.d(TAG, "start tx ret " + ret);
        sleep(Util.TIME_FIFTEEN_SEC); // TODO: is it OK?
        ret = EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STOPTEST);
        Elog.d(TAG, "stop tx ret " + ret);

    }

    @Override
    public String toString() {
        return "WIFI TX";
    }

    @Override
    public String getTag() {
        return "4";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return FeatureSupport.isWifiSupport(mContext);
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.backlightOn();
        Util.lcmOn();

    }
}

/**
 * TestMode.
 */
class WifiRxTest extends TestItem {
    // From WiFiRx6620.java
    private static final int ATPARAM_INDEX_COMMAND = 1;
    private static final int ATPARAM_INDEX_TEMP_COMPENSATION = 9;
    private static final int ATPARAM_INDEX_BANDWIDTH = 15;
    private static final long ATPARAM_INDEX_CHANNEL0 = 18;
    private static final int ATPARAM_INDEX_CHANNEL_BANDWIDTH = 71;
    private static final int ATPARAM_INDEX_DATA_BANDWIDTH = 72;
    private static final int ATPARAM_INDEX_PRIMARY_SETTING = 73;
    private static final int ATPARAM_INDEX_RX = 106;
    private static final int ATPARAM_INDEX_SET_DBDC_ENABLE = 110;
    private static final String CHIP_6632 = "6632";
    private static final String CHIP_ADV = "0000";
    private static final int COMMAND_INDEX_STARTRX = 2;
    private static final int COMMAND_INDEX_STOPTEST = 0;

    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, false);
        Util.lcmOff();
        Util.backlightOff();
        Util.switchWifi(mContext, false);
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        Util.enableWifiEm(mContext);
        startTestMode(TEST_STATE_OFF);
        if (CHIP_6632.equals(Util.getWifiChipId())
                || CHIP_ADV.equals(Util.getWifiChipId())) {
            Elog.d(TAG, "doRx6632");
            doRx6632(2442000);
        } else {
            Elog.d(TAG, "doRx6620");
            doRx6620(2442000);
        }
        stopTestMode(TEST_STATE_OFF);
        Util.disableWifiEm(mContext);
    }

    private void doRx6620(int channel) {
        if (isCancelled()) {
            return;
        }
        // Channel 7, Bandwidth 20MHz
        EMWifi.setChannel(channel);
        EMWifi.setATParam(ATPARAM_INDEX_TEMP_COMPENSATION, 0);
        EMWifi.setATParam(ATPARAM_INDEX_BANDWIDTH, 0);
        int ret = EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STARTRX);
        Elog.d(TAG, "start rx ret " + ret);
        sleep(Util.TIME_THIRTY_SEC);
        ret = EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STOPTEST);
        Elog.d(TAG, "stop rx ret " + ret);
    }

    private void doRx6632(int channel) {
        if (isCancelled()) {
            return;
        }
        EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_ENABLE, 0);
        EMWifi.setATParam(ATPARAM_INDEX_RX, 0x00010000);
        EMWifi.setATParam(ATPARAM_INDEX_CHANNEL_BANDWIDTH, 0);
        EMWifi.setATParam(ATPARAM_INDEX_DATA_BANDWIDTH, 0);
        EMWifi.setATParam(ATPARAM_INDEX_PRIMARY_SETTING, 0);
        EMWifi.setATParam(ATPARAM_INDEX_CHANNEL0, channel);
        int ret = EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STARTRX);
        Elog.d(TAG, "start rx ret " + ret);
        sleep(Util.TIME_THIRTY_SEC);
        ret = EMWifi.setATParam(ATPARAM_INDEX_COMMAND, 0);
        Elog.d(TAG, "stop rx ret " + ret);
    }

    @Override
    public String toString() {
        return "WIFI RX";
    }

    @Override
    public String getTag() {
        return "5";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return FeatureSupport.isWifiSupport(mContext);

    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.backlightOn();
        Util.lcmOn();
    }
}

/**
 * TestMode.
 */
class BtRxInquiryScanTest extends TestItem {
    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, false);
        Util.lcmOff();
        Util.backlightOff();
        Util.switchBt(false);
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        Util.switchBt(true);
        startTestMode(TEST_STATE_OFF);
        BluetoothAdapter.getDefaultAdapter().startDiscovery();
        Elog.d(TAG, "startDiscovery");
        sleep(Util.TIME_THIRTY_SEC);
        BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
        Elog.d(TAG, "cancelDiscovery");
        stopTestMode(TEST_STATE_OFF);
    }

    @Override
    public String toString() {
        return "BT RX inquiry scan";
    }

    @Override
    public String getTag() {
        return "6";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return FeatureSupport.isEmBTSupport();
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.backlightOn();
        Util.lcmOn();
        Util.switchBt(false);
        sleep(Util.TIME_FIVE_SEC);
    }
}

/**
 * TestMode.
 */
class BtTxTest extends TestItem {
    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, false);
        Util.lcmOff();
        Util.backlightOff();
        Util.switchBt(false);
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        Util.enableBtEm();
        startTestMode(TEST_STATE_OFF);

        // reference to com.mediatek.engineermode.bluetooth.TxOnlyTestActivity
        BtTest mBtTest = new BtTest();
        Elog.d(TAG, "start bt tx");
        mBtTest.setPatter(3); // pseudo random bit
        mBtTest.setChannels(1); // 79 channel
        mBtTest.setPocketType(4); // DH1
        mBtTest.setPocketTypeLen(27);
        mBtTest.doBtTest(0);
        mBtTest.pollingStart();
        sleep(Util.TIME_THIRTY_SEC);
        Elog.d(TAG, "stop bt tx");
        mBtTest.pollingStop();
        mBtTest.doBtTest(3);
        stopTestMode(TEST_STATE_OFF);
        Util.disableBtEm();
    }

    @Override
    public String toString() {
        return "BT TX";
    }

    @Override
    public String getTag() {
        return "7";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return FeatureSupport.isEmBTSupport();
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.backlightOn();
        Util.lcmOn();
        Util.switchBt(false);
        sleep(Util.TIME_FIVE_SEC);
    }
}

/**
 * TestMode.
 */
@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
class LiveWallpaperTest extends TestItem {
    private static final String LIVE_WALLPAPER_PKG = "com.android.wallpaper.holospiral";
    private static final String LIVE_WALLPAPER_LIVEPICKER_PKG = "com.android.wallpaper.livepicker";
    private static final String LIVE_WALLPAPER_CLS =
            "com.android.wallpaper.holospiral.HoloSpiralWallpaper";

    @Override
    protected int getDbSpec() {
        return 15;
    }

    @Override
    public void init() {
        Util.lcmOn();
        Util.switchAirplaneMode(mContext, true);
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        ComponentName comp = new ComponentName(LIVE_WALLPAPER_PKG, LIVE_WALLPAPER_CLS);
        Intent intent = new Intent(WallpaperManager.ACTION_CHANGE_LIVE_WALLPAPER);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(WallpaperManager.EXTRA_LIVE_WALLPAPER_COMPONENT, comp);
        mContext.startActivity(intent);
        sleep(Util.TIME_FIVE_SEC);
        startTestMode(TEST_STATE_OFF);
        stopTestMode(TEST_STATE_OFF);
        if (mTestMode.getmTriggerSource() == 0) {
            returnToTestView();
        } else {
            returnToTestView(LIVE_WALLPAPER_LIVEPICKER_PKG);
        }
    }

    @Override
    public String toString() {
        return "Live wallpaper";
    }

    @Override
    public String getTag() {
        return "8";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        ComponentName comp = new ComponentName(LIVE_WALLPAPER_PKG, LIVE_WALLPAPER_CLS);
        Intent preview = new Intent(WallpaperManager.ACTION_CHANGE_LIVE_WALLPAPER);
        preview.putExtra(WallpaperManager.EXTRA_LIVE_WALLPAPER_COMPONENT, comp);
        return (Util.isPkgInstalled(mContext, LIVE_WALLPAPER_PKG)
                && Util.isIntentAvailable(mContext, preview));

    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
    }
}

/**
 * TestMode.
 */
class Mp3PlayTest extends TestItem {
    private MediaPlayer mPlayer;

    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, true);
        Util.lcmOff();
        Util.backlightOff();
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        String filePath = Util.getTestFilePath(mContext, TEST_MP3_FILE);
        Elog.d(TAG, "filePath:" + filePath);
        File file = new File(filePath);
        mPlayer = MediaPlayer.create(mContext, Uri.fromFile(file));
        mPlayer.start();
        startTestMode(TEST_STATE_OFF);
        stopTestMode(TEST_STATE_OFF);
        if (mPlayer != null) {
            mPlayer.release();
            mPlayer = null;
        }
    }

    @Override
    public String toString() {
        return "MP3 playing";
    }

    @Override
    public String getTag() {
        return "9";
    }

    @SuppressLint("InlinedApi")
    @Override
    protected boolean doApiTest() {
        Intent intent = new Intent();
        intent.setAction(android.content.Intent.ACTION_VIEW);
        String filePath = Util.getTestFilePath(mContext, TEST_MP3_FILE);
        Elog.d(TAG, "filePath:" + filePath);
        return (Util.isFileExist(filePath) && (Util.isIntentAvailable(mContext, intent)));
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.backlightOn();
        Util.lcmOn();

    }
}

/**
 * TestMode.
 */
class VideoPlayTest extends TestItem {
    private ViewGroup fView;
    private SurfaceView mSurfaceView;
    private SurfaceHolder surfaceHolder;
    private MediaPlayer mPlayer;
    private WindowManager wm ;

    final Runnable runnable = new Runnable() {
        @Override
        public void run() {
            String filePath = Util.getTestFilePath(mContext, TEST_3GP_FILE);
            Elog.d(TAG, "filePath:" + filePath);
            File file = new File(filePath);
            mPlayer = MediaPlayer.create(mContext, Uri.fromFile(file));
            wm = (WindowManager) mContext.getSystemService("window");
            WindowManager.LayoutParams mParams = new WindowManager.LayoutParams();
            mParams.width = 600;
            mParams.height = 600;
            mParams.type = LayoutParams.TYPE_SYSTEM_OVERLAY;
            mParams.format = PixelFormat.TRANSPARENT;

            fView = (ViewGroup) View.inflate(mContext, R.layout.desense_at_surface_view, null);
            wm.addView(fView, mParams);
            mSurfaceView = (SurfaceView) fView.findViewById(R.id.myView);
            surfaceHolder = mSurfaceView.getHolder();
            surfaceHolder.addCallback(new SurfaceHolder.Callback() {
                @Override
                public void surfaceDestroyed(SurfaceHolder holder) {
                    Elog.d(TAG, "surfaceDestroyed");
                }
                @Override
                public void surfaceCreated(SurfaceHolder holder) {
                    try {
                        Elog.d(TAG, "mPlayer play");
                        mPlayer.setDisplay(holder);
                        mPlayer.start();
                    } catch (Exception e) {
                        Elog.d(TAG, e.getMessage());
                    }
                }
                @Override
                public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                    Elog.d(TAG, "surfaceChanged");
                }
            });
        }
    };
    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, true);
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                Looper.prepare();
                new Handler().post(runnable);
                Looper.loop();
            }
        }).start();
        sleep(Util.TIME_FIVE_SEC);
        startTestMode(TEST_STATE_OFF);
        stopTestMode(TEST_STATE_OFF);
        if (mPlayer != null) {
            mPlayer.release();
            wm.removeView(fView);
            mPlayer = null;
        }
    }

    @Override
    public String toString() {
        return "Video playing";
    }

    @Override
    public String getTag() {
        return "10";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        Intent intent = new Intent();
        intent.setAction(android.content.Intent.ACTION_VIEW);
        String filePath = Util.getTestFilePath(mContext, TEST_3GP_FILE);
        Elog.d(TAG, "filePath:" + filePath);
        return (Util.isFileExist(filePath) && Util.isIntentAvailable(mContext, intent));
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
    }
}

/**
 * TestMode.
 */
class CpuFullLoadingTest extends TestItem {
    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, true);
        Util.lcmOff();
        Util.backlightOff();
        sleep(Util.TIME_FIVE_SEC);
    }

    private Point getStartBtnPoint() {
        DisplayMetrics metrics;
        int height = 0;
        metrics = new DisplayMetrics();
        ((Activity) mContext).getWindowManager().getDefaultDisplay().getMetrics(metrics);
        height = metrics.heightPixels;
        Point point = new Point(100, (int) (0.61 * height) + 30);
        Elog.d(TAG, "cpu loading test, touch coordinate x: "
                + point.x + ", y: " + point.y + "(height:" + height + ")");
        return point;
    }

    @Override
    public void doTest() {
        Intent intent = new Intent();
        intent.setClassName("com.obkircherlukas.cpuprimebenchmark",
                "com.obkircherlukas.cpuprimebenchmark.MainActivity");
        try {
            ((Activity) mContext).startActivity(intent);
            sleep(Util.TIME_FIVE_SEC);
            Point startBtnPoint = getStartBtnPoint();
            Util.clickPoint(startBtnPoint, false);
            sleep(Util.TIME_ONE_SEC);
            startTestMode(TEST_STATE_OFF);
            stopTestMode(TEST_STATE_OFF);
            sleep(Util.TIME_TEN_SEC);
            returnToTestView();
        } catch (ActivityNotFoundException e) {
            Elog.d(TAG, "ActivityNotFoundException:" + e.getMessage());
        }
    }

    @Override
    public String toString() {
        return "CPU full loading";
    }

    @Override
    public String getTag() {
        return "-1";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        Intent intent = new Intent();
        intent.setClassName("com.obkircherlukas.cpuprimebenchmark",
                "com.obkircherlukas.cpuprimebenchmark.MainActivity");
        return (Util.isIntentAvailable(mContext, intent));
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.backlightOn();
        Util.lcmOn();
    }
}

/**
 * TestMode.
 */
class PhoneStorageReadTest extends TestItem {
    private SdTest mSdTest;

    @Override
    public void init() {
        mSdTest = new SdTest(mContext);
        mSdTest.doTest(SdTest.TYPE_PREPARE, SdTest.STORAGE_INTERNAL, this);
        Util.switchAirplaneMode(mContext, true);
        Util.lcmOff();
        Util.backlightOff();
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        startTestMode(TEST_STATE_OFF);
        readFiles();
        stopTestMode(TEST_STATE_OFF);
    }

    private void readFiles() {
        long start = System.currentTimeMillis();
        while (System.currentTimeMillis() - start < 30 * 1000 && !isCancelled() && !isTestDone()) {
            mSdTest.doTest(SdTest.TYPE_READ, SdTest.STORAGE_INTERNAL, this);
        }
    }

    @Override
    public String toString() {
        return "Phone storage reading";
    }

    @Override
    public String getTag() {
        return "11";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        SdTest sdTest = new SdTest(mContext);
        return sdTest.doApiTest(SdTest.STORAGE_INTERNAL);
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.backlightOn();
        Util.lcmOn();
    }
}

/**
 * TestMode.
 */
class PhoneStorageWriteTest extends TestItem {
    private SdTest mSdTest;

    @Override
    public void init() {
        mSdTest = new SdTest(mContext);
        mSdTest.doTest(SdTest.TYPE_PREPARE, SdTest.STORAGE_INTERNAL, this);
        Util.switchAirplaneMode(mContext, true);
        Util.lcmOff();
        Util.backlightOff();
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        startTestMode(TEST_STATE_OFF);
        writeFiles();
        stopTestMode(TEST_STATE_OFF);
    }

    private void writeFiles() {
        long start = System.currentTimeMillis();
        while (System.currentTimeMillis() - start < 30 * 1000 && !isCancelled() && !isTestDone()) {
            mSdTest.doTest(SdTest.TYPE_WRITE, SdTest.STORAGE_INTERNAL, this);
        }
    }

    @Override
    public String toString() {
        return "Phone storage writing";
    }

    @Override
    public String getTag() {
        return "12";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        SdTest sdTest = new SdTest(mContext);
        return sdTest.doApiTest(SdTest.STORAGE_INTERNAL);
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.backlightOn();
        Util.lcmOn();
    }
}

/**
 * TestMode.
 */
class ExternalSdCardReadTest extends TestItem {
    private SdTest mSdTest;

    @Override
    public void init() {
        mSdTest = new SdTest(mContext);
        mSdTest.doTest(SdTest.TYPE_PREPARE, SdTest.STORAGE_SDCARD, this);
        Util.switchAirplaneMode(mContext, true);
        Util.lcmOff();
        Util.backlightOff();
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        startTestMode(TEST_STATE_OFF);
        readFiles();
        stopTestMode(TEST_STATE_OFF);
    }

    private void readFiles() {
        long start = System.currentTimeMillis();
        while (System.currentTimeMillis() - start < 30 * 1000 && !isCancelled()) {
            mSdTest.doTest(SdTest.TYPE_READ, SdTest.STORAGE_SDCARD, this);
        }
    }

    @Override
    public String toString() {
        return "External SD card reading";
    }

    @Override
    public String getTag() {
        return "13";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        SdTest sdTest = new SdTest(mContext);
        return sdTest.doApiTest(SdTest.STORAGE_SDCARD);
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.backlightOn();
        Util.lcmOn();
    }
}

/**
 * TestMode.
 */
class ExternalSdCardWriteTest extends TestItem {
    private SdTest mSdTest;

    @Override
    public void init() {
        mSdTest = new SdTest(mContext);
        mSdTest.doTest(SdTest.TYPE_PREPARE, SdTest.STORAGE_SDCARD, this);
        Util.switchAirplaneMode(mContext, true);
        Util.lcmOff();
        Util.backlightOff();
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        startTestMode(TEST_STATE_OFF);
        writeFiles();
        stopTestMode(TEST_STATE_OFF);

    }

    private void writeFiles() {
        long start = System.currentTimeMillis();
        while (System.currentTimeMillis() - start < 30 * 1000 && !isCancelled()) {
            mSdTest.doTest(SdTest.TYPE_WRITE, SdTest.STORAGE_SDCARD, this);
        }
    }

    @Override
    public String toString() {
        return "External SD card writing";
    }

    @Override
    public String getTag() {
        return "14";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        SdTest sdTest = new SdTest(mContext);
        return sdTest.doApiTest(SdTest.STORAGE_SDCARD);
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.backlightOn();
        Util.lcmOn();
    }
}

/**
 * TestMode.
 */
class MemoryAccessTest extends TestItem {
    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, true);
        Util.lcmOff();
        Util.backlightOff();
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        startTestMode(TEST_STATE_OFF);
        Random rand = new Random();
        int[] test = new int[100000];
        long start = System.currentTimeMillis();
        while (System.currentTimeMillis() - start < 30 * 1000 && !isCancelled() && !isTestDone()) {
            test[rand.nextInt(100000)] = rand.nextInt();
        }
        stopTestMode(TEST_STATE_OFF);

    }

    @Override
    public String toString() {
        return "Memory accessing";
    }

    @Override
    public String getTag() {
        return "15";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return true;
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.backlightOn();
        Util.lcmOn();
    }
}

/**
 * TestMode.
 */
class FrontCameraPreviewTest extends TestItem {
    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, true);
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        Intent intent = new Intent(mContext, CameraTest.class);
        intent.putExtra(CameraTest.FACING_TAG, true);
        mContext.startActivity(intent);
        startTestMode(TEST_STATE_OFF);
        stopTestMode(TEST_STATE_OFF);
        returnToTestView();
    }

    @Override
    public String toString() {
        return "Front camera preview";
    }

    @Override
    public String getTag() {
        return "16";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return CameraTest.isCameraSupport(true);
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
    }
}

/**
 * TestMode.
 */
class BackCameraPreviewTest extends TestItem {
    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, true);
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        Intent intent = new Intent(mContext, CameraTest.class);
        intent.putExtra(CameraTest.FACING_TAG, false);
        mContext.startActivity(intent);
        startTestMode(TEST_STATE_OFF);
        stopTestMode(TEST_STATE_OFF);
        CameraTest.sCameraIns.finish();
        // returnToTestView();
    }

    @Override
    public String toString() {
        return "Back camera preview";
    }

    @Override
    public String getTag() {
        return "17";
    }

    @Override
    protected boolean doApiTest() {
        return CameraTest.isCameraSupport(false);
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
    }
}

/**
 * TestMode.
 */
class Display3DTest extends TestItem {
    private Point mBtnRunPoint = new Point(295, 478);
    private final String PKG_NAME = "se.nena.nenamark2";
    private final String CLASS_NAME = "se.nena.nenamark2.NenaMark2";

    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, true);
        Util.lcmOn();
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        Intent intent = new Intent();
        intent.setClassName(PKG_NAME, CLASS_NAME);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        try {
            mContext.startActivity(intent);
            sleep(Util.TIME_ONE_SEC);
            Util.clickPoint(mBtnRunPoint, true);
            sleep(Util.TIME_ONE_SEC);
            startTestMode(TEST_STATE_OFF);
            stopTestMode(TEST_STATE_OFF);
            if (mTestMode.getmTriggerSource() == 0) {
                returnToTestView();
            } else {
                returnToTestView(PKG_NAME);
            }

        } catch (ActivityNotFoundException e) {
            Elog.d(TAG, "ActivityNotFoundException:" + e.getMessage());
        }

    }

    @Override
    public String toString() {
        return "3D display";
    }

    @Override
    public String getTag() {
        return "0";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub

        Intent intent = new Intent();
        intent.setClassName(PKG_NAME, CLASS_NAME);
        return (Util.isIntentAvailable(mContext, intent));
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
    }
}


/**
 * Virtual class to indicate all items for this modem category.
 */
class BandVirtualTest extends TestItem {

    @Override
    public String toString() {
        return null;
    }

    @Override
    public String getTag() {
        return null;
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return true;
    }

    @Override
    protected void init() {
        // TODO Auto-generated method stub
    }

    @Override
    protected void doTest() {
        // TODO Auto-generated method stub

    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
    }
}

/**
 * Virtual class to indicate all items for GSM band.
 */
class GsmBandVirtualTest extends BandVirtualTest {
    public static final String ITEM_TAG = "GsmBandTest";

    @Override
    public String toString() {
        return ITEM_TAG;
    }

    @Override
    public String getTag() {
        return "-1";
    }
}

/**
 * Virtual class to indicate all items for FDD band.
 */
class FddBandVirtualTest extends BandVirtualTest {
    public static final String ITEM_TAG = "FddBandTest";

    @Override
    public String toString() {
        return ITEM_TAG;
    }

    @Override
    public String getTag() {
        return "-1";
    }
}

/**
 * Virtual class to indicate all items for TDD band.
 */
class TddBandVirtualTest extends BandVirtualTest {
    public static final String ITEM_TAG = "TddBandTest";

    @Override
    public String toString() {
        return ITEM_TAG;
    }

    @Override
    public String getTag() {
        return "-1";
    }
}

/**
 * Virtual class to indicate all items for LTE band.
 */
class LteBandVirtualTest extends BandVirtualTest {
    public static final String ITEM_TAG = "LteBandTest";

    @Override
    public String toString() {
        return ITEM_TAG;
    }

    @Override
    public String getTag() {
        return "-1";
    }
}

/**
 * Virtual class to indicate all items for CDMA band.
 */
class CdmaBandVirtualTest extends BandVirtualTest {
    public static final String ITEM_TAG = "CdmaBandTest";

    @Override
    public String toString() {
        return ITEM_TAG;
    }

    @Override
    public String getTag() {
        return "-1";
    }
}

/**
 * Class for modem related test.
 */
class BandTest extends TestItem {
    public static final String ITEM_TAG_PRE = "(";
    public static final String ITEM_TAG_POST = ")";
    public static final String ITEM_TAG_NA = "NA";
    protected static final String CHANNEL = "CH:";
    protected static final String POWER = "PWR:";
    public static boolean sAtcDone = true;
    protected int mChannel;
    protected int mPower;
    protected int mBandValue;
    protected String mBandName;

    BandTest(String name, int channel, int band, int power, Context context) {
        mBandName = name;
        mChannel = channel;
        mBandValue = band;
        mPower = power;
        setContext(context);
    }

    protected void startDesense() {
    }

    protected void pauseDesense() {
    }

    @Override
    public void init() {
        Util.switchAirplaneMode(mContext, true);
        Util.lcmOff();
        Util.backlightOff();
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void doTest() {
        synchronized (DesenseAtActivity.class) {
            BandTest.sAtcDone = false;
        }
        startDesense();
        while (true) {
            synchronized (DesenseAtActivity.class) {
                sleep(Util.TIME_TEN_SEC);
                if (BandTest.sAtcDone) {
                    Elog.d(TAG, "start BandTest.sAtcDone is :" + BandTest.sAtcDone);
                    break;
                } else{
                    Elog.d(TAG, "start BandTest.sAtcDone is :" + BandTest.sAtcDone);
                    break;
                }
            }
        }
        startTestMode(TEST_STATE_OFF);
        stopTestMode(TEST_STATE_OFF);
        pauseDesense();
        while (true) {
            synchronized (DesenseAtActivity.class) {
                sleep(Util.TIME_TEN_SEC);
                if (BandTest.sAtcDone) {
                    Elog.d(TAG, "stop BandTest.sAtcDone is :" + BandTest.sAtcDone);
                    break;
                } else{
                    Elog.d(TAG, "stop BandTest.sAtcDone is :" + BandTest.sAtcDone);
                    break;
                }
            }
        }
    }

    @Override
    public String toString() {
        return null;
    }

    @Override
    public String getTag() {
        return "-1";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return true;
    }

    @Override
    protected void reset() {
        // TODO Auto-generated method stub
        Util.backlightOn();
        Util.lcmOn();

    }
}

/**
 * Class for GSM band related test.
 */
class GsmBandTest extends BandTest {

    public static final String ITEM_TAG_PRE = "Gsm(";
    private static final int AFC = 4100;
    private static final int TSC = 0;
    private static final int PATTERN = 0;

    GsmBandTest(String name, int channel, int band, int power, Context context) {
        super(name, channel, band, power, context);
    }

    protected void startDesense() {
        String command = "AT+ERFTX=2,1," + mChannel + "," + AFC + "," + mBandValue + ","
                + TSC + "," + mPower + "," + PATTERN;
        Util.sendAtCommand(mContext, command, false, 0);
    }

    protected void pauseDesense() {
        Util.sendAtCommand(mContext, "AT+ERFTX=2,0", false, 0);
    }

    @Override
    public String toString() {
        return mBandName + "_" + ITEM_TAG_PRE + CHANNEL + ((mChannel != -1) ? mChannel :
                ITEM_TAG_NA)
                + POWER + ((mPower != -1) ? mPower : ITEM_TAG_NA) + ITEM_TAG_POST;
    }

    @Override
    public String getTag() {
        return "18";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return true;
    }
}

/**
 * Class for FDD/TDD band related test.
 */
class FddTddBandTest extends BandTest {

    public static final String ITEM_TAG = "FddTddBandTest";

    FddTddBandTest(String name, int channel, int band, int power, Context context) {
        super(name, channel, band, power, context);
    }

    protected void startDesense() {
        String command = "AT+ERFTX=0,0," + mBandValue + "," + mChannel + "," + mPower;

        Util.sendAtCommand(mContext, command, false, 0);
    }

    protected void pauseDesense() {
        Util.sendAtCommand(mContext, "AT+ERFTX=0,1", false, 0);
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return true;
    }
}

/**
 * Class for FDD band related test.
 */
class FddBandTest extends FddTddBandTest {

    FddBandTest(String name, int channel, int band, int power, Context context) {
        super(name, channel, band, power, context);
    }

    @Override
    public String toString() {
        return mBandName + "_" + ITEM_TAG_PRE + CHANNEL + ((mChannel != -1) ? mChannel :
                ITEM_TAG_NA)
                + POWER + ((mPower != -1) ? mPower : ITEM_TAG_NA) + ITEM_TAG_POST;
    }

    @Override
    public String getTag() {
        return "19";
    }

}

/**
 * Class for TDD band related test.
 */
class TddBandTest extends FddTddBandTest {

    TddBandTest(String name, int channel, int band, int power, Context context) {
        super(name, channel, band, power, context);
    }

    @Override
    public String toString() {
        return mBandName + "_" + ITEM_TAG_PRE + CHANNEL + ((mChannel != -1) ? mChannel :
                ITEM_TAG_NA)
                + POWER + ((mPower != -1) ? mPower : ITEM_TAG_NA) + ITEM_TAG_POST;
    }

    @Override
    public String getTag() {
        return "19";
    }
}

/**
 * Class for LTE  band related test.
 */
class LteBandTest extends BandTest {


    public static final String UL_FREQ = "UL Freq";
    private static final int DUPLEX_TDD = 0;
    private static final int FDD_BAND_MIN = 1;
    private static final int TDD_BAND_MIN = 33;
    private static final int DUPLEX_FDD = 1;
    private static final int BAND_WIDTH_INDEX = 2;
    private static final int TDD_CONFIG_INDEX = 0;
    private static final int TDD_SP_SF_CONFIG_INDEX = 0;
    private static final String DEFAULT_VRB_START = "0";
    private static final String DEFAULT_VRB_LENGTH = "1";
    private static final int DEFAULT_MCS = 0;
    private boolean mFDD;

    LteBandTest(String name, int channel, int band, int power, boolean bFdd, Context context) {
        super(name, channel, band, power, context);
        mFDD = bFdd;
    }

    protected void startDesense() {
        String command = "AT+ERFTX=6,0,1,"
                + (mBandValue + (mFDD ? FDD_BAND_MIN : TDD_BAND_MIN)) + ","
                + BAND_WIDTH_INDEX + ","
                + mChannel + ","
                + (mFDD ? DUPLEX_FDD : DUPLEX_TDD) + ","
                + TDD_CONFIG_INDEX + ","
                + TDD_SP_SF_CONFIG_INDEX + ","
                + DEFAULT_VRB_START + ","
                + DEFAULT_VRB_LENGTH + ","
                + DEFAULT_MCS + ","
                + mPower;
        Util.sendAtCommand(mContext, command, false, 0);
    }

    protected void pauseDesense() {
        Util.sendAtCommand(mContext, "AT+ERFTX=6,0,0", false, 0);
    }

    @Override
    public String toString() {
        return mBandName + "_" + ITEM_TAG_PRE + UL_FREQ + ((mChannel != -1) ? mChannel :
                ITEM_TAG_NA)
                + POWER + ((mPower != -1) ? mPower : ITEM_TAG_NA) + ITEM_TAG_POST;
    }

    @Override
    public String getTag() {
        return "20";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return true;
    }

}

/**
 * Class for CDMA band related test.
 */
class CdmaBandTest extends BandTest {
    private static final int MODULATION = 0;

    CdmaBandTest(String name, int channel, int band, int power, Context context) {
        super(name, channel, band, power, context);
    }

    protected void startDesense() {
        String command = "AT+ECRFTX=1," + mChannel + "," + mBandValue + "," + (mPower + 60) + ","
                + MODULATION;

        Util.sendAtCommand(mContext, command, true, 0);
    }

    protected void pauseDesense() {
        Util.sendAtCommand(mContext, "AT+ECRFTX=0", true, 0);
    }

    @Override
    public String toString() {
        return mBandName + "_" + ITEM_TAG_PRE + CHANNEL + ((mChannel != -1) ? mChannel :
                ITEM_TAG_NA)
                + POWER + ((mPower != -1) ? mPower : ITEM_TAG_NA) + ITEM_TAG_POST;
    }

    @Override
    public String getTag() {
        return "21";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return true;
    }
}
