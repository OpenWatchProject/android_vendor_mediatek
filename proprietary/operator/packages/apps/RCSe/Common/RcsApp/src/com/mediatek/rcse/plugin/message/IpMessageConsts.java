/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.plugin.message;
/// M: add for ipmessage

public final class IpMessageConsts {

    public static final String STATUS = "status";
    public static final String RESULT = "result";
    public static final String ID = "id";
    public static final String NUMBER = "number";
    public static final String GROUP_START = "7---";
    public static final String MULTICHAT_START = "7+++";
    public static final String JOYN_START = "9+++";
    public static final String IPMESSAGE_NOTIFICATION = "ipmessage_notification";
    public static final String ACTION_GROUP_NOTIFICATION_CLICKED = "com.mediatek.mms.ipmessage.group_notification_clicked";
    public static final String ACTION_DEL_IP_MSG_DONE = "com.mediatek.mms.ipmessage.delIpMsgDone";
    public static final String ACTION_SERVICE_READY = "com.mediatek.mms.ipmessage.service.ready";
    public static final String ACTION_UPGRADE = "com.mediatek.mms.ipmessage.upgrade";

    public static final class NewMessageAction {
        public static final String ACTION_NEW_MESSAGE = "com.mediatek.mms.ipmessage.newMessage";
        public static final String IP_MESSAGE_KEY = "IpMessageKey";
    }

    public static final class RefreshContactList {
        public static final String ACTION_REFRESH_CONTACTS_LIST = "com.mediatek.mms.ipmessage.refreshContactList";
    }

    public static final class RefreshGroupList {
        public static final String ACTION_REFRESH_GROUP_LIST = "com.mediatek.mms.ipmessage.refreshGroupList";
    }

    public static final class UpdateGroup {
        public static final String UPDATE_GROUP_ACTION = "com.mediatek.mms.ipmessage.updateGroup";
        public static final String GROUP_ID = "group_id";
    }

    public static final class IntegrationMode {
        public static final int NORMAL = 0;
        public static final int FULLY_INTEGRATED = 1;
        public static final int CONVERGED_INBOX = 2;
        public static final int ISMS_MODE = 3;
    }

    public static final class JoynGroupInvite {
        public static final String ACTION_GROUP_IP_INVITATION = "com.mediatek.mms.ipmessage.groupinvitation";
    }

    // Add for Joyn
    public static final class DisableServiceStatus {
        public static final String ACTION_DISABLE_SERVICE_STATUS = "com.mediatek.mms.ipmessage.disableServiceStatus";
        public static final int ENABLE = 0;
        public static final int DISABLE_TEMPORARY = 1;
        public static final int DISABLE_PERMANENTLY = 2;
    }

    // Add for Joyn
    public static final class IsOnlyUseXms {
        public static final String ACTION_ONLY_USE_XMS = "com.mediatek.mms.ipmessage.onlyUseXms";
        public static final int ON  = 1;
        public static final int OFF = 0;
    }

    public static final class ChatMode {
        public static final int JOYN = 1;
        public static final int XMS  = 2;
    }

    public static final class ServiceStatus {
        public static final String ACTION_SERVICE_STATUS = "com.mediatek.mms.ipmessage.serviceStatus";
        public static final int ON  = 1;
        public static final int OFF = 0;
    }

    public static final class ImStatus {
        public static final String ACTION_IM_STATUS = "com.mediatek.mms.ipmessage.IMStatus";
        public static final String CONTACT_CURRENT_STATUS = "com.mediatek.mms.ipmessage.ContactStatus";
    }

    public static final class SimInfoChanged {
        public static final String SIM_INFO_ACTION = "com.mediatek.mms.ipmessage.SIMInfoChanged";
        public static final String SIM_ID = "nmsSIMId";
    }

    public static final class ConnectionStatus {
//        public static final int UNKONW = -2;
//        public static final int STATUS_INIT = 0;
        public static final int STATUS_UNCONNECTED = 1;
//        public static final int STATUS_BLOCKING    = 2;
        public static final int STATUS_CONNECTING  = 3;
        public static final int STATUS_CONNECTED   = 4;
    }

    /// M: add for ipmessage registration status @{
    public static final class RegStatus {
        public static final String ACTION_REG_STATUS = "com.mediatek.mms.ipmessage.regStatus";
        public static final String REGSTATUS = "regStatus";
        public static final int REG_OVER = 6;
        public static final int REG_IN_PROGRESS = 7;
        public static final int REG_FAIL = 8;
    }
    /// @}
    public static final class StoreStatus {
        public static final String ACTION_STORE_STATUS = "com.mediatek.mms.ipmessage.storeStatus";
        public static final String STORESTATUS = "storeStatus";
        public final static int STORE_FULL = 0;
        public final static int STORE_ERR = 1;
    }
    public static final class IpMessageStatus {
        /// M: add for ipmessage @{
        public static final String ACTION_MESSAGE_TIME = "com.mediatek.mms.ipmessage.time";
        public static final String MESSAGE_TIME = "message.time";
        /// @}
        public static final String ACTION_MESSAGE_STATUS = "com.mediatek.mms.ipmessage.messageStatus";
        public static final String IP_MESSAGE_FALLBACK_MMS = "com.mediatek.mms.ipmessage.fallbackMMS";
        public static final String IP_MESSAGE_ID = "com.mediatek.mms.ipmessage.IpMessageRecdId";
        public static final int FAILED = 0;
        public static final int OUTBOX_PENDING = 1; /* not ready to send yet */
        public static final int OUTBOX = 2;
        public static final int SENT   = 3;
        public static final int NOT_DELIVERED = 4;
        public static final int DELIVERED = 5;
        public static final int VIEWED = 6;
        public static final int DRAFT  = 7;
        public static final int INBOX  = 8;

        public static final int MO_TIMEOUT = 10;     // notified download file
        public static final int MO_INVITE = 11;     // notified download file
        public static final int MO_SENDING = 12;    // sending file
        public static final int MO_REJECTED = 13;   // reject sending file
        public static final int MO_SENT = 14;       // file has been sent
        public static final int MO_CANCEL = 15;     // cancel send file
        public static final int MT_INVITED = 21;    // receive a notification for downloading file
        public static final int MT_REJECT = 22;     // reject receive file
        public static final int MT_RECEIVING = 23;  // reveiving file
        public static final int MT_RECEIVED = 24;   // file has been received
        public static final int MT_CANCEL = 25;     // cancel receive file
        public static final int MO_PAUSE  = 26;     // restart from pause
        public static final int MO_RESUME = 27;     // pause the transfering
        public static final int MT_PAUSE  = 28;     // restart from pause
        public static final int MT_RESUME = 29;     // pause the transfering
        public static final int MEDIA_NOT_SUPPORTED = 30;     // media unsupported
        public static final int MO_FALLBACK = 31;     // media unsupported
        public static final int MO_DELIVERED = 32;     // media delivered
        public static final int MO_VIEWED = 33;     // media displayed
        /// M: add for ipmessage
        public static final int MT_NOT_RECEIVED = 100;
    }

    public static final class HandleIpMessageAction {
        public static final int TAP = 1;
    }

    public static final class DownloadAttachStatus {
        public static final String ACTION_DOWNLOAD_ATTACH_STATUS = "com.mediatek.mms.ipmessage.downloadAttachStatus";
        public static final String DOWNLOAD_PERCENTAGE = "DownloadPercentage";
        public static final String DOWNLOAD_MSG_ID = "DownloadMsgId";
        public static final String DOWNLOAD_MSG_STATUS = "DownloadMsgStatus";
        public static final int FAILED  = -1;
        public static final int STARTING = 0;
        public static final int DOWNLOADING = 1; // argument is the downloading percentage
        public static final int DONE = 2;
    }

    public static final class SetProfileResult {
        public static final String ACTION_SET_PROFILE_RESULT = "com.mediatek.mms.ipmessage.setProfileResult";
        public static final int SUCCEED = 0;
        public static final int FAILED  = 1; // or -1 is better?
    }

    public static final class BackupMsgStatus {
        public static final String ACTION_BACKUP_MSG_STATUS = "com.mediatek.mms.ipmessage.backupMsgStatus";
        public static final String UPLOADING_PERCENTAGE = "uploadingPercentage";
        public static final int STARTING = 0;
        public static final int UPLOADING = 1; // argument is the downloading percentage
        public static final int FAILED  = 2; // or -1 is better?
    }

    public static final class RestoreMsgStatus {
        public static final String ACTION_RESTORE_MSG_STATUS = "com.mediatek.mms.ipmessage.restoreMsgStatus";
        public static final String DOWNLOAD_PERCENTAGE = "DownloadPercentage";
        public static final int STARTING = 0;
        public static final int DOWNLOADING = 1; // argument is the downloading percentage
        public static final int FAILED  = 2; // or -1 is better?
    }

    public static final class IpMessageType {
        public static final int TEXT = 0;
        public static final int GROUP_CREATE_CFG = 1;
        public static final int GROUP_ADD_CFG = 2;
        public static final int GROUP_QUIT_CFG = 3;
        public static final int PICTURE = 4;
        public static final int VOICE = 5;
        public static final int VCARD = 6;
        public static final int VIDEO = 9;
        public static final int CALENDAR = 10;
        public static final int UNKNOWN_FILE = 11;
        public static final int COUNT = 12;
        /// M: add for ipmessage readburn
        public static final int READEDBURN = 13;
    }

    public static final class IpMessageMediaTypeFlag {
        public static final int PICTURE = 1 << IpMessageType.PICTURE;
        public static final int VOICE = 1 << IpMessageType.VOICE;
        public static final int VCARD = 1 << IpMessageType.VCARD;
        public static final int VIDEO = 1 << IpMessageType.VIDEO;
        public static final int CALENDAR = 1 << IpMessageType.CALENDAR;

        public static final int ALL = PICTURE | VOICE | VCARD | VIDEO | CALENDAR ;
    }

    public static final class IpMessageCategory {
        public static final int ALL = 0;
        public static final int FAVOURITE = 1;
        public static final int GROUPCHAT = 2;
        public static final int SPAM = 3;
    }

    public static final class MessageProtocol {
        public static final int IP  = 1;
        public static final int SMS = 2;
        public static final int MMS = 3;
    }

    public static final class IpMessageSendMode {
        public static final int NORMAL  = 0;
        public static final int AUTO = 1;
    }

    public static final class ContactType {
        public static final int NOT_HISSAGE_USER   = 0;
        public static final int HISSAGE_USER       = 1;
        public static final int HISSAGE_GROUP_CHAT = 2;
        public static final int HISSAGE_BROADCAST  = 3;
    }

    public static final class SelectContactType {
        public static final int ALL = 0;
        public static final int IP_MESSAGE_USER = 1;
        public static final int NOT_IP_MESSAGE_USER = 2;
    }

    public static final class ContactStatus {
        public static final String CONTACT_UPDATE = "com.mediatek.mms.ipmessage.contactUpdate";
        public static final int OFFLINE     = 0;
        public static final int ONLINE      = 1;
        public static final int TYPING      = 2;
        public static final int STOP_TYPING = 3;
        public static final int RECORDING   = 4;
        public static final int STOP_RECORDING   = 5;
        /// M: add new ipmessage user status @{
        public static final int UNSENDABLE = 8;
        public static final int STATUSCOUNT = 9;
        /// @}
    }

    public static final class SpecialSimId {
        public static final int INVALID_SIM_ID = -1;
        public static final int SINGLE_LOAD_SIM_ID = -2;
        public static final int ALL_SIM_ID = -3;
    }

    public static final class ReminderType {
        public static final int REMINDER_INVALID    = 0;
        public static final int REMINDER_INVITE     = 1;
        public static final int REMINDER_ACTIVATE   = 2;
        public static final int REMINDER_SWITCH     = 3;
        public static final int REMINDER_ENABLE     = 4;
    }

    /// M: add for ipmessage activation Statistics @{
    public static final class ActivateType {
        public static final int AUTO = 0 ;
        public static final int OTHER = 1 ;
        public static final int EMOTION = 2 ;
        public static final int MULTI_MEDIA = 3 ;
        public static final int SETTING = 4 ;
        public static final int DIALOG = 5 ;
        public static final int PROMPT = 7;
        public static final int MESSAGE = 8;
    }
    /// @}

    public static final class FeatureId {
        public static final int CHAT_SETTINGS = 1;
        public static final int APP_SETTINGS = 2;
        public static final int ACTIVITION = 3;
        public static final int ACTIVITION_WIZARD = 4;
        public static final int ALL_MEDIA = 6;      /// M: list all media messages in activity.
        public static final int MEDIA_DETAIL = 7;   /// M: displaying media detail info.
        public static final int GROUP_MESSAGE = 8;
        public static final int CONTACT_SELECTION = 9;
        public static final int TERM = 12;
        public static final int SAVE_CHAT_HISTORY = 13;
        public static final int SAVE_ALL_HISTORY = 14;
        public static final int SHARE_CHAT_HISTORY = 15;
        public static final int SHARE_ALL_HISTORY = 16;
        public static final int FILE_TRANSACTION = 17;
        /// M: add for ipmessage @{
        public static final int READEDBURN = 18;
        public static final int IPMESSAGE_SERVICE_CENTER = 19;
        public static final int IPMESSAGE_ACTIVATE_PROMPT = 20;
        /// @}
        /// M: add for ipmessage private message
        public static final int PRIVATE_MESSAGE = 21;
        public static final int EXTEND_GROUP_CHAT = 22;
        public static final int EXPORT_CHAT       = 23;
        public static final int PARSE_EMO_WITHOUT_ACTIVATE = 24;
    }

    public static final class ResourceId {
        public static final int STR_IPMESSAGE_SETTINGS = 1;
        public static final int STR_IPMESSAGE_RESENT = 2;
        public static final int STR_IPMESSAGE_ACCEPT = 3;
        public static final int STR_IPMESSAGE_REJECT = 4;
    }

    public static final class RemoteActivities {
        public static final String KEY_THREAD_ID        = "thread_id";      // key of thread id, long
        public static final String KEY_CONTACT_ID       = "contact_id";     // key of contact id,
        public static final String KEY_MESSAGE_ID       = "message_id";     // key of message id, int
        public static final String KEY_SIM_ID           = "sim_id";         // key of sim id, int
        public static final String KEY_URI              = "uri";            // key of uri
        public static final String KEY_REQUEST_CODE     = "request_code";   // key of request code, int
        public static final String KEY_NEED_NEW_TASK    = "need_new_task";  // key of need new task, boolean
        public static final String KEY_SIZE             = "size";           // key of size
        public static final String KEY_TYPE             = "type";           // key of type
        public static final String KEY_ARRAY            = "array";          // key of array
        public static final String KEY_BOOLEAN          = "boolean" ;       // key of boolean value
        public static final String KEY_GENERIC_1        = "generic_1";      // key for other cases: array, buddle, etc.
        public static final String KEY_GENERIC_2        = "generic_2";      // key for other cases: array, buddle, etc.
        public static final String KEY_GENERIC_3        = "generic_3";      // key for other cases: array, buddle, etc.
        /// M: add for ipmessage
        public static final String KEY_ACTIVATE_TYPE    = "activate_type" ;

        /// M: add for ipmessage readburn time
        public static final String KEY_MESSAGE_TIME     = "message_time" ;

        public static final int ID_CHAT_SETTINGS                = 1;
        public static final int ID_SYSTEM_SETTINGS              = 2;
        public static final int ID_ACTIVITION                   = 3;
        public static final int ID_ALL_MEDIA                    = 5;
        public static final int ID_CHAT_DETAILS_BY_THREAD_ID    = 7;
        public static final int ID_CONTACT                      = 8;
        public static final int ID_NON_IPMESSAGE_CONTACT        = 9;
        public static final int ID_NEW_GROUP_CHAT               = 10;
        public static final int ID_QUICK_CONTACT                = 11;
        public static final int ID_MEDIA_DETAIL                 = 12;
        public static final int ID_AUDIO                        = 14;
        public static final int ID_SERVICE_CENTER               = 15;
        public static final int ID_PROFILE                      = 16;
        public static final int ID_TERM                         = 17;
        /// M: add for ipmessage readburn @{
        public static final int ID_READEDBURN_DETAIL            = 18;
        public static final int ID_READEDBURN_FINISH            = 19;
        /// @}

        public static final String CHAT_SETTINGS = "content://chat_settings/" + Integer.toString(ID_CHAT_SETTINGS);
        public static final String SYSTEM_SETTINGS = "content://system_settings/" + Integer.toString(ID_SYSTEM_SETTINGS);
        public static final String ACTIVITION = "content://activition/" + Integer.toString(ID_ACTIVITION);
        public static final String ALL_MEDIA = "content://all_media/" + Integer.toString(ID_ALL_MEDIA);
        public static final String CHAT_DETAILS_BY_THREAD_ID = "content://chat_details_by_thread_id/"
                + Integer.toString(ID_CHAT_DETAILS_BY_THREAD_ID);
        public static final String CONTACT = "content://contact_selection/" + Integer.toString(ID_CONTACT);
        public static final String NON_IPMESSAGE_CONTACT = "content://non_ipmessage_contact_selection/"
                + Integer.toString(ID_NON_IPMESSAGE_CONTACT);
        public static final String NEW_GROUP_CHAT = "content://new_group_chat/" + Integer.toString(ID_NEW_GROUP_CHAT);
        public static final String QUICK_CONTACT = "content://quick_contact/" + Integer.toString(ID_QUICK_CONTACT);
        public static final String MEDIA_DETAIL = "content://media_detail/" + Integer.toString(ID_MEDIA_DETAIL);
        public static final String AUDIO = "content://audio/" + Integer.toString(ID_AUDIO);
        public static final String SERVICE_CENTER = "content://service_center/" + Integer.toString(ID_SERVICE_CENTER);
        public static final String PROFILE = "content://profile/" + Integer.toString(ID_PROFILE);
        public static final String TERM = "content://term/" + Integer.toString(ID_TERM);
        /// M: add for ipmessage readburn @{
        public static final String READEDBURN_DETAIL = "content://readedburn_detail/" + Integer.toString(ID_READEDBURN_DETAIL);
        public static final String READEDBURN_FINISH = "content://readedburn_finish/" + Integer.toString(ID_READEDBURN_FINISH);
        /// @}
    }

    public static final class string {
        public static final int ipmsg_ip_message                    = 101;
        public static final int ipmsg_cancel                        = 102;
        public static final int ipmsg_no_such_file                  = 103;
        public static final int ipmsg_over_file_limit               = 104;
        public static final int ipmsg_no_internet                   = 105;
        public static final int ipmsg_load_all_message              = 106;
        public static final int ipmsg_take_photo                    = 107;
        public static final int ipmsg_record_video                  = 108;
        public static final int ipmsg_draw_sketch                   = 109;
        public static final int ipmsg_share_contact                 = 110;
        public static final int ipmsg_choose_photo                  = 111;
        public static final int ipmsg_choose_video                  = 112;
        public static final int ipmsg_record_audio                  = 113;
        public static final int ipmsg_choose_audio                  = 114;
        public static final int ipmsg_share_location                = 115;
        public static final int ipmsg_share_calendar                = 116;
        public static final int ipmsg_delete_important              = 117;
        public static final int ipmsg_chat_setting_updating         = 118;
        public static final int ipmsg_dialog_save_description       = 119;
        public static final int ipmsg_dialog_email_description      = 120;
        public static final int ipmsg_dialog_clear_title            = 121;
        public static final int ipmsg_dialog_clear_description      = 122;
        public static final int ipmsg_chat_setting_saving           = 123;
        public static final int ipmsg_chat_setting_sending          = 124;
        public static final int ipmsg_dialog_save_title             = 125;
        public static final int ipmsg_dialog_email_title            = 126;
        public static final int ipmsg_save_chat_history_failed      = 127;
        public static final int ipmsg_send_chat_history_failed      = 128;
        public static final int ipmsg_conversation_list_all         = 129;
        public static final int ipmsg_conversation_list_important   = 130;
        public static final int ipmsg_conversation_list_group_chats = 131;
        public static final int ipmsg_conversation_list_spam        = 132;
        public static final int ipmsg_mark_as_spam_tips             = 133;
        public static final int ipmsg_typing                        = 134;
        public static final int ipmsg_introduction                  = 135;
        public static final int ipmsg_service_title                 = 136;
        public static final int ipmsg_type_to_compose_text          = 137;
        public static final int ipmsg_ipmsg                         = 138;
        public static final int ipmsg_no_sdcard                     = 139;
        public static final int ipmsg_invite_friends_to_ipmsg_dialog_msg = 140;
        public static final int ipmsg_active                        = 141;
        public static final int ipmsg_welcome_active                = 142;
        public static final int ipmsg_term_warn_welcome             = 143;
        public static final int ipmsg_term_warn_activate            = 144;
        public static final int ipmsg_term_key                      = 145;
        public static final int ipmsg_agree_and_continue            = 146;
        public static final int ipmsg_activate_title                = 147;
        public static final int ipmsg_current_sim_enabled           = 148;
        public static final int ipmsg_sim_selected_dialog_title_for_activate = 149;
        public static final int ipmsg_activate_message              = 150;
        public static final int ipmsg_enable_title                  = 151;
        public static final int ipmsg_enable_message                = 152;
        public static final int ipmsg_sim_selected_dialog_title_for_enable = 153;
        public static final int ipmsg_switch_sim_title              = 154;
        public static final int ipmsg_switch_sim_message            = 155;
        public static final int ipmsg_switch_sim_button             = 156;
        public static final int ipmsg_switch_sim_successfully       = 157;
        public static final int ipmsg_dailog_multi_forward          = 158;
        public static final int ipmsg_dialog_send_result            = 159;
        public static final int ipmsg_dlg_send_all                  = 160;
        public static final int ipmsg_dlg_send_sucess               = 161;
        public static final int ipmsg_dlg_send_failed               = 162;
        public static final int ipmsg_multi_forward_sim_info        = 163;
        public static final int ipmsg_multi_forward_no_sim          = 164;
        public static final int ipmsg_getsim_failed                 = 165;
        public static final int ipmsg_sim_status_error              = 166;
        public static final int ipmsg_forward_failed                = 167;
        public static final int ipmsg_forward_norecipients          = 168;
        public static final int ipmsg_multiforward_no_message       = 169;
        public static final int ipmsg_need_input_recipients         = 170;
        public static final int ipmsg_spam_empty                    = 171;
        public static final int ipmsg_groupchat_empty               = 172;
        public static final int ipmsg_important_empty               = 173;
        public static final int ipmsg_allchat_empty                 = 174;
        public static final int ipmsg_vcard_file_name               = 175;
        public static final int ipmsg_hint                          = 176;
        public static final int ipmsg_caption_hint                  = 177;
        public static final int ipmsg_invite_chat_frequently        = 178;
        public static final int ipmsg_enable_chat_frequently        = 179;
        public static final int ipmsg_invite                        = 180;
        public static final int ipmsg_dismiss                       = 181;
        public static final int ipmsg_enable                        = 182;
        public static final int ipmsg_enable_notice                 = 183;
        public static final int ipmsg_activate                      = 184;
        public static final int ipmsg_activate_chat_frequently      = 185;
        public static final int ipmsg_activate_note                 = 186;
        public static final int ipmsg_dismiss_content               = 187;
        public static final int ipmsg_divider_online                = 188;
        public static final int ipmsg_divider_offline               = 189;
        public static final int ipmsg_divider_never_online          = 190;
        public static final int ipmsg_not_delivered_title           = 191;
        public static final int ipmsg_failed_title                  = 192;
        public static final int ipmsg_try_again                     = 193;
        public static final int ipmsg_try_all_again                 = 194;
        public static final int ipmsg_resend_via_sms                = 195;
        public static final int ipmsg_resend_via_mms                = 196;
        public static final int ipmsg_cant_share                    = 197;
        public static final int ipmsg_file_limit                    = 198;
        public static final int ipmsg_no_app                        = 199;
        public static final int ipmsg_typing_text                   = 200;
        public static final int ipmsg_share_title                   = 201;
        public static final int ipmsg_logo                          = 202;
        public static final int ipmsg_cant_save                     = 203;
        public static final int ipmsg_invalid_file_type             = 204;
        public static final int ipmsg_invite_friends_to_chat        = 205;
        public static final int ipmsg_view_all_media                = 206;
        public static final int ipmsg_view_all_location             = 207;
        public static final int ipmsg_export_to_sdcard              = 208;
        public static final int ipmsg_send_via_text_msg             = 209;
        public static final int ipmsg_send_via_mms                  = 210;
        public static final int ipmsg_retry                         = 211;
        public static final int ipmsg_delete                        = 212;
        public static final int ipmsg_share                         = 213;
        public static final int ipmsg_mark_as_important             = 214;
        public static final int ipmsg_remove_from_important         = 215;
        public static final int ipmsg_save_file                     = 216;
        public static final int ipmsg_convert_to_mms                = 217;
        public static final int ipmsg_convert_to_sms                = 218;
        public static final int ipmsg_convert_to_ipmsg              = 219;
        public static final int ipmsg_convert_to_mms_for_service    = 220;
        public static final int ipmsg_convert_to_sms_for_service    = 221;
        public static final int ipmsg_convert_to_mms_for_recipients = 222;
        public static final int ipmsg_convert_to_sms_for_recipients = 223;
        public static final int ipmsg_sms_convert_to_ipmsg          = 224;
        public static final int ipmsg_mms_convert_to_ipmsg          = 225;
        public static final int ipmsg_keep_mms                      = 226;
        public static final int ipmsg_keep_sms                      = 227;
        public static final int ipmsg_switch                        = 228;
        public static final int ipmsg_replace_attach                = 229;
        public static final int ipmsg_replace_attach_msg            = 230;
        public static final int ipmsg_err_file                      = 231;
        public static final int ipmsg_resend_discard_message        = 232;
        public static final int ipmsg_invite_friends_content        = 233;
        public static final int ipmsg_continue                      = 234;
        public static final int ipmsg_no_sim_card                   = 235;
        public static final int ipmsg_download_history_dlg          = 236;
        public static final int ipmsg_invite_friends_to_ipmsg       = 237;
        public static final int ipmsg_multi_forward_tips_content    = 238;
        public static final int ipmsg_multi_forward_failed_part     = 239;
        public static final int ipmsg_sdcard_space_not_enough       = 240;
        /// M: add for ipmessage readburn @{
        public static final int ipmsg_nms_enable_success = 242;
        public static final int ipmsg_str_nms_menu_license_agreement = 243;
        public static final int ipmsg_str_nms_activation_button = 244;
        public static final int ipmsg_str_nms_activation_button_ok = 245;
        public static final int ipmsg_str_nms_service_center_body = 250;
        public static final int ipmsg_str_nms_service_center_ok = 251;
        public static final int ipmsg_no_ismsuser_message = 252;
        public static final int ipmsg_title_selectphoto = 253;
        public static final int ipmsg_photograph = 254;
        public static final int ipmsg_selectphoto = 255;
        public static final int ipmsg_readed_burn_pic = 256;
        public static final int ipmsg_readed_burn_check = 257;
        public static final int ipmsg_readed_burn_pic_destroy = 258;
        public static final int ipmsg_in_process_of_download = 259;
        public static final int ipmsg_destroy_hint = 260;
        public static final int ipmsg_readed_burn_not_received = 261;
        public static final int ipmsg_readed_burn_download = 262;
        public static final int ipmsg_second = 263;
        public static final int ipmsg_readed_burn = 264;
        public static final int ipmsg_switch_sim = 265;
        /// @}


        /// M: add for ipmessage private message @{
        public static final int ipmsg_private_from_contact          = 266;
        public static final int ipmsg_private_from_diy              = 267;
        public static final int ipmsg_private_no_message            = 268;
        public static final int ipmsg_private_no_contact            = 270;
        public static final int ipmsg_private_add_contact           = 271;
        public static final int ipmsg_private_enter_phonenumber     = 272;
        public static final int ipmsg_private_setting               = 273;
        public static final int ipmsg_private_setting_modify_pwd    = 274;
        public static final int ipmsg_private_setting_enterance     = 275;
        public static final int ipmsg_private_modify_enterance      = 276;
        public static final int ipmsg_private_setting_tips          = 277;
        public static final int ipmsg_private_notification          = 278;
        public static final int ipmsg_private_notification_title    = 279;
        public static final int ipmsg_private_notification_content  = 280;
        public static final int ipmsg_private_notification_title_summary   = 281;
        public static final int ipmsg_private_notification_content_summary = 282;
        public static final int ipmsg_private_notification_setting_content = 283;
        public static final int ipmsg_private_notification_setting_title = 284;
        public static final int ipmsg_private_new_message            = 285;
        public static final int ipmsg_private_contact_del            = 286;
        public static final int ipmsg_nms_not_enough_storage         = 287;
        public static final int ipmsg_nms_store_status_full          = 290;
        public static final int ipmsg_private_msg_add_contact_tips   = 291;
        public static final int ipmsg_private_enter_six_digit        = 292;
        public static final int ipmsg_private_contact_invalid_number = 293;
        public static final int ipmsg_private_long_pressed           = 294;

        public static final int ipmsg_conversation_list_private_msg = 295;
        public static final int ipmsg_conversation_list_private_contact = 296;
        public static final int ipmsg_private_msg_setting_pwd       = 297;
        public static final int ipmsg_private_msg_setting_pwd_tips  = 298;
        public static final int ipmsg_private_msg_confirm_pwd       = 299;
        public static final int ipmsg_private_msg_enter_pwd         = 300;
        public static final int ipmsg_move_to                       = 302;
        public static final int ipmsg_move_from                     = 303;
        public static final int ipmsg_private_msg_list              = 304;
        public static final int ipmsg_private_msg_pwd_cannot_null   = 305;
        public static final int ipmsg_private_msg_pwd_six_digit     = 306;
        public static final int ipmsg_private_msg_pwd_error         = 307;
        public static final int ipmsg_sure                          = 308;
        /// @}

        /// M: add new notification for the current ipmessage user can send ip message @{
        public static final int ipmsg_ip_msg_not_sendable_to_sms = 309;
        public static final int ipmsg_ip_msg_not_sendable_to_mms = 310;
        public static final int ipmsp_delivered_when_get_online = 311;
        /// @}

        /// M : add for ipmsg jira-526
        public static final int ipmsg_nms_forward_download_title1  = 357;
        public static final int ipmsg_nms_forward_download_info1   = 358;
        public static final int ipmsg_nms_forward_download_result  = 359;
        public static final int ipmsg_nms_forward_download_info2   = 360;
        public static final int ipmsg_nms_forward_download_failed  = 361;
        /// @ }
        /// M : add for  kk
        public static final int ipmsg_nms_mms_not_default  = 370;
        /// @ }

        // add for joyn
        public static final int ipmsg_mms_cost_remind               = 371;
        public static final int ipmsg_joyn_cost_remind              = 372;
        public static final int ipmsg_joyn_stranger_remind          = 373;
        public static final int ipmsg_conversation_list_joyn        = 374;
        public static final int ipmsg_conversation_list_xms         = 375;
        public static final int ipmsg_enter_joyn_chat               = 376;
        public static final int ipmsg_enter_xms_chat                = 377;
        public static final int ipmsg_send_by_joyn                  = 378;
        public static final int ipmsg_send_by_xms                   = 379;
        public static final int ipmsg_export_chat                   = 380;
        public static final int mark_as_spam						= 381;
        public static final int ipmsg_copy							= 382;
        public static final int adding_attachments_title            = 383;
        public static final int remove_from_spam                    = 384;
    }

    public static final class array {
        public static final int ipmsg_share_string_array = 101;
        public static final int xms_share_string_array = 102;
    }

    public static final class drawable {
        public static final int emo_praise      = 101;      // LARGE_ICON_ARR
        public static final int emo_gift        = 102;
        public static final int emo_kongfu      = 103;
        public static final int emo_shower      = 104;
        public static final int emo_scare       = 105;
        public static final int emo_ill         = 106;
        public static final int emo_rich        = 107;
        public static final int emo_fly         = 108;
        public static final int emo_angry       = 109;
        public static final int emo_approve     = 110;
        public static final int emo_boring      = 111;
        public static final int emo_cry         = 112;
        public static final int emo_driving     = 113;
        public static final int emo_eating      = 114;
        public static final int emo_happy       = 115;
        public static final int emo_hold        = 116;
        public static final int emo_holiday     = 117;
        public static final int emo_love        = 118;
        public static final int emo_pray        = 119;
        public static final int emo_pressure    = 120;
        public static final int emo_sing        = 121;
        public static final int emo_sleep       = 122;
        public static final int emo_sports      = 123;
        public static final int emo_swimming    = 124;

        public static final int emo_dynamic_01 = 201;       // DYNAMIC_ICON_ARR
        public static final int emo_dynamic_02 = 202;
        public static final int emo_dynamic_03 = 203;
        public static final int emo_dynamic_04 = 204;
        public static final int emo_dynamic_05 = 205;
        public static final int emo_dynamic_06 = 206;
        public static final int emo_dynamic_07 = 207;
        public static final int emo_dynamic_08 = 208;
        public static final int emo_dynamic_09 = 209;
        public static final int emo_dynamic_10 = 210;
        public static final int emo_dynamic_11 = 211;
        public static final int emo_dynamic_12 = 212;
        public static final int emo_dynamic_13 = 213;
        public static final int emo_dynamic_14 = 214;
        public static final int emo_dynamic_15 = 215;
        public static final int emo_dynamic_16 = 216;
        public static final int emo_dynamic_17 = 217;
        public static final int emo_dynamic_18 = 218;
        public static final int emo_dynamic_19 = 219;
        public static final int emo_dynamic_20 = 220;
        public static final int emo_dynamic_21 = 221;
        public static final int emo_dynamic_22 = 222;
        public static final int emo_dynamic_23 = 223;
        public static final int emo_dynamic_24 = 224;

        public static final int emo_dynamic_01_png = 301;   // DYNAMIC_PNG_ICON_ARR
        public static final int emo_dynamic_02_png = 302;
        public static final int emo_dynamic_03_png = 303;
        public static final int emo_dynamic_04_png = 304;
        public static final int emo_dynamic_05_png = 305;
        public static final int emo_dynamic_06_png = 306;
        public static final int emo_dynamic_07_png = 307;
        public static final int emo_dynamic_08_png = 308;
        public static final int emo_dynamic_09_png = 309;
        public static final int emo_dynamic_10_png = 310;
        public static final int emo_dynamic_11_png = 311;
        public static final int emo_dynamic_12_png = 312;
        public static final int emo_dynamic_13_png = 313;
        public static final int emo_dynamic_14_png = 314;
        public static final int emo_dynamic_15_png = 315;
        public static final int emo_dynamic_16_png = 316;
        public static final int emo_dynamic_17_png = 317;
        public static final int emo_dynamic_18_png = 318;
        public static final int emo_dynamic_19_png = 319;
        public static final int emo_dynamic_20_png = 320;
        public static final int emo_dynamic_21_png = 321;
        public static final int emo_dynamic_22_png = 322;
        public static final int emo_dynamic_23_png = 323;
        public static final int emo_dynamic_24_png = 324;

        public static final int ad01 = 401;                 // AD_ICON_ARR
        public static final int ad02 = 402;
        public static final int ad03 = 403;
        public static final int ad04 = 404;
        public static final int ad05 = 405;
        public static final int ad06 = 406;
        public static final int ad07 = 407;
        public static final int ad08 = 408;
        public static final int ad09 = 409;
        public static final int ad10 = 410;
        public static final int ad11 = 411;
        public static final int ad12 = 412;
        public static final int ad13 = 413;
        public static final int ad14 = 414;
        public static final int ad15 = 415;
        public static final int ad16 = 416;
        public static final int ad17 = 417;
        public static final int ad18 = 418;
        public static final int ad19 = 419;
        public static final int ad20 = 420;
        public static final int ad21 = 421;
        public static final int ad22 = 422;
        public static final int ad23 = 423;
        public static final int ad24 = 424;

        public static final int ad01_png = 501;             // AD_PNG_ICON_ARR
        public static final int ad02_png = 502;
        public static final int ad03_png = 503;
        public static final int ad04_png = 504;
        public static final int ad05_png = 505;
        public static final int ad06_png = 506;
        public static final int ad07_png = 507;
        public static final int ad08_png = 508;
        public static final int ad09_png = 509;
        public static final int ad10_png = 510;
        public static final int ad11_png = 511;
        public static final int ad12_png = 512;
        public static final int ad13_png = 513;
        public static final int ad14_png = 514;
        public static final int ad15_png = 515;
        public static final int ad16_png = 516;
        public static final int ad17_png = 517;
        public static final int ad18_png = 518;
        public static final int ad19_png = 519;
        public static final int ad20_png = 520;
        public static final int ad21_png = 521;
        public static final int ad22_png = 522;
        public static final int ad23_png = 523;
        public static final int ad24_png = 524;

        public static final int xm01 = 601;                 // XM_ICON_ARR
        public static final int xm02 = 602;
        public static final int xm03 = 603;
        public static final int xm04 = 604;
        public static final int xm05 = 605;
        public static final int xm06 = 606;
        public static final int xm07 = 607;
        public static final int xm08 = 608;
        public static final int xm09 = 609;
        public static final int xm10 = 610;
        public static final int xm11 = 611;
        public static final int xm12 = 612;
        public static final int xm13 = 613;
        public static final int xm14 = 614;
        public static final int xm15 = 615;
        public static final int xm16 = 616;
        public static final int xm17 = 617;
        public static final int xm18 = 618;
        public static final int xm19 = 619;
        public static final int xm20 = 620;
        public static final int xm21 = 621;
        public static final int xm22 = 622;
        public static final int xm23 = 623;
        public static final int xm24 = 624;

        public static final int xm01_png = 701;             // XM_PNG_ICON_ARR
        public static final int xm02_png = 702;
        public static final int xm03_png = 703;
        public static final int xm04_png = 704;
        public static final int xm05_png = 705;
        public static final int xm06_png = 706;
        public static final int xm07_png = 707;
        public static final int xm08_png = 708;
        public static final int xm09_png = 709;
        public static final int xm10_png = 710;
        public static final int xm11_png = 711;
        public static final int xm12_png = 712;
        public static final int xm13_png = 713;
        public static final int xm14_png = 714;
        public static final int xm15_png = 715;
        public static final int xm16_png = 716;
        public static final int xm17_png = 717;
        public static final int xm18_png = 718;
        public static final int xm19_png = 719;
        public static final int xm20_png = 720;
        public static final int xm21_png = 721;
        public static final int xm22_png = 722;
        public static final int xm23_png = 723;
        public static final int xm24_png = 724;

        /// M: add for ipmessage readburn @{
        public static final int ic_chronograph = 725;
        public static final int ipmsg_share_burn = 726;
        public static final int ipmsg_share_burn_active = 727;
        /// @}

        /// M: add for ipmessage private message @{
        public static final int ipmsg_no_private_contacts = 728;
        public static final int ipmsg_no_private_message = 729;
        public static final int ipmsg_bt_private_add_contact = 730;
        public static final int ipmsg_bt_private_contact = 731;
        public static final int ipmsg_bt_private_setting = 732;
        public static final int ipmsg_private_message_warning = 733;
        public static final int ipmsg_button_blue_press = 734;
        public static final int ipmsg_button_blue_nor = 735;
        /// @}

        public static final int emo_small_01 = 801;
        public static final int emo_small_02 = 802;
        public static final int emo_small_03 = 803;
        public static final int emo_small_04 = 804;
        public static final int emo_small_05 = 805;
        public static final int emo_small_06 = 806;
        public static final int emo_small_07 = 807;
        public static final int emo_small_08 = 808;
        public static final int emo_small_09 = 809;
        public static final int emo_small_10 = 810;
        public static final int emo_small_11 = 811;
        public static final int emo_small_12 = 812;
        public static final int emo_small_13 = 813;
        public static final int emo_small_14 = 814;
        public static final int emo_small_15 = 815;
        public static final int emo_small_16 = 816;
        public static final int emo_small_17 = 817;
        public static final int emo_small_18 = 818;
        public static final int emo_small_19 = 819;
        public static final int emo_small_20 = 820;
        public static final int emo_small_21 = 821;
        public static final int emo_small_22 = 822;
        public static final int emo_small_23 = 823;
        public static final int emo_small_24 = 824;
        public static final int emo_small_25 = 825;
        public static final int emo_small_26 = 826;
        public static final int emo_small_27 = 827;
        public static final int emo_small_28 = 828;
        public static final int emo_small_29 = 829;
        public static final int emo_small_30 = 830;
        public static final int emo_small_31 = 831;
        public static final int emo_small_32 = 832;
        public static final int emo_small_33 = 833;
        public static final int emo_small_34 = 834;
        public static final int emo_small_35 = 835;
        public static final int emo_small_36 = 836;
        public static final int emo_small_37 = 837;
        public static final int emo_small_38 = 838;
        public static final int emo_small_39 = 839;
        public static final int emo_small_40 = 840;
        public static final int emo_good     = 841;
        public static final int emo_no       = 842;
        public static final int emo_ok       = 843;
        public static final int emo_victory  = 844;
        public static final int emo_seduce   = 845;
        public static final int emo_down     = 846;
        public static final int emo_rain     = 847;
        public static final int emo_lightning     = 848;
        public static final int emo_sun      = 849;
        public static final int emo_microphone    = 850;
        public static final int emo_clock     = 851;
        public static final int emo_email     = 852;
        public static final int emo_candle     = 853;
        public static final int emo_birthday_cake = 854;
        public static final int emo_small_gift    = 855;
        public static final int emo_star      = 856;
        public static final int emo_heart     = 857;
        public static final int emo_brokenheart   = 858;
        public static final int emo_bulb      = 859;
        public static final int emo_music     = 860;
        public static final int emo_shenma    = 861;
        public static final int emo_fuyun     = 862;
        public static final int emo_rice      = 863;
        public static final int emo_roses     = 864;
        public static final int emo_film      = 865;
        public static final int emo_aeroplane = 866;
        public static final int emo_umbrella  = 867;
        public static final int emo_caonima   = 868;
        public static final int emo_penguin   = 869;
        public static final int emo_pig       = 870;

        public static final int ipmsg_service = 9001;
        public static final int ipmsg_sim_indicator = 9002;
        /// M: add for ipmessage thumbnail delete
        public static final int ipmsg_delete_caption = 9003;

        // M: add for ipmessage activate
        public static final int ipmsg_emotion_unread_prompt = 10003;
        public static final int ipmsg_chat_button_grey = 10004;
        public static final int ipmsg_chat_button_green = 10005;        

        // add for joyn
        public static final int ipmsg_full_integrated = 11003;
        public static final int enter_joyn_chat       = 11004;
        public static final int ipmsg_file_transfer_pause = 11005;
        public static final int ipmsg_file_transfer_resume = 11006;
        public static final int ipmsg_file_transfer_cancel = 11007;
        
        //add for sharepanel
        public static final int ipmsg_take_photo                    = 11008;
        public static final int ipmsg_record_video                  = 11009;
        public static final int ipmsg_draw_sketch                   = 11010;
        public static final int ipmsg_share_contact                 = 11011;
        public static final int ipmsg_choose_photo                  = 11012;
        public static final int ipmsg_choose_video                  = 11013;
        public static final int ipmsg_record_audio                  = 11014;
        public static final int ipmsg_choose_audio                  = 11015;
        public static final int ipmsg_share_calendar                = 11016;
        public static final int ipmsg_add_slideshow                = 11017;
        public static final int ipmsg_choose_a_file                = 11018;
        public static final int ipmsg_jump_to_joyn                = 11019;
        public static final int ipmsg_jump_to_xms                = 11020;
        public static final int mms_send_msg_background             = 11021;
        //add for emotion
        public static final int ipmsg_emoticon          = 12000;
        public static final int ipmsg_emoticon_default_h          = 12001;
        public static final int ipmsg_emoticon_default_n          = 12002;
        public static final int ipmsg_emoticon_xm_h          = 12003;
        public static final int ipmsg_emoticon_xm_n          = 12004;
        public static final int ipmsg_emoticon_ad_h          = 12005;
        public static final int ipmsg_emoticon_ad_n          = 12006;
        public static final int ipmsg_emoticon_rabbit_h         = 12007;
        public static final int ipmsg_emoticon_rabbit_n          = 12008;
        public static final int ipmsg_emoticon_dragon_h          = 12009;
        public static final int ipmsg_emoticon_dragon_n          = 12010;
    }
    public static final int[] ICON_SELECT = {drawable.ipmsg_emoticon_default_h, drawable.ipmsg_emoticon_default_n,
        drawable.ipmsg_emoticon_xm_h, drawable.ipmsg_emoticon_xm_n, drawable.ipmsg_emoticon_ad_h, drawable.ipmsg_emoticon_ad_n,
        drawable.ipmsg_emoticon_rabbit_h, drawable.ipmsg_emoticon_rabbit_n, drawable.ipmsg_emoticon_dragon_h,
        drawable.ipmsg_emoticon_dragon_n};

    public static final int[] emoticonIdList = { drawable.emo_small_01, drawable.emo_small_02,
        drawable.emo_small_03, drawable.emo_small_04, drawable.emo_small_05,
        drawable.emo_small_06, drawable.emo_small_07, drawable.emo_small_08,
        drawable.emo_small_09, drawable.emo_small_10, drawable.emo_small_11,
        drawable.emo_small_12, drawable.emo_small_13, drawable.emo_small_14,
        drawable.emo_small_15, drawable.emo_small_16, drawable.emo_small_17,
        drawable.emo_small_18, drawable.emo_small_19, drawable.emo_small_20,
        drawable.emo_small_21, drawable.emo_small_22, drawable.emo_small_23,
        drawable.emo_small_24, drawable.emo_small_25, drawable.emo_small_26,
        drawable.emo_small_27, drawable.emo_small_28, drawable.emo_small_29,
        drawable.emo_small_30, drawable.emo_small_31, drawable.emo_small_32,
        drawable.emo_small_33, drawable.emo_small_34, drawable.emo_small_35,
        drawable.emo_small_36, drawable.emo_small_37, drawable.emo_small_38,
        drawable.emo_small_39, drawable.emo_small_40,
        drawable.emo_good, drawable.emo_no, drawable.emo_ok,
        drawable.emo_victory, drawable.emo_seduce, drawable.emo_down,
        drawable.emo_rain, drawable.emo_lightning, drawable.emo_sun,
        drawable.emo_microphone, drawable.emo_clock, drawable.emo_email,
        drawable.emo_candle, drawable.emo_birthday_cake, drawable.emo_small_gift,
        drawable.emo_star, drawable.emo_heart, drawable.emo_brokenheart,
        drawable.emo_bulb, drawable.emo_music, drawable.emo_shenma,
        drawable.emo_fuyun, drawable.emo_rice, drawable.emo_roses,
        drawable.emo_film, drawable.emo_aeroplane, drawable.emo_umbrella,
        drawable.emo_caonima, drawable.emo_penguin, drawable.emo_pig};


    /// M: change the id to the new defined id, we will get these resource by external apk
    public static final int[] LARGE_ICON_ARR = {drawable.emo_praise, drawable.emo_gift,
            drawable.emo_kongfu, drawable.emo_shower, drawable.emo_scare, drawable.emo_ill,
            drawable.emo_rich, drawable.emo_fly, drawable.emo_angry, drawable.emo_approve,
            drawable.emo_boring, drawable.emo_cry, drawable.emo_driving,
            drawable.emo_eating, drawable.emo_happy, drawable.emo_hold,
            drawable.emo_holiday, drawable.emo_love, drawable.emo_pray,
            drawable.emo_pressure, drawable.emo_sing, drawable.emo_sleep,
            drawable.emo_sports, drawable.emo_swimming };
    public static final int[] DYNAMIC_ICON_ARR = {drawable.emo_dynamic_01,
            drawable.emo_dynamic_02, drawable.emo_dynamic_03, drawable.emo_dynamic_04,
            drawable.emo_dynamic_05, drawable.emo_dynamic_06, drawable.emo_dynamic_07,
            drawable.emo_dynamic_08, drawable.emo_dynamic_09, drawable.emo_dynamic_10,
            drawable.emo_dynamic_11, drawable.emo_dynamic_12, drawable.emo_dynamic_13,
            drawable.emo_dynamic_14, drawable.emo_dynamic_15, drawable.emo_dynamic_16,
            drawable.emo_dynamic_17, drawable.emo_dynamic_18, drawable.emo_dynamic_19,
            drawable.emo_dynamic_20, drawable.emo_dynamic_21, drawable.emo_dynamic_22,
            drawable.emo_dynamic_23, drawable.emo_dynamic_24 };
    public static final int[] DYNAMIC_PNG_ICON_ARR = {drawable.emo_dynamic_01_png,
            drawable.emo_dynamic_02_png, drawable.emo_dynamic_03_png,
            drawable.emo_dynamic_04_png, drawable.emo_dynamic_05_png,
            drawable.emo_dynamic_06_png, drawable.emo_dynamic_07_png,
            drawable.emo_dynamic_08_png, drawable.emo_dynamic_09_png,
            drawable.emo_dynamic_10_png, drawable.emo_dynamic_11_png,
            drawable.emo_dynamic_12_png, drawable.emo_dynamic_13_png,
            drawable.emo_dynamic_14_png, drawable.emo_dynamic_15_png,
            drawable.emo_dynamic_16_png, drawable.emo_dynamic_17_png,
            drawable.emo_dynamic_18_png, drawable.emo_dynamic_19_png,
            drawable.emo_dynamic_20_png, drawable.emo_dynamic_21_png,
            drawable.emo_dynamic_22_png, drawable.emo_dynamic_23_png,
            drawable.emo_dynamic_24_png };
    public static final int[] AD_ICON_ARR = {drawable.ad01, drawable.ad02, drawable.ad03,
            drawable.ad04, drawable.ad05, drawable.ad06, drawable.ad07, drawable.ad08,
            drawable.ad09, drawable.ad10, drawable.ad11, drawable.ad12, drawable.ad13,
            drawable.ad14, drawable.ad15, drawable.ad16, drawable.ad17, drawable.ad18,
            drawable.ad19, drawable.ad20, drawable.ad21, drawable.ad22, drawable.ad23,
            drawable.ad24 };
    public static final int[] AD_PNG_ICON_ARR = { drawable.ad01_png, drawable.ad02_png,
            drawable.ad03_png, drawable.ad04_png, drawable.ad05_png, drawable.ad06_png,
            drawable.ad07_png, drawable.ad08_png, drawable.ad09_png, drawable.ad10_png,
            drawable.ad11_png, drawable.ad12_png, drawable.ad13_png, drawable.ad14_png,
            drawable.ad15_png, drawable.ad16_png, drawable.ad17_png, drawable.ad18_png,
            drawable.ad19_png, drawable.ad20_png, drawable.ad21_png, drawable.ad22_png,
            drawable.ad23_png, drawable.ad24_png };
    public static final int[] XM_ICON_ARR = { drawable.xm01, drawable.xm02, drawable.xm03,
            drawable.xm04, drawable.xm05, drawable.xm06, drawable.xm07, drawable.xm08,
            drawable.xm09, drawable.xm10, drawable.xm11, drawable.xm12, drawable.xm13,
            drawable.xm14, drawable.xm15, drawable.xm16, drawable.xm17, drawable.xm18,
            drawable.xm19, drawable.xm20, drawable.xm21, drawable.xm22, drawable.xm23,
            drawable.xm24 };
    public static final int[] XM_PNG_ICON_ARR = {drawable.xm01_png, drawable.xm02_png,
            drawable.xm03_png, drawable.xm04_png, drawable.xm05_png, drawable.xm06_png,
            drawable.xm07_png, drawable.xm08_png, drawable.xm09_png, drawable.xm10_png,
            drawable.xm11_png, drawable.xm12_png, drawable.xm13_png, drawable.xm14_png,
            drawable.xm15_png, drawable.xm16_png, drawable.xm17_png, drawable.xm18_png,
            drawable.xm19_png, drawable.xm20_png, drawable.xm21_png, drawable.xm22_png,
            drawable.xm23_png, drawable.xm24_png };
}
