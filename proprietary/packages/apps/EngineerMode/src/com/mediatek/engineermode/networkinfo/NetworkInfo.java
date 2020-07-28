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

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.CheckBox;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

public class NetworkInfo extends Activity {

    public static final String TAG = "NetworkInfo";
    public static final int TOTAL_ITEM_NUM = 312; // max
    public static final int MODEM_FDD = 1;
    public static final int MODEM_TD = 2;
    public static final int MODEM_NO3G = 3;
    public static final int MODEM_NONE = -1;
    public static final int MODEM_MASK_GPRS = 0x01;
    public static final int MODEM_MASK_EDGE = 0x02;
    public static final int MODEM_MASK_WCDMA = 0x04;
    public static final int MODEM_MASK_TDSCDMA = 0x08;
    public static final int MODEM_MASK_HSDPA = 0x10;
    public static final int MODEM_MASK_HSUPA = 0x20;
    private static final int CHECK_INFOMATION_ID = Menu.FIRST; // only use Menu.FIRST
    private static int sModemType = MODEM_NONE;

    private CheckBox[] mCheckBox;
    private int[] mChecked;
    private int mSimType;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.networkinfo);
        Intent intent = getIntent();
        mSimType = intent.getIntExtra("mSimType", PhoneConstants.SIM_ID_1);
        sModemType = ModemCategory.getModemType();

        mCheckBox = new CheckBox[TOTAL_ITEM_NUM]; // may increase..
        mChecked = new int[TOTAL_ITEM_NUM];

        for (int i = 0; i < TOTAL_ITEM_NUM; i++) {
            mCheckBox[i] = null;
            mChecked[i] = 0;
        }

        mCheckBox[Content.CELL_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_Cell);
        mCheckBox[Content.CHANNEL_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_Ch);
        mCheckBox[Content.CTRL_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_Ctrl);
        mCheckBox[Content.RACH_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_RACH);
        mCheckBox[Content.LAI_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_LAI);
        mCheckBox[Content.RADIO_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_Radio);
        mCheckBox[Content.MEAS_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_Meas);
        mCheckBox[Content.CA_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_Ca);
        mCheckBox[Content.CONTROL_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_Control);
        mCheckBox[Content.SI2Q_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_SI2Q);
        mCheckBox[Content.MI_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_MI);
        mCheckBox[Content.BLK_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_BLK);
        mCheckBox[Content.TBF_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_TBF);
        mCheckBox[Content.GPRS_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_GPRS);

        mCheckBox[Content.URR_3G_GENERAL_INDEX] =
                (CheckBox) findViewById(R.id.NetworkInfo_3GGeneralStatus);
        mCheckBox[Content.SM_INFO_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_3GSmEmInfo);
        mCheckBox[Content.MM_INFO_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_3GMmEmInfo);
        mCheckBox[Content.GMM_INFO_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_3GGmmEmInfo);
        mCheckBox[Content.TCM_MMI_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_3GTcmMmiEmInfo);
        // 47
        mCheckBox[Content.CSCE_SERV_CELL_STATUS_INDEX] = (CheckBox)
                findViewById(R.id.NetworkInfo_3GCsceEMServCellSStatusInd); // FDD
        // !=
        // TDD
        mCheckBox[Content.CSCE_NEIGH_CELL_STATUS_INDEX] = (CheckBox)
                findViewById(R.id.NetworkInfo_xGCsceEMNeighCellSStatusInd); // 2G
        // !=
        // 3G
        mCheckBox[Content.CSCE_MULTIPLMN_INDEX] =
                (CheckBox) findViewById(R.id.NetworkInfo_3GCsceEmInfoMultiPlmn);
        // 53
        mCheckBox[Content.PERIOD_IC_BLER_REPORT_INDEX] = (CheckBox)
                findViewById(R.id.NetworkInfo_3GMemeEmPeriodicBlerReportInd);
        mCheckBox[Content.URR_UMTS_SRNC_INDEX] =
                (CheckBox) findViewById(R.id.NetworkInfo_3GUrrUmtsSrncId);
        mCheckBox[Content.HSERV_CELL_INDEX] =
                (CheckBox) findViewById(R.id.NetworkInfo_3GMemeEmInfoHServCellInd);

        View view2G = (View) findViewById(R.id.View_2G);
        View viewCommon = (View) findViewById(R.id.View_COMMON);
        View view3GFDD = (View) findViewById(R.id.View_3G_FDD);
        View view3GTDD = (View) findViewById(R.id.View_3G_TDD);
        View view3GCommon = (View) findViewById(R.id.View_3G_COMMON);
        if (mSimType == PhoneConstants.SIM_ID_1) {
            if (sModemType == MODEM_FDD) {
                view3GTDD.setVisibility(View.GONE);
                mCheckBox[Content.UMTS_CELL_STATUS_INDEX] = (CheckBox)
                        findViewById(R.id.NetworkInfo_3GMemeEmInfoUmtsCellStatus);
                mCheckBox[Content.PSDATA_RATE_STATUS_INDEX] = (CheckBox)
                        findViewById(R.id.NetworkInfo_3GSlceEmPsDataRateStatusInd);
            } else if (sModemType == MODEM_TD) {
                view3GFDD.setVisibility(View.GONE);
                mCheckBox[Content.HANDOVER_SEQUENCE_INDEX] = (CheckBox)
                        findViewById(R.id.NetworkInfo_3GHandoverSequenceIndStuct);
                mCheckBox[Content.UL_ADM_POOL_STATUS_INDEX] = (CheckBox)
                        findViewById(R.id.NetworkInfo_3GUl2EmAdmPoolStatusIndStruct);
                mCheckBox[Content.UL_PSDATA_RATE_STATUS_INDEX] = (CheckBox)
                        findViewById(R.id.NetworkInfo_3GUl2EmPsDataRateStatusIndStruct);
                mCheckBox[Content.UL_HSDSCH_RECONFIG_STATUS_INDEX] = (CheckBox)
                        findViewById(R.id.NetworkInfo_3Gul2EmHsdschReconfigStatusIndStruct);
                mCheckBox[Content.UL_URLC_EVENT_STATUS_INDEX] = (CheckBox)
                        findViewById(R.id.NetworkInfo_3GUl2EmUrlcEventStatusIndStruct);
                mCheckBox[Content.UL_PERIOD_IC_BLER_REPORT_INDEX] = (CheckBox)
                        findViewById(R.id.NetworkInfo_3GUl2EmPeriodicBlerReportInd);
            } else {
                view3GCommon.setVisibility(View.GONE);
                view3GTDD.setVisibility(View.GONE);
                view3GFDD.setVisibility(View.GONE);
            }
        } else {
            view2G.setVisibility(View.GONE);
            viewCommon.setVisibility(View.GONE);
            view3GCommon.setVisibility(View.GONE);
            view3GTDD.setVisibility(View.GONE);
            view3GFDD.setVisibility(View.GONE);
        }

        // add for CDMA
        if (ModemCategory.isCdma()) {
            mCheckBox[Content.CDMA_1XRTT_RADIO_INDEX]
                    = (CheckBox) findViewById(R.id.NetworkInfo_1xRTT_Radio);
            mCheckBox[Content.CDMA_1XRTT_INFO_INDEX]
                    = (CheckBox) findViewById(R.id.NetworkInfo_1xRTT_Info);
            mCheckBox[Content.CDMA_1XRTT_SCH_INFO_INDEX]
                    = (CheckBox) findViewById(R.id.NetworkInfo_1xRTT_SCH_Info);
            mCheckBox[Content.CDMA_1XRTT_STATISTICS_INDEX]
                    = (CheckBox) findViewById(R.id.NetworkInfo_1xRTT_Statistics);
            mCheckBox[Content.CDMA_1XRTT_SERVING_INDEX]
                    = (CheckBox) findViewById(R.id.NetworkInfo_1xRTT_Serving);
            mCheckBox[Content.CDMA_EVDO_SERVING_INFO_INDEX]
                    = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_Serving_Info);
            mCheckBox[Content.CDMA_EVDO_ACTIVE_SET_INDEX]
                    = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_Active_Set);
            mCheckBox[Content.CDMA_EVDO_CANDICATE_SET_INDEX]
                    = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_Candidate_Set);
            mCheckBox[Content.CDMA_EVDO_NEIGHBOR_SET_INDEX]
                    = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_Neighbor_Set);
            mCheckBox[Content.CDMA_EVDO_FL_INDEX]
                    = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_FL);
            mCheckBox[Content.CDMA_EVDO_RL_INDEX]
                    = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_RL);
            mCheckBox[Content.CDMA_EVDO_STATE_INDEX]
                    = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_State);

            mCheckBox[Content.CDMA_EVDO_FORCE_TX_ANT]
                    = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_Force_Tx_Ant);

        } else {
            findViewById(R.id.View_CDMA).setVisibility(View.GONE);
        }

        if (FeatureSupport.is90Modem()) {
            findViewById(R.id.NetworkInfo_EVDO_Force_Tx_Ant).setVisibility(View.GONE);
        }

        // add for LGE
        mCheckBox[Content.SLCE_VOICE_INDEX] = (CheckBox) findViewById(R.id.NetworkInfo_3GSlceVoice);
        mCheckBox[Content.SECURITY_CONFIGURATION_INDEX] =
                (CheckBox) findViewById(R.id.NetworkInfo_3GSecurityConfiguration);

        mCheckBox[Content.SLCE_VOICE_INDEX].setVisibility(View.GONE);
        mCheckBox[Content.SECURITY_CONFIGURATION_INDEX].setVisibility(View.GONE);

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, CHECK_INFOMATION_ID, 0, getString(R.string.networkinfo_check));
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {

        switch (aMenuItem.getItemId()) {
            case CHECK_INFOMATION_ID:
                Boolean isAnyChechked = false;
                for (int i = 0; i < TOTAL_ITEM_NUM; i++) {
                    if (mCheckBox[i] == null) {
                        continue;
                    }
                    if (mCheckBox[i].isChecked()) {
                        mChecked[i] = 1;
                        isAnyChechked = true;
                    } else {
                        mChecked[i] = 0;
                    }
                }
                if (!isAnyChechked) {
                    Toast.makeText(this, getString(R.string.networkinfo_msg), Toast.LENGTH_LONG)
                            .show();
                    break;
                }
                Intent intent = new Intent(this, NetworkInfoInfomation.class);
                intent.putExtra("mChecked", mChecked);
                intent.putExtra("mSimType", mSimType);
                this.startActivity(intent);
                break;
            default:
                break;

        }
        return super.onOptionsItemSelected(aMenuItem);
    }
}
