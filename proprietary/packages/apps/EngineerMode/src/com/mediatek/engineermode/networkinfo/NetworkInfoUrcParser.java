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

package com.mediatek.engineermode.networkinfo;

import android.content.Context;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

public class NetworkInfoUrcParser {
    private static final String TAG = "NetworkInfo/UrcParser";
    private static final int DATA_OFFSET_2 = 2;
    private static final int DATA_OFFSET_4 = 4;
    private static final int DATA_OFFSET_6 = 6;
    private static final int DATA_OFFSET_8 = 8;
    private static final int DATA_FORMAT = 16;
    private static final int MAX_DATA_PER_LINE = 7;

    private static final int TYPE_UINT8 = 0;
    private static final int TYPE_UINT16 = 1;
    private static final int TYPE_UINT32 = 2;
    private static final int TYPE_INT8 = 3;
    private static final int TYPE_INT16 = 4;
    private static final int TYPE_INT32 = 5;
    private static final int TYPE_LONG = 6;
    private static final int TYPE_FLOAT = 7;
    private static final int TYPE_ALIGNMENT = 8;
    private static final int TYPE_STRING = 9;

    private static final Boolean ALIGN_MENT_ENABLE = true;
    private static final Boolean GPRS_MODE_ENABLE = true;
    private static final Boolean AMR_SUPPORT_ENABLE = true;
    private static final Boolean FWPNC_LAI_INFO_ENABLE = false;
    private static final Boolean UMTS_R8 = true;
    private static final Boolean WISDOM_EM = true;
    private static final Boolean ADVANCED_EM = true;
    private static final Boolean IS_MOLY = Content.IS_MOLY;

    private String mRawString;
    private StringBuilder mResult;
    private int mOffset;
    private String[] mRawValues;
    private String[] mLabels;
    private int mColumn;
    private Context mContext;
    private int mType;

    /**
     * @param context
     *            the context
     */
    public NetworkInfoUrcParser(Context context) {
        mContext = context;
    }

    /**
     * @param type
     *            the integer of the network item to view
     * @return the value of the network item to display
     */
    public String parseInfo(int type, String info, int simType) {
        Elog.v(TAG, "NetworkInfo ------ Type is: " + type);
        Elog.v(TAG, "NetworkInfo ------ Data is:\n");
        Elog.v(TAG, info + "\n");
        Elog.v(TAG, "NetworkInfo ---------------------------");
        mRawString = info;
        mResult = new StringBuilder();
        mOffset = 0;

        switch (type) {
        case Content.CELL_INDEX:
            return getCellSelInfo();
        case Content.CHANNEL_INDEX:
            return getChDscrInfo();
        case Content.CTRL_INDEX:
            return getCtrlchanInfo();
        case Content.RACH_INDEX:
            return getRACHCtrlInfo();
        case Content.LAI_INDEX:
            return getLAIInfo();
        case Content.RADIO_INDEX:
            return getRadioLinkInfo();
        case Content.MEAS_INDEX:
            return getMeasRepInfo();
        case Content.CA_INDEX:
            return getCaListInfo();
        case Content.CONTROL_INDEX:
            return getControlMsgInfo();
        case Content.SI2Q_INDEX:
            return getSI2QInfo();
        case Content.MI_INDEX:
            return getMIInfo();
        case Content.BLK_INDEX:
            return getBLKInfo();
        case Content.TBF_INDEX:
            return getTBFInfo();
        case Content.GPRS_INDEX:
            return getGPRSGenInfo();
        case Content.URR_3G_GENERAL_INDEX:
            return get3GGeneralInfo();
        case Content.GMM_INFO_INDEX:
            return getGmmEmInfo();
        default:
            break;
        }

        if (type == Content.SM_INFO_INDEX) {
            return getSmEmInfo();
        } else if (type == Content.SLCE_VOICE_INDEX) {
            return getSlceVoiceInfo();
        } else if (type == Content.SECURITY_CONFIGURATION_INDEX) {
            return getSecurityConfigInfo();
        } else if (type == Content.MM_INFO_INDEX) {
            return get3GMmEmInfo();
        } else if (type == Content.TCM_MMI_INDEX) {
            return get3GTcmMmiEmInfo();
        } else if (type == Content.CSCE_MULTIPLMN_INDEX) {
            return get3GCsceEmInfoMultiPlmn();
        } else if (type == Content.PERIOD_IC_BLER_REPORT_INDEX) {
            return get3GMemeEmPeriodicBlerReportInd();
        } else if (type == Content.URR_UMTS_SRNC_INDEX) {
            return get3GUrrUmtsSrncId();
        } else if (type == Content.HSERV_CELL_INDEX) {
            return get3GMemeEmInfoHServCellInd();
        } else if (type == Content.CSCE_NEIGH_CELL_STATUS_INDEX) {
            return getxGCsceEMNeighCellSStatusIndStructSize(
                    ModemCategory.getModemType() == NetworkInfo.MODEM_TD);
        } else if (type == Content.CSCE_SERV_CELL_STATUS_INDEX) {
            return get3GCsceEMServCellSStatusInd(
                    ModemCategory.getModemType() == NetworkInfo.MODEM_TD);
        }

        if (ModemCategory.getModemType() == NetworkInfo.MODEM_FDD) {
            if (type == Content.UMTS_CELL_STATUS_INDEX) {
                return get3GMemeEmInfoUmtsCellStatus();
            } else if (type == Content.PSDATA_RATE_STATUS_INDEX) {
                return get3GSlceEmPsDataRateStatusInd();
            }
        } else if (ModemCategory.getModemType() == NetworkInfo.MODEM_TD) {
            if (type == Content.HANDOVER_SEQUENCE_INDEX) {
                return get3GHandoverSequenceIndStuct();
            } else if (type == Content.UL_ADM_POOL_STATUS_INDEX) {
                return get3GUl2EmAdmPoolStatusIndStruct();
            } else if (type == Content.UL_PSDATA_RATE_STATUS_INDEX) {
                return get3GUl2EmPsDataRateStatusIndStruct();
            } else if (type == Content.UL_HSDSCH_RECONFIG_STATUS_INDEX) {
                return get3Gul2EmHsdschReconfigStatusIndStruct();
            } else if (type == Content.UL_URLC_EVENT_STATUS_INDEX) {
                return get3GUl2EmUrlcEventStatusIndStruct();
            } else if (type == Content.UL_PERIOD_IC_BLER_REPORT_INDEX) {
                return get3GUl2EmPeriodicBlerReportInd();
            }
        }

        return null;
    }

    /**
     * @return the cellSel information (rr_em_cell_select_para_info_struct)
     */
    public String getCellSelInfo() {
        parseElement(TYPE_UINT8, "crh: ");
        parseElement(TYPE_UINT8, "ms_txpwr: ");
        parseElement(TYPE_UINT8, "rxlev_access_min: ");
        return mResult.toString();
    }

    /**
     * @return the ChDscr information (rr_em_channel_descr_info_struct)
     */
    public String getChDscrInfo() {
        parseElement(TYPE_UINT8,  "channel_type: ");
        parseElement(TYPE_UINT8,  "tn: ");
        parseElement(TYPE_UINT8,  "tsc: ");
        parseElement(TYPE_UINT8,  "hopping_flag: ");
        parseElement(TYPE_UINT8,  "maio: ");
        parseElement(TYPE_UINT8,  "hsn: ");
        parseElement(TYPE_UINT8,  "num_of_carriers: ");
        parseElement(TYPE_UINT16, "arfcn:", 64);
        parseElement(TYPE_INT8,   "is_BCCH_arfcn_valid: ");
        parseElement(TYPE_UINT16, "BCCH_arfcn: ");
        parseElement(TYPE_UINT8,  "cipher_algo: ");
        parseElement(TYPE_UINT8,  "imeisv_digit: ", 16);
        parseElement(TYPE_UINT8,  "channel_mode: ");
        if (AMR_SUPPORT_ENABLE) {
            parseElement(TYPE_INT8,  "amr_valid: ");
            parseElement(TYPE_UINT8, "mr_ver: ");
            parseElement(TYPE_INT8,  "nscb: ");
            parseElement(TYPE_INT8,  "icmi: ");
            parseElement(TYPE_UINT8, "start_codec_mode: ");
            parseElement(TYPE_UINT8, "acs: ");
            parseElement(TYPE_UINT8, "threshold:", 3);
            parseElement(TYPE_UINT8, "hysteresis:", 3);
        }
        return mResult.toString();
    }

    /**
     * @return the Control channel information (rr_em_ctrl_channel_descr_info_struct)
     */
    public String getCtrlchanInfo() {
        parseElement(TYPE_UINT8, "mscr: ");
        parseElement(TYPE_UINT8, "att: ");
        parseElement(TYPE_UINT8, "bs_ag_blks_res: ");
        parseElement(TYPE_UINT8, "ccch_conf: ");
        parseElement(TYPE_UINT8, "cbq2: ");
        parseElement(TYPE_UINT8, "bs_pa_mfrms: ");
        parseElement(TYPE_UINT8, "t3212: ");
        return mResult.toString();
    }

    /**
     * @return the RACHCtrl information (rr_em_rach_ctrl_para_info_struct)
     */
    public String getRACHCtrlInfo() {
        parseElement(TYPE_UINT8, "max_retrans: ");
        parseElement(TYPE_UINT8, "tx_integer: ");
        parseElement(TYPE_UINT8, "cba: ");
        parseElement(TYPE_UINT8, "re: ");
        parseElement(TYPE_UINT8, "acc_class:", 2);
        parseElement(TYPE_INT8,  "CB_supported: ");
        return mResult.toString();
    }

    /**
     * @return the LAI information
     */
    public String getLAIInfo() {
        parseElement(TYPE_UINT8, "mcc:", 3);
        parseElement(TYPE_UINT8, "mnc:", 3);
        parseElement(TYPE_UINT8, "lac:", 2);
        parseElement(TYPE_UINT16, "cell_id: ");
        parseElement(TYPE_UINT8, "nc_info_index: ");
        parseElement(TYPE_UINT8, "rac: ");
        parseElement(TYPE_UINT8, "nmo: ");
        parseElement(TYPE_UINT8, "supported_Band: ");
        return mResult.toString();
    }

    /**
     * @return the Radio Link information (rr_em_radio_link_counter_info_struct)
     */
    public String getRadioLinkInfo() {
        parseElement(TYPE_UINT16, "max_value: ");
        parseElement(TYPE_INT16, "current_value: ");
        parseElement(TYPE_UINT8, "dtx_ind: ");
        parseElement(TYPE_UINT8, "dtx_used: ");
        parseElement(TYPE_INT8, "is_dsf: ");
        return mResult.toString();
    }

    /**
     * @return the MeasRep information (rr_em_measurement_report_info_struct)
     */
    public String getMeasRepInfo() {
        parseElement(TYPE_UINT8, "rr_state: ");
        if (IS_MOLY) {
            parseElement(TYPE_UINT8, "meas_mode: ");
        }
        parseElement(TYPE_UINT16, "serving_arfcn: ");
        parseElement(TYPE_UINT8, "serving_bsic: ");
        parseElement(TYPE_UINT8, "serving_current_band: ");
        parseElement(TYPE_UINT8, "serv_gprs_supported: ");
        parseElement(TYPE_INT16, "serv_rla_in_quarter_dbm: ");
        if (IS_MOLY) {
            parseElement(TYPE_UINT8, "serv_rla_reported_value: ");
        }
        parseElement(TYPE_INT8, "is_serv_BCCH_rla_valid: ");
        parseElement(TYPE_INT16, "serv_BCCH_rla_in_dedi_state: ");
        parseElement(TYPE_UINT8, "quality: ");
        parseElement(TYPE_INT8, "gprs_pbcch_present: ");
        parseElement(TYPE_INT8, "gprs_c31_c32_enable: ");
        if (!IS_MOLY) {
            parseElement(TYPE_INT16, "c31:", 32);
        }
        parseElement(TYPE_INT16, "c1_serv_cell: ");
        parseElement(TYPE_INT16, "c2_serv_cell: ");
        parseElement(TYPE_INT16, "c31_serv_cell: ");
        parseElement(TYPE_UINT8, "num_of_carriers: ");
        parseElement(TYPE_UINT16, "nc_arfcn:", 32);
        parseElement(TYPE_INT16, "rla_in_quarter_dbm:", 32);
        if (IS_MOLY) {
            parseElement(TYPE_UINT8, "rla_in_reported_value: ", 32);
        }
        parseElement(TYPE_UINT8, "nc_info_status:", 32);
        parseElement(TYPE_UINT8, "nc_bsic:", 32);
        parseElement(TYPE_INT32, "frame_offset:", 32);
        parseElement(TYPE_INT32, "ebit_offset:", 32);
        parseElement(TYPE_INT16, "c1:", 32);
        parseElement(TYPE_INT16, "c2:", 32);
        if (IS_MOLY) {
            parseElement(TYPE_INT16, "c31:", 32);
        }
        parseElement(TYPE_UINT8, "multiband_report: ");
        parseElement(TYPE_UINT8, "timing_advance: ");
        parseElement(TYPE_INT16, "tx_power_level: ");
        parseElement(TYPE_INT16, "serv_rla_full_value_in_quater_dbm: ");
        parseElement(TYPE_UINT8, "nco: ");
        parseElement(TYPE_UINT8, "rxqual_sub: ");
        parseElement(TYPE_UINT8, "rxqual_full: ");
        parseElement(TYPE_INT16, "using_tx_power_in_dbm: ");
        parseElement(TYPE_INT8, "amr_info_valid: ");
        parseElement(TYPE_UINT8, "cmr_cmc_cmiu_cmid: ");
        parseElement(TYPE_UINT8, "c_i: ");
        parseElement(TYPE_UINT16, "icm: ");
        parseElement(TYPE_UINT16, "acs: ");
        parseElement(TYPE_INT8, "dl_dtx_used: ");
        if (FWPNC_LAI_INFO_ENABLE) {
            parseElement(TYPE_UINT8, "num_of_nc_lai: ");
            mResult.append("nc_lai:\n");
            for (int i = 0; i < 6; i++) {
                parseElement(TYPE_UINT8, "nc_lai[" + i + "]:\n" + "mcc:", 3);
                parseElement(TYPE_UINT8, "mnc:", 3);
                parseElement(TYPE_UINT8, "lac:", 2);
                parseElement(TYPE_UINT16, "cell_id: ");
                parseElement(TYPE_UINT8, "nc_info_index: ");
            }
        }
        return mResult.toString();
    }

    /**
     * @return the Calist information (rr_em_ca_list_info_struct)
     */
    public String getCaListInfo() {
        parseElement(TYPE_UINT8, "valid: ");
        parseElement(TYPE_UINT8, "number_of_channels: ");
        parseElement(TYPE_UINT16, "arfcn_list:", 64);
        return mResult.toString();
    }

    /**
     * @return the ControlMsg information (rr_em_control_msg_info_struct)
     */
    public String getControlMsgInfo() {
        parseElement(TYPE_UINT8, "msg_type: ");
        parseElement(TYPE_UINT8, "rr_cause: ");
        return mResult.toString();
    }

    /**
     * @return the SI2Q information (rr_em_si2q_info_struct)
     */
    public String getSI2QInfo() {
        parseElement(TYPE_INT8, "present: ");
        parseElement(TYPE_UINT8, "no_of_instance: ");
        parseElement(TYPE_INT8, "emr_report: ");
        parseElement(TYPE_INT8, "pemr_report: ");
        parseElement(TYPE_INT8, "umts_parameter_exist: ");
        return mResult.toString();
    }

    /**
     * @return the MI information (rr_em_mi_info_struct)
     */
    public String getMIInfo() {
        parseElement(TYPE_INT8, "present: ");
        parseElement(TYPE_UINT8, "no_of_instance: ");
        parseElement(TYPE_INT8, "emr_report: ");
        parseElement(TYPE_INT8, "umts_parameter_exist: ");
        return mResult.toString();
    }

    /**
     * @return the BLK information (rr_em_blk_info_struct)
     */
    public String getBLKInfo() {
        parseElement(TYPE_UINT8, "ul_coding_scheme: ");
        parseElement(TYPE_UINT8, "ul_cv: ");
        parseElement(TYPE_UINT8, "ul_tlli: ");
        parseElement(TYPE_UINT16, "ul_bsn1: ");
        if (GPRS_MODE_ENABLE) {
            parseElement(TYPE_UINT16, "ul_bsn2: ");
            parseElement(TYPE_UINT8, "ul_cps: ");
            parseElement(TYPE_UINT8, "ul_rsb: ");
            parseElement(TYPE_UINT8, "ul_spb: ");
        }
        parseElement(TYPE_UINT8, "dl_c_value_in_rx_level: ");
        parseElement(TYPE_UINT8, "dl_rxqual: ");
        parseElement(TYPE_UINT8, "dl_sign_var: ");
        parseElement(TYPE_UINT8, "dl_coding_scheme: ");
        parseElement(TYPE_UINT8, "dl_fbi: ");
        parseElement(TYPE_UINT16, "dl_bsn1: ");
        if (GPRS_MODE_ENABLE) {
            parseElement(TYPE_UINT16, "dl_bsn2: ");
            parseElement(TYPE_UINT8, "dl_cps: ");
            parseElement(TYPE_UINT8, "dl_gmsk_mean_bep_lev: ");
            parseElement(TYPE_UINT8, "dl_8psk_mean_bep_lev: ");
            parseElement(TYPE_UINT8, "dl_tn_mean_bep_lev:", 8);
        }
        parseElement(TYPE_UINT8, "dl_tn_interference_lev:", 8);
        return mResult.toString();
    }

    /**
     * @return the TBF information (rr_em_tbf_status_struct)
     */
    public String getTBFInfo() {
        parseElement(TYPE_UINT8, "tbf_mode: ");
        parseElement(TYPE_UINT8, "ul_tbf_status: ");
        parseElement(TYPE_UINT8, "ul_rel_cause: ");
        parseElement(TYPE_UINT8, "ul_ts_allocation: ");
        parseElement(TYPE_UINT8, "ul_rlc_mode: ");
        parseElement(TYPE_UINT8, "ul_mac_mode: ");
        parseElement(TYPE_UINT16, "number_rlc_octect: ");
        parseElement(TYPE_UINT8, "ul_tfi: ");
        parseElement(TYPE_UINT8, "ul_granularity: ");
        parseElement(TYPE_UINT8, "ul_usf: ");
        parseElement(TYPE_UINT8, "ul_tai: ");
        parseElement(TYPE_UINT16, "ul_tqi: ");
        parseElement(TYPE_UINT16, "ul_window_size: ");
        parseElement(TYPE_UINT8, "dl_tbf_status: ");
        parseElement(TYPE_UINT8, "dl_rel_cause: ");
        parseElement(TYPE_UINT8, "dl_ts_allocation: ");
        parseElement(TYPE_UINT8, "dl_rlc_mode: ");
        parseElement(TYPE_UINT8, "dl_mac_mode: ");
        parseElement(TYPE_UINT8, "dl_tfi: ");
        parseElement(TYPE_UINT8, "dl_tai: ");
        parseElement(TYPE_UINT16, "dl_window_size: ");
        if (GPRS_MODE_ENABLE) {
            parseElement(TYPE_UINT8, "dl_out_of_memory: ");
        }
        return mResult.toString();
    }

    /**
     * @return the GPRS GEN information (rr_em_gprs_general_info_struct)
     */
    public String getGPRSGenInfo() {
        parseElement(TYPE_UINT32, "t3192: ");
        parseElement(TYPE_UINT32, "t3168: ");
        parseElement(TYPE_UINT8, "rp: ");
        parseElement(TYPE_UINT8, "gprs_support: ");
        parseElement(TYPE_UINT8, "egprs_support: ");
        parseElement(TYPE_UINT8, "sgsn_r: ");
        parseElement(TYPE_UINT8, "pfc_support: ");
        parseElement(TYPE_UINT8, "epcr_support: ");
        parseElement(TYPE_UINT8, "bep_period: ");
        return mResult.toString();
    }

    public String get3GGeneralInfo() {
        if (getValueFromByte(mRawString, mOffset, false).equals("255")) {
            // "FF" means Invalid service_status
            mResult.append("service_status:\n");
            mOffset += 2;
        } else {
            parseElement(TYPE_UINT8, "service_status: ");
        }

        if (getValueFromByte(mRawString, mOffset, false).equals("255")) {
            // "FF" means Invalid umts_rrc_state
            mResult.append("umts_rrc_state:\n");
            mOffset += 2;
        } else {
            parseElement(TYPE_UINT8, "umts_rrc_state: ");
        }

        if (getValueFrom2Byte(mRawString, mOffset, false).equals("65535")) {
            // "FFFF" means Invalid uarfcn_DL
            mResult.append("uarfcn_DL:\n");
            mOffset += 4;
        } else {
            parseElement(TYPE_UINT16, "uarfcn_DL: ");
        }

        if (getValueFrom2Byte(mRawString, mOffset, false).equals("65535")) {
            // "FFFF" means Invalid psc
            mResult.append("psc:\n");
            mOffset += 4;
        } else {
            parseElement(TYPE_UINT16, "psc: ");
        }
        return mResult.toString();
    }

    /**
     * @return the slce voice information
     */
    public String getSlceVoiceInfo() {
        if (IS_MOLY) {
            parseElement(TYPE_UINT8, "ULAMRType: ");
            parseElement(TYPE_UINT8, "DLAMRType: ");
        } else {
            parseElement(TYPE_UINT8, "voice: ");
        }
        return mResult.toString();
    }

    /**
     * @return the slce voice information
     */
    public String getSecurityConfigInfo() {
        parseElement(TYPE_UINT8, "Ciphering Algorithm: ");
        parseElement(TYPE_UINT8, "Integrity Algorithm: ");
        return mResult.toString();
    }

    /**
     * @return the 3G memory information (mm_em_info_struct)
     */
    public String get3GMmEmInfo() {
        parseElement(TYPE_UINT8, "t3212: ");
        parseElement(TYPE_UINT8, "ATT_flag: ");
        parseElement(TYPE_UINT8, "MM_reject_cause: ");
        parseElement(TYPE_UINT8, "MM_state: ");
        parseElement(TYPE_UINT8, "MCC:", 3);
        parseElement(TYPE_UINT8, "MNC:", 3);
        parseElement(TYPE_UINT8, "LOC:", 2);
        parseElement(TYPE_UINT8, "rac: ");
        parseElement(TYPE_UINT8, "TMSI:", 4);
        parseElement(TYPE_UINT8, "is_t3212_running:");
        parseElement(TYPE_UINT16, "t3212_timer_value:");
        parseElement(TYPE_UINT16, "t3212_passed_time:");
        parseElement(TYPE_UINT8, "common_access_class: ");
        parseElement(TYPE_UINT8, "cs_access_class: ");
        parseElement(TYPE_UINT8, "ps_access_class: ");
        mOffset += DATA_OFFSET_8;
        return mResult.toString();
    }

    /**
     * @return the
     */
    public String getSmEmInfo() {
        parseElement(TYPE_UINT8, "num_of_active_pdp_context: ");
        parseElement(TYPE_ALIGNMENT, "");
        for (int i = 0; i < 11; i++) {
            mResult.append("--------------------\n");
            mResult.append("pdp[" + i + "]:\n");
            parseElement(TYPE_UINT8, "pdp_index: ");
            parseElement(TYPE_UINT8, "nsapi: ");
            if (IS_MOLY) {
                parseElement(TYPE_UINT8, "ti_value: ");
            }
            parseElement(TYPE_UINT8, "pdp_context_status: ");
            if (IS_MOLY) {
                parseElement(TYPE_UINT8, "context_type: ");
                parseElement(TYPE_UINT8, "initiated_by: ");
                parseElement(TYPE_UINT8, "pdp_addr_type: ");
            }
            parseElement(TYPE_UINT8, "ip:", 16);
            parseElement(TYPE_UINT16, "sdu_size:");
            parseElement(TYPE_STRING, "apn: ", 100);
            parseElement(TYPE_UINT8, "sm_cause: ");
            if (IS_MOLY) {
                mOffset += 249 * 2;
            } else {
                mOffset += 77 * 2; // Skip the rest 77 bytes
            }
        }
        return mResult.toString();
    }

    /**
     * @return the
     */
    public String getGmmEmInfo() {
        parseElement(TYPE_UINT16, "ready_timer: ");
        parseElement(TYPE_UINT16, "rau_timer: ");
        parseElement(TYPE_UINT8, "ms_state: ");
        parseElement(TYPE_INT8, "is_rau_timer_running: ");
        parseElement(TYPE_UINT16, "rau_timer_passed_time: ");
        parseElement(TYPE_UINT8, "attach_req_mobile_identity: ");
        if (WISDOM_EM && ADVANCED_EM) {
            parseElement(TYPE_UINT8, "ptmsi: ", 4);
            parseElement(TYPE_UINT8, "attach_rej_cause: ");
            parseElement(TYPE_UINT8, "rau_rej_cause: ");
            parseElement(TYPE_UINT8, "gprs_update_status: ");
            parseElement(TYPE_UINT8, "cipher_algo: ");
            parseElement(TYPE_UINT8, "attach_type: ");
            parseElement(TYPE_UINT8, "gmm_state: ");
            parseElement(TYPE_UINT8, "gprs_attach_status: ");
        }
        return mResult.toString();
    }

    /**
     * @return the 3G Tcm information (tcm_mmi_em_info_struct)
     */
    public String get3GTcmMmiEmInfo() {
        parseElement(TYPE_UINT8, "num_of_valid_entries: ");
        for (int i = 0; i < 3; i++) {
            parseElement(TYPE_UINT8, "nsapi" + i + ":");
            parseElement(TYPE_UINT8, "data_speed_value" + i + ":");
        }
        return mResult.toString();
    }

    /**
     * @return the 3G CsceEMServCellSStatusInd information (csce_em_serv_cell_s_status_ind_struct)
     */
    public String get3GCsceEMServCellSStatusInd(boolean isTdd) {
        if (!IS_MOLY) {
            parseElement(TYPE_UINT8, "ref_count:");
            parseElement(TYPE_UINT16, "msg_len");
        }
        parseElement(TYPE_UINT8, "cell_idx: ");
        parseElement(TYPE_UINT16, "uarfacn_DL: ");
        parseElement(TYPE_UINT16, "psc: ");
        parseElement(TYPE_UINT8, "is_s_criteria_satisfied: ");
        parseElement(TYPE_INT8, "qQualmin: ");
        parseElement(TYPE_INT8, "qRxlevmin: ");
        parseElement(TYPE_INT32, "srxlev: ");
        parseElement(TYPE_INT32, "spual: ");
        parseElement(TYPE_LONG, "rscp: ");
        if (!isTdd) {
            parseElement(TYPE_FLOAT, "ec_no: ");
        }
        parseElement(TYPE_UINT16, "cycle_len: ");
        if (!isTdd) {
            parseElement(TYPE_UINT8, "quality_measure: ");
        }
        parseElement(TYPE_UINT8, "band: ");
        parseElement(TYPE_INT32, "rssi: ");
        parseElement(TYPE_UINT32, "cell_identity: ");
        if (UMTS_R8) {
            parseElement(TYPE_UINT32, "csg_id: ");
            parseElement(TYPE_UINT8, "apbcr_priority: ");
            parseElement(TYPE_UINT8, "sprio_search1: ");
            parseElement(TYPE_UINT8, "sprio_search2: ");
            parseElement(TYPE_UINT8, "threshserv_low: ");
            if (IS_MOLY) {
                parseElement(TYPE_UINT8, "threshserv_low2: ");
            }
        }
        if (!isTdd) {
            parseElement(TYPE_UINT8, "multi_plmn_count: ");
            for (int i = 0; i < 6; i++) {
                parseElement(TYPE_UINT8, "multi_plmn_id[" + i + "].mcc: ", 3);
                parseElement(TYPE_UINT8, "multi_plmn_id[" + i + "].mnc: ", 3);
            }

            int lacValid = readIntegerFromByte();
            if (lacValid != 0) {
                parseElement(TYPE_UINT16, "lac: ");
            } else {
                mResult.append("lac: invalid\n");
                mOffset += DATA_OFFSET_4;
            }

            int racValid = readIntegerFromByte();
            if (racValid != 0) {
                parseElement(TYPE_UINT8, "rac: ");
            } else {
                mResult.append("rac: invalid\n");
                mOffset += DATA_OFFSET_2;
            }

            int uraValid = readIntegerFromByte();
            if (uraValid != 0) {
                parseElement(TYPE_UINT8, "num_ura_id: ");
                for (int i = 0; i < 8; i++) {
                    int numBits = readIntegerFromByte();
                    if (numBits == 1) {
                        parseElement(TYPE_UINT8, "uraIdentity[" + i + "]: ");
                        mOffset += DATA_OFFSET_2; // skip high byte
                    } else {
                        parseElement(TYPE_UINT8, "uraIdentity[" + i + "]: ", 2);
                    }
                }
            } else {
                mResult.append("ura: invalid\n");
            }
        }
        return mResult.toString();
    }

    /**
     * @return the 3G CsceEmInfoMultiPlmn information (csce_em_info_multiple_plmn_struct)
     */
    public String get3GCsceEmInfoMultiPlmn() {
        parseElement(TYPE_UINT8, "multi_plmn_count: ");
        for (int i = 0; i < 6; i++) {
            parseElement(TYPE_UINT8, "mcc1_" + i + ":");
            parseElement(TYPE_UINT8, "mcc2_" + i + ":");
            parseElement(TYPE_UINT8, "mcc3_" + i + ":");
            parseElement(TYPE_UINT8, "mnc1_" + i + ":");
            parseElement(TYPE_UINT8, "mnc2_" + i + ":");
            parseElement(TYPE_UINT8, "mnc3_" + i + ":");
        }
        return mResult.toString();
    }

    /**
     * @return the 3G MemeEmInfoUmtsCellStatus information (meme_em_info_umts_cell_status_struct)
     */
    public String get3GMemeEmInfoUmtsCellStatus() {
        parseElement(TYPE_INT8, "tx_power: ");
        parseElement(TYPE_UINT8, "num_cells: ");
        parseElement(TYPE_ALIGNMENT, "");
        String[] t = new String[32];
        for (int i = 0; i < 32; i++) {
            mResult.append("--------------------\n");
            mResult.append("umts_cell_list[" + i + "]:\n");
            parseElement(TYPE_UINT16, "UARFCN: ");
            parseElement(TYPE_UINT16, "PSC: ");
            parseElement(TYPE_INT32, "RSCP: ");
            parseElement(TYPE_INT32, "ECNO: ");
            parseElement(TYPE_UINT8, "cell_type: ");
            parseElement(TYPE_UINT8, "Band: ");
            parseElement(TYPE_INT32, "RSSI: ");
            parseElement(TYPE_UINT32, "Cell_identity: ");

            int validity = readIntegerFromByte();
            int lacValid = validity & 0x01;
            int racValid = validity & 0x02;
            int uraValid = validity & 0x04;

            parseElement(TYPE_UINT8, "num_plmn_id: ");
            for (int j = 0; j < 6; j++) {
                parseElement(TYPE_UINT16, "plmn_id_list[" + j + "].mcc: ");
                parseElement(TYPE_UINT16, "plmn_id_list[" + j + "].mnc: ");
            }

            if (lacValid != 0) {
                parseElement(TYPE_UINT16, "lac: ");
            } else {
                mResult.append("lac: invalid\n");
                mOffset += DATA_OFFSET_4;
            }

            if (racValid != 0) {
                parseElement(TYPE_UINT8, "rac: ");
            } else {
                mResult.append("rac: invalid\n");
                mOffset += DATA_OFFSET_2;
            }

            if (uraValid != 0) {
                parseElement(TYPE_UINT8, "num_ura_id: ");
                for (int j = 0; j < 8; j++) {
                    int numBits = readIntegerFromByte();
                    if (numBits == 1) {
                        parseElement(TYPE_UINT8, "uraIdentity[" + j + "]: ");
                        mOffset += DATA_OFFSET_2; // skip high byte
                    } else {
                        parseElement(TYPE_UINT8, "uraIdentity[" + j + "]: ", 2);
                    }
                }
                mOffset += DATA_OFFSET_4;
            } else {
                mResult.append("ura: invalid\n");
                mOffset += 27 * 2;
            }
        }
        return mResult.toString();
    }

    /**
     * @return the 3G MemeEmPeriodicBlerReport information (ul2_em_periodic_bler_report_ind)
     */
    public String get3GMemeEmPeriodicBlerReportInd() {
        parseElement(TYPE_UINT8, "num_trch: ");
        for (int i = 0; i < 8; i++) {
            parseElement(TYPE_ALIGNMENT, "");
            parseElement(TYPE_UINT8, "TrCHId" + i + ":");
            parseElement(TYPE_UINT32, "TotalCRC" + i + ":");
            parseElement(TYPE_UINT32, "BadCRC" + i + ":");
        }
        return mResult.toString();
    }

    /**
     * @return the 3G UrrUmtsSrnc information (urr_umts_srnc_id_struct)
     */
    public String get3GUrrUmtsSrncId() {
        parseElement(TYPE_UINT16, "srnc: ");
        return mResult.toString();
    }

    /**
     * @return the 3G SlceEmPsDataRateStatus information (slce_em_ps_data_rate_status_ind_struct)
     */
    public String get3GSlceEmPsDataRateStatusInd() {
        parseElement(TYPE_UINT8, "ps_number: ");
        parseElement(TYPE_ALIGNMENT, "");
        for (int i = 0; i < 8; i++) {
            parseElement(TYPE_UINT8, "RAB_ID" + i + ":");
            parseElement(TYPE_INT8, "RB_UD" + i + ":");
            parseElement(TYPE_UINT32, "DL_rate" + i + ":");
            parseElement(TYPE_UINT32, "UL_rate" + i + ":");
        }
        return mResult.toString();
    }

    /**
     * @return the 3G MemeEmInfoHServCell information (meme_em_info_h_serving_cell_ind_struct)
     */
    public String get3GMemeEmInfoHServCellInd() {
        parseElement(TYPE_UINT16, "HSDSCH_Serving_UARFCN: ");
        parseElement(TYPE_UINT16, "HSDSCH_Serving_PSC: ");
        parseElement(TYPE_UINT16, "EDCH_Serving_UARFCN: ");
        parseElement(TYPE_UINT16, "EDCH_Serving_PSC: ");
        return mResult.toString();
    }

    /**
     * @return the 3G HandoverSequence information (uas_em_handover_status)
     */
    public String get3GHandoverSequenceIndStuct() {
        parseElement(TYPE_UINT8, "service_status: ");
        parseElement(TYPE_ALIGNMENT, "");
        parseElement(TYPE_UINT16, "[old_cell_info:-----]\nprimary_uarfcn_DL: ");
        parseElement(TYPE_UINT16, "working_uarfcn: ");
        parseElement(TYPE_UINT16, "physicalCellId: ");
        parseElement(TYPE_UINT16, "[target_cell_info:-----]\nprimary_uarfcn_DL: ");
        parseElement(TYPE_UINT16, "working_uarfcn: ");
        parseElement(TYPE_UINT16, "physicalCellId: ");
        return mResult.toString();
    }

    /**
     * @return the 3G Ul2EmAdmPoolStatus information (ul2_em_adm_pool_status_ind_struct)
     */
    public String get3GUl2EmAdmPoolStatusIndStruct() {
        mResult.append("[dl_adm_poll_info:-----]\n");
        for (int i = 0; i < 4; i++) {
            parseElement(TYPE_UINT16, "max_usage_kbytes" + i + ":");
            parseElement(TYPE_UINT16, "avg_usage_kbytes" + i + ":");
        }
        mResult.append("[ul_adm_poll_info:-----]\n");
        for (int i = 0; i < 4; i++) {
            parseElement(TYPE_UINT16, "max_usage_kbytes" + i + ":");
            parseElement(TYPE_UINT16, "avg_usage_kbytes" + i + ":");
        }
        return mResult.toString();
    }

    /**
     * @return the 3G Ul2EmPsDataRateStatus information (ul2_em_ps_data_rate_status_ind_struct)
     */
    public String get3GUl2EmPsDataRateStatusIndStruct() {
        parseElement(TYPE_UINT16, "rx_mac_data_rate:");
        parseElement(TYPE_UINT16, "rx_pdcp_data_rate:");
        parseElement(TYPE_UINT16, "tx_mac_data_rate:");
        parseElement(TYPE_UINT16, "tx_pdcp_data_rate:");
        return mResult.toString();
    }

    /**
     * @return the 3G ul2EmHsdschReconfigStatus information
     *         (ul2_em_hsdsch_reconfig_status_ind_struct)
     */
    public String get3Gul2EmHsdschReconfigStatusIndStruct() {
        for (int i = 0; i < 8; i++) {
            parseElement(TYPE_UINT8, "reconfig_info" + i + ":");
        }
        return mResult.toString();
    }

    /**
     * @return the 3G Ul2EmUrlcEventStatus information (ul2_em_urlc_event_status_ind_struct)
     */
    public String get3GUl2EmUrlcEventStatusIndStruct() {
        parseElement(TYPE_INT8, "rb_id:");
        parseElement(TYPE_UINT8, "rlc_action:");
        parseElement(TYPE_UINT8, "rb_info:--- \nis_srb:");
        parseElement(TYPE_UINT8, "cn_domain:");
        parseElement(TYPE_UINT8, "rlc_info:--- \nrlc_mode:");
        parseElement(TYPE_UINT8, "direction:");
        parseElement(TYPE_UINT16, "rlc_parameter:--- \npdu_Size:");
        parseElement(TYPE_UINT16, "tx_window_size:");
        parseElement(TYPE_UINT16, "rx_window_size:");
        parseElement(TYPE_UINT8, "discard_mode:");
        parseElement(TYPE_UINT16, "discard_value:");
        parseElement(TYPE_UINT8, "flush_data_indicator:");
        parseElement(TYPE_UINT8, "reset_cause:");
        return mResult.toString();
    }

    /**
     * @return the 3G Ul2EmPeriodicBlerReport information (ul2_em_periodic_bler_report_ind)
     */
    public String get3GUl2EmPeriodicBlerReportInd() {
        parseElement(TYPE_UINT8, "num_trch:");
        parseElement(TYPE_ALIGNMENT, "");
        mResult.append("TrCHBler:--------");
        for (int i = 0; i < 8; i++) {
            parseElement(TYPE_UINT8, "TrCHId" + i + ":");
            parseElement(TYPE_UINT32, "TotalCRC" + i + ":");
            parseElement(TYPE_UINT32, "BadCRC" + i + ":");
        }
        return mResult.toString();
    }

    /**
     * @param isTdd
     *            is tdd or not
     *
     * @return the 3G CsceEMNeighCellSStatus information (csce_em_neigh_cell_s_status_ind_struct)
     */
    public String getxGCsceEMNeighCellSStatusIndStructSize(boolean isTdd) {
        if (!IS_MOLY) {
            parseElement(TYPE_UINT8, "ref_count:");
            parseElement(TYPE_UINT16, "msg_len");
        }
        parseElement(TYPE_UINT8, "neigh_cell_count:");
        parseElement(TYPE_UINT8, "operation:");
        String xgType = getValueFromByte(mRawString, mOffset, false);
        parseElement(TYPE_UINT8, "RAT_type:");
        parseElement(TYPE_ALIGNMENT, "");

        if (xgType.equals("1")) {
            mResult.append("----GSM_neigh_cells----\n");
            for (int i = 0; i < 16; i++) {
                parseElement(TYPE_UINT8, "cellidx" + i + ":");
                parseElement(TYPE_UINT16, "arfcn");
                parseElement(TYPE_UINT8, "bsic" + i + ":");
                parseElement(TYPE_UINT8, "is_bsic_verified" + i + ":");
                parseElement(TYPE_UINT8, "is_s_criteria_saticified" + i + ":");
                parseElement(TYPE_UINT8, "freq_band" + i + ":");
                parseElement(TYPE_INT8, "qRxlevmin" + i + ":");
                parseElement(TYPE_INT32, "srxlev" + i + ":");
                parseElement(TYPE_INT32, "rssi" + i + ":");
                parseElement(TYPE_INT8, "apbcr_priority" + i + ":");
                parseElement(TYPE_UINT8, "threshx_high" + i + ":");
                parseElement(TYPE_UINT8, "threshx_low" + i + ":");
                if (IS_MOLY) {
                    parseElement(TYPE_UINT8, "threshx_high2" + i + ":");
                    parseElement(TYPE_UINT8, "threshx_low2" + i + ":");
                }
                parseElement(TYPE_ALIGNMENT, "");
            }
        } else if (xgType.equals("2")) {
            mResult.append("----LTE_neigh_cells----\n");
            for (int i = 0; i < 16; i++) {
                parseElement(TYPE_UINT16, "earfcn");
                parseElement(TYPE_UINT16, "pci" + i + ":");
                parseElement(TYPE_INT32, "rsrp" + i + ":");
                parseElement(TYPE_INT32, "rsrq" + i + ":");
                parseElement(TYPE_INT32, "Treselection" + i + ":");
                parseElement(TYPE_INT8, "apbcr_priority" + i + ":");
                parseElement(TYPE_INT16, "qRxlevmin" + i + ":");
                parseElement(TYPE_INT8, "qQualMinEUTRA" + i + ":");
                parseElement(TYPE_UINT8, "threshx_high" + i + ":");
                parseElement(TYPE_UINT8, "threshx_low" + i + ":");
                parseElement(TYPE_UINT8, "threshx_high2" + i + ":");
                parseElement(TYPE_UINT8, "threshx_low2" + i + ":");
                parseElement(TYPE_ALIGNMENT, "");
            }
        } else {
            mResult.append("----3G_neigh_cells----\n");
            for (int i = 0; i < 16; i++) {
                parseElement(TYPE_UINT8, "cellidx" + i + ":");
                parseElement(TYPE_UINT16, "arfcn_DL");
                parseElement(TYPE_UINT16, "psc");
                parseElement(TYPE_UINT8, "is_s_criteria_saticified" + i + ":");
                parseElement(TYPE_INT8, "qQualmin" + i + ":");
                parseElement(TYPE_INT8, "qRxlevmin" + i + ":");
                parseElement(TYPE_INT32, "srxlev" + i + ":");
                parseElement(TYPE_INT32, "squal" + i + ":");
                parseElement(TYPE_LONG, "rscp: ");
                parseElement(TYPE_INT32, "ec_no" + i + ":");
                parseElement(TYPE_INT8, "apbcr_priority" + i + ":");
                parseElement(TYPE_UINT8, "threshx_high" + i + ":");
                parseElement(TYPE_UINT8, "threshx_low" + i + ":");
                if (IS_MOLY) {
                    parseElement(TYPE_UINT8, "threshx_high2" + i + ":");
                    parseElement(TYPE_UINT8, "threshx_low2" + i + ":");
                }
                if (!isTdd) {
                    parseElement(TYPE_UINT32, "cell_identity" + i + ":");
                    int plmnValid = readIntegerFromByte();
                    if (plmnValid != 0) {
                        parseElement(TYPE_UINT8, "multi_plmn_count: ");
                        for (int j = 0; j < 6; j++) {
                            parseElement(TYPE_UINT8, "multi_plmn_id[" + j + "].mcc: ", 3);
                            parseElement(TYPE_UINT8, "multi_plmn_id[" + j + "].mnc: ", 3);
                        }
                    } else {
                        mOffset += 37 * 2; // skip plmn data
                    }

                    int lacValid = readIntegerFromByte();
                    if (lacValid != 0) {
                        parseElement(TYPE_UINT16, "lac: ");
                    } else {
                        mResult.append("lac: invalid\n");
                        mOffset += DATA_OFFSET_6;
                    }

                    int racValid = readIntegerFromByte();
                    if (racValid != 0) {
                        parseElement(TYPE_UINT8, "rac: ");
                    } else {
                        mResult.append("rac: invalid\n");
                        mOffset += DATA_OFFSET_2;
                    }

                    int uraValid = readIntegerFromByte();
                    if (uraValid != 0) {
                        parseElement(TYPE_UINT8, "num_ura_id: ");
                        for (int j = 0; j < 8; j++) {
                            int numBits = readIntegerFromByte();
                            if (numBits == 1) {
                                parseElement(TYPE_UINT8, "uraIdentity[" + j + "]: ");
                                mOffset += DATA_OFFSET_2; // skip high byte
                            } else {
                                parseElement(TYPE_UINT8, "uraIdentity[" + j + "]: ", 2);
                            }
                        }
                    } else {
                        mOffset += 25 * 2; // skip ura data
                        mResult.append("ura: invalid\n");
                    }
                }
                parseElement(TYPE_ALIGNMENT, "");
            }
        }
        return mResult.toString();
    }

    private int readIntegerFromByte() {
        if (mRawString.length() < mOffset + 2) {
            mOffset += 2;
            return 0;
        }
        String str = mRawString.substring(mOffset, mOffset + 2);
        mOffset += 2;
        int ret = 0;
        try {
            ret = Integer.parseInt(str, 16);
        } catch (NumberFormatException e) {
            ret = 0;
        }
        return ret;
    }

    private String parseElement(int type, String label, int count) {
        String value = "";
        switch (type) {
        case TYPE_UINT8:
            value = oneBlockFromByte(label, mRawString, mOffset, false, count);
            mOffset += 2 * count;
            break;
        case TYPE_UINT16:
            if (ALIGN_MENT_ENABLE) {
                mOffset = (mOffset + 3) & ~3;
            }
            value = oneBlockFrom2Byte(label, mRawString, mOffset, false, count);
            mOffset += 4 * count;
            break;
        case TYPE_UINT32:
            if (ALIGN_MENT_ENABLE) {
                mOffset = (mOffset + 7) & ~7;
            }
            value = oneBlockFrom4Byte(label, mRawString, mOffset, false, count);
            mOffset += 8 * count;
            break;
        case TYPE_INT8:
            value = oneBlockFromByte(label, mRawString, mOffset, true, count);
            mOffset += 2 * count;
            break;
        case TYPE_INT16:
            if (ALIGN_MENT_ENABLE) {
                mOffset = (mOffset + 3) & ~3;
            }
            value = oneBlockFrom2Byte(label, mRawString, mOffset, true, count);
            mOffset += 4 * count;
            break;
        case TYPE_INT32:
            if (ALIGN_MENT_ENABLE) {
                mOffset = (mOffset + 7) & ~7;
            }
            value = oneBlockFrom4Byte(label, mRawString, mOffset, true, count);
            mOffset += 8 * count;
            break;
        case TYPE_LONG:
            if (ALIGN_MENT_ENABLE) {
                mOffset = (mOffset + 7) & ~7;
            }
            String strRscp = getValueFrom4Byte(mRawString, mOffset, true);
            long rscp = 0;
            try {
                rscp = Long.valueOf(strRscp) / 4096;
            } catch (NumberFormatException e) {
                Elog.v(TAG, "rscp = Long.valueOf(strRscp)/4096 exp.");
            }
            value = label + rscp + "\n";
            mOffset += 8;
            break;
        case TYPE_FLOAT:
            if (ALIGN_MENT_ENABLE) {
                mOffset = (mOffset + 7) & ~7;
            }
            String strEcno = getValueFrom4Byte(mRawString, mOffset, true);
            float ecno = 0;
            try {
                ecno = Float.valueOf(strEcno) / 4096;
            } catch (NumberFormatException e) {
                Elog.e(TAG, "ecno = Long.valueOf(strEcno)/4096 exp.");
            }
            value = label + ecno + "\n";
            mOffset += 8;
            break;
        case TYPE_STRING:
            value = label;
            for (int i = 0; i < count; i++) {
                String str = getValueFromByte(mRawString, mOffset, false);
                mOffset += 2;
                try {
                    short s = Short.valueOf(str);
                    value += Character.toString((char) s);
                } catch (NumberFormatException e) {
                    break;
                }
            }
            value += "\n";
            break;
        case TYPE_ALIGNMENT:
            if (ALIGN_MENT_ENABLE) {
                mOffset = (mOffset + 7) & ~7;
            }
            break;
        default:
            break;
        }
        mResult.append(value);
        return value;
    }

    private String parseElement(int type, String label) {
        return parseElement(type, label, 1);
    }

    /**
     * @param data
     *            the value of the bit
     * @param start
     *            the integer of the start position
     * @param signed
     *            the boolean of the signed is false
     * @return the value of the String for every item
     */
    private String getValueFromByte(String data, int start, boolean signed) {
        if (data.length() < start + DATA_OFFSET_2) {
            return "0";
        }
        try {
            String sub = data.substring(start, start + DATA_OFFSET_2);
            if (signed) {
                short s = Short.valueOf(sub, DATA_FORMAT);
                Byte b = (byte) s;
                return b.toString();
            } else {
                return Short.valueOf(sub, DATA_FORMAT).toString();
            }
        } catch (NumberFormatException e) {
            return "Error.";
        }
    }

    /**
     * @param data
     *            the value of the bit
     * @param start
     *            the integer of the start position
     * @param signed
     *            the boolean of the signed is false
     * @return the value of the String for every item
     */
    private String getValueFrom2Byte(String data, int start, boolean signed) {
        if (data.length() < start + DATA_OFFSET_4) {
            return "0";
        }
        try {
            String low = data.substring(start, start + DATA_OFFSET_2);
            String high = data.substring(start + DATA_OFFSET_2, start + DATA_OFFSET_4);
            String reverse = high + low;
            if (signed) {
                int i = Integer.valueOf(reverse, DATA_FORMAT);
                Short s = (short) i;
                return s.toString();
            } else {
                return Integer.valueOf(reverse, DATA_FORMAT).toString();
            }
        } catch (NumberFormatException e) {
            return "Error.";
        }
    }

    /**
     * @param data
     *            the value of the bit
     * @param start
     *            the integer of the start position
     * @param signed
     *            the boolean of the signed is false
     * @return the value of the String for every item
     */
    private String getValueFrom4Byte(String data, int start, boolean signed) {
        if (data.length() < start + DATA_OFFSET_8) {
            return "0";
        }
        try {
            String byte1 = data.substring(start, start + DATA_OFFSET_2);
            String byte2 = data.substring(start + DATA_OFFSET_2, start + DATA_OFFSET_4);
            String byte3 = data.substring(start + DATA_OFFSET_4, start + DATA_OFFSET_6);
            String byte4 = data.substring(start + DATA_OFFSET_6, start + DATA_OFFSET_8);
            String reverse = byte4 + byte3 + byte2 + byte1;
            if (signed) {
                long lg = Long.valueOf(reverse, DATA_FORMAT);
                Integer i = (int) lg;
                return i.toString();
            } else {
                return Long.valueOf(reverse, DATA_FORMAT).toString();
            }
        } catch (NumberFormatException e) {
            return "Error.";
        }
    }

    /**
     * @param data
     *            the String of the network item information
     * @param start
     *            the value of the network item start position bit
     * @param dataLength
     *            the block total bit
     * @param signed
     *            the define value is false
     * @return the block value to display
     */
    private String oneBlockFromByte(String label, String data, int start,
            boolean signed, int dataLength) {
        String block = new String(label);
        for (int i = 0; i < dataLength; i++) {
            if (dataLength > MAX_DATA_PER_LINE && 0 == i % MAX_DATA_PER_LINE) {
                block += "\n";
            }
            block += getValueFromByte(data, start, signed);
            start += DATA_OFFSET_2;
            if (i != dataLength - 1) {
                block += ", ";
            }
        }
        return block + "\n";
    }

    /**
     * @param data
     *            the String of the network item information
     * @param start
     *            the value of the network item start position bit
     * @param dataLength
     *            the block total bit
     * @param signed
     *            the define value is false
     * @return the block value to display
     */
    private String oneBlockFrom2Byte(String label, String data, int start,
            boolean signed, int dataLength) {
        String block = new String(label);
        for (int i = 0; i < dataLength; i++) {
            if (dataLength > MAX_DATA_PER_LINE && 0 == i % MAX_DATA_PER_LINE) {
                block += "\n";
            }
            block += getValueFrom2Byte(data, start, signed);
            start += DATA_OFFSET_4;
            if (i != dataLength - 1) {
                block += ", ";
            }
        }
        return block + "\n";
    }

    /**
     * @param data
     *            the String of the network item information
     * @param start
     *            the value of the network item start position bit
     * @param dataLength
     *            the block total bit
     * @param signed
     *            the define value is false
     * @return the block value to display
     */
    private String oneBlockFrom4Byte(String label, String data, int start,
            boolean signed, int dataLength) {
        String block = new String(label);
        for (int i = 0; i < dataLength; i++) {
            if (dataLength > MAX_DATA_PER_LINE && 0 == i % MAX_DATA_PER_LINE) {
                block += "\n";
            }
            block += getValueFrom4Byte(data, start, signed);
            start += DATA_OFFSET_8;
            if (i != dataLength - 1) {
                block += ", ";
            }
        }
        return block + "\n";
    }

    /* Add for LG */
    String[] parseSecurityStatus(int type, String info) {
        String[] m2gCipher = new String[] {
                "No Ciphering", "A5/1", "A5/2", "A5/3", "A5/4", "A5/5", "A5/6", "A5/7"};
        String[] m2gGprs = new String[] {"No Ciphering", "GEA1", "GEA2", "GEA3"};
        String[] m3gCipher = new String[] {"No Ciphering", "UEA0", "UEA1", "", "UEA2"};
        String[] m3gIntegrity = new String[] {"No Integrity", "UIA1", "UIA2"};
        String[] m4gEnasCipher = new String[] {
                "EEA0(NULL)", "EEA1(SNOW3G)", "EEA2(AES)", "EEA3(ZUC)"};
        String[] m4gEnasIntegrity = new String[] {
                "EIA0(NULL)", "EIA1(SNOW3G)", "EIA2(AES)", "EIA3(ZUC)"};
        String[] m4gErrcCipher = new String[] {
                "EEA0(NULL)", "EEA1(SNOW3G)", "EEA2(AES)", "EEA3(ZUC)"};
        String[] m4gErrcIntegrity = new String[] {
                "EIA0(NULL)", "EIA1(SNOW3G)", "EIA2(AES)", "EIA3(ZUC)"};

        String[] ret = new String[] {"---", "---"};
        if (info == null) {
            return ret;
        }
        Elog.v(TAG, "NetworkInfo ------ Type is: " + type);
        Elog.v(TAG, "NetworkInfo ------ Data is:\n");
        Elog.v(TAG, info + "\n");
        Elog.v(TAG, "NetworkInfo ---------------------------");
        mRawString = info;
        mResult = new StringBuilder();
        mOffset = 0;
        int cipherAlgo;
        int integrityAlgo;

        if (type == Content.CHANNEL_INDEX) {
            mOffset += 140 * 2;
            cipherAlgo = readIntegerFromByte();
            if (cipherAlgo >= 0 && cipherAlgo < m2gCipher.length) {
                ret[0] = m2gCipher[cipherAlgo];
            }
        } else if (type == Content.LLC_EM_INFO_INDEX) {
            mOffset += 32 * 2;
            cipherAlgo = readIntegerFromByte();
            if (cipherAlgo >= 0 && cipherAlgo < m2gGprs.length) {
                ret[0] = m2gGprs[cipherAlgo];
            }
        } else if (type == Content.SECURITY_CONFIGURATION_INDEX) {
            cipherAlgo = readIntegerFromByte();
            integrityAlgo = readIntegerFromByte();
            if (cipherAlgo >= 0 && cipherAlgo < m3gCipher.length) {
                ret[0] = m3gCipher[cipherAlgo];
            }
            if (integrityAlgo >= 0 && integrityAlgo < m3gIntegrity.length) {
                ret[1] = m3gIntegrity[integrityAlgo];
            }
        } else if (type == Content.ERRC_EM_SEC_PARAM_INDEX) {
            mOffset += 2 * 2;
            integrityAlgo = readIntegerFromByte();
            cipherAlgo = readIntegerFromByte();
            if (cipherAlgo >= 0 && cipherAlgo < m4gErrcCipher.length) {
                ret[0] = m4gErrcCipher[cipherAlgo];
            } else if (cipherAlgo == 0xFF) {
                ret[0] = "N/A";
            }
            if (integrityAlgo >= 0 && integrityAlgo < m4gErrcIntegrity.length) {
                ret[1] = m4gErrcIntegrity[integrityAlgo];
            } else if (integrityAlgo == 0xFF) {
                ret[1] = "N/A";
            }
        } else if (type == Content.EMM_L4C_EMM_INFO_INDEX) {
            mOffset += 89 * 2;
            integrityAlgo = readIntegerFromByte();
            cipherAlgo = readIntegerFromByte();
            if (cipherAlgo >= 0 && cipherAlgo < m4gEnasCipher.length) {
                ret[0] = m4gEnasCipher[cipherAlgo];
            } else if (cipherAlgo == 0xFF) {
                ret[0] = "N/A";
            }
            if (integrityAlgo >= 0 && integrityAlgo < m4gEnasIntegrity.length) {
                ret[1] = m4gEnasIntegrity[integrityAlgo];
            } else if (integrityAlgo == 0xFF) {
                ret[1] = "N/A";
            }
        } else if (type == Content.ERRC_EM_ERRC_STATE_INDEX) {
            ret[0] = getValueFromByte(mRawString, mOffset, false);
        }
        return ret;
    }

    /* Add for LG */
    String[] parseAntennaDiversity(int type, String info) {
        if (info == null) {
            return null;
        }
        String[] ret = null;
        Elog.v(TAG, "NetworkInfo ------ Type is: " + type);
        Elog.v(TAG, "NetworkInfo ------ Data is:\n");
        Elog.v(TAG, info + "\n");
        Elog.v(TAG, "NetworkInfo ---------------------------");
        mRawString = info;
        mResult = new StringBuilder();
        mOffset = 0;

        switch (type) {
        case Content.EL1TX_EM_TX_INFO_INDEX:
            int band = readIntegerFromByte();
            if (band == 0) {
                return null;
            }
            mOffset += 15 * 2;
            int rsrp0 = readIntegerFrom2Byte();
            int rsrp1 = readIntegerFrom2Byte();
            int rssi0 = readIntegerFrom2Byte();
            int rssi1 = readIntegerFrom2Byte();
            int snr0 = readIntegerFrom2Byte();
            int snr1 = readIntegerFrom2Byte();
            int rsrp = readIntegerFrom2Byte();
            int rsrq = readIntegerFrom2Byte();
            int snr = readIntegerFrom2Byte();
            ret = new String[] {String.valueOf(rsrp0), String.valueOf(rsrq), String.valueOf(rssi0),
                    String.valueOf(snr0), String.valueOf(rsrp1), String.valueOf(rsrq),
                    String.valueOf(rssi1), String.valueOf(snr1), String.valueOf(rsrp),
                    String.valueOf(rsrq), String.valueOf(rssi0 > rssi1 ? rssi0 : rssi1),
                    String.valueOf(snr)};
            break;
        case Content.UL1_EM_PRX_DRX_MEASURE_INFO_INDEX:
            int rscp0 = readIntegerFrom2Byte();
            int rscp1 = readIntegerFrom2Byte();
            int pssi0 = readIntegerFrom2Byte();
            int pssi1 = readIntegerFrom2Byte();
            ret = new String[] {String.valueOf(rscp0), String.valueOf(pssi0),
                    String.valueOf(rscp1), String.valueOf(pssi1),
                    String.valueOf(rscp0 > rscp1 ? rscp0 : rscp1),
                    String.valueOf(pssi0 > pssi1 ? pssi0 : pssi1)};
            break;
        default:
            break;
        }
        return ret;
    }

    /* Add for LG */
    private int readIntegerFrom2Byte() {
        if (mRawString.length() < mOffset + DATA_OFFSET_4) {
            mOffset += 4;
            return 0;
        }
        try {
            String low = mRawString.substring(mOffset, mOffset + DATA_OFFSET_2);
            String high = mRawString.substring(mOffset + DATA_OFFSET_2, mOffset + DATA_OFFSET_4);
            String reverse = high + low;
            mOffset += 4;
            int i = Integer.valueOf(reverse, DATA_FORMAT);
            Short s = (short) i;
            return s;
        } catch (NumberFormatException e) {
            return 0;
        }
    }

    /**
     * Parse CDMA info.
     *
     * @param type
     *            info type
     * @param info
     *            info content
     * @return formated info
     */
    public ArrayList<String[]> parseCdmaInfo(int type, String info) {
        Elog.v(TAG, "NetworkInfo ------ Type is: " + type);
        Elog.v(TAG, "NetworkInfo ------ Data is:\n");
        Elog.v(TAG, info + "\n");
        Elog.v(TAG, "NetworkInfo ---------------------------");
        mRawString = info;
        mRawValues = info.split(",");
        mOffset = 1;
        mType = type;

        switch (type) {
        case Content.CDMA_1XRTT_RADIO_INDEX:
            mLabels = mContext.getResources().getStringArray(R.array.network_info_1xrtt_radio);
            break;
        case Content.CDMA_1XRTT_INFO_INDEX:
            mLabels = mContext.getResources().getStringArray(R.array.network_info_1xrtt_info);
            break;
        case Content.CDMA_1XRTT_SCH_INFO_INDEX:
            mLabels = mContext.getResources().getStringArray(R.array.network_info_1xrtt_sch_info);
            break;
        case Content.CDMA_1XRTT_STATISTICS_INDEX:
            mLabels = mContext.getResources().getStringArray(R.array.network_info_1xrtt_statistics);
            break;
        case Content.CDMA_1XRTT_SERVING_INDEX:
            mLabels = mContext.getResources().getStringArray(R.array.network_info_1xrtt_serving);
            break;
        case Content.CDMA_EVDO_SERVING_INFO_INDEX:
            mLabels = mContext.getResources().getStringArray(
                    R.array.network_info_evdo_serving_info);
            break;
        case Content.CDMA_EVDO_ACTIVE_SET_INDEX:
            mLabels = mContext.getResources().getStringArray(R.array.network_info_evdo_active_set);
            break;
        case Content.CDMA_EVDO_CANDICATE_SET_INDEX:
            mLabels = mContext.getResources().getStringArray(R.array.network_info_evdo_set);
            break;
        case Content.CDMA_EVDO_NEIGHBOR_SET_INDEX:
            mLabels = mContext.getResources().getStringArray(R.array.network_info_evdo_set);
            break;
        case Content.CDMA_EVDO_FL_INDEX:
            mLabels = mContext.getResources().getStringArray(R.array.network_info_evdo_fl);
            break;
        case Content.CDMA_EVDO_RL_INDEX:
            mLabels = mContext.getResources().getStringArray(R.array.network_info_evdo_rl);
            break;
        case Content.CDMA_EVDO_STATE_INDEX:
            mLabels = mContext.getResources().getStringArray(R.array.network_info_evdo_state_info);
            break;
        default:
            break;
        }

        switch (type) {
        case Content.CDMA_1XRTT_SERVING_INDEX:
            return parse1xRttServing();
        case Content.CDMA_EVDO_ACTIVE_SET_INDEX:
        case Content.CDMA_EVDO_CANDICATE_SET_INDEX:
        case Content.CDMA_EVDO_NEIGHBOR_SET_INDEX:
            return parseCellInfo();
        default:
            return parseCommonCdmaInfo();
        }
    }

    private ArrayList<String[]> parseCommonCdmaInfo() {
        ArrayList<String[]> ret = new ArrayList<String[]>();
        for (int i = 0; i < mLabels.length; i++) {
            ret.add(new String[] {mLabels[i], nextValue()});
        }
        return ret;
    }

    private ArrayList<String[]> parseCellInfo() {
        ArrayList<String[]> ret = new ArrayList<String[]>();
        ret.add(mLabels);
        int columns = mLabels.length;
        int rows = 0;
        mColumn = -1;
        try {
            rows = Integer.parseInt(nextValue());
        } catch (NumberFormatException e) {
            return ret;
        }
        for (int i = 0; i < rows; i++) {
            ArrayList<String> entry = new ArrayList<String>();
            for (int j = 0; j < columns; j++) {
                mColumn = j;
                entry.add(nextValue());
            }
            ret.add(entry.toArray(new String[entry.size()]));
        }
        return ret;
    }

    private ArrayList<String[]> parse1xRttServing() {
        ArrayList<String[]> ret = new ArrayList<String[]>();
        ret.add(new String[] {"Active Set"});
        ret.addAll(parseCellInfo());
        ret.add(new String[] {"Candicate Set"});
        ret.addAll(parseCellInfo());
        ret.add(new String[] {"Neighbor Set"});
        ret.addAll(parseCellInfo());
        return ret;
    }

    private String nextValue() {
        int index = mOffset;
        mOffset++;
        if (mRawValues == null || mRawValues.length <= index) {
            return "";
        }

        String rawValue = mRawValues[index].trim();
        int value = 0;
        try {
            value = Integer.parseInt(rawValue);
        } catch (NumberFormatException e) {
            return rawValue;
        }

        String[] array = null;
        switch (mType) {
            case Content.CDMA_1XRTT_INFO_INDEX:
            switch (index) {
            case 1:
                array = new String[]
                        {"CP_DISABLED", "CP_SYS_DETERMINATION", "CP_PILOT_ACQUISITION",
                        "CP_SYNC_ACQUISITION", "CP_TIMING_CHANGE", "CP_IDLE", "CP_UPDATE_OHD_INFO",
                        "CP_PAGE_RESPONSE", "CP_ORD_MSG_RESP", "CP_ORIGINATION",
                        "CP_REGISTRATION", "CP_MSG_TRANSMISSION", "CP_TC_INIT",
                        "CP_TC_WAIT_ORDER", "CP_TC_WAIT_ANSWER", "CP_TC_CONVERSATION",
                        "CP_TC_RELEASE", "CP_NST", "CP_FROZEN",
                        "CP_TC_FROZEN"};
                break;
            case 2:
                array = new String[] {"TRUE", "FALSE", "Unknown"};
                break;
            case 14:
                array = new String[] {"FALSE", "TRUE"};
                break;
            case 15:
                return decodeMcc(value);
            case 16:
                return decodeMnc(value);
            case 19:
            case 20:
                return String.format("%.1f", (float) value / -2);
            case 22:
                return String.valueOf(value / 10);
            default:
                break;
            }
            break;
        case Content.CDMA_1XRTT_SCH_INFO_INDEX:
            switch (index) {
            case 1: // for_sch_mux
            case 6: // rev_sch_mux
                return String.format("0x%x", value);
            case 4: // ForSchDuration
            case 9: // RevSchDuration
                if (value == 15) {
                    return "Infinite";
                }
                break;
            default:
                break;
            }
            break;
        case Content.CDMA_1XRTT_SERVING_INDEX:
            if (mColumn == 1) {
                return String.format("%.1f", (float) value / -2);
            }
            break;
        case Content.CDMA_EVDO_SERVING_INFO_INDEX:
            switch (index) {
            case 5: // rssi
            case 6:
                return String.format("%.2f", (float) value / 128);
            case 15: // t_add
            case 16: // t_drop
                return String.format("%.1f", (float) value / -2);
            default:
                break;
            }
            break;
        case Content.CDMA_EVDO_ACTIVE_SET_INDEX:
            if (mColumn == 1) {
                return String.format("%.2f", (float) value / 8);
            }
            break;
        case Content.CDMA_EVDO_CANDICATE_SET_INDEX:
        case Content.CDMA_EVDO_NEIGHBOR_SET_INDEX:
            if (mColumn == 3) {
                return String.format("%.2f", (float) value / 8);
            }
            break;
        case Content.CDMA_EVDO_FL_INDEX:
            switch (index) {
            case 1:
                return String.format("%.2f", (float) value / 64);
            default:
                break;
            }
            break;
        case Content.CDMA_EVDO_RL_INDEX:
            switch (index) {
            case 4:
            case 5:
                return String.format("%.2f", (float) value / 128);
            default:
                break;
            }
            break;
        case Content.CDMA_EVDO_STATE_INDEX:
            switch (index) {
            case 1:
                array = new String[] {"NEW_SESSION", "ALIVE_SESSION", "PRIOR_SESSION",
                        "OPENED_SESSION"};
                break;
            case 2:
                array = new String[] {"AT_PWROFF", "AT_INACTIVE", "AT_PILOTACQ", "AT_SYNC",
                        "AT_IDLE", "AT_ACCESS", "AT_CONNECTED"};
                break;
            case 3:
                array = new String[] {"ALMP_INIT_STATE", "ALMP_IDLE_STATE", "ALMP_CONN_SETUP_STATE",
                        "ALMP_CONNECTED_STATE"};
                break;
            case 4:
                array = new String[] {"INSP_INACTIVE_STATE", "INSP_NETWORK_DET_STATE",
                        "INSP_PILOT_ACQ_STATE", "INSP_SYNC_STATE", "INSP_TIMING_CHANGE_STATE",
                        "INSP_WFR_1XASSTST_STATE"};
                break;
            case 5:
                array = new String[] {"IDP_INACTIVE_ST", "IDP_MONITOR_ST", "IDP_SLEEP_ST",
                        "IDP_CONN_SETUP_ST", "IDP_FREEZE_PENDING_ST", "IDP_FREEZE_ST",
                        "IDP_CONN_FROZEN_ST", "IDP_STATE_MAX"};
                break;
            case 6:
                array = new String[] {"OMP_INACTIVE_ST", "OMP_ACTIVE_ST", "OMP_STATE_MAX"};
                break;
            case 7:
                array = new String[] {"CSP_INACTIVE_STATE", "CSP_CLOSING_STATE", "CSP_OPEN_STATE"};
                break;
            case 8:
                array = new String[] {"RUP_INACTIVE", "RUP_IDLE", "RUP_CONNECTED",
                        "RUP_IRAT_MEASUREMENT", "RUP_INVALID"};
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }

        if (array != null && value >= 0 && value < array.length) {
            return array[value];
        }
        return rawValue;
    }

    /*
     * Modem will encode mcc/mnc before sending to AP, so decode it.
     * Encoding algorithm:
     * Suppose MCC is "abc", then encoded MCC will be:
     * 100 * (a == 0 ? 10 : a) + 10 * (b == 0 ? 10 : b) + (c == 0 ? 10 : c) - 111;
     * Suppose MNC is "ab", then encoded MNC will be:
     * 10 * (a == 0 ? 10 : a) + (b == 0 ? 10 : b) - 11;
     */
    private String decodeMcc(int value) {
        value += 111;
        if (value % 10 == 0) {
            value -= 10;
        }
        if ((value / 10) % 10 == 0) {
            value -= 100;
        }
        if ((value / 100) % 10 == 0) {
            value -= 1000;
        }
        return ("000" + value).substring(String.valueOf(value).length());
    }

    private String decodeMnc(int value) {
        value += 11;
        if (value % 10 == 0) {
            value -= 10;
        }
        if ((value / 10) % 10 == 0) {
            value -= 100;
        }
        return ("00" + value).substring(String.valueOf(value).length());
    }
}

