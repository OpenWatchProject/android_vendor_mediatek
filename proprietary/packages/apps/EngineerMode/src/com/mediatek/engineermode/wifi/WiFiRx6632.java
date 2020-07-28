package com.mediatek.engineermode.wifi;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.SystemClock;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;


import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * Class for Wifi RX test.
 *
 */
public class WiFiRx6632 extends WiFiTestActivity implements OnClickListener,
        OnCheckedChangeListener {
    private static final String TAG = "WifiRx";
    private static final String[] sBandWidth = { "5MHz", "10MHz", "20MHz", "40MHz", "80MHz",
            "160MHz", "160NC" };
    private static final String[] sBandWidthDBW = { "BW5", "BW10", "BW20", "BW40", "BW80",
            "BW160" };

    private static final int BAND_WIDTH_20_INDEX = 2;
    private static final int BAND_WIDTH_40_INDEX = 3;
    private static final int BAND_WIDTH_80_INDEX = 4;
    private static final int BAND_WIDTH_160_INDEX = 5;
    private static final int BAND_WIDTH_160NC_INDEX = 6;
    private static final int COMMAND_INDEX_STARTRX = 2;
    private static final int HANDLER_EVENT_RX = 2;

    private static final int WIFI_MODE_NORMAL = 1;
    private static final int WIFI_MODE_DBDC_RX0 = 2;
    private static final int WIFI_MODE_DBDC_RX1 = 3;

    private static final long PERCENT = 100;
    private static final int WAIT_COUNT = 10;
    private static final int DIALOG_NO_PATH_SELECT = 1;
    private static final String TEXT_ZERO = "0";
    protected static final long HANDLER_RX_DELAY_TIME = 1000;
    private TextView mTvFcs = null;
    private TextView mTvRx = null;
    private TextView mTvPer = null;
    private TextView mTvWiFi0RssiContext = null;
    private TextView mTvWiFi1RssiContext = null;
    private TextView mTvWiFi0RssiText = null;
    private TextView mTvWiFi1RssiText = null;
    private CheckBox mCkWf0 = null;
    private CheckBox mCkWf1 = null;
    private CheckBox mCkDuplicateMode = null;
    private Spinner mSpChannelRx0 = null;
    private Spinner mSpChannelRx1 = null;
    private TextView mTvDbw = null;
    private TextView mTvPrimCh = null;
    private Spinner mSpDbw = null;
    private Spinner mSpPrimCh = null;
    private Spinner mSpBandwidth = null;
    private Button mBtnGo = null;
    private Button mBtnStop = null;
    private View mViewChannelRx1 = null;
    private HandlerThread mTestThread = null;
    private Handler mEventHandler = null;
    private ChannelInfo mChannel = null;
    private ArrayAdapter<String> mChannelAdapter = null;
    private ArrayAdapter<String> mDbwAdapter = null;
    private ArrayAdapter<String> mPrimChAdapter = null;
    private int mChannelRx0Freq;
    private int mChannelRx1Freq;
    private int mBandwidthIndex = 0;
    private int mDataBandwidthIndex = 0;
    private int mPrimaryIndex = 0;
    private int mWiFiMode = WIFI_MODE_NORMAL;
    private ViewGroup mVgAnt = null;
    private RadioButton mRbAntMain = null;

    private final Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            if (!EMWifi.sIsInitialed) {
                showDialog(DIALOG_WIFI_ERROR);
                return;
            }
            if (HANDLER_EVENT_RX == msg.what) {
                long[] i4Rx = new long[3];
                long i4RxCntOk = -1;
                long i4RxCntFcsErr = -1;
                long i4RxPer = -1;
                long i4RxRssiWiFi0 = -1;
                long i4RxRssiWiFi1 = -1;
                Elog.i(TAG, "The Handle event is : HANDLER_EVENT_RX");
                try {
                    i4RxPer = Long.parseLong(mTvPer.getText().toString());
                } catch (NumberFormatException e) {
                    Elog.w(TAG, "Long.parseLong NumberFormatException: " + e.getMessage());
                }

                if (mWiFiMode == WIFI_MODE_DBDC_RX0) {
                    EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_BAND_IDX, 0);
                } else if (mWiFiMode == WIFI_MODE_DBDC_RX1) {
                    EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_BAND_IDX, 1);
                }

                EMWifi.getPacketRxStatus(i4Rx, 3);

                Elog.i(TAG, "rx ok = " + i4Rx[0] + "fcs error = " + i4Rx[1] + "RSSI = " + i4Rx[2]);


                i4RxCntOk = i4Rx[0]/* - i4Init[0] */;
                i4RxCntFcsErr = i4Rx[1]/* - i4Init[1] */;
                i4RxRssiWiFi0 = ((i4Rx[2]) & 0xFF) - 255;
                i4RxRssiWiFi1 = (((i4Rx[2]) & 0xFF00) >> 8) - 255;

                if (i4RxCntFcsErr + i4RxCntOk != 0) {
                    i4RxPer = i4RxCntFcsErr * PERCENT / (i4RxCntFcsErr + i4RxCntOk);
                }
                mTvFcs.setText(String.valueOf(i4RxCntFcsErr));
                mTvRx.setText(String.valueOf(i4RxCntOk));
                mTvPer.setText(String.valueOf(i4RxPer));

                if (mCkWf0.isChecked() && mCkWf1.isChecked()) {
                    mTvWiFi1RssiContext.setVisibility(View.VISIBLE);
                    mTvWiFi1RssiText.setVisibility(View.VISIBLE);
                    mTvWiFi0RssiText.setText("RSSI(WiFi0):");
                    mTvWiFi0RssiContext.setText(String.valueOf(i4RxRssiWiFi0));
                    mTvWiFi1RssiContext.setText(String.valueOf(i4RxRssiWiFi1));
                } else {
                    mTvWiFi1RssiContext.setVisibility(View.GONE);
                    mTvWiFi1RssiText.setVisibility(View.GONE);
                    mTvWiFi0RssiText.setText("RSSI: ");
                    if ((mWiFiMode == WIFI_MODE_DBDC_RX0)
                            || (mWiFiMode == WIFI_MODE_DBDC_RX1)) {
                        mTvWiFi0RssiContext.setText(String.valueOf(i4RxRssiWiFi0));
                    } else {
                        mTvWiFi0RssiContext.setText(mCkWf0.isChecked() ?
                                String.valueOf(i4RxRssiWiFi0) : String.valueOf(i4RxRssiWiFi1));
                    }

                }
            }
            mHandler.sendEmptyMessageDelayed(HANDLER_EVENT_RX, HANDLER_RX_DELAY_TIME);
        }
    };

    private void initUiLayout() {
        mCkWf0 = (CheckBox) findViewById(R.id.wifi_wfx_0);
        mCkWf1 = (CheckBox) findViewById(R.id.wifi_wfx_1);
        mCkDuplicateMode = (CheckBox) findViewById(R.id.wifi_duplicate_mode_rx);
        mCkWf0.setOnCheckedChangeListener(this);
        mCkWf1.setOnCheckedChangeListener(this);
        mCkDuplicateMode.setOnCheckedChangeListener(this);
        mCkWf0.setChecked(true);

        mBtnGo = (Button) findViewById(R.id.WiFi_Go_Rx);
        mBtnStop = (Button) findViewById(R.id.WiFi_Stop_Rx);
        mBtnGo.setOnClickListener(this);
        mBtnStop.setOnClickListener(this);

        mSpBandwidth = (Spinner) findViewById(R.id.wifi_bandwidth_spinner);

        mSpChannelRx0 = (Spinner) findViewById(R.id.wifi_rx0_channel_spinner);
        mSpChannelRx1 = (Spinner) findViewById(R.id.wifi_rx1_channel_spinner);
        mViewChannelRx1 = findViewById(R.id.wifi_rx1_channel_layout);

        mTvDbw = (TextView) findViewById(R.id.wifi_bandwidth_dbw_tv);
        mSpDbw = (Spinner) findViewById(R.id.wifi_bandwidth_dbw_spn);
        mTvPrimCh = (TextView) findViewById(R.id.wifi_bandwidth_prim_ch_tv);
        mSpPrimCh = (Spinner) findViewById(R.id.wifi_bandwidth_prim_ch_spn);

        mTvFcs = (TextView) findViewById(R.id.WiFi_FCS_Content);
        mTvRx = (TextView) findViewById(R.id.WiFi_Rx_Content);
        mTvPer = (TextView) findViewById(R.id.WiFi_PER_Content);
        mTvFcs.setText(R.string.wifi_empty);
        mTvRx.setText(R.string.wifi_empty);
        mTvPer.setText(R.string.wifi_empty);
        mTvWiFi0RssiText = (TextView) findViewById(R.id.WiFi0_RX_RSSI_Text);
        mTvWiFi1RssiText = (TextView) findViewById(R.id.WiFi1_RX_RSSI_Text);
        mTvWiFi0RssiContext = (TextView) findViewById(R.id.WiFi0_RX_RSSI_Content);
        mTvWiFi1RssiContext = (TextView) findViewById(R.id.WiFi1_RX_RSSI_Content);

        mTvWiFi0RssiContext.setText(R.string.wifi_empty);
        mTvWiFi1RssiContext.setText(R.string.wifi_empty);
        mChannel = new ChannelInfo();

        mVgAnt = (ViewGroup) findViewById(R.id.wifi_ant_vg);
        mRbAntMain = (RadioButton) findViewById(R.id.wifi_rx_ant_main);
    }

    private void initUiComponent() {
        // Channel Spinner init
        mChannelAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        mChannelAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mChannelAdapter.clear();
        mChannel.resetSupportedChannels(mChannelAdapter);

        // Rx0 Channel Spinner init
        mSpChannelRx0.setAdapter(mChannelAdapter);
        mSpChannelRx0.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (EMWifi.sIsInitialed) {
                    String name = mChannelAdapter.getItem(position);
                    int channelId = ChannelInfo.parseChannelId(name);
                    mChannelRx0Freq = ChannelInfo.getChannelFrequency(channelId);

                } else {
                    showDialog(DIALOG_WIFI_ERROR);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {

            }
        });

        // Rx1 Channel Spinner
        mSpChannelRx1.setAdapter(mChannelAdapter);
        mSpChannelRx1.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (EMWifi.sIsInitialed) {
                    String name = mChannelAdapter.getItem(position);
                    int channelId = ChannelInfo.parseChannelId(name);
                    mChannelRx1Freq = ChannelInfo.getChannelFrequency(channelId);

                } else {
                    showDialog(DIALOG_WIFI_ERROR);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {

            }
        });

        // RX Bandwidth settings
        ArrayAdapter<String> bandwidthAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        bandwidthAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        int size = sBandWidth.length;

        if (mWiFiMode == WIFI_MODE_DBDC_RX0) {
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
                    mViewChannelRx1.setVisibility(View.GONE);
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
                    mViewChannelRx1.setVisibility(View.GONE);
                } else {
                    mSpPrimCh.setEnabled(false);
                    mSpDbw.setEnabled(false);
                    mViewChannelRx1.setVisibility(View.VISIBLE);
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
                updateChannels();
//                mSpChannelRx0.setAdapter(mChannelAdapter);
//                mSpChannelRx1.setAdapter(mChannelAdapter);

            }

            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });

        // RX DBW settings
        mDbwAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        mDbwAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpDbw.setOnItemSelectedListener(new OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (position == mDataBandwidthIndex) {
                    return;
                }
                mDataBandwidthIndex = position;
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });
        // RX prim settings
        mPrimChAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        mPrimChAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpPrimCh.setOnItemSelectedListener(new OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (position == mPrimaryIndex) {
                    return;
                }
                mPrimaryIndex = position;
                // onAdvancedSelectChanged(mChannelBandwidth,
                // mDataBandwidthIndex, mPrimaryIndex);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });
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
        setContentView(R.layout.wifi_rx_6632);
        Intent intent = getIntent();
        String data = intent.getStringExtra("MODE");
        if (data == null) {
            mWiFiMode = WIFI_MODE_NORMAL;
        } else if (data.equals("RX Band0")) {
            mWiFiMode = WIFI_MODE_DBDC_RX0;
        } else if (data.equals("RX Band1")) {
            mWiFiMode = WIFI_MODE_DBDC_RX1;
        }

        initUiLayout();
        initUiComponent();

        if (mWiFiMode == WIFI_MODE_DBDC_RX0) {
            mCkWf0.setChecked(true);
            mCkWf1.setChecked(false);
            mCkWf0.setVisibility(View.GONE);
            mCkWf1.setVisibility(View.GONE);
            mCkDuplicateMode.setVisibility(View.GONE);
        } else if (mWiFiMode == WIFI_MODE_DBDC_RX1) {
            mCkWf0.setChecked(false);
            mCkWf1.setChecked(true);
            mCkWf0.setVisibility(View.GONE);
            mCkWf1.setVisibility(View.GONE);
            mCkDuplicateMode.setVisibility(View.GONE);
        }

        if (!EMWifi.isAntSwapSupport()) {
            mVgAnt.setVisibility(View.GONE);
        }
    }

    private void updateChannels() {
        // mChannel.resetSupportedChannels(mChannelAdapter);
        ArrayAdapter<String> tempChAdapter = new ArrayAdapter<String>(
                this, android.R.layout.simple_spinner_item);


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

        if (mWiFiMode == WIFI_MODE_DBDC_RX0) {
            mChannel.remove5GChannels(tempChAdapter);
        } else if (mWiFiMode == WIFI_MODE_DBDC_RX1) {
            mChannel.remove2dot4GChannels(tempChAdapter);
        }
        boolean bUpdateWifiChannel = false;
        int count = tempChAdapter.getCount();
        if (count == mChannelAdapter.getCount()) {
            for (int k = 0; k < count; k++) {
                if (!tempChAdapter.getItem(k).equals(mChannelAdapter.getItem(k))) {
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
            //Update dbdc index before set channel
            if (mWiFiMode == WIFI_MODE_DBDC_RX0) {
                EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_BAND_IDX, 0);
            } else if (mWiFiMode == WIFI_MODE_DBDC_RX1) {
                EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_BAND_IDX, 1);
            }
            updateWifiChannel(mChannel, mChannelAdapter, mSpChannelRx0);
            if (mSpChannelRx1.getVisibility() == View.VISIBLE) {
                updateWifiChannel(mChannel, mChannelAdapter, mSpChannelRx1);
            }
        }
    }

    @Override
    public void onClick(View v) {
        // TODO Auto-generated method stub
        if (!EMWifi.sIsInitialed) {
            showDialog(DIALOG_WIFI_ERROR);
            return;
        }
        if (v.getId() == mBtnGo.getId()) {
            onClickBtnRxGo();
        } else if (v.getId() == mBtnStop.getId()) {
            onClickBtnRxStop();
        }
    }

    @SuppressWarnings("deprecation")
    private boolean checkRxPath() {
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

    /**
     * Invoked when "Go" button clicked.
     */
    private void onClickBtnRxGo() {
        if (!checkRxPath()) {
            return;
        }
        setViewEnabled(false);

        if (mWiFiMode == WIFI_MODE_NORMAL) {
            EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_ENABLE, 0);
            EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_BAND_IDX, 0);
        } else if (mWiFiMode == WIFI_MODE_DBDC_RX0) {
            EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_ENABLE, 1);
            EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_BAND_IDX, 0);
        } else if (mWiFiMode == WIFI_MODE_DBDC_RX1) {
            EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_ENABLE, 1);
            EMWifi.setATParam(ATPARAM_INDEX_SET_DBDC_BAND_IDX, 1);
        }
        long wfValue = 0x00010001;
        if (mWiFiMode == WIFI_MODE_DBDC_RX1) {
              wfValue = 0x00010001;
            if (mCkWf1.isChecked()) {
                if (!mCkWf0.isChecked()) {
                    wfValue = 0x00020001;
                } else {
                    wfValue = 0x00030001;
                }
            }
        } else {
             wfValue = 0x00010000;
            if (mCkWf1.isChecked()) {
                if (!mCkWf0.isChecked()) {
                    wfValue = 0x00020000;
                } else {
                    wfValue = 0x00030000;
                }
            }
        }

        // 0. set WF0 or WF1 or both WF0 and Wf1
        EMWifi.setATParam(ATPARAM_INDEX_RX, wfValue);

        // 1. set Bandwidth index(0 -> 20MHz, 1 -> 40MHz, 2 -> 80MHz, 3 ->
        // 160MHz, 4 -> 160NC, 5 -> 5MHz, 6 -> 10MHz)

        int cbw = (mBandwidthIndex >= BAND_WIDTH_20_INDEX) ? (mBandwidthIndex - BAND_WIDTH_20_INDEX)
                : (mBandwidthIndex + BAND_WIDTH_160_INDEX);
        EMWifi.setATParam(ATPARAM_INDEX_CHANNEL_BANDWIDTH, cbw);

        // 2. set mSpDbw
        EMWifi.setATParam(ATPARAM_INDEX_DATA_BANDWIDTH, mDataBandwidthIndex);

        // 3. set mSpPrimCh
        if (mSpPrimCh.getVisibility() == View.VISIBLE) {
            EMWifi.setATParam(ATPARAM_INDEX_PRIMARY_SETTING, mPrimaryIndex);
        }

        // 4.set Rx0 channel
        EMWifi.setATParam(ATPARAM_INDEX_CHANNEL0, mChannelRx0Freq);

        // 5.set Rx1 channel
        if (mViewChannelRx1.getVisibility() == View.VISIBLE) {
            EMWifi.setATParam(ATPARAM_INDEX_CHANNEL1, mChannelRx1Freq);
        }

        // 6. set antenna index if supported
        if (EMWifi.isAntSwapSupport()) {
            long antennaIdx = mRbAntMain.isChecked() ?
                    WiFiTestActivity.EnumRfAtAntswp.RF_AT_ANTSWP_MAIN.ordinal() :
                    WiFiTestActivity.EnumRfAtAntswp.RF_AT_ANTSWP_AUX.ordinal();
            EMWifi.setATParam(ATPARAM_INDEX_ANTENNA_SWAP, antennaIdx);
        }

        // 7. start Rx
        EMWifi.setATParam(ATPARAM_INDEX_COMMAND, COMMAND_INDEX_STARTRX);
        mHandler.sendEmptyMessage(HANDLER_EVENT_RX);
        mTvFcs.setText(TEXT_ZERO);
        mTvRx.setText(TEXT_ZERO);
        mTvPer.setText(TEXT_ZERO);
    }

    /**
     * Invoked when "Stop" button clicked.
     */
    private void onClickBtnRxStop() {
        long[] u4Value = new long[1];
        mHandler.removeMessages(HANDLER_EVENT_RX);
        for (int i = 0; i < WAIT_COUNT; i++) {
            u4Value[0] = EMWifi.setATParam(ATPARAM_INDEX_COMMAND, 0);
            if (u4Value[0] == 0) {
                break;
            } else {
                SystemClock.sleep(WAIT_COUNT);
                Elog.w(TAG, "stop Rx test failed at the " + i + "times try");
            }
        }
        setViewEnabled(true);
    }

    private void setViewEnabled(boolean state) {
        mBtnGo.setEnabled(state);
        mBtnStop.setEnabled(!state);
        mCkWf0.setEnabled(state);
        mCkWf1.setEnabled(state);
        mSpChannelRx0.setEnabled(state);
        mSpChannelRx1.setEnabled(state);
        mSpBandwidth.setEnabled(state);
        mSpDbw.setEnabled(state);
        mSpPrimCh.setEnabled(state);
        mCkDuplicateMode.setEnabled(state);
    }

    @Override
    protected void onDestroy() {
        mHandler.removeMessages(HANDLER_EVENT_RX);
        if (EMWifi.sIsInitialed) {
            EMWifi.setATParam(1, 0);
        }
        super.onDestroy();
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        // TODO Auto-generated method stub
        if (buttonView.getId() == R.id.wifi_duplicate_mode_rx) {
            if (isChecked) {
                mCkWf0.setChecked(true);
                mCkWf1.setChecked(true);
            }
        } else if (buttonView.getId() == R.id.wifi_wfx_0) {
            if (!isChecked) {
                mCkDuplicateMode.setChecked(false);
            }
        } else if (buttonView.getId() == R.id.wifi_wfx_1) {
            if (!isChecked) {
                mCkDuplicateMode.setChecked(false);
            }
        }
    }

}
