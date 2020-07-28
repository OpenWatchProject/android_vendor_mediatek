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

package com.mediatek.engineermode.misc;

import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Misc feature config Activity.
 */
public class MiscConfig extends PreferenceActivity implements
        Preference.OnPreferenceChangeListener {
    public static final String FK_MTK_MISC_SELF_REG_CONFIG = "persist.vendor.radio.selfreg";
    public static final String FK_MTK_MISC_VIBRATE_CONFIG = "persist.vendor.radio.telecom.vibrate";
    public static final String FK_MTK_MISC_UCE_SUPPORT = "persist.vendor.mtk_uce_support";
    private static final String TAG = "MiscConfig";
    private static final int MSG_QUERY = 0;
    private static final int MSG_SET = 1;
    private static final int MSG_QUERY_HVOLTE = 2;
    private static final int MSG_QUERY_SLIENT = 3;
    private static final int MSG_QUERY_VOLTE_HYS = 4;
    private static final int MSG_SET_DISABLE_1X_TIME = 5;
    private static final int MSG_SET_ENABLE_1X_TIME = 6;
    private final static int MSG_QUERY_STATUS_1X_TIME = 7;

    private static final String CMD_QUERY = "AT+ECFGGET=\"sms_over_sgs\"";
    private static final String CMD_SET = "AT+ECFGSET=\"sms_over_sgs\"";
    private static final String VALUE_ENABLE = "1";
    private static final String VALUE_DISABLE = "0";
    private static final String[] mEntries = {"bSRLTE", "hVolte"};
    private static final String[] mEntriesValue = {"1", "3"};
    private static final String[] mATCmdValue93md =
            {"AT+EIMSCFG=0,0,0,0,1,1", "AT+EIMSCFG=1,0,0,0,1,1"};
    private static final String[] mATCmdValue9192md =
            {"AT+EIMSVOICE=0", "AT+EIMSVOICE=1"};

    private static final String[] mEGCMDEntries = {"Disable", "Enable"};
    private static final String[] mEGCMDEntriesValue = {"\"00\"", "\"01\""};

    private CheckBoxPreference mSelfRegPreference;
    private CheckBoxPreference mSmsSgsPreference;
    private CheckBoxPreference mDisable1XTime;

    private CheckBoxPreference mVibratePreference;
    private ListPreference mVdmImsReconfigPreference;
    private Preference mPresence;

    private ListPreference mHVolteDeviceModePreference;
    private CheckBoxPreference mSilentRedialPreference;
    private EditTextPreference mTVolteHysPreference;
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            if (msg.what == MSG_QUERY) {
                if (ar.exception == null && ar.result != null) {
                    String[] data = (String[]) ar.result;
                    if (data.length > 0 && (data[0] != null)) {
                        parseSmsSgsValue(data[0]);
                        return;
                    }
                } else {
                    EmUtils.showToast("Query failed");
                }
            } else if (msg.what == MSG_SET) {
                if (ar.exception == null) {
                    EmUtils.showToast("Set successful");
                    Elog.v(TAG, "Set successful");
                } else {
                    EmUtils.showToast("Set failed");
                    Elog.v(TAG, "Set failed");
                }
            } else if (msg.what == MSG_QUERY_HVOLTE) {
                if (ar.exception == null && ar.result != null) {
                    String[] data = (String[]) ar.result;
                    if (data.length > 0 && (data[0] != null)) {
                        String hVolte = "0";
                        try {
                            hVolte = data[0].replace(" ", "")
                                    .substring(("+CEVDP:").length());
                        } catch (Exception e) {
                            Elog.v(TAG, "CEVDP failed ");
                        }
                        Elog.v(TAG, "hVolte = " + hVolte);
                        mHVolteDeviceModePreference.setValue(hVolte);
                        if (mHVolteDeviceModePreference.getEntry() != null) {
                            Elog.d(TAG, "mHVolteDeviceModePreference.getEntry() = " +
                                    mHVolteDeviceModePreference.getEntry());
                            mHVolteDeviceModePreference.setSummary(
                                    mHVolteDeviceModePreference.getEntry());
                        } else {
                            Elog.d(TAG, "mHVolteDeviceModePreference.getEntry() = null");
                        }
                    }
                }
            } else if (msg.what == MSG_QUERY_SLIENT) {
                if (ar.exception == null && ar.result != null) {
                    String[] data = (String[]) ar.result;
                    if (data.length > 0 && (data[0] != null)) {
                        Elog.v(TAG, "EHVOLTE data[0] = " + data[0]);
                        String slient = "0";
                        try {
                            slient = data[0].replace(" ", "")
                                    .substring(("+EHVOLTE:4,").length());
                        } catch (Exception e) {
                            Elog.v(TAG, "+EHVOLTE: failed ");
                        }
                        Elog.v(TAG, "EHVOLTE data = " + slient);
                        mSilentRedialPreference.setChecked
                                (slient.equals("1") ? true : false);
                    }
                }
            } else if (msg.what == MSG_QUERY_VOLTE_HYS) {
                if (ar.exception == null && ar.result != null) {
                    String[] data = (String[]) ar.result;
                    if (data.length > 0 && (data[0] != null)) {
                        String volteHys = "0";
                        Elog.v(TAG, "volteHys data[0] = " + data[0]);
                        try {
                            volteHys = data[0].replace(" ", "")
                                    .substring(("+EVZWT:11,").length());
                        } catch (Exception e) {
                            Elog.v(TAG, "+EVZWT:11 failed ");
                        }
                        Elog.v(TAG, "volteHys data = " + volteHys);
                        mTVolteHysPreference.setText(volteHys);
                        mTVolteHysPreference.setSummary(
                                getString(R.string.misc_config_tvolte) + " : " + volteHys);
                    }
                }
            } else if (msg.what == MSG_SET_DISABLE_1X_TIME) {
                if (ar.exception == null) {
                    EmUtils.showToast("Disable_Time_REG successful.", Toast.LENGTH_SHORT);
                } else {
                    EmUtils.showToast("Disable_Time_REG failed.", Toast.LENGTH_SHORT);
                }
            } else if (msg.what == MSG_SET_ENABLE_1X_TIME) {
                if (ar.exception == null) {
                    EmUtils.showToast("Enable_Time_REG successful.", Toast.LENGTH_SHORT);
                } else {
                    EmUtils.showToast("Enable_Time_REG failed.", Toast.LENGTH_SHORT);
                }
            } else if (msg.what == MSG_QUERY_STATUS_1X_TIME) {
                if (ar.exception == null) {
                    String[] mReturnData = (String[]) ar.result;
                    if (mReturnData.length > 0) {
                        Elog.d(TAG, "close 1x mReturnData = " + mReturnData[0]);
                        String result = "";
                        try {
                            result = mReturnData[0].split(",")[1];
                        } catch (Exception e) {
                            Elog.e(TAG, e.getMessage());
                            Elog.e(TAG, "mReturnData error");
                        }
                        Elog.d(TAG, "result = " + result);
                        if (result.equals("0")) {
                            mDisable1XTime.setChecked(true);
                        } else {
                            mDisable1XTime.setChecked(false);
                        }
                    }
                } else {
                    Elog.d(TAG, "quary MSG_QUERY_STATUS_TIME_REG failed.");
                }
            }
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.misc);

        // Vibrate
        mVibratePreference = new CheckBoxPreference(this);
        mVibratePreference.setSummary(getString(R.string.misc_config_vibrate));
        mVibratePreference.setPersistent(false);
        getPreferenceScreen().addPreference(mVibratePreference);

        // self register
        if (FeatureSupport.isSupported(FeatureSupport.FK_CT4GREG_APP)) {
            mSelfRegPreference = new CheckBoxPreference(this);
            mSelfRegPreference.setSummary(getString(R.string.misc_config_selfreg));
            mSelfRegPreference.setPersistent(false);
            getPreferenceScreen().addPreference(mSelfRegPreference);
        } else {
            Elog.d(TAG, "Not show entry for CT4GREG.");
        }

        // sgs
        mSmsSgsPreference = new CheckBoxPreference(this);
        mSmsSgsPreference.setSummary(getString(R.string.misc_config_sgs));
        mSmsSgsPreference.setPersistent(false);
        getPreferenceScreen().addPreference(mSmsSgsPreference);

        // Disable 1X Time
        if (ModemCategory.isCdma()) {
            mDisable1XTime = new CheckBoxPreference(this);
            mDisable1XTime.setSummary(getString(R.string.misc_config_disable_1X_time));
            mDisable1XTime.setPersistent(false);
            getPreferenceScreen().addPreference(mDisable1XTime);
        }

        // presence
        if (FeatureSupport.getProperty(FK_MTK_MISC_UCE_SUPPORT).equals("1")) {
            Elog.d(TAG, "init presence");
            mPresence = new Preference(this);
            mPresence.setSummary(getString(R.string.presence));
            mPresence.setPersistent(false);
            getPreferenceScreen().addPreference(mPresence);
        }

        if (!(FeatureSupport.is90Modem() || FeatureSupport.is3GOnlyModem())) {
            // volte
            mHVolteDeviceModePreference = new ListPreference(this);
            mHVolteDeviceModePreference.setSummary(getString(R.string.misc_config_hvolte));
            mHVolteDeviceModePreference.setPersistent(false);
            mHVolteDeviceModePreference.setEntries(mEntries);
            mHVolteDeviceModePreference.setEntryValues(mEntriesValue);
            mHVolteDeviceModePreference.setOnPreferenceChangeListener(this);
            getPreferenceScreen().addPreference(mHVolteDeviceModePreference);

            mSilentRedialPreference = new CheckBoxPreference(this);
            mSilentRedialPreference.setSummary(getString(R.string.misc_config_silent));
            mSilentRedialPreference.setPersistent(false);
            getPreferenceScreen().addPreference(mSilentRedialPreference);

            mTVolteHysPreference = new EditTextPreference(this);
            mTVolteHysPreference.setSummary(getString(R.string.misc_config_tvolte));
            mTVolteHysPreference.setPersistent(false);
            mTVolteHysPreference.setOnPreferenceChangeListener(this);
            getPreferenceScreen().addPreference(mTVolteHysPreference);

            mVdmImsReconfigPreference = new ListPreference(this);
            mVdmImsReconfigPreference.setSummary(getString(R.string.misc_config_vdm));
            mVdmImsReconfigPreference.setPersistent(false);
            mVdmImsReconfigPreference.setEntries(mEGCMDEntries);
            mVdmImsReconfigPreference.setEntryValues(mEGCMDEntriesValue);
            mVdmImsReconfigPreference.setOnPreferenceChangeListener(this);
            getPreferenceScreen().addPreference(mVdmImsReconfigPreference);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (FeatureSupport.isSupported(FeatureSupport.FK_CT4GREG_APP)) {
            querySelfRegValue();
        }
        queryVibrateValue();
        querySmsSgsValue();
        if (!(FeatureSupport.is90Modem() || FeatureSupport.is3GOnlyModem())) {
            queryVolteValue();
        }
        if (!FeatureSupport.is90Modem() && ModemCategory.isCdma()) {
            query1XTimeStatus(MSG_QUERY_STATUS_1X_TIME);
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
                                         Preference preference) {
        if (preference == mSelfRegPreference) {
            boolean isChecked = mSelfRegPreference.isChecked();
            String config = getSelfRegConfig();
            if (isChecked) {
                config = config.charAt(0) + "1";
            } else {
                config = config.charAt(0) + "0";
            }
            Elog.i(TAG, "set self reg config value :" + config);
            EmUtils.systemPropertySet(FK_MTK_MISC_SELF_REG_CONFIG, config);
        }

        if (preference == mVibratePreference) {
            boolean sFlag = mVibratePreference.isChecked();
            Elog.i(TAG, "set VibrateValue flag is :" + sFlag);
            EmUtils.systemPropertySet(FK_MTK_MISC_VIBRATE_CONFIG, sFlag ? "1" : "0");
        }

        if (preference == mSmsSgsPreference) {
            setSgsValue(mSmsSgsPreference.isChecked() ? VALUE_ENABLE : VALUE_DISABLE);
        }

        if (preference == mDisable1XTime) {
            if (mDisable1XTime.isChecked()) {
                set1XTime(0, MSG_SET_DISABLE_1X_TIME);
            } else {
                set1XTime(1, MSG_SET_ENABLE_1X_TIME);
            }
        }

        if (preference == mPresence) {
            Intent intent = new Intent();
            intent.setClassName(this, "com.mediatek.engineermode.misc.PresenceActivity");
            startActivity(intent);
        }

        if (preference == mSilentRedialPreference) {
            String enable = mSilentRedialPreference.isChecked() ? VALUE_ENABLE : VALUE_DISABLE;
            sendATCommand(new String[]{"AT+EHVOLTE=4," + enable, ""}, MSG_SET);
        }

        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object objValue) {
        if (preference == mHVolteDeviceModePreference) {
            int index = mHVolteDeviceModePreference.findIndexOfValue((String) objValue);
            Elog.v(TAG, "mHVolteDeviceModePreference index is :" + index);
            String value = "";
            if (FeatureSupport.is93Modem()) {
                value = mATCmdValue93md[index];
            } else if (FeatureSupport.is92Modem() || FeatureSupport.is91Modem()) {
                value = mATCmdValue9192md[index];
            }

            sendATCommand(new String[]{value, ""}, MSG_SET);

            sendATCommand(new String[]{
                    "AT+CEVDP=" + mEntriesValue[index], ""}, MSG_SET);
            mHVolteDeviceModePreference.setSummary(mEntries[index]);
        }
        if (preference == mVdmImsReconfigPreference) {
            int index = mVdmImsReconfigPreference.findIndexOfValue((String) objValue);
            Elog.v(TAG, "mVdmImsReconfigPreference index is :" + index);
            sendATCommand(new String[]{
                    "AT+EGCMD=499,0," + mEGCMDEntriesValue[index], ""}, MSG_SET);
            mHVolteDeviceModePreference.setSummary(mEGCMDEntries[index]);
        }

        if (preference == mTVolteHysPreference) {
            String values = (String) objValue;
            Elog.d(TAG, "Volte Hys value = " + values);

            sendATCommand(new String[]{
                    "AT+EVZWT=1,11," + values, ""}, MSG_SET);

            if (mTVolteHysPreference.getText() != null) {
                mTVolteHysPreference.setSummary(
                        getString(R.string.misc_config_tvolte) + " : " + values);
            } else {
                Elog.d(TAG, "mTVolteHysPreference.getText() = null");
            }
        }
        return true;
    }

    private void queryVibrateValue() {
        final String sFlag = EmUtils.systemPropertyGet(FK_MTK_MISC_VIBRATE_CONFIG, "1");
        Elog.v(TAG, "queryVibrateValue flag is :" + sFlag);

        if (sFlag.equals("0")) {
            mVibratePreference.setChecked(false);
        } else {
            mVibratePreference.setChecked(true);
        }
    }

    private void querySelfRegValue() {
        String config = getSelfRegConfig();

        if (config.charAt(1) == '1') {
            mSelfRegPreference.setChecked(true);
        } else {
            mSelfRegPreference.setChecked(false);
        }
    }

    /**
     * get value of persist.vendor.radio.selfreg.
     * index 0: for devreg via SMS. (1: enable, 0: disable)
     * index 1: for ct4g via http.  (1: enable, 0: disable)
     */
    private String getSelfRegConfig() {
        String config = EmUtils.systemPropertyGet(FK_MTK_MISC_SELF_REG_CONFIG, "11");
        if (!config.equals("11") && !config.equals("10") && !config.equals("01")
                && !config.equals("00")) {
            config = "11";
        }
        Elog.i(TAG, FK_MTK_MISC_SELF_REG_CONFIG + ": " + config);
        return config;
    }

    private void queryVolteValue() {
        queryHVolteDeviceMode();
        querySilentRedialMode();
        queryTVolteHys();
    }

    private void queryHVolteDeviceMode() {
        sendATCommand(new String[]{"AT+CEVDP?", "+CEVDP:"}, MSG_QUERY_HVOLTE);
    }

    private void querySilentRedialMode() {
        sendATCommand(new String[]{"AT+EHVOLTE=4,2", "+EHVOLTE:"}, MSG_QUERY_SLIENT);
    }

    private void queryTVolteHys() {
        sendATCommand(new String[]{"AT+EVZWT=0,11", "+EVZWT:"}, MSG_QUERY_VOLTE_HYS);
    }

    private void set1XTime(int command, int msg) {
        String[] cmd = new String[3];
        cmd[0] = "AT+ECREGTYPE=0," + (command == 1 ? 1 : 0);
        cmd[1] = "";
        cmd[2] = "DESTRILD:C2K";
        String[] cmd_s = ModemCategory.getCdmaCmdArr(cmd);
        Elog.d(TAG, "set1XTime AT command: " + cmd_s[0] + ",cmd_s.length = " + cmd_s.length);
        EmUtils.invokeOemRilRequestStringsEm(true, cmd_s, mHandler.obtainMessage(msg));
    }

    private void query1XTimeStatus(int msg) {
        String[] cmd = new String[3];
        cmd[0] = "AT+ECREGTYPE=0";
        cmd[1] = "+ECREGTYPE:";
        cmd[2] = "DESTRILD:C2K";
        String[] cmd_s = ModemCategory.getCdmaCmdArr(cmd);
        Elog.d(TAG, "query1XTimeStatus: " + cmd_s[0] + ",cmd_s.length = " + cmd_s.length);
        EmUtils.invokeOemRilRequestStringsEm(true, cmd_s, mHandler.obtainMessage(msg));
    }

    private void querySmsSgsValue() {
        Message msg = mHandler.obtainMessage(MSG_QUERY);
        EmUtils.invokeOemRilRequestStringsEm(new String[]{CMD_QUERY, "+ECFGGET:"}, msg);
        Elog.i(TAG, "send " + CMD_QUERY);
    }

    private void setSgsValue(String value) {
        Message msg = mHandler.obtainMessage(MSG_SET);
        EmUtils.invokeOemRilRequestStringsEm(
                new String[]{CMD_SET + ",\"" + value + "\"", ""}, msg);
        Elog.i(TAG, "send " + CMD_SET + ",\"" + value + "\"");
    }

    private void parseSmsSgsValue(String data) {
        mSmsSgsPreference.setChecked(VALUE_ENABLE.equals(parseCommandResponse(data)));
    }

    private String parseCommandResponse(String data) {
        Elog.d(TAG, "reply data: " + data);
        Pattern p = Pattern.compile("\\+ECFGGET:\\s*\".*\"\\s*,\\s*\"(.*)\"");
        Matcher m = p.matcher(data);
        while (m.find()) {
            String value = m.group(1);
            Elog.d(TAG, "sms over sgs support value: " + value);
            return value;
        }
        Elog.e(TAG, "wrong format: " + data);
        EmUtils.showToast("wrong format: " + data);
        return "";
    }

    private void sendATCommand(String[] atCommand, int msg) {
        Elog.d(TAG, "send at cmd : " + atCommand[0]);
        EmUtils.invokeOemRilRequestStringsEm(atCommand, mHandler.obtainMessage(msg));
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        // TODO Auto-generated method stub
        Elog.i(TAG, "fragmentName is " + fragmentName);
        return false;
    }

}
