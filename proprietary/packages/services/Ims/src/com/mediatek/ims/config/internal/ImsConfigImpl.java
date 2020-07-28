package com.mediatek.ims.config.internal;

import android.content.Context;
import android.os.Binder;
import android.os.Build;
import android.os.Process;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.text.TextUtils;
//import android.telephony.ims.stub.ImsConfigImplBase;
import android.telephony.ims.compat.stub.ImsConfigImplBase;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsConfigListener;
import com.android.ims.ImsException;
import com.android.ims.internal.IImsConfig;

import com.android.internal.telephony.IccCardConstants;

import com.mediatek.ims.ImsCommonUtil;
import com.mediatek.ims.ril.ImsCommandsInterface;

import com.mediatek.ims.plugin.ExtensionFactory;
import com.mediatek.ims.plugin.OemPluginFactory;
import com.mediatek.ims.plugin.ImsCallOemPlugin;

import java.util.HashMap;

/**
 * Class handles IMS parameter provisioning by carrier.
 *
 *  @hide
 */
public class ImsConfigImpl extends ImsConfigImplBase {
    private static final String TAG = "ImsConfigImpl";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean DEBUG = TextUtils.equals(Build.TYPE, "eng")
            || (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);
    /// M: Sync volte setting value. @{
    private static final String PROPERTY_IMSCONFIG_FORCE_NOTIFY =
                                    "vendor.ril.imsconfig.force.notify";
    /// @}

    private Context mContext;
    private int mPhoneId;
    private ImsCommandsInterface mRilAdapter;
    private ImsConfigAdapter mConfigAdapter = null;
    private ImsCallOemPlugin mImsCallOemPlugin = null;
    private String mLogTag;

    /**
     *
     * Construction function for ImsConfigImpl.
     *
     * @param context the application context
     * @param phoneId the phone id this instance handle for
     *
     */
    public ImsConfigImpl(Context context, ImsCommandsInterface imsRilAdapter,
                         ImsConfigAdapter configAdapter, int phoneId) {

        super(context);

        mContext = context;
        mPhoneId = phoneId;
        mRilAdapter = imsRilAdapter;
        mConfigAdapter = configAdapter;
        mLogTag = TAG + "[" + phoneId + "]";
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
            int result = mConfigAdapter.getProvisionedValue(item);
            Rlog.i(mLogTag, "getProvisionedValue(" + item + ") : " + result +
                    " on phone" + mPhoneId + " from binder pid " + Binder.getCallingPid() +
                    ", binder uid " + Binder.getCallingUid() + ", process pid " + Process.myPid() +
                    ", process uid " + Process.myUid());
            return result;
        } catch (ImsException e) {
            Rlog.e(mLogTag, "getProvisionedValue(" + item + ") failed, code: " + e.getCode());

            if (Binder.getCallingPid() == Process.myPid()) {
                return 0;
            } else {
                return -1;
            }
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
            String result = mConfigAdapter.getProvisionedStringValue(item);
            Rlog.i(mLogTag, "getProvisionedStringValue(" + item + ") : " + result +
                    " on phone " + mPhoneId + " from binder pid " + Binder.getCallingPid() +
                    ", binder uid " + Binder.getCallingUid() + ", process pid " + Process.myPid() +
                    ", process uid " + Process.myUid());
            return result;
        } catch (ImsException e) {
            Rlog.e(mLogTag, "getProvisionedStringValue(" + item + ") failed, code: " + e.getCode());

            if (Binder.getCallingPid() == Process.myPid()) {
                return "Unknown";
            } else {
                return null;
            }
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
            mConfigAdapter.setProvisionedValue(item, value);
            if (DEBUG) {
                Rlog.i(mLogTag, "setProvisionedValue(" + item + ", " + value +
                        ") on phone " + mPhoneId + " from pid " + Binder.getCallingPid() +
                        ", uid " + Binder.getCallingUid() + " ,retVal:" +
                        ImsConfig.OperationStatusConstants.SUCCESS);
            }
            return ImsConfig.OperationStatusConstants.SUCCESS;
        } catch (ImsException e) {
            Rlog.e(mLogTag, "setProvisionedValue(" + item + ") failed, code: " + e.getCode());
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
            mConfigAdapter.setProvisionedStringValue(item, value);
            Rlog.i(mLogTag, "setProvisionedStringValue(" + item + ", " + value +
                    ") on phone " + mPhoneId + " from pid " + Binder.getCallingPid() +
                    ", uid " + Binder.getCallingUid() + " ,retVal:" +
                    ImsConfig.OperationStatusConstants.SUCCESS);
            return ImsConfig.OperationStatusConstants.SUCCESS;
        } catch (ImsException e) {
            Rlog.e(mLogTag, "setProvisionedValue(" + item + ") failed, code: " + e.getCode());
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
            try {
                int value = mConfigAdapter.getFeatureValue(feature, network);
                Rlog.i(mLogTag, "getFeatureValue(" + feature + ", " + network + ") : " + value +
                        " on phone " + mPhoneId);
                listener.onGetFeatureResponse(
                        feature, network, value, ImsConfig.OperationStatusConstants.SUCCESS);
            } catch (ImsException e) {
                Rlog.e(mLogTag, "getFeatureValue(" + feature + ") failed, code: " + e.getCode());
                // Return OFF if failed
                listener.onGetFeatureResponse(
                        feature, network, ImsConfig.FeatureValueConstants.OFF,
                        ImsConfig.OperationStatusConstants.FAILED);
            }
        } catch (RemoteException e) {
            Rlog.e(mLogTag, "getFeatureValue(" + feature + ") remote failed!");
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
            try {
                if (DEBUG) {
                    Rlog.i(mLogTag, "setFeatureValue(" + feature + ", " + network + ", " + value +
                          ") on phone " + mPhoneId + " from pid " + Binder.getCallingPid() +
                          ", uid " + Binder.getCallingUid() + ", listener " + listener);
                }

                if ((feature == ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE) ||
                    (feature == ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI)) {
                    Rlog.i(mLogTag, "setFeatureValue is not support UT currently.");
                    throw new ImsException("setFeatureValue is not support UT currently.",
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
                }

                if(ImsCommonUtil.supportMims() == false) {
                    if (ImsCommonUtil.getMainCapabilityPhoneId() != mPhoneId) {
                        Rlog.i(mLogTag, "setFeatureValue is not allow on non main capability phoneId:" +
                                mPhoneId + " in non MIMS project");
                        throw new ImsException("Do not setFeatureValue for non MIMS not main" +
                                               " capability phoneId: " + mPhoneId,
                            ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
                    }
                }

                // Check dynamic IMS switch is support this feature or not.
                if (value == ImsConfig.FeatureValueConstants.ON) {
                    if ("1".equals(SystemProperties.get("persist.vendor.mtk_dynamic_ims_switch"))) {
                        int resCap = mConfigAdapter.getImsResCapability(feature);

                        if (resCap != ImsConfig.FeatureValueConstants.ON &&
                                SystemProperties.getInt(PROPERTY_IMSCONFIG_FORCE_NOTIFY, 0) == 0) {
                            Rlog.i(mLogTag, "setFeatureValue, modify the value in ImsConfig.");
                            value = ImsConfig.FeatureValueConstants.OFF;
                        }
                    }
                }

                mConfigAdapter.setFeatureValue(feature, network, value, ImsConfigAdapter.ISLAST_NULL);

                if (mImsCallOemPlugin == null) {
                    mImsCallOemPlugin = ExtensionFactory.makeOemPluginFactory(mContext)
                            .makeImsCallPlugin(mContext);
                }

                if (mImsCallOemPlugin.isUpdateViwifiFeatureValueAsViLTE()
                    && ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE == feature) {
                    mConfigAdapter.setFeatureValue(
                            ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI,
                            ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN,
                            value,
                            ImsConfigAdapter.ISLAST_NULL);
                }

                // 92 logic
                if (!ImsCommonUtil.supportMdAutoSetupIms()) {
                    // Ims ctrl old logic with WOS.
                    switch(feature) {
                        case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE:
                            int oldViLTEValue = ImsConfigUtils.getFeaturePropValue(
                                    ImsConfigUtils.PROPERTY_VILTE_ENALBE, mPhoneId);
                            if (value != oldViLTEValue) {
                                if (value == ImsConfig.FeatureValueConstants.ON) {
                                    mRilAdapter.turnOnVilte(null);
                                    if (mImsCallOemPlugin.isUpdateViwifiFeatureValueAsViLTE()) {
                                        mRilAdapter.turnOnViwifi(null);
                                    }
                                } else {
                                    mRilAdapter.turnOffVilte(null);
                                    if (mImsCallOemPlugin.isUpdateViwifiFeatureValueAsViLTE()) {
                                        mRilAdapter.turnOffViwifi(null);
                                    }
                                }
                            }
                            break;
                        case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI:
                            int oldViWifiValue = ImsConfigUtils.getFeaturePropValue(
                                    ImsConfigUtils.PROPERTY_VIWIFI_ENALBE, mPhoneId);
                            if (value != oldViWifiValue) {
                                if (value == ImsConfig.FeatureValueConstants.ON) {
                                    mRilAdapter.turnOnViwifi(null);
                                } else {
                                    mRilAdapter.turnOffViwifi(null);
                                }
                            }
                            break;
                        case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI:
                            int oldWfcValue = ImsConfigUtils.getFeaturePropValue(
                                    ImsConfigUtils.PROPERTY_WFC_ENALBE, mPhoneId);
                            if (value != oldWfcValue) {
                                if (value == ImsConfig.FeatureValueConstants.ON) {
                                    mRilAdapter.turnOnWfc(null);
                                } else {
                                    mRilAdapter.turnOffWfc(null);
                                }
                            }
                            break;
                        case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE:
                            int oldVoLTEValue = ImsConfigUtils.getFeaturePropValue(
                                    ImsConfigUtils.PROPERTY_VOLTE_ENALBE, mPhoneId);
                            if (value != oldVoLTEValue) {
                                if (value == ImsConfig.FeatureValueConstants.ON) {
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
                            feature, network, value, ImsConfig.OperationStatusConstants.SUCCESS);
                }
            } catch (ImsException e) {
                Rlog.e(mLogTag, "setFeatureValue(" + feature + ") failed, code: " + e.getCode());
                if (listener != null) {
                    // Return OFF if failed
                    listener.onSetFeatureResponse(
                            feature, network, ImsConfig.FeatureValueConstants.OFF,
                            ImsConfig.OperationStatusConstants.FAILED);
                }
            }
        } catch (RemoteException e) {
            Rlog.e(mLogTag, "setFeatureValue(" + feature + ") remote failed!");
            throw new RuntimeException(e);
        }
    }

    /**
     * Gets the value for IMS volte provisioned.
     * This should be the same as the operator provisioned value if applies.
     *
     * @return boolean
     */
    @Override
    public boolean getVolteProvisioned() {
        // Move caching of VoLTE provisioned value from ImsConfigImpl to ImsManager
        return true;
    }
}
