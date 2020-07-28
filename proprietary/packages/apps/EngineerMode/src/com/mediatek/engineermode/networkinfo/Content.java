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

package com.mediatek.engineermode.networkinfo;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
public class Content {
    public static final boolean NEW_VALUE = ChipSupport.getChip() >= ChipSupport.MTK_6735_SUPPORT;
    public static final boolean IS_MOLY = ModemCategory.isLteSupport();

    /* Item Index */
    public static final int CELL_INDEX = 0;
    public static final int CHANNEL_INDEX = 1;
    public static final int CTRL_INDEX = 2;
    public static final int RACH_INDEX = 3;
    public static final int LAI_INDEX = 4;
    public static final int RADIO_INDEX = 5;
    public static final int MEAS_INDEX = 6;
    public static final int CA_INDEX = 7;
    public static final int CONTROL_INDEX = 8;
    public static final int SI2Q_INDEX = 9;
    public static final int MI_INDEX = 10;
    public static final int BLK_INDEX = 11;
    public static final int TBF_INDEX = 12;
    public static final int GPRS_INDEX = 13;
    public static final int SM_INFO_INDEX = IS_MOLY ? 58 : 63;
    public static final int URR_3G_GENERAL_INDEX = 70;
    public static final int MM_INFO_INDEX = NEW_VALUE ? 53 : 21;
    public static final int GMM_INFO_INDEX = 56;
    public static final int TCM_MMI_INDEX = NEW_VALUE ? 59 : 27;
    public static final int CSCE_SERV_CELL_STATUS_INDEX = NEW_VALUE ? 75 : 47;
    public static final int CSCE_NEIGH_CELL_STATUS_INDEX = NEW_VALUE ? 76 : 48;
    public static final int CSCE_MULTIPLMN_INDEX = NEW_VALUE ? 81 : 52;
    public static final int UMTS_CELL_STATUS_INDEX = NEW_VALUE ? 90 : 53;
    public static final int PERIOD_IC_BLER_REPORT_INDEX = NEW_VALUE ? (IS_MOLY ? 97 : 99) : 62;
    public static final int URR_UMTS_SRNC_INDEX = NEW_VALUE ? 111 : 64;
    public static final int PSDATA_RATE_STATUS_INDEX = NEW_VALUE ? 140 : 65;
    public static final int HSERV_CELL_INDEX = NEW_VALUE ? (IS_MOLY ? 96 : 98) : 61;
    public static final int HANDOVER_SEQUENCE_INDEX = NEW_VALUE ? 130 : 65;
    public static final int UL_ADM_POOL_STATUS_INDEX = NEW_VALUE ? 185 : 67;
    public static final int UL_PSDATA_RATE_STATUS_INDEX = NEW_VALUE ? 186 : 68;
    public static final int UL_HSDSCH_RECONFIG_STATUS_INDEX = NEW_VALUE ? 187 : 69;
    public static final int UL_URLC_EVENT_STATUS_INDEX = NEW_VALUE ? 188 : 70;
    public static final int UL_PERIOD_IC_BLER_REPORT_INDEX = NEW_VALUE ? 189 : 71;

    public static final int CDMA_INDEX_BASE = 300;
    public static final int CDMA_1XRTT_RADIO_INDEX = CDMA_INDEX_BASE + 0;
    public static final int CDMA_1XRTT_INFO_INDEX = CDMA_INDEX_BASE + 1;
    public static final int CDMA_1XRTT_SCH_INFO_INDEX = CDMA_INDEX_BASE + 2;
    public static final int CDMA_1XRTT_STATISTICS_INDEX = CDMA_INDEX_BASE + 3;
    public static final int CDMA_1XRTT_SERVING_INDEX = CDMA_INDEX_BASE + 4;
    public static final int CDMA_EVDO_SERVING_INFO_INDEX = CDMA_INDEX_BASE + 5;
    public static final int CDMA_EVDO_ACTIVE_SET_INDEX = CDMA_INDEX_BASE + 6;
    public static final int CDMA_EVDO_CANDICATE_SET_INDEX = CDMA_INDEX_BASE + 7;
    public static final int CDMA_EVDO_NEIGHBOR_SET_INDEX = CDMA_INDEX_BASE + 8;
    public static final int CDMA_EVDO_FL_INDEX = CDMA_INDEX_BASE + 9;
    public static final int CDMA_EVDO_RL_INDEX = CDMA_INDEX_BASE + 10;
    public static final int CDMA_EVDO_STATE_INDEX = CDMA_INDEX_BASE + 11;
    public static final int CDMA_EVDO_FORCE_TX_ANT = CDMA_INDEX_BASE - 1;

    // add for LGE
    public static final int LLC_EM_INFO_INDEX = 57;
    public static final int UL1_EM_PRX_DRX_MEASURE_INFO_INDEX = 177;
    public static final int ERRC_EM_SEC_PARAM_INDEX = 217;
    public static final int ERRC_EM_ERRC_STATE_INDEX = 222;
    public static final int EMM_L4C_EMM_INFO_INDEX = 244;
    public static final int EL1TX_EM_TX_INFO_INDEX = 249;
    public static final int SLCE_VOICE_INDEX = NEW_VALUE ? (IS_MOLY ? 250 : 141) : 80;
    public static final int SECURITY_CONFIGURATION_INDEX = NEW_VALUE ? (IS_MOLY ? 158 : 157) : 81;

    /* Item data size */
    public static final int CELL_SEL_SIZE = 6;
    public static final int CH_DSCR_SIZE = 340;
    public static final int CTRL_CHAN_SIZE = 14;
    public static final int RACH_CTRL_SIZE = 14;
    public static final int LAI_INFO_SIZE = 28;
    public static final int RADIO_LINK_SIZE = 16;
    public static final int MEAS_REP_SIZE = 1384;
    public static final int CAL_LIST_SIZE = 260;
    public static final int CONTROL_MSG_SIZE = 4;
    public static final int SI2Q_INFO_SIZE = 10;
    public static final int MI_INFO_SIZE = 8;
    public static final int BLK_INFO_SIZE = 80;
    public static final int TBF_INFO_SIZE = 56;
    public static final int GPRS_GEN_SIZE = 32;
    public static final int URR_3G_GENERAL_SIZE = 12;

    // add for LGE
    public static final int SLCE_VOICE_SIZE = 1 * 2;
    public static final int SECURITY_CONFIGURATION_SIZE = 2 * 2;

    // LXO, stupid code..
    public static final int SM_EM_INFO_SIZE = 2204 * 2;
    public static final int M3G_MM_EMINFO_SIZE = 30 * 2;
    public static final int GMM_EM_INFO_SIZE = 20 * 2;
    public static final int M_3G_TCMMMI_INFO_SIZE = 7 * 2;
    public static final int CSCE_SERV_CELL_STATUS_SIZE = 52 * 2;
    public static final int CSCE_MULTI_PLMN_SIZE = 37 * 2;
    public static final int UMTS_CELL_STATUS_SIZE = 772 * 2;
    public static final int PERIOD_IC_BLER_REPORT_SIZE = 100 * 2;
    public static final int URR_UMTS_SRNC_SIZE = 2 * 2;
    public static final int SLCE_PS_DATA_RATE_STATUS_SIZE = 100 * 2;
    public static final int MEME_HSERV_CELL_SIZE = 8 * 2;

    public static final int HANDOVER_SEQUENCE_SIZE = 16 * 2; // alignment enabled
    public static final int ADM_POOL_STATUS_SIZE = 32 * 2;
    public static final int UL2_PSDATA_RATE_STATUS_SIZE = 8 * 2;
    public static final int UL_HSDSCH_RECONFIG_STATUS_SIZE = 8 * 2;
    public static final int URLC_EVENT_STATUS_SIZE = 18 * 2;
    public static final int UL_PERIOD_IC_BLER_REPORT_SIZE = 100 * 2;

    public static final int XGCSCE_NEIGH_CELL_STATUS_SIZE = 520 * 2;
}
