package com.mediatek.ims.config.internal;

import android.content.Context;
import android.os.Binder;
import android.os.Build;
import android.os.Process;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.telephony.ims.stub.ImsConfigImplBase;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.Rlog;

import com.android.ims.ImsConfig;
import com.android.ims.ImsConfigListener;
import com.android.ims.ImsException;
import com.android.ims.internal.IImsConfig;

import com.android.internal.telephony.IccCardConstants;

import com.mediatek.ims.internal.IMtkImsConfig;
import com.mediatek.ims.ril.ImsCommandsInterface;
import com.mediatek.ims.ImsCommonUtil;

import java.util.HashMap;

/**
 * Class handles IMS parameter provisioning by carrier.
 *
 *  @hide
 */
public class MtkImsConfigImpl extends IMtkImsConfig.Stub {
    private static final String TAG = "MtkImsConfigImpl";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean DEBUG = TextUtils.equals(Build.TYPE, "eng")
            || (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private Context mContext;
    private int mPhoneId;
    private ImsCommandsInterface mRilAdapter;
    private ImsConfigAdapter mConfigAdapter;
    private final IImsConfig mImsConfig;

    /// M: Dynamic IMS Switch @{
    /// <ImsConfig.FeatureConstants, ImsConfig.FeatureValueConstants>
    private HashMap<Integer, Boolean> mImsCapabilitiesIsCache =
            new HashMap<Integer, Boolean>();
    private HashMap<Integer, Integer> mImsCapabilities =
            new HashMap<Integer, Integer>();
    /// @}

    /**
     *
     * Construction function for MtkImsConfigImpl.
     *
     * @param context the application context
     * @param phoneId the phone id this instance handle for
     *
     */
    public MtkImsConfigImpl(Context context, ImsCommandsInterface imsRilAdapter,
            IImsConfig imsConfig, ImsConfigAdapter adapter, int phoneId) {
        mContext = context;
        mPhoneId = phoneId;
        mRilAdapter = imsRilAdapter;
        mImsConfig = imsConfig;
        mConfigAdapter = adapter;

        // M: Dynamic IMS Switch, default volte only. @{
        mImsCapabilities.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE,
                ImsConfig.FeatureValueConstants.ON);
        mImsCapabilities.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE,
                ImsConfig.FeatureValueConstants.OFF);
        mImsCapabilities.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI,
                ImsConfig.FeatureValueConstants.OFF);
        /// @}

        // M: Use cache to reduce access DB frequently. @{
        mImsCapabilitiesIsCache.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE, false);
        mImsCapabilitiesIsCache.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE, false);
        mImsCapabilitiesIsCache.put(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI, false);
        /// @}
    }

    /**
     * Gets the value for ims service/capabilities parameters from the provisioned
     * value storage. Synchronous blocking call.
     *
     * @param item, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @return value in Integer format.
     */
    @Override
    public int getProvisionedValue(int item) {
        try {
            int result = mImsConfig.getProvisionedValue(item);
            Rlog.i(TAG, "getProvisionedValue(" + item + ") : " + result +
                    " on phone" + mPhoneId + " from binder pid " + Binder.getCallingPid() +
                    ", binder uid " + Binder.getCallingUid() + ", process pid " + Process.myPid() +
                    ", process uid " + Process.myUid());

            return result;
        } catch (RemoteException e) {
            Rlog.e(TAG, "getProvisionedValue(" + item + ") remote failed!");
            throw new RuntimeException(e);
        }
    }

    /**
     * Gets the value for ims service/capabilities parameters from the provisioned
     * value storage. Synchronous blocking call.
     *
     * @param item, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @return value in String format.
     */
    @Override
    public String getProvisionedStringValue(int item) {
        try {
            String result = mImsConfig.getProvisionedStringValue(item);
            Rlog.i(TAG, "getProvisionedStringValue(" + item + ") : " + result +
                    " on phone " + mPhoneId + " from binder pid " + Binder.getCallingPid() +
                    ", binder uid " + Binder.getCallingUid() + ", process pid " + Process.myPid() +
                    ", process uid " + Process.myUid());

            return result;
        } catch (RemoteException e) {
            Rlog.e(TAG, "getProvisionedStringValue(" + item + ") remote failed!");
            throw new RuntimeException(e);
        }
    }

    /**
     * Sets the value for IMS service/capabilities parameters by the operator device
     * management entity. It sets the config item value in the provisioned storage
     * from which the master value is derived. Synchronous blocking call.
     *
     * @param item, as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @param value in Integer format.
     * @return as defined in com.android.ims.ImsConfig#OperationStatusConstants.
     */
    @Override
    public int setProvisionedValue(int item, int value) {
        try {
            return mImsConfig.setProvisionedValue(item, value);
        } catch (RemoteException e) {
            Rlog.e(TAG, "setProvisionedValue(" + item + ") remote failed!");
            return ImsConfig.OperationStatusConstants.FAILED;
        }
    }

    /**
     * Sets the value for IMS service/capabilities parameters by the operator device
     * management entity. It sets the config item value in the provisioned storage
     * from which the master value is derived.  Synchronous blocking call.
     *
     * @param item as defined in com.android.ims.ImsConfig#ConfigConstants.
     * @param value in String format.
     * @return as defined in com.android.ims.ImsConfig#OperationStatusConstants.
     */
    @Override
    public int setProvisionedStringValue(int item, String value) {
        try {
            return mImsConfig.setProvisionedStringValue(item, value);
        } catch (RemoteException e) {
            Rlog.e(TAG, "setProvisionedValue(" + item + ") remote failed!");
            return ImsConfig.OperationStatusConstants.FAILED;
        }
    }

    /**
     * Gets the value of the specified IMS feature item for specified network type.
     * This operation gets the feature config value from the master storage (i.e. final
     * value). Asynchronous non-blocking call.
     *
     * @param feature as defined in com.android.ims.ImsConfig#FeatureConstants.
     * @param network as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @param listener feature value returned asynchronously through listener.
     */
    @Override
    public void getFeatureValue(int feature, int network, ImsConfigListener listener) {
        try {
            Rlog.i(TAG, "getFeatureValue(" + feature + ", " + network + ") : on phone " + mPhoneId);
            mImsConfig.getFeatureValue(feature, network, listener);
        } catch (RemoteException e) {
            Rlog.e(TAG, "getFeatureValue(" + feature + ") remote failed!");
            throw new RuntimeException(e);
        }
    }

    /**
     * Sets the value for IMS feature item for specified network type.
     * This operation stores the user setting in setting db from which master db
     * is derived.
     *
     * @param feature as defined in com.android.ims.ImsConfig#FeatureConstants.
     * @param network as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @param value as defined in com.android.ims.ImsConfig#FeatureValueConstants.
     * @param listener, provided if caller needs to be notified for set result.
     */
    @Override
    public void setFeatureValue(int feature, int network, int value, ImsConfigListener listener) {
        try {
            mImsConfig.setFeatureValue(feature, network, value, listener);
        } catch (RemoteException e) {
            Rlog.e(TAG, "setFeatureValue(" + feature + ") remote failed!");
            throw new RuntimeException(e);
        }
    }

    /**
     * Sets the value for IMS feature item for specified network type.
     * This operation stores the user setting in setting db from which master db
     * is derived.
     *
     * @param feature array as defined in com.android.ims.ImsConfig#FeatureConstants.
     * @param network array as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @param value array as defined in com.android.ims.ImsConfig#FeatureValueConstants.
     * @param listener, provided if caller needs to be notified for set result.
     */
    @Override
    public void setMultiFeatureValues(int[] feature, int[] network, int[] value, ImsConfigListener listener) {
        try {
            try {
                if(ImsCommonUtil.supportMims() == false) {
                    if (ImsCommonUtil.getMainCapabilityPhoneId() != mPhoneId) {
                        Rlog.i(TAG, "setFeatureValue is not allow on non main capability phoneId:" +
                                mPhoneId + " in non MIMS project");
                        throw new ImsException("Do not setFeatureValue for non MIMS not main" +
                                               " capability phoneId: " + mPhoneId,
                            ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
                    }
                }
            } catch (ImsException e) {
                Rlog.e(TAG, "setMultiFeatureValues failed, code: " + e.getCode());
                if (listener != null) {
                    // Return OFF if failed
                    for (int i = 0; i < feature.length; i++) {
                        listener.onSetFeatureResponse(
                            feature[i], network[i], ImsConfig.FeatureValueConstants.OFF,
                            ImsConfig.OperationStatusConstants.FAILED);
                    }
                }
                return;
            }
        } catch (RemoteException e) {
            Rlog.e(TAG, "setMultiFeatureValues onSetFeatureResponse remote failed!");
            throw new RuntimeException(e);
        }

        for (int i = 0; i < feature.length; i++) {
            try {
                try {
                    // 93 after logic
                    if (ImsCommonUtil.supportMdAutoSetupIms()) {
                        int isLast;

                        if (i == feature.length - 1) {
                            isLast = ImsConfigAdapter.ISLAST_TRUE;
                        } else {
                            isLast = ImsConfigAdapter.ISLAST_FALSE;
                        }

                        // Check dynamic IMS switch is support this feature or not.
                        if (value[i] == ImsConfig.FeatureValueConstants.ON) {
                            if ("1".equals(SystemProperties.get(
                                    "persist.vendor.mtk_dynamic_ims_switch"))) {
                                int resCap = mConfigAdapter.getImsResCapability(feature[i]);

                                if (resCap != ImsConfig.FeatureValueConstants.ON) {
                                    Rlog.i(TAG,
                                    "setMultiFeatureValues, modify the value in ImsConfig.");
                                    value[i] = ImsConfig.FeatureValueConstants.OFF;
                                }
                            }
                        }

                        Rlog.i(TAG, "setMultiFeatureValues i:" + i + " feature: " + feature[i] +
                                " network: " + network[i] + " value: " + value[i] +
                                " isLast: " + isLast);

                        mConfigAdapter.mController.setFeatureValue(feature[i], network[i],
                                value[i], isLast);
                    } else {
                        // Ims ctrl old logic with WOS.
                        mConfigAdapter.mStorage.setFeatureValue(feature[i], network[i], value[i]);

                        switch(feature[i]) {
                            case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE:
                                int oldViLTEValue = ImsConfigUtils.getFeaturePropValue(
                                        ImsConfigUtils.PROPERTY_VILTE_ENALBE, mPhoneId);
                                if (value[i] != oldViLTEValue) {
                                    if (value[i] == ImsConfig.FeatureValueConstants.ON) {
                                        mRilAdapter.turnOnVilte(null);
                                    } else {
                                        mRilAdapter.turnOffVilte(null);
                                    }
                                }
                                break;
                            case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI:
                                int oldViWifiValue = ImsConfigUtils.getFeaturePropValue(
                                        ImsConfigUtils.PROPERTY_VIWIFI_ENALBE, mPhoneId);
                                if (value[i] != oldViWifiValue) {
                                    if (value[i] == ImsConfig.FeatureValueConstants.ON) {
                                        mRilAdapter.turnOnViwifi(null);
                                    } else {
                                        mRilAdapter.turnOffViwifi(null);
                                    }
                                }
                                break;
                            case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI:
                                int oldWfcValue = ImsConfigUtils.getFeaturePropValue(
                                        ImsConfigUtils.PROPERTY_WFC_ENALBE, mPhoneId);
                                if (value[i] != oldWfcValue) {
                                    if (value[i] == ImsConfig.FeatureValueConstants.ON) {
                                        mRilAdapter.turnOnWfc(null);
                                    } else {
                                        mRilAdapter.turnOffWfc(null);
                                    }
                                }
                                break;
                            case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE:
                                int oldVoLTEValue = ImsConfigUtils.getFeaturePropValue(
                                        ImsConfigUtils.PROPERTY_VOLTE_ENALBE, mPhoneId);
                                if (value[i] != oldVoLTEValue) {
                                    if (value[i] == ImsConfig.FeatureValueConstants.ON) {
                                        mRilAdapter.turnOnVolte(null);
                                    } else {
                                        mRilAdapter.turnOffVolte(null);
                                    }
                                }
                                break;
                            default:
                                break;
                        }
                    }
                    if (listener != null) {
                        listener.onSetFeatureResponse(
                                feature[i], network[i], value[i], ImsConfig.OperationStatusConstants.SUCCESS);
                    }
                } catch (ImsException e) {
                    Rlog.e(TAG, "setFeatureValue(" + feature[i] + ") failed, code: " + e.getCode());
                    if (listener != null) {
                        // Return OFF if failed
                        listener.onSetFeatureResponse(
                            feature[i], network[i], ImsConfig.FeatureValueConstants.OFF,
                            ImsConfig.OperationStatusConstants.FAILED);
                    }
                }
            } catch (RemoteException e) {
                Rlog.e(TAG, "setMultiFeatureValues onSetFeatureResponse remote failed!");
                throw new RuntimeException(e);
            }
        }
    }

    /**
     * Gets the value for IMS feature item for video call quality.
     *
     * @param listener, provided if caller needs to be notified for set result.
     * @return void
     */
    @Override
    public void getVideoQuality(ImsConfigListener listener) {

    }

    /**
     * Sets the value for IMS feature item video quality.
     *
     * @param quality, defines the value of video quality.
     * @param listener, provided if caller needs to be notified for set result.
     * @return void
     */
    @Override
    public void setVideoQuality(int quality, ImsConfigListener listener) {

    }

    @Override
    public void setImsResCapability(int feature, int value) {
        // Still keep old design, store res value in runtime
        mImsCapabilities.put(feature, value);
        // Use cache to reduce access DB frequently
        mImsCapabilitiesIsCache.put(feature, true);

        // Resource value will be stored in ImsConfig DB
        try {
            if (DEBUG) Rlog.i(TAG, "setImsResCapability(" + feature + ") : " + value + " on phone " + mPhoneId +
                    " from binder pid " + Binder.getCallingPid() +
                    ", binder uid " + Binder.getCallingUid());
            mConfigAdapter.setImsResCapability(feature, value);
        } catch (ImsException e) {
            Rlog.e(TAG, "setImsResCapability(" + feature + ") failed, code: " + e.getCode());
        }
    }

    @Override
    public int getImsResCapability(int feature) {
        try {
            int value;

            if (mImsCapabilitiesIsCache.get(feature)) {
                value = mImsCapabilities.get(feature);
            } else {
                value = mConfigAdapter.getImsResCapability(feature);
            }

            if (value != ImsConfig.FeatureValueConstants.ON &&
                    value != ImsConfig.FeatureValueConstants.OFF) {
                throw new ImsException(" result value:" + value + " incorrect!",
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
            }

            return value;
        } catch (ImsException e) {
            Rlog.e(TAG, "getImsResCapability(" + feature + ") failed, code: " + e.getCode());

            // no value return old design's default value, volte/vilte/wfc:[1/0/0]
            return mImsCapabilities.get(feature);
        }
    }

    @Override
    public void setWfcMode(int mode) {
        Rlog.i(TAG, "setWfcMode(" + mode + ")");

        // Default value of wfc mode is wifi prefer
        // ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED is mapping to 1 in MTK design
        int rilWfcMode = 1;

        switch (mode) {
            case ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY:
                rilWfcMode = 3;
                break;
            case ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED:
                rilWfcMode = 2;
                break;
            case ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED:
                rilWfcMode = 1;
                break;
            default:
                Rlog.i(TAG, "setWfcMode mapping error, value is invalid!");
                break;
        }
        mConfigAdapter.sendWfcProfileInfo(rilWfcMode);
    }

    /**
     * Sync Volte preference for voice domain to modem.
     * @param Volte preference mode.
     * @return void.
     */
    @Override
    public void setVoltePreference(int mode) {
        mConfigAdapter.setVoltePreference(mode);
    }

    /**
     * Update multiple Ims config to modem.
     * @param keys as multiple Ims config keys.
     * @param values as multiple Ims config values.
     * @return value in int array format (success is 0, fail is -1)
     */
    @Override
    public int[] setModemImsCfg(String[] keys, String[] values, int phoneId) {
        int[] result = null;
        int type = ImsConfigUtils.MdConfigType.TYPE_IMSCFG;

        Rlog.i(TAG, "setModemImsCfg phoneId:" + phoneId);
        result = mConfigAdapter.setModemImsCfg(keys, values, type);

        return result;
    }

    /**
     * Update multiple Ims WO config to modem.
     * @param keys as multiple Ims WO config keys.
     * @param values as multiple Ims IWLAN config values.
     * @return value in int array format (success is 0, fail is -1)
     */
    @Override
    public int[] setModemImsWoCfg(String[] keys, String[] values, int phoneId) {
        int[] result = null;
        int type = ImsConfigUtils.MdConfigType.TYPE_IMSWOCFG;

        Rlog.i(TAG, "setModemImsWoCfg phoneId:" + phoneId);
        result = mConfigAdapter.setModemImsCfg(keys, values, type);

        return result;
    }

    /**
     * Update multiple Ims IWLAN config to modem.
     * @param keys as multiple Ims IWLAN config keys.
     * @param values as multiple Ims IWLAN config values.
     * @return value in int array format (success is 0, fail is -1)
     */
    @Override
    public int[] setModemImsIwlanCfg(String[] keys, String[] values, int phoneId) {
        int[] result = null;
        int type = ImsConfigUtils.MdConfigType.TYPE_IMSIWLANCFG;

        Rlog.i(TAG, "setModemImsIwlanCfg phoneId:" + phoneId);
        result = mConfigAdapter.setModemImsCfg(keys, values, type);

        return result;
    }
}
