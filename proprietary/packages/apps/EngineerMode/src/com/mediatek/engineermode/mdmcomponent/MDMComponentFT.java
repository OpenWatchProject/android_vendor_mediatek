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
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.Toast;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.WorldModeUtil;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.mdml.Msg;
import com.mediatek.mdml.PlainDataDecoder;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;


class FTNetworkInfo extends CombinationViewComponent {
    private static final String[] EM_TYPES = new String[] {
        MDMContent.MSG_ID_EM_EMM_PLMNSEL_INFO_IND,
        MDMContent.MSG_ID_EM_EL2_OV_STATUS_IND,
        MDMContent.MSG_ID_EM_ERRC_SERVING_INFO_IND,
        MDMContent.MSG_ID_EM_EL1_STATUS_IND,
        MDMContent.MSG_ID_EM_ERRC_STATE_IND,
        MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_INTRARAT_INFO_IND,
        MDMContent.MSG_ID_EM_RRM_MEASUREMENT_REPORT_INFO_IND,
        MDMContent.MSG_ID_EM_GSM_TAS_INFO_IND,
        MDMContent.MSG_ID_EM_C2K_L4_EVDO_SERVING_INFO_IND,
        MDMContent.MSG_ID_EM_C2K_L4_RTT_RADIO_INFO_IND,
        MDMContent.MSG_ID_EM_MM_INFO_IND,
        MDMContent.MSG_ID_TDD_EM_CSCE_SERV_CELL_S_STATUS_IND,
        MDMContent.MSG_ID_EM_TDD_TAS_INFO_IND,
        MDMContent.MSG_ID_FDD_EM_CSCE_SERV_CELL_S_STATUS_IND,
        MDMContent.MSG_ID_FDD_EM_MEME_DCH_UMTS_CELL_INFO_IND,
        MDMContent.MSG_ID_FDD_EM_URR_3G_GENERAL_STATUS_IND,
        MDMContent.MSG_ID_FDD_EM_UL1_TAS_INFO_IND,
        MDMContent.MSG_ID_TDD_EM_URR_3G_GENERAL_STATUS_IND,
        MDMContent.MSG_ID_EM_RRM_LAI_INFO_IND,
        MDMContent.MSG_ID_EM_C2K_L4_SPRINT_EVDO_INFO_IND,
        MDMContent.MSG_ID_EM_C2K_L4_SPRINT_XRTT_INFO_IND,
        MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_INTERRAT_UTRAN_INFO_IND,
        MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_IND,
        MDMContent.MSG_ID_TDD_EM_MEME_DCH_LTE_CELL_INFO_IND,
        MDMContent.MSG_ID_TDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND,
        MDMContent.MSG_ID_TDD_EM_MEME_DCH_UMTS_CELL_INFO_IND,
        MDMContent.MSG_ID_TDD_EM_MEME_DCH_GSM_CELL_INFO_IND,
        MDMContent.MSG_ID_EM_RRM_IR_4G_NEIGHBOR_MEAS_STATUS_IND,
        MDMContent.MSG_ID_EM_RRM_IR_3G_NEIGHBOR_MEAS_STATUS_IND,
        MDMContent.MSG_ID_FDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND,
        MDMContent.MSG_ID_FDD_EM_MEME_DCH_LTE_CELL_INFO_IND,
        MDMContent.MSG_ID_FDD_EM_MEME_DCH_GSM_CELL_INFO_IND
    };
    public static final int NETWORK_TYPE_UNKNOWN = 0;
    public static final int NETWORK_TYPE_GPRS = 1;
    public static final int NETWORK_TYPE_EDGE = 2;
    public static final int NETWORK_TYPE_UMTS = 3;
    public static final int NETWORK_TYPE_CDMA = 4;
    public static final int NETWORK_TYPE_1xRTT = 7;
    public static final int NETWORK_TYPE_EVDO_0 = 5;
    public static final int NETWORK_TYPE_EVDO_A = 6;
    public static final int NETWORK_TYPE_HSDPA = 8;
    public static final int NETWORK_TYPE_HSUPA = 9;
    public static final int NETWORK_TYPE_HSPA = 10;
    public static final int NETWORK_TYPE_EVDO_B = 12;
    public static final int NETWORK_TYPE_EHRPD = 14;
    public static final int NETWORK_TYPE_LTE = 13;
    public static final int NETWORK_TYPE_HSPAP = 15;
    public static final int NETWORK_TYPE_GSM = 16;
    public static final int NETWORK_TYPE_TD_SCDMA = 17;
    public static final int NETWORK_TYPE_LTE_CA = 19;

    private static final String ACTION_SIM_STATE_CHANGED
            = "android.intent.action.SIM_STATE_CHANGED";
    private static final int SIM_STATE_UNKNOWN = 0;
    private static final int SIM_STATE_ABSENT = 1;
    private boolean mFirstBroadcast = true;
    private static final String INTENT_KEY_ICC_STATE = "ss";
    private LocationManager locationManager = null;
    private BroadcastReceiver SimCardChangedReceiver = null;
    String[] networkInfo = new String[] {"UnKnown", "UnKnown"};
    private String[] Labels;
    private ExecutorService executorService = Executors.newFixedThreadPool(10);

    int TasVersion = 1;
    private int oldDlMod0Pcell = 0, oldDlMod1Pcell = 0, oldUlMode = 0;

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

    HashMap<Integer, String> ERRCStateMapping = new HashMap<Integer, String>() {
        {
            put(1, "");
            put(2, "Idle");
            put(3, "Connected");
            put(4, "");
            put(5, "Idle");
            put(6, "Connected");
        }
    };

    HashMap<Integer, String> ServingBandMapping = new HashMap<Integer, String>() {
        {
            put(0, "Band 34");
            put(1, "Band 39");
            put(2, "-");
        }
    };

    private final Map<Integer, String> mRrStateMapping = new HashMap<Integer, String>() {
        {
            put(3, "Idle");
            put(5, "Packet Transfer");
            put(6, "Dedicated");
        }
    };

    HashMap<Integer, String> bandMapping = new HashMap<Integer, String>() {
        {
            put(0, "PGSM");
            put(1, "EGSM");
            put(2, "RGSM");
            put(3, "DCS1800");
            put(4, "PCS1900");
            put(5, "GSM450");
            put(6, "GSM480");
            put(7, "GSM850");
        }
    };

    private final HashMap<Integer, String> mStateMapping = new HashMap<Integer, String>() {

        {
            put(0, "Idle");
            put(1, "Cell FACH");
            put(2, "Cell PCH");
            put(3, "URA PCH");
            put(4, "Cell DCH");
            put(5, "INACTIVE");
            put(6, "NO_CHANGE");
            put(7, "Cell FACH");
            put(8, "Cell PCH");
            put(9, "Cell PCH");
            put(10, "URA PCH");
            put(11, "Cell FACH");
            put(12, "Cell PCH");
            put(13, "Cell PCH");
            put(14, "-");
        }
    };


    HashMap<Integer, String> mMappingQam = new HashMap<Integer, String>() {
        {
            put(1, "QPSK");
            put(2, "16QAM");
            put(3, "64QAM");
            put(4, "128QAM");
            put(5, "256QAM");
        }
    };

    HashMap<Integer, String> mMappingBW = new HashMap<Integer, String>() {
        {
            put(14, "1.4");
            put(30, "3");
            put(50, "5");
            put(100, "10");
            put(150, "15");
            put(200, "20");
            put(0xFF, "");
        }
    };

    HashMap<Integer, String> gsmBandMapping = new HashMap<Integer, String>() {
        {
            put(0, "DCS1800");
            put(1, "PCS1900");
            put(2, "-");
        }
    };

    private final HashMap<String, String[]> mKeyWordMapping = new HashMap<String, String[]>() {
        {
//            put("LTE-Scell", new String[]{"EARFCN", "PCI"});
            put("UMTS FDD", new String[]{"Cell ID", "UARFCN", "PSC"});
            put("UMTS TDD", new String[]{"Cell ID", "UARFCN", "PSC"});

        }
    };

    public FTNetworkInfo(Activity context, int simCount) {
        super(context, simCount);
    }

    public void registeListener() {
        initImsiList(getSupportSimCount());
        initLocation();
        startLocation();
        registSimReceiver(mContext);
    }

    public void unRegisteListener() {
        imsi = new String[getSupportSimCount()];
        if (SimCardChangedReceiver != null) {
            try{
                mContext.unregisterReceiver(SimCardChangedReceiver);
            } catch (Exception e) {
                Elog.e(TAG, "mContext.unregisterReceiver SimCardChangedReceiver : " + e.getMessage());
            } finally {
                SimCardChangedReceiver=null;
            }
        }
        if(locationManager != null) {
            try{
                locationManager.removeUpdates(mLocListener);
            } catch (Exception e) {
                Elog.e(TAG, "locationManager.removeUpdates : " + e.getMessage());
            } finally {
                locationManager=null;
            }
        }
    }

    void initImsiList(int simID) {
        TelephonyManager telephonyManager = (TelephonyManager) this.mActivity
                .getSystemService(this.mActivity.TELEPHONY_SERVICE);
        if(isValidSimID(simID)) {
            if(simID >= imsi.length) {
                String[] oldImsi = imsi;
                imsi = new String[simID];
                System.arraycopy(oldImsi, 0, imsi, 0, oldImsi.length);
            }
            String mImsi = !isSimCardAbsent(simID) ?
                    telephonyManager.getSubscriberId(simID) : "";
            imsi[simID] = mImsi != null ? mImsi : "";
        } else {
            imsi = null;
            imsi = new String[getSupportSimCount()];
            for(int i=0; i<getSupportSimCount(); i++) {
                String mImsi = !isSimCardAbsent(i) ?
                        telephonyManager.getSubscriberId(i) : "";
                imsi[i] = mImsi != null ? mImsi : "";
            }
        }
    }

    public boolean isSimCardAbsent(int simID) {
        TelephonyManager telephonyManager = (TelephonyManager) this.mActivity
                .getSystemService(this.mActivity.TELEPHONY_SERVICE);
        return telephonyManager.getSimState() == SIM_STATE_ABSENT ||
                telephonyManager.getSimState() == SIM_STATE_UNKNOWN;

    }
    public void stopGetLocation() {
        if(locationManager != null) {
            locationManager.removeUpdates(mLocListener);
            locationManager = null;
        }
    }

    public void startLocation() {
        locationManager = (LocationManager) mContext.getSystemService(Context.LOCATION_SERVICE);
        if (locationManager != null) {
            if (locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                locationManager.requestLocationUpdates(
                        LocationManager.GPS_PROVIDER, 0, 0, mLocListener);
            } if(locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) {
                locationManager.requestLocationUpdates(
                        LocationManager.NETWORK_PROVIDER, 0, 0, mLocListener);
            }
        }
    }

    public LocationListener mLocListener = new LocationListener() {

        @Override
        public void onLocationChanged(Location mLocation) {
            double latitude = mLocation.getLatitude();
            double longitude = mLocation.getLongitude();
            location = latitude+":"+longitude;
        }
        @Override
        public void onProviderDisabled(String provider) {
            Elog.v(TAG, "Enter onProviderDisabled function");
        }

        @Override
        public void onProviderEnabled(String provider) {
            Elog.v(TAG, "Enter onProviderEnabled function");
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            Elog.v(TAG, "Enter onStatusChanged function");
        }

    };

    public void initLocation() {
        try {
            locationManager = (LocationManager) mContext.getSystemService(Context.LOCATION_SERVICE);
            if (locationManager != null) {
                if (locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                    Location mLocation = locationManager
                            .getLastKnownLocation(LocationManager.GPS_PROVIDER);
                    if (mLocation != null) {
                        double latitude = mLocation.getLatitude();
                        double longitude = mLocation.getLongitude();
                        location = latitude+":"+longitude;
                    }
                } else if(locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) {
                    Location mLocation = locationManager
                            .getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
                    if (mLocation != null) {
                        double latitude = mLocation.getLatitude();
                        double longitude = mLocation.getLongitude();
                        location = latitude+":"+longitude;
                    }
                }
            } else {
                Elog.e(TAG, "[initLocation]new mLocationManager failed");
            }
        } catch (SecurityException e) {
            Toast.makeText(mContext, "security exception", Toast.LENGTH_LONG)
                    .show();
            Elog.e(TAG, "[initLocation]SecurityException: " + e.getMessage());
        } catch (IllegalArgumentException e) {
            Elog.e(TAG, "[initLocation]IllegalArgumentException: " + e.getMessage());
        } finally{
            locationManager = null;
        }
    }

    @Override
    String getName() {
        return "FT Network Info";
    }

    @Override
    String getGroup() {
        return "1. General EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    String[] initLabels() {
        String[] labelKeys = new String[] {"LTE-Pcell", "LTE-Tas Info", "LTE-Scell",
                "LTE-Neighbor Cell","LTE-Neighbor Cell.1","LTE-Neighbor Cell.2",
                "GSM-Serving Cell", "GSM-Tas Info", "GSM-Neighbor Cell",
                "GSM-Neighbor Cell.1","GSM-Neighbor Cell.2", "EVDO-Basic Info", "1xRTT-Basic Info",
                "1xRTT-Tas Info", "UMTS TDD-Serving cell", "UMTS TDD-Tas Info",
                "UMTS TDD-Neighbor Cell", "UMTS TDD-Neighbor Cell.1", "UMTS TDD-Neighbor Cell.2",
                "UMTS FDD-Serving cell", "UMTS FDD-Active Set", "UMTS FDD-Tas Info",
                "UMTS FDD-Neighbor Cell", "UMTS FDD-Neighbor Cell.1", "UMTS FDD-Neighbor Cell.2"};
        return labelKeys;
    }

    @Override
    ArrayList<LinkedHashMap> initHashMapValues() {
        ArrayList<LinkedHashMap> valuesHashMap = new ArrayList<LinkedHashMap>();
        valuesHashMap.add(initHashMap(new String[] {"PLMN", "TAC", "Cell ID", "EARFCN",
                "PCI", "Band", "DL_BW", "RSRP", "RSRQ", "SINR",
                "RSSI", "ERRC State", "Antenna", "RI", "TM", "DL Imcs",
                "DL Mod TB1", "DL Mod TB2", "UL Imcs", "UL Mod"}));
        valuesHashMap.add(initHashMap(new String[] { "TAS Enable", "TX Antenna",
                "TX Power", "RSRP_LANT", "RSRP_UANT"}));
        valuesHashMap.add(initHashMap(new String[] { "EARFCN", "PCI",
                "Band", "RSRP", "RSRQ", "SINR"}));
        valuesHashMap.add(initHashMap(new String[] {"LTE", "EARFCN", "PCI", "RSRP", "RSRQ"}));
        valuesHashMap.add(initHashMap(new String[] {"UMTS", "UARFCN", "PSC", "RSCP", "Ec/No"}));
        valuesHashMap.add(initHashMap(new String[] {"GSM", "ARFCN", "BSIC", "RSSI"}));
        valuesHashMap.add(initHashMap(new String[] { "PLMN", "LAC", "Cell ID",
                "ARFCN", "BSIC", "band", "RLA", "RX quality sub", "RR state"}));
        valuesHashMap.add(initHashMap(new String[] { "TAS Enable",
                "Antenna Index", "Current Antenna Power", "Other Antenna Power"}));
        valuesHashMap.add(initHashMap(new String[] {"LTE", "EARFCN", "PCI", "RSRP", "RSRQ"}));
        valuesHashMap.add(initHashMap(new String[] {"UMTS", "UARFCN", "PSC", "RSCP", "Ec/No"}));
        valuesHashMap.add(initHashMap(new String[] {"GSM", "ARFCN", "BSIC", "RSSI"}));
        valuesHashMap.add(initHashMap(new String[] { "Channel",
                "pilotPN", "band_class", "RSSI", "ecio"}));
        valuesHashMap.add(initHashMap(new String[] { "Channel",
                "pilotPN", "band_class", "RSSI", "ecio"}));
        valuesHashMap.add(initHashMap(new String[] { "TAS Enable",
                "tx_Ant", "rxPower_LANT", "rxPower_UANT"}));
        valuesHashMap.add(initHashMap(new String[] { "PLMN",
                "LAC", "Cell ID", "UARFCN", "PSC", "band", "RSCP",
                "RSSI", "RRC state"}));
        valuesHashMap.add(initHashMap(new String[] { "TAS Enable",
                "TX Antenna", "TX Power", "RSRP_LANT", "RSRP_UANT"}));
        valuesHashMap.add(initHashMap(new String[] {"LTE", "EARFCN", "PCI", "RSRP", "RSRQ"}));
        valuesHashMap.add(initHashMap(new String[] {"UMTS", "UARFCN", "PSC", "RSCP"}));
        valuesHashMap.add(initHashMap(new String[] {"GSM", "ARFCN", "BSIC", "RSSI"}));
        valuesHashMap.add(initHashMap(new String[] { "PLMN",
                "LAC", "Cell ID", "UARFCN", "PSC", "band", "RSCP", "Ec/No",
                "RSSI", "RRC state" }));
        valuesHashMap.add(initHashMap(new String[] { "UARFCN",
                "PSC", "RSCP", "Ec/No"}));
        valuesHashMap.add(initHashMap(new String[] { "TAS Enable",
                "TX Antenna", "TX Power", "RSRP_LANT", "RSRP_UANT", "RSRP_UANT(')"}));
        valuesHashMap.add(initHashMap(new String[] {"LTE", "EARFCN", "PCI", "RSRP", "RSRQ"}));
        valuesHashMap.add(initHashMap(new String[] {"UMTS", "UARFCN", "PSC", "RSCP", "Ec/No"}));
        valuesHashMap.add(initHashMap(new String[] {"GSM", "ARFCN", "BSIC", "RSSI"}));
        return valuesHashMap;
    }

    public ArrayList<String> getArrayTypeLabels(){
        ArrayList<String> arrayTypeKeys = new ArrayList<String>();
        arrayTypeKeys.add("LTE-Scell");
        arrayTypeKeys.add("LTE-Neighbor Cell");
        arrayTypeKeys.add("GSM-Neighbor Cell");
        arrayTypeKeys.add("UMTS FDD-Active Set");
        arrayTypeKeys.add("UMTS TDD-Neighbor Cell");
        arrayTypeKeys.add("UMTS FDD-Neighbor Cell");
        return arrayTypeKeys;
    }

    String rscpCheck(int value) {
        String value_s = String.valueOf(value);
        if (value == -255)
            value_s = " ";
        return value_s;
    }

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
            tasidx_s = TasEnableMapping.get(2);
        return tasidx_s;
    }

    String servingBandMapping(int bandidx) {
        String bandidx_s = "";
        if (bandidx >= 0 && bandidx <= 1) {
            bandidx_s = ServingBandMapping.get(bandidx);
        } else
            bandidx_s = ServingBandMapping.get(2) + "(" + bandidx + ")";
        return bandidx_s;
    }

    int getIndexOfEmTypes(String name) {
        for(int i=0; i<EM_TYPES.length; i++) {
            if (EM_TYPES[i].equals(name)) {
                return i;
            }
        }
        return -1;
    }

    String getBandMapping(int bandidx) {
        String bandidx_s = "";
        if (bandidx == 0 || bandidx == 1)
            bandidx_s = gsmBandMapping.get(bandidx);
        else
            bandidx_s = gsmBandMapping.get(2) + "(" + bandidx + ")";
        return bandidx_s;
    }


    void registSimReceiver(Context context) {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_SIM_STATE_CHANGED);
        SimCardChangedReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();

                if (mFirstBroadcast == true) {
                    mFirstBroadcast = false;
                    return;
                }

                if (action != null && action.equals(ACTION_SIM_STATE_CHANGED)) {
                    String newState = intent.getStringExtra(INTENT_KEY_ICC_STATE);
                    int changeSlot = intent.getIntExtra(PhoneConstants.SLOT_KEY, 0);
                    Elog.d(TAG, "SIM state change" + " changeSlot=" + changeSlot
                            + " new state =" + newState);
                    initImsiList(changeSlot);
                    updateCommonView(getCurrentSimID());
                }
            }
        };
        context.registerReceiver(SimCardChangedReceiver, intentFilter);
    }

    @Override
    void update( String name, Object msg) {
        Msg data = (Msg) msg;
        long curTime = System.currentTimeMillis();
        if(Labels == null) {
            Labels  = initLabels();
        }
        int simID = (data.getSimIdx() - 1) % 2 == 0 ?
                ModemCategory.getCapabilitySim() % 2 :
                    (ModemCategory.getCapabilitySim() + 1) % 2;
        Task task = new Task(UpdateTaskDriven.TASK_UPDATE_DATA, name, data, simID);
        mUpdateTaskDriven.appendTask(task);
    }

    @Override
    void startUpdateProcess(Task task) {
        new UpdateViewTask().execute(task);
    }

    public HashMap<String, Boolean> getUpdateMap(String[] Labels,
            int simID, String name, Object msg) {
         Msg data = (Msg) msg;
         String label = "";
         HashMap<String, Boolean> updateLabels=new HashMap<String, Boolean>();
         String coName = "";
         int arfcn, bsic, rssi, earfcn, uarfcn, rrState;
         String state;
         String[] indexArrays, arfcnArrays, pciArrays, rscpArrays,
             pscArrays, rsrpArrays, rsrqArrays, bsicArrays, rssiArrays;
         int cellId, pci, band, channel, band_class, bw;
         int antidx = 0;
         int rsrp_l_ant = 0;
         int rsrp_u_ant = 0;
         int rsrp_l_ant_a = 0;
         int tx_pwr = 0;
         int tasidx = 2;
         int cellNum = 0;
         int indexTotal = 0;
         int rat, rsrp, rsrq, sinr, iRssi0, iRssi1, iRssi;
         float rscp;
         String plmn = "";
         int emType = getIndexOfEmTypes(name);
         Elog.d(TAG, "Sim" + simID + ", update: " + emType + "," + name);
         initHashMap(simID);
         switch(emType){
             case 0:
                 label = Labels[0];
                 updateLabels.put(label, true);
                 coName = MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN + ".";
                 int mcc1 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MCC + 1);
                 int mcc2 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MCC + 2);
                 int mcc3 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MCC + 3);
                 int mnc1 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MNC + 1);
                 int mnc2 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MNC + 2);
                 int mnc3 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MNC + 3);
                 int tac = getFieldValue(data,
                         MDMContent.EM_EMM_L4C_PLMNSEL_PARA__MNC);
                 if (mcc1 == 0xF && mcc2 == 0xF && mcc3 == 0xF && mnc1 == 0xF
                         && mnc2 == 0xF && mnc3 == 0xF) {
                     setHashMapKeyValues(label, simID, "PLMN", "-");
                 } else {
                     setHashMapKeyValues(label, simID, "PLMN", "" + mcc1 + mcc2 + mcc3 + mnc1 + mnc2
                             + (mnc3 == 0xF ? "" : mnc3));
                 }
                 if (tac == 0xFFFE || tac == 0) {
                     setHashMapKeyValues(label, simID, "TAC", "-");
                 } else {
                     setHashMapKeyValues(label, simID, "TAC", tac);
                 }
                 break;
             case 1:
                 break;
             case 2:
                 label = Labels[0];
                 updateLabels.put(label, true);
                 earfcn = getFieldValue(data, "serv_inf.dl_earfcn");
                 pci = getFieldValue(data, "serv_inf.pci");
                 band = getFieldValue(data, "serv_inf.band");
                 cellId = getFieldValue(data, "serv_inf.cell_id");
                 setHashMapKeyValues(label, simID, "Cell ID", cellId);
                 setHashMapKeyValues(label, simID, "EARFCN", earfcn == 0xFFFFFFFF ? "" : earfcn);
                 setHashMapKeyValues(label, simID, "PCI", earfcn == 0xFFFFFFFF ? "" : pci);
                 setHashMapKeyValues(label, simID, "Band", band == 0xFFFF ? "" : "Band" + band);
                 break;
             case 3:
                 label = Labels[0];
                 updateLabels.put(label, true);
                 bw = getFieldValue(data, MDMContent.EM_EL1_STATUS_CELL_INFO + "[0]."
                         + MDMContent.EM_EL1_STATUS_CELL_INFO_DL_BW);
                 setHashMapKeyValues(label, simID, "DL_BW",
                         mMappingBW.containsKey(bw) ? mMappingBW.get(bw) : "");

                 coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
                 rsrp = getFieldValue(data, coName
                         + MDMContent.EM_EL1_STATUS_DL_INFO_RSRP, true);
                 setHashMapKeyValues(label, simID, "RSRP", rsrp+"");

                 rsrq = getFieldValue(data, coName
                         + MDMContent.EM_EL1_STATUS_DL_INFO_RSRQ, true);
                 setHashMapKeyValues(label, simID, "RSRQ", rsrq+"");

                 sinr = getFieldValue(data, coName
                         + MDMContent.EM_EL1_STATUS_DL_INFO_SINR, true);
                 setHashMapKeyValues(label, simID, "SINR", sinr +"");

                 iRssi0 = getFieldValue(data, coName
                         + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RSSI + "[0]", true);
                 iRssi1 = getFieldValue(data, coName
                         + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RSSI + "[1]", true);
                 setHashMapKeyValues(label, simID, "RSSI", iRssi0+","+iRssi1);

                 int ant_port = getFieldValue(data, MDMContent.EM_EL1_STATUS_CELL_INFO + "[0]."
                         + MDMContent.EM_EL1_STATUS_CELL_INFO_ANT_PORT);
                 int rssi_num = 0;
                 for(int i=0; i<2; i++) {
                     int dl_rssi = getFieldValue(data, coName
                             + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RSSI + "["+i+"]", true);
                     if(dl_rssi <= 18 || dl_rssi >= -140) {
                         rssi_num ++;
                     }
                 }
                 setHashMapKeyValues(label, simID, "Antenna", ant_port+"x"+rssi_num);

                 int ri = getFieldValue(data, coName
                         + MDMContent.EM_EL1_STATUS_DL_INFO_RI, true);
                 setHashMapKeyValues(label, simID, "RI", ri+"");

                 int tm = getFieldValue(data, coName
                         + MDMContent.EM_EL1_STATUS_DL_INFO_TM, true);
                 setHashMapKeyValues(label, simID, "TM", "TM"+tm);

                 int dl_imcs = getFieldValue(data, coName
                         + MDMContent.EM_EL1_STATUS_DL_INFO_DL_IMCS, true);
                 setHashMapKeyValues(label, simID, "DL Imcs", ""+dl_imcs);

                 int dlMod0Pcell = getFieldValue(data, coName
                         + MDMContent.EM_EL1_STATUS_DL_INFO_DL_MOD0);
                 oldDlMod0Pcell = (dlMod0Pcell == 0xFF) ? oldDlMod0Pcell
                         : dlMod0Pcell;
                 setHashMapKeyValues(label, simID, "DL Mod TB1",
                         mMappingQam.containsKey(oldDlMod0Pcell) ?
                                 mMappingQam.get(oldDlMod0Pcell) : "");

                 int dlMod1Pcell = getFieldValue(data, coName
                         + MDMContent.EM_EL1_STATUS_DL_INFO_DL_MOD1);
                 oldDlMod1Pcell = (dlMod1Pcell == 0xFF) ? oldDlMod1Pcell
                         : dlMod1Pcell;
                 setHashMapKeyValues(label, simID, "DL Mod TB2",
                        mMappingQam.containsKey(oldDlMod1Pcell) ?
                                mMappingQam.get(oldDlMod1Pcell) : "");

                 int ul_imcs = getFieldValue(data,  MDMContent.EM_EL1_STATUS_UL_INFO + "[0]." +
                         MDMContent.EM_EL1_STATUS_UL_INFO_UL_IMCS, true);
                 setHashMapKeyValues(label, simID, "UL Imcs", ""+ul_imcs);

                 int ul_mode = getFieldValue(data,  MDMContent.EM_EL1_STATUS_UL_INFO + "[0]." +
                         MDMContent.EM_EL1_STATUS_UL_INFO_UL_MOD, true);
                 oldUlMode = (ul_mode == 0xFF) ? oldUlMode : ul_mode;
                 setHashMapKeyValues(label, simID, "UL Mod",
                         mMappingQam.containsKey(oldUlMode) ? mMappingQam.get(oldUlMode) : "");

                 label = Labels[1];
                 updateLabels.put(label, true);
                 int dl_cc_count = getFieldValue(data, MDMContent.DL_CC_COUNT);
                 int ul_cc_count = getFieldValue(data, MDMContent.UL_CC_COUNT);
                 TasVersion = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_VERISION);
                 TasVersion = (TasVersion == 0) ? 1 : 2;

                 antidx = getFieldValue(data, "ul_info[0].tx_ant_type", true);
                 rsrp_l_ant = getFieldValue(data, "ul_info[0].rsrp_l_ant", true);
                 rsrp_u_ant = getFieldValue(data, "ul_info[0].rsrp_u_ant", true);
                 tx_pwr = getFieldValue(data, "ul_info[0].tx_power", true);

                 tasidx = FeatureSupport.is93Modem() ?
                         getFieldValue(data, "ul_info[0].tas_status") : 2;
                 if (dl_cc_count == 0) {
                     TasVersion = 1;
                     setHashMapKeyValues(label, simID,
                             new String[]{ tasEableMapping(tasidx),
                             "", "", "", "" });
                 } else if (dl_cc_count == 1 && ul_cc_count == 0) {
                     setHashMapKeyValues(label, simID,
                             new String[]{ tasEableMapping(tasidx), "", "",
                             rscpCheck(rsrp_l_ant),
                             rscpCheck(rsrp_u_ant) });
                 } else if (dl_cc_count >= 1 && ul_cc_count >= 1) {
                     setHashMapKeyValues(label, simID,
                             new String[]{ tasEableMapping(tasidx),
                             antidxMapping(antidx),
                             tx_pwr + "",
                             rscpCheck(rsrp_l_ant),
                             rscpCheck(rsrp_u_ant) });
                 }
                 break;
             case 4:
                 label = Labels[0];
                 updateLabels.put(label, true);
                 state = ERRCStateMapping.get(getFieldValue(data,
                         MDMContent.EM_ERRC_STATE_ERRC_STS));
                 setHashMapKeyValues(label, simID, "ERRC State", state);
                 break;
             case 5:
                 label = Labels[2];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 coName = MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_LIST
                         + "." + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO;
                 cellNum = getFieldValue(data,
                         MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_LIST +
                         "." + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_NUM_SCELL);
                 for(int i=0; i<cellNum; i++) {
                     earfcn = getFieldValue(data,
                         coName+"["+i+"]." +
                             MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_EARFCN);
                     pci = getFieldValue(data,
                         coName+"["+i+"]." +
                             MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_PCI);
                     rsrp = getFieldValue(data,
                         coName+"["+i+"]." +
                             MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_RSRP, true);
                     rsrq = getFieldValue(data,
                         coName+"["+i+"]." +
                     MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_RSRQ, true);
                     sinr = getFieldValue(data,
                         coName+"["+i+"]." +
                             MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_RS_SNR_IN_QDB, true);
                     band = getFieldValue(data,
                         coName+"["+i+"]." +
                             MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_SERV_LTE_BAND);
                     if(earfcn != 0 && pci !=0 && earfcn != 0xFFFFFFFF) {
                         addHashMapKeyValues(label, simID, "EARFCN",
                                 (earfcn == 0xFFFFFFFF ? "" : earfcn));
                         addHashMapKeyValues(label, simID, "PCI",
                                 (earfcn == 0xFFFFFFFF ? "" : pci));
                         addHashMapKeyValues(label, simID, "Band",
                                 band == 0xFFFF ? "" : band);
                         addHashMapKeyValues(label, simID, "RSRP",
                                 rsrp == 0xFFFFFFFF ? "" : (float) rsrp / 4);
                         addHashMapKeyValues(label, simID, "RSRQ",
                                 rsrq == 0xFFFFFFFF ? "" : (float) rsrq / 4);
                         addHashMapKeyValues(label, simID, "SINR",
                                 sinr == 0xFFFFFFFF ? "" : (float) sinr / 4);
                     }
                 }
                 label = Labels[3];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 coName = MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO + ".";
                 int freqNum = getFieldValue(data, coName +
                     MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_FREQ_NUM);
                 indexTotal = 0;
                 for (int i = 0; i < freqNum && i < 4; i++) {
                     String coNameNew = coName +
                         MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_INTER_FREQ + "[" + i + "].";
                     int valid = getFieldValue(data,
                         coNameNew + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_VALID);
                     earfcn = getFieldValue(data, coNameNew +
                         MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_INTER_CELL_EARFCN);
                     cellNum = getFieldValue(data, coNameNew +
                         MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_CELL_NUM);
                     indexArrays = new String[cellNum < 6 ? cellNum : 6];
                     pciArrays = new String[cellNum < 6 ? cellNum : 6];
                     rsrpArrays = new String[cellNum < 6 ? cellNum : 6];
                     rsrqArrays = new String[cellNum < 6 ? cellNum : 6];
                     arfcnArrays = new String[cellNum < 6 ? cellNum : 6];
                     for (int j = 0; j < cellNum && j < 6; j++) {
                         String interName = coNameNew
                             + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_INTER_CELL
                                 + "[" + j + "].";
                         pci = getFieldValue(data, interName +
                             MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_INTER_CELL_PCI);
                         rsrp = getFieldValue(data, interName +
                             MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_INTER_CELL_RSRP, true);
                         rsrq = getFieldValue(data, interName +
                             MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_INTER_CELL_RSRQ, true);
                         indexArrays[j] = "" + indexTotal;
                         if(valid > 0) {
                             arfcnArrays[j] = "" + earfcn;
                             pciArrays[j] = "" + pci;
                             rsrpArrays[j] = "" + (rsrp != 0xFFFFFFFF ? (float) rsrp / 4  : "");
                             rsrqArrays[j] = "" + (rsrq != 0xFFFFFFFF ? (float) rsrq / 4 : "");
                         } else {
                             indexArrays[i] = arfcnArrays[i] =
                                     pciArrays[i] = rsrpArrays[i] = rsrqArrays[i] = "";
                         }
                         indexTotal ++;
                     }
                     addHashMapKeyValues(label, simID, "LTE", indexArrays);
                     addHashMapKeyValues(label, simID, "EARFCN", arfcnArrays);
                     addHashMapKeyValues(label, simID, "PCI", pciArrays);
                     addHashMapKeyValues(label, simID, "RSRP", rsrpArrays);
                     addHashMapKeyValues(label, simID, "RSRQ", rsrqArrays);
                 }
                 coName = MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO + ".";
                 cellNum = getFieldValue(data, coName
                         + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO_CELL_NUM);
                 indexArrays = new String[cellNum < 16 ? cellNum : 16];
                 arfcnArrays = new String[cellNum < 16 ? cellNum : 16];
                 pciArrays = new String[cellNum < 16 ? cellNum : 16];
                 rsrpArrays = new String[cellNum < 16 ? cellNum : 16];
                 rsrqArrays = new String[cellNum < 16 ? cellNum : 16];
                 earfcn = getFieldValue(data, MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SERVING_INFO
                         + "." + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_EARFCN);
                 for (int i = 0; i < cellNum && i < 16; i++) {
                     String coNameNew = coName
                             + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO_INTRA_CELL
                             + "[" + i + "].";
                     int valid = getFieldValue(data, coNameNew
                             + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO_VALID);
                     pci = getFieldValue(data, coNameNew
                             + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO_PCI);
                     rsrp = getFieldValue(data, coNameNew
                             + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO_RSRP,
                             true);
                     rsrq = getFieldValue(data, coNameNew
                             + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO_RSRQ,
                             true);
                     indexArrays[i] = "" + indexTotal;
                     if(valid > 0) {
                         arfcnArrays[i] = "" + earfcn;
                         pciArrays[i] = "" + pci;
                         rsrpArrays[i] = "" + (rsrp != 0xFFFFFFFF ? (float) rsrp / 4 : "");
                         rsrqArrays[i] = "" + (rsrq != 0xFFFFFFFF ? (float) rsrq / 4 : "");
                     } else {
                         indexArrays[i] = arfcnArrays[i] =
                                 pciArrays[i] = rsrpArrays[i] = rsrqArrays[i] = "";
                     }
                     indexTotal ++;
                 }
                 addHashMapKeyValues(label, simID, "LTE", indexArrays);
                 addHashMapKeyValues(label, simID, "EARFCN", arfcnArrays);
                 addHashMapKeyValues(label, simID, "PCI", pciArrays);
                 addHashMapKeyValues(label, simID, "RSRP", rsrpArrays);
                 addHashMapKeyValues(label, simID, "RSRQ", rsrqArrays);
                 break;
             case 6:
                 label = Labels[6];
                 updateLabels.put(label, true);
                 coName = MDMContent.RR_EM_MEASUREMENT_REPORT_INFO + ".";
                 rrState = getFieldValue(data, coName
                         + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_RR_STATE);
                 if (rrState >= 3 && rrState <= 7) {
                     band = getFieldValue(data, coName +
                             MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERVING_CURRENT_BAND);
                     arfcn = getFieldValue(data, coName +
                             MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERVING_ARFCN);
                     bsic = getFieldValue(data, coName +
                             MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERVING_BSIC);
                     int rla = getFieldValue(
                             data, coName +
                             MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERV_RLA_IN_QUARTER_DBM,
                             true);
                     int rxSub = getFieldValue(data, coName +
                             MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERV_RXQUAL_SUB);
                     state = mRrStateMapping.get(getFieldValue(data,
                             MDMContent.RR_EM_MEASUREMENT_REPORT_INFO + "." +
                             MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_RR_STATE));

                     setHashMapKeyValues(label, simID, "RLA", rla == -1000 ? "-" :
                         String.format("%.2f", (float) rla / 4));
                     setHashMapKeyValues(label, simID, "RX quality sub",
                             rxSub == 0xFF ? "-" : rxSub);
                     setHashMapKeyValues(label, simID, "ARFCN", arfcn);
                     setHashMapKeyValues(label, simID, "BSIC", bsic);
                     setHashMapKeyValues(label, simID, "band", bandMapping.get(band));
                     setHashMapKeyValues(label, simID, "RR state", state);
                 }
                 label = Labels[10];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 coName = MDMContent.RR_EM_MEASUREMENT_REPORT_INFO + ".";
                 cellNum = getFieldValue(data, coName +
                         MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_NUM_OF_CARRIERS);
                 for(int i=0; i<cellNum && i<32; i++) {
                     arfcn = getFieldValue(data, coName +
                             MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_NC_ARFCN
                             + "[" + i + "]");
                     bsic = getFieldValue(data, coName +
                             MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_NC_BSIC
                             + "[" + i + "]");
                     rssi = getFieldValue(data, coName +
                             MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_RLA_IN_QUARTER_DBM
                             + "[" + i + "]", true);
                     addHashMapKeyValues(label, simID, "GSM", i + "");
                     addHashMapKeyValues(label, simID, "ARFCN", arfcn + "");
                     addHashMapKeyValues(label, simID, "BSIC", bsic + "");
                     addHashMapKeyValues(label, simID, "RSSI", (float)rssi / 4.0 + "");
                 }
                 break;
             case 7:
                 label = Labels[7];
                 updateLabels.put(label, true);
                 tasidx = FeatureSupport.is93Modem() ?
                         getFieldValue(data, MDMContent.GSM_TAS_ENABLE) : 2;
                 antidx = getFieldValue(data, MDMContent.GSM_ANTENNA);
                 int currentAntRxLevel = getFieldValue(data,
                         MDMContent.GSM_CURRENT_ANTENNA_RXLEVEL, true);
                 int otherAntRxLevel = getFieldValue(data,
                         MDMContent.GSM_OTHER_ANTENNA_RXLEVEL, true);
                 setHashMapKeyValues(label, simID, new String[]{ tasEableMapping(tasidx),
                         antidxMapping(antidx),
                         currentAntRxLevel+"",
                         otherAntRxLevel+"" });
                 break;
             case 8:
                 label = Labels[11];
                 updateLabels.put(label, true);
                 int rssi_dbm = getFieldValue(data, "rssi_dbm", true) / 128;
                 band_class = getFieldValue(data, "bandClass");
                 channel = getFieldValue(data, "channel");
                 int pilotPN = getFieldValue(data, "pilotPN");
                 setHashMapKeyValues(label, simID, "Channel", channel);
                 setHashMapKeyValues(label, simID, "pilotPN", pilotPN);
                 setHashMapKeyValues(label, simID, "band_class", band_class);
                 setHashMapKeyValues(label, simID, "RSSI", rssi_dbm);
                 break;
             case 9:
                 label = Labels[12];
                 updateLabels.put(label, true);
                 channel = getFieldValue(data, "channel");
                 band_class = getFieldValue(data, "band_class");
                 int pilot_pn_offset = getFieldValue(data, "pilot_pn_offset");
                 iRssi = getFieldValue(data, "rx_power", true);
                 setHashMapKeyValues(label, simID, "RSSI", iRssi);
                 setHashMapKeyValues(label, simID, "Channel", channel);
                 setHashMapKeyValues(label, simID, "pilotPN", pilot_pn_offset);
                 setHashMapKeyValues(label, simID, "band_class", band_class);
                 label = Labels[13];
                 updateLabels.put(label, true);
                 tasidx = FeatureSupport.is93Modem() ? getFieldValue(data, "tas_enable") : 2;
                 int tx_ant_id = getFieldValue(data, "tx_ant", true);
                 int rx_power_dbmL = getFieldValue(data, "rx_power_dbmL", true);
                 int rx_power_dbmU = getFieldValue(data, "rx_power_dbmU", true);
                 int rx_power_dbmLp = getFieldValue(data, "rx_power_dbmLp", true);
                 int tas_ver = getFieldValue(data, "tas_ver", true);

                 if (tas_ver != 1) {
                     setHashMapKeyValues(label, simID, "rxPower_LANT",
                             (rx_power_dbmL == -150) ? "" : rx_power_dbmL);
                     setHashMapKeyValues(label, simID, "rxPower_UANT",
                             (rx_power_dbmU == -150) ? "" : rx_power_dbmU);
                     setHashMapKeyValues(label, simID, "rxPower_UANT(')",
                             (rx_power_dbmLp == -150) ? "" : rx_power_dbmLp);
                 }
                 setHashMapKeyValues(label, simID, "TAS Enable", tasEableMapping(tasidx));
                 setHashMapKeyValues(label, simID, "tx_Ant", tx_ant_id);
                 break;
             case 10:
                 label = Labels[14];
                 updateLabels.put(label, true);
                 mcc1 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MCC + "[0]");
                 mcc2 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MCC + "[1]");
                 mcc3 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MCC + "[2]");
                 mnc1 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MNC + "[0]");
                 mnc2 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MNC + "[1]");
                 mnc3 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MNC + "[2]");
                 if (mcc1 == 0xF && mcc2 == 0xF && mcc3 == 0xF && mnc1 == 0xF
                         && mnc2 == 0xF && mnc3 == 0xF) {
                     setHashMapKeyValues(label, simID, "PLMN", "-");
                 } else {
                     setHashMapKeyValues(label, simID, "PLMN",
                         "" + mcc1 + mcc2 + mcc3 + mnc1 + mnc2 + (mnc3 == 0xF ? "" : mnc3));
                 }
                 String loc0 = String.format("%d",
                         getFieldValue(data, MDMContent.EM_MM_LOC + "[0]"));
                 String loc1 = String.format("%d",
                         getFieldValue(data, MDMContent.EM_MM_LOC + "[1]"));
                 setHashMapKeyValues(label, simID, "LAC", loc0 + loc1);
                 label = Labels[19];
                 updateLabels.put(label, true);
                 setHashMapKeyValues(label, simID, "LAC", loc0 + loc1);
                 break;
             case 11:
                 label = Labels[14];
                 updateLabels.put(label, true);
                 coName = MDMContent.EM_CSCE_SERV_CELL + ".";
                 uarfcn = getFieldValue(data, coName
                         + MDMContent.EM_CSCE_SERV_CELL_UARFCN_DL);
                 int cellParaId = getFieldValue(data, coName
                         + MDMContent.EM_CSCE_SERV_CELL_PSC);
                 rscp = (float) getFieldValue(data, coName
                         + MDMContent.EM_CSCE_SERV_CELL_RSCP, true);
                 cellId = getFieldValue(data, coName
                         + MDMContent.CELL_IDENTITY);
                 iRssi = getFieldValue(data, coName
                         + MDMContent.RSSI);
                 setHashMapKeyValues(label, simID, "PSC", cellParaId);
                 setHashMapKeyValues(label, simID, "UARFCN", uarfcn);
                 setHashMapKeyValues(label, simID, "RSCP", (float) rscp / 4096);
                 setHashMapKeyValues(label, simID, "Cell ID", cellId);
                 setHashMapKeyValues(label, simID, "RSSI", iRssi);
                 label = Labels[17];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 setHashMapKeyValues(label, simID, "UMTS", "0");
                 setHashMapKeyValues(label, simID, "UARFCN", uarfcn);
                 setHashMapKeyValues(label, simID, "PSC", cellParaId);
                 setHashMapKeyValues(label, simID, "RSCP", (float) rscp / 4096);
                 break;
             case 12:
                 label = Labels[14];
                 updateLabels.put(label, true);
                 band = FeatureSupport.is93Modem() ?
                         getFieldValue(data, MDMContent.TDD_SERVING_BAND) : 2;
                 setHashMapKeyValues(label, simID, "band", servingBandMapping(band));
                 label = Labels[15];
                 updateLabels.put(label, true);
                 tasidx = FeatureSupport.is93Modem() ?
                         getFieldValue(data, MDMContent.TDD_TAS_ENABLE) : 2;
                 tx_pwr = getFieldValue(data, MDMContent.TDD_TX_PWR, true);
                 int force_ant_idx = getFieldValue(data, MDMContent.TDD_FORCE_ANT_IDX);
                 int ant0_rscp = getFieldValue(data, MDMContent.TDD_ANT0_RSCP, true);
                 int ant1_rscp = getFieldValue(data, MDMContent.TDD_ANT1_RSCP, true);
                 setHashMapKeyValues(label, simID, "TAS Enable", tasEableMapping(tasidx));
                 setHashMapKeyValues(label, simID, "TX Antenna", antidxMapping(force_ant_idx));
                 setHashMapKeyValues(label, simID, "TX Power", tx_pwr);
                 setHashMapKeyValues(label, simID, "RSRP_LANT", ant0_rscp);
                 setHashMapKeyValues(label, simID, "RSRP_UANT", ant1_rscp);
                 break;
             case 13:
                 label = Labels[19];
                 updateLabels.put(label, true);
                 coName = MDMContent.FDD_EM_CSCE_SERV_CELL + ".";
                 int count = getFieldValue(data, coName
                         + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_COUNT);
                 for (int i = 0; i < count && i < 6; i++) {
                     String secName = coName
                             + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID + "[" + i
                             + "].";
                     plmn = "";
                     mcc1 = getFieldValue(data, secName
                             + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID_MCC + 1);
                     mcc2 = getFieldValue(data, secName
                             + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID_MCC + 2);
                     mcc3 = getFieldValue(data, secName
                             + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID_MCC + 3);
                     mnc1 = getFieldValue(data, secName
                             + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID_MNC + 1);
                     mnc2 = getFieldValue(data, secName
                             + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID_MNC + 2);
                     mnc3 = getFieldValue(data, secName
                             + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID_MNC + 3);
                     if (mcc1 == 0xF && mcc2 == 0xF && mcc3 == 0xF && mnc1 == 0xF
                             && mnc2 == 0xF && mnc3 == 0xF) {
                         setHashMapKeyValues(label, simID, "PLMN", "-");
                     } else {
                         setHashMapKeyValues(label, simID, "PLMN", "" +
                             mcc1 + mcc2 + mcc3 + mnc1 + mnc2 + (mnc3 == 0xF ? "" : mnc3));
                     }
                 }
                 coName = MDMContent.FDD_EM_CSCE_SERV_CELL + ".";
                 uarfcn = getFieldValue(data, coName
                         + MDMContent.FDD_EM_CSCE_SERV_CELL_UARFCN_DL);
                 int psc = getFieldValue(data, coName
                         + MDMContent.FDD_EM_CSCE_SERV_CELL_PSC);
                 rscp = (float) getFieldValue(data, coName
                         + MDMContent.FDD_EM_CSCE_SERV_CELL_RSCP, true) / 4096;
                 float ecn0 = (float) getFieldValue(data, coName
                         + MDMContent.FDD_EM_CSCE_SERV_CELL_EC_N0, true) / 4096;
                 if (rscp > -120 && ecn0 > -25) {
                     setHashMapKeyValues(label, simID, "UARFCN", uarfcn);
                     setHashMapKeyValues(label, simID, "PSC", psc);
                     setHashMapKeyValues(label, simID, "RSCP", rscp);
                     setHashMapKeyValues(label, simID, "Ec/No", ecn0);
                 }
                 cellId = getFieldValue(data, coName + MDMContent.CELL_IDENTITY);
                 setHashMapKeyValues(label, simID, "Cell ID", cellId);
                 iRssi = getFieldValue(data, coName + MDMContent.RSSI);
                 setHashMapKeyValues(label, simID, "RSSI", iRssi);
                 break;
             case 14:
                 label = Labels[20];
                 updateLabels.put(label, true);
                 coName = MDMContent.FDD_EM_MEME_DCH_UMTS_UMTS_CELL_LIST + "[";
                 cellNum = getFieldValue(data, MDMContent.FDD_EM_MEME_DCH_UMTS_NUM_CELLS);
                 resetHashMapKeyValues(label, simID);
                 String[][] values = new String[4][cellNum < 32 ? cellNum : 32];
                 for (int i = 0; i < cellNum && i < 32; i++) {
                     uarfcn = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_UMTS_UARFCN);
                     psc = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_UMTS_PSC);
                     rscp = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_UMTS_RSCP, true);
                     ecn0 = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_UMTS_ECN0, true);
                     values[0][i] = uarfcn+"";
                     values[1][i] = psc+"";
                     values[2][i] = rscp+"";
                     values[3][i] = ecn0+"";
                 }
                 setHashMapKeyValues(label, simID, "UARFCN", values[0]);
                 setHashMapKeyValues(label, simID, "PSC", values[1]);
                 setHashMapKeyValues(label, simID, "RSCP", values[2]);
                 setHashMapKeyValues(label, simID, "Ec/No", values[3]);
                 label = Labels[23];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 cellNum = getFieldValue(data,
                         MDMContent.FDD_EM_MEME_DCH_UMTS_NUM_CELLS);
                 coName = MDMContent.FDD_EM_MEME_DCH_UMTS_UMTS_CELL_LIST
                         + "[";
                 for (int i = 0; i < cellNum && i < 32; i++) {
                     int cellType = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_UMTS_CELL_TYPE);
                     uarfcn = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_UMTS_UARFCN);
                     psc = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_UMTS_PSC);
                     rscp = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_UMTS_RSCP, true);
                     ecn0 = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_UMTS_ECN0, true);
                     addHashMapKeyValues(label, simID, "UMTS", "" + i);
                     addHashMapKeyValues(label, simID, "UARFCN", "" + uarfcn);
                     addHashMapKeyValues(label, simID, "PSC", "" + psc);
                     addHashMapKeyValues(label, simID, "RSCP", "" + rscp);
                     addHashMapKeyValues(label, simID, "Ec/No", "" + ecn0);
                 }
                 break;
             case 15:
                 label = Labels[19];
                 updateLabels.put(label, true);
                 coName = MDMContent.TDD_EM_URR_3G_GENERAL_UMTS_UAS_3G_GENERAL_STATUS
                 + ".";
                 rrState = getFieldValue(data, coName
                         + MDMContent.TDD_EM_URR_3G_GENERAL_UMTS_RRC_STATE);
                 if (rrState != 6) {
                     state = mStateMapping.get(rrState);
                     setHashMapKeyValues(label, simID, "RRC state", state);
                 }
                 break;
             case 16:
                 label = Labels[21];
                 updateLabels.put(label, true);
                 coName = MDMContent.FDD_EM_UL1_TAS_EMUL1TAS + ".";
                 tasidx = FeatureSupport.is93Modem() ? getFieldValue(data, coName
                         + MDMContent.FDD_EM_UL1_TAS_ENABLE) : 2;
                 band_class = FeatureSupport.is93Modem() ? getFieldValue(data, coName
                         + MDMContent.FDD_EM_UL1_TAS_BAND) : -1;
                 antidx = getFieldValue(data, coName
                                 + MDMContent.FDD_EM_UL1_TAS_MAIN_ANT_IDX);
                 tx_pwr = getFieldValue(data, coName
                                 + MDMContent.FDD_EM_UL1_TAS_TX_PWR, true);
                 String tx_pwr_s, rscp_0_s, rscp_1_s;
                 int rscp_0 = getFieldValue(data, coName
                                 + MDMContent.FDD_EM_UL1_TAS_RSCP0, true);
                 int rscp_1 = getFieldValue(data, coName
                                 + MDMContent.FDD_EM_UL1_TAS_RSCP1, true);
                 int rscp_2 = getFieldValue(data, coName
                                 + MDMContent.FDD_EM_UL1_TAS_RSCP2, true);
                 TasVersion = (getFieldValue(data, coName
                                 + MDMContent.FDD_EM_UL1_TAS_VERISION, true) == 2) ? 2 : 1;
                 setHashMapKeyValues(label, simID, "TAS Enable", tasEableMapping(tasidx));
                 setHashMapKeyValues(label, simID, "TX Antenna", antidxMapping(antidx));
                 setHashMapKeyValues(label, simID, "TX Power", tx_pwr == -128 ? "" : tx_pwr);
                 setHashMapKeyValues(label, simID, "RSRP_LANT", rscp_0 == -480 ? "" : (rscp_0 / 4));
                 setHashMapKeyValues(label, simID, "RSRP_UANT", rscp_1 == -480 ? "" : (rscp_1 / 4));
                 if (TasVersion == 2) {
                     setHashMapKeyValues(label, simID, "RSRP_UANT(')",
                             rscp_2 == -480 ? "" : (rscp_2 / 4));
                 }
                 label = Labels[19];
                 updateLabels.put(label, true);
                 setHashMapKeyValues(label, simID, "band",
                     band_class == -1 ? "=" : "Band"+band_class);
                 break;
             case 17:
                 label = Labels[14];
                 updateLabels.put(label, true);
                 coName = MDMContent.UAS_3G_GENERAL_STATUS + "." + MDMContent.UMTS_RRC_STATE;
                 rrState = getFieldValue(data, coName);
                 setHashMapKeyValues(label, simID, "RRC state", rrState <= 6 ?
                         mStateMapping.get(rrState) : "-");
                 break;
             case 18:
                 label = Labels[6];
                 updateLabels.put(label, true);
                 coName = MDMContent.RR_EM_LAI_INFO + ".";
                 int lac0 = getFieldValue(data, coName + MDMContent.LAC +"[0]");
                 int lac1 = getFieldValue(data, coName + MDMContent.LAC +"[1]");
                 String lac = String.format("%d", lac0);
                 lac += String.format("%d", lac1);
                 cellId = getFieldValue(data, coName + MDMContent.CELL_ID);
                 mcc1 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MCC + "[0]");
                 mcc2 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MCC + "[1]");
                 mcc3 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MCC + "[2]");
                 mnc1 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MNC + "[0]");
                 mnc2 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MNC + "[1]");
                 mnc3 = getFieldValue(data, coName
                         + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MNC + "[2]");
                 if (mcc1 == 0xF && mcc2 == 0xF && mcc3 == 0xF && mnc1 == 0xF
                         && mnc2 == 0xF && mnc3 == 0xF) {
                     setHashMapKeyValues(label, simID, "PLMN", "-");
                 } else {
                     setHashMapKeyValues(label, simID, "PLMN", "" +
                         mcc1 + mcc2 + mcc3 + mnc1 + mnc2 + (mnc3 == 0xF ? "" : mnc3));
                 }
                 setHashMapKeyValues(label, simID, "LAC", lac);
                 setHashMapKeyValues(label, simID, "Cell ID", cellId);
                 break;
             case 19:
                 label = Labels[11];
                 updateLabels.put(label, true);
                 int ecio = getFieldValue(data, MDMContent.PILOT_ENERGY);
                 setHashMapKeyValues(label, simID, "ecio", ecio);
                 break;
             case 20:
                 label = Labels[12];
                 updateLabels.put(label, true);
                 ecio = getFieldValue(data, MDMContent.ECIO, true);
                 setHashMapKeyValues(label, simID, "ecio", ecio);
                 break;
             case 21:
                 label = Labels[4];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 int umtsfreqNum = getFieldValue(data,
                         MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_FREQ_NUM);
                 coName = MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_INTER_FREQ
                         + "[";
                 for (int i = 0; i < umtsfreqNum && i < 16; i++) {
                     int valid = getFieldValue(data, coName + i + "]."
                             + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_VALID);
                     uarfcn = getFieldValue(data, coName + i + "]."
                             + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_EARFCN);
                     cellNum = getFieldValue(data, coName + i + "]."
                             + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_UCELL_NUM);
                     String secName = coName + i + "]."
                             + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_UCELL + "[";
                     String[] index = new String[cellNum<6 ? cellNum : 6];
                     String[] pscArray = new String[cellNum<6 ? cellNum : 6];
                     String[] rscpArray = new String[cellNum<6 ? cellNum : 6];
                     String[] ecnArray = new String[cellNum<6 ? cellNum : 6];
                     String[] uarfcnArray = new String[cellNum<6 ? cellNum : 6];
                     for (int j = 0; j < cellNum && j < 6; j++) {
                         int valid2 = getFieldValue(data, secName + j + "]."
                                 + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_UCELL_VALID);
                         psc = getFieldValue(data, secName + j + "]."
                                 + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_UCELL_PSC);
                         rscp = getFieldValue(data, secName + j + "]."
                                 + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_UCELL_RSCP);
                         ecn0 = getFieldValue(data, secName + j + "]."
                                 + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_UCELL_EC_N0);
                         index[j] = j + "";
                         uarfcnArray[j] = "" + ((valid > 0) ? uarfcn : "");
                         pscArray[j] = "" + ((valid2 > 0) ? psc : "");
                         rscpArray[j] = "" + ((valid2 > 0 && rscp != 0xFFFFFFFF)
                                 ? (float) rscp / 4 : "");
                         ecnArray[j] = "" + ((valid2 > 0 && ecn0 != 0xFFFFFFFF)
                                 ? (float) ecn0 / 4 : "");
                     }
                     addHashMapKeyValues(label, simID, "UMTS", index);
                     addHashMapKeyValues(label, simID, "UARFCN", uarfcnArray);
                     addHashMapKeyValues(label, simID, "PSC", pscArray);
                     addHashMapKeyValues(label, simID, "RSCP", rscpArray);
                     addHashMapKeyValues(label, simID, "Ec/No", ecnArray);
                 }
                 break;
             case 22:
                 label = Labels[5];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 coName = MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_GCELL;
                 cellNum = getFieldValue(data,
                         MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_TOTAL_GCELL_NUM);
                 indexArrays = new String[cellNum<6 ? cellNum : 6];
                 arfcnArrays = new String[cellNum<6 ? cellNum : 6];
                 bsicArrays = new String[cellNum<6 ? cellNum : 6];
                 rssiArrays = new String[cellNum<6 ? cellNum : 6];
                 for (int i = 0; i < cellNum && i < 6; i++) {
                     int valid = getFieldValue(data, coName + "[" + i + "]."
                             + MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_VALID);
                     band = getFieldValue(data, coName + "[" + i + "]."
                             + MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_BAND_IND);
                     arfcn = getFieldValue(data, coName + "[" + i + "]."
                             + MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_ARFCN);
                     bsic = getFieldValue(data, coName + "[" + i + "]."
                             + MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_BSIC);
                     rssi = getFieldValue(data, coName + "[" + i + "]."
                             + MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_RSSI, true);
                     indexArrays[i] = "" + i;
                     arfcnArrays[i] = "" + ((valid > 0) ? arfcn : "");
                     bsicArrays[i] = "" + ((valid > 0) ? bsic : "");
                     rssiArrays[i] = "" + ((valid > 0) ? (float) rssi / 4 : "");
                 }
                 addHashMapKeyValues(label, simID, "GSM", indexArrays);
                 addHashMapKeyValues(label, simID, "ARFCN", arfcnArrays);
                 addHashMapKeyValues(label, simID, "BSIC", bsicArrays);
                 addHashMapKeyValues(label, simID, "RSSI", rssiArrays);
                 break;
             case 23:
                 label = Labels[16];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 coName = MDMContent.EM_MEME_DCH_LTE_CELL_LIST;
                 cellNum = getFieldValue(data, MDMContent.EM_MEME_DCH_LTE_NUM_CELLS);
                 indexArrays = new String[cellNum<32 ? cellNum : 32];
                 arfcnArrays = new String[cellNum<32 ? cellNum : 32];
                 pciArrays = new String[cellNum<32 ? cellNum : 32];
                 rsrpArrays = new String[cellNum<32 ? cellNum : 32];
                 rsrqArrays = new String[cellNum<32 ? cellNum : 32];
                 for (int i = 0; i < cellNum && i < 32; i++) {
                     earfcn = getFieldValue(data, coName + "[" + i + "]."
                             + MDMContent.EM_MEME_DCH_LTE_CELL_EARFCN);
                     pci = getFieldValue(data, coName + "[" + i + "]."
                             + MDMContent.EM_MEME_DCH_LTE_CELL_PCI);
                     rsrp = getFieldValue(data, coName + "[" + i + "]."
                             + MDMContent.EM_MEME_DCH_LTE_CELL_RSRP, true);
                     rsrq = getFieldValue(data, coName + "[" + i + "]."
                             + MDMContent.EM_MEME_DCH_LTE_CELL_RSRQ, true);
                     indexArrays[i] = "" + i;
                     arfcnArrays[i] = "" + earfcn;
                     pciArrays[i] = "" + pci;
                     rsrpArrays[i] = "" + rsrp;
                     rsrqArrays[i] = "" + rsrq;
                 }
                 addHashMapKeyValues(label, simID, "LTE", indexArrays);
                 addHashMapKeyValues(label, simID, "EARFCN", arfcnArrays);
                 addHashMapKeyValues(label, simID, "PCI", pciArrays);
                 addHashMapKeyValues(label, simID, "RSRP", rsrpArrays);
                 addHashMapKeyValues(label, simID, "RSRQ", rsrqArrays);
                 break;
             case 24:
                 label = Labels[16];
                 coName = MDMContent.EM_CSCE_NEIGH_CELL_CHOICE + "."
                         + MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_LTE_NEIGH_CELLS;
                 cellNum = getFieldValue(data, MDMContent.EM_CSCE_NEIGH_CELL_COUNT);
                 rat = getFieldValue(data,
                         MDMContent.EM_CSCE_NEIGH_CELL_RAT_TYPE);
                 if (rat == 2) {
                     updateLabels.put(label, true);
                     resetHashMapKeyValues(label, simID);
                     indexArrays = new String[cellNum<16 ? cellNum : 16];
                     arfcnArrays = new String[cellNum<16 ? cellNum : 16];
                     pciArrays = new String[cellNum<16 ? cellNum : 16];
                     rsrpArrays = new String[cellNum<16 ? cellNum : 16];
                     rsrqArrays = new String[cellNum<16 ? cellNum : 16];
                     for (int i = 0; i < cellNum && i < 16; i++) {
                         earfcn = getFieldValue(
                                 data,
                                 coName + "[" + i + "]." +
                                     MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_LTE_NEIGH_CELLS_EARFCN);
                         pci = getFieldValue(
                                 data,
                                 coName + "[" + i + "]." +
                                     MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_LTE_NEIGH_CELLS_PCI);
                         rsrp = getFieldValue(
                                 data, coName + "[" + i + "]." +
                                     MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_LTE_NEIGH_CELLS_RSRP,
                                 true);
                         rsrq = getFieldValue(
                                 data,
                                 coName + "[" + i + "]." +
                                     MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_LTE_NEIGH_CELLS_RSRQ,
                                 true);
                         indexArrays[i] = "" + i;
                         arfcnArrays[i] = "" + earfcn;
                         pciArrays[i] = "" + pci;
                         rsrpArrays[i] = "" + (float) rsrp / 4;
                         rsrqArrays[i] = "" + (float) rsrq / 4;
                     }
                     addHashMapKeyValues(label, simID, "LTE", indexArrays);
                     addHashMapKeyValues(label, simID, "EARFCN", arfcnArrays);
                     addHashMapKeyValues(label, simID, "PCI", pciArrays);
                     addHashMapKeyValues(label, simID, "RSRP", rsrpArrays);
                     addHashMapKeyValues(label, simID, "RSRQ", rsrqArrays);
                 }
                 label = Labels[18];
                 updateLabels.put(label, true);
                 coName = MDMContent.EM_CSCE_NEIGH_CELL_CHOICE + "."
                         + MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_GSM_NEIGH_CELLS;
                 cellNum = getFieldValue(data, MDMContent.EM_CSCE_NEIGH_CELL_COUNT);
                 resetHashMapKeyValues(label, simID);
                 for (int i = 0; i < cellNum && i < 16; i++) {
                     arfcn = getFieldValue(
                             data,
                             coName + "[" + i + "]." +
                                 MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_GSM_NEIGH_CELLS_ARFCN);
                     bsic = getFieldValue(
                             data,
                             coName + "[" + i + "]." +
                                 MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_GSM_NEIGH_CELLS_BSIC);
                     rssi = getFieldValue(
                             data,
                             coName + "[" + i + "]." +
                                 MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_GSM_NEIGH_CELLS_RSSI, true);
                     int bcc = bsic & 0x7;
                     int ncc = (bsic >> 3) & 0x7;
                     addHashMapKeyValues(label, simID, "GSM", "" + i);
                     addHashMapKeyValues(label, simID, "ARFCN", "" + arfcn);
                     addHashMapKeyValues(label, simID, "BSIC", ncc + "" + bcc);
                     addHashMapKeyValues(label, simID, "RSSI", "" + rssi);
                 }
                 break;
             case 25:
                 label = Labels[17];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 for (int i = 0; i < 64; i++) {
                     coName = MDMContent.EM_MEME_DCH_UMTS_CELL_LIST + "[" + i
                             + "].";
                     uarfcn = getFieldValue(data, coName
                             + MDMContent.EM_MEME_DCH_UMTS_CELL_LIST_UARFCN);
                     cellParaId = getFieldValue(data, coName
                             + MDMContent.EM_MEME_DCH_UMTS_CELL_LIST_CELLPARAID);
                     rscp = getFieldValue(data, coName
                             + MDMContent.EM_MEME_DCH_UMTS_CELL_LIST_RSCP, true);
                     int isServingCell = getFieldValue(data, coName
                             + MDMContent.EM_MEME_DCH_UMTS_CELL_LIST_IS_SERVING_CELL);

                     if ((isServingCell != 0) && (rscp > -120)) {
                         addHashMapKeyValues(label, simID, "UMTS", "" + i);
                         addHashMapKeyValues(label, simID, "UARFCN", "" + uarfcn);
                         addHashMapKeyValues(label, simID, "PSC", "" + cellParaId);
                         addHashMapKeyValues(label, simID, "RSCP", "" + (float) rscp / 4096);
                     }
                 }
                 break;
             case 26:
                 label = Labels[18];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 coName = MDMContent.EM_MEME_DCH_GSM_CELL_LIST;
                 cellNum = getFieldValue(data, MDMContent.EM_MEME_DCH_GSM_NCELL_NUM);
                 for (int i = 0; i < cellNum && i < 6; i++) {
                     arfcn = getFieldValue(data, coName + "[" + i + "]."
                             + MDMContent.EM_MEME_DCH_GSM_CELL_LIST_ARFCN);
                     bsic = getFieldValue(data, coName + "[" + i + "]."
                             + MDMContent.EM_MEME_DCH_GSM_CELL_LIST_BSIC);
                     rssi = getFieldValue(data, coName + "[" + i + "]."
                             + MDMContent.EM_MEME_DCH_GSM_CELL_LIST_RSSI, true);
                     int bcc = bsic & 0x7;
                     int ncc = (bsic >> 3) & 0x7;
                     addHashMapKeyValues(label, simID, "GSM", "" + i);
                     addHashMapKeyValues(label, simID, "ARFCN", "" + arfcn);
                     addHashMapKeyValues(label, simID, "BSIC", ncc + "" + bcc);
                     addHashMapKeyValues(label, simID, "RSSI", "" + rssi);
                 }
             case 27:
                 label = Labels[8];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 coName = MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_STATUS + "[";
                 for (int i = 0; i < 6; i++) {
                     int isValid = getFieldValue(data, coName + i + "]."
                             + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_IS_VALID);
                     earfcn = getFieldValue(data, coName + i + "]."
                             + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_EARFCN);
                     pci = getFieldValue(data, coName + i + "]."
                             + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_PCI);
                     rsrp = getFieldValue(data, coName + i + "]."
                             + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_RSRP, true);
                     rsrq = getFieldValue(data, coName + i + "]."
                             + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_RSRQ, true);
                     if (isValid > 0) {
                         addHashMapKeyValues(label, simID, "LTE", "" + i);
                         addHashMapKeyValues(label, simID, "EARFCN", "" + earfcn);
                         addHashMapKeyValues(label, simID, "PCI", "" + pci);
                         addHashMapKeyValues(label, simID, "RSRP", "" + rsrp);
                         addHashMapKeyValues(label, simID, "RSRQ", "" + rsrq);
                     }
                 }
                 break;
             case 28:
                 label = Labels[9];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 coName = MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS + "[";
                 for (int i = 0; i < 6; i++) {
                     int isValid = getFieldValue(data, coName + i + "]."
                             + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_IS_VALID);
                     uarfcn = getFieldValue(data, coName + i + "]."
                             + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_UARFCN);
                     psc = getFieldValue(data, coName + i + "]."
                             + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_PHY_ID);
                     rscp = getFieldValue(data, coName + i + "]."
                             + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_STRENGTH,
                             true);
                     ecn0 = getFieldValue(data, coName + i + "]."
                             + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_QUALITY,
                             true);
                     if (isValid > 0) {
                         addHashMapKeyValues(label, simID, "UMTS", "" + i);
                         addHashMapKeyValues(label, simID, "UARFCN", "" + uarfcn);
                         addHashMapKeyValues(label, simID, "PSC", "" + psc);
                         addHashMapKeyValues(label, simID, "RSCP", "" + rscp);
                         addHashMapKeyValues(label, simID, "Ec/No", "" + ecn0);
                     }
                 }
                 break;
             case 29:
                 int operation = getFieldValue(data,
                         MDMContent.FDD_EM_CSCE_NEIGH_CELL_OPERATION);
                 rat = getFieldValue(data,
                         MDMContent.FDD_EM_CSCE_NEIGH_CELL_RAT_TYPE);
                 if (rat == 0) {
                     label = Labels[23];
                     updateLabels.put(label, true);
                     resetHashMapKeyValues(label, simID);
                     if(operation == 1 || operation == 2) {
                         cellNum = getFieldValue(data,
                                 MDMContent.FDD_EM_CSCE_NEIGH_CELL_COUNT);
                         coName = MDMContent.FDD_EM_CSCE_NEIGH_CELL_CHOICE + "."
                                 + MDMContent.FDD_EM_CSCE_NEIGH_CELL_NEIGH_CELL + "[";
                         for (int i = 0; i < cellNum && i < 16; i++) {
                             uarfcn = getFieldValue(data, coName + i + "]."
                                     + MDMContent.FDD_EM_CSCE_NEIGH_CELL_UARFCN_DL);
                             psc = getFieldValue(data, coName + i + "]."
                                     + MDMContent.FDD_EM_CSCE_NEIGH_CELL_PSC);
                             rscp = (float)getFieldValue(data, coName + i + "]."
                                     + MDMContent.FDD_EM_CSCE_NEIGH_CELL_RSCP, true) / 4096;
                             ecn0 = (float)getFieldValue(data, coName + i + "]."
                                     + MDMContent.FDD_EM_CSCE_NEIGH_CELL_EC_N0, true) / 4096;
                             if (rscp > -120 && ecn0 > -25) {
                                 addHashMapKeyValues(label, simID, "UMTS", "" + i);
                                 addHashMapKeyValues(label, simID, "UARFCN", "" + uarfcn);
                                 addHashMapKeyValues(label, simID, "PSC", "" + psc);
                                 addHashMapKeyValues(label, simID, "RSCP", "" + rscp);
                                 addHashMapKeyValues(label, simID, "Ec/No", "" + ecn0);
                             }
                         }
                     }
                 } else if(rat == 1){
                     label = Labels[24];
                     updateLabels.put(label, true);
                     resetHashMapKeyValues(label, simID);
                     if(operation == 1 || operation == 2) {
                         coName = MDMContent.EM_CSCE_NEIGH_CELL_CHOICE + "."
                                 + MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_GSM_NEIGH_CELLS + "[";
                         cellNum = getFieldValue(data, MDMContent.EM_CSCE_NEIGH_CELL_COUNT);
                         for (int i = 0; i < cellNum && i < 16; i++) {
                             arfcn = getFieldValue(data, coName + i + "]." +
                                 MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_GSM_NEIGH_CELLS_ARFCN);
                             bsic =  getFieldValue(data, coName + i + "]." +
                                 MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_GSM_NEIGH_CELLS_BSIC);
                             rssi =  getFieldValue(data, coName + i + "]." +
                                 MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_GSM_NEIGH_CELLS_RSSI, true);
                             int bcc = bsic & 0x7;
                             int ncc = (bsic >> 3) & 0x7;
                             addHashMapKeyValues(label, simID, "GSM", "" + i);
                             addHashMapKeyValues(label, simID, "ARFCN", "" + arfcn);
                             addHashMapKeyValues(label, simID, "BSIC", ncc + "" + bcc);
                             addHashMapKeyValues(label, simID, "RSSI", "" + rssi);
                         }
                     }
                 } else if(rat == 2) {
                     label = Labels[22];
                     updateLabels.put(label, true);
                     resetHashMapKeyValues(label, simID);
                     if(operation == 2) {
                         cellNum = getFieldValue(data,
                                 MDMContent.FDD_EM_CSCE_NEIGH_CELL_COUNT);
                         coName = MDMContent.FDD_EM_CSCE_NEIGH_CELL_CHOICE + "."
                                 + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_NEIGH_CELL + "[";
                         for (int i = 0; i < cellNum && i < 16; i++) {
                             earfcn = getFieldValue(data, coName + i + "]."
                                     + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_EARFCN);
                             pci = getFieldValue(data, coName + i + "]."
                                     + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_PCI);
                             rsrp = getFieldValue(data, coName + i + "]."
                                     + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_RSRP, true);
                             rsrq = getFieldValue(data, coName + i + "]."
                                     + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_RSRQ, true);
                             addHashMapKeyValues(label, simID, "LTE", "" + i);
                             addHashMapKeyValues(label, simID, "EARFCN", "" + earfcn);
                             addHashMapKeyValues(label, simID, "PCI", "" + pci);
                             addHashMapKeyValues(label, simID, "RSRP", "" + (float)rsrp / 4096);
                             addHashMapKeyValues(label, simID, "RSRQ", "" + (float)rsrq / 4096);
                         }
                     }
                 }
                 break;
             case 30:
                 label = Labels[22];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 cellNum = getFieldValue(data,
                         MDMContent.FDD_EM_MEME_DCH_LTE_CELL_INFO_NUM_CELLS);
                 coName = MDMContent.FDD_EM_MEME_DCH_LTE_CELL_INFO_LTE_CELL_LIST + "[";
                 for (int i = 0; i < cellNum && i < 32; i++) {
                     earfcn = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_LTE_CELL_INFO_EARFCN);
                     pci = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_LTE_CELL_INFO_PCI);
                     rsrp = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_LTE_CELL_INFO_RSRP, true);
                     rsrq = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_LTE_CELL_INFO_RSRQ, true);
                     addHashMapKeyValues(label, simID, "LTE", "" + i);
                     addHashMapKeyValues(label, simID, "EARFCN", "" + earfcn);
                     addHashMapKeyValues(label, simID, "PCI", "" + pci);
                     addHashMapKeyValues(label, simID, "RSRP", "" + (float) rsrp / 4096);
                     addHashMapKeyValues(label, simID, "RSRQ", "" + (float) rsrq / 4096);
                 }
                 break;
             case 31:
                 label = Labels[24];
                 updateLabels.put(label, true);
                 resetHashMapKeyValues(label, simID);
                 cellNum = getFieldValue(data,
                         MDMContent.FDD_EM_MEME_DCH_GSM_CELL_INFO_NUM_CELLS);
                 coName = MDMContent.FDD_EM_MEME_DCH_GSM_CELL_INFO_GSM_CELL_LIST + "[";
                 for (int i = 0; i < cellNum && i < 6; i++) {
                     earfcn = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_GSM_CELL_INFO_ARFCN);
                     bsic = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_GSM_CELL_INFO_BSIC);
                     rssi = getFieldValue(data, coName + i + "]."
                             + MDMContent.FDD_EM_MEME_DCH_GSM_CELL_INFO_RSSI, true);
                     int bcc = bsic & 0x7;
                     int ncc = (bsic >> 3) & 0x7;
                     addHashMapKeyValues(label, simID, "GSM", "" + i);
                     addHashMapKeyValues(label, simID, "ARFCN", "" + earfcn);
                     addHashMapKeyValues(label, simID, "BSIC", ncc + "" + bcc);
                     addHashMapKeyValues(label, simID, "RSSI", "" + rssi);
                 }
                 break;
             default:
                 break;
         }
         return updateLabels;
    }

    private class UpdateViewTask extends AsyncTask<Task, Object[], Void> {

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            if(Labels == null) Labels = initLabels();
        }

        @Override
        protected Void doInBackground(Task... params) {
            String name = params[0].getExtraName();
            Object msg = params[0].getExtraMsg();
            int simID = params[0].getExtraSimID();
            HashMap<String, Boolean> updateLabels = getUpdateMap(Labels, simID, name, msg);
            String[] networkInfo = getSurpportedLabel(simID);
            Set<String> validLabelSets = getSupportedLabelsByKeyWord(simID);
            Set<String> tempLabels = new HashSet<String>();
            boolean hasValidLabel = false;
            for(String mLabel : Labels) {
                if(mUpdateTaskDriven.isDriverDead()){
                    Elog.e(TAG, "Exit doInBackground, Driver dead");
                    return null;
                }
                if(networkInfo[0].equals("Unknown") &&
                        networkInfo[1].equals("Unknown")) {
                    if(updateLabels.keySet().contains(mLabel) &&
                            (validLabelSets.contains(mLabel) ||
                                    validLabelSets.contains(mLabel.split("-")[0]))) {
                        clearData(mLabel, simID);
                        addData(mLabel, simID);
                        publishProgress(new Object[]{simID, mLabel, true, true});
                        hasValidLabel = true;
                    } else {
                        publishProgress(new Object[]{simID, mLabel, false});
                        tempLabels.add(mLabel);
                    }
                } else if ((mLabel.indexOf(networkInfo[0]) >= 0 ||
                        mLabel.indexOf(networkInfo[1]) >= 0) &&
                        (validLabelSets.contains(mLabel) ||
                                validLabelSets.contains(mLabel.split("-")[0]))) {
                    hasValidLabel = true;
                    if(updateLabels.keySet().contains(mLabel) && updateLabels.get(mLabel)) {
                        publishProgress(new Object[]{simID, mLabel, true, true});
                    } else {
                        publishProgress(new Object[]{simID, mLabel, true, false});
                    }
                } else {
                    publishProgress(new Object[]{simID, mLabel, false});
                    tempLabels.add(mLabel);
                }
            }
            if(!hasValidLabel){
                for(String tempLabel : tempLabels) {
                    if((tempLabel.indexOf(networkInfo[0]) >= 0 ||
                            tempLabel.indexOf(networkInfo[1]) >= 0) ||
                            (networkInfo[0].equals("Unknown") &&
                            networkInfo[1].equals("Unknown"))){
                        publishProgress(new Object[]{simID, tempLabel, false});
                        Elog.d(TAG, "[showView] Sim" + simID + " has no valid labels, show " +
                                "label:" + tempLabel);
                    }
                }
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Object[]... values) {
            super.onProgressUpdate(values);
            if(mUpdateTaskDriven.isDriverDead()){
                this.cancel(true);
                Elog.e(TAG, "Exit onProgressUpdate, Driver dead");
                taskDone();
                return;
            }
            int simID = Integer.valueOf(values[0][0].toString());
            String mLabel = values[0][1].toString();
            boolean updateViewFlag = (Boolean) values[0][2];
            if(updateViewFlag) {
                boolean updateDataFlag = (Boolean) values[0][3];
                if(updateDataFlag) {
                    clearData(mLabel, simID);
                    addData(mLabel, simID);
                }
            }
            displayView(mLabel, simID , updateViewFlag);
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            taskDone();
        }
    }

    private Set<String> getSupportedLabelsByKeyWord(int simID) {
        Set<String> validLabels = new HashSet<String>();
        if(isValidSimID(simID)) {
            for(String mLabel: initLabels()){
                boolean isValid = true;
                if(mKeyWordMapping.containsKey(mLabel) ||
                        mKeyWordMapping.containsKey(mLabel.split("-")[0])){
                    String [] keyWords = mKeyWordMapping.get(mLabel) != null ?
                            mKeyWordMapping.get(mLabel):
                            mKeyWordMapping.get(mLabel.split("-")[0]);
                    if(isLabelArrayType(mLabel)){
                        if(hmapLabelsList.get(simID).get(mLabel).size() <= 1) {
                            isValid = false;
                            continue;
                        }
                    } else {
                        for(String key : keyWords){
                            if(hmapLabelsList.get(simID).get(mLabel).get(key) == null ||
                                    hmapLabelsList.get(simID).get(mLabel).get(key).equals("")||
                                    hmapLabelsList.get(simID).get(mLabel).get(key).equals("0")){
                                isValid = false;
                                break;
                            }
                        }
                    }
                }
                if(isValid) {
                    validLabels.add(mKeyWordMapping.containsKey(mLabel.split("-")[0]) ?
                            mLabel.split("-")[0] : mLabel);
                }
            }
        }
        return validLabels;
    }

    private String[] getSurpportedLabel(int simID) {
        int[] networkType = getNetworkType(simID);
        String[] rtString = new String[networkType.length];
        for(int i=0; i<networkType.length; i++){
            switch(networkType[i]) {
            case NETWORK_TYPE_UNKNOWN:
                rtString[i] = "Unknow";
                break;
            case NETWORK_TYPE_GPRS:
                rtString[i] = "GSM";
                break;
            case NETWORK_TYPE_EDGE:
                rtString[i] = "GSM";
                break;
            case NETWORK_TYPE_UMTS:
                rtString[i] = "UMTS";
                break;
            case NETWORK_TYPE_CDMA:
                rtString[i] = "1xRTT";
                break;
            case NETWORK_TYPE_1xRTT:
                rtString[i] = "1xRTT";
                break;
            case NETWORK_TYPE_EVDO_0:
                rtString[i] = "EVDO";
                break;
            case NETWORK_TYPE_EVDO_A:
                rtString[i] = "EVDO";
                break;
            case NETWORK_TYPE_HSDPA:
                rtString[i] = "UMTS FDD";
                break;
            case NETWORK_TYPE_HSUPA:
                rtString[i] = "UMTS FDD";
                break;
            case NETWORK_TYPE_HSPA:
                rtString[i] = "UMTS FDD";
                break;
            case NETWORK_TYPE_EVDO_B:
                rtString[i] = "EVDO";
                break;
            case NETWORK_TYPE_EHRPD:
                rtString[i] = "EVDO";
                break;
            case NETWORK_TYPE_LTE:
                rtString[i] = "LTE";
                break;
            case NETWORK_TYPE_HSPAP:
                rtString[i] = "UMTS FDD";
                break;
            case NETWORK_TYPE_GSM:
                rtString[i] = "GSM";
                break;
            case NETWORK_TYPE_TD_SCDMA:
                rtString[i] = "UMTS FDD";
                break;
            case NETWORK_TYPE_LTE_CA:
                rtString[i] = "LTE";
                break;
            default:
                rtString[i] = networkInfo[i];
                Elog.e(TAG, "Unexpected radioTechnology");
                break;
            }
        }
        if(rtString[0].indexOf("UMTS") >= 0 || rtString[1].indexOf("UMTS") >= 0) {
            int mask = WorldModeUtil.get3GDivisionDuplexMode();
            if(mask != 0) {
                String rt3GString = (mask==1 ? "UMTS FDD" : "UMTS TDD");
                if(ModemCategory.getCapabilitySim() != simID ||
                        mask==1){
                    rt3GString = "UMTS FDD";
                }
                rtString[0] = rtString[0].indexOf("UMTS") >= 0 ? rt3GString : rtString[0];
                rtString[1] = rtString[1].indexOf("UMTS") >= 0 ? rt3GString : rtString[1];
            }
        }
        return rtString;
    }

    private int[] getNetworkType(int simID) {
        TelephonyManager telephonyManager = (TelephonyManager) this.mActivity
                .getSystemService(this.mActivity.TELEPHONY_SERVICE);
        if(telephonyManager == null) {
            Elog.e(TAG, "[getNetworkType] telephonyManager == null");
             return new int[]{TelephonyManager.NETWORK_TYPE_UNKNOWN,
                     TelephonyManager.NETWORK_TYPE_UNKNOWN};
        }
        int dataNetworkType = telephonyManager.getDataNetworkType(simID);
        int voiceNetworkType = telephonyManager.getVoiceNetworkType(simID);
        Elog.d(TAG, "[getNetworkType] SIM" + simID +
                ", getDataNetworkType " + dataNetworkType +
                ", getVoiceNetworkType " + voiceNetworkType);
        return new int[]{ dataNetworkType,
                voiceNetworkType };
    }

    boolean isLabelArrayType(String label) {
        if(getArrayTypeLabels().contains(label.indexOf(".") > 0 ?
                label.substring(0, label.indexOf(".")) : label) ) {
            return true;
        }
        return false;
    }

    @Override
    void clearData() {
        Elog.d(TAG, "removeView()");
        unRegisteListener();
        if (layout != null && layout.getChildCount() > 0) {
            layout.removeAllViews();
            scrollView.removeAllViews();
        }
        commonView.setAdapter(null);
        commonInfoAdapter.clear();
        mUpdateTaskDriven.resetDriver();
        clearViewData(-1);
    }

}

abstract class CombinationViewComponent extends MDMComponent {
    private static final int SHOW_SIM_ALL = -1;
    private static final int SHOW_SIM_ONE = 0;
    private static final int SHOW_SIM_TWO = 1;
    private int currentSimID = 0;
    private int supportSimCount = 2;
    List<LinkedHashMap<String, LinkedHashMap>> hmapLabelsList = null;
    List<HashMap<String, MdmLinearLayout>> hmapViewList = null;
    List<HashMap<String, TableInfoAdapter>> hmapAdapterList = null;
    MdmLinearLayout commonView;
    TableInfoAdapter commonInfoAdapter;
    ScrollView scrollView;
    LinearLayout layout;
    Activity mContext;
    String location;
    String[] imsi;
    HandlerThread mHandlerThread;
    UpdateTaskDriven mUpdateTaskDriven;

    public int getSupportSimCount() {
        return supportSimCount;
    }

    public int getCurrentSimID() {
        return currentSimID;
    }

    public void setCurrentSimID(int currentSimID) {
        this.currentSimID = currentSimID;
    }
    public CombinationViewComponent(Activity context, int simCount) {
        super(context);
        mContext = context;
        commonView = new MdmLinearLayout(context);
        commonInfoAdapter = new TableInfoAdapter(context);
        this.supportSimCount = simCount;
        imsi = new String[getSupportSimCount()];

        if (scrollView == null) {
            scrollView = new ScrollView(mContext);
            scrollView.setLayoutParams(
                    new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT) );
        }
        if (layout == null) {
            layout = new LinearLayout(mContext);
            layout.setOrientation(LinearLayout.VERTICAL);
            layout.setLayoutParams(
                    new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT) );
        }
        if(hmapLabelsList == null) {
            hmapLabelsList = new ArrayList<LinkedHashMap<String, LinkedHashMap>>();
            hmapViewList = new ArrayList<HashMap<String, MdmLinearLayout>>();
            hmapAdapterList = new ArrayList<HashMap<String, TableInfoAdapter>>();
            for(int i=0; i<getSupportSimCount(); i++) {
                hmapLabelsList.add(i, new LinkedHashMap<String, LinkedHashMap>());
                hmapViewList.add(i, new HashMap<String, MdmLinearLayout>());
                hmapAdapterList.add(i, new HashMap<String, TableInfoAdapter>());
            }
        }
        if(mHandlerThread == null) {
            mHandlerThread = new HandlerThread("MtkMdmViewManager");
            mHandlerThread.start();
            Looper looper = mHandlerThread.getLooper();
            mUpdateTaskDriven = new UpdateTaskDriven(looper);
        } else {
            mUpdateTaskDriven.setDriverState(UpdateTaskDriven.DRIVER_NOT_READY);
        }
    }

    void taskDone() {
        Message ssmsg = mUpdateTaskDriven.obtainMessage(UpdateTaskDriven.EVENT_DONE);
        ssmsg.sendToTarget();
    }

    private void taskStop() {
        Elog.d(TAG, "taskStop()");
        Task startTask = new Task(UpdateTaskDriven.TASK_REMOVE_VIEW);
        mUpdateTaskDriven.addFirstTask(startTask, UpdateTaskDriven.DRIVER_DEAD);
    }

    private void taskStart() {
        Elog.d(TAG, "taskStart()");
        Task startTask = new Task(UpdateTaskDriven.TASK_INIT_VIEW);
        mUpdateTaskDriven.addFirstTask(startTask, UpdateTaskDriven.DRIVER_NOT_READY);
    }

    class Task {
        private int mTaskId = -1;
        private String mName = "";
        private Object mData = null;
        private int mSimID = 0;

        public Task(int taskId, String name, Object msg, int simID) {
            mTaskId = taskId;
            mName = name;
            mData = msg;
            mSimID = simID;
        }
        public Task(int taskId) {
            mTaskId   = taskId;
        }
        public int getTaskId() {
            return mTaskId;
        }
        public String getExtraName() {
            return mName;
        }
        public Object getExtraMsg() {
            return mData;
        }
        public int getExtraSimID() {
            return mSimID;
        }
        public String toString() {
            return "Task ID: " + mTaskId +
                 ", msgName: " + mName +
                 ", mSimID: " + mSimID;
        }
    }

    class UpdateTaskDriven extends Handler {
        private ArrayList <Task> mPendingTask = new ArrayList<Task>();
        private final static String TAG = "EmInfo/MDMComponentFT/UpdateTaskDriven";

        private Object mTaskLock = new Object();
        private Object mStateLock = new Object();
        private Object mDriverStateLock = new Object();

        private final static int EVENT_EXEC_NEXT  = 1;
        private final static int EVENT_DONE = 2;

        private final static int STATE_NO_PENDING = 0;
        private final static int STATE_DOING      = 1;
        private final static int STATE_DONE       = 2;

        private final static int DRIVER_READY = 1;
        private final static int DRIVER_NOT_READY = 2;
        private final static int DRIVER_DEAD = 0;

        protected static final int TASK_INIT_VIEW = 0;
        protected static final int TASK_UPDATE_DATA = 1;
        protected static final int TASK_REMOVE_VIEW = 2;

        private int mState = STATE_NO_PENDING;
        private int mDriverState = DRIVER_NOT_READY;

        public UpdateTaskDriven() {
        }

        public UpdateTaskDriven(Looper looper) {
            super(looper);
        }

        public void appendTask(Task task) {
            synchronized (mTaskLock) {
                if(isDriverDead()) {
                    Elog.d(TAG, "Driver dead! current task returned " + task.toString());
                    return;
                }
                for(int i=1; i<mPendingTask.size(); i++) {
                    if(isTaskRepeate(task, mPendingTask.get(i))) {
                        mPendingTask.remove(i);
                        break;
                    }
                }
                mPendingTask.add(task);
            }
            if(isDriverReady()) {
                Message msg = obtainMessage(EVENT_EXEC_NEXT);
                msg.sendToTarget();
            }
        }

        public void addFirstTask(Task task, int driverState) {
            setDriverState(driverState);
            synchronized (mTaskLock) {
                if(driverState == DRIVER_DEAD) {
                    if(isTaskRunning()) {
                        Task curTask = mPendingTask.get(0);
                        mPendingTask.clear();
                        mPendingTask.add(curTask);
                    } else {
                        mPendingTask.clear();
                    }
                    mPendingTask.add(task);
                } else {
                    int start = 0;
                    for(int i=0; i<mPendingTask.size(); i++) {
                        if(mPendingTask.get(i).getTaskId() == TASK_REMOVE_VIEW) {
                            Elog.d(TAG, "remove task" + i + " : TASK_REMOVE_VIEW");
                            start = i;
                            break;
                        }
                    }
                    for(int j=isTaskRunning() ? 1 : 0; j<=start && j<mPendingTask.size(); j++) {
                        mPendingTask.remove(j);
                    }
                    mPendingTask.add(isTaskRunning() ? 1 : 0, task);
                }
            }

            Elog.d(TAG, "[addFirstTask] task: " + task.toString() +
                    ", mPendingTask: " + Arrays.toString(mPendingTask.toArray()));
            Message msg = obtainMessage(EVENT_EXEC_NEXT);
            msg.sendToTarget();
        }

        public void resetDriver() {
            mPendingTask.clear();
            setDriverState(DRIVER_NOT_READY);
        }

        private int getState() {
            synchronized (mStateLock) {
                return mState;
            }
        }

        private int getDriverState() {
            synchronized (mDriverStateLock) {
                return mDriverState;
            }
        }

        private void setDriverState(int driverState) {
            Elog.d(TAG, "[setDriverState] "+ mDriverState + " --> " + driverState);
            synchronized (mDriverStateLock) {
                mDriverState = driverState;
            }
        }

        public boolean isDriverDead() {
            return getDriverState() == DRIVER_DEAD;
        }

        public boolean isTaskRunning() {
            return getState() == STATE_DOING;
        }

        public boolean isDriverReady() {
            return getState() == DRIVER_READY;
        }

        public boolean isStopTask(Task task) {
            return task.getTaskId() == TASK_REMOVE_VIEW;
        }

        public boolean isTaskRepeate(Task A, Task B) {
            return A.getTaskId() == B.getTaskId() &&
                    A.getExtraName() == B.getExtraName() &&
                    A.getExtraSimID() == B.getExtraSimID();
        }

        private void setState(int state) {
            synchronized (mStateLock) {
                mState = state;
            }
        }

        private Task getCurrentPendingTask() {
            synchronized (mTaskLock) {
                if (mPendingTask.size() == 0) {
                    return null;
                }
                return mPendingTask.get(0);
            }
        }

        private void removePendingTask(int index) {
            synchronized (mTaskLock) {
                if (mPendingTask.size() > 0) {
                    Elog.d(TAG, "removePendingTask " + mPendingTask.get(index).toString() +
                            " ,remain mPendingTask: " + (mPendingTask.size()-1) );
                    mPendingTask.remove(index);
                }
            }
        }

        public void clearPendingTask() {
            synchronized (mTaskLock) {
                mPendingTask.clear();
            }
        }

        public void exec() {

            Task task = getCurrentPendingTask();
            if (task == null) {
                setState(STATE_NO_PENDING);
                return;
            }

            if (getState() == STATE_DOING) {
                return;
            }

            if(isDriverDead() && !isStopTask(task)) {
                taskDone();
                return;
            }
            setState(STATE_DOING);
            int taskId = task.getTaskId();
            Elog.d(TAG, "Task State = " + stateToString(getState()) + " Task :" + task.toString());

            switch (taskId) {
                case TASK_INIT_VIEW: {
                    new InitViewTask().execute();
                    break;
                }
                case TASK_UPDATE_DATA: {
                    startUpdateProcess(task);
                    break;
                }
                case TASK_REMOVE_VIEW: {
                    clearViewData(-1);
                    break;
                }
                default: {
                    taskDone();
                    break;
                }
            }
        }

        @Override
        public void handleMessage(Message msg) {

            switch (msg.what) {
                case EVENT_DONE:
                    Elog.d(TAG, "Task receive EVENT_DONE");
                    removePendingTask(0);  // Do next task directly.
                    setState(STATE_DONE);
                case EVENT_EXEC_NEXT:
                    exec();
                    break;
            }
        }

        private String stateToString(int state) {
            switch (state) {
                case STATE_NO_PENDING:
                    return "STATE_NO_PENDING";
                case STATE_DOING:
                    return "STATE_DOING";
                case STATE_DONE:
                    return "STATE_DONE";
            }
            return "UNKNOWN_STATE";
        }

        private String eventToString(int event) {
            switch (event) {
                case EVENT_DONE:
                    return "EVENT_DONE";
                case EVENT_EXEC_NEXT:
                    return "EVENT_EXEC_NEXT";
            }
            return "UNKNOWN_EVENT";
        }
    }

    void clearViewData(int SimID) {
        if(!isValidSimID(SimID)) {
            for(int i=0; i<getSupportSimCount(); i++) {
                if (hmapLabelsList.size() > i && hmapLabelsList.get(i) != null) {
                    hmapLabelsList.get(i).clear();
                    hmapViewList.get(i).clear();
                    hmapAdapterList.get(i).clear();
                }
            }
        } else {
            if (hmapLabelsList.contains(SimID)){
                hmapLabelsList.get(SimID).clear();
                hmapViewList.get(SimID).clear();
                hmapAdapterList.get(SimID).clear();
            }
        }
    }

    void initHashMap(int simID) {
        if(isValidSimID(simID)) {
            initHashMapByID(simID);
        } else {
            for(int i=0; i<getSupportSimCount(); i++) {
                initHashMapByID(i);
            }
        }
    }

    void initHashMapByID(int id) {
        if(id < hmapLabelsList.size() && hmapLabelsList.get(id).isEmpty()) {
            hmapLabelsList.set(id, getHashMapLabels());
            for(String key: hmapLabelsList.get(id).keySet()) {
                hmapAdapterList.get(id).put(key, new TableInfoAdapter(mContext));
                hmapViewList.get(id).put(key, new MdmLinearLayout(mContext));
            }
        }
    }

    LinkedHashMap<String, String> initHashMap(Object[] keys) {
        LinkedHashMap <String, String> hashMapObj = new LinkedHashMap <String, String>();
        for(int i = 0; i < keys.length; i++) {
            hashMapObj.put((String) keys[i], null);
        }
        return hashMapObj;
    }

    LinkedHashMap<String, LinkedHashMap> getHashMapLabels() {
        // TODO Auto-generated method stub
        LinkedHashMap<String, LinkedHashMap> hashMapkeyValues =
                new LinkedHashMap <String, LinkedHashMap>();
        if(hashMapkeyValues.size() == 0) {
            ArrayList<LinkedHashMap> hashMapValues = initHashMapValues();
            String[] labelKeys = initLabels();
            for(int i=0; i < labelKeys.length; i++) {
                hashMapkeyValues.put(labelKeys[i], hashMapValues.get(i));
            }
        }
        return hashMapkeyValues;
    }

    void resetHashMapKeyValues(String Label, int simID) {
        if(isValidSimID(simID)) {
            LinkedHashMap map = new LinkedHashMap();
            map = initHashMap(hmapLabelsList.get(simID).get(Label).keySet().toArray());
            hmapLabelsList.get(simID).get(Label).clear();
            hmapLabelsList.get(simID).get(Label).putAll(map);
        }
    }

    void setHashMapKeyValues(String label, int simID, String key, Object value) {
        if(isValidSimID(simID)) {
            if(isLabelArrayType(label)){
                if(value instanceof String[]) {
                    hmapLabelsList.get(simID).get(label).put(key, (String[]) value);
                } else {
                    hmapLabelsList.get(simID).get(label).put(key, new String[]{value.toString()});
                }
            } else {
                hmapLabelsList.get(simID).get(label).put(key, String.valueOf(value));
            }
        }
    }

    void addHashMapKeyValues(String label, int simID, String key, Object value) {
        if(isValidSimID(simID)) {
            if(isLabelArrayType(label)){
                String[] oldValue = hmapLabelsList.get(simID).get(label).get(key) != null ?
                    (String[]) hmapLabelsList.get(simID).get(label).get(key) : new String[0];
                String[] setValue = value instanceof String[] ?
                    (String[]) value : new String[]{value.toString()};
                String[] newValue = new String[oldValue.length + setValue.length];
                System.arraycopy(oldValue, 0, newValue, 0, oldValue.length);
                System.arraycopy(setValue, 0, newValue, oldValue.length, setValue.length);
                hmapLabelsList.get(simID).get(label).put(key, newValue);
            }
        }
    }

    void setHashMapKeyValues(String Label, int simID, LinkedHashMap<String, String> keyValues) {
        if(isValidSimID(simID)) {
            hmapLabelsList.get(simID).put(Label, keyValues);
        }
    }

    void setHashMapKeyValues(String Label, int simID, String[] values) {
        if(isValidSimID(simID)) {
            String[] keys = new String[hmapLabelsList.get(simID).get(Label).keySet().size()];
            hmapLabelsList.get(simID).get(Label).keySet().toArray(keys);
            for(int i=0; i<keys.length; i++) {
                setHashMapKeyValues(Label, simID, keys[i], values[i]);
            }
        }
    }

    @Override
    View getView() {
        Elog.d(TAG, "getView()");
        registeListener();
        if(scrollView.getParent() != null)
            ((ViewGroup)scrollView.getParent()).removeView(scrollView);
        if(scrollView.getChildCount() > 0)  scrollView.removeAllViews();
        if(layout.getParent() != null) ((ViewGroup) layout.getParent()).removeView(layout);
        safeAddView(scrollView, layout);
        updateCommonView(getCurrentSimID());
        safeAddView(layout, commonView);
        taskStart();
        return scrollView;
    }

    private class InitViewTask extends AsyncTask<Void, String[], Void> {

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            clearViewData(-1);
        }

        @Override
        protected Void doInBackground(Void... params) {
            for(int i=0; i<getSupportSimCount(); i++) {
                initHashMap(i);
                if(hmapLabelsList.get(i) != null) {
                    for(String label:  hmapLabelsList.get(i).keySet()) {
                        if(label.split("-").length > 1 &&
                            !(label.split("-")[1].indexOf(".") >=0 )) {
                            setHmapAdapterByLabel(label, i);
                            publishProgress(new String[]{i+"", label});
                        }
                    }
                }
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(String[]... values) {
            super.onProgressUpdate(values);
            int index = Integer.valueOf(values[0][0]);
            String label = values[0][1];
            if(label.split("-").length > 1 && !(label.split("-")[1].indexOf(".") >=0 )) {
                hmapViewList.get(index).get(label).showTitle();
                hmapViewList.get(index).get(label).setTextContent("SIM" + (index+1) + ":" + label);
            }
            hmapViewList.get(index).get(label).setAdapter(hmapAdapterList.get(index).get(label));
            hmapAdapterList.get(index).get(label).notifyDataSetChanged();
            hmapViewList.get(index).get(label).setListViewHeightBasedOnChildren();
            if(hmapViewList.get(index).get(label).getParent() != null) {
                ((ViewGroup) hmapViewList.get(index).get(label).getParent()).removeView(
                        hmapViewList.get(index).get(label));
            }
            safeAddView(layout, hmapViewList.get(index).get(label));
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            mUpdateTaskDriven.setDriverState(UpdateTaskDriven.DRIVER_READY);
            taskDone();
        }
    }

    public void updateCommonView(int simID) {
        commonInfoAdapter.clear();
        if(isValidSimID(simID)) {
            commonInfoAdapter.add(new String[] {"Location", "IMSI" + (1+simID)});
            commonInfoAdapter.add(new String[] {location, imsi[simID]});
        } else {
            String[] commonTitles = new String[getSupportSimCount() + 1];
            String[] commonValues = new String[getSupportSimCount() + 1];
            commonTitles[0] = "Location";
            commonValues[0] = this.location;
            for(int i=0; i<getSupportSimCount(); i++){
                commonTitles[i+1] = "IMSI"+(i+1);
                commonValues[i+1] = imsi[i] == null ? " " : imsi[i];
            }
            commonInfoAdapter.add(commonTitles);
            commonInfoAdapter.add(commonValues);
        }
        commonView.setAdapter(commonInfoAdapter);
        commonView.setTextContent("Common");
        commonInfoAdapter.notifyDataSetChanged();
        commonView.setListViewHeightBasedOnChildren();
    }

    public void safeAddView(ViewGroup parent, View view){
        if(view.getParent() != null) ((ViewGroup) view.getParent()).removeView(view);
        parent.addView(view);
    }

    @Override
    void removeView() {
        Elog.d(TAG, "removeView()");
        unRegisteListener();
        if (layout != null && layout.getChildCount() > 0) {
            layout.removeAllViews();
            scrollView.removeAllViews();
        }
        commonView.setAdapter(null);
        commonInfoAdapter.clear();
        if(mUpdateTaskDriven.isTaskRunning()){
            taskStop();
            return;
        }
        clearViewData(-1);
    }


    void clearData(String key, int simID) {
        if(isValidSimID(simID)) {
            if (hmapAdapterList.get(simID).get(key) != null) {
                hmapAdapterList.get(simID).get(key).clear();
            }
        } else {
            for(int i=0; i<hmapLabelsList.size(); i++) {
                if (hmapAdapterList.get(i).get(key) != null) {
                        hmapAdapterList.get(i).get(key).clear();
                }
            }
        }
    }

    void displayView(String label,int simID, boolean isShow) {
        if(isValidSimID(simID)) {
            if(isShow && (!isValidSimID(getCurrentSimID()) || getCurrentSimID() == simID)) {
                hmapViewList.get(simID).get(label).showView();
            } else {
                hmapViewList.get(simID).get(label).hideView();
            }
        } else {
            for(int i=0; i<getSupportSimCount(); i++) {
                if(isShow) {
                    hmapViewList.get(i).get(label).showView();
                } else {
                    hmapViewList.get(i).get(label).hideView();
                }
            }
        }
    }

    boolean isValidSimID(int simID) {
        if(simID < getSupportSimCount() && simID >= 0) {
            return true;
        }
        return false;
    }

    void addData(String label, int simID) {
        if(isValidSimID(simID)) {
            setHmapAdapterByLabel(label, simID);
            hmapViewList.get(simID).get(label).setAdapter(hmapAdapterList.get(simID).get(label));
            hmapAdapterList.get(simID).get(label).notifyDataSetChanged();
            hmapViewList.get(simID).get(label).setListViewHeightBasedOnChildren();
        }
    }

    void setHmapAdapter(int simID) {
        for(String label:  hmapLabelsList.get(simID).keySet()) {
            if (hmapAdapterList.get(simID).get(label).getCount() == 0) {
                setHmapAdapterByLabel(label, simID);
            }
        }
    }

    void setHmapAdapterByLabel(String label, int simID) {
        if(isValidSimID(simID)) {
            String[] keys = new String[hmapLabelsList.get(simID).get(label).keySet().size()];
            hmapLabelsList.get(simID).get(label).keySet().toArray(keys);
            if(isLabelArrayType(label)) {
                Map<String, String[]> arrayValuesMap = new HashMap<String, String[]>();
                int length = 0;
                hmapAdapterList.get(simID).get(label).add(keys);
                for(int i=0; i < keys.length; i++) {
                    if(hmapLabelsList.get(simID).get(label).get(keys[i]) == null) {
                        arrayValuesMap.put(keys[i], new String[]{""});
                    } else {
                        Object[] values =
                            (Object[]) hmapLabelsList.get(simID).get(label).get(keys[i]);
                        length = length < values.length ? values.length : length;
                        arrayValuesMap.put(keys[i], (String[])values);
                    }
                }
                for(int i=0; i<length; i++) {
                    String[] arrayValue = new String[keys.length];
                    boolean valid = false;
                    for(int j=0; j<keys.length; j++) {
                        arrayValue[j] = arrayValuesMap.get(keys[j])!= null &&
                            i<arrayValuesMap.get(keys[j]).length ?
                                arrayValuesMap.get(keys[j])[i] : "";
                        if (!arrayValue[j].equals("")) valid = true;
                    }
                    if (valid) {
                        hmapAdapterList.get(simID).get(label).add(arrayValue);
                        Elog.d(TAG, "[setHmapAdapter][ArrayType] label: " + label +
                               " ,values: " + Arrays.toString(arrayValue));
                    }
                }
            } else {
                for(int i=0; i<keys.length; i+=2) {
                    if(i+1 < keys.length) {
                        hmapAdapterList.get(simID).get(label).add(new String[] {
                            keys[i],
                            (String) (hmapLabelsList.get(simID).get(label).get(keys[i]) == null ?
                                 "" : hmapLabelsList.get(simID).get(label).get(keys[i])),
                            keys[i + 1],
                            (String) (hmapLabelsList.get(simID).get(label).get(keys[i+1]) == null ?
                                 "" : hmapLabelsList.get(simID).get(label).get(keys[i+1]))});
                    } else {
                        hmapAdapterList.get(simID).get(label).add(new String[] {
                            keys[i],
                            (String) (hmapLabelsList.get(simID).get(label).get(keys[i]) == null ?
                                 "" : hmapLabelsList.get(simID).get(label).get(keys[i])),
                            "",
                            ""});
                    }
                }
            }
        }
    }

    abstract ArrayList<LinkedHashMap> initHashMapValues();
    abstract boolean isLabelArrayType(String label);
    abstract String[] initLabels();
    abstract void registeListener();
    abstract void unRegisteListener();
    abstract void startUpdateProcess(Task task);
}
