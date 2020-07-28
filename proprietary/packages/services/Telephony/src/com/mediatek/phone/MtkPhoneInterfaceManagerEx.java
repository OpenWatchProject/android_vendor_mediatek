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

package com.mediatek.phone;

import android.Manifest.permission;
import android.annotation.Nullable;
import android.app.ActivityManager;
import android.app.AppOpsManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.NetworkStats;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.ResultReceiver;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.UserManager;
import android.os.WorkSource;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.service.carrier.CarrierIdentifier;
import android.telephony.AccessNetworkConstants;
import android.telephony.CellInfo;
import android.telephony.CellLocation;
import android.telephony.LocationAccessPolicy;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.telephony.RadioAccessFamily;
import android.text.TextUtils;
import android.util.ArraySet;
import android.util.Log;
import android.util.Pair;
import android.util.Slog;

import com.android.phone.PhoneGlobals;

import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.IccPhoneBookInterfaceManager;
import com.android.internal.telephony.MccTable;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.RIL;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.ProxyController;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyPermissions;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.IccRecords;
// MTK-START: SIM
import com.android.phone.PhoneInterfaceManager;
import com.android.internal.telephony.uicc.IccIoResult;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.mediatek.internal.telephony.imsphone.MtkImsPhone;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.MtkProxyController;
import com.mediatek.telephony.MtkTelephonyManagerEx;
import com.mediatek.internal.telephony.uicc.MtkUiccController;
import com.mediatek.internal.telephony.uicc.MtkUiccProfile;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.IccCard;
// MTK-END
import com.mediatek.internal.telephony.dataconnection.MtkDcTracker;
import com.mediatek.internal.telephony.FemtoCellInfo;

import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.MtkRILConstants;
import com.mediatek.internal.telephony.PseudoCellInfo;
import com.mediatek.internal.telephony.phb.CsimPhbUtil;
import com.mediatek.internal.telephony.phb.MtkIccPhoneBookInterfaceManager;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.RadioManager;
import com.mediatek.internal.telephony.selfactivation.ISelfActivation;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;
import java.util.Map;

/**
 * Implementation of the IMtkTelephonyEx interface.
 */
public class MtkPhoneInterfaceManagerEx extends IMtkTelephonyEx.Stub {
    protected static final String LOG_TAG = "MtkPhoneIntfMgrEx";
    protected static final boolean DBG = true;//(PhoneGlobals.DBG_LEVEL >= 2);
    protected static final boolean DBG_LOC = false;
    protected static final boolean DBG_MERGE = false;

    // MTK-START: SIM
    private static final int CMD_EXCHANGE_SIM_IO_EX = 102;
    private static final int EVENT_EXCHANGE_SIM_IO_EX_DONE = 103;
    private static final int CMD_GET_ATR = 104;
    private static final int EVENT_GET_ATR_DONE = 105;
    private static final int CMD_LOAD_EF_TRANSPARENT = 108;
    private static final int EVENT_LOAD_EF_TRANSPARENT_DONE = 109;
    private static final int CMD_LOAD_EF_LINEARFIXEDALL = 110;
    private static final int EVENT_LOAD_EF_LINEARFIXEDALL_DONE = 111;
    private static final int CMD_INVOKE_OEM_RIL_REQUEST_RAW = 112;
    private static final int EVENT_INVOKE_OEM_RIL_REQUEST_RAW_DONE = 113;
    private static final int CMD_SET_SIM_POWER = 114;
    private static final int EVENT_SET_SIM_POWER_DONE = 115;

    private static final int COMMAND_READ_BINARY = 0xb0;
    private static final int COMMAND_READ_RECORD = 0xb2;
    private final Object mSimPowerLock = new Object();
    // MTK-END

    // M: [LTE][Low Power][UL traffic shaping] @{
    private static final int CMD_SET_LTE_ACCESS_STRATUM_STATE = 35;
    private static final int EVENT_SET_LTE_ACCESS_STRATUM_STATE_DONE = 36;
    private static final int CMD_SET_LTE_UPLINK_DATA_TRANSFER_STATE = 37;
    private static final int EVENT_SET_LTE_UPLINK_DATA_TRANSFER_STATE_DONE = 38;
    // M: [LTE][Low Power][UL traffic shaping] @}

    // MTK-START: NW
    private static final int CMD_GET_ALL_CELL_INFO = 60;
    private static final int EVENT_GET_ALL_CELL_INFO_DONE = 61;
    private static final int CMD_SET_DISABLE_2G = 62;
    private static final int EVENT_SET_DISABLE_2G_DONE = 63;
    private static final int CMD_GET_DISABLE_2G = 64;
    private static final int EVENT_GET_DISABLE_2G_DONE = 65;
    private static final int CMD_GET_FEMTOCELL_LIST = 66;
    private static final int EVENT_GET_FEMTOCELL_LIST_DONE = 67;
    private static final int CMD_ABORT_FEMTOCELL_LIST = 68;
    private static final int EVENT_ABORT_FEMTOCELL_LIST_DONE = 69;
    private static final int CMD_SELECT_FEMTOCELL = 70;
    private static final int EVENT_SELECT_FEMTOCELL_DONE = 71;
    private static final int CMD_QUERY_FEMTOCELL_SELECTION_MODE = 72;
    private static final int EVENT_QUERY_FEMTOCELL_SELECTION_MODE_DONE = 73;
    private static final int CMD_SET_FEMTOCELL_SELECTION_MODE = 74;
    private static final int EVENT_SET_FEMTOCELL_SELECTION_MODE_DONE = 75;
    private static final int CMD_CANCEL_AVAILABLE_NETWORK = 76;
    private static final int EVENT_CANCEL_AVAILABLE_NETWORK_DONE = 77;
    private static final int CMD_GET_SUGGESTED_PLMN_LIST = 78;
    private static final int EVENT_GET_SUGGESTED_PLMN_LIST_DONE = 79;

    private static final int CMD_SET_ROAMING_ENABLE = 1000;
    private static final int EVENT_SET_ROAMING_ENABLE_DONE = 1001;
    private static final int CMD_GET_ROAMING_ENABLE = 1002;
    private static final int EVENT_GET_ROAMING_ENABLE_DONE = 1003;
    // MTK-END

    /** The singleton instance. */
    private static MtkPhoneInterfaceManagerEx sInstance;

    // Query SIM phonebook Adn stroage info thread
    private QueryAdnInfoThread mAdnInfoThread = null;
    private final Object mAdnInfoLock = new Object();

    private PhoneGlobals mApp;
    private CallManager mCM;
    private UserManager mUserManager;
    private AppOpsManager mAppOps;
    protected MainThreadHandler mMainThreadHandler;
    private SubscriptionController mSubscriptionController;
    private SharedPreferences mTelephonySharedPreferences;

    private boolean mIsLastEccIms;

    // MTK-START: SIM GBA
    // SIM authenthication thread
    private SimAuth mSimAuthThread = null;
    // MTK-END
    // MTK-START: SIM
    private static final String[] PROPERTY_RIL_TEST_SIM = {
         "vendor.gsm.sim.ril.testsim",
         "vendor.gsm.sim.ril.testsim.2",
         "vendor.gsm.sim.ril.testsim.3",
         "vendor.gsm.sim.ril.testsim.4",
     };

    /**
     * The property is used to get supported card type of each SIM card in the slot.
     * @hide
     */
    private static final String[] PROPERTY_RIL_FULL_UICC_TYPE  = {
        "vendor.gsm.ril.fulluicctype",
        "vendor.gsm.ril.fulluicctype.2",
        "vendor.gsm.ril.fulluicctype.3",
        "vendor.gsm.ril.fulluicctype.4",
    };
    // MTK-END

    // [SIM-C2K] @{
    /**
     * The property shows uim imsi that is only for cdma card.
     */
    private static final String[] PROPERTY_UIM_SUBSCRIBER_ID = {
        "vendor.ril.uim.subscriberid.1",
        "vendor.ril.uim.subscriberid.2",
        "vendor.ril.uim.subscriberid.3",
        "vendor.ril.uim.subscriberid.4",
    };

    /**
     * The property is used to check if the card is cdma 3G dual mode card in the slot.
     * @hide
     */
     private static final String[] PROPERTY_RIL_CT3G = {
         "vendor.gsm.ril.ct3g",
         "vendor.gsm.ril.ct3g.2",
         "vendor.gsm.ril.ct3g.3",
         "vendor.gsm.ril.ct3g.4",
     };

    /**
     * The property record the card's ICC ID.
     */
    private String[] PROPERTY_ICCID_SIM = {
        "vendor.ril.iccid.sim1",
        "vendor.ril.iccid.sim2",
        "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4",
    };
    // [SIM-C2K] @}

    private String[] PROPERTY_GSM_MCC_MNC = {
        "vendor.gsm.ril.uicc.mccmnc",
        "vendor.gsm.ril.uicc.mccmnc.1",
        "vendor.gsm.ril.uicc.mccmnc.2",
        "vendor.gsm.ril.uicc.mccmnc.3",
    };

    private String[] PROPERTY_CDMA_MCC_MNC = {
        "vendor.cdma.ril.uicc.mccmnc",
        "vendor.cdma.ril.uicc.mccmnc.1",
        "vendor.cdma.ril.uicc.mccmnc.2",
        "vendor.cdma.ril.uicc.mccmnc.3",
    };

    // Exit ECBM mode
    private static final int EVENT_EXIT_ECBM_MODE_REQ = 44;

    // RX Test EVENT
    private static final int EVENT_SET_RX_TEST_CONFIG = 42;
    private static final int EVENT_GET_RX_TEST_RESULT = 43;

    private static final String[] ICCRECORD_PROPERTY_ICCID = {
        "vendor.ril.iccid.sim1",
        "vendor.ril.iccid.sim2",
        "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4",
    };
     // MTK-START: SIM
     /**
      * A request object to use for transmitting data to an ICC.
      */
     public static final class MtkIccAPDUArgument {
         public int channel, cla, command, p1, p2, p3;
         public String data;
         public String pathId;
         public String pin2;
         public int slotId;
         public int family;

         public MtkIccAPDUArgument(int channel, int cla, int command,
                 int p1, int p2, int p3, String data) {
             //super(channel, cla, command, p1, p2, p3, data);
             int slot = SubscriptionManager
                     .getSlotIndex(SubscriptionManager.getDefaultSubscriptionId());
             if (DBG) log("MtkIccAPDUArgument, default slot " + slot);
             this.channel = channel;
             this.cla = cla;
             this.command = command;
             this.p1 = p1;
             this.p2 = p2;
             this.p3 = p3;
             this.pathId = null;
             this.data = data;
             this.pin2 = null;
             this.slotId = slot;
         }

         public MtkIccAPDUArgument(int slotId, int channel, int cla, int command,
                 int p1, int p2, int p3, String pathId) {
             //super(channel, cla, command, p1, p2, p3, null);
             this.channel = channel;
             this.cla = cla;
             this.command = command;
             this.p1 = p1;
             this.p2 = p2;
             this.p3 = p3;
             this.pathId = pathId;
             this.data = null;
             this.pin2 = null;
             this.slotId = slotId;
         }

         public MtkIccAPDUArgument(int slotId, int family, int channel, int cla,
                 int command, int p1, int p2, int p3, String pathId) {
             //super(channel, cla, command, p1, p2, p3, null);
             this.channel = channel;
             this.cla = cla;
             this.command = command;
             this.p1 = p1;
             this.p2 = p2;
             this.p3 = p3;
             this.pathId = pathId;
             this.data = null;
             this.pin2 = null;
             this.slotId = slotId;
             this.family = family;
         }

         public MtkIccAPDUArgument(int channel, int cla, int command,
                 int p1, int p2, int p3, String data, String pathId, String pin2) {
             //super(channel, cla, command, p1, p2, p3, data);
             int slot = SubscriptionManager
                     .getSlotIndex(SubscriptionManager.getDefaultSubscriptionId());
             this.channel = channel;
             this.cla = cla;
             this.command = command;
             this.p1 = p1;
             this.p2 = p2;
             this.p3 = p3;
             this.pathId = pathId;
             this.data = data;
             this.pin2 = pin2;
             this.slotId = slotId;
         }
     }
     // MTK-END
    /**
     * A request object for use with {@link MainThreadHandler}. Requesters should wait() on the
     * request after sending. The main thread will notify the request when it is complete.
     */
    private static final class MainThreadRequest {
        /** The argument to use for the request */
        public Object argument;
        /** The result of the request that is run on the main thread */
        public Object result;
        // The subscriber id that this request applies to. Defaults to
        // SubscriptionManager.INVALID_SUBSCRIPTION_ID
        public Integer subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;

        // In cases where subId is unavailable, the caller needs to specify the phone.
        public Phone phone;

        public WorkSource workSource;

        public MainThreadRequest(Object argument) {
            this.argument = argument;
        }

        MainThreadRequest(Object argument, Phone phone, WorkSource workSource) {
            this.argument = argument;
            if (phone != null) {
                this.phone = phone;
            }
            this.workSource = workSource;
        }

        MainThreadRequest(Object argument, Integer subId, WorkSource workSource) {
            this.argument = argument;
            if (subId != null) {
                this.subId = subId;
            }
            this.workSource = workSource;
        }

    }

    /**
     * Class to listens for Emergency Callback Mode state change intents
     */
    private class EcmExitReceiver extends BroadcastReceiver {
        private int mSubId;
        private MainThreadRequest mRequest;

        /**
         * Create a broadcast receiver to listen ECBM exit.
         * @param subId the subscription ID
         * @param request the request to th main thread
         */
        public EcmExitReceiver(int subId, MainThreadRequest request) {
            mSubId = subId;
            mRequest = request;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(
                    TelephonyIntents.ACTION_EMERGENCY_CALLBACK_MODE_CHANGED)) {
                if (intent.getBooleanExtra(PhoneConstants.PHONE_IN_ECM_STATE, false) == false) {
                    if (intent.getExtras().getInt(PhoneConstants.SUBSCRIPTION_KEY) == mSubId) {
                        mRequest.result = new EcmExitResult(this);
                        synchronized (mRequest) {
                            mRequest.notifyAll();
                        }
                    }
                }
            }
        }

    }

    /**
     * Class to record the result to exiting ECBM mode.
     */
    private class EcmExitResult {
        private EcmExitReceiver mReceiver;
        /**
         * Create a ECBM exit result object.
         * @param receiver the broadcast receiver
         */
        public EcmExitResult(EcmExitReceiver receiver) {
            mReceiver = receiver;
        }

        /**
         * Get the broadcast receiver instance.
         * @return the broadcast receiver
         */
        public EcmExitReceiver getReceiver() {
            return mReceiver;
        }
    }

    /**
     * A handler that processes messages on the main thread in the phone process. Since many
     * of the Phone calls are not thread safe this is needed to shuttle the requests from the
     * inbound binder threads to the main thread in the phone process.  The Binder thread
     * may provide a {@link MainThreadRequest} object in the msg.obj field that they are waiting
     * on, which will be notified when the operation completes and will contain the result of the
     * request.
     *
     * <p>If a MainThreadRequest object is provided in the msg.obj field,
     * note that request.result must be set to something non-null for the calling thread to
     * unblock.
     */
    public class MainThreadHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            MainThreadRequest request;
            Message onCompleted;
            AsyncResult ar;
            IccCard iccCard;
            MtkIccAPDUArgument iccArgument;
            IccFileHandler fh;
            Phone phone = null;

            log("MainThreadHandler.handleMessage : " + msg.what);
            switch (msg.what) {
              // MTK-START: SIM
              case CMD_LOAD_EF_TRANSPARENT:
                  request = (MainThreadRequest) msg.obj;
                  iccArgument = (MtkIccAPDUArgument) request.argument;
                  log("CMD_LOAD_EF_TRANSPARENT: slot " + iccArgument.slotId);

                  fh = UiccController.getInstance().getIccFileHandler(
                          iccArgument.slotId, iccArgument.family);
                  if (fh == null) {
                      loge("loadEFTransparent: No UICC");
                      request.result = new AsyncResult(null, null, null);
                      synchronized (request) {
                          request.notifyAll();
                      }
                  } else {
                      onCompleted = obtainMessage(EVENT_LOAD_EF_TRANSPARENT_DONE,
                              request);
                      fh.loadEFTransparent(iccArgument.cla, iccArgument.pathId,
                              onCompleted);
                  }
                  break;

              case EVENT_LOAD_EF_TRANSPARENT_DONE:
                  log("EVENT_LOAD_EF_TRANSPARENT_DONE");
                  ar = (AsyncResult) msg.obj;
                  request = (MainThreadRequest) ar.userObj;
                  if (ar.exception == null && ar.result != null) {
                      request.result = new AsyncResult(null, (byte [])ar.result, null);
                  } else {
                      request.result = new AsyncResult(null, null, null);
                  }
                  synchronized (request) {
                      request.notifyAll();
                  }
                  break;

              case CMD_LOAD_EF_LINEARFIXEDALL:
                  request = (MainThreadRequest) msg.obj;
                  iccArgument = (MtkIccAPDUArgument) request.argument;
                  log("CMD_LOAD_EF_LINEARFIXEDALL: slot " + iccArgument.slotId);

                  fh = UiccController.getInstance().getIccFileHandler(
                          iccArgument.slotId, iccArgument.family);
                  if (fh == null) {
                      loge("loadEFLinearFixedAll: No UICC");
                      request.result = new AsyncResult(null, null, null);
                      synchronized (request) {
                          request.notifyAll();
                      }
                  } else {
                      onCompleted = obtainMessage(EVENT_LOAD_EF_LINEARFIXEDALL_DONE,
                              request);
                      fh.loadEFLinearFixedAll(iccArgument.cla, iccArgument.pathId,
                              onCompleted);
                  }
                  break;

              case EVENT_LOAD_EF_LINEARFIXEDALL_DONE:
                  log("EVENT_LOAD_EF_LINEARFIXEDALL_DONE");
                  ar = (AsyncResult) msg.obj;
                  request = (MainThreadRequest) ar.userObj;
                  if (ar.exception == null && ar.result != null) {
                      request.result = new AsyncResult(null, (ArrayList<byte[]>)ar.result, null);
                  } else {
                      request.result = new AsyncResult(null, null, null);
                  }
                  synchronized (request) {
                      request.notifyAll();
                  }
                  break;

              case CMD_SET_SIM_POWER:
                  request = (MainThreadRequest) msg.obj;
                  int simState = ((Integer) request.argument).intValue();
                  log("CMD_SET_SIM_POWER: slotId = " + request.subId + " simState = " + simState);

                  onCompleted = obtainMessage(EVENT_SET_SIM_POWER_DONE, request);
                  ((MtkUiccController) UiccController.getInstance()).setSimPower(request.subId,
                        simState, onCompleted);
                  break;

              case EVENT_SET_SIM_POWER_DONE:
                   ar = (AsyncResult) msg.obj;
                   request = (MainThreadRequest) ar.userObj;

                   log("EVENT_SET_SIM_POWER_DONE ar.exception: " + ar.exception + " ar.result: "
                            + ar.result);
                   if (ar.exception == null && ar.result != null) {
                       request.result = ar.result;
                   } else {
                       request.result = 0;
                   }
                   synchronized (request) {
                       request.notifyAll();
                   }
                   break;

              case CMD_SET_DISABLE_2G: {
                  request = (MainThreadRequest) msg.obj;
                  boolean enabled = ((Boolean) request.argument).booleanValue();
                  log("CMD_SET_DISABLE_2G:" + " mode = " + enabled);
                  onCompleted = obtainMessage(EVENT_SET_DISABLE_2G_DONE, request);
                  ((MtkGsmCdmaPhone) request.phone).setDisable2G(enabled, onCompleted);
                  break;
              }
              case EVENT_SET_DISABLE_2G_DONE:
                  ar = (AsyncResult) msg.obj;
                  request = (MainThreadRequest) ar.userObj;
                  log("EVENT_SET_DISABLE_2G_DONE ar.exception: " + ar.exception);
                  // no need to check ar.result because it returns empty.
                  if (ar.exception == null) {
                      request.result = true;
                  } else {
                      request.result = false;
                  }
                  synchronized (request) {
                      request.notifyAll();
                  }
                  break;

              case CMD_GET_DISABLE_2G:
                  request = (MainThreadRequest) msg.obj;
                  log("CMD_GET_DISABLE_2G");
                  onCompleted = obtainMessage(EVENT_GET_DISABLE_2G_DONE, request);
                  ((MtkGsmCdmaPhone) request.phone).getDisable2G(onCompleted);
                  break;

              case EVENT_GET_DISABLE_2G_DONE:
                  ar = (AsyncResult) msg.obj;
                  request = (MainThreadRequest) ar.userObj;
                  log("EVENT_GET_DISABLE_2G_DONE ar.exception: " + ar.exception + " ar.result: "
                          + ar.result);
                  if (ar.exception == null && ar.result != null) {
                      request.result = ar.result;
                  } else {
                      request.result = -1;
                  }
                  synchronized (request) {
                      request.notifyAll();
                  }
                  break;

              case CMD_GET_FEMTOCELL_LIST:
                  request = (MainThreadRequest) msg.obj;
                  log("CMD_GET_FEMTOCELL_LIST");
                  onCompleted = obtainMessage(EVENT_GET_FEMTOCELL_LIST_DONE, request);
                  ((MtkGsmCdmaPhone) request.phone).getFemtoCellList(onCompleted);
                  break;

              case EVENT_GET_FEMTOCELL_LIST_DONE:
                  ar = (AsyncResult) msg.obj;
                  request = (MainThreadRequest) ar.userObj;
                  log("EVENT_GET_FEMTOCELL_LIST_DONE ar.exception: " + ar.exception + " ar.result: "
                          + ar.result);
                  request.result = (ar.exception == null && ar.result != null)
                          ? ar.result : null;
                  synchronized (request) {
                      request.notifyAll();
                  }
                  break;

              case CMD_ABORT_FEMTOCELL_LIST:
                  request = (MainThreadRequest) msg.obj;
                  log("CMD_ABORT_FEMTOCELL_LIST");
                  onCompleted = obtainMessage(EVENT_ABORT_FEMTOCELL_LIST_DONE, request);
                  ((MtkGsmCdmaPhone) request.phone).abortFemtoCellList(onCompleted);
                  break;

              case EVENT_ABORT_FEMTOCELL_LIST_DONE:
                  ar = (AsyncResult) msg.obj;
                  request = (MainThreadRequest) ar.userObj;
                  log("EVENT_ABORT_FEMTOCELL_LIST_DONE ar.exception: " + ar.exception + " ar.result: "
                          + ar.result);
                  if (ar.exception == null && ar.result != null) {
                      request.result = true;
                  } else {
                      request.result = false;
                  }
                  synchronized (request) {
                      request.notifyAll();
                  }
                  break;

              case CMD_SELECT_FEMTOCELL:
                  request = (MainThreadRequest) msg.obj;
                  log("CMD_SELECT_FEMTOCELL");
                  onCompleted = obtainMessage(EVENT_SELECT_FEMTOCELL_DONE, request);
                  ((MtkGsmCdmaPhone) request.phone).selectFemtoCell((FemtoCellInfo) request.argument, onCompleted);
                  break;

              case EVENT_SELECT_FEMTOCELL_DONE:
                  ar = (AsyncResult) msg.obj;
                  request = (MainThreadRequest) ar.userObj;
                  log("EVENT_SELECT_FEMTOCELL_DONE ar.exception: " + ar.exception + " ar.result: "
                          + ar.result);
                  if (ar.exception == null && ar.result != null) {
                      request.result = true;
                  } else {
                      request.result = false;
                  }
                  synchronized (request) {
                      request.notifyAll();
                  }
                  break;

              case CMD_QUERY_FEMTOCELL_SELECTION_MODE:
                  request = (MainThreadRequest) msg.obj;
                  log("CMD_QUERY_FEMTOCELL_SELECTION_MODE");
                  onCompleted = obtainMessage(EVENT_QUERY_FEMTOCELL_SELECTION_MODE_DONE, request);
                  ((MtkGsmCdmaPhone) request.phone).queryFemtoCellSystemSelectionMode(onCompleted);
                  break;

              case EVENT_QUERY_FEMTOCELL_SELECTION_MODE_DONE:
                  ar = (AsyncResult) msg.obj;
                  request = (MainThreadRequest) ar.userObj;
                  log("EVENT_QUERY_FEMTOCELL_SELECTION_MODE_DONE ar.exception: " + ar.exception + " ar.result: "
                          + ar.result);
                  if (ar.exception == null && ar.result != null) {
                      request.result = ar.result;
                  } else {
                      request.result = -1;
                  }
                  synchronized (request) {
                      request.notifyAll();
                  }
                  break;

              case CMD_SET_FEMTOCELL_SELECTION_MODE:
                  request = (MainThreadRequest) msg.obj;
                  Integer mode = ((Integer) request.argument).intValue();
                  log("CMD_SET_FEMTOCELL_SELECTION_MODE");
                  onCompleted = obtainMessage(EVENT_SET_FEMTOCELL_SELECTION_MODE_DONE, request);
                  ((MtkGsmCdmaPhone) request.phone).setFemtoCellSystemSelectionMode(mode, onCompleted);
                  break;

              case EVENT_SET_FEMTOCELL_SELECTION_MODE_DONE:
                  ar = (AsyncResult) msg.obj;
                  request = (MainThreadRequest) ar.userObj;

                  log("EVENT_SET_FEMTOCELL_SELECTION_MODE_DONE ar.exception: " + ar.exception + " ar.result: "
                          + ar.result);
                  if (ar.exception == null && ar.result != null) {
                      request.result = true;
                  } else {
                      request.result = false;
                  }
                  synchronized (request) {
                      request.notifyAll();
                  }
                  break;

              case CMD_CANCEL_AVAILABLE_NETWORK:
                  request = (MainThreadRequest) msg.obj;
                  log("CMD_CANCEL_AVAILABLE_NETWORK");
                  onCompleted = obtainMessage(EVENT_CANCEL_AVAILABLE_NETWORK_DONE, request);
                  ((MtkGsmCdmaPhone) request.phone).cancelAvailableNetworks(onCompleted);
                  break;

              case EVENT_CANCEL_AVAILABLE_NETWORK_DONE:
                  ar = (AsyncResult) msg.obj;
                  request = (MainThreadRequest) ar.userObj;
                  log("EVENT_CANCEL_AVAILABLE_NETWORK_DONE ar.exception: " + ar.exception + " ar.result: "
                          + ar.result);
                  if (ar.exception == null && ar.result != null) {
                      request.result = true;
                  } else {
                      request.result = false;
                  }
                  synchronized (request) {
                      request.notifyAll();
                  }
                  break;

              case CMD_EXCHANGE_SIM_IO_EX:
                  request = (MainThreadRequest) msg.obj;
                  iccArgument = (MtkIccAPDUArgument) request.argument;
                  iccCard = getUiccCardFromRequest(request);
                  if (iccCard == null) {
                      loge("iccExchangeSimIOExUsingSlot: No UICC");
                      request.result = new IccIoResult(0x6F, 0, (byte[]) null);
                      synchronized (request) {
                          request.notifyAll();
                      }
                  } else {
                      onCompleted = obtainMessage(EVENT_EXCHANGE_SIM_IO_EX_DONE,
                              request);
                      iccCard.iccExchangeSimIOEx(iccArgument.cla, /* cla is fileID here! */
                              iccArgument.command, iccArgument.p1, iccArgument.p2, iccArgument.p3,
                              iccArgument.pathId, iccArgument.data,
                              iccArgument.pin2, onCompleted);
                  }
                  break;

             case EVENT_EXCHANGE_SIM_IO_EX_DONE:
                  ar = (AsyncResult) msg.obj;
                  request = (MainThreadRequest) ar.userObj;

                  log("EVENT_EXCHANGE_SIM_IO_EX_DONE");
                  if (ar.exception == null && ar.result != null) {
                      request.result = ar.result;
                  } else {
                      request.result = new IccIoResult(0x6f, 0, (byte[]) null);
                      if (ar.result == null) {
                          loge("iccExchangeSimIOExUsingSlot: Empty response");
                      } else if (ar.exception != null && ar.exception instanceof CommandException) {
                          loge("iccExchangeSimIOExUsingSlot: CommandException: " +
                                  ar.exception);
                      } else {
                          loge("iccExchangeSimIOExUsingSlot: Unknown exception");
                      }
                  }

                  synchronized (request) {
                      request.notifyAll();
                  }
                  break;

              case CMD_GET_ATR:
                  request = (MainThreadRequest) msg.obj;
                  iccArgument = (MtkIccAPDUArgument) request.argument;
                  iccCard = getUiccCardFromRequest(request);

                  if (iccCard == null) {
                      loge("get ATR: No UICC");
                      request.result = "";
                      synchronized (request) {
                          request.notifyAll();
                      }
                  } else {
                      onCompleted = obtainMessage(EVENT_GET_ATR_DONE, request);
                      iccCard.iccGetAtr(onCompleted);
                  }
                  break;

              case EVENT_GET_ATR_DONE:
                  ar = (AsyncResult) msg.obj;
                  request = (MainThreadRequest) ar.userObj;
                  if (ar.exception == null) {
                      log("EVENT_GET_ATR_DONE, no exception");
                      request.result = ar.result;
                  } else {
                      loge("EVENT_GET_ATR_DONE, exception happens");
                      request.result = "";
                  }
                  synchronized (request) {
                      request.notifyAll();
                  }
                  break;
              // M: [LTE][Low Power][UL traffic shaping] @{
              case CMD_SET_LTE_ACCESS_STRATUM_STATE: {
                  request = (MainThreadRequest) msg.obj;
                  boolean enabled = ((Boolean) request.argument).booleanValue();
                  if (DBG) {
                      log("CMD_SET_LTE_ACCESS_STRATUM_STATE: enabled " + enabled
                              + "subId" + request.subId);
                  }
                  phone = getPhoneFromRequest(request);
                  if (phone == null) {
                      loge("setLteAccessStratumReport: No MainPhone");
                      request.result = new Boolean(false);
                      synchronized (request) {
                          request.notifyAll();
                      }
                  } else {
                      MtkDcTracker dcTracker = (MtkDcTracker) phone.getDcTracker(
                              AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
                      onCompleted = obtainMessage(EVENT_SET_LTE_ACCESS_STRATUM_STATE_DONE,
                              request);
                      dcTracker.onSetLteAccessStratumReport((Boolean) enabled, onCompleted);
                  }
                  break;
              }
              case EVENT_SET_LTE_ACCESS_STRATUM_STATE_DONE:
                  if (DBG) log("EVENT_SET_LTE_ACCESS_STRATUM_STATE_DONE");
                  handleNullReturnEvent(msg, "setLteAccessStratumReport");
                  break;

              case CMD_SET_LTE_UPLINK_DATA_TRANSFER_STATE: {
                  request = (MainThreadRequest) msg.obj;
                  int state = ((Integer) request.argument).intValue();
                  if (DBG) {
                      log("CMD_SET_LTE_UPLINK_DATA_TRANSFER_STATE: state " + state
                              + "subId " + request.subId);
                  }
                  phone = getPhoneFromRequest(request);
                  if (phone == null) {
                      loge("setLteUplinkDataTransfer: No MainPhone");
                      request.result = new Boolean(false);
                      synchronized (request) {
                          request.notifyAll();
                      }
                  } else {
                      MtkDcTracker dcTracker = (MtkDcTracker) phone.getDcTracker(
                              AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
                      onCompleted = obtainMessage(EVENT_SET_LTE_UPLINK_DATA_TRANSFER_STATE_DONE,
                              request);
                      dcTracker.onSetLteUplinkDataTransfer((Integer) state, onCompleted);
                  }
                  break;
              }
              case EVENT_SET_LTE_UPLINK_DATA_TRANSFER_STATE_DONE:
                  if (DBG) log("EVENT_SET_LTE_UPLINK_DATA_TRANSFER_STATE_DONE");
                  handleNullReturnEvent(msg, "setLteUplinkDataTransfer");
                  break;
              // M: [LTE][Low Power][UL traffic shaping] @}
              // MTK-END

                case EVENT_SET_RX_TEST_CONFIG:
                case EVENT_GET_RX_TEST_RESULT:
                    if (DBG) log("handle RX_TEST");
                    ar = (AsyncResult) msg.obj;
                    RxTestObject rt = (RxTestObject) ar.userObj;
                    synchronized(rt.lockObj) {
                        if (ar.exception != null) {
                            log("RX_TEST: error ret null, e=" + ar.exception);
                            rt.result = null;
                        } else {
                            rt.result = (int[]) ar.result;
                        }
                        rt.lockObj.notify();
                        if (DBG) log("RX_TEST notify result");
                    }
                    break;

              case EVENT_EXIT_ECBM_MODE_REQ:
                  request = (MainThreadRequest) msg.obj;
                  Integer subId = (Integer) request.argument;
                  if (getPhone(subId).isInEcm()) {
                      IntentFilter filter = new IntentFilter();
                      filter.addAction(TelephonyIntents.ACTION_EMERGENCY_CALLBACK_MODE_CHANGED);
                      EcmExitReceiver receiver = new EcmExitReceiver(subId, request);
                      log("Exit ECBM mode receiver " + receiver);
                      mApp.registerReceiver(receiver, filter);
                      getPhone(subId).exitEmergencyCallbackMode();
                  } else {
                      request.result = new EcmExitResult(null);
                      synchronized (request) {
                          request.notifyAll();
                      }
                  }
                  break;

                case CMD_INVOKE_OEM_RIL_REQUEST_RAW:
                    request = (MainThreadRequest)msg.obj;
                    if (request.subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                        phone = getPhone(request.subId);
                    } else {
                        Log.i(LOG_TAG, "get main phone for instead");
                        phone = PhoneFactory.getPhone(getMainCapabilityPhoneId());
                    }
                    if (phone == null) {
                        Log.e(LOG_TAG, "MainThreadHandler: phone == null ");
                    } else {
                        onCompleted = obtainMessage(EVENT_INVOKE_OEM_RIL_REQUEST_RAW_DONE, request);
                        ((MtkGsmCdmaPhone)phone).invokeOemRilRequestRaw((byte[])request.argument,
                                onCompleted);
                    }
                    break;

                case EVENT_INVOKE_OEM_RIL_REQUEST_RAW_DONE:
                    ar = (AsyncResult)msg.obj;
                    request = (MainThreadRequest)ar.userObj;
                    request.result = ar;
                    synchronized (request) {
                        request.notifyAll();
                    }
                    break;

                case CMD_GET_ALL_CELL_INFO:
                    request = (MainThreadRequest) msg.obj;
                    onCompleted = obtainMessage(EVENT_GET_ALL_CELL_INFO_DONE, request);
                    request.phone.requestCellInfoUpdate(request.workSource, onCompleted);
                    break;

                case EVENT_GET_ALL_CELL_INFO_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (MainThreadRequest) ar.userObj;
                    // If a timeout occurs, the response will be null
                    request.result = (ar.exception == null && ar.result != null)
                            ? ar.result : new ArrayList<CellInfo>();
                    synchronized (request) {
                        request.notifyAll();
                    }
                    break;

                case CMD_SET_ROAMING_ENABLE:
                    request = (MainThreadRequest) msg.obj;
                    int[] config = (int[]) request.argument;
                    onCompleted = obtainMessage(EVENT_SET_ROAMING_ENABLE_DONE, request);
                    ((MtkGsmCdmaPhone) request.phone).setRoamingEnable(config, onCompleted);
                    break;

                case EVENT_SET_ROAMING_ENABLE_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (MainThreadRequest) ar.userObj;
                    request.result = ar.exception == null;
                    synchronized (request) {
                        request.notifyAll();
                    }
                    break;

                case CMD_GET_ROAMING_ENABLE:
                    request = (MainThreadRequest) msg.obj;
                    onCompleted = obtainMessage(EVENT_GET_ROAMING_ENABLE_DONE, request);
                    ((MtkGsmCdmaPhone) request.phone).getRoamingEnable(onCompleted);
                    break;

                case EVENT_GET_ROAMING_ENABLE_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (MainThreadRequest) ar.userObj;
                    request.result = (ar.exception == null && ar.result != null) ? ar.result : null;
                    synchronized (request) {
                        request.notifyAll();
                    }
                    break;

                case CMD_GET_SUGGESTED_PLMN_LIST:
                    request = (MainThreadRequest) msg.obj;
                    SuggestedPlmnListRequest splr = (SuggestedPlmnListRequest) request.argument;
                    onCompleted = obtainMessage(EVENT_GET_SUGGESTED_PLMN_LIST_DONE, request);
                    ((MtkGsmCdmaPhone) request.phone).getSuggestedPlmnList
                            (splr.rat, splr.num, splr.timer, onCompleted);
                    break;

                case EVENT_GET_SUGGESTED_PLMN_LIST_DONE:
                    ar = (AsyncResult) msg.obj;
                    request = (MainThreadRequest) ar.userObj;
                    request.result = (ar.exception == null && ar.result != null) ? ar.result : new String[0];
                    synchronized (request) {
                        request.notifyAll();
                    }
                    break;

                default:
                    Log.w(LOG_TAG, "MainThreadHandler: unexpected message code: " + msg.what);
                    break;
            }
        }

        private void handleNullReturnEvent(Message msg, String command) {
            AsyncResult ar = (AsyncResult) msg.obj;
            MainThreadRequest request = (MainThreadRequest) ar.userObj;
            if (ar.exception == null) {
                request.result = true;
            } else {
                request.result = false;
                if (ar.exception instanceof CommandException) {
                    loge(command + ": CommandException: " + ar.exception);
                } else {
                    loge(command + ": Unknown exception");
                }
            }
            synchronized (request) {
                request.notifyAll();
            }
        }
    }

    /**
     * Posts the specified command to be executed on the main thread,
     * waits for the request to complete, and returns the result.
     * @see #sendRequestAsync
     */
    private Object sendRequest(int command, Object argument) {
        return sendRequest(
                command, argument, SubscriptionManager.INVALID_SUBSCRIPTION_ID, null, null);
    }

    /**
     * Posts the specified command to be executed on the main thread,
     * waits for the request to complete, and returns the result.
     * @see #sendRequestAsync
     */
    private Object sendRequest(int command, Object argument, WorkSource workSource) {
        return sendRequest(command, argument,  SubscriptionManager.INVALID_SUBSCRIPTION_ID,
                null, workSource);
    }

    /**
     * Posts the specified command to be executed on the main thread,
     * waits for the request to complete, and returns the result.
     * @see #sendRequestAsync
     */
    private Object sendRequest(int command, Object argument, Integer subId) {
        return sendRequest(command, argument, subId, null, null);
    }

    /**
     * Posts the specified command to be executed on the main thread,
     * waits for the request to complete, and returns the result.
     * @see #sendRequestAsync
     */
    private Object sendRequest(int command, Object argument, int subId, WorkSource workSource) {
        return sendRequest(command, argument, subId, null, workSource);
    }

    /**
     * Posts the specified command to be executed on the main thread,
     * waits for the request to complete, and returns the result.
     * @see #sendRequestAsync
     */
    private Object sendRequest(int command, Object argument, Phone phone, WorkSource workSource) {
        return sendRequest(
                command, argument, SubscriptionManager.INVALID_SUBSCRIPTION_ID, phone, workSource);
    }


    /**
     * Posts the specified command to be executed on the main thread,
     * waits for the request to complete, and returns the result.
     * @see #sendRequestAsync
     */
    private Object sendRequest(
            int command, Object argument, Integer subId, Phone phone, WorkSource workSource) {
        if (Looper.myLooper() == mMainThreadHandler.getLooper()) {
            throw new RuntimeException("This method will deadlock if called from the main thread.");
        }

        MainThreadRequest request = null;
        if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID && phone != null) {
            throw new IllegalArgumentException("subId and phone cannot both be specified!");
        } else if (phone != null) {
            request = new MainThreadRequest(argument, phone, workSource);
        } else {
            request = new MainThreadRequest(argument, subId, workSource);
        }

        Message msg = mMainThreadHandler.obtainMessage(command, request);
        msg.sendToTarget();

        // Wait for the request to complete
        synchronized (request) {
            while (request.result == null) {
                try {
                    request.wait();
                } catch (InterruptedException e) {
                    // Do nothing, go back and wait until the request is complete
                }
            }
        }
        return request.result;
    }


    /**
     * Asynchronous ("fire and forget") version of sendRequest():
     * Posts the specified command to be executed on the main thread, and
     * returns immediately.
     * @see #sendRequest
     */
    private void sendRequestAsync(int command) {
        mMainThreadHandler.sendEmptyMessage(command);
    }

    /**
     * Same as {@link #sendRequestAsync(int)} except it takes an argument.
     * @see {@link #sendRequest(int)}
     */
    private void sendRequestAsync(int command, Object argument) {
        sendRequestAsync(command, argument, null, null);
    }

    /**
     * Same as {@link #sendRequestAsync(int,Object)} except it takes a Phone and WorkSource.
     * @see {@link #sendRequest(int,Object)}
     */
    private void sendRequestAsync(
            int command, Object argument, Phone phone, WorkSource workSource) {
        MainThreadRequest request = new MainThreadRequest(argument, phone, workSource);
        Message msg = mMainThreadHandler.obtainMessage(command, request);
        msg.sendToTarget();
    }


    /**
     * Initialize the singleton PhoneInterfaceManager instance.
     * This is only done once, at startup, from PhoneApp.onCreate().
     */
    public static MtkPhoneInterfaceManagerEx init(PhoneGlobals app) {
        synchronized (MtkPhoneInterfaceManagerEx.class) {
            if (sInstance == null) {
                sInstance = new MtkPhoneInterfaceManagerEx(app);
            } else {
                Log.wtf(LOG_TAG, "init() called multiple times!  sInstance = " + sInstance);
            }
            return sInstance;
        }
    }

    /** Private constructor; @see init() */
    protected MtkPhoneInterfaceManagerEx(PhoneGlobals app) {
        mApp = app;
        //mCM = PhoneGlobals.getInstance().mCM;
        mUserManager = (UserManager) app.getSystemService(Context.USER_SERVICE);
        mAppOps = (AppOpsManager)app.getSystemService(Context.APP_OPS_SERVICE);
        mMainThreadHandler = new MainThreadHandler();
        mTelephonySharedPreferences =
                PreferenceManager.getDefaultSharedPreferences(mApp);
        mSubscriptionController = SubscriptionController.getInstance();

        publish();
    }

    private void publish() {
        if (DBG) log("publish: " + this);

        ServiceManager.addService("phoneEx", this);
    }

    private Phone getPhoneFromRequest(MainThreadRequest request) {
        return (request.subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID)
                ? getDefaultPhone() : getPhone(request.subId);
    }

    private Phone getDefaultPhone() {
        Phone thePhone = getPhone(getDefaultSubscription());
        return (thePhone != null) ? thePhone : PhoneFactory.getDefaultPhone();
    }

    // returns phone associated with the subId.
    private static Phone getPhone(int subId) {
        int phoneId = SubscriptionController.getInstance().getPhoneId(subId);
        return PhoneFactory.getPhone(
                ((phoneId < 0) ? SubscriptionManager.DEFAULT_PHONE_INDEX : phoneId));
    }

    protected static void log(String msg) {
        Log.e(LOG_TAG, msg);
    }

    protected static void logv(String msg) {
        Log.e(LOG_TAG, msg);
    }

    protected static void loge(String msg) {
        Log.e(LOG_TAG, msg);
    }

    // MTK-START: SIM ME LOCK
    private class UnlockSim extends Thread {

        /* Query network lock start */
        // Fail, need retry
        public static final int FAIL_RETRY = -1;
        // Fail, needn't retry
        public static final int FAIL_RETRY_NO_NEED = -2;
        // Fail, needn't retry, modem return error
        public static final int FAIL_MD = -3;

        // API function result
        private static final int CALL_ERROR = 1;

        // Verify network lock result.
        public static final int VERIFY_RESULT_PASS = 0;
        public static final int VERIFY_INCORRECT_PASSWORD = 1;
        public static final int VERIFY_RESULT_EXCEPTION = 2;

        // Total network lock count.
        public static final int NETWORK_LOCK_TOTAL_COUNT = 5;
        public static final String QUERY_SIMME_LOCK_RESULT =
                "com.mediatek.phone.QUERY_SIMME_LOCK_RESULT";
        public static final String SIMME_LOCK_LEFT_COUNT =
                "com.mediatek.phone.SIMME_LOCK_LEFT_COUNT";

        /* Query network lock end */

        private MtkUiccProfile mSimCard = null;
        private boolean mDone = false;
        private boolean mResult = false;

        // For replies from SimCard interface
        private Handler mHandler;

        private static final int QUERY_NETWORK_STATUS_COMPLETE = 100;
        private static final int SET_NETWORK_LOCK_COMPLETE = 101;
        private static final int SET_DEVICE_NETWORK_LOCK_COMPLETE = 102;
        private static final int GET_SIM_LOCK_STATE_FOR_RSU_COMPLETE = 103;

        private int mRsuLockState = FAIL_RETRY;

        private int mVerifyResult = -1;
        private int mSIMMELockRetryCount = -1;

        private int mRetryCount = -1;
        private int mUnlockResult = MtkIccCardConstants.SML_SLOT_LOCK_POLICY_UNLOCK_NO_LOCK_POLICY;

        public UnlockSim(IccCard simCard) {
            if (simCard instanceof MtkUiccProfile) {
                mSimCard = (MtkUiccProfile)simCard;
            } else {
                log("UnlockSim: Not MtkUiccProfile instance.");
            }
        }

        public UnlockSim() {
        }
        @Override
        public void run() {
            Looper.prepare();
            synchronized (UnlockSim.this) {
                mHandler = new Handler() {
                    @Override
                    public void handleMessage(Message msg) {
                        AsyncResult ar = (AsyncResult) msg.obj;
                        switch (msg.what) {
                            case QUERY_NETWORK_STATUS_COMPLETE:
                                synchronized (UnlockSim.this) {
                                    int [] LockState = (int []) ar.result;
                                    if (ar.exception != null) { //Query exception occurs
                                        log("Query network lock fail");
                                        mResult = false;
                                    } else {
                                        mSIMMELockRetryCount = LockState[2];
                                        log("[SIMQUERY] Category = " + LockState[0]
                                            + " ,Network status =" + LockState[1]
                                            + " ,Retry count = " + LockState[2]);

                                         mResult = true;
                                    }
                                    mDone = true;
                                    UnlockSim.this.notifyAll();
                                }
                                break;
                            case SET_NETWORK_LOCK_COMPLETE:
                                log("SUPPLY_NETWORK_LOCK_COMPLETE");
                                synchronized (UnlockSim.this) {
                                    if ((ar.exception != null) &&
                                           (ar.exception instanceof CommandException)) {
                                        log("ar.exception " + ar.exception);
                                        if (((CommandException) ar.exception).getCommandError()
                                            == CommandException.Error.PASSWORD_INCORRECT) {
                                            mVerifyResult = VERIFY_INCORRECT_PASSWORD;
                                       } else {
                                            mVerifyResult = VERIFY_RESULT_EXCEPTION;
                                       }
                                    } else {
                                        mVerifyResult = VERIFY_RESULT_PASS;
                                    }
                                    mDone = true;
                                    UnlockSim.this.notifyAll();
                                }
                                break;
                            case SET_DEVICE_NETWORK_LOCK_COMPLETE:
                                log("SET_DEVICE_NETWORK_LOCK_COMPLETE");
                                synchronized (UnlockSim.this) {
                                    mRetryCount = msg.arg1;
                                    if (ar.exception != null) {
                                        if (ar.exception instanceof CommandException &&
                                                ((CommandException)(ar.exception)).getCommandError()
                                                == CommandException.Error.PASSWORD_INCORRECT) {
                                            mUnlockResult = MtkIccCardConstants
                                                    .SML_SLOT_LOCK_POLICY_UNLOCK_INCORRECT_PASSWORD;
                                        } else {
                                            mUnlockResult = MtkIccCardConstants
                                                    .SML_SLOT_LOCK_POLICY_UNLOCK_GENERAL_FAIL;
                                        }
                                    } else {
                                        mUnlockResult = MtkIccCardConstants
                                                .SML_SLOT_LOCK_POLICY_UNLOCK_SUCCESS;
                                    }
                                    mDone = true;
                                    UnlockSim.this.notifyAll();
                                }
                                break;
                            case GET_SIM_LOCK_STATE_FOR_RSU_COMPLETE:
                                synchronized (UnlockSim.this) {
                                    if (ar.exception != null) { // Exception occurs
                                        log("[RSU-SIMLOCK] get RSU lock state fail");
                                        mRsuLockState = FAIL_MD;
                                    } else {
                                        mRsuLockState = (int) ar.result;
                                        log("[RSU-SIMLOCK] mRsuLockState = " + mRsuLockState);
                                    }
                                    mDone = true;
                                    UnlockSim.this.notifyAll();
                                }
                                break;
                        }
                    }
                };
                UnlockSim.this.notifyAll();
            }
            Looper.loop();
        }

        synchronized Bundle queryNetworkLock(int category) {

            while (mHandler == null) {
                try {
                    wait();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            }

            log("Enter queryNetworkLock");
            Message callback = Message.obtain(mHandler, QUERY_NETWORK_STATUS_COMPLETE);
            mSimCard.queryIccNetworkLock(category, callback);

            while (!mDone) {
                try {
                    log("wait for done");
                    wait();
                } catch (InterruptedException e) {
                    // Restore the interrupted status
                    Thread.currentThread().interrupt();
                }
            }
            try {
                mHandler.getLooper().quit();
                if (mHandler.getLooper().getThread() != null) {
                    mHandler.getLooper().getThread().interrupt();
                }
            } catch (NullPointerException ne) {
                loge("queryNetworkLock Null looper");
                ne.printStackTrace();
            }

            Bundle bundle = new Bundle();
            bundle.putBoolean(QUERY_SIMME_LOCK_RESULT, mResult);
            bundle.putInt(SIMME_LOCK_LEFT_COUNT, mSIMMELockRetryCount);

            log("done");
            return bundle;
        }

        synchronized int supplyNetworkLock(String strPasswd) {

            while (mHandler == null) {
                try {
                    wait();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            }

            log("Enter supplyNetworkLock");
            Message callback = Message.obtain(mHandler, SET_NETWORK_LOCK_COMPLETE);
            mSimCard.supplyNetworkDepersonalization(strPasswd, callback);

            while (!mDone) {
                try {
                    log("wait for done");
                    wait();
                } catch (InterruptedException e) {
                    // Restore the interrupted status
                    Thread.currentThread().interrupt();
                }
            }
            try {
                mHandler.getLooper().quit();
                if (mHandler.getLooper().getThread() != null) {
                    mHandler.getLooper().getThread().interrupt();
                }
            } catch (NullPointerException ne) {
                loge("supplyNetworkLock Null looper");
                ne.printStackTrace();
            }

            log("done");
            return mVerifyResult;
        }

        /*
         * Use password to unlock the device
         * pwd: the input password
         */
        synchronized int[] supplyDeviceNetworkLock(String pwd) {
            while (mHandler == null) {
                try {
                    wait();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            }
            Message callback = Message.obtain(mHandler, SET_DEVICE_NETWORK_LOCK_COMPLETE);

            ((MtkUiccController)UiccController.getInstance())
                    .supplyDeviceNetworkDepersonalization(pwd, callback);

            while (!mDone) {
                try {
                    Log.d(LOG_TAG, "wait for done");
                    wait();
                } catch (InterruptedException e) {
                    // Restore the interrupted status
                    Thread.currentThread().interrupt();
                }
            }
            int[] resultArray = new int[2];
            resultArray[0] = mUnlockResult;
            resultArray[1] = mRetryCount;
            log("done, mUnlockResult = " + mUnlockResult + ", mRetryCount = " + mRetryCount);
            return resultArray;
        }

        synchronized int getSimLockStateForRSU(int phoneId) {
            while (mHandler == null) {
                try {
                    wait();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            }

            log("[RSU-SIMLOCK] Enter getSimLockStateForRSU");
            Message callback = Message.obtain(mHandler, GET_SIM_LOCK_STATE_FOR_RSU_COMPLETE);
            MtkUiccController ctrl = (MtkUiccController) UiccController.getInstance();
            if (ctrl != null) {
                try {
                    int r = ctrl.getRsuSml().remoteSimlockGetSimlockStatusEx(callback);
                    if (r == CALL_ERROR) {
                        log("[RSU-SIMLOCK] getSimLockStateForRSU, API error!");
                        return FAIL_RETRY_NO_NEED;
                    }
                } catch (NullPointerException e) {
                    loge("[RSU-SIMLOCK] getSimLockStateForRSU, No RSU instance!");
                    return FAIL_RETRY_NO_NEED;
                }
            } else {
                log("[RSU-SIMLOCK] getSimLockStateForRSU, MtkUiccController not start now!");
                return FAIL_RETRY;
            }

            while (!mDone) {
                try {
                    log("[RSU-SIMLOCK] getSimLockStateForRSU, wait for done");
                    wait();
                } catch (InterruptedException e) {
                    // Restore the interrupted status
                    Thread.currentThread().interrupt();
                }
            }
            try {
                mHandler.getLooper().quit();
                if (mHandler.getLooper().getThread() != null) {
                    mHandler.getLooper().getThread().interrupt();
                }
            } catch (NullPointerException ne) {
                loge("[RSU-SIMLOCK] getSimLockStateForRSU Null looper");
                ne.printStackTrace();
            }

            log("[RSU-SIMLOCK] getSimLockStateForRSU, done");
            return mRsuLockState;
        }
    }

    public Bundle queryNetworkLock(int subId, int category) {
        final UnlockSim queryNetworkLockState;

        log("queryNetworkLock");

        queryNetworkLockState = new UnlockSim(getPhone(subId).getIccCard());
        // MTK-START: should confirm with keygurad can handle null Bundle.
        if (queryNetworkLockState.mSimCard == null) {
            return null;
        }
        // MTK-END
        queryNetworkLockState.start();

        return queryNetworkLockState.queryNetworkLock(category);
    }

    public int supplyNetworkDepersonalization(int subId, String strPasswd) {
        final UnlockSim supplyNetworkLock;

        log("supplyNetworkDepersonalization");

        supplyNetworkLock = new UnlockSim(getPhone(subId).getIccCard());
        if (supplyNetworkLock.mSimCard == null) {
            return supplyNetworkLock.VERIFY_RESULT_EXCEPTION;
        }
        supplyNetworkLock.start();

        return supplyNetworkLock.supplyNetworkLock(strPasswd);
    }

    private static final String PROPERTY_SML_MODE = "ro.vendor.sim_me_lock_mode";

    private boolean mIsSmlLockMode = SystemProperties.get(PROPERTY_SML_MODE, "").equals("3");

    /**
     * unlock device lock
     * @param pwd the input password
     * @return
     * when SIM_ME_LOCK_MODE == 3 will return
     *     result: SML_SLOT_LOCK_POLICY_UNLOCK_UNKNOWN = -1, left number = -1
     *     result: SML_SLOT_LOCK_POLICY_UNLOCK_NO_LOCK_POLICY = 0, left number = -1
     *     result: SML_SLOT_LOCK_POLICY_UNLOCK_INCORRECT_PASSWORD = 1, left number = mRetryCount
     *     result: SML_SLOT_LOCK_POLICY_UNLOCK_GENERAL_FAIL = 2, left number = mRetryCount
     *     result: SML_SLOT_LOCK_POLICY_UNLOCK_SUCCESS = 3, left number = mRetryCount
     * When SIM_ME_LOCK_MODE != 3 will only return
     *     result: SML_SLOT_LOCK_POLICY_UNLOCK_NO_LOCK_POLICY = 0, left number = -1
     */
    public int[] supplyDeviceNetworkDepersonalization(String pwd) {
        log("supplyDeviceNetworkDepersonalization");
        int[] resultArray = new int[2];
        if (TextUtils.isEmpty(pwd) || pwd.length() < 1) {
            resultArray[0] = MtkIccCardConstants.SML_SLOT_LOCK_POLICY_UNLOCK_UNKNOWN;
            resultArray[1] = -1;
            return resultArray;
        }
        if (mIsSmlLockMode) {
            int simLockPolicy = MtkTelephonyManagerEx.getDefault().getSimLockPolicy();
            if (simLockPolicy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_UNKNOWN) {
                resultArray[0] = MtkIccCardConstants.SML_SLOT_LOCK_POLICY_UNLOCK_UNKNOWN;
                resultArray[1] = -1;
                return resultArray;
            } else if (simLockPolicy == MtkIccCardConstants.SML_SLOT_LOCK_POLICY_NONE) {
                resultArray[0] = MtkIccCardConstants.SML_SLOT_LOCK_POLICY_UNLOCK_NO_LOCK_POLICY;
                resultArray[1] = -1;
                return resultArray;
            }
        } else {
            resultArray[0] = MtkIccCardConstants.SML_SLOT_LOCK_POLICY_UNLOCK_NO_LOCK_POLICY;
            resultArray[1] = -1;
            return resultArray;
        }

        final UnlockSim supplyDeviceNetworkLock = new UnlockSim();
        supplyDeviceNetworkLock.start();
        return supplyDeviceNetworkLock.supplyDeviceNetworkLock(pwd);
    }

    /**
     * To get RSU lock state. This state is from Blob. this API is expected to be used by all RSU
     *
     * @param phoneId: Indicate which slot to query
     *
     * @return RSU lock state in blob or error
     *
     */
    public int getSimLockStateForRSU(int phoneId) {
        final UnlockSim getRSULockState;

        log("[RSU-SIMLOCK] getSimLockStateForRSU");

        getRSULockState = new UnlockSim();
        getRSULockState.start();

        return getRSULockState.getSimLockStateForRSU(phoneId);
    }
    // MTK-END

    // MTK-START: CMCC DUAL SIM DEPENDENCY LOCK
    /**
     * Modem SML change feature.
     * This function will query the SIM state of the given slot. And broadcast
     * ACTION_UNLOCK_SIM_LOCK if the SIM state is in network lock.
     *
     * @param subId: Indicate which sub to query
     * @param needIntent: The caller can deside to broadcast ACTION_UNLOCK_SIM_LOCK or not
     *                    in this time, because some APs will receive this intent (eg. Keyguard).
     *                    That can avoid this intent to effect other AP.
     */
    public void repollIccStateForNetworkLock(int subId, boolean needIntent) {
        if (TelephonyManager.getDefault().getPhoneCount() > 1) {
            int phoneId = SubscriptionController.getInstance().getPhoneId(subId);
            if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
                log("[repollIccStateForNetworkLock] phoneId: " + phoneId);
                    return;
            }

            MtkUiccController ctrl = (MtkUiccController) UiccController.getInstance();
            if (ctrl != null) {
                ctrl.repollIccStateForModemSmlChangeFeatrue(phoneId, needIntent);
            }
        } else {
            log("Not Support in Single SIM.");
        }
    }
    // MTK-END

    /// M: [Network][C2K] Add isInHomeNetwork interface. @{
    @Override
    public boolean isInHomeNetwork(int subId) {
         final int phoneId = SubscriptionManager.getPhoneId(subId);
         boolean isInHomeNetwork = false;
         final Phone phone = PhoneFactory.getPhone(phoneId);
         if (phone != null) {
             ServiceState serviceState = phone.getServiceState();
             if (serviceState != null) {
                 isInHomeNetwork = inSameCountry(phoneId, serviceState.getVoiceOperatorNumeric());
             }
         }
         log("isInHomeNetwork, subId=" + subId + " ,phoneId=" + phoneId
                 + " ,isInHomeNetwork=" + isInHomeNetwork);
         return isInHomeNetwork;
     }

     /**
      * Check ISO country by MCC to see if phone is roaming in same registered country.
      *
      * @param phoneId for which phone inSameCountry is returned
      * @param operatorNumeric registered operator numeric
      * @return true if in same country.
      */
     private static final boolean inSameCountry(int phoneId, String operatorNumeric) {
         if (TextUtils.isEmpty(operatorNumeric) || (operatorNumeric.length() < 5)
                 || (!TextUtils.isDigitsOnly(operatorNumeric))) {
             // Not a valid network
             log("inSameCountry, Not a valid network"
                     + ", phoneId=" + phoneId + ", operatorNumeric=" + operatorNumeric);
             return true;
         }

         final String homeNumeric = getHomeOperatorNumeric(phoneId);
         if (TextUtils.isEmpty(homeNumeric) || (homeNumeric.length() < 5)
                 || (!TextUtils.isDigitsOnly(homeNumeric))) {
             // Not a valid SIM MCC
             log("inSameCountry, Not a valid SIM MCC"
                     + ", phoneId=" + phoneId + ", homeNumeric=" + homeNumeric);
             return true;
         }

         boolean inSameCountry = true;
         final String networkMCC = operatorNumeric.substring(0, 3);
         final String homeMCC = homeNumeric.substring(0, 3);
         final String networkCountry = MccTable.countryCodeForMcc(Integer.parseInt(networkMCC));
         final String homeCountry = MccTable.countryCodeForMcc(Integer.parseInt(homeMCC));
         log("inSameCountry, phoneId=" + phoneId
                 + ", homeMCC=" + homeMCC
                 + ", networkMCC=" + networkMCC
                 + ", homeCountry=" + homeCountry
                 + ", networkCountry=" + networkCountry);
         if (networkCountry.isEmpty() || homeCountry.isEmpty()) {
             // Not a valid country
             return true;
         }
         inSameCountry = homeCountry.equals(networkCountry);
         if (inSameCountry) {
             return inSameCountry;
         }
         // special same country cases
         if ("us".equals(homeCountry) && "vi".equals(networkCountry)) {
             inSameCountry = true;
         } else if ("vi".equals(homeCountry) && "us".equals(networkCountry)) {
             inSameCountry = true;
         } else if ("cn".equals(homeCountry) && "mo".equals(networkCountry)) {
             inSameCountry = true;
         }

         log("inSameCountry, phoneId=" + phoneId + ", inSameCountry=" + inSameCountry);
         return inSameCountry;
     }

     /**
      * Returns the Service Provider Name (SPN).
      *
      * @param phoneId for which Home Operator Numeric is returned
      * @return the Service Provider Name (SPN)
      */
     private static final String getHomeOperatorNumeric(int phoneId) {
         String numeric = TelephonyManager.getDefault().getSimOperatorNumericForPhone(phoneId);
         if (TextUtils.isEmpty(numeric)) {
             numeric = SystemProperties.get("ro.cdma.home.operator.numeric", "");
         }

         // For CT 3G special case
         MtkTelephonyManagerEx telEx = MtkTelephonyManagerEx.getDefault();
         boolean isCt3gDualMode = telEx.isCt3gDualMode(phoneId);
         if (isCt3gDualMode && "20404".equals(numeric)) {
             numeric = "46003";
         }

         log("getHomeOperatorNumeric, phoneId=" + phoneId + ", numeric=" + numeric);
         return numeric;
     }
     /// @}

    @Override
    public Bundle getCellLocationUsingSlotId(int slotId) {
        enforceFineOrCoarseLocationPermission("getCellLocationUsingSlotId");

        if (DBG_LOC) log("getCellLocationUsingSlotId: is active user");
        Bundle data = new Bundle();
        int subId = getSubIdBySlot(slotId);
        Phone phone = getPhone(subId);
        if (phone == null) {
            return null;
        }

        phone.getCellLocation().fillInNotifierBundle(data);
        return data;
    }

    private void enforceFineOrCoarseLocationPermission(String message) {
        try {
            mApp.enforceCallingOrSelfPermission(
                    android.Manifest.permission.ACCESS_FINE_LOCATION, null);
        } catch (SecurityException e) {
            // If we have ACCESS_FINE_LOCATION permission, skip the check for ACCESS_COARSE_LOCATION
            // A failure should throw the SecurityException from ACCESS_COARSE_LOCATION since this
            // is the weaker precondition
            mApp.enforceCallingOrSelfPermission(
                    android.Manifest.permission.ACCESS_COARSE_LOCATION, message);
        }
    }

    private static boolean checkIfCallerIsSelfOrForegroundUser() {
        boolean ok;

        boolean self = Binder.getCallingUid() == Process.myUid();
        if (!self) {
            // Get the caller's user id then clear the calling identity
            // which will be restored in the finally clause.
            int callingUser = UserHandle.getCallingUserId();
            long ident = Binder.clearCallingIdentity();

            try {
                // With calling identity cleared the current user is the foreground user.
                int foregroundUser = ActivityManager.getCurrentUser();
                ok = (foregroundUser == callingUser);
                if (DBG_LOC) {
                    log("checkIfCallerIsSelfOrForegoundUser: foregroundUser=" + foregroundUser
                            + " callingUser=" + callingUser + " ok=" + ok);
                }
            } catch (NullPointerException ex) {
                if (DBG_LOC) {
                    loge("checkIfCallerIsSelfOrForegoundUser: Exception ex=" + ex);
                }
                ok = false;
            } finally {
                Binder.restoreCallingIdentity(ident);
            }
        } else {
            if (DBG_LOC) {
                log("checkIfCallerIsSelfOrForegoundUser: is self");
            }
            ok = true;
        }
        if (DBG_LOC) {
            log("checkIfCallerIsSelfOrForegoundUser: ret=" + ok);
        }
        return ok;
    }

    // MTK-START: SIM
    // Added by M start
    private int getSubIdBySlot(int slot) {
        int [] subId = SubscriptionManager.getSubId(slot);
        if (subId == null || subId.length == 0) {
            return getDefaultSubscription();
        }
        if (DBG) log("getSubIdBySlot, simId " + slot + "subId " + subId[0]);
        return subId[0];
    }

    @Override
    public String getIccAtr(int subId) {
        enforceModifyPermissionOrCarrierPrivilege(subId);

        if (DBG) log("> getIccAtr " + ", subId = " + subId);
        String response = (String) sendRequest(CMD_GET_ATR, null, subId);
        if (DBG) log("< getIccAtr: " + response);
        return response;
    }

    @Override
    public byte[] iccExchangeSimIOEx(int subId, int fileID, int command,
            int p1, int p2, int p3, String filePath, String data, String pin2) {
        enforceModifyPermissionOrCarrierPrivilege(subId);

        if (DBG) log("Exchange SIM_IO Ex " + fileID + ":" + command + " " +
                 p1 + " " + p2 + " " + p3 + ":" + filePath + ", " + data + ", " + pin2 +
                 ", subId = " + subId);

        IccIoResult response =
                (IccIoResult) sendRequest(CMD_EXCHANGE_SIM_IO_EX,
                        new MtkIccAPDUArgument(-1, fileID, command,
                        p1, p2, p3, data, filePath, pin2), subId);

        if (DBG) log("Exchange SIM_IO Ex [R]" + response);
        byte[] result = null; int length = 2;
        if (response.payload != null) {
            length = 2 + response.payload.length;
            result = new byte[length];
            System.arraycopy(response.payload, 0, result, 0, response.payload.length);
        } else result = new byte[length];

        if (DBG) log("Exchange SIM_IO Ex [L] " + length);
        result[length - 1] = (byte) response.sw2;
        result[length - 2] = (byte) response.sw1;
        return result;
    }

    @Override
    /**
     * Returns the response APDU for a command APDU sent through SIM_IO.
     *
     * <p>Requires Permission:
     *   {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}
     * Or the calling app has carrier privileges. @see #hasCarrierPrivileges.
     *
     * @param slotId
     * @param family
     * @param fileID
     * @param filePath
     * @return The APDU response
     */
    public byte[] loadEFTransparent(int slotId, int family, int fileID, String filePath) {
        enforceModifyPermissionOrCarrierPrivilege(getSubIdBySlot(slotId));

        if (DBG) {
            log("loadEFTransparent slot " + slotId + " " + family + " " + fileID + ":" + filePath);
        }

        AsyncResult ar = (AsyncResult)  sendRequest(CMD_LOAD_EF_TRANSPARENT,
                new MtkIccAPDUArgument(slotId, family, -1, fileID, COMMAND_READ_BINARY, 0, 0, 0,
                filePath));
        byte[] response = (byte[])ar.result;
        if (DBG) {
            log("loadEFTransparent " + response);
        }
        return response;
    }

    @Override
    /**
     * Returns the response APDU for a command APDU sent through SIM_IO.
     *
     * <p>Requires Permission:
     *   {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}
     * Or the calling app has carrier privileges. @see #hasCarrierPrivileges.
     *
     * @param slotId
     * @param family
     * @param fileID
     * @param filePath
     * @return The APDU response
     */
    public List<String> loadEFLinearFixedAll(int slotId, int family, int fileID,
            String filePath) {
        enforceModifyPermissionOrCarrierPrivilege(getSubIdBySlot(slotId));

        if (DBG) {
            log("loadEFLinearFixedAll slot " + slotId + " " + family + " " + fileID + ":"
                    + filePath);
        }
        AsyncResult ar = (AsyncResult) sendRequest(CMD_LOAD_EF_LINEARFIXEDALL,
                new MtkIccAPDUArgument(slotId, family, -1, fileID, COMMAND_READ_RECORD, 0, 0, 0,
                filePath));
        ArrayList<byte[]> result = (ArrayList<byte[]>) ar.result;
        if (result == null) {
            log("loadEFLinearFixedAll return null");
            return null;
        }
        List<String> response = new ArrayList<String>();
        for (int i = 0 ; i < result.size(); i++) {
            if (result.get(i) == null) {
                continue;
            }
            String res = IccUtils.bytesToHexString(result.get(i));
            response.add(res);
        }
        if (DBG) {
            log("loadEFLinearFixedAll " + response);
        }
        return response;
    }

    /**
     * Set SIM power state.
     *
     * @param slotId SIM slot id
     * @param state 1: SIM ON; 0: SIMOFF
     * @return -1: SET_SIM_POWER_ERROR_NOT_SUPPORT, 0: SET_SIM_POWER_SUCCESS,
     * 54: SET_SIM_POWER_ERROR_NOT_ALLOWED, 11: SET_SIM_POWER_ERROR_SIM_ABSENT,
     * 12: SET_SIM_POWER_ERROR_EXECUTING_SIM_OFF, 13: SET_SIM_POWER_ERROR_EXECUTING_SIM_ON
     * 14: SET_SIM_POWER_ERROR_ALREADY_SIM_OFF, 15: SET_SIM_POWER_ERROR_ALREADY_SIM_ON
     * <p>Requires Permission:
     *   {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}
     * Or the calling app has carrier privileges. @see #hasCarrierPrivileges.
     *
     * @hide
     **/
    @Override
    public int setSimPower(int slotId, int state) {
        if (slotId < 0 || slotId >= TelephonyManager.getDefault().getSimCount()) {
            log("setSimPower error with invalid slotId:" + slotId);
            return -1;
        }
        enforceModifyPermissionOrCarrierPrivilege(getSubIdBySlot(slotId));

        log("setSimPower slotId = " + slotId + " state = " + state);
        if (!(SystemProperties.get(MtkTelephonyManagerEx.PROPERTY_SIM_ONOFF_SUPPORT).
                equals("1"))) {
            return MtkTelephonyManagerEx.SET_SIM_POWER_ERROR_NOT_SUPPORT;
        }

        synchronized (mSimPowerLock) {
            if (getSimOnOffExecutingState(slotId) == state) {
                if (state == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON) {
                    return MtkTelephonyManagerEx.SET_SIM_POWER_ERROR_EXECUTING_SIM_ON;
                } else if (state == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_OFF) {
                    return MtkTelephonyManagerEx.SET_SIM_POWER_ERROR_EXECUTING_SIM_OFF;
                }
            } else if (getSimOnOffExecutingState(slotId) == -1 &&
                    getSimOnOffState(slotId) == state) {
                if (state == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON) {
                    return MtkTelephonyManagerEx.SET_SIM_POWER_ERROR_ALREADY_SIM_ON;
                } else if (state == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_OFF) {
                    return MtkTelephonyManagerEx.SET_SIM_POWER_ERROR_ALREADY_SIM_OFF;
                }
            } else {
                ((MtkUiccController) UiccController.getInstance())
                .setSimOnOffExecutingState(slotId, state);
            }
        }

        int response = (int) sendRequest(CMD_SET_SIM_POWER, new Integer(state),
                new Integer(slotId));
        log("setSimPower response = " + response);
        return response;
    }

    /**
     * Get SIM on/off state.
     * @param slotId SIM slot id
     * @return -1: unknown, SIM_POWER_STATE_SIM_OFF: SIM off, SIM_POWER_STATE_SIM_OFF: SIM on.
     */
    public int getSimOnOffState(int slotId) {
        if (slotId < 0 || slotId >= TelephonyManager.getDefault().getSimCount()) {
            log("getSimOnOffState error with invalid slotId:" + slotId);
            return -1;
        }
        return ((MtkUiccController) UiccController.getInstance()).getSimOnOffState(slotId);
    }

    /**
     * Check whether SIM is in process of on/off state.
     * @param slotId SIM slot id
     * @return -1: not in executing, SIM_POWER_STATE_EXECUTING_SIM_ON: in process of on state,
     * SIM_POWER_STATE_EXECUTING_SIM_OFF: in process of off state
     */
    public int getSimOnOffExecutingState(int slotId) {
        if (slotId < 0 || slotId >= TelephonyManager.getDefault().getSimCount()) {
            log("getSimOnOffExecutingState error with invalid slotId:" + slotId);
            return -1;
        }
        return ((MtkUiccController) UiccController.getInstance()).getSimOnOffExecutingState(slotId);
    }

    public String getIccCardType(int subId) {
        if (DBG) log("getIccCardType  subId=" + subId);

        Phone phone = getPhone(subId);
        if (phone == null) {
            if (DBG) log("getIccCardType(): phone is null");
            return "";
        }
        return (phone.getIccCard()).getIccCardType();
    }

    public boolean isAppTypeSupported(int slotId, int appType) {
        if (DBG) log("isAppTypeSupported  slotId=" + slotId);

        UiccCard uiccCard = UiccController.getInstance().getUiccCard(slotId);
        if (uiccCard == null) {
            if (DBG) log("isAppTypeSupported(): uiccCard is null");
            return false;
        }

        return ((uiccCard.getApplicationByType(appType) == null) ?  false : true);
    }

    public boolean isTestIccCard(int slotId) {
        String mTestCard = null;

        mTestCard = SystemProperties.get(PROPERTY_RIL_TEST_SIM[slotId], "");
        if (DBG) log("isTestIccCard(): slot id =" + slotId + ", iccType = " + mTestCard);
        return (mTestCard != null && mTestCard.equals("1"));
    }

    /**
     * Get icc app family by slot id.
     * @param slotId slot id
     * @return the family type
     * @hide
     */
    public int getIccAppFamily(int slotId) {
        int iccType = MtkTelephonyManagerEx.APP_FAM_NONE;
        int phoneCount = TelephonyManager.getDefault().getSimCount();
        if (slotId < 0 || slotId >= phoneCount) {
            log("getIccAppFamily, invalid slotId:" + slotId);
            return iccType;
        }

        String uiccType = SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[slotId]);
        String appType[] = uiccType.split(",");
        int fullType = MtkTelephonyManagerEx.CARD_TYPE_NONE;
        for (int i = 0; i < appType.length; i++) {
            if ("USIM".equals(appType[i])) {
                fullType = fullType | MtkTelephonyManagerEx.CARD_TYPE_USIM;
            } else if ("SIM".equals(appType[i])) {
                fullType = fullType | MtkTelephonyManagerEx.CARD_TYPE_SIM;
            } else if ("CSIM".equals(appType[i])) {
                fullType = fullType | MtkTelephonyManagerEx.CARD_TYPE_CSIM;
            } else if ("RUIM".equals(appType[i])) {
                fullType = fullType | MtkTelephonyManagerEx.CARD_TYPE_RUIM;
            }
        }

        if (fullType == MtkTelephonyManagerEx.CARD_TYPE_NONE) {
            iccType = MtkTelephonyManagerEx.APP_FAM_NONE;
        } else if ((fullType & MtkTelephonyManagerEx.CARD_TYPE_CSIM) != 0
                && (fullType & MtkTelephonyManagerEx.CARD_TYPE_USIM) != 0) {
            iccType = MtkTelephonyManagerEx.APP_FAM_3GPP2 | MtkTelephonyManagerEx.APP_FAM_3GPP;
        } else if ((fullType & MtkTelephonyManagerEx.CARD_TYPE_CSIM) != 0
                || (fullType & MtkTelephonyManagerEx.CARD_TYPE_RUIM) != 0) {
            iccType = MtkTelephonyManagerEx.APP_FAM_3GPP2;
        } else {
            iccType = MtkTelephonyManagerEx.APP_FAM_3GPP;

            // Uim dual mode sim, may switch to SIM type for use
            if (fullType == MtkTelephonyManagerEx.CARD_TYPE_SIM) {
                String uimDualMode = SystemProperties.get(PROPERTY_RIL_CT3G[slotId]);
                if ("1".equals(uimDualMode)) {
                    iccType = MtkTelephonyManagerEx.APP_FAM_3GPP2;
                }
            }

        }
        log("getIccAppFamily, " + "uiccType[" + slotId + "] = "
                    + uiccType + "fullType = " + fullType + " iccType = " + iccType);
        return iccType;
    }

    /**
     * Make sure either system app or the caller has carrier privilege.
     *
     * @throws SecurityException if the caller does not have the required permission/privilege
     */
    private void enforceModifyPermissionOrCarrierPrivilege(int subId) {
        int permission = mApp.checkCallingOrSelfPermission(
                android.Manifest.permission.MODIFY_PHONE_STATE);
        if (permission == PackageManager.PERMISSION_GRANTED) {
            return;
        }

        log("No modify permission, check carrier privilege next.");
        enforceCarrierPrivilege(subId);
    }

    /**
     * Make sure the caller has the READ_PRIVILEGED_PHONE_STATE permission.
     *
     * @throws SecurityException if the caller does not have the required permission
     */
    private void enforcePrivilegedPhoneStatePermission() {
        mApp.enforceCallingOrSelfPermission(
                android.Manifest.permission.READ_PRIVILEGED_PHONE_STATE, null);
    }

    /**
     * Make sure the caller has carrier privilege.
     *
     * @throws SecurityException if the caller does not have the required permission
     */
    private void enforceCarrierPrivilege(int subId) {
        if (getCarrierPrivilegeStatus(subId) !=
                    TelephonyManager.CARRIER_PRIVILEGE_STATUS_HAS_ACCESS) {
            loge("No Carrier Privilege.");
            throw new SecurityException("No Carrier Privilege.");
        }
    }

    private int getCarrierPrivilegeStatus(int subId) {
        final Phone phone = getPhone(subId);
        if (phone == null) {
            loge("getCarrierPrivilegeStatus: Invalid subId");
            return TelephonyManager.CARRIER_PRIVILEGE_STATUS_NO_ACCESS;
        }
        UiccCard card = UiccController.getInstance().getUiccCard(phone.getPhoneId());
        if (card == null) {
            loge("getCarrierPrivilegeStatus: No UICC");
            return TelephonyManager.CARRIER_PRIVILEGE_STATUS_RULES_NOT_LOADED;
        }
        return card.getCarrierPrivilegeStatusForCurrentTransaction(
                phone.getContext().getPackageManager());
    }

    private IccCard getUiccCardFromRequest(MainThreadRequest request) {
        Phone phone = getPhoneFromRequest(request);
        return (phone == null) ? null : phone.getIccCard();
    }

    private int getDefaultSubscription() {
        return mSubscriptionController.getDefaultSubId();
    }

    // MTK-START: MVNO
    public String getMvnoMatchType(int subId) {
        String type = ((MtkGsmCdmaPhone)getPhone(subId)).getMvnoMatchType();
        if (DBG) log("getMvnoMatchType sub = " + subId + " ,type = " + type);
        return type;
    }

    public String getMvnoPattern(int subId, String type) {
        String pattern = ((MtkGsmCdmaPhone)getPhone(subId)).getMvnoPattern(type);
        if (DBG) log("getMvnoPattern sub = " + subId + " ,pattern = " + pattern);
        return pattern;
    }
    // MTK-END

    /**
     * Query if the radio is turned off by user.
     *
     * @param subId inidicated subscription
     *
     * @return true radio is turned off by user.
     *         false radio isn't turned off by user.
     *
     */
    public boolean isRadioOffBySimManagement(int subId) {
        boolean result = true;
        try {
            Context otherAppsContext = mApp.createPackageContext(
                    "com.android.phone", Context.CONTEXT_IGNORE_SECURITY);
            SharedPreferences mIccidPreference =
                    otherAppsContext.getSharedPreferences("RADIO_STATUS", 0);

            if (SubscriptionController.getInstance() != null) {
                int mSlotId = SubscriptionController.getInstance().getPhoneId(subId);

                if (mSlotId < 0 || mSlotId >= TelephonyManager.getDefault().getPhoneCount()) {
                    log("[isRadioOffBySimManagement]mSlotId: " + mSlotId);
                        return false;
                }

                String mIccId = SystemProperties.get(ICCRECORD_PROPERTY_ICCID[mSlotId], "");
                if ((mIccId != null) && (mIccidPreference != null)) {
                    log("[isRadioOffBySimManagement]SharedPreferences: "
                            + mIccidPreference.getAll().size() + ", IccId: " + mIccId);
                    result = mIccidPreference.contains(
                            RadioManager.getInstance().getHashCode(mIccId));
                }
            }

            log("[isRadioOffBySimManagement]result: " + result);
        } catch (NameNotFoundException e) {
            log("Fail to create com.android.phone createPackageContext");
        }
        return result;
    }
    // MTK-END

    /**
    * Return true if the FDN of the ICC card is enabled
    */
    public boolean isFdnEnabled(int subId) {
        log("isFdnEnabled subId=" + subId);

        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            loge("Error subId: " + subId);
            return false;
        }

        /* We will rollback the temporary solution after SubscriptionManager merge to L1 */
        Phone phone = getPhone(subId);
        if (phone != null && phone.getIccCard() != null) {
            return phone.getIccCard().getIccFdnAvailable() && phone.getIccCard().getIccFdnEnabled();
        } else {
            return false;
        }
    }

    private boolean canReadPhoneState(String callingPackage, String message) {
        try {
            mApp.enforceCallingOrSelfPermission(
                    android.Manifest.permission.READ_PRIVILEGED_PHONE_STATE, message);

            // SKIP checking for run-time permission since caller or self has PRIVILEDGED permission
            return true;
        } catch (SecurityException e) {
            mApp.enforceCallingOrSelfPermission(android.Manifest.permission.READ_PHONE_STATE,
                    message);
        }

        if (mAppOps.noteOp(AppOpsManager.OP_READ_PHONE_STATE, Binder.getCallingUid(),
                callingPackage) != AppOpsManager.MODE_ALLOWED) {
            return false;
        }

        return true;
    }

    // [SIM-C2K] @{
    /**
     * Get uim imsi by sub id.
     * @param callingPackage The package get UIM subscriber id.
     * @param subId subscriber id
     * @return uim imsi
     */
    public String getUimSubscriberId(String callingPackage, int subId) {
        if (!canReadPhoneState(callingPackage, "getUimSubscriberId")) {
            log("getUimImsiBySubId: permission denied");
            return null;
        }

        int phoneId = SubscriptionManager.getPhoneId(subId);
        if (phoneId < 0 || phoneId >= PROPERTY_UIM_SUBSCRIBER_ID.length) {
            log("getUimImsiBySubId:invalid phoneId " + phoneId);
            return null;
        }

        return SystemProperties.get(PROPERTY_UIM_SUBSCRIBER_ID[phoneId], "");
    }

    /**
     * Get IccId by slotId.
     * @param callingPackage The package get SIM serial number.
     * @param slotId int
     * @return Iccid
     */
    public String getSimSerialNumber(String callingPackage, int slotId) {
        if (!canReadPhoneState(callingPackage, "getSimSerialNumber")) {
            log("getSimSerialNumber: permission denied");
            return null;
        }
        return SystemProperties.get(PROPERTY_ICCID_SIM[slotId], "");
    }
    // [SIM-C2K] @}

    /**
     * Returns the MCC+MNC (mobile country code + mobile network code) of the
     * provider of the SIM for a particular subscription. 5 or 6 decimal digits
     * for GSM and CDMA applications.
     *
     * @param phoneId for which SimOperator is returned
     * @return MCCMNC array. array[0]: GSM MCCMNC array[1]: CDMA MCCMNC
     * If there is no GSM or CDMA MCCMNC, it is set "". It returns null for invalid
     * phoneId and returns "" when card is not in ready state.
     */
    public String[] getSimOperatorNumericForPhoneEx(int phoneId) {
        if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getSimCount()) {
            log("getSimOperatorNumericForPhoneEx with invalid phoneId:" + phoneId);
            return null;
        }

        String values[] = new String[2];
        //Get GSM MCCMNC
        values[0] = SystemProperties.get(PROPERTY_GSM_MCC_MNC[phoneId], "");
        //Get CDMA MCCMNC
        values[1] = SystemProperties.get(PROPERTY_CDMA_MCC_MNC[phoneId], "");

        return values;
    }

    /**
     * Set phone radio type and access technology.
     *
     * @param rafs an RadioAccessFamily array to indicate all phone's
     *        new radio access family. The length of RadioAccessFamily
     *        must equal to phone count.
     * @return true if start setPhoneRat successfully.
     */
    @Override
    public boolean setRadioCapability(RadioAccessFamily[] rafs) {
        boolean ret = true;
        try {
            ((MtkProxyController)ProxyController.getInstance()).setRadioCapability(rafs);
        } catch (RuntimeException e) {
            Log.w(LOG_TAG, "setRadioCapability: Runtime Exception");
            e.printStackTrace();
            ret = false;
        }
        return ret;
    }
    /**
     * Check if under capability switching.
     *
     * @return true if switching
     */
    public boolean isCapabilitySwitching() {
        return ((MtkProxyController)ProxyController.getInstance()).isCapabilitySwitching();
    }

    /**
     * Get main capability phone id.
     * @return The phone id with highest capability.
     */
    public int getMainCapabilityPhoneId() {
        return RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
    }

    /**
     * Get IMS registration state by given sub-id.
     * @param subId The subId for query
     * @return true if IMS is registered, or false
     * @hide
     */
    public boolean isImsRegistered(int subId) {
        Phone phone = getPhone(subId);
        if (phone == null) {
            return false;
        }
        boolean result = phone.isImsRegistered();
        if (DBG) {
            log("isImsRegistered(" + subId + ")=" + result);
        }
        return result;
    }

    /**
     * Get Volte registration state by given sub-id.
     * @param subId The subId for query
     * @return true if volte is registered, or false
     * @hide
     */
    public boolean isVolteEnabled(int subId) {
        Phone phone = getPhone(subId);
        if (phone == null) {
            return false;
        }
        boolean result = phone.isVolteEnabled();
        if (DBG) {
            log("isVolteEnabled=(" + subId + ")=" + result);
        }
        return result;
    }

    /**
     * Get WFC registration state by given sub-id.
     * @param subId The subId for query
     * @return true if wfc is registered, or false
     * @hide
     */
    public boolean isWifiCallingEnabled(int subId) {
        Phone phone = getPhone(subId);
        if (phone == null) {
            return false;
        }
        boolean result = phone.isWifiCallingEnabled();
        if (DBG) {
            log("isWifiCallingEnabled(" + subId + ")=" + result);
        }
        return result;
    }

    /**
     * Get WFC wifi PDN is active or not due to wifi may out of service.
     * @param subId The subId for query
     * @return true if wifi pdn is active, or false
     * @hide
     */
    public boolean isWifiCallingActive(int subId) {
        Phone phone = getPhone(subId);
        if (phone == null) {
            return false;
        }
        boolean isEnabled = phone.isWifiCallingEnabled();
        if (DBG) {
            log("isWifiCallingEnabled(" + subId + ")=" + isEnabled);
        }
        boolean isOOS = false;
        if (phone instanceof MtkImsPhone) {
            isOOS = ((MtkImsPhone)phone).isWifiPdnOutOfService();
        } else {
            log("phone is not instance of MtkImsPhone");
        }
        if (DBG) {
            log("isWifiPdnOutOfService(" + subId + ")=" + isOOS);
        }
        boolean result = isEnabled && !isOOS;
        log("isWifiCallingActive(" + subId + ")=" + result);
        return result;
    }

    // MTK-START: SIM GBA / AUTH
    /**
     * Helper thread to turn async call to {@link #SimAuthentication} into
     * a synchronous one.
     */
    private static class SimAuth extends Thread {
        private Phone mTargetPhone;
        private boolean mDone = false;
        private IccIoResult mResponse = null;

        // For replies from SimCard interface
        private Handler mHandler;

        // For async handler to identify request type
        private static final int SIM_AUTH_GENERAL_COMPLETE = 300;

        public SimAuth(Phone phone) {
            mTargetPhone = phone;
        }

        @Override
        public void run() {
            Looper.prepare();
            synchronized (SimAuth.this) {
                mHandler = new Handler() {
                    @Override
                    public void handleMessage(Message msg) {
                        AsyncResult ar = (AsyncResult) msg.obj;
                        switch (msg.what) {
                            case SIM_AUTH_GENERAL_COMPLETE:
                                log("SIM_AUTH_GENERAL_COMPLETE");
                                synchronized (SimAuth.this) {
                                    if (ar.exception != null) {
                                        log("SIM Auth Fail");
                                        mResponse = (IccIoResult) (ar.result);
                                    } else {
                                        mResponse = (IccIoResult) (ar.result);
                                    }
                                    log("SIM_AUTH_GENERAL_COMPLETE result is " + mResponse);
                                    mDone = true;
                                    SimAuth.this.notifyAll();
                                }
                                break;
                        }
                    }
                };
                SimAuth.this.notifyAll();
            }
            Looper.loop();
        }

        byte[] doGeneralSimAuth(int slotId, int family, int mode, int tag,
                String strRand, String strAutn) {
           synchronized (SimAuth.this) {
                while (mHandler == null) {
                    try {
                        SimAuth.this.wait();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }

                mDone = false;
                mResponse = null;

                Message callback = Message.obtain(mHandler, SIM_AUTH_GENERAL_COMPLETE);
                int sessionId = ((MtkUiccController)UiccController.getInstance())
                        .getIccApplicationChannel(
                        slotId, family);
                log("family = " + family + ", sessionId = " + sessionId);

                int[] subId = SubscriptionManager.getSubId(slotId);
                if (subId == null) {
                    log("slotId = " + slotId + ", subId is invalid.");
                    return null;
                } else {
                    ((MtkGsmCdmaPhone)getPhone(subId[0])).doGeneralSimAuthentication
                            (sessionId, mode, tag, strRand, strAutn, callback);
                }

                while (!mDone) {
                    try {
                        log("wait for done");
                        SimAuth.this.wait();
                    } catch (InterruptedException e) {
                        // Restore the interrupted status
                        Thread.currentThread().interrupt();
                    }
                }
                int len = 0;
                byte[] result = null;

                if (mResponse != null) {
                    // 2 bytes for sw1 and sw2
                    len = 2 + ((mResponse.payload == null) ? 0 : mResponse.payload.length);
                    result = new byte[len];

                    if (mResponse.payload != null) {
                        System.arraycopy(mResponse.payload, 0, result, 0, mResponse.payload.length);
                    }

                    result[len - 1] = (byte) mResponse.sw2;
                    result[len - 2] = (byte) mResponse.sw1;

                    // TODO: Should use IccUtils.bytesToHexString to print log info.
                    //for (int i = 0; i < len ; i++) {
                    //    log("Result = " + result[i]);
                    //}
                    //log("Result = " + new String(result));
                } else {
                    log("mResponse is null.");
                }

                log("done");
                return result;
            }
        }
    }
    // MTK-END
    // MTK-START: SIM GBA
    /**
     * Request to run AKA authenitcation on UICC card by indicated family.
     *
     * @param slotId indicated sim id
     * @param family indiacted family category
     *        UiccController.APP_FAM_3GPP =  1; //SIM/USIM
     *        UiccController.APP_FAM_3GPP2 = 2; //RUIM/CSIM
     *        UiccController.APP_FAM_IMS   = 3; //ISIM
     * @param byteRand random challenge in byte array
     * @param byteAutn authenication token in byte array
     *
     * @return reponse paramenters/data from UICC
     *
     */
    public byte[] simAkaAuthentication(int slotId, int family, byte[] byteRand, byte[] byteAutn) {
        enforcePrivilegedPhoneStatePermission();

        String strRand = "";
        String strAutn = "";
        log("simAkaAuthentication session is " + family + " simId " + slotId);

        if (byteRand != null && byteRand.length > 0) {
            strRand = IccUtils.bytesToHexString(byteRand).substring(0, byteRand.length * 2);
        }

        if (byteAutn != null && byteAutn.length > 0) {
            strAutn = IccUtils.bytesToHexString(byteAutn).substring(0, byteAutn.length * 2);
        }
        log("simAkaAuthentication Randlen " + strRand.length() + " strRand is "
                + strRand + ", AutnLen " + strAutn.length() + " strAutn " + strAutn);
        String akaData = Integer.toHexString(strRand.length()) + strRand +
                Integer.toHexString(strAutn.length()) + strAutn;
        if (DBG) {
            log("akaData: " + akaData);
        }


        int subId = getSubIdBySlot(slotId);
        int appType = PhoneConstants.APPTYPE_UNKNOWN;
        switch (family) {
            case 1:
                appType = PhoneConstants.APPTYPE_USIM;
                break;
            case 2:
                appType = PhoneConstants.APPTYPE_CSIM;
                break;
            case 3:
                appType = PhoneConstants.APPTYPE_ISIM;
                break;
        }
        if (appType == PhoneConstants.APPTYPE_UNKNOWN) {
            return null;
        } else {
            Context context = getPhone(subId).getContext();
            String responseData = TelephonyManager.from(context).getIccAuthentication(
                       subId, appType, TelephonyManager.AUTHTYPE_EAP_SIM, akaData);
            return IccUtils.hexStringToBytes(responseData);
        }
    }

    /**
     * Request to run GBA authenitcation (Bootstrapping Mode)on UICC card
     * by indicated family.
     *
     * @param slotId indicated sim id
     * @param family indiacted family category
     *        UiccController.APP_FAM_3GPP =  1; //SIM/USIM
     *        UiccController.APP_FAM_3GPP2 = 2; //RUIM/CSIM
     *        UiccController.APP_FAM_IMS   = 3; //ISIM
     * @param byteRand random challenge in byte array
     * @param byteAutn authenication token in byte array
     *
     * @return reponse paramenters/data from UICC
     *
     */
    public byte[] simGbaAuthBootStrapMode(int slotId, int family, byte[] byteRand, byte[] byteAutn) {
        enforcePrivilegedPhoneStatePermission();

        if (mSimAuthThread == null) {
            log("simGbaAuthBootStrapMode new thread");
            mSimAuthThread = new SimAuth(getPhone(getSubIdBySlot(slotId)));
            mSimAuthThread.start();
        } else {
            log("simGbaAuthBootStrapMode thread has been created.");
        }

        String strRand = "";
        String strAutn = "";
        log("simGbaAuthBootStrapMode session is " + family + " simId " + slotId);

        if (byteRand != null && byteRand.length > 0) {
            strRand = IccUtils.bytesToHexString(byteRand).substring(0, byteRand.length * 2);
        }

        if (byteAutn != null && byteAutn.length > 0) {
            strAutn = IccUtils.bytesToHexString(byteAutn).substring(0, byteAutn.length * 2);
        }
        log("simGbaAuthBootStrapMode strRand is " + strRand + " strAutn " + strAutn);

        return mSimAuthThread.doGeneralSimAuth(slotId, family, 1, 0xDD, strRand, strAutn);
    }

    /**
     * Request to run GBA authenitcation (NAF Derivation Mode)on UICC card
     * by indicated family.
     *
     * @param slotId indicated sim id
     * @param family indiacted family category
     *        UiccController.APP_FAM_3GPP =  1; //SIM/USIM
     *        UiccController.APP_FAM_3GPP2 = 2; //RUIM/CSIM
     *        UiccController.APP_FAM_IMS   = 3; //ISIM
     * @param byteNafId network application function id in byte array
     * @param byteImpi IMS private user identity in byte array
     *
     * @return reponse paramenters/data from UICC
     *
     */
    public byte[] simGbaAuthNafMode(int slotId, int family, byte[] byteNafId, byte[] byteImpi) {
        enforcePrivilegedPhoneStatePermission();

        if (mSimAuthThread == null) {
            log("simGbaAuthNafMode new thread");
            mSimAuthThread = new SimAuth(getPhone(getSubIdBySlot(slotId)));
            mSimAuthThread.start();
        } else {
            log("simGbaAuthNafMode thread has been created.");
        }

        String strNafId = "";
        String strImpi = "";
        log("simGbaAuthNafMode session is " + family + " simId " + slotId);

        if (byteNafId != null && byteNafId.length > 0) {
            strNafId = IccUtils.bytesToHexString(byteNafId).substring(0, byteNafId.length * 2);
        }

        /* ISIM GBA NAF mode parameter should be NAF_ID.
         * USIM GAB NAF mode parameter should be NAF_ID + IMPI
         * If getIccApplicationChannel got 0, mean that ISIM not support */
        if (((MtkUiccController)UiccController.getInstance()).
                getIccApplicationChannel(slotId, family) == 0) {
            log("simGbaAuthNafMode ISIM not support.");
            if (byteImpi != null && byteImpi.length > 0) {
                strImpi = IccUtils.bytesToHexString(byteImpi).substring(0, byteImpi.length * 2);
            }
        }
        log("simGbaAuthNafMode NAF ID is " + strNafId + " IMPI " + strImpi);

        return mSimAuthThread.doGeneralSimAuth(slotId, family, 1, 0xDE, strNafId, strImpi);
    }
    // MTK-END

    // M: [LTE][Low Power][UL traffic shaping] @{
    public boolean setLteAccessStratumReport(boolean enabled) {
        int phoneId = SubscriptionManager
                .getPhoneId(SubscriptionManager.getDefaultDataSubscriptionId());
        Phone phone = PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            loge("setLteAccessStratumReport: phone[" + phoneId + "] is null");
            return false;
        }
        if (DBG) log("setLteAccessStratumReport: enabled = " + enabled);
        Boolean success = (Boolean) sendRequest(CMD_SET_LTE_ACCESS_STRATUM_STATE,
                new Boolean(enabled), new Integer(phoneId));
        if (DBG) log("setLteAccessStratumReport: success = " + success);
        return success;

    }

    public boolean setLteUplinkDataTransfer(boolean isOn, int timeMillis) {
        int state = 1;
        int phoneId = SubscriptionManager
                .getPhoneId(SubscriptionManager.getDefaultDataSubscriptionId());
        Phone phone = PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            loge("setLteUplinkDataTransfer: phone[" + phoneId + "] is null");
            return false;
        }
        if (DBG) {
            log("setLteUplinkDataTransfer: isOn = " + isOn
                    + ", Tclose timer = " + (timeMillis/1000));
        }
        if (!isOn) state = (timeMillis/1000) << 16 | 0;
        Boolean success = (Boolean) sendRequest(CMD_SET_LTE_UPLINK_DATA_TRANSFER_STATE,
                new Integer(state), new Integer(phoneId));
        if (DBG) log("setLteUplinkDataTransfer: success = " + success);
        return success;
    }

    public String getLteAccessStratumState() {
        int phoneId = SubscriptionManager
                .getPhoneId(SubscriptionManager.getDefaultDataSubscriptionId());
        Phone phone = PhoneFactory.getPhone(phoneId);
        String state = MtkPhoneConstants.LTE_ACCESS_STRATUM_STATE_UNKNOWN;
        if (phone == null) {
            loge("getLteAccessStratumState: phone[" + phoneId + "] is null");
        } else {
            MtkDcTracker dcTracker = (MtkDcTracker) phone.getDcTracker(
                    AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
            state = dcTracker.getLteAccessStratumState();
        }
        if (DBG) log("getLteAccessStratumState: " + state);
        return state;
    }

    public boolean isSharedDefaultApn() {
        int phoneId = SubscriptionManager
                .getPhoneId(SubscriptionManager.getDefaultDataSubscriptionId());
        Phone phone = PhoneFactory.getPhone(phoneId);
        boolean isSharedDefaultApn = false;
        if (phone == null) {
            loge("isSharedDefaultApn: phone[" + phoneId + "] is null");
        } else {
            MtkDcTracker dcTracker = (MtkDcTracker) phone.getDcTracker(
                    AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
            isSharedDefaultApn = dcTracker.isSharedDefaultApn();
        }
        if (DBG) log("isSharedDefaultApn: " + isSharedDefaultApn);
        return isSharedDefaultApn;
    }
    // M: [LTE][Low Power][UL traffic shaping] @}

    // PHB START
    /**
     * This function is used to get SIM phonebook storage information
     * by sim id.
     *
     * @param simId Indicate which sim(slot) to query
     * @return int[] which incated the storage info
     *         int[0]; // # of remaining entries
     *         int[1]; // # of total entries
     *         int[2]; // # max length of number
     *         int[3]; // # max length of alpha id
     *
     */
    public int[] getAdnStorageInfo(int subId) {
        Log.d(LOG_TAG, "getAdnStorageInfo " + subId);

        if (SubscriptionManager.isValidSubscriptionId(subId) == true) {
            synchronized (mAdnInfoLock) {
                if (mAdnInfoThread == null) {
                    Log.d(LOG_TAG, "getAdnStorageInfo new thread !");
                    mAdnInfoThread  = new QueryAdnInfoThread(subId);
                    mAdnInfoThread.start();
                } else {
                    mAdnInfoThread.setSubId(subId);
                    Log.d(LOG_TAG, "getAdnStorageInfo old thread !");
                }
                return mAdnInfoThread.GetAdnStorageInfo();
            }
        } else {
            Log.d(LOG_TAG, "getAdnStorageInfo subId is invalid.");
            int[] recordSize;
            recordSize = new int[4];
            recordSize[0] = 0; // # of remaining entries
            recordSize[1] = 0; // # of total entries
            recordSize[2] = 0; // # max length of number
            recordSize[3] = 0; // # max length of alpha id
            return recordSize;
        }
    }

    private static class QueryAdnInfoThread extends Thread {

        private int mSubId;
        private boolean mDone = false;
        private int[] recordSize;

        private Handler mHandler;

        // For async handler to identify request type
        private static final int EVENT_QUERY_PHB_ADN_INFO = 100;

        public QueryAdnInfoThread(int subId) {
            mSubId = subId;
        }
        public void setSubId(int subId) {
            synchronized (QueryAdnInfoThread.this) {
                mSubId = subId;
                mDone = false;
            }
        }

        @Override
        public void run() {
            Looper.prepare();
            synchronized (QueryAdnInfoThread.this) {
                mHandler = new Handler() {
                    @Override
                    public void handleMessage(Message msg) {
                        AsyncResult ar = (AsyncResult) msg.obj;

                        switch (msg.what) {
                            case EVENT_QUERY_PHB_ADN_INFO:
                                Log.d(LOG_TAG, "EVENT_QUERY_PHB_ADN_INFO");
                                synchronized (QueryAdnInfoThread.this) {
                                    mDone = true;
                                    int[] info = (int[]) (ar.result);
                                    if (info != null && info.length == 4) {
                                        recordSize = new int[4];
                                        recordSize[0] = info[0]; // # of remaining entries
                                        recordSize[1] = info[1]; // # of total entries
                                        recordSize[2] = info[2]; // # max length of number
                                        recordSize[3] = info[3]; // # max length of alpha id
                                        Log.d(LOG_TAG, "recordSize[0]=" + recordSize[0] +
                                                ",recordSize[1]=" + recordSize[1] +
                                                ",recordSize[2]=" + recordSize[2] +
                                                ",recordSize[3]=" + recordSize[3]);
                                    }
                                    else {
                                        recordSize = new int[4];
                                        recordSize[0] = 0; // # of remaining entries
                                        recordSize[1] = 0; // # of total entries
                                        recordSize[2] = 0; // # max length of number
                                        recordSize[3] = 0; // # max length of alpha id
                                    }
                                    QueryAdnInfoThread.this.notifyAll();

                                }
                                break;
                            }
                      }
                };
                QueryAdnInfoThread.this.notifyAll();
            }
            Looper.loop();
        }

        public int[] GetAdnStorageInfo() {
            synchronized (QueryAdnInfoThread.this) {
                while (mHandler == null) {
                    try {
                        QueryAdnInfoThread.this.wait();

                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
                Message response = Message.obtain(mHandler, EVENT_QUERY_PHB_ADN_INFO);

                ((MtkGsmCdmaPhone)getPhone(mSubId)).queryPhbStorageInfo(MtkRILConstants.PHB_ADN, response);

                while (!mDone) {
                    try {
                        Log.d(LOG_TAG, "wait for done");
                        QueryAdnInfoThread.this.wait();
                    } catch (InterruptedException e) {
                        // Restore the interrupted status
                        Thread.currentThread().interrupt();
                    }
                }
                Log.d(LOG_TAG, "done");
                return recordSize;
            }
        }
    }

   /**
    * This function is used to check if the SIM phonebook is ready
    * by sub id.
    *
    * @param subId Indicate which sim(slot) to query
    * @return true if phone book is ready.
    *
    */
    public boolean isPhbReady(int subId) {
        int phoneId = SubscriptionManager.getPhoneId(subId);
        int slotId = SubscriptionManager.getSlotIndex(subId);
        boolean phbReady = false;

        if (SubscriptionManager.isValidSlotIndex(slotId) == true) {
            Phone phone = PhoneFactory.getPhone(phoneId);
            if (phone != null) {
                IccPhoneBookInterfaceManager iccPhbIfMgr = phone.getIccPhoneBookInterfaceManager();
                if (iccPhbIfMgr != null && iccPhbIfMgr instanceof MtkIccPhoneBookInterfaceManager) {
                    phbReady = ((MtkIccPhoneBookInterfaceManager) iccPhbIfMgr).isPhbReady();
                }
            }
        }

        return phbReady;
    }
    // PHB END

    private class RxTestObject {
        int result[] = null;
        Object lockObj = new Object();
    }

    public int[] setRxTestConfig(int phoneId, int config) {
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone)PhoneFactory.getPhone(phoneId);
        if (phone != null) {
            if (phone.mMtkCi != null) {
                RxTestObject RxTest = new RxTestObject();
                synchronized(RxTest.lockObj) {
                    phone.setRxTestConfig(config, mMainThreadHandler.obtainMessage(
                            EVENT_SET_RX_TEST_CONFIG, RxTest));
                    try {
                        RxTest.lockObj.wait(5000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                synchronized(RxTest.lockObj) {
                    if (RxTest.result != null) {
                        if (DBG) log("setRxTestConfig return: " + RxTest.result);
                        return RxTest.result;
                    } else {
                        if (DBG) log("setRxTestConfig return: null");
                        return null;
                    }
                }
            } else {
                if (DBG) log("setRxTestConfig phone.mMtkCi = null");
            }
        } else {
            if (DBG) log("setRxTestConfig phone = null");
        }
        return null;
    }

    public int[] getRxTestResult(int phoneId) {
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone)PhoneFactory.getPhone(phoneId);
        if (phone != null) {
            if (phone.mMtkCi != null) {
                RxTestObject RxTest = new RxTestObject();
                synchronized(RxTest.lockObj) {
                    phone.getRxTestResult(mMainThreadHandler.obtainMessage(
                            EVENT_GET_RX_TEST_RESULT, RxTest));
                    try {
                        RxTest.lockObj.wait(5000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                synchronized(RxTest.lockObj) {
                    if (RxTest.result != null) {
                        if (DBG) log("getRxTestResult return: " + RxTest.result);
                        return RxTest.result;
                    } else {
                        if (DBG) log("getRxTestResult return: null");
                        return null;
                    }
                }
            } else {
                if (DBG) log("getRxTestResult mMtkCi.mCi = null");
            }
        } else {
            if (DBG) log("getRxTestResult phone = null");
        }
        return null;
    }

    // M: [VzW] PCO based Self Activation @{
    public int selfActivationAction(int action, Bundle param, int subId) {
        // Default success
        int retVal = 0;
        ISelfActivation instance =
                ((MtkGsmCdmaPhone)getPhone(subId)).getSelfActivationInstance();
        if (instance != null) {
            retVal = instance.selfActivationAction(action, param);
        } else {
            if (DBG) log("null SelfActivation instance");
        }
        if (DBG) log("selfActivationAction: action = " +
                action + " subId = " + subId + " retVal = " + retVal);
        return retVal;
    }

    public int getSelfActivateState(int subId) {
        // Default value is STATE_NONE
        int retVal = 0;
        ISelfActivation instance =
                ((MtkGsmCdmaPhone)getPhone(subId)).getSelfActivationInstance();
        if (instance != null) {
            retVal = instance.getSelfActivateState();
        } else {
            if (DBG) log("null SelfActivation instance");
        }
        if (DBG) log("getSelfActivateState: subId = " + subId + " retVal = " + retVal);
        return retVal;
    }

    public int getPCO520State(int subId) {
        // Default value is FIVETOZERO_NONE
        int retVal = 0;
        ISelfActivation instance =
                ((MtkGsmCdmaPhone)getPhone(subId)).getSelfActivationInstance();
        if (instance != null) {
            retVal = instance.getPCO520State();
        } else {
            if (DBG) log("null SelfActivation instance");
        }
        if (DBG) log("getPCO520State: subId = " + subId + " retVal = " + retVal);
        return retVal;
    }
    // @}

    @Override
    public boolean exitEmergencyCallbackMode(int subId) {
        log("exitEmergencyCallbackMode, subId: " + subId);

        int phoneId = SubscriptionManager.getPhoneId(subId);
        if (phoneId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            log("no corresponding phone id");
            return false;
        }
        EcmExitResult result = (EcmExitResult) sendRequest(EVENT_EXIT_ECBM_MODE_REQ, subId);
        if (result.getReceiver() != null) {
            log("unregisterReceiver " + result.getReceiver());
            mApp.unregisterReceiver(result.getReceiver());
        }
        return true;
    }

    @Override
    public void setApcModeUsingSlotId(int slotId, int mode,
                        boolean reportOn, int reportInterval) {
        log("setApcModeUsingSlotId, slotId:" + slotId + ", mode:" + mode +
             ", reportOn:" + reportOn + ", reportInterval:" + reportInterval);
        int subId = getSubIdBySlot(slotId);
        final Phone phone = getPhone(subId);
        if (phone != null) {
            if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM) {
                ((MtkGsmCdmaPhone)phone).setApcMode(mode, reportOn, reportInterval);
            } else {
                log("setApcModeUsingSlotId: phone type is abnormal");
            }
        } else {
            log("setApcModeUsingSlotId, phone or subId: is null");
        }
    }

    @Override
    public PseudoCellInfo getApcInfoUsingSlotId(int slotId) {
        log("getApcInfoUsingSlotId, slotId:" + slotId);
        PseudoCellInfo info = null;
        int subId = getSubIdBySlot(slotId);
        final Phone phone = getPhone(subId);
        if (phone != null) {
            if (phone.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM) {
                info = ((MtkGsmCdmaPhone)phone).getApcInfo();
            } else {
                log("getApcInfoUsingSlotId: phone type is abnormal");
            }
        } else {
            log("getApcInfoUsingSlotId, phone or subId: is null");
        }
        return info;
    }

    /**
     * Get CDMA subscription active status  by subId.
     * @param subId subId
     * @return active status. 1 is active, 0 is deactive
     */
    public int getCdmaSubscriptionActStatus(int subId) {
        int actStatus = 0;
        Phone p = getPhone(subId);
        if (p != null){
            if (DBG) {
                log("getCdmaSubscriptionActStatus, phone type " + p.getPhoneType());
            }
            if (p.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
                actStatus = ((MtkGsmCdmaPhone)p).getCdmaSubscriptionActStatus();
            }
        } else {
            log("fail to getCdmaSubscriptionActStatus due to phone is null");
        }
        return actStatus;
    }

    public void setIsLastEccIms(boolean val) {
        if (DBG) log("setIsLastEccIms(): " + val);
        mIsLastEccIms = val;
    }

    public boolean getIsLastEccIms() {
        if (DBG) log("getIsLastEccIms(): " + mIsLastEccIms);
        return mIsLastEccIms;
    }

    @Override
    public int invokeOemRilRequestRaw(byte[] oemReq, byte[] oemResp) {
        enforceModifyPermission();

        int returnValue = 0;
        try {
            AsyncResult result = (AsyncResult)sendRequest(CMD_INVOKE_OEM_RIL_REQUEST_RAW, oemReq);
            if(result.exception == null) {
                if (result.result != null) {
                    byte[] responseData = (byte[])(result.result);
                    if(responseData.length > oemResp.length) {
                        Log.w(LOG_TAG, "Buffer to copy response too small: Response length is " +
                                responseData.length +  "bytes. Buffer Size is " +
                                oemResp.length + "bytes.");
                    }
                    System.arraycopy(responseData, 0, oemResp, 0, responseData.length);
                    returnValue = responseData.length;
                }
            } else {
                CommandException ex = (CommandException) result.exception;
                returnValue = ex.getCommandError().ordinal();
                if(returnValue > 0) returnValue *= -1;
            }
        } catch (RuntimeException e) {
            Log.w(LOG_TAG, "sendOemRilRequestRaw: Runtime Exception");
            returnValue = (CommandException.Error.GENERIC_FAILURE.ordinal());
            if(returnValue > 0) returnValue *= -1;
        }

        return returnValue;
    }

    @Override
    public int invokeOemRilRequestRawBySlot(int slotId, byte[] oemReq, byte[] oemResp) {
        enforceModifyPermission();

        int returnValue = 0;
        try {
            if (getSubIdBySlot(slotId) == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                Log.w(LOG_TAG, "sendOemRilRequestRawBySlot: invalid slot id");
                returnValue = (CommandException.Error.GENERIC_FAILURE.ordinal());
                if(returnValue > 0) returnValue *= -1;
                return returnValue;
            }
            AsyncResult result = (AsyncResult)sendRequest(CMD_INVOKE_OEM_RIL_REQUEST_RAW, oemReq
                    , getSubIdBySlot(slotId));
            if(result.exception == null) {
                if (result.result != null) {
                    byte[] responseData = (byte[])(result.result);
                    if(responseData.length > oemResp.length) {
                        Log.w(LOG_TAG, "Buffer to copy response too small: Response length is " +
                                responseData.length +  "bytes. Buffer Size is " +
                                oemResp.length + "bytes.");
                    }
                    System.arraycopy(responseData, 0, oemResp, 0, responseData.length);
                    returnValue = responseData.length;
                }
            } else {
                CommandException ex = (CommandException) result.exception;
                returnValue = ex.getCommandError().ordinal();
                if(returnValue > 0) returnValue *= -1;
            }
        } catch (RuntimeException e) {
            Log.w(LOG_TAG, "sendOemRilRequestRawBySlot: Runtime Exception");
            returnValue = (CommandException.Error.GENERIC_FAILURE.ordinal());
            if(returnValue > 0) returnValue *= -1;
        }

        return returnValue;
    }

    /**
     * Make sure the caller has the MODIFY_PHONE_STATE permission.
     *
     * @throws SecurityException if the caller does not have the required permission
     */
    private void enforceModifyPermission() {
        mApp.enforceCallingOrSelfPermission(android.Manifest.permission.MODIFY_PHONE_STATE, null);
    }

    /*
     * For CDMA system UI display requirement. Check whether in CS call.
     */
    @Override
    public boolean isInCsCall(int phoneId) {
        log("[isInCsCall] phoneId:" + phoneId);
        final Phone phone = PhoneFactory.getPhone(phoneId);
        if (phone != null && phone.getCallTracker() != null) {
            return phone.getCallTracker().getState() != PhoneConstants.State.IDLE;
        }
        return false;
    }

    @Override
    public List<CellInfo> getAllCellInfo(int phoneId, String callingPackage) {
        Phone phone = PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            return null;
        }

        mApp.getSystemService(AppOpsManager.class).checkPackage(Binder.getCallingUid(),
                callingPackage);

        LocationAccessPolicy.LocationPermissionResult locationResult =
                LocationAccessPolicy.checkLocationPermission(mApp,
                        new LocationAccessPolicy.LocationPermissionQuery.Builder()
                                .setCallingPackage(callingPackage)
                                .setCallingPid(Binder.getCallingPid())
                                .setCallingUid(Binder.getCallingUid())
                                .setMethod("getAllCellInfo")
                                .setMinSdkVersionForCoarse(Build.VERSION_CODES.BASE)
                                .setMinSdkVersionForFine(Build.VERSION_CODES.Q)
                                .build());
        switch (locationResult) {
            case DENIED_HARD:
                throw new SecurityException("Not allowed to access cell info");
            case DENIED_SOFT:
                return new ArrayList<>();
        }

        if (DBG_LOC) log("getAllCellInfo: is active user");
        WorkSource workSource = getWorkSource(Binder.getCallingUid());
        final long identity = Binder.clearCallingIdentity();
        try {
            List<CellInfo> cellInfos = new ArrayList<CellInfo>();
            final List<CellInfo> info = (List<CellInfo>) sendRequest(
                    CMD_GET_ALL_CELL_INFO, null, phone, workSource);
            if (info != null) {
                cellInfos.addAll(info);
            }
            return cellInfos;
        } finally {
            Binder.restoreCallingIdentity(identity);
        }
    }

    private final WorkSource getWorkSource(int uid) {
        String packageName = mApp.getPackageManager().getNameForUid(uid);
        return new WorkSource(uid, packageName);
    }

    /*
     * Get current located PLMN from service state tracker
     */
    public String getLocatedPlmn(int phoneId) {
        String plmn = null;
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone)PhoneFactory.getPhone(phoneId);
        if (phone != null) {
            plmn = phone.getLocatedPlmn();
        }
        return plmn;
    }

    /*
     * Set disable 2G state
     */
    public boolean setDisable2G(int phoneId, boolean mode) {
        String plmn = null;
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone)PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            log("setDisable2G, phone == null, phoneId=" + phoneId);
            return false;
        }
        boolean response = (boolean) sendRequest(CMD_SET_DISABLE_2G, new Boolean(mode), phone, null);
        return response;
    }

    /*
     * get disable 2G state
     */
    public int getDisable2G(int phoneId) {
        String plmn = null;
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone)PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            log("getDisable2G, phone == null, phoneId=" + phoneId);
            return -1;
        }
        int[] response = (int[]) sendRequest(CMD_GET_DISABLE_2G, null, phone, null);
        return response[0];
    }

    /*
     * Set femto cell list
     * @param phoneId  the id of the phone.
     */
    public List<FemtoCellInfo> getFemtoCellList(int phoneId) {
        String plmn = null;
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone)PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            log("getFemtoCellList, phone == null, phoneId=" + phoneId);
            return null;
        }
        List<FemtoCellInfo> cellInfos = new ArrayList<FemtoCellInfo>();
        final List<FemtoCellInfo> info = (List<FemtoCellInfo>) sendRequest(
                CMD_GET_FEMTOCELL_LIST, null, phone, null);
        if (info != null) {
            cellInfos.addAll(info);
        }
        return cellInfos;
    }

    /*
     * Abort femto cell list
     * @param phoneId  the id of the phone.
     */
    public boolean abortFemtoCellList(int phoneId) {
        String plmn = null;
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone)PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            log("abortFemtoCellList, phone == null, phoneId=" + phoneId);
            return false;
        }
        boolean response = (boolean) sendRequest(CMD_ABORT_FEMTOCELL_LIST, null, phone, null);
        return response;
    }

    /*
     * Manually selects a femtocell
     * @param phoneId  the id of the phone.
     * @param femtocell is the specified femtocell to be selected
     */
    public boolean selectFemtoCell(int phoneId, FemtoCellInfo femtocell) {
        String plmn = null;
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone)PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            log("selectFemtoCell, phone == null, phoneId=" + phoneId);
            return false;
        }
        boolean response = (boolean) sendRequest(CMD_SELECT_FEMTOCELL, femtocell, phone, null);
        return response;
    }

    /*
     * Query femto cell system selection mode
     * @param phoneId  the id of the phone.
     */
    public int queryFemtoCellSystemSelectionMode(int phoneId) {
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone)PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            log("queryFemtoCellSystemSelectionMode, phone == null, phoneId=" + phoneId);
            return -1;
        }
        int[] response = (int[]) sendRequest(CMD_QUERY_FEMTOCELL_SELECTION_MODE, null, phone, null);
        return response[0];
    }

    /*
     * Set femto cell system selection mode
     * @param phoneId  the id of the phone.
     * @param mode specifies the preferred system selection mode
     */
    public boolean setFemtoCellSystemSelectionMode(int phoneId, int mode) {
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone)PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            log("queryFemtoCellSystemSelectionMode, phone == null, phoneId=" + phoneId);
            return false;
        }
        boolean response = (boolean) sendRequest(CMD_SET_FEMTOCELL_SELECTION_MODE, new Integer(mode), phone, null);
        return response;
    }

    /*
     * Cancel available networks
     * @param phoneId the id of the phone.
     */
    public boolean cancelAvailableNetworks(int phoneId) {
        String plmn = null;
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone)PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            log("cancelAvailableNetworks, phone == null, phoneId=" + phoneId);
            return false;
        }
        boolean response = (boolean) sendRequest(CMD_CANCEL_AVAILABLE_NETWORK, null, phone, null);
        return response;
    }

    /**
     * Try to clean up the PDN connection of specific type
     *
     * @param phoneId the phone the request is sent to.
     * @param type the PDN type of the connection to be cleaned up.
     * @return false if exception occurs or true if the request is successfully sent.
     */
    public boolean tearDownPdnByType(int phoneId, String type) {
        if (type == null || !SubscriptionManager.isValidPhoneId(phoneId)) {
            return false;
        }

        final Phone phone = PhoneFactory.getPhone(phoneId);
        if (phone != null) {
            MtkDcTracker dct = (MtkDcTracker) phone.getDcTracker(
                    AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
            dct.tearDownPdnByType(type);
        } else {
            return false;
        }
        return true;
    }

    /**
     * Try to establish the PDN connection of specific type
     *
     * @param phoneId the phone the request is sent to.
     * @param type the PDN type of the connection to be cleaned up.
     * @return false if exception occurs or true if the request is successfully sent.
     */
    public boolean setupPdnByType(int phoneId, String type) {
        if (type == null || !SubscriptionManager.isValidPhoneId(phoneId)) {
            return false;
        }

        final Phone phone = PhoneFactory.getPhone(phoneId);
        if (phone != null) {
            MtkDcTracker dct = (MtkDcTracker) phone.getDcTracker(
                    AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
            dct.setupPdnByType(type);
        } else {
            return false;
        }
        return true;
    }

    /**
     * {@hide}
     * Returns the service state information on specified phoneId.
     */
    public ServiceState getServiceStateByPhoneId(int phoneId, String callingPackage) {
        if (!TelephonyPermissions.checkCallingOrSelfReadPhoneState(
                mApp, -1 /* unknown subid */, callingPackage, "getServiceStateForSubscriber")) {
            return null;
        }

        LocationAccessPolicy.LocationPermissionResult fineLocationResult =
                LocationAccessPolicy.checkLocationPermission(mApp,
                        new LocationAccessPolicy.LocationPermissionQuery.Builder()
                                .setCallingPackage(callingPackage)
                                .setCallingPid(Binder.getCallingPid())
                                .setCallingUid(Binder.getCallingUid())
                                .setMethod("getServiceStateForSubscriber")
                                .setMinSdkVersionForFine(Build.VERSION_CODES.Q)
                                .build());

        LocationAccessPolicy.LocationPermissionResult coarseLocationResult =
                LocationAccessPolicy.checkLocationPermission(mApp,
                        new LocationAccessPolicy.LocationPermissionQuery.Builder()
                                .setCallingPackage(callingPackage)
                                .setCallingPid(Binder.getCallingPid())
                                .setCallingUid(Binder.getCallingUid())
                                .setMethod("getServiceStateForSubscriber")
                                .setMinSdkVersionForCoarse(Build.VERSION_CODES.Q)
                                .build());
        // We don't care about hard or soft here -- all we need to know is how much info to scrub.
        boolean hasFinePermission =
                fineLocationResult == LocationAccessPolicy.LocationPermissionResult.ALLOWED;
        boolean hasCoarsePermission =
                coarseLocationResult == LocationAccessPolicy.LocationPermissionResult.ALLOWED;

        final long identity = Binder.clearCallingIdentity();
        try {
            final Phone phone = PhoneFactory.getPhone(phoneId);
            if (phone == null) {
                return null;
            }

            ServiceState ss = phone.getServiceState();

            // Scrub out the location info in ServiceState depending on what level of access
            // the caller has.
            if (hasFinePermission) return ss;
            if (hasCoarsePermission) return ss.sanitizeLocationInfo(false);
            return ss.sanitizeLocationInfo(true);
        } finally {
            Binder.restoreCallingIdentity(identity);
        }
    }

    @Override
    public boolean setRoamingEnable(int phoneId, int[] config) {
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone) PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            log("setRoamingEnable, phone == null, phoneId=" + phoneId);
            return false;
        }

        Boolean success = (Boolean) sendRequest(CMD_SET_ROAMING_ENABLE, config, phone, null);
        if (DBG) {
            log("setRoamingEnable: phoneId=" + phoneId + ", success=" + success);
        }
        return success;
    }

    @Override
    public int[] getRoamingEnable(int phoneId) {
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone) PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            log("getRoamingEnable, phone == null, phoneId=" + phoneId);
            return null;
        }

        int[] config = (int[]) sendRequest(CMD_GET_ROAMING_ENABLE, null, phone, null);
        if (DBG) {
            log("getRoamingEnable: phoneId=" + phoneId + ", config=" + config);
        }
        return config;
    }

    class SuggestedPlmnListRequest {
        public SuggestedPlmnListRequest() {
            rat = 3;
            num = 1;
            timer = 10;
        }

        public SuggestedPlmnListRequest(int rat, int num, int timer) {
            this.rat = rat;
            this.num = num;
            this.timer = timer;
        }

        int rat;
        int num;
        int timer;
    }
    public String[] getSuggestedPlmnList(int phoneId, int rat, int num, int timer,
                String callingPackage) {
        TelephonyPermissions.enforceCallingOrSelfModifyPermissionOrCarrierPrivilege(
                mApp, getSubIdBySlot(phoneId), "getSuggestedPlmnList");
        LocationAccessPolicy.LocationPermissionResult locationResult =
                LocationAccessPolicy.checkLocationPermission(mApp,
                        new LocationAccessPolicy.LocationPermissionQuery.Builder()
                                .setCallingPackage(callingPackage)
                                .setCallingPid(Binder.getCallingPid())
                                .setCallingUid(Binder.getCallingUid())
                                .setMethod("getSuggestedPlmnList")
                                .setMinSdkVersionForFine(Build.VERSION_CODES.Q)
                                .build());
        switch (locationResult) {
            case DENIED_HARD:
                throw new SecurityException("Not allowed to access scan results -- location");
            case DENIED_SOFT:
                return null;
        }

        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone) PhoneFactory.getPhone(phoneId);

        if (phone == null) {
            log("getSuggestedPlmnList, phone == null, phoneId=" + phoneId);
            return null;
        }
        SuggestedPlmnListRequest splr = new SuggestedPlmnListRequest(rat, num, timer);
        String[] result = (String[]) sendRequest(CMD_GET_SUGGESTED_PLMN_LIST, splr, phone, null);
        if (DBG) {
            log("getSuggestedPlmnList: phoneId=" + phoneId
                + " rat=" + rat + " num=" + num + " timer=" + timer
                + ", result=" + (result == null ? null : Arrays.toString(result)));
        }
        return result;
    }

    /*
     * Get current mobile data usage from MtkDctracker
     */
    public NetworkStats getMobileDataUsage(int phoneId) {
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone)PhoneFactory.getPhone(phoneId);
        if (phone != null) {
            final MtkDcTracker dct = (MtkDcTracker) phone.getDcTracker(
                    AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
            return dct.getMobileDataUsage();
        } else {
            return null;
        }
    }

    /*
     * Set previous mobile data usage to MtkDctracker
     */
    public void setMobileDataUsageSum(int phoneId, long txBytes, long txPkts,
                long rxBytes, long rxPkts) {
        MtkGsmCdmaPhone phone = (MtkGsmCdmaPhone)PhoneFactory.getPhone(phoneId);
        if (phone != null) {
            final MtkDcTracker dct = (MtkDcTracker) phone.getDcTracker(
                    AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
            dct.setMobileDataUsageSum(txBytes, txPkts, rxBytes, rxPkts);
        } else {
            log("setMobileDataUsageSum, phoneId: is null ");
        }
    }

    @Override
    public boolean isEmergencyNumber(int phoneId, String number) {
        final long identity = Binder.clearCallingIdentity();
        try {
            Phone phone = PhoneFactory.getPhone(phoneId);
            if (phone != null){
                if (phone.getEmergencyNumberTracker() != null) {
                    return phone.getEmergencyNumberTracker().isEmergencyNumber(
                            number, true);
                } else {
                    log("isEmergencyNumber return false due to tracker is null");
                    return false;
                }
            } else {
                log("isEmergencyNumber return false due to phone is null");
                return false;
            }
        } finally {
            Binder.restoreCallingIdentity(identity);
        }
    }
}

