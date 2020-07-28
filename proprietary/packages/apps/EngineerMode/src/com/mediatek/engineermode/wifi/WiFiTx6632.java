package com.mediatek.engineermode.wifi;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.HashMap;

/**
 * Class for 6632 wifi chip TX test.
 *
 */
public class WiFiTx6632 extends WiFiTestActivity implements OnClickListener,
        OnCheckedChangeListener {
    private static final String TAG = "WifiTx";
    private static final int TEST_MODE_TX = 0;
    private static final int TEST_MODE_DUTY = 1;
    private static final int TEST_MODE_CW_TONE = 2;
    private static final int DEFAULT_PKT_CNT = 0;
    private static final int DEFAULT_PKT_LEN = 1024;
    private static final int ONE_SENCOND = 1000;
    private static final int DEFAULT_TX_GAIN = 22;
    private static final int RATE_MODE_MASK = 31;
    private static final int POWER_UNIT_DBM = 0;
    private static final int COMMAND_INDEX_STOPTEST = 0;
    private static final int COMMAND_INDEX_STARTTX = 1;
    private static final int COMMAND_INDEX_OUTPUTPOWER = 4;
    private static final int COMMAND_INDEX_LOCALFREQ = 5;
    private static final int DIALOG_NO_PATH_SELECT = 1;
    private static final String[] mMode = { "continuous packet tx", "100% duty cycle",
        "CW tone" };
    private static final String[] mPreamble = { "Normal", "CCK short", "802.11n mixed mode",
            "802.11n green field", "802.11ac" };
    private static final String[] mGuardInterval = { "normal GI", "short GI" };
    private static final String[] sBandWidth = { "5MHz", "10MHz", "20MHz", "40MHz", "80MHz",
            "160MHz", "160NC" };
    private static final String[] sBandWidthDBW = { "BW5", "BW10", "BW20", "BW40", "BW80",
            "BW160" };
    private static final HashMap<String, Integer> sMapDbwValue = new HashMap<String, Integer>() {
        {
            put("BW5", 5);
            put("BW10", 6);
            put("BW20", 0);
            put("BW40", 1);
            put("BW80", 2);
            put("BW160", 3);
        }

    };
    private static final int BAND_WIDTH_160NC_INDEX = 6;
    private static final int BAND_WIDTH_20_INDEX = 2;
    private static final int BAND_WIDTH_40_INDEX = 3;
    private static final int BAND_WIDTH_80_INDEX = 4;
    private static final int BAND_WIDTH_160_INDEX = 5;
    private static final int HANDLER_EVENT_GO = 1;
    private static final int HANDLER_EVENT_STOP = 2;
    private static final int HANDLER_EVENT_TIMER = 3;
    private static final int HANDLER_EVENT_FINISH = 4;

    private static final int WIFI_MODE_NORMAL = 1;
    private static final int WIFI_MODE_DBDC_TX0 = 2;
    private static final int WIFI_MODE_DBDC_TX1 = 3;

    private RadioGroup mRGNss = null;
    private TextView mTvNss = null;

    private CheckBox mCkWf0 = null;
    private CheckBox mCkWf1 = null;
    private CheckBox mCkDuplicateMode = null;

    private Spinner mSpChannelTx0 = null;
    private Spinner mSpChannelTx1 = null;
    private TextView mTvDbw = null;
    private TextView mTvPrimCh = null;
    private Spinner mSpDbw = null;
    private Spinner mSpPrimCh = null;
    private Spinner mSpGuardInterval = null;
    private Spinner mSpBandwidth = null;
    private Spinner mSpPreamble = null;
    private EditText mEtPkt = null;
    private EditText mEtPktCnt = null;
    private EditText mEtTxGain = null;
    private Spinner mSpRate = null;
    private RadioButton mRbNss1 = null;
    private RadioButton mRbNss2 = null;
    private Spinner mSpMode = null;
    private Button mBtnGo = null;
    private Button mBtnStop = null;
    private View mViewChannelTx1 = null;
    private HandlerThread mTestThread = null;
    private Handler mEventHandler = null;
    private ChannelInfo mChannel = null;
    private RateInfo mRate = null;
    private ArrayAdapter<String> mChannelAdapter = null;
    private ArrayAdapter<String> mModeAdapter = null;
    private ArrayAdapter<String> mPreambleAdapter = null;
    private ArrayAdapter<String> mRateAdapter = null;
    private ArrayAdapter<String> mDbwAdapter = null;
    private ArrayAdapter<String> mPrimChAdapter = null;
    private int mBandwidthIndex = 0;
    private int mDataBandwidthValue = 0;
    private int mPrimaryIndex = 0;
    private int mModeIndex = 0;
    private int mPreambleIndex = 0;
    private int mGuardIntervalIndex = 0;
    private int mChannelTx0Freq;
    private int mChannelTx1Freq;
    private int mNssValue = 1;
    private long mPktLenNum = DEFAULT_PKT_LEN;
    private long mCntNum = DEFAULT_PKT_CNT;
    private long mTxGainVal = DEFAULT_TX_GAIN;
    private boolean mHighRateSelected = false;
    private boolean mTestInProcess = false;
    private int mTargetModeIndex = 0;
    private ViewGroup mVgAnt = null;
    private RadioButton mRbAntMain = null;

    private int mWiFiMode = WIFI_MODE_NORMAL;

    private final Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            if (HANDLER_EVENT_FINISH == msg.what) {
                Elog.v(TAG, "receive HANDLER_EVENT_FINISH");
                setViewEnabled(true);
            }
        }
    };

    /**
     * Rate information management.
     *
     */
    static class RateInfo {
        private static final int MAX_LOWER_RATE_NUMBER = 12;
        private static final short EEPROM_RATE_GROUP_CCK = 0;
        private static final short EEPROM_RATE_GROUP_OFDM_6_9M = 1;
        private static final short EEPROM_RATE_GROUP_OFDM_12_18M = 2;
        private static final short EEPROM_RATE_GROUP_OFDM_24_36M = 3;
        private static final short EEPROM_RATE_GROUP_OFDM_48_54M = 4;
        private static final short EEPROM_RATE_GROUP_OFDM_MCS0_32 = 5;


        int mRateIndex = 0;

        int mOFDMStartIndex = 4;

        private final short[] mUcRateGroupEep = { EEPROM_RATE_GROUP_CCK,
                EEPROM_RATE_GROUP_CCK, EEPROM_RATE_GROUP_CCK, EEPROM_RATE_GROUP_CCK,
                EEPROM_RATE_GROUP_OFDM_6_9M, EEPROM_RATE_GROUP_OFDM_6_9M,
                EEPROM_RATE_GROUP_OFDM_12_18M, EEPROM_RATE_GROUP_OFDM_12_18M,
                EEPROM_RATE_GROUP_OFDM_24_36M, EEPROM_RATE_GROUP_OFDM_24_36M,
                EEPROM_RATE_GROUP_OFDM_48_54M, EEPROM_RATE_GROUP_OFDM_48_54M,
                /* for future use */
                EEPROM_RATE_GROUP_OFDM_MCS0_32, EEPROM_RATE_GROUP_OFDM_MCS0_32,
                EEPROM_RATE_GROUP_OFDM_MCS0_32, EEPROM_RATE_GROUP_OFDM_MCS0_32,
                EEPROM_RATE_GROUP_OFDM_MCS0_32, EEPROM_RATE_GROUP_OFDM_MCS0_32,
                EEPROM_RATE_GROUP_OFDM_MCS0_32, EEPROM_RATE_GROUP_OFDM_MCS0_32,
                EEPROM_RATE_GROUP_OFDM_MCS0_32, EEPROM_RATE_GROUP_OFDM_MCS0_32,
                EEPROM_RATE_GROUP_OFDM_MCS0_32, };

        static final String[] RATE_ITEM_54M = { "1M", "2M", "5.5M", "11M", "6M", "9M", "12M",
                "18M", "24M", "36M", "48M", "54M" };
        static final String[] RATE_ITEM_11M = { "1M", "2M", "5.5M", "11M" };
        static final String[] RATE_ITEM_CS7 = { "MCS0", "MCS1", "MCS2", "MCS3", "MCS4",
                "MCS5", "MCS6", "MCS7" };
        static final String[] RATE_ITEM_CS15 = { "MCS0", "MCS1", "MCS2", "MCS3", "MCS4",
                "MCS5", "MCS6", "MCS7", "MCS8", "MCS9", "MCS10", "MCS11", "MCS12", "MCS13",
                "MCS14", "MCS15" };

        int getRateGroup(int rateIndex) {
            int group = -1;
            if (rateIndex >= 0 && rateIndex < mUcRateGroupEep.length) {
                group = mUcRateGroupEep[rateIndex];
            }
            return group;
        }
    }

    private void initUiLayout() {
        mBtnGo = (Button) findViewById(R.id.WiFi_Go);
        mBtnStop = (Button) findViewById(R.id.WiFi_Stop);
        mBtnGo.setOnClickListener(this);
        mBtnStop.setOnClickListener(this);
        mRGNss = (RadioGroup) findViewById(R.id.wifi_tx_nss);
        mRGNss.setOnCheckedChangeListener(this);
        mSpBandwidth = (Spinner) findViewById(R.id.wifi_bandwidth_spinner);
        mSpRate = (Spinner) findViewById(R.id.WiFi_Rate_Spinner);

        mTvNss = (TextView) findViewById(R.id.wifi_nss_text);
        mRbNss1 = (RadioButton) findViewById(R.id.wifi_tx_nss_1);
        mRbNss2 = (RadioButton) findViewById(R.id.wifi_tx_nss_2);
        mTvDbw = (TextView) findViewById(R.id.wifi_bandwidth_dbw_tv);
        mSpDbw = (Spinner) findViewById(R.id.wifi_bandwidth_dbw_spn);
        mTvPrimCh = (TextView) findViewById(R.id.wifi_bandwidth_prim_ch_tv);
        mSpPrimCh = (Spinner) findViewById(R.id.wifi_bandwidth_prim_ch_spn);
        mEtPkt = (EditText) findViewById(R.id.WiFi_Pkt_Edit);
        mEtPktCnt = (EditText) findViewById(R.id.WiFi_Pktcnt_Edit);
        mEtTxGain = (EditText) findViewById(R.id.WiFi_Tx_Gain_Edit); // Tx gain
        mSpMode = (Spinner) findViewById(R.id.WiFi_Mode_Spinner);
        mSpPreamble = (Spinner) findViewById(R.id.WiFi_Preamble_Spinner);
        mSpGuardInterval = (Spinner) findViewById(R.id.WiFi_Guard_Interval_Spinner);
        mCkWf0 = (CheckBox) findViewById(R.id.wifi_wfx_0);

        mCkWf1 = (CheckBox) findViewById(R.id.wifi_wfx_1);
        mCkDuplicateMode = (CheckBox) findViewById(R.id.wifi_duplicate_mode_tx);
        mCkDuplicateMode.setOnCheckedChangeListener(mCheckedListener);
        mCkWf0.setOnCheckedChangeListener(mCheckedListener);
        mCkWf1.setOnCheckedChangeListener(mCheckedListener);
        mCkWf0.setChecked(true);

        mSpChannelTx0 = (Spinner) findViewById(R.id.wifi_tx0_channel_spinner);
        mSpChannelTx1 = (Spinner) findViewById(R.id.wifi_tx1_channel_spinner);
        mViewChannelTx1 = findViewById(R.id.wifi_tx1_channel_layout);
        mChannel = new ChannelInfo();

        mEtPkt.setText(DEFAULT_PKT_LEN + "");
        mEtPktCnt.setText(DEFAULT_PKT_CNT + "");
        mEtTxGain.setText(DEFAULT_TX_GAIN + "");

        mVgAnt = (ViewGroup) findViewById(R.id.wifi_ant_vg);
        mRbAntMain = (RadioButton) findViewById(R.id.wifi_tx_ant_main);
    }

    private void initUiComponent() {
        // Channel Spinner init
        mChannelAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        mChannelAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mChannel.addSupported2dot4gChannels(mChannelAdapter, false);

        // Tx0 Channel Spinner init
        mSpChannelTx0.setAdapter(mChannelAdapter);
        mSpChannelTx0.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (EMWifi.sIsInitialed) {
                    String name = mChannelAdapter.getItem(position);
                    int channelId = ChannelInfo.parseChannelId(name);
                    mChannelTx0Freq = ChannelInfo.getChannelFrequency(channelId);
                    updateTxPower();
                } else {
                    showDialog(DIALOG_WIFI_ERROR);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });
        // Tx1 Channel Spinner init
        mSpChannelTx1.setAdapter(mChannelAdapter);
        mSpChannelTx1.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (EMWifi.sIsInitialed) {
                    String name = mChannelAdapter.getItem(position);
                    int channelId = ChannelInfo.parseChannelId(name);
                    mChannelTx1Freq = ChannelInfo.getChannelFrequency(channelId);
                } else {
                    showDialog(DIALOG_WIFI_ERROR);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });

        // Tx Bandwidth settings
        ArrayAdapter<String> bandwidthAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        bandwidthAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        int size = sBandWidth.length;

        if (mWiFiMode == WIFI_MODE_DBDC_TX0) {
            size = sBandWidth.length - 3;
        }

        for (int i = 0; i < size; i++) {
            bandwidthAdapter.add(sBandWidth[i]);
        }
        mSpBandwidth.setAdapter(bandwidthAdapter);
        mSpBandwidth.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                mBandwidthIndex = arg2;
                mTvPrimCh.setEnabled(true);
                mSpPrimCh.setEnabled(true);
                mTvDbw.setEnabled(true);
                mSpDbw.setEnabled(true);
                mTvPrimCh.setVisibility(View.VISIBLE);
                mSpPrimCh.setVisibility(View.VISIBLE);
                if (arg2 < BAND_WIDTH_20_INDEX) {
                    mTvPrimCh.setVisibility(View.GONE);
                    mSpPrimCh.setVisibility(View.GONE);
                    mDbwAdapter.clear();
                    mDbwAdapter.add(sBandWidthDBW[arg2]);
                    mViewChannelTx1.setVisibility(View.GONE);
                    mSpDbw.setAdapter(mDbwAdapter);
                } else if (arg2 <= BAND_WIDTH_160_INDEX) {
                    mDbwAdapter.clear();
                    for (int i = BAND_WIDTH_20_INDEX; i <= arg2; i++) {
                        mDbwAdapter.add(sBandWidthDBW[i]);
                    }
                    mSpDbw.setAdapter(mDbwAdapter);
                    mPrimChAdapter.clear();
                    int maxPrimCh = (int) Math.pow(2, arg2 - BAND_WIDTH_20_INDEX) - 1;
                    for (int i = 0; i <= maxPrimCh; i++) {
                        mPrimChAdapter.add(String.valueOf(i));
                    }
                    mSpPrimCh.setAdapter(mPrimChAdapter);
                    mViewChannelTx1.setVisibility(View.GONE);
                } else {
                    mSpPrimCh.setEnabled(false);
                    mSpDbw.setEnabled(false);
                    mViewChannelTx1.setVisibility(View.VISIBLE);
                    mDbwAdapter.clear();
                    for (int i = BAND_WIDTH_20_INDEX; i < arg2; i++) {
                        mDbwAdapter.add(sBandWidthDBW[i]);
                    }
                    mSpDbw.setAdapter(mDbwAdapter);
                    mSpDbw.setSelection(3); // 160

                    mPrimChAdapter.clear();
                    mPrimChAdapter.add("0");
                    mSpPrimCh.setAdapter(mPrimChAdapter);
                }

                updateRate();


            }

            public void onNothingSelected(AdapterView<?> arg0) {

            }
        });

        // TX DBW settings
        mDbwAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        mDbwAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpDbw.setOnItemSelectedListener(new OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                String content = ((TextView) view).getText().toString();
                Integer value = sMapDbwValue.get(content);
                Elog.i(TAG, "dbw value:" + value);
                if ((value == null)
                        || (value.intValue() == mDataBandwidthValue)) {
                    return;
                }

                mDataBandwidthValue = value;
                updateTxPower();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        // TX prim settings
        mPrimChAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        mPrimChAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpPrimCh.setOnItemSelectedListener(new OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (position == mPrimaryIndex) {
                    return;
                }
                mPrimaryIndex = position;

            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        // TX mSpRate settings
        mRate = new RateInfo();
        mRateAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        mRateAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        mSpRate.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (!EMWifi.sIsInitialed) {
                    showDialog(DIALOG_WIFI_ERROR);
                    return;
                }
                int oldRate = mRate.mRateIndex;
                mRate.mRateIndex = arg2;
                if (mPreambleIndex == 2 || mPreambleIndex == 3 || mPreambleIndex == 4) {
                    mRate.mRateIndex = arg2 + 12;
                }
                updateChannels();
                if (oldRate != mRate.mRateIndex) {
                    updateTxPower();
                }

            }

            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });

        // Tx mPreamble 802.11n select seetings
        mPreambleAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        mPreambleAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        int sizePreamble = mPreamble.length;
        for (int i = 0; i < sizePreamble; i++) {
            mPreambleAdapter.add(mPreamble[i]);
        }
        mSpPreamble.setAdapter(mPreambleAdapter);
        mSpPreamble.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                mPreambleIndex = arg2;
                updateRate();
                updateTxPower();
            }

            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });

        // Tx Mode settings
        mModeAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        mModeAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 0; i < mMode.length; i++) {
            mModeAdapter.add(mMode[i]);
        }
        mSpMode.setAdapter(mModeAdapter);
        mSpMode.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                mModeIndex = arg2;
            }

            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });

        // Guard Interval settings
        ArrayAdapter<String> guardIntervalAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        guardIntervalAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 0; i < mGuardInterval.length; i++) {
            guardIntervalAdapter.add(mGuardInterval[i]);
        }

        mSpGuardInterval.setAdapter(guardIntervalAdapter);
        mSpGuardInterval.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                mGuardIntervalIndex = arg2;
            }

            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });

        mTestThread = new HandlerThread("Wifi Tx Test");
        mTestThread.start();
        mEventHandler = new EventHandler(mTestThread.getLooper());

        setViewEnabled(true);

        mRbNss1.setChecked(true);

    }

    @Override
    public void onClick(View v) {
        // TODO Auto-generated method stub
        if (!EMWifi.sIsInitialed) {
            showDialog(DIALOG_WIFI_ERROR);
            return;
        }

        if (v.getId() == mBtnGo.getId()) {
            onClickBtnTxGo();
        } else if (v.getId() == mBtnStop.getId()) {
            onClickBtnTxStop();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        if (!EMWifi.sIsInitialed) {
            Elog.d(TAG, "sIsInitialed:" + EMWifi.sIsInitialed);
            Toast.makeText(this, R.string.wifi_dialog_error_message, Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        setContentView(R.layout.wifi_tx_6632);

        Intent intent = getIntent();
        String data = intent.getStringExtra("MODE");

        if (data == null) {
            mWiFiMode = WIFI_MODE_NORMAL;
        } else if (data.equals("TX Band0")) {
            mWiFiMode = WIFI_MODE_DBDC_TX0;
        } else if (data.equals("TX Band1")) {
            mWiFiMode = WIFI_MODE_DBDC_TX1;
        }

        initUiLayout();
        initUiComponent();

        if (mWiFiMode == WIFI_MODE_DBDC_TX0) {
            mCkWf0.setChecked(true);
            mRGNss.setVisibility(View.GONE);
            mCkWf0.setVisibility(View.GONE);
            mCkWf1.setVisibility(View.GONE);
            mCkDuplicateMode.setVisibility(View.GONE);
            mTvNss.setVisibility(View.GONE);
        } else if (mWiFiMode == WIFI_MODE_DBDC_TX1) {
            mCkWf1.setChecked(true);
            mRGNss.setVisibility(View.GONE);
            mCkWf0.setVisibility(View.GONE);
            mCkWf1.setVisibility(View.GONE);
            mCkDuplicateMode.setVisibility(View.GONE);
            mTvNss.setVisibility(View.GONE);
        }

        if (!EMWifi.isAntSwapSupport()) {
            mVgAnt.setVisibility(View.GONE);
        }
    }

    /**
     * Update channels.
     */
    private void updateChannels() {

        ArrayAdapter<String> tempChAdapter = new ArrayAdapter<String>(
                this, android.R.layout.simple_spinner_item);
        int targetBandwidth = mBandwidthIndex;
        Elog.d(TAG, "updateChannels");

        if (mBandwidthIndex < BAND_WIDTH_20_INDEX) {
            mChannel.insertBw20MChannels(tempChAdapter);
            mChannel.insertBw40MChannels(tempChAdapter);
            mChannel.insertBw80MChannels(tempChAdapter);
            mChannel.insertBw160MChannels(tempChAdapter);
        }

        if (mBandwidthIndex == BAND_WIDTH_20_INDEX) {
            mChannel.insertBw20MChannels(tempChAdapter);
        }

        if (mBandwidthIndex == BAND_WIDTH_40_INDEX) {
            mChannel.insertBw40MChannels(tempChAdapter);
        }
        if (mBandwidthIndex == BAND_WIDTH_80_INDEX) {
            mChannel.insertBw80MChannels(tempChAdapter);
        }
        if (mBandwidthIndex == BAND_WIDTH_160_INDEX) {
            mChannel.insertBw160MChannels(tempChAdapter);
        }
        if (mBandwidthIndex == BAND_WIDTH_160NC_INDEX) {
            mChannel.insertBw80MChannels(tempChAdapter);
        }

        if (mWiFiMode == WIFI_MODE_DBDC_TX0) {
            mChannel.remove5GChannels(tempChAdapter);
        } else if (mWiFiMode == WIFI_MODE_DBDC_TX1) {
            mChannel.remove2dot4GChannels(tempChAdapter);
        }

        updateChannelByRateBandwidth(tempChAdapter, mRate.mRateIndex, targetBandwidth);

        boolean bUpdateWifiChannel = false;
        int count = tempChAdapter.getCount();
        if (count == mChannelAdapter.getCount()) {
            for (int k = 0; k < count; k++) {
                if (!tempChAdapter.getItem(k).equals(mChannelAdapter.getItem(k))) {
                    Elog.i(TAG, "index" + k + "new:" + tempChAdapter.getItem(k)
                            + " old:" + mChannelAdapter.getItem(k));
                    bUpdateWifiChannel = true;
                    break;
                }
            }

        } else {
            bUpdateWifiChannel = true;
        }
        if (!bUpdateWifiChannel) {
            Elog.i(TAG, "no update");
            return;
        }
        Elog.i(TAG, "need update");
        mChannelAdapter.clear();
        for (int i = 0; i < count; i++) {
            mChannelAdapter.add(tempChAdapter.getItem(i));
        }
        if (mChannelAdapter.getCount() == 0) {
            mBtnGo.setEnabled(false);
            bUpdateWifiChannel = false;
        } else {
            mBtnGo.setEnabled(true);
        }
        if (bUpdateWifiChannel) {
            updateWifiChannel(mChannel, mChannelAdapter, mSpChannelTx0);
            if (mViewChannelTx1.getVisibility() == View.VISIBLE) {
                updateWifiChannel(mChannel, mChannelAdapter, mSpChannelTx1);
            }
        }
        mSpChannelTx0.setAdapter(mChannelAdapter);
        mSpChannelTx1.setAdapter(mChannelAdapter);
    }



    private void updateChannelByRateBandwidth(ArrayAdapter<String> adapter,
            int rateIndex, int bandwidthIndex) {

        if (mRate.getRateGroup(rateIndex) == RateInfo.EEPROM_RATE_GROUP_CCK) {
            mChannel.remove5GChannels(adapter);
            Elog.i(TAG, "The mode not support 5G channel : " + rateIndex);
        }
    }

    private void setAntSwpIdx() {
        if (!EMWifi.isAntSwapSupport()) {
            return;
        }
        long antennaIdx = mRbAntMain.isChecked() ?
                WiFiTestActivity.EnumRfAtAntswp.RF_AT_ANTSWP_MAIN.ordinal() :
                WiFiTestActivity.EnumRfAtAntswp.RF_AT_ANTSWP_AUX.ordinal();
        EMWifi.setATParam(ATPARAM_INDEX_ANTENNA_SWAP, antennaIdx);
    }

    private void setRateIndex() {
        int rateIndex = mRate.mRateIndex;
        mHighRateSelected = rateIndex >= RateInfo.MAX_LOWER_RATE_NUMBER;
        Elog.i(TAG, "rateIndex : " + rateIndex);
        if (mHighRateSelected) {
            rateIndex -= RateInfo.MAX_LOWER_RATE_NUMBER;
            rateIndex |= (1 << RATE_MODE_MASK);
        }

        EMWifi.setATParam(ATPARAM_INDEX_RATE, rateIndex);
    }

    private void startContinueTx() {
        // To do: If neither WF0 nor WF1 is checked, should we use the default

        if (mWiFiMode == WIFI_MODE_NORMAL) {
            EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_ENABLE, 0);
            EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_BAND_IDX, 0);
        } else if (mWiFiMode == WIFI_MODE_DBDC_TX0) {
            EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_ENABLE, 1);
            EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_BAND_IDX, 0);
        } else if (mWiFiMode == WIFI_MODE_DBDC_TX1) {
            EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_ENABLE, 1);
            EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_BAND_IDX, 1);
        }

        // 0.set mNss Value
        EMWifi.setATParam(ATPARAM_INDEX_NSS, mNssValue);
        long wfValue = 1;
        if (mWiFiMode == WIFI_MODE_NORMAL) {
            if (mCkWf1.isChecked()) {
                if (!mCkWf0.isChecked()) {
                    wfValue = 2;
                } else {
                    wfValue = 3;
                }
            }
        }
        // 1.set wf Value
        EMWifi.setATParam(ATPARAM_INDEX_WF0, wfValue);

        // 2.set Bandwidth
        int cbw = (mBandwidthIndex >= BAND_WIDTH_20_INDEX) ? (mBandwidthIndex - BAND_WIDTH_20_INDEX)
                : (mBandwidthIndex + BAND_WIDTH_160_INDEX);

        EMWifi.setATParam(ATPARAM_INDEX_CHANNEL_BANDWIDTH, cbw);

        // 3.set DBW
        EMWifi.setATParam(ATPARAM_INDEX_DATA_BANDWIDTH, mDataBandwidthValue);

        // 4.set PrimCh
        if (mSpPrimCh.getVisibility() == View.VISIBLE) {
            EMWifi.setATParam(ATPARAM_INDEX_PRIMARY_SETTING, mPrimaryIndex);
        }

        // 5.set TX0 channel freq
        EMWifi.setATParam(ATPARAM_INDEX_CHANNEL0, mChannelTx0Freq);

        // 6.set TX0 channel freq
        if (mViewChannelTx1.getVisibility() == View.VISIBLE) {
            EMWifi.setATParam(ATPARAM_INDEX_CHANNEL1, mChannelTx1Freq);
        }

        // 7.set rate
        setRateIndex();

        // 8.set mPkt length
        EMWifi.setATParam(ATPARAM_INDEX_PACKLENGTH, mPktLenNum);

        // 9.set mPkt cnt
        EMWifi.setATParam(ATPARAM_INDEX_PACKCOUNT, mCntNum);

        // 10.set Tx power
        EMWifi.setATParam(ATPARAM_INDEX_POWER_UNIT, POWER_UNIT_DBM);
        EMWifi.setATParam(ATPARAM_INDEX_POWER, mTxGainVal);

        // 11.set Tx mPreamble Index
        EMWifi.setATParam(ATPARAM_INDEX_PREAMBLE, mPreambleIndex);

        // 12.set Tx mGuardInterva lIndex
        EMWifi.setATParam(ATPARAM_INDEX_GI, mGuardIntervalIndex);

        // 13. set antenna index if supported
        setAntSwpIdx();

        // start tx test
        if (0 == EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STARTTX)) {
            mTestInProcess = true;
        }
        //If pkt count is 0, keeping tx until the stop button is pressed.
        //Otherwise to get real tx pkt number in 1S period to know when test done.
        if (mCntNum != 0) {
            mEventHandler.sendEmptyMessageDelayed(HANDLER_EVENT_TIMER, ONE_SENCOND);
        }

    }

    private void startDutyTx() {
        int cbw = (mBandwidthIndex >= BAND_WIDTH_20_INDEX) ? (mBandwidthIndex - BAND_WIDTH_20_INDEX)
                : (mBandwidthIndex + BAND_WIDTH_160_INDEX);
        EMWifi.setATParam(ATPARAM_INDEX_CHANNEL_BANDWIDTH, cbw);
        EMWifi.setATParam(ATPARAM_INDEX_DATA_BANDWIDTH, mDataBandwidthValue);

        if (mSpPrimCh.getVisibility() == View.VISIBLE) {
            EMWifi.setATParam(ATPARAM_INDEX_PRIMARY_SETTING, mPrimaryIndex);
        }
        setRateIndex();
        EMWifi.setATParam(ATPARAM_INDEX_POWER_UNIT, POWER_UNIT_DBM);
        EMWifi.setATParam(ATPARAM_INDEX_POWER, mTxGainVal);
        EMWifi.setATParam(ATPARAM_INDEX_PREAMBLE, mPreambleIndex);
        setAntSwpIdx();
        // start output power test
        if (0 == EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_OUTPUTPOWER)) {
            mTestInProcess = true;
        }
    }

    private void startCWToneTx() {
        EMWifi.setATParam(ATPARAM_INDEX_POWER_UNIT, POWER_UNIT_DBM);
        EMWifi.setATParam(ATPARAM_INDEX_POWER, mTxGainVal);
        // set TX0 channel freq
        EMWifi.setATParam(ATPARAM_INDEX_CHANNEL0, mChannelTx0Freq);
        // set TX1 channel freq
        if (mViewChannelTx1.getVisibility() == View.VISIBLE) {
            EMWifi.setATParam(ATPARAM_INDEX_CHANNEL1, mChannelTx1Freq);
        }
        setAntSwpIdx();
        long wfValue = 1;
        if (mWiFiMode == WIFI_MODE_NORMAL) {
            if (mCkWf1.isChecked()) {
                if (!mCkWf0.isChecked()) {
                    wfValue = 2;
                } else {
                    wfValue = 3;
                }
            }
        }
        //set wf Value
        EMWifi.setATParam(ATPARAM_INDEX_WF0, wfValue);
        // start carriar suppression test
        if (EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_LOCALFREQ) == 0) {
            mTestInProcess = true;
        }
    }

    private void startTxTest(int mode) {
        switch (mTargetModeIndex) {
        case TEST_MODE_TX:
            startContinueTx();
            break;
        case TEST_MODE_DUTY:
            startDutyTx();
            break;
        case TEST_MODE_CW_TONE:
            startCWToneTx();
            break;
        default:
            break;
        }
    }

    private void stopTxTest() {
        Elog.i(TAG, "stopTxTest");
        if (mTestInProcess) {
            EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STOPTEST);
            mTestInProcess = false;
        }
        if (mEventHandler != null) {
            mEventHandler.removeMessages(HANDLER_EVENT_TIMER);
        }
        mHandler.sendEmptyMessage(HANDLER_EVENT_FINISH);
    }

    private void handleEventTimer() {

        long[] u4Value = new long[2];
        u4Value[0] = 0;
        long pktCnt = 0;
        Elog.i(TAG, "handleEventTimer");
        int indexTransmitCount = ATPARAM_INDEX_TRANSMITCOUNT_BAND0;
        if (mWiFiMode == WIFI_MODE_DBDC_TX1) {
            indexTransmitCount = ATPARAM_INDEX_TRANSMITCOUNT_BAND1;
        }

        if (mModeIndex == TEST_MODE_DUTY) {
            pktCnt = 100;
            boolean completed = false;
            if (0 == EMWifi.getATParam(indexTransmitCount, u4Value)) {
                Elog.i(TAG, "query Transmitted packet count succeed, count = "
                        + u4Value[0] + " target count = " + pktCnt);
                if (u4Value[0] == pktCnt) {
                    completed = true;
                }
            } else {
                Elog.w(TAG, "query Transmitted packet count failed");
            }
            if (!completed) {
                EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STOPTEST);

            }
            mTargetModeIndex = TEST_MODE_DUTY;
            mEventHandler.sendEmptyMessage(HANDLER_EVENT_GO);
            return;
        }
        try {
            pktCnt = Long.parseLong(mEtPktCnt.getText().toString());
        } catch (NumberFormatException e) {
            Toast.makeText(WiFiTx6632.this, "invalid input value", Toast.LENGTH_SHORT).show();
            return;
        }
        // here we need to judge whether target number packet is
        // finished sent or not
        if (0 == EMWifi.getATParam(indexTransmitCount, u4Value)) {
            Elog.i(TAG, "query Transmitted packet count succeed, count = " + u4Value[0]
                    + " target count = " + pktCnt);
            if (u4Value[0] == pktCnt) {
                mEventHandler.removeMessages(HANDLER_EVENT_TIMER);
                mHandler.sendEmptyMessage(HANDLER_EVENT_FINISH);
                return;
            }
        } else {
            Elog.w(TAG, "query Transmitted packet count failed");
        }
        mEventHandler.sendEmptyMessageDelayed(HANDLER_EVENT_TIMER, ONE_SENCOND);
    }

    /**
     * Class for handling msg.
     *
     */
    class EventHandler extends Handler {

        /**
         * Constructor function.
         *
         * @param looper
         *            Use the provided queue instead of the default one
         */
        public EventHandler(Looper looper) {
            super(looper);
        }

        public void handleMessage(Message msg) {
            if (!EMWifi.sIsInitialed) {
                showDialog(DIALOG_WIFI_ERROR);
                return;
            }

            switch (msg.what) {
            case HANDLER_EVENT_GO:
                startTxTest(mTargetModeIndex);
                break;
            case HANDLER_EVENT_STOP:
                stopTxTest();
                break;
            case HANDLER_EVENT_TIMER:
                handleEventTimer();
                break;
            default:
                break;
            }
        }
    }

    @SuppressWarnings("deprecation")
    private boolean checkTxPath() {
        if ((!mCkWf0.isChecked()) && (!mCkWf1.isChecked())) {
            showDialog(DIALOG_NO_PATH_SELECT);
            return false;
        }
        return true;
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        // TODO Auto-generated method stub
        switch (id) {
        case DIALOG_NO_PATH_SELECT:
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.wifi_dialog_warn);
            builder.setCancelable(false);
            builder.setMessage(getString(R.string.wifi_dialog_no_path_select));
            builder.setPositiveButton(R.string.dialog_ok, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    finish();
                }
            });
            return builder.create();
        default:
            return super.onCreateDialog(id);
        }
    }

    private void onClickBtnTxGo() {
        if (!checkTxPath()) {
            return;
        }
        long u4TxGainVal = 0;
        int i = 0;
        long pktNum;
        long cntNum;
        CharSequence inputVal;
        try {
            mTxGainVal = (long) (Float.parseFloat(mEtTxGain.getText().toString()) * 2);
        } catch (NumberFormatException e) {
            Toast.makeText(WiFiTx6632.this, "invalid input value", Toast.LENGTH_SHORT).show();
            return;
        }
        mTargetModeIndex = mModeIndex;
        switch (mModeIndex) {
        case TEST_MODE_TX:
            try {
                pktNum = Long.parseLong(mEtPkt.getText().toString());
                cntNum = Long.parseLong(mEtPktCnt.getText().toString());
            } catch (NumberFormatException e) {
                Toast.makeText(WiFiTx6632.this, "invalid input value", Toast.LENGTH_SHORT)
                        .show();
                return;
            }
            mPktLenNum = pktNum;
            mCntNum = cntNum;
            break;
        case TEST_MODE_DUTY:
            mPktLenNum = 100;
            mCntNum = 100;
            mTargetModeIndex = TEST_MODE_TX;
            break;

        default:
            break;
        }
        if (mEventHandler != null) {
            mEventHandler.sendEmptyMessage(HANDLER_EVENT_GO);
            setViewEnabled(false);
        }
    }

    @Override
    protected void onDestroy() {
        if (mEventHandler != null) {
            mEventHandler.removeMessages(HANDLER_EVENT_TIMER);
            if (mTestInProcess) {
                if (EMWifi.sIsInitialed) {
                    EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STOPTEST);
                }
                mTestInProcess = false;
            }
        }
        if (mTestThread != null) {
            mTestThread.quit();
        }
        super.onDestroy();
    }

    private void setViewEnabled(boolean state) {
        mSpChannelTx0.setEnabled(state);
        mSpChannelTx1.setEnabled(state);
        mSpGuardInterval.setEnabled(state);
        mSpBandwidth.setEnabled(state);
        mRbNss1.setEnabled(state);
        mRbNss2.setEnabled(state);
        mCkDuplicateMode.setEnabled(state);
        mSpPreamble.setEnabled(state);
        mEtPkt.setEnabled(state);
        mEtPktCnt.setEnabled(state);
        mEtTxGain.setEnabled(state);
        mSpRate.setEnabled(state);
        mSpMode.setEnabled(state);
        mBtnGo.setEnabled(state);
        mBtnStop.setEnabled(!state);
        mSpDbw.setEnabled(state);
        mSpPrimCh.setEnabled(state);
        mRGNss.setEnabled(state);
        mCkWf0.setEnabled(state);
        mCkWf1.setEnabled(state);
        mSpDbw.setEnabled(state);
        mSpPrimCh.setEnabled(state);
    }

    private void onClickBtnTxStop() {
        if (mEventHandler != null) {
            mEventHandler.sendEmptyMessage(HANDLER_EVENT_STOP);
        }
        switch (mModeIndex) {
        case TEST_MODE_TX:
            break;
        default:
            EMWifi.setStandBy();
            break;
        }
    }

    /** mCheck or mUncheck checkbox items. */
    private final CheckBox.OnCheckedChangeListener mCheckedListener =
            new CheckBox.OnCheckedChangeListener() {

                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    // TODO Auto-generated method stub
                    if (buttonView.getId() == R.id.wifi_duplicate_mode_tx) {

                        if (isChecked) {
                            mCkWf0.setChecked(true);
                            mCkWf1.setChecked(true);
                        }
                    } else if (buttonView.getId() == R.id.wifi_wfx_0) {
                        if (!isChecked) {
                            mCkDuplicateMode.setChecked(false);
                        }
                        updateTxPower();
                    } else if (buttonView.getId() == R.id.wifi_wfx_1) {
                        if (!isChecked) {
                            mCkDuplicateMode.setChecked(false);
                        }
                        updateTxPower();
                    }
                }

            };

    private void updateRate() {
        mRateAdapter.clear();
        int size = 0;
        Elog.i(TAG, "updateRate, mNssValue = " + mNssValue);
        if (mPreambleIndex == 0) { // normal
            size = RateInfo.RATE_ITEM_54M.length;
            for (int i = 0; i < size; i++) {
                mRateAdapter.add(RateInfo.RATE_ITEM_54M[i]);
            }
        } else if (mPreambleIndex == 1) { // cck short
            size = RateInfo.RATE_ITEM_11M.length;
            for (int i = 0; i < size; i++) {
                mRateAdapter.add(RateInfo.RATE_ITEM_11M[i]);
            }
        } else if (mPreambleIndex == 2 || mPreambleIndex == 3) { // mixed mode
                                                                 // or green
                                                                 // field
            if (mNssValue == 1) {
                size = RateInfo.RATE_ITEM_CS7.length;
                for (int i = 0; i < size; i++) {
                    mRateAdapter.add(RateInfo.RATE_ITEM_CS7[i]);
                }
            } else if (mNssValue == 2) {
                size = RateInfo.RATE_ITEM_CS15.length;
                for (int i = 0; i < size; i++) {
                    mRateAdapter.add(RateInfo.RATE_ITEM_CS15[i]);
                }
            }
        } else if (mPreambleIndex == 4) { // 802.11 ac
            size = RateInfo.RATE_ITEM_CS7.length;
            for (int i = 0; i < size; i++) {
                mRateAdapter.add(RateInfo.RATE_ITEM_CS7[i]);
            }
            if (mBandwidthIndex >= BAND_WIDTH_20_INDEX) {
                mRateAdapter.add("MCS8");
            }
            if (mBandwidthIndex >= BAND_WIDTH_40_INDEX) {
                mRateAdapter.add("MCS9");
            }
        }
        mSpRate.setAdapter(mRateAdapter);

        if (mBandwidthIndex >= BAND_WIDTH_80_INDEX && mPreambleIndex == 0) {
            mSpRate.setSelection(4); // "1M", "2M", "5.5M", "11M" not support 5G
        }
    }

    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {
        // TODO Auto-generated method stub
        if (checkedId == R.id.wifi_tx_nss_1) {
            mNssValue = 1;
            mCkDuplicateMode.setVisibility(View.VISIBLE);
        } else if (checkedId == R.id.wifi_tx_nss_2) {
            mNssValue = 2;
            mCkDuplicateMode.setVisibility(View.GONE);
        }
        Elog.i(TAG, "mNssValue changed = " + mNssValue);
        updateRate();
    }


    private void updateTxPower() {

        Elog.i(TAG, "updateTxPower");
        if (mRate == null) {
            return;
        }

        EMWifi.setATParam(ATPARAM_INDEX_CHANNEL0, mChannelTx0Freq);
        EMWifi.setATParam(ATPARAM_INDEX_PREAMBLE, mPreambleIndex);
        setRateIndex();
        EMWifi.setATParam(ATPARAM_INDEX_DATA_BANDWIDTH, mDataBandwidthValue);
        long wfValue = 1;
        if (mWiFiMode == WIFI_MODE_NORMAL) {
            if (mCkWf1.isChecked()) {
                if (!mCkWf0.isChecked()) {
                    wfValue = 2;
                } else {
                    wfValue = 3;
                }
            }
        }
        EMWifi.setATParam(ATPARAM_INDEX_WF0, wfValue);
        long[] u4Value = new long[2];
        u4Value[0] = 0;
        if ((EMWifi.getATParam(ATPARAM_INDEX_TX_POWER, u4Value) == 0)
                && (u4Value[0] != 0)) {
            mEtTxGain.setText(String.valueOf(u4Value[0] * 0.5));
        } else {
            mEtTxGain.setText(String.valueOf(DEFAULT_TX_GAIN));
        }
    }
}
