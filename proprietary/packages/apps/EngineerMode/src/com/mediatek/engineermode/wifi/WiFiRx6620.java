/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.wifi;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * Wifi RX test for MT6620.
 *
 */
public class WiFiRx6620 extends WiFiTestActivity implements OnClickListener {

    private static final String TAG = "WifiRx";
    private static final int HANDLER_EVENT_RX = 2;
    protected static final long HANDLER_RX_DELAY_TIME = 1000;
    private static final long PERCENT = 100;
    private static final String TEXT_ZERO = "0";
    private static final int BANDWIDTH_INDEX_40M = 1;
    private static final int BANDWIDTH_INDEX_80M = 2;
    private static final int WAIT_COUNT = 10;
    private final String[] mBandwidth = { "20MHz", "40MHz", "U20MHz", "L20MHz", "Advanced"};
    private TextView mTvFcs = null;
    private TextView mTvRx = null;
    private TextView mTvPer = null;
    private Button mBtnGo = null;
    private Button mBtnStop = null;
    private Spinner mChannelSpinner = null;
    private Spinner mBandwidthSpinner = null;
    private int mBandwidthIndex = 0;
    private ArrayAdapter<String> mChannelAdapter = null;
    private ChannelInfo mChannel = null;
    private long[] mInitData = null;
    private Spinner mSpBwAdvCbw = null;
    private Spinner mSpBwAdvDbw = null;
    private Spinner mSpBwAdvPrimCh = null;
    int mChannelBandwidth = 0;
    int mDataBandwidth = 0;
    int mPrimarySetting = 0;
    private static final String[] BW_ADVANCED_ITEMS = {"BW20", "BW40", "BW80"};
    private static final int BW_INDX_ADVANCED = 4;
    private ArrayAdapter<String> mDbwAdapter = null;
    private ArrayAdapter<String> mPrimChAdapter = null;
    private boolean mIsAdvancedMode = false;

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
                Elog.i(TAG, "The Handle event is : HANDLER_EVENT_RX");
                try {
                    i4RxPer = Long.parseLong(mTvPer.getText().toString());
                } catch (NumberFormatException e) {
                    Elog.e(TAG, "Long.parseLong NumberFormatException: " + e.getMessage());
                }
                EMWifi.getPacketRxStatus(i4Rx, 3);
                Elog.i(TAG, "rx ok = " + String.valueOf(i4Rx[0]) +
                       "fcs error = " + String.valueOf(i4Rx[1]));
                i4RxCntOk = i4Rx[0]/* - i4Init[0] */;
                i4RxCntFcsErr = i4Rx[1]/* - i4Init[1] */;
                if (i4RxCntFcsErr + i4RxCntOk != 0) {
                    i4RxPer = i4RxCntFcsErr * PERCENT
                            / (i4RxCntFcsErr + i4RxCntOk);
                }
                mTvFcs.setText(String.valueOf(i4RxCntFcsErr));
                mTvRx.setText(String.valueOf(i4RxCntOk));
                mTvPer.setText(String.valueOf(i4RxPer));
            }
            mHandler.sendEmptyMessageDelayed(HANDLER_EVENT_RX,
                    HANDLER_RX_DELAY_TIME);
        }
    };
    private final OnItemSelectedListener mChannelSpinnerListener = new OnItemSelectedListener() {

        public void onItemSelected(AdapterView<?> parent, View view, int position,
                long id) {
            if (EMWifi.sIsInitialed) {
                mChannel.setSelectedChannel(mChannelAdapter.getItem(position));
                EMWifi.setChannel(mChannel.getSelectedFrequency());
            } else {
                showDialog(DIALOG_WIFI_ERROR);
            }
        }

        public void onNothingSelected(AdapterView<?> arg0) {

        }
    };

    private final OnItemSelectedListener mBandwidthSpinnerListener = new OnItemSelectedListener() {
        public void onItemSelected(android.widget.AdapterView<?> parent,
                View view, int position, long id) {
            mBandwidthIndex = position < mBandwidth.length ? position
                    : mBandwidthIndex;
            if (mBandwidth[BW_INDX_ADVANCED].equals(
                    mBandwidthSpinner.getSelectedItem().toString())) {
                mBandwidthIndex = BW_INDX_ADVANCED;
            }
            if (mBandwidthIndex == BW_INDX_ADVANCED) {
                onAdvancedBandwidthSelected();
            } else {
                updateChannels();
                findViewById(R.id.wifi_bandwidth_advanced_ll).setVisibility(View.GONE);
            }
        }

        public void onNothingSelected(android.widget.AdapterView<?> parent) {
        }
    };

    private void updateChannels() {
        mChannel.resetSupportedChannels(mChannelAdapter);
        updateChannelByBandwidth(mBandwidthIndex);
        if (mBandwidthIndex == BW_INDX_ADVANCED) {
            updateChannelByAdvancedSetting(mChannelBandwidth, mDataBandwidth, mPrimarySetting);
        }
        boolean bUpdateWifiChannel = true;
        if (mChannelAdapter.getCount() == 0) {
            mBtnGo.setEnabled(false);
            bUpdateWifiChannel = false;
        } else {
            mBtnGo.setEnabled(true);
        }
        if (bUpdateWifiChannel) {
            updateWifiChannel(mChannel, mChannelAdapter, mChannelSpinner);
        }
    }

    private void updateChannelByBandwidth(int bandwidthIndex) {
        if (bandwidthIndex == BANDWIDTH_INDEX_40M) {
            mChannel.removeBw40mUnsupported2dot4GChannels(mChannelAdapter);
            mChannel.remove5GChannels(mChannelAdapter);
            mChannel.insertBw40MChannels(mChannelAdapter);
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (!EMWifi.sIsInitialed) {
            Elog.d(TAG, "sIsInitialed:" + EMWifi.sIsInitialed);
            Toast.makeText(this, R.string.wifi_dialog_error_message, Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        setContentView(R.layout.wifi_rx_6620);
        if (WiFi.is11acSupported()) {
            mIsAdvancedMode = true;
        }
        mTvFcs = (TextView) findViewById(R.id.WiFi_FCS_Content);
        mTvRx = (TextView) findViewById(R.id.WiFi_Rx_Content);
        mTvPer = (TextView) findViewById(R.id.WiFi_PER_Content);
        mTvFcs.setText(R.string.wifi_empty);
        mTvRx.setText(R.string.wifi_empty);
        mTvPer.setText(R.string.wifi_empty);
        mBtnGo = (Button) findViewById(R.id.WiFi_Go_Rx);
        mBtnStop = (Button) findViewById(R.id.WiFi_Stop_Rx);
        mBtnGo.setOnClickListener(this);
        mBtnStop.setOnClickListener(this);
        mInitData = new long[3];
        mChannel = new ChannelInfo();
        mChannelSpinner = (Spinner) findViewById(R.id.WiFi_RX_Channel_Spinner);
        mChannelAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        mChannelAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mChannelAdapter.clear();

        mChannel.resetSupportedChannels(mChannelAdapter);
        mChannelSpinner.setAdapter(mChannelAdapter);
        mChannelSpinner.setOnItemSelectedListener(mChannelSpinnerListener);

        mBandwidthSpinner = (Spinner) findViewById(R.id.WiFi_Bandwidth_Spinner);
        // Bandwidth setings
        ArrayAdapter<String> bwAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        bwAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        if (mIsAdvancedMode) {
            bwAdapter.add(mBandwidth[BW_INDX_ADVANCED]);
        } else {
            for (int i = 0; i < BW_INDX_ADVANCED; i++) {
                bwAdapter.add(mBandwidth[i]);
            }
        }
        mBandwidthSpinner.setAdapter(bwAdapter);
        mBandwidthSpinner.setOnItemSelectedListener(mBandwidthSpinnerListener);
        mSpBwAdvCbw = (Spinner) findViewById(R.id.wifi_bandwidth_cbw_spn);
        mSpBwAdvDbw = (Spinner) findViewById(R.id.wifi_bandwidth_dbw_spn);
        mSpBwAdvPrimCh = (Spinner) findViewById(R.id.wifi_bandwidth_prim_ch_spn);
        setViewEnabled(true);
        initUiComponent();
    }

    private void initUiComponent() {
        ArrayAdapter<String> cbwAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, BW_ADVANCED_ITEMS);
        cbwAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpBwAdvCbw.setAdapter(cbwAdapter);
        mSpBwAdvCbw.setOnItemSelectedListener(new OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view,
                    int position, long id) {
                if (mChannelBandwidth == position) {
                    return;
                }
                mChannelBandwidth = position;
                // update DBW
                mDbwAdapter.clear();
                for (int i = 0; i <= position; i++) {
                    mDbwAdapter.add(BW_ADVANCED_ITEMS[i]);
                }
                mSpBwAdvDbw.setAdapter(mDbwAdapter);

                // update PRIMARY CH
                mPrimChAdapter.clear();
                int maxPrimCh = (int) Math.pow(2, position) - 1;
                for (int i = 0; i <= maxPrimCh; i++) {
                    mPrimChAdapter.add(String.valueOf(i));
                }
                mSpBwAdvPrimCh.setAdapter(mPrimChAdapter);

                onAdvancedSelectChanged(mChannelBandwidth, mDataBandwidth, mPrimarySetting);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }

        });

        mDbwAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        mDbwAdapter.add(BW_ADVANCED_ITEMS[0]);
        mDbwAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpBwAdvDbw.setAdapter(mDbwAdapter);
        mSpBwAdvDbw.setOnItemSelectedListener(new OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view,
                    int position, long id) {
                if (position == mDataBandwidth) {
                    return;
                }
                mDataBandwidth = position;
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mPrimChAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        mPrimChAdapter.add("0");
        mPrimChAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpBwAdvPrimCh.setAdapter(mPrimChAdapter);
        mSpBwAdvPrimCh.setOnItemSelectedListener(new OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view,
                    int position, long id) {
                if (position == mPrimarySetting) {
                    return;
                }
                mPrimarySetting = position;
                //onAdvancedSelectChanged(mChannelBandwidth, mDataBandwidth, mPrimarySetting);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });
    }

    private void onAdvancedSelectChanged(int cbw, int dbw, int primCh) {
        updateChannels();
    }

    private void onAdvancedBandwidthSelected() {
        findViewById(R.id.wifi_bandwidth_advanced_ll).setVisibility(View.VISIBLE);
        updateChannels();
    }

    private void updateChannelByAdvancedSetting(int cbw, int dbw, int primCh) {
        Elog.i(TAG, "updateChannelByAdvancedSetting cbw:" + cbw);
        if (cbw == BANDWIDTH_INDEX_40M) { // BW40
            mChannelAdapter.clear();
            mChannel.addSupported2dot4gChannels(mChannelAdapter, false);
            mChannel.removeBw40mUnsupported2dot4GChannels(mChannelAdapter);
            mChannel.insertBw40MChannels(mChannelAdapter);
        } else if (cbw == BANDWIDTH_INDEX_80M) { // BW80
            mChannelAdapter.clear();
            mChannel.insertBw80MChannels(mChannelAdapter);
        }
    }

    @Override
    public void onClick(View arg0) {
        if (!EMWifi.sIsInitialed) {
            showDialog(DIALOG_WIFI_ERROR);
            return;
        }
        if (arg0.getId() == mBtnGo.getId()) {
            onClickBtnRxGo();
        } else if (arg0.getId() == mBtnStop.getId()) {
            onClickBtnRxStop();
        }
    }

    @Override
    protected void onDestroy() {
        mHandler.removeMessages(HANDLER_EVENT_RX);
        if (EMWifi.sIsInitialed) {
            EMWifi.setATParam(1, 0);
        }
        super.onDestroy();
    }

    /**
     * Invoked when "Go" button clicked.
     */
    private void onClickBtnRxGo() {
        setViewEnabled(false);
        EMWifi.getPacketRxStatus(mInitData, 3);
        Elog.i(TAG, "before rx test: rx ok = " + String.valueOf(mInitData[0]) + " fcs error = "
                + String.valueOf(mInitData[1]));

        EMWifi.setATParam(ATPARAM_INDEX_TEMP_COMPENSATION, 0);

        // Bandwidth setting
        if (BW_INDX_ADVANCED == mBandwidthIndex) {
            Elog.i(TAG, "mChannelBandwidth:" + mChannelBandwidth + " mDataBandwidth:"
                    + mDataBandwidth + " mPrimarySetting:" + mPrimarySetting);
            EMWifi.setATParam(ATPARAM_INDEX_CHANNEL_BANDWIDTH, mChannelBandwidth);
            EMWifi.setATParam(ATPARAM_INDEX_DATA_BANDWIDTH, mDataBandwidth);
            EMWifi.setATParam(ATPARAM_INDEX_PRIMARY_SETTING, mPrimarySetting);
        } else {
            EMWifi.setATParam(ATPARAM_INDEX_BANDWIDTH, mBandwidthIndex);
        }
        // start Rx
        EMWifi.setATParam(ATPARAM_INDEX_COMMAND, 2);
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

    /**
     * Set views status.
     *
     * @param state
     *            True if view need to set enabled
     */
    private void setViewEnabled(boolean state) {
        mBtnGo.setEnabled(state);
        mBtnStop.setEnabled(!state);
        mChannelSpinner.setEnabled(state);
        mBandwidthSpinner.setEnabled(state);
        mSpBwAdvCbw.setEnabled(state);
        mSpBwAdvDbw.setEnabled(state);
        mSpBwAdvPrimCh.setEnabled(state);
    }
}
