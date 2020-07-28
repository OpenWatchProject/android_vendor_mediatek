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

package com.mediatek.engineermode.mdmdiagnosticinfo;

import android.R.integer;
import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.mdmcomponent.MdmTextView;
import com.mediatek.mdml.Msg;
import com.mediatek.mdml.PlainDataDecoder;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;
import java.util.Set;

public class MdmComponentUtils {

    public static final String TAG = "EM_DiagnosticMetric_Info";
    private static final String GROUP_RADIO_STACK = "1. Radio Stack";
    private static final String GROUP_NATIVE_IMS_STACK = "2. Native IMS Stack";
    protected Activity mActivity;

    private static List<MdmBaseComponent> mComponents;

    public static List<MdmBaseComponent> getComponents(Activity mActivity) {
        if(mComponents == null || mComponents.size() == 0) {
            initComponentsList(mActivity);
        }
        return mComponents;
    }

    private static void initComponentsList(Activity mActivity) {
        HashMap<String, List<Object>> mComponentList =
                new LinkedHashMap<String, List<Object>>();
        mComponents = new ArrayList<MdmBaseComponent>();
        /* mComponentList.put("GSM/UMTS Circuit Switched Call Start",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_CC_CALL_INFO_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new GSMUMTSCallStartCallBack());
                }});
        mComponentList.put("GSM/UMTS Circuit Switched Call State Transition",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_CC_CALL_INFO_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new GSMUMTSCallStateTransitionCallBack());
                }});
        mComponentList.put("GSM/UMTS Circuit Switched Call End",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_CC_CALL_INFO_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new GSMUMTSCallEndCallBack());
                }}); */
        mComponentList.put("GSM/GPRS/UMTS Network Info",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_MM_INFO_IND, -1L);
                            put(MDMContent.MSG_ID_FDD_EM_CSCE_SERV_CELL_S_STATUS_IND, -1L);
                            put(MDMContent.MSG_ID_EM_SLCE_SRNCID_STATUS_IND, -1L);
                            put(MDMContent.MSG_ID_EM_L4C_MDMI_RAT_INFO_IND, -1L);
                            put(MDMContent.MSG_ID_EM_RRM_MEASUREMENT_REPORT_INFO_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new GSMUMTSNetworkInfoCallBack());
                }});
        mComponentList.put("UMTS/HSPA RF Info",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_FDD_EM_CSCE_SERV_CELL_S_STATUS_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new UMTSHSPARFInfoCallBack());
                }});
        mComponentList.put("UMTS/HSPA Active and Monitored Set Info",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_FDD_EM_MEME_DCH_UMTS_CELL_INFO_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new ArrayTableBuilder());
                    add(new UMTSHSPAActiveMonitoredSetInfoCallBack());
                }});
        mComponentList.put("UMTS/HSPA Radio Link Sync Status",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_UL1_RADIO_LINK_SYNC_STATUS_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new ArrayTableBuilder());
                    add(new UMTSHSPARadioLinkSyncStatusCallBack());
                }});
        mComponentList.put("E-UTRA RRC State",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_NAME_ERRC_STATE_CHANGE_EVENT,
                                    MDMContent.MSG_ID_ERRC_STATE_CHANGE_EVENT);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_ICD_EVENT);
                    add(new NormalTableBuilder());
                    add(new EUTRARRCStateCallBack());
                }});
        mComponentList.put("E-UTRA EPS Bearer Context Status",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_NAME_ENAS_ESM_CONTEXT_INFO,
                                    MDMContent.MSG_ID_ENAS_ESM_CONTEXT_INFO);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_ICD_EVENT);
                    add(new ArrayTableBuilder());
                    add(new EUTRAEPSBearerContextCallBack());
                }});
        mComponentList.put("E-UTRA PUSCH Transmission Status",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_NAME_EL1_PUSCH_REPORT,
                                    MDMContent.MSG_ID_EL1_PUSCH_REPORT);
                            put(MDMContent.MSG_NAME_EL1_PUSCH_CSF,
                                    MDMContent.MSG_ID_EL1_PUSCH_CSF);
                            put(MDMContent.MSG_NAME_EL1_PUCCH_CSF,
                                    MDMContent.MSG_ID_EL1_PUCCH_CSF);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_ICD_RECORD);
                    add(new ArrayTableBuilder());
                    add(new EUTRAPUSCHTransmissionStatusCallBack());
                }});
        mComponentList.put("E-UTRA Radio Link Sync Status",
                        new ArrayList<Object>() {{
                            add(GROUP_RADIO_STACK);
                            add(new HashMap<String, Long>() {
                                {
                                    put(MDMContent.MSG_NAME_EL1_RADIO_LINK_MONITORING,
                                            MDMContent.MSG_ID_EL1_RADIO_LINK_MONITORING);
                                }
                            });
                            add(MDMCoreOperation.MDML_TRAP_TYPE_ICD_EVENT);
                            add(new NormalTableBuilder());
                            add(new EUTRARadioLinkSyncStatusCallBack());
                        }});
        mComponentList.put("E-UTRA RLC Data Transfer Report",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_NAME_EL2_RLC_UL_STATS,
                                    MDMContent.MSG_ID_EL2_RLC_UL_STATS);
                            put(MDMContent.MSG_NAME_EL2_RLC_DL_STATS,
                                    MDMContent.MSG_ID_EL2_RLC_DL_STATS);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_ICD_RECORD);
                    add(new ArrayTableBuilder());
                    add(new EUTRARLCDataTransferReportCallBack());
                }});
        mComponentList.put("E-UTRA Measurement Report",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_NAME_ERRC_SERVING_CELL_INFO,
                                    MDMContent.MSG_ID_ERRC_SERVING_CELL_INFO);
                            put(MDMContent.MSG_NAME_ERRC_MEAS_REPORT_INFO,
                                    MDMContent.MSG_ID_ERRC_MEAS_REPORT_INFO);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_ICD_RECORD);
                    add(new NormalTableBuilder());
                    add(new EUTRAMeasurementReportCallBack());
                }});
        mComponentList.put("E-UTRA MAC Random Access Attempt",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_NAME_EL2_MAC_RACH_ATTEMPT_EVENT,
                                    MDMContent.MSG_ID_EL2_MAC_RACH_ATTEMPT_EVENT);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_ICD_EVENT);
                    add(new NormalTableBuilder());
                    add(new EUTRAMACRandomAccessAttemptCallBack());
                }});
        mComponentList.put("E-UTRA Physical Data Transfer Report",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_NAME_EL1_MIMO_PDSCH_THROUGHPUT0,
                                    MDMContent.MSG_ID_EL1_MIMO_PDSCH_THROUGHPUT0);
                            put(MDMContent.MSG_NAME_EL1_MIMO_PDSCH_THROUGHPUT1,
                                    MDMContent.MSG_ID_EL1_MIMO_PDSCH_THROUGHPUT1);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_ICD_RECORD);
                    add(new ArrayTableBuilder());
                    add(new EUTRAPhysicalDataTransferReportCallBack());
                }});
        mComponentList.put("E-UTRA Carrier Aggregation Event",
                new ArrayList<Object>() {{
                    add(GROUP_RADIO_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_NAME_ERRC_CA_CONFIG_INFO,
                                    MDMContent.MSG_ID_ERRC_CA_CONFIG_INFO);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_ICD_RECORD);
                    add(new ArrayTableBuilder());
                    add(new EUTRACarrierAggregationEventCallBack());
                }});
        mComponentList.put("SIP Registration Attempt (VoLTE and VoWiFi)",
                new ArrayList<Object>() {{
                    add(GROUP_NATIVE_IMS_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_IMC_REG_INFO_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new SIPRegistrationAttemptCallBack());
                }});
        mComponentList.put("SIP Registration State (VoLTE and VoWiFi)",
                new ArrayList<Object>() {{
                    add(GROUP_NATIVE_IMS_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_IMC_REG_INFO_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new SIPRegistrationStateCallBack());
                }});
        mComponentList.put("SIP Registration Result (VoLTE and VoWiFi)",
                new ArrayList<Object>() {{
                    add(GROUP_NATIVE_IMS_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_IMC_REG_INFO_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new SIPRegistrationResultCallBack());
                }});
        mComponentList.put("SIP Session Start (VoLTE and VoWiFi)",
                new ArrayList<Object>() {{
                    add(GROUP_NATIVE_IMS_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_IMC_CALL_INFO_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new SIPSessionStartCallBack());
                }});
        mComponentList.put("SIP Session State (VoLTE and VoWiFi)",
                new ArrayList<Object>() {{
                    add(GROUP_NATIVE_IMS_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_IMC_CALL_INFO_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new SIPSessionStateCallBack());
                }});
        mComponentList.put("SIP Session End (VoLTE and VoWiFi)",
                new ArrayList<Object>() {{
                    add(GROUP_NATIVE_IMS_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_IMC_CALL_INFO_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new SIPSessionEndCallBack());
                }});
        mComponentList.put("SIP Session Statistics (VoLTE and VoWiFi)",
                new ArrayList<Object>() {{
                    add(GROUP_NATIVE_IMS_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_LTECSR_SESSION_STAT_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new ArrayTableBuilder());
                    add(new SIPSessionStatisticsCallBack());
                }});
        mComponentList.put(" SIP Message Received (VoLTE and VoWiFi)",
                new ArrayList<Object>() {{
                    add(GROUP_NATIVE_IMS_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_IMC_SIP_INFO_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new SIPMessageReceivedCallBack());
                }});
        mComponentList.put("SIP Message Transmitted (VoLTE and VoWiFi)",
                new ArrayList<Object>() {{
                    add(GROUP_NATIVE_IMS_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_IMC_SIP_INFO_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new SIPMessageTransmittedCallBack());
                }});
        mComponentList.put("RTP Transmit Packet (VoLTE and VoWiFi)",
                new ArrayList<Object>() {{
                    add(GROUP_NATIVE_IMS_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_LTECSR_XMIT_PKT_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new RTPTransmitPacketCallBack());
                }});
        mComponentList.put("RTP Receive Packet (VoLTE and VoWiFi)",
                new ArrayList<Object>() {{
                    add(GROUP_NATIVE_IMS_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_LTECSR_RECV_PKT_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new RTPReceivePacketCallBack());
                }});
        mComponentList.put("RTP Transmit Statistics (VoLTE and VoWiFi)",
                new ArrayList<Object>() {{
                    add(GROUP_NATIVE_IMS_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_LTECSR_XMIT_STAT_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new RTPTransmitStatisticsCallBack());
                }});
        mComponentList.put("RTP Receive Statistics (VoLTE and VoWiFi)",
                new ArrayList<Object>() {{
                    add(GROUP_NATIVE_IMS_STACK);
                    add(new HashMap<String, Long>() {
                        {
                            put(MDMContent.MSG_ID_EM_LTECSR_RECV_STAT_IND, -1L);
                        }
                    });
                    add(MDMCoreOperation.MDML_TRAP_TYPE_EM);
                    add(new NormalTableBuilder());
                    add(new RTPReceiveStatisticsCallBack());
                }});

        for(String key: mComponentList.keySet()){
            mComponents.add(new MdmComponent(mActivity, key, mComponentList.get(key)));
        }
    }
}

abstract class MdmBaseComponent {

    abstract View getView();

    abstract HashMap<String, Long> getEmComponentMap();

    abstract void removeView();

    abstract String getName();

    abstract int getTrapType();

    abstract String getGroup();

    abstract Object[] getLabels();

    abstract public boolean supportMultiSIM();

    abstract public IMdmViewUpdateCallBack getCallBack();

    abstract public IMdmViewBuilder getBuilder();

    abstract public Activity getContext();

    abstract public void resetView();

    abstract public void test();

    abstract public void update(Object msgName, Object objectMsg);

    public int getFieldValue(Msg data, String msgName) {
        byte[] bData = data.getFieldValue(msgName);
        int iData = (int) Utils.getIntFromByte(bData);

        return iData;
    }

    public int getFieldValue(Msg data, String msgName, boolean sign) {
        if(!sign) {
            return getFieldValue(data, msgName);
        }
        byte[] bData = data.getFieldValue(msgName);
        int iData = (int) Utils.getIntFromByte(bData, true);
        return iData;
    }

    public long getFieldLongValue(Msg data, String msgName) {
        byte[] bData = data.getFieldValue(msgName);
        long iData = Utils.getIntFromByte(bData);
        return iData;
    }


    public long getFieldLongValue(Msg data, String msgName, boolean sign) {
     //   if(!sign) {
     //       return getFieldLongValue(data, msgName);
     //   }
        byte[] bData = data.getFieldValue(msgName);
        long iData = (int) Utils.getIntFromByte(bData, true);
        return iData;
    }

//    public long getFieldLongValue(ICDMsg icdMsg, String fieldName) {
//        ICDFieldValue fieldValue = icdMsg.GetPayloadFieldValue(fieldName);
//        Elog.d(MdmComponentUtils.TAG, "[MdmComponentUtils] " + fieldName +
//                " ICD return code:value=" + fieldValue.return_code + ":"+ fieldValue.value);
//        if(fieldValue.return_code == 1){
//            Elog.d(MdmComponentUtils.TAG, "[MdmComponentUtils] ICD return value: "
//                    + fieldValue.value);
//            return fieldValue.value;
//        }
//        return 0;
 //   }

//    public int getFieldValue(ICDMsg icdMsg, String fieldName) {
//        ICDFieldValue fieldValue = icdMsg.GetPayloadFieldValue(fieldName);
//        Elog.d(MdmComponentUtils.TAG, "[MdmComponentUtils] " + fieldName +
//                " ICD return code:value=" + fieldValue.return_code + ":"+ fieldValue.value);
//        if(fieldValue.return_code == 1){
//            return (int)fieldValue.value;
//        }
//        return 0;
//    }
}

class MdmComponent extends MdmBaseComponent implements Comparable<MdmBaseComponent> {
    private Activity mActivity;
    private View view;
    private String name;
    private String group;
    private int trapType;
    private HashMap<String, Long> mComponentMap;

    private IMdmViewBuilder viewBuilder;
    private IMdmViewUpdateCallBack viewUpdateCallBack;

    @Override
    public Object[] getLabels() {
        if (viewUpdateCallBack != null) {
            return viewUpdateCallBack.getLabels();
        }
        return new String[0];
    }

    public MdmComponent(Activity activity, String name, int trapType,
            String group, HashMap<String, Long>mComponentMap) {
        this.mActivity = activity;
        this.trapType = trapType;
        this.name = name;
        this.group = group;
        this.mComponentMap = mComponentMap;
    }

    public MdmComponent(Activity activity, String name, List<Object> list) {
        // TODO Auto-generated constructor stub
        this.mActivity = activity;
        this.name = name;
        this.group = (String) list.get(0);
        this.mComponentMap = (HashMap<String, Long>)list.get(1);
        this.trapType = (int)list.get(2);
        this.viewBuilder = (IMdmViewBuilder) list.get(3);
        this.viewBuilder.setMdmComponent(this);
        this.viewUpdateCallBack = (IMdmViewUpdateCallBack) list.get(4);
        this.viewUpdateCallBack.setMdmComponent(this);
    }

    public void setCallBack(IMdmViewUpdateCallBack callBack) {
        this.viewUpdateCallBack = callBack;
    }

    public int getTrapType(){
        return this.trapType;
    }

    @Override
    public Activity getContext() {
        return this.mActivity;
    }

    @Override
    public View getView() {
        return viewBuilder.constructView();
    }

    @Override
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    @Override
    public String getGroup() {
        return group;
    }

    public void setGroup(String group) {
        this.group = group;
    }

    @Override
    public HashMap<String, Long> getEmComponentMap() {
        return mComponentMap;
    }

    public void setEmComponentMap(HashMap<String, Long> mComponentMap) {
        this.mComponentMap = mComponentMap;
    }

    public IMdmViewBuilder getBuilder() {
        return this.viewBuilder;
    }

    public void setBuilder(IMdmViewBuilder mdmViewBuilder) {
        this.viewBuilder = mdmViewBuilder;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        // TODO Auto-generated method stub
        viewUpdateCallBack.update(msgName, objectMsg);
    }

    @Override
    public boolean supportMultiSIM() {
        return viewUpdateCallBack.supportMultiSIM();
    }

    boolean hasTrapType(String msgName, int msgId) {
        Set<String> msgNames = getEmComponentMap().keySet();
        Collection<Long> msgIds = getEmComponentMap().values();
        if (msgNames != null && msgNames.contains(msgName)) {
            return true;
        }
        if (msgId != -1 && msgIds.contains(Long.valueOf(msgId))) {
            return true;
        }
        return false;
    }

    String[] addLablesAtPosition(String[] a, String[] b, int position) {
        String[] c = new String[a.length + b.length];
        if (position >= a.length) {
            System.arraycopy(a, 0, c, 0, a.length);
            System.arraycopy(b, 0, c, a.length, b.length);
        } else {
            for (int i = 0; i < a.length; i++) {
                if (i < position) {
                    c[i] = a[i];
                } else if (i == position) {
                    for (int j = 0; j < b.length; j++) {
                        c[i + j] = b[j];
                    }
                    c[i + b.length] = a[i];
                } else {
                    c[i + b.length] = a[i];
                }
            }
        }
        return c;
    }

    String[] removeLablesAtPosition(String[] a, int position, int num) {
        if (position >= a.length) {
            return a;
        }
        String[] c = new String[a.length - num];
        for (int i = 0; i < a.length - num; i++) {
            if (i < position) {
                c[i] = a[i];
            } else if (i == position) {
                if ((a.length - position) <= num) {
                    break;
                }
                c[i] = a[i + num];
            } else {
                c[i] = a[i + num];
            }
        }
        return c;
    }

    @Override
    void removeView() {
        viewBuilder.removeView();
    }

    @Override
    public int compareTo(MdmBaseComponent another) {
        return getName().compareTo(another.getName());
    }

    @Override
    public IMdmViewUpdateCallBack getCallBack() {
        return this.viewUpdateCallBack;
    }

    @Override
    public void resetView() {
        viewBuilder.resetView();
        viewUpdateCallBack.resetValues();
    }

    @Override
    public void test() {
        // TODO Auto-generated method stub
        viewUpdateCallBack.test();
    }

}

interface IMdmViewBuilder {

    public void setMdmComponent(MdmComponent mdmComponent);
    public void resetView();
    public View constructView();
    public void removeView();
    public void clearData();
    public void updateDataOnView(LinkedHashMap<String, Object> updateValues);
};

interface IMdmViewUpdateCallBack {

    public void update(Object msgName, Object objectMsg);
    public boolean supportMultiSIM();
    public Object[] getLabels();
    public void resetValues();
    public void test();
    public void setMdmComponent(MdmBaseComponent mdmComponent);
};

interface ICurveViewUpdateCallBack extends IMdmViewUpdateCallBack{
    public CurveView.AxisConfig configY();
};

interface ICurveViewExUpdateCallBack extends IMdmViewUpdateCallBack{
    public CurveViewEx.AxisConfig configY();
    public CurveViewEx.AxisConfig configX();
};

class TableInfoAdapter extends ArrayAdapter<String[]> {
    private Activity mActivity;

    public TableInfoAdapter(Activity activity) {
        super(activity, 0);
        this.mActivity = activity;
    }

    private class ViewHolder {
        public MdmTextView[] texts;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder;
        LayoutInflater inflater = mActivity.getLayoutInflater();
        if (convertView == null) {
            convertView = inflater.inflate(R.layout.em_info_entry, null);
            holder = new ViewHolder();
            holder.texts = new MdmTextView[16];
            holder.texts[0] = (MdmTextView) convertView.findViewById(R.id.info1);
            holder.texts[1] = (MdmTextView) convertView.findViewById(R.id.info2);
            holder.texts[2] = (MdmTextView) convertView.findViewById(R.id.info3);
            holder.texts[3] = (MdmTextView) convertView.findViewById(R.id.info4);
            holder.texts[4] = (MdmTextView) convertView.findViewById(R.id.info5);
            holder.texts[5] = (MdmTextView) convertView.findViewById(R.id.info6);
            holder.texts[6] = (MdmTextView) convertView.findViewById(R.id.info7);
            holder.texts[7] = (MdmTextView) convertView.findViewById(R.id.info8);
            holder.texts[8] = (MdmTextView) convertView.findViewById(R.id.info9);
            holder.texts[9] = (MdmTextView) convertView.findViewById(R.id.info10);
            holder.texts[10] = (MdmTextView) convertView.findViewById(R.id.info11);
            holder.texts[11] = (MdmTextView) convertView.findViewById(R.id.info12);
            holder.texts[12] = (MdmTextView) convertView.findViewById(R.id.info13);
            holder.texts[13] = (MdmTextView) convertView.findViewById(R.id.info14);
            holder.texts[14] = (MdmTextView) convertView.findViewById(R.id.info15);
            holder.texts[15] = (MdmTextView) convertView.findViewById(R.id.info16);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }
        String[] text = getItem(position);
        for (int i = 0; i<text.length && i<holder.texts.length; i++) {
            holder.texts[i].setText(text[i]);
            holder.texts[i].setVisibility(View.VISIBLE);
        }
        for (int i = text.length; i<holder.texts.length; i++) {
            holder.texts[i].setVisibility(View.GONE);
        }
        return convertView;
    }
}

class NormalTableBuilder implements IMdmViewBuilder{

    ListView mListView;
    TableInfoAdapter mAdapter;
    MdmBaseComponent mdmComponent;
    String[] mLabels;

    @Override
    public View constructView() {
        // TODO Auto-generated method stub
        mLabels = (String[]) mdmComponent.getLabels();
        if (mAdapter.getCount() == 0) {
            // Show placeholder to avoid showing empty list
            for (int i = 0; i < mLabels.length; i++) {
                mAdapter.add(new String[] { mLabels[i], "" });
            }
        }
        mListView.setAdapter(mAdapter);
        mAdapter.notifyDataSetChanged();
        return mListView;
    }

    @Override
    public void removeView() {
        mListView.setAdapter(null);
    }

    public void clearData() {
        mAdapter.clear();
    }

    void notifyDataSetChanged() {
        if (mLabels == null) {
            mLabels = (String[]) mdmComponent.getLabels();
        }
        if (mAdapter.getCount() < mLabels.length) {
            // Show placeholder to avoid showing incomplete list
            for (int i = mAdapter.getCount(); i < mLabels.length; i++) {
                mAdapter.add(new String[] { mLabels[i], "" });
            }
        }
        mAdapter.notifyDataSetChanged();
    }

    @Override
    public void setMdmComponent(MdmComponent mdmComponent) {
        this.mdmComponent = mdmComponent;
        if (mAdapter == null) {
            mAdapter = new TableInfoAdapter(mdmComponent.getContext());
        }
        if (mListView == null) {
            mListView = new ListView(mdmComponent.getContext());
        }
    }


    @Override
    public void updateDataOnView(LinkedHashMap<String, Object> updateValues) {
        // TODO Auto-generated method stub
        clearData();
        mLabels = (String[]) mdmComponent.getLabels();
        for(int i=0; i<mLabels.length; i++) {
            mAdapter.add(new String[] { mLabels[i],
                    updateValues.get(mLabels[i]) == null ? "" :
                        updateValues.get(mLabels[i]).toString() });
        }
        mAdapter.notifyDataSetChanged();
    }

    @Override
    public void resetView() {
        // TODO Auto-generated method stub
        clearData();
        removeView();
    }

}

class ArrayTableBuilder implements IMdmViewBuilder {
    ListView mListView;
    TableInfoAdapter mAdapter;
    ArrayList<String[]> mLabels = new ArrayList<String[]>();
    MdmBaseComponent mdmComponent;

    public void initLabels() {
        mLabels.clear();
        Object[] tempLabels = mdmComponent.getLabels();
        for(int i=0; i<tempLabels.length; i++) {
            if(tempLabels[i].getClass().isArray()){
                mLabels.add((String[]) tempLabels[i]);
            } else {
                mLabels.add((String[]) tempLabels);
                break;
            }
        }
    }

    @Override
    public View constructView() {
        initLabels();
        if (mAdapter.getCount() == 0) {
            // Show placeholder to avoid showing empty list
            for(String[] labels : mLabels){
                mAdapter.add(labels);
            }

        }
        mListView.setAdapter(mAdapter);
        mAdapter.notifyDataSetChanged();
        return mListView;
    }

    @Override
    public void removeView() {
        mListView.setAdapter(null);
    }

    public void clearData() {
        initLabels();
        mAdapter.clear();
        for(String[] labels : mLabels){
            mAdapter.add(labels);
        }
        mAdapter.notifyDataSetChanged();
    }

    @Override
    public void setMdmComponent(MdmComponent mdmComponent) {
        this.mdmComponent = mdmComponent;
        if (mAdapter == null) {
            mAdapter = new TableInfoAdapter(mdmComponent.getContext());
        }
        if (mListView == null) {
            mListView = new ListView(mdmComponent.getContext());
        }
    }

    @Override
    public void updateDataOnView(LinkedHashMap<String, Object> updateValues) {
        initLabels();
        mAdapter.clear();
        if(mLabels == null || mLabels.size() == 0) return;
        if(mLabels.size() == 1) {
            mAdapter.add(mLabels.get(0));
            for(String key: updateValues.keySet()) {
                if(updateValues.get(key)!= null &&
                        updateValues.get(key) instanceof String[]) {
                    String[] datas = (String[])updateValues.get(key);
                    mAdapter.add(datas);
                } else if(updateValues.get(key)!= null &&
                        updateValues.get(key) instanceof Object[]) {
                    String[] datas = new String[((Object[])updateValues.get(key)).length];
                    for(int i=0; i<datas.length; i++){
                        datas[i] = ((Object[])updateValues.get(key))[i] != null ? "" :
                            ((Object[])updateValues.get(key))[i].toString();
                    }
                    mAdapter.add(datas);
                } else if(updateValues.get(key)!= null &&
                        updateValues.get(key) instanceof int[]) {
                    String[] datas = new String[((int[])updateValues.get(key)).length];
                    for(int i=0; i<datas.length; i++){
                        datas[i] = ((int[])updateValues.get(key))[i]+"";
                    }
                    mAdapter.add(datas);
                } else {
                    String[] datas = new String[] {""};
                    mAdapter.add(datas);
                }
            }
        } else {
            int k = 0;
            int curTableCount = 0, lastCurTableCount = 0, labelIndex = 0;
            for(String key: updateValues.keySet()) {
                if(!updateValues.get(key).getClass().isArray() && key.contains("Num")){
                    curTableCount = (Integer) updateValues.get(key);
                    if (labelIndex > 0 && lastCurTableCount > 0) mAdapter.add(new String[]{""});
                    mAdapter.add((String[]) mLabels.get(labelIndex));
                    if(curTableCount == 0 && ((String[]) mLabels.get(labelIndex)).length > 1) {
                        mAdapter.add(new String[] {""});
                    }
                    k = 0;
                    labelIndex++;
                    lastCurTableCount = curTableCount;
                } else if(curTableCount > 0 && k < curTableCount){
                    String[] datas;
                    if(updateValues.get(key) instanceof String[]) {
                        datas = (String[])(updateValues.get(key));
                        mAdapter.add(datas);
                    } else if(updateValues.get(key) instanceof int[]){
                        int[] values = (int[])(updateValues.get(key));
                        datas = new String[values.length];
                        for(int j=0; j<datas.length; j++){
                           datas[j] = values.length == 0 ? "" :
                                values[j] + "";
                        }
                        mAdapter.add(datas);
                    } else if(updateValues.get(key) instanceof Object[]){
                        Object[] values = (Object[])(updateValues.get(key));
                        datas = new String[values.length];
                        for(int j=0; j<datas.length; j++){
                            datas[j] = values.length == 0 ? "" :
                                values[j] + "";
                        }
                        mAdapter.add(datas);
                    } else {
                        mAdapter.add(new String[] {""});
                    }
                    k++;
                }
            }
        }

        mAdapter.notifyDataSetChanged();
    }

    @Override
    public void resetView() {
        clearData();
        removeView();
    }

}

class CurveViewBuilder implements IMdmViewBuilder {
    CurveView mCurveView;
    View mView;
    TextView mXLabel;
    TextView mYLabel;
    HashMap<Integer, float[]> mData = new HashMap<Integer, float[]>();
    Random rand = new Random();
    long mStartTime;
    boolean mStarted = false;
    MdmBaseComponent mdmComponent;

    @Override
    public void setMdmComponent(MdmComponent mdmComponent){
        this.mdmComponent = mdmComponent;
    }

    @Override
    public View constructView() {
        if (mView == null) {
            Elog.d(MdmComponentUtils.TAG, "[CurveComponent] mView is null!");
            mView = mdmComponent.getContext().getLayoutInflater().inflate(
                    R.layout.mdm_diagnostic_info_curve, null);
            mXLabel = (TextView) mView.findViewById(R.id.em_info_curve_x_label);
            mYLabel = (TextView) mView.findViewById(R.id.em_info_curve_y_label);
            mCurveView = (CurveView) mView.findViewById(R.id.em_info_curve);
            if (!mStarted) {
                mStartTime = System.currentTimeMillis() / 10000 * 10;
                mStarted = true;
            }

            mXLabel.setText("Time");
            CurveView.AxisConfig xConfig = new CurveView.AxisConfig();
            xConfig.base = mStartTime;
            xConfig.min = 0;
            xConfig.max = 200;
            xConfig.step = 1;
            xConfig.configMin = true;
            xConfig.configMax = true;
            xConfig.configStep = true;
            xConfig.type = CurveView.AxisConfig.TYPE_TIME;
            CurveView.AxisConfig yConfig = this.mdmComponent.getCallBack()
                    instanceof ICurveViewUpdateCallBack ?
                            ((ICurveViewUpdateCallBack)this.mdmComponent.getCallBack()).configY() :
                                configY();
            mCurveView.setAxisConfig(xConfig, yConfig);
        }
        return mView;
    }

    CurveView.AxisConfig configY() {
        mYLabel.setText("dBm");

        CurveView.Config[] configs = new CurveView.Config[1];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF880000;
        configs[0].lineWidth = 2;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = 0;
        yConfig.max = 30;
        yConfig.step = 2;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    public void removeView() {

    }

    @Override
    public void updateDataOnView(LinkedHashMap<String, Object> updateValues) {

        int index = (Integer.valueOf(updateValues.get("index").toString()));
        float data = (Float.valueOf(updateValues.get("data").toString()));
        if (!mStarted) {
            mStartTime = System.currentTimeMillis() / 10000 * 10;
            mStarted = true;
        }
        long time = System.currentTimeMillis() / 1000;

        float[] d = mData.get(index);
        if (d == null) {
            float[] tmp = new float[2];
            tmp[0] = time - mStartTime;
            tmp[1] = data;
            mData.put(index, tmp);
        } else {

            float[] tmp = new float[d.length + 2];
            for (int i = 0; i < d.length; i++) {
                tmp[i] = d[i];
            }
            tmp[d.length] = time - mStartTime;
            tmp[d.length + 1] = data;
            mData.put(index, tmp);
        }
        if (mCurveView != null) {
            mCurveView.setData(index, mData.get(index));
        }
    }

    @Override
    public void clearData() {
        mData.clear();
    }

    @Override
    public void resetView() {
        mView = null;
        mStarted = false;
        clearData();
    }
}

class CurveExViewBuilder implements IMdmViewBuilder {
    CurveViewEx mCurveView;
    View mView;
    TextView mXLabel;
    TextView mYLabel;
    HashMap<Integer, float[]> mData = new HashMap<Integer, float[]>();
    MdmBaseComponent mdmComponent;

    @Override
    public void setMdmComponent(MdmComponent mdmComponent) {
        this.mdmComponent = mdmComponent;
    }

    @Override
    public View constructView() {
        if (mView == null) {
            mView = mdmComponent.getContext().getLayoutInflater().inflate(
                    R.layout.mdm_diagnostic_info_curveex, null);
            mXLabel = (TextView) mView.findViewById(R.id.em_info_curve_x_label);
            mYLabel = (TextView) mView.findViewById(R.id.em_info_curve_y_label);
            mCurveView = (CurveViewEx) mView.findViewById(R.id.em_info_curve);

            CurveViewEx.AxisConfig xConfig = this.mdmComponent.getCallBack()
                    instanceof ICurveViewExUpdateCallBack ?
                    ((ICurveViewExUpdateCallBack)this.mdmComponent.getCallBack()).configX() :
                    configX();
            CurveViewEx.AxisConfig yConfig = this.mdmComponent.getCallBack()
                    instanceof ICurveViewExUpdateCallBack ?
                    ((ICurveViewExUpdateCallBack)this.mdmComponent.getCallBack()).configY() :
                        configY();
            mCurveView.setAxisConfig(xConfig, yConfig);
            mCurveView.getRSRPSINRConfig();
        }
        return mView;
    }

    CurveViewEx.AxisConfig configX() {
        mXLabel.setText("RSRP");
        CurveViewEx.AxisConfig xConfig = new CurveViewEx.AxisConfig();
        xConfig.min = -140;
        xConfig.max = -30;
        xConfig.step = 10;
        xConfig.configMin = true;
        xConfig.configMax = true;
        xConfig.configStep = true;
        return xConfig;
    }

    CurveViewEx.AxisConfig configY() {
        mYLabel.setText("SNR");
        CurveViewEx.Config[] configs = new CurveViewEx.Config[5];
        configs[0] = new CurveViewEx.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveViewEx.Config.TYPE_CIRCLE;
        configs[0].name = "PCell";
        configs[1] = new CurveViewEx.Config();
        configs[1].color = 0xFFFF0000;
        configs[1].lineWidth = 3;
        configs[1].nodeType = CurveViewEx.Config.TYPE_CIRCLE;
        configs[1].name = "SCell";
        configs[2] = new CurveViewEx.Config();
        configs[2].color = Color.rgb(43, 101, 171);
        configs[2].lineWidth = 3;
        configs[2].nodeType = CurveViewEx.Config.TYPE_RHOMBUS;
        configs[2].name = "Strength";
        configs[3] = new CurveViewEx.Config();
        configs[3].color = Color.rgb(204, 153, 0);
        configs[3].lineWidth = 3;
        configs[3].nodeType = CurveViewEx.Config.TYPE_SQUARE;
        configs[3].name = "MediumWeak";
        configs[4] = new CurveViewEx.Config();
        configs[4].color = Color.rgb(152, 152, 186);
        configs[4].lineWidth = 3;
        configs[4].nodeType = CurveViewEx.Config.TYPE_TRIANGLE;
        configs[4].name = "Weak";
        mCurveView.setConfig(configs);
        CurveViewEx.AxisConfig yConfig = new CurveViewEx.AxisConfig();
        yConfig.min = -20;
        yConfig.max = 30;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    public void removeView() {
         mCurveView = null;
    }

    public void clearData() {
        mData.clear();
    }

    @Override
    public void resetView() {
        mView = null;
        clearData();
    }

    @Override
    public void updateDataOnView(LinkedHashMap<String, Object> updateValues) {
        clearData();
        int index = (Integer.valueOf(updateValues.get("index").toString()));
        float dataX = (Float.valueOf(updateValues.get("dataX").toString()));
        float dataY = (Float.valueOf(updateValues.get("dataY").toString()));
        float dataA = (Float.valueOf(updateValues.get("dataA").toString()));
        float dataB = (Float.valueOf(updateValues.get("dataB").toString()));
        float[] d = mData.get(index);
        if (d == null) {
            float[] tmp = new float[4];
            tmp[0] = dataX;
            tmp[1] = dataY;
            tmp[2] = dataA;
            tmp[3] = dataB;
            mData.put(index, tmp);
        } else {

            float[] tmp = new float[d.length + 4];
            for (int i = 0; i < d.length; i++) {
                tmp[i] = d[i];
            }
            tmp[d.length] = dataX;
            tmp[d.length + 1] = dataY;
            tmp[d.length + 2] = dataA;
            tmp[d.length + 3] = dataB;
            mData.put(index, tmp);
        }
        if (mCurveView != null) {
            mCurveView.setData(index, mData.get(index));
        }
    }
}

class GSMUMTSCallStartCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "dwCallId", "ucCallAttr", "ucCallDir",
            "ucCallMode", "szNumber" };
    private final HashMap<Integer, String> mStateMapping = new HashMap<Integer, String>() {
        {
            put(0, "UNKNOWN");
            put(1, "IDLE");
            put(2, "STAGING");
            put(3, "ATTEMPTING");
            put(4, "ESTABLISHED");
            put(5, "CONNECTED");
            put(6, "DISCONNECTING");
            put(7, "FAILED");
            put(8, "HELD");
            put(9, "TRANSFERRING");
        }
    };

    private final HashMap<Integer, String> mCallDirMapping = new HashMap<Integer, String>() {
        {
            put(0, "Outgoing");
            put(1, "Incoming");
        }
    };

    private final HashMap<Integer, String> mCallModeMapping = new HashMap<Integer, String>() {
        {
            put(0, "Speech");
            put(1, "Video");
        }
    };

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        int dwCallId = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_CALL_ID);
        int ucCallMode = mMdmComponent.getFieldValue(data,  MDMContent.MSG_VALUE_CALL_MODE);
        int ucCallDir= mMdmComponent.getFieldValue(data,  MDMContent.MSG_VALUE_DIR);
        int ucCallState = mMdmComponent.getFieldValue(data,  MDMContent.MSG_VALUE_STATE);
        int szNumber = mMdmComponent.getFieldValue(data,  MDMContent.MSG_VALUE_CALL_NUM +
                "." + MDMContent.MSG_VALUE_ADDR_BCD +
                "[" +  MDMContent.MSG_VALUE_MAX_CC_ADDR_BCD_LEN + "]");
        updateValues.clear();
        String stateString = mStateMapping.get(ucCallState);
        updateValues.put(mLabels[0], dwCallId);
        updateValues.put(mLabels[1], ucCallMode);
        updateValues.put(mLabels[2], mCallDirMapping.containsKey(ucCallDir) ?
                mCallDirMapping.get(ucCallDir) : "-(" + ucCallDir + ")");
        updateValues.put(mLabels[3], mCallModeMapping.containsKey(ucCallState) ?
                mCallModeMapping.get(ucCallState) : "-(" + ucCallState + ")");
        updateValues.put(mLabels[4], szNumber);
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }
}

class GSMUMTSCallStateTransitionCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "dwCallId", "ucCallState"};
    private final HashMap<Integer, String> mStateMapping = new HashMap<Integer, String>() {
        {
            put(0, "UNKNOWN");
            put(1, "IDLE");
            put(2, "STAGING");
            put(3, "ATTEMPTING");
            put(4, "ESTABLISHED");
            put(5, "CONNECTED");
            put(6, "DISCONNECTING");
            put(7, "FAILED");
            put(8, "HELD");
            put(9, "TRANSFERRING");
        }
    };

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        int dwCallId = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_CALL_ID);
        int ucCallState = mMdmComponent.getFieldValue(data,  MDMContent.MSG_VALUE_STATE);
        updateValues.clear();
        String stateString = mStateMapping.containsKey(ucCallState) ?
                mStateMapping.get(ucCallState) : "-(" + ucCallState + ")";
        updateValues.put(mLabels[0], dwCallId);
        updateValues.put(mLabels[1], stateString);
        mMdmComponent.getBuilder().updateDataOnView(updateValues);;
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }
}

class GSMUMTSCallEndCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "dwCallId", "dwErrCode", "wTermCode"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        int dwCallId = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_CALL_ID);
        int dwErrCode = mMdmComponent.getFieldValue(data,  MDMContent.MSG_VALUE_STATE);
        int wTermCode = mMdmComponent.getFieldValue(data,  MDMContent.MSG_VALUE_STATE);
        updateValues.clear();
        updateValues.put(mLabels[0], dwCallId);
        updateValues.put(mLabels[1], dwErrCode);
        updateValues.put(mLabels[2], wTermCode);
        mMdmComponent.getBuilder().updateDataOnView(updateValues);;
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }
}

class GSMUMTSNetworkInfoCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] {"ucRAC", "ucAccessTech",
            "ucFreqBand", "wMCC", "wMNC", "wLAC", "wCellId", "wRNCId"};
    private final HashMap<Integer, String> ucFreqBandMapping =
            new HashMap<Integer, String>() {
            {
                put(1, "UTRA FDD Band1");
                put(2, "UTRA FDD Band2");
                put(3, "UTRA FDD Band3");
                put(4, "UTRA FDD Band4");
                put(5, "UTRA FDD Band5");
                put(6, "UTRA FDD Band6");
                put(7, "UTRA FDD Band7");
                put(8, "UTRA FDD Band8");
                put(9, "UTRA FDD Band9");
                put(10, "UTRA FDD Band10");
                put(11, "UTRA FDD Band11");
                put(12, "UTRA FDD Band12");
                put(13, "UTRA FDD Band13");
                put(14, "UTRA FDD Band14");
                put(15, "UTRA FDD Band15");
                put(16, "UTRA FDD Band16");
                put(17, "UTRA FDD Band17");
                put(18, "UTRA FDD Band18");
                put(19, "UTRA FDD Band19");
                put(20, "UTRA FDD Band20");
                put(21, "UTRA FDD Band21");
                put(22, "UTRA FDD Band22");
                put(23, "UTRA FDD Band23");
                put(24, "UTRA FDD Band24");
                put(25, "UTRA FDD Band25");
                put(26, "UTRA FDD Band26");
            }
    };
    private final HashMap<Integer, String> ucCurBandMapping =
            new HashMap<Integer, String>() {
            {
                put(0, "P_GSM_900");
                put(1, "E_GSM_900");
                put(2, "R_GSM_900");
                put(3, "DCS_1800");
                put(4, "PCS_1900");
                put(5, "GSM_450");
                put(6, "GSM_480");
                put(7, "GSM_850");
            }
    };
    private final HashMap<Integer, String> ucAccessTechMapping =
            new HashMap<Integer, String>() {
            {
                put(-1, "UNKNOWN");
                put(0, "NONE");
                put(1, "GERAN");
                put(2, "UTRAN");
            }
    };
    int ucRAC, ucAccessTech, ucFreqBand, wCellId, wRNCId;
    String wMCC = "";
    String wMNC = "";
    String wLAC = "";

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        if (name.equals(MDMContent.MSG_ID_EM_MM_INFO_IND)) {
            ucRAC = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_RAC);
            wMCC = "0x";
            wMNC = "0x";
            wLAC = "";
            for(int i=0; i<3; i++){
                wMCC += String.format("%x",
                        mMdmComponent.getFieldValue(data,
                                MDMContent.MSG_VALUE_MCC + "[" + i + "]"));
                wMNC += String.format("%x",
                        mMdmComponent.getFieldValue(data,
                                MDMContent.MSG_VALUE_MNC + "[" + i + "]"));
            }
            wMNC = String.format("%x",
                    mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_MNC + "[2]")).
                        contains("ff") ? "" : wMNC ;
            String lac0 = String.format("0x%x",
                    mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_LOC + "[0]"));
            String lac1 = String.format("%x",
                    mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_LOC + "[1]"));
            wLAC = lac0 + lac1;
            updateValues.put(mLabels[0], ucRAC);
            updateValues.put(mLabels[3], wMCC);
            updateValues.put(mLabels[4], wMNC);
            updateValues.put(mLabels[5], wLAC);
        } else if(name.equals(MDMContent.MSG_ID_EM_L4C_MDMI_RAT_INFO_IND)) {
            ucAccessTech = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_MDMI_RAT_INFO);
            String ucAccessTechString = ucAccessTechMapping.get(
                    (ucAccessTech >= 0 && ucAccessTech <= 2) ? ucAccessTech : -1);
            updateValues.put(mLabels[1], ucAccessTechString);
        } else if(name.equals(MDMContent.MSG_ID_FDD_EM_CSCE_SERV_CELL_S_STATUS_IND)) {
            ucFreqBand = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_SERV_CELL +
                    "." + MDMContent.MSG_VALUE_BAND);
            String bandString = ucFreqBandMapping.containsKey(ucFreqBand) ?
                    ucFreqBandMapping.get(ucFreqBand) : "-(" + ucFreqBand + ")";
            updateValues.put(mLabels[2], bandString);
            wCellId = mMdmComponent.getFieldValue(data,  MDMContent.MSG_VALUE_SERV_CELL +
                    "." + MDMContent.MSG_VALUE_CELL_IDENTITY);
            updateValues.put(mLabels[6], wCellId);
        } else if(name.equals(MDMContent.MSG_ID_EM_SLCE_SRNCID_STATUS_IND)) {
            wRNCId = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_SRNC);
            updateValues.put(mLabels[7], wRNCId);
        } else if(name.equals(MDMContent.MSG_ID_EM_RRM_MEASUREMENT_REPORT_INFO_IND)) {
            ucFreqBand = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_RR_EM_MEASUREMENT_REPORT_INFO +
                    "." + MDMContent.MSG_VALUE_SERVING_CURRENT_BAND);
            String bandString = ucCurBandMapping.containsKey(ucFreqBand) ?
                    ucCurBandMapping.get(ucFreqBand) : "-(" + ucFreqBand + ")";
            updateValues.put(mLabels[2], bandString);
        }
        mMdmComponent.getBuilder().updateDataOnView(updateValues);;
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }

}

class UMTSHSPARFInfoCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "wUarfcn", "wScellScramblingCode", "ucScellRssi",
            "ucScellEcN0", "ucScellRscp", "ucTxPower"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        String coName = MDMContent.MSG_VALUE_SERV_CELL + ".";
        int wUarfcn = mMdmComponent.getFieldValue(data,
                coName + MDMContent.MSG_VALUE_UARFCN_DL);
        int wScellScramblingCode = mMdmComponent.getFieldValue(data,
                coName + MDMContent.MSG_VALUE_PSC);
        int ucScellRssi = mMdmComponent.getFieldValue(data,
                coName + MDMContent.MSG_VALUE_RSSI, true);
        int ucScellEcN0 = mMdmComponent.getFieldValue(data,
                coName + MDMContent.MSG_VALUE_EC_NO, true);
        int ucScellRscp = mMdmComponent.getFieldValue(data,
                coName + MDMContent.MSG_VALUE_RSCP, true);
        int ucTxPower = mMdmComponent.getFieldValue(data, coName +
                MDMContent.MSG_VALUE_PRIMARYCPICH_TX_POWER, true);
        updateValues.clear();
        updateValues.put(mLabels[0], wUarfcn);
        updateValues.put(mLabels[1], wScellScramblingCode);
        updateValues.put(mLabels[2], ucScellRssi);
        updateValues.put(mLabels[3], (float) ucScellEcN0 / 4096);
        updateValues.put(mLabels[4], (float) ucScellRscp / 4096);
        updateValues.put(mLabels[5], ucTxPower);
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }

}

class UMTSHSPAActiveMonitoredSetInfoCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();
    private Object[] mLabels = new Object[] {
            new String[] { "ucNumActive", "ucNumMonitored", "ucNumDetected" },
            new String[] { "wUarfcn", "wScramblingCode", "ucEcN0", "cRscp" },
            new String[] { "wUarfcn", "wScramblingCode", "ucEcN0", "cRscp" },
            new String[] { "wUarfcn", "wScramblingCode", "ucEcN0", "cRscp" }};

    private final HashMap<Integer, String> ucCellTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "ucActive");
            put(1, "ucMonitored");
            put(2, "ucDetected");
        }
    };

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        int[] count = new int[] {0, 0, 0};
        HashMap<Integer, ArrayList<String[]>> ucCells = new HashMap<Integer, ArrayList<String[]>>();
        int activeSetCount = 0;
        int intraMonitoredCount = 0;
        int intraDetectedCount = 0;
        int numCell = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_NUM_CELLS);
        for(int i=0; i<numCell; i++) {
            String coName = MDMContent.MSG_VALUE_UMTS_CELL_LIST + "[" + i + "].";
            int cell_type = mMdmComponent.getFieldValue(data,
                    coName + MDMContent.MSG_VALUE_CELL_TYPE);
            if(ucCellTypeMapping.containsKey(cell_type)){
                count[cell_type]++;
                String[] cells = new String[4];
                cells[0] =  "" + mMdmComponent.getFieldValue(data,
                        coName + MDMContent.MSG_VALUE_UARFCN);
                cells[1] =  "" + mMdmComponent.getFieldValue(data,
                        coName + MDMContent.MSG_VALUE_CELL_PSC);
                cells[2] =  "" + mMdmComponent.getFieldValue(data,
                        coName + MDMContent.MSG_VALUE_ECN0, true);
                cells[3] =  "" + mMdmComponent.getFieldValue(data,
                        coName + MDMContent.MSG_VALUE_CELL_RSCP, true);
                ArrayList<String[]> ucCellValue = ucCells.keySet().contains(cell_type) ?
                        ucCells.get(cell_type) : new ArrayList<String[]>();
                ucCellValue.add(cells);
                ucCells.put(cell_type, ucCellValue);
            }
        }
        updateValues.clear();
        int k = 0;
        updateValues.put("cellTypeNum", 1);
        updateValues.put("0", new String[]{count[0]+"", count[1]+"", count[2]+""});
        for(int i=0; i<ucCellTypeMapping.size(); i++){
            if(ucCells.containsKey(i)) {
                ArrayList<String[]> ucCellValue = ucCells.get(i);
                updateValues.put(ucCellTypeMapping.get(i) + "Num", ucCellValue.size());
                for(int j=0; j<ucCellValue.size(); j++) {
                    k++;
                    updateValues.put(k+"", ucCellValue.get(j));
                }
            } else {
                updateValues.put(ucCellTypeMapping.get(i) + "Num", 0);
            }
        }
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

    @Override
    public Object[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }
}

class UMTSHSPARadioLinkSyncStatusCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();
    private Object[] mLabels = new Object[] {
            new String[] { "ucNumRLSyncStatus", "ucPad"},
            new String[] { "wScramblingCode", "ucTpcCombinationIndex", "ucT313Status"}};

    private final HashMap<Integer, String> ucT313StatusMapping = new HashMap<Integer, String>() {
        {
            put(0, "STOPPED");
            put(1, "RUNNING");
            put(2, "EXPIRED");
            put(3, "UNKNOWN");
        }
    };

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        String[] tableLabels = new String[]{"ucNumRLSyncStatus", "tRlSyncStatus"};
        updateValues.clear();
        updateValues.put(tableLabels[0]+"Num", 1);
        int ucNumRLSyncStatus = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_UCNUMRLSYNCSTATUS);
        int ucPad = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_UCPAD);
        updateValues.put("0", new String[]{ucNumRLSyncStatus+"", ucPad+""});

        updateValues.put(tableLabels[1]+"Num", 6);
        for(int i=0; i<ucNumRLSyncStatus; i++) {
            String coName = MDMContent.MSG_VALUE_TRLSYNCSTATUS + "[" + i + "].";
            String[] cells = new String[3];
            cells[0] =  "" + mMdmComponent.getFieldValue(data,
                    coName + MDMContent.MSG_VALUE_WSCRAMBLINGCODE);
            cells[1] =  "" + mMdmComponent.getFieldValue(data,
                    coName + MDMContent.MSG_VALUE_UCTPCCOMBINATIONINDEX);
            int cell_utc313_status = mMdmComponent.getFieldValue(data,
                    coName + MDMContent.MSG_VALUE_UCT313STATUS);
            cells[2] =  ucT313StatusMapping.containsKey(cell_utc313_status) ?
                    ucT313StatusMapping.get(cell_utc313_status) : "-(" + cell_utc313_status + ")";
            updateValues.put((i + 1) + "", cells);
        }
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

    @Override
    public Object[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }
}

class EUTRARRCStateCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues =
            new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "ucRrcState", "ucRrcCause" };

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    HashMap<Integer, String> rrcStateMapping = new HashMap<Integer, String>() {
        {
            put(0, "NULL");
            put(1, "IDLE");
            put(2, "IDLE");
            put(3, "ATMPT_CONNECTION");
            put(4, "CONNECTED");
            put(5, "ENDING");
            put(6, "ATMPT_OUTBND_MOBILITY");
            put(7, "ATMPT_INBND_MOBILITY");

        }};

        HashMap<Integer, String> sateChangeCauseMapping = new HashMap<Integer, String>() {
            {
                put(0, "NA");
                put(1, "NO_SERVICE");
                put(2, "EST_FAIL_ABORTED");
                put(3, "EST_FAIL_NO_RESP_FROM_CELL");
                put(4, "EST_FAIL_REJ");
                put(5, "EST_FAIL_CELL_RESEL");
                put(6, "EST_FAIL_CELL_BARRED");
                put(7, "NA");
                put(8, "NA");
                put(9, "NA");
                put(10, "EST_EMERGENCY");
                put(11, "EST_HIGH_PRIO_ACC");
                put(12, "EST_MT_ACC");
                put(13, "EST_MO_SIGNAL");
                put(14, "EST_MO_DATA");
                put(15, "EST_MO_DATA");
                put(16, "EST_MO_DATA");
                put(17, "REEST_RECFG_FAIL");
                put(18, "REEST_HO_FAIL");
                put(19, "REEST_OTHER_FAIL");
                put(20, "REL_SUCC_MOB_FROM_EUTRAN");
                put(21, "REL_T310_EXP");
                put(22, "REL_RND_ACC");
                put(23, "REL_MAX_RLC_RETRANS");
                put(24, "REL_CONN_FAIL_T311_EXP");
                put(25, "REL_CONN_FAIL_CELL_NOT_SUIT");
                put(26, "REL_CONN_FAIL_REEST_REJ");
                put(27, "REL_LOAD_BAL_TAU_REQD");
                put(28, "REL_OTHER");
                put(29, "REL_OTHER_RECFG_FAIL");
                put(30, "REL_CONN_FAIL_IRAT_RESEL");
                put(31, "NA");
                put(32, "NA");
                put(33, "NA");

            }};

    @Override
    public void update(Object msgName, Object objectMsg) {
/*        ICDMsg data = (ICDMsg) objectMsg;
        int msgId = msgName instanceof Integer ? (int)msgName : -1;
        if(msgId == -1) return;
        int rrc_state = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_RRC_STATE);
        int rrc_state_change_cause = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_RRC_STATE_CHANGE_CAUSE);
        updateValues.clear();
        updateValues.put(mLabels[0], rrcStateMapping.containsKey(rrc_state) ?
                rrcStateMapping.get(rrc_state) : "-(" + rrc_state + ")");
        updateValues.put(mLabels[1], sateChangeCauseMapping.containsKey(rrc_state_change_cause) ?
                sateChangeCauseMapping.get(rrc_state_change_cause) : "NA");
        mMdmComponent.getBuilder().updateDataOnView(updateValues);*/
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {
        int rrc_state = 3;
        int rrc_state_change_cause = 21;
        updateValues.clear();
        updateValues.put(mLabels[0], rrcStateMapping.containsKey(rrc_state) ?
                rrcStateMapping.get(rrc_state) : "-(" + rrc_state + ")");
        updateValues.put(mLabels[1], sateChangeCauseMapping.containsKey(rrc_state_change_cause) ?
                sateChangeCauseMapping.get(rrc_state_change_cause) :
                    "-(" + rrc_state_change_cause + ")");
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

}

class EUTRAEPSBearerContextCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();
    private Object[] tableLabels = new Object[] {
        new String[] { "ucPdnConnectionId", "ucEpsBearerId",
                "ucEpsBearerType", "ucEpsBearerNewState"}
    };

    public void initUpdateValues() {
        if(updateValues == null) updateValues = new LinkedHashMap<String, Object>();
        if(updateValues.size() == 0) {
            for(int i=0; i<11; i++) {
                updateValues.put(i+"", new int[]{0,i+5,0,0});
            }
        }
    }

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    HashMap<Integer, String> epsBearerTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "DEFAULT_BEARER");
            put(1, "DEDICATED_BEARER");

        }};

    HashMap<Integer, String> epsBearerNewStateMapping = new HashMap<Integer, String>() {
        {
            put(0, "INACTIVE");
            put(1, "ACTIVE_PENDING");
            put(2, "ACTIVE");
            put(3, "MODIFY");

    }};

    @Override
    public void update(Object msgName, Object objectMsg) {
 /*       ICDMsg data = (ICDMsg) objectMsg;
        int msgId = msgName instanceof Integer ? (int)msgName : -1;
        if(msgId == -1) return;
        initUpdateValues();
        int linked_eps_bearer_id = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_LINKED_EPS_BEARER_ID);
        int linked_bearer_id = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_BEARER_ID);
        int esm_context_type = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_ESM_CONTEXT_TYPE);
        int bearer_state = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_BEARER_STATE);

        if(updateValues.containsKey((linked_bearer_id-5)+"")) {
            updateValues.put((linked_bearer_id-5)+"", new String[]{
                    linked_eps_bearer_id+"",
                    linked_bearer_id+"",
                    epsBearerTypeMapping.containsKey(esm_context_type) ?
                        epsBearerTypeMapping.get(esm_context_type) :
                            "-(" + esm_context_type + ")",
                    epsBearerNewStateMapping.containsKey(bearer_state) ?
                        epsBearerNewStateMapping.get(bearer_state) :
                            "-(" + bearer_state + ")"});
        }
        mMdmComponent.getBuilder().updateDataOnView(updateValues);*/
    }

    @Override
    public Object[] getLabels() {
        return tableLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
        initUpdateValues();
    }

    @Override
    public void test() {
        int linked_eps_bearer_id = 12;
        int linked_bearer_id = 14;
        int esm_context_type = 1;
        int bearer_state = 3;
        initUpdateValues();
        if(updateValues.containsKey((linked_bearer_id-5)+"")) {
            updateValues.put((linked_bearer_id-5)+"", new String[]{
                    linked_eps_bearer_id+"",
                    linked_bearer_id+"",
                    epsBearerTypeMapping.containsKey(esm_context_type) ?
                        epsBearerTypeMapping.get(esm_context_type) :
                            "-(" + esm_context_type + ")",
                    epsBearerNewStateMapping.containsKey(bearer_state) ?
                        epsBearerNewStateMapping.get(bearer_state) :
                            "-(" + bearer_state + ")"});
        }
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

}

class EUTRAPUSCHTransmissionStatusCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;

    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();

    public void initUpdateValues() {
        if(updateValues == null) updateValues = new LinkedHashMap<String, Object>();
        if(updateValues.size() == 0) {
            updateValues.put("scTotalPUSCHTxNum", 1);
            updateValues.put("0", new String[]{"0", "0"});
            updateValues.put("ucWbReportNum", 1);
            updateValues.put("1", new String[]{"0", "0"});
            updateValues.put("utCQILabelNum", 0);
            updateValues.put("tCQIWBandDistNum", 1);
            updateValues.put("2", new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0});
            updateValues.put("utCQISubBandLabelNum", 0);
            updateValues.put("tCQISubBandDistNum", 1);
            updateValues.put("3", new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0});
            updateValues.put("utRankIndLabelNum", 0);
            updateValues.put("wRankIndNum", 1);
            updateValues.put("4", new int[]{0,0,0,0,0});
            updateValues.put("utPMILabelNum", 0);
            updateValues.put("wPMIDistNum", 1);
            updateValues.put("5", new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0});
        }
    }
    private Object[] tableLabels = new Object[] {
            new String[] {"scTotalPUSCHTxPower", "scPUSCHTxPowerPerRB"},
            new String[] {"ucWbReportPresent", "ucNumSubBandReport"},
            new String[] {"tCQIDist"},
            new String[] {"0", "1", "2","3", "4", "5","6",
                    "7","8", "9", "10","11", "12","13", "14", "15"},
            new String[] {"tCQISubDist"},
            new String[] {"0", "1", "2","3", "4", "5","6",
                    "7","8", "9", "10","11", "12","13", "14", "15"},
            new String[] {"wRankIndDist"},
            new String[] {"0", "1", "2","3", "4"},
            new String[] {"wPMIDist"},
            new String[] {"0", "1", "2","3", "4", "5","6",
                    "7","8", "9", "10","11", "12","13", "14", "15"},
    };

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    public boolean isValid(int key, int length) {
        if(key >= 0 && key < length)
            return true;
        return false;
    }
    HashMap<Integer, String> reportingTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "PRESENT");
            put(1, "NOT_PRESENT");

        }};

    public void resetValiables() {
        PucchCsfCount = 0;
        PuschCsfCount = 0;
        ucNumPuschSubBandReport = 0;
        ucNumPucchSubBandReport = 0;
        tPuschCQIDist = new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        tPuschCQISubDist = new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        wPuschRankIndDist = new int[]{0,0,0,0,0};
        wPuschPMIDist = new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        tPucchCQIDist = new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        tPucchCQISubDist = new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        wPucchRankIndDist = new int[]{0,0,0,0,0};
        wPucchPMIDist = new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        ucWbReportValid = false;
    }

    int PuschCsfCount = 0;
    int PucchCsfCount = 0;
    int ucNumPuschSubBandReport = 0, ucNumPucchSubBandReport = 0;
    boolean ucWbReportValid = false;
    int[] tPuschCQIDist = new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int[] tPuschCQISubDist = new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int[] wPuschRankIndDist = new int[]{0,0,0,0,0};
    int[] wPuschPMIDist = new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int[] tPucchCQIDist = new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int[] tPucchCQISubDist = new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int[] wPucchRankIndDist = new int[]{0,0,0,0,0};
    int[] wPucchPMIDist = new int[]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    @Override
    public void update(Object msgName, Object objectMsg) {
 /*       ICDMsg data = (ICDMsg) objectMsg;
        int msgId = msgName instanceof Integer ? (int)msgName : -1;
        if(msgId == -1) return;
        initUpdateValues();
        int cCQIDist0 = 0, cCQIDist1 = 0;
        int cRankIndDist = 0;
        if(msgId == MDMContent.MSG_ID_EL1_PUSCH_REPORT) {
            int numCells = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_NUMBER_OF_REPORTS);
            int scTotalPUSCHTxPower = 0;
            int scPUSCHTxPowerPerRB = 0;
            String pushTxPowerCoName = MDMContent.MSG_VALUE_PUSCH_TX_REPORTS + "[";
            for(int i=0; i<numCells; i++) {
                scTotalPUSCHTxPower += mMdmComponent.getFieldValue(data,
                        pushTxPowerCoName + i+ "]." + MDMContent.MSG_VALUE_POWER);
                scPUSCHTxPowerPerRB += mMdmComponent.getFieldValue(data,
                        pushTxPowerCoName + i+ "]." + MDMContent.MSG_VALUE_NUMBER_OF_RBS);
            }
            updateValues.put("0", new String[]{(float) scTotalPUSCHTxPower / numCells + "",
                    (float) scTotalPUSCHTxPower / scPUSCHTxPowerPerRB + ""});
        } else if(msgId == MDMContent.MSG_ID_EL1_PUSCH_CSF) {
            PuschCsfCount ++;
            ucWbReportValid = true;
            String recordsCoName = MDMContent.MSG_VALUE_CSF_RECORDS + "[";
            String coNameSubCQIDist0 = MDMContent.MSG_VALUE_SUBBAND_CQI_CODEWORD_0 + ".";
            String coNameSubCQIDist1 = MDMContent.MSG_VALUE_SUBBAND_CQI_CODEWORD_1 + ".";
            int numRecords = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_NUMBER_OF_RECORDS);
            if(numRecords > 0) {
                ucNumPuschSubBandReport = mMdmComponent.getFieldValue(data,
                        recordsCoName + "0]." + MDMContent.MSG_VALUE_NUMBER_OF_SUBBANDS);
            } else {
                ucNumPuschSubBandReport = 0;
            };
            for(int i=0; i<numRecords; i++) {
                cCQIDist0 = mMdmComponent.getFieldValue(data, recordsCoName + i + "]."
                    + MDMContent.MSG_VALUE_WIDEBAND_CQI_CODEWORD_0);
                cCQIDist1 = mMdmComponent.getFieldValue(data, recordsCoName + i + "]."
                    + MDMContent.MSG_VALUE_WIDEBAND_CQI_CODEWORD_1);
                if(isValid(cCQIDist0, tPuschCQIDist.length)) tPuschCQIDist[cCQIDist0] ++;
                if(isValid(cCQIDist1, tPuschCQIDist.length)) tPuschCQIDist[cCQIDist1] ++;
                cRankIndDist = mMdmComponent.getFieldValue(data, recordsCoName + i + "]."
                        + MDMContent.MSG_VALUE_RANK_INDICATOR);
                if (cRankIndDist == 0) {
                    wPuschRankIndDist[1] ++;
                } else if (cRankIndDist == 1) {
                    wPuschRankIndDist[2] ++;
                }
                wPuschRankIndDist[0] = 0;
                wPuschRankIndDist[3] = 0;
                wPuschRankIndDist[4] = 0;
                int cFPMIDist = mMdmComponent.getFieldValue(data, recordsCoName + i + "]."
                        + MDMContent.MSG_VALUE_FIRST_WB_PMI);
                int cSPMIDist = mMdmComponent.getFieldValue(data, recordsCoName + i + "]."
                        + MDMContent.MSG_VALUE_SECOND_WB_PMI);
                if (isValid(cFPMIDist, wPuschPMIDist.length)) wPuschPMIDist[cFPMIDist]++;
                if (isValid(cSPMIDist, wPuschPMIDist.length)) wPuschPMIDist[cSPMIDist]++;
                for(int j = 0; j<4; j++) {
                    int subCQIDist0 = mMdmComponent.getFieldValue(data, recordsCoName + i + "]."
                            + coNameSubCQIDist0 + MDMContent.MSG_VALUE_SUBBAND + " " + j);
                    int subCQIDist1 = mMdmComponent.getFieldValue(data, recordsCoName + i + "]."
                            + coNameSubCQIDist1 + MDMContent.MSG_VALUE_SUBBAND + " " + j);
                    if(isValid(subCQIDist0, tPuschCQISubDist.length))
                        tPuschCQISubDist[subCQIDist0] ++;
                    if(isValid(subCQIDist1, tPuschCQISubDist.length))
                        tPuschCQISubDist[subCQIDist1] ++;
                }
            }
        } else if(msgId == MDMContent.MSG_ID_EL1_PUCCH_CSF) {
            PucchCsfCount ++;
            int numRecords = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_NUMBER_OF_RECORDS);
            String recordsCoName = MDMContent.MSG_VALUE_CSF_RECORDS + "[";
            if(numRecords > 0) {
                ucNumPucchSubBandReport = mMdmComponent.getFieldValue(data,
                        recordsCoName + "0]." + MDMContent.MSG_VALUE_TOTAL_NUMBER_OF_SUBBANDS);
            } else {
                ucNumPucchSubBandReport = 0;
            };
            for(int i=0; i<numRecords; i++) {
                int ucWbReportPresent = mMdmComponent.getFieldValue(data,
                        recordsCoName + i +"]." + MDMContent.MSG_VALUE_REPORTING_TYPE);
                if (ucWbReportPresent == 0 || ucWbReportPresent == 1 || ucWbReportPresent == 3) {
                    ucWbReportValid = true;
                }
                cCQIDist0 = mMdmComponent.getFieldValue(data,
                        recordsCoName + i +"]." + MDMContent.MSG_VALUE_CQI_CODEWORD_0);
                cCQIDist1 = mMdmComponent.getFieldValue(data,
                        recordsCoName + i +"]." + MDMContent.MSG_VALUE_CQI_CODEWORD_1);
                if(isValid(cCQIDist0, tPucchCQIDist.length)){
                    tPucchCQIDist[cCQIDist0] ++;
                    tPucchCQISubDist[cCQIDist0] ++;
                }
                if(isValid(cCQIDist1, tPucchCQIDist.length)){
                    tPucchCQIDist[cCQIDist1] ++;
                    tPucchCQISubDist[cCQIDist1] ++;
                }
                cRankIndDist = mMdmComponent.getFieldValue(data,
                        recordsCoName + i +"]." + MDMContent.MSG_VALUE_RANK_INDICATOR);
                if (cRankIndDist == 0) {
                    wPucchRankIndDist[1] ++;
                } else if (cRankIndDist == 1) {
                    wPucchRankIndDist[2] ++;
                }
                wPucchRankIndDist[0] = 0;
                wPucchRankIndDist[3] = 0;
                wPucchRankIndDist[4] = 0;
                int cWPMIDist = mMdmComponent.getFieldValue(data,
                        recordsCoName + i +"]." + MDMContent.MSG_VALUE_WIDEBAND_PMI);
                if (isValid(cWPMIDist, wPucchPMIDist.length)) wPucchPMIDist[cWPMIDist]++;
            }
        }
        if(PucchCsfCount + PuschCsfCount == 4) {
            if(PuschCsfCount > 0) {
                updateValues.put("1", new String[]{reportingTypeMapping.get(0),
                        (ucNumPuschSubBandReport < 4 ? ucNumPuschSubBandReport : 4) + "" });
                updateValues.put("2", tPuschCQIDist);
                updateValues.put("3", tPuschCQISubDist);
                updateValues.put("4", wPuschRankIndDist);
                updateValues.put("5", wPuschPMIDist);
            } else {
                updateValues.put("1", new String[]{
                        ucWbReportValid ? reportingTypeMapping.get(0) : reportingTypeMapping.get(1),
                        (ucNumPucchSubBandReport < 4 ? ucNumPucchSubBandReport : 4)+""});
                updateValues.put("2", tPucchCQIDist);
                updateValues.put("3", tPucchCQISubDist);
                updateValues.put("4", wPucchRankIndDist);
                updateValues.put("5", wPucchPMIDist);
            }
            resetValiables();
        }
        mMdmComponent.getBuilder().updateDataOnView(updateValues);*/
    }

    @Override
    public Object[] getLabels() {
        return tableLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
        initUpdateValues();
        resetValiables();
    }

    @Override
    public void test() {
        initUpdateValues();
        int ucWbReportPresent = -1, ucNumPuschSubBandReport = 0, ucNumPucchSubBandReport = 0;
        int cCQIDist0 = 0, cCQIDist1 = 0;
        long msgId = MDMContent.MSG_ID_EL1_PUSCH_REPORT;
        int cRankIndDist = 0, cPMIDist = 0, cFPMIDist = 0, cSPMIDist = 0, cWPMIDist = 0;
        if(msgId == MDMContent.MSG_ID_EL1_PUSCH_REPORT) {
            Elog.d(MdmComponentUtils.TAG, MDMContent.MSG_VALUE_NUMBER_OF_REPORTS);
            int scTotalPUSCHTxPower = 0;
            int scPUSCHTxPowerPerRB = 0;
            int numCells = 1;
            String pushTxPowerCoName = MDMContent.MSG_VALUE_PUSCH_TX_REPORTS + "[";
            for(int i=0; i<numCells; i++) {
                Elog.d(MdmComponentUtils.TAG,  pushTxPowerCoName + i+ "]." +
                        MDMContent.MSG_VALUE_POWER);
                scTotalPUSCHTxPower += 12;
                Elog.d(MdmComponentUtils.TAG, pushTxPowerCoName + i+ "]." +
                        MDMContent.MSG_VALUE_NUMBER_OF_RBS);
                scPUSCHTxPowerPerRB += 15;
            }
            updateValues.put("0", new String[]{(float) scTotalPUSCHTxPower / numCells + "",
                    (float) scTotalPUSCHTxPower / scPUSCHTxPowerPerRB + ""});
        }
        msgId = MDMContent.MSG_ID_EL1_PUSCH_CSF;
        if(msgId == MDMContent.MSG_ID_EL1_PUSCH_CSF) {
            PuschCsfCount ++;
            ucWbReportValid = true;
            String recordsCoName = MDMContent.MSG_VALUE_CSF_RECORDS + "[";
            String coNameCQIDist0 = MDMContent.MSG_VALUE_WIDEBAND_CQI_CODEWORD_0 + ".";
            String coNameCQIDist1 = MDMContent.MSG_VALUE_WIDEBAND_CQI_CODEWORD_1 + ".";
            Elog.d(MdmComponentUtils.TAG,  MDMContent.MSG_VALUE_NUMBER_OF_RECORDS);
            int numRecords = 4;
            if(numRecords > 0) {
                Elog.d(MdmComponentUtils.TAG,  recordsCoName + "0]."
                        + MDMContent.MSG_VALUE_NUMBER_OF_SUBBANDS);
                ucNumPuschSubBandReport = 2;
            } else {
                ucNumPuschSubBandReport = 0;
            };
            for(int i=0; i<numRecords; i++) {
                Elog.d(MdmComponentUtils.TAG,  recordsCoName + i + "]."
                        + MDMContent.MSG_VALUE_WIDEBAND_CQI_CODEWORD_0);
                cCQIDist0 = PuschCsfCount;
                Elog.d(MdmComponentUtils.TAG,  recordsCoName + i + "]."
                        + MDMContent.MSG_VALUE_WIDEBAND_CQI_CODEWORD_1);
                cCQIDist1 = PuschCsfCount + 4;
                if(isValid(cCQIDist0, tPuschCQIDist.length)) tPuschCQIDist[cCQIDist0] ++;
                if(isValid(cCQIDist0, tPuschCQIDist.length)) tPuschCQIDist[cCQIDist1] ++;
                Elog.d(MdmComponentUtils.TAG,  recordsCoName + i + "]."
                        + MDMContent.MSG_VALUE_RANK_INDICATOR);
                cRankIndDist = 1;
                if (cRankIndDist == 0) {
                    wPuschRankIndDist[1] ++;
                } else if (cRankIndDist == 1) {
                    wPuschRankIndDist[2] ++;
                }
                wPuschRankIndDist[0] = 0;
                wPuschRankIndDist[3] = 0;
                wPuschRankIndDist[4] = 0;
                Elog.d(MdmComponentUtils.TAG, recordsCoName + i + "]."
                        + MDMContent.MSG_VALUE_FIRST_WB_PMI);
                cFPMIDist = 1;
                Elog.d(MdmComponentUtils.TAG,  recordsCoName + i + "]."
                        + MDMContent.MSG_VALUE_SECOND_WB_PMI);
                cSPMIDist = 2;
                if (isValid(cFPMIDist, wPuschPMIDist.length)) wPuschPMIDist[cFPMIDist]++;
                if (isValid(cSPMIDist, wPuschPMIDist.length)) wPuschPMIDist[cSPMIDist]++;
                for(int j = 0; j<4; j++) {
                    Elog.d(MdmComponentUtils.TAG,  recordsCoName + i + "]."
                            + coNameCQIDist0 + MDMContent.MSG_VALUE_SUBBAND + " " + j);
                    int subCQIDist0 = 1;
                    Elog.d(MdmComponentUtils.TAG,  recordsCoName + i + "]."
                            + coNameCQIDist1 + MDMContent.MSG_VALUE_SUBBAND + " " + j);
                    int subCQIDist1 = 2;
                    if(isValid(subCQIDist0, tPuschCQISubDist.length))
                        tPuschCQISubDist[subCQIDist0] ++;
                    if(isValid(subCQIDist1, tPuschCQISubDist.length))
                        tPuschCQISubDist[subCQIDist1] ++;
                }
            }
        }
        msgId = MDMContent.MSG_ID_EL1_PUCCH_CSF;
        if(msgId == MDMContent.MSG_ID_EL1_PUCCH_CSF) {
            PucchCsfCount ++;
            Elog.d(MdmComponentUtils.TAG,  MDMContent.MSG_VALUE_NUMBER_OF_RECORDS);
            int numRecords = 4;
            String recordsCoName = MDMContent.MSG_VALUE_CSF_RECORDS + "[";
            if(numRecords > 0) {
                Elog.d(MdmComponentUtils.TAG,  recordsCoName + "0]."
                        + MDMContent.MSG_VALUE_TOTAL_NUMBER_OF_SUBBANDS);
                ucNumPucchSubBandReport = 5;
            } else {
                ucNumPucchSubBandReport = 0;
            }
            for(int i=0; i<numRecords; i++) {
                Elog.d(MdmComponentUtils.TAG,  recordsCoName + i +"]."
                        + MDMContent.MSG_VALUE_REPORTING_TYPE);
                ucWbReportPresent = 1;
                if (ucWbReportPresent == 0 || ucWbReportPresent == 1 || ucWbReportPresent == 3) {
                    ucWbReportValid = true;
                }
                Elog.d(MdmComponentUtils.TAG,  recordsCoName + i +"]."
                        + MDMContent.MSG_VALUE_CQI_CODEWORD_0);
                cCQIDist0 = PucchCsfCount;
                Elog.d(MdmComponentUtils.TAG,  recordsCoName + i +"]."
                        + MDMContent.MSG_VALUE_CQI_CODEWORD_1);
                cCQIDist1 = PucchCsfCount + 4;
                if(isValid(cCQIDist0, tPucchCQIDist.length)){
                    tPucchCQIDist[cCQIDist0] ++;
                    tPucchCQISubDist[cCQIDist0] ++;
                }
                if(isValid(cCQIDist1, tPucchCQIDist.length)){
                    tPucchCQIDist[cCQIDist1] ++;
                    tPucchCQISubDist[cCQIDist1] ++;
                }
                Elog.d(MdmComponentUtils.TAG,  recordsCoName + i +"]."
                        + MDMContent.MSG_VALUE_RANK_INDICATOR);
                cRankIndDist = PucchCsfCount % 2;
                if (cRankIndDist == 0) {
                    wPucchRankIndDist[1] ++;
                } else if (cRankIndDist == 1) {
                    wPucchRankIndDist[2] ++;
                }
                wPucchRankIndDist[0] = 0;
                wPucchRankIndDist[3] = 0;
                wPucchRankIndDist[4] = 0;
                Elog.d(MdmComponentUtils.TAG, recordsCoName + i +"]."
                        + MDMContent.MSG_VALUE_WIDEBAND_PMI);
                cWPMIDist = PucchCsfCount % 2;
                if (isValid(cWPMIDist, wPucchPMIDist.length)) wPucchPMIDist[cWPMIDist]++;
            }
        }
        if(PucchCsfCount + PuschCsfCount == 4) {
            if(PuschCsfCount > 0) {
                updateValues.put("1", new String[]{reportingTypeMapping.get(0),
                        (ucNumPuschSubBandReport < 4 ? ucNumPuschSubBandReport : 4) + "" });
                updateValues.put("2", tPuschCQIDist);
                updateValues.put("3", tPuschCQISubDist);
                updateValues.put("4", wPuschRankIndDist);
                updateValues.put("5", wPuschPMIDist);
            } else {
                updateValues.put("1", new String[]{
                        ucWbReportValid ? reportingTypeMapping.get(0) : reportingTypeMapping.get(1),
                        (ucNumPucchSubBandReport < 4 ? ucNumPucchSubBandReport : 4)+""});
                updateValues.put("2", tPucchCQIDist);
                updateValues.put("3", tPucchCQISubDist);
                updateValues.put("4", wPucchRankIndDist);
                updateValues.put("5", wPucchPMIDist);
            }
            resetValiables();
        }
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }
}

class EUTRARadioLinkSyncStatusCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();

    private String[] mLabels = new String[] { "ucT310Status"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    HashMap<Integer, String> ucT310StatusMapping = new HashMap<Integer, String>() {
        {
            put(0, "STOPPED");
            put(1, "STARTED");
            put(2, "EXPIRED");
            put(3, "UNKNOWN");

        }};

    @Override
    public void update(Object msgName, Object objectMsg) {
/*        ICDMsg data = (ICDMsg) objectMsg;
        int msgId = msgName instanceof Integer ? (int)msgName : -1;
        if(msgId == -1) return;
        int ucT310Status = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_T310_TIMER_STATUS);
        updateValues.clear();
        updateValues.put(mLabels[0], ucT310StatusMapping.containsKey(ucT310Status) ?
                ucT310StatusMapping.get(ucT310Status) : ucT310StatusMapping.get(3));
        mMdmComponent.getBuilder().updateDataOnView(updateValues);*/
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {
        int ucT310Status = 2;
        updateValues.clear();
        updateValues.put(mLabels[0], ucT310StatusMapping.containsKey(ucT310Status) ?
                ucT310StatusMapping.get(ucT310Status) : ucT310StatusMapping.get(3));
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

}

class EUTRARLCDataTransferReportCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();

    public void initUpdateValues() {
        if(updateValues == null) updateValues = new LinkedHashMap<String, Object>();
        if(updateValues.size() == 0) {
            updateValues.put("ulDurationRbsNum", 1);
            updateValues.put("0", new String[]{"", "", ""});
            updateValues.put("dlDurationRbsNum", 1);
            updateValues.put("1", new String[]{"", "", ""});
            updateValues.put("tSrbUlStatsNum", 2);
            updateValues.put("2", new String[]{"", "", "", "", "", ""});
            updateValues.put("3", new String[]{"", "", "", "", "", ""});
            updateValues.put("tSrbDlStatsNum", 2);
            updateValues.put("4", new String[]{"", "", "", "", "", ""});
            updateValues.put("5", new String[]{"", "", "", "", "", ""});
        }
    }

    public void resetSrbUl() {
        updateValues.put("2", new String[]{"", "", "", "", "", ""});
        updateValues.put("3", new String[]{"", "", "", "", "", ""});
    }

    public void resetSrbDl() {
        updateValues.put("4", new String[]{"", "", "", "", "", ""});
        updateValues.put("5", new String[]{"", "", "", "", "", ""});
    }

    private Object[] tableLabels = new Object[] {
            new String[] { "dwRlcUlDuration", "ucNumSrbUl", "ucNumDrbUl"},
            new String[] { "dwRlcDlDuration", "ucNumSrbDl", "ucNumDrbDl"},
            new String[] {"ucSrbId", "ucPad",
                    "wTxSduCount", "wTxByteCount", "wTxPduCount", "wReTxPduCount"},
            new String[] {"ucSrbId", "ucPad", "wRxSduCount",
                    "wRxByteCount", "wRxPduCount", "wReRxPduCount"},
            new String[] {"ucDrbId", "ucEpsBearerId", "ucLogicalChanId",
                "ucRlcMode", "dwTxSduCount", "dwTxByteCount", "dwTxPduCount", "dwReTxPduCount"},
            new String[] {"ucDrbId", "ucEpsBearerId", "ucLogicalChanId",
                "ucRlcMode", "dwRxSduCount", "dwRxByteCount", "dwRxPduCount", "dwReRxPduCount"}};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    HashMap<Integer, String> reportingTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "NOT_PRESENT");
            put(1, "PRESENT");

        }};

    int ulDuration = 0, dlDuration = 0, ucNumSrbUl = 0, ucNumSrbDl = 0,
            ucNumDrbUl = 0, ucNumDrbDl = 0;

    public void resetValiable() {
        ulDuration = 0;
        dlDuration = 0;
        ucNumSrbUl = 0;
        ucNumSrbDl = 0;
        ucNumDrbUl = 0;
        ucNumDrbDl = 0;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        /*
        ICDMsg data = (ICDMsg) objectMsg;
        int msgId = msgName instanceof Integer ? (int)msgName : -1;
        if(msgId == -1) return;
        initUpdateValues();
        if(msgId == MDMContent.MSG_ID_EL2_RLC_UL_STATS) {
            ulDuration = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DURATION);
            int ulRbsNum = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_NUMBER_OF_RBS);
            ucNumSrbUl = (ulRbsNum <= 2 ? ulRbsNum : 2);
            ucNumDrbUl = (ulRbsNum > 2 ? ulRbsNum - 2 : 0);
            updateValues.put("0", new String[]{ulDuration+"ms", ucNumSrbUl+"", ucNumDrbUl+""});
            String tSrbUlStatsCoName = MDMContent.MSG_VALUE_RB_UL_STATISTICS + "[";
            if(ucNumSrbUl == 0) {
                resetSrbUl();
            } else{
                for(int i=0; i<ucNumSrbUl; i++) {
                    String[] srbcells = new String[6];
                    srbcells[0] =  "" + mMdmComponent.getFieldValue(data,
                            tSrbUlStatsCoName+i+"]."+ MDMContent.MSG_VALUE_RB_ID);
                    srbcells[1] = "0xFF";
                    srbcells[2] =  "" + mMdmComponent.getFieldValue(data,
                            tSrbUlStatsCoName+i+"]."+ MDMContent.MSG_VALUE_TX_SDU_CNT);
                    srbcells[3] =  "" + mMdmComponent.getFieldValue(data,
                            tSrbUlStatsCoName+i+"]."+ MDMContent.MSG_VALUE_TX_SDU_BYTES);
                    srbcells[4] =  "" + mMdmComponent.getFieldValue(data,
                            tSrbUlStatsCoName+i+"]."+ MDMContent.MSG_VALUE_TX_PDU_CNT);
                    srbcells[5] =  "" + mMdmComponent.getFieldValue(data,
                            tSrbUlStatsCoName+i+"]."+ MDMContent.MSG_VALUE_PDU_RETX_CNT);
                    updateValues.put((i+2)+"", srbcells);
                }
            }
            updateValues.put("ucNumDrbUlNum", ucNumDrbUl > 0 ? 8 : 0);
            for(int i=0; i<ucNumDrbUl; i++) {
                String[] drbCells = new String[8];
                drbCells[0] = "" + mMdmComponent.getFieldValue(data,
                        tSrbUlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_RB_ID);
                drbCells[1] = "" + mMdmComponent.getFieldValue(data,
                        tSrbUlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_EPS_BEARER_ID);
                drbCells[2] = "" + mMdmComponent.getFieldValue(data,
                        tSrbUlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_LCID);
                int mode = mMdmComponent.getFieldValue(data,
                        tSrbUlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_MODE);
                drbCells[3] = mode == 0 ? "2" : "1";
                drbCells[4] = "" + mMdmComponent.getFieldValue(data,
                        tSrbUlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_TX_SDU_CNT);
                drbCells[5] = "" + mMdmComponent.getFieldValue(data,
                        tSrbUlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_TX_SDU_BYTES);
                drbCells[6] = "" + mMdmComponent.getFieldValue(data,
                        tSrbUlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_TX_PDU_CNT);
                drbCells[7] = "" + mMdmComponent.getFieldValue(data,
                        tSrbUlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_PDU_RETX_CNT);
                updateValues.put((i+6)+"", drbCells);
            }
            if (ucNumDrbDl == 0) updateValues.put("ucNumDrbDlNum", ucNumDrbDl);
        } else if(msgId == MDMContent.MSG_ID_EL2_RLC_DL_STATS) {
            dlDuration = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DURATION);
            int dlRbsNum = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_NUMBER_OF_RBS);
            ucNumSrbDl = (dlRbsNum <= 2 ? dlRbsNum : 2);
            ucNumDrbDl = (dlRbsNum > 2 ? dlRbsNum - 2 : 0);
            updateValues.put("1", new String[]{dlDuration+"ms", ucNumSrbDl+"", ucNumDrbDl+""});
            String tSrbDlStatsCoName = MDMContent.MSG_VALUE_RB_DL_STATISTICS + "[";
            if(ucNumSrbDl == 0) {
                resetSrbDl();
            } else{
                for(int i=0; i<ucNumSrbDl; i++) {
                    String[] cells = new String[6];
                    cells[0] =  "" + mMdmComponent.getFieldValue(data,
                            tSrbDlStatsCoName+i+"]."+ MDMContent.MSG_VALUE_RB_ID);
                    cells[1] = String.format("ox%x", 0xFF);
                    cells[2] =  "" + mMdmComponent.getFieldValue(data,
                            tSrbDlStatsCoName+i+"]."+ MDMContent.MSG_VALUE_SDU_REASSEMB_CNT);
                    cells[3] =  "" + mMdmComponent.getFieldValue(data,
                            tSrbDlStatsCoName+i+"]."+ MDMContent.MSG_VALUE_SDU_REASSEMB_BYTES);
                    cells[4] =  "" + mMdmComponent.getFieldValue(data,
                            tSrbDlStatsCoName+i+"]."+ MDMContent.MSG_VALUE_RX_PDU_CNT);
                    cells[5] =  "" + (mMdmComponent.getFieldValue(data,
                            tSrbDlStatsCoName+i+"]."+ MDMContent.MSG_VALUE_TX_COMPLETE_NACK_CNT)
                            + mMdmComponent.getFieldValue(data,
                            tSrbDlStatsCoName+i+"]."+ MDMContent.MSG_VALUE_TX_SEG_NACK_CNT));
                    updateValues.put((i+4)+"", cells);
                }
            }
            if (ucNumDrbUl == 0) updateValues.put("ucNumDrbUlNum", ucNumDrbUl);
            updateValues.put("ucNumDrbDlNum", ucNumDrbDl);
            for(int i=0; i<ucNumDrbDl; i++) {
                String[] drbCells = new String[8];
                drbCells[0] = "" + mMdmComponent.getFieldValue(data,
                        tSrbDlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_RB_ID);
                drbCells[1] = "" + mMdmComponent.getFieldValue(data,
                        tSrbDlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_EPS_BEARER_ID);
                drbCells[2] = "" + mMdmComponent.getFieldValue(data,
                        tSrbDlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_LCID);
                int mode = mMdmComponent.getFieldValue(data,
                        tSrbDlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_MODE);
                drbCells[3] = mode == 0 ? "2" : "1";
                drbCells[4] = "" + mMdmComponent.getFieldValue(data,
                        tSrbDlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_SDU_REASSEMB_CNT);
                drbCells[5] = "" + mMdmComponent.getFieldValue(data,
                        tSrbDlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_SDU_REASSEMB_BYTES);
                drbCells[6] = "" + mMdmComponent.getFieldValue(data,
                        tSrbDlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_RX_PDU_CNT);
                drbCells[7] = "" + (mMdmComponent.getFieldValue(data,
                        tSrbDlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_TX_COMPLETE_NACK_CNT)
                        + mMdmComponent.getFieldValue(data,
                        tSrbDlStatsCoName+(i+2)+"]." + MDMContent.MSG_VALUE_TX_SEG_NACK_CNT));
                updateValues.put((i+6+8)+"", drbCells);
            }
        }

        mMdmComponent.getBuilder().updateDataOnView(updateValues);*/
    }

    @Override
    public Object[] getLabels() {
        return tableLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
        initUpdateValues();
        resetValiable();
    }

    int count = 0;
    @Override
    public void test() {
        initUpdateValues();
        count ++;
        long msgId = count % 2 != 0 ?
                MDMContent.MSG_ID_EL2_RLC_UL_STATS : MDMContent.MSG_ID_EL2_RLC_DL_STATS;
        if(msgId == MDMContent.MSG_ID_EL2_RLC_UL_STATS) {
            ulDuration = 12;
            int ulRbsNum = (count == 4 ? 0 : 9);
            ucNumSrbUl = (ulRbsNum <= 2 ? ulRbsNum : 2);
            ucNumDrbUl = (ulRbsNum > 2 ? ulRbsNum - 2 : 0);
            updateValues.put("0", new String[]{ulDuration+"ms", ucNumSrbUl+"", ucNumDrbUl+""});
            String tSrbUlStatsCoName = MDMContent.MSG_VALUE_RB_UL_STATISTICS + "[";
            if(ucNumSrbUl == 0) {
                resetSrbDl();
            } else {
                for(int i=0; i<ucNumSrbUl; i++) {
                    String[] srbcells = new String[6];
                    srbcells[0] =  "" + 1;
                    srbcells[1] = "0xFF";
                    srbcells[2] =  "" + 2;
                    srbcells[3] =  "" + 3;
                    srbcells[4] =  "" + 4;
                    srbcells[5] =  "" + 5;
                    updateValues.put((i+2)+"", srbcells);
                }
            }
            updateValues.put("ucNumDrbUlNum", ucNumDrbUl > 0 ? 8 : 0);
            for(int i=0; i<ucNumDrbUl; i++) {
                String[] drbCells = new String[]{"1", "2", "3", "4", "5", "6", "7", "8"};
                int mode = 2;
                drbCells[3] = mode == 0 ? "2" : "1";
                updateValues.put((i+6)+"", drbCells);
            }
            if (ucNumDrbDl == 0) updateValues.put("ucNumDrbDlNum", ucNumDrbDl);
        }
        else if(msgId == MDMContent.MSG_ID_EL2_RLC_DL_STATS) {
            dlDuration = 13;
            int dlRbsNum = (count == 4 ? 5 : 6);
            ucNumSrbDl = (dlRbsNum <= 2 ? dlRbsNum : 2);
            ucNumDrbDl = (dlRbsNum > 2 ? dlRbsNum - 2 : 0);
            updateValues.put("1", new String[]{dlDuration+"ms", ucNumSrbDl+"", ucNumDrbDl+""});
            String tSrbDlStatsCoName = MDMContent.MSG_VALUE_RB_DL_STATISTICS + "[";
            if(ucNumSrbDl == 0) {
                resetSrbDl();
            } else {
                for(int i=0; i<ucNumSrbDl; i++) {
                    String[] cells = new String[]{"1", "2", "3", "4", "5", "6"};
                    cells[1] = "0xFF";
                    updateValues.put((i+4)+"", cells);
                }
            }
            updateValues.put("ucNumDrbDlNum", ucNumDrbDl);
            for(int i=0; i<ucNumDrbDl; i++) {
                String[] drbCells = new String[]{"1", "2", "3", "4", "5", "6", "7", "8"};
                updateValues.put((i+6+8)+"", drbCells);
            }
        }
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

}

class EUTRAMeasurementReportCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();

    private String[] mLabels = new String[] { "wMcc", "wMnc", "dwCellID", "wTac", "wServEarfcn",
            "wServPhysCellId","ucServRsrp", "ucServRsrq", "ucServRssnr", "ucFreqBandInd",
            "ucDLBandwidth", "ucULBandwidth", "ucServIndex"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        /*
        ICDMsg data = (ICDMsg) objectMsg;
        int msgId = msgName instanceof Integer ? (int)msgName : -1;
        if(msgId == -1) return;

        if(msgId == MDMContent.MSG_ID_ERRC_SERVING_CELL_INFO) {
            int wMCC = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_ICD_MCC);
            int wMNC = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_ICD_MNC);
            int dwCellID = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_CELL_ID);
            int wTac = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_TRACKING_AREA_CODE);
            int ucFreqBandInd = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_FREQ_BAND_INDICATOR);
            int ucDLBandwidth = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_DL_BANDWIDTH);
            int ucULBandwidth = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_UL_BANDWIDTH);
            int ucServIndex = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_SERVINDEX);
            if(ucServIndex == 0) {
                updateValues.put(mLabels[0], wMCC);
                updateValues.put(mLabels[1], wMNC);
                updateValues.put(mLabels[2], dwCellID);
                updateValues.put(mLabels[3], wTac);
                updateValues.put(mLabels[9], ucFreqBandInd);
                updateValues.put(mLabels[10], ucDLBandwidth);
                updateValues.put(mLabels[11], ucULBandwidth);
            }
        } else if(msgId == MDMContent.MSG_ID_ERRC_MEAS_REPORT_INFO) {
            long wServEarfcn = mMdmComponent.getFieldLongValue(data,
                    MDMContent.MSG_VALUE_SERV_EARFCN);
            long wServPhysCellId = mMdmComponent.getFieldLongValue(data,
                    MDMContent.MSG_VALUE_PHYSICAL_CELL_ID);
            long ucServRsrp = mMdmComponent.getFieldLongValue(data,
                    MDMContent.MSG_VALUE_SERV_RSRP);
            long ucServRsrq = mMdmComponent.getFieldLongValue(data,
                    MDMContent.MSG_VALUE_SERV_RSRQ);
            long ucServRssnr = mMdmComponent.getFieldLongValue(data,
                    MDMContent.MSG_VALUE_SERV_RSSNR);
            int ucServIndex = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_SERVINDEX);
            if(ucServIndex == 0) {
                updateValues.put(mLabels[4], wServEarfcn);
                updateValues.put(mLabels[5], wServPhysCellId);
                updateValues.put(mLabels[6], ucServRsrp);
                updateValues.put(mLabels[7], ucServRsrq);
                updateValues.put(mLabels[8], ucServRssnr);
                updateValues.put(mLabels[12], ucServIndex);
            }
        }

        mMdmComponent.getBuilder().updateDataOnView(updateValues);
        */
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {
        long msgId = MDMContent.MSG_ID_ERRC_SERVING_CELL_INFO;
        if(msgId == MDMContent.MSG_ID_ERRC_SERVING_CELL_INFO) {
            int wMCC = 00;
            int wMNC = 2;
            int dwCellID = 1234;
            int wTac = 12321;
            updateValues.put(mLabels[0], wMCC);
            updateValues.put(mLabels[1], wMNC);
            updateValues.put(mLabels[2], dwCellID);
            updateValues.put(mLabels[3], wTac);
            msgId = MDMContent.MSG_ID_ERRC_MEAS_REPORT_INFO;
        }
        if(msgId == MDMContent.MSG_ID_ERRC_MEAS_REPORT_INFO) {
            long wServEarfcn = 34;
            int wServPhysCellId = 0;
            int ucServRsrp = 0;
            int ucServRsrq = 0;
            int ucServRssnr = 0;
            int ucFreqBandInd = 0;
            int ucDLBandwidth = 0;
            int ucULBandwidth = 0;
            int ucServIndex = 0;
            updateValues.put(mLabels[4], wServEarfcn);
            updateValues.put(mLabels[5], wServPhysCellId);
            updateValues.put(mLabels[6], ucServRsrp);
            updateValues.put(mLabels[7], ucServRsrq);
            updateValues.put(mLabels[8], ucServRssnr);
            updateValues.put(mLabels[9], ucFreqBandInd);
            updateValues.put(mLabels[10], ucDLBandwidth);
            updateValues.put(mLabels[11], ucULBandwidth);
            updateValues.put(mLabels[12], ucServIndex);
        }

        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

}

class EUTRAMACRandomAccessAttemptCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues =
            new LinkedHashMap<String, Object>();

    private String[] mLabels = new String[] { "wEarfcn", "wPhysCellId", "wRaRnti",
            "ucPreambleCount", "ucLastTxPower", "dwUIGrant",
            "wRaTempCrnti", "ucRaRespSucc", "ucTimingAdv"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        /*
        ICDMsg data = (ICDMsg) objectMsg;
        int msgId = msgName instanceof Integer ? (int)msgName : -1;
        if(msgId == -1) return;

        updateValues.clear();
        long wEarfcn = mMdmComponent.getFieldLongValue(data,
                MDMContent.MSG_VALUE_EARFCN);
        long wPhysCellId = mMdmComponent.getFieldLongValue(data,
                MDMContent.MSG_VALUE_PHYSICAL_CELL_ID);
        int wRaRnti = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_RA_RNTI);
        int ucPreambleCount = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_RACH_ATTEMPT_COUNTER);
        int ucLastTxPower = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_LAST_TX_POWER);
        int dwUIGrant = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_MSG3_GRANT);
        int wRaTempCrnti = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_TEMPORARY_CRNTI);
        int ucRaRespSucc = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_RAR_SUCCESS);
        int ucTimingAdv = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_TA_VALUE);
        updateValues.put(mLabels[0], wEarfcn);
        updateValues.put(mLabels[1], wPhysCellId);
        updateValues.put(mLabels[2], wRaRnti);
        updateValues.put(mLabels[3], ucPreambleCount);
        updateValues.put(mLabels[4], ucLastTxPower);
        updateValues.put(mLabels[5], ucRaRespSucc == 0 ?
                "0xFFFFFFFF" : String.format("0x%x", dwUIGrant));
        updateValues.put(mLabels[6], ucRaRespSucc == 0 ?
                "0xFFFF" : String.format("0x%x", wRaTempCrnti));
        updateValues.put(mLabels[7], ucRaRespSucc);
        updateValues.put(mLabels[8], ucRaRespSucc == 0 ?
                "0xFFFF" : String.format("0x%x", ucTimingAdv));

        mMdmComponent.getBuilder().updateDataOnView(updateValues);
        */
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {
        long wEarfcn = 4294967295l;
        int wPhysCellId = 0;
        int wRaRnti = 0;
        int ucPreambleCount = 0;
        int ucLastTxPower = 0;
        int dwUIGrant = 123;
        int wRaTempCrnti = 22;
        int ucRaRespSucc = 1;
        int ucTimingAdv = 12345;
        updateValues.put(mLabels[0], wEarfcn);
        updateValues.put(mLabels[1], wPhysCellId);
        updateValues.put(mLabels[2], wRaRnti);
        updateValues.put(mLabels[3], ucPreambleCount);
        updateValues.put(mLabels[4], ucLastTxPower);
        updateValues.put(mLabels[5], ucRaRespSucc == 0 ?
                "0xFFFFFFFF" : String.format("0x%x", dwUIGrant));
        updateValues.put(mLabels[6], ucRaRespSucc == 0 ?
                "0xFFFF" : String.format("0x%x", wRaTempCrnti));
        updateValues.put(mLabels[7], ucRaRespSucc);
        updateValues.put(mLabels[8], ucRaRespSucc == 0 ?
                "0xFFFF" : String.format("0x%x", ucTimingAdv));

        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

}

class EUTRAPhysicalDataTransferReportCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;

    private LinkedHashMap<String, Object> updateValues =
            new LinkedHashMap<String, Object>();

    public void initUpdateValues() {
        if(updateValues == null) updateValues = new LinkedHashMap<String, Object>();
        if(updateValues.size() == 0) {
            updateValues.put("0", new String[]{"0", "pCell"});
            updateValues.put("1", new String[]{"0", "sCell1"});
            updateValues.put("2", new String[]{"0", "sCell2"});
            updateValues.put("3", new String[]{"0", "sCell3"});
            updateValues.put("4", new String[]{"0", "sCell4"});
            updateValues.put("5", new String[]{"0", "sCell5"});
            updateValues.put("6", new String[]{"0", "sCell6"});
            updateValues.put("7", new String[]{"0", "sCell7"});
        }
    }
    private Object[] tableLabels = new Object[] {
        new String[] { "dwDLTput", "ucServIndex"},
    };

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    private HashMap<Integer, String> ucServIndexMapping =
            new HashMap<Integer, String>(){{
                put(0, "pCell");
                put(1, "sCell1");
                put(2, "sCell2");
                put(3, "sCell3");
                put(4, "sCell4");
                put(5, "sCell5");
                put(6, "sCell6");
                put(7, "sCell7");
            }};

    int dwDLTput0 = 0, dwDLTput1 = 0, wServingCell0 = -1, wServingCell1 = -1;

    public void resetValiable() {
        dwDLTput0 = 0;
        dwDLTput1 = 0;
        wServingCell0 = -1;
        wServingCell1 = -1;
    }
    @Override
    public void update(Object msgName, Object objectMsg) {
        /*
        ICDMsg data = (ICDMsg) objectMsg;
        int msgId = msgName instanceof Integer ? (int)msgName : -1;
        if(msgId == -1) return;
        if(msgId == MDMContent.MSG_ID_EL1_MIMO_PDSCH_THROUGHPUT0) {
            wServingCell0 = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_SERVING_CELL_INDEX);
            dwDLTput0 = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_MIMO_PDSCH_THROUGHPUT0);
        } else if(msgId == MDMContent.MSG_ID_EL1_MIMO_PDSCH_THROUGHPUT1) {
            wServingCell1 = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_SERVING_CELL_INDEX);
            dwDLTput1 = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_MIMO_PDSCH_THROUGHPUT1);
        }
        if(wServingCell0 != -1 && wServingCell1 != -1) {
            initUpdateValues();
            if(wServingCell0 == wServingCell1 &&
                    ucServIndexMapping.containsKey(wServingCell0)) {
                updateValues.put(""+wServingCell0, new String[]{
                    "" + (dwDLTput0 + dwDLTput1), ucServIndexMapping.get(wServingCell0)});
            } else {
                if(ucServIndexMapping.containsKey(wServingCell0)){
                    updateValues.put(""+wServingCell0, new String[]{
                        "" + (dwDLTput0), ucServIndexMapping.get(wServingCell0)});
                }
                if(ucServIndexMapping.containsKey(wServingCell1)){
                    updateValues.put(""+wServingCell1, new String[]{
                        "" + (dwDLTput1), ucServIndexMapping.get(wServingCell1)});
                }
            }
            resetValiable();
        }
        mMdmComponent.getBuilder().updateDataOnView(updateValues);*/
    }

    @Override
    public Object[] getLabels() {
        return tableLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
        initUpdateValues();
        resetValiable();
    }

    @Override
    public void test() {
        initUpdateValues();
        long msgId = (wServingCell0 == -1 ? MDMContent.MSG_ID_EL1_MIMO_PDSCH_THROUGHPUT0 :
            MDMContent.MSG_ID_EL1_MIMO_PDSCH_THROUGHPUT1);
        if(msgId == MDMContent.MSG_ID_EL1_MIMO_PDSCH_THROUGHPUT0) {
            wServingCell0 = 1;
            dwDLTput0 = 30;
//            msgId = MDMContent.MSG_ID_EL1_MIMO_PDSCH_THROUGHPUT1;
        } else if(msgId == MDMContent.MSG_ID_EL1_MIMO_PDSCH_THROUGHPUT1) {
            wServingCell1 = 1;
            dwDLTput1 = 4;
        }
        if(wServingCell0 != -1 && wServingCell1 != -1) {
            initUpdateValues();
            if(wServingCell0 == wServingCell1 &&
                    ucServIndexMapping.containsKey(wServingCell0)) {
                updateValues.put(""+wServingCell0, new String[]{
                    "" + (dwDLTput0 + dwDLTput1), ucServIndexMapping.get(wServingCell0)});
            } else {
                if(ucServIndexMapping.containsKey(wServingCell0)){
                    updateValues.put(""+wServingCell0, new String[]{
                        "" + (dwDLTput0), ucServIndexMapping.get(wServingCell0)});
                }
                if(ucServIndexMapping.containsKey(wServingCell1)){
                    updateValues.put(""+wServingCell1, new String[]{
                        "" + (dwDLTput1), ucServIndexMapping.get(wServingCell1)});
                }
            }
            resetValiable();
        }
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

}

class EUTRACarrierAggregationEventCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;

    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();

    private Object[] tableLabels = new Object[] {
        new String[] { "ucEvent", "ucNumSServCells", "wPad" },
        new String[] { "dwEarfcn", "wPhysCellId", "ucFreqBandInd", "ucDlBandwidth" },
        new String[] { "dwEarfcn", "wPhysCellId", "ucFreqBandInd",
                "ucDlBandwidth", "ucDlState", "ucUlState"}
    };

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    private HashMap<Integer, String> ucEventMapping = new HashMap<Integer, String>(){{
        put(0, "CONFIG");
        put(1, "DECONFIG");
        put(2, "ACTIVATE");
        put(3, "DEACTIVATE");
        put(4, "VRLF_ACTIVATE");
        put(5, "VRLF_DEACTIVATE");
        put(6, "CONN_RELEASE");
        put(7, "HANDOVER");
    }};

    @Override
    public void update(Object msgName, Object objectMsg) {
        /*
        ICDMsg data = (ICDMsg) objectMsg;
        int msgId = msgName instanceof Integer ? (int)msgName : -1;
        if(msgId == -1) return;

        updateValues.clear();
        int ucEvent = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_EVENT);
        int ucNumSServCells = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_NUM_OF_SCELLS);
        long wPad = 0xFFFF;
        updateValues.put("commonNum", 1);
        updateValues.put("0", new String[]{ucEventMapping.containsKey(ucEvent) ?
                ucEventMapping.get(ucEvent) : "-(" + ucEvent + ")",
                ""+ucNumSServCells,
                String.format("0x%04X", wPad)});

        String coPccName = MDMContent.MSG_VALUE_PCC_CONFIG + ".";
        long pEarfcn = mMdmComponent.getFieldLongValue(data,
                coPccName + MDMContent.MSG_VALUE_EARFCN);
        long pPhysCellId = mMdmComponent.getFieldLongValue(data,
                coPccName + MDMContent.MSG_VALUE_PHYSICAL_CELL_ID);
        int pFreqBandInd = mMdmComponent.getFieldValue(data,
                coPccName + MDMContent.MSG_VALUE_FREQ_BAND_INDICATOR);
        int pDlBandwidth = mMdmComponent.getFieldValue(data,
                coPccName + MDMContent.MSG_VALUE_DL_BANDWIDTH);
        updateValues.put("tPccConfigNum", 1);
        updateValues.put("1", new String[]{""+pEarfcn,
                ""+pPhysCellId,""+pFreqBandInd,""+pDlBandwidth});

        String coSccName = MDMContent.MSG_VALUE_SCC_CONFIG + "[";
        updateValues.put("tSccConfigNum", ucNumSServCells);
        for(int i=0; i<ucNumSServCells; i++) {
            long sEarfcn = mMdmComponent.getFieldLongValue(data,
                    coSccName+i+"]." + MDMContent.MSG_VALUE_EARFCN);
            long sPhysCellId = mMdmComponent.getFieldLongValue(data,
                    coSccName+i+"]." + MDMContent.MSG_VALUE_PHYSICAL_CELL_ID);
            int sFreqBandInd = mMdmComponent.getFieldValue(data,
                    coSccName+i+"]." + MDMContent.MSG_VALUE_FREQ_BAND_INDICATOR);
            int sDlBandwidth = mMdmComponent.getFieldValue(data,
                    coSccName+i+"]." + MDMContent.MSG_VALUE_DL_BANDWIDTH);
            int sDlState = mMdmComponent.getFieldValue(data,
                    coSccName+i+"]." + MDMContent.MSG_VALUE_DL_STATE);
            int sUlState = mMdmComponent.getFieldValue(data,
                    coSccName+i+"]." + MDMContent.MSG_VALUE_UL_STATE);
            updateValues.put(""+(i+2), new String[]{""+sEarfcn, ""+sPhysCellId,
                    ""+sFreqBandInd,""+sDlBandwidth,""+sDlState,""+sUlState});
        }

        mMdmComponent.getBuilder().updateDataOnView(updateValues);
       */
    }

    @Override
    public Object[] getLabels() {
        return tableLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {
        updateValues.clear();
        int ucEvent = 0;
        int ucNumSServCells = 5;
        long wPad = 0xFFFF;
        updateValues.put("commonNum", 1);
        updateValues.put("0", new String[]{ucEventMapping.containsKey(ucEvent) ?
                ucEventMapping.get(ucEvent) : "-(" + ucEvent + ")",
                ""+ucNumSServCells,""+String.format("0x%X", wPad)});

        String coPccName = MDMContent.MSG_VALUE_PCC_CONFIG + ".";
        long pEarfcn = 0xFFFF;
        long pPhysCellId = 0;
        int pFreqBandInd = 0;
        int pDlBandwidth = 0;
        updateValues.put("tPccConfigNum", 1);
        updateValues.put("1", new String[]{
            ""+pEarfcn, ""+pPhysCellId,""+pFreqBandInd,""+pDlBandwidth});

        String coSccName = MDMContent.MSG_VALUE_SCC_CONFIG + "[";
        updateValues.put("tSccConfigNum", ucNumSServCells);
        for(int i=0; i<ucNumSServCells; i++) {
            long sEarfcn = 0;
            long sPhysCellId = 0;
            int sFreqBandInd = 2;
            int sDlBandwidth = 0;
            int sDlState = 3;
            int sUlState = 0;
            updateValues.put(""+(i+2), new String[]{""+sEarfcn, ""+sPhysCellId,
                    ""+sFreqBandInd,""+sDlBandwidth,""+sDlState,""+sUlState});
        }

        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

}

class SIPRegistrationAttemptCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues =
            new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "szCallId", "szRequestUri", "szTo"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        int is_att_call_id_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_ATT_CALL_ID_VALID);
        int is_att_request_uri_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_ATT_REQUEST_URI_VALID);
        int is_att_to_valid  = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_ATT_TO_VALID );
        String coNameAttCallId = MDMContent.MSG_VALUE_ATT_CALL_ID + "[";
        String coNameAttRequestUri = MDMContent.MSG_VALUE_ATT_REQUEST_URI + "[";
        String coNameAttTo = MDMContent.MSG_VALUE_ATT_TO + "[";
        String attCallId = "";
        String requestUri = "";
        String attTo = "";
        for(int i=0; i<20; i++){
            int curAttCallId = mMdmComponent.getFieldValue(data, coNameAttCallId+i+"]");
            if(curAttCallId == 0) {
                break;
            } else {
                attCallId += (Character.toString ((char) curAttCallId));
            }
        }
        for(int i=0; i<128; i++){
            int curRequestUri = mMdmComponent.getFieldValue(data, coNameAttRequestUri+i+"]");
            requestUri += (curRequestUri != 0 ?
                    (Character.toString ((char) curRequestUri)) : "");
            int curAttTo = mMdmComponent.getFieldValue(data, coNameAttTo+i+"]");
            attTo += (curAttTo != 0 ? (Character.toString ((char) curAttTo)) : "");
        }
        if (is_att_call_id_valid == 1) updateValues.put(mLabels[0], attCallId);
        if (is_att_request_uri_valid == 1) updateValues.put(mLabels[1], requestUri);
        if (is_att_to_valid == 1) updateValues.put(mLabels[2], attTo);
        mMdmComponent.getBuilder().updateDataOnView(updateValues);;
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }

}

class SIPRegistrationStateCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues =
            new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "ucRegState", "szCallId"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    HashMap<Integer, String> attRegStateMapping = new HashMap<Integer, String>() {
        {
            put(0, "UNKNOWN");
            put(1, "NOT_REGISTERED");
            put(2, "REGISTERING");
            put(3, "AUTH_CHALLENGE");
            put(4, "AUTH_REGISTER");
            put(5, "REGISTERED");
            put(6, "NOTIFIED");
            put(7, "TIMEOUT");
        }};

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        String coNameAttCallId = MDMContent.MSG_VALUE_ATT_CALL_ID + "[";
        String attCallId = "";
        int is_att_reg_state_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_ATT_REG_STATE_VALID);
        int is_att_call_id_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_ATT_CALL_ID_VALID);
        int attRegState = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_ATT_REG_STATE);
        for(int i=0; i<20; i++){
            int curAttCallId = mMdmComponent.getFieldValue(data, coNameAttCallId+i+"]");
            if(curAttCallId == 0) {
                break;
            } else {
                attCallId += (Character.toString ((char) curAttCallId));
            }
        }

        if (is_att_reg_state_valid == 1) updateValues.put(mLabels[0],
                attRegStateMapping.containsKey(attRegState) ?
                attRegStateMapping.get(attRegState) : ("-(" + attRegState + ")"));
        if (is_att_call_id_valid == 1) updateValues.put(mLabels[1], attCallId);

        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }

}

class SIPRegistrationResultCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues =
            new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "wResult", "szCallId"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        String coNameAttCallId = MDMContent.MSG_VALUE_ATT_CALL_ID + "[";
        String attCallId = "";
        int attRegResult = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_ATT_REG_RESULT);
        int is_att_reg_result_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_ATT_REG_RESULT_VALID);
        int is_att_call_id_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_ATT_CALL_ID_VALID);
        for(int i=0; i<20; i++){
            int curAttCallId = mMdmComponent.getFieldValue(data, coNameAttCallId+i+"]");
            if(curAttCallId == 0) {
                break;
            } else {
                attCallId += (Character.toString ((char) curAttCallId));
            }
        }
        if (is_att_reg_result_valid == 1) updateValues.put(mLabels[0], attRegResult);
        if (is_att_call_id_valid == 1) updateValues.put(mLabels[1], attCallId);
        mMdmComponent.getBuilder().updateDataOnView(updateValues);;
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }

}

class SIPSessionStartCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues =
            new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "szDialedString", "szCallId",
            "szOriginatingURI", "szTerminatingURI"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        String coNamePhoneNum = MDMContent.MSG_VALUE_PHONE_NUMBER + "[";
        String coNameOrigUri = MDMContent.MSG_VALUE_ORIGINATING_URI + "[";
        String coNameTermUri = MDMContent.MSG_VALUE_TERMINATING_URI + "[";
        String coNameAttCallId = MDMContent.MSG_VALUE_SIP_CALL_ID + "[";
        String szDialedString = "";
        String szOriginatingURI = "";
        String szTerminatingURI = "";
        String attCallId = "";
        int is_originating_uri_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_ORIGINATING_URI_VALID);
        int is_terminating_uri_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_TERMINATING_URI_VALID);
        int is_phone_number_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_PHONE_NUMBER_VALID);
        int is_call_id_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_SIP_CALLID_VALID);
        Elog.d(MdmComponentUtils.TAG, "[SIPSessionStartCallBack] is_sip_callid_valid:"
                + is_call_id_valid);
        for(int i=0; i<128; i++){
            int curPhoneNum = mMdmComponent.getFieldValue(data, coNamePhoneNum+i+"]");
            szDialedString += (curPhoneNum != 0 ?
                    (Character.toString ((char) curPhoneNum)) : "");
            int curOrigUri = mMdmComponent.getFieldValue(data, coNameOrigUri+i+"]");
            szOriginatingURI += (curOrigUri != 0 ?
                    (Character.toString ((char) curOrigUri)) : "");
            int curTermUri = mMdmComponent.getFieldValue(data, coNameTermUri+i+"]");
            szTerminatingURI += (curTermUri != 0 ?
                    (Character.toString ((char) curTermUri)) : "");
        }
        for(int i=0; i<20; i++){
            int curAttCallId = mMdmComponent.getFieldValue(data, coNameAttCallId+i+"]");
            if(curAttCallId == 0) {
                break;
            } else {
                attCallId += (Character.toString ((char) curAttCallId));
            }
        }
        if (is_phone_number_valid == 1) updateValues.put(mLabels[0], szDialedString);
        if (is_call_id_valid == 1) updateValues.put(mLabels[1], attCallId);
        if (is_originating_uri_valid == 1) updateValues.put(mLabels[2], szOriginatingURI);
        if (is_terminating_uri_valid == 1) updateValues.put(mLabels[3], szTerminatingURI);

        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }
}

class SIPSessionStateCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "ucCallState", "szCallId"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    HashMap<Integer, String> ucCallStateMapping = new HashMap<Integer, String>() {
        {
            put(0, "UNKNOWN");
            put(1, "IDLE");
            put(2, "INVITE");
            put(3, "TRYING");
            put(4, "PROGRESS");
            put(5, "NEGOTIATING");
            put(6, "UPDATED");
            put(7, "RINGING");
            put(8, "ANSWERED");
            put(9, "CONNECTED");
            put(10, "HELD");
            put(11, "DISCONNECTING");
        }
    };

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        String coNameAttCallId = MDMContent.MSG_VALUE_SIP_CALL_ID + "[";
        String attCallId = "";
        for(int i=0; i<20; i++){
            int curAttCallId = mMdmComponent.getFieldValue(data, coNameAttCallId+i+"]");
            if(curAttCallId == 0) {
                break;
            } else {
                attCallId += (Character.toString ((char) curAttCallId));
            }
        }
        int ucCallState = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_CALL_STATE);
        int is_call_state_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_CALL_STATE_VALID);
        int is_call_id_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_SIP_CALLID_VALID);
        Elog.d(MdmComponentUtils.TAG, "[SIPSessionStateCallBack] is_sip_callid_valid:"
                + is_call_id_valid);
        if (is_call_state_valid == 1) updateValues.put(mLabels[0],
                ucCallStateMapping.containsKey(ucCallState) ?
                ucCallStateMapping.get(ucCallState) : "-(" + ucCallState + ")");
        if (is_call_id_valid == 1) updateValues.put(mLabels[1], attCallId);
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }

}

class SIPSessionEndCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues =
            new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "wResult", "szCallId"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    String failureCode = "", isMtCall = "";

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        int is_failure_code_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_FAILURE_CODE_VALID);
        int is_mt_call_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_MT_CALL_VALID);
        int is_call_id_valid = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_IS_SIP_CALLID_VALID);
        Elog.d(MdmComponentUtils.TAG, "[SIPSessionEndCallBack] is_sip_callid_valid:"
                + is_call_id_valid);
        failureCode = (is_failure_code_valid == 1 ?
                "" + mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_FAILURE_CODE) :
                    failureCode);
        isMtCall = (is_mt_call_valid == 1 ?
                "" + mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_IS_MT_CALL) :
                    isMtCall);
        String coNameAttCallId = MDMContent.MSG_VALUE_SIP_CALL_ID + "[";
        String attCallId = "";
        for(int i=0; i<20; i++){
            int curAttCallId = mMdmComponent.getFieldValue(data, coNameAttCallId+i+"]");
            if(curAttCallId == 0) {
                break;
            } else {
                attCallId += (Character.toString ((char) curAttCallId));
            }
        }
        updateValues.put(mLabels[0], isMtCall.equals("") &&
                failureCode.equals("") ? "" : (isMtCall+"," + failureCode));
        if (is_call_id_valid == 1) updateValues.put(mLabels[1], attCallId);
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }
}

class SIPSessionStatisticsCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues =
            new LinkedHashMap<String, Object>();
    private Object[] tableLabels = new Object[] {
            new String[] { "wNumMediaTracks"},
            new String[] {"wPort", "ucMediaType",
            "ucFormat", "dwDuration", "dwPacketsRcvd", "dwPacketsDrop", "dwPacketsLate"}};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    HashMap<Integer, String> ucMediaTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "UNKNOWN");
            put(1, "AUDIO");
            put(2, "VIDEO" );
            put(3, "TEXT" );
            put(4, "APPLICATION" );
            put(5, "MESSAGE" );
            put(6, "CONTROL" );
            put(7, "DATA" );
        }
    };

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        int wNumMediaTracks = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_WNUMMEDIATRACKS);
        updateValues.clear();
        updateValues.put(tableLabels[0]+"Num", 1);
        updateValues.put("0", new String[]{wNumMediaTracks+""});
        String coNameTMediaStats = MDMContent.MSG_VALUE_TMEDIASTATS + "[";
        String wPort = "";
        String ucMediaType = "";
        String ucFormat = "";
        String dwDuration = "";
        String dwPacketsRcvd = "";
        String dwPacketsDrop = "";
        String dwPacketsLate = "";
        updateValues.put(tableLabels[1]+"Num", wNumMediaTracks);
        for(int i=0; i<wNumMediaTracks; i++){
            String[] cells = new String[7];
            cells[0] =  "" + mMdmComponent.getFieldValue(data,
                    coNameTMediaStats+i+"]" + "." + MDMContent.MSG_VALUE_WPORT);
            int ucMediaTypeValue = mMdmComponent.getFieldValue(data,
                    coNameTMediaStats+i+"]" + "." + MDMContent.MSG_VALUE_UCMEDIATYPE);
            cells[1] = ucMediaTypeMapping.containsKey(ucMediaTypeValue) ?
                    ucMediaTypeMapping.get(ucMediaTypeValue) : "-(" + ucMediaTypeValue + ")";
            cells[2] = "" + mMdmComponent.getFieldValue(data,
                    coNameTMediaStats+i+"]" + "." + MDMContent.MSG_VALUE_UCFORMAT);
            cells[3] = "" + mMdmComponent.getFieldValue(data,
                    coNameTMediaStats+i+"]" + "." + MDMContent.MSG_VALUE_DWDURATION);
            cells[4] = "" + mMdmComponent.getFieldValue(data,
                    coNameTMediaStats+i+"]" + "." + MDMContent.MSG_VALUE_DWPACKETSRCVD);
            cells[5] = "" + mMdmComponent.getFieldValue(data,
                    coNameTMediaStats+i+"]" + "." + MDMContent.MSG_VALUE_DWPACKETSDROP);
            cells[6] = "" + mMdmComponent.getFieldValue(data,
                    coNameTMediaStats+i+"]" + "." + MDMContent.MSG_VALUE_DWPACKETSLATE);
            updateValues.put((i + 1) + "", cells);
        }
        mMdmComponent.getBuilder().updateDataOnView(updateValues);

    }

    @Override
    public Object[] getLabels() {
        return tableLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }

}

class SIPMessageReceivedCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "Transaction ID", "Value of SIP Message",
            "Message Length"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        int direction = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DIRECTION);
        if(direction == 1) {
            updateValues.clear();
            int transactionId = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_TRANSACTION_ID);
            updateValues.put(mLabels[0], transactionId);
            int peerSize = data.getPeerBufferLength();
            updateValues.put(mLabels[2], peerSize);
            if(peerSize > 0) {
                byte [] peerBufBytes = data.getPeerBufferValue();
                String peer_data = peerBufBytes == null ? "" : new String(peerBufBytes);
                updateValues.put(mLabels[1], peer_data);
            }
        }
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }
}

class SIPMessageTransmittedCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "Transaction ID", "Value of SIP Message",
            "Message Length"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        int direction = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DIRECTION);
        if(direction == 0) {
            updateValues.clear();
            int transactionId = mMdmComponent.getFieldValue(data,
                    MDMContent.MSG_VALUE_TRANSACTION_ID);
            updateValues.put(mLabels[0], transactionId);
            int peerSize = data.getPeerBufferLength();
            updateValues.put(mLabels[2], peerSize);
            if(peerSize > 0) {
                byte [] peerBufBytes = data.getPeerBufferValue();
                String peer_data = peerBufBytes == null ? "" : new String(peerBufBytes);
                updateValues.put(mLabels[1], peer_data);
            }
        }
        mMdmComponent.getBuilder().updateDataOnView(updateValues);
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }

}

class RTPTransmitPacketCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "Byte Count", "Destination Port", "Flags",
            "Payload Type", "Sequence Number", "Timestamp", "Source ID" };

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        int byteCount = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_WBYTECOUNT);
        int destinationPort = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_WDSTPORT);
        int flags = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_UCFLAGS);
        int payloadType = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_UCPAYLOADTYPE);
        int sequenceNumber = mMdmComponent.getFieldValue(data
                , MDMContent.MSG_VALUE_WSEQUENCENUM);
        int timestamp = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DWTIMESTAMP);
        int sourceID = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DWSOURCEID);
        updateValues.clear();
        updateValues.put(mLabels[0], byteCount);
        updateValues.put(mLabels[1], destinationPort);
        updateValues.put(mLabels[2], flags);
        updateValues.put(mLabels[3], payloadType);
        updateValues.put(mLabels[4], sequenceNumber);
        updateValues.put(mLabels[5], timestamp);
        updateValues.put(mLabels[6], sourceID);
        mMdmComponent.getBuilder().updateDataOnView(updateValues);;
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }
}

class RTPReceivePacketCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues = new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "Byte Count", "Destination Port", "Flags",
            "Payload Type", "Sequence Number", "Timestamp", "Source ID"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        int byteCount = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_WBYTECOUNT);
        int destinationPort = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_WDSTPORT);
        int flags = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_UCFLAGS);
        int payloadType = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_UCPAYLOADTYPE);
        int sequenceNumber = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_WSEQUENCENUM);
        int timestamp = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DWTIMESTAMP);
        int sourceID = mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DWSOURCEID);
        updateValues.clear();
        updateValues.put(mLabels[0], byteCount);
        updateValues.put(mLabels[1], destinationPort);
        updateValues.put(mLabels[2], flags);
        updateValues.put(mLabels[3], payloadType);
        updateValues.put(mLabels[4], sequenceNumber);
        updateValues.put(mLabels[5], timestamp);
        updateValues.put(mLabels[6], sourceID);
        mMdmComponent.getBuilder().updateDataOnView(updateValues);;
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }

}

class RTPTransmitStatisticsCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues =
            new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "dwSsrc", "dwDuration", "dwPktCount",
            "dwByteCount", "wDstPort", "wMeanJitter",
            "ucMediaType", "ucIpVersion", "strIpDstAddr"};

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    HashMap<Integer, String> ucMediaTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "UNKNOWN");
            put(1, "AUDIO");
            put(2, "VIDEO" );
            put(3, "TEXT" );
            put(4, "APPLICATION" );
            put(5, "MESSAGE" );
            put(6, "CONTROL");
            put(7, "DATA");
        }
    };

    HashMap<Integer, String> ucIpVersionMapping = new HashMap<Integer, String>() {
        {
            put(0, "UNKNOWN");
            put(1, "IPv4");
            put(2, "IPv6" );
        }
    };

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        int dwSsrc= mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DWSSRC);
        int dwDuration= mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_DWDURATION);
        int dwPktCount = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_DWPKTCOUNT);
        int dwByteCount= mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_DWBYTECOUNT);
        int wDstPort = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_WDSTPORT);
        int wMeanJitter = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_WMEANJITTER);
        int ucMediaType = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_UCMEDIATYPE);
        int ucIpVersion = mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_UCIPVERSION);
        String strIpDstAddr = "";
        String coNameIpDstAddr = MDMContent.MSG_VALUE_STRIPDSTADDR + "[";
        if(ucIpVersion == 1) {
            for(int i=0; i<4; i++) {
                int curIpDstAddr = mMdmComponent.getFieldValue(data, coNameIpDstAddr+i+"]");
                strIpDstAddr += (strIpDstAddr.equals("") ? curIpDstAddr : ("."+curIpDstAddr));
            }
        } else {
            int lastIpDstAddr = mMdmComponent.getFieldValue(data, coNameIpDstAddr + "0]");
            for(int i=1; i<16; i++) {
                int curIpDstAddr = mMdmComponent.getFieldValue(data, coNameIpDstAddr+i+"]");
                if (i % 2 == 1) {
                    String ipStr = String.format("%02x", lastIpDstAddr)
                            + String.format("%02x", curIpDstAddr);
                    strIpDstAddr += (strIpDstAddr.equals("") ? ipStr : (":" + ipStr));
                }
                lastIpDstAddr = curIpDstAddr;
            }
        }

        updateValues.clear();
        updateValues.put(mLabels[0], dwSsrc);
        updateValues.put(mLabels[1], dwDuration);
        updateValues.put(mLabels[2], dwPktCount);
        updateValues.put(mLabels[3], dwByteCount);
        updateValues.put(mLabels[4], wDstPort);
        updateValues.put(mLabels[5], wMeanJitter);
        updateValues.put(mLabels[6], ucMediaTypeMapping.containsKey(ucMediaType) ?
                ucMediaTypeMapping.get(ucMediaType) : "-(" + ucMediaType + ")");
        updateValues.put(mLabels[7], ucIpVersionMapping.containsKey(ucIpVersion) ?
                ucIpVersionMapping.get(ucIpVersion) : "-(" + ucIpVersion + ")");
        updateValues.put(mLabels[8], strIpDstAddr);
        mMdmComponent.getBuilder().updateDataOnView(updateValues);;
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }
}

class RTPReceiveStatisticsCallBack implements IMdmViewUpdateCallBack {
    private MdmBaseComponent mMdmComponent;
    private LinkedHashMap<String, Object> updateValues =
            new LinkedHashMap<String, Object>();
    private String[] mLabels = new String[] { "dwSsrc", "dwDuration", "dwPktCount",
            "dwPktLoss", "dwByteCount", "wDstPort",
            "wMeanJitter", "wMaxJitter", "wMaxDelta",
            "wCumAvgPktSize", "ucMediaType", "ucIpVersion", "strIpSrcAddr" };

    @Override
    public void setMdmComponent(MdmBaseComponent mMdmComponent) {
        this.mMdmComponent = mMdmComponent;
    }

    @Override
    public boolean supportMultiSIM() {
        return true;
    }

    HashMap<Integer, String> ucMediaTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "UNKNOWN");
            put(1, "AUDIO");
            put(2, "VIDEO" );
            put(3, "TEXT" );
            put(4, "APPLICATION" );
            put(5, "MESSAGE" );
            put(6, "CONTROL");
            put(7, "DATA");
        }
    };

    HashMap<Integer, String> ucIpVersionMapping = new HashMap<Integer, String>() {
        {
            put(0, "UNKNOWN");
            put(1, "IPv4");
            put(2, "IPv6" );
        }
    };

    @Override
    public void update(Object msgName, Object objectMsg) {
        Msg data = (Msg) objectMsg;
        String name = msgName instanceof String ? (String)msgName : null;
        if(name == null) return;
        int dwSsrc= mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DWSSRC);
        int dwDuration= mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DWDURATION);
        int dwPktCount= mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DWPKTCOUNT);
        int dwPktLoss= mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DWPKTLOSS);
        int dwByteCount= mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_DWBYTECOUNT);
        int wDstPort= mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_WDSTPORT);
        int wMeanJitter= mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_WMEANJITTER);
        int wMaxJitter= mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_WMAXJITTER);
        int wMaxDelta= mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_WMAXDELTA);
        int wCumAvgPktSize= mMdmComponent.getFieldValue(data,
                MDMContent.MSG_VALUE_WCUMAVGPKTSIZE);
        int ucMediaType= mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_UCMEDIATYPE);
        int ucIpVersion= mMdmComponent.getFieldValue(data, MDMContent.MSG_VALUE_UCIPVERSION);
        String strIpSrcAddr = "";
        String coNameIpSrcAddr = MDMContent.MSG_VALUE_STRIPSRCADDR + "[";
        if(ucIpVersion == 1) {
            for(int i=0; i<4; i++) {
                int curIpSrcAddr = mMdmComponent.getFieldValue(data, coNameIpSrcAddr+i+"]");
                strIpSrcAddr += (strIpSrcAddr.equals("") ? curIpSrcAddr : ("." + curIpSrcAddr));
            }
        } else {
            int lastIpSrcAddr = mMdmComponent.getFieldValue(data, coNameIpSrcAddr + "0]");
            for(int i=1; i<16; i++) {
                int curIpSrcAddr = mMdmComponent.getFieldValue(data, coNameIpSrcAddr+i+"]");
                if (i % 2 == 1) {
                    String ipStr = String.format("%02x", lastIpSrcAddr) + String.format("%02x", curIpSrcAddr);
                    strIpSrcAddr += (strIpSrcAddr.equals("") ? ipStr : (":" + ipStr));
                }
                lastIpSrcAddr = curIpSrcAddr;
            }
        }

        updateValues.clear();
        updateValues.put(mLabels[0], dwSsrc);
        updateValues.put(mLabels[1], dwDuration);
        updateValues.put(mLabels[2], dwPktCount);
        updateValues.put(mLabels[3], dwPktLoss);
        updateValues.put(mLabels[4], dwByteCount);
        updateValues.put(mLabels[5], wDstPort);
        updateValues.put(mLabels[6], wMeanJitter);
        updateValues.put(mLabels[7], wMaxJitter);
        updateValues.put(mLabels[8], wMaxDelta);
        updateValues.put(mLabels[9], wCumAvgPktSize);
        updateValues.put(mLabels[10], ucMediaTypeMapping.containsKey(ucMediaType) ?
                ucMediaTypeMapping.get(ucMediaType) : "-(" + ucMediaType + ")");
        updateValues.put(mLabels[11], ucIpVersionMapping.containsKey(ucIpVersion) ?
                ucIpVersionMapping.get(ucIpVersion) : "-(" + ucIpVersion + ")");
        updateValues.put(mLabels[12], strIpSrcAddr);
        mMdmComponent.getBuilder().updateDataOnView(updateValues);;
    }

    @Override
    public String[] getLabels() {
        return mLabels;
    }

    @Override
    public void resetValues() {
        updateValues.clear();
    }

    @Override
    public void test() {

    }
}

