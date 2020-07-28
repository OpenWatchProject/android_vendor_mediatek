package com.mediatek.ims.config.internal;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.CarrierConfigManager;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.ims.ImsReasonInfo;
import android.text.TextUtils;
import com.mediatek.ims.ImsCommonUtil;
import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.ims.config.ImsConfigContract;
import com.mediatek.ims.MtkImsConstants;
import com.mediatek.ims.plugin.ExtensionFactory;
import com.mediatek.ims.plugin.ImsManagerOemPlugin;
import com.mediatek.ims.ril.ImsCommandsInterface;

public class ImsConfigController {
    private static final String TAG = "ImsConfigController";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean DEBUG = TextUtils.equals(Build.TYPE, "eng") || (SystemProperties
            .getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    // For add-on decoupling
    private static final String ACTION_CXP_NOTIFY_FEATURE =
        "com.mediatek.common.carrierexpress.cxp_notify_feature";

    private String mLogTag;


    private int mPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
    private ImsCommandsInterface mRilAdapter;
    private Context mContext = null;

    private Handler mHandler;
    private Handler mEventHandler;
    private BroadcastReceiver mReceiver;


    ///M: IMS config locks
    private Object mFeatureValueLock = new Object();
    private Object mProvisionedValueLock = new Object();
    private Object mResourceValueLock = new Object();
    private Object mWfcLock = new Object();
    private Object mMdCfgLock = new Object();

    private int mCurWfcMode = -1;

    private ImsManagerOemPlugin mImsManagerOemPlugin = null;

    /// IMS MSG
    static final int MSG_IMS_GET_PROVISION_DONE = 101;
    static final int MSG_IMS_SET_PROVISION_DONE = 102;
    static final int MSG_IMS_GET_FEATURE_DONE = 103;
    static final int MSG_IMS_SET_FEATURE_DONE = 104;

    static final int MSG_IMS_GET_RESOURCE_DONE = 106;

    static final int MSG_IMS_SET_MDCFG_DONE = 107;

    static final int MSG_RESET_WFC_MODE_FLAG = 108;
    // Update IMS service config
    static final int MSG_UPDATE_IMS_SERVICE_CONFIG = 109;


    /// URC
    static final int EVENT_IMS_CFG_DYNAMIC_IMS_SWITCH_COMPLETE = 1001;
    static final int EVENT_IMS_CFG_FEATURE_CHANGED = 1002;
    static final int EVENT_IMS_CFG_CONFIG_CHANGED = 1003;
    static final int EVENT_IMS_CFG_CONFIG_LOADED = 1004;

    /// error code
    static final int CONFIG_CMD_SUCCESS = 1;
    static final int CONFIG_CMD_ERROR = 2;
    static final int CONFIG_TIMEOUT_ERROR = 3;
    static final int CONFIG_INTERRUPT_ERROR = 4;


    /// M: IMS Provisioning. @{
    private class ProvisioningResult {
        String provisionInfo;
        int provisionResult = CONFIG_TIMEOUT_ERROR;
        Object lockObj = new Object();
    }
    /// @}

    private class MdConfigResult {
        int requestConfigNum = 0;
        int[] resultArray = null;
        int configResult = CONFIG_TIMEOUT_ERROR;
        Object lockObj = new Object();
    }

    private class FeatureResult {
        int featureResult;
        int featureValue;
        Object lockObj = new Object();
    }

    private ImsConfigController() {
    }

    public ImsConfigController(Context context, int phoneId, ImsCommandsInterface imsRilAdapter) {
        mContext = context;
        mPhoneId = phoneId;
        mRilAdapter = imsRilAdapter;

        mLogTag = TAG + "[" + phoneId + "]";

        // use two threads to handle different event type
        HandlerThread configThread = new HandlerThread("ImsConfigThread-" + mPhoneId);
        configThread.start();

        HandlerThread eventThread = new HandlerThread("ImsEventThread-" + mPhoneId);
        eventThread.start();

        mHandler = new EventHandler(mPhoneId, configThread.getLooper());
        mEventHandler = new EventHandler(mPhoneId, eventThread.getLooper());

        // Setup a receiver observes to notify reloading when sim changed.
        mReceiver = new ImsConfigEventReceiver(mEventHandler, mPhoneId, mRilAdapter);

        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        filter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
        filter.addAction(ACTION_CXP_NOTIFY_FEATURE);
        filter.addAction(MtkImsConstants.ACTION_MTK_MMTEL_READY);

        if (ImsCommonUtil.isDssNoResetSupport()) {
            filter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
        }
        mContext.registerReceiver(mReceiver, filter);

        mRilAdapter.registerForImsCfgDynamicImsSwitchComplete(mHandler,
                EVENT_IMS_CFG_DYNAMIC_IMS_SWITCH_COMPLETE, null);
        mRilAdapter.registerForImsCfgConfigChanged(mHandler,
                EVENT_IMS_CFG_CONFIG_CHANGED, null);
        mRilAdapter.registerForImsCfgFeatureChanged(mHandler,
                EVENT_IMS_CFG_FEATURE_CHANGED, null);
        mRilAdapter.registerForImsCfgConfigLoaded(mHandler,
                EVENT_IMS_CFG_CONFIG_LOADED, null);
    }

    private String messageToString(int msg) {
        switch (msg) {
            case MSG_IMS_GET_PROVISION_DONE:
                return "MSG_IMS_GET_PROVISION_DONE";
            case MSG_IMS_SET_PROVISION_DONE:
                return "MSG_IMS_SET_PROVISION_DONE";
            case MSG_IMS_GET_FEATURE_DONE:
                return "MSG_IMS_GET_FEATURE_DONE";
            case MSG_IMS_SET_FEATURE_DONE:
                return "MSG_IMS_SET_FEATURE_DONE";
            case MSG_IMS_GET_RESOURCE_DONE:
                return "MSG_IMS_GET_RESOURCE_DONE";
            case MSG_IMS_SET_MDCFG_DONE:
                return "MSG_IMS_SET_MDCFG_DONE";
            case MSG_RESET_WFC_MODE_FLAG:
                return "MSG_RESET_WFC_MODE_FLAG";
            case MSG_UPDATE_IMS_SERVICE_CONFIG:
                return "MSG_UPDATE_IMS_SERVICE_CONFIG";
            case EVENT_IMS_CFG_DYNAMIC_IMS_SWITCH_COMPLETE:
                return "EVENT_IMS_CFG_DYNAMIC_IMS_SWITCH_COMPLETE";
            case EVENT_IMS_CFG_FEATURE_CHANGED:
                return "EVENT_IMS_CFG_FEATURE_CHANGED";
            case EVENT_IMS_CFG_CONFIG_CHANGED:
                return "EVENT_IMS_CFG_CONFIG_CHANGED";
            case EVENT_IMS_CFG_CONFIG_LOADED:
                return "EVENT_IMS_CFG_CONFIG_LOADED";
            default:
                return "" + msg;
        }

    }
    class EventHandler extends Handler {

        private int mPhoneId;

        EventHandler(int phoneId, Looper looper) {
            super(looper);
            mPhoneId = phoneId;
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_IMS_SET_FEATURE_DONE: {
                    AsyncResult ar = (AsyncResult) msg.obj;
                    ImsConfigController.FeatureResult result = (ImsConfigController
                            .FeatureResult) ar.userObj;
                    synchronized (result.lockObj) {
                        if (ar.exception != null) {
                            // Retry to send AT cmd again when failure in future
                            result.featureResult = CONFIG_CMD_ERROR;
                            Rlog.e(mLogTag, "MSG_IMS_SET_FEATURE_DONE: error ret null, e=" + ar
                                    .exception);
                        } else {
                            result.featureResult = CONFIG_CMD_SUCCESS;

                            if (DEBUG)
                                Rlog.d(mLogTag, "MSG_IMS_SET_FEATURE_DONE: Finish set feature!");
                        }
                        result.lockObj.notify();
                    }
                }
                break;

                case MSG_IMS_GET_FEATURE_DONE: {
                    AsyncResult ar = (AsyncResult) msg.obj;
                    ImsConfigController.FeatureResult result = (ImsConfigController
                            .FeatureResult) ar.userObj;
                    synchronized (result.lockObj) {
                        if (ar.exception != null) {
                            result.featureResult = CONFIG_CMD_ERROR;
                            Rlog.e(mLogTag, "MSG_IMS_GET_FEATURE_DONE: error ret null, e=" + ar
                                    .exception);
                        } else {
                            result.featureValue = (int) ar.result;
                            result.featureResult = CONFIG_CMD_SUCCESS;

                            if (DEBUG)
                                Rlog.d(mLogTag, "MSG_IMS_GET_FEATURE_DONE: featureValue:" +
                                        result.featureValue);
                        }
                        result.lockObj.notify();
                    }
                }
                break;

                case MSG_IMS_GET_PROVISION_DONE: {
                    AsyncResult ar = (AsyncResult) msg.obj;
                    ImsConfigController.ProvisioningResult result = (ImsConfigController
                            .ProvisioningResult) ar.userObj;
                    synchronized (result.lockObj) {
                        if (ar.exception != null) {
                            result.provisionResult = CONFIG_CMD_ERROR;
                            Rlog.e(mLogTag, "MSG_IMS_GET_PROVISION_DONE: error ret null, e=" + ar
                                    .exception);
                        } else {
                            ar.result =
                            result.provisionInfo = (String) ar.result;
                            result.provisionResult = CONFIG_CMD_SUCCESS;

                            if (DEBUG)
                                Rlog.d(mLogTag, "MSG_IMS_GET_PROVISION_DONE: provisionInfo:" +
                                        result.provisionInfo);
                        }
                        result.lockObj.notify();
                    }
                }
                break;

                case MSG_IMS_SET_PROVISION_DONE: {
                    AsyncResult ar = (AsyncResult) msg.obj;
                    ImsConfigController.ProvisioningResult result = (ImsConfigController
                            .ProvisioningResult) ar.userObj;
                    synchronized (result.lockObj) {
                        if (ar.exception != null) {
                            // Retry to send AT cmd again when failure in future
                            result.provisionResult = CONFIG_CMD_ERROR;
                            Rlog.e(mLogTag, "MSG_IMS_SET_PROVISION_DONE: error ret null, e=" + ar
                                    .exception);
                        } else {
                            result.provisionResult = CONFIG_CMD_SUCCESS;

                            if (DEBUG)
                                Rlog.d(mLogTag, "MSG_IMS_SET_PROVISION_DONE: Finish set provision!");
                        }
                        result.lockObj.notify();
                    }
                }
                break;

                case MSG_IMS_GET_RESOURCE_DONE: {
                    AsyncResult ar = (AsyncResult) msg.obj;
                    ImsConfigController.FeatureResult result = (ImsConfigController
                            .FeatureResult) ar.userObj;
                    synchronized (result.lockObj) {
                        if (ar.exception != null) {
                            result.featureResult = CONFIG_CMD_ERROR;
                            Rlog.e(mLogTag, "MSG_IMS_GET_RESOURCE_DONE: error ret null, e=" + ar
                                    .exception);
                        } else {
                            int[] value = (int[]) ar.result;
                            result.featureValue = value[0];
                            result.featureResult = CONFIG_CMD_SUCCESS;
                        }
                        result.lockObj.notify();
                    }
                }
                break;

                case MSG_RESET_WFC_MODE_FLAG:
                    resetWfcModeFlag();
                    break;

                case MSG_IMS_SET_MDCFG_DONE: {
                    AsyncResult ar = (AsyncResult) msg.obj;
                    ImsConfigController.MdConfigResult cfgResult = (ImsConfigController.MdConfigResult)
                            ar.userObj;
                    String resultStr;
                    synchronized (cfgResult.lockObj) {
                        if (ar.exception != null) {
                            int[] errorResult = new int[cfgResult.requestConfigNum];
                            for (int i = 0; i < errorResult.length; i++) {
                                errorResult[i] = -1;
                            }
                            // Retry to send AT cmd again when failure in future
                            cfgResult.resultArray = errorResult;
                            cfgResult.configResult = CONFIG_CMD_ERROR;
                            Rlog.e(mLogTag, "SET_MDCFG_DONE, error ret, e=" + ar.exception);
                        } else {
                            resultStr = (String) ar.result;

                            String[] resultStrArray = resultStr.split(",");
                            int[] resultIntArray = new int[resultStrArray.length];
                            for (int i = 0; i < resultStrArray.length; i++) {
                                resultIntArray[i] = Integer.parseInt(resultStrArray[i]);
                            }
                            cfgResult.resultArray = resultIntArray;

                            cfgResult.configResult = CONFIG_CMD_SUCCESS;

                            if (DEBUG)
                                Rlog.d(mLogTag, "SET_MDCFG_DONE, finish set MD Ims config!");
                        }
                        cfgResult.lockObj.notify();
                    }
                }
                break;

                case MSG_UPDATE_IMS_SERVICE_CONFIG: {
                    if (mImsManagerOemPlugin == null) {
                        mImsManagerOemPlugin = ExtensionFactory.makeOemPluginFactory(mContext)
                                .makeImsManagerPlugin(mContext);
                    }

                    mImsManagerOemPlugin.updateImsServiceConfig(mContext, ImsCommonUtil
                            .getMainCapabilityPhoneId(), true);
                }
                break;

                case EVENT_IMS_CFG_DYNAMIC_IMS_SWITCH_COMPLETE: {
                    // Use to notify App to check platform support status again
                    Intent intent = new Intent(ImsConfigContract
                            .ACTION_DYNAMIC_IMS_SWITCH_COMPLETE);
                    intent.putExtra(PhoneConstants.PHONE_KEY, mPhoneId);
                    mContext.sendBroadcast(intent, android.Manifest.permission.READ_PHONE_STATE);

                    if (DEBUG)
                        Rlog.d(mLogTag, "DYNAMIC_IMS_SWITCH_COMPLETE phoneId:" + mPhoneId);
                }
                break;

                case EVENT_IMS_CFG_CONFIG_CHANGED:
                    // AOSP will broadcast this, so do nothing here
                    break;

                case EVENT_IMS_CFG_FEATURE_CHANGED: {
                    AsyncResult ar = (AsyncResult) msg.obj;
                    int[] value = (int[]) ar.result;
                    Intent intent = new Intent(ImsConfig.ACTION_IMS_FEATURE_CHANGED);
                    intent.putExtra(ImsConfigContract.EXTRA_PHONE_ID, value[0]);
                    intent.putExtra(ImsConfig.EXTRA_CHANGED_ITEM, value[1]);
                    intent.putExtra(ImsConfig.EXTRA_NEW_VALUE, value[2]);
                    mContext.sendBroadcast(intent);

                    if (DEBUG)
                        Rlog.d(TAG, "EVENT_IMS_CFG_FEATURE_CHANGED: phoneId = " + value[0] +
                                " feature =" + value[1] + " value=" + value[2]);
                }
                break;

                case EVENT_IMS_CFG_CONFIG_LOADED: {
                    // Use to notify App to check platform support status again
                    Intent intent = new Intent(ImsConfigContract
                            .ACTION_CONFIG_LOADED);
                    intent.putExtra(PhoneConstants.PHONE_KEY, mPhoneId);
                    mContext.sendBroadcast(intent, android.Manifest.permission.READ_PHONE_STATE);

                    if (DEBUG)
                        Rlog.d(mLogTag, "EVENT_IMS_CFG_CONFIG_LOADED phoneId:" + mPhoneId);
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
     * @param network,   defined as in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @return void
     * @throws ImsException if calling the IMS service results in an error.
     */
    public int getFeatureValue(int featureId, int network) throws ImsException {
        synchronized (mFeatureValueLock) {
            FeatureResult result = new FeatureResult();
            Message msg = mHandler.obtainMessage(MSG_IMS_GET_FEATURE_DONE, result);

            synchronized (result.lockObj) {
                mRilAdapter.getImsCfgFeatureValue(featureId, network, msg);
                try {
                    result.lockObj.wait(10000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    result.featureResult = CONFIG_INTERRUPT_ERROR;
                }
            }

            if (!isConfigSuccess(result.featureResult)) {
                throw new ImsException("Something wrong, reason:" + result.featureResult,
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
            }
            return result.featureValue;
        }
    }

    /**
     * Sets the value for IMS feature item for specified network type.
     *
     * @param featureId, as defined in FeatureConstants.
     * @param network,   as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @param value,     as defined in FeatureValueConstants.
     * @param isLast, is last command of set multiple feature values.
     * @return void
     * @throws ImsException if calling the IMS service results in an error.
     */
    public void setFeatureValue(int featureId, int network, int value, int isLast)
            throws ImsException {
        synchronized(mFeatureValueLock) {
            ImsConfigController.FeatureResult result = new ImsConfigController.FeatureResult();
            Message msg = mHandler.obtainMessage(MSG_IMS_SET_FEATURE_DONE, result);
            synchronized(result.lockObj) {
                mRilAdapter.setImsCfgFeatureValue(featureId, network, value, isLast, msg);
                try {
                    result.lockObj.wait(10000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    result.featureResult = CONFIG_INTERRUPT_ERROR;
                }

                if (!isConfigSuccess(result.featureResult)) {
                    throw new ImsException("Something wrong, reason:" + result.featureResult,
                            ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
                }
            }
        }
    }

    /**
     * API to retrieve provisioned int value of IMS configurations.
     *
     * @param configId The id defined in ImsConfig.ConfigConstants.
     * @return The int type configuration value.
     * @throws ImsException with following reason code
     *                      1.) CODE_LOCAL_ILLEGAL_ARGUMENT if the configId can't match any data.
     *                      2.) CODE_UNSPECIFIED if the config is without setting any value, even
     *                      default.
     */
    public String getProvisionedValue(int configId) throws ImsException {
        synchronized (mProvisionedValueLock) {
            ProvisioningResult result = new ProvisioningResult();

            Message msg = mHandler.obtainMessage(MSG_IMS_GET_PROVISION_DONE, result);
            synchronized (result.lockObj) {
                mRilAdapter.getImsCfgProvisionValue(configId, msg);
                try {
                    result.lockObj.wait(10000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    result.provisionResult = CONFIG_INTERRUPT_ERROR;
                }
            }
            if (!isConfigSuccess(result.provisionResult)) {
                throw new ImsException("Something wrong, reason:" + result.provisionResult,
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
            }
            return result.provisionInfo;
        }
    }

    /**
     * Sets the value for IMS service/capabilities parameters by
     * the operator device management entity.
     * This function should not be called from main thread as it could block
     * mainthread.
     *
     * @param configId, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @param value     in Integer format.
     * @return as defined in com.android.ims.ImsConfig#OperationStatusConstants
     * @throws ImsException if calling the IMS service results in an error.
     */
    public void setProvisionedValue(int configId, String value) throws ImsException {
        synchronized (mProvisionedValueLock) {
            ProvisioningResult result = new ProvisioningResult();
            Message msg = mHandler.obtainMessage(MSG_IMS_SET_PROVISION_DONE, result);
            synchronized (result.lockObj) {
                mRilAdapter.setImsCfgProvisionValue(configId, value, msg);
                try {
                    result.lockObj.wait(10000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    result.provisionResult = CONFIG_INTERRUPT_ERROR;
                }
            }
            if (!isConfigSuccess(result.provisionResult)) {
                throw new ImsException("Something wrong, reason:" + result.provisionResult,
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
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
     * @param value     in String format.
     * @return as defined in com.android.ims.ImsConfig#OperationStatusConstants
     * @throws ImsException if calling the IMS service results in an error.
     */
    public void setProvisionedStringValue(int configId, String value) throws ImsException {
        setProvisionedValue(configId, value);
    }

    public synchronized int getImsResCapability(int featureId) throws ImsException {
        synchronized (mResourceValueLock) {
            FeatureResult result = new FeatureResult();
            Message msg = mHandler.obtainMessage(MSG_IMS_GET_RESOURCE_DONE, result);

            synchronized (result.lockObj) {
                mRilAdapter.getImsCfgResourceCapValue(featureId,msg);
                try {
                    result.lockObj.wait(10000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    result.featureResult = CONFIG_INTERRUPT_ERROR;
                }
            }

            if (!isConfigSuccess(result.featureResult)) {
                throw new ImsException("Something wrong, reason:" + result.featureResult,
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
            }
            return result.featureValue;
        }
    }

    private static boolean isConfigSuccess(int reason) {
        boolean isSuccess = false;
        switch (reason) {
            case CONFIG_CMD_SUCCESS:
                isSuccess = true;
                break;
            default:
                // do nothing...
                break;
        }
        return isSuccess;
    }
    /// @}


    /**
     * Sync Volte preference for voice domain to modem.
     */
    public synchronized void setVoltePreference(int mode) {
        Rlog.i(mLogTag, "setVoltePreference mode:" + mode + ", phoneId:" + mPhoneId);
        mRilAdapter.setVoiceDomainPreference(mode, null);
    }

    public int[] setModemImsCfg(String[] keys, String[] values, int type) {
        synchronized (mMdCfgLock) {
            String keysStr = null;
            String valuesStr = null;
            int[] resultArray = null;

            // need to transfer String[] keys to keys String
            if (keys == null) {
                Rlog.d(mLogTag, "keys is null, return null");
                return null;
            }

            if (keys.length < 1 || values.length < 1) {
                Rlog.d(mLogTag, "keys or values length is smaller than 1, return null");
                return null;
            }

            if (keys.length == values.length) {
                Rlog.d(mLogTag, "keys and values length equals");
                keysStr = ImsConfigUtils.arrayToString(keys);
                valuesStr = ImsConfigUtils.arrayToString(values);
                Rlog.d(mLogTag, "keysStr:" + keysStr + ", valuesStr:" + valuesStr);

                MdConfigResult cfgResult = new MdConfigResult();
                cfgResult.requestConfigNum = keys.length;

                Message msg = mHandler.obtainMessage(MSG_IMS_SET_MDCFG_DONE, cfgResult);
                synchronized (cfgResult.lockObj) {
                    // should wait for callback to return
                    mRilAdapter.setModemImsCfg(keysStr, valuesStr, type, msg);
                    try {
                        cfgResult.lockObj.wait(10000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                        cfgResult.configResult = CONFIG_INTERRUPT_ERROR;
                    }
                }
                resultArray = cfgResult.resultArray;
            } else {
                Rlog.d(mLogTag, "keys and values length not equals");
                return null;
            }
            return resultArray;
        }
    }

    private void resetWfcModeFlag() {
        Rlog.d(mLogTag, "resetWfcModeFlag()");
        synchronized (mWfcLock) {
            mCurWfcMode = -1;
        }
    }

    /**
     * Sync WFC preferred mode to modem.
     */
    public void sendWfcProfileInfo(int rilWfcMode) {
        synchronized(mWfcLock) {
            Rlog.i(mLogTag, "sendWfcProfileInfo rilWfcMode:" + rilWfcMode +
                    ", mCurWfcMode:" + mCurWfcMode);
            if (rilWfcMode != mCurWfcMode) {
                mRilAdapter.sendWfcProfileInfo(rilWfcMode, null);

                if (rilWfcMode == 3) {   // RIL define wfc mode 3 is WIFI ONLY
                    if (ImsConfigUtils.isWfcEnabledByUser(mContext, mPhoneId)) {
                        if (DEBUG) Rlog.d(mLogTag, "Wifi-only and WFC setting enabled, " +
                                "send intent to turn radio OFF");
                        ImsConfigUtils.sendWifiOnlyModeIntent(mContext, mPhoneId, true);
                    } else {
                        if (DEBUG) Rlog.d(mLogTag, "Wifi-only and WFC setting disabled, " +
                                "send intent to turn radio ON");
                        ImsConfigUtils.sendWifiOnlyModeIntent(mContext, mPhoneId, false);
                    }
                } else {
                    if (DEBUG) Rlog.d(mLogTag, "Not wifi-only mode, turn radio ON");
                    ImsConfigUtils.sendWifiOnlyModeIntent(mContext, mPhoneId, false);
                }

                mCurWfcMode = rilWfcMode;
            }
        }
    }

}
