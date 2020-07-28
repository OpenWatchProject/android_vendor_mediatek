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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERfETO. RECEIVER EXPRESSLY ACKNOWLEDGES
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

package com.mediatek.engineermode.wfdsettings;

import android.app.Activity;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Settings;
import android.view.View;
import android.widget.CheckBox;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.provider.MtkSettingsExt;

/**
 * Class for config WfdLatency.
 *
 */
public class WfdLatency extends Activity {
    private static final String TAG = "WfdLatency";

    private TextView mCapabilityTv = null;
    private TextView mWifiInfoTv = null;
    private TextView mLatencyTv = null;
    private CheckBox mCbProfiling = null;

    /**
     * Wifi Display chosen capability
     * The value is an string. "%s,%s,%s" means (Audio, video, resolution/frame rate)
     */
    private String mCapabilityStr = null;
    /**
     * Wifi Display WIFI info
     * The value is an string. "%d %d %d %d" means (channdlID, AP num, Score, Data rate)
     */
    private String mWifiInfoStr = null;
    /**
     * Wifi Display WFD Latency
     * The value is an string. "%d %d %d" means (avg latency, max latency, timestamp)
     * @hide
     */
    private String mLatencyStr = null;

    /**
     * Enable Wifi Display latency profiling panel
     * The value is an integer. 0: disable & in EM latency screen,
     * 1: enable & in EM latency screen,
     * 2: disable & not in EM latency screen,
     * 3: enable & not in EM latency screen,
     * @hide
     */
    private int mProfilingInfo = 0;

    private WifiContentObserver mWifiObserver = null;
    private LatencyContentObserver mLatencyObserver = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wfd_latency_profiling);

        mCapabilityTv = (TextView) findViewById(R.id.Wfd_CL_Content);
        mWifiInfoTv = (TextView) findViewById(R.id.Wfd_Wifi_Content);
        mLatencyTv = (TextView) findViewById(R.id.Wfd_Latency_Content);

        setCapabilityInfo();

        updateWifiInfo();
        updateLatencyInfo();

        registerContentObserver();

        mProfilingInfo = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_LATENCY_PROFILING, 0);

        mCbProfiling = (CheckBox) findViewById(R.id.Wfd_Profiling_Screen);
        if (mProfilingInfo == 1 || mProfilingInfo == 3) {
            mCbProfiling.setChecked(true);
            setProfilingInfo(1);
        } else {
            mCbProfiling.setChecked(false);
            setProfilingInfo(0);
        }

        mCbProfiling.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                boolean newState = mCbProfiling.isChecked();
                Elog.i(TAG, "EnableSettingMenu : " + newState);
                mProfilingInfo = newState ? 1 : 0;
                setProfilingInfo(mProfilingInfo);
            }
        });

    }

    private void setProfilingInfo(int state) {
         Elog.i(TAG, "setProfilingInfo " + state);
         mProfilingInfo = state;
         Settings.Global.putInt(getContentResolver(),
                 MtkSettingsExt.Global.WIFI_DISPLAY_LATENCY_PROFILING, state);
    }

    private void setCapabilityInfo() {
        mCapabilityStr = Settings.Global.getString(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_CHOSEN_CAPABILITY);
        if (mCapabilityStr == null) {
            mCapabilityTv.setText("Audio: \n" +
            "Video: \n" +
            "Resolution/Frame rate:");
        } else {
            Elog.i(TAG, "Chosen capability info: " + mCapabilityStr);
            String[] strA = mCapabilityStr.split(",");
            String[] titles = {"Audio", "Video", "Resolution/Frame rate", "HDCP", "UIBC"};
            StringBuilder builder = new StringBuilder();
            for (int i = 0; i < titles.length; i++) {
                String content = null;
                if (i < strA.length) {
                    content = strA[i];
                } else {
                    content = "";
                }
                builder.append(titles[i]).append(": ").append(content);
                if (i < titles.length - 1) {
                    builder.append("\n");
                }
            }
            mCapabilityTv.setText(builder.toString());
        }
    }
    private void updateWifiInfo() {
        mWifiInfoStr = Settings.Global.getString(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_WIFI_INFO);
        if (mWifiInfoStr == null) {
            mWifiInfoTv.setText("WFD channel id: \n" +
                    "Surrounding AP num: \n" +
                    "Wifi score: \n" +
                    "Wifi data rate: \n" +
                    "Wifi RSSI:");
        } else {
            Elog.i(TAG, "update wifi info: " + mWifiInfoStr);
            String[] strA = mWifiInfoStr.split(",");

            if (strA.length == 5) {
                 mWifiInfoTv.setText("WFD channel id: " + strA[0] +
                    "\nSurrounding AP num: " + strA[1] +
                    "\nWifi score: " + strA[2] +
                    "\nWifi data rate: " + strA[3] +
                    "\nWifi RSSI: " + strA[4]);
            } else {
                mWifiInfoTv.setText("WFD channel id: \n" +
                    "Surrounding AP num: \n" +
                    "Wifi score: \n" +
                    "Wifi data rate: \n" +
                    "Wifi RSSI:");
            }
        }
    }

    private void updateLatencyInfo() {
        mLatencyStr = Settings.Global.getString(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_WFD_LATENCY);
        if (mLatencyStr == null) {
            mLatencyTv.setText("Latency(avg): ");
        } else {
            Elog.i(TAG, "update latency info: " + mLatencyStr);
            String[] strA = mLatencyStr.split(",");

            if (strA.length == 3) {  // strA[1], strA[2] is always 0, remove it from UI.
                 mLatencyTv.setText("Latency(avg): " + strA[0]);
            } else {
                //mLatencyTv.setText("Receive wrong data format: \n" +  mLatencyStr);
                mLatencyTv.setText("Latency(avg): ");
            }
        }
    }

    private void registerContentObserver() {
        mWifiObserver = new WifiContentObserver(null);
        Uri mUri = Settings.Global.getUriFor(MtkSettingsExt.Global.WIFI_DISPLAY_WIFI_INFO);
        getContentResolver().registerContentObserver(mUri, true, mWifiObserver);
        mLatencyObserver = new LatencyContentObserver(null);
        mUri = Settings.Global.getUriFor(MtkSettingsExt.Global.WIFI_DISPLAY_WFD_LATENCY);
        getContentResolver().registerContentObserver(mUri, true, mLatencyObserver);
    }
    private void unregisterContentObserver() {
        getContentResolver().unregisterContentObserver(mWifiObserver);
        getContentResolver().unregisterContentObserver(mLatencyObserver);
    }
    @Override
    protected void onStop() {
        super.onStop();
        setProfilingInfo(mProfilingInfo + 2);
    }

    @Override
    protected void onRestart() {
        mProfilingInfo = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_LATENCY_PROFILING, 0);

        if (mProfilingInfo == 1 || mProfilingInfo == 3) {
            mCbProfiling.setChecked(true);
            setProfilingInfo(1);
        } else {
            mCbProfiling.setChecked(false);
            setProfilingInfo(0);
        }
        super.onRestart();
    }

    @Override
    protected void onDestroy() {
        unregisterContentObserver();
        super.onDestroy();
    }
    /**
     * Wifi content observer class.
     *
     */
    private class WifiContentObserver extends ContentObserver {
        public WifiContentObserver(Handler handler) {
            super(handler);
        }
        @Override
        public void onChange(boolean selfChange) {
            super.onChange(selfChange);
            Elog.i(TAG, "WifiContentObserver - onChange" + selfChange);

            updateWifiInfo();
        }
    }
    /**
     * Latency content observer class.
     *
     */
    private class LatencyContentObserver extends ContentObserver {
        public LatencyContentObserver(Handler handler) {
            super(handler);
        }
        @Override
        public void onChange(boolean selfChange) {
            super.onChange(selfChange);
            Elog.i(TAG, "LatencyContentObserver - onChange" + selfChange);

            updateLatencyInfo();
        }
    }
}
