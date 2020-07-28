package com.mediatek.ims.config.internal;

import android.content.Context;
import android.os.Build;
import android.os.SystemProperties;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;

import com.mediatek.ims.ImsCommonUtil;
import com.mediatek.ims.ril.ImsCommandsInterface;

public class ImsConfigAdapter {
    private static final String TAG = "ImsConfigAdapter";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean DEBUG = TextUtils.equals(Build.TYPE, "eng")
            || (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);


    ImsConfigStorage mStorage;
    ImsConfigController mController;

    private String mLogTag;

    private int mPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;

    private Context mContext = null;
    private ImsConfigAdapter() {}

    // isLast value of setMultiFeatureValues
    public static final int ISLAST_NULL = -1;
    public static final int ISLAST_FALSE = 0;
    public static final int ISLAST_TRUE =  1;

    public ImsConfigAdapter(Context context, ImsCommandsInterface imsRilAdapter, int phoneId) {
        mContext = context;
        mPhoneId = phoneId;

        mLogTag = TAG + "[" + phoneId + "]";

        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            mController = new ImsConfigController(context, phoneId, imsRilAdapter);
        } else {
            mStorage = new ImsConfigStorage(context, phoneId, imsRilAdapter);
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

        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            return mController.getFeatureValue(featureId, network);
        } else {
            return mStorage.getFeatureValue(featureId, network);
        }
    }

    /**
     * Sets the value for IMS feature item for specified network type.
     *
     * @param featureId, as defined in FeatureConstants.
     * @param network, as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
     * @param value, as defined in FeatureValueConstants.
     * @param isLast, is last command of set multiple feature values.
     * @return void
     *
     * @throws ImsException if calling the IMS service results in an error.
     */
    public void setFeatureValue(int featureId, int network, int value, int isLast)
            throws ImsException {
        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            mController.setFeatureValue(featureId, network, value, isLast);
        } else {
            mStorage.setFeatureValue(featureId, network, value);
        }
    }

    public void setProvisionedValue(int configId, int value) throws ImsException {
        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            mController.setProvisionedValue(configId, String.valueOf(value));
        } else {
            mStorage.setProvisionedValue(configId, value);
        }
    }

    public void setProvisionedStringValue(int configId, String value)
            throws ImsException {
        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            mController.setProvisionedValue(configId, value);
        } else {
            mStorage.setProvisionedStringValue(configId, value);
        }
    }

    public int getProvisionedValue(int configId) throws ImsException {
        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            int value = -1;
            String stringValue = mController.getProvisionedValue(configId);
            try {
                value = Integer.parseInt(stringValue);
            } catch (NumberFormatException e) {
                throw new ImsException("getProvisionedValue wrong, reason: return string while "
                        + "expecting int, result:" + stringValue, ImsReasonInfo
                        .CODE_LOCAL_ILLEGAL_ARGUMENT);

            }
            return value;
        } else {
            return mStorage.getProvisionedValue(configId);
        }
    }

    public void setImsResCapability(int featureId, int value)
            throws ImsException {
        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            Rlog.e(mLogTag, "setImsResCapability not supported!");
        } else {
            mStorage.setImsResCapability(featureId, value);
        }
    }

    public int getImsResCapability(int featureId)
            throws ImsException {
        // Currently, the dynamic IMS swicth config of ViWifi is follow ViLTE.
        if (featureId == ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI) {
            featureId = ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE;
        }

        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            return mController.getImsResCapability(featureId);
        } else {
            return mStorage.getImsResCapability(featureId);
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
        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            return mController.getProvisionedValue(configId);
        } else {
            return mStorage.getProvisionedStringValue(configId);
        }
    }

    /**
     * Sync WFC preferred mode to modem.
     */
    public void sendWfcProfileInfo(int rilWfcMode) {
        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            mController.sendWfcProfileInfo(rilWfcMode);
        } else {
            mStorage.sendWfcProfileInfo(rilWfcMode);
        }
    }

    /**
     * Sync Volte preference for voice domain to modem.
     * @param Volte preference mode.
     * @return void.
     */
    public void setVoltePreference(int mode) {
        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            mController.setVoltePreference(mode);
        } else {
            mStorage.setVoltePreference(mode);
        }
    }

    public int[] setModemImsCfg(String[] keys, String[] values, int type) {
        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            return mController.setModemImsCfg(keys, values, type);
        } else {
            return mStorage.setModemImsCfg(keys, values, type);
        }
    }
}
