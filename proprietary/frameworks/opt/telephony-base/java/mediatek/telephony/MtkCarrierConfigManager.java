/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package mediatek.telephony;

import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;

public class MtkCarrierConfigManager {
    private final static String LOG_TAG = "MtkCarrierConfigMgr";

    /**
     * Add Mediatek proprietary key here with "MTK_KEY" leading.
     * For example:
     *   public static final String
     *       MTK_KEY_EXAMPLE_BOOL = "mtk_example_bool";
     */

     /**
     * @M: Added for config volte conference enhanced
     * Values: true if volte conference enhanced enable, else false
     * @hide
     */
    public static final String MTK_KEY_VOLTE_CONFERENCE_ENHANCED_ENABLE_BOOL =
            "mtk_volte_conference_enhanced_enable_bool";

    /**
     * @M: Added for allow one video call only
     * Values: true if allow one video call only, else false
     * @hide
     */
    public static final String MTK_KEY_ALLOW_ONE_VIDEO_CALL_ONLY_BOOL =
            "mtk_allow_one_video_call_only_bool";

    /**
     * International roaming exceptions that are defined by the carrier
     * @hide
     */
    public static final String KEY_CARRIER_INTERNATIONAL_ROAMING_EXCEPTION_LIST_STRINGS =
            "carrier_international_roaming_exception_list_strings";

    /**
     * Determine whether to remove WFC preference mode or not.
     * @hide
     */
    public static final String MTK_KEY_WFC_REMOVE_PREFERENCE_MODE_BOOL =
        "mtk_wfc_remove_preference_mode_bool";

   /**
    * Added for showing dialog when user access supplymentary services in Call Settings when
    * Mobile data disabled.
    * Values: true, shows the dialog to user to open mobile data else false
    * @hide
    */
    public static final String MTK_KEY_SHOW_OPEN_MOBILE_DATA_DIALOG_BOOL
            = "mtk_show_open_mobile_data_dialog_bool";

   /** Display Caller id settings menu if true
    * @hide
    */
    public static final String MTK_KEY_SHOW_CLIR_SETTING_BOOL = "mtk_show_clir_setting_bool";

    /**
     * @M: Default enhanced_4g_mode_enabled value
     * Values: true, if the default enhanced_4g_mode_enabled value is on else false
     * @hide
     */
    public static final String
            MTK_KEY_DEFAULT_ENHANCED_4G_MODE_BOOL = "mtk_default_enhanced_4g_mode_bool";

   /**
    * @M: Added for check if need support ALFMS00872039:The device shall use the
    * ADMINISTRATIVE APN in LTE area for the UICC/OTA data connection
    * @hide
    */
    public static final String MTK_KEY_USE_ADMINISTRATIVE_APN_BOOL = "mtk_use_administrative_apn_bool";

   /**
    * @M: Added for VDF Settings UX customization on the basis of SIM present
    * If value = 0, mode to change in WFC preferecne in Wireless Settings
    * if value = 1, mode to change WFC mode
    * @hide
    */
    public static final String MTK_KEY_WFC_SETTINGS_UX_TYPE_INT = "mtk_wfc_settings_ux_type_int";

    /**
    * @M: Added for VDF Settings UX customization on the basis of SIM present
    * Values as per ImsConfig.WfcModeFeatureValueConstants.
    * @hide
    */
    public static final String MTK_KEY_DEFAULT_WFC_MODE_INT = "mtk_default_wfc_mode_int";

    /**
    * @M: Added to indicate whether WFC provisioning via OMACP supported or not
    * Values: true, if IMS provisioning via OMACP supported else false
    * @hide
    */
    public static final String MTK_KEY_WFC_PROV_SUPPORTED_BOOL =
                                    "mtk_wfc_provisioning_supported_bool";

    /**
     * Added to check for Video Call support in Supplymentary services
     * @hide
     */
    public static final String MTK_KEY_SUPPORT_VT_SS_BOOL = "mtk_support_vt_ss_bool";

   /**
    * Added to support the call pull in dialer
    * Values: true, if Call pull is supported else false
    * @hide
    */
    public static final String MTK_KEY_DIALER_CALL_PULL_BOOL = "mtk_dialer_call_pull_bool";

    /**
     * Add for support change to one way video or not, default true.
     * Values: Default true, if not support change to one way video set false.
     * @hide
     */
    public static final String MTK_KEY_ALLOW_ONE_WAY_VIDEO_BOOL =
            "mtk_allow_one_way_video_bool";

    /**
     * Add for support cancel video update during the session progress.
     * Values: true if allow cancel the upgrade request.
     * @hide
     */
    public static final String MTK_KEY_ALLOW_CANCEL_VIDEO_UPGRADE_BOOL =
            "mtk_allow_cancel_video_upgrade_bool";

    /**
    * @M: IMS data retry requirements, including PCSC and RA fail.
    * Modem requires this information to do corresponding actions.
    * Default value is false.
    * @hide
    */
    public static final String KEY_IMS_PDN_SYNC_FAIL_CAUSE_TO_MODEM_BOOL =
            "ims_pdn_sync_fail_cause_to_modem_bool";

    /**
     * @M: Operator ID
     * AP requires this information to know which operator SIM card is inserted.
     * Default value is 0(OM or no SIM inserted).
     * @hide
     */
    public static final String KEY_OPERATOR_ID_INT = "operator_id";

    /**
     * Determined if domestic roaming is enabled only by mobile data setting
     * Default value is false
     * @hide
     */
    public static final String KEY_DOMESTIC_ROAMING_ENABLED_ONLY_BY_MOBILE_DATA_SETTING =
            "mtk_domestic_roaming_enabled_only_by_mobile_data_setting";

    /**
     * If the key is defined, the domestic roaming enabled by mobile data setting needs to verify
     * if the current camped network is listed in the key's valuse.
     * Default value is null
     * @hide
     */
    public static final String KEY_DOMESTIC_ROAMING_ENABLED_ONLY_BY_MOBILE_DATA_SETTING_CHECK_NW_PLMN =
            "mtk_domestic_roaming_enabled_only_by_mobile_data_setting_check_nw_plmn";

    /**
     * Determined if international roaming is enabled only by roaming data setting
     * Default value is false
     * @hide
     */
    public static final String KEY_INTL_ROAMING_ENABLED_ONLY_BY_ROAMING_DATA_SETTING =
            "mtk_intl_roaming_enabled_only_by_roaming_data_setting";

    /**
     * Determined if there are unique domestic and international roaming settings for both
     * domestic and international roaming, respectively. And the AOSP's default roaming
     * setting is not used.
     * Default value is false
     * @hide
     */
    public static final String KEY_UNIQUE_SETTINGS_FOR_DOMESTIC_AND_INTL_ROAMING =
            "mtk_unique_settings_for_domestic_and_intl_roaming";

    /**
     * Support UT over GSM.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_GSM_UT_SUPPORT =
            "mtk_carrier_ss_gsm_ut_support";

    /**
     * Not support xcap.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_NOT_SUPPORT_XCAP =
            "mtk_carrier_ss_not_support_xcap";

    /**
     * Support terminal base CLIR.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_TB_CLIR =
            "mtk_carrier_ss_tb_clir";

    /**
     * Support IMS network call waiting.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_IMS_NW_CW =
            "mtk_carrier_ss_ims_nw_cw";

    /**
     * Support response http 409 code.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_ENABLE_XCAP_HTTP_RESPONSE_409 =
            "mtk_carrier_ss_enable_xcap_http_response_409";

    /**
     * Support response http 404 code.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_TRANSFER_XCAP_404 =
            "mtk_carrier_ss_transfer_xcap_404";

    /**
     * Not support CLIR.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_NOT_SUPPORT_CALL_IDENTITY =
            "mtk_carrier_ss_not_support_call_identity";

    /**
     * Re-register for call forwarding.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_RE_REGISTER_FOR_CF =
            "mtk_carrier_ss_re_register_for_cf";

    /**
     * Save call forwarding number.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_SUPPORT_SAVE_CF_NUMBER =
            "mtk_carrier_ss_support_save_cf_number";

    /**
     * Query CFU after set.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_QUERY_CFU_AGAIN_AFTER_SET =
            "mtk_carrier_ss_query_cfu_again_after_set";

    /**
     * Not support OCB.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_NOT_SUPPORT_OCB =
            "mtk_carrier_ss_not_support_ocb";

   /**
    * Added to support the ct volte check in cellconnMgr
    * Values: true, if ct volte is support else false
    * @hide
    */
    public static final String MTK_KEY_CT_VOLTE_STATUS_BOOL = "mtk_ct_volte_status_bool";

    /**
     * Not support WFC for supplementary.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_NOT_SUPPORT_WFC_UT =
            "mtk_carrier_ss_not_support_wfc_ut";

    /**
     * Need to check data enable for supplemtary service.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_NEED_CHECK_DATA_ENABLE =
            "mtk_carrier_ss_need_check_data_enable";


    /**
     * Need to check data roaming for supplemtary service.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_NEED_CHECK_DATA_ROAMING =
            "mtk_carrier_ss_need_check_data_roaming";

    /**
     * Need to check ims when data roaming for supplemtary service.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SS_NEED_CHECK_IMS_WHEN_ROAMING =
            "mtk_carrier_ss_need_check_ims_when_roaming";

    /**
     * @M: Emergency bearer management policy
     * The priority to deactivate PDN before EMERGENCY PDN activation if PDN poll is full.
     * @hide
     */
    public static final String KEY_EMERGENCY_BEARER_MANAGEMENT_POLICY =
            "emergency_bearer_management_policy";

    /**
     * @M: Added to disable Emergency alert list from CellBroadcast app.
     * Values: true, if no need to show Cmas channels in list
     * @hide
     */
     public static final String KEY_CARRIER_AVOID_CMAS_LIST_BOOL = "carrier_avoid_cmas_list_bool";

    /**
     * Added to support settings for Sprint Roaming and bar settings.
     * @hide
     */
     public static final String MTK_KEY_ROAMING_BAR_GUARD_BOOL =
            "mtk_key_roaming_bar_guard_bool";

    /**
     * Support nourth America high priority CLIR such as *82
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_NOURTH_AMERICA_HIGH_PRIORITY_CLIR_PREFIX_SUPPORTED =
            "mtk_carrier_nouth_america_high_priority_clir_prefix_supported";

   /**
    * @M: Added to support WFC enable in flight mode
    * Values: true, if WFC is supported in flight mode & false if not.
    * @hide
    */
    public static final String MTK_KEY_WOS_SUPPORT_WFC_IN_FLIGHTMODE =
            "wos_flight_mode_support_bool";

    /**
    * @M: Added to support WFC prefer mode for domestic roaming.
    * Values: true, if WFC prefer mode should treat domestic roaming as HOME network.
    * @hide
    */
    public static final String MTK_KEY_WFC_MODE_DOMESTIC_ROMAING_TO_HOME =
            "mtk_carrier_wfc_mode_domestic_roaming_to_home";

    /**
    * @M: Specific operator request WFC to get location info always.
    * Whether or not it is ECC.
    * Whether or not user allow device to get location info.
    * Values: true, if operator request WFC to get location info always.
    * @hide
    */
    public static final String MTK_KEY_WFC_GET_LOCATION_ALWAYS =
            "mtk_carrier_wfc_get_location_always";

    /**
    * @M: Added to support disable WFC when receive auth fail.
    * Values: true, if WFC should disable after receive auth fail.
    * @hide
    */
    public static final String MTK_KEY_DISABLE_WFC_AFTER_AUTH_FAIL =
            "mtk_carrier_disable_wfc_after_auth_fail_bool";

    /**
     * Need to swap conference to foreground before merge
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SWAP_CONFERENCE_TO_FOREGROUND_BEFORE_MERGE =
            "mtk_carrier_swap_conference_to_foreground_before_merge";

    /**
     * Need to update dialing address from PAU
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_UPDATE_DIALING_ADDRESS_FROM_PAU =
            "mtk_carrier_update_dialing_address_from_pau";

    /**
     * Need to turn off wifi before E911
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_TURN_OFF_WIFI_BEFORE_E911 =
            "mtk_carrier_turn_off_wifi_before_e911";

    /**
     * IMS ECBM supported
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_IMS_ECBM_SUPPORTED =
            "mtk_carrier_ims_ecbm_supported";

    /**
     * WFC bad Wifi quality disconnect cause
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_NOTIFY_BAD_WIFI_QUALITY_DISCONNECT_CAUSE =
            "mtk_carrier_notify_bad_wifi_quality_disconnect_cause";

    /**
     * Support transmission of video stream when VideoCall goes to background.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_VIDEO_CALL_BACKGROUND_TRANSMISSION =
            "mtk_video_call_background_transmission";

    /**
     * Flag specifying whether switching WFC mode to cellular preferred is required.
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SWITCH_WFC_MODE_REQUIRED_BOOL =
            "mtk_carrier_switch_wfc_mode_required_bool";

    /**
     * Support blind / assured explicit call transfer
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_BLIND_ASSURED_ECT_SUPPORTED =
            "mtk_carrier_blind_assured_ect_supported";

    /**
     * Support consultative explicit call transfer
     * Default value is true
     * @hide
     */
    public static final String MTK_KEY_CONSULTATIVE_ECT_SUPPORTED =
            "mtk_carrier_consultative_ect_supported";

    /**
     * Support device switch
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_DEVICE_SWITCH_SUPPORTED =
            "mtk_carrier_device_switch_supported";

    /**
     * Support default dns
     * @hide
     */
    public static final String MTK_KEY_DEFAULT_DNS_ENABLED_BOOL =
            "default_dns_enabled_bool";

    /**
     * Add MnoApns into allApnList.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_ADD_MNOAPNS_INTO_ALLAPNLIST =
            "mtk_key_add_mnoapns_into_allapnlist";

    /**
     * Restore participants address for IMS conference.
     * @hide
     */
    public static final String MTK_KEY_RESTORE_ADDRESS_FOR_IMS_CONFERENCE_PARTICIPANTS =
            "mtk_key_restore_address_for_ims_conference_participants";

    /**
     * Operate IMS conference participants by user untity from CEP.
     * @hide
     */
    public static final String MTK_KEY_OPERATE_IMS_CONFERENCE_PARTICIPANTS_BY_USER_ENTITY =
            "mtk_key_operate_ims_conference_participants_by_user_entity";

    /**
     * @M: Whether merge/invite request is sent when adding new member to the conference
     * with max participants
     * Values: true, merge/invite request will be NOT sent after reaching max participants
     * @hide
     */
    public static final String MTK_KEY_IMS_NO_CONF_REQ_AFTER_MAX_CONNECTION_BOOL =
            "no_merge_req_after_max_connection";

    /**
     * @M: Show toast when adding new member to the conference with max participants
     * Default value is true
     * @hide
     */
    public static final String MTK_KEY_SHOW_TOAST_WHEN_CONFERENCE_FULL_BOOL =
            "show_toast_when_conference_full";

    /**
     * @M: No need to show roaming icon.
     * Need to show roaming icon on status bar or not
     * Default value is true
     */
    public static final String MTK_KEY_CARRIER_NEED_SHOW_ROAMING_ICON =
            "mtk_key_carrier_need_show_roaming_icon";

    /**
     * Added to disable Video Call over WIFI.
     * @hide
     */
    public static final String MTK_KEY_DISABLE_VT_OVER_WIFI_BOOL = "disable_vt_over_wifi_bool";


   /**
    * When {@code true}, changes to the mobile data roaming enabled switch will not cause the VT
    * registration state to change.  That is, turning on or off mobile data raoming will not cause VT to be
    * enabled or disabled when DUT is on roaming.
    * When {@code false}, disabling mobile data roaming settings will cause VT to be de-registered.
    * Default value is true
    * @hide
    */
   public static final String MTK_KEY_IGNORE_DATA_ROAMING_FOR_VIDEO_CALLS =
           "mtk_ignore_data_roaming_for_video_calls";

    /**
     * Need to check data enabled for Video Call over wifi enable.
     * @hide
     */
    public static final String MTK_KEY_VT_OVER_WIFI_CHECK_DATA_ENABLE_BOOL =
            "mtk_vt_over_wifi_check_data_enable_bool";

    /**
     * Need to check WFC enabled for Video Call over wifi enable.
     * @hide
     */
    public static final String MTK_KEY_VT_OVER_WIFI_CHECK_WFC_ENABLE_BOOL =
            "mtk_vt_over_wifi_check_wfc_enable_bool";

    /**
     * Need to check VoLTE enabled for Video Call over wifi enable.
     * @hide
     */
    public static final String MTK_KEY_VT_OVER_WIFI_CHECK_VOLTE_ENABLE_BOOL =
            "mtk_vt_over_wifi_check_volte_enable_bool";

   /**
    * Show Call Barring password when ims is avialable for CSFB operators.
    * Default value is false since no need show password when ims is avialable according to AOSP.
    * Values: true, need show call barring password for CSFB operators.
    * @hide
    */
    public static final String MTK_KEY_SHOW_CALL_BARRING_PASSWORD_BOOL
            = "mtk_show_call_barring_password_bool";

    /**
     * @M: Multiline feature call control allow cross line conference.
     * Values: true if multiline call control allow cross line conference.
     * @hide
     */
    public static final String MTK_KEY_MULTILINE_ALLOW_CROSS_LINE_CONFERENCE_BOOL =
            "mtk_key_multiline_allow_cross_line_conference_bool";

    /**
     * @M: Setting to indicate whether SIP message need to be sent on Pause/Unpause video
     * request or not.
     * Values: true, perform pause video without sending SIP message
     * @hide
     */
    public static final String MTK_KEY_IMS_NO_SIP_MESSAGE_ON_PAUSE_VIDEO_BOOL =
            "no_sip_message_on_pause_video_bool";

    /**
     * Added to support dialpad during video call for dtmf events.
     * @hide
     */
    public static final String MTK_KEY_VT_DIALPAD_SUPPORT_BOOL =
            "mtk_vt_dialpad_support_bool";

    /**
     * @M: Whether resume hold call automatically after active call ended by remote side.
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_RESUME_HOLD_CALL_AFTER_ACTIVE_CALL_END_BY_REMOTE =
            "mtk_resume_hold_call_after_active_call_end_by_remote";

    /**
     * @M: Setting to Control Auto reject video call upgrade request.
     * Values: true, automatically reject video call upgrade request after timer expiry.
     * @hide
     */
    public static final String MTK_KEY_IMS_SUPPORT_AUTO_DECLINE_UPGRADE_REQUEST_BOOL =
            "support_auto_decline_upgrade_request_bool";

    /**
     * Need to disable Wifi calling via tty
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_UPDATE_WIFICALLING_BY_TTY =
            "mtk_key_update_wificalling_by_tty";

    /**
     * Support auto start timer for cancel upgrade to video call
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_ALLOW_AUTO_START_TIMER_FOR_CANCEL_UPGRADE_BOOL =
            "mtk_allow_auto_start_timer_for_cancel_upgrade_bool";

    /**
     * Timer for cancel upgrade to video call.
     * Default value is 20
     * @hide
     */
    public static final String MTK_KEY_TIMER_FOR_CANCEL_UPGRADE_INT =
            "mtk_timer_for_cancel_upgrade_int";

    /**
     * Timer for decline upgrade to video call.
     * Default value is 20
     * @hide
     */
    public static final String MTK_KEY_TIMER_FOR_DECLINE_UPGRADE_INT =
            "mtk_timer_for_decline_upgrade_int";

    /**
     * Added to check the case if Operator server supports Conference management or not.
     * @hide
     */
    public static final String MTK_KEY_CONFERENCE_MANAGEMENT_SUPPORTED =
            "mtk_key_conference_management_supported";

    /**
     * Added to check the case if Operator server supports first participant as host.
     * @hide
     */
    public static final String MTK_KEY_CONF_FIRST_PARTICIPANT_AS_HOST_SUPPORTED =
            "mtk_key_conference_first_participant_as_host_supported";
    /**
     * Need to swap conference to foreground before merge
     * Default value is false
     * @hide
     */
    public static final String MTK_KEY_CARRIER_SWAP_VT_CONFERENCE_TO_FOREGROUND_BEFORE_MERGE =
            "mtk_carrier_swap_vt_conference_to_foreground_before_merge";

    /**
     * Should disconnect all call when placing emergency call.
     */
    public static final String MTK_KEY_DISCONNECT_ALL_CALLS_WHEN_ECC_BOOL =
            "mtk_key_disconnect_all_calls_when_ecc_bool";

    /**
     * Boolean to decide if participant in the conference allows merge or not
     * @hide
     */
    public static final String KEY_SHOW_MERGE_ON_PARTICIPANT =
            "show_merge_button_on_conference_participant";


    /**
     * Same as KEY_IMS_CONFERENCE_SIZE_LIMIT_INT but only for IMS video conference
     * @hide
     */
    public static final String MTK_KEY_IMS_VIDEO_CONFERENCE_SIZE_LIMIT_INT =
            "mtk_ims_video_conference_size_limit_int";

    /**
     * Need to check special constraint for IMS video conference
     * @hide
     */
    public static final String MTK_KEY_SPECIAL_CONSTRAINT_FOR_IMS_VIDEO_CONFERENCE =
            "mtk_special_constraint_for_ims_video_conference";

    /**
     * Show 4.5G option and remove network mode option and Volte Settings.
     */
    public static final String MTK_KEY_SHOW_45G_OPTIONS =
            "mtk_key_show_45g_options";

    /**
     * Indicates if carrier supports RTT and Video call switch.
     * @hide
     */
    public static final String MTK_KEY_RTT_VIDEO_SWITCH_SUPPORTED_BOOL =
            "mtk_rtt_video_switch_supported_bool";

    /**
     * Indicates if carrier supports RTT call merge to voice only conference call.
     * @hide
     */
    public static final String MTK_KEY_RTT_CALL_MERGE_SUPPORTED_BOOL =
            "mtk_rtt_call_merge_supported_bool";

    /**
     * Indicates if carrier wants to auto accept RTT upgrade/downgrade request
     * @hide
     */
    public static final String MTK_KEY_RTT_AUTO_ACCEPT_REQUEST_BOOL =
            "mtk_rtt_auto_accept_request_bool";

    /**
     * Indicates if carrier supports multiple RTT calls coexisting
     * @hide
     */
    public static final String MTK_KEY_MULTI_RTT_CALLS_SUPPORTED_BOOL =
            "mtk_multi_rtt_calls_supported_bool";

    /**
     * Indicates if carrier needs EMC RTT guard timer, ex. TMO
     * @hide
     */
    public static final String MTK_KEY_EMC_RTT_GUARD_TIMER_BOOL =
            "mtk_emc_rtt_guard_timer_bool";

    /**
     * Indicates if carrier is without precondition when MT RTT call CRING time, ex. VzW
     * @hide
     */
    public static final String MTK_KEY_MT_RTT_WITHOUT_PRECONDITION_BOOL =
            "mtk_mt_rtt_without_precondition_bool";

    /**
     * Indicates if carrier supports RTT and Video capability coexist.
     * Google default not support coexist.
     * @hide
     */
    public static final String MTK_KEY_RTT_VIDEO_CAPABILITY_COEXIST_BOOL =
            "mtk_rtt_video_capability_coexist_bool";

    /**
     * For call blocking enhancement feature, reject call with cause.
     * @hide
     */
    public static final String MTK_KEY_SUPPORT_ENHANCED_CALL_BLOCKING_BOOL =
            "mtk_support_enhanced_call_blocking_bool";

    /**
     * Should disallow outgoing video and voice call during video call.
     * Should disallow outgoing video call during voice call.
     * @hide
     */
    public static final String MTK_KEY_DISALLOW_OUTGOING_CALLS_DURING_VIDEO_OR_VOICE_CALL_BOOL =
            "mtk_disallow_outgoing_calls_during_video_or_voice_call_bool";

    /**
     * @hide
     * Should disallow outgoing call during conference call.
     */
    public static final String MTK_KEY_DISALLOW_OUTGOING_CALLS_DURING_CONFERENCE_BOOL =
            "mtk_disallow_outgoing_calls_during_conference_bool";

   /**
     * Otasp call number for provisioning.
     * @hide
     */
    public static final String MTK_KEY_OTASP_CALL_NUMBER_STRING =
            "mtk_key_otasp_call_number_string";

    /**
     * One roaming data setting to control domestic and international roaming data.
     * @hide
     */
    public static final String MTK_KEY_ONE_SETTING_FOR_DOMESTIC_AND_INTL_ROAMING_DATA =
            "mtk_one_setting_for_domestic_and_intl_roaming_data";

    /**
     * Single call after conference SRVCC.
     * @hide
     */
    public static final String MTK_KEY_SINGLE_CALL_AFTER_CONFERENCE_SRVCC =
            "mtk_single_call_after_conference_srvcc";

     /**
    * To control need ignore CLIR when ECC (set as presentation not restricted).
    * @hide
    */
    public static final String MTK_KEY_CARRIER_IGNORE_CLIR_WHEN_ECC_BOOL =
           "mtk_ignore_clir_when_ecc";

    /**
     * Need to send NETWORK_COVERAGE_LOST after APM for operator
     * @hide
     */
    public static final String MTK_KEY_SEND_NETWORK_COVERAGE_LOST =
            "mtk_send_network_coverage_lost";

    /**
     * Constructor function.
     */
    public MtkCarrierConfigManager() {
    }

    public static void putDefault(PersistableBundle sDefaults) {
        /* example: sDefaults.putBoolean(MTK_KEY_EXAMPLE_BOOL, true); */

        /* Set default value of volte, vilte and wfc to true as don't care in ImsManager */
        sDefaults.putBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL, true);
        sDefaults.putBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL, true);
        sDefaults.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        sDefaults.putBoolean(MTK_KEY_VOLTE_CONFERENCE_ENHANCED_ENABLE_BOOL, false);
        sDefaults.putBoolean(MTK_KEY_ALLOW_ONE_VIDEO_CALL_ONLY_BOOL, false);
        sDefaults.putStringArray(KEY_CARRIER_INTERNATIONAL_ROAMING_EXCEPTION_LIST_STRINGS,
                new String[]{""});
        // M: Default enhanced_4g_mode_enabled value
        sDefaults.putBoolean(MTK_KEY_DEFAULT_ENHANCED_4G_MODE_BOOL, true);
        // Used for BIP to check if need support KDDI feature
        sDefaults.putBoolean(MTK_KEY_USE_ADMINISTRATIVE_APN_BOOL, false);
        // M: WFC List Preference mode
        sDefaults.putBoolean(MTK_KEY_WFC_REMOVE_PREFERENCE_MODE_BOOL, false);
        /// M: config whether show CLIR Settings
        sDefaults.putBoolean(MTK_KEY_SHOW_CLIR_SETTING_BOOL, true);
        /// M: config whether show enable data dialog when doing SS
        sDefaults.putBoolean(MTK_KEY_SHOW_OPEN_MOBILE_DATA_DIALOG_BOOL, false);
        sDefaults.putInt(MTK_KEY_WFC_SETTINGS_UX_TYPE_INT, -1);
        /* Default value is in sync with ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED*/
        sDefaults.putInt(MTK_KEY_DEFAULT_WFC_MODE_INT, 2);
        sDefaults.putBoolean(MTK_KEY_WFC_PROV_SUPPORTED_BOOL, false);
        //M: VT support for SS
        sDefaults.putBoolean(MTK_KEY_SUPPORT_VT_SS_BOOL, false);
        //M: For Call pull support in Call logs
        sDefaults.putBoolean(MTK_KEY_DIALER_CALL_PULL_BOOL, false);
        // VILTE support change to one way video.
        sDefaults.putBoolean(MTK_KEY_ALLOW_ONE_WAY_VIDEO_BOOL, true);
        /// M: config cancel upgrade feature,including the timer and the start timer ways. @{
        // VILTE support cancel the video upgrade request.
        sDefaults.putBoolean(MTK_KEY_ALLOW_CANCEL_VIDEO_UPGRADE_BOOL, false);
        // config the way of start timer
        sDefaults.putBoolean(MTK_KEY_ALLOW_AUTO_START_TIMER_FOR_CANCEL_UPGRADE_BOOL, false);
        // the timer for cancel upgrade
        sDefaults.putInt(MTK_KEY_TIMER_FOR_CANCEL_UPGRADE_INT, 20);
        // the timer for decline upgrade
        sDefaults.putInt(MTK_KEY_TIMER_FOR_DECLINE_UPGRADE_INT, 20);
        /// @}
        // VILTE support pause ims video call
        sDefaults.putBoolean(CarrierConfigManager.KEY_SUPPORT_PAUSE_IMS_VIDEO_CALLS_BOOL, true);
        // VILTE support not ignore data eanbled changed
        sDefaults.putBoolean(CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS, false);
        // M: IMS data retry requirements
        sDefaults.putBoolean(KEY_IMS_PDN_SYNC_FAIL_CAUSE_TO_MODEM_BOOL, false);
        // M: Operator ID
        sDefaults.putInt(KEY_OPERATOR_ID_INT, 0);

        sDefaults.putBoolean(KEY_DOMESTIC_ROAMING_ENABLED_ONLY_BY_MOBILE_DATA_SETTING, false);
        sDefaults.putStringArray(
                KEY_DOMESTIC_ROAMING_ENABLED_ONLY_BY_MOBILE_DATA_SETTING_CHECK_NW_PLMN, null);
        sDefaults.putBoolean(KEY_INTL_ROAMING_ENABLED_ONLY_BY_ROAMING_DATA_SETTING, false);
        sDefaults.putBoolean(KEY_UNIQUE_SETTINGS_FOR_DOMESTIC_AND_INTL_ROAMING, false);

        // M: For Supplementary Service. @{
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_GSM_UT_SUPPORT, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_NOT_SUPPORT_XCAP, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_TB_CLIR, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_IMS_NW_CW, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_ENABLE_XCAP_HTTP_RESPONSE_409, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_TRANSFER_XCAP_404, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_NOT_SUPPORT_CALL_IDENTITY, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_RE_REGISTER_FOR_CF, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_SUPPORT_SAVE_CF_NUMBER, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_QUERY_CFU_AGAIN_AFTER_SET, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_NOT_SUPPORT_OCB, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_NOT_SUPPORT_WFC_UT, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_NEED_CHECK_DATA_ENABLE, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_NEED_CHECK_DATA_ROAMING, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SS_NEED_CHECK_IMS_WHEN_ROAMING, false);
        // M: For Supplementary Service. @}
        // M: Emergency bearer management policy
        sDefaults.putStringArray(KEY_EMERGENCY_BEARER_MANAGEMENT_POLICY,
                new String[]{""});
        //M: For CT VoLTE status check in CellConnMgr
        sDefaults.putBoolean(MTK_KEY_CT_VOLTE_STATUS_BOOL, false);
        //M: For Sprint roaming settings add bar settings
        sDefaults.putBoolean(MTK_KEY_ROAMING_BAR_GUARD_BOOL, false);
        // M: For IMS Call Control. @{
        sDefaults.putBoolean(
                MTK_KEY_CARRIER_NOURTH_AMERICA_HIGH_PRIORITY_CLIR_PREFIX_SUPPORTED, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SWAP_CONFERENCE_TO_FOREGROUND_BEFORE_MERGE, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_UPDATE_DIALING_ADDRESS_FROM_PAU, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_TURN_OFF_WIFI_BEFORE_E911, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_IMS_ECBM_SUPPORTED, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_NOTIFY_BAD_WIFI_QUALITY_DISCONNECT_CAUSE, false);
        sDefaults.putBoolean(MTK_KEY_VIDEO_CALL_BACKGROUND_TRANSMISSION, false);
        sDefaults.putBoolean(MTK_KEY_CARRIER_SWITCH_WFC_MODE_REQUIRED_BOOL, false);
        sDefaults.putBoolean(MTK_KEY_BLIND_ASSURED_ECT_SUPPORTED, false);
        sDefaults.putBoolean(MTK_KEY_CONSULTATIVE_ECT_SUPPORTED, true);
        sDefaults.putBoolean(MTK_KEY_DEVICE_SWITCH_SUPPORTED, false);
        sDefaults.putBoolean(MTK_KEY_RESTORE_ADDRESS_FOR_IMS_CONFERENCE_PARTICIPANTS, true);
        sDefaults.putBoolean(MTK_KEY_OPERATE_IMS_CONFERENCE_PARTICIPANTS_BY_USER_ENTITY, false);
        sDefaults.putBoolean(MTK_KEY_CONF_FIRST_PARTICIPANT_AS_HOST_SUPPORTED, false);
        sDefaults.putBoolean(MTK_KEY_SINGLE_CALL_AFTER_CONFERENCE_SRVCC, true);
        sDefaults.putBoolean(MTK_KEY_CARRIER_IGNORE_CLIR_WHEN_ECC_BOOL, true);
        // @}
        // M: default dns is enabled value
        sDefaults.putBoolean(MTK_KEY_DEFAULT_DNS_ENABLED_BOOL, true);

        sDefaults.putBoolean(MTK_KEY_ADD_MNOAPNS_INTO_ALLAPNLIST, false);
        //M: For WFC support status in flight mode
        sDefaults.putBoolean(MTK_KEY_WOS_SUPPORT_WFC_IN_FLIGHTMODE, true);

        //M: For WFC prefer mode for domestic roaming
        sDefaults.putBoolean(MTK_KEY_WFC_MODE_DOMESTIC_ROMAING_TO_HOME, false);

        //M: Specific operator request WFC to get location info always
        sDefaults.putBoolean(MTK_KEY_WFC_GET_LOCATION_ALWAYS, false);

        //M for disable WFC setting when receive auth fail.
        sDefaults.putBoolean(MTK_KEY_DISABLE_WFC_AFTER_AUTH_FAIL, false);

        sDefaults.putBoolean(KEY_CARRIER_AVOID_CMAS_LIST_BOOL, false);

        // M: Show toast when adding new member to the conference with max participants
        sDefaults.putBoolean(MTK_KEY_SHOW_TOAST_WHEN_CONFERENCE_FULL_BOOL, true);

        //M: Merge/invite request will be NOT sent after reaching max participants
        sDefaults.putBoolean(MTK_KEY_IMS_NO_CONF_REQ_AFTER_MAX_CONNECTION_BOOL, true);

        // M: need to show roaming icon or not
        sDefaults.putBoolean(MTK_KEY_CARRIER_NEED_SHOW_ROAMING_ICON, true);

        //M: For disabling video call over WIFI
        sDefaults.putBoolean(MTK_KEY_DISABLE_VT_OVER_WIFI_BOOL, false);

        //M: Show Call Barring password when ims is available for CSFB operators.
        sDefaults.putBoolean(MTK_KEY_SHOW_CALL_BARRING_PASSWORD_BOOL, false);

        // M: CC: For Call control related @{
        // M: CC: Enable GsmCdmaCall can be HD audio default setting
        sDefaults.putBoolean(CarrierConfigManager.KEY_GSM_CDMA_CALLS_CAN_BE_HD_AUDIO, true);

        // M: CC: Multiline feature support cross line conference
        sDefaults.putBoolean(MTK_KEY_MULTILINE_ALLOW_CROSS_LINE_CONFERENCE_BOOL, false);
        // @}

        // M: Set default value of call barring
        sDefaults.putBoolean(CarrierConfigManager.KEY_CALL_BARRING_VISIBILITY_BOOL, true);

        //M: Pause video without Sending SIP message
        sDefaults.putBoolean(MTK_KEY_IMS_NO_SIP_MESSAGE_ON_PAUSE_VIDEO_BOOL, false);

        // M: Added to support dialpad during video call for dtmf events.
        sDefaults.putBoolean(MTK_KEY_VT_DIALPAD_SUPPORT_BOOL, false);

        //M: resume hold call automatically after active call ended by remove side.
        sDefaults.putBoolean(MTK_KEY_RESUME_HOLD_CALL_AFTER_ACTIVE_CALL_END_BY_REMOTE, false);

        //M: Pause video without Sending SIP message
        sDefaults.putBoolean(MTK_KEY_IMS_SUPPORT_AUTO_DECLINE_UPGRADE_REQUEST_BOOL, true);

        //M: To update wfc settings via tty
        sDefaults.putBoolean(MTK_KEY_UPDATE_WIFICALLING_BY_TTY, false);

        // M: Added to support turn off ViLTE when DUT in roaming and roaming settings off.
        sDefaults.putBoolean(MTK_KEY_IGNORE_DATA_ROAMING_FOR_VIDEO_CALLS, false);

        //M: For swapping a video conf to foreground before merging participants in it
        sDefaults.putBoolean(MTK_KEY_CARRIER_SWAP_VT_CONFERENCE_TO_FOREGROUND_BEFORE_MERGE, false);

        // M: Control if we need to disconnect all call when placing emergency call.
        sDefaults.putBoolean(MTK_KEY_DISCONNECT_ALL_CALLS_WHEN_ECC_BOOL, true);
        sDefaults.putInt(MTK_KEY_IMS_VIDEO_CONFERENCE_SIZE_LIMIT_INT, 5);
        sDefaults.putBoolean(MTK_KEY_SPECIAL_CONSTRAINT_FOR_IMS_VIDEO_CONFERENCE, false);

        // M: Showing 4.5G Enable option for network mode as well Volte Settings.
        sDefaults.putBoolean(MTK_KEY_SHOW_45G_OPTIONS, false);

        // M: AOSP defined RTT carrier config default value
        sDefaults.putBoolean(CarrierConfigManager.KEY_RTT_AUTO_UPGRADE_BOOL, true);
        sDefaults.putBoolean(CarrierConfigManager.KEY_RTT_SUPPORTED_FOR_VT_BOOL, false);
        sDefaults.putBoolean(CarrierConfigManager.KEY_RTT_UPGRADE_SUPPORTED_BOOL, true);
        sDefaults.putBoolean(CarrierConfigManager.KEY_RTT_DOWNGRADE_SUPPORTED_BOOL, true);
        // M: indicates if carrier supports RTT and Video call switch
        sDefaults.putBoolean(MTK_KEY_RTT_VIDEO_SWITCH_SUPPORTED_BOOL, true);
        // M: indicates if carrier supports RTT call merge
        sDefaults.putBoolean(MTK_KEY_RTT_CALL_MERGE_SUPPORTED_BOOL, false);
        // M: indicates if carrier supports auto accept RTT upgrade/downgrade request
        sDefaults.putBoolean(MTK_KEY_RTT_AUTO_ACCEPT_REQUEST_BOOL, false);
        // M: indicates if carrier supports multiple RTT calls coexisting
        sDefaults.putBoolean(MTK_KEY_MULTI_RTT_CALLS_SUPPORTED_BOOL, false);
        // M: indicates if carrier needs EMC RTT guard timer
        sDefaults.putBoolean(MTK_KEY_EMC_RTT_GUARD_TIMER_BOOL, false);
        // M: indicates if carrier is without precondition when MT RTT call CRING time.
        sDefaults.putBoolean(MTK_KEY_MT_RTT_WITHOUT_PRECONDITION_BOOL, false);
        // M: indicates if carrier supports RTT and Video capability coexist
        sDefaults.putBoolean(MTK_KEY_RTT_VIDEO_CAPABILITY_COEXIST_BOOL, false);

        //M: Default no need to check data enabled for video call over wifi enable.
        sDefaults.putBoolean(MTK_KEY_VT_OVER_WIFI_CHECK_DATA_ENABLE_BOOL, false);
        //M: Default need to check wfc enabled for video call over wifi enable.
        sDefaults.putBoolean(MTK_KEY_VT_OVER_WIFI_CHECK_WFC_ENABLE_BOOL, true);
        //M: Default need to check volte enabled for video call over wifi enable.
        sDefaults.putBoolean(MTK_KEY_VT_OVER_WIFI_CHECK_VOLTE_ENABLE_BOOL, true);

        /// M: For enhanced call blocking, reject call with cause.
        sDefaults.putBoolean(MTK_KEY_SUPPORT_ENHANCED_CALL_BLOCKING_BOOL, false);

        ///M: Show 4G for LTE
        sDefaults.putBoolean(CarrierConfigManager.KEY_SHOW_4G_FOR_LTE_DATA_ICON_BOOL, true);

        //M: For disallow MO call during video or voice call.
        sDefaults.putBoolean(MTK_KEY_DISALLOW_OUTGOING_CALLS_DURING_VIDEO_OR_VOICE_CALL_BOOL, false);

        // M: For disallow MO call during conference call.
        sDefaults.putBoolean(MTK_KEY_DISALLOW_OUTGOING_CALLS_DURING_CONFERENCE_BOOL, false);

        // M: Otasp call number for provisioning.
        sDefaults.putString(MTK_KEY_OTASP_CALL_NUMBER_STRING, "");

        // M: Change for 5G. Config D.
        sDefaults.putString(CarrierConfigManager.KEY_5G_ICON_CONFIGURATION_STRING,
                "connected_mmwave:5G_PLUS,connected:5G,not_restricted:5G,restricted:4G");

        // M: One roaming data setting for domestic and international roaming data.
        sDefaults.putBoolean(MTK_KEY_ONE_SETTING_FOR_DOMESTIC_AND_INTL_ROAMING_DATA, false);

        // M: Set GNSS visibility control proxy app list
        sDefaults.putString(CarrierConfigManager.Gps.KEY_NFW_PROXY_APPS_STRING,
                "com.mediatek.gnss.nonframeworklbs");

        // M: For Version PCO feature
        sDefaults.putStringArray(CarrierConfigManager.KEY_CARRIER_APP_WAKE_SIGNAL_CONFIG_STRING_ARRAY,
                new String[]{
                        "com.android.carrierdefaultapp/.CarrierDefaultBroadcastReceiver:"
                                + "com.android.internal.telephony.CARRIER_SIGNAL_RESET",
                        "com.mediatek.ims.pco/.PCOBootCompletedReceiver:"
                                + "com.android.internal.telephony.CARRIER_SIGNAL_PCO_VALUE"
        });

        sDefaults.putBoolean(KEY_SHOW_MERGE_ON_PARTICIPANT, false);

        // M: Need to send NETWORK_COVERAGE_LOST after APM for operator.
        sDefaults.putBoolean(MTK_KEY_SEND_NETWORK_COVERAGE_LOST, false);
    }
}
