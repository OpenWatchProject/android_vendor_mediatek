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

package com.mediatek.engineermode.power;

import android.app.TabActivity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.InputFilter;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TabHost;
import android.widget.TabHost.OnTabChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;

import java.io.File;
import java.io.IOException;

public class PMU6575 extends TabActivity implements OnClickListener {
    private static final String TAG = "Power/PMU";
    private static final int TAB_REG = 1;
    private static final int TAB_INFO = 2;
    private static final int EVENT_UPDATE = 1;
    private static final int MAX_LENGTH_89 = 4;

    private static final int RADIX_HEX = 16;
    private static final int UPDATE_INTERVAL = 1500;
    private static final int WAIT_INTERVAL = 500;
    private static final float MAGIC_TEN = 10.0f;

    private static final String FS_MT6333_ACCESS = "/sys/devices/platform/mt6333-user/mt6333_access";
    private static final String FS_MT6311_ACCESS = "/sys/devices/platform/mt6311-user/mt6311_access";

    private static final String[] FS_SUB_REG_ADDRS = new String[] {
            "d/rt-regmap/mt6360_pmu.5-0034/reg_addr",
            "d/rt-regmap/mt6360_pmic.5-001a/reg_addr",
            "d/rt-regmap/mt6360_ldo.5-0064/reg_addr",
            "d/rt-regmap/mt6360-4e/reg_addr"};
    private static final String[] FS_SUB_REG_DATAS = new String[] {
            "d/rt-regmap/mt6360_pmu.5-0034/data",
            "d/rt-regmap/mt6360_pmic.5-001a/data",
            "d/rt-regmap/mt6360_ldo.5-0064/data",
            "d/rt-regmap/mt6360-4e/data"};

    private static final String INFO_PATH = "/sys/bus/platform/devices/mt-pmic/";
    private static final String INFO_FILES = "buck_ldo_info";

    static String[][] mFileArray = null;

    private int mWhichTab = TAB_INFO;
    private TextView mInfo = null;
    private Button mBtnGetRegister = null;
    private Button mBtnSetRegister = null;
    private EditText mEditAddr = null;
    private EditText mEditVal = null;
    private Button mBtnGetReg6333 = null;
    private Button mBtnSetReg6333 = null;
    private EditText mEditAddr6333 = null;
    private EditText mEditVal6333 = null;
    private String mFSAccessFile;

    private Spinner mSubPmicSpinner;
    private Button mBtnGetRegSub;
    private Button mBtnSetRegSub;
    private EditText mEditAddrSub;
    private EditText mEditValSub;

    private ArrayAdapter<String> mBankAdatper;
    private String[] mBankType = { "Bank0", "Bank1" };
    private int mBankIndex = 0;
    private boolean mRun = false;
    private String mPromptSw = "0/1=off/on";
    private String mPromptUnit = "mV";
    private String mPromptUv = "uV";
    private String[][] mFiles = { { "BUCK_VAPROC_STATUS", mPromptSw },
            { "BUCK_VCORE_STATUS", mPromptSw }, { "BUCK_VIO18_STATUS", mPromptSw },
            { "BUCK_VPA_STATUS", mPromptSw }, { "BUCK_VRF18_STATUS", mPromptSw }, { "SEP", "" },
            { "LDO_VA1_STATUS", mPromptSw }, { "LDO_VA2_STATUS", mPromptSw },
            { "LDO_VCAM_AF_STATUS", mPromptSw }, { "LDO_VCAM_IO_STATUS", mPromptSw },
            { "LDO_VCAMA_STATUS", mPromptSw }, { "LDO_VCAMD_STATUS", mPromptSw },
            { "LDO_VGP_STATUS", mPromptSw }, { "LDO_VGP2_STATUS", mPromptSw },
            { "LDO_VIBR_STATUS", mPromptSw }, { "LDO_VIO28_STATUS", mPromptSw },
            { "LDO_VM12_1_STATUS", mPromptSw }, { "LDO_VM12_2_STATUS", mPromptSw },
            { "LDO_VM12_INT_STATUS", mPromptSw }, { "LDO_VMC_STATUS", mPromptSw },
            { "LDO_VMCH_STATUS", mPromptSw }, { "LDO_VRF_STATUS", mPromptSw },
            { "LDO_VRTC_STATUS", mPromptSw }, { "LDO_VSIM_STATUS", mPromptSw },
            { "LDO_VSIM2_STATUS", mPromptSw }, { "LDO_VTCXO_STATUS", mPromptSw },
            { "LDO_VUSB_STATUS", mPromptSw }, { "SEP", "" },
            { "BUCK_VAPROC_VOLTAGE", mPromptUnit }, { "BUCK_VCORE_VOLTAGE", mPromptUnit },
            { "BUCK_VIO18_VOLTAGE", mPromptUnit }, { "BUCK_VPA_VOLTAGE", mPromptUnit },
            { "BUCK_VRF18_VOLTAGE", mPromptUnit }, { "SEP", "" },
            { "LDO_VA1_VOLTAGE", mPromptUnit }, { "LDO_VA2_VOLTAGE", mPromptUnit },
            { "LDO_VCAM_AF_VOLTAGE", mPromptUnit }, { "LDO_VCAM_IO_VOLTAGE", mPromptUnit },
            { "LDO_VCAMA_VOLTAGE", mPromptUnit }, { "LDO_VCAMD_VOLTAGE", mPromptUnit },
            { "LDO_VGP_VOLTAGE", mPromptUnit }, { "LDO_VGP2_VOLTAGE", mPromptUnit },
            { "LDO_VIBR_VOLTAGE", mPromptUnit }, { "LDO_VIO28_VOLTAGE", mPromptUnit },
            { "LDO_VM12_1_VOLTAGE", mPromptUnit }, { "LDO_VM12_2_VOLTAGE", mPromptUnit },
            { "LDO_VM12_INT_VOLTAGE", mPromptUnit }, { "LDO_VMC_VOLTAGE", mPromptUnit },
            { "LDO_VMCH_VOLTAGE", mPromptUnit }, { "LDO_VRF_VOLTAGE", mPromptUnit },
            { "LDO_VRTC_VOLTAGE", mPromptUnit }, { "LDO_VSIM_VOLTAGE", mPromptUnit },
            { "LDO_VSIM2_VOLTAGE", mPromptUnit }, { "LDO_VTCXO_VOLTAGE", mPromptUnit },
            { "LDO_VUSB_VOLTAGE", mPromptUnit } };
    private String[][] mFileFor6755 = { { "BUCK_VCORE_STATUS", mPromptSw },
            { "BUCK_VGPU_STATUS", mPromptSw }, { "BUCK_VSRAM_PROC_STATUS", mPromptSw },
            { "BUCK_VMODEM_STATUS", mPromptSw }, { "BUCK_VMD1_STATUS", mPromptSw },
            { "BUCK_VSRAM_MD_STATUS", mPromptSw }, { "BUCK_VS1_STATUS", mPromptSw },
            { "BUCK_VS2_STATUS", mPromptSw }, { "BUCK_VPA_STATUS", mPromptSw }, { "SEP", "" },
            { "LDO_va18_STATUS", mPromptSw }, { "LDO_vtcxo28_STATUS", mPromptSw },
            { "LDO_vtcxo24_STATUS", mPromptSw }, { "LDO_vcn28_STATUS", mPromptSw },
            { "LDO_vcama_STATUS", mPromptSw }, { "LDO_vcn33_bt_STATUS", mPromptSw },
            { "LDO_vcn33_wifi_STATUS", mPromptSw }, { "LDO_vusb33_STATUS", mPromptSw },
            { "LDO_vefuse_STATUS", mPromptSw }, { "LDO_vsim1_STATUS", mPromptSw },
            { "LDO_vsim2_STATUS", mPromptSw }, { "LDO_vemc_STATUS", mPromptSw },
            { "LDO_vmch_STATUS", mPromptSw }, { "LDO_vmc_STATUS", mPromptSw },
            { "LDO_vio28_STATUS", mPromptSw }, { "LDO_vldo28_STATUS", mPromptSw },
            { "LDO_vibr_STATUS", mPromptSw }, { "LDO_vcamd_STATUS", mPromptSw },
            { "LDO_vrf18_STATUS ", mPromptSw }, { "LDO_vio18_STATUS ", mPromptSw },
            { "LDO_vcn18_STATUS", mPromptSw }, { "LDO_vcamio_STATUS", mPromptSw },
            { "LDO_vxo22_STATUS", mPromptSw }, { "LDO_vrf12_STATUS", mPromptSw },
            { "LDO_va10_STATUS", mPromptSw }, { "LDO_vdram_STATUS ", mPromptSw },
            { "LDO_vmipi_STATUS ", mPromptSw }, { "LDO_vgp3_STATUS", mPromptSw },
            { "LDO_vbif28_STATUS", mPromptSw }, { "SEP", "" },
            { "BUCK_VCORE_VOLTAGE", mPromptUnit }, { "BUCK_VGPU_VOLTAGE", mPromptUnit },
            { "BUCK_VSRAM_PROC_VOLTAGE", mPromptUnit }, { "BUCK_VMODEM_VOLTAGE", mPromptUnit },
            { "BUCK_VMD1_VOLTAGE", mPromptUnit }, { "BUCK_VSRAM_MD_VOLTAGE", mPromptUnit },
            { "BUCK_VS1_VOLTAGE", mPromptUnit }, { "BUCK_VS2_VOLTAGE", mPromptUnit },
            { "BUCK_VPA_VOLTAGE", mPromptUnit }, { "SEP", "" },
            { "LDO_va18_VOLTAGE", mPromptUnit }, { "LDO_vtcxo28_VOLTAGE", mPromptUnit },
            { "LDO_vtcxo24_VOLTAGE", mPromptUnit }, { "LDO_vcn28_VOLTAGE", mPromptUnit },
            { "LDO_vcama_VOLTAGE", mPromptUnit }, { "LDO_vcn33_bt_VOLTAGE", mPromptUnit },
            { "LDO_vcn33_wifi_VOLTAGE", mPromptUnit }, { "LDO_vusb33_VOLTAGE", mPromptUnit },
            { "LDO_vefuse_VOLTAGE", mPromptUnit }, { "LDO_vsim1_VOLTAGE", mPromptUnit },
            { "LDO_vsim2_VOLTAGE", mPromptUnit }, { "LDO_vemc_VOLTAGE", mPromptUnit },
            { "LDO_vmch_VOLTAGE", mPromptUnit }, { "LDO_vmc_VOLTAGE", mPromptUnit },
            { "LDO_vio28_VOLTAGE", mPromptUnit }, { "LDO_vldo28_VOLTAGE", mPromptUnit },
            { "LDO_vibr_VOLTAGE", mPromptUnit }, { "LDO_vcamd_VOLTAGE", mPromptUnit },
            { "LDO_vrf18_VOLTAGE", mPromptUnit }, { "LDO_vio18_VOLTAGE", mPromptUnit },
            { "LDO_vcn18_VOLTAGE", mPromptUnit }, { "LDO_vcamio_VOLTAGE", mPromptUnit },
            { "LDO_vxo22_VOLTAGE", mPromptUnit }, { "LDO_vrf12_VOLTAGE", mPromptUnit },
            { "LDO_va10_VOLTAGE", mPromptUnit }, { "LDO_vdram_VOLTAGE", mPromptUnit },
            { "LDO_vmipi_VOLTAGE", mPromptUnit }, { "LDO_vgp3_VOLTAGE", mPromptUnit },
            { "LDO_vbif28_VOLTAGE", mPromptUnit }, };
    private String[][] mFilesFor6750 = { { "BUCK_VPROC_STATUS", "1/1=off/on" },
            { "BUCK_VCORE_STATUS", "0/1=off/on" }, { "BUCK_VCORE2_STATUS", "0/1=off/on" },
            { "BUCK_VPA_STATUS", "0/1=off/on" }, { "BUCK_VS1_STATUS", "0/1=off/on" },
            { "SEP", "" }, { "LDO_VTCXO24_STATUS", "0/1=off/on" },
            { "LDO_VXO22_STATUS", "0/1=off/on" }, { "LDO_VCN33_BT_STATUS", "0/1=off/on" },
            { "LDO_VCN33_WIFI_STATUS", "0/1=off/on" }, { "LDO_VSRAM_PROC_STATUS", "0/1=off/on" },
            { "LDO_VLDO28_STATUS", "0/1=off/on" }, { "LDO_VLDO28_1_STATUS", "0/1=off/on" },
            { "LDO_VTCXO28_STATUS", "0/1=off/on" }, { "LDO_VRF18_STATUS", "0/1=off/on" },
            { "LDO_VRF12_STATUS", "0/1=off/on" }, { "LDO_VCN28_STATUS", "0/1=off/on" },
            { "LDO_VCN18_STATUS", "0/1=off/on" }, { "LDO_VCAMA_STATUS", "0/1=off/on" },
            { "LDO_VCAMIO_STATUS", "0/1=off/on" }, { "LDO_VCAMD_STATUS", "0/1=off/on" },
            { "LDO_VAUX18_STATUS ", "0/1=off/on" }, { "LDO_VAUD28_STATUS ", "0/1=off/on" },
            { "LDO_VDRAM_STATUS", "0/1=off/on" }, { "LDO_VSIM1_STATUS", "0/1=off/on" },
            { "LDO_VSIM2_STATUS", "0/1=off/on" }, { "LDO_VIO28_STATUS", "0/1=off/on" },
            { "LDO_VMC_STATUS", "0/1=off/on" }, { "LDO_VMCH_STATUS ", "0/1=off/on" },
            { "LDO_VUSB33_STATUS ", "0/1=off/on" }, { "LDO_VEMC_STATUS", "0/1=off/on" },
            { "LDO_VIO18_STATUS", "0/1=off/on" }, { "LDO_VIBR_STATUS", "0/1=off/on" },
            { "SEP", "" }, { "BUCK_VPROC_VOLTAGE", "mV" }, { "BUCK_VCORE_VOLTAGE", "mV" },
            { "BUCK_VCORE2_VOLTAGE", "mV" }, { "BUCK_VPA_PROC_VOLTAGE", "mV" },
            { "BUCK_VS1_VOLTAGE", "mV" }, { "SEP", "" }, { "LDO_VTCXO24_VOLTAGE", "mv" },
            { "LDO_VXO22_VOLTAGE", "mv" }, { "LDO_VCN33_BT_VOLTAGE", "mv" },
            { "LDO_VCN33_WIFI_VOLTAGE", "mv" }, { "LDO_VSRAM_PROC_VOLTAGE", "mv" },
            { "LDO_VLDO28_VOLTAGE", "mv" }, { "LDO_VLDO28_1_VOLTAGE", "mv" },
            { "LDO_VTCXO28_VOLTAGE", "mv" }, { "LDO_VRF18_VOLTAGE", "mv" },
            { "LDO_VRF12_VOLTAGE", "mv" }, { "LDO_VCN28_VOLTAGE", "mv" },
            { "LDO_VCN18_VOLTAGE", "mv" }, { "LDO_VCAMA_VOLTAGE", "mv" },
            { "LDO_VCAMIO_VOLTAGE", "mv" }, { "LDO_VCAMD_VOLTAGE", "mv" },
            { "LDO_VAUX18_VOLTAGE ", "mv" }, { "LDO_VAUD28_VOLTAGE ", "mv" },
            { "LDO_VDRAM_VOLTAGE", "mv" }, { "LDO_VSIM1_VOLTAGE", "mv" },
            { "LDO_VSIM2_VOLTAGE", "mv" }, { "LDO_VIO28_VOLTAGE", "mv" },
            { "LDO_VMC_VOLTAGE", "mv" }, { "LDO_VMCH_VOLTAGE ", "mv" },
            { "LDO_VUSB33_VOLTAGE ", "mv" }, { "LDO_VEMC_VOLTAGE", "mv" },
            { "LDO_VIO18_VOLTAGE", "mv" }, { "LDO_VIBR_VOLTAGE", "mv" }, };
    private String[][] mFileFor35 = { { "BUCK_VCORE1_STATUS", mPromptSw },
            { "BUCK_VLTE_STATUS", mPromptSw }, { "BUCK_VPA_STATUS", mPromptSw },
            { "BUCK_VPROC_STATUS", mPromptSw }, { "BUCK_VSYS22_STATUS", mPromptSw }, { "SEP", "" },
            { "LDO_TREF_STATUS", mPromptSw }, { "LDO_VAUD28_STATUS", mPromptSw },
            { "LDO_VAUX18_STATUS", mPromptSw }, { "LDO_VCAMAF_STATUS", mPromptSw },
            { "LDO_VCAMA_STATUS", mPromptSw }, { "LDO_VCAMD_STATUS", mPromptSw },
            { "LDO_VCAMIO_STATUS", mPromptSw }, { "LDO_VCN18_STATUS", mPromptSw },
            { "LDO_VCN28_STATUS", mPromptSw }, { "LDO_VCN33_BT_STATUS", mPromptSw },
            { "LDO_VCN33_WIFI_STATUS", mPromptSw }, { "LDO_VEFUSE_STATUS", mPromptSw },
            { "LDO_VEMC_3V3_STATUS", mPromptSw }, { "LDO_VGP1_STATUS", mPromptSw },
            { "LDO_VIBR_STATUS", mPromptSw }, { "LDO_VIO18_STATUS", mPromptSw },
            { "LDO_VIO28_STATUS", mPromptSw }, { "LDO_VMCH_STATUS", mPromptSw },
            { "LDO_VMC_STATUS", mPromptSw }, { "LDO_VM_STATUS", mPromptSw },
            { "LDO_VRF18_0_STATUS", mPromptSw }, { "LDO_VRF18_1_STATUS", mPromptSw },
            { "LDO_VSIM1_STATUS", mPromptSw }, { "LDO_VSIM2_STATUS", mPromptSw },
            { "LDO_VSRAM_STATUS", mPromptSw }, { "LDO_VTCXO_0_STATUS", mPromptSw },
            { "LDO_VTCXO_1_STATUS", mPromptSw }, { "LDO_VUSB33_STATUS", mPromptSw }, { "SEP", "" },
            { "BUCK_VCORE1_VOLTAGE", mPromptUv }, { "BUCK_VLTE_VOLTAGE", mPromptUv },
            { "BUCK_VPA_VOLTAGE", mPromptUv }, { "BUCK_VPROC_VOLTAGE", mPromptUv },
            { "BUCK_VSYS22_VOLTAGE", mPromptUv }, { "SEP", "" },
            { "LDO_TREF_VOLTAGE", mPromptUnit }, { "LDO_VAUD28_VOLTAGE", mPromptUnit },
            { "LDO_VAUX18_VOLTAGE", mPromptUnit }, { "LDO_VCAMAF_VOLTAGE", mPromptUnit },
            { "LDO_VCAMA_VOLTAGE", mPromptUnit }, { "LDO_VCAMD_VOLTAGE", mPromptUnit },
            { "LDO_VCAMIO_VOLTAGE", mPromptUnit }, { "LDO_VCN18_VOLTAGE", mPromptUnit },
            { "LDO_VCN28_VOLTAGE", mPromptUnit }, { "LDO_VCN33_BT_VOLTAGE", mPromptUnit },
            { "LDO_VCN33_WIFI_VOLTAGE", mPromptUnit }, { "LDO_VEFUSE_VOLTAGE", mPromptUnit },
            { "LDO_VEMC_3V3_VOLTAGE", mPromptUnit }, { "LDO_VGP1_VOLTAGE", mPromptUnit },
            { "LDO_VIBR_VOLTAGE", mPromptUnit }, { "LDO_VIO18_VOLTAGE", mPromptUnit },
            { "LDO_VIO28_VOLTAGE", mPromptUnit }, { "LDO_VMCH_VOLTAGE", mPromptUnit },
            { "LDO_VMC_VOLTAGE", mPromptUnit }, { "LDO_VM_VOLTAGE", mPromptUnit },
            { "LDO_VRF18_0_VOLTAGE", mPromptUnit }, { "LDO_VRF18_1_VOLTAGE", mPromptUnit },
            { "LDO_VSIM1_VOLTAGE", mPromptUnit }, { "LDO_VSIM2_VOLTAGE", mPromptUnit },
            { "LDO_VSRAM_VOLTAGE", mPromptUnit }, { "LDO_VTCXO_0_VOLTAGE", mPromptUnit },
            { "LDO_VTCXO_1_VOLTAGE", mPromptUnit }, { "LDO_VUSB33_VOLTAGE", mPromptUnit }, };
    private String[][] mFileFor6570 = {
            {"BUCK_VPA_STATUS", mPromptSw},
            {"BUCK_VPROC_STATUS", mPromptSw},
            {"SEP", ""},
            {"LDO_VCN28_STATUS", mPromptSw},
            {"LDO_VTCXO_STATUS", mPromptSw},
            {"LDO_VA_STATUS", mPromptSw},
            {"LDO_VCAMA_STATUS", mPromptSw},
            {"LDO_VCN33_BT_STATUS", mPromptSw},
            {"LDO_VCN33_WIFI_STATUS", mPromptSw},
            {"LDO_VIO28_STATUS", mPromptSw},
            {"LDO_VSIM1_STATUS", mPromptSw},
            {"LDO_VSIM2_STATUS", mPromptSw},
            {"LDO_VUSB_STATUS", mPromptSw},
            {"LDO_VGP1_STATUS", mPromptSw},
            {"LDO_VGP2_STATUS", mPromptSw},
            {"LDO_VEMC_3V3_STATUS", mPromptSw},
            {"LDO_VCAMAF_STATUS", mPromptSw},
            {"LDO_VMC_STATUS", mPromptSw},
            {"LDO_VMCH_STATUS", mPromptSw},
            {"LDO_VIBR_STATUS", mPromptSw},
            {"LDO_VRTC_STATUS", mPromptSw},
            {"LDO_VM_STATUS", mPromptSw},
            {"LDO_VRF18_STATUS", mPromptSw},
            {"LDO_VIO18_STATUS", mPromptSw},
            {"LDO_VCAMD_STATUS", mPromptSw},
            {"LDO_VCAMIO_STATUS", mPromptSw},
            {"LDO_VGP3_STATUS", mPromptSw},
            {"LDO_VCN_1V8_STATUS", mPromptSw},
            {"SEP", ""},
            {"BUCK_VPA_VOLTAGE", mPromptUv},
            {"BUCK_VPROC_VOLTAGE", mPromptUv},
            {"SEP", ""},
            {"LDO_VCN28_VOLTAGE", mPromptUnit},
            {"LDO_VTCXO_VOLTAGE", mPromptUnit},
            {"LDO_VA_VOLTAGE", mPromptUnit},
            {"LDO_VCAMA_VOLTAGE", mPromptUnit},
            {"LDO_VCN33_BT_VOLTAGE", mPromptUnit},
            {"LDO_VCN33_WIFI_VOLTAGE", mPromptUnit},
            {"LDO_VIO28_VOLTAGE", mPromptUnit},
            {"LDO_VSIM1_VOLTAGE", mPromptUnit},
            {"LDO_VSIM2_VOLTAGE", mPromptUnit},
            {"LDO_VUSB_VOLTAGE", mPromptUnit},
            {"LDO_VGP1_VOLTAGE", mPromptUnit},
            {"LDO_VGP2_VOLTAGE", mPromptUnit},
            {"LDO_VEMC_3V3_VOLTAGE", mPromptUnit},
            {"LDO_VCAMAF_VOLTAGE", mPromptUnit},
            {"LDO_VMC_VOLTAGE", mPromptUnit},
            {"LDO_VMCH_VOLTAGE", mPromptUnit},
            {"LDO_VIBR_VOLTAGE", mPromptUnit},
            {"LDO_VRTC_VOLTAGE", mPromptUnit},
            {"LDO_VM_VOLTAGE", mPromptUnit},
            {"LDO_VRF18_VOLTAGE", mPromptUnit},
            {"LDO_VIO18_VOLTAGE", mPromptUnit},
            {"LDO_VCAMD_VOLTAGE", mPromptUnit},
            {"LDO_VCAMIO_VOLTAGE", mPromptUnit},
            {"LDO_VGP3_VOLTAGE", mPromptUnit},
            {"LDO_VCN_1V8_VOLTAGE", mPromptUnit},
        };
    private String[][] mFileFor80 = { { "BUCK_VPA_STATUS", mPromptSw },
            { "BUCK_VPROC_STATUS", mPromptSw }, { "SEP", "" }, { "LDO_VCN28_STATUS", mPromptSw },
            { "LDO_VTCXO_STATUS", mPromptSw }, { "LDO_VA_STATUS", mPromptSw },
            { "LDO_VCAMA_STATUS", mPromptSw }, { "LDO_VCN33_BT_STATUS", mPromptSw },
            { "LDO_VCN33_WIFI_STATUS", mPromptSw }, { "LDO_VIO28_STATUS", mPromptSw },
            { "LDO_VSIM1_STATUS", mPromptSw }, { "LDO_VSIM2_STATUS", mPromptSw },
            { "LDO_VUSB_STATUS", mPromptSw }, { "LDO_VGP1_STATUS", mPromptSw },
            { "LDO_VGP2_STATUS", mPromptSw }, { "LDO_VEMC_3V3_STATUS", mPromptSw },
            { "LDO_VCAMAF_STATUS", mPromptSw }, { "LDO_VMC_STATUS", mPromptSw },
            { "LDO_VMCH_STATUS", mPromptSw }, { "LDO_VIBR_STATUS", mPromptSw },
            { "LDO_VRTC_STATUS", mPromptSw }, { "LDO_VM_STATUS", mPromptSw },
            { "LDO_VRF18_STATUS", mPromptSw }, { "LDO_VIO18_STATUS", mPromptSw },
            { "LDO_VCAMD_STATUS", mPromptSw }, { "LDO_VCAMIO_STATUS", mPromptSw },
            { "LDO_VGP3_STATUS", mPromptSw }, { "LDO_VCN_1V8_STATUS", mPromptSw }, { "SEP", "" },
            { "BUCK_VPA_VOLTAGE", mPromptUv }, { "BUCK_VPROC_VOLTAGE", mPromptUv }, { "SEP", "" },
            { "LDO_VCN28_VOLTAGE", mPromptUnit }, { "LDO_VTCXO_VOLTAGE", mPromptUnit },
            { "LDO_VA_VOLTAGE", mPromptUnit }, { "LDO_VCAMA_VOLTAGE", mPromptUnit },
            { "LDO_VCN33_BT_VOLTAGE", mPromptUnit }, { "LDO_VCN33_WIFI_VOLTAGE", mPromptUnit },
            { "LDO_VIO28_VOLTAGE", mPromptUnit }, { "LDO_VSIM1_VOLTAGE", mPromptUnit },
            { "LDO_VSIM2_VOLTAGE", mPromptUnit }, { "LDO_VUSB_VOLTAGE", mPromptUnit },
            { "LDO_VGP1_VOLTAGE", mPromptUnit }, { "LDO_VGP2_VOLTAGE", mPromptUnit },
            { "LDO_VEMC_3V3_VOLTAGE", mPromptUnit }, { "LDO_VCAMAF_VOLTAGE", mPromptUnit },
            { "LDO_VMC_VOLTAGE", mPromptUnit }, { "LDO_VMCH_VOLTAGE", mPromptUnit },
            { "LDO_VIBR_VOLTAGE", mPromptUnit }, { "LDO_VRTC_VOLTAGE", mPromptUnit },
            { "LDO_VM_VOLTAGE", mPromptUnit }, { "LDO_VRF18_VOLTAGE", mPromptUnit },
            { "LDO_VIO18_VOLTAGE", mPromptUnit }, { "LDO_VCAMD_VOLTAGE", mPromptUnit },
            { "LDO_VCAMIO_VOLTAGE", mPromptUnit }, { "LDO_VGP3_VOLTAGE", mPromptUnit },
            { "LDO_VCN_1V8_VOLTAGE", mPromptUnit }, };
    private String[][] mFilesFor6799 = { { "buck_vcore_status", "0/1=off/on" },
            { "buck_vdram_status", "0/1=off/on" }, { "buck_vmodem_status", "0/1=off/on" },
            { "buck_vmd1_status", "0/1=off/on" }, { "buck_vs1_status", "0/1=off/on" },
            { "buck_vs2_status", "0/1=off/on" }, { "buck_vpa1_status", "0/1=off/on" },
            { "buck_vimvo_status", "0/1=off/on" }, { "SEP", "" },
            { "ldo_vio28_status", "0/1=off/on" }, { "ldo_vio18_status", "0/1=off/on" },
            { "ldo_vufs18_status", "0/1=off/on" }, { "ldo_va10_status", "0/1=off/on" },
            { "ldo_va12_status", "0/1=off/on" }, { "ldo_va18_status", "0/1=off/on" },
            { "ldo_vusb33_status", "0/1=off/on" }, { "ldo_vemc_status", "0/1=off/on" },
            { "ldo_vxo22_status", "0/1=off/on" }, { "ldo_vefuse_status", "0/1=off/on" },
            { "ldo_vsim1_status", "0/1=off/on" }, { "ldo_vsim2_status", "0/1=off/on" },
            { "ldo_vcamaf_status", "0/1=off/on" }, { "ldo_vtouch_status", "0/1=off/on" },
            { "ldo_vcamd1_status", "0/1=off/on" }, { "ldo_vcamd2_status", "0/1=off/on" },
            { "ldo_vcamio_status", "0/1=off/on" }, { "ldo_vmipi_status", "0/1=off/on" },
            { "ldo_vgp3_status", "0/1=off/on" }, { "ldo_vcn33_bt_status", "0/1=off/on" },
            { "ldo_vcn33_wifi_status", "0/1=off/on" }, { "ldo_vcn18_bt_status", "0/1=off/on" },
            { "ldo_vcn18_wifi_status", "0/1=off/on" }, { "ldo_vcn28_status", "0/1=off/on" },
            { "ldo_vibr_status", "0/1=off/on" }, { "ldo_vbif28_status", "0/1=off/on" },
            { "ldo_vfe28_status", "0/1=off/on" }, { "ldo_vmch_status", "0/1=off/on" },
            { "ldo_vmc_status", "0/1=off/on" }, { "ldo_vrf18_1_status", "0/1=off/on" },
            { "ldo_vrf18_2_status", "0/1=off/on" }, { "ldo_vrf12_status", "0/1=off/on" },
            { "ldo_vcama1_status", "0/1=off/on" }, { "ldo_vcama2_status", "0/1=off/on" },
            { "ldo_vsram_dvfs1_status", "0/1=off/on" }, { "ldo_vsram_dvfs2_status", "0/1=off/on" },
            { "ldo_vsram_vgpu_status", "0/1=off/on" }, { "ldo_vsram_vcore_status", "0/1=off/on" },
            { "ldo_vsram_vmd_status", "0/1=off/on" }, { "SEP", "" },
            { "buck_vcore_voltage", "mv" }, { "buck_vdram_voltage", "mv" },
            { "buck_vmodem_voltage", "mv" }, { "buck_vmd1_voltage", "mv" },
            { "buck_vs1_voltage", "mv" }, { "buck_vs2_voltage", "mv" },
            { "buck_vpa1_voltage", "mv" }, { "buck_vimvo_voltage", "mv" }, { "SEP", "" },
            { "ldo_vio28_voltage", "mv" }, { "ldo_vio18_voltage", "mv" },
            { "ldo_vufs18_voltage", "mv" }, { "ldo_va10_voltage", "mv" },
            { "ldo_va12_voltage", "mv" }, { "ldo_va18_voltage", "mv" },
            { "ldo_vusb33_voltage", "mv" }, { "ldo_vemc_voltage", "mv" },
            { "ldo_vxo22_voltage", "mv" }, { "ldo_vefuse_voltage", "mv" },
            { "ldo_vsim1_voltage", "mv" }, { "ldo_vsim2_voltage", "mv" },
            { "ldo_vcamaf_voltage", "mv" }, { "ldo_vtouch_voltage", "mv" },
            { "ldo_vcamd1_voltage", "mv" }, { "ldo_vcamd2_voltage", "mv" },
            { "ldo_vcamio_voltage", "mv" }, { "ldo_vmipi_voltage", "mv" },
            { "ldo_vgp3_voltage", "mv" }, { "ldo_vcn33_bt_voltage", "mv" },
            { "ldo_vcn33_wifi_voltage", "mv" }, { "ldo_vcn18_bt_voltage", "mv" },
            { "ldo_vcn18_wifi_voltage", "mv" }, { "ldo_vcn28_voltage", "mv" },
            { "ldo_vibr_voltage", "mv" }, { "ldo_vbif28_voltage", "mv" },
            { "ldo_vfe28_voltage", "mv" }, { "ldo_vmch_voltage", "mv" },
            { "ldo_vmc_voltage", "mv" }, { "ldo_vrf18_1_voltage", "mv" },
            { "ldo_vrf18_2_voltage", "mv" }, { "ldo_vrf12_voltage", "mv" },
            { "ldo_vcama1_voltage", "mv" }, { "ldo_vcama2_voltage", "mv" },
            { "ldo_vsram_dvfs1_voltage", "mv" }, { "ldo_vsram_dvfs2_voltage", "mv" },
            { "ldo_vsram_vgpu_voltage", "mv" }, { "ldo_vsram_vcore_voltage", "mv" },
            { "ldo_vsram_vmd_voltage", "mv" }, };
    /**
     * PMU stucture for KiboPlus
     */
    private String[][] mFilesFor6757CH = { { "buck_vdram2_status", "0/1=off/on" },
            { "buck_vs1_status", "0/1=off/on" }, { "buck_vdram1_status", "0/1=off/on" },
            { "buck_vmodem_status", "0/1=off/on" }, { "buck_vcore_status", "0/1=off/on" },
            { "buck_vs2_status", "0/1=off/on" }, { "buck_vpa_status", "0/1=off/on" },
            { "buck_vproc12_status", "0/1=off/on" }, { "buck_vgpu_status", "0/1=off/on" },
            { "buck_vproc11_status", "0/1=off/on" }, { "SEP", "" },
            { "ldo_vcamd1_status", "0/1=off/on" }, { "ldo_vsim1_status", "0/1=off/on" },
            { "ldo_vgp_status", "0/1=off/on" }, { "ldo_vusb33_status", "0/1=off/on" },
            { "ldo_vrf12_status", "0/1=off/on" }, { "ldo_vcamio_status", "0/1=off/on" },
            { "ldo_vcn18_status", "0/1=off/on" }, { "ldo_vfe28_status", "0/1=off/on" },
            { "ldo_vrf18_2_status", "0/1=off/on" }, { "ldo_va18_status", "0/1=off/on" },
            { "ldo_vcn28_status", "0/1=off/on" }, { "ldo_vmipi_status", "0/1=off/on" },
            { "ldo_vsram_gpu_status", "0/1=off/on" }, { "ldo_vsram_core_status", "0/1=off/on" },
            { "ldo_vsram_proc_status", "0/1=off/on" }, { "ldo_vxo22_status", "0/1=off/on" },
            { "ldo_vrf18_1_status", "0/1=off/on" }, { "ldo_vcamd2_status", "0/1=off/on" },
            { "ldo_vmch_status", "0/1=off/on" }, { "ldo_vcama1_status", "0/1=off/on" },
            { "ldo_vtcxo24_status", "0/1=off/on" }, { "ldo_vio28_status", "0/1=off/on" },
            { "ldo_vemc_status", "0/1=off/on" }, { "ldo_va12_status", "0/1=off/on" },
            { "ldo_va10_status", "0/1=off/on" }, { "ldo_vgp2_status", "0/1=off/on" },
            { "ldo_vio18_status", "0/1=off/on" }, { "ldo_vcn33_bt_status", "0/1=off/on" },
            { "ldo_vcn33_wifi_status", "0/1=off/on" }, { "ldo_vsram_md_status", "0/1=off/on" },
            { "ldo_vbif28_status", "0/1=off/on" }, { "ldo_vufs18_status", "0/1=off/on" },
            { "ldo_vcama2_status", "0/1=off/on" }, { "ldo_vmc_status", "0/1=off/on" },
            { "ldo_vldo28_status", "0/1=off/on" }, { "ldo_vxo18_status", "0/1=off/on" },
            { "ldo_vsim2_status", "0/1=off/on" }, { "SEP", "" }, { "buck_vdram2_voltage", "mv" },
            { "buck_vs1_voltage", "mv" }, { "buck_vdram1_voltage", "mv" },
            { "buck_vmodem_voltage", "mv" }, { "buck_vcore_voltage", "mv" },
            { "buck_vs2_voltage", "mv" }, { "buck_vpa_voltage", "mv" },
            { "buck_vproc12_voltage", "mv" }, { "buck_vgpu_voltage", "mv" },
            { "buck_vproc11_voltage", "mv" }, { "SEP", "" }, { "ldo_vcamd1_voltage", "mv" },
            { "ldo_vsim1_voltage", "mv" }, { "ldo_vgp_voltage", "mv" },
            { "ldo_vusb33_voltage", "mv" }, { "ldo_vrf12_voltage", "mv" },
            { "ldo_vcamio_voltage", "mv" }, { "ldo_vcn18_voltage", "mv" },
            { "ldo_vfe28_voltage", "mv" }, { "ldo_vrf18_2_voltage", "mv" },
            { "ldo_va18_voltage", "mv" }, { "ldo_vcn28_voltage", "mv" },
            { "ldo_vmipi_voltage", "mv" }, { "ldo_vsram_gpu_voltage", "mv" },
            { "ldo_vsram_core_voltage", "mv" }, { "ldo_vsram_proc_voltage", "mv" },
            { "ldo_vxo22_voltage", "mv" }, { "ldo_vrf18_1_voltage", "mv" },
            { "ldo_vcamd2_voltage", "mv" }, { "ldo_vmch_voltage", "mv" },
            { "ldo_vcama1_voltage", "mv" }, { "ldo_vtcxo24_voltage", "mv" },
            { "ldo_vio28_voltage", "mv" }, { "ldo_vemc_voltage", "mv" },
            { "ldo_va12_voltage", "mv" }, { "ldo_va10_voltage", "mv" },
            { "ldo_vgp2_voltage", "mv" }, { "ldo_vio18_voltage", "mv" },
            { "ldo_vcn33_bt_voltage", "mv" }, { "ldo_vcn33_wifi_voltage", "mv" },
            { "ldo_vsram_md_voltage", "mv" }, { "ldo_vbif28_voltage", "mv" },
            { "ldo_vufs18_voltage", "mv" }, { "ldo_vcama2_voltage", "mv" },
            { "ldo_vmc_voltage", "mv" }, { "ldo_vldo28_voltage", "mv" },
            { "ldo_vxo18_voltage", "mv" }, { "ldo_vsim2_voltage", "mv" }, };

    // private int mUpdateInterval = 1500; // 1.5 sec
    private Handler mUpdateHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_UPDATE:
                Bundle b = msg.getData();
                mInfo.setText(b.getString("INFO"));
                break;
            default:
                break;
            }
        }
    };

    private void setLayout() {
        mInfo = (TextView) findViewById(R.id.pmu_info_text);
        mBtnGetRegister = (Button) findViewById(R.id.pmu_get_btn);
        mBtnSetRegister = (Button) findViewById(R.id.pmu_set_btn);
        mEditAddr = (EditText) findViewById(R.id.pmu_addr_edit);
        mEditVal = (EditText) findViewById(R.id.pmu_val_edit);
        mBtnGetRegister.setOnClickListener(this);
        mBtnSetRegister.setOnClickListener(this);

        mBtnGetReg6333 = (Button) findViewById(R.id.pmu_get_mt6333_btn);
        mBtnSetReg6333 = (Button) findViewById(R.id.pmu_set_mt6333_btn);
        mBtnGetReg6333.setOnClickListener(this);
        mBtnSetReg6333.setOnClickListener(this);
        mEditAddr6333 = (EditText) findViewById(R.id.pmu_addr_mt6333_edit);
        mEditVal6333 = (EditText) findViewById(R.id.pmu_val_mt6333_edit);
        if (new File(FS_MT6333_ACCESS).exists()) {
            mFSAccessFile = FS_MT6333_ACCESS;
        } else if (new File(FS_MT6311_ACCESS).exists()) {
            mFSAccessFile = FS_MT6311_ACCESS;
        } else {
            ((LinearLayout) findViewById(R.id.mt6333_layout_controler)).setVisibility(View.GONE);
        }

        if(isSubRegister()){
            mSubPmicSpinner = (Spinner) findViewById(R.id.pmu_sub_spinner);
            mBtnGetRegSub = (Button) findViewById(R.id.pmu_get_sub_btn);
            mBtnSetRegSub = (Button) findViewById(R.id.pmu_set_sub_btn);
            mBtnGetRegSub.setOnClickListener(this);
            mBtnSetRegSub.setOnClickListener(this);
            mEditAddrSub = (EditText) findViewById(R.id.pmu_addr_sub_edit);
            mEditValSub = (EditText) findViewById(R.id.pmu_val_sub_edit);
            ((LinearLayout) findViewById(R.id.sub_layout_controler)).setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (!FeatureSupport.isSupportedEmSrv()) {
            Toast.makeText(this, R.string.notice_wo_emsvr, Toast.LENGTH_LONG).show();
            finish();
            return;
        }
        TabHost tabHost = getTabHost();

        LayoutInflater.from(this).inflate(R.layout.power_pmu6575_tabs, tabHost.getTabContentView(),
                true);

        // tab1
        tabHost.addTab(tabHost.newTabSpec(this.getString(R.string.pmu_info_text))
                .setIndicator(this.getString(R.string.pmu_info_text))
                .setContent(R.id.LinerLayout_pmu_info_text));

        // tab2
        tabHost.addTab(tabHost.newTabSpec(this.getString(R.string.pmu_reg))
                .setIndicator(this.getString(R.string.pmu_reg))
                .setContent(R.id.LinerLayout_pmu_reg));

        setLayout();
        tabHost.setOnTabChangedListener(new OnTabChangeListener() {
            public void onTabChanged(String tabId) {
                String pmuInfoText = PMU6575.this.getString(R.string.pmu_info_text);
                String pmuReg = PMU6575.this.getString(R.string.pmu_reg);
                if (tabId.equals(pmuInfoText)) {
                    onTabInfo();
                } else if (tabId.equals(pmuReg)) {
                    onTabReg();
                }
            }
        });
        // init
    }

    private void onTabReg() {
        mWhichTab = TAB_REG;
    }

    private void onTabInfo() {
        mWhichTab = TAB_INFO;
    }

    private boolean isSubRegister() {
        return new File(FS_SUB_REG_ADDRS[0]).exists();
    }

    private void handleGetClick(String addr, EditText toSetTxt, String regFile, String dataFile) {
        if (checkAddr(addr.trim())) {
            if(dataFile == null) {
                dataFile  = regFile;
            }
            String[] cmds = new String[] {
                    "echo " + addr + " > " + regFile,
                    "cat " + dataFile};
            handleGetCmd(cmds, toSetTxt);
        } else {
            Toast.makeText(this, "Please check address.", Toast.LENGTH_LONG).show();
        }
    }

    private void handleSetClick(String addr, String val, String regFile, String dataFile) {
        if (checkAddr(addr.trim()) && checkVal(val.trim())) {
            String[] cmds = null;
            if (dataFile == null) {
                cmds = new String[] {"echo " + addr + " " + val + " > " + regFile};
            } else {
                cmds = new String[] {
                        "echo " + addr + " > " + regFile,
                        "echo " + val + " > " + dataFile};
            }
            handleSetCmd(cmds);
        } else {
            Toast.makeText(this, "Please check address or value.", Toast.LENGTH_LONG).show();
        }
    }

    private void handleGetCmd(String[] cmds, EditText toSetTxt) {
        String out = null;
        for(String s: cmds) {
            out = getInfo(s);
            Elog.i(TAG, s + ",out :" + out);
        }
        try {
            if(out!=null && out.startsWith("0x")) {
                toSetTxt.setText(out.substring(2));
            } else {
                String text = Integer.toHexString(Integer.parseInt(out));
                toSetTxt.setText(text);
            }
        } catch (NumberFormatException e) {
            e.printStackTrace();
            Toast.makeText(this, "Please check return value :" + out, Toast.LENGTH_LONG).show();
        }
    }

    private void handleSetCmd(String[] cmds) {
        String out = null;
        for(String s: cmds) {
            out = getInfo(s);
            Elog.i(TAG, s + ",out :" + out);
        }
        if (null != out && out.length() != 0) {
            Toast.makeText(this, out, Toast.LENGTH_LONG).show();
        }
    }

    @Override
    public void onClick(View arg0) {
        switch (arg0.getId()) {
            case R.id.pmu_get_btn:
                String addr = mEditAddr.getText().toString();
                handleGetClick(addr, mEditVal, INFO_PATH + "pmic_access", null);
                break;
            case R.id.pmu_set_btn:
                addr = mEditAddr.getText().toString();
                String val = mEditVal.getText().toString();
                handleSetClick(addr, val, INFO_PATH + "pmic_access", null);
                break;
            case R.id.pmu_get_mt6333_btn:
                addr = mEditAddr6333.getText().toString();
                handleGetClick(addr, mEditVal6333, mFSAccessFile, null);
                break;
            case R.id.pmu_set_mt6333_btn:
                addr = mEditAddr6333.getText().toString();
                val = mEditVal6333.getText().toString();
                handleSetClick(addr, val, mFSAccessFile, null);
                break;
            case R.id.pmu_get_sub_btn:
                int pos = mSubPmicSpinner.getSelectedItemPosition();
                if(pos >= 0 && pos <4) {
                    addr = mEditAddrSub.getText().toString();
                    addr = "0x" + addr;
                    handleGetClick(addr, mEditValSub, FS_SUB_REG_ADDRS[pos],
                            FS_SUB_REG_DATAS[pos]);
                } else {
                    Elog.w(TAG, "SUB Register[GET] get spinner position error");
                }
                break;
            case R.id.pmu_set_sub_btn:
                pos = mSubPmicSpinner.getSelectedItemPosition();
                if(pos >= 0 && pos <4) {
                    addr = mEditAddrSub.getText().toString();
                    addr = "0x" + addr;
                    val = mEditValSub.getText().toString();
                    if (1 == val.length()) {
                        val = "0x0" + val;
                    } else {
                        val = "0x" + val;
                    }
                    handleSetClick(addr, val, FS_SUB_REG_ADDRS[pos], FS_SUB_REG_DATAS[pos]);
                } else {
                    Elog.w(TAG, "SUB Register[SET] get spinner position error");
                }
                break;
            default:
                break;
        }
    }

    private boolean checkAddr(String s) {
        if (s == null || s.length() < 1) {
            return false;
        }

        String temp = s;
        if(temp.startsWith("0x")) {
            temp = temp.substring(2);
        }
        try {
            Integer.parseInt(temp, RADIX_HEX);
        } catch (NumberFormatException e) {
            return false;
        }
        return true;
    }

    private boolean checkVal(String s) {
        if (s == null || s.length() < 1) {
            if (s != null) {
                Elog.w(TAG, "s.length() is wrong!" + s.length());
            }
            return false;
        }

        String temp = s;
        if(temp.startsWith("0x")) {
            temp = temp.substring(2);
        }

        if (temp.length() > MAX_LENGTH_89) {
            Elog.w(TAG, "s.length() is too long: " + temp.length());
            return false;
        }

        try {
            Integer.parseInt(temp, RADIX_HEX);
        } catch (NumberFormatException e) {
            return false;
        }
        return true;
    }

    private String getInfo(String cmd) {
        String result = null;
        try {
            String[] cmdx = { "/system/bin/sh", "-c", cmd }; // file must
            // exist// or
            // wait()
            // return2
            int ret = 0;
            ret = ShellExe.execCommand(cmdx);
            if (0 == ret) {
                result = ShellExe.getOutput();
            } else {
                // result = "ERROR";
                result = ShellExe.getOutput();
            }

        } catch (IOException e) {
            Elog.i(TAG, e.toString());
            result = "ERROR.JE";
        }
        return result;
    }

    private void initFileArray() {

        if (mFileArray == null) {
            Elog.v(TAG, "-----------fileArray is null------------");
            if ((new File(INFO_PATH + INFO_FILES)).exists()) {
                String result = getInfo("cat " + INFO_PATH + INFO_FILES);
                if(result.contains("ERROR")) {
                    return;
                }
                String[] lines = result.split("\n");
                int len = lines.length;
                mFileArray = new String[len][2];
                for (int i = 0; i < len; ++i) {
                    mFileArray[i] = lines[i].split(",");
                }
            } else {
                if (ChipSupport.isCurrentChipEquals(ChipSupport.MTK_6735_SUPPORT)) {
                    mFileArray = mFileFor35;
                } else if (ChipSupport.isCurrentChipEquals(ChipSupport.MTK_6580_SUPPORT)) {
                    mFileArray = mFileFor80;
                } else if (ChipSupport.isCurrentChipEquals(ChipSupport.MTK_6570_SUPPORT)) {
                        mFileArray = mFileFor6570;
                } else if (ChipSupport.isCurrentChipEquals(ChipSupport.MTK_6755_SUPPORT)
                        || ChipSupport.isCurrentChipEquals(ChipSupport.MTK_6797_SUPPORT)
                        || ChipSupport.isCurrentChipEquals(ChipSupport.MTK_6757_SUPPORT)) {
                    mFileArray = mFileFor6755;
                } else if (ChipSupport.isCurrentChipEquals(ChipSupport.MTK_6750_SUPPORT)) {
                    mFileArray = mFilesFor6750;
                } else if (ChipSupport.isCurrentChipEquals(ChipSupport.MTK_6799_SUPPORT)) {
                    mFileArray = mFilesFor6799;
                } else if (ChipSupport.isCurrentChipEquals(ChipSupport.MTK_6757CH_SUPPORT)
                        || ChipSupport.isCurrentChipEquals(ChipSupport.MTK_6759_SUPPORT)) {
                    mFileArray = mFilesFor6757CH;
                } else {
                    mFileArray = mFiles;
                }
            }
        }
    }

    class RunThread extends Thread {

        public void run() {
            while (mRun) {
                StringBuilder text = new StringBuilder("");
                String cmd = "";
                if(mFileArray == null) {
                    return;
                }
                for (int i = 0; i < mFileArray.length; i++) {
                    if ("SEP".equalsIgnoreCase(mFileArray[i][0])) {
                        text.append("- - - - - - - - - -\n");
                        continue;
                    }
                    cmd = "cat " + INFO_PATH + mFileArray[i][0];

                    if ("mA".equalsIgnoreCase(mFileArray[i][1])) {
                        double f = 0.0f;
                        try {
                            f = Float.valueOf(getInfo(cmd)) / MAGIC_TEN;
                        } catch (NumberFormatException e) {
                            Elog.e(TAG, "read file error " + mFileArray[i][0]);
                        }
                        text.append(String.format("%1$-28s:[%2$-6s]%3$s\n", mFileArray[i][0], f,
                                mFileArray[i][1]));
                    } else {
                        text.append(String.format("%1$-28s:[%2$-6s]%3$s\n", mFileArray[i][0],
                                getInfo(cmd), mFileArray[i][1]));
                    }
                }

                Bundle b = new Bundle();
                b.putString("INFO", text.toString());

                Message msg = new Message();
                msg.what = EVENT_UPDATE;
                msg.setData(b);
                mUpdateHandler.sendMessage(msg);
                try {
                    sleep(UPDATE_INTERVAL);
                    while (mWhichTab == TAB_REG && mRun) {
                        sleep(WAIT_INTERVAL);
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        mRun = false;
    }

    @Override
    protected void onResume() {
        super.onResume();
        initFileArray();
        mRun = true;
        new RunThread().start();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }
}
