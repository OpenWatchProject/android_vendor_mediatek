package com.mediatek.engineermode.clkqualityat;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.support.v4.content.LocalBroadcastManager;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.desenseat.ATEServer;
import com.mediatek.engineermode.desenseat.ATEServerSockets;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * ATEServerClock
 */
public class ATEServerClock {
    private final int TRANSMIT_WARRING = 0;
    private final int TRANSMIT_ERROR = 1;
    private final int TRANSMIT_SUCCEED = 2;
    private final String ATC_SEND_ACTION = "com.mediatek.engineermode.clkqualityat.atc_send";
    private final String ATC_EXTRA_CMD = "atc_send.cmd";
    private final String ATC_EXTRA_MODEM_TYPE = "atc_send.modem";
    private final String ATC_EXTRA_MSG_ID = "atc_send.msgId";
    private final String TAG = "ATEServer";

    private Context mContext = null;
    private boolean mInited = false;

    private ATEServerSockets mServerSockets = null;
    private List<TestItem> mTestItems;
    private List<TestItem> mSelectedTestItems;
    private List<TestItem> mGenTestItems;
    private HashMap<String, String> sApiTestErrorCode;
    private String itemSupport = "";
    private List<String> mItemsToOmit = new ArrayList<String>();
    private int mTestModeValue = TestMode.MODE_NONE;
    private ClkQualityAtActivity.ClockType mClockType = ClkQualityAtActivity.ClockType.TCXO;
    private String mStrHeatingTimes = "12";
    private String mStrCoolTimes = "60";
    private Handler responseHandler = new Handler() {
        public void handleMessage(final Message msg) {
            Elog.v(TAG, "responseHander receive a message: " + msg.what);

            AsyncResult asyncResult = (AsyncResult) msg.obj;
            switch (AtcMsg.getAtcMsg(msg.what)) {
                case FLIGHT_MODE:
                case FLIGHT_MODE_CDMA:
                case START_CDMA:
                case START_FDDTDD:
                case START_GSM:
                case START_LTE:
                case PAUSE_CDMA:
                case PAUSE_FDDTDD:
                case PAUSE_GSM:
                case PAUSE_LTE:
                case REBOOT_LTE:
                case REBOOT_CDMA: {
                    if (asyncResult.exception == null) {
                        BandTest.sAtcDone = true;
                        Elog.v(TAG, "atc done for " + AtcMsg.getAtcMsg(msg.what).getName());
                    } else {
                        Elog.i(TAG, "fail to execute atc for "
                                                     + AtcMsg.getAtcMsg(msg.what).getName());
                        Toast.makeText(mContext, "fail to execute atc for",
                                                     Toast.LENGTH_LONG).show();
                        BandTest.sAtcDone = false;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    };
    private BroadcastReceiver atcReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO Auto-generated method stub
            if (ATC_SEND_ACTION.equals(intent.getAction())) {
                String param = intent.getStringExtra(ATC_EXTRA_CMD);
                Elog.d(TAG, "atcReceiver receive ATC_SEND_ACTION and param is " + param);
                String[] cmd = new String[2];
                String[] cmdCdma = new String[3];
                cmdCdma[0] = cmd[0] = param;
                cmdCdma[1] = cmd[1] = "";
                cmdCdma[2] = "DESTRILD:C2K";
                String[] cmd_s = ModemCategory.getCdmaCmdArr(cmdCdma);
                Elog.d(TAG, "atcReceiver cmd: " + cmd_s[0] + ",cmd_s.length = " + cmd_s.length);

                boolean isCDMA = intent.getBooleanExtra(ATC_EXTRA_MODEM_TYPE, false);
                int msgId = intent.getIntExtra(ATC_EXTRA_MSG_ID, -1);

                if (msgId > 0) {
                    if (!isCDMA)
                        EmUtils.invokeOemRilRequestStringsEm(false,cmd, getAtCmdHandler().obtainMessage(msgId));
                    else
                        EmUtils.invokeOemRilRequestStringsEm(true,cmd_s, getAtCmdHandler().obtainMessage(msgId));
                } else {
                    BandTest.sAtcDone = true;
                }
            }
        }
    };

    public void ATEServerClock() {

    }

    public Handler getAtCmdHandler() {
        return responseHandler;
    }

    private void sendCommandResult(int type, String text) {
        String cmd = "PMTK2024";
        cmd = cmd + "," + ATEServer.mCurrectRunCMD + "," + type + "," + text;
        mServerSockets.sendCommand(cmd);
    }

    public void init(Context context, ATEServerSockets serverSockets, long[] bandSupported) {
        Elog.d(TAG, "ATEServerClock -> init");
        mContext = context;

        mServerSockets = serverSockets;
        IntentFilter atcFilter = new IntentFilter();
        atcFilter.addAction(ATC_SEND_ACTION);
        LocalBroadcastManager.getInstance(mContext).registerReceiver(atcReceiver, atcFilter);
        mInited = true;
        ClkQualityAtActivity.initItemToCheck(mContext);
        apiTestErrorCodeMappingInit(mContext);
        mSelectedTestItems = new ArrayList<TestItem>();
        mGenTestItems = new ArrayList<TestItem>();
        mTestItems = TestItem.getTestItems(mContext, bandSupported);
    }

    /**
     * Release resource when usage ends.
     */
    public void unit() {
        if (mInited) {
            LocalBroadcastManager.getInstance(mContext).unregisterReceiver(atcReceiver);
        }

    }

    String getCmdValue(String response) {
        String strTemp = "";
        int endIndex = 0;
        strTemp = response.substring("$PMTK20XX,".length());
        endIndex = strTemp.indexOf("*");
        String result = strTemp.substring(0, endIndex);
        Elog.d(TAG, "result = " + result);
        return result;
    }

    private void apiTestErrorCodeMappingInit(Context context) {
        sApiTestErrorCode = new HashMap<String, String>();
        sApiTestErrorCode.put(context.getResources().getString(R.string
                .desense_at_no_3d_display_apk), "1");
    }

    private String checkTestItemValid(int itemIndex) {
        TestItem testItem = mTestItems.get(itemIndex);
        String testItemCode = ClkQualityAtActivity.sItemsToCheck.get(testItem.toString());
        String errorCode = "";
        if ((testItemCode != null)
                && (!testItem.doApiTest())) {
            errorCode = sApiTestErrorCode.get(testItemCode);
            Elog.d(TAG, "error code:(" + errorCode + "): " + testItemCode);
            return errorCode;
        }
        return errorCode;
    }

    //PMTK_CMD_CLOCK_QUALITY_AUTO_TEST_ITEMS PMTK2020
    public void testItemSet(String response) {
        Elog.d(TAG, "ATEServerClock -> testItemSet = " + response);
        String[] splited = null;
        if (mTestItems == null) {
            sendCommandResult(TRANSMIT_ERROR, "The modem type get failed, please try again");
            return;
        }
        mItemsToOmit.clear();
        Elog.d(TAG, "mTestItems.size() = " + mTestItems.size());
        try {
            String result = getCmdValue(response);
            splited = result.split(",");
        } catch (Exception e) {
            Elog.e(TAG, "get the chekced label failed:" + e.getMessage());
        }
        Elog.d(TAG, "received size = " + splited.length);
        if (splited.length != mTestItems.size()) {
            sendCommandResult(TRANSMIT_ERROR, "Testitems count mismatch");
            return;
        }
        mSelectedTestItems.clear();
        for (int i = 0; i < splited.length; i++) {
            if (splited[i].equals("1")) {
                mSelectedTestItems.add(mTestItems.get(i));
                Elog.d(TAG, mTestItems.get(i).toString() + " : checked");
                String errorCode = checkTestItemValid(i);
                if (!errorCode.equals("")) {
                    mServerSockets.sendCommand("PMTK2026," + i + ",0," + errorCode);
                    mItemsToOmit.add(mTestItems.get(i).toString());
                } else {
                    mServerSockets.sendCommand("PMTK2026," + i + ",1," + 0);
                }
            } else {
                Elog.d(TAG, mTestItems.get(i).toString() + " : unchecked");
            }
        }
        Elog.d(TAG, "mItemsToOmit:" + mItemsToOmit);
        sendCommandResult(2, "TestItems set succeed");

    }

    //PMTK_CMD_CLOCK_QUALITY_AUTO_TEST_ON_OFF PMTK2022
    public void stopClockQualityTest(String response) {
        ClkQualityAtActivity.setCancelled(true);
        sendCommandResult(TRANSMIT_WARRING, "Current test item will finish and stop before next " +
                "case");
    }

    public void startClockQualityTest(String response) {

        new Thread(new Runnable() {
            @Override
            public void run() {

                Elog.d(TAG, "test method = " + mTestModeValue);

                if (mTestModeValue == TestMode.MODE_NONE) {
                    Elog.d(TAG, "Please select test mode");
                    sendCommandResult(TRANSMIT_ERROR, "Please select test mode");
                    return;
                }

                if (mSelectedTestItems.size() <= 0) {
                    Elog.d(TAG, "Please select test items");
                    sendCommandResult(TRANSMIT_ERROR, "Please select test items");
                    return;
                }
                genRealTestItems();
                ClkQualityAtActivity.setCancelled(false);
                TestMode testMode = new TestMode(Integer.valueOf(mStrHeatingTimes), Integer.valueOf
                        (mStrCoolTimes), mTestModeValue);
                Elog.d(TAG, "checkReadyBit...");
                if (!testMode.checkReadyBit(mClockType)) {
                    if (testMode.isClockTypeCorrect())
                        sendCommandResult(TRANSMIT_ERROR, mContext.getString(R.string
                                .clk_quality_at_check_bit_fail));
                    else
                        sendCommandResult(TRANSMIT_ERROR, mContext.getString(R.string
                                .clk_quality_at_clock_type_wrong));
                    return;
                }
                //  sendCommandResult(TRANSMIT_SUCCEED,"checkReadyBit enable");
                testMode.setmTriggerSource(1, mServerSockets);
                int size = mGenTestItems.size();
                Elog.d(TAG, "mGenTestItems.size = " + size);
                if (size == 0) {
                    return;
                }

                Util.flightMode(mContext, true);
                for (int k = 0; k < size; k++) {
                    TestItem item = mGenTestItems.get(k);
                    if (ClkQualityAtActivity.getCancelled() || (ATEServer.mServerSockets == null) ){
                        Elog.d(TAG, "cancel and break");
                        sendCommandResult(TRANSMIT_SUCCEED, "stop test succeed");
                        return;
                    }
                    // run the test item
                    Elog.d(TAG, "run " + item.toString());
                    mServerSockets.sendCommand("PMTK2023," + item.getTag() + ",-1,0.0,0.0");
                    item.setTestMode(testMode);
                    TestResult r = item.run();
                    if ( r == null ){
                        Elog.d(TAG, "cancel and break");
                        sendCommandResult(TRANSMIT_SUCCEED, "stop test succeed");
                        return;
                    }
                    mServerSockets.sendCommand("PMTK2023," + item.getTag() + "," + r.result + ","
                            + r.getMaxClkDriftRate() + "," + r.getMaxCompclkdriftrate());
                }
                sendCommandResult(TRANSMIT_SUCCEED, "clock test succeed");
                // reboot modem
                Util.flightMode(mContext, false);
            }
        }).start();

    }

    //PMTK_CMD_CLOCK_QUALITY_AUTO_TEST_CONFIG PMTK2021
    public void testConfigSet(String response) {
        Elog.d(TAG, "ATEServerClock -> testConfigSet = " + response);
        String[] splited = null;

        try {
            String result = getCmdValue(response);
            splited = result.split(",");
        } catch (Exception e) {
            Elog.e(TAG, "get the chekced label failed:" + e.getMessage());
        }
        for (int i = 0; i < splited.length; i++) {
            Elog.d(TAG, "splited[" + i + "] = " + splited[i]);
        }

        if (splited[1].equals("0")) {
            mTestModeValue = TestMode.MODE_SIGNAL;
            Elog.d(TAG, "TestMode = SIGNAL");
        } else if (splited[1].equals("1")) {
            mTestModeValue = TestMode.MODE_CW;
            Elog.d(TAG, "TestMode = CW");
        } else {
            mTestModeValue = TestMode.MODE_NONE;
        }
        if (splited[0].equals("0")) {
            mClockType = ClkQualityAtActivity.ClockType.TCXO;
            Elog.d(TAG, "mClockType = TCXO");
            if (mSelectedTestItems.size() > 0) {
                int idxWifi = getTestItemIndex(mSelectedTestItems, WifiTxTest.NAME);
                if (idxWifi > 0) {
                    mSelectedTestItems.remove(idxWifi);
                    //       saveSettings();
                    Elog.d(TAG, "mSelectedTestItems remove wifi tx item in TCXO, idxWifi:" +
                            idxWifi);
                }
            }
        } else if (splited[0].equals("1")) {
            mClockType = ClkQualityAtActivity.ClockType.Co_TMS;
            Elog.d(TAG, "mClockType = Co_TMS");
        }
        if (!splited[2].equals("")) {
            mStrHeatingTimes = splited[2];
        } else {
            mStrHeatingTimes = "12";
        }
        if (!splited[3].equals("")) {
            mStrCoolTimes = splited[3];
        } else {
            mStrCoolTimes = "60";
        }
        sendCommandResult(TRANSMIT_SUCCEED, "Test config Set succeed");
    }

    private void genRealTestItems() {
        mGenTestItems.clear();
        for (TestItem testitem : mSelectedTestItems) {
            Elog.d(TAG, "mItemsToOmit = " + mItemsToOmit);
            Elog.d(TAG, "testitem  = " + testitem.toString());
            if (mItemsToOmit.contains(testitem.toString())) {
                continue;
            }
            mGenTestItems.add(testitem);
        }

        int size = mGenTestItems.size();
        for (int k = 0; k < size; k++) {
            Elog.d(TAG, "TestItem = " + mGenTestItems.get(k).toString());
            Elog.d(TAG, "TestItem index = " + (k + 1));
            mGenTestItems.get(k).setItemIndex(k + 1);
        }
    }
    private int getTestItemIndex(List<TestItem> items, String name) {
        int idx = 0;
        for (TestItem item : items) {
            if (item.toString().equals(name)) {
                return idx;
            }
            idx++;
        }
        return -1;
    }
    private enum AtcMsg{
        ATCMSG_NONE(0, "xxxx"),
        SUPPORT_QUERY(100, "query lte"),
        SUPPORT_QUERY_CDMA(101, "query cdma"),
        FLIGHT_MODE(102, "flight mode"),
        FLIGHT_MODE_CDMA(103, "flight mode cdma"),
        START_CDMA(104, "start cdma"),
        START_FDDTDD(105, "start FddTdd"),
        START_GSM(106, "start gsm"),
        START_LTE(107, "start lte"),
        PAUSE_CDMA(108, "pause cdma"),
        PAUSE_FDDTDD(109, "pause FddTdd"),
        PAUSE_GSM(110, "pause gsm"),
        PAUSE_LTE(111, "pause lte"),
        REBOOT_LTE(112, "reboot lte"),
        REBOOT_CDMA(113, "reboot cdma"),
        MSG_UPDATE_TEST_ITEM_QUERY(114, "update the test item");
        private final int value;
        private final String name;
        AtcMsg( int value, String name){
            this.value = value;
            this.name = name;
        }
        public static AtcMsg getAtcMsg(int id) {
            for (AtcMsg msg : AtcMsg.values()) {
                if (id == msg.getValue()) {
                    return msg;
                }
            }
            return ATCMSG_NONE;
        }
        public final int getValue() {
            return this.value;
        }
        public final String getName() {
            return this.name;
        }
    }
}

