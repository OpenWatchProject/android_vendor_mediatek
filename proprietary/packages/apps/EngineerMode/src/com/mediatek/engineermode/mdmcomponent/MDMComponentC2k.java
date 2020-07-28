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

package com.mediatek.engineermode.mdmcomponent;

import android.app.Activity;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.mdml.Msg;
import com.mediatek.mdml.PlainDataDecoder;

import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.HashMap;
import java.lang.*;

class Cdma1xrttRadioInfo extends NormalTableTasComponent {
    protected static final String TAG = "EmInfo";

    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_RTT_RADIO_INFO_IND };

    int TasVersionC2K = 1;
    HashMap<Integer, String> AntennaMapping = new HashMap<Integer, String>() {
        {
            put(0, "LANT");
            put(1, "UANT");
            put(2, "LANT(')");
            put(3, "UANT");
            put(4, "-");
        }
    };
    HashMap<Integer, String> TasEnableMapping = new HashMap<Integer, String>() {
        {
            put(0, "DISABLE");
            put(1, "ENABLE");
            put(2, "-");
        }
    };

    String antidxMapping(int antidx) {
        String antidx_s = " ";
        if (antidx >= 0 && antidx <= 3)
            antidx_s = AntennaMapping.get(antidx);
        else
            antidx_s = AntennaMapping.get(4) + "(" + antidx + ")";
        return antidx_s;
    }

    String tasEableMapping(int tasidx) {
        String tasidx_s = "";
        if (tasidx >= 0 && tasidx <= 1) {
            tasidx_s = TasEnableMapping.get(tasidx);
        } else
            tasidx_s = TasEnableMapping.get(2) + "(" + tasidx + ")";
        return tasidx_s;
    }

    String bandClassMapping(int bandidx) {
        if (bandidx < 0 || bandidx > 20) {
            return "-" + "(" + bandidx + ")";
        }
        String bandidx_s = "BC";
        bandidx_s = bandidx_s + bandidx;
        return bandidx_s;
    }

    public Cdma1xrttRadioInfo(Activity context) {
        super(context);
    }

    String[] conbineLablesByModem(String[] lables1, String[] lables2,
            int position) {
        if (FeatureSupport.is93Modem()) {
            if (position < 0) {
                position = Math.abs(position);
                return addLablesAtPosition(lables2, lables1, position);
            }
            return addLablesAtPosition(lables1, lables2, position);
        }
        return lables2;
    }

    @Override
    String getName() {
        return "1xRTT radio info(TAS info)";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        final String[] Lables_Verison_v1 = new String[] { "Channel",
                "bandClass", "pilotPN", "rxPower_main(dbm)",
                "rxPower_div(dbm)", "txPower", "tx_Ant", "FER" };

        final String[] Lables_Verison_v2 = new String[] { "Channel",
                "bandClass", "Cur ant state", "pilotPN", "rxPower_LANT (dbm)",
                "rxPower_UANT (dbm)", "rxPower_LANT(') (dbm)", "tx_Ant", "FER" };
        final String[] Lables_Tas = new String[] { "Tas Enable Info" };

        if (TasVersionC2K == 2) {
            return conbineLablesByModem(Lables_Tas, Lables_Verison_v2,
                    Lables_Tas.length);
        }
        return conbineLablesByModem(Lables_Tas, Lables_Verison_v1,
                Lables_Tas.length);
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        int utas_info_valid = getFieldValue(data, MDMContent.C2K_L4_UTAS_ENABLE);
        setInfoValid(utas_info_valid);
        //@test
        //utas_info_valid = 1;
        if(isInfoValid()) {
            clearData();
            mAdapter.add(new String[] { "Use " + getName().replace("UTAS", "TAS") });
            return;
        }
        int tasidx = getFieldValue(data, "tas_enable");
        int channel = getFieldValue(data, "channel");
        int band_class = getFieldValue(data, "band_class");
        int pilot_pn_offset = getFieldValue(data, "pilot_pn_offset");
        int rx_power1 = getFieldValue(data, "rx_power", true);
        int rx_power2 = getFieldValue(data, "div_rx_power", true);
        int tx_power = getFieldValue(data, "tx_power", true);
        int tx_ant_id = getFieldValue(data, "tx_ant", true);
        int FER = getFieldValue(data, "fer");
        int tas_ver = getFieldValue(data, "tas_ver", true);

        if (tas_ver == 1) {
            TasVersionC2K = 1;
        } else {
            TasVersionC2K = 2;
        }
        Elog.d(TAG, "tas_ver = " + tas_ver);
        int tas_state = getFieldValue(data, "tas_state", true);

        int rx_power_dbmL = getFieldValue(data, "rx_power_dbmL", true);
        int rx_power_dbmU = getFieldValue(data, "rx_power_dbmU", true);
        int rx_power_dbmLp = getFieldValue(data, "rx_power_dbmLp", true);
        clearData();
        if (FeatureSupport.is93Modem()) {
            addData(tasEableMapping(tasidx));
        }
        if (TasVersionC2K == 1) {

            addData(channel);
            addData(bandClassMapping(band_class));
            addData(pilot_pn_offset);
            addData((rx_power1 == -150) ? "" : rx_power1);
            addData((rx_power2 == -150) ? "" : rx_power2);
            addData((tx_power == -150) ? "" : tx_power);
            addData(tx_ant_id);
            addData(FER);
        } else {
            addData(channel);
            addData(bandClassMapping(band_class));
            addData(antidxMapping(tas_state));
            addData(pilot_pn_offset);
            addData((rx_power_dbmL == -150) ? "" : rx_power_dbmL);
            addData((rx_power_dbmU == -150) ? "" : rx_power_dbmU);
            addData((rx_power_dbmLp == -150) ? "" : rx_power_dbmLp);
            addData(tx_ant_id);
            addData(FER);
        }

        notifyDataSetChanged();

    }
}

class Cdma1xrttRadioUTasInfo extends CombinationTableComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_RTT_RADIO_INFO_IND };
    HashMap<Integer, String> ServingBandMapping = new HashMap<Integer, String>() {
        {
            put(0, "B34");
            put(1, "B39");
            put(5, "-");
        }
    };
    HashMap<Integer, String> TasEnableMapping = new HashMap<Integer, String>() {
        {
            put(0, "DISABLE");
            put(1, "ENABLE");
            put(2, "-");
        }
    };

    String tasEableMapping(int tasidx) {
        String tasidx_s = " ";
        if (tasidx >= 0 && tasidx <= 1) {
            tasidx_s = TasEnableMapping.get(tasidx);
        }
        return tasidx_s;
    }

    String servingBandMapping(int bandidx) {
        String bandidx_s = " ";
        if (ServingBandMapping.containsKey(bandidx))
            bandidx_s = ServingBandMapping.get(bandidx);

        return bandidx_s;
    }

    private String[] tabTitle = new String[] {"Common", "Detail"};

    public Cdma1xrttRadioUTasInfo(Activity context) {
        super(context);
        initTableComponent(tabTitle);
    }

    List<String[]> labelsList = new ArrayList<String[]>(){{
        add(new String[] {"TAS_EN", "TAS Version",
                "BandClass", "Channel", "PilotPN", "FER"});
        add(new String[] {"Cur Ant State", "TX Ant"});
        add(new String[] { "ANT Index", "RxPower(dBm)", "TxPower(dBm)", "PHR(dB)"});

    }};

    String[] labelsKey = new String[] {"Tas Common", "Main Info", "ANT Info"};

    List<LinkedHashMap> valuesHashMap = new ArrayList<LinkedHashMap>(){{
        add(initHashMap(labelsList.get(0)));
        add(initHashMap(labelsList.get(1)));
        add(initArrayHashMap(labelsList.get(2)));
    }};

    @Override
    String getName() {
        return "1xRTT radio info(UTAS info)";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    ArrayList<String> getArrayTypeKey() {
        ArrayList<String> arrayTypeKeys = new ArrayList<String>();
        arrayTypeKeys.add(labelsKey[2]);
        return arrayTypeKeys;
    }

    @Override
    boolean isLabelArrayType(String label) {
        if(getArrayTypeKey().contains(label)) {
            return true;
        }
        return false;
    }

    @Override
    LinkedHashMap<String, LinkedHashMap> getHashMapLabels(int index) {
        LinkedHashMap<String, LinkedHashMap> hashMapkeyValues =
                new LinkedHashMap <String, LinkedHashMap>();
        switch(index) {
        case 0:
            hashMapkeyValues.put(labelsKey[0], valuesHashMap.get(0));
            break;
        case 1:
            hashMapkeyValues.put(labelsKey[1], valuesHashMap.get(1));
            hashMapkeyValues.put(labelsKey[2], valuesHashMap.get(2));
            break;
        }
        return hashMapkeyValues;
    }

    @Override
    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        int utas_info_valid = getFieldValue(data, MDMContent.C2K_L4_UTAS_ENABLE);
        setInfoValid(utas_info_valid);
        if(!isInfoValid()) {
            resetView();
            TextView textView = new TextView(mContext);
            ListView.LayoutParams layoutParams = new ListView.LayoutParams(
                    ListView.LayoutParams.MATCH_PARENT, ListView.LayoutParams.WRAP_CONTENT);
            textView.setLayoutParams(layoutParams);
            textView.setPadding(20, 0, 20, 0);
            textView.setText("Use " + getName().replace("UTAS", "TAS"));
            textView.setTextSize(16);
            scrollView.addView(textView);
            return;
        }
        int tas_enable = getFieldValue(data, MDMContent.C2K_L4_TAS_ENABLE);
        int tas_ver = getFieldValue(data, MDMContent.C2K_L4_TAS_VER);
        int band_class = getFieldValue(data, MDMContent.C2K_L4_BAND_CLASS);
        int channel = getFieldValue(data, MDMContent.C2K_L4_CHANNEL);
        int pilot_pn_offset = getFieldValue(data, MDMContent.C2K_L4_PILOT_PN_OFFSET);
        int fer = getFieldValue(data, MDMContent.C2K_L4_FER);
        setHashMapKeyValues(labelsKey[0], 0, labelsList.get(0), new String[] {
            tasEableMapping(tas_enable),
            tas_ver >=1 && tas_ver <= 3 ? tas_ver+".0" : "-("+tas_ver+")",
            band_class+"",
            channel+"",
            pilot_pn_offset+"",
            fer+""
        });

        int tas_state = getFieldValue(data, MDMContent.C2K_L4_TAS_STATE);
        int tx_ant = getFieldValue(data, MDMContent.C2K_L4_TX_ANT);
        setHashMapKeyValues(labelsKey[1], 1, labelsList.get(1), new String[] {
            tas_state >= 0 ? "State"+tas_state : "-("+tas_state+")",
            tx_ant >= 0 && tx_ant <= 4 ? tx_ant+"" : "-("+tx_ant+")"
        });
        String coName = MDMContent.C2K_L4_EM_C2K_L4_RTT_ANT_INFO;
        for(int i=0; i<8; i++) {
            int valid = getFieldValue(data, coName+"["+i+"]."+MDMContent.C2K_L4_VALID);
            if(valid ==0) continue;
            int rx_power = getFieldValue(data, coName+"["+i+"]."+MDMContent.C2K_L4_RX_POWER, true);
            int tx_power = getFieldValue(data, coName+"["+i+"]."+MDMContent.C2K_L4_TX_POWER, true);
            int phr = getFieldValue(data, coName+"["+i+"]."+MDMContent.C2K_L4_PHR, true);
            setHashMapKeyValues(labelsKey[2], 1, labelsList.get(2), new String[] {
                i+"",
                rx_power <= -35 && rx_power >= -110 ? rx_power +"" : "-("+rx_power+")",
                tx_power <= 24 && tx_power >= -60 ? tx_power +"" : "-("+tx_power+")",
                phr <= 85 && phr >= -50 ? phr +"" : "-("+phr+")",
            });
        }
        addData(labelsKey[0], 0);
        addData(labelsKey[1], 1);
        addData(labelsKey[2], 1);
    }
}

class EvdoServingInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_EVDO_SERVING_INFO_IND };

    public EvdoServingInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EVDO Serving Info";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[] { "bandClass", "Channel", "pilotPN", "PhySubtype",
                "rssi_dbm", "div_rssi", "tx_Ant", "SectorID", "SubnetMask",
                "ColorCode", "UATI", "PilotInc", "ActiveSetWindow",
                "NeighborSetWindow", "RemainSetWindow", "sameFreq_T_ADD",
                "sameFreq_T_DROP", "sameFreq_T_tDROP", "diffFreq_T_ADD",
                "diffFreq_T_DROP", "diffFreq_T_tDROP" };
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        int band_class = getFieldValue(data, "band_class");
        int channel = getFieldValue(data, "channel");

        int pilotPN = getFieldValue(data, "pilotPN");
        int phy_subtype = getFieldValue(data, "phy_subtype", true);
        int rssi_dbm = getFieldValue(data, "rssi_dbm", true) / 128;
        int div_rssi = getFieldValue(data, "div_rssi", true) / 128;
        int tx_ant_id = getFieldValue(data, "tx_ant_id", true);

        char[] sectorlD = new char[34];
        for (int i = 0; i < 34; i++) {
            sectorlD[i] = (char) getFieldValue(data, "sectorlD[" + i + "]");
        }
        String sectorlD_s = new String(sectorlD);

        int subnetMask = getFieldValue(data, "subnetMask");
        int colorCode = getFieldValue(data, "colorCode", true);

        char[] uati = new char[34];
        for (int i = 0; i < 34; i++) {
            uati[i] = (char) getFieldValue(data, "uati[" + i + "]");
        }
        String uati_s = new String(uati);
        int pilotlnc = getFieldValue(data, "pilotlnc", true);
        int activeSetSchWin = getFieldValue(data, "activeSetSchWin", true);
        int neighborSetSchWin = getFieldValue(data, "neighborSetSchWin", true);

        int remainSetSchWin = getFieldValue(data, "remainSetSchWin");
        int sameFreq_T_ADD = getFieldValue(data, "sameFreq_T_ADD", true);
        int sameFreq_T_DROP = getFieldValue(data, "sameFreq_T_DROP", true);
        int sameFreq_T_tDROP = getFieldValue(data, "sameFreq_T_tDROP", true);

        int diffFreq_T_ADD = getFieldValue(data, "diffFreq_T_ADD", true);
        int diffFreq_T_DROP = getFieldValue(data, "diffFreq_T_DROP", true);
        int diffFreq_T_tDROP = getFieldValue(data, "diffFreq_T_tDROP", true);
        Elog.d(TAG, "band_class = " + band_class);
        clearData();
        addData(band_class);
        addData(channel);
        addData(pilotPN);
        addData(phy_subtype);
        addData(rssi_dbm);
        addData(div_rssi);
        addData(tx_ant_id);

        addData(sectorlD_s);

        addData(subnetMask);
        addData(colorCode);

        addData(uati_s);

        addData(pilotlnc);
        addData(activeSetSchWin);
        addData(neighborSetSchWin);
        addData(remainSetSchWin);
        addData((float) sameFreq_T_ADD / -2.0f);
        addData((float) sameFreq_T_DROP / -2.0f);
        addData(sameFreq_T_tDROP);
        addData((float) diffFreq_T_ADD / -2.0f);
        addData((float) diffFreq_T_DROP / -2.0f);
        addData(diffFreq_T_tDROP);

        notifyDataSetChanged();

    }
}

class Cdma1xrttInfo extends NormalTableComponent {

    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_RTT_INFO_IND };

    HashMap<Integer, String> StateMapping = new HashMap<Integer, String>() {
        {
            put(0, "CALIBRATED");
            put(1, "NOT_CALIBRATED");
            put(2, "FILE_NOT_EXIST");
        }
    };
    HashMap<Integer, String> ValueMapping = new HashMap<Integer, String>() {
        {
            put(0, "FALSE");
            put(1, "TRUE");
        }
    };
    HashMap<Integer, String> CPMapping = new HashMap<Integer, String>() {
        {
            put(0, "CP_DISABLED");
            put(1, "CP_SYS_DETERMINATION");
            put(2, "CP_PILOT_ACQUISITION");
            put(3, "CP_SYNC_ACQUISITION");
            put(4, "CP_TIMING_CHANGE");
            put(5, "CP_IDLE");
            put(6, "CP_UPDATE_OHD_INFO");
            put(7, "CP_ORD_MSG_RESP");
            put(8, "CP_ORIGINATION");
            put(9, "CP_REGISTRATION");

            put(10, "CP_MSG_TRANSMISSION");
            put(11, "CP_TC_INIT");
            put(12, "CP_TC_WAIT_ORDER");
            put(13, "CP_TC_WAIT_ANSWER");
            put(14, "CP_TC_CONVERSATION");

            put(15, "CP_TC_RELEASE");
            put(16, "CP_NST");
            put(17, "CP_FROZEN");
            put(18, "CP_TC_FROZEN");

        }
    };

    public Cdma1xrttInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "1xRTT info";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[] { "cp_state", "rf_file_cal_state",
                "RfFileMajorVersion", "RfFileMinorVersion",
                "RfFileValueVersion", "RfFileCustVersion", "sid", "nid",
                "sys_det_ind", "reg_zone", "base_lat", "base_long",
                "nwk_pref_sci", "qpch_mode", "mcc", "imsi_11_12",
                "pkt_zone_id", "service_option", "t_add", "t_drop", "t_comp",
                "t_tdrop" };
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    /*
     * Modem will encode mcc/mnc before sending to AP, so decode it. Encoding
     * algorithm: Suppose MCC is "abc", then encoded MCC will be: 100 * (a == 0
     * ? 10 : a) + 10 * (b == 0 ? 10 : b) + (c == 0 ? 10 : c) - 111; Suppose MNC
     * is "ab", then encoded MNC will be: 10 * (a == 0 ? 10 : a) + (b == 0 ? 10
     * : b) - 11;
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

    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int cp_state = getFieldValue(data, "cp_state");
        int rf_file_cal_state = getFieldValue(data, "rf_file_cal_state");

        int major_ver = getFieldValue(data, "rf_file_ver.major_ver");
        int minor_ver = getFieldValue(data, "rf_file_ver.minor_ver");
        int value_ver = getFieldValue(data, "rf_file_ver.value_ver");
        int cust_ver = getFieldValue(data, "rf_file_ver.cust_ver");

        int sid = getFieldValue(data, "sid");
        int nid = getFieldValue(data, "nid");

        int sys_det_ind = getFieldValue(data, "sys_det_ind");

        int reg_zone = getFieldValue(data, "reg_zone");
        int base_lat = getFieldValue(data, "base_lat");
        int base_long = getFieldValue(data, "base_long");

        int nwk_pref_sci = getFieldValue(data, "nwk_pref_sci");
        int qpch_mode = getFieldValue(data, "qpch_mode");
        int mcc = getFieldValue(data, "mcc");
        int imsi_11_12 = getFieldValue(data, "imsi_11_12");

        int pkt_zone_id = getFieldValue(data, "pkt_zone_id");
        int service_option = getFieldValue(data, "service_option");
        int t_add = getFieldValue(data, "t_add");

        int t_drop = getFieldValue(data, "t_drop");
        int t_comp = getFieldValue(data, "t_comp");
        int t_tdrop = getFieldValue(data, "t_tdrop");

        Elog.d(TAG, "t_tdrop = " + t_tdrop);
        clearData();
        addData(CPMapping.get(cp_state));
        addData(StateMapping.get(rf_file_cal_state));
        addData(major_ver);
        addData(minor_ver);
        addData(value_ver);
        addData(cust_ver);

        addData(sid);
        addData(nid);
        addData(sys_det_ind);
        addData(reg_zone);
        addData(base_lat);
        addData(base_long);
        addData(nwk_pref_sci);
        addData(ValueMapping.get(qpch_mode));
        addData(decodeMcc(mcc));
        addData(decodeMnc(imsi_11_12));
        addData(pkt_zone_id);
        addData(service_option);
        addData((float) t_add / -2.0f);
        addData((float) t_drop / -2.0f);
        addData(t_comp);
        addData(t_tdrop);
        notifyDataSetChanged();
    }
}

class Cdma1xSchInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_RTT_SCH_INFO_IND };

    public Cdma1xSchInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "1xRTT SCH info";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[] { "for_sch_mux", "for_sch_rc", "for_sch_status",
                "for_sch_duration", "for_sch_rate", "rev_sch_mux",
                "rev_sch_rc", "rev_sch_status", "rev_sch_duration",
                "rev_sch_rate" };
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        int for_sch_mux = getFieldValue(data, "for_sch_mux");
        int for_sch_rc = getFieldValue(data, "for_sch_rc");
        int for_sch_status = getFieldValue(data, "for_sch_status");
        int for_sch_duration = getFieldValue(data, "for_sch_duration");
        int for_sch_rate = getFieldValue(data, "for_sch_rate");
        int rev_sch_mux = getFieldValue(data, "rev_sch_mux");
        int rev_sch_rc = getFieldValue(data, "rev_sch_rc");

        int rev_sch_status = getFieldValue(data, "rev_sch_status");
        int rev_sch_duration = getFieldValue(data, "rev_sch_duration");
        int rev_sch_rate = getFieldValue(data, "rev_sch_rate");

        Elog.d(TAG, "rev_sch_rate = " + rev_sch_rate);
        clearData();
        addData(for_sch_mux);
        addData(for_sch_rc);
        addData(for_sch_status);
        addData(for_sch_duration);
        addData(for_sch_rate);
        addData(rev_sch_mux);
        addData(rev_sch_rc);
        addData(rev_sch_status);
        addData(rev_sch_duration);
        addData(rev_sch_rate);
        notifyDataSetChanged();
    }
}

class Cdma1xStatisticsInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_RTT_STAT_INFO_IND };

    public Cdma1xStatisticsInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "1xRTT statistics info";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[] { "total_msg", "error_msg", "acc_1", "acc_2",
                "acc_8", "dpchLoss_count", "dtchLoss_count", "idleHO_count",
                "hardHO_count", "interFreqldleHO_count",
                "silentryRetryTimeout_count", "T40_count", "T41_count" };
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        int total_msg = getFieldValue(data, "total_msg");
        int error_msg = getFieldValue(data, "error_msg");
        int acc_1 = getFieldValue(data, "acc_1");
        int acc_2 = getFieldValue(data, "acc_2");
        int acc_8 = getFieldValue(data, "acc_8");

        int dpchLoss_count = getFieldValue(data, "dpchLoss_count");

        int dtchLoss_count = getFieldValue(data, "dtchLoss_count");
        int idleHO_count = getFieldValue(data, "idleHO_count");
        int hardHO_count = getFieldValue(data, "hardHO_count");

        int interFreqldleHO_count = getFieldValue(data, "interFreqldleHO_count");
        int silentryRetryTimeout_count = getFieldValue(data,
                "silentryRetryTimeout_count", true);
        int T40_count = getFieldValue(data, "T40_count");
        int T41_count = getFieldValue(data, "T41_count");
        Elog.d(TAG, "T41_count = " + T41_count);
        clearData();
        addData(total_msg);
        addData(error_msg);
        addData(acc_1);
        addData(acc_2);
        addData(acc_8);
        addData(dpchLoss_count);
        addData(dtchLoss_count);
        addData(idleHO_count);
        addData(hardHO_count);
        addData(interFreqldleHO_count);
        addData(silentryRetryTimeout_count);
        addData(T40_count);
        addData(T41_count);
        notifyDataSetChanged();
    }
}

class Cdma1xSeringNeihbrInfo extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_RTT_SERVING_NEIGHBR_SET_INFO_IND };

    public Cdma1xSeringNeihbrInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "1xRTT Serving/Neighbr info";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[] { "pn", "ecio", "phase" };
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        int[] value = new int[3];
        clearData();
        int num_in_active_set = getFieldValue(data, "num_in_active_set");
        int num_candidate_set = getFieldValue(data, "num_candidate_set");
        int num_neighbor_set = getFieldValue(data, "num_neighbor_set");

        addData("in_active_set(" + num_in_active_set + ")");
        for (int i = 0; i < num_in_active_set; i++) {
            value[0] = getFieldValue(data, "in_active_set[" + i + "]"
                    + ".pilot_pn");
            value[1] = getFieldValue(data, "in_active_set[" + i + "]"
                    + ".pilot_ecio");
            float ecio_f = -(value[1] / 2.0f);
            value[2] = getFieldValue(data, "in_active_set[" + i + "]"
                    + ".pilot_phase");
            addData(value[0], ecio_f, value[2]);
        }

        addData("candidate_set(" + num_candidate_set + ")");
        for (int i = 0; i < num_candidate_set; i++) {
            value[0] = getFieldValue(data, "candidate_set[" + i + "]"
                    + ".pilot_pn");
            value[1] = getFieldValue(data, "candidate_set[" + i + "]"
                    + ".pilot_ecio");
            float ecio_f = -(value[1] / 2.0f);
            value[2] = getFieldValue(data, "candidate_set[" + i + "]"
                    + ".pilot_phase");
            addData(value[0], ecio_f, value[2]);
        }

        addData("neighbor_set(" + num_neighbor_set + ")");
        for (int i = 0; i < num_neighbor_set; i++) {
            value[0] = getFieldValue(data, "neighbor_set[" + i + "]"
                    + ".pilot_pn");
            value[1] = getFieldValue(data, "neighbor_set[" + i + "]"
                    + ".pilot_ecio");
            float ecio_f = -(value[1] / 2.0f);
            value[2] = getFieldValue(data, "neighbor_set[" + i + "]"
                    + ".pilot_phase");
            addData(value[0], ecio_f, value[2]);
        }

        Elog.d(TAG, "num_neighbor_set = " + num_neighbor_set);
    }
}

class EvdoFlInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_EVDO_FL_INFO_IND };

    public EvdoFlInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EVDO FL info";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[] { "c_i", "drcAverageValue", "ftcCrcErrorCount",
                "ftcTotalCount", "syncCrcErrorRatio" };
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        int c_i = getFieldValue(data, "c_i", true);
        int drcAverageValue = getFieldValue(data, "drcAverageValue");
        int ftcCrcErrorCount = getFieldValue(data, "ftcCrcErrorCount");
        int ftcTotalCount = getFieldValue(data, "ftcTotalCount");
        int syncCrcErrorRatio = getFieldValue(data, "syncCrcErrorRatio");

        DecimalFormat df = new DecimalFormat("#0.00");
        String c_iFloat = df.format((float) c_i / 64);

        Elog.d(TAG, "syncCrcErrorRatio = " + syncCrcErrorRatio);
        clearData();

        addData(c_iFloat);
        addData(drcAverageValue);
        addData(ftcCrcErrorCount);
        addData(ftcTotalCount);
        addData(syncCrcErrorRatio);
        notifyDataSetChanged();
    }
}

class EvdoRlInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_EVDO_RL_INFO_IND };

    public EvdoRlInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EVDO RL info";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[] { "averageTbsize", "rtcRetransmitCount",
                "rtcTransmitTotalCount", "txPower", "pilotPower", "rab_1_ratio" };
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        int averageTbsize = getFieldValue(data, "averageTbsize");
        int rtcRetransmitCount = getFieldValue(data, "rtcRetransmitCount");
        int rtcTransmitTotalCount = getFieldValue(data, "rtcTransmitTotalCount");
        int txPower = getFieldValue(data, "txPower", true);
        int pilotPower = getFieldValue(data, "pilotPower", true);
        int rab_1_ratio = getFieldValue(data, "rab_1_ratio");

        DecimalFormat df = new DecimalFormat("#0.00");
        String txPowerFloat = df.format((float) txPower / 128);
        String pilotPowerFloat = df.format((float) pilotPower / 128);

        clearData();

        addData(averageTbsize);
        addData(rtcRetransmitCount);
        addData(rtcTransmitTotalCount);
        addData(txPowerFloat);
        addData(pilotPowerFloat);
        addData(rab_1_ratio);

        notifyDataSetChanged();
    }
}

class EvdoStatueInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_EVDO_STATE_INFO_IND, };

    HashMap<Integer, String> SessionMapping = new HashMap<Integer, String>() {
        {
            put(0, "NEW_SESSION");
            put(1, "ALIVE_SESSION");
            put(2, "PRIOR_SESSION");
            put(3, "OPENED_SESSION");
        }
    };

    HashMap<Integer, String> AtMapping = new HashMap<Integer, String>() {
        {
            put(0, "AT_PWROFF");
            put(1, "AT_INACTIVE");
            put(2, "AT_PILOTACQ");
            put(3, "AT_SYNC");
            put(4, "AT_IDLE");
            put(5, "AT_ACCESS");
            put(6, "AT_CONNECTED");
        }
    };

    HashMap<Integer, String> ALMPMapping = new HashMap<Integer, String>() {
        {
            put(0, "ALMP_INIT_STATE");
            put(1, "ALMP_IDLE_STATE");
            put(2, "ALMP_CONN_SETUP_STATE");
            put(3, "ALMP_CONNECTED_STATE");
        }
    };

    HashMap<Integer, String> InspMapping = new HashMap<Integer, String>() {
        {
            put(0, "INSP_INACTIVE_STATE");
            put(1, "INSP_NETWORK_DET_STATE");
            put(2, "INSP_PILOT_ACQ_STATE");
            put(3, "INSP_SYNC_STATE");
            put(4, "INSP_TIMING_CHANGE_STATE");
            put(5, "INSP_WFR_1XASSTST_STATE");
        }
    };

    HashMap<Integer, String> IdpMapping = new HashMap<Integer, String>() {
        {
            put(0, "IDP_INACTIVE_ST");
            put(1, "IDP_MONITOR_ST");
            put(2, "IDP_SLEEP_ST");
            put(3, "IDP_CONN_SETUP_ST");
            put(4, "IDP_FREEZE_PENDING_ST");
            put(5, "IDP_FREEZE_ST");
            put(6, "IDP_CONN_FROZEN_ST");
            put(7, "IDP_STATE_MAX");
        }
    };
    HashMap<Integer, String> OmpMapping = new HashMap<Integer, String>() {
        {
            put(0, "OMP_INACTIVE_ST");
            put(1, "OMP_ACTIVE_ST");
            put(2, "OMP_STATE_MAX");
        }
    };
    HashMap<Integer, String> CspMapping = new HashMap<Integer, String>() {
        {
            put(0, "CSP_INACTIVE_STATE");
            put(1, "CSP_CLOSING_STATE");
            put(2, "CSP_OPEN_STATE");
        }
    };
    HashMap<Integer, String> RupMapping = new HashMap<Integer, String>() {
        {
            put(0, "RUP_INACTIVE");
            put(1, "RUP_IDLE");
            put(2, "RUP_CONNECTED");
            put(3, "RUP_IRAT_MEASUREMENT");
            put(4, "RUP_INVALID");
        }

    };

    public EvdoStatueInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EVDO Status info";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[] { "sessionState", "atState", "almpState",
                "inspState", "idpState", "ompState", "cspState", "rupState" };
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        int sessionState = getFieldValue(data, "sessionState");
        int atState = getFieldValue(data, "atState");
        int almpState = getFieldValue(data, "almpState");
        int inspState = getFieldValue(data, "inspState");
        int idpState = getFieldValue(data, "idpState");
        int ompState = getFieldValue(data, "ompState");
        int cspState = getFieldValue(data, "cspState");
        int rupState = getFieldValue(data, "rupState");

        Elog.d(TAG, "rupState = " + rupState);
        clearData();

        addData(SessionMapping.get(sessionState));
        addData(AtMapping.get(atState));
        addData(ALMPMapping.get(almpState));
        addData(InspMapping.get(inspState));
        addData(IdpMapping.get(idpState));
        addData(OmpMapping.get(ompState));
        addData(CspMapping.get(cspState));
        addData(RupMapping.get(rupState));
        notifyDataSetChanged();
    }
}

class EvdoSprintXRttInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_SPRINT_XRTT_INFO_IND };

    HashMap<Integer, String> AtMapping = new HashMap<Integer, String>() {
        {
            put(0, "NOSVC");
            put(1, "INIT");
            put(2, "IDLE");
            put(3, "TRAFFICINI");
            put(4, "TRAFFIC");
            put(5, "STATE NUM");
        }
    };

    public EvdoSprintXRttInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Sprint XRTT info";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[] { "State", "serviceOption", "rateReduction",
                "channel", "bandClass", "sid", "nid", "baseId",
                "pilotPNOffset", "mob_p_revp", "baseLat", "baseLong",
                "rxPower", "EcIo", "FER", "txPower" };
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        int State = getFieldValue(data, "State");
        int serviceOption = getFieldValue(data, "serviceOption");
        int rateReduction = getFieldValue(data, "rateReduction");
        int channel = getFieldValue(data, "channel");
        int bandClass = getFieldValue(data, "bandClass");
        int sid = getFieldValue(data, "sid");
        int nid = getFieldValue(data, "nid");
        int baseId = getFieldValue(data, "baseId");
        int pilotPNOffset = getFieldValue(data, "pilotPNOffset");
        int mob_p_revp = getFieldValue(data, "mob_p_revp");
        int baseLat = getFieldValue(data, "baseLat");
        int baseLong = getFieldValue(data, "baseLong");

        int rxPower = getFieldValue(data, "rxPower", true);
        int EcIo = getFieldValue(data, "EcIo", true);
        float ecio_f = (EcIo / 2.0f);

        int FER = getFieldValue(data, "FER", true);
        int txPower = getFieldValue(data, "txPower", true);

        clearData();

        addData(AtMapping.get(State));
        addData(serviceOption);
        addData(rateReduction);
        addData(channel);
        addData(bandClass);
        addData(sid);
        addData(nid);
        addData(baseId);
        addData(pilotPNOffset);
        addData(mob_p_revp);
        addData(baseLat);
        addData(baseLong);
        addData(rxPower);
        addData(ecio_f);
        addData(FER);
        addData(txPower);

        notifyDataSetChanged();
    }
}

class EvdoSprintInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_SPRINT_EVDO_INFO_IND };

    HashMap<Integer, String> AtMapping = new HashMap<Integer, String>() {
        {
            put(0, "INACTIVE");
            put(1, "ACQUISITION");
            put(2, "SYNC");
            put(3, "IDLE");
            put(4, "ACCESS");
            put(5, "CONNECTED");
            put(6, "STATE NUM");
        }

    };

    public EvdoSprintInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EVDO Sprint info";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[] { "DO_state", "Mac_index", "Channel", "Color_Code",
                "Sector_ID", "PN", "Rx_Pwr", "Rx_PER", "Pilot_Energy", "DRC",
                "SINR", "AN_AAA", "IPv4_Address", "IPv6_Address" };
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        int DO_state = getFieldValue(data, "DO_state");
        int Mac_index = getFieldValue(data, "Mac_index");
        int Channel = getFieldValue(data, "Channel");
        int Color_Code = getFieldValue(data, "Color_Code");

        char[] Sector_ID = new char[34];
        for (int i = 0; i < 34; i++) {
            Sector_ID[i] = (char) getFieldValue(data, "Sector_ID[" + i + "]");
        }
        String Sector_ID_s = new String(Sector_ID);

        int PN = getFieldValue(data, "bandPNClass");
        int Rx_Pwr = getFieldValue(data, "Rx_Pwr", true) / 128;
        int Rx_PER = getFieldValue(data, "Rx_PER");
        NumberFormat fmt = NumberFormat.getPercentInstance();
        fmt.setMaximumFractionDigits(2);
        String Rx_PER_DOUBLE = fmt.format((double) Rx_PER / 25600);

        int Pilot_Energy = getFieldValue(data, "Pilot_Energy", true);
        int DRC = getFieldValue(data, "DRC");
        int SINR = getFieldValue(data, "SINR", true);
        int AN_AAA = getFieldValue(data, "AN_AAA");

        char[] IPv4_Address = new char[16];
        for (int i = 0; i < 16; i++) {
            IPv4_Address[i] = (char) getFieldValue(data, "IPv4_Address[" + i
                    + "]");
        }
        String IPv4_Address_s = new String(IPv4_Address);

        char[] IPv6_Address = new char[64];
        for (int i = 0; i < 64; i++) {
            IPv6_Address[i] = (char) getFieldValue(data, "IPv6_Address[" + i
                    + "]");
        }
        String IPv6_Address_s = new String(IPv6_Address);

        Elog.d(TAG, "AN_AAA = " + AN_AAA);
        clearData();

        addData(AtMapping.get(DO_state));
        addData(Mac_index);
        addData(Channel);
        addData(Color_Code);

        addData(Sector_ID_s);

        addData(PN);
        addData(Rx_Pwr);
        addData(Rx_PER_DOUBLE);
        addData(Pilot_Energy);
        addData(DRC);
        addData(SINR);
        addData(AN_AAA);
        addData(IPv4_Address_s);
        addData(IPv6_Address_s);

        notifyDataSetChanged();
    }
}

class EvdoActiveSet extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_EVDO_ACTIVE_SET_INFO_IND };

    public EvdoActiveSet(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EVDO Acitve Set";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[] { "pilotPN", "pilotEclo", "DRC Cover" };
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        int num_evdo_active_set = getFieldValue(data, "num_evdo_active_set");
        addData("num_evdo_active_set(" + num_evdo_active_set + ")");
        for (int i = 0; i < num_evdo_active_set; i++) {
            int activePilotPN = getFieldValue(data, "evdo_active_set[" + i
                    + "]" + ".activePilotPN");
            int activepilotEclo = getFieldValue(data, "evdo_active_set[" + i
                    + "]" + ".activepilotEclo", true);

            DecimalFormat df = new DecimalFormat("#0.00");
            String activepilotEcloFloat = df
                    .format((float) activepilotEclo / 8);

            int activeDrcCover = getFieldValue(data, "evdo_active_set[" + i
                    + "]" + ".activeDrcCover");
            addData(activePilotPN, activepilotEcloFloat, activeDrcCover);
        }

    }
}

class EvdoCandSet extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_EVDO_CAND_SET_INFO_IND };

    public EvdoCandSet(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EVDO Cand Set";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[] { "Band", "Channel", "PilotPN", "PilotEclo" };
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        int num_evdo_cand_set = getFieldValue(data, "num_evdo_cand_set");
        addData("num_evdo_cand_set(" + num_evdo_cand_set + ")");
        for (int i = 0; i < num_evdo_cand_set; i++) {
            int Band = getFieldValue(data, "evdo_cand_set[" + i + "]" + ".Band");
            int Channel = getFieldValue(data, "evdo_cand_set[" + i + "]"
                    + ".Channel");
            int PilotPN = getFieldValue(data, "evdo_cand_set[" + i + "]"
                    + ".PilotPN");
            int PilotEclo = getFieldValue(data, "evdo_cand_set[" + i + "]"
                    + ".PilotEclo", true);
            DecimalFormat df = new DecimalFormat("#0.00");
            String PilotEcloFloat = df.format((float) PilotEclo / 8);
            addData(Band, Channel, PilotPN, PilotEcloFloat);
        }
    }
}

class EvdoNghdrSet extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_C2K_L4_EVDO_NGHDR_SET_INFO_IND };

    public EvdoNghdrSet(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EVDO Nghdr Set";
    }

    @Override
    String getGroup() {
        return "7. CDMA EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[] { "Band", "Channel", "PilotPN", "PilotEclo" };
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        int num_evdo_nghdr_set = getFieldValue(data, "num_evdo_nghdr_set");
        addData("num_evdo_nghdr_set(" + num_evdo_nghdr_set + ")");
        for (int i = 0; i < num_evdo_nghdr_set; i++) {
            int Band = getFieldValue(data, "evdo_nghdr_set[" + i + "]"
                    + ".Band");
            int Channel = getFieldValue(data, "evdo_nghdr_set[" + i + "]"
                    + ".Channel");
            int PilotPN = getFieldValue(data, "evdo_nghdr_set[" + i + "]"
                    + ".PilotPN");
            int PilotEclo = getFieldValue(data, "evdo_nghdr_set[" + i + "]"
                    + ".PilotEclo", true);

            DecimalFormat df = new DecimalFormat("#0.00");
            String PilotEcloFloat = df.format((float) PilotEclo / 8);
            addData(Band, Channel, PilotPN, PilotEcloFloat);
        }
    }
}