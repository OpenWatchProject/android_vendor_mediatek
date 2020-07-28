package com.mediatek.engineermode.npt;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.android.internal.telephony.Phone;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.bandselect.BandModeContent;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;


public class BandConfigActivity extends Activity {
    public static final String TAG = "NPT/BandConfig";
    public static final int BAND_NAME_INDEX = 0;
    public static final int BAND_VALUE_INDEX = 1;
    public static final int CHANNEL_START_INDEX = 2;
    public static final int CHANNEL_STEP_INDEX = 3;
    public static final int CHANNEL_STOP_INDEX = 4;

    public static final int POWER_DEFAULT_INDEX = 5;
    public static final int TX_ON_FLAG_REF = 6;
    public static final int TX_ON_FLAG_NPT = 7;
    public static final int REPEAT_TIMES = 8;

    public static final int RX_BW_TIMES = 9;
    public static final int TX_BW_TIMES = 10;
    public static final int RB_START_TIMES = 11;
    public static final int RB_LENGTH_TIMES = 12;

    public static final int DOWNLINK_MIN_FREQ = 13;
    public static final int UPLINK_MIN_FREQ = 14;
    public static final int ANT_STATUS = 15;

    public static final int INDEX_GSM_BAND = 0;
    public static final int INDEX_UMTS_WCDMA_BAND = 1;
    public static final int INDEX_LTE_FDD_BAND = 2;
    public static final int INDEX_LTE_TDD_BAND = 3;
    public static final int INDEX_CDMA_BAND = 4;
    public static final int INDEX_UMTS_TDSCDMA_BAND = 5;
    public static final int INDEX_BAND_MAX = 6;
    public static final int RAT_TYPE_GSM = 1;
    public static final int RAT_TYPE_TDSCDMA = 2;
    public static final int RAT_TYPE_WCDMA = 3;
    public static final int RAT_TYPE_LTE = 4;
    public static final int RAT_TYPE_CDMD_1X = 5;
    public static final int RAT_TYPE_CDMD_EVDO = 6;
    public static final String QUERY_CURRENT_COMMAND_CDMA = "AT+ECBANDCFG?";

    private static final int SUPPORT_QUERY = 0;
    private static final int SUPPORT_QUERY_CDMA = 1;
    private static final String QUERY_SUPPORT_COMMAND = "AT+EPBSE=?";
    private static final String SAME_COMMAND = "+EPBSE:";
    private static final String SAME_COMMAND_CDMA = "+ECBANDCFG:";

    private static final String[] InputMappingKey = {"GSM", "TD-SCDMA", "WCDMA", "LTE-FDD",
            "LTE-TDD", "CDMA", "EVDO", "values", "PreNPT", "PostNPT","PreREF"};

    public static long[] bandSupported = new long[INDEX_BAND_MAX];
    public static HashMap<String, Integer> sGsmBandValueMapping = new HashMap<String, Integer>();
    public static HashMap<String, ArrayList<String>> sInputMapping = new HashMap<String,
            ArrayList<String>>();
    public static List<BandItem> sGsmItems = new ArrayList<BandItem>();
    public static List<BandItem> sTddItems = new ArrayList<BandItem>();
    public static List<BandItem> sFddItems = new ArrayList<BandItem>();
    public static List<BandItem> sLteItems = new ArrayList<BandItem>();
    public static List<BandItem> sCdmaItems = new ArrayList<BandItem>();
    public static List<BandItem> sEvdoItems = new ArrayList<BandItem>();

    private static BandItem sItemClicked;
    private static BandAdapter mGsmAdapter;
    private static BandAdapter mFddAdapter;
    private static BandAdapter mTddAdapter;
    private static BandAdapter mLteAdapter;
    private static BandAdapter mCdmaAdapter;
    private static ListView mGsmList;
    private static ListView mFddList;
    private static ListView mTddList;
    private static ListView mLteList;
    private static ListView mCdmaList;

    private static Context mContext = null;
    private static Handler responseHandler = new Handler() {
        public void handleMessage(final Message msg) {
            AsyncResult asyncResult = (AsyncResult) msg.obj;
            switch (msg.what) {
                case SUPPORT_QUERY:
                    if (asyncResult.exception == null) {
                        String[] value = (String[]) asyncResult.result;
                        getSupportedBand(value);

                        String[] cmd = new String[2];
                        cmd[0] = QUERY_CURRENT_COMMAND_CDMA;
                        cmd[1] = SAME_COMMAND_CDMA;
                        Elog.d(TAG, "query at cmd: " + cmd[0]);
                        EmUtils.invokeOemRilRequestStringsEm(cmd,
                                responseHandler.obtainMessage(SUPPORT_QUERY_CDMA));
                    } else {
                        Elog.e(TAG, "responseHandler get have exception!");
                    }
                    break;
                case SUPPORT_QUERY_CDMA:
                    if (asyncResult.exception == null) {
                        String[] value = (String[]) asyncResult.result;
                        bandSupported[INDEX_CDMA_BAND] = getSupportedBandCdma(value);
                    } else {
                        Elog.e(TAG, "responseHandler get have exception!");
                    }
                    initRfConfigList(mContext, bandSupported);
                    NoiseProfilingActivity.load_default_configure_fime(mContext);
                    Elog.v(TAG, "query support bands finished.");
                    break;
                default:
                    break;
            }
        }
    };
    private View viewGsm;
    private View viewTdscdma;
    private View viewWcdma;
    private View viewCdma;
    private View viewLte;
    private int mRatType = 0;
    private AdapterView.OnItemClickListener mCommonListener =
            new AdapterView.OnItemClickListener() {
                public void onItemClick(AdapterView parent, View view, int position, long id) {
                    sItemClicked = (BandItem) parent.getItemAtPosition(position);
                    Elog.d(TAG, "clicked item is : " + sItemClicked.toString());
                    Intent mIntentBandDetail = new Intent(BandConfigActivity.this,
                            BandDetailActivity.class);
                    startActivity(mIntentBandDetail);
                }
            };


    public static BandItem getClickedItem() {
        Elog.d(TAG, "return item is : " + sItemClicked.toString());
        return sItemClicked;
    }

    public static boolean isSupportedBand(long bits, int idx) {
        if ((bits & (1L << idx)) == 0)
            return false;
        return true;
    }

    static public void initBandItems(Context context) {
        mContext = context;
        sGsmBandValueMapping.put("GSM 850", 7);
        sGsmBandValueMapping.put("GSM 900", 1);
        sGsmBandValueMapping.put("GSM 1800", 3);
        sGsmBandValueMapping.put("GSM 1900", 4);

        sGsmItems.clear();
        sTddItems.clear();
        sFddItems.clear();
        sLteItems.clear();
        sCdmaItems.clear();
        sEvdoItems.clear();

        final String[] queryCommon = {QUERY_SUPPORT_COMMAND, SAME_COMMAND};
        Elog.v(TAG, "sendAtCommand: " + queryCommon[0] + ", " + queryCommon[1]);
        EmUtils.invokeOemRilRequestStringsEm(queryCommon, responseHandler.obtainMessage
                (SUPPORT_QUERY));
    }

    private static long getSupportedBandCdma(String[] bandStr) {
        Elog.d(TAG, "query SupportedBandCdma:" + Arrays.toString(bandStr));
        String strRes = bandStr[0];
        if ((strRes != null) && (strRes.contains(BandModeContent.SAME_COMMAND_CDMA))) {
            String splitString = strRes.substring(BandModeContent.SAME_COMMAND_CDMA.length());
            final String[] getDigitalVal = splitString.split(",");
            long[] value = new long[2];
            try {
                for (int i = 0; i < 2; i++) {
                    if (getDigitalVal[i] != null) {
                        value[i] = Integer.parseInt(getDigitalVal[i].substring(2), 16);
                    }
                }
            } catch (NumberFormatException e) {
                value[0] = 0;
            }
            Elog.d(TAG, "getSupportedBandCdma return: " + value[0]);
            return value[0];
        } else {
            //Default band 0
            return 1;
        }
    }

    //<!--bitpos 7 1 3 4 -->
    private static void initGsmItems(Context context, long bitsSupported) {
        String[] bandDefault = context.getResources().getStringArray(R.array.npt_gsm_data);
        int bandAarayLength = bandDefault.length;

        Elog.i(TAG, "initGsmItems bitsSupported: " + bitsSupported);

        for (int k = 0; k < bandAarayLength; k++) {

            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsSupported, sGsmBandValueMapping.get(values[BAND_NAME_INDEX])
                    .intValue()))
                continue;
            sGsmItems.add(new BandItem(values[BAND_NAME_INDEX],
                    Integer.parseInt(values[BAND_VALUE_INDEX]),
                    Integer.parseInt(values[CHANNEL_START_INDEX]),
                    Integer.parseInt(values[CHANNEL_STEP_INDEX]),
                    Integer.parseInt(values[CHANNEL_STOP_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX]),
                    Integer.parseInt(values[TX_ON_FLAG_REF]),
                    Integer.parseInt(values[TX_ON_FLAG_NPT]),
                    Integer.parseInt(values[REPEAT_TIMES]),
                    BandItem.BandType.BAND_GSM,
                    Integer.parseInt(values[UPLINK_MIN_FREQ]),
                    Integer.parseInt(values[DOWNLINK_MIN_FREQ]),
                    Integer.parseInt(values[ANT_STATUS])
            ));
        }
        for (BandItem item : sGsmItems) {
            Elog.d(TAG, "item content: " + item.getmBandName() + " " + item.getSummary1());
        }
    }

    private static void initTddItems(Context context, long bitsSupported) {
        String[] bandDefault = context.getResources().getStringArray(R.array
                .npt_td_scdma_data);
        Elog.i(TAG, "initTddItems bitsSupported: " + bitsSupported);
        int bandAarayLength = bandDefault.length;
        sTddItems = new ArrayList<BandItem>();
        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsSupported, Integer.parseInt(values[BAND_VALUE_INDEX]) - 1))
                continue;
            sTddItems.add(new BandItem(values[BAND_NAME_INDEX],
                    Integer.parseInt(values[BAND_VALUE_INDEX]),
                    Integer.parseInt(values[CHANNEL_START_INDEX]),
                    Integer.parseInt(values[CHANNEL_STEP_INDEX]),
                    Integer.parseInt(values[CHANNEL_STOP_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX]),
                    Integer.parseInt(values[TX_ON_FLAG_REF]),
                    Integer.parseInt(values[TX_ON_FLAG_NPT]),
                    Integer.parseInt(values[REPEAT_TIMES]),
                    BandItem.BandType.BAND_TD,
                    Integer.parseInt(values[UPLINK_MIN_FREQ]),
                    Integer.parseInt(values[DOWNLINK_MIN_FREQ]),
                    Integer.parseInt(values[ANT_STATUS])
            ));
        }
    }

    private static void initFddItems(Context context, long bitsSupported) {
        String[] bandDefault = context.getResources().getStringArray(R.array.npt_wcdma_data);
        Elog.i(TAG, "initFddItems bitsSupported: " + bitsSupported);
        int bandAarayLength = bandDefault.length;

        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsSupported, Integer.parseInt(values[BAND_VALUE_INDEX]) - 1))
                continue;
            sFddItems.add(new BandItem(values[BAND_NAME_INDEX],
                    Integer.parseInt(values[BAND_VALUE_INDEX]),
                    Integer.parseInt(values[CHANNEL_START_INDEX]),
                    Integer.parseInt(values[CHANNEL_STEP_INDEX]),
                    Integer.parseInt(values[CHANNEL_STOP_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX]),
                    Integer.parseInt(values[TX_ON_FLAG_REF]),
                    Integer.parseInt(values[TX_ON_FLAG_NPT]),
                    Integer.parseInt(values[REPEAT_TIMES]),
                    BandItem.BandType.BAND_WCDMA,
                    Integer.parseInt(values[UPLINK_MIN_FREQ]),
                    Integer.parseInt(values[DOWNLINK_MIN_FREQ]),
                    Integer.parseInt(values[ANT_STATUS])
            ));
        }
    }

    private static void initLteItems(Context context, long bitsLSupported, long bitsHSupported) {
        String[] bandDefault = context.getResources().getStringArray(
                R.array.npt_lte_fdd_data);
        Elog.i(TAG, "initLTE FDD bitsSupported: " + bitsLSupported);
        int bandAarayLength = bandDefault.length;

        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsLSupported, Integer.parseInt(values[BAND_VALUE_INDEX]) - 1))
                continue;
            sLteItems.add(new BandItem(values[BAND_NAME_INDEX],
                            Integer.parseInt(values[BAND_VALUE_INDEX]),
                            Integer.parseInt(values[CHANNEL_START_INDEX]),
                            Integer.parseInt(values[CHANNEL_STEP_INDEX]),
                            Integer.parseInt(values[CHANNEL_STOP_INDEX]),
                            Integer.parseInt(values[POWER_DEFAULT_INDEX]),
                            Integer.parseInt(values[TX_ON_FLAG_REF]),
                            Integer.parseInt(values[TX_ON_FLAG_NPT]),
                            Integer.parseInt(values[REPEAT_TIMES]),
                            BandItem.BandType.BAND_LTE,
                            Integer.parseInt(values[RX_BW_TIMES]),
                            Integer.parseInt(values[TX_BW_TIMES]),
                            Integer.parseInt(values[RB_START_TIMES]),
                            Integer.parseInt(values[RB_LENGTH_TIMES]),
                            Integer.parseInt(values[UPLINK_MIN_FREQ]),
                            Integer.parseInt(values[DOWNLINK_MIN_FREQ]),
                            Integer.parseInt(values[ANT_STATUS])
                    )
            );
        }

        bandDefault = context.getResources().getStringArray(
                R.array.npt_lte_tdd_data);
        Elog.i(TAG, "initLTE TDD Items bitsSupported: " + bitsHSupported);
        bandAarayLength = bandDefault.length;

        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsHSupported, Integer.parseInt(values[BAND_VALUE_INDEX]) - 33))
                continue;
            sLteItems.add(new BandItem(values[BAND_NAME_INDEX],
                            Integer.parseInt(values[BAND_VALUE_INDEX]),
                            Integer.parseInt(values[CHANNEL_START_INDEX]),
                            Integer.parseInt(values[CHANNEL_STEP_INDEX]),
                            Integer.parseInt(values[CHANNEL_STOP_INDEX]),
                            Integer.parseInt(values[POWER_DEFAULT_INDEX]),
                            Integer.parseInt(values[TX_ON_FLAG_REF]),
                            Integer.parseInt(values[TX_ON_FLAG_NPT]),
                            Integer.parseInt(values[REPEAT_TIMES]),
                            BandItem.BandType.BAND_LTE,
                            Integer.parseInt(values[RX_BW_TIMES]),
                            Integer.parseInt(values[TX_BW_TIMES]),
                            Integer.parseInt(values[RB_START_TIMES]),
                            Integer.parseInt(values[RB_LENGTH_TIMES]),
                            Integer.parseInt(values[UPLINK_MIN_FREQ]),
                            Integer.parseInt(values[DOWNLINK_MIN_FREQ]),
                            Integer.parseInt(values[ANT_STATUS])
                    )
            );
        }
    }

    private static void initCdmaItems(Context context, long bitsSupported) {
        String[] bandDefault = context.getResources().getStringArray(
                R.array.npt_cdma_data);
        Elog.i(TAG, "initCdmaItems bitsSupported: " + bitsSupported);
        int bandAarayLength = bandDefault.length;

        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsSupported, Integer.parseInt(values[BAND_VALUE_INDEX])))
                continue;
            sCdmaItems.add(new BandItem(values[BAND_NAME_INDEX],
                    Integer.parseInt(values[BAND_VALUE_INDEX]),
                    Integer.parseInt(values[CHANNEL_START_INDEX]),
                    Integer.parseInt(values[CHANNEL_STEP_INDEX]),
                    Integer.parseInt(values[CHANNEL_STOP_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX]),
                    Integer.parseInt(values[TX_ON_FLAG_REF]),
                    Integer.parseInt(values[TX_ON_FLAG_NPT]),
                    Integer.parseInt(values[REPEAT_TIMES]),
                    BandItem.BandType.BAND_CDMA,
                    Integer.parseInt(values[UPLINK_MIN_FREQ]),
                    Integer.parseInt(values[DOWNLINK_MIN_FREQ]),
                    Integer.parseInt(values[ANT_STATUS])
            ));
        }
    }

    private static void initEvdoItems(Context context, long bitsSupported) {
        String[] bandDefault = context.getResources().getStringArray(
                R.array.npt_evdo_data);
        Elog.i(TAG, "initEvdoItems bitsSupported: " + bitsSupported);
        int bandAarayLength = bandDefault.length;

        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsSupported, Integer.parseInt(values[BAND_VALUE_INDEX])))
                continue;
            sEvdoItems.add(new BandItem(values[BAND_NAME_INDEX],
                    Integer.parseInt(values[BAND_VALUE_INDEX]),
                    Integer.parseInt(values[CHANNEL_START_INDEX]),
                    Integer.parseInt(values[CHANNEL_STEP_INDEX]),
                    Integer.parseInt(values[CHANNEL_STOP_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX]),
                    Integer.parseInt(values[TX_ON_FLAG_REF]),
                    Integer.parseInt(values[TX_ON_FLAG_NPT]),
                    Integer.parseInt(values[REPEAT_TIMES]),
                    BandItem.BandType.BAND_EVDO,
                    Integer.parseInt(values[UPLINK_MIN_FREQ]),
                    Integer.parseInt(values[DOWNLINK_MIN_FREQ]),
                    Integer.parseInt(values[ANT_STATUS])
            ));
        }
    }

    public static void initRfConfigList(Context context, long[] bandSupported) {

        initGsmItems(context, bandSupported[INDEX_GSM_BAND]);
        if (ModemCategory.getModemType() == ModemCategory.MODEM_TD) {
            bandSupported[INDEX_UMTS_TDSCDMA_BAND] = bandSupported[INDEX_UMTS_WCDMA_BAND];
            initTddItems(context, bandSupported[INDEX_UMTS_TDSCDMA_BAND]);
            bandSupported[INDEX_UMTS_WCDMA_BAND] = 147;
            initFddItems(context, bandSupported[INDEX_UMTS_WCDMA_BAND]);
        } else if (ModemCategory.getModemType() == ModemCategory.MODEM_FDD) {
            initFddItems(context, bandSupported[INDEX_UMTS_WCDMA_BAND]);
            bandSupported[INDEX_UMTS_TDSCDMA_BAND] = 33;
            initTddItems(context, bandSupported[INDEX_UMTS_TDSCDMA_BAND]);
        }
        if (ModemCategory.isLteSupport()) {
            initLteItems(context, bandSupported[INDEX_LTE_FDD_BAND],
                    bandSupported[INDEX_LTE_TDD_BAND]);
        }
        if (ModemCategory.isCdma()) {
            initCdmaItems(context, bandSupported[INDEX_CDMA_BAND]);
            initEvdoItems(context, bandSupported[INDEX_CDMA_BAND]);
        }
    }

    static BandItem findItem(List<BandItem> items, String value) {
        String bandName = value.split(",")[0];
        for (int i = 0; i < items.size(); i++) {
            if (bandName.equals(items.get(i).getmBandName())) {
                return items.get(i);
            }
        }
        return null;
    }

    static void settings_load() {
        for (String value : sInputMapping.get(InputMappingKey[7])) {
            Elog.i(TAG, "settings = " + value);
            try {
                String[] settings = value.split(",");
                NoiseProfilingActivity.mRssiThreshold0 = Integer.parseInt(settings[1].trim());
                NoiseProfilingActivity.mRssiDelta0 = Integer.parseInt(settings[2].trim());
                NoiseProfilingActivity.mRssiThreshold1 = Integer.parseInt(settings[3].trim());
                NoiseProfilingActivity.mRssiDelta1 = Integer.parseInt(settings[4].trim());
                NoiseProfilingActivity.mDelayTime = Integer.parseInt(settings[5].trim());
                NoiseProfilingActivity.mNptPreScanTime = Integer.parseInt(settings[6].trim());
                NoiseProfilingActivity.mScanMode = Integer.parseInt(settings[7].trim());
                NoiseProfilingActivity.mAntMode = Integer.parseInt(settings[8].trim());
                NoiseProfilingActivity.mAntStatus = Integer.parseInt(settings[9].trim());
            } catch (Exception e) {
                Elog.e(TAG, "the settings values format error!");
            }
        }
    }

    static void npt_cmd_load() {
        for (String value : sInputMapping.get(InputMappingKey[8])) {
            Elog.i(TAG, "PreNPT = " + value);
            try {
                NoiseProfilingActivity.mScanCmdPreNpt = value.substring(7);
            } catch (Exception e) {
                Elog.e(TAG, "the settings values format error!");
                NoiseProfilingActivity.mScanCmdPreNpt = NoiseProfilingActivity
                        .SCAN_CMD_PRE_NPT_DEFAULT;
            }
        }

        for (String value : sInputMapping.get(InputMappingKey[9])) {
            Elog.i(TAG, "PostNPT = " + value);
            try {
                NoiseProfilingActivity.mScanCmdPostNpt = value.substring(8);
            } catch (Exception e) {
                NoiseProfilingActivity.mScanCmdPostNpt = NoiseProfilingActivity
                        .SCAN_CMD_POST_NPT_DEFAULT;
                Elog.e(TAG, "the settings values format error!");
            }
        }

        for (String value : sInputMapping.get(InputMappingKey[10])) {
            Elog.i(TAG, "PostREF = " + value);
            try {
                NoiseProfilingActivity.mScanCmdPreRef = value.substring(7);
            } catch (Exception e) {
                NoiseProfilingActivity.mScanCmdPreRef = NoiseProfilingActivity
                        .SCAN_CMD_PRE_REF_DEFAULT;
                Elog.e(TAG, "the settings values format error!");
            }
        }
    }

    public static int initItemsFromInput() {
        int result = parse();
        if (result < 0) {
            return result;
        }

        for (BandItem item : sGsmItems) {
            item.setmSelected(false);
        }
        for (BandItem item : sTddItems) {
            item.setmSelected(false);
        }
        for (BandItem item : sFddItems) {
            item.setmSelected(false);
        }
        for (BandItem item : sLteItems) {
            item.setmSelected(false);
        }
        for (BandItem item : sCdmaItems) {
            item.setmSelected(false);
        }
        for (BandItem item : sEvdoItems) {
            item.setmSelected(false);
        }


        for (int i = 0; i < InputMappingKey.length; i++) {
            ArrayList bandDefault = sInputMapping.get(InputMappingKey[i]);
            if (bandDefault == null) {
                Elog.i(TAG, InputMappingKey[i] + " not found!");
                continue;
            }

            if (i == 7) {       //settings,value
                settings_load();
                continue;
            }

            if (i == 8 || i == 9 || i == 10) {       //settings,value
                npt_cmd_load();
                continue;
            }

            for (String value : sInputMapping.get(InputMappingKey[i])) {
                BandItem item = null;

                if (i == 0) { //GSM
                    item = findItem(sGsmItems, value);
                } else if (i == 1) {//tdscdma
                    item = findItem(sTddItems, value);
                } else if (i == 2) {//WCDMA
                    item = findItem(sFddItems, value);
                } else if ((i == 3) || (i == 4)) {//LTE
                    item = findItem(sLteItems, value);
                } else if (i == 5) {//cdma
                    item = findItem(sCdmaItems, value);
                } else if (i == 6) {//evdo
                    item = findItem(sEvdoItems, value);
                }
                if (item == null) {
                    Elog.e(TAG, "the input value not support by phone: " + value);
                    continue;
                }
                String[] values = value.split(",");
                if (values.length != 13) {
                    Elog.e(TAG, "the input num is error");
                    return -2;
                }

                int[] channels = new int[3];
                try {
                    channels[0] = Integer.parseInt(values[CHANNEL_START_INDEX - 1].trim());
                    channels[1] = Integer.parseInt(values[CHANNEL_STEP_INDEX - 1].trim());
                    channels[2] = Integer.parseInt(values[CHANNEL_STOP_INDEX - 1].trim());
                    item.setmChannleScope(channels);
                    item.setmTxPowerValue(Integer.parseInt(values[POWER_DEFAULT_INDEX - 1].trim()));
                    item.setmTxOnflagValueRef(Integer.parseInt(values[TX_ON_FLAG_REF - 1].trim()));
                    item.setmTxOnflagValueNpt(Integer.parseInt(values[TX_ON_FLAG_NPT - 1].trim()));
                    item.setmRepeatTimesValue(Integer.parseInt(values[REPEAT_TIMES - 1].trim()));
                    item.setmRxBwValue(Integer.parseInt(values[RX_BW_TIMES - 1].trim()));
                    item.setmTxBwValue(Integer.parseInt(values[TX_BW_TIMES - 1].trim()));
                    item.setmRbStartValue(Integer.parseInt(values[RB_START_TIMES - 1].trim()));
                    item.setmRblengthValue(Integer.parseInt(values[RB_LENGTH_TIMES - 1].trim()));
                    item.setmAntennaStateValue(Integer.parseInt(values[ANT_STATUS - 3].trim()));
                    item.setSelected(true);
                } catch (Exception e) {
                    Elog.e(TAG, "the input format is error");
                    return -2;
                }
            }
        }
        return result;
    }


    protected static void getSupportedBand(String[] strInfo) {
        for (int i = 0; i < INDEX_BAND_MAX; ++i)
            bandSupported[i] = 0;
        for (final String value : strInfo) {
            Elog.i(TAG, "getSupportedBand strInfo: " + value);
            if (!value.substring(0, SAME_COMMAND.length()).equals(SAME_COMMAND))
                continue;
            final String splitString = value.substring(SAME_COMMAND.length());
            final String[] getDigitalVal = splitString.split(",");
            if (getDigitalVal != null && getDigitalVal.length > 1) {
                for (int i = 0; i < bandSupported.length; i++) {
                    if (i >= getDigitalVal.length || getDigitalVal[i] == null) {
                        bandSupported[i] = 0;
                        continue;
                    }
                    try {
                        bandSupported[i] = Long.valueOf(getDigitalVal[i].trim());
                        Elog.i(TAG, "getSupportedBand #" + i + ": " + bandSupported[i]);
                    } catch (NumberFormatException e) {
                        bandSupported[i] = 0;
                    }
                }
                break;
            }
        }
    }

    private static void doParse(BufferedReader in) throws IOException {
        for (int i = 0; i < InputMappingKey.length; i++) {
            ArrayList<String> values = new ArrayList<String>();
            sInputMapping.put(InputMappingKey[i], values);
        }

        String line = in.readLine();
        while (line != null) {
            line = line.trim();
            for (int i = 0; i < InputMappingKey.length; i++) {
                if (line.startsWith(InputMappingKey[i])) {
                    sInputMapping.get(InputMappingKey[i]).add(line);
                    break;
                }
            }
            line = in.readLine();
        }

        for (int i = 0; i < InputMappingKey.length; i++) {
            for (String value : sInputMapping.get(InputMappingKey[i])) {
                Elog.d(TAG, "value = " + value);
            }
        }

    }

    private static int parse() {
        FileInputStream inputStream = null;
        try {
            File file = new File(NoiseProfilingFileSave.getNptInputFileName());
            inputStream = new FileInputStream(file);
            doParse(new BufferedReader(new InputStreamReader(inputStream)));
            inputStream.close();
        } catch (FileNotFoundException e) {
            Elog.e(TAG, "Config file not found.");
            return -1;
        } catch (IOException e) {
            Elog.e(TAG, "Read config file error");
            return -2;
            //    EmUtils.showToast(this, "Read config file error.", Toast.LENGTH_SHORT);
        } finally {
            if (null != inputStream) {
                try {
                    inputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return 0;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.noise_profiling_band_config);
        Intent intent = getIntent();
        mRatType = intent.getIntExtra("mRatType", -1);
        viewGsm = findViewById(R.id.gsmCategory);
        viewTdscdma = findViewById(R.id.tddCategory);
        viewWcdma = findViewById(R.id.fddCategory);
        viewCdma = findViewById(R.id.cdmaCategory);
        viewLte = findViewById(R.id.lteCategory);
        viewGsm.setVisibility(View.GONE);
        viewTdscdma.setVisibility(View.GONE);
        viewWcdma.setVisibility(View.GONE);
        viewCdma.setVisibility(View.GONE);
        viewLte.setVisibility(View.GONE);

        updateListView();
    }

    private void updateEditItem() {
        if (sItemClicked == null) {
            return;
        }
        BandItem.BandType type = sItemClicked.getmBandType();
        if (type == BandItem.BandType.BAND_GSM) {
            Elog.d(TAG, "edit gsm");
            mGsmAdapter.notifyDataSetChanged();
        } else if (type == BandItem.BandType.BAND_WCDMA) {
            Elog.d(TAG, "edit wcdma");
            mFddAdapter.notifyDataSetChanged();
        } else if (type == BandItem.BandType.BAND_TD) {
            Elog.d(TAG, "edit td");
            mTddAdapter.notifyDataSetChanged();
        } else if (type == BandItem.BandType.BAND_LTE) {
            Elog.d(TAG, "edit lte");
            mLteAdapter.notifyDataSetChanged();
        } else if (type == BandItem.BandType.BAND_CDMA) {
            Elog.d(TAG, "edit cdma");
            mCdmaAdapter.notifyDataSetChanged();
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        updateEditItem();
    }


    private void setListViewHeightBasedOnChildren(ListView listView) {
        ListAdapter listAdapter = listView.getAdapter();
        if (listAdapter == null) {
            return;
        }
        int totalHeight = 0;
        int count = listAdapter.getCount();
        for (int i = 0; i < count; i++) {
            View listItem = listAdapter.getView(i, null, listView);
            listItem.measure(0, 0);
            totalHeight += listItem.getMeasuredHeight();
        }
        ViewGroup.LayoutParams params = listView.getLayoutParams();
        params.height = totalHeight + (listView.getDividerHeight()
                * (listAdapter.getCount() - 1));
        listView.setLayoutParams(params);
    }

    private void updateListView() {

        if (mRatType == RAT_TYPE_GSM) {
            viewGsm.setVisibility(View.VISIBLE);
            mGsmAdapter = new BandAdapter(this);
            mGsmList = (ListView) findViewById(R.id.desense_at_gsm_list);
            mGsmList.setAdapter(mGsmAdapter);
            mGsmAdapter.setListView(mGsmList);
            mGsmAdapter.clear();
            mGsmAdapter.addAll(sGsmItems);
            mGsmList.setOnItemClickListener(mCommonListener);
            mGsmAdapter.notifyDataSetChanged();
            setListViewHeightBasedOnChildren(mGsmList);
        }

        if (mRatType == RAT_TYPE_WCDMA && sFddItems != null) {
            viewWcdma.setVisibility(View.VISIBLE);
            mFddAdapter = new BandAdapter(this);
            mFddList = (ListView) findViewById(R.id.desense_at_fdd_list);
            mFddList.setAdapter(mFddAdapter);
            mFddAdapter.setListView(mFddList);
            mFddAdapter.clear();
            mFddAdapter.addAll(sFddItems);
            setListViewHeightBasedOnChildren(mFddList);
            mFddList.setOnItemClickListener(mCommonListener);
            mFddAdapter.notifyDataSetChanged();
        }
        if (mRatType == RAT_TYPE_TDSCDMA && sTddItems != null) {
            viewTdscdma.setVisibility(View.VISIBLE);
            mTddAdapter = new BandAdapter(this);
            mTddList = (ListView) findViewById(R.id.desense_at_tdd_list);
            mTddList.setAdapter(mTddAdapter);
            mTddAdapter.setListView(mTddList);
            mTddAdapter.clear();
            mTddAdapter.addAll(sTddItems);
            setListViewHeightBasedOnChildren(mTddList);
            mTddList.setOnItemClickListener(mCommonListener);
            mTddAdapter.notifyDataSetChanged();
        }

        if (mRatType == RAT_TYPE_LTE && sLteItems != null) {
            viewLte.setVisibility(View.VISIBLE);
            mLteAdapter = new BandAdapter(this);
            mLteList = (ListView) findViewById(R.id.desense_at_lte_list);
            mLteList.setAdapter(mLteAdapter);
            mLteAdapter.setListView(mLteList);
            mLteAdapter.clear();
            mLteAdapter.addAll(sLteItems);
            setListViewHeightBasedOnChildren(mLteList);
            mLteList.setOnItemClickListener(mCommonListener);
            mLteAdapter.notifyDataSetChanged();
        }

        if ((mRatType == RAT_TYPE_CDMD_1X) && sCdmaItems != null) {
            viewCdma.setVisibility(View.VISIBLE);
            mCdmaAdapter = new BandAdapter(this);
            mCdmaList = (ListView) findViewById(R.id.desense_at_cdma_list);
            mCdmaList.setAdapter(mCdmaAdapter);
            mCdmaAdapter.setListView(mCdmaList);
            mCdmaAdapter.clear();
            mCdmaAdapter.addAll(sCdmaItems);
            setListViewHeightBasedOnChildren(mCdmaList);
            mCdmaList.setOnItemClickListener(mCommonListener);
            mCdmaAdapter.notifyDataSetChanged();
        }

        if ((mRatType == RAT_TYPE_CDMD_EVDO) && sEvdoItems != null) {
            viewCdma.setVisibility(View.VISIBLE);
            mCdmaAdapter = new BandAdapter(this);
            mCdmaList = (ListView) findViewById(R.id.desense_at_cdma_list);
            mCdmaList.setAdapter(mCdmaAdapter);
            mCdmaAdapter.setListView(mCdmaList);
            mCdmaAdapter.clear();
            mCdmaAdapter.addAll(sEvdoItems);
            setListViewHeightBasedOnChildren(mCdmaList);
            mCdmaList.setOnItemClickListener(mCommonListener);
            mCdmaAdapter.notifyDataSetChanged();
        }
    }


    private class BandAdapter extends ArrayAdapter<BandItem> {

        private ListView mListView;

        BandAdapter(Context activity) {
            super(activity, 0);
        }

        public void setListView(ListView listview) {
            mListView = listview;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            int visiblePosition = mListView.getFirstVisiblePosition();
            if (position - visiblePosition < 0) {
                return null;
            }
            ViewHolder holder;
            LayoutInflater inflater = BandConfigActivity.this.getLayoutInflater();
            if (convertView == null) {
                convertView = inflater.inflate(R.layout.noise_profiling_band_entry, null);
                holder = new ViewHolder();
                holder.summary = new TextView[3];
                holder.title = (TextView) convertView.findViewById(R.id.title);
                holder.summary[0] = (TextView) convertView.findViewById(R.id.summary1);
                holder.summary[1] = (TextView) convertView.findViewById(R.id.summary2);
                holder.summary[2] = (TextView) convertView.findViewById(R.id.summary3);
                holder.checkbox = (CheckBox) convertView.findViewById(R.id.band_checkbox);
                final int pos = position;
                holder.checkbox.setOnCheckedChangeListener(new CheckBox.OnCheckedChangeListener() {
                    @Override
                    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                        getItem(pos).setSelected(isChecked);
                    }
                });
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }
            BandItem testItem = getItem(position);
            holder.title.setText(testItem.getmBandName());
            holder.summary[0].setText(testItem.getSummary1());
            holder.summary[1].setText(testItem.getSummary2());
            holder.summary[2].setText(testItem.getSummary3());
            holder.checkbox.setChecked(testItem.isSelected());
            return convertView;
        }

        private class ViewHolder {
            public CheckBox checkbox;
            public TextView title;
            public TextView []summary;
        }
    }

}
