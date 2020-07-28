package com.mediatek.ims.config;

import android.os.SystemProperties;
import android.util.Log;

import com.mediatek.ims.config.op.*;

import java.util.HashMap;

/**
 * Factory class to load default IMS configuration for operator.
 * Provide specific operator flavor or selection automatically according to current device status.
 */
public class DefaultConfigPolicyFactory {
    private static final String TAG = "DefaultCfgPolicyFactory";
    private HashMap<Integer, ImsConfigPolicy.DefaultConfig> mDefConfigs =
            new HashMap<Integer, ImsConfigPolicy.DefaultConfig>();
    private DefaultConfigPolicyFactory() {};
    private ImsConfigPolicy mConfigPolicy = null;

    public static DefaultConfigPolicyFactory getInstance(int phoneId) {
        String optr = SystemProperties.get("persist.vendor.operator.optr");
        int opCode = ImsConfigContract.Operator.OP_DEFAULT;
        // Skip prefix "OP"
        if (optr != null && optr.length() > 2) {
            opCode = Integer.parseInt(optr.substring(2));
        }
        return getInstanceByOpCode(opCode);
    }

    public static DefaultConfigPolicyFactory getInstanceByOpCode(int opCode) {
        return new DefaultConfigPolicyFactory(opCode);
    }

    private DefaultConfigPolicyFactory(int op) {
        Log.d(TAG, "Load defalut policy operator: " + op);
        switch (op) {
            case ImsConfigContract.Operator.OP_06:
                mConfigPolicy = new Op06ConfigPolicy();
                break;
            case ImsConfigContract.Operator.OP_08:
                mConfigPolicy = new Op08ConfigPolicy();
                break;
            case ImsConfigContract.Operator.OP_12:
                mConfigPolicy = new Op12ConfigPolicy();
                break;
            default:
                mConfigPolicy = new DefaultConfigPolicy();
        }
    }

    public HashMap load() {
        if (mConfigPolicy != null) {
            mDefConfigs = mConfigPolicy.fetchDefaultValues();
        }
        return mDefConfigs;
    }

    public void clear() {
        mDefConfigs = null;
    }

    public boolean hasDefaultValue(int configId) {
        return mDefConfigs.containsKey(configId);
    }
}
