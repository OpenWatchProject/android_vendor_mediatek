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

package com.mediatek.engineermode.clkqualityat;


import android.app.ActivityManager;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.graphics.Point;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.wifi.EMWifi;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;


/**
 * Test items.
 */
public abstract class TestItem {
    protected static final String TAG = "ClkQualityAt/TestItem";
    private static ArrayList<TestItem> sTestItems = null;
    protected Context mContext;
    protected TestMode mTestMode;
    private TestResult mTestModeResult;
    private boolean mTimeOut = false;
    private boolean isOnGoingTestMode = true;
    private Timer tmr;
    private TimerTask tmrTask;
    private String tag;
    public String getTag() {
        return tag;
    }

    public void setTag(String tag) {
        this.tag = tag;
    }
    protected static final int TEST_STATE_ON = 0;
    protected static final int TEST_STATE_OFF = 1;
    protected static final int BAND_NAME_INDEX = 0;
    protected static final int BAND_VALUE_INDEX = 1;
    protected static final int CHANNEL_DEFAULT_INDEX = 2;
    protected static final int POWER_DEFAULT_INDEX = 3;
    protected static final int SUPPORT_BIT_INDEX = 4;

    public static final int INDEX_GSM_BAND = 0;
    public static final int INDEX_UMTS_BAND = 1;
    public static final int INDEX_LTE_FDD_BAND = 2;
    public static final int INDEX_LTE_TDD_BAND = 3;
    public static final int INDEX_CDMA_BAND = 4;
    public static final int INDEX_BAND_MAX = 5;

    public static List<String> sItemsToCheck;
    private int mItemIndex;
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

    static ArrayList<TestItem> getTestItems(Context context, long[] bandSupported) {
        if (sTestItems == null) {
            sTestItems = new ArrayList<TestItem>();
            sTestItems.add(new EmptyTest());
            sTestItems.add(new WifiTxTest());
            sTestItems.add(new Display3DTest());
            Elog.w(TAG, "getTestItems");
            if (bandSupported.length < INDEX_BAND_MAX) {
                Elog.w(TAG, "get band supported info is not enough, @" + bandSupported.length);
            } else {
                initRfConfigList(context, bandSupported);
            }
        }

        for(int i = 0 ;i < sTestItems.size(); i++){
            TestItem item = sTestItems.get(i);
            item.setContext(context);
            item.setTag(i+"");
        }
        return sTestItems;
    }

    static private void initRfConfigList(Context context, long[] bandSupported) {

        initGsmItems(context, bandSupported[INDEX_GSM_BAND]);

        if (ModemCategory.getModemType() == ModemCategory.MODEM_TD) {

            initTddItems(context, bandSupported[INDEX_UMTS_BAND]);

        } else if (ModemCategory.getModemType() == ModemCategory.MODEM_FDD) {

            initFddItems(context, bandSupported[INDEX_UMTS_BAND]);

        }
        if (ModemCategory.isLteSupport()) {

            initLteItems(context, bandSupported[INDEX_LTE_FDD_BAND], bandSupported[INDEX_LTE_TDD_BAND]);

        }
        if (ModemCategory.isCdma()) {

            initCdmaItems(context, bandSupported[INDEX_CDMA_BAND]);

        }
    }

    static private void initGsmItems(Context context, long bitsSupported) {

        String[] bandDefault = context.getResources().getStringArray(
                R.array.clk_rf_desense_gsm_data);
        Elog.i(TAG, "initGsmItems bitsSupported: " + bitsSupported);
        int bandAarayLength = bandDefault.length;

        for (int k = 0; k < bandAarayLength; k++) {

            if (bandDefault[k] == null) {
                continue;
            }

            String[] values = bandDefault[k].split(",");

            if (!isSupportedBand(bitsSupported, Integer.parseInt(values[SUPPORT_BIT_INDEX])))
                continue;
           sTestItems.add(new GsmBandTest(
                    values[BAND_NAME_INDEX], Integer.parseInt(values[BAND_VALUE_INDEX]),
                    Integer.parseInt(values[CHANNEL_DEFAULT_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX])));
        }
    }

    static private void initTddItems(Context context, long bitsSupported) {
        String[] bandDefault = context.getResources().getStringArray(
                R.array.clk_rf_desense_td_scdma_data);
        Elog.i(TAG, "initTddItems bitsSupported: " + bitsSupported);
        int bandAarayLength = bandDefault.length;

        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsSupported, Integer.parseInt(values[SUPPORT_BIT_INDEX])))
                continue;
            sTestItems.add(new TddBandTest(
                    values[BAND_NAME_INDEX], Integer.parseInt(values[BAND_VALUE_INDEX]),
                    Integer.parseInt(values[CHANNEL_DEFAULT_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX])));
        }
    }

    static private void initFddItems(Context context, long bitsSupported) {
        String[] bandDefault = context.getResources().getStringArray(
                R.array.clk_rf_desense_wcdma_data);
        Elog.i(TAG, "initFddItems bitsSupported: " + bitsSupported);
        int bandAarayLength = bandDefault.length;

        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsSupported, Integer.parseInt(values[SUPPORT_BIT_INDEX])))
                continue;
            sTestItems.add(new FddBandTest(
                    values[BAND_NAME_INDEX], Integer.parseInt(values[BAND_VALUE_INDEX]),
                    Integer.parseInt(values[CHANNEL_DEFAULT_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX])));
        }
    }

    static private void initLteItems(Context context, long bitsLSupported, long bitsHSupported) {
        String[] bandDefault = context.getResources().getStringArray(
                R.array.clk_rf_desense_lte_fdd_data);
        Elog.i(TAG, "initGsmItems bitsLSupported: " + bitsLSupported);
        int bandAarayLength = bandDefault.length;

        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsLSupported, Integer.parseInt(values[SUPPORT_BIT_INDEX])))
                continue;
            sTestItems.add(new LteBandTest(
                    values[BAND_NAME_INDEX], Integer.parseInt(values[BAND_VALUE_INDEX]),
                    Integer.parseInt(values[CHANNEL_DEFAULT_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX]), true));
        }

        bandDefault = context.getResources().getStringArray(
                R.array.clk_rf_desense_lte_tdd_data);
        Elog.i(TAG, "initGsmItems bitsHSupported: " + bitsHSupported);
        bandAarayLength = bandDefault.length;

        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsHSupported, Integer.parseInt(values[SUPPORT_BIT_INDEX])))
                continue;
            sTestItems.add(new LteBandTest(
                    values[BAND_NAME_INDEX], Integer.parseInt(values[BAND_VALUE_INDEX]),
                    Integer.parseInt(values[CHANNEL_DEFAULT_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX]), false));
        }
    }

    static private void initCdmaItems(Context context, long bitsSupported) {
        String[] bandDefault = context.getResources().getStringArray(
                R.array.clk_rf_desense_cdma_data);
        Elog.i(TAG, "initCdmaItems bitsSupported: " + bitsSupported);
        int bandAarayLength = bandDefault.length;

        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsSupported, Integer.parseInt(values[SUPPORT_BIT_INDEX])))
                continue;
            sTestItems.add(new CdmaBandTest(
                    values[BAND_NAME_INDEX], Integer.parseInt(values[BAND_VALUE_INDEX]),
                    Integer.parseInt(values[CHANNEL_DEFAULT_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX])));
        }
    }

    static private boolean isSupportedBand(long bits, int idx) {
        if ((bits & (1L << idx)) == 0)
            return false;
        return true;
    }

    abstract protected void init();

    abstract protected void openTest();

    abstract protected void pauseTest();

    abstract protected void closeTest();

    abstract protected boolean doApiTest();

    public boolean isCancelled() {
        return ClkQualityAtActivity.getCancelled();
    }

    void sleep(long time) {
        // TODO: not exactly
        if (!isCancelled()) {
            try {
                Thread.sleep(time);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    TestResult run() {
        // Pass 1
        init();
        if (isCancelled()) {
            Elog.d(TAG, "return null");
            pauseTest();
            return null;
       }
        Elog.d(TAG, "startTestMode: heat!");

        openTest();
        if(BandTest.sAtcDone == false){
            mTestModeResult = new TestResult();
            mTestModeResult.result = TestResult.MODEM_FAIL;
            return mTestModeResult;
        }
        startTestMode(TEST_STATE_ON);

        timerAlert(mTestMode.getHeatTestTime()*1000+2000);
        while (isOnGoingTestMode) {
            sleep(100);
            if (mTimeOut) {
                Elog.w(TAG, "heat test mode time out");
                mTimeOut = false;
                break;
            }
        }
        cancelTimer();
        // Pass 2
        if (isCancelled()) {
            Elog.d(TAG, "return null");
            pauseTest();
            return null;
        }
        Elog.d(TAG, "startTestMode: cool!");
        startTestMode(TEST_STATE_OFF);
        pauseTest();
        timerAlert(mTestMode.getCoolTestTime()*1000+2000);
        while (isOnGoingTestMode) {
            sleep(100);
            if (mTimeOut) {
                Elog.w(TAG, "cool test mode time out");
                mTimeOut = false;
                break;
            }
        }
        cancelTimer();
        closeTest();
        Elog.d(TAG, "startTestMode: finish!");
        if (mTestModeResult.result != TestResult.FAIL
              && mTestModeResult.result != TestResult.PASS
              && mTestModeResult.result != TestResult.SERIOUS
              && mTestModeResult.result != TestResult.VENIAL) {
            mTestModeResult.result = TestResult.CONNECTION_FAIL;
        }
        return mTestModeResult;
    }

    void setContext(Context context) {
        mContext = context;
    }

    void setTestMode(TestMode mode) {
        mTestMode = mode;
    }

    protected boolean isEmptyCase() {
        return false;
    }

    protected void startTestMode(int testState) {
        synchronized (TestItem.this) {
            isOnGoingTestMode = true;
        }
        mTestMode.setParameters(mItemIndex, testState, isEmptyCase());
        if (testState == TEST_STATE_ON)
            mTestModeResult = null;
        new Thread() {
            public void run() {
                mTestMode.setTag(tag);
                mTestModeResult = mTestMode.run();
                synchronized (TestItem.this) {
                    isOnGoingTestMode = false;
                }
            }
        }.start();
    }

    public void timerAlert(int ms) {
        mTimeOut = false;
        tmr = new Timer();
        tmrTask = new TimerTask() {
            public void run() {
                mTimeOut = true;
            }
        };
        tmr.schedule(tmrTask, ms);
    }

    public void cancelTimer() {
        if (tmr == null || tmrTask == null)
            return ;
        tmr.cancel();
        tmrTask.cancel();
        tmr.purge();
        tmr = null;
        tmrTask = null;
    }
}

/**
 * Empty test item.
 */
class EmptyTest extends TestItem {

    @Override
    public void init() {

    }

    @Override
    public void openTest() {

    }

    @Override
    public void pauseTest() {

    }

    @Override
    public String toString() {
        return "Static";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return true;
    }

    protected boolean isEmptyCase() {
        return true;
    }

    @Override
    protected void closeTest() {
        // TODO Auto-generated method stub
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
            { 0xff220004, 0x33440006, 0x55660008, 0x55550019, 0xaaaa001b, 0xbbbb001d };
    private static final int COMMAND_INDEX_STARTTX = 1;
    private static final int COMMAND_INDEX_STOPTEST = 0;
    public static final String NAME = "WIFI TX";
    private static final String CHIP_6632 = "6632";
    private static final String CHIP_ADV = "0000";

    @Override
    public void init() {
        Util.switchWifi(mContext, false);
        sleep(Util.TIME_FIVE_SEC);
    }

    @Override
    public void openTest() {
        Util.enableWifi(mContext);
        if (CHIP_6632.equals(Util.getWifiChipId())
                || CHIP_ADV.equals(Util.getWifiChipId())) {
            Elog.d(TAG, "doTx6632");
            doTx6632(19, 0, 2412000); // 19dBm, 1M, Channel 1

        } else {
            Elog.d(TAG, "doTx6620");
            doTx6620(19, 0, 2412000); // 19dBm, 1M, Channel 1

        }
//        doTx(16, 0, 2437000); // 19dBm, 1M, Channel 6
//        doTx(16, 0, 2462000); // 19dBm, 1M, Channel 11
//        doTx(16, 11, 5500000); // 16dBm, 54M, Channel 100
    }

    private void doTx6620(int power, int rateIndex, int channel) {
        if (isCancelled()) {
            return;
        }

        // From WiFiTx6620.java
        // Mode = continuous packet tx
        power *= 2;
        EMWifi.setATParam(ATPARAM_INDEX_POWER, power);        // Tx power = 19
        EMWifi.setATParam(ATPARAM_INDEX_PREAMBLE, 0);        // Preamble = Normal
        EMWifi.setATParam(ATPARAM_INDEX_ANTENNA, ANTENNA);
        EMWifi.setATParam(ATPARAM_INDEX_PACKLENGTH, 1024);    // Pkt length = 1024
        EMWifi.setATParam(ATPARAM_INDEX_PACKCOUNT, 0);        // Pkt cnt = 0
        EMWifi.setATParam(ATPARAM_INDEX_PACKINTERVAL, 20);
        EMWifi.setATParam(ATPARAM_INDEX_TEMP_COMPENSATION, 0);
        EMWifi.setATParam(ATPARAM_INDEX_TXOP_LIMIT, TXOP_LIMIT_VALUE);
        for (int i = 0; i < PACKCONTENT_BUFFER.length; i++) {
            EMWifi.setATParam(ATPARAM_INDEX_PACKCONTENT, PACKCONTENT_BUFFER[i]);
        }
        EMWifi.setATParam(ATPARAM_INDEX_RETRY_LIMIT, 1);
        EMWifi.setATParam(ATPARAM_INDEX_QOS_QUEUE, 2);
        EMWifi.setATParam(ATPARAM_INDEX_GI, 0);            // Guard interval = 800 ns
        EMWifi.setATParam(ATPARAM_INDEX_BANDWIDTH, 0);  // Bandwidth = 20M
        EMWifi.setATParam(ATPARAM_INDEX_RATE, rateIndex);    // Rate = 1M
        EMWifi.setChannel(channel);
        int ret = EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STARTTX);
        Elog.d(TAG, "start tx ret " + ret);
        sleep(10 * 1000); // TODO: is it OK?
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
        EMWifi.setATParam(ATPARAM_INDEX_PACKCOUNT, 0);
        EMWifi.setATParam(ATPARAM_INDEX_POWER_UNIT, 0);
        power *= 2;
        EMWifi.setATParam(ATPARAM_INDEX_POWER, power);
        EMWifi.setATParam(ATPARAM_INDEX_PREAMBLE, 0);
        EMWifi.setATParam(ATPARAM_INDEX_GI, 0);
        int ret = EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STARTTX);
        Elog.d(TAG, "start tx ret " + ret);
        sleep(10 * 1000); // TODO: is it OK?
        ret = EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STOPTEST);
        Elog.d(TAG, "stop tx ret " + ret);

    }
    @Override
    public String toString() {
        return NAME;
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return FeatureSupport.isWifiSupport(mContext);
    }

    @Override
    protected void pauseTest() {
        Util.disableWifi(mContext);
    }

    @Override
    protected void closeTest() {
        // TODO Auto-generated method stub
    }
}


/**
 * 3D play test item.
 *
 */
class Display3DTest extends TestItem {
    private Point mBtnRunPoint = new Point(295, 478);
    private final String PKG_NAME = "se.nena.nenamark2";
    private final String CLASS_NAME = "se.nena.nenamark2.NenaMark2";

    @Override
    public void init() {

        Intent intent = new Intent();
        intent.setClassName(PKG_NAME, CLASS_NAME);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        try {
            mContext.startActivity(intent);
        } catch (ActivityNotFoundException e) {
            Elog.d(TAG, "ActivityNotFoundException:" + e.getMessage());
        }
        sleep(1000);
    }
    protected void returnToTestView(String packetname) {
        Elog.d(TAG, "killing app packet name = " + packetname);
        try {
            ActivityManager mActivityManager = (ActivityManager) mContext.
                    getSystemService(Context.ACTIVITY_SERVICE);
            Method method = Class.forName("android.app.ActivityManager").
                    getMethod("forceStopPackage", String.class);
            method.invoke(mActivityManager, packetname);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    protected void returnToTestView() {
        do {
            Util.pressBack();
            sleep(Util.TIME_THREE_SEC);
        } while (!Util.isTopActivity(mContext, mContext.getClass().getName()));

    }
    @Override
    public void openTest() {
        Util.clickPoint(mBtnRunPoint, true);
        sleep(1000);
    }

    @Override
    protected void pauseTest() {
        Elog.d(TAG, "mTestMode triggersource" + mTestMode.getmTriggerSource());
        if (mTestMode.getmTriggerSource() == 0) {
            returnToTestView();
        } else {
            returnToTestView(PKG_NAME);
        }
    }

    @Override
    public String toString() {
        return "Play 3D";
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        Intent intent = new Intent();
        intent.setClassName(PKG_NAME, CLASS_NAME);
        return (Util.isIntentAvailable(mContext, intent));
    }

    @Override
    protected void closeTest() {

    }
}

/**
 * Class for modem related test.
 *
 */
class BandTest extends TestItem {
    protected int mChannel;
    protected int mPower;
    protected int mBandValue;
    protected String mBandName;
    protected static final String CHANNEL = "CH:";
    protected static final String POWER = "PWR:";
    public static final String ITEM_TAG_PRE = "(";
    public static final String ITEM_TAG_POST = ")";
    public static final String ITEM_TAG_NA = "NA";
    public static volatile boolean sAtcDone = true;

    protected void startDesense() {

    }

    protected void pauseDesense() {

    }

    BandTest(String name, int band, int channel, int power) {
        mBandName = name;
        mBandValue = band;
        mChannel = channel;
        mPower = power;
    }

    @Override
    public void init() {
    }

    @Override
    public void openTest() {
        BandTest.sAtcDone = false;
        startDesense();
        while (true) {
            sleep(1000);
            if (BandTest.sAtcDone) {
                Elog.d(TAG, "@BandTest,openTest, BandTest.sAtcDone: true");
                break;
            } else{
                Elog.d(TAG, "@BandTest,openTest, BandTest.sAtcDone: false");
                break;
            }

        }
    }

    @Override
    protected void pauseTest() {
        BandTest.sAtcDone = false;
        pauseDesense();
        while (true) {
            sleep(1000);
            if (BandTest.sAtcDone) {
                Elog.d(TAG, "@BandTest,pauseTest, BandTest.sAtcDone: true");
                break;
            } else{
                Elog.d(TAG, "@BandTest,openTest, BandTest.sAtcDone: false");
                break;
            }
        }
    }

    @Override
    protected void closeTest() {
    }

    @Override
    public String toString() {
        return mBandName;
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return true;
    }
}

/**
 * Class for GSM band related test.
 *
 */
class GsmBandTest extends BandTest {

    private static final int AFC = 4100;
    private static final int TSC = 0;
    private static final int PATTERN = 0;
    public static final String ITEM_TAG_PRE = "Gsm(";
    protected void startDesense() {
        String command = "AT+ERFTX=2,1," + mChannel + "," + AFC + "," + mBandValue + ","
                + TSC + "," + mPower + "," + PATTERN;
        Util.sendAtCommand(mContext, command, false,
                ClkQualityAtActivity.AtcMsg.START_GSM.getValue());
    }

    protected void pauseDesense() {
        Util.sendAtCommand(mContext, "AT+ERFTX=2,0", false,
                ClkQualityAtActivity.AtcMsg.PAUSE_GSM.getValue());
    }

    GsmBandTest(String name, int band, int channel, int power) {
        super(name, band, channel, power);
    }

}

/**
 * Class for FDD/TDD band related test.
 *
 */
class FddTddBandTest extends BandTest {

    public static final String ITEM_TAG = "FddTddBandTest";
    protected void startDesense() {
        String command = "AT+ERFTX=0,0," + mBandValue + "," + mChannel + "," + mPower;

        Util.sendAtCommand(mContext, command, false,
                ClkQualityAtActivity.AtcMsg.START_FDDTDD.getValue());
    }

    protected void pauseDesense() {
        Util.sendAtCommand(mContext, "AT+ERFTX=0,1", false,
                ClkQualityAtActivity.AtcMsg.PAUSE_FDDTDD.getValue());
    }

    FddTddBandTest(String name, int band, int channel, int power) {
        super(name, band, channel, power);
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return true;
    }
}

/**
 * Class for FDD band related test.
 *
 */
class FddBandTest extends FddTddBandTest {

    FddBandTest(String name, int band, int channel, int power) {
        super(name, band, channel, power);
    }
}

/**
 * Class for TDD band related test.
 *
 */
class TddBandTest extends FddTddBandTest {

    TddBandTest(String name, int band, int channel, int power) {
        super(name, band, channel, power);
    }
}

/**
 * Class for LTE  band related test.
 *
 */
class LteBandTest extends BandTest {
    private static final int FDD_BAND_MIN = 1;
    private static final int TDD_BAND_MIN = 33;
    private static final int DUPLEX_TDD = 0;
    private static final int DUPLEX_FDD = 1;
    private static final int BAND_WIDTH_INDEX  = 0;
    private static final int TDD_CONFIG_INDEX = 0;
    private static final int TDD_SP_SF_CONFIG_INDEX = 0;
    private static final String DEFAULT_VRB_START = "0";
    private static final String DEFAULT_VRB_LENGTH = "1";
    private static final int DEFAULT_MCS = 0;
    public static final String UL_FREQ = "UL Freq";
    private boolean mFDD;

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
        Util.sendAtCommand(mContext, command, false,
                ClkQualityAtActivity.AtcMsg.START_LTE.getValue());
    }

    protected void pauseDesense() {
        Util.sendAtCommand(mContext, "AT+ERFTX=6,0,0", false,
                ClkQualityAtActivity.AtcMsg.PAUSE_LTE.getValue());
    }

    LteBandTest(String name, int band, int channel, int power, boolean isFdd) {
        super(name, band, channel, power);
        mFDD = isFdd;
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return true;
    }
}

/**
 * Class for CDMA band related test.
 *
 */
class CdmaBandTest extends BandTest {
    private static final int MODULATION = 0;

    protected void startDesense() {
        String command = "AT+ECRFTX=1," + mChannel + "," + mBandValue + "," + (mPower+60) + ","
                + MODULATION;

        Util.sendAtCommand(mContext, command, true,
                ClkQualityAtActivity.AtcMsg.START_CDMA.getValue());
    }

    protected void pauseDesense() {
        Util.sendAtCommand(mContext, "AT+ECRFTX=0", true,
                ClkQualityAtActivity.AtcMsg.PAUSE_CDMA.getValue());
    }

    CdmaBandTest(String name, int band, int channel, int power) {
        super(name, band, channel, power);
    }

    @Override
    protected boolean doApiTest() {
        // TODO Auto-generated method stub
        return true;
    }
}
