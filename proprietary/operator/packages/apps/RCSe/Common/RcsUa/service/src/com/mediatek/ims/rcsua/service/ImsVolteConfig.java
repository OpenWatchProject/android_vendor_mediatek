/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.ims.rcsua.service;

import android.content.Context;
import android.content.res.XmlResourceParser;
import android.os.SystemProperties;

import com.mediatek.ims.rcsua.service.R;
import com.mediatek.ims.rcsua.service.utils.Logger;
import com.mediatek.ims.rcsua.service.utils.Utils;

import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParser;

import java.io.IOException;
import java.util.ArrayList;

public class ImsVolteConfig {
    Logger logger = Logger.getLogger(ImsVolteConfig.class.getName());
    private ArrayList < configItem > mConfigs = new ArrayList < configItem > ();

    public class configItem {
        public String name = null;
        public String value = null;

        public String toString() {
            return "\n\tname=" + name + ", value=" + value;
        }
    }

    /**
     * name@class configItemm, total=110
     */
    private static final String CFG_SYSTEM_OPERATOR_ID = "System_Operator_ID";
    private static final String CFG_NET_LOCAL_ADDRESS = "Net_Local_Address";
    private static final String CFG_NET_LOCAL_PORT = "Net_Local_Port";
    private static final String CFG_NET_LOCAL_PROTOCOL_TYPE = "Net_Local_Protocol_Type";
    private static final String CFG_NET_LOCAL_PROTOCOL_VERSION = "Net_Local_Protocol_Version";
    private static final String CFG_NET_LOCAL_IPSEC_PORT_START = "Net_Local_IPSec_Port_Start";
    private static final String CFG_NET_LOCAL_IPSEC_PORT_RANGE = "Net_Local_IPSec_Port_Range";
    private static final String CFG_NET_IPSEC = "Net_IPSec";
    private static final String CFG_NET_IF_NAME = "Net_IF_Name";
    private static final String CFG_NET_NETWORK_ID = "Net_Network_Id";
    private static final String CFG_NET_SIP_DSCP = "Net_SIP_Dscp";
    private static final String CFG_NET_SIP_SOC_PRIORITY = "Net_SIP_Soc_Priority";
    private static final String CFG_NET_SIP_SOC_TCP_MSS = "Net_SIP_Soc_Tcp_Mss";
    private static final String CFG_NET_PCSCF_PORT = "Net_PCSCF_Port";
    private static final String CFG_NET_PCSCF_NUMBER = "Net_PCSCF_Number";
    private static final String CFG_ACCOUNT_PRIVATE_UID = "Account_Private_UID";
    private static final String CFG_ACCOUNT_HOME_URI = "Account_Home_URI";
    private static final String CFG_ACCOUNT_IMEI = "Account_IMEI";
    private static final String CFG_SERVER_PCSCF_LIST = "Server_PCSCF_List";
    private static final String CFG_VOLTE_CALL_USERAGENT = "VoLTE_Call_UserAgent";
    private static final String CFG_REG_REGISTER_EXPIRY = "Reg_Register_Expiry";
    private static final String CFG_REG_URI_WITH_PORT = "Reg_URI_With_Port";
    private static final String CFG_REG_IPSEC_ALGO_SET = "Reg_IPSec_Algo_Set";
    private static final String CFG_REG_ENABLE_HTTP_DIGEST = "Reg_Enable_HTTP_Digest";
    private static final String CFG_REG_AUTH_NAME = "Reg_Auth_Name";
    private static final String CFG_REG_AUTH_PASSWORD = "Reg_Auth_Password";
    private static final String CFG_REG_SPECIFIC_IPSEC_ALGO = "Reg_Specific_IPSec_Algo";
    private static final String CFG_REG_CONTACT_WITH_USERNAME = "Reg_CONTACT_WITH_USERNAME";
    private static final String CFG_REG_CONTACT_WITH_TRANSPORT = "Reg_CONTACT_WITH_TRANSPORT";
    private static final String CFG_REG_CONTACT_WITH_REGID = "Reg_CONTACT_WITH_REGID";
    private static final String CFG_REG_CONTACT_WITH_MOBILITY = "Reg_CONTACT_WITH_MOBILITY";
    private static final String CFG_REG_CONTACT_WITH_EXPIRES = "Reg_CONTACT_WITH_EXPIRES";
    private static final String CFG_REG_AUTHORIZATION_WITH_ALGO = "Reg_AUTHORIZATION_WITH_ALGO";
    private static final String CFG_REG_REREG_IN_RAT_CHANGE = "Reg_REREG_IN_RAT_CHANGE";
    private static final String CFG_REG_REREG_IN_OOS_END = "Reg_REREG_IN_OOS_END";
    private static final String CFG_REG_DE_SUBSCRIBE = "Reg_DE_SUBSCRIBE";
    private static final String CFG_REG_USE_SPECIFIC_IPSEC_ALGO = "Reg_USE_SPECIFIC_IPSEC_ALGO";
    private static final String CFG_REG_TRY_NEXT_PCSCF = "Reg_TRY_NEXT_PCSCF";
    private static final String CFG_REG_DEREG_CLEAR_IPSEC = "Reg_DEREG_CLEAR_IPSEC";
    private static final String CFG_REG_INITIAL_REG_WITHOUT_PANI = "Reg_INITIAL_REG_WITHOUT_PANI";
    private static final String CFG_REG_DEREG_RESET_TCP_CLIENT = "Reg_DEREG_RESET_TCP_CLIENT";
    private static final String CFG_REG_TREG = "Reg_TREG";
    private static final String CFG_REG_REREG_23G4 = "Reg_REREG_23G4";
    private static final String CFG_REG_RESUB_23G4 = "Reg_RESUB_23G4";
    private static final String CFG_REG_NOT_AUTO_REG_403 = "Reg_NOT_AUTO_REG_403";
    private static final String CFG_REG_CALL_ID_WITH_HOST_INREG = "Reg_CALL_ID_WITH_HOST_INREG";
    private static final String CFG_REG_KEEP_ALIVE_MODE = "Reg_KEEP_ALIVE_MODE";
    private static final String CFG_REG_TCP_CONNECT_MAX_TIME_INVITE = "Reg_TCP_CONNECT_MAX_TIME_INVITE";
    private static final String CFG_REG_EMS_MODE_IND = "Reg_EMS_MODE_IND";
    private static final String CFG_REG_CONTACT_WITH_ACCESSTYPE = "Reg_CONTACT_WITH_ACCESSTYPE";
    private static final String CFG_REG_WFC_WITH_PLANI = "Reg_WFC_WITH_PLANI";
    private static final String CFG_REG_USE_UDP_ON_TCP_FAIL = "Reg_USE_UDP_ON_TCP_FAIL";
    private static final String CFG_REG_IPSEC_FAIL_ALLOWED = "Reg_IPSEC_FAIL_ALLOWED";
    private static final String CFG_REG_CONTACT_WITH_VIDEO_FEATURE_TAG_IN_SUBSCRIBE = "Reg_CONTACT_WITH_VIDEO_FEATURE_TAG_IN_SUBSCRIBE";
    private static final String CFG_REG_VIA_WITHOUT_RPORT = "Reg_VIA_WITHOUT_RPORT";
    private static final String CFG_REG_REG_ROUTE_HDR = "Reg_REG_ROUTE_HDR";
    private static final String CFG_REG_VIA_URI_WITH_DEFAULT_PORT = "Reg_VIA_URI_WITH_DEFAULT_PORT";
    private static final String CFG_REG_NOTIFY_SMS_NOTIFY_DONE = "Reg_NOTIFY_SMS_NOTIFY_DONE";
    private static final String CFG_REG_EMERGENCY_USE_IMSI = "Reg_EMERGENCY_USE_IMSI";
    private static final String CFG_REG_CHECK_MSISDN = "Reg_CHECK_MSISDN";
    private static final String CFG_REG_RETRY_INTERVAL_AFTER_403 = "Reg_RETRY_INTERVAL_AFTER_403";
    private static final String CFG_REG_SUPPORT_THROTTLING_ALGO = "Reg_SUPPORT_THROTTLING_ALGO";
    private static final String CFG_REG_REG_AFTER_NW_DEREG_60S = "Reg_REG_AFTER_NW_DEREG_60S";
    private static final String CFG_REG_SUB_CONTACT_WITH_SIP_INSTANCE = "Reg_SUB_CONTACT_WITH_SIP_INSTANCE";
    private static final String CFG_REG_STOP_REG_MD_LOWER_LAYER_ERR = "Reg_STOP_REG_MD_LOWER_LAYER_ERR";
    private static final String CFG_REG_REG_GRUU_SUPPORT = "Reg_REG_GRUU_SUPPORT";
    private static final String CFG_REG_OOS_END_RESET_TCP_CLIENT = "Reg_oos_end_reset_tcp_client";
    private static final String CFG_REG_PIDF_COUNTRY = "Reg_pidf_country";
    private static final String CFG_SIP_A_TIMER = "SIP_A_Timer";
    private static final String CFG_SIP_B_TIMER = "SIP_B_Timer";
    private static final String CFG_SIP_C_TIMER = "SIP_C_Timer";
    private static final String CFG_SIP_D_TIMER = "SIP_D_Timer";
    private static final String CFG_SIP_E_TIMER = "SIP_E_Timer";
    private static final String CFG_SIP_G_TIMER = "SIP_G_Timer";
    private static final String CFG_SIP_H_TIMER = "SIP_H_Timer";
    private static final String CFG_SIP_I_TIMER = "SIP_I_Timer";
    private static final String CFG_SIP_J_TIMER = "SIP_J_Timer";
    private static final String CFG_SIP_K_TIMER = "SIP_K_Timer";
    private static final String CFG_REG_FAIL_NOT_NEED_REMOVE_BINDING = "Reg_FAIL_NOT_NEED_REMOVE_BINDING";
    private static final String CFG_REG_REG_RCS_STATE = "Reg_REG_RCS_State";
    private static final String CFG_REG_PEND_DEREG_IN_INITIAL_REG = "Reg_PEND_DEREG_IN_INITIAL_REG";
    private static final String CFG_REG_TRY_NEXT_PCSCF_5626 = "Reg_TRY_NEXT_PCSCF_5626";
    private static final String CFG_REG_TRY_SAME_PCSCF_REREG = "Reg_TRY_SAME_PCSCF_REREG";
    private static final String CFG_REG_PUBLIC_UIDS = "Reg_PUBLIC_UIDS";
    private static final String CFG_REG_TRY_ALL_PCSCF_5626 = "Reg_TRY_ALL_PCSCF_5626";
    private static final String CFG_REG_REG_TDELAY = "Reg_REG_Tdelay";
    private static final String CFG_REG_TRY_N_NEXT_PCSCF_5626 = "Reg_Try_N_Next_Pcscf_5626";
    private static final String CFG_REG_DEREG_DELAY_TIME = "Reg_Dereg_Delay_Time";
    private static final String CFG_REG_CHECK_NOTIFY_INSTANCE_ID = "Reg_CHECK_NOTIFY_INSTANCE_ID";
    private static final String CFG_REG_ADD_CS_VOLTE_FEATURE_TAG = "Reg_ADD_CS_VOLTE_FEATURE_TAG";
    private static final String CFG_REG_CHECK_CALL_DOMAIN = "Reg_CHECK_CALL_DOMAIN";
    private static final String CFG_REG_SUPPORT_SIP_BLOCK = "Reg_SUPPORT_SIP_BLOCK";
    private static final String CFG_SIP_WHEN_STOP_B_TIMER = "SIP_WHEN_STOP_B_TIMER";
    private static final String CFG_REG_SUPPORT_RESUB = "Reg_support_resub";
    private static final String CFG_REG_EXPIRE_TIME_BY_USER = "Reg_Expire_Time_By_User";
    private static final String CFG_ADD_CNI_IN_WIFI = "Add_CNI_in_WIFI";
    private static final String CFG_REG_REG_OVER_TCP = "Reg_Reg_Over_Tcp";
    private static final String CFG_ADD_CELL_INFO_AGE_TO_CNI = "Add_cell_info_age_to_cni";
    private static final String CFG_SIP_CHECK_REG_CONTACT = "SIP_check_reg_contact";
    private static final String CFG_REG_GEOLOCATION_TYPE = "REG_Geolocation_Type";
    private static final String CFG_REG_INIT_REG_DELAY_TIME = "Reg_Init_Reg_Delay_Time";
    private static final String CFG_REG_UPDATE_IPSEC_PORT_494 = "Reg_Update_IPSec_Port_494";
    private static final String CFG_SIP_T1_TIMER = "SIP_T1_Timer";
    private static final String CFG_SIP_T2_TIMER = "SIP_T2_Timer";
    private static final String CFG_SIP_T4_TIMER = "SIP_T4_Timer";
    private static final String CFG_UA_REG_KEEP_ALIVE = "UA_reg_keep_alive";  //specific
    private static final String CFG_SIP_FORCE_USE_UDP = "SIP_Force_Use_UDP";
    private static final String CFG_SIP_TCP_ON_DEMAND = "SIP_TCP_On_Demand";
    private static final String CFG_SIP_TCP_MTU_SIZE = "SIP_TCP_MTU_Size";
    private static final String CFG_SIP_TRANSACTION_TIMER = "SIP_Transaction_Timer";

    private static final String DEFAULT_VALUE_STR = "NULL";
    private static final int DEFAULT_VALUE_INT = -1;
    /**
     * value@class configItem
     */
    public int mParam_system_operator_id;
    public String mParam_net_local_address;
    public int mParam_net_local_port;
    public int mParam_net_local_protocol_type;
    public int mParam_net_local_protocol_version;
    public int mParam_net_local_ipsec_port_start;
    public int mParam_net_local_ipsec_port_range;
    public int mParam_net_ipsec;
    public String mParam_net_if_name;
    public int mParam_net_network_id;
    public int mParam_net_sip_dscp;
    public int mParam_net_sip_soc_priority;
    public int mParam_net_sip_soc_tcp_mss;
    public int mParam_net_pcscf_port;
    public int mParam_net_pcscf_number;
    public String mParam_account_private_uid;
    public String mParam_account_home_uri;
    public String mParam_account_imei;
    public String mParam_server_pcscf_list;
    public String mParam_volte_call_useragent;
    public String mParam_reg_reg_rcs_state;
    public int mParam_reg_register_expiry;
    public int mParam_reg_uri_with_port;
    public int mParam_reg_ipsec_algo_set;
    public int mParam_reg_enable_http_digest;
    public int mParam_reg_auth_name;
    public int mParam_reg_auth_password;
    public int mParam_reg_specific_ipsec_algo;
    public int mParam_reg_contact_with_username;
    public int mParam_reg_contact_with_transport;
    public int mParam_reg_contact_with_regid;
    public int mParam_reg_contact_with_mobility;
    public int mParam_reg_contact_with_expires;
    public int mParam_reg_authorization_with_algo;
    public int mParam_reg_rereg_in_rat_change;
    public int mParam_reg_rereg_in_oos_end;
    public int mParam_reg_de_subscribe;
    public int mParam_reg_use_specific_ipsec_algo;
    public int mParam_reg_try_next_pcscf;
    public int mParam_reg_dereg_clear_ipsec;
    public int mParam_reg_initial_reg_without_pani;
    public int mParam_reg_dereg_reset_tcp_client;
    public int mParam_reg_treg;
    public int mParam_reg_rereg_23g4;
    public int mParam_reg_resub_23g4;
    public int mParam_reg_not_auto_reg_403;
    public int mParam_reg_call_id_with_host_inreg;
    public int mParam_reg_keep_alive_mode;
    public int mParam_reg_tcp_connect_max_time_invite;
    public int mParam_reg_ems_mode_ind;
    public int mParam_reg_contact_with_accesstype;
    public int mParam_reg_wfc_with_plani;
    public int mParam_reg_use_udp_on_tcp_fail;
    public int mParam_reg_ipsec_fail_allowed;
    public int mParam_reg_contact_with_video_feature_tag_in_subscribe;
    public int mParam_reg_via_without_rport;
    public int mParam_reg_reg_route_hdr;
    public int mParam_reg_via_uri_with_default_port;
    public int mParam_reg_notify_sms_notify_done;
    public int mParam_reg_emergency_use_imsi;
    public int mParam_reg_check_msisdn;
    public int mParam_reg_retry_interval_after_403;
    public int mParam_reg_support_throttling_algo;
    public int mParam_reg_reg_after_nw_dereg_60s;
    public int mParam_reg_sub_contact_with_sip_instance;
    public int mParam_reg_stop_reg_md_lower_layer_err;
    public int mParam_reg_reg_gruu_support;
    public int mParam_reg_oos_end_reset_tcp_client;
    public int mParam_reg_pidf_country;
    public int mParam_sip_a_timer;
    public int mParam_sip_b_timer;
    public int mParam_sip_c_timer;
    public int mParam_sip_d_timer;
    public int mParam_sip_e_timer;
    public int mParam_sip_g_timer;
    public int mParam_sip_h_timer;
    public int mParam_sip_i_timer;
    public int mParam_sip_j_timer;
    public int mParam_sip_k_timer;
    public int mParam_reg_fail_not_need_remove_binding;
    public int mParam_reg_pend_dereg_in_initial_reg;
    public int mParam_reg_try_next_pcscf_5626;
    public int mParam_reg_try_same_pcscf_rereg;
    public int mParam_reg_public_uids;
    public int mParam_reg_try_all_pcscf_5626;
    public int mParam_reg_reg_tdelay;
    public int mParam_reg_try_n_next_pcscf_5626;
    public int mParam_reg_dereg_delay_time;
    public int mParam_reg_check_notify_instance_id;
    public int mParam_reg_add_cs_volte_feature_tag;
    public int mParam_reg_check_call_domain;
    public int mParam_reg_support_sip_block;
    public int mParam_sip_when_stop_b_timer;
    public int mParam_reg_support_resub;
    public int mParam_reg_expire_time_by_user;
    public int mParam_add_cni_in_wifi;
    public int mParam_reg_reg_over_tcp;
    public int mParam_add_cell_info_age_to_cni;
    public int mParam_sip_check_reg_contact;
    public int mParam_reg_geolocation_type;
    public int mParam_reg_init_reg_delay_time;
    public int mParam_reg_update_ipsec_port_494;
    public int mParam_sip_t1_timer;
    public int mParam_sip_t2_timer;
    public int mParam_sip_t4_timer;
    public int mParam_ua_reg_keep_alive;
    public int mParam_sip_force_use_udp;
    public int mParam_sip_tcp_on_demand;
    public int mParam_sip_tcp_mtu_size;
    public int mParam_sip_transaction_timer;


    public ImsVolteConfig(Context ctx) {
        XmlResourceParser parser = ctx.getResources().getXml(R.xml.rcs_ims_config);
        if (parser == null) {
            logger.error("getXml(rcs_ims_config.xml) failed!");
            return;
        }
        resetParameters();

        try {
            int eventType = parser.getEventType();
            String name = parser.getName();
            String text = null;
            configItem config = null;

            while (eventType != XmlPullParser.END_DOCUMENT) {
                switch (eventType) {
                    case XmlPullParser.START_TAG:
                        if ("setting".equals(name)) {
                            config = new configItem();
                        }
                        break;
                    case XmlPullParser.END_TAG:
                        if ("label".equals(name)) {
                            config.name = text;
                        } else if ("default".equals(name)) {
                            config.value = text;
                        } else if ("setting".equals(name)) {
                            mConfigs.add(config);
                        } else if ("options".equals(name)) {}
                        break;
                    case XmlPullParser.TEXT:
                        text = parser.getText();
                        break;
                    default:
                } //switch()

                eventType = parser.next();
                name = parser.getName();
            } //while()

        } catch (IOException e) {
            logger.error("" + e);
        } catch (XmlPullParserException e) {
            logger.error("" + e);
        }
        // logger.debug("loading all configs: " + mConfigs);
    }

    public String toString01() {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("===== ImsVolteConfig, part 1 =====>");
        stringBuilder.append("\n\tmParam_system_operator_id = " + mParam_system_operator_id);
        stringBuilder.append("\n\tmParam_net_local_address = " + mParam_net_local_address);
        stringBuilder.append("\n\tmParam_net_local_port = " + mParam_net_local_port);
        stringBuilder.append("\n\tmParam_net_local_protocol_type = " + mParam_net_local_protocol_type);
        stringBuilder.append("\n\tmParam_net_local_protocol_version = " + mParam_net_local_protocol_version);
        stringBuilder.append("\n\tmParam_net_local_ipsec_port_start = " + mParam_net_local_ipsec_port_start);
        stringBuilder.append("\n\tmParam_net_local_ipsec_port_range = " + mParam_net_local_ipsec_port_range);
        stringBuilder.append("\n\tmParam_net_ipsec = " + mParam_net_ipsec);
        stringBuilder.append("\n\tmParam_net_if_name = " + mParam_net_if_name);
        stringBuilder.append("\n\tmParam_net_network_id = " + mParam_net_network_id);
        stringBuilder.append("\n\tmParam_net_sip_dscp = " + mParam_net_sip_dscp);
        stringBuilder.append("\n\tmParam_net_sip_soc_priority = " + mParam_net_sip_soc_priority);
        stringBuilder.append("\n\tmParam_net_sip_soc_tcp_mss = " + mParam_net_sip_soc_tcp_mss);
        stringBuilder.append("\n\tmParam_net_pcscf_port = " + mParam_net_pcscf_port);
        stringBuilder.append("\n\tmParam_net_pcscf_number = " + mParam_net_pcscf_number);
        stringBuilder.append("\n\tmParam_account_private_uid = " + mParam_account_private_uid);
        stringBuilder.append("\n\tmParam_account_home_uri = " + mParam_account_home_uri);
        stringBuilder.append("\n\tmParam_account_imei = " + mParam_account_imei);
        stringBuilder.append("\n\tmParam_server_pcscf_list = " + mParam_server_pcscf_list);
        stringBuilder.append("\n\tmParam_volte_call_useragent = " + mParam_volte_call_useragent);
        stringBuilder.append("\n\tmParam_reg_register_expiry = " + mParam_reg_register_expiry);
        stringBuilder.append("\n\tmParam_reg_uri_with_port = " + mParam_reg_uri_with_port);
        stringBuilder.append("\n\tmParam_reg_ipsec_algo_set = " + mParam_reg_ipsec_algo_set);
        stringBuilder.append("\n\tmParam_reg_enable_http_digest = " + mParam_reg_enable_http_digest);
        stringBuilder.append("\n\tmParam_reg_auth_name = " + mParam_reg_auth_name);
        stringBuilder.append("\n\tmParam_reg_auth_password = " + mParam_reg_auth_password);
        stringBuilder.append("\n\tmParam_reg_specific_ipsec_algo = " + mParam_reg_specific_ipsec_algo);
        stringBuilder.append("\n\tmParam_reg_contact_with_username = " + mParam_reg_contact_with_username);
        stringBuilder.append("\n\tmParam_reg_contact_with_transport = " + mParam_reg_contact_with_transport);
        stringBuilder.append("\n\tmParam_reg_contact_with_regid = " + mParam_reg_contact_with_regid);
        stringBuilder.append("\n\tmParam_reg_contact_with_mobility = " + mParam_reg_contact_with_mobility);
        stringBuilder.append("\n\tmParam_reg_contact_with_expires = " + mParam_reg_contact_with_expires);
        stringBuilder.append("\n\tmParam_reg_authorization_with_algo = " + mParam_reg_authorization_with_algo);
        stringBuilder.append("\n\tmParam_reg_rereg_in_rat_change = " + mParam_reg_rereg_in_rat_change);
        stringBuilder.append("\n\tmParam_reg_rereg_in_oos_end = " + mParam_reg_rereg_in_oos_end);
        stringBuilder.append("\n\tmParam_reg_de_subscribe = " + mParam_reg_de_subscribe);
        stringBuilder.append("\n\tmParam_reg_use_specific_ipsec_algo = " + mParam_reg_use_specific_ipsec_algo);
        stringBuilder.append("\n\tmParam_reg_try_next_pcscf = " + mParam_reg_try_next_pcscf);
        stringBuilder.append("\n\tmParam_reg_dereg_clear_ipsec = " + mParam_reg_dereg_clear_ipsec);
        stringBuilder.append("\n\tmParam_reg_initial_reg_without_pani = " + mParam_reg_initial_reg_without_pani);
        stringBuilder.append("\n\tmParam_reg_dereg_reset_tcp_client = " + mParam_reg_dereg_reset_tcp_client);
        stringBuilder.append("\n\tmParam_reg_treg = " + mParam_reg_treg);
        stringBuilder.append("\n\tmParam_reg_rereg_23g4 = " + mParam_reg_rereg_23g4);
        stringBuilder.append("\n\tmParam_reg_resub_23g4 = " + mParam_reg_resub_23g4);
        stringBuilder.append("\n\tmParam_reg_not_auto_reg_403 = " + mParam_reg_not_auto_reg_403);
        stringBuilder.append("\n\tmParam_reg_call_id_with_host_inreg = " + mParam_reg_call_id_with_host_inreg);
        stringBuilder.append("\n\tmParam_reg_keep_alive_mode = " + mParam_reg_keep_alive_mode);
        stringBuilder.append("\n\tmParam_reg_tcp_connect_max_time_invite = " + mParam_reg_tcp_connect_max_time_invite);
        stringBuilder.append("\n\tmParam_reg_ems_mode_ind = " + mParam_reg_ems_mode_ind);
        stringBuilder.append("\n\tmParam_reg_contact_with_accesstype = " + mParam_reg_contact_with_accesstype);
        stringBuilder.append("\n\tmParam_reg_wfc_with_plani = " + mParam_reg_wfc_with_plani);
        stringBuilder.append("\n\tmParam_reg_use_udp_on_tcp_fail = " + mParam_reg_use_udp_on_tcp_fail);
        stringBuilder.append("\n\tmParam_reg_ipsec_fail_allowed = " + mParam_reg_ipsec_fail_allowed);
        stringBuilder.append("\n\tmParam_reg_contact_with_video_feature_tag_in_subscribe = " + mParam_reg_contact_with_video_feature_tag_in_subscribe);
        stringBuilder.append("\n\tmParam_reg_via_without_rport = " + mParam_reg_via_without_rport);
        stringBuilder.append("\n\tmParam_reg_reg_route_hdr = " + mParam_reg_reg_route_hdr);
        stringBuilder.append("\n\tmParam_reg_via_uri_with_default_port = " + mParam_reg_via_uri_with_default_port);
        stringBuilder.append("\n\tmParam_reg_notify_sms_notify_done = " + mParam_reg_notify_sms_notify_done);
        stringBuilder.append("\n\tmParam_reg_emergency_use_imsi = " + mParam_reg_emergency_use_imsi);
        stringBuilder.append("\n\tmParam_reg_check_msisdn = " + mParam_reg_check_msisdn);
        stringBuilder.append("\n<===== ImsVolteConfig, part 1 =====\n");
        return stringBuilder.toString();
    }

    public String toString02() {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("===== ImsVolteConfig, part 2 =====>");
        stringBuilder.append("\n\tmParam_reg_retry_interval_after_403 = " + mParam_reg_retry_interval_after_403);
        stringBuilder.append("\n\tmParam_reg_support_throttling_algo = " + mParam_reg_support_throttling_algo);
        stringBuilder.append("\n\tmParam_reg_reg_after_nw_dereg_60s = " + mParam_reg_reg_after_nw_dereg_60s);
        stringBuilder.append("\n\tmParam_reg_sub_contact_with_sip_instance = " + mParam_reg_sub_contact_with_sip_instance);
        stringBuilder.append("\n\tmParam_reg_stop_reg_md_lower_layer_err = " + mParam_reg_stop_reg_md_lower_layer_err);
        stringBuilder.append("\n\tmParam_reg_reg_gruu_support = " + mParam_reg_reg_gruu_support);
        stringBuilder.append("\n\tmParam_reg_oos_end_reset_tcp_client = " + mParam_reg_oos_end_reset_tcp_client);
        stringBuilder.append("\n\tmParam_reg_pidf_country = " + mParam_reg_pidf_country);
        stringBuilder.append("\n\tmParam_sip_a_timer = " + mParam_sip_a_timer);
        stringBuilder.append("\n\tmParam_sip_b_timer = " + mParam_sip_b_timer);
        stringBuilder.append("\n\tmParam_sip_c_timer = " + mParam_sip_c_timer);
        stringBuilder.append("\n\tmParam_sip_d_timer = " + mParam_sip_d_timer);
        stringBuilder.append("\n\tmParam_sip_e_timer = " + mParam_sip_e_timer);
        stringBuilder.append("\n\tmParam_sip_g_timer = " + mParam_sip_g_timer);
        stringBuilder.append("\n\tmParam_sip_h_timer = " + mParam_sip_h_timer);
        stringBuilder.append("\n\tmParam_sip_i_timer = " + mParam_sip_i_timer);
        stringBuilder.append("\n\tmParam_sip_j_timer = " + mParam_sip_j_timer);
        stringBuilder.append("\n\tmParam_sip_k_timer = " + mParam_sip_k_timer);
        stringBuilder.append("\n\tmParam_reg_fail_not_need_remove_binding = " + mParam_reg_fail_not_need_remove_binding);
        stringBuilder.append("\n\tmParam_reg_reg_rcs_state = " + mParam_reg_reg_rcs_state);
        stringBuilder.append("\n\tmParam_reg_pend_dereg_in_initial_reg = " + mParam_reg_pend_dereg_in_initial_reg);
        stringBuilder.append("\n\tmParam_reg_try_next_pcscf_5626 = " + mParam_reg_try_next_pcscf_5626);
        stringBuilder.append("\n\tmParam_reg_try_same_pcscf_rereg = " + mParam_reg_try_same_pcscf_rereg);
        stringBuilder.append("\n\tmParam_reg_public_uids = " + mParam_reg_public_uids);
        stringBuilder.append("\n\tmParam_reg_try_all_pcscf_5626 = " + mParam_reg_try_all_pcscf_5626);
        stringBuilder.append("\n\tmParam_reg_reg_tdelay = " + mParam_reg_reg_tdelay);
        stringBuilder.append("\n\tmParam_reg_try_n_next_pcscf_5626 = " + mParam_reg_try_n_next_pcscf_5626);
        stringBuilder.append("\n\tmParam_reg_dereg_delay_time = " + mParam_reg_dereg_delay_time);
        stringBuilder.append("\n\tmParam_reg_check_notify_instance_id = " + mParam_reg_check_notify_instance_id);
        stringBuilder.append("\n\tmParam_reg_add_cs_volte_feature_tag = " + mParam_reg_add_cs_volte_feature_tag);
        stringBuilder.append("\n\tmParam_reg_check_call_domain = " + mParam_reg_check_call_domain);
        stringBuilder.append("\n\tmParam_reg_support_sip_block = " + mParam_reg_support_sip_block);
        stringBuilder.append("\n\tmParam_sip_when_stop_b_timer = " + mParam_sip_when_stop_b_timer);
        stringBuilder.append("\n\tmParam_reg_support_resub = " + mParam_reg_support_resub);
        stringBuilder.append("\n\tmParam_reg_expire_time_by_user = " + mParam_reg_expire_time_by_user);
        stringBuilder.append("\n\tmParam_add_cni_in_wifi = " + mParam_add_cni_in_wifi);
        stringBuilder.append("\n\tmParam_reg_reg_over_tcp = " + mParam_reg_reg_over_tcp);
        stringBuilder.append("\n\tmParam_add_cell_info_age_to_cni = " + mParam_add_cell_info_age_to_cni);
        stringBuilder.append("\n\tmParam_sip_check_reg_contact = " + mParam_sip_check_reg_contact);
        stringBuilder.append("\n\tmParam_reg_geolocation_type = " + mParam_reg_geolocation_type);
        stringBuilder.append("\n\tmParam_reg_init_reg_delay_time = " + mParam_reg_init_reg_delay_time);
        stringBuilder.append("\n\tmParam_reg_update_ipsec_port_494 = " + mParam_reg_update_ipsec_port_494);
        stringBuilder.append("\n\tmParam_sip_t1_timer = " + mParam_sip_t1_timer);
        stringBuilder.append("\n\tmParam_sip_t2_timer = " + mParam_sip_t2_timer);
        stringBuilder.append("\n\tmParam_sip_t4_timer = " + mParam_sip_t4_timer);
        stringBuilder.append("\n\tmParam_ua_reg_keep_alive = " + mParam_ua_reg_keep_alive);
        stringBuilder.append("\n\tmParam_sip_force_use_udp = " + mParam_sip_force_use_udp);
        stringBuilder.append("\n\tmParam_sip_tcp_on_demand = " + mParam_sip_tcp_on_demand);
        stringBuilder.append("\n\tmParam_sip_tcp_mtu_size = " + mParam_sip_tcp_mtu_size);
        stringBuilder.append("\n\tmParam_sip_transaction_timer = " + mParam_sip_transaction_timer);
        stringBuilder.append("\n<===== ImsVolteConfig, part 2 =====\n");
        return stringBuilder.toString();
    }

    public void doAssignParam() {
        if (mConfigs.size() == 0) {
            logger.error("no config record from xml!");
            return;
        }

        for (configItem config: mConfigs) {
            if (mParam_system_operator_id == DEFAULT_VALUE_INT && config.name.equals(CFG_SYSTEM_OPERATOR_ID)) {
                mParam_system_operator_id = parseInt(config.value);
                continue;
            } else
            if (mParam_net_local_address == DEFAULT_VALUE_STR && config.name.equals(CFG_NET_LOCAL_ADDRESS)) {
                mParam_net_local_address = config.value;
                continue;
            } else
            if (mParam_net_local_port == DEFAULT_VALUE_INT && config.name.equals(CFG_NET_LOCAL_PORT)) {
                mParam_net_local_port = parseInt(config.value);
                continue;
            } else
            if (mParam_net_local_protocol_type == DEFAULT_VALUE_INT && config.name.equals(CFG_NET_LOCAL_PROTOCOL_TYPE)) {
                mParam_net_local_protocol_type = parseInt(config.value);
                continue;
            } else
            if (mParam_net_local_protocol_version == DEFAULT_VALUE_INT && config.name.equals(CFG_NET_LOCAL_PROTOCOL_VERSION)) {
                mParam_net_local_protocol_version = parseInt(config.value);
                continue;
            } else
            if (mParam_net_local_ipsec_port_start == DEFAULT_VALUE_INT && config.name.equals(CFG_NET_LOCAL_IPSEC_PORT_START)) {
                mParam_net_local_ipsec_port_start = parseInt(config.value);
                continue;
            } else
            if (mParam_net_local_ipsec_port_range == DEFAULT_VALUE_INT && config.name.equals(CFG_NET_LOCAL_IPSEC_PORT_RANGE)) {
                mParam_net_local_ipsec_port_range = parseInt(config.value);
                continue;
            } else
            if (mParam_net_ipsec == DEFAULT_VALUE_INT && config.name.equals(CFG_NET_IPSEC)) {
                mParam_net_ipsec = parseInt(config.value);
                continue;
            } else
            if (mParam_net_if_name == DEFAULT_VALUE_STR && config.name.equals(CFG_NET_IF_NAME)) {
                mParam_net_if_name = config.value;
                continue;
            } else
            if (mParam_net_network_id == DEFAULT_VALUE_INT && config.name.equals(CFG_NET_NETWORK_ID)) {
                mParam_net_network_id = parseInt(config.value);
                continue;
            } else
            if (mParam_net_sip_dscp == DEFAULT_VALUE_INT && config.name.equals(CFG_NET_SIP_DSCP)) {
                mParam_net_sip_dscp = parseInt(config.value);
                continue;
            } else
            if (mParam_net_sip_soc_priority == DEFAULT_VALUE_INT && config.name.equals(CFG_NET_SIP_SOC_PRIORITY)) {
                mParam_net_sip_soc_priority = parseInt(config.value);
                continue;
            } else
            if (mParam_net_sip_soc_tcp_mss == DEFAULT_VALUE_INT && config.name.equals(CFG_NET_SIP_SOC_TCP_MSS)) {
                mParam_net_sip_soc_tcp_mss = parseInt(config.value);
                continue;
            } else
            if (mParam_net_pcscf_port == DEFAULT_VALUE_INT && config.name.equals(CFG_NET_PCSCF_PORT)) {
                mParam_net_pcscf_port = parseInt(config.value);
                continue;
            } else
            if (mParam_net_pcscf_number == DEFAULT_VALUE_INT && config.name.equals(CFG_NET_PCSCF_NUMBER)) {
                mParam_net_pcscf_number = parseInt(config.value);
                continue;
            } else
            if (mParam_account_private_uid == DEFAULT_VALUE_STR && config.name.equals(CFG_ACCOUNT_PRIVATE_UID)) {
                mParam_account_private_uid = config.value;
                continue;
            } else
            if (mParam_account_home_uri == DEFAULT_VALUE_STR && config.name.equals(CFG_ACCOUNT_HOME_URI)) {
                mParam_account_home_uri = config.value;
                continue;
            } else
            if (mParam_account_imei == DEFAULT_VALUE_STR && config.name.equals(CFG_ACCOUNT_IMEI)) {
                mParam_account_imei = config.value;
                continue;
            } else
            if (mParam_server_pcscf_list == DEFAULT_VALUE_STR && config.name.equals(CFG_SERVER_PCSCF_LIST)) {
                mParam_server_pcscf_list = config.value;
                continue;
            } else
            if (mParam_volte_call_useragent == DEFAULT_VALUE_STR && config.name.equals(CFG_VOLTE_CALL_USERAGENT)) {
                String custUa = SystemProperties.get("persist.vendor.roi.useragent");
                if (!custUa.isEmpty())
                    mParam_volte_call_useragent = custUa;
                else
                    mParam_volte_call_useragent = config.value;
                continue;
            } else
            if (mParam_reg_register_expiry == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_REGISTER_EXPIRY)) {
                mParam_reg_register_expiry = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_uri_with_port == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_URI_WITH_PORT)) {
                mParam_reg_uri_with_port = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_ipsec_algo_set == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_IPSEC_ALGO_SET)) {
                mParam_reg_ipsec_algo_set = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_enable_http_digest == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_ENABLE_HTTP_DIGEST)) {
                mParam_reg_enable_http_digest = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_auth_name == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_AUTH_NAME)) {
                mParam_reg_auth_name = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_auth_password == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_AUTH_PASSWORD)) {
                mParam_reg_auth_password = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_specific_ipsec_algo == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_SPECIFIC_IPSEC_ALGO)) {
                mParam_reg_specific_ipsec_algo = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_contact_with_username == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_CONTACT_WITH_USERNAME)) {
                mParam_reg_contact_with_username = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_contact_with_transport == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_CONTACT_WITH_TRANSPORT)) {
                mParam_reg_contact_with_transport = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_contact_with_regid == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_CONTACT_WITH_REGID)) {
                mParam_reg_contact_with_regid = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_contact_with_mobility == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_CONTACT_WITH_MOBILITY)) {
                mParam_reg_contact_with_mobility = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_contact_with_expires == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_CONTACT_WITH_EXPIRES)) {
                mParam_reg_contact_with_expires = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_authorization_with_algo == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_AUTHORIZATION_WITH_ALGO)) {
                mParam_reg_authorization_with_algo = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_rereg_in_rat_change == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_REREG_IN_RAT_CHANGE)) {
                mParam_reg_rereg_in_rat_change = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_rereg_in_oos_end == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_REREG_IN_OOS_END)) {
                mParam_reg_rereg_in_oos_end = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_de_subscribe == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_DE_SUBSCRIBE)) {
                mParam_reg_de_subscribe = parseInt(config.value);
                if (Utils.isTestSim()) {
                    logger.debug("update de_subscribe to 0 for test sim");
                    mParam_reg_de_subscribe = 0;
                }
                continue;
            } else
            if (mParam_reg_use_specific_ipsec_algo == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_USE_SPECIFIC_IPSEC_ALGO)) {
                mParam_reg_use_specific_ipsec_algo = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_try_next_pcscf == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_TRY_NEXT_PCSCF)) {
                mParam_reg_try_next_pcscf = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_dereg_clear_ipsec == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_DEREG_CLEAR_IPSEC)) {
                mParam_reg_dereg_clear_ipsec = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_initial_reg_without_pani == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_INITIAL_REG_WITHOUT_PANI)) {
                mParam_reg_initial_reg_without_pani = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_dereg_reset_tcp_client == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_DEREG_RESET_TCP_CLIENT)) {
                mParam_reg_dereg_reset_tcp_client = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_treg == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_TREG)) {
                mParam_reg_treg = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_rereg_23g4 == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_REREG_23G4)) {
                mParam_reg_rereg_23g4 = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_resub_23g4 == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_RESUB_23G4)) {
                mParam_reg_resub_23g4 = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_not_auto_reg_403 == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_NOT_AUTO_REG_403)) {
                mParam_reg_not_auto_reg_403 = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_call_id_with_host_inreg == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_CALL_ID_WITH_HOST_INREG)) {
                mParam_reg_call_id_with_host_inreg = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_keep_alive_mode == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_KEEP_ALIVE_MODE)) {
                mParam_reg_keep_alive_mode = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_tcp_connect_max_time_invite == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_TCP_CONNECT_MAX_TIME_INVITE)) {
                mParam_reg_tcp_connect_max_time_invite = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_ems_mode_ind == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_EMS_MODE_IND)) {
                mParam_reg_ems_mode_ind = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_contact_with_accesstype == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_CONTACT_WITH_ACCESSTYPE)) {
                mParam_reg_contact_with_accesstype = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_wfc_with_plani == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_WFC_WITH_PLANI)) {
                mParam_reg_wfc_with_plani = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_use_udp_on_tcp_fail == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_USE_UDP_ON_TCP_FAIL)) {
                mParam_reg_use_udp_on_tcp_fail = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_ipsec_fail_allowed == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_IPSEC_FAIL_ALLOWED)) {
                mParam_reg_ipsec_fail_allowed = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_contact_with_video_feature_tag_in_subscribe == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_CONTACT_WITH_VIDEO_FEATURE_TAG_IN_SUBSCRIBE)) {
                mParam_reg_contact_with_video_feature_tag_in_subscribe = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_via_without_rport == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_VIA_WITHOUT_RPORT)) {
                mParam_reg_via_without_rport = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_reg_route_hdr == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_REG_ROUTE_HDR)) {
                mParam_reg_reg_route_hdr = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_via_uri_with_default_port == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_VIA_URI_WITH_DEFAULT_PORT)) {
                mParam_reg_via_uri_with_default_port = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_notify_sms_notify_done == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_NOTIFY_SMS_NOTIFY_DONE)) {
                mParam_reg_notify_sms_notify_done = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_emergency_use_imsi == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_EMERGENCY_USE_IMSI)) {
                mParam_reg_emergency_use_imsi = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_check_msisdn == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_CHECK_MSISDN)) {
                mParam_reg_check_msisdn = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_retry_interval_after_403 == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_RETRY_INTERVAL_AFTER_403)) {
                mParam_reg_retry_interval_after_403 = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_support_throttling_algo == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_SUPPORT_THROTTLING_ALGO)) {
                mParam_reg_support_throttling_algo = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_reg_after_nw_dereg_60s == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_REG_AFTER_NW_DEREG_60S)) {
                mParam_reg_reg_after_nw_dereg_60s = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_sub_contact_with_sip_instance == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_SUB_CONTACT_WITH_SIP_INSTANCE)) {
                mParam_reg_sub_contact_with_sip_instance = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_stop_reg_md_lower_layer_err == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_STOP_REG_MD_LOWER_LAYER_ERR)) {
                mParam_reg_stop_reg_md_lower_layer_err = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_reg_gruu_support == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_REG_GRUU_SUPPORT)) {
                mParam_reg_reg_gruu_support = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_oos_end_reset_tcp_client == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_OOS_END_RESET_TCP_CLIENT)) {
                mParam_reg_oos_end_reset_tcp_client = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_pidf_country == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_PIDF_COUNTRY)) {
                mParam_reg_pidf_country = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_a_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_A_TIMER)) {
                mParam_sip_a_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_b_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_B_TIMER)) {
                mParam_sip_b_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_c_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_C_TIMER)) {
                mParam_sip_c_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_d_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_D_TIMER)) {
                mParam_sip_d_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_e_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_E_TIMER)) {
                mParam_sip_e_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_g_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_G_TIMER)) {
                mParam_sip_g_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_h_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_H_TIMER)) {
                mParam_sip_h_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_i_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_I_TIMER)) {
                mParam_sip_i_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_j_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_J_TIMER)) {
                mParam_sip_j_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_k_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_K_TIMER)) {
                mParam_sip_k_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_fail_not_need_remove_binding == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_FAIL_NOT_NEED_REMOVE_BINDING)) {
                mParam_reg_fail_not_need_remove_binding = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_reg_rcs_state == DEFAULT_VALUE_STR && config.name.equals(CFG_REG_REG_RCS_STATE)) {
                mParam_reg_reg_rcs_state = config.value;
                continue;
            } else
            if (mParam_reg_pend_dereg_in_initial_reg == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_PEND_DEREG_IN_INITIAL_REG)) {
                mParam_reg_pend_dereg_in_initial_reg = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_try_next_pcscf_5626 == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_TRY_NEXT_PCSCF_5626)) {
                mParam_reg_try_next_pcscf_5626 = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_try_same_pcscf_rereg == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_TRY_SAME_PCSCF_REREG)) {
                mParam_reg_try_same_pcscf_rereg = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_public_uids == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_PUBLIC_UIDS)) {
                mParam_reg_public_uids = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_try_all_pcscf_5626 == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_TRY_ALL_PCSCF_5626)) {
                mParam_reg_try_all_pcscf_5626 = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_reg_tdelay == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_REG_TDELAY)) {
                mParam_reg_reg_tdelay = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_try_n_next_pcscf_5626 == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_TRY_N_NEXT_PCSCF_5626)) {
                mParam_reg_try_n_next_pcscf_5626 = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_dereg_delay_time == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_DEREG_DELAY_TIME)) {
                mParam_reg_dereg_delay_time = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_check_notify_instance_id == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_CHECK_NOTIFY_INSTANCE_ID)) {
                mParam_reg_check_notify_instance_id = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_add_cs_volte_feature_tag == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_ADD_CS_VOLTE_FEATURE_TAG)) {
                mParam_reg_add_cs_volte_feature_tag = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_check_call_domain == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_CHECK_CALL_DOMAIN)) {
                mParam_reg_check_call_domain = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_support_sip_block == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_SUPPORT_SIP_BLOCK)) {
                mParam_reg_support_sip_block = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_when_stop_b_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_WHEN_STOP_B_TIMER)) {
                mParam_sip_when_stop_b_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_support_resub == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_SUPPORT_RESUB)) {
                mParam_reg_support_resub = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_expire_time_by_user == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_EXPIRE_TIME_BY_USER)) {
                mParam_reg_expire_time_by_user = parseInt(config.value);
                continue;
            } else
            if (mParam_add_cni_in_wifi == DEFAULT_VALUE_INT && config.name.equals(CFG_ADD_CNI_IN_WIFI)) {
                mParam_add_cni_in_wifi = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_reg_over_tcp == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_REG_OVER_TCP)) {
                mParam_reg_reg_over_tcp = parseInt(config.value);
                continue;
            } else
            if (mParam_add_cell_info_age_to_cni == DEFAULT_VALUE_INT && config.name.equals(CFG_ADD_CELL_INFO_AGE_TO_CNI)) {
                mParam_add_cell_info_age_to_cni = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_check_reg_contact == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_CHECK_REG_CONTACT)) {
                mParam_sip_check_reg_contact = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_geolocation_type == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_GEOLOCATION_TYPE)) {
                mParam_reg_geolocation_type = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_init_reg_delay_time == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_INIT_REG_DELAY_TIME)) {
                mParam_reg_init_reg_delay_time = parseInt(config.value);
                continue;
            } else
            if (mParam_reg_update_ipsec_port_494 == DEFAULT_VALUE_INT && config.name.equals(CFG_REG_UPDATE_IPSEC_PORT_494)) {
                mParam_reg_update_ipsec_port_494 = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_t1_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_T1_TIMER)) {
                mParam_sip_t1_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_t2_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_T2_TIMER)) {
                mParam_sip_t2_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_t4_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_T4_TIMER)) {
                mParam_sip_t4_timer = parseInt(config.value);
                continue;
            } else
            if (mParam_ua_reg_keep_alive == DEFAULT_VALUE_INT && config.name.equals(CFG_UA_REG_KEEP_ALIVE)) {
                mParam_ua_reg_keep_alive = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_force_use_udp == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_FORCE_USE_UDP)) {
                mParam_sip_force_use_udp = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_tcp_on_demand == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_TCP_ON_DEMAND)) {
                mParam_sip_tcp_on_demand = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_tcp_mtu_size == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_TCP_MTU_SIZE)) {
                mParam_sip_tcp_mtu_size = parseInt(config.value);
                continue;
            } else
            if (mParam_sip_transaction_timer == DEFAULT_VALUE_INT && config.name.equals(CFG_SIP_TRANSACTION_TIMER)) {
                mParam_sip_transaction_timer = parseInt(config.value);
                continue;
            }
        } //for()

    }

    private int parseInt(String s) {
        try {
            return Integer.parseInt(s);
        } catch (NumberFormatException e) {
            return DEFAULT_VALUE_INT;
        }
    }

    private void resetParameters() {
        mParam_system_operator_id = DEFAULT_VALUE_INT;
        mParam_net_local_address = DEFAULT_VALUE_STR;
        mParam_net_local_port = DEFAULT_VALUE_INT;
        mParam_net_local_protocol_type = DEFAULT_VALUE_INT;
        mParam_net_local_protocol_version = DEFAULT_VALUE_INT;
        mParam_net_local_ipsec_port_start = DEFAULT_VALUE_INT;
        mParam_net_local_ipsec_port_range = DEFAULT_VALUE_INT;
        mParam_net_ipsec = DEFAULT_VALUE_INT;
        mParam_net_if_name = DEFAULT_VALUE_STR;
        mParam_net_network_id = DEFAULT_VALUE_INT;
        mParam_net_sip_dscp = DEFAULT_VALUE_INT;
        mParam_net_sip_soc_priority = DEFAULT_VALUE_INT;
        mParam_net_sip_soc_tcp_mss = DEFAULT_VALUE_INT;
        mParam_net_pcscf_port = DEFAULT_VALUE_INT;
        mParam_net_pcscf_number = DEFAULT_VALUE_INT;
        mParam_account_private_uid = DEFAULT_VALUE_STR;
        mParam_account_home_uri = DEFAULT_VALUE_STR;
        mParam_account_imei = DEFAULT_VALUE_STR;
        mParam_server_pcscf_list = DEFAULT_VALUE_STR;
        mParam_volte_call_useragent = DEFAULT_VALUE_STR;
        mParam_reg_register_expiry = DEFAULT_VALUE_INT;
        mParam_reg_uri_with_port = DEFAULT_VALUE_INT;
        mParam_reg_ipsec_algo_set = DEFAULT_VALUE_INT;
        mParam_reg_enable_http_digest = DEFAULT_VALUE_INT;
        mParam_reg_auth_name = DEFAULT_VALUE_INT;
        mParam_reg_auth_password = DEFAULT_VALUE_INT;
        mParam_reg_specific_ipsec_algo = DEFAULT_VALUE_INT;
        mParam_reg_contact_with_username = DEFAULT_VALUE_INT;
        mParam_reg_contact_with_transport = DEFAULT_VALUE_INT;
        mParam_reg_contact_with_regid = DEFAULT_VALUE_INT;
        mParam_reg_contact_with_mobility = DEFAULT_VALUE_INT;
        mParam_reg_contact_with_expires = DEFAULT_VALUE_INT;
        mParam_reg_authorization_with_algo = DEFAULT_VALUE_INT;
        mParam_reg_rereg_in_rat_change = DEFAULT_VALUE_INT;
        mParam_reg_rereg_in_oos_end = DEFAULT_VALUE_INT;
        mParam_reg_de_subscribe = DEFAULT_VALUE_INT;
        mParam_reg_use_specific_ipsec_algo = DEFAULT_VALUE_INT;
        mParam_reg_try_next_pcscf = DEFAULT_VALUE_INT;
        mParam_reg_dereg_clear_ipsec = DEFAULT_VALUE_INT;
        mParam_reg_initial_reg_without_pani = DEFAULT_VALUE_INT;
        mParam_reg_dereg_reset_tcp_client = DEFAULT_VALUE_INT;
        mParam_reg_treg = DEFAULT_VALUE_INT;
        mParam_reg_rereg_23g4 = DEFAULT_VALUE_INT;
        mParam_reg_resub_23g4 = DEFAULT_VALUE_INT;
        mParam_reg_not_auto_reg_403 = DEFAULT_VALUE_INT;
        mParam_reg_call_id_with_host_inreg = DEFAULT_VALUE_INT;
        mParam_reg_keep_alive_mode = DEFAULT_VALUE_INT;
        mParam_reg_tcp_connect_max_time_invite = DEFAULT_VALUE_INT;
        mParam_reg_ems_mode_ind = DEFAULT_VALUE_INT;
        mParam_reg_contact_with_accesstype = DEFAULT_VALUE_INT;
        mParam_reg_wfc_with_plani = DEFAULT_VALUE_INT;
        mParam_reg_use_udp_on_tcp_fail = DEFAULT_VALUE_INT;
        mParam_reg_ipsec_fail_allowed = DEFAULT_VALUE_INT;
        mParam_reg_contact_with_video_feature_tag_in_subscribe = DEFAULT_VALUE_INT;
        mParam_reg_via_without_rport = DEFAULT_VALUE_INT;
        mParam_reg_reg_route_hdr = DEFAULT_VALUE_INT;
        mParam_reg_via_uri_with_default_port = DEFAULT_VALUE_INT;
        mParam_reg_notify_sms_notify_done = DEFAULT_VALUE_INT;
        mParam_reg_emergency_use_imsi = DEFAULT_VALUE_INT;
        mParam_reg_check_msisdn = DEFAULT_VALUE_INT;
        mParam_reg_retry_interval_after_403 = DEFAULT_VALUE_INT;
        mParam_reg_support_throttling_algo = DEFAULT_VALUE_INT;
        mParam_reg_reg_after_nw_dereg_60s = DEFAULT_VALUE_INT;
        mParam_reg_sub_contact_with_sip_instance = DEFAULT_VALUE_INT;
        mParam_reg_stop_reg_md_lower_layer_err = DEFAULT_VALUE_INT;
        mParam_reg_reg_gruu_support = DEFAULT_VALUE_INT;
        mParam_reg_oos_end_reset_tcp_client = DEFAULT_VALUE_INT;
        mParam_reg_pidf_country = DEFAULT_VALUE_INT;
        mParam_sip_a_timer = DEFAULT_VALUE_INT;
        mParam_sip_b_timer = DEFAULT_VALUE_INT;
        mParam_sip_c_timer = DEFAULT_VALUE_INT;
        mParam_sip_d_timer = DEFAULT_VALUE_INT;
        mParam_sip_e_timer = DEFAULT_VALUE_INT;
        mParam_sip_g_timer = DEFAULT_VALUE_INT;
        mParam_sip_h_timer = DEFAULT_VALUE_INT;
        mParam_sip_i_timer = DEFAULT_VALUE_INT;
        mParam_sip_j_timer = DEFAULT_VALUE_INT;
        mParam_sip_k_timer = DEFAULT_VALUE_INT;
        mParam_reg_fail_not_need_remove_binding = DEFAULT_VALUE_INT;
        mParam_reg_reg_rcs_state = DEFAULT_VALUE_STR;
        mParam_reg_pend_dereg_in_initial_reg = DEFAULT_VALUE_INT;
        mParam_reg_try_next_pcscf_5626 = DEFAULT_VALUE_INT;
        mParam_reg_try_same_pcscf_rereg = DEFAULT_VALUE_INT;
        mParam_reg_public_uids = DEFAULT_VALUE_INT;
        mParam_reg_try_all_pcscf_5626 = DEFAULT_VALUE_INT;
        mParam_reg_reg_tdelay = DEFAULT_VALUE_INT;
        mParam_reg_try_n_next_pcscf_5626 = DEFAULT_VALUE_INT;
        mParam_reg_dereg_delay_time = DEFAULT_VALUE_INT;
        mParam_reg_check_notify_instance_id = DEFAULT_VALUE_INT;
        mParam_reg_add_cs_volte_feature_tag = DEFAULT_VALUE_INT;
        mParam_reg_check_call_domain = DEFAULT_VALUE_INT;
        mParam_reg_support_sip_block = DEFAULT_VALUE_INT;
        mParam_sip_when_stop_b_timer = DEFAULT_VALUE_INT;
        mParam_reg_support_resub = DEFAULT_VALUE_INT;
        mParam_reg_expire_time_by_user = DEFAULT_VALUE_INT;
        mParam_add_cni_in_wifi = DEFAULT_VALUE_INT;
        mParam_reg_reg_over_tcp = DEFAULT_VALUE_INT;
        mParam_add_cell_info_age_to_cni = DEFAULT_VALUE_INT;
        mParam_sip_check_reg_contact = DEFAULT_VALUE_INT;
        mParam_reg_geolocation_type = DEFAULT_VALUE_INT;
        mParam_reg_init_reg_delay_time = DEFAULT_VALUE_INT;
        mParam_reg_update_ipsec_port_494 = DEFAULT_VALUE_INT;
        mParam_sip_t1_timer = DEFAULT_VALUE_INT;
        mParam_sip_t2_timer = DEFAULT_VALUE_INT;
        mParam_sip_t4_timer = DEFAULT_VALUE_INT;
        mParam_ua_reg_keep_alive = DEFAULT_VALUE_INT;
        mParam_sip_force_use_udp = DEFAULT_VALUE_INT;
        mParam_sip_tcp_on_demand = DEFAULT_VALUE_INT;
        mParam_sip_tcp_mtu_size = DEFAULT_VALUE_INT;
        mParam_sip_transaction_timer = DEFAULT_VALUE_INT;
    }
}