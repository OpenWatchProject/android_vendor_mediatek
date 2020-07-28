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

package com.mediatek.atci.service;

import android.app.Service;
import android.app.Instrumentation;
import android.app.NotificationManager;
import android.app.NotificationChannel;
import android.view.KeyEvent;

import android.content.ComponentName;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;

import android.database.Cursor;
import android.media.AudioManager;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.net.NetworkCapabilities;
import android.net.Network;
import android.net.Uri;

import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.HwBinder;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.Message;
import android.os.PowerManager;
import android.os.SystemClock;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.provider.Telephony;
import android.support.v4.app.NotificationCompat;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.ITelephony;
import com.android.internal.util.ArrayUtils;

import com.mediatek.atci.utils.AtCommandHandler;
import com.mediatek.atci.utils.AtCommandResult;
import com.mediatek.atci.utils.AtParser;

import vendor.mediatek.hardware.atci.V1_0.IAtcid;
import vendor.mediatek.hardware.atci.V1_0.IAtcidCommandHandler;
import vendor.mediatek.hardware.netdagent.V1_0.INetdagent;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.NoSuchElementException;


/**
 *
 */
public class AtciService extends Service {
    static final String LOG_TAG = "ATCIJ";
    static final String SOCKET_NAME_ATCI = "atci-serv-fw";
    private static final String EMERGENCY_CALL_ACTION = "android.location.agps.EMERGENCY_CALL";
    private static final boolean DBG = true;
    private static final String ATCI_USERMODE = "persist.vendor.service.atci.usermode";
    private static final String TITLE = "ATCI Service";
    private static final int MSG_ID_CONNECT_TO_ATCID = 1;

    private static final char[] HEX_EX_STRING = new char[] {0x02, 0x03};

    static final int SOCKET_OPEN_RETRY_MILLIS = 4 * 1000;
    static final int ATCI_MAX_BUFFER_BYTES = 1024;

    LocalSocket mSocket;
    Thread mReceiverThread;
    AtciReceiver mReceiver;
    AtParser mAtParser;
    Context mContext;
    private WakeLock mWakeLock;  // held while processing an AT command
    private WakeLock mFullWakeLock;
    private IAtcid mAtcid = null;
    private AtcidCommandHandler mAtcidCommandHandler = null;
    private AtcidDeathRecipient mAtcidDeathRecipient = null;

    private AudioManager mAudioManager = null;

    static final int NUM_ZERO = 0;
    static final int NUM_ONE = 1;
    static final int NUM_TWO = 2;
    static final int NUM_THREE = 3;
    static final int NUM_FOUR = 4;
    static final int NUM_FIVE = 5;
    static final int NUM_SIX = 6;
    static final int NUM_SEVEN = 7;
    static final int NUM_TEN = 10;
    static final int NUM_ELEVEN = 11;
    static final int NUM_TWELVE = 12;
    static final int NUM_SIXTEEN = 16;

    static final int THREAD_SLEEP_ONE = 2000;
    static final int THREAD_SLEEP_TWO = 4000;

    public static final int VZW_INTERNET = 0;
    public static final int VZW_IMS      = 1;
    public static final int VZW_APP      = 2;
    public static final int VZW_FOTA     = 3;
    public static final int VZW_SUPPORT_APN_NUM = 4;
    private NetworkCallback mNetworkCallback[] = new NetworkCallback[VZW_SUPPORT_APN_NUM];
    private NetworkRequest mNetworkRequest[] = new NetworkRequest[VZW_SUPPORT_APN_NUM];
    private Integer mVzwApnStatus[] = new Integer[VZW_SUPPORT_APN_NUM];
    private static final int CONN_MGR_TIMEOUT = 50 * 1000;
    private static final int APN_CLASS_0 = 0;
    private static final int APN_CLASS_1 = 1;
    private static final int APN_CLASS_2 = 2;
    private static final int APN_CLASS_3 = 3;
    private static final int APN_CLASS_4 = 4;
    private static final int APN_CLASS_5 = 5;
    private static final int APN_CLASS_X = 6;
    private static final int APN_CLASS_NUM = 7;
    private static final String VZW_EMERGENCY_NI = "VZWEMERGENCY";
    private static final String VZW_IMS_NI = "IMS";
    private static final String VZW_ADMIN_NI = "VZWADMIN";
    private static final String VZW_INTERNET_NI = "VZWINTERNET";
    private static final String VZW_APP_NI = "VZWAPP";
    private static final String VZW_800_NI = "VZW800";
    private static final String VZW_DUN_NI = "dungsm.vzw3g.com";
    private static final String KEY_APN_NAME = "apn_name";

    //must be same toMtkSettingsExt.Global.MSIM_MODE_SETTING
    private static final String MSIM_MODE_SETTING = "msim_mode_setting";

    private static final String ACTION_IMS_COMMAND =
            "mediatek.intent.action.ATCTION_ATCI_IMS_COMMAND";
    private static final String NEXT_OP = "NEXT_OPERATION";

    /* (non-Javadoc)
     * @see android.app.Service#onBind(android.content.Intent)
     */
    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
        super.onCreate();

        mAtcidCommandHandler = new AtcidCommandHandler();
        mAtcidDeathRecipient = new AtcidDeathRecipient();
        mHandler.sendMessage(mHandler.obtainMessage(MSG_ID_CONNECT_TO_ATCID));

        mContext = getApplicationContext();
        PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "AtciService");
        mWakeLock.setReferenceCounted(false);
        mFullWakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK |
                        PowerManager.ACQUIRE_CAUSES_WAKEUP, "AtciService");

        //Construct one At command parser
        mAtParser = new AtParser();

        startAtciImsService();

        initializeAtParser();
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            if (MSG_ID_CONNECT_TO_ATCID == msg.what) {
                try {
                    mAtcid = IAtcid.getService("default", true);
                    if (null != mAtcid) {
                        Log.d(LOG_TAG, "init");
                        mAtcid.setCommandHandler(mAtcidCommandHandler);
                        mAtcid.linkToDeath(mAtcidDeathRecipient, 0);
                    } else {
                        Log.e(LOG_TAG, "atcid is null");
                    }
                } catch (RemoteException e) {
                    Log.e(LOG_TAG, "RemoteException");
                } catch (NoSuchElementException e) {
                    Log.e(LOG_TAG, "NoSuchElementException");
                }
            }
        }
    };

    private void startAtciImsService() {
        Log.d(LOG_TAG, "startAtciImsService");
        new Thread(new Runnable() {
            @Override
            public void run() {
                //Intent AtciIntent = new Intent("com.mediatek.START_IMS_ATCI");
                //AtciIntent.setPackage("com.mediatek.atci.service");
                //mContext.startService(AtciIntent);
                mContext.startService(new Intent(mContext, AtciImsService.class));
            }
        }).start();
    }


    private String formatResponse(String res) {
        return String.format("%c%s%c", HEX_EX_STRING[0], res, HEX_EX_STRING[1]);
    }

    protected void initializeAtTelephony() {

        //AT% Commands Name : FLIGHT Description : Enter FLIGHT Mode of Phone
        //Commands Syntax :  AT%FLIGHT / AT% FLIGHT? / AT% FLIGHT =? / AT% FLIGHT =
        mAtParser.register("%FLIGHT", new AtCommandHandler() {
            private AtCommandResult isAirplaneModeOn() {
                int mode = Settings.System.getInt(
                               mContext.getContentResolver(),
                               Settings.System.AIRPLANE_MODE_ON,
                               0);
                Log.d(LOG_TAG, "Get airplane mode:" + mode);
                String response = formatResponse("" + mode);
                return new AtCommandResult(response);
            }

            @Override
            public AtCommandResult handleActionCommand() {
                return isAirplaneModeOn();
            }

            @Override
            public AtCommandResult handleReadCommand() {
                return isAirplaneModeOn();
            }

            @Override
            public AtCommandResult handleTestCommand() {
                return isAirplaneModeOn();
            }

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {

                Log.d(LOG_TAG, "args:" + args.length);
                Log.d(LOG_TAG, "args string:" + args[0]);

                if (args.length == 1) {
                    char c = 0;
                    boolean enabling;
                    String response = null;

                    if (args[0] instanceof Integer) {
                        c = ((Integer) args[0]).toString().charAt(0);
                    }
                    Log.d(LOG_TAG, "c is " + c);

                    if (c == '0' || c == '1') {
                        Log.d(LOG_TAG, "Set airplane mode:" + c);
                        enabling = ((c == '0') ? false : true);

                        if (enabling) {
                            SystemProperties.set("vendor.ril.atci.flightmode", "1");
                        }

                        // Change the system setting
                        Settings.Global.putInt(
                                       mContext.getContentResolver(),
                                       Settings.Global.AIRPLANE_MODE_ON,
                                       enabling ? 1 : 0);

                        // Post the intent
                        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
                        intent.putExtra("state", enabling);
                        mContext.sendBroadcastAsUser(intent, UserHandle.ALL);

                        if (c == '0') {
                            response = formatResponse("[0] FLIGHT Mode OFF");
                        } else {
                            response = formatResponse("[1] FLIGHT Mode ON");
                        }
                        return new AtCommandResult(response);
                    }
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
            });

        mAtParser.register("%SUPPLEMENTARYSERVICE", new AtCommandHandler() {
            private static final String ACTION_SUPPLEMENTARY_SERVICE_UT_TEST
                        = "android.intent.action.ACTION_SUPPLEMENTARY_SERVICE_UT_TEST";
            private static final String PACKAGE_NAME = "com.mediatek.internal.telephony";
            private static final String EXTRA_ACTION = "action";
            private static final String EXTRA_SERVICE_CODE = "serviceCode";
            private static final String EXTRA_SERVICE_INFO_A = "serviceInfoA";
            private static final String EXTRA_SERVICE_INFO_B = "serviceInfoB";
            private static final String EXTRA_SERVICE_INFO_C = "serviceInfoC";
            private static final String EXTRA_SERVICE_INFO_D = "serviceInfoD";
            private static final String EXTRA_PHONE_ID = "phoneId";
            private static final String SUPPLEMENTARY_SERVICE_PERMISSION =
                    "com.mediatek.permission.SUPPLEMENTARY_SERVICE_UT_TEST";

            private String removeDoubleQuotationMarks(String string) {
                Log.d(LOG_TAG, "old string" + string);
                int length = string.length();

                if ((length > 1) && (string.charAt(0) == '"')
                        && (string.charAt(length - 1) == '"')) {
                    return string.substring(1, length - 1);
                }
                return string;
            }

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                Log.d(LOG_TAG, "[%SUPPLEMENTARYSERVICE]");
                String response = "OK\r\n";
                if (args.length == 7) {
                    int action = -1;
                    String serviceCode = null;
                    String serviceInfoA = null;
                    String serviceInfoB = null;
                    String serviceInfoC = null;
                    String serviceInfoD = null;
                    int phoneId = -1;

                    if (args[0] instanceof Integer) {
                        action = ((Integer) args[0]).intValue();
                    }
                    if (args[1] instanceof String) {
                        serviceCode = removeDoubleQuotationMarks((String) args[1]);
                    }
                    if (args[2] instanceof String) {
                        serviceInfoA = removeDoubleQuotationMarks((String) args[2]);
                    }
                    if (args[3] instanceof String) {
                        serviceInfoB = removeDoubleQuotationMarks((String) args[3]);
                    }
                    if (args[4] instanceof String) {
                        serviceInfoC = removeDoubleQuotationMarks((String) args[4]);
                    }
                    if (args[5] instanceof String) {
                        serviceInfoD = removeDoubleQuotationMarks((String) args[5]);
                    }
                    if (args[6] instanceof Integer) {
                        phoneId = ((Integer) args[6]).intValue();
                    }
                    Log.d(LOG_TAG, "[%SUPPLEMENTARYSERVICE], action = " + action +
                            ", serviceCode = " + serviceCode +
                            ", serviceInfoA = " + serviceInfoA +
                            ", serviceInfoB = " + serviceInfoB +
                            ", serviceInfoC = " + serviceInfoC +
                            ", serviceInfoD = " + serviceInfoD +
                            ", phoneId = " + phoneId);

                    Intent intent = new Intent(ACTION_SUPPLEMENTARY_SERVICE_UT_TEST);
                    //intent.setPackage(PACKAGE_NAME);
                    intent.putExtra(EXTRA_ACTION, action);
                    intent.putExtra(EXTRA_SERVICE_CODE, serviceCode);
                    intent.putExtra(EXTRA_SERVICE_INFO_A, serviceInfoA);
                    intent.putExtra(EXTRA_SERVICE_INFO_B, serviceInfoB);
                    intent.putExtra(EXTRA_SERVICE_INFO_C, serviceInfoC);
                    intent.putExtra(EXTRA_SERVICE_INFO_D, serviceInfoD);
                    intent.putExtra(EXTRA_PHONE_ID, phoneId);
                    mContext.sendBroadcast(intent, SUPPLEMENTARY_SERVICE_PERMISSION);

                    return new AtCommandResult(response);
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });

        mAtParser.register('D', new AtCommandHandler() {
            private static final int AUDIO_CALL = 0x0;
            private static final int VIDEO_CALL = 0x3;

            private static final String SCHEME_TEL = "tel";
            private static final String EXTRA_START_CALL_WITH_VIDEO_STATE =
                    "android.telecom.extra.START_CALL_WITH_VIDEO_STATE";

            private String removeSemicolon(String string) {
                int length = string.length();
                if ((length > 1) && (string.charAt(length - 1) == ';')) {
                    return string.substring(0, length - 1);
                }
                return string;
            }

            /**
             * Return an Intent for making a phone call. Scheme (e.g. tel, sip) will be determined
             * automatically.
             */
            private Intent getCallIntent(int type, String number) {
                Uri uri = Uri.fromParts(SCHEME_TEL, number, null);
                Log.d(LOG_TAG, "[ATD] uri = " + uri);
                Intent intent = new Intent(Intent.ACTION_CALL_PRIVILEGED, uri);
                intent.putExtra(EXTRA_START_CALL_WITH_VIDEO_STATE, type);

                return intent;
            }

            @Override
            public AtCommandResult handleBasicCommand(String arg) {
                Log.d(LOG_TAG, "[ATD] arg = " + arg);
                if (arg != null) {
                    int callType = arg.contains(";") ? AUDIO_CALL : VIDEO_CALL;
                    String number = removeSemicolon(arg);
                    Log.d(LOG_TAG, "[ATD] callType = " + callType + " number = " + number);
                    Intent intent = getCallIntent(callType, number);
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mContext.startActivity(intent);

                    return new AtCommandResult("OK\r\n");
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });

        mAtParser.register("%DIAL", new AtCommandHandler() {
            // private static final int AUDIO_ONLY = 0x0;
            private static final String SCHEME_TEL = "tel";
            private static final String SCHEME_SIP = "sip";

            private static final String EXTRA_START_CALL_WITH_VIDEO_STATE =
                    "android.telecom.extra.START_CALL_WITH_VIDEO_STATE";

            private String removeDoubleQuotationMarks(String string) {
                int length = string.length();
                if ((length > 1) && (string.charAt(0) == '"')
                        && (string.charAt(length - 1) == '"')) {
                    return string.substring(1, length - 1);
                }
                return string;
            }

            /**
             * Determines if the specified number is actually a URI (i.e. a SIP address) rather than a
             * regular PSTN phone number, based on whether or not the number contains an "@" character.
             *
             * @param number Phone number
             * @return true if number contains @
             *
             */
            private boolean isUriNumber(String number) {
                return number != null && (number.contains("@") || number.contains("%40"));
            }

            /**
             * Return Uri with an appropriate scheme, accepting both SIP and usual phone call
             * numbers.
             */
            private Uri getCallUri(String number) {
                // if (isUriNumber(number)) {
                    // return Uri.fromParts(SCHEME_SIP, number, null);
                // }
                return Uri.fromParts(SCHEME_TEL, number, null);
             }

            /**
             * Return an Intent for making a phone call. Scheme (e.g. tel, sip) will be determined
             * automatically.
             */
            private Intent getCallIntent(int type, String number) {
                Uri uri = getCallUri(number);
                Log.d(LOG_TAG, "[AT%DIAL] uri = " + uri);
                Intent intent = new Intent(Intent.ACTION_CALL_PRIVILEGED, uri);
                intent.putExtra(EXTRA_START_CALL_WITH_VIDEO_STATE, type);

                return intent;
            }

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                Log.d(LOG_TAG, "[AT%DIAL]");
                if (args.length <= 2) {
                    String number = null;
                    int type = 0;
                    if (args.length == 1) {
                        if (args[0] instanceof String) {
                            number = removeDoubleQuotationMarks((String) args[0]);
                        }
                    } else if (args.length == 2) {
                        if (args[0] instanceof Integer) {
                            type = ((Integer) args[0]).intValue();
                        }
                        if (args[1] instanceof String) {
                            number = removeDoubleQuotationMarks((String) args[1]);
                        }
                    }
                    Log.d(LOG_TAG, "[AT%DIAL] type = " + type + " number = " + number);
                    Intent intent = getCallIntent(type, number);
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mContext.startActivity(intent);

                    return new AtCommandResult("OK\r\n");
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });

        mAtParser.register("%VILTEUPGRADE", new AtCommandHandler() {

            private static final String VILTE_UPGRADE = "vilte_upgrade";
            private static final String VILTE_DOWNGRADE = "vilte_downgrade";
            private static final String VILTE_ACCEPT_UPGRADE = "vilte_accept_upgrade";
            private static final String VILTE_REJECT_UPGRADE = "vilte_reject_upgrade";
            private static final String VILTE_PAUSE_VIDEO = "vilte_pause_video";
            private static final String VILTE_RESUME_VIDEO = "vilte_resume_video";

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                Log.d(LOG_TAG, "[AT%VILTEUPGRADE] args string:" + args[0]);
                if (args.length == 1) {
                    char c = 0;
                    if (args[0] instanceof Integer) {
                        c = ((Integer) args[0]).toString().charAt(0);
                    }

                    Intent intent = new Intent(ACTION_IMS_COMMAND);
                    intent.setPackage(mContext.getPackageName());

                    switch (c) {
                    case '0':
                        Log.d(LOG_TAG, "[AT%VILTEUPGRADE] Downgrade");
                        intent.putExtra(NEXT_OP, VILTE_DOWNGRADE);
                        break;
                    case '1':
                        Log.d(LOG_TAG, "[AT%VILTEUPGRADE] Upgrade");
                        intent.putExtra(NEXT_OP, VILTE_UPGRADE);
                        break;
                    case '2':
                        Log.d(LOG_TAG, "[AT%VILTEUPGRADE] Reject upgrade");
                        intent.putExtra(NEXT_OP, VILTE_REJECT_UPGRADE);
                        break;
                    case '3':
                        Log.d(LOG_TAG, "[AT%VILTEUPGRADE] Accept upgrade");
                        intent.putExtra(NEXT_OP, VILTE_ACCEPT_UPGRADE);
                        break;
                    case '4':
                        Log.d(LOG_TAG, "[AT%VILTEUPGRADE] Pause video");
                        intent.putExtra(NEXT_OP, VILTE_PAUSE_VIDEO);
                        break;
                    case '5':
                        Log.d(LOG_TAG, "[AT%VILTEUPGRADE] Restart video");
                        intent.putExtra(NEXT_OP, VILTE_RESUME_VIDEO);
                        break;
                    default:
                        return new AtCommandResult("PARAMETER ERROR\r\n");
                    }

                    mContext.sendBroadcast(intent);

                    return new AtCommandResult("OK\r\n");
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });

        mAtParser.register("%CONFERENCE", new AtCommandHandler() {
            private static final String SCHEME_TEL = "tel";
            private static final String CONF_SERVER_ADDRESS =
                    "mmtel@conf-factory.3gpp.org";

            public static final String EXTRA_START_VOLTE_CONFERENCE =
                    "mediatek.telecom.extra.EXTRA_START_VOLTE_CONFERENCE";
            public static final String EXTRA_VOLTE_CONFERENCE_NUMBERS =
                    "mediatek.telecom.extra.VOLTE_CONFERENCE_NUMBERS";
            public static final String EXTRA_START_CALL_WITH_VIDEO_STATE =
                    "android.telecom.extra.START_CALL_WITH_VIDEO_STATE";
            private static final int VIDEO_CALL = 0x3;

            private static final String MERGE_CONFERENCE = "merge_conference";

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                Log.d(LOG_TAG, "[AT%CONFERENCE] args string:" + args[0]);
                if (args.length == 1) {
                    char c = 0;
                    if (args[0] instanceof Integer) {
                        c = ((Integer) args[0]).toString().charAt(0);
                    }
                    switch (c) {
                    case '0': {
                        Log.d(LOG_TAG, "[AT%CONFERENCE] Merge conference");
                        Intent intent = new Intent(ACTION_IMS_COMMAND);
                        intent.putExtra(NEXT_OP, MERGE_CONFERENCE);
                        intent.setPackage(mContext.getPackageName());
                        mContext.sendBroadcast(intent);
                        break;
                    }
                    case '1':
                    case '2': {
                        ArrayList<String> members = new ArrayList<String>();
                        Uri uri = Uri.fromParts(SCHEME_TEL, CONF_SERVER_ADDRESS, null);
                        Intent intent = new Intent(Intent.ACTION_CALL_PRIVILEGED, uri);
                        intent.putExtra(EXTRA_START_VOLTE_CONFERENCE, true);
                        intent.putStringArrayListExtra(EXTRA_VOLTE_CONFERENCE_NUMBERS, members);
                        if (c == '1') {
                            Log.d(LOG_TAG, "[AT%CONFERENCE] One-key voice conference");
                        } else {
                            Log.d(LOG_TAG, "[AT%CONFERENCE] One-key video conference");
                            intent.putExtra(EXTRA_START_CALL_WITH_VIDEO_STATE, VIDEO_CALL);
                        }
                        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        TelecomManager tm =
                                (TelecomManager) mContext.getSystemService(Context.TELECOM_SERVICE);
                        tm.placeCall(intent.getData(), intent.getExtras());
                        Log.d(LOG_TAG, "[AT%CONFERENCE] placeCall()");
                        break;
                    }
                    default:
                        return new AtCommandResult("PARAMETER ERROR\r\n");
                    }

                    return new AtCommandResult("OK\r\n");
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });

        mAtParser.register("%MUTE", new AtCommandHandler() {
            private static final String ACTION_MUTE =
                    "mediatek.intent.action.ACTION_MUTE";
            private static final String ACTION_UNMUTE =
                    "mediatek.intent.action.ACTION_UNMUTE";

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                Log.d(LOG_TAG, "[AT%MUTE] args string:" + args[0]);
                if (args.length == 1) {
                    char c = 0;
                    if (args[0] instanceof Integer) {
                        c = ((Integer) args[0]).toString().charAt(0);
                    }

                    if (mAudioManager == null) {
                        mAudioManager = (AudioManager) getSystemService(mContext.AUDIO_SERVICE);
                    }

                    switch (c) {
                    case '0':
                        Log.d(LOG_TAG, "[AT%MUTE] Unmute");

                        if (mAudioManager != null) {
                            mAudioManager.setMicrophoneMute(false);
                        } else {
                            Log.d(LOG_TAG, "[AT%MUTE] mAudioManager is null");
                        }

                        break;
                    case '1':
                        Log.d(LOG_TAG, "[AT%MUTE] Mute");

                        if (mAudioManager != null) {
                            mAudioManager.setMicrophoneMute(true);
                        } else {
                            Log.d(LOG_TAG, "[AT%MUTE] mAudioManager is null");
                        }

                        break;
                    default:
                        return new AtCommandResult("PARAMETER ERROR\r\n");
                    }

                    return new AtCommandResult("OK\r\n");
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });

        mAtParser.register("%DATASIM", new AtCommandHandler() {
            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                if (args.length == 1 && args[0] instanceof Integer) {
                    Log.d(LOG_TAG, "slotId: " + args[0]);

                    int slotId = (Integer) args[0];
                    int[] subIds = SubscriptionManager.getSubId(slotId);
                    int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
                    if (subIds != null && subIds.length >= 1) {
                        subId = subIds[0];
                    }

                    //int subId = MtkSubscriptionManager.getSubIdUsingPhoneId((Integer) args[0]);
                    if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                        SubscriptionManager.from(mContext).setDefaultDataSubId(subId);
                        return new AtCommandResult(AtCommandResult.OK);
                    }
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });

        mAtParser.register("%CALLSIM", new AtCommandHandler() {
            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                if (args.length == 1 && args[0] instanceof Integer) {
                    Log.d(LOG_TAG, "slotId: " + args[0]);

                    int slotId = (Integer) args[0];
                    int[] subIds = SubscriptionManager.getSubId(slotId);
                    int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
                    if (subIds != null && subIds.length >= 1) {
                        subId = subIds[0];
                    }

                    Log.d(LOG_TAG, "subId: " + subId);
                    if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                        TelecomManager.from(mContext).setUserSelectedOutgoingPhoneAccount(
                                subscriptionIdToPhoneAccountHandle(subId));
                        return new AtCommandResult(AtCommandResult.OK);
                    }
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }

            private PhoneAccountHandle subscriptionIdToPhoneAccountHandle(final int subId) {
                Iterator<PhoneAccountHandle> phoneAccounts =
                        TelecomManager.from(mContext).getCallCapablePhoneAccounts().listIterator();

                Log.d(LOG_TAG, "Match phone account, subId=" + subId
                        + ", phone account list exist=" + phoneAccounts.hasNext());
                while (phoneAccounts.hasNext()) {
                    final PhoneAccountHandle phoneAccountHandle = phoneAccounts.next();
                    final PhoneAccount phoneAccount = TelecomManager.from(mContext)
                            .getPhoneAccount(phoneAccountHandle);
                    final int phoneAccountSubId = TelephonyManager.from(mContext)
                            .getSubIdForPhoneAccount(phoneAccount);
                    Log.d(LOG_TAG, "Match phone account, phoneAccountSubId=" + phoneAccountSubId
                            + ", phoneAccount=" + phoneAccount);
                    if (subId == phoneAccountSubId) {
                        return phoneAccountHandle;
                    }
                }
                return null;
            }
        });

        mAtParser.register("%ENABLESIM", new AtCommandHandler() {
            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                Log.d(LOG_TAG, "args: " + args.length);

                if (args.length == 2 && args[0] instanceof Integer && args[1] instanceof Integer) {
                    Log.d(LOG_TAG, "args string: " + args[0] + "," + args[1]);
                    int slotId = (Integer) args[0];
                    boolean enable = ((Integer) args[1] == 1);

                    ITelephony iTel = ITelephony.Stub.asInterface(
                            ServiceManager.getService(Context.TELEPHONY_SERVICE));

                    int[] subIds = SubscriptionManager.getSubId(slotId);
                    int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
                    if (subIds != null && subIds.length >= 1) {
                        subId = subIds[0];
                    }

                    if (iTel != null && subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                        try {
                            if (iTel.setRadioForSubscriber(subId, enable)) {
                                int mode = Settings.Global.getInt(mContext.getContentResolver(),
                                        MSIM_MODE_SETTING, 0);
                                Settings.Global.putInt(mContext.getContentResolver(),
                                        MSIM_MODE_SETTING, enable ?
                                        (mode | (1 << slotId)) : (mode & ~(1 << slotId)));
                                return new AtCommandResult(AtCommandResult.OK);
                            }
                        } catch (RemoteException e) {
                            Log.e(LOG_TAG, "RemoteException");
                            e.printStackTrace();
                        }
                    }
                }

                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });
    }

    protected void initializeAtSystemCall() {

        mAtParser.register("%DATAON", new AtCommandHandler() {
            private TelephonyManager mTeleMgr = (TelephonyManager) getSystemService(
                Context.TELEPHONY_SERVICE);

            private void setDataEnable(boolean isEnable) {
                try {
                    INetdagent agent = INetdagent.getService();
                    if (null != agent) {
                        if (isEnable) {
                            Log.d(LOG_TAG, "clearIotFirewall");
                            agent.dispatchNetdagentCmd("netdagent firewall clear_nsiot_firewall");
                        } else {
                            Log.d(LOG_TAG, "setIotFirewall");
                            agent.dispatchNetdagentCmd("netdagent firewall set_nsiot_firewall");
                        }
                    } else {
                        Log.d(LOG_TAG, "netdagent is null");
                    }
                } catch (RemoteException e) {
                    Log.e(LOG_TAG, "Iot firewall exception");
                }

            }
            private void setMobileDataState(boolean isEnable) {
                if (mTeleMgr != null) {
                    mTeleMgr.setDataEnabled(isEnable);
                }
            }
            private boolean isMobileDataEnabled() {
                if (mTeleMgr != null) {
                    return mTeleMgr.getDataEnabled();
                } else {
                    Log.d(LOG_TAG, "[+DATACONNECT] mTeleMgr is null");
                    return false;
                }
            }
            @Override
            public AtCommandResult handleActionCommand() {
                String response = "OK\r\n";
                if (!isMobileDataEnabled()) {
                    setMobileDataState(true);
                }
                try {
                    Thread.sleep(THREAD_SLEEP_TWO);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                if (!isMobileDataEnabled()) {
                    response = "FAILED\r\n";
                }
                setDataEnable(false);

                return new AtCommandResult(response);
            }
        });

        mAtParser.register("+DATACONNECT", new AtCommandHandler() {
            private TelephonyManager mTeleMgr = (TelephonyManager)
                getSystemService(Context.TELEPHONY_SERVICE);

            private void setMobileDataState(boolean isEnable) {
                if (mTeleMgr != null) {
                    mTeleMgr.setDataEnabled(isEnable);
                }
            }

            private boolean isMobileDataEnabled() {
                if (mTeleMgr != null) {
                    return mTeleMgr.getDataEnabled();
                } else {
                    Log.d(LOG_TAG, "[+DATACONNECT] mTeleMgr is null");
                    return false;
                }
            }

            @Override
            public AtCommandResult handleReadCommand() {
                return new AtCommandResult(formatResponse(
                    isMobileDataEnabled() ? "1" : "0"));
            }

            @Override
            public AtCommandResult handleTestCommand() {
                return new AtCommandResult("(0-1)");
            }

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                Log.d(LOG_TAG, "[+DATACONNECT] args string:" + args[0]);
                String response = "OK\r\n";
                if (args.length == 1) {
                    char c = 0;
                    if (args[0] instanceof Integer) {
                        c = ((Integer) args[0]).toString().charAt(0);
                    }
                    Log.d(LOG_TAG, "[+DATACONNECT] c is " + c);

                    switch (c) {
                    case '0':
                        if (isMobileDataEnabled()) {
                            setMobileDataState(false);
                        }
                        try {
                            Thread.sleep(THREAD_SLEEP_TWO);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        if (isMobileDataEnabled()) {
                            response = "FAILED\r\n";
                        }
                        break;
                    case '1':
                        if (!isMobileDataEnabled()) {
                            setMobileDataState(true);
                        }
                        try {
                            Thread.sleep(THREAD_SLEEP_TWO);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        if (!isMobileDataEnabled()) {
                            response = "FAILED\r\n";
                        }
                        break;
                    default:
                        response = "PARAMETER ERROR\r\n";
                    }

                    return new AtCommandResult(response);
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });

        mAtParser.register("%ENABLEDATA", new AtCommandHandler() {
            private void setDataEnable(boolean isEnable) {
                try {
                    INetdagent agent = INetdagent.getService();
                    if (agent != null) {
                        if (isEnable) {
                            Log.d(LOG_TAG, "clearIotFirewall");
                            agent.dispatchNetdagentCmd("netdagent firewall clear_nsiot_firewall");
                        } else {
                            Log.d(LOG_TAG, "setIotFirewall");
                            agent.dispatchNetdagentCmd("netdagent firewall set_nsiot_firewall");
                        }
                    } else {
                        Log.d(LOG_TAG, "netdagent is null");
                    }
                } catch (RemoteException e) {
                    Log.e(LOG_TAG, "Iot firewall exception");
                }
            }

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                Log.d(LOG_TAG, "[%ENABLEDATA] args string:" + args[0]);
                String response = "OK\r\n";
                if (args.length == 1) {
                    char c = 0;
                    if (args[0] instanceof Integer) {
                        c = ((Integer) args[0]).toString().charAt(0);
                    }
                    Log.d(LOG_TAG, "[%ENABLEDATA] c is " + c);

                    switch (c) {
                    case '0':
                        setDataEnable(false);
                        break;
                    case '1':
                        setDataEnable(true);
                        break;
                    default:
                        response = "PARAMETER ERROR\r\n";
                    }

                    return new AtCommandResult(response);
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });

    mAtParser.register("%DATAROAMING", new AtCommandHandler() {

        private TelephonyManager mTeleMgr =
                (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);

        private AtCommandResult isDataOnRoamingEnabled() {
            int enabled = Settings.System.getInt(
                           mContext.getContentResolver(),
                           Settings.Global.DATA_ROAMING,
                           0);
            Log.d(LOG_TAG, "Data on Roaming enabled:" + enabled);
            String response = formatResponse("" + enabled);
            return new AtCommandResult(response);
        }

        @Override
        public AtCommandResult handleActionCommand() {
            return isDataOnRoamingEnabled();
        }

        @Override
        public AtCommandResult handleReadCommand() {
            return isDataOnRoamingEnabled();
        }

        @Override
        public AtCommandResult handleTestCommand() {
            return new AtCommandResult("(0-1)");
        }

        @Override
        public AtCommandResult handleSetCommand(Object[] args) {

            Log.d(LOG_TAG, "[DATAROAMING] args string:" + args[0]);

            if (args.length == 1) {
                char c = 0;
                boolean enabling;
                String response = "OK\r\n";

                if (args[0] instanceof Integer) {
                    c = ((Integer) args[0]).toString().charAt(0);
                }
                Log.d(LOG_TAG, "c is " + c);

                if (c == '0' || c == '1') {
                    Log.d(LOG_TAG, "Set data on roaming: " + c);
                    enabling = ((c == '0') ? false : true);

                    int phoneSubId = SubscriptionManager.getDefaultSubscriptionId();
                    Log.d(LOG_TAG, "phoneSubId: " + phoneSubId);

                    if (mTeleMgr.getSimCount() == 1) {
                        Settings.Global.putInt(mContext.getContentResolver(),
                                Settings.Global.DATA_ROAMING, enabling ? 1 : 0);
                    } else {
                        Settings.Global.putInt(mContext.getContentResolver(),
                                Settings.Global.DATA_ROAMING + phoneSubId, enabling ? 1 : 0);
                    }

                    return new AtCommandResult(response);
                }
            }
            return new AtCommandResult(AtCommandResult.ERROR);
        }
    });

    mAtParser.register("%BROWSER", new AtCommandHandler() {

        @Override
        public AtCommandResult handleActionCommand() {

            Log.d(LOG_TAG, "[BROWSER]");

            String response = "OK\r\n";
            Intent intent = new Intent();
            intent.setAction("android.intent.action.VIEW");
            intent.setClassName("com.android.browser", "com.android.browser.BrowserActivity");
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mContext.startActivity(intent);
            return new AtCommandResult(response);
        }
    });

    }

    /*
     *    AT+CKPD
     *    Keypad control .
     *    Commands Syntax :  AT+CKPD =? / AT+CKPD =
     */
    protected void initializeAtCKPD() {
        if (DBG) {
            Log.d(LOG_TAG, "initializeAtCKPD");
        }
        mAtParser.register("+CKPD", new AtCommandHandler() {

            /*   Range  Commands :
             *   User types => AT+CKPD =?
             *   Return String => Same as "Action"
             */
            @Override
            public AtCommandResult handleTestCommand() {
                return new AtCommandResult(AtCommandResult.OK);
            }


            private int getArgumentInt(String args)
            {
                Log.d(LOG_TAG, "args length = " + args.toString().length());
                int result = 0, index = 0;

                index = args.toString().length();

                for (int len = 0; len < index; len++)
                {
                    if ((args.charAt(len) >= '0') && (args.charAt(len) <= '9'))
                    {
                        result = result * 10 + (args.charAt(len) - '0');
                        Log.d(LOG_TAG, "result = " + result);
                    }
                }

                Log.d(LOG_TAG, "final result = " + result);
                return result;
            }
            /*   Assign  Commands :
             *   User types    =>  AT+CKPD ="XXX"
             *   Return String => OK
             */
            @Override
            public AtCommandResult handleSetCommand(Object[] args) {

                char command = 0;
                int time = 0, pause = 0, index = 0;

                if (args.length >= 2) {
                    time = getArgumentInt(args[1].toString());
                }

                if (args.length == 3) {
                    pause = getArgumentInt(args[2].toString());
                }

                if (args.length >= 1 && args[0].toString().length() > 0) {
                    char c = 0;
                    String oldargument = args[0].toString().toLowerCase();
                    String argument = "";
                    Log.d(LOG_TAG, "ar length = " + oldargument.length());

                    for (int len = 0; len < oldargument.length(); len++) {
                        if ((oldargument.charAt(len) >= ' ') &&
                             (oldargument.charAt(len) <= '~'))
                             argument += oldargument.charAt(len);
                    }

                    if ((argument.startsWith("\"")) && (argument.endsWith("\""))) {
                        argument = argument.substring(1, argument.length() - 1);
                    }

                    // VzW requirement. The argument is a single ASCII representing one character.
                    if ("OP12".equals(SystemProperties.get("persist.vendor.operator.optr", "OM"))) {
                        try {
                            int i = Integer.parseInt(argument);
                            Log.d(LOG_TAG, "interger argument = " + i);
                            if (i >= 0 && i < 128) {
                                argument = String.valueOf((char) i).toLowerCase();
                            } else {
                                argument = "";
                            }
                        } catch (NumberFormatException e) {
                            Log.e(LOG_TAG, "invalid argument = " + argument);
                            argument = "";
                        }
                    }

                    index = argument.length();
                    do {
                        index--;
                    } while (argument.charAt(index) == 0x0a);

                    c = argument.charAt(index);
                    Log.d(LOG_TAG, "ar length = " + argument.length());
                    Log.d(LOG_TAG, "ar = " + argument);
                    Log.d(LOG_TAG, "c is " + c);

                    switch(c) {
                    case 's': {
                            command = c;
                            argument = argument.substring(0, argument.length() - 1);
                            Intent intent = new Intent(Intent.ACTION_DIAL);
                            intent.setData(Uri.parse("tel:"));
                            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                            ComponentName componentName =
                                    intent.resolveActivity(getPackageManager());
                            intent.setComponent(componentName);
                            startActivity(intent);
                            Log.d(LOG_TAG, "argument=" + argument + ", command=" + command);
                        }
                        break;
                    default:
                        break;
                    }

                    SystemClock.sleep(100);

                    Instrumentation inst = new Instrumentation();
                    for (index = 0; index < argument.length(); index++) {
                        c = argument.charAt(index);
                        Log.d(LOG_TAG, "inst c=" + c);

                        if (c >= '0' && c <= '9') {
                            inst.sendKeyDownUpSync(KeyEvent.KEYCODE_0 + (c - '0'));
                        } else if (c == '#') {
                            inst.sendKeyDownUpSync(KeyEvent.KEYCODE_POUND);
                        } else if (c == '+') {
                            inst.sendKeyDownUpSync(KeyEvent.KEYCODE_PLUS);
                        } else if (c == '*') {
                            inst.sendKeyDownUpSync(KeyEvent.KEYCODE_STAR);
                        } else if (c == '<') {
                            inst.sendKeyDownUpSync(KeyEvent.KEYCODE_SOFT_LEFT);
                        } else if (c == '>') {
                            inst.sendKeyDownUpSync(KeyEvent.KEYCODE_SOFT_RIGHT);
                        } else if (c == '@') {
                            inst.sendKeyDownUpSync(KeyEvent.KEYCODE_AT);
                        } else if (c == '^') {
                            inst.sendKeyDownUpSync(KeyEvent.KEYCODE_VOLUME_UP);
                        } else if (c == '[') {
                            inst.sendKeyDownUpSync(KeyEvent.KEYCODE_SOFT_LEFT);
                        } else if (c == ']') {
                            inst.sendKeyDownUpSync(KeyEvent.KEYCODE_SOFT_RIGHT);
                        } else if (c >= 'a' && c <= 'z') {
                            if (command == 's') {
                                inst.sendKeyDownUpSync(KeyEvent.KEYCODE_A + (c - 'a'));
                            } else {
                                    if (c == 'c') {
                                        inst.sendKeyDownUpSync(KeyEvent.KEYCODE_CLEAR);
                                    } else if (c == 'd') {
                                        inst.sendKeyDownUpSync(KeyEvent.KEYCODE_VOLUME_DOWN);
                                    } else if (c == 'e') {
                                        inst.sendKeyDownUpSync(KeyEvent.KEYCODE_ENDCALL);
                                    } else if (c == 'm') {
                                        inst.sendKeyDownUpSync(KeyEvent.KEYCODE_MENU);
                                    } else if (c == 'p') {
                                        inst.sendKeyDownUpSync(KeyEvent.KEYCODE_POWER);
                                    } else if (c == 'q') {
                                        inst.sendKeyDownUpSync(KeyEvent.KEYCODE_VOLUME_MUTE);
                                    } else if (c == 'r') {
                                        inst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
                                  /*} else if (c == 's') {
                                        inst.sendKeyDownUpSync(KeyEvent.ACTION_DIAL);*/
                                    } else if (c == 'u') {
                                        inst.sendKeyDownUpSync(KeyEvent.KEYCODE_VOLUME_UP);
                                    } else if (c == 'v') {
                                        inst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
                                    } else if (c == 'w') {
                                        inst.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_PAUSE);
                                    } else if (c == 'y') {
                                        inst.sendKeyDownUpSync(KeyEvent.KEYCODE_DEL);
                                    }
                            } //a~z && !s
                        }     //a~z

                        if (pause > 0) {
                            Log.d(LOG_TAG, "pause = " + pause);
                        }
                    }

                    switch (command) {
                        case 's': {
                                Log.d(LOG_TAG, "command =" + command);
                                //inst.sendKeyDownUpSync(KeyEvent.KEYCODE_CALL);
                                Intent intent = new Intent(Intent.ACTION_CALL_PRIVILEGED,
                                Uri.fromParts("tel", argument, null));
                                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                                startActivity(intent);
                            }
                            break;
                        default:
                            break;
                    }
                }
                return new AtCommandResult("");
            }
        });
    }

    protected void initializeAtParser() {
        Log.i(LOG_TAG, "initializeAtParser");

        initializeAtTelephony();

        initializeAtCKPD();
        initializeAtSystemCall();
        initializeAtVzw();
        initializeAtMmo();
    }

    /* Process an incoming AT command line
     */
    protected void handleInput(String input) {
        acquireWakeLock();
        long timestamp;

        if (DBG) {
            timestamp = System.currentTimeMillis();
        }

        AtCommandResult result = mAtParser.process(input);
        if (DBG) {
            Log.d(LOG_TAG, "Processing " + input + " took " +
                   (System.currentTimeMillis() - timestamp) + " ms");
        }

        if (result.getResultCode() == AtCommandResult.ERROR) {
            Log.i(LOG_TAG, "Error processing <" + input +
                  "> with result <" + result.toString() + ">");
        }

        if (result.getResultCode() != AtCommandResult.RILRSP
                && result.getResultCode() != AtCommandResult.UNKNOWN_COMMAND) {
            sendURC("\r\n" + result.toString() + "\r\n");
        }

        releaseWakeLock();
    }

    public AtParser getAtParser() {
        return mAtParser;
    }

    @Override
    public void onDestroy() {
        // TODO Auto-generated method stub
        Log.d(LOG_TAG, " onDestroy");

        try {
            InputStream is = mSocket.getInputStream();
            if (is != null) {
                is.close();
            }
            mReceiver.requestStop();
            mReceiverThread.interrupt();
        } catch (IOException e) {
            Log.d(LOG_TAG, " IOException");
        } catch (NullPointerException npe) {
            Log.d(LOG_TAG, " NullPointerException");
        }
        super.onDestroy();
    }

    @Override
    public void onStart(Intent intent, int startId) {
        // TODO Auto-generated method stub
        super.onStart(intent, startId);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        NotificationManager notificationManager
                = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        NotificationChannel channel = new NotificationChannel(TITLE, TITLE,
                NotificationManager.IMPORTANCE_DEFAULT);
        notificationManager.createNotificationChannel(channel);
        startForeground(1, new NotificationCompat.Builder(this, TITLE)
                .setContentTitle(TITLE)
                .setSmallIcon(android.R.drawable.ic_dialog_alert)
                .build());
        if (!"1".equals(SystemProperties.get(ATCI_USERMODE, ""))) {
            stopForeground(true);
        }
        return START_STICKY;
    }

    public synchronized boolean sendURC(String urc) {
        boolean ret = true;

        if (urc.length() > 0) {
            Log.d(LOG_TAG, "URC Processing:" + urc + ">");
            if (mAtcid != null) {
                try {
                    mAtcid.sendCommandResponse(urc);
                } catch (RemoteException e) {
                    Log.e(LOG_TAG, "RemoteException");
                    ret = false;
                } catch (NoSuchElementException e) {
                    Log.e(LOG_TAG, "NoSuchElementException");
                    ret = false;
                }
            } else {
                ret = false;
            }
        }
        return true;
    }

    private synchronized void acquireWakeLock() {
        if (!mWakeLock.isHeld()) {
            mWakeLock.acquire();
        }
    }

    private synchronized void releaseWakeLock() {
        if (mWakeLock.isHeld()) {
            mWakeLock.release();
        }
    }

    private class AtcidCommandHandler extends IAtcidCommandHandler.Stub {
        @Override
        public void sendCommand(String data) {
            Log.d(LOG_TAG, "sendCommand " + data);
            handleInput(data);
        }
    };

    private class AtcidDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            Log.e(LOG_TAG, "serviceDied");
            mHandler.sendMessage(mHandler.obtainMessage(MSG_ID_CONNECT_TO_ATCID));
        }
    }

    class AtciReceiver implements Runnable {
        byte[] mBuffer;
        private volatile boolean mStopThread = false;

        protected synchronized void requestStop() {
            mStopThread = true;
            Log.i(LOG_TAG, "stop AtciReceiver thread.");
        }

        AtciReceiver() {
            mBuffer = new byte[ATCI_MAX_BUFFER_BYTES];
        }

        public void run() {
            int retryCount = 0;
            String socketAtci = SOCKET_NAME_ATCI;

            for (;; ) {
                LocalSocket s = null;
                LocalSocketAddress l;

                if (false == mStopThread) {
                    try {
                        s = new LocalSocket();
                        l = new LocalSocketAddress(socketAtci,
                                LocalSocketAddress.Namespace.RESERVED);
                        s.connect(l);
                    } catch (IOException ex) {
                        try {
                            if (s != null) {
                                s.close();
                            }
                        } catch (IOException ex2) {
                            //ignore failure to close after failure to connect
                            Log.e(LOG_TAG, "NullPointerException ex2");
                        }

                        if (retryCount == NUM_SIXTEEN) {
                            Log.e(LOG_TAG,
                                   "Couldn't find '" + socketAtci
                                   + "' socket after " + retryCount
                                   + " times, continuing to retry silently");
                        } else if (retryCount > NUM_ZERO && retryCount < NUM_SIXTEEN) {
                            Log.i(LOG_TAG,
                                   "Couldn't find '" + socketAtci
                                   + "' socket; retrying after timeout");
                        }

                        try {
                            Thread.sleep(SOCKET_OPEN_RETRY_MILLIS);
                        } catch (InterruptedException er) {
                            Log.e(LOG_TAG, "InterruptedException er");
                        }

                        retryCount++;
                        continue;
                    }

                    retryCount = 0;
                    mSocket = s;
                    Log.i(LOG_TAG, "Connected to '" + socketAtci + "' socket");

                    int length = 0;
                    InputStream is = null;
                    try {
                        int countRead = 0;
                        is = mSocket.getInputStream();

                        for (;; ) {
                            Log.i(LOG_TAG, "Wait to read command from ATCI generic service");
                            countRead = is.read(mBuffer);
                            if (countRead < 0) {
                                Log.e(LOG_TAG, "Hit EOS while reading message");
                                break;
                            }
                            if (countRead > 0) {
                                handleInput(new String(mBuffer, 0, countRead));
                            }
                        }
                    } catch (IOException ex) {
                        ex.printStackTrace();
                        Log.i(LOG_TAG, "'" + socketAtci + "' socket closed", ex);
                    } finally {
                        try {
                            if (is != null) {
                                is.close();
                                is = null;
                            }
                        } catch (IOException ex) {
                            Log.e(LOG_TAG, "IOException ex");
                        }
                    }

                    Log.i(LOG_TAG, "Disconnected from '" + socketAtci + "' socket");
                    try {
                        mSocket.close();
                    } catch (IOException ex) {
                        Log.e(LOG_TAG, "IOException ex2");
                    }
                    mSocket = null;
                }
            }
        }
    }



    protected void initializeAtVzw() {
        if (DBG) {
            Log.d(LOG_TAG, "initializeAtVzw");
        }
        mVzwApnStatus[VZW_INTERNET] = 0;
        mVzwApnStatus[VZW_IMS] = 0;
        mVzwApnStatus[VZW_APP] = 0;
        mVzwApnStatus[VZW_FOTA] = 0;
        mAtParser.register("+VZWACT", new AtCommandHandler() {
            /*   Query Commands :
             *   User types    => AT+VZWACT?
             *   Return String => APN and status
             */
            @Override
            public AtCommandResult handleReadCommand() {
                String status = "\"VZWINTERNET\"," + mVzwApnStatus[VZW_INTERNET] + "\r\n" +
                        "\"VZWIMS\"," + mVzwApnStatus[VZW_IMS] + "\r\n" +
                        "\"VZWAPP\"," + mVzwApnStatus[VZW_APP] + "\r\n" +
                        "\"VZWADMIN\"," + mVzwApnStatus[VZW_FOTA] + "\r\n";
                return new AtCommandResult(status);
            }


            /*   Assign  Commands :
             *   User types    =>  AT+VZWACT=active,APN
             *   Return String => OK or Error
             */
            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                Log.d(LOG_TAG, "[VzW] args:" + args.length);

                if (args.length == 2) {
                    String active = args[0].toString().toLowerCase();
                    String apn = args[1].toString().toLowerCase();

                    int response = AtCommandResult.OK;

                    Log.d(LOG_TAG, "[VzW] active: " + active + ", apn: " + apn);

                    switch(active) {
                    case "0":
                        if (apn.equals("\"ims\"") || apn.equals("\"vzwims\"")) {
                            releaseRequest(mNetworkCallback[VZW_IMS]);
                        } else if (apn.equals("\"vzwinternet\"")) {
                            releaseRequest(mNetworkCallback[VZW_INTERNET]);
                        } else if (apn.equals("\"vzwapp\"")) {
                            releaseRequest(mNetworkCallback[VZW_APP]);
                        } else if (apn.equals("\"vzwadmin\"")) {
                            releaseRequest(mNetworkCallback[VZW_FOTA]);
                        } else {
                            response = AtCommandResult.ERROR;
                        }
                        break;
                    case "1":
                        if (apn.equals("\"vzwims\"")) {
                            newRequest(NetworkCapabilities.NET_CAPABILITY_IMS, VZW_IMS);
                        } else if (apn.equals("\"vzwinternet\"")) {
                            newRequest(NetworkCapabilities.NET_CAPABILITY_INTERNET, VZW_INTERNET);
                        } else if (apn.equals("\"vzwapp\"")) {
                            newRequest(NetworkCapabilities.NET_CAPABILITY_MMS, VZW_APP);
                        } else if (apn.equals("\"vzwadmin\"")) {
                            newRequest(NetworkCapabilities.NET_CAPABILITY_FOTA, VZW_FOTA);
                        } else {
                            response = AtCommandResult.ERROR;
                        }
                        break;
                    default:
                        response = AtCommandResult.ERROR;
                    }

                    return new AtCommandResult(response);
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });

        mAtParser.register("+VZWAPNE", new AtCommandHandler() {
            // AT+VZWAPNE=<wapn>,<apncl>,<apnni>,<apntype>,<apnb>,<apned>,<apntime>
            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                Log.d(LOG_TAG, "[VzW] args: " + args.length);
                if (args.length >= 7) {
                    try {
                        int apnClass = Integer.parseInt(args[1].toString());
                        String apn = args[2].toString().replaceAll("\"", "");
                        String protocol = args[3].toString().replaceAll("\"", "");
                        String enabled = args[5].toString().replaceAll("\"", "");
                        Log.d(LOG_TAG, "[VzW] args: " + apnClass + "," + apn + "," + protocol
                                + "," + enabled);

                        ContentValues values = new ContentValues();
                        values.put(Telephony.Carriers.APN, apn);
                        values.put(Telephony.Carriers.PROTOCOL, protocol);
                        values.put(Telephony.Carriers.CARRIER_ENABLED,
                                "Enabled".compareToIgnoreCase(enabled) == 0 ? 1 : 0);
                        if (updateVzwApnSetting(apnClass, values)) {
                            return new AtCommandResult(AtCommandResult.OK);
                        }
                    } catch (NumberFormatException e) {
                        Log.e(LOG_TAG, "Exception parsing apn class: " + e);
                    }
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });

        mAtParser.register("+VZWAPNETMR", new AtCommandHandler() {
            // AT+VZWAPNETMR=<wapn>,<max_conn>,<max_conn_t>,<wait_time>,<throttling_time>,
            // <apncl>,<apnni>
            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                Log.d(LOG_TAG, "[VzW] args: " + args.length);
                if (args.length >= 7) {
                    try {
                        int maxConns = Integer.parseInt(args[1].toString());
                        int maxConnsTime = Integer.parseInt(args[2].toString());
                        int waitTime = Integer.parseInt(args[3].toString());
                        int apnClass = Integer.parseInt(args[5].toString());
                        String apn = args[6].toString().replaceAll("\"", "");
                        Log.d(LOG_TAG, "[VzW] args: " + maxConns + "," + maxConnsTime
                                + "," + waitTime + "," + apnClass + "," + apn);

                        ContentValues values = new ContentValues();
                        values.put(Telephony.Carriers.MAX_CONNECTIONS, maxConns);
                        values.put(Telephony.Carriers.TIME_LIMIT_FOR_MAX_CONNECTIONS, maxConnsTime);
                        values.put(Telephony.Carriers.WAIT_TIME_RETRY, waitTime);
                        if (updateVzwApnSetting(apnClass, values)) {
                            return new AtCommandResult(AtCommandResult.OK);
                        }
                    } catch (NumberFormatException e) {
                        Log.e(LOG_TAG, "Exception parsing apn class: " + e);
                    }
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });
    }

    private boolean updateVzwApnSetting(int apnClass, ContentValues values) {
        Log.d(LOG_TAG, "[VzW] updateVzwApnSetting: apnClass=" + apnClass + " values=" + values);
        if (apnClass < APN_CLASS_0 || apnClass >= APN_CLASS_NUM) {
            return false;
        }

        TelephonyManager telephonyManager
                = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        String plmn = telephonyManager.getSimOperatorNumeric();
        Log.d(LOG_TAG, "[VzW] updateVzwApnSetting: plmn=" + plmn);
        if (TextUtils.isEmpty(plmn)) {
            return false;
        }

        // Map class to id, check comments in getAllVzwApnSetting() for details.
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mContext);
        getAllVzwApnSetting(sp, plmn);
        String id = sp.getString(KEY_APN_NAME + apnClass, null);
        Log.d(LOG_TAG, "[VzW] updateVzwApnSetting: id=" + id);
        if (TextUtils.isEmpty(id)) {
            return false;
        }

        // Query and update telephony provider
        String where = "_id = ?";
        String[] args = new String[] {id};
        int updatedCount = getContentResolver().update(Telephony.Carriers.CONTENT_URI, values,
                where, args);
        Log.d(LOG_TAG, "[VzW] updateVzwApnSetting: updatedCount=" + updatedCount);

        return (updatedCount > 0);
    }

    private void getAllVzwApnSetting(SharedPreferences sp, String plmn) {
        /**
         * APN NI in db can be modified during test, cannot just use apn NI as key to query db. To
         * solve this problem, before the first +VZWAPNE command, use default VzW NI to query db and
         * save class<->id mapping in local. Then atci can use apn class as key.
         *
         * Currently, the apn configuration should make class<->id one-to-one mapping, or the latest
         * key will overwrite the old. And all apns with mvno defined will be skipped.
         */
        String[] projects = {Telephony.Carriers._ID, Telephony.Carriers.NAME,
                Telephony.Carriers.APN, Telephony.Carriers.TYPE, Telephony.Carriers.MVNO_TYPE};
        String where = "numeric = ?";
        String[] args = new String[] {plmn};
        Cursor cursor = getContentResolver().query(Telephony.Carriers.CONTENT_URI, projects, where,
                args, null);

        if (cursor == null) {
            Log.e(LOG_TAG, "[VzW] getAllVzwApnSetting: cursor is null when numeric=" + plmn);
            return;
        }

        SharedPreferences.Editor editor = sp.edit();
        while (cursor.moveToNext()) {
            String id = cursor.getString(cursor.getColumnIndex(projects[0]));
            String name = cursor.getString(cursor.getColumnIndex(projects[1]));
            String apn = cursor.getString(cursor.getColumnIndex(projects[2]));
            String types = cursor.getString(cursor.getColumnIndex(projects[3]));
            String mvno_type = cursor.getString(cursor.getColumnIndex(projects[4]));
            Log.d(LOG_TAG, "[VzW] getAllVzwApnSetting: id=" + id + " name=" + name + " apn=" + apn +
                    " types=" + types + " mnvo_type=" + mvno_type);
            if (!TextUtils.isEmpty(mvno_type)) {
                Log.d(LOG_TAG, "[VzW] getAllVzwApnSetting: skip for apn with mvno_type");
                continue;
            }
            int apnClass = getClassType(apn, parseTypes(types));
            editor.putString(KEY_APN_NAME + apnClass, id);
        }
        editor.commit();
    }

    private int getClassType(String apn, String[] types) {
        int classType = APN_CLASS_3;

        if (apn == null || types == null) {
            classType = APN_CLASS_X;
            Log.e(LOG_TAG, "[VzW] getClassType: apn or types undefined, set to undefined class x");
        } else if (ArrayUtils.contains(types, PhoneConstants.APN_TYPE_EMERGENCY)
                || VZW_EMERGENCY_NI.compareToIgnoreCase(apn) == 0) {
            classType = APN_CLASS_0;
        } else if (ArrayUtils.contains(types, PhoneConstants.APN_TYPE_IMS)
                || apn.toLowerCase().contains(VZW_IMS_NI.toLowerCase())) {
            classType = APN_CLASS_1;
        } else if (VZW_ADMIN_NI.compareToIgnoreCase(apn) == 0) {
            classType = APN_CLASS_2;
        } else if (VZW_APP_NI.compareToIgnoreCase(apn) == 0) {
            classType = APN_CLASS_4;
        } else if (VZW_800_NI.compareToIgnoreCase(apn) == 0) {
            classType = APN_CLASS_5;
        } else if (ArrayUtils.contains(types, PhoneConstants.APN_TYPE_DEFAULT)) {
            classType = APN_CLASS_3;
        } else if (VZW_DUN_NI.compareToIgnoreCase(apn) == 0) {
            classType = APN_CLASS_X;
        } else {
            Log.w(LOG_TAG, "[VzW] getClassType: set to default class 3");
        }

        Log.d(LOG_TAG, "[VzW] getClassType:" + classType);
        return classType;
    }

    private String[] parseTypes(String types) {
        String[] result;
        if (types == null || types.equals("")) {
            result = new String[1];
            result[0] = PhoneConstants.APN_TYPE_ALL;
        } else {
            result = types.split(",");
        }
        return result;
    }

    protected void initializeAtMmo() {
        if (DBG) {
            Log.d(LOG_TAG, "initializeAtMmo");
        }

        mAtParser.register("%EMMO", new AtCommandHandler() {
            private int getPreferredNwType(String args) {
                Log.d(LOG_TAG, "[AtMmo]args length = " + args.toString().length());
                int nwType = 0;
                int index = 0;

                index = args.toString().length();

                for (int len = 0; len < index; len++) {
                    if ((args.charAt(len) >= '0') && (args.charAt(len) <= '9')) {
                        nwType = nwType * 10 + (args.charAt(len) - '0');
                        Log.d(LOG_TAG, "[AtMmo]nwType = " + nwType);
                    }
                }

                Log.d(LOG_TAG, "[AtMmo]PreferredNwType = " + nwType);
                return nwType;
            }

            @Override
            public AtCommandResult handleReadCommand() {
                int subId = 0;
                int type = -1;

                TelephonyManager mTeleMgr = (TelephonyManager) getSystemService(
                        Context.TELEPHONY_SERVICE);
                if (mTeleMgr != null) {
                    subId = SubscriptionManager.from(mContext).getDefaultSubscriptionId();
                    type = mTeleMgr.getPreferredNetworkType(subId);
                    Log.d(LOG_TAG, "[AtMmo]get nw type to subId=" + subId + " type=" + type);
                }

                if (type != -1) {
                    String response = formatResponse("%EMMO: " + type);
                    return new AtCommandResult(response);
                } else {
                    return new AtCommandResult(AtCommandResult.ERROR);
                }
            }

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                int subId = 0;
                boolean result = false;

                Log.d(LOG_TAG, "[AtMmo]args:" + args.length);
                Log.d(LOG_TAG, "[AtMmo]args string:" + args[0]);

                if (args.length == 1) {
                    TelephonyManager mTeleMgr = (TelephonyManager) getSystemService(
                            Context.TELEPHONY_SERVICE);
                    int preferrendNwType = getPreferredNwType(args[0].toString());
                    if (mTeleMgr != null) {
                        subId = SubscriptionManager.from(mContext).getDefaultSubscriptionId();
                        result = mTeleMgr.setPreferredNetworkType(subId, preferrendNwType);
                        Log.d(LOG_TAG, "[AtMmo]set nw type to subId="
                                + subId + " result=" + result);
                    }
                }
                if (result) {
                    return new AtCommandResult(AtCommandResult.OK);
                } else {
                    return new AtCommandResult(AtCommandResult.ERROR);
                }
            }
        });
    }
    /**
     * Start a new {@link android.net.NetworkRequest} for VzW request
     */
    private void newRequest(int capability, int apn_type) {
        ConnectivityManager connectivityManager =
                (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        Log.d(LOG_TAG, "[VzW] newRequest, capability: " + capability);
        mNetworkCallback[apn_type] = new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(Network network) {
                super.onAvailable(network);
                if (mNetworkCallback[VZW_INTERNET] == this) {
                    mVzwApnStatus[VZW_INTERNET] = 1;
                    Log.d(LOG_TAG, "[VzW] onAvailable internet");
                } else if (mNetworkCallback[VZW_IMS] == this) {
                    mVzwApnStatus[VZW_IMS] = 1;
                    Log.d(LOG_TAG, "[VzW] onAvailable ims");
                } else if (mNetworkCallback[VZW_APP] == this) {
                    mVzwApnStatus[VZW_APP] = 1;
                    Log.d(LOG_TAG, "[VzW] onAvailable app");
                } else if (mNetworkCallback[VZW_FOTA] == this) {
                    mVzwApnStatus[VZW_FOTA] = 1;
                    Log.d(LOG_TAG, "[VzW] onAvailable fota");
                }
                Log.d(LOG_TAG, "[VzW] onAvailable");
            }

            @Override
            public void onLost(Network network) {
                super.onLost(network);
                if (mNetworkCallback[VZW_INTERNET] == this) {
                    mVzwApnStatus[VZW_INTERNET] = 0;
                } else if (mNetworkCallback[VZW_IMS] == this) {
                    mVzwApnStatus[VZW_IMS] = 0;
                } else if (mNetworkCallback[VZW_APP] == this) {
                    mVzwApnStatus[VZW_APP] = 0;
                } else if (mNetworkCallback[VZW_FOTA] == this) {
                    mVzwApnStatus[VZW_FOTA] = 0;
                }
                Log.d(LOG_TAG, "[VzW] onLost");
            }

            @Override
            public void onUnavailable() {
                super.onUnavailable();
                if (mNetworkCallback[VZW_INTERNET] == this) {
                    mVzwApnStatus[VZW_INTERNET] = 0;
                } else if (mNetworkCallback[VZW_IMS] == this) {
                    mVzwApnStatus[VZW_IMS] = 0;
                } else if (mNetworkCallback[VZW_APP] == this) {
                    mVzwApnStatus[VZW_APP] = 0;
                } else if (mNetworkCallback[VZW_FOTA] == this) {
                    mVzwApnStatus[VZW_FOTA] = 0;
                }
                Log.d(LOG_TAG, "[VzW] onUnavailable");
            }
        };
        mNetworkRequest[apn_type] = new NetworkRequest.Builder()
            .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
            .addCapability(capability)
            .build();

        connectivityManager.requestNetwork(
                mNetworkRequest[apn_type], mNetworkCallback[apn_type], CONN_MGR_TIMEOUT);
    }

    private void releaseRequest(ConnectivityManager.NetworkCallback callback) {
        if (callback != null) {
            Log.d(LOG_TAG, "[VzW] releaseRequest");
            ConnectivityManager connectivityManager =
                (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
            connectivityManager.unregisterNetworkCallback(callback);
            callback = null;
        } else {
            Log.d(LOG_TAG, "[VzW] releaseRequest, null");
        }
    }
}
