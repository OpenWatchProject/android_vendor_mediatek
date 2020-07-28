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


package com.mediatek.op07.settings;

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.util.Log;
import android.view.MenuItem;
import android.widget.Toast;
//import com.android.ims.ImsConfig; // build error.
import com.android.ims.ImsException;
import com.android.ims.ImsManager;

import com.mediatek.entitlement.ISesService;
import com.mediatek.entitlement.ISesServiceListener;
import com.mediatek.ims.internal.MtkImsManager;

public class EntitlementActivity extends Activity
        implements E911WebsheetFragment.E911WebSheetListener {

    private static final boolean DBG = true;
    private static final String TAG = "op07settingsECCUIEntitlementActivity";

    private static final String VOWIFI_SERVICE = "vowifi";
    private static final String SERVICE_PKG_NAME = "com.mediatek.entitlement";
    private static final String SERVICE_NAME = "com.mediatek.entitlement.EntitlementService";

    public static final int STATE_NOT_ENTITLED = 0;
    public static final int STATE_PENDING = 1;
    public static final int STATE_ENTITLED = 2;
    public static final int STATE_ENTITLEMENT_FAILED = 3;

    private boolean mIsActivityVisible = false;
    private int mEntitlementState = STATE_NOT_ENTITLED;
    private E911WebsheetFragment mE911WebsheetFragment = null;
    private WiFiCallingFragment mWifiCallingFragment = null;
    private ProgressDialog mLoadingDialog;
    private ISesService mSesService = null;
    private boolean mShouldShowWebShet = false;
    private String mUrl = null;
    private String mPostData = null;
    private Context mContext = null;
    private int mLaunchMode = -1;
    private int mSlotId = -1;
    //Carrier config constants
    public final String EXTRA_LAUNCH_CARRIER_APP = "EXTRA_LAUNCH_CARRIER_APP";
    public final String EXTRA_LAUNCH_E911_URL = "EXTRA_LAUNCH_E911_URL";
    public final String EXTRA_LAUNCH_E911_POSTDATA = "EXTRA_LAUNCH_E911_POSTDATA";
    public final String EXTRA_LAUNCH_SLOTID = "EXTRA_LAUNCH_SLOTID";

    public final int LAUNCH_APP_ACTIVATE = 0;
    public final int LAUNCH_APP_CONNECT_UPDATE = 1;
    public final int LAUNCH_APP_ONLY_UPDATE = 2;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "action:" + action);
            if (action.equals(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED)) {
                if (!ImsManager.isWfcEnabledByPlatform(context)) {
                    Log.d(TAG, "carrier config changed, finish WFC activity");
                    stopActivity();
                }
            }
        }
    };

    private ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            mSesService = ISesService.Stub.asInterface(binder);
            log("onServiceConnected with mSesService = " + mSesService);
            Intent intent = new Intent();
            intent.setClassName(SERVICE_PKG_NAME, SERVICE_NAME);
            startService(intent);
            try {
                mSesService.registerListener(mSlotId, mSesStateListener);
                handleServiceConnected();
            } catch (RemoteException e) {
                Log.e(TAG, "Exception happened! " + e.getMessage());
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            log("onServiceDisconnected()");
            try {
                if (mSesService != null) {
                    mSesService.unregisterListener(mSlotId, mSesStateListener);
                    mSesService = null;
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception happened! " + e.getMessage());
            }
        }
    };

    private ISesServiceListener.Stub mSesStateListener = new ISesServiceListener.Stub() {
        @Override
        public void onEntitlementEvent(int slotId, String event, Bundle extras) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    log("onEntitlementEvent: " + slotId + ", " + event + ", " + extras);
                    mEntitlementState = translateToInternalState(event);
                    dismissProgressDialog();
                    switch (event) {
                        case "entitled":
                            Toast.makeText(EntitlementActivity.this,
                                    "Entitlement succeeded",
                                 Toast.LENGTH_SHORT).show();
                            break;
                        case "failed":
                            Toast.makeText(EntitlementActivity.this,
                                    "Entitlement failed",
                                Toast.LENGTH_SHORT).show();
                            break;
                        case "not-entitled":
                            Toast.makeText(EntitlementActivity.this,
                                    "Not Entitled",
                                Toast.LENGTH_SHORT).show();
                            break;
                        case "pending":
                            Toast.makeText(EntitlementActivity.this,
                                    "Entitlement pending",
                                Toast.LENGTH_SHORT).show();
                            break;
                        case "remove_sim":
                        case "new_sim":
                            Toast.makeText(EntitlementActivity.this, "New/Remove SIM",
                                Toast.LENGTH_SHORT).show();
                            break;
                    }
                    if (mEntitlementState != STATE_PENDING) {
                        stopActivity();
                    }
                }
            });
        }

        @Override
        public void onWebsheetPost(int slotId, String url, String postData) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    log("onWebsheetPost, url:" + url + ",postData: " + postData +
                        "mIsActivityVisible" + mIsActivityVisible);
                    mUrl = url;
                    mPostData = postData;
                    if (mIsActivityVisible) {
                        dismissProgressDialog();
                        showEmergencyFragment(slotId);
                    } else {
                        mShouldShowWebShet = true;
                    }
                }
            });
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wfc_settings_activity_layout);
        mContext = getApplicationContext();
        mLaunchMode = getIntent().getIntExtra(EXTRA_LAUNCH_CARRIER_APP, -1);
        mSlotId = getIntent().getIntExtra(EXTRA_LAUNCH_SLOTID, -1);
        log("EntitlementActivity onCreate:"
                + " mLaunchMode = " + mLaunchMode
                + " mSlotId = " + mSlotId);
        if (mLaunchMode == LAUNCH_APP_ONLY_UPDATE) {
            mUrl = getIntent().getStringExtra(EXTRA_LAUNCH_E911_URL);
            mPostData = getIntent().getStringExtra(EXTRA_LAUNCH_E911_POSTDATA);
            log("mUrl = " + mUrl + " ,mPostData = " + mPostData);
            if (mUrl == null || mPostData == null) {
                Toast.makeText(EntitlementActivity.this,
                        R.string.try_again, Toast.LENGTH_SHORT).show();
                setResult(Activity.RESULT_CANCELED);
                finish();
            } else {
                showEmergencyFragment(mSlotId);
            }
        } else {
            if (!checkErrorConditions()) {
                stopActivity();
            }
            //TODO: check if need do this via Carrierconfig
            ImsManager imsManager = ImsManager.getInstance(mContext, mSlotId);
            imsManager.setWfcMode(1, false);
            mWifiCallingFragment = new WiFiCallingFragment(mSlotId);
            FragmentTransaction ft = getFragmentManager().beginTransaction();
            ft.add(R.id.fragment_container, mWifiCallingFragment);
            ft.commit();
        }
    }

    @Override
    public void onPause() {
        mIsActivityVisible = false;
        unregisterReceiver(mReceiver);
        super.onPause();
    }

    @Override
    public void onResume() {
        mIsActivityVisible = true;
        if (mShouldShowWebShet) {
            log("onResume(): Show web sheet");
            showEmergencyFragment(mSlotId);
            mShouldShowWebShet = false;
        }
        IntentFilter filter = new IntentFilter(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
        registerReceiver(mReceiver, filter);
        super.onResume();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unbindWithService();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            stopActivity();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onBackPressed(int slotId) {
        Fragment f = getFragmentManager().findFragmentById(R.id.fragment_container);
        Log.d(TAG, "onBackPressed:" + f);
        if (f instanceof E911WebsheetFragment && mEntitlementState != STATE_ENTITLED) {
            if (mLaunchMode == LAUNCH_APP_ONLY_UPDATE) {
                setResult(Activity.RESULT_CANCELED);
                finish();
            } else {
                cancelEntitlementCheck(slotId);
            }
        } else if (f instanceof WiFiCallingFragment) {
            dismissProgressDialog();
        }
        stopActivity();
        super.onBackPressed();
    }

    private boolean checkErrorConditions() {
        if (mLaunchMode == LAUNCH_APP_ONLY_UPDATE) {
            //No need connect to Service in this case. So return directly.
            return true;
        }
        if (!isEntitlementEnabled()) {
            log("Entitlement system property not enabled, return fail");
            return false;
        }
        if (!isPackageExist(SERVICE_PKG_NAME)) {
            log("Entitlement service package doesn't exist");
            return false;
        }
        if (mLaunchMode == LAUNCH_APP_CONNECT_UPDATE) {
            //Double check if VoWiFi is provisioned. If not, shouldn't be here.
            try {
                ImsManager imsManager = ImsManager.getInstance(getApplicationContext(),
                        SubscriptionManager.getSubId(mSlotId)[0]);
                int wfcProvisionStatus = imsManager.getConfigInterface().getProvisionedValue(27);
                                //temp .ImsConfig.ConfigConstants.VOICE_OVER_WIFI_MODE);
                log("Wfc provisioning state = " + wfcProvisionStatus);
                if (wfcProvisionStatus != 1) {
                    Toast.makeText(this, "Wi-Fi Calling not provisioned",
                                 Toast.LENGTH_LONG).show();
                    return false;
                }
            } catch (ImsException e) {
                Log.e(TAG, "Exception happened! " + e.getMessage());
            }
        }
        return true;
    }

    public void stopActivity() {
        log("stopActivity(), mLaunchMode = " + mLaunchMode);
        if (mLaunchMode == LAUNCH_APP_ACTIVATE) {
            int resultCode = (mEntitlementState == STATE_ENTITLED)
                ? Activity.RESULT_OK : Activity.RESULT_CANCELED;
            setResult(resultCode);
        }
        finish();
    }

    public void bindWithService() {
        log("bindWithService()");
        Intent intent = new Intent();
        intent.setClassName(SERVICE_PKG_NAME, SERVICE_NAME);
        //startService(intent);
        if (!bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE)) {
            log("Bind to service fail!!");
            stopActivity();
        }
    }

    /**
     * Unbinds with Ses Service.
     * @param context context
     * @param serviceConnection serviceConnection
     * @return
     */
    private void unbindWithService() {
        if (mSesService != null) {
            log("unbindWithService Service instance = " + mSesService);
            try {
                mSesService.unregisterListener(mSlotId, mSesStateListener);
                unbindService(mServiceConnection);
                mSesService = null;
            } catch (RemoteException e) {
                Log.e(TAG, "Exception happened! " + e.getMessage());
            }
        }
    }

    private static void log(String msg) {
        if (DBG) {
            Log.d(TAG, msg);
        }
    }

    private void handleServiceConnected() {
        dismissProgressDialog();
        log("handleServiceConnected(), mLaunchMode = " + mLaunchMode);
        if (mLaunchMode == LAUNCH_APP_ACTIVATE) {
            turnWfcOn(mSlotId);
        } else {
            //showEmergencyFragment();
            updateLocationAndTc(mSlotId);
        }
    }

    private boolean isPackageExist(String pckName) {
        try {
            mContext.getPackageManager().getPackageInfo(pckName, PackageManager.GET_SERVICES);
        } catch (PackageManager.NameNotFoundException e) {
            log("Entitlement Srv package exist: false");
            return false;
        }
        log("Entitlement Srv package exist: true");
        return true;
    }

    private boolean isEntitlementEnabled() {
        boolean isEntitlementEnabled = (1 == SystemProperties.getInt
                ("persist.vendor.entitlement_enabled", 1) ? true : false);
        log("Check Entitlement Sys property isEntitlementEnabled:" + isEntitlementEnabled);
        return isEntitlementEnabled;
    }

    /**
     * Determines whether Wifi is ON or not.
     * @param context context
     * @return boolean
     */
    public boolean isWifiConnected() {
        WifiManager wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        if (wifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED) {
            log("In isWifiConnected(), Wifi ON");
            return true;
        }
        log(" In isWifiConnected(), Wifi OFF");
        return false;
    }

    public void turnWfcOn(int slotId) {
        startEntitlement(slotId, false);
        showProgressDialog(R.string.SES_ongoing_string);
    }
        /**
         * Checks whether wfc is entitled or not.
         * @param service service
         * @return boolean
         */
    public boolean isWfcEntitled() {
        boolean isWfcEntitled = false;
        try {
            if (mSesService != null) {
                String state = mSesService.getCurrentEntitlementState(0);
                isWfcEntitled = (translateToInternalState(state) == STATE_ENTITLED) ? true : false;
            }
        } catch (RemoteException e) {
            Log.e(TAG, "isWfcEntitled:Exception happened! " + e.getMessage());
        }
        return isWfcEntitled;
    }

    @Override
    public String getUrl(int slotId) {
        return mUrl;
    }

    @Override
    public String getServerData(int slotId) {
        return mPostData;
    }

    @Override
    public void startEntitlement(int slotId, boolean retry) {
        log("Start Entitlement with retry = " + retry + "launchmode = " + mLaunchMode);
        if (mLaunchMode != LAUNCH_APP_ONLY_UPDATE) {
            if (mSesService != null) {
                try {
                    if (retry) {
                        mSesService.startEntitlementCheck(slotId, 30, 2);
                    } else {
                        mSesService.startEntitlementCheck(slotId, 0, 0);
                    }
                } catch (RemoteException e) {
                    Log.e(TAG, "startEntitlementCheck :Exception happened! " + e.getMessage());
                }
            }
        }
    }

    public void updateLocationAndTc(int slotId) {
        showProgressDialog(R.string.loading_message);
        try {
            mSesService.updateLocationAndTc(slotId);
        } catch (RemoteException e) {
            Log.e(TAG, "updateLocationAndTc :Exception happened! " + e.getMessage());
        }
    }

    public void cancelEntitlementCheck(int slotId) {
        try {
            mSesService.stopEntitlementCheck(slotId);
        } catch (RemoteException e) {
            Log.e(TAG, "cancelEntitlementCheck :Exception happened! " + e.getMessage());
        }

    }

    @Override
    public boolean isServiceConnected(int slotId) {
        if (mLaunchMode == LAUNCH_APP_ONLY_UPDATE) {
            return true;
        }
        return mSesService != null ? true : false;
    }

    /// todo for P AND Q , can not find why method call
    public void deactivateService() {
        log("deactivateService mSesService = " + mSesService);
        if (mSesService != null) {
            try {
                mSesService.deactivateService(mSlotId);
            } catch (RemoteException e) {
                Log.e(TAG, "deactivateService :Exception happened! " + e.getMessage());
            }
        } else {
            Log.d(TAG, "deactivateService mSesService is null");
        }
    }

    private int translateToInternalState(String internalState) {
        if (internalState == null) {
            return STATE_NOT_ENTITLED;
        }
        switch (internalState) {
            case "not-entitled":
                return STATE_NOT_ENTITLED;
            case "failed":
                return STATE_ENTITLEMENT_FAILED;
            case "pending":
                return STATE_PENDING;
            case "entitled":
                return STATE_ENTITLED;
            default:
                Log.e(TAG, "Unhandled state: " + internalState);
                return STATE_NOT_ENTITLED;
        }
    }

    private void showProgressDialog(int message) {
        if (mLoadingDialog == null) {
            boolean isConnected = isServiceConnected(mSlotId);
            log("Showing progress Dialog to user, isConnected = " + isConnected);
            mLoadingDialog = ProgressDialog.show(EntitlementActivity.this, null,
                    getResources().getString(message), true,
                    true,
                    new DialogInterface.OnCancelListener() {
                        @Override
                        public void onCancel(DialogInterface dialog) {
                        // TODO:: Should do the cancel dialog handling here
                            log(" ProgressDialog: OnCancel");
                            mLoadingDialog = null;
                            cancelEntitlementCheck(mSlotId);
                            stopActivity();
                        }
                    });
            mLoadingDialog.setCanceledOnTouchOutside(false);
            mLoadingDialog.setInverseBackgroundForced(false);
        }
    }

    private void dismissProgressDialog() {
        if (mLoadingDialog != null && mLoadingDialog.isShowing()) {
            log("Dismiss Progress dialog");
            mLoadingDialog.dismiss();
            mLoadingDialog = null;
        }
    }

    private void showEmergencyFragment(int slotId) {
        /// maybe get fragement is null
        Fragment fragment = getFragmentManager().findFragmentById(R.id.fragment_container);
        log("showEmergencyFragment changed:" + fragment);
        if (null == fragment) {
            log("showEmergencyFragment changed: null == fragment");
            FragmentTransaction ft = getFragmentManager().beginTransaction();
            if (mE911WebsheetFragment == null) {
                mE911WebsheetFragment = new E911WebsheetFragment(slotId);
            }
            ft.add(R.id.fragment_container, mE911WebsheetFragment);
            ft.commit();
        } else if (!(fragment instanceof E911WebsheetFragment)) {
             FragmentTransaction ft = getFragmentManager().beginTransaction();
             if (mE911WebsheetFragment == null) {
                 mE911WebsheetFragment = new E911WebsheetFragment(slotId);
             }
             log("Start Loading E911WebsheetFragment" + mE911WebsheetFragment);
             ft.replace(R.id.fragment_container, mE911WebsheetFragment);
             ft.addToBackStack(null);
             ft.commit();
         }
     }
    /**
     * Method of E911WebsheetFragment.E911WebsheetFragmentListener interface.
     * @return
     */
    public void finishFragment() {
        log("Current fragment count = " + getFragmentManager().getBackStackEntryCount());
        getFragmentManager().popBackStackImmediate();
    }

    /**
    * Method of E911WebsheetFragment.E911WebsheetFragmentListener interface.
    * @param webSheetState Abandoned/Completed
    * @param finishWebSheet true if to be finished else false
    * @return
    */
    @Override
    public void webSheetStateChange(int slotId, boolean isComplete, boolean error) {
        log("webSheetStateChange, isComplete = " + isComplete + "error =" + error);
        if (mLaunchMode == LAUNCH_APP_ONLY_UPDATE) {
            Intent resultIntent = new Intent();
            // TODO Add extras or a data URI to this intent as appropriate.
            resultIntent.putExtra("isComplete", isComplete);
            if (error) {
                setResult(Activity.RESULT_CANCELED, resultIntent);
            } else {
                setResult(Activity.RESULT_OK, resultIntent);
            }
            finish();
        } else {
            if(isComplete) {
                showProgressDialog(R.string.SES_ongoing_string);
            } else {
                stopActivity();
            }
        }
        /* if (!isComplete) {
            cancelEntitlementCheck();
        }*/
    }

    public static class ErrorCodes {
        public static final int INVALID_RESPONSE = -1;

        public static final int REQUEST_SUCCESSFUL = 1000;
        public static final int AKA_CHALLENGE = 1003;
        public static final int INVALID_REQUEST = 1004;
        public static final int INVALID_IP_AUTH = 1005;
        public static final int AKA_AUTH_FAILED = 1006;
        public static final int FORBIDDEN_REQUEST = 1007;
        public static final int INVALID_CLIENT_ID = 1008;
        public static final int MAX_DEVICES_REACHED = 1010;
        public static final int UNKNOWN_DEVICE = 1020;
        public static final int EPDG_NOT_FOUND = 1021;
        public static final int CERT_FAILURE = 1022;
        public static final int REMOVE_FAILURE = 1023;
        public static final int INVALID_OWNER_ID = 1024;
        public static final int INVALID_CSR = 1025;
        public static final int DEVICE_LOCKED = 1028;
        public static final int INVALID_DEVICE_STATUS = 1029;
        public static final int MAX_SERVICES_REACHED = 1040;
        public static final int INVALID_SERVICE_FINGERPRINT = 1041;
        public static final int INVALID_TARGET_DEVICE = 1042;
        public static final int INVALID_TARGET_USER = 1043;
        public static final int MAX_SERVICE_INSTANCE_REACHED = 1044;
        public static final int FORBIDDEN_COPY = 1045;
        public static final int INVALID_SERVICE_NAME = 1046;
        public static final int SERVICE_INVALID_IMSI = 1047;
        public static final int SERVICE_NOT_ENTITLED = 1048;
        public static final int SERVICE_NO_PERMISSION = 1049;
        public static final int SERVICE_PARAMS_FAILURE = 1050;
        public static final int REMOVE_SERVICE_FAILURE = 1052;
        public static final int INVALID_SERVICE_INSTANCE_ID = 1053;
        public static final int INVALID_DEVICE_GROUP = 1054;
        public static final int NO_MSISDN = 1060;
        public static final int ERROR_IN_MSISDN_CREATION = 1061;
        public static final int MAX_MSISDN_EXCEEDED = 1062;
        public static final int INVALID_MSISDN = 1070;
        public static final int INVALID_PUSH_TOKEN = 1080;
        public static final int SERVER_ERROR = 1111;
        public static final int THREEGPP_AUTH_ONGOING = 1112;
        public static final int ONGOING_REQUEST = 1500;
        public static final int UNSUPPORTED_OPERATION = 9999;

        /**
         * Is error code valid.
         * @param errorCode errorCode
         * @return boolean
         */
        public static boolean isValidErrorCode(int errorCode) {
            switch(errorCode) {
                case REQUEST_SUCCESSFUL:
                case AKA_CHALLENGE:
                case INVALID_REQUEST:
                case INVALID_IP_AUTH:
                case AKA_AUTH_FAILED:
                case FORBIDDEN_REQUEST:
                case INVALID_CLIENT_ID:
                case MAX_DEVICES_REACHED:
                case UNKNOWN_DEVICE:
                case EPDG_NOT_FOUND:
                case CERT_FAILURE:
                case REMOVE_FAILURE:
                case INVALID_OWNER_ID:
                case INVALID_CSR:
                case DEVICE_LOCKED:
                case INVALID_DEVICE_STATUS:
                case MAX_SERVICES_REACHED:
                case INVALID_SERVICE_FINGERPRINT:
                case INVALID_TARGET_DEVICE:
                case INVALID_TARGET_USER:
                case MAX_SERVICE_INSTANCE_REACHED:
                case FORBIDDEN_COPY:
                case INVALID_SERVICE_NAME:
                case SERVICE_INVALID_IMSI:
                case SERVICE_NOT_ENTITLED:
                case SERVICE_NO_PERMISSION:
                case SERVICE_PARAMS_FAILURE:
                case REMOVE_SERVICE_FAILURE:
                case INVALID_SERVICE_INSTANCE_ID:
                case INVALID_DEVICE_GROUP:
                case NO_MSISDN:
                case ERROR_IN_MSISDN_CREATION:
                case MAX_MSISDN_EXCEEDED:
                case INVALID_MSISDN:
                case INVALID_PUSH_TOKEN:
                case SERVER_ERROR:
                case THREEGPP_AUTH_ONGOING:
                case ONGOING_REQUEST:
                case UNSUPPORTED_OPERATION:
                    return true;
                default:
                    return false;
            }
        }
    }
}
