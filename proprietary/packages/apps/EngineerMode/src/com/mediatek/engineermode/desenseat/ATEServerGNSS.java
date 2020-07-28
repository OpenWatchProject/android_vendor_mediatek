package com.mediatek.engineermode.desenseat;

import android.content.Context;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * ATEServerClock
 */
public class ATEServerGNSS {
    private static HashMap<String, String> sItemsToCheck;
    private final String TAG = "ATEServer";
    private final int MAX_SV_COUNT = 5;
    private final int TRANSMIT_WARRING = 0;
    private final int TRANSMIT_ERROR = 1;
    private final int TRANSMIT_SUCCEED = 2;
    public List<String> mItemsToOmit = new ArrayList<String>();
    public TestCondition mTestConditionGNSS = null;
    private ATEServerSockets mServerSockets = null;
    private Context mContext = null;

    private List<TestItem> mTestItemsGNSS = null;
    private List<TestItem> mSelectedTestItemsGNSS = null;
    private List<TestItem> mGenTestItemsGNSS = null;
    private List<TestMode> mTestModesGNSS = null;
    private HashMap<TestItem, TestResult> mTestResultsGNSS;
    private HashMap<String, String> sRfBandConfigGNSS;
    private HashMap<String, String> sApiTestErrorCodeGNSS;
    private int[] mSelectedSvidGNSS = new int[MAX_SV_COUNT];
    private TestMode mCurrentModeGNSS = null;
    private HashMap<String, String> mRfBandMapping = new HashMap<String, String>();

    void mRfBandMappingData() {
        mRfBandMapping.put("GSM 850", "0");
        mRfBandMapping.put("P-GSM 900", "1");
        mRfBandMapping.put("E-GSM 900", "2");
        mRfBandMapping.put("R-GSM 900", "3");
        mRfBandMapping.put("DCS 1800", "4");
        mRfBandMapping.put("PCS 1900", "5");

        mRfBandMapping.put("WCDMA Band 1", "100");
        mRfBandMapping.put("WCDMA Band 2", "101");
        mRfBandMapping.put("WCDMA Band 3", "102");
        mRfBandMapping.put("WCDMA Band 4", "103");
        mRfBandMapping.put("WCDMA Band 5", "104");
        mRfBandMapping.put("WCDMA Band 6", "105");
        mRfBandMapping.put("WCDMA Band 7", "106");
        mRfBandMapping.put("WCDMA Band 8", "107");
        mRfBandMapping.put("WCDMA Band 9", "108");
        mRfBandMapping.put("WCDMA Band 10", "109");
        mRfBandMapping.put("WCDMA Band 11", "110");
        mRfBandMapping.put("WCDMA Band 12", "111");
        mRfBandMapping.put("WCDMA Band 13", "112");
        mRfBandMapping.put("WCDMA Band 14", "113");
        mRfBandMapping.put("WCDMA Band 19", "114");
        mRfBandMapping.put("WCDMA Band 20", "115");
        mRfBandMapping.put("WCDMA Band 21", "116");
        mRfBandMapping.put("WCDMA Band 22", "117");

        mRfBandMapping.put("TD-SCDMA Band A", "200");
        mRfBandMapping.put("TD-SCDMA Band B", "201");
        mRfBandMapping.put("TD-SCDMA Band C", "202");
        mRfBandMapping.put("TD-SCDMA Band D", "203");
        mRfBandMapping.put("TD-SCDMA Band E", "204");
        mRfBandMapping.put("TD-SCDMA Band F", "205");
    }
    private void apiTestErrorCodeMappingInit(Context context) {
        sApiTestErrorCodeGNSS = new HashMap<String, String>();
        sApiTestErrorCodeGNSS.put(context.getResources().getString(R.string
                .desense_at_no_3d_display_apk), "1");
        sApiTestErrorCodeGNSS.put(context.getResources().getString(R.string
                .desense_at_no_live_wallpaper), "2");
        sApiTestErrorCodeGNSS.put(context.getResources().getString(R.string
                .desense_at_no_mp3_warning), "3");
        sApiTestErrorCodeGNSS.put(context.getResources().getString(R.string
                .desense_at_no_3gp_warning), "4");
        sApiTestErrorCodeGNSS.put(context.getResources().getString(R.string.
                desense_at_no_sd_warning), "5");
        sApiTestErrorCodeGNSS.put(context.getResources().getString(R.string
                .desense_at_front_camera_not_support), "6");
        sApiTestErrorCodeGNSS.put(context.getResources().getString(R.string
                .desense_at_back_camera_not_support), "7");
        sApiTestErrorCodeGNSS.put("Suspend not support", "8");
    }

    private void genGsmTestItem() {
        for (BandItem banditem : DesenseAtActivity.sGsmItems) {
            if (!banditem.isSelected()) {
                continue;
            }
            int power = banditem.getPower();
            if (power != -1) {
                int[] channel = banditem.getChannel();
                for (int k = 0; k < 3; k++) {
                    if (channel[k] != -1) {
                        mGenTestItemsGNSS.add(new GsmBandTest(banditem.getBandName(), channel[k],
                                banditem.getBandValue(), power, mContext));
                    }
                }
            }
        }
    }

    private void genFddTestItem() {
        for (BandItem banditem : DesenseAtActivity.sFddItems) {
            if (!banditem.isSelected()) {
                continue;
            }
            int power = banditem.getPower();
            if (power != -1) {
                int[] channel = banditem.getChannel();
                for (int k = 0; k < 3; k++) {
                    if (channel[k] != -1) {
                        mGenTestItemsGNSS.add(new FddBandTest(banditem.getBandName(),
                                channel[k], banditem.getBandValue(), power, mContext));
                    }
                }
            }
        }
    }

    private void genTddTestItem() {
        for (BandItem banditem : DesenseAtActivity.sTddItems) {
            if (!banditem.isSelected()) {
                continue;
            }
            int power = banditem.getPower();
            if (power != -1) {
                int[] channel = banditem.getChannel();
                for (int k = 0; k < 3; k++) {
                    if (channel[k] != -1) {
                        mGenTestItemsGNSS.add(new TddBandTest(banditem.getBandName(), channel[k],
                                banditem.getBandValue(), power, mContext));
                    }
                }
            }
        }
    }

    private void genLteTestItem() {
        for (BandItem banditem : DesenseAtActivity.sLteItems) {
            if (!banditem.isSelected()) {
                continue;
            }
            int power = banditem.getPower();
            if (power != -1) {
                int[] channel = banditem.getChannel();
                for (int k = 0; k < 3; k++) {
                    if (channel[k] != -1) {
                        mGenTestItemsGNSS.add(new LteBandTest
                                (banditem.getBandName(), channel[k], banditem.getBandValue(), power,
                                        (banditem.getType() == BandItem.BandType.BAND_LTE_FDD),
                                        mContext));
                    }
                }
            }
        }
    }

    private void genCdmaTestItem() {
        for (BandItem banditem : DesenseAtActivity.sCdmaItems) {
            if (!banditem.isSelected()) {
                continue;
            }
            int power = banditem.getPower();
            if (power != -1) {
                int[] channel = banditem.getChannel();
                for (int k = 0; k < 3; k++) {
                    if (channel[k] != -1) {
                        mGenTestItemsGNSS.add(new CdmaBandTest
                                (banditem.getBandName(),
                                        channel[k], banditem.getBandValue(), power, mContext));
                    }
                }
            }
        }
    }

    private void sendCommandResult(int type, String text) {
        String cmd = "PMTK2011";
        cmd = cmd + "," + ATEServer.mCurrectRunCMD + "," + type + "," + text;
        mServerSockets.sendCommand(cmd);
    }

    private String getCmdValue(String response) {
        String strTemp = "";
        int endIndex = 0;
        strTemp = response.substring("$PMTK20XX,".length());
        endIndex = strTemp.indexOf("*");
        String result = strTemp.substring(0, endIndex);
        Elog.d(TAG, "result = " + result);
        return result;
    }

    //PMTK_CMD_DESENSE_AUTO_TEST_START_API_TEST  PMTK2004
    public void startAPITest(String response) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                String test_status = "0";
                DesenseAtActivity.setCancelled(false);
                mTestConditionGNSS.setCondition(mContext);
                for (int i = 0; i < mTestItemsGNSS.size(); i++) {
                    if (DesenseAtActivity.getCancelled() || (ATEServer.mServerSockets == null) ) {
                        mTestConditionGNSS.resetCondition(mContext);
                        sendCommandResult(TRANSMIT_SUCCEED, "API test stoped succeed");
                        return;
                    }

                    TestItem item = mTestItemsGNSS.get(i);
                    boolean result = item.doApiTest();
                    if (i == (mTestItemsGNSS.size() - 1)) {
                        test_status = "1";
                    }
                    if (result == true) {
                        mServerSockets.sendCommand("PMTK2005," + i + ",1," + test_status);
                        Elog.d(TAG, "run " + item.toString() + " : succeed");
                    } else {
                        mServerSockets.sendCommand("PMTK2005," + i + ",0," + test_status);
                        Elog.d(TAG, "run " + item.toString() + " : failed");
                    }
                }
                sendCommandResult(TRANSMIT_SUCCEED, "API test succeed");
                mTestConditionGNSS.resetCondition(mContext);
            }
        }).start();
    }

    //PMTK_CMD_DESENSE_AUTO_TEST_ STOP_API_TEST  PMTK2013
    public void stopAPITest(String response) {
        Elog.d(TAG, "API Test off");
        DesenseAtActivity.setCancelled(true);
    }

    //PMTK_CMD_DESENSE_AUTO_TEST_MODE  PMTK2003
    public void testModesSet(String response) {
        String result = getCmdValue(response);
        int testModes = Integer.valueOf(result);
        if (testModes > 0 && testModes < 3) {
            mCurrentModeGNSS = mTestModesGNSS.get(testModes - 1);
            Elog.d(TAG, "testModes = " + testModes + "," + mCurrentModeGNSS.toString());
            sendCommandResult(TRANSMIT_SUCCEED, "TestModes set succeed");
        } else {
            sendCommandResult(TRANSMIT_ERROR, "TestModes not support");
        }
    }

    //PMTK_CMD_DESENSE_AUTO_TEST_QUERY_RF_CONFIG PMTK2012
    public void rfBandQuery(String response) {
        for (int k = 0; k < DesenseAtActivity.sGsmItems.size(); k++) {
            if (DesenseAtActivity.sGsmItems.get(k).isSelected()) {

                String bandIndex = getRfConfigID("18", DesenseAtActivity.sGsmItems.get(k)
                        .getBandName() + "_");

                mServerSockets.sendCommand("PMTK2006," + "1,1," + bandIndex + ","
                        + DesenseAtActivity.sGsmItems.get(k).getChannel()[0] + ","
                        + DesenseAtActivity.sGsmItems.get(k).getChannel()[1] + ","
                        + DesenseAtActivity.sGsmItems.get(k).getChannel()[2] + ","
                        + DesenseAtActivity.sGsmItems.get(k).getPower());
            }
        }
        if (ModemCategory.getModemType() == ModemCategory.MODEM_TD) {
            for (int k = 0; k < DesenseAtActivity.sTddItems.size(); k++) {
                if (DesenseAtActivity.sTddItems.get(k).isSelected()) {
                    String bandIndex = getRfConfigID("19", DesenseAtActivity.sTddItems.get(k)
                            .getBandName() + "_");

                    mServerSockets.sendCommand("PMTK2006," + "1,1," + bandIndex + ","
                            + DesenseAtActivity.sTddItems.get(k).getChannel()[0] + ","
                            + DesenseAtActivity.sTddItems.get(k).getChannel()[1] + ","
                            + DesenseAtActivity.sTddItems.get(k).getChannel()[2] + ","
                            + DesenseAtActivity.sTddItems.get(k).getPower());
                }
            }
        } else if (ModemCategory.getModemType() == ModemCategory.MODEM_FDD) {
            for (int k = 0; k < DesenseAtActivity.sFddItems.size(); k++) {
                if (DesenseAtActivity.sFddItems.get(k).isSelected()) {
                    String bandIndex = getRfConfigID("19", DesenseAtActivity.sFddItems.get(k)
                            .getBandName() + "_");
                    mServerSockets.sendCommand("PMTK2006," + "1,1," + bandIndex + ","
                            + DesenseAtActivity.sFddItems.get(k).getChannel()[0] + ","
                            + DesenseAtActivity.sFddItems.get(k).getChannel()[1] + ","
                            + DesenseAtActivity.sFddItems.get(k).getChannel()[2] + ","
                            + DesenseAtActivity.sFddItems.get(k).getPower());
                }
            }
        }
        if (ModemCategory.isLteSupport()) {
            for (int k = 0; k < DesenseAtActivity.sLteItems.size(); k++) {
                if (DesenseAtActivity.sLteItems.get(k).isSelected()) {
                    String bandIndex = getRfConfigID("20", DesenseAtActivity.sLteItems.get(k)
                            .getBandName() + "_");
                    mServerSockets.sendCommand("PMTK2006," + "1,1," + bandIndex + ","
                            + DesenseAtActivity.sLteItems.get(k).getChannel()[0] + ","
                            + DesenseAtActivity.sLteItems.get(k).getChannel()[1] + ","
                            + DesenseAtActivity.sLteItems.get(k).getChannel()[2] + ","
                            + DesenseAtActivity.sLteItems.get(k).getPower());
                }
            }
        }
        if (ModemCategory.isCdma()) {
            for (int k = 0; k < DesenseAtActivity.sCdmaItems.size(); k++) {
                if (DesenseAtActivity.sCdmaItems.get(k).isSelected()) {
                    String bandIndex = getRfConfigID("21", DesenseAtActivity.sCdmaItems.get(k)
                            .getBandName() + "_");
                    mServerSockets.sendCommand("PMTK2006," + "1,1," + bandIndex + ","
                            + DesenseAtActivity.sCdmaItems.get(k).getChannel()[0] + ","
                            + DesenseAtActivity.sCdmaItems.get(k).getChannel()[1] + ","
                            + DesenseAtActivity.sCdmaItems.get(k).getChannel()[2] + ","
                            + DesenseAtActivity.sCdmaItems.get(k).getPower());
                }
            }
        }
        sendCommandResult(TRANSMIT_SUCCEED, "GNSS Band query succeed");
    }


    public void updateBandInfo(String[] info) {
        int ratType = Integer.valueOf(info[2]);
        boolean isSelected = Integer.valueOf(info[1]) == 1 ? true : false;
        Elog.d(TAG, "ratType = " + ratType + " isSelected = " + isSelected);
        if (ratType >= 0 && ratType <= 5) {
            Elog.d(TAG, "update GSM band info ");
            for (int i = 0; i < DesenseAtActivity.sGsmItems.size(); i++) {
                String bandIndex = getRfConfigID("18", DesenseAtActivity.sGsmItems.get(i)
                        .getBandName() + "_");
                int tag = Integer.valueOf(bandIndex);
                if (ratType == tag) {
                    DesenseAtActivity.sGsmItems.get(i).setSelected(isSelected);
                    DesenseAtActivity.sGsmItems.get(i).setChannel(Integer.valueOf(info[3]),
                            Integer.valueOf(info[4]), Integer.valueOf(info[5]));
                    DesenseAtActivity.sGsmItems.get(i).setPower(Integer.valueOf(info[6]));
                    break;
                }
            }
        } else if (ratType >= 100 && ratType <= 117) {
            Elog.d(TAG, "update WCDMA band info ");
            for (int i = 0; DesenseAtActivity.sFddItems != null && i < DesenseAtActivity
                    .sFddItems.size(); i++) {
                String bandIndex = getRfConfigID("19", DesenseAtActivity.sFddItems.get(i)
                        .getBandName() + "_");
                int tag = Integer.valueOf(bandIndex);
                if (ratType == tag) {
                    DesenseAtActivity.sFddItems.get(i).setSelected(isSelected);
                    DesenseAtActivity.sFddItems.get(i).setChannel(Integer.valueOf(info[3]),
                            Integer.valueOf(info[4]), Integer.valueOf(info[5]));
                    DesenseAtActivity.sFddItems.get(i).setPower(Integer.valueOf(info[6]));
                    break;
                }
            }
        } else if (ratType >= 200 && ratType <= 201) {
            Elog.d(TAG, "update TDSCDMA band info ");
            for (int i = 0; DesenseAtActivity.sTddItems != null && i < DesenseAtActivity
                    .sTddItems.size(); i++) {
                String bandIndex = getRfConfigID("19", DesenseAtActivity.sTddItems.get(i)
                        .getBandName() + "_");
                int tag = Integer.valueOf(bandIndex);
                if (ratType == tag) {
                    DesenseAtActivity.sTddItems.get(i).setSelected(isSelected);
                    DesenseAtActivity.sTddItems.get(i).setChannel(Integer.valueOf(info[3]),
                            Integer.valueOf(info[4]), Integer.valueOf(info[5]));
                    DesenseAtActivity.sTddItems.get(i).setPower(Integer.valueOf(info[6]));
                    break;
                }
            }
        } else if (ratType >= 300 && ratType <= 315) {
            Elog.d(TAG, "update CDMA band info ");
            for (int i = 0; DesenseAtActivity.sCdmaItems != null && i < DesenseAtActivity
                    .sCdmaItems.size(); i++) {
                String bandIndex = getRfConfigID("21", DesenseAtActivity.sCdmaItems.get(i)
                        .getBandName() + "_");
                int tag = Integer.valueOf(bandIndex);
                if (ratType == tag) {
                    DesenseAtActivity.sCdmaItems.get(i).setSelected(isSelected);
                    DesenseAtActivity.sCdmaItems.get(i).setChannel(Integer.valueOf(info[3]),
                            Integer.valueOf(info[4]), Integer.valueOf(info[5]));
                    DesenseAtActivity.sCdmaItems.get(i).setPower(Integer.valueOf(info[6]));
                    break;
                }
            }
        } else if (ratType >= 400 && ratType <= 443) {
            Elog.d(TAG, "update LTE band info ");
            for (int i = 0; DesenseAtActivity.sLteItems != null && i < DesenseAtActivity
                    .sLteItems.size(); i++) {
                String bandIndex = getRfConfigID("20", DesenseAtActivity.sLteItems.get(i)
                        .getBandName() + "_");
                int tag = Integer.valueOf(bandIndex);
                if (ratType == tag) {
                    DesenseAtActivity.sLteItems.get(i).setSelected(isSelected);
                    DesenseAtActivity.sLteItems.get(i).setChannel(Integer.valueOf(info[3]),
                            Integer.valueOf(info[4]), Integer.valueOf(info[5]));
                    DesenseAtActivity.sLteItems.get(i).setPower(Integer.valueOf(info[6]));
                    break;
                }
            }
        } else {
            Elog.d(TAG, "not suppoty type ");
        }

    }

    //PMTK_CMD_DESENSE_AUTO_TEST_RF_BAND_CONFIG PMTK2006
    public void rfBandConfig(String response) {
        String[] splited = null;
        String result = getCmdValue(response);
        splited = result.split(",");
        for (int i = 0; i < splited.length; i++) {
            Elog.d(TAG, "splited[" + i + "] = " + splited[i]);
        }
        updateBandInfo(splited);
        sendCommandResult(TRANSMIT_SUCCEED, "Rf Band Config succeed");
    }

    //PMTK_CMD_DESENSE_AUTO_TEST_GNSS_BAND PMTK2002
    public void gnssBandeSet(String response) {
        String[] splited = null;
        String result = getCmdValue(response);
        splited = result.split(",");
        for (int i = 0; i < splited.length; i++) {
            mSelectedSvidGNSS[i] = Integer.valueOf(splited[i]);
            Elog.d(TAG, "mSelectedSvidGNSS[" + i + "] = " + mSelectedSvidGNSS[i]);
        }
        for (TestMode m : mTestModesGNSS) {
            m.setSvIds(mSelectedSvidGNSS);
        }
        sendCommandResult(TRANSMIT_SUCCEED, "GNSS Bande set succeed");
    }

    //PMTK_CMD_DESENSE_AUTO_TEST_ITEMS PMTK2001
    public void testItemSet(String response) {
        String[] splited = null;
        mItemsToOmit.clear();
        Elog.d(TAG, "mTestItemsGNSS.size() = " + mTestItemsGNSS.size());
        try {
            String result = getCmdValue(response);
            splited = result.split(",");
        } catch (Exception e) {
            Elog.e(TAG, "get the chekced label failed:" + e.getMessage());
        }
        Elog.d(TAG, "received size = " + splited.length);
        if (splited.length != mTestItemsGNSS.size()) {
            sendCommandResult(TRANSMIT_ERROR, "Testitems count mismatch");
            return;
        }
        mSelectedTestItemsGNSS.clear();
        for (int i = 0; i < splited.length; i++) {
            if (splited[i].equals("1")) {
                mSelectedTestItemsGNSS.add(mTestItemsGNSS.get(i));
                Elog.d(TAG, mTestItemsGNSS.get(i).toString() + " : checked");
                String errorCode = checkTestItemValid(i);
                if (!errorCode.equals("")) {
                    mServerSockets.sendCommand("PMTK2009," + (i) + ",0," + errorCode);
                    mItemsToOmit.add(mTestItemsGNSS.get(i).toString());
                } else {
                    mServerSockets.sendCommand("PMTK2009," + (i) + ",1," + 0);
                }
            } else {
                Elog.d(TAG, mTestItemsGNSS.get(i).toString() + " : unchecked");
            }
        }
        Elog.d(TAG, "mItemsToOmit:" + mItemsToOmit);

        sendCommandResult(TRANSMIT_SUCCEED, "TestItems set succeed");
    }

    private String checkTestItemValid(int itemIndex) {
        TestItem testItem = mTestItemsGNSS.get(itemIndex);
        String testItemCode = sItemsToCheck.get(testItem.toString());
        String errorCode = "";
        if ((testItemCode != null)
                && (!testItem.doApiTest())) {
            errorCode = sApiTestErrorCodeGNSS.get(testItemCode);
            Elog.d(TAG, "error code:(" + errorCode + "): " + testItemCode);
            return errorCode;
        }
        return errorCode;
    }

    private boolean validSvIdSelected() {
        boolean ret = false;
        Elog.d(TAG, "validSvIdSelected");
        for (int id : mSelectedSvidGNSS) {
            Elog.d(TAG, "id:" + id);
            if (id != Util.INVALID_SV_ID) {
                ret = true;
            }
        }
        return ret;
    }

    private void genRealTestItems() {
        mGenTestItemsGNSS.clear();
        for (TestItem testitem : mSelectedTestItemsGNSS) {
            if (mItemsToOmit.contains(testitem.toString())) {
                continue;
            }
            if (GsmBandVirtualTest.ITEM_TAG.equals(testitem.toString())) {
                genGsmTestItem();
            } else if (FddBandVirtualTest.ITEM_TAG.equals(testitem.toString())) {
                genFddTestItem();
            } else if (TddBandVirtualTest.ITEM_TAG.equals(testitem.toString())) {
                genTddTestItem();
            } else if (LteBandVirtualTest.ITEM_TAG.equals(testitem.toString())) {
                genLteTestItem();
            } else if (CdmaBandVirtualTest.ITEM_TAG.equals(testitem.toString())) {
                genCdmaTestItem();
            } else {
                mGenTestItemsGNSS.add(testitem);
            }
        }

        int size = mGenTestItemsGNSS.size();
        for (int k = 0; k < size; k++) {
            TestItem item = mGenTestItemsGNSS.get(k);
            Elog.d(TAG, "TestItem = " + item.toString());
            Elog.d(TAG, "TestItem index = " + (k + 1));
            Elog.d(TAG, "TestItem tag = " + item.getTag());
            Elog.d(TAG, "Band index = " + getRfConfigID(item.getTag(), item.toString()));
            item.setItemIndex(k + 1);
        }
    }

    public void stopGNSSTest(String response) {
        DesenseAtActivity.setCancelled(true);
        sendCommandResult(TRANSMIT_WARRING, "Current test item will finish and stop before next " +
                "case");
    }

    //PMTK_CMD_DESENSE_AUTO_TEST_ON_OFF PMTK2007
    public void startGNSSTest(String response) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                Elog.d(TAG, "test method = " + mCurrentModeGNSS);
                if (mCurrentModeGNSS == null) {
                    Elog.d(TAG, "Please select test mode");
                    sendCommandResult(TRANSMIT_ERROR, "Please select test mode");
                    return;
                }

                if (mSelectedTestItemsGNSS.size() <= 0) {
                    Elog.d(TAG, "Please select test items");
                    sendCommandResult(TRANSMIT_ERROR, "Please select test items");
                    return;
                }

                if (("Signal".equals(mCurrentModeGNSS.toString())) && (!validSvIdSelected())) {
                    Elog.d(TAG, "Please select GNSS Band");
                    sendCommandResult(TRANSMIT_ERROR, "Please select GNSS Band");
                    return;
                }
                genRealTestItems();

                int size = mGenTestItemsGNSS.size();
                if (size == 0) {
                    return;
                }
                mTestConditionGNSS.setCondition(mContext);

                StringBuilder strCmd = new StringBuilder(String.valueOf(size));
                for (TestItem item : mGenTestItemsGNSS) {
                    strCmd.append(',').append(item.getDbSpec());
                }
                if (!mCurrentModeGNSS.setSpec(strCmd.toString())) {
                    sendCommandResult(TRANSMIT_WARRING, "Set spec fail. Default DB spec will be " +
                            "used");
                } else {
                    Elog.d(TAG, "Set spec succeed");
                }

                mCurrentModeGNSS.setmTriggerSource(1, mServerSockets);

                DesenseAtActivity.setCancelled(false);
                for (int k = 0; k < size; k++) {
                    TestItem item = mGenTestItemsGNSS.get(k);
                    if (DesenseAtActivity.getCancelled() || (ATEServer.mServerSockets == null )) {
                        mTestConditionGNSS.resetCondition(mContext);
                        sendCommandResult(TRANSMIT_SUCCEED, "stop succeed");
                        return;
                    }
                    // run the test item
                    Elog.d(TAG, "run " + item.toString());
                    mServerSockets.sendCommand("PMTK2010," + (k + 1) + "," + item.getTag() + "," +
                            getRfConfigID(item.getTag(), item.toString()));

                    item.setTestMode(mCurrentModeGNSS);
                    TestResult result = item.run();
                    Elog.d(TAG, "TestResult = " + result);
                }
                sendCommandResult(TRANSMIT_SUCCEED, "GNSS desense succeed");
                mTestConditionGNSS.resetCondition(mContext);
            }
        }).start();
    }

    String getRfConfigID(String tag, String itemString) {
        int index = Integer.valueOf(tag);
        String bandName = itemString.split("_")[0];
        Elog.d(TAG, "bandName = " + bandName);
        int bandIndex = 0;
        if (index >= 0 && index <= 17) {
            return "-1";
        } else if (index == 20) {           //LTE
            bandIndex = Integer.valueOf(bandName.split(" ")[2]) + 399;
        } else if (index == 21) {             //cdma
            bandIndex = Integer.valueOf(bandName.split(" ")[2]) + 300;
        } else if (index == 18 || index == 19) {//GSM WCDMA TDSCDMA
            bandIndex = Integer.valueOf(mRfBandMapping.get(bandName));
        }
        return bandIndex + "";
    }


    public void ATEServerGNSS() {

    }

    private void initItemToCheck() {
        sItemsToCheck = new HashMap<String, String>();
        sItemsToCheck.put(new SleepToNormalTest().toString(),
                "Suspend not support");

        sItemsToCheck.put(new Display3DTest().toString(),
                mContext.getResources().getString(R.string.desense_at_no_3d_display_apk));
//        sItemsToCheck.put(new CpuFullLoadingTest().toString(),
//                getResources().getString(R.string.desense_at_no_3d_display_apk));
        sItemsToCheck.put(new LiveWallpaperTest().toString(),
                mContext.getResources().getString(R.string.desense_at_no_live_wallpaper));
        sItemsToCheck.put(new Mp3PlayTest().toString(),
                mContext.getResources().getString(R.string.desense_at_no_mp3_warning));
        sItemsToCheck.put(new VideoPlayTest().toString(),
                mContext.getResources().getString(R.string.desense_at_no_3gp_warning));
        sItemsToCheck.put(new ExternalSdCardReadTest().toString(),
                mContext.getResources().getString(R.string.desense_at_no_sd_warning));
        sItemsToCheck.put(new ExternalSdCardWriteTest().toString(),
                mContext.getResources().getString(R.string.desense_at_no_sd_warning));
        sItemsToCheck.put(new FrontCameraPreviewTest().toString(),
                mContext.getResources().getString(R.string.desense_at_front_camera_not_support));
        sItemsToCheck.put(new BackCameraPreviewTest().toString(),
                mContext.getResources().getString(R.string.desense_at_back_camera_not_support));
    }

    public void init(Context context, ATEServerSockets serverSockets, long[] bandSupported) {
        Elog.d(TAG, "ATEServerGNSS -> init");
        mContext = context;
        mServerSockets = serverSockets;

        apiTestErrorCodeMappingInit(mContext);

        mSelectedTestItemsGNSS = new ArrayList<TestItem>();
        mGenTestItemsGNSS = new ArrayList<TestItem>();
        mTestResultsGNSS = new HashMap<TestItem, TestResult>();

        mTestItemsGNSS = TestItem.getTestItems();
        mTestModesGNSS = TestMode.getTestModes();
        for (TestMode m : mTestModesGNSS) {
            m.setSvIds(mSelectedSvidGNSS);
        }
        DesenseAtActivity.initRfConfigList(mContext, bandSupported);
        mRfBandMappingData();
        initItemToCheck();
        TestItem.updateContext(mContext);

        mTestConditionGNSS = new TestCondition();
        mTestConditionGNSS.init(mContext);

    }


}

