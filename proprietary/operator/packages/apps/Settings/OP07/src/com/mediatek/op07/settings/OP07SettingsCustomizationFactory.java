package com.mediatek.op07.settings;

import android.content.Context;

import com.mediatek.settings.ext.IApnSettingsExt;
import com.mediatek.settings.ext.IMobileNetworkSettingsExt;
import com.mediatek.settings.ext.ISettingsMiscExt;
import com.mediatek.settings.ext.ISimRoamingExt;
import com.mediatek.settings.ext.IDataUsageSummaryExt;
import com.mediatek.settings.ext.IWfcSettingsExt;
import com.mediatek.settings.ext.OpSettingsCustomizationFactoryBase;

public class OP07SettingsCustomizationFactory extends OpSettingsCustomizationFactoryBase {
    private Context mContext;

    public OP07SettingsCustomizationFactory(Context context) {
        super(context);
        mContext = context;
    }

    public ISimRoamingExt makeSimSimRoamingExt() {
        return new OP07SimRoamingExt(mContext);
    }

    /**
     * Api to make dataUsage setting plugin .
     * @return IDataUsageSummaryExt
     */
    @Override
    public IDataUsageSummaryExt makeDataUsageSummaryExt() {
        return new OP07DataUsageSummaryExt(mContext);
    }

    @Override
    public ISettingsMiscExt makeSettingsMiscExt(Context context) {
        return new Op07SettingsMiscExt(mContext);
    }

    @Override
    public IApnSettingsExt makeApnSettingsExt(Context context) {
        return new Op07ApnSettingsExt(mContext);
    }

    @Override
    public IMobileNetworkSettingsExt makeMobileNetworkSettingsExt(
            Context context) {
        return new OP07MobileNetworkSettingsExt(mContext);
    }

    @Override
    public IWfcSettingsExt makeWfcSettingsExt() {
        return new Op07WfcSettingsExt(mContext);
    }
}
