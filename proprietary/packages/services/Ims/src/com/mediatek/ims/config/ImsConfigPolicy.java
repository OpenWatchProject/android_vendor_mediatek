package com.mediatek.ims.config;

import static com.mediatek.ims.config.ImsConfigSettings.Setting;

import java.util.HashMap;
import java.util.Map;

/**
 * Abstract class for fetching default value for each IMS configurations.
 */
public abstract class ImsConfigPolicy {
    private HashMap<Integer, DefaultConfig> mDefConfigs =
            new HashMap<Integer, DefaultConfig>();
    private String mTag = "ImsConfigPolicy";
    public ImsConfigPolicy(String tag) {
        mTag = tag;
    }

    /**
     * Call this abstract function when loading each IMS configurations with its config ID.
     *
     * @param configId The IMS config id defined ImsConfig.ConfigConstants.
     * @param config The config item expected override its default value and unit if need.
     * @return Return true is this config is handled.
     */
    public abstract boolean onSetDefaultValue(int configId, DefaultConfig config);

    HashMap<Integer, DefaultConfig> fetchDefaultValues() {
        boolean handled = false;
        Map<Integer, Setting> settings = ImsConfigSettings.getConfigSettings();
        for (Integer configId : settings.keySet()) {
            DefaultConfig config = new DefaultConfig(configId);
            handled = onSetDefaultValue(configId, config);
            if (handled) {
                if (!ImsConfigContract.Validator.isValidUnitId(config.unitId)) {
                    throw new IllegalArgumentException("Invalid unitId " + config.unitId +
                            " on config " + configId);
                }
                mDefConfigs.put(configId, config);
            }
        }
        return mDefConfigs;
    }

    public static class DefaultConfig {
        int configId = 0;
        public String defVal = null;
        public int unitId = ImsConfigContract.Unit.UNIT_NONE;
        private DefaultConfig() {};
        public DefaultConfig(int _configId) {
            configId = _configId;
        };
    }
}
