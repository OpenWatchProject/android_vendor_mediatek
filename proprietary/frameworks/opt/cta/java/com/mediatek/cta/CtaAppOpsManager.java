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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.cta;

import android.app.ActivityThread;
import android.app.ActivityManager;
import android.app.AppOpsManager;
import android.content.Context;
import android.content.pm.PackageManager;
import android.Manifest;
import android.os.UserManager;

import java.util.Arrays;
import java.util.HashMap;


/**
 * API for interacting with "application operation" tracking.
 *
 * <p>This API is not generally intended for third party application developers; most
 * features are only available to system applications.
 */
final class CtaAppOpsManager {

    /// M: CTA requirement - permission control  @{
    /** @hide Control whether an application is allowed to make conference call.*/
    public static final int OP_CTA_CONFERENCE_CALL = AppOpsManager._NUM_OP;
    /** @hide Control whether an application is allowed to call forward.*/
    public static final int OP_CTA_CALL_FORWARD = AppOpsManager._NUM_OP + 1;
    /** @hide Control whether an application is allowed to send MMS.*/
    public static final int OP_CTA_SEND_MMS = AppOpsManager._NUM_OP + 2;
    /** @hide Control whether an application is allowed to send emails.*/
    public static final int OP_CTA_SEND_EMAIL = AppOpsManager._NUM_OP + 3;
    /** @hide */
    public static final int _NUM_OP = AppOpsManager._NUM_OP + 4;

    /// M: CTA requirement - permission control  @{
    /** @hide */
    public static final String OPSTR_CTA_CONFERENCE_CALL
            = "android:cta_conference_call";
    /** @hide */
    public static final String OPSTR_CTA_CALL_FORWARD
            = "android:cta_call_forward";
    /** @hide */
    public static final String OPSTR_CTA_SEND_MMS
            = "android:cta_send_mms";
    /** @hide */
    public static final String OPSTR_CTA_SEND_EMAIL
            = "android:cta_send_email";
    ///@}

    // Warning: If an permission is added here it also has to be added to
    // com.android.packageinstaller.permission.utils.EventLogger
    private static final int[] RUNTIME_AND_APPOP_PERMISSIONS_OPS = {
            // RUNTIME PERMISSIONS
            // Contacts
            AppOpsManager.OP_READ_CONTACTS,
            AppOpsManager.OP_WRITE_CONTACTS,
            AppOpsManager.OP_GET_ACCOUNTS,
            // Calendar
            AppOpsManager.OP_READ_CALENDAR,
            AppOpsManager.OP_WRITE_CALENDAR,
            // SMS
            AppOpsManager.OP_SEND_SMS,
            AppOpsManager.OP_RECEIVE_SMS,
            AppOpsManager.OP_READ_SMS,
            AppOpsManager.OP_RECEIVE_WAP_PUSH,
            AppOpsManager.OP_RECEIVE_MMS,
            AppOpsManager.OP_READ_CELL_BROADCASTS,
            // Storage
            AppOpsManager.OP_READ_EXTERNAL_STORAGE,
            AppOpsManager.OP_WRITE_EXTERNAL_STORAGE,
            AppOpsManager.OP_ACCESS_MEDIA_LOCATION,
            // Location
            AppOpsManager.OP_COARSE_LOCATION,
            AppOpsManager.OP_FINE_LOCATION,
            // Phone
            AppOpsManager.OP_READ_PHONE_STATE,
            AppOpsManager.OP_READ_PHONE_NUMBERS,
            AppOpsManager.OP_CALL_PHONE,
            AppOpsManager.OP_READ_CALL_LOG,
            AppOpsManager.OP_WRITE_CALL_LOG,
            AppOpsManager.OP_ADD_VOICEMAIL,
            AppOpsManager.OP_USE_SIP,
            AppOpsManager.OP_PROCESS_OUTGOING_CALLS,
            AppOpsManager.OP_ANSWER_PHONE_CALLS,
            AppOpsManager.OP_ACCEPT_HANDOVER,
            // Microphone
            AppOpsManager.OP_RECORD_AUDIO,
            // Camera
            AppOpsManager.OP_CAMERA,
            // Body sensors
            AppOpsManager.OP_BODY_SENSORS,
            // Activity recognition
            AppOpsManager.OP_ACTIVITY_RECOGNITION,
            // Aural
            AppOpsManager.OP_READ_MEDIA_AUDIO,
            AppOpsManager.OP_WRITE_MEDIA_AUDIO,
            // Visual
            AppOpsManager.OP_READ_MEDIA_VIDEO,
            AppOpsManager.OP_WRITE_MEDIA_VIDEO,
            AppOpsManager.OP_READ_MEDIA_IMAGES,
            AppOpsManager.OP_WRITE_MEDIA_IMAGES,

            // APPOP PERMISSIONS
            AppOpsManager.OP_ACCESS_NOTIFICATIONS,
            AppOpsManager.OP_SYSTEM_ALERT_WINDOW,
            AppOpsManager.OP_WRITE_SETTINGS,
            AppOpsManager.OP_REQUEST_INSTALL_PACKAGES,
            AppOpsManager.OP_START_FOREGROUND,
            AppOpsManager.OP_SMS_FINANCIAL_TRANSACTIONS,

            /// M: CTA requirement - permission control  @{
            OP_CTA_CONFERENCE_CALL,
            OP_CTA_CALL_FORWARD,
            OP_CTA_SEND_MMS,
            OP_CTA_SEND_EMAIL,
            ///@}
    };

    /**
     * This maps each operation to the operation that serves as the
     * switch to determine whether it is allowed.  Generally this is
     * a 1:1 mapping, but for some things (like location) that have
     * multiple low-level operations being tracked that should be
     * presented to the user as one switch then this can be used to
     * make them all controlled by the same single operation.
     */
    private static int[] sOpToSwitch = new int[] {
            AppOpsManager.OP_COARSE_LOCATION,                 // COARSE_LOCATION
            AppOpsManager.OP_COARSE_LOCATION,                 // FINE_LOCATION
            AppOpsManager.OP_COARSE_LOCATION,                 // GPS
            AppOpsManager.OP_VIBRATE,                         // VIBRATE
            AppOpsManager.OP_READ_CONTACTS,                   // READ_CONTACTS
            AppOpsManager.OP_WRITE_CONTACTS,                  // WRITE_CONTACTS
            AppOpsManager.OP_READ_CALL_LOG,                   // READ_CALL_LOG
            AppOpsManager.OP_WRITE_CALL_LOG,                  // WRITE_CALL_LOG
            AppOpsManager.OP_READ_CALENDAR,                   // READ_CALENDAR
            AppOpsManager.OP_WRITE_CALENDAR,                  // WRITE_CALENDAR
            AppOpsManager.OP_COARSE_LOCATION,                 // WIFI_SCAN
            AppOpsManager.OP_POST_NOTIFICATION,               // POST_NOTIFICATION
            AppOpsManager.OP_COARSE_LOCATION,                 // NEIGHBORING_CELLS
            AppOpsManager.OP_CALL_PHONE,                      // CALL_PHONE
            AppOpsManager.OP_READ_SMS,                        // READ_SMS
            AppOpsManager.OP_WRITE_SMS,                       // WRITE_SMS
            AppOpsManager.OP_RECEIVE_SMS,                     // RECEIVE_SMS
            AppOpsManager.OP_RECEIVE_SMS,                     // RECEIVE_EMERGECY_SMS
            AppOpsManager.OP_RECEIVE_MMS,                     // RECEIVE_MMS
            AppOpsManager.OP_RECEIVE_WAP_PUSH,                // RECEIVE_WAP_PUSH
            AppOpsManager.OP_SEND_SMS,                        // SEND_SMS
            AppOpsManager.OP_READ_SMS,                        // READ_ICC_SMS
            AppOpsManager.OP_WRITE_SMS,                       // WRITE_ICC_SMS
            AppOpsManager.OP_WRITE_SETTINGS,                  // WRITE_SETTINGS
            AppOpsManager.OP_SYSTEM_ALERT_WINDOW,             // SYSTEM_ALERT_WINDOW
            AppOpsManager.OP_ACCESS_NOTIFICATIONS,            // ACCESS_NOTIFICATIONS
            AppOpsManager.OP_CAMERA,                          // CAMERA
            AppOpsManager.OP_RECORD_AUDIO,                    // RECORD_AUDIO
            AppOpsManager.OP_PLAY_AUDIO,                      // PLAY_AUDIO
            AppOpsManager.OP_READ_CLIPBOARD,                  // READ_CLIPBOARD
            AppOpsManager.OP_WRITE_CLIPBOARD,                 // WRITE_CLIPBOARD
            AppOpsManager.OP_TAKE_MEDIA_BUTTONS,              // TAKE_MEDIA_BUTTONS
            AppOpsManager.OP_TAKE_AUDIO_FOCUS,                // TAKE_AUDIO_FOCUS
            AppOpsManager.OP_AUDIO_MASTER_VOLUME,             // AUDIO_MASTER_VOLUME
            AppOpsManager.OP_AUDIO_VOICE_VOLUME,              // AUDIO_VOICE_VOLUME
            AppOpsManager.OP_AUDIO_RING_VOLUME,               // AUDIO_RING_VOLUME
            AppOpsManager.OP_AUDIO_MEDIA_VOLUME,              // AUDIO_MEDIA_VOLUME
            AppOpsManager.OP_AUDIO_ALARM_VOLUME,              // AUDIO_ALARM_VOLUME
            AppOpsManager.OP_AUDIO_NOTIFICATION_VOLUME,       // AUDIO_NOTIFICATION_VOLUME
            AppOpsManager.OP_AUDIO_BLUETOOTH_VOLUME,          // AUDIO_BLUETOOTH_VOLUME
            AppOpsManager.OP_WAKE_LOCK,                       // WAKE_LOCK
            AppOpsManager.OP_COARSE_LOCATION,                 // MONITOR_LOCATION
            AppOpsManager.OP_COARSE_LOCATION,                 // MONITOR_HIGH_POWER_LOCATION
            AppOpsManager.OP_GET_USAGE_STATS,                 // GET_USAGE_STATS
            AppOpsManager.OP_MUTE_MICROPHONE,                 // MUTE_MICROPHONE
            AppOpsManager.OP_TOAST_WINDOW,                    // TOAST_WINDOW
            AppOpsManager.OP_PROJECT_MEDIA,                   // PROJECT_MEDIA
            AppOpsManager.OP_ACTIVATE_VPN,                    // ACTIVATE_VPN
            AppOpsManager.OP_WRITE_WALLPAPER,                 // WRITE_WALLPAPER
            AppOpsManager.OP_ASSIST_STRUCTURE,                // ASSIST_STRUCTURE
            AppOpsManager.OP_ASSIST_SCREENSHOT,               // ASSIST_SCREENSHOT
            AppOpsManager.OP_READ_PHONE_STATE,                // READ_PHONE_STATE
            AppOpsManager.OP_ADD_VOICEMAIL,                   // ADD_VOICEMAIL
            AppOpsManager.OP_USE_SIP,                         // USE_SIP
            AppOpsManager.OP_PROCESS_OUTGOING_CALLS,          // PROCESS_OUTGOING_CALLS
            AppOpsManager.OP_USE_FINGERPRINT,                 // USE_FINGERPRINT
            AppOpsManager.OP_BODY_SENSORS,                    // BODY_SENSORS
            AppOpsManager.OP_READ_CELL_BROADCASTS,            // READ_CELL_BROADCASTS
            AppOpsManager.OP_MOCK_LOCATION,                   // MOCK_LOCATION
            AppOpsManager.OP_READ_EXTERNAL_STORAGE,           // READ_EXTERNAL_STORAGE
            AppOpsManager.OP_WRITE_EXTERNAL_STORAGE,          // WRITE_EXTERNAL_STORAGE
            AppOpsManager.OP_TURN_SCREEN_ON,                  // TURN_SCREEN_ON
            AppOpsManager.OP_GET_ACCOUNTS,                    // GET_ACCOUNTS
            AppOpsManager.OP_RUN_IN_BACKGROUND,               // RUN_IN_BACKGROUND
            AppOpsManager.OP_AUDIO_ACCESSIBILITY_VOLUME,      // AUDIO_ACCESSIBILITY_VOLUME
            AppOpsManager.OP_READ_PHONE_NUMBERS,              // READ_PHONE_NUMBERS
            AppOpsManager.OP_REQUEST_INSTALL_PACKAGES,        // REQUEST_INSTALL_PACKAGES
            AppOpsManager.OP_PICTURE_IN_PICTURE,              // ENTER_PICTURE_IN_PICTURE_ON_HIDE
            AppOpsManager.OP_INSTANT_APP_START_FOREGROUND,    // INSTANT_APP_START_FOREGROUND
            AppOpsManager.OP_ANSWER_PHONE_CALLS,              // ANSWER_PHONE_CALLS
            AppOpsManager.OP_RUN_ANY_IN_BACKGROUND,           // OP_RUN_ANY_IN_BACKGROUND
            AppOpsManager.OP_CHANGE_WIFI_STATE,               // OP_CHANGE_WIFI_STATE
            AppOpsManager.OP_REQUEST_DELETE_PACKAGES,         // OP_REQUEST_DELETE_PACKAGES
            AppOpsManager.OP_BIND_ACCESSIBILITY_SERVICE,      // OP_BIND_ACCESSIBILITY_SERVICE
            AppOpsManager.OP_ACCEPT_HANDOVER,                 // ACCEPT_HANDOVER
            AppOpsManager.OP_MANAGE_IPSEC_TUNNELS,            // MANAGE_IPSEC_HANDOVERS
            AppOpsManager.OP_START_FOREGROUND,                // START_FOREGROUND
            AppOpsManager.OP_COARSE_LOCATION,                 // BLUETOOTH_SCAN
            AppOpsManager.OP_USE_BIOMETRIC,                   // BIOMETRIC
            AppOpsManager.OP_ACTIVITY_RECOGNITION,            // ACTIVITY_RECOGNITION
            AppOpsManager.OP_SMS_FINANCIAL_TRANSACTIONS,      // SMS_FINANCIAL_TRANSACTIONS
            AppOpsManager.OP_READ_MEDIA_AUDIO,                // READ_MEDIA_AUDIO
            AppOpsManager.OP_WRITE_MEDIA_AUDIO,               // WRITE_MEDIA_AUDIO
            AppOpsManager.OP_READ_MEDIA_VIDEO,                // READ_MEDIA_VIDEO
            AppOpsManager.OP_WRITE_MEDIA_VIDEO,               // WRITE_MEDIA_VIDEO
            AppOpsManager.OP_READ_MEDIA_IMAGES,               // READ_MEDIA_IMAGES
            AppOpsManager.OP_WRITE_MEDIA_IMAGES,              // WRITE_MEDIA_IMAGES
            AppOpsManager.OP_LEGACY_STORAGE,                  // LEGACY_STORAGE
            AppOpsManager.OP_ACCESS_ACCESSIBILITY,            // ACCESS_ACCESSIBILITY
            AppOpsManager.OP_READ_DEVICE_IDENTIFIERS,         // READ_DEVICE_IDENTIFIERS
            AppOpsManager.OP_ACCESS_MEDIA_LOCATION,           // ACCESS_MEDIA_LOCATION
            /// M: CTA requirement - permission control  @{
            OP_CTA_CONFERENCE_CALL,
            OP_CTA_CALL_FORWARD,
            OP_CTA_SEND_MMS,
            OP_CTA_SEND_EMAIL
            ///@}
    };

    /**
     * This maps each operation to the public string constant for it.
     */
    private static String[] sOpToString = new String[]{
            AppOpsManager.OPSTR_COARSE_LOCATION,
            AppOpsManager.OPSTR_FINE_LOCATION,
            AppOpsManager.OPSTR_GPS,
            AppOpsManager.OPSTR_VIBRATE,
            AppOpsManager.OPSTR_READ_CONTACTS,
            AppOpsManager.OPSTR_WRITE_CONTACTS,
            AppOpsManager.OPSTR_READ_CALL_LOG,
            AppOpsManager.OPSTR_WRITE_CALL_LOG,
            AppOpsManager.OPSTR_READ_CALENDAR,
            AppOpsManager.OPSTR_WRITE_CALENDAR,
            AppOpsManager.OPSTR_WIFI_SCAN,
            AppOpsManager.OPSTR_POST_NOTIFICATION,
            AppOpsManager.OPSTR_NEIGHBORING_CELLS,
            AppOpsManager.OPSTR_CALL_PHONE,
            AppOpsManager.OPSTR_READ_SMS,
            AppOpsManager.OPSTR_WRITE_SMS,
            AppOpsManager.OPSTR_RECEIVE_SMS,
            AppOpsManager.OPSTR_RECEIVE_EMERGENCY_BROADCAST,
            AppOpsManager.OPSTR_RECEIVE_MMS,
            AppOpsManager.OPSTR_RECEIVE_WAP_PUSH,
            AppOpsManager.OPSTR_SEND_SMS,
            AppOpsManager.OPSTR_READ_ICC_SMS,
            AppOpsManager.OPSTR_WRITE_ICC_SMS,
            AppOpsManager.OPSTR_WRITE_SETTINGS,
            AppOpsManager.OPSTR_SYSTEM_ALERT_WINDOW,
            AppOpsManager.OPSTR_ACCESS_NOTIFICATIONS,
            AppOpsManager.OPSTR_CAMERA,
            AppOpsManager.OPSTR_RECORD_AUDIO,
            AppOpsManager.OPSTR_PLAY_AUDIO,
            AppOpsManager.OPSTR_READ_CLIPBOARD,
            AppOpsManager.OPSTR_WRITE_CLIPBOARD,
            AppOpsManager.OPSTR_TAKE_MEDIA_BUTTONS,
            AppOpsManager.OPSTR_TAKE_AUDIO_FOCUS,
            AppOpsManager.OPSTR_AUDIO_MASTER_VOLUME,
            AppOpsManager.OPSTR_AUDIO_VOICE_VOLUME,
            AppOpsManager.OPSTR_AUDIO_RING_VOLUME,
            AppOpsManager.OPSTR_AUDIO_MEDIA_VOLUME,
            AppOpsManager.OPSTR_AUDIO_ALARM_VOLUME,
            AppOpsManager.OPSTR_AUDIO_NOTIFICATION_VOLUME,
            AppOpsManager.OPSTR_AUDIO_BLUETOOTH_VOLUME,
            AppOpsManager.OPSTR_WAKE_LOCK,
            AppOpsManager.OPSTR_MONITOR_LOCATION,
            AppOpsManager.OPSTR_MONITOR_HIGH_POWER_LOCATION,
            AppOpsManager.OPSTR_GET_USAGE_STATS,
            AppOpsManager.OPSTR_MUTE_MICROPHONE,
            AppOpsManager.OPSTR_TOAST_WINDOW,
            AppOpsManager.OPSTR_PROJECT_MEDIA,
            AppOpsManager.OPSTR_ACTIVATE_VPN,
            AppOpsManager.OPSTR_WRITE_WALLPAPER,
            AppOpsManager.OPSTR_ASSIST_STRUCTURE,
            AppOpsManager.OPSTR_ASSIST_SCREENSHOT,
            AppOpsManager.OPSTR_READ_PHONE_STATE,
            AppOpsManager.OPSTR_ADD_VOICEMAIL,
            AppOpsManager.OPSTR_USE_SIP,
            AppOpsManager.OPSTR_PROCESS_OUTGOING_CALLS,
            AppOpsManager.OPSTR_USE_FINGERPRINT,
            AppOpsManager.OPSTR_BODY_SENSORS,
            AppOpsManager.OPSTR_READ_CELL_BROADCASTS,
            AppOpsManager.OPSTR_MOCK_LOCATION,
            AppOpsManager.OPSTR_READ_EXTERNAL_STORAGE,
            AppOpsManager.OPSTR_WRITE_EXTERNAL_STORAGE,
            AppOpsManager.OPSTR_TURN_SCREEN_ON,
            AppOpsManager.OPSTR_GET_ACCOUNTS,
            AppOpsManager.OPSTR_RUN_IN_BACKGROUND,
            AppOpsManager.OPSTR_AUDIO_ACCESSIBILITY_VOLUME,
            AppOpsManager.OPSTR_READ_PHONE_NUMBERS,
            AppOpsManager.OPSTR_REQUEST_INSTALL_PACKAGES,
            AppOpsManager.OPSTR_PICTURE_IN_PICTURE,
            AppOpsManager.OPSTR_INSTANT_APP_START_FOREGROUND,
            AppOpsManager.OPSTR_ANSWER_PHONE_CALLS,
            AppOpsManager.OPSTR_RUN_ANY_IN_BACKGROUND,
            AppOpsManager.OPSTR_CHANGE_WIFI_STATE,
            AppOpsManager.OPSTR_REQUEST_DELETE_PACKAGES,
            AppOpsManager.OPSTR_BIND_ACCESSIBILITY_SERVICE,
            AppOpsManager.OPSTR_ACCEPT_HANDOVER,
            AppOpsManager.OPSTR_MANAGE_IPSEC_TUNNELS,
            AppOpsManager.OPSTR_START_FOREGROUND,
            AppOpsManager.OPSTR_BLUETOOTH_SCAN,
            AppOpsManager.OPSTR_USE_BIOMETRIC,
            AppOpsManager.OPSTR_ACTIVITY_RECOGNITION,
            AppOpsManager.OPSTR_SMS_FINANCIAL_TRANSACTIONS,
            AppOpsManager.OPSTR_READ_MEDIA_AUDIO,
            AppOpsManager.OPSTR_WRITE_MEDIA_AUDIO,
            AppOpsManager.OPSTR_READ_MEDIA_VIDEO,
            AppOpsManager.OPSTR_WRITE_MEDIA_VIDEO,
            AppOpsManager.OPSTR_READ_MEDIA_IMAGES,
            AppOpsManager.OPSTR_WRITE_MEDIA_IMAGES,
            AppOpsManager.OPSTR_LEGACY_STORAGE,
            AppOpsManager.OPSTR_ACCESS_ACCESSIBILITY,
            AppOpsManager.OPSTR_READ_DEVICE_IDENTIFIERS,
            AppOpsManager.OPSTR_ACCESS_MEDIA_LOCATION,
            /// M: CTA requirement - permission control  @{
            OPSTR_CTA_CONFERENCE_CALL,
            OPSTR_CTA_CALL_FORWARD,
            OPSTR_CTA_SEND_MMS,
            OPSTR_CTA_SEND_EMAIL,
            ///@}
    };

    /**
     * This provides a simple name for each operation to be used
     * in debug output.
     */
    private static String[] sOpNames = new String[] {
            "COARSE_LOCATION",
            "FINE_LOCATION",
            "GPS",
            "VIBRATE",
            "READ_CONTACTS",
            "WRITE_CONTACTS",
            "READ_CALL_LOG",
            "WRITE_CALL_LOG",
            "READ_CALENDAR",
            "WRITE_CALENDAR",
            "WIFI_SCAN",
            "POST_NOTIFICATION",
            "NEIGHBORING_CELLS",
            "CALL_PHONE",
            "READ_SMS",
            "WRITE_SMS",
            "RECEIVE_SMS",
            "RECEIVE_EMERGECY_SMS",
            "RECEIVE_MMS",
            "RECEIVE_WAP_PUSH",
            "SEND_SMS",
            "READ_ICC_SMS",
            "WRITE_ICC_SMS",
            "WRITE_SETTINGS",
            "SYSTEM_ALERT_WINDOW",
            "ACCESS_NOTIFICATIONS",
            "CAMERA",
            "RECORD_AUDIO",
            "PLAY_AUDIO",
            "READ_CLIPBOARD",
            "WRITE_CLIPBOARD",
            "TAKE_MEDIA_BUTTONS",
            "TAKE_AUDIO_FOCUS",
            "AUDIO_MASTER_VOLUME",
            "AUDIO_VOICE_VOLUME",
            "AUDIO_RING_VOLUME",
            "AUDIO_MEDIA_VOLUME",
            "AUDIO_ALARM_VOLUME",
            "AUDIO_NOTIFICATION_VOLUME",
            "AUDIO_BLUETOOTH_VOLUME",
            "WAKE_LOCK",
            "MONITOR_LOCATION",
            "MONITOR_HIGH_POWER_LOCATION",
            "GET_USAGE_STATS",
            "MUTE_MICROPHONE",
            "TOAST_WINDOW",
            "PROJECT_MEDIA",
            "ACTIVATE_VPN",
            "WRITE_WALLPAPER",
            "ASSIST_STRUCTURE",
            "ASSIST_SCREENSHOT",
            "READ_PHONE_STATE",
            "ADD_VOICEMAIL",
            "USE_SIP",
            "PROCESS_OUTGOING_CALLS",
            "USE_FINGERPRINT",
            "BODY_SENSORS",
            "READ_CELL_BROADCASTS",
            "MOCK_LOCATION",
            "READ_EXTERNAL_STORAGE",
            "WRITE_EXTERNAL_STORAGE",
            "TURN_ON_SCREEN",
            "GET_ACCOUNTS",
            "RUN_IN_BACKGROUND",
            "AUDIO_ACCESSIBILITY_VOLUME",
            "READ_PHONE_NUMBERS",
            "REQUEST_INSTALL_PACKAGES",
            "PICTURE_IN_PICTURE",
            "INSTANT_APP_START_FOREGROUND",
            "ANSWER_PHONE_CALLS",
            "RUN_ANY_IN_BACKGROUND",
            "CHANGE_WIFI_STATE",
            "REQUEST_DELETE_PACKAGES",
            "BIND_ACCESSIBILITY_SERVICE",
            "ACCEPT_HANDOVER",
            "MANAGE_IPSEC_TUNNELS",
            "START_FOREGROUND",
            "BLUETOOTH_SCAN",
            "USE_BIOMETRIC",
            "ACTIVITY_RECOGNITION",
            "SMS_FINANCIAL_TRANSACTIONS",
            "READ_MEDIA_AUDIO",
            "WRITE_MEDIA_AUDIO",
            "READ_MEDIA_VIDEO",
            "WRITE_MEDIA_VIDEO",
            "READ_MEDIA_IMAGES",
            "WRITE_MEDIA_IMAGES",
            "LEGACY_STORAGE",
            "ACCESS_ACCESSIBILITY",
            "READ_DEVICE_IDENTIFIERS",
            "ACCESS_MEDIA_LOCATION",
            /// M: CTA requirement - permission control  @{
            "CTA_CONFERENCE_CALL",
            "CTA_CALL_FORWARD",
            "CTA_SEND_MMS",
            "CTA_SEND_EMAIL",
            ///@}
    };

    /**
     * This optionally maps a permission to an operation.  If there
     * is no permission associated with an operation, it is null.
     */
    private static String[] sOpPerms = new String[] {
            android.Manifest.permission.ACCESS_COARSE_LOCATION,
            android.Manifest.permission.ACCESS_FINE_LOCATION,
            null,
            android.Manifest.permission.VIBRATE,
            android.Manifest.permission.READ_CONTACTS,
            android.Manifest.permission.WRITE_CONTACTS,
            android.Manifest.permission.READ_CALL_LOG,
            android.Manifest.permission.WRITE_CALL_LOG,
            android.Manifest.permission.READ_CALENDAR,
            android.Manifest.permission.WRITE_CALENDAR,
            android.Manifest.permission.ACCESS_WIFI_STATE,
            null, // no permission required for notifications
            null, // neighboring cells shares the coarse location perm
            android.Manifest.permission.CALL_PHONE,
            android.Manifest.permission.READ_SMS,
            null, // no permission required for writing sms
            android.Manifest.permission.RECEIVE_SMS,
            android.Manifest.permission.RECEIVE_EMERGENCY_BROADCAST,
            android.Manifest.permission.RECEIVE_MMS,
            android.Manifest.permission.RECEIVE_WAP_PUSH,
            android.Manifest.permission.SEND_SMS,
            android.Manifest.permission.READ_SMS,
            null, // no permission required for writing icc sms
            android.Manifest.permission.WRITE_SETTINGS,
            android.Manifest.permission.SYSTEM_ALERT_WINDOW,
            android.Manifest.permission.ACCESS_NOTIFICATIONS,
            android.Manifest.permission.CAMERA,
            android.Manifest.permission.RECORD_AUDIO,
            null, // no permission for playing audio
            null, // no permission for reading clipboard
            null, // no permission for writing clipboard
            null, // no permission for taking media buttons
            null, // no permission for taking audio focus
            null, // no permission for changing master volume
            null, // no permission for changing voice volume
            null, // no permission for changing ring volume
            null, // no permission for changing media volume
            null, // no permission for changing alarm volume
            null, // no permission for changing notification volume
            null, // no permission for changing bluetooth volume
            android.Manifest.permission.WAKE_LOCK,
            null, // no permission for generic location monitoring
            null, // no permission for high power location monitoring
            android.Manifest.permission.PACKAGE_USAGE_STATS,
            null, // no permission for muting/unmuting microphone
            null, // no permission for displaying toasts
            null, // no permission for projecting media
            null, // no permission for activating vpn
            null, // no permission for supporting wallpaper
            null, // no permission for receiving assist structure
            null, // no permission for receiving assist screenshot
            Manifest.permission.READ_PHONE_STATE,
            Manifest.permission.ADD_VOICEMAIL,
            Manifest.permission.USE_SIP,
            Manifest.permission.PROCESS_OUTGOING_CALLS,
            Manifest.permission.USE_FINGERPRINT,
            Manifest.permission.BODY_SENSORS,
            Manifest.permission.READ_CELL_BROADCASTS,
            null,
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            null, // no permission for turning the screen on
            Manifest.permission.GET_ACCOUNTS,
            null, // no permission for running in background
            null, // no permission for changing accessibility volume
            Manifest.permission.READ_PHONE_NUMBERS,
            Manifest.permission.REQUEST_INSTALL_PACKAGES,
            null, // no permission for entering picture-in-picture on hide
            Manifest.permission.INSTANT_APP_FOREGROUND_SERVICE,
            Manifest.permission.ANSWER_PHONE_CALLS,
            null, // no permission for OP_RUN_ANY_IN_BACKGROUND
            Manifest.permission.CHANGE_WIFI_STATE,
            Manifest.permission.REQUEST_DELETE_PACKAGES,
            Manifest.permission.BIND_ACCESSIBILITY_SERVICE,
            Manifest.permission.ACCEPT_HANDOVER,
            null, // no permission for OP_MANAGE_IPSEC_TUNNELS
            Manifest.permission.FOREGROUND_SERVICE,
            null, // no permission for OP_BLUETOOTH_SCAN
            Manifest.permission.USE_BIOMETRIC,
            Manifest.permission.ACTIVITY_RECOGNITION,
            Manifest.permission.SMS_FINANCIAL_TRANSACTIONS,
            null,
            null, // no permission for OP_WRITE_MEDIA_AUDIO
            null,
            null, // no permission for OP_WRITE_MEDIA_VIDEO
            null,
            null, // no permission for OP_WRITE_MEDIA_IMAGES
            null, // no permission for OP_LEGACY_STORAGE
            null, // no permission for OP_ACCESS_ACCESSIBILITY
            null, // no direct permission for OP_READ_DEVICE_IDENTIFIERS
            Manifest.permission.ACCESS_MEDIA_LOCATION,
            /// M: CTA requirement - permission control @{
            com.mediatek.Manifest.permission.CTA_CONFERENCE_CALL,
            com.mediatek.Manifest.permission.CTA_CALL_FORWARD,
            com.mediatek.Manifest.permission.CTA_SEND_MMS,
            com.mediatek.Manifest.permission.CTA_SEND_EMAIL,
            ///@}
    };

    /**
     * Specifies whether an Op should be restricted by a user restriction.
     * Each Op should be filled with a restriction string from UserManager or
     * null to specify it is not affected by any user restriction.
     */
    private static String[] sOpRestrictions = new String[] {
            UserManager.DISALLOW_SHARE_LOCATION, //COARSE_LOCATION
            UserManager.DISALLOW_SHARE_LOCATION, //FINE_LOCATION
            UserManager.DISALLOW_SHARE_LOCATION, //GPS
            null, //VIBRATE
            null, //READ_CONTACTS
            null, //WRITE_CONTACTS
            UserManager.DISALLOW_OUTGOING_CALLS, //READ_CALL_LOG
            UserManager.DISALLOW_OUTGOING_CALLS, //WRITE_CALL_LOG
            null, //READ_CALENDAR
            null, //WRITE_CALENDAR
            UserManager.DISALLOW_SHARE_LOCATION, //WIFI_SCAN
            null, //POST_NOTIFICATION
            null, //NEIGHBORING_CELLS
            null, //CALL_PHONE
            UserManager.DISALLOW_SMS, //READ_SMS
            UserManager.DISALLOW_SMS, //WRITE_SMS
            UserManager.DISALLOW_SMS, //RECEIVE_SMS
            null, //RECEIVE_EMERGENCY_SMS
            UserManager.DISALLOW_SMS, //RECEIVE_MMS
            null, //RECEIVE_WAP_PUSH
            UserManager.DISALLOW_SMS, //SEND_SMS
            UserManager.DISALLOW_SMS, //READ_ICC_SMS
            UserManager.DISALLOW_SMS, //WRITE_ICC_SMS
            null, //WRITE_SETTINGS
            UserManager.DISALLOW_CREATE_WINDOWS, //SYSTEM_ALERT_WINDOW
            null, //ACCESS_NOTIFICATIONS
            UserManager.DISALLOW_CAMERA, //CAMERA
            UserManager.DISALLOW_RECORD_AUDIO, //RECORD_AUDIO
            null, //PLAY_AUDIO
            null, //READ_CLIPBOARD
            null, //WRITE_CLIPBOARD
            null, //TAKE_MEDIA_BUTTONS
            null, //TAKE_AUDIO_FOCUS
            UserManager.DISALLOW_ADJUST_VOLUME, //AUDIO_MASTER_VOLUME
            UserManager.DISALLOW_ADJUST_VOLUME, //AUDIO_VOICE_VOLUME
            UserManager.DISALLOW_ADJUST_VOLUME, //AUDIO_RING_VOLUME
            UserManager.DISALLOW_ADJUST_VOLUME, //AUDIO_MEDIA_VOLUME
            UserManager.DISALLOW_ADJUST_VOLUME, //AUDIO_ALARM_VOLUME
            UserManager.DISALLOW_ADJUST_VOLUME, //AUDIO_NOTIFICATION_VOLUME
            UserManager.DISALLOW_ADJUST_VOLUME, //AUDIO_BLUETOOTH_VOLUME
            null, //WAKE_LOCK
            UserManager.DISALLOW_SHARE_LOCATION, //MONITOR_LOCATION
            UserManager.DISALLOW_SHARE_LOCATION, //MONITOR_HIGH_POWER_LOCATION
            null, //GET_USAGE_STATS
            UserManager.DISALLOW_UNMUTE_MICROPHONE, // MUTE_MICROPHONE
            UserManager.DISALLOW_CREATE_WINDOWS, // TOAST_WINDOW
            null, //PROJECT_MEDIA
            null, // ACTIVATE_VPN
            UserManager.DISALLOW_WALLPAPER, // WRITE_WALLPAPER
            null, // ASSIST_STRUCTURE
            null, // ASSIST_SCREENSHOT
            null, // READ_PHONE_STATE
            null, // ADD_VOICEMAIL
            null, // USE_SIP
            null, // PROCESS_OUTGOING_CALLS
            null, // USE_FINGERPRINT
            null, // BODY_SENSORS
            null, // READ_CELL_BROADCASTS
            null, // MOCK_LOCATION
            null, // READ_EXTERNAL_STORAGE
            null, // WRITE_EXTERNAL_STORAGE
            null, // TURN_ON_SCREEN
            null, // GET_ACCOUNTS
            null, // RUN_IN_BACKGROUND
            UserManager.DISALLOW_ADJUST_VOLUME, //AUDIO_ACCESSIBILITY_VOLUME
            null, // READ_PHONE_NUMBERS
            null, // REQUEST_INSTALL_PACKAGES
            null, // ENTER_PICTURE_IN_PICTURE_ON_HIDE
            null, // INSTANT_APP_START_FOREGROUND
            null, // ANSWER_PHONE_CALLS
            null, // OP_RUN_ANY_IN_BACKGROUND
            null, // OP_CHANGE_WIFI_STATE
            null, // REQUEST_DELETE_PACKAGES
            null, // OP_BIND_ACCESSIBILITY_SERVICE
            null, // ACCEPT_HANDOVER
            null, // MANAGE_IPSEC_TUNNELS
            null, // START_FOREGROUND
            null, // maybe should be UserManager.DISALLOW_SHARE_LOCATION, //BLUETOOTH_SCAN
            null, // USE_BIOMETRIC
            null, // ACTIVITY_RECOGNITION
            UserManager.DISALLOW_SMS, // SMS_FINANCIAL_TRANSACTIONS
            null, // READ_MEDIA_AUDIO
            null, // WRITE_MEDIA_AUDIO
            null, // READ_MEDIA_VIDEO
            null, // WRITE_MEDIA_VIDEO
            null, // READ_MEDIA_IMAGES
            null, // WRITE_MEDIA_IMAGES
            null, // LEGACY_STORAGE
            null, // ACCESS_ACCESSIBILITY
            null, // READ_DEVICE_IDENTIFIERS
            null, // ACCESS_MEDIA_LOCATION
            /// M: CTA requirement - permission control  @{
            null, // CTA_CONFERENCE_CALL
            null, // CTA_CALL_FORWARD
            null, // CTA_SEND_MMS
            null, // CTA_SEND_EMAIL
            ///@}
    };

    /**
     * This specifies whether each option should allow the system
     * (and system ui) to bypass the user restriction when active.
     */
    private static boolean[] sOpAllowSystemRestrictionBypass = new boolean[] {
            true, //COARSE_LOCATION
            true, //FINE_LOCATION
            false, //GPS
            false, //VIBRATE
            false, //READ_CONTACTS
            false, //WRITE_CONTACTS
            false, //READ_CALL_LOG
            false, //WRITE_CALL_LOG
            false, //READ_CALENDAR
            false, //WRITE_CALENDAR
            true, //WIFI_SCAN
            false, //POST_NOTIFICATION
            false, //NEIGHBORING_CELLS
            false, //CALL_PHONE
            false, //READ_SMS
            false, //WRITE_SMS
            false, //RECEIVE_SMS
            false, //RECEIVE_EMERGECY_SMS
            false, //RECEIVE_MMS
            false, //RECEIVE_WAP_PUSH
            false, //SEND_SMS
            false, //READ_ICC_SMS
            false, //WRITE_ICC_SMS
            false, //WRITE_SETTINGS
            true, //SYSTEM_ALERT_WINDOW
            false, //ACCESS_NOTIFICATIONS
            false, //CAMERA
            false, //RECORD_AUDIO
            false, //PLAY_AUDIO
            false, //READ_CLIPBOARD
            false, //WRITE_CLIPBOARD
            false, //TAKE_MEDIA_BUTTONS
            false, //TAKE_AUDIO_FOCUS
            false, //AUDIO_MASTER_VOLUME
            false, //AUDIO_VOICE_VOLUME
            false, //AUDIO_RING_VOLUME
            false, //AUDIO_MEDIA_VOLUME
            false, //AUDIO_ALARM_VOLUME
            false, //AUDIO_NOTIFICATION_VOLUME
            false, //AUDIO_BLUETOOTH_VOLUME
            false, //WAKE_LOCK
            false, //MONITOR_LOCATION
            false, //MONITOR_HIGH_POWER_LOCATION
            false, //GET_USAGE_STATS
            false, //MUTE_MICROPHONE
            true, //TOAST_WINDOW
            false, //PROJECT_MEDIA
            false, //ACTIVATE_VPN
            false, //WALLPAPER
            false, //ASSIST_STRUCTURE
            false, //ASSIST_SCREENSHOT
            false, //READ_PHONE_STATE
            false, //ADD_VOICEMAIL
            false, // USE_SIP
            false, // PROCESS_OUTGOING_CALLS
            false, // USE_FINGERPRINT
            false, // BODY_SENSORS
            false, // READ_CELL_BROADCASTS
            false, // MOCK_LOCATION
            false, // READ_EXTERNAL_STORAGE
            false, // WRITE_EXTERNAL_STORAGE
            false, // TURN_ON_SCREEN
            false, // GET_ACCOUNTS
            false, // RUN_IN_BACKGROUND
            false, // AUDIO_ACCESSIBILITY_VOLUME
            false, // READ_PHONE_NUMBERS
            false, // REQUEST_INSTALL_PACKAGES
            false, // ENTER_PICTURE_IN_PICTURE_ON_HIDE
            false, // INSTANT_APP_START_FOREGROUND
            false, // ANSWER_PHONE_CALLS
            false, // OP_RUN_ANY_IN_BACKGROUND
            false, // OP_CHANGE_WIFI_STATE
            false, // OP_REQUEST_DELETE_PACKAGES
            false, // OP_BIND_ACCESSIBILITY_SERVICE
            false, // ACCEPT_HANDOVER
            false, // MANAGE_IPSEC_HANDOVERS
            false, // START_FOREGROUND
            true, // BLUETOOTH_SCAN
            false, // USE_BIOMETRIC
            false, // ACTIVITY_RECOGNITION
            false, // SMS_FINANCIAL_TRANSACTIONS
            false, // READ_MEDIA_AUDIO
            false, // WRITE_MEDIA_AUDIO
            false, // READ_MEDIA_VIDEO
            false, // WRITE_MEDIA_VIDEO
            false, // READ_MEDIA_IMAGES
            false, // WRITE_MEDIA_IMAGES
            false, // LEGACY_STORAGE
            false, // ACCESS_ACCESSIBILITY
            false, // READ_DEVICE_IDENTIFIERS
            false, // ACCESS_MEDIA_LOCATION
            /// M: CTA requirement - permission control  @{
            false, // CTA_CONFERENCE_CALL
            false, // CTA_CALL_FORWARD
            false, // CTA_SEND_MMS
            false, // CTA_SEND_EMAIL
            ///@}
    };

    /**
     * This specifies the default mode for each operation.
     */
    private static int[] sOpDefaultMode = new int[] {
            AppOpsManager.MODE_ALLOWED, // COARSE_LOCATION
            AppOpsManager.MODE_ALLOWED, // FINE_LOCATION
            AppOpsManager.MODE_ALLOWED, // GPS
            AppOpsManager.MODE_ALLOWED, // VIBRATE
            AppOpsManager.MODE_ALLOWED, // READ_CONTACTS
            AppOpsManager.MODE_ALLOWED, // WRITE_CONTACTS
            AppOpsManager.MODE_ALLOWED, // READ_CALL_LOG
            AppOpsManager.MODE_ALLOWED, // WRITE_CALL_LOG
            AppOpsManager.MODE_ALLOWED, // READ_CALENDAR
            AppOpsManager.MODE_ALLOWED, // WRITE_CALENDAR
            AppOpsManager.MODE_ALLOWED, // WIFI_SCAN
            AppOpsManager.MODE_ALLOWED, // POST_NOTIFICATION
            AppOpsManager.MODE_ALLOWED, // NEIGHBORING_CELLS
            AppOpsManager.MODE_ALLOWED, // CALL_PHONE
            AppOpsManager.MODE_ALLOWED, // READ_SMS
            AppOpsManager.MODE_IGNORED, // WRITE_SMS
            AppOpsManager.MODE_ALLOWED, // RECEIVE_SMS
            AppOpsManager.MODE_ALLOWED, // RECEIVE_EMERGENCY_BROADCAST
            AppOpsManager.MODE_ALLOWED, // RECEIVE_MMS
            AppOpsManager.MODE_ALLOWED, // RECEIVE_WAP_PUSH
            AppOpsManager.MODE_ALLOWED, // SEND_SMS
            AppOpsManager.MODE_ALLOWED, // READ_ICC_SMS
            AppOpsManager.MODE_ALLOWED, // WRITE_ICC_SMS
            AppOpsManager.MODE_DEFAULT, // WRITE_SETTINGS
            getSystemAlertWindowDefault(), // SYSTEM_ALERT_WINDOW
            AppOpsManager.MODE_ALLOWED, // ACCESS_NOTIFICATIONS
            AppOpsManager.MODE_ALLOWED, // CAMERA
            AppOpsManager.MODE_ALLOWED, // RECORD_AUDIO
            AppOpsManager.MODE_ALLOWED, // PLAY_AUDIO
            AppOpsManager.MODE_ALLOWED, // READ_CLIPBOARD
            AppOpsManager.MODE_ALLOWED, // WRITE_CLIPBOARD
            AppOpsManager.MODE_ALLOWED, // TAKE_MEDIA_BUTTONS
            AppOpsManager.MODE_ALLOWED, // TAKE_AUDIO_FOCUS
            AppOpsManager.MODE_ALLOWED, // AUDIO_MASTER_VOLUME
            AppOpsManager.MODE_ALLOWED, // AUDIO_VOICE_VOLUME
            AppOpsManager.MODE_ALLOWED, // AUDIO_RING_VOLUME
            AppOpsManager.MODE_ALLOWED, // AUDIO_MEDIA_VOLUME
            AppOpsManager.MODE_ALLOWED, // AUDIO_ALARM_VOLUME
            AppOpsManager.MODE_ALLOWED, // AUDIO_NOTIFICATION_VOLUME
            AppOpsManager.MODE_ALLOWED, // AUDIO_BLUETOOTH_VOLUME
            AppOpsManager.MODE_ALLOWED, // WAKE_LOCK
            AppOpsManager.MODE_ALLOWED, // MONITOR_LOCATION
            AppOpsManager.MODE_ALLOWED, // MONITOR_HIGH_POWER_LOCATION
            AppOpsManager.MODE_DEFAULT, // GET_USAGE_STATS
            AppOpsManager.MODE_ALLOWED, // MUTE_MICROPHONE
            AppOpsManager.MODE_ALLOWED, // TOAST_WINDOW
            AppOpsManager.MODE_IGNORED, // PROJECT_MEDIA
            AppOpsManager.MODE_IGNORED, // ACTIVATE_VPN
            AppOpsManager.MODE_ALLOWED, // WRITE_WALLPAPER
            AppOpsManager.MODE_ALLOWED, // ASSIST_STRUCTURE
            AppOpsManager.MODE_ALLOWED, // ASSIST_SCREENSHOT
            AppOpsManager.MODE_ALLOWED, // READ_PHONE_STATE
            AppOpsManager.MODE_ALLOWED, // ADD_VOICEMAIL
            AppOpsManager.MODE_ALLOWED, // USE_SIP
            AppOpsManager.MODE_ALLOWED, // PROCESS_OUTGOING_CALLS
            AppOpsManager.MODE_ALLOWED, // USE_FINGERPRINT
            AppOpsManager.MODE_ALLOWED, // BODY_SENSORS
            AppOpsManager.MODE_ALLOWED, // READ_CELL_BROADCASTS
            AppOpsManager.MODE_ERRORED, // MOCK_LOCATION
            AppOpsManager.MODE_ALLOWED, // READ_EXTERNAL_STORAGE
            AppOpsManager.MODE_ALLOWED, // WRITE_EXTERNAL_STORAGE
            AppOpsManager.MODE_ALLOWED, // TURN_SCREEN_ON
            AppOpsManager.MODE_ALLOWED, // GET_ACCOUNTS
            AppOpsManager.MODE_ALLOWED, // RUN_IN_BACKGROUND
            AppOpsManager.MODE_ALLOWED, // AUDIO_ACCESSIBILITY_VOLUME
            AppOpsManager.MODE_ALLOWED, // READ_PHONE_NUMBERS
            AppOpsManager.MODE_DEFAULT, // REQUEST_INSTALL_PACKAGES
            AppOpsManager.MODE_ALLOWED, // PICTURE_IN_PICTURE
            AppOpsManager.MODE_DEFAULT, // INSTANT_APP_START_FOREGROUND
            AppOpsManager.MODE_ALLOWED, // ANSWER_PHONE_CALLS
            AppOpsManager.MODE_ALLOWED, // RUN_ANY_IN_BACKGROUND
            AppOpsManager.MODE_ALLOWED, // CHANGE_WIFI_STATE
            AppOpsManager.MODE_ALLOWED, // REQUEST_DELETE_PACKAGES
            AppOpsManager.MODE_ALLOWED, // BIND_ACCESSIBILITY_SERVICE
            AppOpsManager.MODE_ALLOWED, // ACCEPT_HANDOVER
            AppOpsManager.MODE_ERRORED, // MANAGE_IPSEC_TUNNELS
            AppOpsManager.MODE_ALLOWED, // START_FOREGROUND
            AppOpsManager.MODE_ALLOWED, // BLUETOOTH_SCAN
            AppOpsManager.MODE_ALLOWED, // USE_BIOMETRIC
            AppOpsManager.MODE_ALLOWED, // ACTIVITY_RECOGNITION
            AppOpsManager.MODE_DEFAULT, // SMS_FINANCIAL_TRANSACTIONS
            AppOpsManager.MODE_ALLOWED, // READ_MEDIA_AUDIO
            AppOpsManager.MODE_ERRORED, // WRITE_MEDIA_AUDIO
            AppOpsManager.MODE_ALLOWED, // READ_MEDIA_VIDEO
            AppOpsManager.MODE_ERRORED, // WRITE_MEDIA_VIDEO
            AppOpsManager.MODE_ALLOWED, // READ_MEDIA_IMAGES
            AppOpsManager.MODE_ERRORED, // WRITE_MEDIA_IMAGES
            AppOpsManager.MODE_DEFAULT, // LEGACY_STORAGE
            AppOpsManager.MODE_ALLOWED, // ACCESS_ACCESSIBILITY
            AppOpsManager.MODE_ERRORED, // READ_DEVICE_IDENTIFIERS
            AppOpsManager.MODE_ALLOWED, // ALLOW_MEDIA_LOCATION
            /// M: CTA requirement - permission control  @{
            AppOpsManager.MODE_ALLOWED,  // OP_CTA_CONFERENCE_CALL
            AppOpsManager.MODE_ALLOWED,  // OP_CTA_CALL_FORWARD
            AppOpsManager.MODE_ALLOWED,  // OP_CTA_SEND_MMS
            AppOpsManager.MODE_ALLOWED,  // OP_CTA_SEND_EMAIL
            ///@}
    };

    /**
     * This specifies whether each option is allowed to be reset
     * when resetting all app preferences.  Disable reset for
     * app ops that are under strong control of some part of the
     * system (such as OP_WRITE_SMS, which should be allowed only
     * for whichever app is selected as the current SMS app).
     */
    private static boolean[] sOpDisableReset = new boolean[] {
            false, // COARSE_LOCATION
            false, // FINE_LOCATION
            false, // GPS
            false, // VIBRATE
            false, // READ_CONTACTS
            false, // WRITE_CONTACTS
            false, // READ_CALL_LOG
            false, // WRITE_CALL_LOG
            false, // READ_CALENDAR
            false, // WRITE_CALENDAR
            false, // WIFI_SCAN
            false, // POST_NOTIFICATION
            false, // NEIGHBORING_CELLS
            false, // CALL_PHONE
            true, // READ_SMS
            true, // WRITE_SMS
            true, // RECEIVE_SMS
            false, // RECEIVE_EMERGENCY_BROADCAST
            false, // RECEIVE_MMS
            true, // RECEIVE_WAP_PUSH
            true, // SEND_SMS
            false, // READ_ICC_SMS
            false, // WRITE_ICC_SMS
            false, // WRITE_SETTINGS
            false, // SYSTEM_ALERT_WINDOW
            false, // ACCESS_NOTIFICATIONS
            false, // CAMERA
            false, // RECORD_AUDIO
            false, // PLAY_AUDIO
            false, // READ_CLIPBOARD
            false, // WRITE_CLIPBOARD
            false, // TAKE_MEDIA_BUTTONS
            false, // TAKE_AUDIO_FOCUS
            false, // AUDIO_MASTER_VOLUME
            false, // AUDIO_VOICE_VOLUME
            false, // AUDIO_RING_VOLUME
            false, // AUDIO_MEDIA_VOLUME
            false, // AUDIO_ALARM_VOLUME
            false, // AUDIO_NOTIFICATION_VOLUME
            false, // AUDIO_BLUETOOTH_VOLUME
            false, // WAKE_LOCK
            false, // MONITOR_LOCATION
            false, // MONITOR_HIGH_POWER_LOCATION
            false, // GET_USAGE_STATS
            false, // MUTE_MICROPHONE
            false, // TOAST_WINDOW
            false, // PROJECT_MEDIA
            false, // ACTIVATE_VPN
            false, // WRITE_WALLPAPER
            false, // ASSIST_STRUCTURE
            false, // ASSIST_SCREENSHOT
            false, // READ_PHONE_STATE
            false, // ADD_VOICEMAIL
            false, // USE_SIP
            false, // PROCESS_OUTGOING_CALLS
            false, // USE_FINGERPRINT
            false, // BODY_SENSORS
            true, // READ_CELL_BROADCASTS
            false, // MOCK_LOCATION
            false, // READ_EXTERNAL_STORAGE
            false, // WRITE_EXTERNAL_STORAGE
            false, // TURN_SCREEN_ON
            false, // GET_ACCOUNTS
            false, // RUN_IN_BACKGROUND
            false, // AUDIO_ACCESSIBILITY_VOLUME
            false, // READ_PHONE_NUMBERS
            false, // REQUEST_INSTALL_PACKAGES
            false, // PICTURE_IN_PICTURE
            false, // INSTANT_APP_START_FOREGROUND
            false, // ANSWER_PHONE_CALLS
            false, // RUN_ANY_IN_BACKGROUND
            false, // CHANGE_WIFI_STATE
            false, // REQUEST_DELETE_PACKAGES
            false, // BIND_ACCESSIBILITY_SERVICE
            false, // ACCEPT_HANDOVER
            false, // MANAGE_IPSEC_TUNNELS
            false, // START_FOREGROUND
            false, // BLUETOOTH_SCAN
            false, // USE_BIOMETRIC
            false, // ACTIVITY_RECOGNITION
            false, // SMS_FINANCIAL_TRANSACTIONS
            false, // READ_MEDIA_AUDIO
            false, // WRITE_MEDIA_AUDIO
            false, // READ_MEDIA_VIDEO
            false, // WRITE_MEDIA_VIDEO
            false, // READ_MEDIA_IMAGES
            false, // WRITE_MEDIA_IMAGES
            false, // LEGACY_STORAGE
            false, // ACCESS_ACCESSIBILITY
            false, // READ_DEVICE_IDENTIFIERS
            false, // ACCESS_MEDIA_LOCATION
            /// M: CTA requirement - permission control  @{
            false, // CTA_CONFERENCE_CALL
            false, // CTA_CALL_FORWARD
            false, // CTA_SEND_MMS
            false, // CTA_SEND_EMAIL
            ///@}
    };

    /**
     * Mapping from an app op name to the app op code.
     */
    private static HashMap<String, Integer> sOpStrToOp = new HashMap<>();

    /**
     * Mapping from a permission to the corresponding app op.
     */
    private static HashMap<String, Integer> sPermToOp = new HashMap<>();

    static {
        if (sOpToSwitch.length != _NUM_OP) {
            throw new IllegalStateException("sOpToSwitch length " + sOpToSwitch.length
                    + " should be " + _NUM_OP);
        }
        if (sOpToString.length != _NUM_OP) {
            throw new IllegalStateException("sOpToString length " + sOpToString.length
                    + " should be " + _NUM_OP);
        }
        if (sOpNames.length != _NUM_OP) {
            throw new IllegalStateException("sOpNames length " + sOpNames.length
                    + " should be " + _NUM_OP);
        }
        if (sOpPerms.length != _NUM_OP) {
            throw new IllegalStateException("sOpPerms length " + sOpPerms.length
                    + " should be " + _NUM_OP);
        }
        if (sOpDefaultMode.length != _NUM_OP) {
            throw new IllegalStateException("sOpDefaultMode length " + sOpDefaultMode.length
                    + " should be " + _NUM_OP);
        }
        if (sOpDisableReset.length != _NUM_OP) {
            throw new IllegalStateException("sOpDisableReset length " + sOpDisableReset.length
                    + " should be " + _NUM_OP);
        }
        if (sOpRestrictions.length != _NUM_OP) {
            throw new IllegalStateException("sOpRestrictions length " + sOpRestrictions.length
                    + " should be " + _NUM_OP);
        }
        if (sOpAllowSystemRestrictionBypass.length != _NUM_OP) {
            throw new IllegalStateException("sOpAllowSYstemRestrictionsBypass length "
                    + sOpRestrictions.length + " should be " + _NUM_OP);
        }
        for (int i=0; i<_NUM_OP; i++) {
            if (sOpToString[i] != null) {
                sOpStrToOp.put(sOpToString[i], i);
            }
        }
        for (int op : RUNTIME_AND_APPOP_PERMISSIONS_OPS) {
            if (sOpPerms[op] != null) {
                sPermToOp.put(sOpPerms[op], op);
            }
        }
    }

    /**
     * Retrieve the op switch that controls the given operation.
     * @hide
     */
    public static int opToSwitch(int op) {
        return sOpToSwitch[op];
    }

    /**
     * Retrieve a non-localized name for the operation, for debugging output.
     * @hide
     */
    public static String opToName(int op) {
        if (op == AppOpsManager.OP_NONE) return "NONE";
        return op < sOpNames.length ? sOpNames[op] : ("Unknown(" + op + ")");
    }

    /**
     * Retrieve a non-localized public name for the operation.
     *
     * @hide
     */
    public static String opToPublicName(int op) {
        return sOpToString[op];
    }

    /**
     * @hide
     */
    public static int strDebugOpToOp(String op) {
        for (int i=0; i<sOpNames.length; i++) {
            if (sOpNames[i].equals(op)) {
                return i;
            }
        }
        throw new IllegalArgumentException("Unknown operation string: " + op);
    }

    /**
     * Retrieve the permission associated with an operation, or null if there is not one.
     * @hide
     */
    public static String opToPermission(int op) {
        return sOpPerms[op];
    }

    /**
     * Retrieve the permission associated with an operation, or null if there is not one.
     *
     * @param op The operation name.
     *
     * @hide
     */
    public static String opToPermission(String op) {
        return opToPermission(strOpToOp(op));
    }

    /**
     * Retrieve the user restriction associated with an operation, or null if there is not one.
     * @hide
     */
    public static String opToRestriction(int op) {
        return sOpRestrictions[op];
    }

    /**
     * Retrieve the app op code for a permission, or null if there is not one.
     * This API is intended to be used for mapping runtime or appop permissions
     * to the corresponding app op.
     * @hide
     */
    public static int permissionToOpCode(String permission) {
        Integer boxedOpCode = sPermToOp.get(permission);
        return boxedOpCode != null ? boxedOpCode : AppOpsManager.OP_NONE;
    }

    /**
     * Retrieve whether the op allows the system (and system ui) to
     * bypass the user restriction.
     * @hide
     */
    public static boolean opAllowSystemBypassRestriction(int op) {
        return sOpAllowSystemRestrictionBypass[op];
    }

    /**
     * Retrieve the default mode for the operation.
     * @hide
     */
    public static int opToDefaultMode(int op) {
        return sOpDefaultMode[op];
    }

    /**
     * Retrieve whether the op allows itself to be reset.
     * @hide
     */
    public static boolean opAllowsReset(int op) {
        return !sOpDisableReset[op];
    }

    /**
     * Gets the app op name associated with a given permission.
     * The app op name is one of the public constants defined
     * in this class such as {@link #OPSTR_COARSE_LOCATION}.
     * This API is intended to be used for mapping runtime
     * permissions to the corresponding app op.
     *
     * @param permission The permission.
     * @return The app op associated with the permission or null.
     */
    public static String permissionToOp(String permission) {
        final Integer opCode = sPermToOp.get(permission);
        if (opCode == null) {
            return null;
        }
        return sOpToString[opCode];
    }

    /**
     * {@hide}
     */
    public static int strOpToOp(String op) {
        Integer val = sOpStrToOp.get(op);
        if (val == null) {
            throw new IllegalArgumentException("Unknown operation string: " + op);
        }
        return val;
    }

    /**
     * Returns all supported operation names.
     * @hide
     */
    public static String[] getOpStrs() {
        return Arrays.copyOf(sOpToString, sOpToString.length);
    }


    /**
     * @return number of App ops
     * @hide
     */
    public static int getNumOps() {
        return _NUM_OP;
    }

    private static int getSystemAlertWindowDefault() {
        final Context context = ActivityThread.currentApplication();
        if (context == null) {
            return AppOpsManager.MODE_DEFAULT;
        }

        // system alert window is disable on low ram phones starting from Q
        final PackageManager pm = context.getPackageManager();
        // TVs are constantly plugged in and has less concern for memory/power
        if (ActivityManager.isLowRamDeviceStatic()
                && pm != null && !pm.hasSystemFeature(PackageManager.FEATURE_LEANBACK, 0)) {
            return AppOpsManager.MODE_IGNORED;
        }

        return AppOpsManager.MODE_DEFAULT;
    }

}
