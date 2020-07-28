/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.omadm;

import android.app.admin.DevicePolicyManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.hardware.usb.*;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.BatteryManager;
import android.os.Binder;
import android.os.Build;
import android.os.DeadObjectException;
import android.os.Environment;
import android.os.FileUtils;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.os.Handler;
import android.os.HwBinder;
import android.os.UserHandle;
import android.os.SystemProperties;
import android.provider.Telephony.Sms.Intents;
import android.provider.Settings;
import android.provider.Settings.Global;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.PhoneStateListener;
import android.telephony.SmsMessage;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.util.Log;
import android.util.Slog;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.WspTypeDecoder;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.R;
import com.mediatek.common.omadm.IOmadmManager;
import com.mediatek.common.omadm.OmadmManager;
import com.mediatek.omadm.PalConstDefs;
import com.mediatek.omadm.Apns;
import com.mediatek.omadm.FotaApnSettings;
import com.mediatek.omadm.FotaNetworkManager;
import com.mediatek.omadm.FotaException;
import com.mediatek.omadm.ImsConfigManager;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.ParseException;
import java.util.Calendar;
import java.lang.Exception;
import java.lang.IllegalStateException;
import java.lang.reflect.Method;
import java.lang.Math;
import java.net.NetworkInterface;
import java.text.SimpleDateFormat;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.concurrent.atomic.AtomicLong;
import java.util.ArrayList;

import vendor.mediatek.hardware.omadm.V1_0.IOmadm;
import vendor.mediatek.hardware.omadm.V1_0.IOmadmIndication;


public class OmadmServiceImpl extends IOmadmManager.Stub {

    private final static String TAG = "OmadmServiceImpl";
    public static boolean DEBUG = true;

    private static final int MSG_SET_SMSIP = 0;
    private static final int MSG_GET_SMSIP = 1;

/*********************COMMON GET STRING TYPE API*************************/
    private static final int HIDL_GET_DEVICE_ID = 0;
    // Interface for pal_network_apn_class* functions
    private static final int HIDL_NW_APN_GET_ID = 1;
    private static final int HIDL_NW_APN_GET_NAME = 2;
    private static final int HIDL_NW_APN_GET_IP = 3;

    // Interface for pal_*_ims_* functions
    private static final int HIDL_NETWORK_IMS_DOMAIN = 4;
    private static final int HIDL_NETWORK_IMS_SMS_FORMAT_GET = 5;

    //Interface for pal_devDetail functions
    private static final int HIDL_SYSTEM_OEM_GET = 6;
    private static final int HIDL_SYSTEM_FWV_GET = 7;
    private static final int HIDL_SYSTEM_SWV_GET = 8;
    private static final int HIDL_SYSTEM_HWV_GET = 9;
    private static final int HIDL_SYSTEM_SUPPORT_LRGOBJ_GET = 10;
    private static final int HIDL_SYSTEM_DATE_GET = 11;
    private static final int HIDL_SYSTEM_TIMEUTC_GET = 12;
    private static final int HIDL_SYSTEM_HOSTDEVICE_MANU_GET = 13;
    private static final int HIDL_SYSTEM_HOSTDEVICE_MODEL_GET = 14;
    private static final int HIDL_SYSTEM_HOSTDEVICE_SWV_GET = 15;
    private static final int HIDL_SYSTEM_HOSTDEVICE_FWV_GET = 16;
    private static final int HIDL_SYSTEM_HOSTDEVICE_HWV_GET = 17;
    private static final int HIDL_SYSTEM_HOSTDEVICE_DATESTAMP_GET = 18;
    private static final int HIDL_SYSTEM_HOSTDEVICE_DEVICEID_GET = 19;
    private static final int HIDL_SYSTEM_DEVICETYPE_GET = 20;

    // Interface for pal_devInfo functions
    private static final int HIDL_MANUFACTURER = 21;
    private static final int HIDL_MODEL = 22;
    private static final int HIDL_DMV = 23;
    private static final int HIDL_GET_LANGUAGE = 24;
    private static final int HIDL_GET_ICCID = 25;
    private static final int HIDL_GETGET_EXT = 26;

/*********************COMMON GET INT TYPE API*************************/
    private static final int HIDL_IMS_SMS_OVER_IP_IS_ENABLED = 0;
    private static final int HIDL_IMS_SMS_FORMAT_GET = 1;
    private static final int HIDL_DCMO_VLT_GET = 2;
    private static final int HIDL_DCMO_LVC_GET = 3;
    private static final int HIDL_DCMO_VWF_GET = 4;

/*********************COMMON SET INT TYPE API*************************/
    private static final int HIDL_IMS_SMS_OVER_IP_ENABLE = 0;
    private static final int HIDL_IMS_SMS_FORMAT_SET = 1;
    private static final int HIDL_DCMO_VLT_SET = 2;
    private static final int HIDL_DCMO_LVC_SET = 3;
    private static final int HIDL_DCMO_VWF_SET = 4;
    private static final int HIDL_OMADM_ISFACTORY_SET = 5;

/*********************APN GET STRING TYPE API*************************/
    private static final int HIDL_APN_GET_NAME = 0;

/*********************APN SET STRING TYPE API*************************/
    private static final int HIDL_APN_SET_NAME = 0;
    private static final int HIDL_APN_RESTORE_INFO = 1;

/*********************APN GET INT TYPE API*************************/
    private static final int HIDL_APN_GET_IPVX = 0;
    private static final int HIDL_APN_GET_ID = 1;
    private static final int HIDL_APN_IS_ENABLE = 2;

/*********************APN SET INT TYPE API*************************/
    private static final int HIDL_APN_SET_IPVX = 0;
    private static final int HIDL_APN_SET_ENABLE = 1;
/******************************************************************/

    private final Context mContext;

    private final Object mLock = new Object();
    private final Object mAdNetLock = new Object();

    private final Handler mHandler = new Handler();
    private ImsConfigManager mImsConfigManager = null;
    private NetworkDetector mNwDetector = null;
    private FotaNetworkManager mFotaManager = null;
    private FotaApnSettings mFotaApn = null;

    private int mNetId = 0;
    private int mFactoyMode = 0;

    final class OmadmProxyDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            // Deal with service going away
            Log.d("@M_" + TAG, "OMADM HIDL serviceDied");
        }
    }

    public class OmadmIndication extends IOmadmIndication.Stub {


        private byte[] arrayListTobyte(ArrayList<Byte> data, int length) {
            byte[] byteList = new byte[length];
            for(int i = 0; i < length; i++)
            {
                byteList[i] = data.get(i);
            }
            Log.d("@M_" + TAG, "OMADM HIDL : arrayListTobyte, byteList = " + byteList);
            return byteList;
        }

        public String getStringTypeInfo(int iidex) {
            Log.d("@M_" + TAG, "getStringTypeInfo: idex" + iidex);
            switch (iidex) {
                case HIDL_GET_DEVICE_ID: return get_DeviceId();
                case HIDL_NETWORK_IMS_DOMAIN: return get_ImsDomain();
                case HIDL_SYSTEM_OEM_GET:{
                    try {
                        return get_DeviceOEM();
                    } catch (Exception e) {
                        Log.e(TAG, "Unexpected Exception ", e);
                    }
                    break;
                }
                case HIDL_SYSTEM_FWV_GET: return get_FWV();
                case HIDL_SYSTEM_SWV_GET: return get_SWV();
                case HIDL_SYSTEM_HWV_GET: return get_HWV();
                case HIDL_SYSTEM_SUPPORT_LRGOBJ_GET: return support_lrgobj_get();
                case HIDL_SYSTEM_DATE_GET: return get_Date();
                case HIDL_SYSTEM_TIMEUTC_GET: return get_Time();
                case HIDL_SYSTEM_HOSTDEVICE_MANU_GET:{
                    try {
                         return get_HostDeviceManu();
                    } catch (Exception e) {
                        Log.e(TAG, "Unexpected Exception ", e);
                    }
                    break;
                }
                case HIDL_SYSTEM_HOSTDEVICE_MODEL_GET: {
                    try {
                         return get_HostDeviceModel();
                    } catch (Exception e) {
                        Log.e(TAG, "Unexpected Exception ", e);
                    }
                    break;
                }
                case HIDL_SYSTEM_HOSTDEVICE_SWV_GET: return get_HostSWV();
                case HIDL_SYSTEM_HOSTDEVICE_FWV_GET: return get_HostFWV();
                case HIDL_SYSTEM_HOSTDEVICE_HWV_GET: return get_HostHWV();
                case HIDL_SYSTEM_HOSTDEVICE_DATESTAMP_GET: return get_HostDateStamp();
                case HIDL_SYSTEM_HOSTDEVICE_DEVICEID_GET:{
                    try {
                          return get_HostID();
                    } catch (Exception e) {
                        Log.e(TAG, "Unexpected Exception ", e);
                    }
                    break;
                }
                case HIDL_SYSTEM_DEVICETYPE_GET: {
                    try {
                          return get_Type();
                    } catch (Exception e) {
                        Log.e(TAG, "Unexpected Exception ", e);
                    }
                    break;
                }
                case HIDL_MANUFACTURER: {
                    try {
                        return get_Manufacturer();
                    } catch (RemoteException e) {
                        Log.e(TAG, "Unexpected Exception ", e);
                    }
                    break;
                }
                case HIDL_DMV: return get_Dmversion();
                case HIDL_GET_LANGUAGE: return get_Language();
                case HIDL_GET_ICCID: return get_Iccid();
                case HIDL_GETGET_EXT: return get_Ext();
                case HIDL_MODEL: return get_Model();
                default: Log.e(TAG, "Fault getStringTypeInfo iidex"); break;
            }
            return PalConstDefs.EMPTY_STRING;
        }

        public int getIntTypeInfo(int iidex) {
            Log.d("@M_" + TAG, "getIntTypeInfo: idex" + iidex);
            try {
                switch (iidex) {
                    case HIDL_IMS_SMS_OVER_IP_IS_ENABLED: return is_ImsSmsOverIpEnabled();
                    case HIDL_IMS_SMS_FORMAT_GET: return get_ImsSmsFormat();
                    case HIDL_DCMO_VLT_GET: return get_ImsVlt();
                    case HIDL_DCMO_LVC_GET: return get_ImsLvcState();
                    case HIDL_DCMO_VWF_GET: return get_ImsVwfState();
                    default: Log.e(TAG, "Fault getIntTypeInfo iidex"); break;
                }
            } catch (Exception e) {
                Log.e(TAG, "Unexpected Exception ", e);
            }
            return PalConstDefs.RET_ERR_NORES;
        }

        public int setIntTypeInfo(int iidex, int i_info) {
            Log.d("@M_" + TAG, "setIntTypeInfo: idex" + iidex + "info: " + i_info);
            try {
                switch (iidex) {
                    case HIDL_IMS_SMS_OVER_IP_ENABLE: return enable_ImsSmsOverIp(i_info);
                    case HIDL_IMS_SMS_FORMAT_SET: return set_ImsSmsFormat(i_info);
                    case HIDL_DCMO_VLT_SET: return set_ImsVlt(i_info);
                    case HIDL_DCMO_LVC_SET: return set_ImsLvcState(i_info);
                    case HIDL_DCMO_VWF_SET: return set_ImsVwfState(i_info);
                    case HIDL_OMADM_ISFACTORY_SET: return set_IsFactory(i_info);
                    default: Log.e(TAG, "Fault setIntTypeInfo iidex"); break;
                }
            } catch (Exception e) {
                Log.e(TAG, "Unexpected Exception ", e);
            }
            return PalConstDefs.RET_ERR;
        }

        public String getApnStringTypeInfo(int iidex, int apncls) {
            Log.d("@M_" + TAG, "getApnStringTypeInfo: idex" + iidex + "class: "+apncls);
            switch (iidex) {
                case HIDL_APN_GET_NAME: return get_ApnName(apncls);
                default: Log.e(TAG, "Fault getApnStringTypeInfo iidex"); break;
            }
            return PalConstDefs.EMPTY_STRING;
        }

        public int setApnStringTypeInfo(int iidex, int apncls, String s_info) {
            Log.d("@M_" + TAG, "setApnStringTypeInfo: idex"
                  + iidex + "class: " +apncls + "info: " + s_info);
            switch (iidex) {
                case HIDL_APN_SET_NAME: {
                    try {
                        return set_ApnName(apncls, s_info);
                    } catch (Exception e) {
                        Log.e(TAG, "Unexpected Exception ", e);
                    }
                    break;
                }
                case HIDL_APN_RESTORE_INFO: {
                    checkApnRestore(apncls, s_info);
                    break;
                }
                default: Log.e(TAG, "Fault setApnStringTypeInfo iidex"); break;
            }
            return PalConstDefs.RET_ERR;
        }

        public int getApnIntTypeInfo(int iidex, int apncls) {
            Log.d("@M_" + TAG, "getApnIntTypeInfo: idex" + iidex + "class: " +apncls);
            switch (iidex) {
                case HIDL_APN_GET_IPVX: return get_ApnIpvX(apncls);
                case HIDL_APN_GET_ID: return get_ApnId(apncls);
                case HIDL_APN_IS_ENABLE: return is_ApnEnabled(apncls);
                default: Log.e(TAG, "Fault getApnIntTypeInfo iidex"); break;
            }
            return PalConstDefs.RET_ERR;
        }

        public int setApnIntTypeInfo(int iidex, int apncls, int i_info) {
            Log.d("@M_" + TAG, "setApnIntTypeInfo: idex" + iidex + "class: "
                  +apncls + "info: " + i_info);
            try {
                switch (iidex) {
                    case HIDL_APN_SET_IPVX: return set_ApnIpvX(apncls, i_info);
                    case HIDL_APN_SET_ENABLE: return enable_Apn(apncls, i_info);
                    default: Log.e(TAG, "Fault setApnIntTypeInfo iidex"); break;
                }
            } catch (Exception e) {
                Log.e(TAG, "Unexpected Exception ", e);
            }

            return PalConstDefs.RET_ERR;
        }

        public int requestAdminNetwork(boolean enable) {
            Log.d("@M_" + TAG, "requestAdminNetwork : enable = " + enable);
            int ret = PalConstDefs.RET_SUCC;
            try {
                ret = request_AdminNetwork(enable);
            } catch (RemoteException e) {
                Log.e(TAG, "Unexpected Exception ", e);
            }
            return ret;
        }
    }

    //HIDL
    private volatile IOmadm mOmadmProxy = null;
    private final OmadmProxyDeathRecipient mOmadmProxyDeathRecipient=new OmadmProxyDeathRecipient();
    private final AtomicLong mOmadmProxyCookie = new AtomicLong(0);
    private OmadmIndication mOmadmIndication = new OmadmIndication();

    public IOmadm getOmadmProxy() {
        if (mOmadmProxy != null) {
            return mOmadmProxy;
        }
        try {
            Log.d("@M_" + TAG, "IOmadm.getService");
            mOmadmProxy = IOmadm.getService("omadm");

            if (mOmadmProxy != null) {
                mOmadmProxy.linkToDeath(mOmadmProxyDeathRecipient,
                        mOmadmProxyCookie.incrementAndGet());
                Log.d("@M_" + TAG, "setResponseFunctions");
                mOmadmProxy.setResponseFunctions(mOmadmIndication);
                setEventhndlIntentListeners();
                regNwkEventReceiver();

            } else {
                Log.d("@M_" + TAG, "getOmadmProxy: mImsaProxy == null");
            }
        } catch (RemoteException | RuntimeException e) {
            mOmadmProxy = null;
            Log.d("@M_" + TAG, "mOmadmProxy getService/setResponseFunctions: " + e);
        }

        return mOmadmProxy;
    }

    public OmadmServiceImpl(Context context) {
        Log.d(TAG, "OmadmServiceImpl() ... constructor");
        mContext = context;
        mImsConfigManager = new ImsConfigManager(context);

        getOmadmProxy();
    }

    public BroadcastReceiver mEvHandlerReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.w(TAG, "BroadcastReceiver::onReceive()");
            String action = intent.getAction();
            synchronized(this) {
                if (mOmadmProxy != null) {
                    if (action.equals(Intents.WAP_PUSH_RECEIVED_ACTION) && isOmadmWapPush(intent)) {
                        if (DEBUG) Log.d(TAG, "WAP PUSH received");
                        checkOmadmWapPush(intent);
                        abortBroadcast();
                    } else if (action.equals(Intents.DATA_SMS_RECEIVED_ACTION)) {
                        if (DEBUG) Log.d(TAG, "DATA SMS received");
                        checkDataSms(intent);
                    } else if (action.equals(Intents.SMS_RECEIVED_ACTION)) {
                        if (DEBUG) Log.d(TAG, "DATA SMS received");
                        checkDataSms(intent);
                    } else if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
                        Log.d(TAG, "ACTION_BOOT_COMPLETED received");
                        //int sup = mImsConfigManager.isImcPvsInfoSupport();
                        //Log.d(TAG, "ACTION_BOOT_COMPLETED received ... pVs supp="+sup);
                    } else if (action.equals("android.intent.action.SIM_STATE_CHANGED")) {
                        String state = intent.getStringExtra("ss");
                        Log.d(TAG, "SIM_STATE_CHANGED received, state = "+state);
                        if(state.equals("LOADED") && mFactoyMode == 1){
                            Log.d(TAG,
                                 "SIM_STATE_CHANGED Loaded & is factory mode check APN NVsettings");
                            try {
                                mOmadmProxy.writeInt(6);
                            } catch(Exception e) {
                                e.printStackTrace();
                            }
                            mFactoyMode = 0;
                        }
                    }
                }
            }
        }
    };

    private void regNwkEventReceiver() {
        unregNwkEventReceiver();
        mNwDetector = new NetworkDetector(mContext, this);

        if (mNwDetector != null) {
            mNwDetector.register(mContext);
        } else {
            Log.e(TAG, "regNwkEventReceiver() cannot allocate NetworkDetector object");
        }
    }

    private void unregNwkEventReceiver() {
        if (mNwDetector != null) {
            mNwDetector.unregister(mContext);
            mNwDetector = null;
        }
    }

    private void setEventhndlIntentListeners() {
        Log.w(TAG, "setEventhndlIntentListeners");
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intents.WAP_PUSH_RECEIVED_ACTION);

        try {
            intentFilter.addDataType(WspTypeDecoder.CONTENT_TYPE_B_PUSH_SYNCML_NOTI);
        } catch (IntentFilter.MalformedMimeTypeException e) {
            Log.w(TAG, "Malformed SUPL init mime type");
        }
        mContext.registerReceiver(mEvHandlerReceiver, intentFilter, null, mHandler);

        intentFilter = new IntentFilter();
        intentFilter.addAction(Intents.DATA_SMS_RECEIVED_ACTION);
        intentFilter.addDataScheme("sms");
        intentFilter.addDataAuthority("localhost","0");
        mContext.registerReceiver(mEvHandlerReceiver, intentFilter, null, mHandler);

        intentFilter = new IntentFilter();
        intentFilter.addAction(Intents.SMS_RECEIVED_ACTION);
        mContext.registerReceiver(mEvHandlerReceiver, intentFilter, null, mHandler);

        intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_BOOT_COMPLETED);
        mContext.registerReceiver(mEvHandlerReceiver, intentFilter, null, mHandler);

        intentFilter = new IntentFilter();
        intentFilter.addAction("android.intent.action.SIM_STATE_CHANGED");
        mContext.registerReceiver(mEvHandlerReceiver, intentFilter, null, mHandler);
    }

    private boolean isOmadmWapPush(Intent intent) {
        if (DEBUG) Slog.d(TAG, "isOmadmPush()");
        String type = intent.getType();
        return type.equals(WspTypeDecoder.CONTENT_TYPE_B_PUSH_SYNCML_NOTI);
    }

   public void checkDataSms(Intent intent) {
        if (DEBUG) Log.d(TAG, "checkDataSms()");
        try {
            SmsMessage[] sms = Intents.getMessagesFromIntent(intent);
            for (SmsMessage msg : sms) {
                if (DEBUG) Log.d(TAG, "message body " + msg.getMessageBody());
                mOmadmProxy.SmsCancelSysUpdate(msg.getMessageBody());
            }
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    public ArrayList<Byte> byteToArrayList(int length, byte [] value) {
        ArrayList<Byte> myarraylist = new ArrayList<Byte>();
        Log.d("@M_" + TAG, " byteToArrayList, value.length = "
              + value.length + ", value = " + value + ", length = " + length);
        for(int i = 0; i < length; i++)
        {
            myarraylist.add(value[i]);
        }
        return myarraylist;
    }

    public void checkOmadmWapPush(Intent intent) {
        if (DEBUG) Log.d(TAG, "checkOmadmWapPush()");
        try {
            byte[] data = (byte[])intent.getExtra("data");
            int cnt = data.length;
            for (int i = 0; i < cnt; i++) {
                int positive = data[i] & 0xff;
                Log.d(TAG, "checkOmadmWapPush: get content["+i+"]="+positive);
            }
            Log.d(TAG, "checkOmadmWapPush(): get WAP data len = "+cnt);
            mOmadmProxy.writeEvent((int)intent.getExtra("transactionId"),
                                    cnt, byteToArrayList(cnt, data));
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

   public String get_DeviceId(){
        TelephonyManager manager =
          (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        String str = manager.getDeviceId();

        if (str != null) {
            return str;
        } else {
            Log.w(TAG, PalConstDefs.TELEPHONYMANAGER + PalConstDefs.NOT_READY);
            return PalConstDefs.EMPTY_STRING;
        }
    }

    public String get_Manufacturer()throws RemoteException {
        String manufacturer =
          (Build.MANUFACTURER.substring(0, 1)).toUpperCase() + Build.MANUFACTURER.substring(1);
        return manufacturer;
    }

    public String get_Model(){
        String str = System.getProperty("ro.vendor.product.model", "k71v1_64_bsp");
        Log.d(TAG, "Model prop =  "
              + System.getProperty("ro.vendor.product.model", "") + "get str = " + str);
        return str;
    }

    public String get_Dmversion(){
        return "1.2";
    }

    public String get_Language(){
        return Locale.getDefault().getDisplayLanguage();
    }

    public String get_Iccid() {
        TelephonyManager mgr =
          (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);

        String ssn = null;
        if (mgr != null) {
            if (mgr.getSimState() == TelephonyManager.SIM_STATE_READY) {
                ssn = mgr.getSimSerialNumber();
            } else {
                Log.w(TAG, PalConstDefs.SIM_STATE + PalConstDefs.NOT_READY);
                return PalConstDefs.NULL_STRING;
            }
        } else {
            Log.w(TAG, PalConstDefs.TELEPHONYMANAGER + PalConstDefs.NOT_READY);
            return PalConstDefs.NULL_STRING;
        }
        return ssn;
    }

   public String get_Ext(){
        return "./DevInfo/Ext/ConfigurationVer";
    }

   public String get_DeviceOEM() {
        String man = Build.MANUFACTURER;
        String model = Build.MODEL;

        if ((model == null) && (man == null)) {
            Log.w(TAG, PalConstDefs.MANUFACTURER + PalConstDefs.NOT_IDENTIFIED);
            throw new IllegalStateException(PalConstDefs.MANUFACTURER
                                            + PalConstDefs.NOT_IDENTIFIED);
        }
        if (man != null) {
            if (model != null) {
                if (model.startsWith(man)) {
                    return model;
                } else {
                    return man;
                }
            } else {
                return man;
            }
        } else {
            Log.w(TAG, PalConstDefs.MANUFACTURER + PalConstDefs.NOT_IDENTIFIED);
            throw new IllegalStateException(PalConstDefs.MANUFACTURER
                                            + PalConstDefs.NOT_IDENTIFIED);
        }
    }

  public String get_FWV() {
        String ver = PalConstDefs.EMPTY_STRING;
        try {
            ver = readFile(PalConstDefs.FIRMWARE_VER_PATH);
        } catch (IOException e) {
            Log.e(TAG, e.getMessage());
        }
        if (ver == PalConstDefs.EMPTY_STRING) {
            return (PalConstDefs.EMPTY_STRING);
        }
        return ver;
    }

  public String get_SWV() {
        String ver = Build.VERSION.RELEASE+" API:"+Build.VERSION.SDK_INT+" build: "+Build.DISPLAY;
        if (ver == PalConstDefs.EMPTY_STRING) {
            return ("n/a");
        }
        return ver;
    }

    public String get_HWV() {
        String ver = Build.HARDWARE;
        if (ver == PalConstDefs.EMPTY_STRING) {
            return ("n/a");
        }
        return ver;
    }

    public String support_lrgobj_get() {
        return "false";
    }

    public String get_Date() {
        long sec = Build.TIME;
        Date date = new Date(sec);
        SimpleDateFormat sdf = new SimpleDateFormat("MM:dd:yyyy");
        String dateString = sdf.format(date);
        return dateString;
    }

    public String get_Time() {
        long sec = Build.TIME;
        Date time = new Date(sec);
        SimpleDateFormat sdf = new SimpleDateFormat("hh:mm");
        String timeString = sdf.format(time);
        return timeString;
    }

    public String get_HostDeviceManu() {
        int type = PalConstDefs.PARENT_DEVICE;

        if (type == PalConstDefs.CURRENT_DEVICE) {
            return get_DeviceOEM();
        } else if (type == PalConstDefs.CHILD_DEVICE) {
            UsbManager mngr = (UsbManager) mContext.getSystemService(Context.USB_SERVICE);
            int devices = mngr.getDeviceList().size();

            if (devices == 0) {
                Log.w(TAG, "no devices");
                throw new IllegalStateException(PalConstDefs.OPERATION_NOT_SUPPORTED);
            } else {
                Object[] dev = mngr.getDeviceList().values().toArray();
                UsbDevice usbDevice = (UsbDevice) dev[0];
                return usbDevice.getManufacturerName();
            }
        } else {
            Log.w(TAG, PalConstDefs.HOST_OPERATION +" "+ PalConstDefs.OPERATION_NOT_SUPPORTED);
            return (PalConstDefs.NOT_AVAILABLE);
        }
    }

    public String get_HostDeviceModel() {
        int type = PalConstDefs.PARENT_DEVICE;
        if (type == PalConstDefs.CURRENT_DEVICE) {
            return Build.MODEL;
        } else if (type == PalConstDefs.CHILD_DEVICE) {
            UsbManager mngr = (UsbManager) mContext.getSystemService(Context.USB_SERVICE);
            int devices = mngr.getDeviceList().size();

            if (devices == 0) {
                Log.w(TAG, "no devices");
                throw new IllegalStateException(PalConstDefs.OPERATION_NOT_SUPPORTED);

            } else {
                Object[] device = mngr.getDeviceList().values().toArray();
                UsbDevice usbDevice = (UsbDevice) device[0];
                return usbDevice.getProductName();
            }
        } else {
            Log.w(TAG, PalConstDefs.HOST_OPERATION  +" "+  PalConstDefs.OPERATION_NOT_SUPPORTED);
            return (PalConstDefs.NOT_AVAILABLE);

        }
    }

    public String get_HostHWV() {
        int type = PalConstDefs.PARENT_DEVICE;
        String res = PalConstDefs.EMPTY_STRING;

        if (type == PalConstDefs.CURRENT_DEVICE) {
            return get_HWV();
        } else {
            Log.w(TAG, PalConstDefs.HOST_OPERATION +" "+ PalConstDefs.OPERATION_NOT_SUPPORTED);
            return (PalConstDefs.NOT_AVAILABLE);

        }
    }

    public String get_HostSWV() {
        int type = PalConstDefs.PARENT_DEVICE;
        String res = PalConstDefs.EMPTY_STRING;
        if (type == PalConstDefs.CURRENT_DEVICE) {
            return get_SWV();

        } else {
            Log.w(TAG, PalConstDefs.HOST_OPERATION  +" "+  PalConstDefs.OPERATION_NOT_SUPPORTED);
            return (PalConstDefs.NOT_AVAILABLE);
        }
    }

    public String get_HostFWV() {
        int type = PalConstDefs.PARENT_DEVICE;

        if (type == PalConstDefs.CURRENT_DEVICE) {
            return get_FWV();
        } else {
            Log.w(TAG, PalConstDefs.HOST_OPERATION  +" "+  PalConstDefs.OPERATION_NOT_SUPPORTED);
            return (PalConstDefs.NOT_AVAILABLE);
        }
    }

    public String get_HostDateStamp() {
        int type = PalConstDefs.PARENT_DEVICE;

        if (type == PalConstDefs.CURRENT_DEVICE) {
            long sec = Build.TIME;
            Date time = new Date(sec);
            SimpleDateFormat sdf = new SimpleDateFormat("dd:MM:yyyy hh:mm:ss");
            String timeString = sdf.format(time);
            return timeString;
        } else {
            Log.w(TAG, PalConstDefs.HOST_OPERATION  +" "+  PalConstDefs.OPERATION_NOT_SUPPORTED);
            return (PalConstDefs.NOT_AVAILABLE);
        }
    }

    public String get_HostID() {
        int type = PalConstDefs.PARENT_DEVICE;
        if (type == PalConstDefs.CURRENT_DEVICE) {
            return get_DeviceId() + " " + Build.SERIAL;
        } else {
            Log.w(TAG, PalConstDefs.HOST_OPERATION  +" "+  PalConstDefs.OPERATION_NOT_SUPPORTED);
            return (PalConstDefs.NOT_AVAILABLE);
        }
    }

    public String get_Type() {
        String result;
        TelephonyManager mngr =
          (TelephonyManager)mContext.getSystemService(Context.TELEPHONY_SERVICE);
        if (mngr == null) {
            Log.w(TAG, PalConstDefs.TELEPHONYMANAGER + PalConstDefs.NOT_READY);
            throw new IllegalStateException(PalConstDefs.TELEPHONYMANAGER + PalConstDefs.NOT_READY);
        }

        if (mngr.getPhoneType() == TelephonyManager.PHONE_TYPE_NONE) {
            result = PalConstDefs.FEATURE_PHONE;
        } else {
            result = PalConstDefs.SMART_DEVICE;
        }

        if (result != null) {
            return result;
        } else {
            Log.w(TAG, PalConstDefs.TELEPHONYMANAGER + PalConstDefs.NOT_READY);
            throw new IllegalStateException(PalConstDefs.TELEPHONYMANAGER + PalConstDefs.NOT_READY);
        }
    }


    private String readFile(String fileName) throws IOException {
        String data = PalConstDefs.EMPTY_STRING;
        File file = new File(PalConstDefs.MO_WORK_PATH, fileName);

        if (file.exists()) {
            FileInputStream confVerFos = new FileInputStream(file);
            int dataLength = (int) file.length();

            if (dataLength > 0) {
                byte[] buffer = new byte[dataLength];
                confVerFos.read(buffer, 0, dataLength);
                data = new String(buffer);
            }
        }
        else {
            file.createNewFile();
            Log.w(TAG, "read attempt on file [" + fileName +"] failed... create new one");
        }
        return data;
    }

    private void writeToFile(String fileName, String value) throws IOException {
        FileOutputStream fileOutputStream =
          new FileOutputStream(PalConstDefs.MO_WORK_PATH + fileName, false);
        fileOutputStream.write(value.getBytes());
        fileOutputStream.close();
    }

    public String get_ApnName(int apnCls) {
        try {
            String apnName = Apns.getName(mContext, apnCls);
            if (DEBUG) Slog.d(TAG, "get_ApnName[" + apnCls + "] = " + apnName);
            return apnName;
        } catch (Exception e) {
            Slog.w(TAG, "get_ApnName cannot get valid info", e);
            return PalConstDefs.EMPTY_STRING;
        }
    }

    public int set_ApnName(int apnCls, String apnName) {
        if (DEBUG) Slog.d(TAG, "set_ApnName[" + apnCls + "] = " + apnName);
        int ret = Apns.setName(mContext, apnCls, apnName);
        storeApnConfig(apnCls);
        return ret;
    }

    public int get_ApnIpvX(int apnCls) {
        try {
            int mask = Apns.getIpVersions(mContext, apnCls);
            if (DEBUG) Slog.d(TAG, "get_ApnIpvX[" + apnCls + "] = " + mask);
            return mask;
        } catch (Exception e) {
            Slog.w(TAG, "get_ApnIpvX cannot get valid info", e);
            return -1;
        }
    }

    public int get_ApnId(int apnCls) {
        int id = Apns.getId(mContext, apnCls);
        if (DEBUG) Slog.d(TAG, "get_ApnId[" + apnCls + "] = " + id);
        return id;
    }

   public int set_ApnIpvX(int apnCls, int protocol) {
        int rc = Apns.setIpVersions(mContext, apnCls, protocol);
        if (DEBUG) Slog.d(TAG, "set_ApnIpvX[" + apnCls + "] = " + protocol);
        storeApnConfig(apnCls);
        return rc;
    }

    public int is_ApnEnabled(int apnCls) {
        try {
            int enabled = Apns.isEnabled(mContext, apnCls);
            if (DEBUG) Slog.d(TAG, "is_ApnEnabled [ " + apnCls + "] = " + enabled);
            return enabled;
        } catch (Exception e) {
            Slog.w(TAG, "is_ApnEnabled cannot get valid info", e);
            return -1;
        }
    }

    public int enable_Apn(int apnCls, int enable) {
        int ret = Apns.enable(mContext, apnCls, enable);
        /*if(apnCls == 3) {
            mImsConfigManager.setImcProvisioned(
                ImsConfig.ConfigConstants.LVC_SETTING_ENABLED, enable);
        }*/
        if (DEBUG) Slog.d(TAG, "enable_Apn [ " + apnCls + "] = " + enable);
        storeApnConfig(apnCls);
        return ret;
    }

    public void checkApnRestore(int apnCls, String data) {
        if (data != PalConstDefs.EMPTY_STRING) {
            String field[] = data.split("/");
            Log.d(TAG, "checkApnRestore ... get and resotre config["+apnCls+"] = "
                  + field[0] + ", " + field[1] + " ," + field[2]);
            set_ApnName(apnCls,field[0]);
            int ipvX = field[1].equals("3")? 3 : (field[1].equals("2")? 2 : 1);
            set_ApnIpvX(apnCls, ipvX);
            int en = field[2].equals("0")? 0 : 1;
            enable_Apn(apnCls, en);
        }
    }

    public void storeApnConfig(int idex) {
        String fileName = PalConstDefs.APN_CFG_IDEX + idex;
        String Config = fileName + "/" + get_ApnName(idex) + "/"
          + get_ApnIpvX(idex) + "/" + is_ApnEnabled(idex)+"|";
        Log.d(TAG, "storeApnConfig: " + Config);
        try {
            byte[] data = Config.getBytes();
            int cnt = data.length;
            mOmadmProxy.writeBytes(byteToArrayList(cnt, data));
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    public int is_ImsSmsOverIpEnabled() {
        //return mImsConfigManager.getImcProvisioned(ImsConfig.ConfigConstants.SMS_OVER_IP);
        return 1;
    }

    public int enable_ImsSmsOverIp(int enable) {
        mImsConfigManager.setProvisionedIntValue(ImsConfig.ConfigConstants.SMS_OVER_IP, enable);
        //mImsConfigManager.setImcProvisioned(ImsConfig.ConfigConstants.SMS_OVER_IP, enable);
        return PalConstDefs.RET_SUCC;
    }

    public String get_ImsDomain(){
        return PalConstDefs.IMS_DOMAIN;
    }

    public int get_ImsSmsFormat() {
        return mImsConfigManager.getProvisionedIntValue(ImsConfig.ConfigConstants.SMS_FORMAT);
    }

    public int set_ImsSmsFormat(int format) {
        mImsConfigManager.setProvisionedIntValue(ImsConfig.ConfigConstants.SMS_FORMAT, format);
        return PalConstDefs.RET_SUCC;
    }

    public void notifyOmadmNetworkManager(int netFeature, boolean enabled) {
        Slog.d(TAG, "notifyOmadmNetworkManager [ " + netFeature + "] = " + enabled);
        synchronized (mLock) {
            if (mOmadmProxy != null) {
                try {
                    mOmadmProxy.omadmNetManagerReply(netFeature, (enabled?1:0));
                } catch (DeadObjectException e) {
                    Log.w(TAG, "Binder died. Remove listener");
                    mOmadmProxy = null;
                } catch (RemoteException e) {
                    Log.e(TAG, "Unexpected Exception ", e);
                }
            }
        }
    }

    public void omadmControllerDispachAdminNetStatus(int status, int netId) {
        synchronized (mLock) {
            if (mOmadmProxy != null) {
                try {
                    mOmadmProxy.omadmControllerDispachAdminNetStatus(status, netId);
                } catch (DeadObjectException e) {
                    Log.w(TAG, "Binder died. Remove listener");
                    mOmadmProxy = null;
                } catch (RemoteException e) {
                    Log.e(TAG, "Unexpected Exception ", e);
                }
            }
        }
    }

    private static AtomicInteger trCounter = new AtomicInteger(0);

    public int request_AdminNetwork(boolean enable) throws RemoteException {
        synchronized(mAdNetLock) {
            final int subID = SubscriptionManager.getDefaultDataSubscriptionId();
            if( SubscriptionManager.INVALID_SUBSCRIPTION_ID == subID ) {
                Log.e(TAG, "Phone is not ready. Sub ID = " + subID);
                return PalConstDefs.RET_ERR_NORES;
            }
            if( Settings.Global.getInt(mContext.getContentResolver(),
                            Settings.Global.AIRPLANE_MODE_ON, 0) != 0 ) {
                Log.w(TAG, "Unable to acquire Admin Network if Airplane mode ON");
                omadmControllerDispachAdminNetStatus(PalConstDefs.ADMIN_NET_UNAVAILABLE, 0);
            }
            if (mFotaManager == null) {
                mFotaManager = new FotaNetworkManager(this, mContext, subID);
            }
            if(enable) {
                if (mOmadmProxy == null) {
                    Log.e(TAG, "No admin network listeners registred");
                    return PalConstDefs.RET_ERR_STATE;
                }
                trCounter.incrementAndGet();
                new Thread() {
                    @Override
                    public void run() {
                        try {
                            mNetId = mFotaManager.acquireNetwork(TAG);
                            String apnName = mFotaManager.getApnName();
                            if (DEBUG) Log.v(TAG, "Network ID = " + mNetId
                                             + " APN name = " + apnName);
                            mFotaApn = FotaApnSettings.load(mContext, apnName, subID, TAG, true);
                            if (DEBUG) Log.v(TAG, "Using " + mFotaApn.toString());
                            omadmControllerDispachAdminNetStatus(
                              PalConstDefs.ADMIN_NET_AVAILABLE, mNetId);
                        } catch(FotaException e) {
                            Log.w(TAG, "Unable to acquire Admin Network. Timed Out");
                            omadmControllerDispachAdminNetStatus(
                              PalConstDefs.ADMIN_NET_UNAVAILABLE, 0);
                        } catch(Exception e) {
                            // Catch everything to avoid system server crash
                            Log.e(TAG, "Caught exception when acquiring Admin Network" + e);
                            omadmControllerDispachAdminNetStatus(
                              PalConstDefs.ADMIN_NET_UNAVAILABLE, 0);
                        } finally {
                            trCounter.decrementAndGet();
                        }
                    }
                }.start();
            } else {
                if(mFotaManager.releaseNetwork(TAG)) {
                    if(trCounter.compareAndSet(0, 0)) {
                        mFotaManager = null;
                    }
                }
            }
            return PalConstDefs.RET_SUCC;
        }
    }

    public int get_ImsVlt() {
        if (DEBUG) Slog.d(TAG, "getImsVlt()");
        int value = mImsConfigManager.getProvisionedIntValue(
                ImsConfig.ConfigConstants.VLT_SETTING_ENABLED);
        if (DEBUG) Slog.d(TAG, "getImsVlt() = " + value);
        return value;
    }

    public int set_ImsVlt(int value) {
        if (DEBUG) Slog.d(TAG, "setImsVlt(" + value + ")");
        mImsConfigManager.setProvisionedIntValue(
                ImsConfig.ConfigConstants.VLT_SETTING_ENABLED, value);
        return PalConstDefs.RET_SUCC;
    }

    public int get_ImsLvcState() {
        if (DEBUG) Slog.d(TAG, "getImsLvcState()");
        int value = mImsConfigManager.getProvisionedIntValue(
                ImsConfig.ConfigConstants.LVC_SETTING_ENABLED);
        if (DEBUG) Slog.d(TAG, "getImsLvcState() = " + value);
        return value;
    }

    public int set_ImsLvcState(int val) {
        if (DEBUG) Slog.d(TAG, "setImsLvcState(" + val + ")");
        mImsConfigManager.setProvisionedIntValue(
                ImsConfig.ConfigConstants.LVC_SETTING_ENABLED, val);
        return PalConstDefs.RET_SUCC;
    }

    public int get_ImsVwfState() {
        if (DEBUG) Slog.d(TAG, "getImsVwfState()");
        int value = mImsConfigManager.getProvisionedIntValue(
                ImsConfig.ConfigConstants.VOICE_OVER_WIFI_MODE);
        if (DEBUG) Slog.d(TAG, "getImsVwfState() = " + value);
        if (value == ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY)  {
            value = 1;
        } else {
            value = 0;
        }
        return value;
    }

    public int set_ImsVwfState(int val) {
        if (DEBUG) Slog.d(TAG, "setImsVwfState(" + val + ")");
        if (val != 0) {
            val = ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY;
        } else {
            val = ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED;
        }
        mImsConfigManager.setProvisionedIntValue(
                ImsConfig.ConfigConstants.VOICE_OVER_WIFI_MODE, val);
        return PalConstDefs.RET_SUCC;
    }

    public int set_IsFactory(int val) {
        if (DEBUG) Slog.d(TAG, "set_IsFactory(" + val + ")");
        mFactoyMode = val;
        return PalConstDefs.RET_SUCC;
    }

    /**
     * Opens protected file.
     * Service is used here as ContentProvider.
     * @param path
     * @return
     * @throws RemoteException
     */
    public ParcelFileDescriptor inputStream(String path) throws RemoteException {
        if (DEBUG) Slog.d(TAG, "inputStream(" + path + ")");
        if (!com.mediatek.omadm.FileUtils.checkPathAllow(path)) {
            Slog.w(TAG, "inputStream(" + path + "): is not allowed");
            throw new SecurityException("");
        }
        try {
            FileInputStream is = new FileInputStream(path);
            ParcelFileDescriptor pfd = com.mediatek.omadm.FileUtils.pipeTo(is);
            return pfd;
        } catch (FileNotFoundException excp) {
            PalConstDefs.throwEcxeption(
                    PalConstDefs.RET_ERR_NORES);
        } catch (IOException excp) {
            PalConstDefs.throwEcxeption(
                    PalConstDefs.RET_ERR_TMOUT);
        }
        return null;
    }

    /**
     * Add your methods here
     */

}
