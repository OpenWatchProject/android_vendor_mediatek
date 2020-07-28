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

package com.mediatek.engineermode;


import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.UserManager;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.telephony.TelephonyManager;
import android.widget.Toast;

import com.mediatek.engineermode.cxp.CarrierExpressUtil;
import com.mediatek.engineermode.rsc.RuntimeSwitchConfig;
import com.mediatek.engineermode.tellogsetting.TelLogSettingActivity;
import com.mediatek.engineermode.dm.DiagnosticMonitoringSettingActivity;

import java.io.File;

/**
 * PreferenceFragment class for EngineerMode UI items.
 */
public class PrefsFragment extends PreferenceFragment {
    public static final String PR_OPERATOR_OPTR = "persist.vendor.operator.optr";
    public static final String PR_OPERATOR_SEG = "persist.vendor.operator.seg";
    public static final String PR_USB_CBA_SUPPORT = "ro.vendor.mtk_usb_cba_support";
    public static final String PR_MODEM_MONITOR_SUPPORT = "ro.vendor.mtk_modem_monitor_support";
    public static final String PR_SIMME_LOCK_MODE = "ro.vendor.sim_me_lock_mode";
    public static final String PR_SIM_RIL_TESTSIM = "vendor.gsm.sim.ril.testsim";
    public static final String PR_SIM_RIL_TESTSIM2 = "vendor.gsm.sim.ril.testsim.2";
    public static final String PR_SIM_RIL_TESTSIM3 = "vendor.gsm.sim.ril.testsim.3";
    public static final String PR_SIM_RIL_TESTSIM4 = "vendor.gsm.sim.ril.testsim.4";
    private static final String PROPERTY_MCF_SUPPORT = "ro.vendor.mtk_mcf_support";
    private static final String TAG = "PrefsFragment";
    private static final String INNER_LOAD_INDICATOR_FILE =
            "/vendor/etc/system_update/address.xml";
    private static final String[] KEY_REMOVE_ARRAY = {"de_sense", "display", "battery_log",
            "io", "memory"};
    private static final int[] FRAGMENT_RES = {R.xml.telephony,
            R.xml.connectivity, R.xml.hardware_testing, R.xml.location,
            R.xml.log_and_debugging, R.xml.others};
    private static final int[] FRAGMENT_RES_WIFIONLY = {
            R.xml.connectivity, R.xml.hardware_testing, R.xml.location,
            R.xml.log_and_debugging, R.xml.others};
    private int[] mFragmentRest;
    private int mXmlResId;
    private boolean mIsInit = false;

    /* check MCF support */
    public static boolean isMcfSupport() {
        return (SystemProperties.getInt(PROPERTY_MCF_SUPPORT, 0) == 1);
    }

    /**
     * Set this fragment resource.
     *
     * @param resIndex Resource ID
     */
    public void setResource(int resIndex) {
        mXmlResId = resIndex;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (EngineerMode.sWifiOnly) {
            mFragmentRest = FRAGMENT_RES_WIFIONLY;
        } else if (UserHandle.MU_ENABLED && UserManager.supportsMultipleUsers()
                && UserManager.get(getActivity()).getUserHandle() != UserHandle.USER_OWNER) {
            mFragmentRest = FRAGMENT_RES_WIFIONLY;
        } else {
            mFragmentRest = FRAGMENT_RES;
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen screen, Preference pref) {
        if (FeatureSupport.isSupportedEmSrv()) {
            return super.onPreferenceTreeClick(screen, pref);
        }
        String id = pref.getKey();
        for (String c : KEY_REMOVE_ARRAY) {
            if (id.equals(c)) {
                Toast.makeText(getActivity(), R.string.notice_wo_emsvr, Toast.LENGTH_LONG).show();
                return true;
            }
        }
        return super.onPreferenceTreeClick(screen, pref);
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    private void removePreference(PreferenceScreen prefScreen, String prefId) {
        Preference pref = (Preference) findPreference(prefId);
        if (pref != null) {
            prefScreen.removePreference(pref);
        }
    }

    private void removeUnsupportedItems() {
        PreferenceScreen screen = getPreferenceScreen();
        // em info only for 91 modem and later
        if (FeatureSupport.is90Modem() || FeatureSupport.is3GOnlyModem()) {
            removePreference(screen, "mdm_em_info");
            removePreference(screen, "cdma_network_info");
            // LTE CA on/off only for 91 92 93 and later modem
            removePreference(screen, "ltecaconfig");
        }

        if (FeatureSupport.is93Modem()) {
            removePreference(screen, "c2k_ap_ir");
            removePreference(screen, "c2k_ir_settings");
            removePreference(screen, "cdma_network_select");
            removePreference(screen, "u3_phy");
            removePreference(screen, "network_info");
            removePreference(screen, "cdma_network_info");
            removePreference(screen, "test_sim_switch");
            removePreference(screen, "bip");
            removePreference(screen, "iatype");
        }

        if (FeatureSupport.is91Modem() || FeatureSupport.is92Modem()) {
            removePreference(screen, "network_info");
            removePreference(screen, "md_low_power_monitor");
        }

        // anttunerdebug only for 92 93 and later modem
        if (FeatureSupport.is90Modem() || FeatureSupport.is91Modem()
                || FeatureSupport.is3GOnlyModem()) {
            removePreference(screen, "anttunerdebug");
        }

        if (!FeatureSupport.is95Modem()) {
            removePreference(screen, "lte_rx_mimo_configure");
        }

        if (!RuntimeSwitchConfig.supportRsc()) {
            removePreference(screen, "rsc");
        }
        if (!TelLogSettingActivity.isSupport()) {
            removePreference(screen, "telephony_log_setting");
        }
        if (!DiagnosticMonitoringSettingActivity.isSupport()) {
            removePreference(screen, "diagnostic_monitoring_setting");
        }
        if (!(isTestSim()
                || FeatureSupport.isEngLoad()
                || FeatureSupport.isUserDebugLoad()
                || ChipSupport.isFeatureSupported(ChipSupport.MTK_INIERNAL_LOAD))) {
            removePreference(screen, "auto_answer");
            Elog.d(TAG, "Hide the auto_answer item");
        }
        if(!FeatureSupport.isSupportTelephony(this.getActivity())){
            removePreference(screen, "auto_answer");
        }

        if (!(ChipSupport.isFeatureSupported(ChipSupport.MTK_INIERNAL_LOAD)
                || FeatureSupport.isEngLoad() || FeatureSupport.isUserDebugLoad())) {
            Elog.i(TAG, "it is customer user load!");
            removePreference(screen, "atci");
            removePreference(screen, "bip");
            removePreference(screen, "voice_settings");
            removePreference(screen, "md_log_filter");
            removePreference(screen, "video_log");
        }

        if (FeatureSupport.isUserDebugLoad()) {
            removePreference(screen, "battery_log");
        }

        if (FeatureSupport.isUserLoad()) {
            //telephony
            removePreference(screen, "amr_wb");
            removePreference(screen, "lte_tool");
            removePreference(screen, "cmas");
            removePreference(screen, "diagnostic");
            removePreference(screen, "fast_dormancy");
            removePreference(screen, "gprs");
            removePreference(screen, "hspa_info");
            removePreference(screen, "iatype");
            removePreference(screen, "md_em_filter");
            removePreference(screen, "sbp");
            removePreference(screen, "spc");
            removePreference(screen, "sim_switch");
            removePreference(screen, "simme_lock1");
            removePreference(screen, "simme_lock2");
            removePreference(screen, "u3_phy");
            removePreference(screen, "gwsd_setting");
            removePreference(screen, "noise_profiling_tool");

            //connectivity
            removePreference(screen, "bt_test_tool");
            removePreference(screen, "cds_information");
            removePreference(screen, "fm_receiver");
            removePreference(screen, "fm_transmitter");
            removePreference(screen, "wfd_settings");

            //hardware testing
            removePreference(screen, "aal");
            removePreference(screen, "de_sense");
            removePreference(screen, "io");
            removePreference(screen, "memory");
            removePreference(screen, "power");
            removePreference(screen, "sensor_hub_test");
            removePreference(screen, "usbmode_switch");
            removePreference(screen, "usb");
            removePreference(screen, "usb_speed");

            //Location
            removePreference(screen, "auto_dialer");
            removePreference(screen, "clk_quality_at");
            removePreference(screen, "cw_test");
            removePreference(screen, "fused_location_provider");
            removePreference(screen, "desense_at");

            //log and debugging
            removePreference(screen, "battery_log");
            removePreference(screen, "modem_reset_delay");
            removePreference(screen, "modem_warning");
            removePreference(screen, "wcn_core_dump");

            //others
            removePreference(screen, "carrier_express");
            removePreference(screen, "mdml_sample");
            removePreference(screen, "swla");
            removePreference(screen, "system_update");
            removePreference(screen, "usbacm");
            removePreference(screen, "usb_checker_enabler");
        }

        // AGPS is not ready if MTK_AGPS_APP isn't defined
        if (!isVoiceCapable() || EngineerMode.sWifiOnly) {
            removePreference(screen, "auto_dialer");
        }

        if (!FeatureSupport.isGpsSupport(getActivity())) {
            removePreference(screen, "ygps");
            removePreference(screen, "cw_test");
        }

        if (!FeatureSupport.isWifiSupport(getActivity())) {
            removePreference(screen, "wifi");
        }

        if (!FeatureSupport.isSupported(FeatureSupport.FK_APC_SUPPORT)) {
            removePreference(screen, "apc");
        }

        // if it single sim, then the flow is the same as before
        if (TelephonyManager.getDefault().getSimCount() > 1) {
            removePreference(screen, "simme_lock1");
        } else {
            removePreference(screen, "simme_lock2");
        }

        Intent intent = new Intent();
        intent.setClassName("com.mediatek.emcamera",
                "com.mediatek.emcamera.AutoCalibration");
        if (!isActivityAvailable(intent)) {
            Elog.i(TAG, "emcamera is not available");
            removePreference(screen, "camerasolo");
        }

        if (!FeatureSupport.isSupported(FeatureSupport.FK_FD_SUPPORT)) {
            removePreference(screen, "fast_dormancy");
        }

        File innerLoadIndicator = new File(INNER_LOAD_INDICATOR_FILE);
        if (!innerLoadIndicator.exists()) {
            removePreference(screen, "system_update");
        }

        if (!FeatureSupport.isPackageExisted(this.getActivity(), FeatureSupport.PK_CDS_EM)) {
            removePreference(screen, "cds_information");
        }

        Preference pref = (Preference) findPreference("cmas");
        if (pref != null && !isActivityAvailable(pref.getIntent())) {
            removePreference(screen, "cmas");
        }

        String mOptr = SystemProperties.get(PR_OPERATOR_OPTR);

        if (!"OP12".equals(mOptr)) {
            removePreference(screen, "spc");
        }

        if (!(("OP01".equals(mOptr)) &&
                (ModemCategory.isCdma()))) {
            removePreference(screen, "c2k_ap_ir");
        }

        if (!("OP07".equals(mOptr))) {
            removePreference(screen, "sim_info");
            removePreference(screen, "diagnostic");
        }

        String usbCheckerState = SystemProperties.get(PR_USB_CBA_SUPPORT, "0");
        if (!usbCheckerState.equals("1")) {
            removePreference(screen, "usb_checker_enabler");
        }

        if (!(SystemProperties.get(PR_MODEM_MONITOR_SUPPORT).equals("1"))) {
            removePreference(screen, "mdml_sample");
            removePreference(screen, "mdm_em_info");
            removePreference(screen, "mdm_config");
        }

        if (!ModemCategory.isCdma()) { // For C2K
            removePreference(screen, "cdma_network_select");
        }

        if (!FeatureSupport.isSupported(FeatureSupport.FK_DEVREG_APP)) {
            removePreference(screen, "device_register");
        }

        if (!FeatureSupport.isSupported(FeatureSupport.FK_WFD_SUPPORT)) {
            removePreference(screen, "wfd_settings");
        }

        if (!ModemCategory.isLteSupport()) {
            removePreference(screen, "lte_network_info");
        }

        if (!CarrierExpressUtil.isCarrierExpressSupported()) {
            removePreference(screen, "carrier_express");
        }

        if (!ModemCategory.isLteSupport()
                && ChipSupport.getChip() <= ChipSupport.MTK_6735_SUPPORT) {
            removePreference(screen, "antenna");
        }
        if (!FeatureSupport.isSupported(FeatureSupport.FK_IMS_SUPPORT)) {
            removePreference(screen, "ims");
        }


        if (!(SystemProperties.get(PR_SIMME_LOCK_MODE, "0").equals("0") ||
                SystemProperties.get(PR_SIMME_LOCK_MODE, "0").equals("2") ||
                SystemProperties.get(PR_SIMME_LOCK_MODE, "0").equals("3"))
                ) {
            removePreference(screen, "simme_lock1");
            removePreference(screen, "simme_lock2");
        }

        if (!FeatureSupport.isSupported(FeatureSupport.FK_AAL_SUPPORT)) {
            removePreference(screen, "aal");
        }

        if (ChipSupport.getChip() < ChipSupport.MTK_6735_SUPPORT) {
            removePreference(screen, "amr_wb");
        }
        if (!ModemCategory.isCdma()) {
            removePreference(screen, "c2k_ir_settings");
        }

        if (!FeatureSupport.isSupported(FeatureSupport.FK_VILTE_SUPPORT)) {
            removePreference(screen, "vilte");
        }

        if (!ModemCategory.isCdma()) {
            removePreference(screen, "bypass");
        }
        if (!FeatureSupport.isSupported(FeatureSupport.FK_MD_WM_SUPPORT)) {
            removePreference(screen, "rat_config");
        }

        if (WorldModeUtil.isWorldPhoneSupport()) {
            if (WorldModeUtil.isWorldModeSupport()) {
                removePreference(screen, "modem_switch");
            } else {
                removePreference(screen, "world_mode");
            }
        } else {
            removePreference(screen, "world_mode");
            removePreference(screen, "modem_switch");
        }

        //RTN menu is an operator specific feature
        if (!"OP20".equals(mOptr)) {
            removePreference(screen, "rtn");
        }

        if (!isMcfSupport()) {
            removePreference(screen, "mcf_config");
        }

        if (!FeatureSupport.isMtkTelephonyAddOnPolicyEnable()) {
            removePreference(screen, "aal");
            removePreference(screen, "camerasolo");
            removePreference(screen, "voice_settings");
            removePreference(screen, "wfd_settings");
            removePreference(screen, "modem_switch");
            removePreference(screen, "sim_info");
            removePreference(screen, "iatype");
            removePreference(screen, "rtn");
            removePreference(screen, "apc");
            removePreference(screen, "bip");
            removePreference(screen, "c2k_ap_ir");
            removePreference(screen, "c2k_ir_settings");
            removePreference(screen, "cfu");
            removePreference(screen, "emergency_num_key");
            removePreference(screen, "gprs");
            removePreference(screen, "test_sim_switch");
            removePreference(screen, "cdma_network_info");
            removePreference(screen, "network_info");
            removePreference(screen, "diagnostic");
            removePreference(screen, "cmas");
            removePreference(screen, "simme_lock1");
            removePreference(screen, "simme_lock2");
            removePreference(screen, "sim_recoverytest_tool");
            removePreference(screen, "bt_test_tool");
            removePreference(screen, "iot_config");
            removePreference(screen, "misc_config");
            removePreference(screen, "gwsd_setting");
        }

        if (!FeatureSupport.isSupported(FeatureSupport.FK_GWSD_SUPPORT)) {
            removePreference(screen, "gwsd_setting");
        }

        if (ChipSupport.isFeatureSupported(ChipSupport.MTK_TC1_COMMON_SERVICE)) {
            Elog.d(TAG, "it is tc1 branch");
            removePreference(screen, "force_tx");
            removePreference(screen, "anttunerdebug");
            removePreference(screen, "antenna");
            removePreference(screen, "apc");
            removePreference(screen, "lte_tool");
            removePreference(screen, "auto_answer");
            removePreference(screen, "ehrpd_bg_data");
            removePreference(screen, "bip");
            removePreference(screen, "c2k_ap_ir");
            removePreference(screen, "cfu");
            removePreference(screen, "channel_lock");
            removePreference(screen, "cmas");
            removePreference(screen, "diagnostic");
            removePreference(screen, "emergency_num_key");
            removePreference(screen, "gwsd_setting");
            removePreference(screen, "iatype");
            removePreference(screen, "ims");
            removePreference(screen, "iot_config");
            removePreference(screen, "ltecaconfig");
            removePreference(screen, "lte_hpue_configue");
            removePreference(screen, "lte_rx_mimo_configure");
            removePreference(screen, "mcf_config");
            removePreference(screen, "md_em_filter");
            removePreference(screen, "md_low_power_monitor");
            removePreference(screen, "misc_config");
            removePreference(screen, "mobile_data_prefer");
            removePreference(screen, "network_info");
            removePreference(screen, "noise_profiling_tool");
            removePreference(screen, "ota_airplane_mode");
            removePreference(screen, "rtn");
            removePreference(screen, "spc");
            removePreference(screen, "sim_info");
            removePreference(screen, "sim_switch");
            removePreference(screen, "simme_lock1");
            removePreference(screen, "simme_lock2");
            removePreference(screen, "sim_recoverytest_tool");
            removePreference(screen, "test_sim_switch");
            removePreference(screen, "u3_phy");
            removePreference(screen, "usb_tethering");
            removePreference(screen, "vilte");
            removePreference(screen, "wifi_calling");
            removePreference(screen, "modem_switch");

            removePreference(screen, "bt_test_tool");
            removePreference(screen, "connsys_patch_info");
            removePreference(screen, "nfc_st");
            removePreference(screen, "wfd_settings");

            removePreference(screen, "modem_reset_delay");
            removePreference(screen, "modem_warning");
            removePreference(screen, "video_log");
            removePreference(screen, "wcn_core_dump");

            removePreference(screen, "carrier_express");
            removePreference(screen, "moms");
            removePreference(screen, "voice_settings");
        }else{
            removePreference(screen, "security_status");
            removePreference(screen, "antenna_diversity");
        }

    }

    private boolean isVoiceCapable() {
        TelephonyManager telephony = (TelephonyManager) getActivity()
                .getSystemService(Context.TELEPHONY_SERVICE);
        boolean bVoiceCapable = telephony != null && telephony.isVoiceCapable();
        Elog.i(TAG, "sIsVoiceCapable : " + bVoiceCapable);
        return bVoiceCapable;
    }

    private boolean isTestSim() {
        boolean isTestSim = SystemProperties.get(PR_SIM_RIL_TESTSIM).equals("1") ||
                SystemProperties.get(PR_SIM_RIL_TESTSIM2).equals("1") ||
                SystemProperties.get(PR_SIM_RIL_TESTSIM3).equals("1") ||
                SystemProperties.get(PR_SIM_RIL_TESTSIM4).equals("1");
        return isTestSim;
    }

    private boolean isActivityAvailable(Intent intent) {
        return null != getActivity().getPackageManager().resolveActivity(intent, 0);
    }

    @Override
    public void setUserVisibleHint(boolean isVisibleToUser) {
        // TODO Auto-generated method stub
        super.setUserVisibleHint(isVisibleToUser);
        Elog.i(TAG, "setUserVisibleHint : " + isVisibleToUser + " index of " + mXmlResId);
        if (isVisibleToUser) {
            if (!mIsInit) {
                // Load preferences from xml.
                addPreferencesFromResource(mFragmentRest[mXmlResId]);
                removeUnsupportedItems();
                mIsInit = true;
            }
            PreferenceScreen screen = getPreferenceScreen();

            int count = screen.getPreferenceCount();
            for (int i = count - 1; i >= 0; i--) {
                Preference pre = screen.getPreference(i);
                if (null != pre) {
                    Intent intent = pre.getIntent();
                    if (!isActivityAvailable(intent)) {
                        screen.removePreference(pre);
                        Elog.i(TAG, "intent : " + intent);
                    }
                }
            }
        }
    }


}
