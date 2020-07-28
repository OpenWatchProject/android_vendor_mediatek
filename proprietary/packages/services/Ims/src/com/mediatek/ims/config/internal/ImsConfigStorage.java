package com.mediatek.ims.config.internal;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsReasonInfo;
import android.text.TextUtils;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandException.Error;

import com.mediatek.ims.config.*;
import com.mediatek.ims.ImsCommonUtil;
import com.mediatek.ims.MtkImsConstants;
import com.mediatek.ims.plugin.ExtensionFactory;
import com.mediatek.ims.plugin.ImsManagerOemPlugin;
import com.mediatek.ims.ril.ImsCommandsInterface;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;




/**
 * The wrapper class to manage IMS configuration storage, including
 * 1.) Load default value from carrier's customization xml file.
 * 2.) TODO: Load provisioned value from non-volatile memory.
 * 3.) Initialize IMS configuration databases.
 */
public class ImsConfigStorage {
    private static final String TAG = "ImsConfigStorage";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean DEBUG = TextUtils.equals(Build.TYPE, "eng")
            || (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    // For add-on decoupling
    private static final String ACTION_CXP_NOTIFY_FEATURE =
        "com.mediatek.common.carrierexpress.cxp_notify_feature";

    private FeatureHelper mFeatureHelper = null;
    private ConfigHelper mConfigHelper = null;
    private ResourceHelper mResourceHelper = null;

    private int mPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
    /// M: IMS Provisioning. @{
    private ImsCommandsInterface mRilAdapter;
    /// @}
    private Context mContext = null;
    private ImsConfigStorage() {}
    private Handler mHandler;
    private BroadcastReceiver mReceiver;
    private int curWfcMode = -1;

    private ImsManagerOemPlugin mImsManagerOemPlugin = null;

    /// M: Ims provisioning error
    private int IMS_PROVISION_NO_DEFAULT_ERROR = 6604;

    /// M: Used for 93 to check if feature already send AT cmd once. @{
    static ImsFeatureMap[] FeatureSendArray = new ImsFeatureMap[4];
    /// @}

    // Clear config store w/o reloading default value.
    static final int MSG_RESET_CONFIG_STORAGE = 0;
    // Clear config store w/ reloading default value.
    static final int MSG_LOAD_CONFIG_STORAGE = 1;
    // Clear broadcast flag to still not broadcast status.
    static final int MSG_RESET_BROADCAST_FLAG = 2;
    // Flag for ECC usage, all feature false after sim absent
    static final int MSG_SIM_ABSENT_ECC_BROADCAST = 3;

    /// M: IMS Provisioning. @{
    // EVENT_GET_PROVISION_DONE_URC should receive before
    // OK msg for success (MSG_IMS_GET_PROVISION_DONE)
    static final int MSG_IMS_GET_PROVISION_DONE = 4;
    static final int MSG_IMS_SET_PROVISION_DONE = 5;
    /// @}
    /// M: IMS Provisioning URC. @{
    static final int EVENT_GET_PROVISION_DONE_URC = 6;
    /// @}

    static final int MSG_IMS_SET_MDCFG_DONE = 7;

    // Flag for reset wfc mode flag
    static final int MSG_RESET_WFC_MODE_FLAG = 8;
    // Update IMS service config
    static final int MSG_UPDATE_IMS_SERVICE_CONFIG = 9;
    // Force to set wfc mode again
    static final int MSG_FORCE_TO_SEND_WFC_MODE = 10;

    /// M: IMS Provisioning. @{
    static final int PROVISION_TIMEOUT_ERROR = 20;
    static final int PROVISION_INTERRUPT_ERROR = 21;
    static final int PROVISION_CMD_ERROR = 22;
    static final int PROVISION_CMD_SUCCESS = 23;
    static final int PROVISION_URC_PARSE_ERROR = 24;
    static final int PROVISION_NO_DEFAULT_VALUE = 25;
    /// @}

    /// M: Update MD IMS config. @{
    static final int MDCONFIG_TIMEOUT_ERROR = 30;
    static final int MDCONFIG_INTERRUPT_ERROR = 31;
    static final int MDCONFIG_CMD_ERROR = 32;
    static final int MDCONFIG_CMD_SUCCESS = 33;
    /// @}

    ///M: IMS config locks
    private Object mFeatureLock = new Object();
    private Object mProvisionedValueLock = new Object();
    private Object mProvisionedStringValueLock = new Object();
    private Object mWfcLock = new Object();
    private Object mMdCfgLock = new Object();

    /// M: IMS Provisioning. @{
    private class ProvisioningResult {
        String[] provisionInfo;
        int provisionResult = PROVISION_TIMEOUT_ERROR;
        Object lockObj = new Object();
    }
    /// @}

    private class MdConfigResult {
        int requestConfigNum = 0;
        int[] resultArray = null;
        int configResult = MDCONFIG_TIMEOUT_ERROR;
        Object lockObj = new Object();
    }

    public ImsConfigStorage(Context context, int phoneId, ImsCommandsInterface imsRilAdapter) {
        Log.d(TAG, "ImsConfigStorage() on phone " + phoneId);
        mContext = context;
        mPhoneId = phoneId;
        /// M: IMS Provisioning. @{
        mRilAdapter = imsRilAdapter;
        /// @}

        FeatureSendArray[mPhoneId] = new ImsFeatureMap(mPhoneId);

        HandlerThread thread = new HandlerThread("ImsConfig-" + mPhoneId);
        thread.start();
        mHandler = new CarrierConfigHandler(mPhoneId, thread.getLooper());

        mFeatureHelper = new FeatureHelper(mContext, mPhoneId);
        mConfigHelper = new ConfigHelper(mContext, mHandler, mPhoneId);
        mResourceHelper = new ResourceHelper(mContext, mPhoneId);

        // To init VoLTE feature value at the beginning
        mFeatureHelper.initFeatureStorage();

        /// M: IMS provisioning URC. @{
        mRilAdapter.registerForGetProvisionComplete(mHandler, EVENT_GET_PROVISION_DONE_URC, null);
        /// @}

        // Initial default value for feature values
        resetFeatureSendCmd();

        // Setup a receiver observes to notify reloading when sim changed.
        mReceiver = new ImsConfigReceiver(mHandler, mPhoneId, mRilAdapter);
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        filter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
        filter.addAction(ACTION_CXP_NOTIFY_FEATURE);
        filter.addAction(MtkImsConstants.ACTION_MTK_MMTEL_READY);
        if (ImsCommonUtil.isDssNoResetSupport()) {
            filter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
        }
        mContext.registerReceiver(mReceiver, filter);
    }

    class CarrierConfigHandler extends Handler {
        /// M: IMS Provisioning URC. @{
        private String[] provisionInfoTemp;
        private boolean isGetProvisionUrc;
        /// @}
        private int mPhoneId;
        CarrierConfigHandler(int phoneId, Looper looper) {
            super(looper);
            mPhoneId = phoneId;
        }

        @Override
        public void handleMessage(Message msg) {
            if (DEBUG) Log.d(TAG, "Received msg = " + msg.hashCode() + ", what = " + msg.what);
            switch (msg.what) {
                case MSG_RESET_CONFIG_STORAGE:
                    Log.d(TAG, "Reset config storage");
                    mConfigHelper.clear();
                    break;
                case MSG_LOAD_CONFIG_STORAGE:
                    synchronized (mConfigHelper) {
                        int opCode = (int) msg.obj;
                        if (mConfigHelper.getOpCode() != opCode) {
                            mConfigHelper.setOpCode(opCode);
                            Log.d(TAG, "Start load config storage for " + opCode +
                                    " on phone " + mPhoneId);
                            mConfigHelper.clear();
                            mConfigHelper.init(opCode);
                            mConfigHelper.setInitDone(true);
                            Log.d(TAG, "Finish Loading config storage for " + opCode);
                        } else {
                            Log.d(TAG, "Skip reloading config by same opCode: " + opCode +
                                " on phone " + mPhoneId);
                            mConfigHelper.setInitDone(true);
                        }
                    }
                    break;
                case MSG_RESET_BROADCAST_FLAG:
                    // Reset after sim absent or not ready
                    // 93 ImsConfig logic
                    resetFeatureSendCmd();
                    // Ims CTRL logic
                    mFeatureHelper.resetBroadcastFlag();
                    break;
                case MSG_RESET_WFC_MODE_FLAG:
                    resetWfcModeFlag();
                    break;
                case MSG_FORCE_TO_SEND_WFC_MODE:
                    int oldWfcMode = -1;
                    if (curWfcMode == -1) {
                        Log.i(TAG, "Should not set invalid wfc mode");
                    } else {
                        oldWfcMode = curWfcMode;
                        resetWfcModeFlag();
                        sendWfcProfileInfo(oldWfcMode);
                    }
                    break;
                case MSG_UPDATE_IMS_SERVICE_CONFIG: {
                    if (mImsManagerOemPlugin == null) {
                        mImsManagerOemPlugin = ExtensionFactory.makeOemPluginFactory(mContext)
                                .makeImsManagerPlugin(mContext);
                    }

                    mImsManagerOemPlugin.updateImsServiceConfig(mContext,
                            ImsCommonUtil.getMainCapabilityPhoneId(), true);
                }
                break;
                case MSG_SIM_ABSENT_ECC_BROADCAST:
                    // Reset after sim absent for ECC
                    // 93 ImsConfig logic
                    /* Raise a flag to enable VoLTE/IMS after SIM absent
                       which will be calculated by updateImsServiceConfig,
                       and send AT+EIMSVOLTE later if VoLTE is supported by platform
                    */
                    ImsConfigProvider.ECCAllowSendCmd.put(mPhoneId, true);
                    // Ims CTRL logic
                    if (mFeatureHelper.isAllFeatureFalse()) {
                        ImsConfigProvider.ECCAllowBroadcast.put(mPhoneId, true);
                        Log.d(TAG, "All feature false after sim absent," +
                                "should broadcast VoLTE feature value once for ECC");
                    }
                    break;
                /// M: IMS Provisioning. @{
                case MSG_IMS_GET_PROVISION_DONE: {
                    if (DEBUG) Log.d(TAG, "MSG_IMS_GET_PROVISION_DONE: Enter messege");

                    AsyncResult ar = (AsyncResult) msg.obj;
                    ProvisioningResult result = (ProvisioningResult) ar.userObj;

                    synchronized(result.lockObj) {
                        if (ar.exception != null) {
                            if ((ar.exception instanceof CommandException)
                                    && (((CommandException) (ar.exception)).getCommandError()
                                    == Error.OEM_ERROR_24)) {
                                // Error.OEM_ERROR_24, change after Android O
                                // ex: MD no default value, so return default value from AP
                                result.provisionResult = PROVISION_NO_DEFAULT_VALUE;
                                Log.d(TAG, "MSG_IMS_GET_PROVISION_DONE: MD no default value");
                            } else {
                                result.provisionResult = PROVISION_CMD_ERROR;
                                Log.d(TAG, "MSG_IMS_GET_PROVISION_DONE: error ret null, " +
                                    "e=" + ar.exception);
                            }
                        } else {
                            if (!isGetProvisionUrc || provisionInfoTemp.length < 2) {
                                Log.e(TAG, "MSG_IMS_GET_PROVISION_DONE: Error getting," +
                                        " URC error or no URC received!");
                                result.provisionResult = PROVISION_CMD_ERROR;
                            } else {
                                result.provisionInfo = new String[2];
                                result.provisionInfo[0] = provisionInfoTemp[0];
                                result.provisionInfo[1] = provisionInfoTemp[1];
                                result.provisionResult = PROVISION_CMD_SUCCESS;

                                Log.d(TAG, "MSG_IMS_GET_PROVISION_DONE: " +
                                        "provisionInfo[0]:" + result.provisionInfo[0] +
                                        ", provisionInfo[1]:" + result.provisionInfo[1]);
                            }
                        }
                        result.lockObj.notify();
                        if (DEBUG) Log.d(TAG, "MSG_IMS_GET_PROVISION_DONE: notify result");
                    }
                    break;
                }
                case MSG_IMS_SET_PROVISION_DONE:
                    if (DEBUG) Log.d(TAG, "MSG_IMS_SET_PROVISION_DONE: Enter messege");

                    AsyncResult ar = (AsyncResult) msg.obj;
                    ProvisioningResult result = (ProvisioningResult) ar.userObj;
                    synchronized(result.lockObj) {
                        if (ar.exception != null) {
                            // Retry to send AT cmd again when failure in future
                            result.provisionResult = PROVISION_CMD_ERROR;
                            Log.e(TAG, "MSG_IMS_SET_PROVISION_DONE: error ret null, e=" + ar.exception);
                        } else {
                            result.provisionResult = PROVISION_CMD_SUCCESS;
                            Log.d(TAG, "MSG_IMS_SET_PROVISION_DONE: Finish set provision!");
                        }
                        result.lockObj.notify();
                        if (DEBUG) Log.d(TAG, "MSG_IMS_SET_PROVISION_DONE: notify result");
                    }

                    break;
                /// @}
                case EVENT_GET_PROVISION_DONE_URC:
                    if (DEBUG) Log.d(TAG, "EVENT_GET_PROVISION_DONE_URC: Enter messege");

                    ar = (AsyncResult) msg.obj;
                    provisionInfoTemp = (String[]) ar.result;
                    isGetProvisionUrc = false;

                    if (ar.exception != null) {
                        Log.e(TAG, "EVENT_GET_PROVISION_DONE_URC: error, e=" + ar.exception);
                    } else {
                        Log.d(TAG, "EVENT_GET_PROVISION_DONE_URC: provisionInfoTemp.length: " +
                                provisionInfoTemp.length);
                        if(provisionInfoTemp != null && provisionInfoTemp.length >= 2) {
                            isGetProvisionUrc = true;
                        }
                    }
                    break;
                /// @}
                case MSG_IMS_SET_MDCFG_DONE:
                    if (DEBUG) Log.d(TAG, "MSG_IMS_SET_MDCFG_DONE: Enter messege");

                    ar = (AsyncResult) msg.obj;
                    MdConfigResult cfgResult = (MdConfigResult) ar.userObj;
                    String resultStr = null;

                    synchronized(cfgResult.lockObj) {
                        if (ar.exception != null) {
                            int[] errorResult = new int[cfgResult.requestConfigNum];
                            for (int i = 0 ; i < errorResult.length ; i++) {
                                errorResult[i] = -1;
                            }
                            // Retry to send AT cmd again when failure in future
                            cfgResult.resultArray = errorResult;
                            cfgResult.configResult = MDCONFIG_CMD_ERROR;
                            Log.e(TAG, "SET_MDCFG_DONE, error ret, e=" + ar.exception);
                        } else {
                            resultStr = (String) ar.result;

                            String[] resultStrArray = resultStr.split(",");
                            int[] resultIntArray = new int[resultStrArray.length];
                            for(int i = 0; i < resultStrArray.length; i++) {
                                resultIntArray[i] = Integer.parseInt(resultStrArray[i]);
                            }
                            cfgResult.resultArray = resultIntArray;

                            cfgResult.configResult = MDCONFIG_CMD_SUCCESS;
                            Log.d(TAG, "SET_MDCFG_DONE, finish set MD Ims config!");
                        }
                        cfgResult.lockObj.notify();
                        if (DEBUG) Log.d(TAG, "SET_MDCFG_DONE, notify result");
                    }
                    break;
                default:
                    // do nothing...
                    break;
            }
        }
    }

    /**
     * Gets the value for IMS feature item for specified network type.
     *
     * @param featureId, defined as in FeatureConstants.
     * @param network, defined as in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @return void
     *
     * @throws ImsException if calling the IMS service results in an error.
     */
    public int getFeatureValue(int featureId, int network)
            throws ImsException {
        synchronized(mFeatureLock) {
            return mFeatureHelper.getFeatureValue(featureId, network);
        }
    }

    /**
     * Sets the value for IMS feature item for specified network type.
     *
     * @param featureId, as defined in FeatureConstants.
     * @param network, as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @param value, as defined in FeatureValueConstants.
     * @return void
     *
     * @throws ImsException if calling the IMS service results in an error.
     */
    public void setFeatureValue(int featureId, int network, int value)
            throws ImsException {
        synchronized(mFeatureLock) {
            mFeatureHelper.updateFeature(featureId, network, value);
        }
    }

    /**
     * API to retrieve provisioned int value of IMS configurations.
     * @param configId The id defined in ImsConfig.ConfigConstants.
     * @return The int type configuration value.
     * @throws ImsException with following reason code
     *         1.) CODE_LOCAL_ILLEGAL_ARGUMENT if the configId can't match any data.
     *         2.) CODE_UNSPECIFIED if the config is without setting any value, even default.
     */
    public int getProvisionedValue(int configId)
            throws ImsException {
        synchronized(mProvisionedValueLock) {
            /// M: IMS Provisioning. @{
            if (isProvisionStoreModem(configId)) {
                ProvisioningResult result = new ProvisioningResult();

                String mProvisionStr = ImsConfigSettings.getProvisionStr(configId);
                Message msg = mHandler.obtainMessage(MSG_IMS_GET_PROVISION_DONE, result);
                synchronized (result.lockObj) {
                    mRilAdapter.getProvisionValue(mPhoneId, mProvisionStr, msg);
                    try {
                        result.lockObj.wait(10000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                        result.provisionResult = PROVISION_INTERRUPT_ERROR;
                    }
                }
                // If MD no deafult value, return from AP default config in DB
                if (result.provisionResult == PROVISION_NO_DEFAULT_VALUE) {
                    enforceConfigStorageInit("MD no default value, " +
                        "getProvisionedValue(" + configId + ")");
                    return mConfigHelper.getConfigValue(ImsConfigContract.TABLE_MASTER, configId);
                }
                if (!isProvisionSuccess(result.provisionResult)) {
                    throw new ImsException("Something wrong, reason:" + result.provisionResult ,
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
                }
                return Integer.parseInt(result.provisionInfo[1]);
            } else {
            /// @}
                enforceConfigStorageInit("getProvisionedValue(" + configId + ")");
                return mConfigHelper.getConfigValue(ImsConfigContract.TABLE_MASTER, configId);
            }
        }
    }

    /**
     * API to retrieve provisioned String value of IMS configurations.
     * @param configId The id defined in ImsConfig.ConfigConstants.
     * @return The int type configuration value.
     * @throws ImsException with following reason code
     *         1.) CODE_LOCAL_ILLEGAL_ARGUMENT if the configId can't match any data.
     *         2.) CODE_UNSPECIFIED if the config is without setting any value, even default.
     */
    public String getProvisionedStringValue(int configId)
            throws ImsException  {
        synchronized(mProvisionedStringValueLock) {
            /// M: IMS Provisioning. @{
            if (isProvisionStoreModem(configId)) {
                ProvisioningResult result = new ProvisioningResult();

                String mProvisionStr = ImsConfigSettings.getProvisionStr(configId);
                Message msg = mHandler.obtainMessage(MSG_IMS_GET_PROVISION_DONE, result);
                synchronized (result.lockObj) {
                    mRilAdapter.getProvisionValue(mPhoneId, mProvisionStr, msg);
                    try {
                        result.lockObj.wait(10000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                        result.provisionResult = PROVISION_INTERRUPT_ERROR;
                    }
                }
                // If MD no deafult value, return from AP default config in DB
                if (result.provisionResult == PROVISION_NO_DEFAULT_VALUE) {
                    enforceConfigStorageInit("MD no default value, " +
                        "getProvisionedStringValue(" + configId + ")");
                    return mConfigHelper.getConfigStringValue(ImsConfigContract.TABLE_MASTER, configId);
                }
                if (!isProvisionSuccess(result.provisionResult)) {
                    throw new ImsException("Something wrong, reason:" + result.provisionResult ,
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
                }
                return result.provisionInfo[1];
            } else {
            /// @}
                enforceConfigStorageInit("getProvisionedStringValue(" + configId + ")");
                return mConfigHelper.getConfigStringValue(ImsConfigContract.TABLE_MASTER, configId);
            }
        }
    }

    /**
     * Sets the value for IMS service/capabilities parameters by
     * the operator device management entity.
     * This function should not be called from main thread as it could block
     * mainthread.
     *
     * @param configId, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @param value in Integer format.
     * @return as defined in com.android.ims.ImsConfig#OperationStatusConstants
     *
     * @throws ImsException if calling the IMS service results in an error.
     */
    public void setProvisionedValue(int configId, int value)
            throws ImsException {
        synchronized(mProvisionedValueLock) {
            enforceConfigStorageInit("setProvisionedValue(" + configId + ", " + value + ")");
            /// M: IMS Provisioning. @{
            if (isProvisionStoreModem(configId)) {
                ProvisioningResult result = new ProvisioningResult();

                String mProvisionStr = ImsConfigSettings.getProvisionStr(configId);
                Message msg = mHandler.obtainMessage(MSG_IMS_SET_PROVISION_DONE, result);
                synchronized (result.lockObj) {
                    mRilAdapter.setProvisionValue(mPhoneId, mProvisionStr, Integer.toString
                            (value), msg);
                    try {
                        result.lockObj.wait(10000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                        result.provisionResult = PROVISION_INTERRUPT_ERROR;
                    }
                }
                if (!isProvisionSuccess(result.provisionResult)) {
                    throw new ImsException("Something wrong, reason:" + result.provisionResult ,
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
                }
            }
            /// @}
            // 1. Add / update to provision table
            mConfigHelper.addConfig(ImsConfigContract.TABLE_PROVISION,
                    configId, ImsConfigContract.MimeType.INTEGER, value);
            // 2. Update master table
            mConfigHelper.updateConfig(ImsConfigContract.TABLE_MASTER,
                    configId, ImsConfigContract.MimeType.INTEGER, value);
        }
    }

    /**
     * Sets the value for IMS service/capabilities parameters by
     * the operator device management entity.
     * This function should not be called from main thread as it could block
     * mainthread.
     *
     * @param configId, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @param value in String format.
     * @return as defined in com.android.ims.ImsConfig#OperationStatusConstants
     *
     * @throws ImsException if calling the IMS service results in an error.
     */
    public void setProvisionedStringValue(int configId, String value)
            throws ImsException {
        synchronized(mProvisionedStringValueLock) {
            enforceConfigStorageInit("setProvisionedStringValue(" + configId + ", " + value + ")");
            /// M: IMS Provisioning. @{
            if (isProvisionStoreModem(configId)) {
                ProvisioningResult result = new ProvisioningResult();

                String mProvisionStr = ImsConfigSettings.getProvisionStr(configId);
                Message msg = mHandler.obtainMessage(MSG_IMS_SET_PROVISION_DONE, result);
                synchronized (result.lockObj) {
                    mRilAdapter.setProvisionValue(mPhoneId, mProvisionStr, value, msg);
                    try {
                        result.lockObj.wait(10000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                        result.provisionResult = PROVISION_INTERRUPT_ERROR;
                    }
                }
                if (!isProvisionSuccess(result.provisionResult)) {
                    throw new ImsException("Something wrong, reason:" + result.provisionResult ,
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
                }
            }
            /// @}
            // 1. Add / update to provision table
            mConfigHelper.addConfig(ImsConfigContract.TABLE_PROVISION, configId,
                    ImsConfigContract.MimeType.STRING, value);
            // 2. Update master table
            mConfigHelper.updateConfig(ImsConfigContract.TABLE_MASTER, configId,
                    ImsConfigContract.MimeType.STRING, value);
        }
    }

    public synchronized void setImsResCapability(int featureId, int value)
            throws ImsException {
        mResourceHelper.updateResource(featureId, value);
    }

    public synchronized int getImsResCapability(int featureId)
            throws ImsException {
        return mResourceHelper.getResourceValue(featureId);
    }

    private void enforceConfigStorageInit(String msg) throws ImsException {
        if (!mConfigHelper.isInitDone()) {
            Log.e(TAG, msg);
            throw new ImsException("Config storage not ready",
                    ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
        }
    }

    /// M: IMS Provisioning. @{
    private static boolean isProvisionStoreModem(int configId) {
        boolean checkIsStoreModem = ImsConfigSettings.getIsStoreModem(configId);
        if (DEBUG) Log.d(TAG, "isProvisionStoreModem: " + configId +
                ", checkIsStoreModem: " + checkIsStoreModem);
        return checkIsStoreModem;
    }

    private void resetFeatureSendCmd() {
        HashMap<Integer, Boolean> map = FeatureSendArray[mPhoneId].getFeatureMap();

        map.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE, false);
        map.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE, false);
        map.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI, false);
        map.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI, false);
    }

    private static boolean isProvisionSuccess(int reason) {
       boolean isSuccess = false;
       switch (reason) {
           case PROVISION_CMD_SUCCESS:
               isSuccess = true;
               break;
           default:
               // do nothing...
               break;
       }
       return isSuccess;
    }
    /// @}

    /* @hide */
    public void resetConfigStorage() {
        resetConfigStorage(ImsConfigContract.Operator.OP_DEFAULT);
    }

    /* @hide */
    public void resetConfigStorage(int opCode) {
        Log.d(TAG, "resetConfigStorage(" + opCode + ")");
        synchronized(mConfigHelper) {
            mConfigHelper.clear();
            mConfigHelper.init(opCode);
        }
    }

    /* @hide */
    public void resetFeatureStorage() {
        Log.d(TAG, "resetFeatureStorage()");
        synchronized(mFeatureHelper) {
            mFeatureHelper.clear();
        }
    }

    /**
     * Sync Volte preference for voice domain to modem.
     */
    public synchronized void setVoltePreference(int mode) {
        Log.i(TAG, "setVoltePreference mode:" + mode + ", phoneId:" + mPhoneId);
        mRilAdapter.setVoiceDomainPreference(mode, null);
    }

    /**
     * Sync WFC preferred mode to modem.
     */
    public void sendWfcProfileInfo(int rilWfcMode) {
        synchronized(mWfcLock) {
            Log.i(TAG, "sendWfcProfileInfo rilWfcMode:" + rilWfcMode +
                    ", curWfcMode:" + curWfcMode);
            if (rilWfcMode != curWfcMode) {
                mRilAdapter.sendWfcProfileInfo(rilWfcMode, null);

                if (rilWfcMode == ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY) {
                    if (ImsConfigUtils.isWfcEnabledByUser(mContext, mPhoneId)) {
                        if (DEBUG) Log.d(TAG, "Wifi-only and WFC setting enabled, " +
                                "send intent to turn radio OFF");
                        ImsConfigUtils.sendWifiOnlyModeIntent(mContext, mPhoneId, true);
                    } else {
                        if (DEBUG) Log.d(TAG, "Wifi-only and WFC setting disabled, " +
                                "send intent to turn radio ON");
                        ImsConfigUtils.sendWifiOnlyModeIntent(mContext, mPhoneId, false);
                    }
                } else {
                    if (DEBUG) Log.d(TAG, "Not wifi-only mode, trun radio ON");
                    ImsConfigUtils.sendWifiOnlyModeIntent(mContext, mPhoneId, false);
                }

                curWfcMode = rilWfcMode;
            }
        }
    }

    public int[] setModemImsCfg(String[] keys, String[] values, int type) {
        synchronized (mMdCfgLock) {
            String keysStr = null;
            String valuesStr = null;
            int[] resultArray = null;

            // need to transfer String[] keys to keys String
            if (keys == null) {
                Log.d(TAG, "keys is null, return null");
                return null;
            }

            if (keys.length < 1 || values.length < 1) {
                Log.d(TAG, "keys or values length is smaller than 1, return null");
                return null;
            }

            if (keys.length == values.length) {
                Log.d(TAG, "keys and values length equals");
                keysStr = ImsConfigUtils.arrayToString(keys);
                valuesStr = ImsConfigUtils.arrayToString(values);
                Log.d(TAG, "keysStr:" + keysStr + ", valuesStr:" + valuesStr);

                MdConfigResult cfgResult = new MdConfigResult();
                cfgResult.requestConfigNum = keys.length;

                Message msg = mHandler.obtainMessage(MSG_IMS_SET_MDCFG_DONE, cfgResult);
                synchronized(cfgResult.lockObj) {
                    // should wait for callback to return
                    mRilAdapter.setModemImsCfg(keysStr, valuesStr, type, msg);
                    try {
                        cfgResult.lockObj.wait(10000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                        cfgResult.configResult = MDCONFIG_INTERRUPT_ERROR;
                    }
                }
                resultArray = cfgResult.resultArray;
            } else {
                Log.d(TAG, "keys and values length not equals");
                return null;
            }
            return resultArray;
        }
    }

    private void resetWfcModeFlag() {
        if (TELDBG) Log.d(TAG, "resetWfcModeFlag()");
        synchronized(mWfcLock) {
            curWfcMode = -1;
        }
    }

    private static class FeatureHelper {
        private int mPhoneId;
        private Context mContext = null;
        private ContentResolver mContentResolver = null;
        private HashMap<Integer, Integer> mIsFeatureBroadcast =
                new HashMap<Integer, Integer>();

        FeatureHelper(Context context, int phoneId) {
            mPhoneId = phoneId;
            mContext = context;
            mContentResolver = mContext.getContentResolver();

            resetBroadcastFlag();
        }

        private void initFeatureStorage() {
            // VoLTE
            int volte = ImsConfigUtils.getFeaturePropValue(
                    ImsConfigUtils.PROPERTY_VOLTE_ENALBE, mPhoneId);
            updateFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE,
                    TelephonyManager.NETWORK_TYPE_LTE, volte);
            Log.d(TAG, "updateFeature: VoLTE initial value:" + volte + " for phoneId:" + mPhoneId);

            // ViLTE
            int vilte = ImsConfigUtils.getFeaturePropValue(
                    ImsConfigUtils.PROPERTY_VILTE_ENALBE, mPhoneId);
            updateFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE,
                    TelephonyManager.NETWORK_TYPE_LTE, vilte);
            Log.d(TAG, "updateFeature: ViLTE initial value:" + vilte + " for phoneId:" + mPhoneId);

            // VoWIFI
            int vowifi = ImsConfigUtils.getFeaturePropValue(
                    ImsConfigUtils.PROPERTY_WFC_ENALBE, mPhoneId);
            updateFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI,
                    TelephonyManager.NETWORK_TYPE_IWLAN, vowifi);
            Log.d(TAG, "updateFeature: VoWIFI initial value:" + vowifi + " for phoneId:" + mPhoneId);

            /*
            // ViWIFI Not support currently
            value = ImsConfig.FeatureValueConstants.OFF;
            addFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI, value);
            */
        }

        private void clear() {
            String selection = ImsConfigContract.BasicConfigTable.PHONE_ID + " = ?";
            String[] args = {String.valueOf(mPhoneId)};
            mContentResolver.delete(ImsConfigContract.Feature.CONTENT_URI, selection, args);
        }

        private void updateFeature(int featureId, int network, int value) {
            int curValue = -1;
            boolean result = false;
            ContentValues cv = new ContentValues();
            cv.put(ImsConfigContract.Feature.PHONE_ID, mPhoneId);
            cv.put(ImsConfigContract.Feature.FEATURE_ID, featureId);
            cv.put(ImsConfigContract.Feature.NETWORK_ID, network);
            cv.put(ImsConfigContract.Feature.VALUE, value);

            // Check exist or not
            try {
                curValue = getFeatureValue(featureId, network);
                if (DEBUG) Log.d(TAG, "updateFeature() comparing: curValue: " +
                        curValue + ", value:" + value);
                if (!checkIfBroadcastOnce(featureId, mPhoneId) || curValue != value || curValue == -1) {
                    mContentResolver.update(
                            ImsConfigContract.Feature.getUriWithFeatureId(mPhoneId, featureId, network),
                            cv, null, null);
                }
            } catch (ImsException e) {
                Log.e(TAG, "updateFeature() ImsException featureId:" + featureId +", value:" + value);
                mContentResolver.insert(ImsConfigContract.Feature.CONTENT_URI, cv);
            }
        }

        int getFeatureValue(int featureId, int network) throws ImsException {
            Cursor c = null;
            int result = -1;
            String[] projection = {
                    ImsConfigContract.Feature.PHONE_ID,
                    ImsConfigContract.Feature.FEATURE_ID,
                    ImsConfigContract.Feature.NETWORK_ID,
                    ImsConfigContract.Feature.VALUE};

            try {
                c = mContentResolver.query(
                        ImsConfigContract.Feature.getUriWithFeatureId(mPhoneId, featureId, network),
                        projection, null, null, null);
                if (c != null && c.getCount() == 1) {
                    c.moveToFirst();
                    int valueIndex = c.getColumnIndex(ImsConfigContract.Feature.VALUE);
                    result = c.getInt(valueIndex);
                    c.close();
                } else {
                    throw new ImsException("Feature " + featureId + " not assigned with value!",
                            ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
                }

            } catch (Exception e) {
                throw new ImsException("Feature " + featureId + " not assigned with value!" +
                        " or something wrong with cursor",
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
            } finally {
                if (c != null) {
                    c.close();
                }
            }

            return result;
        }

        private boolean checkIfBroadcastOnce(int feature, int phoneId) {
            String simState;

            simState = ImsConfigProvider.LatestSimState.get(phoneId);
            if (TELDBG) Log.d(TAG, "checkIfBroadcastOnce() phoneId: " +
                    phoneId + ", Sim state: " + simState);
            if (simState == null) {
                return false;
            }
            if (simState != null && !(simState.equals(IccCardConstants.INTENT_VALUE_ICC_READY) ||
                    simState.equals(IccCardConstants.INTENT_VALUE_ICC_IMSI) ||
                    simState.equals(IccCardConstants.INTENT_VALUE_ICC_LOADED) ||
                    simState.equals(IccCardConstants.INTENT_VALUE_ICC_LOCKED))) {
                return false;
            } else {
                if (mIsFeatureBroadcast.get(feature) == 0) {
                    mIsFeatureBroadcast.put(feature, 1);
                    return false;
                } else {
                    return true;
                }
            }
        }

        private void resetBroadcastFlag() {
            mIsFeatureBroadcast.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE, 0);
            mIsFeatureBroadcast.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE, 0);
            mIsFeatureBroadcast.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI, 0);
            mIsFeatureBroadcast.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI, 0);
        }

        private boolean isAllFeatureFalse() {
            int volte = -1, vilte = -1, wfc = -1;

            try {
                volte = getFeatureValue(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE,
                        ImsConfigContract.getNetworkTypeByFeature(
                                ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE));
                vilte = getFeatureValue(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE,
                        ImsConfigContract.getNetworkTypeByFeature(
                                ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE));
                wfc = getFeatureValue(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI,
                        ImsConfigContract.getNetworkTypeByFeature(
                                ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI));
                if ((volte == 0) && (vilte == 0) && (wfc == 0)) {
                    return true;
                } else {
                    return false;
                }
            } catch (ImsException e) {
                Log.e(TAG, "isAllFeatureFalse volte:" + volte + ", vilte:" + vilte + ", wfc:" + wfc);
                return false;
            }
        }

    }

    private static class ConfigHelper {
        private Context mContext = null;
        private ContentResolver mContentResolver = null;
        private int mPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        DefaultConfigPolicyFactory mDefConfigPolicyFactory = null;
        private int mOpCode = ImsConfigContract.Operator.OP_NONE;
        private Handler mHandler = null;
        private boolean mInitDone = false;

        ConfigHelper(Context context, Handler handler, int phoneId) {
            mContext = context;
            mHandler = handler;
            mPhoneId = phoneId;
            mContentResolver = mContext.getContentResolver();

            String opCode = null;
            try {
                opCode = getConfigSetting(ImsConfigContract.ConfigSetting.SETTING_ID_OPCODE);
                mOpCode = Integer.parseInt(opCode);
            } catch (ImsException e) {
                mOpCode = ImsConfigContract.Operator.OP_NONE;
            } catch (NumberFormatException e) {
                Log.e(TAG, "Parse SETTING_ID_OPCODE error: " + opCode);
                mOpCode = ImsConfigContract.Operator.OP_NONE;
            }
        }

        synchronized void setOpCode(int opCode) {
            mOpCode = opCode;
        }

        synchronized int getOpCode() {
            return mOpCode;
        }

        synchronized void setInitDone(boolean done) {
            mInitDone = done;

            // Send broadcast to notify observers that storage is restored.
            Intent intent = new Intent(ImsConfigContract.ACTION_CONFIG_LOADED);
            intent.putExtra(ImsConfigContract.EXTRA_PHONE_ID, mPhoneId);
            mContext.sendBroadcast(intent, android.Manifest.permission.READ_PHONE_STATE);
        }

        synchronized boolean isInitDone() {
            return mInitDone;
        }

        void init() {
            initDefaultStorage(ImsConfigContract.Operator.OP_DEFAULT);
            initMasterStorage();
        }

        void init(int opCode) {
            initDefaultStorage(opCode);
            initMasterStorage();
            // To remember config storage.
            initConfigSettingStorage(opCode);
        }

        public boolean isStorageInitialized() {
            boolean initialized = false;
            String[] projection = {
                    ImsConfigContract.ConfigSetting.PHONE_ID,
                    ImsConfigContract.ConfigSetting.SETTING_ID,
                    ImsConfigContract.ConfigSetting.VALUE};
            Cursor c = mContentResolver.query(
                    ImsConfigContract.ConfigSetting.getUriWithSettingId(mPhoneId,
                            ImsConfigContract.ConfigSetting.SETTING_ID_OPCODE),
                    projection, null, null, null);

            if (c != null && c.getCount() == 1) {
                initialized = true;
            }
            return initialized;
        }

        static void loadConfigStorage(Handler handler, int operatorCode) {
            if (handler != null) {
                // Remove previous load event
                handler.removeMessages(ImsConfigStorage.MSG_LOAD_CONFIG_STORAGE);
                Message msg = new Message();
                msg.what = ImsConfigStorage.MSG_LOAD_CONFIG_STORAGE;
                msg.obj = operatorCode;
                if (DEBUG) {
                    Log.d(TAG, "LoadConfigStorage() msg = " + msg.hashCode());
                }
                handler.sendMessage(msg);
            }
        }

        private void initConfigSettingStorage(int opCode) {
            addConfigSetting(ImsConfigContract.ConfigSetting.SETTING_ID_OPCODE,
                    Integer.toString(opCode));
        }

        private void addConfigSetting(int id, String value) {
            Uri result = null;
            ContentValues cv = new ContentValues();
            cv.put(ImsConfigContract.ConfigSetting.PHONE_ID, mPhoneId);
            cv.put(ImsConfigContract.ConfigSetting.SETTING_ID, id);
            cv.put(ImsConfigContract.ConfigSetting.VALUE, value);
            result = mContentResolver.insert(ImsConfigContract.ConfigSetting.CONTENT_URI, cv);
            if (result == null) {
                throw new IllegalArgumentException("addConfigSetting " + id +
                        " for phone " + mPhoneId + " failed!");
            }
        }

        private void updateConfigSetting(int id, int value) {
            ContentValues cv = new ContentValues();
            cv.put(ImsConfigContract.ConfigSetting.PHONE_ID, mPhoneId);
            cv.put(ImsConfigContract.ConfigSetting.SETTING_ID, id);
            cv.put(ImsConfigContract.ConfigSetting.VALUE, value);
            Uri uri = ImsConfigContract.getConfigUri(
                    ImsConfigContract.TABLE_CONFIG_SETTING, mPhoneId, id);
            int count = mContentResolver.update(uri, cv, null, null);
            if (count != 1) {
                throw new IllegalArgumentException("updateConfigSetting " + id +
                        " for phone " + mPhoneId + " failed!");
            }
        }

        private String getConfigSetting(int id) throws ImsException {
            Cursor c = null;
            String result = "";
            String[] projection = {
                    ImsConfigContract.ConfigSetting.PHONE_ID,
                    ImsConfigContract.ConfigSetting.SETTING_ID,
                    ImsConfigContract.ConfigSetting.VALUE};
            try {
                c = mContentResolver.query(
                        ImsConfigContract.ConfigSetting.getUriWithSettingId(mPhoneId, id),
                        projection, null, null, null);

                if (c != null && c.getCount() == 1) {
                    c.moveToFirst();
                    int index = c.getColumnIndex(ImsConfigContract.ConfigSetting.VALUE);
                    result = c.getString(index);
                    c.close();
                } else {
                    throw new ImsException("getConfigSetting " + id + " for phone " +
                            mPhoneId + " not found", ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
                }

            } catch (Exception e) {
                throw new ImsException("getConfigSetting " + id + " for phone " +
                            mPhoneId + " not found or something wrong with cursor",
                            ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
            } finally {
                if (c != null) {
                    c.close();
                }
            }

            return result;
        }

        private void initDefaultStorage(int opCode) {
            Map<Integer, ImsConfigSettings.Setting> configSettings =
                    ImsConfigSettings.getConfigSettings();
            Map<Integer, ImsConfigPolicy.DefaultConfig> defSettings =
                    new HashMap<Integer, ImsConfigPolicy.DefaultConfig>();

            mDefConfigPolicyFactory = DefaultConfigPolicyFactory.getInstanceByOpCode(opCode);
            defSettings = mDefConfigPolicyFactory.load();
            if (defSettings != null && defSettings.isEmpty()) {
                Log.d(TAG, "No default value");
                return;
            }

            for (Integer configId : configSettings.keySet()) {
                String value = ImsConfigContract.VALUE_NO_DEFAULT;
                int unitId = ImsConfigContract.Unit.UNIT_NONE;
                if (!mDefConfigPolicyFactory.hasDefaultValue(configId)) {
                    continue;
                }
                ImsConfigPolicy.DefaultConfig base = defSettings.get(configId);
                if (base != null) {
                    value = base.defVal;
                    unitId = base.unitId;
                }
                ImsConfigSettings.Setting setting = configSettings.get(configId);
                if (ImsConfigContract.MimeType.INTEGER == setting.mimeType) {
                    ContentValues cv = getConfigCv(
                            configId, setting.mimeType, Integer.parseInt(value));
                    cv.put(ImsConfigContract.Default.UNIT_ID, unitId);
                    mContentResolver.insert(ImsConfigContract.Default.CONTENT_URI, cv);
                } else if (ImsConfigContract.MimeType.STRING == setting.mimeType) {
                    ContentValues cv = getConfigCv(
                            configId, setting.mimeType, value);
                    cv.put(ImsConfigContract.Default.UNIT_ID, unitId);
                    mContentResolver.insert(ImsConfigContract.Default.CONTENT_URI, cv);
                } else { // For Object type
                    // ToDo: May be different Object type in IMS MO. ex. String.class.cast(obj)
                }
            }
        }

        private void initMasterStorage() {
            Map<Integer, ImsConfigSettings.Setting> configSettings =
                    ImsConfigSettings.getConfigSettings();

            for (Integer configId : configSettings.keySet()) {
                Cursor c = null;
                ContentValues cv = new ContentValues();
                boolean isFoundInNvRam = false;
                boolean isFoundInAny = true;

                try {
                    // Step 1: TODO: Phase II Get / Load provisioned value from RAM if exist
                    // isfoundInNvRAM = true;
                    throw new ImsException("here", ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
                } catch (ImsException e) {
                    // Step 2: Try to get default value.
                    try {
                        c = getConfigFirstCursor(ImsConfigContract.TABLE_DEFAULT, configId);
                        if (c != null) {
                            int phoneIdIndex =
                                    c.getColumnIndex(ImsConfigContract.BasicConfigTable.PHONE_ID);
                            int configIndex =
                                    c.getColumnIndex(ImsConfigContract.BasicConfigTable.CONFIG_ID);
                            int mimeTypeIndex =
                                    c.getColumnIndex(ImsConfigContract.Master.MIMETYPE_ID);
                            int dataIndex =
                                    c.getColumnIndex(ImsConfigContract.BasicConfigTable.DATA);
                            cv.put(ImsConfigContract.BasicConfigTable.PHONE_ID,
                                    c.getInt(phoneIdIndex));
                            cv.put(ImsConfigContract.BasicConfigTable.CONFIG_ID,
                                    c.getInt(configIndex));
                            cv.put(ImsConfigContract.BasicConfigTable.MIMETYPE_ID,
                                    c.getInt(mimeTypeIndex));
                            cv.put(ImsConfigContract.BasicConfigTable.DATA,
                                    c.getString(dataIndex));
                            Log.d(TAG, "Load default value " + c.getString(dataIndex) +
                                    " for config " + configId);
                            c.close();
                        }
                    } catch (ImsException e2) {
                        isFoundInAny = false;
                    } finally {
                        if (c != null) {
                            c.close();
                        }
                    }
                }
                if (!isFoundInAny || c == null) {
                    cv.put(ImsConfigContract.BasicConfigTable.PHONE_ID, mPhoneId);
                    cv.put(ImsConfigContract.BasicConfigTable.CONFIG_ID, configId);
                    cv.put(ImsConfigContract.BasicConfigTable.MIMETYPE_ID,
                            ImsConfigSettings.getMimeType(configId));
                    cv.put(ImsConfigContract.BasicConfigTable.DATA,
                            ImsConfigContract.VALUE_NO_DEFAULT);
                }

                // Step 3: Add to master db
                mContentResolver.insert(ImsConfigContract.Master.CONTENT_URI, cv);

                // Step4: Record provisioned data if found any
                if (isFoundInNvRam) {
                    mContentResolver.insert(ImsConfigContract.Provision.CONTENT_URI, cv);
                }
            }
        }

        private void clear() {
            String selection = ImsConfigContract.BasicConfigTable.PHONE_ID + " = ?";
            String[] args = {String.valueOf(mPhoneId)};
            mContentResolver.delete(ImsConfigContract.ConfigSetting.CONTENT_URI, selection, args);
            mContentResolver.delete(ImsConfigContract.Provision.CONTENT_URI, selection, args);
            mContentResolver.delete(ImsConfigContract.Master.CONTENT_URI, selection, args);
            mContentResolver.delete(ImsConfigContract.Default.CONTENT_URI, selection, args);
        }
        private ContentValues getConfigCv(int configId, int mimeType, int value) {
            ContentValues cv = new ContentValues();
            cv.put(ImsConfigContract.BasicConfigTable.PHONE_ID, mPhoneId);
            cv.put(ImsConfigContract.BasicConfigTable.CONFIG_ID, configId);
            cv.put(ImsConfigContract.BasicConfigTable.MIMETYPE_ID, mimeType);
            cv.put(ImsConfigContract.BasicConfigTable.DATA, value);
            return cv;
        }

        private ContentValues getConfigCv(int configId, int mimeType, String value) {
            ContentValues cv = new ContentValues();
            cv.put(ImsConfigContract.BasicConfigTable.PHONE_ID, mPhoneId);
            cv.put(ImsConfigContract.BasicConfigTable.CONFIG_ID, configId);
            cv.put(ImsConfigContract.BasicConfigTable.MIMETYPE_ID, mimeType);
            cv.put(ImsConfigContract.BasicConfigTable.DATA, value);
            return cv;
        }

        private Uri addConfig(String table, int configId, int mimeType, int value)
                throws ImsException {
            enforceConfigId(configId);

            ContentValues cv = getConfigCv(configId, mimeType, value);
            return mContentResolver.insert(ImsConfigContract.getTableUri(table), cv);
        }

        private Uri addConfig(String table, int configId, int mimeType, String value)
                throws ImsException {
            enforceConfigId(configId);

            ContentValues cv = getConfigCv(configId, mimeType, value);
            return mContentResolver.insert(ImsConfigContract.getTableUri(table), cv);
        }

        private int updateConfig(String table, int configId, int mimeType, int value)
                throws ImsException {
            enforceConfigId(configId);

            ContentValues cv = new ContentValues();
            cv.put(ImsConfigContract.BasicConfigTable.PHONE_ID, mPhoneId);
            cv.put(ImsConfigContract.BasicConfigTable.CONFIG_ID, configId);
            cv.put(ImsConfigContract.BasicConfigTable.MIMETYPE_ID, mimeType);
            cv.put(ImsConfigContract.BasicConfigTable.DATA, value);

            return mContentResolver.update(
                    ImsConfigContract.getConfigUri(table, mPhoneId, configId), cv, null, null);
        }

        private int updateConfig(String table, int configId, int mimeType, String value)
                throws ImsException {
            enforceConfigId(configId);

            ContentValues cv = new ContentValues();
            cv.put(ImsConfigContract.BasicConfigTable.PHONE_ID, mPhoneId);
            cv.put(ImsConfigContract.BasicConfigTable.CONFIG_ID, configId);
            cv.put(ImsConfigContract.BasicConfigTable.MIMETYPE_ID, mimeType);
            cv.put(ImsConfigContract.BasicConfigTable.DATA, value);

            return mContentResolver.update(
                    ImsConfigContract.getConfigUri(table, mPhoneId, configId), cv, null, null);
        }

        private Cursor getConfigFirstCursor(String table, int configId) throws ImsException {
            String[] projection = {
                    ImsConfigContract.BasicConfigTable.PHONE_ID,
                    ImsConfigContract.BasicConfigTable.CONFIG_ID,
                    ImsConfigContract.BasicConfigTable.MIMETYPE_ID,
                    ImsConfigContract.BasicConfigTable.DATA};
            Uri uri = ImsConfigContract.getConfigUri(table, mPhoneId, configId);
            Cursor c = mContentResolver.query(uri, projection, null, null, null);
            if (c != null) {
                if (c.getCount() == 1) {
                    c.moveToFirst();
                    return c;
                } else if (c.getCount() == 0) {
                    c.close();
                    throw new ImsException("Config " + configId +
                            " shall exist in table: " + table,
                            ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
                } else {
                    c.close();
                    throw new ImsException("Config " + configId +
                            " shall exist once in table: " + table,
                            ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
                }
            } else {
                throw new ImsException("Null cursor with config: " +
                        configId + " in table: " + table,
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
            }
        }

        private int getConfigValue(String table, int configId) throws ImsException {
            int dataIndex, mimeTypeIndex, mimeType = -1, result;
            Cursor c = null;
            enforceConfigId(configId);

            try {
                c = getConfigFirstCursor(table, configId);
                dataIndex = c.getColumnIndex(ImsConfigContract.BasicConfigTable.DATA);
                mimeTypeIndex = c.getColumnIndex(ImsConfigContract.Master.MIMETYPE_ID);
                mimeType = c.getInt(mimeTypeIndex);

                enforceDefaultValue(configId, c.getString(dataIndex));
                if (mimeType != ImsConfigContract.MimeType.INTEGER) {
                    throw new ImsException("Config " + configId +
                            " shall be type " + ImsConfigContract.MimeType.INTEGER +
                            ", but " + mimeType, ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
                }
                result = Integer.parseInt(c.getString(dataIndex));

            } catch (Exception e) {
                throw new ImsException("Config " + configId +
                        " shall be type " + ImsConfigContract.MimeType.INTEGER +
                        ", but " + mimeType + " or something wrong with cursor",
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
            } finally {
                if (c != null) {
                    c.close();
                }
            }

            return result;
        }

        private String getConfigStringValue(String table, int configId) throws ImsException {
            int dataIndex, mimeTypeIndex, mimeType = -1;
            String result;
            Cursor c = null;
            enforceConfigId(configId);

            try {
                c = getConfigFirstCursor(table, configId);
                dataIndex = c.getColumnIndex(ImsConfigContract.BasicConfigTable.DATA);
                mimeTypeIndex = c.getColumnIndex(ImsConfigContract.Master.MIMETYPE_ID);
                mimeType = c.getInt(mimeTypeIndex);

                enforceDefaultValue(configId, c.getString(dataIndex));
                if (mimeType != ImsConfigContract.MimeType.STRING) {
                    throw new ImsException("Config " + configId +
                            " shall be type " + ImsConfigContract.MimeType.STRING +
                            ", but " + mimeType, ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
                }
                result = c.getString(dataIndex);

            } catch (Exception e) {
                throw new ImsException("Config " + configId +
                        " shall be type " + ImsConfigContract.MimeType.STRING +
                        ", but " + mimeType + " or something wrong with cursor",
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
            } finally {
                if (c != null) {
                    c.close();
                }
            }

            return result;
        }

        private void enforceDefaultValue(int configId, String data) throws ImsException {
            if (ImsConfigContract.VALUE_NO_DEFAULT.equals(data)) {
                throw new ImsException("No deafult value for config " +
                        configId, ImsReasonInfo.CODE_UNSPECIFIED);
            }
        }

        private void enforceConfigId(int configId) throws ImsException {
            if (!ImsConfigContract.Validator.isValidConfigId(configId)) {
                throw new ImsException("No deafult value for config " +
                        configId, ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
            }
        }
    }

    private static class ResourceHelper {
        private int mPhoneId;
        private Context mContext = null;
        private ContentResolver mContentResolver = null;

        ResourceHelper(Context context, int phoneId) {
            mPhoneId = phoneId;
            mContext = context;
            mContentResolver = mContext.getContentResolver();
        }

        private void clear() {
            String selection = ImsConfigContract.BasicConfigTable.PHONE_ID + " = ?";
            String[] args = {String.valueOf(mPhoneId)};
            mContentResolver.delete(ImsConfigContract.Resource.CONTENT_URI, selection, args);
        }

        private void updateResource(int featureId, int value) {
            int curValue = -1;

            ContentValues cv = new ContentValues();
            cv.put(ImsConfigContract.Resource.PHONE_ID, mPhoneId);
            cv.put(ImsConfigContract.Resource.FEATURE_ID, featureId);
            cv.put(ImsConfigContract.Resource.VALUE, value);

            // Check exist or not
            try {
                curValue = getResourceValue(featureId);
                Log.d(TAG, "updateResource() comparing: curValue: " + curValue + ", value:" + value);

                String selection = ImsConfigContract.Resource.PHONE_ID + "=? AND "
                        + ImsConfigContract.Resource.FEATURE_ID +"=?";
                String[] args = {String.valueOf(mPhoneId), String.valueOf(featureId)};
                mContentResolver.update(ImsConfigContract.Resource.CONTENT_URI, cv, selection, args);

            } catch (ImsException e) {
                Log.e(TAG, "updateResource() ImsException featureId:" + featureId +", value:" + value);
                mContentResolver.insert(ImsConfigContract.Resource.CONTENT_URI, cv);
            }
        }

        int getResourceValue(int featureId) throws ImsException {
            Cursor c = null;
            int result = -1;
            String[] projection = {
                    ImsConfigContract.Resource.PHONE_ID,
                    ImsConfigContract.Resource.FEATURE_ID,
                    ImsConfigContract.Resource.VALUE};

            try {
                c = mContentResolver.query(
                        ImsConfigContract.Resource.getUriWithFeatureId(mPhoneId, featureId),
                        projection, null, null, null);

                if (c != null && c.getCount() == 1) {
                    c.moveToFirst();
                    int valueIndex = c.getColumnIndex(ImsConfigContract.Resource.VALUE);
                    result = c.getInt(valueIndex);
                    c.close();
                } else {
                    throw new ImsException("Feature " + featureId + " not assigned with res value!",
                            ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
                }
            } catch (Exception e) {
                throw new ImsException("Feature " + featureId + " not assigned with res value" +
                        " or something wrong with cursor",
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
            } finally {
                if (c != null) {
                    c.close();
                }
            }

            return result;
        }

    }

}
